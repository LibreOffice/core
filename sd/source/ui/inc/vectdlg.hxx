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
    VclPtr<NumericField>       m_pNmLayers;
    VclPtr<MetricField>        m_pMtReduce;
    VclPtr<FixedText>          m_pFtFillHoles;
    VclPtr<MetricField>        m_pMtFillHoles;
    VclPtr<CheckBox>           m_pCbFillHoles;

    VclPtr<GraphCtrl>          m_pBmpWin;

    VclPtr<GraphCtrl>          m_pMtfWin;

    VclPtr<ProgressBar>        m_pPrgs;

    VclPtr<OKButton>           m_pBtnOK;
    VclPtr<PushButton>         m_pBtnPreview;

    Bitmap              aBmp;
    Bitmap              aPreviewBmp;
    GDIMetaFile         aMtf;

    void                LoadSettings();
    void                SaveSettings() const;
    void                InitPreviewBmp();

    static ::tools::Rectangle    GetRect( const Size& rDispSize, const Size& rBmpSize );
    Bitmap              GetPreparedBitmap( Bitmap const & rBmp, Fraction& rScale );
    void                Calculate( Bitmap const & rBmp, GDIMetaFile& rMtf );
    void                AddTile( BitmapReadAccess const * pRAcc, GDIMetaFile& rMtf,
                                 long nPosX, long nPosY, long nWidth, long nHeight );

                        DECL_LINK( ProgressHdl, long, void );
                        DECL_LINK( ClickPreviewHdl, Button*, void );
                        DECL_LINK( ClickOKHdl, Button*, void );
                        DECL_LINK( ToggleHdl, CheckBox&, void );
                        DECL_LINK( ModifyHdl, Edit&, void );

public:

                        SdVectorizeDlg( vcl::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell );
                        virtual ~SdVectorizeDlg() override;
    virtual void        dispose() override;

    const GDIMetaFile&  GetGDIMetaFile() const { return aMtf; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
