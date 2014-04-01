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


#ifndef INCLUDED_SD_SOURCE_UI_INC_VECTDLG_HXX
#define INCLUDED_SD_SOURCE_UI_INC_VECTDLG_HXX

#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/dialog.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/prgsbar.hxx>
#include <svx/graphctl.hxx>

namespace sd {
class DrawDocShell;
}

/******************************************************************************
|*
|* SdVectorizeDlg
|*
\******************************************************************************/

class SdVectorizeDlg : public ModalDialog
{
    ::sd::DrawDocShell *    mpDocSh;
    NumericField*       m_pNmLayers;
    MetricField*        m_pMtReduce;
    FixedText*          m_pFtFillHoles;
    MetricField*        m_pMtFillHoles;
    CheckBox*           m_pCbFillHoles;

    GraphCtrl*          m_pBmpWin;

    GraphCtrl*          m_pMtfWin;

    ProgressBar*        m_pPrgs;

    OKButton*           m_pBtnOK;
    PushButton*         m_pBtnPreview;

    Bitmap              aBmp;
    Bitmap              aPreviewBmp;
    GDIMetaFile         aMtf;

    void                LoadSettings();
    void                SaveSettings() const;
    void                InitPreviewBmp();
    void                UpdatePreviewMtf();

    Rectangle           GetRect( const Size& rDispSize, const Size& rBmpSize ) const;
    Bitmap              GetPreparedBitmap( Bitmap& rBmp, Fraction& rScale );
    void                Calculate( Bitmap& rBmp, GDIMetaFile& rMtf );
    void                AddTile( BitmapReadAccess* pRAcc, GDIMetaFile& rMtf,
                                 long nPosX, long nPosY, long nWidth, long nHeight );

                        DECL_LINK( ProgressHdl, void* );
                        DECL_LINK( ClickPreviewHdl, void* );
                        DECL_LINK( ClickOKHdl, void* );
                        DECL_LINK( ToggleHdl, CheckBox* );
                        DECL_LINK( ModifyHdl, void* );

public:

                        SdVectorizeDlg( Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell );
                        virtual ~SdVectorizeDlg();

    const GDIMetaFile&  GetGDIMetaFile() const { return aMtf; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
