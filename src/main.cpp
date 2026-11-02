#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
using namespace geode::prelude;

static int g_lastClick = -1;

class $modify(WindowGBL, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
        if (down && PlayLayer::get()) {
            g_lastClick = m_gameState.m_currentProgress;
        }
    }
};

class $modify(WindowPL, PlayLayer) {
    struct Fields {
        CCLabelBMFont* label = nullptr;
        CCDrawNode* draw = nullptr;
        float startX = -1.f;
        float endX = -1.f;
        bool gotStart = false;
        bool gotEnd = false;
        int lastStart = -1;
        int lastLen = -1;
        bool drawn = false;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        auto label = CCLabelBMFont::create("", "bigFont.fnt");
        label->setScale(0.4f);
        label->setOpacity(190);
        label->setAnchorPoint({0.f, 1.f});
        label->setAlignment(kCCTextAlignmentLeft);
        auto h = CCDirector::get()->getWinSize().height;
        label->setPosition({6.f, h - 6.f});
        m_uiLayer->addChild(label, 100);
        m_fields->label = label;

        auto draw = CCDrawNode::create();
        m_objectLayer->addChild(draw, 1000);
        m_fields->draw = draw;
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        g_lastClick = -1;
        m_fields->gotStart = false;
        m_fields->gotEnd = false;
    }

    void drawWindow() {
        auto f = m_fields.self();
        f->draw->clear();
        float a = f->startX;
        float b = std::max(f->endX, f->startX + 1.f);
        float top = 4000.f;
        ccColor4F fill = {1.f, 0.f, 0.f, 0.25f};
        ccColor4F line = {1.f, 0.f, 0.f, 1.f};

        CCPoint fillPts[4] = {{a, 0.f}, {b, 0.f}, {b, top}, {a, top}};
        f->draw->drawPolygon(fillPts, 4, fill, 0.f, fill);

        CCPoint l1[4] = {{a - 1.f, 0.f}, {a + 1.f, 0.f}, {a + 1.f, top}, {a - 1.f, top}};
        CCPoint l2[4] = {{b - 1.f, 0.f}, {b + 1.f, 0.f}, {b + 1.f, top}, {b - 1.f, top}};
        f->draw->drawPolygon(l1, 4, line, 0.f, line);
        f->draw->drawPolygon(l2, 4, line, 0.f, line);
        f->drawn = true;
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        auto f = m_fields.self();
        if (!f->label) return;

        int frame = m_gameState.m_currentProgress;
        int start = (int)Mod::get()->getSettingValue<int64_t>("window-start");
        int len = (int)Mod::get()->getSettingValue<int64_t>("window-length");
        int end = start + len;

        if (start != f->lastStart || len != f->lastLen) {
            f->lastStart = start;
            f->lastLen = len;
            f->startX = -1.f;
            f->endX = -1.f;
            f->drawn = false;
            f->draw->clear();
        }

        if (!f->gotStart && frame >= start) {
            f->startX = m_player1->getPositionX();
            f->gotStart = true;
            f->drawn = false;
        }
        if (!f->gotEnd && frame >= end) {
            f->endX = m_player1->getPositionX();
            f->gotEnd = true;
            f->drawn = false;
        }
        if (!f->drawn && f->startX >= 0.f && f->endX >= 0.f) {
            drawWindow();
        }

        std::string win;
        if (frame < start) win = fmt::format("Window opens in {}", start - frame);
        else if (frame < end) win = fmt::format("OPEN - {} left", end - frame);
        else win = "CLOSED";

        std::string click = "Click: none";
        if (g_lastClick >= 0) {
            int off = g_lastClick - start;
            bool inside = g_lastClick >= start && g_lastClick < end;
            click = fmt::format("Click: {} ({}{}) {}",
                g_lastClick, off >= 0 ? "+" : "", off,
                inside ? "IN" : "MISS");
        }

        f->label->setString(
            fmt::format("Frame: {}\n{}\n{}", frame, win, click).c_str());
    }
};
