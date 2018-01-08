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

#include "Qt5Data.hxx"

#include <QtGui/QCursor>
#include <QtGui/QBitmap>

#include <unx/x11_cursors/salcursors.h>

Qt5Data::Qt5Data(SalInstance* pInstance)
    : GenericUnixSalData(SAL_DATA_QT5, pInstance)
{
    ImplSVData* pSVData = ImplGetSVData();

    // draw toolbars on separate lines
    pSVData->maNWFData.mbDockingAreaSeparateTB = true;
}

Qt5Data::~Qt5Data() {}

static QCursor* getQCursorFromXBM(const unsigned char* pBitmap, const unsigned char* pMask,
                                  int nWidth, int nHeight, int nXHot, int nYHot)
{
    QBitmap aPixmap;
    aPixmap.loadFromData(pBitmap, nWidth * nHeight / 8, "XPM");
    QBitmap aMask;
    aMask.loadFromData(pMask, nWidth * nHeight / 8, "XPM");
    aPixmap.setMask(aMask);
    return new QCursor(aPixmap, nXHot, nYHot);
}

#define MAKE_CURSOR(vcl_name, name)                                                                \
    case vcl_name:                                                                                 \
        pCursor = getQCursorFromXBM(name##curs##_bits, name##mask##_bits, name##curs_width,        \
                                    name##curs_height, name##curs_x_hot, name##curs_y_hot);        \
        break

#define MAP_BUILTIN(vcl_name, qt_enum)                                                             \
    case vcl_name:                                                                                 \
        pCursor = new QCursor(qt_enum);                                                            \
        break

QCursor& Qt5Data::getCursor(PointerStyle ePointerStyle)
{
    if (!m_aCursors[ePointerStyle])
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
#if 0
            MAP_BUILTIN( PointerStyle::WindowNSize, GDK_TOP_SIDE );
            MAP_BUILTIN( PointerStyle::WindowSSize, GDK_BOTTOM_SIDE );
            MAP_BUILTIN( PointerStyle::WindowWSize, GDK_LEFT_SIDE );
            MAP_BUILTIN( PointerStyle::WindowESize, GDK_RIGHT_SIDE );
#endif
            MAP_BUILTIN(PointerStyle::WindowNWSize, Qt::SizeFDiagCursor);
            MAP_BUILTIN(PointerStyle::WindowNESize, Qt::SizeBDiagCursor);
            MAP_BUILTIN(PointerStyle::WindowSWSize, Qt::SizeBDiagCursor);
            MAP_BUILTIN(PointerStyle::WindowSESize, Qt::SizeFDiagCursor);

            MAP_BUILTIN(PointerStyle::HSizeBar, Qt::SizeHorCursor);
            MAP_BUILTIN(PointerStyle::VSizeBar, Qt::SizeVerCursor);

            MAP_BUILTIN(PointerStyle::RefHand, Qt::OpenHandCursor);
            MAP_BUILTIN(PointerStyle::Hand, Qt::OpenHandCursor);
#if 0
            MAP_BUILTIN( PointerStyle::Pen, GDK_PENCIL );
#endif
            MAP_BUILTIN(PointerStyle::HSplit, Qt::SizeHorCursor);
            MAP_BUILTIN(PointerStyle::VSplit, Qt::SizeVerCursor);

            MAP_BUILTIN(PointerStyle::Move, Qt::SizeAllCursor);

            MAP_BUILTIN(PointerStyle::Null, Qt::BlankCursor);
            MAKE_CURSOR(PointerStyle::Magnify, magnify_);
            MAKE_CURSOR(PointerStyle::Fill, fill_);
            MAKE_CURSOR(PointerStyle::MoveData, movedata_);
            MAKE_CURSOR(PointerStyle::CopyData, copydata_);
            MAKE_CURSOR(PointerStyle::MoveFile, movefile_);
            MAKE_CURSOR(PointerStyle::CopyFile, copyfile_);
            MAKE_CURSOR(PointerStyle::MoveFiles, movefiles_);
            MAKE_CURSOR(PointerStyle::CopyFiles, copyfiles_);
            MAKE_CURSOR(PointerStyle::NotAllowed, nodrop_);
            MAKE_CURSOR(PointerStyle::Rotate, rotate_);
            MAKE_CURSOR(PointerStyle::HShear, hshear_);
            MAKE_CURSOR(PointerStyle::VShear, vshear_);
            MAKE_CURSOR(PointerStyle::DrawLine, drawline_);
            MAKE_CURSOR(PointerStyle::DrawRect, drawrect_);
            MAKE_CURSOR(PointerStyle::DrawPolygon, drawpolygon_);
            MAKE_CURSOR(PointerStyle::DrawBezier, drawbezier_);
            MAKE_CURSOR(PointerStyle::DrawArc, drawarc_);
            MAKE_CURSOR(PointerStyle::DrawPie, drawpie_);
            MAKE_CURSOR(PointerStyle::DrawCircleCut, drawcirclecut_);
            MAKE_CURSOR(PointerStyle::DrawEllipse, drawellipse_);
            MAKE_CURSOR(PointerStyle::DrawConnect, drawconnect_);
            MAKE_CURSOR(PointerStyle::DrawText, drawtext_);
            MAKE_CURSOR(PointerStyle::Mirror, mirror_);
            MAKE_CURSOR(PointerStyle::Crook, crook_);
            MAKE_CURSOR(PointerStyle::Crop, crop_);
            MAKE_CURSOR(PointerStyle::MovePoint, movepoint_);
            MAKE_CURSOR(PointerStyle::MoveBezierWeight, movebezierweight_);
            MAKE_CURSOR(PointerStyle::DrawFreehand, drawfreehand_);
            MAKE_CURSOR(PointerStyle::DrawCaption, drawcaption_);
            MAKE_CURSOR(PointerStyle::LinkData, linkdata_);
            MAKE_CURSOR(PointerStyle::MoveDataLink, movedlnk_);
            MAKE_CURSOR(PointerStyle::CopyDataLink, copydlnk_);
            MAKE_CURSOR(PointerStyle::LinkFile, linkfile_);
            MAKE_CURSOR(PointerStyle::MoveFileLink, moveflnk_);
            MAKE_CURSOR(PointerStyle::CopyFileLink, copyflnk_);
            MAKE_CURSOR(PointerStyle::Chart, chart_);
            MAKE_CURSOR(PointerStyle::Detective, detective_);
            MAKE_CURSOR(PointerStyle::PivotCol, pivotcol_);
            MAKE_CURSOR(PointerStyle::PivotRow, pivotrow_);
            MAKE_CURSOR(PointerStyle::PivotField, pivotfld_);
            MAKE_CURSOR(PointerStyle::PivotDelete, pivotdel_);
            MAKE_CURSOR(PointerStyle::Chain, chain_);
            MAKE_CURSOR(PointerStyle::ChainNotAllowed, chainnot_);
            MAKE_CURSOR(PointerStyle::AutoScrollN, asn_);
            MAKE_CURSOR(PointerStyle::AutoScrollS, ass_);
            MAKE_CURSOR(PointerStyle::AutoScrollW, asw_);
            MAKE_CURSOR(PointerStyle::AutoScrollE, ase_);
            MAKE_CURSOR(PointerStyle::AutoScrollNW, asnw_);
            MAKE_CURSOR(PointerStyle::AutoScrollNE, asne_);
            MAKE_CURSOR(PointerStyle::AutoScrollSW, assw_);
            MAKE_CURSOR(PointerStyle::AutoScrollSE, asse_);
            MAKE_CURSOR(PointerStyle::AutoScrollNS, asns_);
            MAKE_CURSOR(PointerStyle::AutoScrollWE, aswe_);
            MAKE_CURSOR(PointerStyle::AutoScrollNSWE, asnswe_);
            MAKE_CURSOR(PointerStyle::TextVertical, vertcurs_);

            MAKE_CURSOR(PointerStyle::TabSelectS, tblsels_);
            MAKE_CURSOR(PointerStyle::TabSelectE, tblsele_);
            MAKE_CURSOR(PointerStyle::TabSelectSE, tblselse_);
            MAKE_CURSOR(PointerStyle::TabSelectW, tblselw_);
            MAKE_CURSOR(PointerStyle::TabSelectSW, tblselsw_);

            MAKE_CURSOR(PointerStyle::HideWhitespace, hidewhitespace_);
            MAKE_CURSOR(PointerStyle::ShowWhitespace, showwhitespace_);
            default:
                break;
        }
        if (!pCursor)
        {
            pCursor = new QCursor(Qt::ArrowCursor);
            SAL_WARN("vcl.qt5", "pointer " << static_cast<int>(ePointerStyle) << "not implemented");
        }

        m_aCursors[ePointerStyle].reset(pCursor);
    }

    return *m_aCursors[ePointerStyle];
}

void Qt5Data::ErrorTrapPush() {}

bool Qt5Data::ErrorTrapPop(bool /*bIgnoreError*/) { return false; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
