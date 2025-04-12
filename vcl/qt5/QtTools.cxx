/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <QtTools.hxx>
#include <QtFontFace.hxx>
#include <unx/fontmanager.hxx>

#include <cairo.h>

#include <tools/stream.hxx>
#include <vcl/event.hxx>
#include <vcl/image.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#include <vcl/qt/QtUtils.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/svapp.hxx>

#include <QtGui/QImage>

void CairoDeleter::operator()(cairo_surface_t* pSurface) const { cairo_surface_destroy(pSurface); }

sal_uInt16 toVclKeyboardModifiers(Qt::KeyboardModifiers eKeyModifiers)
{
    sal_uInt16 nCode = 0;
    if (eKeyModifiers & Qt::ShiftModifier)
        nCode |= KEY_SHIFT;
    if (eKeyModifiers & Qt::ControlModifier)
        nCode |= KEY_MOD1;
    if (eKeyModifiers & Qt::AltModifier)
        nCode |= KEY_MOD2;
    if (eKeyModifiers & Qt::MetaModifier)
        nCode |= KEY_MOD3;
    return nCode;
}

sal_uInt16 toVclKeyCode(int nKeyval, Qt::KeyboardModifiers eModifiers)
{
    if (nKeyval >= Qt::Key_0 && nKeyval <= Qt::Key_9)
        return KEY_0 + (nKeyval - Qt::Key_0);
    if (nKeyval >= Qt::Key_A && nKeyval <= Qt::Key_Z)
        return KEY_A + (nKeyval - Qt::Key_A);
    if (nKeyval >= Qt::Key_F1 && nKeyval <= Qt::Key_F26)
        return KEY_F1 + (nKeyval - Qt::Key_F1);
    if (eModifiers.testFlag(Qt::KeypadModifier)
        && (nKeyval == Qt::Key_Period || nKeyval == Qt::Key_Comma))
        // Qt doesn't use a special keyval for decimal separator ("," or ".")
        // on numerical keypad, but sets Qt::KeypadModifier in addition
        return KEY_DECIMAL;

    switch (nKeyval)
    {
        case Qt::Key_Down:
            return KEY_DOWN;
        case Qt::Key_Up:
            return KEY_UP;
        case Qt::Key_Left:
            return KEY_LEFT;
        case Qt::Key_Right:
            return KEY_RIGHT;
        case Qt::Key_Home:
            return KEY_HOME;
        case Qt::Key_End:
            return KEY_END;
        case Qt::Key_PageUp:
            return KEY_PAGEUP;
        case Qt::Key_PageDown:
            return KEY_PAGEDOWN;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            return KEY_RETURN;
        case Qt::Key_Escape:
            return KEY_ESCAPE;
        case Qt::Key_Tab:
        // oddly enough, Qt doesn't send Shift-Tab event as 'Tab key pressed with Shift
        // modifier' but as 'Backtab key pressed' (while its modifier bits are still
        // set to Shift) -- so let's map both Key_Tab and Key_Backtab to VCL's KEY_TAB
        case Qt::Key_Backtab:
            return KEY_TAB;
        case Qt::Key_Backspace:
            return KEY_BACKSPACE;
        case Qt::Key_Space:
            return KEY_SPACE;
        case Qt::Key_Insert:
            return KEY_INSERT;
        case Qt::Key_Delete:
            return KEY_DELETE;
        case Qt::Key_Plus:
            return KEY_ADD;
        case Qt::Key_Minus:
            return KEY_SUBTRACT;
        case Qt::Key_Asterisk:
            return KEY_MULTIPLY;
        case Qt::Key_Slash:
            return KEY_DIVIDE;
        case Qt::Key_Period:
            return KEY_POINT;
        case Qt::Key_Comma:
            return KEY_COMMA;
        case Qt::Key_Less:
            return KEY_LESS;
        case Qt::Key_Greater:
            return KEY_GREATER;
        case Qt::Key_Equal:
            return KEY_EQUAL;
        case Qt::Key_Find:
            return KEY_FIND;
        case Qt::Key_Menu:
            return KEY_CONTEXTMENU;
        case Qt::Key_Help:
            return KEY_HELP;
        case Qt::Key_Undo:
            return KEY_UNDO;
        case Qt::Key_Redo:
            return KEY_REPEAT;
        case Qt::Key_Cancel:
            return KEY_F11;
        case Qt::Key_AsciiTilde:
            return KEY_TILDE;
        case Qt::Key_QuoteLeft:
            return KEY_QUOTELEFT;
        case Qt::Key_BracketLeft:
            return KEY_BRACKETLEFT;
        case Qt::Key_BracketRight:
            return KEY_BRACKETRIGHT;
        case Qt::Key_NumberSign:
            return KEY_NUMBERSIGN;
        case Qt::Key_Forward:
            return KEY_XF86FORWARD;
        case Qt::Key_Back:
            return KEY_XF86BACK;
        case Qt::Key_Colon:
            return KEY_COLON;
        case Qt::Key_Semicolon:
            return KEY_SEMICOLON;
        case Qt::Key_Copy:
            return KEY_COPY;
        case Qt::Key_Cut:
            return KEY_CUT;
        case Qt::Key_Open:
            return KEY_OPEN;
        case Qt::Key_Paste:
            return KEY_PASTE;
        default:
            return 0;
    }
}

KeyEvent toVclKeyEvent(QKeyEvent& rEvent)
{
    const sal_uInt16 nKeyCode = toVclKeyCode(rEvent.key(), rEvent.modifiers());
    const QString sText = rEvent.text();
    const sal_Unicode nChar = sText.isEmpty() ? 0 : sText.at(0).unicode();
    vcl::KeyCode aKeyCode(nKeyCode, toVclKeyboardModifiers(rEvent.modifiers()));
    return KeyEvent(nChar, aKeyCode, 0);
}

sal_uInt16 toVclMouseButtons(Qt::MouseButtons eButtons)
{
    sal_uInt16 nCode = 0;
    if (eButtons & Qt::LeftButton)
        nCode |= MOUSE_LEFT;
    if (eButtons & Qt::MiddleButton)
        nCode |= MOUSE_MIDDLE;
    if (eButtons & Qt::RightButton)
        nCode |= MOUSE_RIGHT;
    return nCode;
}

MouseEvent toVclMouseEvent(QMouseEvent& rEvent)
{
    const Point aPos = toPoint(rEvent.pos());
    const sal_uInt16 nClicks = rEvent.type() == QMouseEvent::MouseButtonDblClick ? 2 : 1;
    const sal_uInt16 nButtons = toVclMouseButtons(rEvent.buttons());
    const sal_uInt16 nModifiers = toVclKeyboardModifiers(rEvent.modifiers());

    return MouseEvent(aPos, nClicks, MouseEventModifiers::NONE, nButtons, nModifiers);
}

Qt::DropActions toQtDropActions(sal_Int8 dragOperation)
{
    Qt::DropActions eRet = Qt::IgnoreAction;
    if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_COPY)
        eRet |= Qt::CopyAction;
    if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_MOVE)
        eRet |= Qt::MoveAction;
    if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_LINK)
        eRet |= Qt::LinkAction;
    return eRet;
}

sal_Int8 toVclDropActions(Qt::DropActions dragOperation)
{
    sal_Int8 nRet(0);
    if (dragOperation & Qt::CopyAction)
        nRet |= css::datatransfer::dnd::DNDConstants::ACTION_COPY;
    if (dragOperation & Qt::MoveAction)
        nRet |= css::datatransfer::dnd::DNDConstants::ACTION_MOVE;
    if (dragOperation & Qt::LinkAction)
        nRet |= css::datatransfer::dnd::DNDConstants::ACTION_LINK;
    return nRet;
}

sal_Int8 toVclDropAction(Qt::DropAction dragOperation)
{
    sal_Int8 nRet(0);
    if (dragOperation == Qt::CopyAction)
        nRet = css::datatransfer::dnd::DNDConstants::ACTION_COPY;
    else if (dragOperation == Qt::MoveAction)
        nRet = css::datatransfer::dnd::DNDConstants::ACTION_MOVE;
    else if (dragOperation == Qt::LinkAction)
        nRet = css::datatransfer::dnd::DNDConstants::ACTION_LINK;
    return nRet;
}

Qt::DropAction getPreferredDropAction(sal_Int8 dragOperation)
{
    Qt::DropAction eAct = Qt::IgnoreAction;
    if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_MOVE)
        eAct = Qt::MoveAction;
    else if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_COPY)
        eAct = Qt::CopyAction;
    else if (dragOperation & css::datatransfer::dnd::DNDConstants::ACTION_LINK)
        eAct = Qt::LinkAction;
    return eAct;
}

QImage toQImage(const Image& rImage)
{
    QImage aImage;

    if (!!rImage)
    {
        SvMemoryStream aMemStm;
        auto rBitmapEx = rImage.GetBitmapEx();
        vcl::PngImageWriter aWriter(aMemStm);
        aWriter.write(rBitmapEx);
        aImage.loadFromData(static_cast<const uchar*>(aMemStm.GetData()), aMemStm.TellEnd());
    }

    return aImage;
}

bool toVclFont(const QFont& rQFont, const css::lang::Locale& rLocale, vcl::Font& rVclFont)
{
    FontAttributes aFA;
    QtFontFace::fillAttributesFromQFont(rQFont, aFA);

    bool bFound = psp::PrintFontManager::get().matchFont(aFA, rLocale);
    SAL_INFO("vcl.qt",
             "font match result for '"
                 << rQFont.family() << "': "
                 << (bFound ? OUString::Concat("'") + aFA.GetFamilyName() + "'" : u"failed"_ustr));

    if (!bFound)
        return false;

    QFontInfo qFontInfo(rQFont);
    int nPointHeight = qFontInfo.pointSize();
    if (nPointHeight <= 0)
        nPointHeight = rQFont.pointSize();

    vcl::Font aFont(aFA.GetFamilyName(), Size(0, nPointHeight));
    if (aFA.GetWeight() != WEIGHT_DONTKNOW)
        aFont.SetWeight(aFA.GetWeight());
    if (aFA.GetWidthType() != WIDTH_DONTKNOW)
        aFont.SetWidthType(aFA.GetWidthType());
    if (aFA.GetItalic() != ITALIC_DONTKNOW)
        aFont.SetItalic(aFA.GetItalic());
    if (aFA.GetPitch() != PITCH_DONTKNOW)
        aFont.SetPitch(aFA.GetPitch());

    rVclFont = aFont;
    return true;
}

QMessageBox::Icon vclMessageTypeToQtIcon(VclMessageType eType)
{
    QMessageBox::Icon eRet = QMessageBox::Information;
    switch (eType)
    {
        case VclMessageType::Info:
            eRet = QMessageBox::Information;
            break;
        case VclMessageType::Warning:
            eRet = QMessageBox::Warning;
            break;
        case VclMessageType::Question:
            eRet = QMessageBox::Question;
            break;
        case VclMessageType::Error:
            eRet = QMessageBox::Critical;
            break;
        case VclMessageType::Other:
            eRet = QMessageBox::Information;
            break;
    }
    return eRet;
}

QString vclMessageTypeToQtTitle(VclMessageType eType)
{
    QString title;
    switch (eType)
    {
        case VclMessageType::Info:
            title = toQString(GetStandardInfoBoxText());
            break;
        case VclMessageType::Warning:
            title = toQString(GetStandardWarningBoxText());
            break;
        case VclMessageType::Question:
            title = toQString(GetStandardQueryBoxText());
            break;
        case VclMessageType::Error:
            title = toQString(GetStandardErrorBoxText());
            break;
        case VclMessageType::Other:
            title = toQString(Application::GetDisplayName());
            break;
    }
    return title;
}

QString vclToQtStringWithAccelerator(const OUString& rText)
{
    // preserve literal '&'s and use '&' instead of '~' for the accelerator
    return toQString(rText.replaceAll("&", "&&").replace('~', '&'));
}

OUString qtToVclStringWithAccelerator(const QString& rText)
{
    // find and replace single "&" used for accelerator
    qsizetype nIndex = 0;
    while (nIndex < rText.size())
    {
        nIndex = rText.indexOf('&', nIndex);
        // skip "&&", i.e. escaped '&'
        if (nIndex < rText.length() - 1 && rText.at(nIndex + 1) == '&')
            nIndex += 2;
        else
            break;
    }

    QString sModified = rText;
    if (nIndex >= 0)
        sModified.replace(nIndex, 1, '~');

    // replace escaped "&&" with plain "&"
    return toOUString(sModified.replace("&&", "&"));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
