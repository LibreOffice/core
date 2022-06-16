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

#include <QtTools.hxx>

#include <cairo.h>

#include <tools/stream.hxx>
#include <vcl/event.hxx>
#include <vcl/image.hxx>
#include <vcl/pngwrite.hxx>

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
        vcl::PNGWriter aWriter(rImage.GetBitmapEx());
        aWriter.Write(aMemStm);
        aImage.loadFromData(static_cast<const uchar*>(aMemStm.GetData()), aMemStm.TellEnd());
    }

    return aImage;
}

sal_Int32 GetSgpMetricFromQImage(vcl::SGPmetric eMetric, QImage& rImage)
{
    switch (eMetric)
    {
        case vcl::SGPmetric::Width:
            return rImage.width();
        case vcl::SGPmetric::Height:
            return rImage.height();
        case vcl::SGPmetric::ScalePercentage:
            return round(100 * rImage.devicePixelRatio());
        case vcl::SGPmetric::OffScreen:
            return true;
        case vcl::SGPmetric::BitCount:
            return getFormatBits(rImage.format());
        case vcl::SGPmetric::DPIX:
        case vcl::SGPmetric::DPIY:
            return round(96 * rImage.devicePixelRatioF());
    }

    return -1;
}

QImage toQImage(cairo_surface_t& rSurface, sal_Int32 nScalePercentage)
{
    cairo_surface_flush(&rSurface);

    QImage aImage(cairo_image_surface_get_data(&rSurface), cairo_image_surface_get_width(&rSurface),
                  cairo_image_surface_get_height(&rSurface), Qt_DefaultFormat32);
    if (nScalePercentage > 0)
        aImage.setDevicePixelRatio(nScalePercentage / 100.0f);
    return aImage;
}

QImage* toQImage(cairo_surface_t* pSurface, sal_Int32 nScalePercentage)
{
    if (!pSurface)
        return nullptr;
    return new QImage(toQImage(*pSurface, nScalePercentage));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
