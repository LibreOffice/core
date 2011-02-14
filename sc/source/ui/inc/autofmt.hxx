/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_AUTOFMT_HXX
#define SC_AUTOFMT_HXX

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif
#ifndef SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef SV_MOREBTN_HXX
#include <vcl/morebtn.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#include <svtools/scriptedtext.hxx>
#include <svx/framelinkarray.hxx>
#include "scdllapi.h"

//------------------------------------------------------------------------

class ScAutoFormat;
class ScAutoFormatData;
class SvxBoxItem;
class SvxLineItem;
class ScAutoFmtPreview; // s.u.
class SvNumberFormatter;
class ScDocument;

//------------------------------------------------------------------------

enum AutoFmtLine { TOP_LINE, BOTTOM_LINE, LEFT_LINE, RIGHT_LINE };

//========================================================================

//CHINA001 class ScAutoFormatDlg : public ModalDialog
//CHINA001 {
//CHINA001 public:
//CHINA001 ScAutoFormatDlg( Window*                 pParent,
//CHINA001 ScAutoFormat*                pAutoFormat,
//CHINA001 const ScAutoFormatData*    pSelFormatData,
//CHINA001 ScDocument*                pDoc );
//CHINA001 ~ScAutoFormatDlg();
//CHINA001
//CHINA001 sal_uInt16 GetIndex() const { return nIndex; }
//CHINA001 String GetCurrFormatName();
//CHINA001
//CHINA001 private:
//CHINA001 FixedLine       aFlFormat;
//CHINA001 ListBox          aLbFormat;
//CHINA001 ScAutoFmtPreview*    pWndPreview;
//CHINA001 OKButton     aBtnOk;
//CHINA001 CancelButton aBtnCancel;
//CHINA001 HelpButton       aBtnHelp;
//CHINA001 PushButton       aBtnAdd;
//CHINA001 PushButton       aBtnRemove;
//CHINA001 MoreButton       aBtnMore;
//CHINA001 FixedLine       aFlFormatting;
//CHINA001 CheckBox     aBtnNumFormat;
//CHINA001 CheckBox     aBtnBorder;
//CHINA001 CheckBox     aBtnFont;
//CHINA001 CheckBox     aBtnPattern;
//CHINA001 CheckBox     aBtnAlignment;
//CHINA001 CheckBox     aBtnAdjust;
//CHINA001 PushButton       aBtnRename;
//CHINA001 String           aStrTitle;
//CHINA001 String           aStrLabel;
//CHINA001 String           aStrClose;
//CHINA001 String           aStrDelTitle;
//CHINA001 String           aStrDelMsg;
//CHINA001 String           aStrRename;
//CHINA001
//CHINA001 //------------------------
//CHINA001 ScAutoFormat*            pFormat;
//CHINA001 const ScAutoFormatData*  pSelFmtData;
//CHINA001 sal_uInt16                   nIndex;
//CHINA001 sal_Bool                 bCoreDataChanged;
//CHINA001 sal_Bool                 bFmtInserted;
//CHINA001
//CHINA001 void Init            ();
//CHINA001 void UpdateChecks    ();
//CHINA001 //------------------------
//CHINA001 DECL_LINK( CheckHdl, Button * );
//CHINA001 DECL_LINK( AddHdl, void * );
//CHINA001 DECL_LINK( RemoveHdl, void * );
//CHINA001 DECL_LINK( SelFmtHdl, void * );
//CHINA001 DECL_LINK( CloseHdl, PushButton * );
//CHINA001 DECL_LINK( DblClkHdl, void * );
//CHINA001 DECL_LINK( RenameHdl, void *);
//CHINA001
//CHINA001 };
//CHINA001
//========================================================================

class SC_DLLPUBLIC ScAutoFmtPreview : public Window
{
public:
            ScAutoFmtPreview( Window* pParent, const ResId& rRes, ScDocument* pDoc );
            ~ScAutoFmtPreview();

    void NotifyChange( ScAutoFormatData* pNewData );

protected:
    virtual void Paint( const Rectangle& rRect );

private:
    ScAutoFormatData*       pCurData;
    VirtualDevice           aVD;
    SvtScriptedTextHelper   aScriptedText;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator > xBreakIter;
    sal_Bool                    bFitWidth;
    svx::frame::Array       maArray;            /// Implementation to draw the frame borders.
    bool                    mbRTL;
    Size                    aPrvSize;
    long                    mnLabelColWidth;
    long                    mnDataColWidth1;
    long                    mnDataColWidth2;
    long                    mnRowHeight;
    const String            aStrJan;
    const String            aStrFeb;
    const String            aStrMar;
    const String            aStrNorth;
    const String            aStrMid;
    const String            aStrSouth;
    const String            aStrSum;
    SvNumberFormatter*      pNumFmt;
    //-------------------------------------------
    SC_DLLPRIVATE void  Init            ();
    SC_DLLPRIVATE void  DoPaint         ( const Rectangle& rRect );
    SC_DLLPRIVATE void  CalcCellArray   ( sal_Bool bFitWidth );
    SC_DLLPRIVATE void  CalcLineMap     ();
    SC_DLLPRIVATE void  PaintCells      ();

/*  Usage of type size_t instead of SCCOL/SCROW is correct here - used in
    conjunction with class svx::frame::Array (svx/framelinkarray.hxx), which
    expects size_t coordinates. */

    SC_DLLPRIVATE sal_uInt16              GetFormatIndex( size_t nCol, size_t nRow ) const;
    SC_DLLPRIVATE const SvxBoxItem&   GetBoxItem( size_t nCol, size_t nRow ) const;
    SC_DLLPRIVATE const SvxLineItem&  GetDiagItem( size_t nCol, size_t nRow, bool bTLBR ) const;

    SC_DLLPRIVATE void                DrawString( size_t nCol, size_t nRow );
    SC_DLLPRIVATE void                DrawStrings();
    SC_DLLPRIVATE void                DrawBackground();

    SC_DLLPRIVATE void    MakeFonts       ( sal_uInt16 nIndex,
                              Font& rFont,
                              Font& rCJKFont,
                              Font& rCTLFont );

    SC_DLLPRIVATE String    MakeNumberString( String cellString, sal_Bool bAddDec );
    SC_DLLPRIVATE void  DrawFrameLine   ( const SvxBorderLine&  rLineD,
                              Point                 from,
                              Point                 to,
                              sal_Bool                  bHorizontal,
                              const SvxBorderLine&  rLineLT,
                              const SvxBorderLine&  rLineL,
                              const SvxBorderLine&  rLineLB,
                              const SvxBorderLine&  rLineRT,
                              const SvxBorderLine&  rLineR,
                              const SvxBorderLine&  rLineRB );
    SC_DLLPRIVATE void CheckPriority    ( sal_uInt16            nCurLine,
                              AutoFmtLine       eLine,
                              SvxBorderLine&    rLine );
    SC_DLLPRIVATE void  GetLines        ( sal_uInt16 nIndex, AutoFmtLine eLine,
                              SvxBorderLine&    rLineD,
                              SvxBorderLine&    rLineLT,
                              SvxBorderLine&    rLineL,
                              SvxBorderLine&    rLineLB,
                              SvxBorderLine&    rLineRT,
                              SvxBorderLine&    rLineR,
                              SvxBorderLine&    rLineRB );
};

#endif // SC_AUTOFMT_HXX


