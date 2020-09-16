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

#ifndef INCLUDED_SC_INC_DETFUNC_HXX
#define INCLUDED_SC_INC_DETFUNC_HXX

#include "address.hxx"
#include <tools/gen.hxx>
#include <tools/color.hxx>
#include "scdllapi.h"

#include <vector>

class SdrObject;

class ScDetectiveData;
class ScDocument;

#define SC_DET_MAXCIRCLE    1000

enum class ScDetectiveDelete { Detective, Circles, Arrows };

enum ScDetectiveObjType
{
    SC_DETOBJ_NONE,
    SC_DETOBJ_ARROW,
    SC_DETOBJ_FROMOTHERTAB,
    SC_DETOBJ_TOOTHERTAB,
    SC_DETOBJ_CIRCLE,
    SC_DETOBJ_RECTANGLE
};

class SC_DLLPUBLIC ScDetectiveFunc
{
    static Color     nArrowColor;
    static Color     nErrorColor;
    static Color     nCommentColor;
    static bool      bColorsInitialized;

    ScDocument&     rDoc;
    SCTAB           nTab;

    enum class DrawPosMode
    {
        TopLeft,        ///< Top-left edge of the cell.
        BottomRight,    ///< Bottom-right edge of the cell.
        DetectiveArrow, ///< Position inside cell for detective arrows.
    };

    /** @return a drawing layer position for the passed cell address. */
    Point       GetDrawPos( SCCOL nCol, SCROW nRow, DrawPosMode eMode ) const;

    /** @return the drawing layer rectangle for the passed cell range. */
    tools::Rectangle   GetDrawRect( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const;

    /** @return the drawing layer rectangle for the passed cell address. */
    tools::Rectangle   GetDrawRect( SCCOL nCol, SCROW nRow ) const;

    bool        HasArrow( const ScAddress& rStart,
                            SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab );

    void        DeleteArrowsAt( SCCOL nCol, SCROW nRow, bool bDestPnt );
    void        DeleteBox( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );

    bool        HasError( const ScRange& rRange, ScAddress& rErrPos );

                /// called from DrawEntry/DrawAlienEntry and InsertObject
    void        InsertArrow( SCCOL nCol, SCROW nRow,
                                SCCOL nRefStartCol, SCROW nRefStartRow,
                                SCCOL nRefEndCol, SCROW nRefEndRow,
                                bool bFromOtherTab, bool bRed,
                                ScDetectiveData& rData );
    void        InsertToOtherTab( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, bool bRed,
                                ScDetectiveData& rData );

                /// DrawEntry / DrawAlienEntry check for existing arrows and errors
    bool        DrawEntry( SCCOL nCol, SCROW nRow, const ScRange& rRef,
                                ScDetectiveData& rData );
    bool        DrawAlienEntry( const ScRange& rRef,
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

    void        FindFrameForObject( const SdrObject* pObject, ScRange& rRange );

    void        Modified();

public:
                ScDetectiveFunc(ScDocument& rDocument, SCTAB nTable) : rDoc(rDocument),nTab(nTable) {}

    bool        ShowSucc( SCCOL nCol, SCROW nRow );
    bool        ShowPred( SCCOL nCol, SCROW nRow );
    bool        ShowError( SCCOL nCol, SCROW nRow );

    bool        DeleteSucc( SCCOL nCol, SCROW nRow );
    bool        DeletePred( SCCOL nCol, SCROW nRow );
    bool        DeleteAll( ScDetectiveDelete eWhat );

    bool        MarkInvalid(bool& rOverflow);

    void        GetAllPreds(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ::std::vector<ScTokenRef>& rRefTokens);
    void        GetAllSuccs(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ::std::vector<ScTokenRef>& rRefTokens);

    static void UpdateAllComments( ScDocument& rDoc );        ///< on all tables
    void        UpdateAllArrowColors();     ///< on all tables

    static bool IsNonAlienArrow( const SdrObject* pObject );

    ScDetectiveObjType GetDetectiveObjectType( SdrObject* pObject, SCTAB nObjTab,
                                ScAddress& rPosition, ScRange& rSource, bool& rRedLine );
    void        InsertObject( ScDetectiveObjType eType, const ScAddress& rPosition,
                                const ScRange& rSource, bool bRedLine );

    static Color GetArrowColor();
    static Color GetErrorColor();
    static Color GetCommentColor();
    static void InitializeColors();
    static bool IsColorsInitialized();
    static void AppendChangTrackNoteSeparator(OUString &str);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
