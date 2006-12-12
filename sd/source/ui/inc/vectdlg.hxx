/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vectdlg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:53:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef _SD_VECTDLG_HXX
#define _SD_VECTDLG_HXX

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_GDIMTF_HXX //autogen
#include <vcl/gdimtf.hxx>
#endif
#ifndef _GRAPHCTL_HXX //autogen
#include <svx/graphctl.hxx>
#endif
#ifndef _PRGSBAR_HXX
#include <svtools/prgsbar.hxx>
#endif

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

