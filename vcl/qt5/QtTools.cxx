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
    sal_uInt16 nCode = 0;
    if (nKeyval >= Qt::Key_0 && nKeyval <= Qt::Key_9)
        nCode = KEY_0 + (nKeyval - Qt::Key_0);
    else if (nKeyval >= Qt::Key_A && nKeyval <= Qt::Key_Z)
        nCode = KEY_A + (nKeyval - Qt::Key_A);
    else if (nKeyval >= Qt::Key_F1 && nKeyval <= Qt::Key_F26)
        nCode = KEY_F1 + (nKeyval - Qt::Key_F1);
    else if (eModifiers.testFlag(Qt::KeypadModifier)
             && (nKeyval == Qt::Key_Period || nKeyval == Qt::Key_Comma))
        // Qt doesn't use a special keyval for decimal separator ("," or ".")
        // on numerical keypad, but sets Qt::KeypadModifier in addition
        nCode = KEY_DECIMAL;
    else
    {
        switch (nKeyval)
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
