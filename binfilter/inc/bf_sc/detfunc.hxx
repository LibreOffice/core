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

#include <tools/gen.hxx>

#include <tools/color.hxx>
class String;
namespace binfilter {

class SdrObject;
class SdrPage;

class ScCommentData;
class ScDetectiveData;
class ScDocument;
class ScTripel;
class ScAddress;
class ScRange;

#define SC_DET_MAXCIRCLE	1000

enum ScDetectiveDelete { SC_DET_ALL, SC_DET_DETECTIVE, SC_DET_CIRCLES, SC_DET_COMMENTS, SC_DET_ARROWS };

enum ScDetectiveObjType
{
    SC_DETOBJ_NONE,
    SC_DETOBJ_ARROW,
    SC_DETOBJ_FROMOTHERTAB,
    SC_DETOBJ_TOOTHERTAB,
    SC_DETOBJ_CIRCLE
};

class ScDetectiveFunc
{
    static ColorData nArrowColor;
    static ColorData nErrorColor;
    static ColorData nCommentColor;
    static BOOL		 bColorsInitialized;

    ScDocument*		pDoc;
    USHORT			nTab;

    Point		GetDrawPos( USHORT nCol, USHORT nRow, BOOL bArrow );
    BOOL		HasArrow( USHORT nStartCol, USHORT nStartRow, USHORT nStartTab,
                            USHORT nEndCol, USHORT nEndRow, USHORT nEndTab );

    void		DeleteArrowsAt( USHORT nCol, USHORT nRow, BOOL bDestPnt );
    void		DeleteBox( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 );

    BOOL		HasError( const ScTripel& rStart, const ScTripel& rEnd, ScTripel& rErrPos );

    void		FillAttributes( ScDetectiveData& rData );

                // called from DrawEntry/DrawAlienEntry and InsertObject
    BOOL		InsertArrow( USHORT nCol, USHORT nRow,
                                USHORT nRefStartCol, USHORT nRefStartRow, 
                                USHORT nRefEndCol, USHORT nRefEndRow,
                                BOOL bFromOtherTab, BOOL bRed,
                                ScDetectiveData& rData );
    BOOL		InsertToOtherTab( USHORT nStartCol, USHORT nStartRow,
                                USHORT nEndCol, USHORT nEndRow, BOOL bRed,
                                ScDetectiveData& rData );

                // DrawEntry / DrawAlienEntry check for existing arrows and errors
    BOOL		DrawEntry( USHORT nCol, USHORT nRow, const ScTripel& rRefStart, const ScTripel& rRefEnd,
                                ScDetectiveData& rData );
    BOOL		DrawAlienEntry( const ScTripel& rRefStart, const ScTripel& rRefEnd,
                                ScDetectiveData& rData );

    void		DrawCircle( USHORT nCol, USHORT nRow, ScDetectiveData& rData );


    USHORT		InsertPredLevel( USHORT nCol, USHORT nRow, ScDetectiveData& rData, USHORT nLevel );
    USHORT		InsertPredLevelArea( const ScTripel& rRefStart, const ScTripel& rRefEnd,
                                        ScDetectiveData& rData, USHORT nLevel );
    USHORT		FindPredLevel( USHORT nCol, USHORT nRow, USHORT nLevel, USHORT nDeleteLevel );
    USHORT		FindPredLevelArea( const ScTripel& rRefStart, const ScTripel& rRefEnd,
                                    USHORT nLevel, USHORT nDeleteLevel );

    USHORT		InsertErrorLevel( USHORT nCol, USHORT nRow, ScDetectiveData& rData, USHORT nLevel );

    USHORT		InsertSuccLevel( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                        ScDetectiveData& rData, USHORT nLevel );
    USHORT		FindSuccLevel( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                USHORT nLevel, USHORT nDeleteLevel );

/*N*/ 	BOOL		FindFrameForObject( SdrObject* pObject, ScRange& rRange );


public:
                ScDetectiveFunc(ScDocument* pDocument, USHORT nTable) : pDoc(pDocument),nTab(nTable) {}

    BOOL		ShowSucc( USHORT nCol, USHORT nRow );
    BOOL		ShowPred( USHORT nCol, USHORT nRow );
    BOOL		ShowError( USHORT nCol, USHORT nRow );

    BOOL		DeleteSucc( USHORT nCol, USHORT nRow );
    BOOL		DeletePred( USHORT nCol, USHORT nRow );
    BOOL		DeleteAll( ScDetectiveDelete eWhat );

    BOOL		MarkInvalid(BOOL& rOverflow);

    SdrObject*	ShowComment( USHORT nCol, USHORT nRow, BOOL bForce, SdrPage* pDestPage = NULL );
    SdrObject*	ShowCommentUser( USHORT nCol, USHORT nRow, const String& rUserText,
                                    const Rectangle& rVisible, BOOL bLeft,
                                    BOOL bForce, SdrPage* pDestPage );
    BOOL		HideComment( USHORT nCol, USHORT nRow );

    void		UpdateAllComments();		// on all tables
/*N*/ 	void		UpdateAllArrowColors();		// on all tables


    ScDetectiveObjType GetDetectiveObjectType( SdrObject* pObject,
                                ScAddress& rPosition, ScRange& rSource, BOOL& rRedLine );
    void		InsertObject( ScDetectiveObjType eType, const ScAddress& rPosition,
                                const ScRange& rSource, BOOL bRedLine );

    static ColorData GetArrowColor();
    static ColorData GetErrorColor();
    static ColorData GetCommentColor();
    static void InitializeColors();
};



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
