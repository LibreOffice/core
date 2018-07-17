/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <Qt5Widget.hxx>
#include <Qt5Widget.moc>

#include <Qt5Frame.hxx>
#include <Qt5Graphics.hxx>
#include <Qt5Tools.hxx>

#include <QtGui/QFocusEvent>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QShowEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QtWidgets>
#include <QtWidgets/QMainWindow>

#include <cairo.h>
#include <headless/svpgdi.hxx>

class VclQtMixinBase
{
public:
    VclQtMixinBase(Qt5Frame* pFrame) { m_pFrame = pFrame; }

    void mixinFocusInEvent(QFocusEvent*);
    void mixinFocusOutEvent(QFocusEvent*);
    void mixinKeyPressEvent(QKeyEvent*);
    void mixinKeyReleaseEvent(QKeyEvent*);
    void mixinMouseMoveEvent(QMouseEvent*);
    void mixinMousePressEvent(QMouseEvent*);
    void mixinMouseReleaseEvent(QMouseEvent*);
    void mixinMoveEvent(QMoveEvent*);
    void mixinPaintEvent(QPaintEvent*, QWidget* widget);
    void mixinResizeEvent(QResizeEvent*, QSize aSize);
    void mixinShowEvent(QShowEvent*);
    void mixinWheelEvent(QWheelEvent*);
    void mixinCloseEvent(QCloseEvent*);

private:
    bool mixinHandleKeyEvent(QKeyEvent*, bool);
    void mixinHandleMouseButtonEvent(QMouseEvent*, bool);

    Qt5Frame* m_pFrame;
};

void VclQtMixinBase::mixinPaintEvent(QPaintEvent* pEvent, QWidget* widget)
{
    QPainter p(widget);
    if (m_pFrame->m_bUseCairo)
    {
        cairo_surface_t* pSurface = m_pFrame->m_pSurface.get();
        cairo_surface_flush(pSurface);

        QImage aImage(cairo_image_surface_get_data(pSurface), widget->size().width(),
                      widget->size().height(), Qt5_DefaultFormat32);
        p.drawImage(pEvent->rect().topLeft(), aImage, pEvent->rect());
    }
    else
        p.drawImage(pEvent->rect().topLeft(), *m_pFrame->m_pQImage, pEvent->rect());
}

void VclQtMixinBase::mixinResizeEvent(QResizeEvent*, QSize aSize)
{
    if (m_pFrame->m_bUseCairo)
    {
        int width = aSize.width();
        int height = aSize.height();
        cairo_surface_t* pSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
        cairo_surface_set_user_data(pSurface, SvpSalGraphics::getDamageKey(),
                                    &m_pFrame->m_aDamageHandler, nullptr);
        m_pFrame->m_pSvpGraphics->setSurface(pSurface, basegfx::B2IVector(width, height));
        m_pFrame->m_pSurface.reset(pSurface);
    }
    else
    {
        QImage* pImage = new QImage(aSize, Qt5_DefaultFormat32);
        m_pFrame->m_pQt5Graphics->ChangeQImage(pImage);
        m_pFrame->m_pQImage.reset(pImage);
    }

    m_pFrame->maGeometry.nWidth = aSize.width();
    m_pFrame->maGeometry.nHeight = aSize.height();

    m_pFrame->CallCallback(SalEvent::Resize, nullptr);
}

void VclQtMixinBase::mixinHandleMouseButtonEvent(QMouseEvent* pEvent, bool bReleased)
{
    SalMouseEvent aEvent;
    switch (pEvent->button())
    {
        case Qt::LeftButton:
            aEvent.mnButton = MOUSE_LEFT;
            break;
        case Qt::MidButton:
            aEvent.mnButton = MOUSE_MIDDLE;
            break;
        case Qt::RightButton:
            aEvent.mnButton = MOUSE_RIGHT;
            break;
        default:
            return;
    }

    aEvent.mnTime = pEvent->timestamp();
    aEvent.mnX = static_cast<long>(pEvent->pos().x());
    aEvent.mnY = static_cast<long>(pEvent->pos().y());
    aEvent.mnCode = GetKeyModCode(pEvent->modifiers()) | GetMouseModCode(pEvent->buttons());

    SalEvent nEventType;
    if (bReleased)
        nEventType = SalEvent::MouseButtonUp;
    else
        nEventType = SalEvent::MouseButtonDown;
    m_pFrame->CallCallback(nEventType, &aEvent);
}

void VclQtMixinBase::mixinMousePressEvent(QMouseEvent* pEvent)
{
    mixinHandleMouseButtonEvent(pEvent, false);
}

void VclQtMixinBase::mixinMouseReleaseEvent(QMouseEvent* pEvent)
{
    mixinHandleMouseButtonEvent(pEvent, true);
}

void VclQtMixinBase::mixinMouseMoveEvent(QMouseEvent* pEvent)
{
    SalMouseEvent aEvent;
    aEvent.mnTime = pEvent->timestamp();
    aEvent.mnX = pEvent->pos().x();
    aEvent.mnY = pEvent->pos().y();
    aEvent.mnCode = GetKeyModCode(pEvent->modifiers()) | GetMouseModCode(pEvent->buttons());
    aEvent.mnButton = 0;

    m_pFrame->CallCallback(SalEvent::MouseMove, &aEvent);
    pEvent->accept();
}

void VclQtMixinBase::mixinWheelEvent(QWheelEvent* pEvent)
{
    SalWheelMouseEvent aEvent;

    aEvent.mnTime = pEvent->timestamp();
    aEvent.mnX = pEvent->pos().x();
    aEvent.mnY = pEvent->pos().y();
    aEvent.mnCode = GetKeyModCode(pEvent->modifiers()) | GetMouseModCode(pEvent->buttons());

    int nDelta = pEvent->angleDelta().x();
    aEvent.mbHorz = true;
    if (!nDelta)
    {
        nDelta = pEvent->angleDelta().y();
        aEvent.mbHorz = false;
    }
    if (!nDelta)
        return;
    nDelta /= 8;

    aEvent.mnDelta = nDelta;
    aEvent.mnNotchDelta = nDelta > 0 ? 1 : -1;
    aEvent.mnScrollLines = 3;

    m_pFrame->CallCallback(SalEvent::WheelMouse, &aEvent);
    pEvent->accept();
}

void VclQtMixinBase::mixinMoveEvent(QMoveEvent*)
{
    m_pFrame->CallCallback(SalEvent::Move, nullptr);
}

void VclQtMixinBase::mixinShowEvent(QShowEvent*)
{
    QSize aSize(m_pFrame->GetQWidget()->size());
    SalPaintEvent aPaintEvt(0, 0, aSize.width(), aSize.height(), true);
    m_pFrame->CallCallback(SalEvent::Paint, &aPaintEvt);
}

void VclQtMixinBase::mixinCloseEvent(QCloseEvent* pEvent)
{
    bool bRet = false;
    bRet = m_pFrame->CallCallback(SalEvent::Close, nullptr);

    if (bRet)
        pEvent->accept();
    // SalEvent::Close returning false may mean that user has vetoed
    // closing the frame ("you have unsaved changes" dialog for example)
    // We should't process the event in such case
    else
        pEvent->ignore();
}

static sal_uInt16 GetKeyCode(int keyval)
{
    sal_uInt16 nCode = 0;
    if (keyval >= Qt::Key_0 && keyval <= Qt::Key_9)
        nCode = KEY_0 + (keyval - Qt::Key_0);
    else if (keyval >= Qt::Key_A && keyval <= Qt::Key_Z)
        nCode = KEY_A + (keyval - Qt::Key_A);
    else if (keyval >= Qt::Key_F1 && keyval <= Qt::Key_F26)
        nCode = KEY_F1 + (keyval - Qt::Key_F1);
    else
    {
        switch (keyval)
        {
            case Qt::Key_Down:
                nCode = KEY_DOWN;
                break;
            case Qt::Key_Up:
                nCode = KEY_UP;
                break;
            case Qt::Key_Left:
                nCode = KEY_LEFT;
                break;
            case Qt::Key_Right:
                nCode = KEY_RIGHT;
                break;
            case Qt::Key_Home:
                nCode = KEY_HOME;
                break;
            case Qt::Key_End:
                nCode = KEY_END;
                break;
            case Qt::Key_PageUp:
                nCode = KEY_PAGEUP;
                break;
            case Qt::Key_PageDown:
                nCode = KEY_PAGEDOWN;
                break;
            case Qt::Key_Return:
                nCode = KEY_RETURN;
                break;
            case Qt::Key_Escape:
                nCode = KEY_ESCAPE;
                break;
            case Qt::Key_Tab:
            // oddly enough, Qt doesn't send Shift-Tab event as 'Tab key pressed with Shift
            // modifier' but as 'Backtab key pressed' (while its modifier bits are still
            // set to Shift) -- so let's map both Key_Tab and Key_Backtab to VCL's KEY_TAB
            case Qt::Key_Backtab:
                nCode = KEY_TAB;
                break;
            case Qt::Key_Backspace:
                nCode = KEY_BACKSPACE;
                break;
            case Qt::Key_Space:
                nCode = KEY_SPACE;
                break;
            case Qt::Key_Insert:
                nCode = KEY_INSERT;
                break;
            case Qt::Key_Delete:
                nCode = KEY_DELETE;
                break;
            case Qt::Key_Plus:
                nCode = KEY_ADD;
                break;
            case Qt::Key_Minus:
                nCode = KEY_SUBTRACT;
                break;
            case Qt::Key_Asterisk:
                nCode = KEY_MULTIPLY;
                break;
            case Qt::Key_Slash:
                nCode = KEY_DIVIDE;
                break;
            case Qt::Key_Period:
                nCode = KEY_POINT;
                break;
            case Qt::Key_Comma:
                nCode = KEY_COMMA;
                break;
            case Qt::Key_Less:
                nCode = KEY_LESS;
                break;
            case Qt::Key_Greater:
                nCode = KEY_GREATER;
                break;
            case Qt::Key_Equal:
                nCode = KEY_EQUAL;
                break;
            case Qt::Key_Find:
                nCode = KEY_FIND;
                break;
            case Qt::Key_Menu:
                nCode = KEY_CONTEXTMENU;
                break;
            case Qt::Key_Help:
                nCode = KEY_HELP;
                break;
            case Qt::Key_Undo:
                nCode = KEY_UNDO;
                break;
            case Qt::Key_Redo:
                nCode = KEY_REPEAT;
                break;
            case Qt::Key_Cancel:
                nCode = KEY_F11;
                break;
            case Qt::Key_AsciiTilde:
                nCode = KEY_TILDE;
                break;
            case Qt::Key_QuoteLeft:
                nCode = KEY_QUOTELEFT;
                break;
            case Qt::Key_BracketLeft:
                nCode = KEY_BRACKETLEFT;
                break;
            case Qt::Key_BracketRight:
                nCode = KEY_BRACKETRIGHT;
                break;
            case Qt::Key_Semicolon:
                nCode = KEY_SEMICOLON;
                break;
            case Qt::Key_Copy:
                nCode = KEY_COPY;
                break;
            case Qt::Key_Cut:
                nCode = KEY_CUT;
                break;
            case Qt::Key_Open:
                nCode = KEY_OPEN;
                break;
            case Qt::Key_Paste:
                nCode = KEY_PASTE;
                break;
        }
    }

    return nCode;
}

bool VclQtMixinBase::mixinHandleKeyEvent(QKeyEvent* pEvent, bool bDown)
{
    SalKeyEvent aEvent;

    aEvent.mnCharCode = (pEvent->text().isEmpty() ? 0 : pEvent->text().at(0).unicode());
    aEvent.mnRepeat = 0;
    aEvent.mnCode = GetKeyCode(pEvent->key());
    aEvent.mnCode |= GetKeyModCode(pEvent->modifiers());

    bool bStopProcessingKey;
    if (bDown)
        bStopProcessingKey = m_pFrame->CallCallback(SalEvent::KeyInput, &aEvent);
    else
        bStopProcessingKey = m_pFrame->CallCallback(SalEvent::KeyUp, &aEvent);
    return bStopProcessingKey;
}

void VclQtMixinBase::mixinKeyPressEvent(QKeyEvent* pEvent)
{
    if (mixinHandleKeyEvent(pEvent, true))
        pEvent->accept();
}

void VclQtMixinBase::mixinKeyReleaseEvent(QKeyEvent* pEvent)
{
    if (mixinHandleKeyEvent(pEvent, false))
        pEvent->accept();
}

void VclQtMixinBase::mixinFocusInEvent(QFocusEvent*)
{
    m_pFrame->CallCallback(SalEvent::GetFocus, nullptr);
}

void VclQtMixinBase::mixinFocusOutEvent(QFocusEvent*)
{
    m_pFrame->CallCallback(SalEvent::LoseFocus, nullptr);
}

template <class ParentClassT> class Qt5Widget : public ParentClassT
{
    //Q_OBJECT

    VclQtMixinBase maMixin;

    virtual void focusInEvent(QFocusEvent* event) override
    {
        return maMixin.mixinFocusInEvent(event);
    }

    virtual void focusOutEvent(QFocusEvent* event) override
    {
        return maMixin.mixinFocusOutEvent(event);
    }

    virtual void keyPressEvent(QKeyEvent* event) override
    {
        return maMixin.mixinKeyPressEvent(event);
    }

    virtual void keyReleaseEvent(QKeyEvent* event) override
    {
        return maMixin.mixinKeyReleaseEvent(event);
    }

    virtual void mouseMoveEvent(QMouseEvent* event) override
    {
        return maMixin.mixinMouseMoveEvent(event);
    }

    virtual void mousePressEvent(QMouseEvent* event) override
    {
        return maMixin.mixinMousePressEvent(event);
    }

    virtual void mouseReleaseEvent(QMouseEvent* event) override
    {
        return maMixin.mixinMouseReleaseEvent(event);
    }

    virtual void moveEvent(QMoveEvent* event) override { return maMixin.mixinMoveEvent(event); }

    virtual void paintEvent(QPaintEvent* event) override
    {
        return maMixin.mixinPaintEvent(event, this);
    }

    virtual void resizeEvent(QResizeEvent* event) override
    {
        return maMixin.mixinResizeEvent(event, ParentClassT::size());
    }

    virtual void showEvent(QShowEvent* event) override { return maMixin.mixinShowEvent(event); }

    virtual void wheelEvent(QWheelEvent* event) override { return maMixin.mixinWheelEvent(event); }

    virtual void closeEvent(QCloseEvent* event) override { return maMixin.mixinCloseEvent(event); }

private:
    Qt5Widget(Qt5Frame& rFrame, Qt::WindowFlags f)
        : ParentClassT(Q_NULLPTR, f)
        , maMixin(&rFrame)
    {
        Init();
    }

    void Init()
    {
        ParentClassT::create();
        ParentClassT::setMouseTracking(true);
        ParentClassT::setFocusPolicy(Qt::StrongFocus);
    }

public:
    virtual ~Qt5Widget() override{};

    friend QWidget* createQt5Widget(Qt5Frame& rFrame, Qt::WindowFlags f);
    friend QWidget* createQMainWindow(Qt5Frame& rFrame, Qt::WindowFlags f);
};

QWidget* createQt5Widget(Qt5Frame& rFrame, Qt::WindowFlags f)
{
    return new Qt5Widget<QWidget>(rFrame, f);
}

QWidget* createQMainWindow(Qt5Frame& rFrame, Qt::WindowFlags f)
{
    return new Qt5Widget<QMainWindow>(rFrame, f);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
