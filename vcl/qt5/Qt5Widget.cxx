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

#include <QtCore/QMimeData>
#include <QtGui/QDrag>
#include <QtGui/QFocusEvent>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QShowEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QWidget>

#include <cairo.h>
#include <headless/svpgdi.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>

void Qt5Widget::paintEvent(QPaintEvent* pEvent)
{
    QPainter p(this);
    if (!m_pFrame->m_bNullRegion)
        p.setClipRegion(m_pFrame->m_aRegion);

    if (m_pFrame->m_bUseCairo)
    {
        cairo_surface_t* pSurface = m_pFrame->m_pSurface.get();
        cairo_surface_flush(pSurface);

        QImage aImage(cairo_image_surface_get_data(pSurface), size().width(), size().height(),
                      Qt5_DefaultFormat32);
        p.drawImage(pEvent->rect().topLeft(), aImage, pEvent->rect());
    }
    else
        p.drawImage(pEvent->rect().topLeft(), *m_pFrame->m_pQImage, pEvent->rect());
}

void Qt5Widget::resizeEvent(QResizeEvent* pEvent)
{
    if (m_pFrame->m_bUseCairo)
    {
        int width = size().width();
        int height = size().height();

        if (m_pFrame->m_pSvpGraphics)
        {
            cairo_surface_t* pSurface
                = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
            cairo_surface_set_user_data(pSurface, SvpSalGraphics::getDamageKey(),
                                        &m_pFrame->m_aDamageHandler, nullptr);
            m_pFrame->m_pSvpGraphics->setSurface(pSurface, basegfx::B2IVector(width, height));
            UniqueCairoSurface old_surface(m_pFrame->m_pSurface.release());
            m_pFrame->m_pSurface.reset(pSurface);

            int min_width = qMin(pEvent->oldSize().width(), pEvent->size().width());
            int min_height = qMin(pEvent->oldSize().height(), pEvent->size().height());

            SalTwoRect rect(0, 0, min_width, min_height, 0, 0, min_width, min_height);

            m_pFrame->m_pSvpGraphics->copySource(rect, old_surface.get());
        }
    }
    else
    {
        QImage* pImage = nullptr;

        if (m_pFrame->m_pQImage)
            pImage = new QImage(
                m_pFrame->m_pQImage->copy(0, 0, pEvent->size().width(), pEvent->size().height()));
        else
        {
            pImage = new QImage(size(), Qt5_DefaultFormat32);
            pImage->fill(Qt::transparent);
        }

        m_pFrame->m_pQt5Graphics->ChangeQImage(pImage);
        m_pFrame->m_pQImage.reset(pImage);
    }

    m_pFrame->maGeometry.nWidth = size().width();
    m_pFrame->maGeometry.nHeight = size().height();

    m_pFrame->CallCallback(SalEvent::Resize, nullptr);
}

void Qt5Widget::handleMouseButtonEvent(QMouseEvent* pEvent, bool bReleased)
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

void Qt5Widget::mousePressEvent(QMouseEvent* pEvent) { handleMouseButtonEvent(pEvent, false); }

void Qt5Widget::mouseReleaseEvent(QMouseEvent* pEvent) { handleMouseButtonEvent(pEvent, true); }

void Qt5Widget::mouseMoveEvent(QMouseEvent* pEvent)
{
    QPoint point = pEvent->pos();

    SalMouseEvent aEvent;
    aEvent.mnTime = pEvent->timestamp();
    aEvent.mnX = point.x();
    aEvent.mnY = point.y();
    aEvent.mnCode = GetKeyModCode(pEvent->modifiers()) | GetMouseModCode(pEvent->buttons());
    aEvent.mnButton = 0;

    m_pFrame->CallCallback(SalEvent::MouseMove, &aEvent);
    pEvent->accept();
}

void Qt5Widget::wheelEvent(QWheelEvent* pEvent)
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

void Qt5Widget::startDrag(sal_Int8 nSourceActions)
{
    // internal drag source
    QMimeData* mimeData = new QMimeData;
    mimeData->setData(sInternalMimeType, nullptr);

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(toQtDropActions(nSourceActions), Qt::MoveAction);
}

void Qt5Widget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat(sInternalMimeType))
        event->accept();
    else
        event->acceptProposedAction();
}

void Qt5Widget::dragMoveEvent(QDragMoveEvent* event)
{
    QPoint point = event->pos();

    m_pFrame->draggingStarted(point.x(), point.y(), event->possibleActions(), event->mimeData());
    QWidget::dragMoveEvent(event);
}

void Qt5Widget::dropEvent(QDropEvent* event)
{
    QPoint point = event->pos();

    m_pFrame->dropping(point.x(), point.y(), event->mimeData());
    QWidget::dropEvent(event);
}

void Qt5Widget::moveEvent(QMoveEvent*) { m_pFrame->CallCallback(SalEvent::Move, nullptr); }

void Qt5Widget::showEvent(QShowEvent*)
{
    QSize aSize(m_pFrame->GetQWidget()->size());
    SalPaintEvent aPaintEvt(0, 0, aSize.width(), aSize.height(), true);
    m_pFrame->CallCallback(SalEvent::Paint, &aPaintEvt);
}

void Qt5Widget::closeEvent(QCloseEvent* /*pEvent*/)
{
    m_pFrame->CallCallback(SalEvent::Close, nullptr);
}

static sal_uInt16 GetKeyCode(int keyval, Qt::KeyboardModifiers modifiers)
{
    sal_uInt16 nCode = 0;
    if (keyval >= Qt::Key_0 && keyval <= Qt::Key_9)
        nCode = KEY_0 + (keyval - Qt::Key_0);
    else if (keyval >= Qt::Key_A && keyval <= Qt::Key_Z)
        nCode = KEY_A + (keyval - Qt::Key_A);
    else if (keyval >= Qt::Key_F1 && keyval <= Qt::Key_F26)
        nCode = KEY_F1 + (keyval - Qt::Key_F1);
    else if (modifiers.testFlag(Qt::KeypadModifier)
             && (keyval == Qt::Key_Period || keyval == Qt::Key_Comma))
        // Qt doesn't use a special keyval for decimal separator ("," or ".")
        // on numerical keypad, but sets Qt::KeypadModifier in addition
        nCode = KEY_DECIMAL;
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
            case Qt::Key_Enter:
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

bool Qt5Widget::handleKeyEvent(QKeyEvent* pEvent, bool bDown)
{
    SalKeyEvent aEvent;

    aEvent.mnCharCode = (pEvent->text().isEmpty() ? 0 : pEvent->text().at(0).unicode());
    aEvent.mnRepeat = 0;
    aEvent.mnCode = GetKeyCode(pEvent->key(), pEvent->modifiers());
    aEvent.mnCode |= GetKeyModCode(pEvent->modifiers());

    bool bStopProcessingKey;
    if (bDown)
        bStopProcessingKey = m_pFrame->CallCallback(SalEvent::KeyInput, &aEvent);
    else
        bStopProcessingKey = m_pFrame->CallCallback(SalEvent::KeyUp, &aEvent);
    return bStopProcessingKey;
}

bool Qt5Widget::event(QEvent* pEvent)
{
    if (pEvent->type() == QEvent::ShortcutOverride)
    {
        // Accepted event disables shortcut activation,
        // but enables keypress event.
        // If event is not accepted and shortcut is successfully activated,
        // KeyPress event is omitted.
        //
        // Instead of processing keyPressEvent, handle ShortcutOverride event,
        // and if it's handled - disable the shortcut, it should have been activated.
        // Don't process keyPressEvent generated after disabling shortcut since it was handled here.
        // If event is not handled, don't accept it and let Qt activate related shortcut.
        if (handleKeyEvent(static_cast<QKeyEvent*>(pEvent), true))
            pEvent->accept();
    }

    return QWidget::event(pEvent);
}

void Qt5Widget::keyReleaseEvent(QKeyEvent* pEvent)
{
    if (handleKeyEvent(pEvent, false))
        pEvent->accept();
}

void Qt5Widget::focusInEvent(QFocusEvent*) { m_pFrame->CallCallback(SalEvent::GetFocus, nullptr); }

void Qt5Widget::focusOutEvent(QFocusEvent*)
{
    m_pFrame->CallCallback(SalEvent::LoseFocus, nullptr);
}

void Qt5Widget::showTooltip(const OUString& rTooltip)
{
    QPoint pt = QCursor::pos();
    QToolTip::showText(pt, toQString(rTooltip));
}

Qt5Widget::Qt5Widget(Qt5Frame& rFrame, Qt::WindowFlags f)
    : QWidget(Q_NULLPTR, f)
    , m_pFrame(&rFrame)
{
    create();
    setMouseTracking(true);
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
}

void Qt5Widget::inputMethodEvent(QInputMethodEvent* pEvent)
{
    SolarMutexGuard aGuard;
    SalExtTextInputEvent aInputEvent;
    aInputEvent.mpTextAttr = nullptr;
    aInputEvent.mnCursorFlags = 0;

    if (!pEvent->commitString().isEmpty())
    {
        vcl::DeletionListener aDel(m_pFrame);
        aInputEvent.maText = toOUString(pEvent->commitString());
        aInputEvent.mnCursorPos = aInputEvent.maText.getLength();
        m_pFrame->CallCallback(SalEvent::ExtTextInput, &aInputEvent);
        pEvent->accept();
        if (!aDel.isDeleted())
            m_pFrame->CallCallback(SalEvent::EndExtTextInput, nullptr);
    }
    else
    {
        aInputEvent.maText = toOUString(pEvent->preeditString());
        aInputEvent.mnCursorPos = 0;
        sal_Int32 nLength = aInputEvent.maText.getLength();
        std::vector<ExtTextInputAttr> aTextAttrs(nLength, ExtTextInputAttr::Underline);
        if (nLength)
            aInputEvent.mpTextAttr = &aTextAttrs[0];
        m_pFrame->CallCallback(SalEvent::ExtTextInput, &aInputEvent);
        pEvent->accept();
    }
}

QVariant Qt5Widget::inputMethodQuery(Qt::InputMethodQuery property) const
{
    switch (property)
    {
        case Qt::ImCursorRectangle:
        {
            SalExtTextInputPosEvent aPosEvent;
            m_pFrame->CallCallback(SalEvent::ExtTextInputPos, &aPosEvent);
            return QVariant(
                QRect(aPosEvent.mnX, aPosEvent.mnY, aPosEvent.mnWidth, aPosEvent.mnHeight));
        }
        default:
            return QWidget::inputMethodQuery(property);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
