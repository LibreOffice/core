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

#include <Qt5Data.hxx>

#include <QtGui/QBitmap>
#include <QtGui/QCursor>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyle>

#include <sal/log.hxx>

#include <unx/x11_cursors/ase_curs.h>
#include <unx/x11_cursors/ase_mask.h>
#include <unx/x11_cursors/asn_curs.h>
#include <unx/x11_cursors/asn_mask.h>
#include <unx/x11_cursors/asne_curs.h>
#include <unx/x11_cursors/asne_mask.h>
#include <unx/x11_cursors/asns_curs.h>
#include <unx/x11_cursors/asns_mask.h>
#include <unx/x11_cursors/asnswe_curs.h>
#include <unx/x11_cursors/asnswe_mask.h>
#include <unx/x11_cursors/asnw_curs.h>
#include <unx/x11_cursors/asnw_mask.h>
#include <unx/x11_cursors/ass_curs.h>
#include <unx/x11_cursors/ass_mask.h>
#include <unx/x11_cursors/asse_curs.h>
#include <unx/x11_cursors/asse_mask.h>
#include <unx/x11_cursors/assw_curs.h>
#include <unx/x11_cursors/assw_mask.h>
#include <unx/x11_cursors/asw_curs.h>
#include <unx/x11_cursors/asw_mask.h>
#include <unx/x11_cursors/aswe_curs.h>
#include <unx/x11_cursors/aswe_mask.h>
#include <unx/x11_cursors/chain_curs.h>
#include <unx/x11_cursors/chain_mask.h>
#include <unx/x11_cursors/chainnot_curs.h>
#include <unx/x11_cursors/chainnot_mask.h>
#include <unx/x11_cursors/chart_curs.h>
#include <unx/x11_cursors/chart_mask.h>
#include <unx/x11_cursors/copydata_curs.h>
#include <unx/x11_cursors/copydata_mask.h>
#include <unx/x11_cursors/copydlnk_curs.h>
#include <unx/x11_cursors/copydlnk_mask.h>
#include <unx/x11_cursors/copyfile_curs.h>
#include <unx/x11_cursors/copyfile_mask.h>
#include <unx/x11_cursors/copyfiles_curs.h>
#include <unx/x11_cursors/copyfiles_mask.h>
#include <unx/x11_cursors/copyflnk_curs.h>
#include <unx/x11_cursors/copyflnk_mask.h>
#include <unx/x11_cursors/crook_curs.h>
#include <unx/x11_cursors/crook_mask.h>
#include <unx/x11_cursors/crop_curs.h>
#include <unx/x11_cursors/crop_mask.h>
#include <unx/x11_cursors/detective_curs.h>
#include <unx/x11_cursors/detective_mask.h>
#include <unx/x11_cursors/drawarc_curs.h>
#include <unx/x11_cursors/drawarc_mask.h>
#include <unx/x11_cursors/drawbezier_curs.h>
#include <unx/x11_cursors/drawbezier_mask.h>
#include <unx/x11_cursors/drawcaption_curs.h>
#include <unx/x11_cursors/drawcaption_mask.h>
#include <unx/x11_cursors/drawcirclecut_curs.h>
#include <unx/x11_cursors/drawcirclecut_mask.h>
#include <unx/x11_cursors/drawconnect_curs.h>
#include <unx/x11_cursors/drawconnect_mask.h>
#include <unx/x11_cursors/drawellipse_curs.h>
#include <unx/x11_cursors/drawellipse_mask.h>
#include <unx/x11_cursors/drawfreehand_curs.h>
#include <unx/x11_cursors/drawfreehand_mask.h>
#include <unx/x11_cursors/drawline_curs.h>
#include <unx/x11_cursors/drawline_mask.h>
#include <unx/x11_cursors/drawpie_curs.h>
#include <unx/x11_cursors/drawpie_mask.h>
#include <unx/x11_cursors/drawpolygon_curs.h>
#include <unx/x11_cursors/drawpolygon_mask.h>
#include <unx/x11_cursors/drawrect_curs.h>
#include <unx/x11_cursors/drawrect_mask.h>
#include <unx/x11_cursors/drawtext_curs.h>
#include <unx/x11_cursors/drawtext_mask.h>
#include <unx/x11_cursors/fill_curs.h>
#include <unx/x11_cursors/fill_mask.h>
#include <unx/x11_cursors/hshear_curs.h>
#include <unx/x11_cursors/hshear_mask.h>
#include <unx/x11_cursors/linkdata_curs.h>
#include <unx/x11_cursors/linkdata_mask.h>
#include <unx/x11_cursors/linkfile_curs.h>
#include <unx/x11_cursors/linkfile_mask.h>
#include <unx/x11_cursors/magnify_curs.h>
#include <unx/x11_cursors/magnify_mask.h>
#include <unx/x11_cursors/mirror_curs.h>
#include <unx/x11_cursors/mirror_mask.h>
#include <unx/x11_cursors/movebezierweight_curs.h>
#include <unx/x11_cursors/movebezierweight_mask.h>
#include <unx/x11_cursors/movedata_curs.h>
#include <unx/x11_cursors/movedata_mask.h>
#include <unx/x11_cursors/movedlnk_curs.h>
#include <unx/x11_cursors/movedlnk_mask.h>
#include <unx/x11_cursors/movefile_curs.h>
#include <unx/x11_cursors/movefile_mask.h>
#include <unx/x11_cursors/movefiles_curs.h>
#include <unx/x11_cursors/movefiles_mask.h>
#include <unx/x11_cursors/moveflnk_curs.h>
#include <unx/x11_cursors/moveflnk_mask.h>
#include <unx/x11_cursors/movepoint_curs.h>
#include <unx/x11_cursors/movepoint_mask.h>
#include <unx/x11_cursors/nodrop_curs.h>
#include <unx/x11_cursors/nodrop_mask.h>
#include <unx/x11_cursors/pivotcol_curs.h>
#include <unx/x11_cursors/pivotcol_mask.h>
#include <unx/x11_cursors/pivotdel_curs.h>
#include <unx/x11_cursors/pivotdel_mask.h>
#include <unx/x11_cursors/pivotfld_curs.h>
#include <unx/x11_cursors/pivotfld_mask.h>
#include <unx/x11_cursors/pivotrow_curs.h>
#include <unx/x11_cursors/pivotrow_mask.h>
#include <unx/x11_cursors/rotate_curs.h>
#include <unx/x11_cursors/rotate_mask.h>
#include <unx/x11_cursors/tblsele_curs.h>
#include <unx/x11_cursors/tblsele_mask.h>
#include <unx/x11_cursors/tblsels_curs.h>
#include <unx/x11_cursors/tblsels_mask.h>
#include <unx/x11_cursors/tblselse_curs.h>
#include <unx/x11_cursors/tblselse_mask.h>
#include <unx/x11_cursors/tblselsw_curs.h>
#include <unx/x11_cursors/tblselsw_mask.h>
#include <unx/x11_cursors/tblselw_curs.h>
#include <unx/x11_cursors/tblselw_mask.h>
#include <unx/x11_cursors/vertcurs_curs.h>
#include <unx/x11_cursors/vertcurs_mask.h>
#include <unx/x11_cursors/vshear_curs.h>
#include <unx/x11_cursors/vshear_mask.h>
#include <unx/x11_cursors/wshide_curs.h>
#include <unx/x11_cursors/wshide_mask.h>
#include <unx/x11_cursors/wsshow_curs.h>
#include <unx/x11_cursors/wsshow_mask.h>

#include <unx/glyphcache.hxx>

Qt5Data::Qt5Data(SalInstance* pInstance)
    : GenericUnixSalData(SAL_DATA_QT5, pInstance)
{
    ImplSVData* pSVData = ImplGetSVData();

    pSVData->maNWFData.mbDockingAreaSeparateTB = true;
    pSVData->maNWFData.mbFlatMenu = true;
    pSVData->maNWFData.mbRolloverMenubar = true;
    pSVData->maNWFData.mbNoFocusRects = true;
    pSVData->maNWFData.mbNoFocusRectsForFlatButtons = true;

    QStyle* style = QApplication::style();
    pSVData->maNWFData.mnMenuFormatBorderX = style->pixelMetric(QStyle::PM_MenuPanelWidth)
                                             + style->pixelMetric(QStyle::PM_MenuHMargin);
    pSVData->maNWFData.mnMenuFormatBorderY = style->pixelMetric(QStyle::PM_MenuPanelWidth)
                                             + style->pixelMetric(QStyle::PM_MenuVMargin);
}

// outline dtor b/c of FreetypeManager incomplete type
Qt5Data::~Qt5Data() {}

static QCursor* getQCursorFromXBM(const unsigned char* pBitmap, const unsigned char* pMask,
                                  int nWidth, int nHeight, int nXHot, int nYHot)
{
    QBitmap aPixmap = QBitmap::fromData(QSize(nWidth, nHeight), pBitmap);
    QBitmap aMask = QBitmap::fromData(QSize(nWidth, nHeight), pMask);
    return new QCursor(aPixmap, aMask, nXHot, nYHot);
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
            SAL_WARN("vcl.qt5",
                     "pointer " << static_cast<int>(ePointerStyle) << " not implemented");
        }

        m_aCursors[ePointerStyle].reset(pCursor);
    }

    return *m_aCursors[ePointerStyle];
}

void Qt5Data::ErrorTrapPush() {}

bool Qt5Data::ErrorTrapPop(bool /*bIgnoreError*/) { return false; }

bool Qt5Data::noNativeControls()
{
    static const bool bNoNative
        = ((nullptr != getenv("SAL_VCL_QT5_NO_NATIVE")) && (nullptr != ImplGetSVData())
           && ImplGetSVData()->maAppData.mxToolkitName
           && ImplGetSVData()->maAppData.mxToolkitName->match("qt5"));
    return bNoNative;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
