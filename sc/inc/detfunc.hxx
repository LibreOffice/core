/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: detfunc.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:23:03 $
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

#ifndef SC_DETFUNC_HXX
#define SC_DETFUNC_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

class SdrObject;
class SdrPage;
class String;

class ScCommentData;
class ScDetectiveData;
class ScDocument;
class ScAddress;
class ScRange;

#define SC_DET_MAXCIRCLE    1000

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
    static BOOL      bColorsInitialized;

    ScDocument*     pDoc;
    SCTAB           nTab;

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

    SdrObject*  DrawCaption( SCCOL nCol, SCROW nRow, const String& rText,
                                ScCommentData& rData, SdrPage* pDestPage,
                                BOOL bHasUserText, BOOL bLeft,
                                const Rectangle& rVisible );

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


public:
                ScDetectiveFunc(ScDocument* pDocument, SCTAB nTable) : pDoc(pDocument),nTab(nTable) {}

    Point       GetDrawPos( SCCOL nCol, SCROW nRow, BOOL bArrow );
    BOOL        ShowSucc( SCCOL nCol, SCROW nRow );
    BOOL        ShowPred( SCCOL nCol, SCROW nRow );
    BOOL        ShowError( SCCOL nCol, SCROW nRow );

    BOOL        DeleteSucc( SCCOL nCol, SCROW nRow );
    BOOL        DeletePred( SCCOL nCol, SCROW nRow );
    BOOL        DeleteAll( ScDetectiveDelete eWhat );

    BOOL        MarkInvalid(BOOL& rOverflow);

    SdrObject*  ShowComment( SCCOL nCol, SCROW nRow, BOOL bForce, SdrPage* pDestPage = NULL );
    SdrObject*  ShowCommentUser( SCCOL nCol, SCROW nRow, const String& rUserText,
                                    const Rectangle& rVisible, BOOL bLeft,
                                    BOOL bForce, SdrPage* pDestPage );
    BOOL        HideComment( SCCOL nCol, SCROW nRow );

    void        UpdateAllComments();        // on all tables
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
