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

#ifndef SC_DETFUNC_HXX
#define SC_DETFUNC_HXX

#include "address.hxx"
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include "scdllapi.h"
#include "token.hxx"

#include <vector>

class SdrObject;
class SdrPage;
class String;

class ScPostIt;
class ScCommentData;
class ScDetectiveData;
class ScDocument;
class ScAddress;
class ScRange;
class ScRangeList;

#define SC_DET_MAXCIRCLE    1000

enum ScDetectiveDelete { SC_DET_ALL, SC_DET_DETECTIVE, SC_DET_CIRCLES, SC_DET_ARROWS };

enum ScDetectiveObjType
{
    SC_DETOBJ_NONE,
    SC_DETOBJ_ARROW,
    SC_DETOBJ_FROMOTHERTAB,
    SC_DETOBJ_TOOTHERTAB,
    SC_DETOBJ_CIRCLE
};

class SC_DLLPUBLIC ScDetectiveFunc
{
    static ColorData nArrowColor;
    static ColorData nErrorColor;
    static ColorData nCommentColor;
    static sal_Bool      bColorsInitialized;

    ScDocument*     pDoc;
    SCTAB           nTab;

    enum DrawPosMode
    {
        DRAWPOS_TOPLEFT,        /// Top-left edge of the cell.
        DRAWPOS_BOTTOMRIGHT,    /// Bottom-right edge of the cell.
        DRAWPOS_DETARROW,       /// Position inside cell for detective arrows.
        DRAWPOS_CAPTIONLEFT,    /// Top-left edge of the cell for captions.
        DRAWPOS_CAPTIONRIGHT    /// Top-right edge of the cell for captions (incl. merged cells).
    };

    /** Returns a drawing layer position for the passed cell address. */
    Point       GetDrawPos( SCCOL nCol, SCROW nRow, DrawPosMode eMode ) const;

    /** Returns the drawing layer rectangle for the passed cell range. */
    Rectangle   GetDrawRect( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const;

    /** Returns the drawing layer rectangle for the passed cell address. */
    Rectangle   GetDrawRect( SCCOL nCol, SCROW nRow ) const;

    sal_Bool        HasArrow( const ScAddress& rStart,
                            SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab );

    void        DeleteArrowsAt( SCCOL nCol, SCROW nRow, sal_Bool bDestPnt );
    void        DeleteBox( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );

    sal_Bool        HasError( const ScRange& rRange, ScAddress& rErrPos );

    void        FillAttributes( ScDetectiveData& rData );

                // called from DrawEntry/DrawAlienEntry and InsertObject
    sal_Bool        InsertArrow( SCCOL nCol, SCROW nRow,
                                SCCOL nRefStartCol, SCROW nRefStartRow,
                                SCCOL nRefEndCol, SCROW nRefEndRow,
                                sal_Bool bFromOtherTab, sal_Bool bRed,
                                ScDetectiveData& rData );
    sal_Bool        InsertToOtherTab( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, sal_Bool bRed,
                                ScDetectiveData& rData );

                // DrawEntry / DrawAlienEntry check for existing arrows and errors
    sal_Bool        DrawEntry( SCCOL nCol, SCROW nRow, const ScRange& rRef,
                                ScDetectiveData& rData );
    sal_Bool        DrawAlienEntry( const ScRange& rRef,
                                ScDetectiveData& rData );

    void        DrawCircle( SCCOL nCol, SCROW nRow, ScDetectiveData& rData );

    sal_uInt16      InsertPredLevel( SCCOL nCol, SCROW nRow, ScDetectiveData& rData, sal_uInt16 nLevel );
    sal_uInt16      InsertPredLevelArea( const ScRange& rRef,
                                        ScDetectiveData& rData, sal_uInt16 nLevel );
    sal_uInt16      FindPredLevel( SCCOL nCol, SCROW nRow, sal_uInt16 nLevel, sal_uInt16 nDeleteLevel );
    sal_uInt16      FindPredLevelArea( const ScRange& rRef,
                                    sal_uInt16 nLevel, sal_uInt16 nDeleteLevel );

    sal_uInt16      InsertErrorLevel( SCCOL nCol, SCROW nRow, ScDetectiveData& rData, sal_uInt16 nLevel );

    sal_uInt16      InsertSuccLevel( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        ScDetectiveData& rData, sal_uInt16 nLevel );
    sal_uInt16      FindSuccLevel( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                sal_uInt16 nLevel, sal_uInt16 nDeleteLevel );

    sal_Bool        FindFrameForObject( SdrObject* pObject, ScRange& rRange );

    void        Modified();

public:
                ScDetectiveFunc(ScDocument* pDocument, SCTAB nTable) : pDoc(pDocument),nTab(nTable) {}

    sal_Bool        ShowSucc( SCCOL nCol, SCROW nRow );
    sal_Bool        ShowPred( SCCOL nCol, SCROW nRow );
    sal_Bool        ShowError( SCCOL nCol, SCROW nRow );

    sal_Bool        DeleteSucc( SCCOL nCol, SCROW nRow );
    sal_Bool        DeletePred( SCCOL nCol, SCROW nRow );
    sal_Bool        DeleteAll( ScDetectiveDelete eWhat );

    sal_Bool        MarkInvalid(sal_Bool& rOverflow);

    void        GetAllPreds(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ::std::vector<ScTokenRef>& rRefTokens);
    void        GetAllSuccs(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ::std::vector<ScTokenRef>& rRefTokens);

    static void UpdateAllComments( ScDocument& rDoc );        // on all tables
    void        UpdateAllArrowColors();     // on all tables

    static sal_Bool IsNonAlienArrow( SdrObject* pObject );

    ScDetectiveObjType GetDetectiveObjectType( SdrObject* pObject, SCTAB nObjTab,
                                ScAddress& rPosition, ScRange& rSource, sal_Bool& rRedLine );
    void        InsertObject( ScDetectiveObjType eType, const ScAddress& rPosition,
                                const ScRange& rSource, sal_Bool bRedLine );

    static ColorData GetArrowColor();
    static ColorData GetErrorColor();
    static ColorData GetCommentColor();
    static void InitializeColors();
    static sal_Bool IsColorsInitialized();
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
