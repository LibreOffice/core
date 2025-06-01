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

#include <QtData.hxx>

#include <QtGui/QBitmap>
#include <QtGui/QCursor>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyle>

#include <i18nlangtag/languagetag.hxx>
#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <vcl/ImageTree.hxx>

#include <bitmaps.hlst>
#include <cursor_hotspots.hxx>

QtData::QtData()
    : GenericUnixSalData()
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
QtData::~QtData() {}

static QCursor* getQCursorFromIconTheme(const OUString& rIconName, int nXHot, int nYHot)
{
    const OUString sIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();
    const OUString sUILang = Application::GetSettings().GetUILanguageTag().getBcp47();
    auto xMemStream = ImageTree::get().getImageStream(rIconName, sIconTheme, sUILang);
    if (!xMemStream)
        return nullptr;
    auto nLength = xMemStream->TellEnd();
    if (!nLength)
    {
        SAL_WARN("vcl.qt", "Cannot load cursor pixmap from empty stream.");
        return nullptr;
    }

    const unsigned char* pData = static_cast<const unsigned char*>(xMemStream->GetData());
    QPixmap aPixmap;
    aPixmap.loadFromData(pData, nLength);
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

QCursor& QtData::getCursor(PointerStyle ePointerStyle)
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

        m_aCursors[ePointerStyle].reset(pCursor);
    }

    return *m_aCursors[ePointerStyle];
}

void QtData::ErrorTrapPush() {}

bool QtData::ErrorTrapPop(bool /*bIgnoreError*/) { return false; }

bool QtData::noNativeControls()
{
    static const bool bNoNative = (getenv("SAL_VCL_QT_NO_NATIVE") != nullptr);
    return bNoNative;
}

bool QtData::noWeldedWidgets()
{
    static const bool bNoWeldedWidgets = (getenv("SAL_VCL_QT_NO_WELDED_WIDGETS") != nullptr);
    return bNoWeldedWidgets;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
