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
    static BOOL      bColorsInitialized;

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

    BOOL        HasArrow( const ScAddress& rStart,
                            SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab );

    void        DeleteArrowsAt( SCCOL nCol, SCROW nRow, BOOL bDestPnt );
    void        DeleteBox( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );

    BOOL        HasError( const ScRange& rRange, ScAddress& rErrPos );

    void        FillAttributes( ScDetectiveData& rData );

                // called from DrawEntry/DrawAlienEntry and InsertObject
    BOOL        InsertArrow( SCCOL nCol, SCROW nRow,
                                SCCOL nRefStartCol, SCROW nRefStartRow,
                                SCCOL nRefEndCol, SCROW nRefEndRow,
                                BOOL bFromOtherTab, BOOL bRed,
                                ScDetectiveData& rData );
    BOOL        InsertToOtherTab( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, BOOL bRed,
                                ScDetectiveData& rData );

                // DrawEntry / DrawAlienEntry check for existing arrows and errors
    BOOL        DrawEntry( SCCOL nCol, SCROW nRow, const ScRange& rRef,
                                ScDetectiveData& rData );
    BOOL        DrawAlienEntry( const ScRange& rRef,
                                ScDetectiveData& rData );

    void        DrawCircle( SCCOL nCol, SCROW nRow, ScDetectiveData& rData );

    USHORT      InsertPredLevel( SCCOL nCol, SCROW nRow, ScDetectiveData& rData, USHORT nLevel );
    USHORT      InsertPredLevelArea( const ScRange& rRef,
                                        ScDetectiveData& rData, USHORT nLevel );
    USHORT      FindPredLevel( SCCOL nCol, SCROW nRow, USHORT nLevel, USHORT nDeleteLevel );
    USHORT      FindPredLevelArea( const ScRange& rRef,
                                    USHORT nLevel, USHORT nDeleteLevel );

    USHORT      InsertErrorLevel( SCCOL nCol, SCROW nRow, ScDetectiveData& rData, USHORT nLevel );

    USHORT      InsertSuccLevel( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        ScDetectiveData& rData, USHORT nLevel );
    USHORT      FindSuccLevel( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                USHORT nLevel, USHORT nDeleteLevel );

    BOOL        FindFrameForObject( SdrObject* pObject, ScRange& rRange );

    void        Modified();

public:
                ScDetectiveFunc(ScDocument* pDocument, SCTAB nTable) : pDoc(pDocument),nTab(nTable) {}

    BOOL        ShowSucc( SCCOL nCol, SCROW nRow );
    BOOL        ShowPred( SCCOL nCol, SCROW nRow );
    BOOL        ShowError( SCCOL nCol, SCROW nRow );

    BOOL        DeleteSucc( SCCOL nCol, SCROW nRow );
    BOOL        DeletePred( SCCOL nCol, SCROW nRow );
    BOOL        DeleteAll( ScDetectiveDelete eWhat );

    BOOL        MarkInvalid(BOOL& rOverflow);

    void        GetAllPreds(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ::std::vector<ScSharedTokenRef>& rRefTokens);
    void        GetAllSuccs(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ::std::vector<ScSharedTokenRef>& rRefTokens);

    static void UpdateAllComments( ScDocument& rDoc );        // on all tables
    void        UpdateAllArrowColors();     // on all tables

    static BOOL IsNonAlienArrow( SdrObject* pObject );

    ScDetectiveObjType GetDetectiveObjectType( SdrObject* pObject, SCTAB nObjTab,
                                ScAddress& rPosition, ScRange& rSource, BOOL& rRedLine );
    void        InsertObject( ScDetectiveObjType eType, const ScAddress& rPosition,
                                const ScRange& rSource, BOOL bRedLine );

    static ColorData GetArrowColor();
    static ColorData GetErrorColor();
    static ColorData GetCommentColor();
    static void InitializeColors();
    static BOOL IsColorsInitialized();
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
