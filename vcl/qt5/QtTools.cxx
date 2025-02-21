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

sal_uInt16 GetKeyModCode(Qt::KeyboardModifiers eKeyModifiers)
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

sal_uInt16 GetMouseModCode(Qt::MouseButtons eButtons)
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
