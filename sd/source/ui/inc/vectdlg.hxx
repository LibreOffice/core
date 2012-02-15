/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




#ifndef _SD_VECTDLG_HXX
#define _SD_VECTDLG_HXX

#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/dialog.hxx>
#include <vcl/gdimtf.hxx>
#include <svx/graphctl.hxx>
#include <svtools/prgsbar.hxx>

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
    FixedLine           aGrpSettings;
    FixedText           aFtLayers;
    NumericField        aNmLayers;
    FixedText           aFtReduce;
    MetricField         aMtReduce;
    FixedText           aFtFillHoles;
    MetricField         aMtFillHoles;
    CheckBox            aCbFillHoles;

    FixedText           aFtOriginal;
    GraphCtrl           aBmpWin;

    FixedText           aFtVectorized;
    GraphCtrl           aMtfWin;

    FixedText           aGrpPrgs;
    ProgressBar         aPrgs;

    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;
    PushButton          aBtnPreview;

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
                        DECL_LINK( ClickPreviewHdl, PushButton* );
                        DECL_LINK( ClickOKHdl, OKButton* );
                        DECL_LINK( ToggleHdl, CheckBox* );
                        DECL_LINK( ModifyHdl, void* );

public:

                        SdVectorizeDlg( Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell );
                        ~SdVectorizeDlg();

    const GDIMetaFile&  GetGDIMetaFile() const { return aMtf; }
};

#endif

