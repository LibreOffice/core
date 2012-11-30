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
    bool                    bFitWidth;
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
    SC_DLLPRIVATE void  CalcCellArray   ( bool bFitWidth );
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
    SC_DLLPRIVATE void  DrawFrameLine   ( const ::editeng::SvxBorderLine&   rLineD,
                              Point                 from,
                              Point                 to,
                              sal_Bool                  bHorizontal,
                              const ::editeng::SvxBorderLine&   rLineLT,
                              const ::editeng::SvxBorderLine&   rLineL,
                              const ::editeng::SvxBorderLine&   rLineLB,
                              const ::editeng::SvxBorderLine&   rLineRT,
                              const ::editeng::SvxBorderLine&   rLineR,
                              const ::editeng::SvxBorderLine&   rLineRB );
    SC_DLLPRIVATE void CheckPriority    ( sal_uInt16            nCurLine,
                              AutoFmtLine       eLine,
                              ::editeng::SvxBorderLine& rLine );
    SC_DLLPRIVATE void  GetLines        ( sal_uInt16 nIndex, AutoFmtLine eLine,
                              ::editeng::SvxBorderLine& rLineD,
                              ::editeng::SvxBorderLine& rLineLT,
                              ::editeng::SvxBorderLine& rLineL,
                              ::editeng::SvxBorderLine& rLineLB,
                              ::editeng::SvxBorderLine& rLineRT,
                              ::editeng::SvxBorderLine& rLineR,
                              ::editeng::SvxBorderLine& rLineRB );
};

#endif // SC_AUTOFMT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
