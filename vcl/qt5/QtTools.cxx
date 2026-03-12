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

#include <bitmaps.hlst>
#include <cursor_hotspots.hxx>
#include <QtFont.hxx>
#include <QtFontFace.hxx>
#include <QtTools.hxx>
#include <QtTransferable.hxx>
#include <unx/fontmanager.hxx>

#include <o3tl/enumarray.hxx>
#include <tools/stream.hxx>
#include <vcl/event.hxx>
#include <vcl/image.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#include <vcl/qt/QtUtils.hxx>
#include <vcl/svapp.hxx>

#include <QtCore/QBuffer>
#include <QtGui/QImage>

Qt::Alignment toQtAlignment(TxtAlign eAlign)
{
    switch (eAlign)
    {
        case TxtAlign::Left:
            return Qt::AlignLeft;
        case TxtAlign::Center:
            return Qt::AlignHCenter;
        case TxtAlign::Right:
            return Qt::AlignRight;
        default:
            assert(false && "unhandled TxtAlign value");
            return Qt::AlignLeft;
    }
}

Qt::CheckState toQtCheckState(TriState eTristate)
{
    switch (eTristate)
    {
        case TRISTATE_FALSE:
            return Qt::CheckState::Unchecked;
        case TRISTATE_TRUE:
            return Qt::CheckState::Checked;
        case TRISTATE_INDET:
            return Qt::CheckState::PartiallyChecked;
        default:
            assert(false && "unhandled Tristate value");
            return Qt::CheckState::PartiallyChecked;
    }
};

QClipboard::Mode toQClipboardMode(ClipboardSelectionType eSelection)
{
    switch (eSelection)
    {
        case ClipboardSelectionType::Clipboard:
            return QClipboard::Mode::Clipboard;
        case ClipboardSelectionType::Primary:
            return QClipboard::Mode::Selection;
        default:
            assert(false && "unhandled ClipboardSelectionType value");
            return QClipboard::Mode::Clipboard;
    }
}

TriState toVclTriState(Qt::CheckState eTristate)
{
    switch (eTristate)
    {
        case Qt::CheckState::Unchecked:
            return TRISTATE_FALSE;
        case Qt::CheckState::Checked:
            return TRISTATE_TRUE;
        case Qt::CheckState::PartiallyChecked:
            return TRISTATE_INDET;
        default:
            assert(false && "unhandled Qt::CheckState value");
            return TRISTATE_INDET;
    }
};

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

KeyEvent toVclKeyEvent(const QKeyEvent& rEvent)
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

MouseEvent toVclMouseEvent(const QMouseEvent& rEvent)
{
    const Point aPos = toPoint(rEvent.pos());
    const sal_uInt16 nClicks = rEvent.type() == QMouseEvent::MouseButtonDblClick ? 2 : 1;
    const sal_uInt16 nButtons = toVclMouseButtons(rEvent.button() | rEvent.buttons());
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

QImage toQImage(const Image& rImage)
{
    QImage aImage;

    if (!!rImage)
    {
        SvMemoryStream aMemStm;
        auto rBitmap = rImage.GetBitmap();
        vcl::PngImageWriter aWriter(aMemStm);
        aWriter.write(rBitmap);
        aImage.loadFromData(static_cast<const uchar*>(aMemStm.GetData()), aMemStm.TellEnd());
    }

    return aImage;
}

Image toImage(const QImage& rImage)
{
    if (rImage.isNull())
        return {};

    QByteArray aByteArray;
    QBuffer aBuffer(&aByteArray);
    aBuffer.open(QIODevice::WriteOnly);
    rImage.save(&aBuffer, "PNG");

    Bitmap aBitmap;
    SvMemoryStream aMemoryStream(aByteArray.data(), aByteArray.size(), StreamMode::READ);
    vcl::PngImageReader aReader(aMemoryStream);
    assert(aReader.read(aBitmap));

    return Image(aBitmap);
}

QFont toQtFont(const vcl::Font& rVclFont)
{
    QFont aQFont(toQString(rVclFont.GetFamilyName()), rVclFont.GetFontHeight());

    QtFont::applyStretch(aQFont, rVclFont.GetWidthType());
    QtFont::applyStyle(aQFont, rVclFont.GetItalic());
    QtFont::applyWeight(aQFont, rVclFont.GetWeight());

    return aQFont;
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

static QCursor* getQCursorFromIconTheme(const OUString& rIconName, int nXHot, int nYHot)
{
    QPixmap aPixmap = loadQPixmapIcon(rIconName);
    return new QCursor(aPixmap, nXHot, nYHot);
}

#define MAKE_CURSOR(vcl_name, name, icon_name)                                                     \
    case vcl_name:                                                                                 \
        pCursor = getQCursorFromIconTheme(icon_name, name##curs_x_hot, name##curs_y_hot);          \
        break

#define MAP_BUILTIN(vcl_name, qt_enum)                                                             \
    case vcl_name:                                                                                 \
        pCursor = new QCursor(qt_enum);                                                            \
        break

const QCursor& toQCursor(PointerStyle ePointerStyle)
{
    static o3tl::enumarray<PointerStyle, std::unique_ptr<QCursor>> aCursors;
    if (!aCursors[ePointerStyle])
    {
        QCursor* pCursor = nullptr;

        switch (ePointerStyle)
        {
            MAP_BUILTIN(PointerStyle::Arrow, Qt::ArrowCursor);
            MAP_BUILTIN(PointerStyle::Text, Qt::IBeamCursor);
            MAP_BUILTIN(PointerStyle::Help, Qt::WhatsThisCursor);
            MAP_BUILTIN(PointerStyle::Cross, Qt::CrossCursor);
            MAP_BUILTIN(PointerStyle::Wait, Qt::WaitCursor);
            MAP_BUILTIN(PointerStyle::NSize, Qt::SizeVerCursor);
            MAP_BUILTIN(PointerStyle::SSize, Qt::SizeVerCursor);
            MAP_BUILTIN(PointerStyle::WSize, Qt::SizeHorCursor);
            MAP_BUILTIN(PointerStyle::ESize, Qt::SizeHorCursor);

            MAP_BUILTIN(PointerStyle::NWSize, Qt::SizeFDiagCursor);
            MAP_BUILTIN(PointerStyle::NESize, Qt::SizeBDiagCursor);
            MAP_BUILTIN(PointerStyle::SWSize, Qt::SizeBDiagCursor);
            MAP_BUILTIN(PointerStyle::SESize, Qt::SizeFDiagCursor);
            MAP_BUILTIN(PointerStyle::WindowNSize, Qt::SizeVerCursor);
            MAP_BUILTIN(PointerStyle::WindowSSize, Qt::SizeVerCursor);
            MAP_BUILTIN(PointerStyle::WindowWSize, Qt::SizeHorCursor);
            MAP_BUILTIN(PointerStyle::WindowESize, Qt::SizeHorCursor);
            MAP_BUILTIN(PointerStyle::WindowNWSize, Qt::SizeFDiagCursor);
            MAP_BUILTIN(PointerStyle::WindowNESize, Qt::SizeBDiagCursor);
            MAP_BUILTIN(PointerStyle::WindowSWSize, Qt::SizeBDiagCursor);
            MAP_BUILTIN(PointerStyle::WindowSESize, Qt::SizeFDiagCursor);

            MAP_BUILTIN(PointerStyle::HSizeBar, Qt::SizeHorCursor);
            MAP_BUILTIN(PointerStyle::VSizeBar, Qt::SizeVerCursor);

            MAP_BUILTIN(PointerStyle::RefHand, Qt::PointingHandCursor);
            MAP_BUILTIN(PointerStyle::Hand, Qt::OpenHandCursor);
#if 0
            MAP_BUILTIN( PointerStyle::Pen, GDK_PENCIL );
#endif
            MAP_BUILTIN(PointerStyle::HSplit, Qt::SizeHorCursor);
            MAP_BUILTIN(PointerStyle::VSplit, Qt::SizeVerCursor);

            MAP_BUILTIN(PointerStyle::Move, Qt::SizeAllCursor);

            MAP_BUILTIN(PointerStyle::Null, Qt::BlankCursor);
            MAKE_CURSOR(PointerStyle::Magnify, magnify_, RID_CURSOR_MAGNIFY);
            MAKE_CURSOR(PointerStyle::Fill, fill_, RID_CURSOR_FILL);
            MAKE_CURSOR(PointerStyle::MoveData, movedata_, RID_CURSOR_MOVE_DATA);
            MAKE_CURSOR(PointerStyle::CopyData, copydata_, RID_CURSOR_COPY_DATA);
            MAKE_CURSOR(PointerStyle::MoveFile, movefile_, RID_CURSOR_MOVE_FILE);
            MAKE_CURSOR(PointerStyle::CopyFile, copyfile_, RID_CURSOR_COPY_FILE);
            MAKE_CURSOR(PointerStyle::MoveFiles, movefiles_, RID_CURSOR_MOVE_FILES);
            MAKE_CURSOR(PointerStyle::CopyFiles, copyfiles_, RID_CURSOR_COPY_FILES);
            MAKE_CURSOR(PointerStyle::NotAllowed, nodrop_, RID_CURSOR_NOT_ALLOWED);
            MAKE_CURSOR(PointerStyle::Rotate, rotate_, RID_CURSOR_ROTATE);
            MAKE_CURSOR(PointerStyle::HShear, hshear_, RID_CURSOR_H_SHEAR);
            MAKE_CURSOR(PointerStyle::VShear, vshear_, RID_CURSOR_V_SHEAR);
            MAKE_CURSOR(PointerStyle::DrawLine, drawline_, RID_CURSOR_DRAW_LINE);
            MAKE_CURSOR(PointerStyle::DrawRect, drawrect_, RID_CURSOR_DRAW_RECT);
            MAKE_CURSOR(PointerStyle::DrawPolygon, drawpolygon_, RID_CURSOR_DRAW_POLYGON);
            MAKE_CURSOR(PointerStyle::DrawBezier, drawbezier_, RID_CURSOR_DRAW_BEZIER);
            MAKE_CURSOR(PointerStyle::DrawArc, drawarc_, RID_CURSOR_DRAW_ARC);
            MAKE_CURSOR(PointerStyle::DrawPie, drawpie_, RID_CURSOR_DRAW_PIE);
            MAKE_CURSOR(PointerStyle::DrawCircleCut, drawcirclecut_, RID_CURSOR_DRAW_CIRCLE_CUT);
            MAKE_CURSOR(PointerStyle::DrawEllipse, drawellipse_, RID_CURSOR_DRAW_ELLIPSE);
            MAKE_CURSOR(PointerStyle::DrawConnect, drawconnect_, RID_CURSOR_DRAW_CONNECT);
            MAKE_CURSOR(PointerStyle::DrawText, drawtext_, RID_CURSOR_DRAW_TEXT);
            MAKE_CURSOR(PointerStyle::Mirror, mirror_, RID_CURSOR_MIRROR);
            MAKE_CURSOR(PointerStyle::Crook, crook_, RID_CURSOR_CROOK);
            MAKE_CURSOR(PointerStyle::Crop, crop_, RID_CURSOR_CROP);
            MAKE_CURSOR(PointerStyle::MovePoint, movepoint_, RID_CURSOR_MOVE_POINT);
            MAKE_CURSOR(PointerStyle::MoveBezierWeight, movebezierweight_,
                        RID_CURSOR_MOVE_BEZIER_WEIGHT);
            MAKE_CURSOR(PointerStyle::DrawFreehand, drawfreehand_, RID_CURSOR_DRAW_FREEHAND);
            MAKE_CURSOR(PointerStyle::DrawCaption, drawcaption_, RID_CURSOR_DRAW_CAPTION);
            MAKE_CURSOR(PointerStyle::LinkData, linkdata_, RID_CURSOR_LINK_DATA);
            MAKE_CURSOR(PointerStyle::MoveDataLink, movedlnk_, RID_CURSOR_MOVE_DATA_LINK);
            MAKE_CURSOR(PointerStyle::CopyDataLink, copydlnk_, RID_CURSOR_COPY_DATA_LINK);
            MAKE_CURSOR(PointerStyle::LinkFile, linkfile_, RID_CURSOR_LINK_FILE);
            MAKE_CURSOR(PointerStyle::MoveFileLink, moveflnk_, RID_CURSOR_MOVE_FILE_LINK);
            MAKE_CURSOR(PointerStyle::CopyFileLink, copyflnk_, RID_CURSOR_COPY_FILE_LINK);
            MAKE_CURSOR(PointerStyle::Chart, chart_, RID_CURSOR_CHART);
            MAKE_CURSOR(PointerStyle::Detective, detective_, RID_CURSOR_DETECTIVE);
            MAKE_CURSOR(PointerStyle::PivotCol, pivotcol_, RID_CURSOR_PIVOT_COLUMN);
            MAKE_CURSOR(PointerStyle::PivotRow, pivotrow_, RID_CURSOR_PIVOT_ROW);
            MAKE_CURSOR(PointerStyle::PivotField, pivotfld_, RID_CURSOR_PIVOT_FIELD);
            MAKE_CURSOR(PointerStyle::PivotDelete, pivotdel_, RID_CURSOR_PIVOT_DELETE);
            MAKE_CURSOR(PointerStyle::Chain, chain_, RID_CURSOR_CHAIN);
            MAKE_CURSOR(PointerStyle::ChainNotAllowed, chainnot_, RID_CURSOR_CHAIN_NOT_ALLOWED);
            MAKE_CURSOR(PointerStyle::AutoScrollN, asn_, RID_CURSOR_AUTOSCROLL_N);
            MAKE_CURSOR(PointerStyle::AutoScrollS, ass_, RID_CURSOR_AUTOSCROLL_S);
            MAKE_CURSOR(PointerStyle::AutoScrollW, asw_, RID_CURSOR_AUTOSCROLL_W);
            MAKE_CURSOR(PointerStyle::AutoScrollE, ase_, RID_CURSOR_AUTOSCROLL_E);
            MAKE_CURSOR(PointerStyle::AutoScrollNW, asnw_, RID_CURSOR_AUTOSCROLL_NW);
            MAKE_CURSOR(PointerStyle::AutoScrollNE, asne_, RID_CURSOR_AUTOSCROLL_NE);
            MAKE_CURSOR(PointerStyle::AutoScrollSW, assw_, RID_CURSOR_AUTOSCROLL_SW);
            MAKE_CURSOR(PointerStyle::AutoScrollSE, asse_, RID_CURSOR_AUTOSCROLL_SE);
            MAKE_CURSOR(PointerStyle::AutoScrollNS, asns_, RID_CURSOR_AUTOSCROLL_NS);
            MAKE_CURSOR(PointerStyle::AutoScrollWE, aswe_, RID_CURSOR_AUTOSCROLL_WE);
            MAKE_CURSOR(PointerStyle::AutoScrollNSWE, asnswe_, RID_CURSOR_AUTOSCROLL_NSWE);
            MAKE_CURSOR(PointerStyle::TextVertical, vertcurs_, RID_CURSOR_TEXT_VERTICAL);

            MAKE_CURSOR(PointerStyle::TabSelectS, tblsels_, RID_CURSOR_TAB_SELECT_S);
            MAKE_CURSOR(PointerStyle::TabSelectE, tblsele_, RID_CURSOR_TAB_SELECT_E);
            MAKE_CURSOR(PointerStyle::TabSelectSE, tblselse_, RID_CURSOR_TAB_SELECT_SE);
            MAKE_CURSOR(PointerStyle::TabSelectW, tblselw_, RID_CURSOR_TAB_SELECT_W);
            MAKE_CURSOR(PointerStyle::TabSelectSW, tblselsw_, RID_CURSOR_TAB_SELECT_SW);

            MAKE_CURSOR(PointerStyle::HideWhitespace, hidewhitespace_, RID_CURSOR_HIDE_WHITESPACE);
            MAKE_CURSOR(PointerStyle::ShowWhitespace, showwhitespace_, RID_CURSOR_SHOW_WHITESPACE);

            MAKE_CURSOR(PointerStyle::FatCross, fatcross_, RID_CURSOR_FATCROSS);
            default:
                break;
        }
        if (!pCursor)
        {
            pCursor = new QCursor(Qt::ArrowCursor);
            SAL_WARN("vcl.qt", "pointer " << static_cast<int>(ePointerStyle) << " not implemented");
        }

        aCursors[ePointerStyle].reset(pCursor);
    }

    return *aCursors[ePointerStyle];
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

QString vclToQtStringWithAccelerator(const OUString& rText)
{
    // preserve literal '&'s and use '&' instead of '~' for the accelerator
    return toQString(rText.replaceAll("&", "&&").replace('~', '&'));
}

OUString qtToVclStringWithAccelerator(const QString& rText)
{
    if (rText.isEmpty())
        return OUString();

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

QString toRichTextTooltip(const OUString& rText)
{
    if (rText.isEmpty())
        return QString();

    return QStringLiteral(u"<html>") + toQString(rText).toHtmlEscaped()
           + QStringLiteral(u"</html>");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
