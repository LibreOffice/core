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

#pragma once

#include <vcl/weld.hxx>
#include <svx/graphctl.hxx>

namespace sd {
class DrawDocShell;
}

/******************************************************************************
|*
|* SdVectorizeDlg
|*
\******************************************************************************/

class SdVectorizeDlg : public weld::GenericDialogController
{
    ::sd::DrawDocShell* m_pDocSh;
    Bitmap              aBmp;
    Bitmap              aPreviewBmp;
    GDIMetaFile         aMtf;

    GraphCtrl m_aBmpWin;
    GraphCtrl m_aMtfWin;

    std::unique_ptr<weld::SpinButton> m_xNmLayers;
    std::unique_ptr<weld::MetricSpinButton> m_xMtReduce;
    std::unique_ptr<weld::Label> m_xFtFillHoles;
    std::unique_ptr<weld::MetricSpinButton> m_xMtFillHoles;
    std::unique_ptr<weld::CheckButton> m_xCbFillHoles;
    std::unique_ptr<weld::CustomWeld> m_xBmpWin;
    std::unique_ptr<weld::CustomWeld> m_xMtfWin;
    std::unique_ptr<weld::ProgressBar> m_xPrgs;
    std::unique_ptr<weld::Button> m_xBtnOK;
    std::unique_ptr<weld::Button> m_xBtnPreview;

    void                LoadSettings();
    void                SaveSettings() const;
    void                InitPreviewBmp();

    static ::tools::Rectangle    GetRect( const Size& rDispSize, const Size& rBmpSize );
    Bitmap              GetPreparedBitmap( Bitmap const & rBmp, Fraction& rScale );
    void                Calculate( Bitmap const & rBmp, GDIMetaFile& rMtf );
    static void         AddTile( BitmapReadAccess const * pRAcc, GDIMetaFile& rMtf,
                                 tools::Long nPosX, tools::Long nPosY, tools::Long nWidth, tools::Long nHeight );

    DECL_LINK( ProgressHdl, tools::Long, void );
    DECL_LINK( ClickPreviewHdl, weld::Button&, void );
    DECL_LINK( ClickOKHdl, weld::Button&, void );
    DECL_LINK( ToggleHdl, weld::ToggleButton&, void );
    DECL_LINK( ModifyHdl, weld::SpinButton&, void );
    DECL_LINK( MetricModifyHdl, weld::MetricSpinButton&, void );

public:

    SdVectorizeDlg(weld::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell);
    virtual ~SdVectorizeDlg() override;

    const GDIMetaFile&  GetGDIMetaFile() const { return aMtf; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
