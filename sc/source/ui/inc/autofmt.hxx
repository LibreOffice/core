/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <vcl/virdev.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/morebtn.hxx>
#include <vcl/dialog.hxx>
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
