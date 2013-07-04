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

#ifndef SC_DETFUNC_HXX
#define SC_DETFUNC_HXX

#include "address.hxx"
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include "scdllapi.h"
#include "token.hxx"

#include <vector>

class SdrObject;

class ScDetectiveData;
class ScDocument;
class ScAddress;
class ScRange;

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
        DRAWPOS_TOPLEFT,        ///< Top-left edge of the cell.
        DRAWPOS_BOTTOMRIGHT,    ///< Bottom-right edge of the cell.
        DRAWPOS_DETARROW,       ///< Position inside cell for detective arrows.
        DRAWPOS_CAPTIONLEFT,    ///< Top-left edge of the cell for captions.
        DRAWPOS_CAPTIONRIGHT    ///< Top-right edge of the cell for captions (incl. merged cells).
    };

    /** @return a drawing layer position for the passed cell address. */
    Point       GetDrawPos( SCCOL nCol, SCROW nRow, DrawPosMode eMode ) const;

    /** @return the drawing layer rectangle for the passed cell range. */
    Rectangle   GetDrawRect( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const;

    /** @return the drawing layer rectangle for the passed cell address. */
    Rectangle   GetDrawRect( SCCOL nCol, SCROW nRow ) const;

    sal_Bool        HasArrow( const ScAddress& rStart,
                            SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab );

    void        DeleteArrowsAt( SCCOL nCol, SCROW nRow, sal_Bool bDestPnt );
    void        DeleteBox( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );

    sal_Bool        HasError( const ScRange& rRange, ScAddress& rErrPos );

    void        FillAttributes( ScDetectiveData& rData );

                /// called from DrawEntry/DrawAlienEntry and InsertObject
    sal_Bool        InsertArrow( SCCOL nCol, SCROW nRow,
                                SCCOL nRefStartCol, SCROW nRefStartRow,
                                SCCOL nRefEndCol, SCROW nRefEndRow,
                                sal_Bool bFromOtherTab, sal_Bool bRed,
                                ScDetectiveData& rData );
    sal_Bool        InsertToOtherTab( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, sal_Bool bRed,
                                ScDetectiveData& rData );

                /// DrawEntry / DrawAlienEntry check for existing arrows and errors
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

    static void UpdateAllComments( ScDocument& rDoc );        ///< on all tables
    void        UpdateAllArrowColors();     ///< on all tables

    static sal_Bool IsNonAlienArrow( SdrObject* pObject );

    ScDetectiveObjType GetDetectiveObjectType( SdrObject* pObject, SCTAB nObjTab,
                                ScAddress& rPosition, ScRange& rSource, bool& rRedLine );
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
