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

#include <QtWidget.hxx>
#include <QtWidget.moc>

#include <QtFrame.hxx>
#include <QtGraphics.hxx>
#include <QtInstance.hxx>
#include <QtMainWindow.hxx>
#include <QtSvpGraphics.hxx>
#include <QtTransferable.hxx>
#include <QtTools.hxx>

#include <QtCore/QMimeData>
#include <QtGui/QDrag>
#include <QtGui/QFocusEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QShowEvent>
#include <QtGui/QTextCharFormat>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QWidget>

#include <cairo.h>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/toolkit/floatwin.hxx>
#include <window.h>
#include <comphelper/diagnose_ex.hxx>

#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>

#if CHECK_ANY_QT_USING_X11
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#endif

using namespace com::sun::star;

void QtWidget::paintEvent(QPaintEvent* pEvent)
{
    QPainter p(this);
    if (!m_rFrame.m_bNullRegion)
        p.setClipRegion(m_rFrame.m_aRegion);

    QImage aImage;
    if (m_rFrame.m_bUseCairo)
    {
        cairo_surface_t* pSurface = m_rFrame.m_pSurface.get();
        cairo_surface_flush(pSurface);

        aImage = QImage(cairo_image_surface_get_data(pSurface),
                        cairo_image_surface_get_width(pSurface),
                        cairo_image_surface_get_height(pSurface), Qt_DefaultFormat32);
    }
    else
        aImage = *m_rFrame.m_pQImage;

    const qreal fRatio = m_rFrame.devicePixelRatioF();
    aImage.setDevicePixelRatio(fRatio);
    QRectF source(pEvent->rect().topLeft() * fRatio, pEvent->rect().size() * fRatio);
    p.drawImage(pEvent->rect(), aImage, source);
}

void QtWidget::resizeEvent(QResizeEvent* pEvent)
{
    const qreal fRatio = m_rFrame.devicePixelRatioF();
    const int nWidth = ceil(pEvent->size().width() * fRatio);
    const int nHeight = ceil(pEvent->size().height() * fRatio);

    m_rFrame.maGeometry.setSize({ nWidth, nHeight });

    if (m_rFrame.m_bUseCairo)
    {
        if (m_rFrame.m_pSurface)
        {
            const int nOldWidth = cairo_image_surface_get_width(m_rFrame.m_pSurface.get());
            const int nOldHeight = cairo_image_surface_get_height(m_rFrame.m_pSurface.get());
            if (nOldWidth != nWidth || nOldHeight != nHeight)
            {
                cairo_surface_t* pSurface
                    = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, nWidth, nHeight);
                cairo_surface_set_user_data(pSurface, SvpSalGraphics::getDamageKey(),
                                            &m_rFrame.m_aDamageHandler, nullptr);
                m_rFrame.m_pSvpGraphics->setSurface(pSurface, basegfx::B2IVector(nWidth, nHeight));
                UniqueCairoSurface old_surface(m_rFrame.m_pSurface.release());
                m_rFrame.m_pSurface.reset(pSurface);

                const int nMinWidth = qMin(nOldWidth, nWidth);
                const int nMinHeight = qMin(nOldHeight, nHeight);
                SalTwoRect rect(0, 0, nMinWidth, nMinHeight, 0, 0, nMinWidth, nMinHeight);
                m_rFrame.m_pSvpGraphics->copySource(rect, old_surface.get());
            }
        }
    }
    else
    {
        if (m_rFrame.m_pQImage && m_rFrame.m_pQImage->size() != QSize(nWidth, nHeight))
        {
            QImage* pImage = new QImage(m_rFrame.m_pQImage->copy(0, 0, nWidth, nHeight));
            m_rFrame.m_pQtGraphics->ChangeQImage(pImage);
            m_rFrame.m_pQImage.reset(pImage);
        }
    }

    m_rFrame.CallCallback(SalEvent::Resize, nullptr);
}

void QtWidget::fakeResize()
{
    QResizeEvent aEvent(size(), QSize());
    resizeEvent(&aEvent);
}

void QtWidget::fillSalAbstractMouseEvent(const QtFrame& rFrame, const QInputEvent* pQEvent,
                                         const QPoint& rPos, Qt::MouseButtons eButtons, int nWidth,
                                         SalAbstractMouseEvent& aSalEvent)
{
    const qreal fRatio = rFrame.devicePixelRatioF();
    const Point aPos = toPoint(rPos * fRatio);

    aSalEvent.mnX = QGuiApplication::isLeftToRight() ? aPos.X() : round(nWidth * fRatio) - aPos.X();
    aSalEvent.mnY = aPos.Y();
    aSalEvent.mnTime = pQEvent->timestamp();
    aSalEvent.mnCode = GetKeyModCode(pQEvent->modifiers()) | GetMouseModCode(eButtons);
}

#define FILL_SAME(rFrame, nWidth)                                                                  \
    fillSalAbstractMouseEvent(rFrame, pEvent, pEvent->pos(), pEvent->buttons(), nWidth, aEvent)

void QtWidget::handleMouseButtonEvent(const QtFrame& rFrame, const QMouseEvent* pEvent)
{
    SalMouseEvent aEvent;
    FILL_SAME(rFrame, rFrame.GetQWidget()->width());

    switch (pEvent->button())
    {
        case Qt::LeftButton:
            aEvent.mnButton = MOUSE_LEFT;
            break;
        case Qt::MiddleButton:
            aEvent.mnButton = MOUSE_MIDDLE;
            break;
        case Qt::RightButton:
            aEvent.mnButton = MOUSE_RIGHT;
            break;
        default:
            return;
    }

    SalEvent nEventType;
    if (pEvent->type() == QEvent::MouseButtonPress || pEvent->type() == QEvent::MouseButtonDblClick)
        nEventType = SalEvent::MouseButtonDown;
    else
        nEventType = SalEvent::MouseButtonUp;
    rFrame.CallCallback(nEventType, &aEvent);
}

void QtWidget::mousePressEvent(QMouseEvent* pEvent)
{
    handleMouseButtonEvent(m_rFrame, pEvent);
    if (m_rFrame.isPopup()
        && !geometry().translated(geometry().topLeft() * -1).contains(pEvent->pos()))
        closePopup();
}

void QtWidget::mouseReleaseEvent(QMouseEvent* pEvent) { handleMouseButtonEvent(m_rFrame, pEvent); }

void QtWidget::mouseMoveEvent(QMouseEvent* pEvent)
{
    SalMouseEvent aEvent;
    FILL_SAME(m_rFrame, width());

    aEvent.mnButton = 0;

    m_rFrame.CallCallback(SalEvent::MouseMove, &aEvent);
    pEvent->accept();
}

void QtWidget::handleMouseEnterLeaveEvents(const QtFrame& rFrame, QEvent* pQEvent)
{
    const qreal fRatio = rFrame.devicePixelRatioF();
    const QWidget* pWidget = rFrame.GetQWidget();
    const Point aPos = toPoint(pWidget->mapFromGlobal(QCursor::pos()) * fRatio);

    SalMouseEvent aEvent;
    aEvent.mnX
        = QGuiApplication::isLeftToRight() ? aPos.X() : round(pWidget->width() * fRatio) - aPos.X();
    aEvent.mnY = aPos.Y();
    aEvent.mnTime = 0;
    aEvent.mnButton = 0;
    aEvent.mnCode = GetKeyModCode(QGuiApplication::keyboardModifiers())
                    | GetMouseModCode(QGuiApplication::mouseButtons());

    SalEvent nEventType;
    if (pQEvent->type() == QEvent::Enter)
        nEventType = SalEvent::MouseMove;
    else
        nEventType = SalEvent::MouseLeave;
    rFrame.CallCallback(nEventType, &aEvent);
    pQEvent->accept();
}

void QtWidget::leaveEvent(QEvent* pEvent) { handleMouseEnterLeaveEvents(m_rFrame, pEvent); }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void QtWidget::enterEvent(QEnterEvent* pEvent)
#else
void QtWidget::enterEvent(QEvent* pEvent)
#endif
{
    handleMouseEnterLeaveEvents(m_rFrame, pEvent);
}

void QtWidget::wheelEvent(QWheelEvent* pEvent)
{
    SalWheelMouseEvent aEvent;
    fillSalAbstractMouseEvent(m_rFrame, pEvent, pEvent->position().toPoint(), pEvent->buttons(),
                              width(), aEvent);

    // mouse wheel ticks are 120, which we map to 3 lines.
    // we have to accumulate for touch scroll to keep track of the absolute delta.

    int nDelta = pEvent->angleDelta().y(), lines;
    aEvent.mbHorz = nDelta == 0;
    if (aEvent.mbHorz)
    {
        nDelta = (QGuiApplication::isLeftToRight() ? 1 : -1) * pEvent->angleDelta().x();
        if (!nDelta)
            return;

        m_nDeltaX += nDelta;
        lines = m_nDeltaX / 40;
        m_nDeltaX = m_nDeltaX % 40;
    }
    else
    {
        m_nDeltaY += nDelta;
        lines = m_nDeltaY / 40;
        m_nDeltaY = m_nDeltaY % 40;
    }

    aEvent.mnDelta = nDelta;
    aEvent.mnNotchDelta = nDelta < 0 ? -1 : 1;
    aEvent.mnScrollLines = std::abs(lines);

    m_rFrame.CallCallback(SalEvent::WheelMouse, &aEvent);
    pEvent->accept();
}

void QtWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (dynamic_cast<const QtMimeData*>(event->mimeData()))
        event->accept();
    else
        event->acceptProposedAction();
}

// also called when a drop is rejected
void QtWidget::dragLeaveEvent(QDragLeaveEvent*) { m_rFrame.handleDragLeave(); }

void QtWidget::dragMoveEvent(QDragMoveEvent* pEvent) { m_rFrame.handleDragMove(pEvent); }

void QtWidget::dropEvent(QDropEvent* pEvent) { m_rFrame.handleDrop(pEvent); }

void QtWidget::moveEvent(QMoveEvent* pEvent)
{
    // already handled by QtMainWindow::moveEvent
    if (m_rFrame.m_pTopLevel)
        return;

    m_rFrame.maGeometry.setPos(toPoint(pEvent->pos() * m_rFrame.devicePixelRatioF()));
    m_rFrame.CallCallback(SalEvent::Move, nullptr);
}

void QtWidget::showEvent(QShowEvent*)
{
    QSize aSize(m_rFrame.GetQWidget()->size() * m_rFrame.devicePixelRatioF());
    // forcing an immediate update somehow interferes with the hide + show
    // sequence from QtFrame::SetModal, if the frame was already set visible,
    // resulting in a hidden / unmapped window
    SalPaintEvent aPaintEvt(0, 0, aSize.width(), aSize.height());
    if (m_rFrame.isPopup())
        GetQtInstance()->setActivePopup(&m_rFrame);
    m_rFrame.CallCallback(SalEvent::Paint, &aPaintEvt);
}

void QtWidget::hideEvent(QHideEvent*)
{
    if (m_rFrame.isPopup() && GetQtInstance()->activePopup() == &m_rFrame)
        GetQtInstance()->setActivePopup(nullptr);
}

void QtWidget::closeEvent(QCloseEvent* /*pEvent*/)
{
    m_rFrame.CallCallback(SalEvent::Close, nullptr);
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
            case Qt::Key_NumberSign:
                nCode = KEY_NUMBERSIGN;
                break;
            case Qt::Key_Forward:
                nCode = KEY_XF86FORWARD;
                break;
            case Qt::Key_Back:
                nCode = KEY_XF86BACK;
                break;
            case Qt::Key_Colon:
                nCode = KEY_COLON;
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

void QtWidget::commitText(QtFrame& rFrame, const QString& aText)
{
    SalExtTextInputEvent aInputEvent;
    aInputEvent.mpTextAttr = nullptr;
    aInputEvent.mnCursorFlags = 0;
    aInputEvent.maText = toOUString(aText);
    aInputEvent.mnCursorPos = aInputEvent.maText.getLength();

    SolarMutexGuard aGuard;
    vcl::DeletionListener aDel(&rFrame);
    rFrame.CallCallback(SalEvent::ExtTextInput, &aInputEvent);
    if (!aDel.isDeleted())
        rFrame.CallCallback(SalEvent::EndExtTextInput, nullptr);
}

void QtWidget::deleteReplacementText(QtFrame& rFrame, int nReplacementStart, int nReplacementLength)
{
    // get the surrounding text
    SolarMutexGuard aGuard;
    SalSurroundingTextRequestEvent aSurroundingTextEvt;
    aSurroundingTextEvt.maText.clear();
    aSurroundingTextEvt.mnStart = aSurroundingTextEvt.mnEnd = 0;
    rFrame.CallCallback(SalEvent::SurroundingTextRequest, &aSurroundingTextEvt);

    // Turn nReplacementStart, nReplacementLength into a UTF-16 selection
    const Selection aSelection = SalFrame::CalcDeleteSurroundingSelection(
        aSurroundingTextEvt.maText, aSurroundingTextEvt.mnStart, nReplacementStart,
        nReplacementLength);

    const Selection aInvalid(SAL_MAX_UINT32, SAL_MAX_UINT32);
    if (aSelection == aInvalid)
    {
        SAL_WARN("vcl.qt", "Invalid selection when deleting IM replacement text");
        return;
    }

    SalSurroundingTextSelectionChangeEvent aEvt;
    aEvt.mnStart = aSelection.Min();
    aEvt.mnEnd = aSelection.Max();
    rFrame.CallCallback(SalEvent::DeleteSurroundingTextRequest, &aEvt);
}

bool QtWidget::handleGestureEvent(QtFrame& rFrame, QGestureEvent* pGestureEvent)
{
    if (QGesture* pGesture = pGestureEvent->gesture(Qt::PinchGesture))
    {
        if (!pGesture->hasHotSpot())
        {
            pGestureEvent->ignore();
            return false;
        }

        GestureEventZoomType eType = GestureEventZoomType::Begin;
        switch (pGesture->state())
        {
            case Qt::GestureStarted:
                eType = GestureEventZoomType::Begin;
                break;
            case Qt::GestureUpdated:
                eType = GestureEventZoomType::Update;
                break;
            case Qt::GestureFinished:
                eType = GestureEventZoomType::End;
                break;
            case Qt::NoGesture:
            case Qt::GestureCanceled:
            default:
                SAL_WARN("vcl.qt", "Unhandled pinch gesture state: " << pGesture->state());
                pGestureEvent->ignore();
                return false;
        }

        QPinchGesture* pPinchGesture = static_cast<QPinchGesture*>(pGesture);
        const QPointF aHotspot = pGesture->hotSpot();
        SalGestureZoomEvent aEvent;
        aEvent.meEventType = eType;
        aEvent.mnX = aHotspot.x();
        aEvent.mnY = aHotspot.y();
        aEvent.mfScaleDelta = 1 + pPinchGesture->totalScaleFactor();
        rFrame.CallCallback(SalEvent::GestureZoom, &aEvent);
        pGestureEvent->accept();
        return true;
    }

    pGestureEvent->ignore();
    return false;
}

bool QtWidget::handleKeyEvent(QtFrame& rFrame, const QWidget& rWidget, QKeyEvent* pEvent)
{
    const bool bIsKeyPressed
        = pEvent->type() == QEvent::KeyPress || pEvent->type() == QEvent::ShortcutOverride;
    sal_uInt16 nCode = GetKeyCode(pEvent->key(), pEvent->modifiers());
    if (bIsKeyPressed && nCode == 0 && pEvent->text().length() > 1
        && rWidget.testAttribute(Qt::WA_InputMethodEnabled))
    {
        commitText(rFrame, pEvent->text());
        pEvent->accept();
        return true;
    }

    QGuiApplication::inputMethod()->update(Qt::ImCursorRectangle);

    if (nCode == 0 && pEvent->text().isEmpty())
    {
        sal_uInt16 nModCode = GetKeyModCode(pEvent->modifiers());
        SalKeyModEvent aModEvt;
        aModEvt.mbDown = bIsKeyPressed;
        aModEvt.mnModKeyCode = ModKeyFlags::NONE;

#if CHECK_ANY_QT_USING_X11
        if (QGuiApplication::platformName() == "xcb")
        {
            // pressing just the ctrl key leads to a keysym of XK_Control but
            // the event state does not contain ControlMask. In the release
            // event it's the other way round: it does contain the Control mask.
            // The modifier mode therefore has to be adapted manually.
            ModKeyFlags nExtModMask = ModKeyFlags::NONE;
            sal_uInt16 nModMask = 0;
            switch (pEvent->nativeVirtualKey())
            {
                case XK_Control_L:
                    nExtModMask = ModKeyFlags::LeftMod1;
                    nModMask = KEY_MOD1;
                    break;
                case XK_Control_R:
                    nExtModMask = ModKeyFlags::RightMod1;
                    nModMask = KEY_MOD1;
                    break;
                case XK_Alt_L:
                    nExtModMask = ModKeyFlags::LeftMod2;
                    nModMask = KEY_MOD2;
                    break;
                case XK_Alt_R:
                    nExtModMask = ModKeyFlags::RightMod2;
                    nModMask = KEY_MOD2;
                    break;
                case XK_Shift_L:
                    nExtModMask = ModKeyFlags::LeftShift;
                    nModMask = KEY_SHIFT;
                    break;
                case XK_Shift_R:
                    nExtModMask = ModKeyFlags::RightShift;
                    nModMask = KEY_SHIFT;
                    break;
                // Map Meta/Super keys to MOD3 modifier on all Unix systems
                // except macOS
                case XK_Meta_L:
                case XK_Super_L:
                    nExtModMask = ModKeyFlags::LeftMod3;
                    nModMask = KEY_MOD3;
                    break;
                case XK_Meta_R:
                case XK_Super_R:
                    nExtModMask = ModKeyFlags::RightMod3;
                    nModMask = KEY_MOD3;
                    break;
            }

            if (!bIsKeyPressed)
            {
                // sending the old mnModKeyCode mask on release is needed to
                // implement the writing direction switch with Ctrl + L/R-Shift
                aModEvt.mnModKeyCode = rFrame.m_nKeyModifiers;
                nModCode &= ~nModMask;
                rFrame.m_nKeyModifiers &= ~nExtModMask;
            }
            else
            {
                nModCode |= nModMask;
                rFrame.m_nKeyModifiers |= nExtModMask;
                aModEvt.mnModKeyCode = rFrame.m_nKeyModifiers;
            }
        }
#endif
        aModEvt.mnCode = nModCode;

        rFrame.CallCallback(SalEvent::KeyModChange, &aModEvt);
        return false;
    }

#if CHECK_ANY_QT_USING_X11
    // prevent interference of writing direction switch (Ctrl + L/R-Shift) with "normal" shortcuts
    rFrame.m_nKeyModifiers = ModKeyFlags::NONE;
#endif

    SalKeyEvent aEvent;
    aEvent.mnCharCode = (pEvent->text().isEmpty() ? 0 : pEvent->text().at(0).unicode());
    aEvent.mnRepeat = 0;
    aEvent.mnCode = nCode;
    aEvent.mnCode |= GetKeyModCode(pEvent->modifiers());

    bool bStopProcessingKey;
    if (bIsKeyPressed)
        bStopProcessingKey = rFrame.CallCallback(SalEvent::KeyInput, &aEvent);
    else
        bStopProcessingKey = rFrame.CallCallback(SalEvent::KeyUp, &aEvent);
    if (bStopProcessingKey)
        pEvent->accept();
    return bStopProcessingKey;
}

bool QtWidget::handleEvent(QtFrame& rFrame, QWidget& rWidget, QEvent* pEvent)
{
    if (pEvent->type() == QEvent::Gesture)
    {
        QGestureEvent* pGestureEvent = static_cast<QGestureEvent*>(pEvent);
        return handleGestureEvent(rFrame, pGestureEvent);
    }
    else if (pEvent->type() == QEvent::ShortcutOverride)
    {
        // ignore non-spontaneous QEvent::ShortcutOverride events,
        // since such an extra event is sent e.g. with Orca screen reader enabled,
        // so that two events of that kind (the "real one" and a non-spontaneous one)
        // would otherwise be processed, resulting in duplicate input as 'handleKeyEvent'
        // is called below (s. tdf#122053)
        if (!pEvent->spontaneous())
        {
            // accept event so shortcut action (from menu) isn't triggered in addition
            // to the processing for the spontaneous event further below
            pEvent->accept();
            return false;
        }

        // Accepted event disables shortcut activation,
        // but enables keypress event.
        // If event is not accepted and shortcut is successfully activated,
        // KeyPress event is omitted.
        //
        // Instead of processing keyPressEvent, handle ShortcutOverride event,
        // and if it's handled - disable the shortcut, it should have been activated.
        // Don't process keyPressEvent generated after disabling shortcut since it was handled here.
        // If event is not handled, don't accept it and let Qt activate related shortcut.
        if (handleKeyEvent(rFrame, rWidget, static_cast<QKeyEvent*>(pEvent)))
            return true;
    }
    else if (pEvent->type() == QEvent::ToolTip)
    {
        // Qt's POV on the active popup is wrong due to our fake popup, so check LO's state.
        // Otherwise Qt will continue handling ToolTip events from the "parent" window.
        const QtFrame* pPopupFrame = GetQtInstance()->activePopup();
        if (!rFrame.m_aTooltipText.isEmpty() && (!pPopupFrame || pPopupFrame == &rFrame))
            QToolTip::showText(QCursor::pos(), toQString(rFrame.m_aTooltipText), &rWidget,
                               rFrame.m_aTooltipArea);
        else
        {
            QToolTip::hideText();
            pEvent->ignore();
        }
        return true;
    }
    return false;
}

bool QtWidget::event(QEvent* pEvent)
{
    return handleEvent(m_rFrame, *this, pEvent) || QWidget::event(pEvent);
}

void QtWidget::keyReleaseEvent(QKeyEvent* pEvent)
{
    if (!handleKeyReleaseEvent(m_rFrame, *this, pEvent))
        QWidget::keyReleaseEvent(pEvent);
}

void QtWidget::focusInEvent(QFocusEvent*) { m_rFrame.CallCallback(SalEvent::GetFocus, nullptr); }

void QtWidget::closePopup()
{
    VclPtr<FloatingWindow> pFirstFloat = ImplGetSVData()->mpWinData->mpFirstFloat;
    if (pFirstFloat && !(pFirstFloat->GetPopupModeFlags() & FloatWinPopupFlags::NoAppFocusClose))
    {
        SolarMutexGuard aGuard;
        pFirstFloat->EndPopupMode(FloatWinPopupEndFlags::Cancel | FloatWinPopupEndFlags::CloseAll);
    }
}

void QtWidget::focusOutEvent(QFocusEvent*)
{
#if CHECK_ANY_QT_USING_X11
    m_rFrame.m_nKeyModifiers = ModKeyFlags::NONE;
#endif
    endExtTextInput();
    m_rFrame.CallCallback(SalEvent::LoseFocus, nullptr);
    closePopup();
}

QtWidget::QtWidget(QtFrame& rFrame, Qt::WindowFlags f)
    // if you try to set the QWidget parent via the QtFrame, instead of using the Q_NULLPTR, at
    // least test Wayland popups; these horribly broke last time doing this (read commits)!
    : QWidget(Q_NULLPTR, f)
    , m_rFrame(rFrame)
    , m_bNonEmptyIMPreeditSeen(false)
    , m_bInInputMethodQueryCursorRectangle(false)
    , m_nDeltaX(0)
    , m_nDeltaY(0)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setMouseTracking(true);
    if (!rFrame.isPopup())
        setFocusPolicy(Qt::StrongFocus);
    else
        setFocusPolicy(Qt::ClickFocus);

    grabGesture(Qt::PinchGesture);
}

static ExtTextInputAttr lcl_MapUnderlineStyle(QTextCharFormat::UnderlineStyle us)
{
    switch (us)
    {
        case QTextCharFormat::NoUnderline:
            return ExtTextInputAttr::NONE;
        case QTextCharFormat::DotLine:
            return ExtTextInputAttr::DottedUnderline;
        case QTextCharFormat::DashDotDotLine:
        case QTextCharFormat::DashDotLine:
            return ExtTextInputAttr::DashDotUnderline;
        case QTextCharFormat::WaveUnderline:
            return ExtTextInputAttr::GrayWaveline;
        default:
            return ExtTextInputAttr::Underline;
    }
}

void QtWidget::inputMethodEvent(QInputMethodEvent* pEvent)
{
    const bool bHasCommitText = !pEvent->commitString().isEmpty();
    const int nReplacementLength = pEvent->replacementLength();

    if (nReplacementLength > 0 || bHasCommitText)
    {
        if (nReplacementLength > 0)
            deleteReplacementText(m_rFrame, pEvent->replacementStart(), nReplacementLength);
        if (bHasCommitText)
            commitText(m_rFrame, pEvent->commitString());
    }
    else
    {
        SalExtTextInputEvent aInputEvent;
        aInputEvent.mpTextAttr = nullptr;
        aInputEvent.mnCursorFlags = 0;
        aInputEvent.maText = toOUString(pEvent->preeditString());
        aInputEvent.mnCursorPos = 0;

        const sal_Int32 nLength = aInputEvent.maText.getLength();
        const QList<QInputMethodEvent::Attribute>& rAttrList = pEvent->attributes();
        std::vector<ExtTextInputAttr> aTextAttrs(std::max(sal_Int32(1), nLength),
                                                 ExtTextInputAttr::NONE);
        aInputEvent.mpTextAttr = aTextAttrs.data();

        for (const QInputMethodEvent::Attribute& rAttr : rAttrList)
        {
            switch (rAttr.type)
            {
                case QInputMethodEvent::TextFormat:
                {
                    QTextCharFormat aCharFormat
                        = qvariant_cast<QTextFormat>(rAttr.value).toCharFormat();
                    if (aCharFormat.isValid())
                    {
                        ExtTextInputAttr aETIP
                            = lcl_MapUnderlineStyle(aCharFormat.underlineStyle());
                        if (aCharFormat.hasProperty(QTextFormat::BackgroundBrush))
                            aETIP |= ExtTextInputAttr::Highlight;
                        if (aCharFormat.fontStrikeOut())
                            aETIP |= ExtTextInputAttr::RedText;
                        for (int j = rAttr.start; j < rAttr.start + rAttr.length; j++)
                        {
                            SAL_WARN_IF(j >= static_cast<int>(aTextAttrs.size()), "vcl.qt",
                                        "QInputMethodEvent::Attribute out of range. Broken range: "
                                            << rAttr.start << "," << rAttr.start + rAttr.length
                                            << " Legal range: 0," << aTextAttrs.size());
                            if (j >= static_cast<int>(aTextAttrs.size()))
                                break;
                            aTextAttrs[j] = aETIP;
                        }
                    }
                    break;
                }
                case QInputMethodEvent::Cursor:
                {
                    aInputEvent.mnCursorPos = rAttr.start;
                    if (rAttr.length == 0)
                        aInputEvent.mnCursorFlags |= EXTTEXTINPUT_CURSOR_INVISIBLE;
                    break;
                }
                default:
                    SAL_WARN("vcl.qt", "Unhandled QInputMethodEvent attribute: "
                                           << static_cast<int>(rAttr.type));
                    break;
            }
        }

        const bool bIsEmpty = aInputEvent.maText.isEmpty();
        if (m_bNonEmptyIMPreeditSeen || !bIsEmpty)
        {
            SolarMutexGuard aGuard;
            vcl::DeletionListener aDel(&m_rFrame);
            m_rFrame.CallCallback(SalEvent::ExtTextInput, &aInputEvent);
            if (!aDel.isDeleted() && bIsEmpty)
                m_rFrame.CallCallback(SalEvent::EndExtTextInput, nullptr);
            m_bNonEmptyIMPreeditSeen = !bIsEmpty;
        }
    }

    pEvent->accept();
}

static bool lcl_retrieveSurrounding(sal_Int32& rPosition, sal_Int32& rAnchor, QString* pText,
                                    QString* pSelection)
{
    SolarMutexGuard aGuard;
    vcl::Window* pFocusWin = Application::GetFocusWindow();
    if (!pFocusWin)
        return false;

    uno::Reference<accessibility::XAccessibleEditableText> xText;
    try
    {
        uno::Reference<accessibility::XAccessible> xAccessible(pFocusWin->GetAccessible());
        if (xAccessible.is())
            xText = FindFocusedEditableText(xAccessible->getAccessibleContext());
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("vcl.qt", "Exception in getting input method surrounding text");
    }

    if (xText.is())
    {
        rPosition = xText->getCaretPosition();
        if (rPosition != -1)
        {
            if (pText)
                *pText = toQString(xText->getText());

            sal_Int32 nSelStart = xText->getSelectionStart();
            sal_Int32 nSelEnd = xText->getSelectionEnd();
            if (nSelStart == nSelEnd)
            {
                rAnchor = rPosition;
            }
            else
            {
                if (rPosition == nSelStart)
                    rAnchor = nSelEnd;
                else
                    rAnchor = nSelStart;
                if (pSelection)
                    *pSelection = toQString(xText->getSelectedText());
            }
            return true;
        }
    }

    return false;
}

QVariant QtWidget::inputMethodQuery(Qt::InputMethodQuery property) const
{
    switch (property)
    {
        case Qt::ImSurroundingText:
        {
            QString aText;
            sal_Int32 nCursorPos, nAnchor;
            if (lcl_retrieveSurrounding(nCursorPos, nAnchor, &aText, nullptr))
                return QVariant(aText);
            return QVariant();
        }
        case Qt::ImCursorPosition:
        {
            sal_Int32 nCursorPos, nAnchor;
            if (lcl_retrieveSurrounding(nCursorPos, nAnchor, nullptr, nullptr))
                return QVariant(static_cast<int>(nCursorPos));
            return QVariant();
        }
        case Qt::ImCursorRectangle:
        {
            if (!m_bInInputMethodQueryCursorRectangle)
            {
                m_bInInputMethodQueryCursorRectangle = true;
                SalExtTextInputPosEvent aPosEvent;
                m_rFrame.CallCallback(SalEvent::ExtTextInputPos, &aPosEvent);
                const qreal fRatio = m_rFrame.devicePixelRatioF();
                m_aImCursorRectangle.setRect(aPosEvent.mnX / fRatio, aPosEvent.mnY / fRatio,
                                             aPosEvent.mnWidth / fRatio,
                                             aPosEvent.mnHeight / fRatio);
                m_bInInputMethodQueryCursorRectangle = false;
            }
            return QVariant(m_aImCursorRectangle);
        }
        case Qt::ImAnchorPosition:
        {
            sal_Int32 nCursorPos, nAnchor;
            if (lcl_retrieveSurrounding(nCursorPos, nAnchor, nullptr, nullptr))
                return QVariant(static_cast<int>(nAnchor));
            return QVariant();
        }
        case Qt::ImCurrentSelection:
        {
            QString aSelection;
            sal_Int32 nCursorPos, nAnchor;
            if (lcl_retrieveSurrounding(nCursorPos, nAnchor, nullptr, &aSelection))
                return QVariant(aSelection);
            return QVariant();
        }
        default:
            return QWidget::inputMethodQuery(property);
    }
}

void QtWidget::endExtTextInput()
{
    if (m_bNonEmptyIMPreeditSeen)
    {
        m_rFrame.CallCallback(SalEvent::EndExtTextInput, nullptr);
        m_bNonEmptyIMPreeditSeen = false;
    }
}

void QtWidget::changeEvent(QEvent* pEvent)
{
    switch (pEvent->type())
    {
        case QEvent::FontChange:
            [[fallthrough]];
        case QEvent::PaletteChange:
            [[fallthrough]];
        case QEvent::StyleChange:
        {
            auto* pSalInst(GetQtInstance());
            assert(pSalInst);
            pSalInst->UpdateStyle(QEvent::FontChange == pEvent->type());
            break;
        }
        default:
            break;
    }
    QWidget::changeEvent(pEvent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
