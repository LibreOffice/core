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
#include <Qt5Instance.hxx>
#include <Qt5SvpGraphics.hxx>
#include <Qt5Transferable.hxx>
#include <Qt5Tools.hxx>

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
#include <QtWidgets/QWidget>

#include <cairo.h>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <window.h>
#include <tools/diagnose_ex.h>

#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>

using namespace com::sun::star;

void Qt5Widget::paintEvent(QPaintEvent* pEvent)
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
                        cairo_image_surface_get_height(pSurface), Qt5_DefaultFormat32);
    }
    else
        aImage = *m_rFrame.m_pQImage;

    const qreal fRatio = m_rFrame.devicePixelRatioF();
    aImage.setDevicePixelRatio(fRatio);
    QRectF source(pEvent->rect().topLeft() * fRatio, pEvent->rect().size() * fRatio);
    p.drawImage(pEvent->rect(), aImage, source);
}

void Qt5Widget::resizeEvent(QResizeEvent* pEvent)
{
    const qreal fRatio = m_rFrame.devicePixelRatioF();
    const int nWidth = ceil(pEvent->size().width() * fRatio);
    const int nHeight = ceil(pEvent->size().height() * fRatio);

    m_rFrame.maGeometry.nWidth = nWidth;
    m_rFrame.maGeometry.nHeight = nHeight;

    if (m_rFrame.m_bUseCairo)
    {
        if (m_rFrame.m_pSvpGraphics)
        {
            cairo_surface_t* pSurface
                = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, nWidth, nHeight);
            cairo_surface_set_user_data(pSurface, SvpSalGraphics::getDamageKey(),
                                        &m_rFrame.m_aDamageHandler, nullptr);
            m_rFrame.m_pSvpGraphics->setSurface(pSurface, basegfx::B2IVector(nWidth, nHeight));
            UniqueCairoSurface old_surface(m_rFrame.m_pSurface.release());
            m_rFrame.m_pSurface.reset(pSurface);

            int min_width = qMin(cairo_image_surface_get_width(old_surface.get()), nWidth);
            int min_height = qMin(cairo_image_surface_get_height(old_surface.get()), nHeight);

            SalTwoRect rect(0, 0, min_width, min_height, 0, 0, min_width, min_height);

            m_rFrame.m_pSvpGraphics->copySource(rect, old_surface.get());
        }
    }
    else
    {
        QImage* pImage = nullptr;

        if (m_rFrame.m_pQImage)
            pImage = new QImage(m_rFrame.m_pQImage->copy(0, 0, nWidth, nHeight));
        else
        {
            pImage = new QImage(nWidth, nHeight, Qt5_DefaultFormat32);
            pImage->fill(Qt::transparent);
        }

        m_rFrame.m_pQt5Graphics->ChangeQImage(pImage);
        m_rFrame.m_pQImage.reset(pImage);
    }

    m_rFrame.CallCallback(SalEvent::Resize, nullptr);
}

void Qt5Widget::fillSalAbstractMouseEvent(const Qt5Frame& rFrame, const QInputEvent* pQEvent,
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

void Qt5Widget::handleMouseButtonEvent(const Qt5Frame& rFrame, const QMouseEvent* pEvent,
                                       const ButtonKeyState eState)
{
    SalMouseEvent aEvent;
    FILL_SAME(rFrame, rFrame.GetQWidget()->width());

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

    SalEvent nEventType;
    if (eState == ButtonKeyState::Pressed)
        nEventType = SalEvent::MouseButtonDown;
    else
        nEventType = SalEvent::MouseButtonUp;
    rFrame.CallCallback(nEventType, &aEvent);
}

void Qt5Widget::mousePressEvent(QMouseEvent* pEvent) { handleMousePressEvent(m_rFrame, pEvent); }

void Qt5Widget::mouseReleaseEvent(QMouseEvent* pEvent)
{
    handleMouseReleaseEvent(m_rFrame, pEvent);
}

void Qt5Widget::mouseMoveEvent(QMouseEvent* pEvent)
{
    SalMouseEvent aEvent;
    FILL_SAME(m_rFrame, width());

    aEvent.mnButton = 0;

    m_rFrame.CallCallback(SalEvent::MouseMove, &aEvent);
    pEvent->accept();
}

void Qt5Widget::wheelEvent(QWheelEvent* pEvent)
{
    SalWheelMouseEvent aEvent;
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    fillSalAbstractMouseEvent(m_rFrame, pEvent, pEvent->position().toPoint(), pEvent->buttons(),
                              width(), aEvent);
#else
    fillSalAbstractMouseEvent(m_rFrame, pEvent, pEvent->pos(), pEvent->buttons(), width(), aEvent);
#endif

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

void Qt5Widget::dragEnterEvent(QDragEnterEvent* event)
{
    if (dynamic_cast<const Qt5MimeData*>(event->mimeData()))
        event->accept();
    else
        event->acceptProposedAction();
}

// also called when a drop is rejected
void Qt5Widget::dragLeaveEvent(QDragLeaveEvent*) { m_rFrame.handleDragLeave(); }

void Qt5Widget::dragMoveEvent(QDragMoveEvent* pEvent) { m_rFrame.handleDragMove(pEvent); }

void Qt5Widget::dropEvent(QDropEvent* pEvent) { m_rFrame.handleDrop(pEvent); }

void Qt5Widget::moveEvent(QMoveEvent* pEvent)
{
    if (m_rFrame.m_pTopLevel)
        return;

    const Point aPos = toPoint(pEvent->pos() * m_rFrame.devicePixelRatioF());
    m_rFrame.maGeometry.nX = aPos.X();
    m_rFrame.maGeometry.nY = aPos.Y();
    m_rFrame.CallCallback(SalEvent::Move, nullptr);
}

void Qt5Widget::showEvent(QShowEvent*)
{
    QSize aSize(m_rFrame.GetQWidget()->size() * m_rFrame.devicePixelRatioF());
    // forcing an immediate update somehow interferes with the hide + show
    // sequence from Qt5Frame::SetModal, if the frame was already set visible,
    // resulting in a hidden / unmapped window
    SalPaintEvent aPaintEvt(0, 0, aSize.width(), aSize.height());
    m_rFrame.CallCallback(SalEvent::Paint, &aPaintEvt);
}

void Qt5Widget::closeEvent(QCloseEvent* /*pEvent*/)
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

void Qt5Widget::commitText(Qt5Frame& rFrame, const QString& aText)
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

bool Qt5Widget::handleKeyEvent(Qt5Frame& rFrame, const QWidget& rWidget, QKeyEvent* pEvent,
                               const ButtonKeyState eState)
{
    sal_uInt16 nCode = GetKeyCode(pEvent->key(), pEvent->modifiers());
    if (eState == ButtonKeyState::Pressed && nCode == 0 && pEvent->text().length() > 1
        && rWidget.testAttribute(Qt::WA_InputMethodEnabled))
    {
        commitText(rFrame, pEvent->text());
        pEvent->accept();
        return true;
    }

    SalKeyEvent aEvent;
    aEvent.mnCharCode = (pEvent->text().isEmpty() ? 0 : pEvent->text().at(0).unicode());
    aEvent.mnRepeat = 0;
    aEvent.mnCode = nCode;
    aEvent.mnCode |= GetKeyModCode(pEvent->modifiers());

    QGuiApplication::inputMethod()->update(Qt::ImCursorRectangle);

    bool bStopProcessingKey;
    if (eState == ButtonKeyState::Pressed)
        bStopProcessingKey = rFrame.CallCallback(SalEvent::KeyInput, &aEvent);
    else
        bStopProcessingKey = rFrame.CallCallback(SalEvent::KeyUp, &aEvent);
    if (bStopProcessingKey)
        pEvent->accept();
    return bStopProcessingKey;
}

bool Qt5Widget::handleEvent(Qt5Frame& rFrame, const QWidget& rWidget, QEvent* pEvent)
{
    if (pEvent->type() == QEvent::ShortcutOverride)
    {
        // ignore non-spontaneous QEvent::ShortcutOverride events,
        // since such an extra event is sent e.g. with Orca screen reader enabled,
        // so that two events of that kind (the "real one" and a non-spontaneous one)
        // would otherwise be processed, resulting in duplicate input as 'handleKeyEvent'
        // is called below (s. tdf#122053)
        if (!pEvent->spontaneous())
        {
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
        if (handleKeyEvent(rFrame, rWidget, static_cast<QKeyEvent*>(pEvent),
                           ButtonKeyState::Pressed))
            return true;
    }
    return false;
}

bool Qt5Widget::event(QEvent* pEvent)
{
    return handleEvent(m_rFrame, *this, pEvent) || QWidget::event(pEvent);
}

void Qt5Widget::keyReleaseEvent(QKeyEvent* pEvent)
{
    if (!handleKeyReleaseEvent(m_rFrame, *this, pEvent))
        QWidget::keyReleaseEvent(pEvent);
}

void Qt5Widget::focusInEvent(QFocusEvent*) { m_rFrame.CallCallback(SalEvent::GetFocus, nullptr); }

void Qt5Widget::focusOutEvent(QFocusEvent*)
{
    endExtTextInput();
    m_rFrame.CallCallback(SalEvent::LoseFocus, nullptr);
}

Qt5Widget::Qt5Widget(Qt5Frame& rFrame, Qt::WindowFlags f)
    : QWidget(Q_NULLPTR, f)
    , m_rFrame(rFrame)
    , m_bNonEmptyIMPreeditSeen(false)
    , m_nDeltaX(0)
    , m_nDeltaY(0)
{
    create();
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

static ExtTextInputAttr lcl_MapUndrelineStyle(QTextCharFormat::UnderlineStyle us)
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

void Qt5Widget::inputMethodEvent(QInputMethodEvent* pEvent)
{
    if (!pEvent->commitString().isEmpty())
        commitText(m_rFrame, pEvent->commitString());
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

        for (int i = 0; i < rAttrList.size(); ++i)
        {
            const QInputMethodEvent::Attribute& rAttr = rAttrList.at(i);
            switch (rAttr.type)
            {
                case QInputMethodEvent::TextFormat:
                {
                    QTextCharFormat aCharFormat
                        = qvariant_cast<QTextFormat>(rAttr.value).toCharFormat();
                    if (aCharFormat.isValid())
                    {
                        ExtTextInputAttr aETIP
                            = lcl_MapUndrelineStyle(aCharFormat.underlineStyle());
                        if (aCharFormat.hasProperty(QTextFormat::BackgroundBrush))
                            aETIP |= ExtTextInputAttr::Highlight;
                        if (aCharFormat.fontStrikeOut())
                            aETIP |= ExtTextInputAttr::RedText;
                        for (int j = rAttr.start; j < rAttr.start + rAttr.length; j++)
                            aTextAttrs[j] = aETIP;
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
                    SAL_WARN("vcl.qt5", "Unhandled QInputMethodEvent attribute: "
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
        TOOLS_WARN_EXCEPTION("vcl.qt5", "Exception in getting input method surrounding text");
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

QVariant Qt5Widget::inputMethodQuery(Qt::InputMethodQuery property) const
{
    switch (property)
    {
        case Qt::ImSurroundingText:
        {
            QString aText;
            sal_Int32 nCursorPos, nAnchor;
            if (lcl_retrieveSurrounding(nCursorPos, nAnchor, &aText, nullptr))
                return QVariant(aText);
            [[fallthrough]];
        }
        case Qt::ImCursorPosition:
        {
            sal_Int32 nCursorPos, nAnchor;
            if (lcl_retrieveSurrounding(nCursorPos, nAnchor, nullptr, nullptr))
                return QVariant(static_cast<int>(nCursorPos));
            [[fallthrough]];
        }
        case Qt::ImCursorRectangle:
        {
            SalExtTextInputPosEvent aPosEvent;
            m_rFrame.CallCallback(SalEvent::ExtTextInputPos, &aPosEvent);
            return QVariant(
                QRect(aPosEvent.mnX, aPosEvent.mnY, aPosEvent.mnWidth, aPosEvent.mnHeight));
        }
        case Qt::ImAnchorPosition:
        {
            sal_Int32 nCursorPos, nAnchor;
            if (lcl_retrieveSurrounding(nCursorPos, nAnchor, nullptr, nullptr))
                return QVariant(static_cast<int>(nAnchor));
            [[fallthrough]];
        }
        case Qt::ImCurrentSelection:
        {
            QString aSelection;
            sal_Int32 nCursorPos, nAnchor;
            if (lcl_retrieveSurrounding(nCursorPos, nAnchor, nullptr, &aSelection))
                return QVariant(aSelection);
            [[fallthrough]];
        }
        default:
            return QWidget::inputMethodQuery(property);
    }

    return QVariant();
}

void Qt5Widget::endExtTextInput()
{
    if (m_bNonEmptyIMPreeditSeen)
    {
        m_rFrame.CallCallback(SalEvent::EndExtTextInput, nullptr);
        m_bNonEmptyIMPreeditSeen = false;
    }
}

void Qt5Widget::changeEvent(QEvent* pEvent)
{
    switch (pEvent->type())
    {
        case QEvent::FontChange:
            [[fallthrough]];
        case QEvent::PaletteChange:
            [[fallthrough]];
        case QEvent::StyleChange:
        {
            auto* pSalInst(static_cast<Qt5Instance*>(GetSalData()->m_pInstance));
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
