/*************************************************************************
 *
 *  $RCSfile: detfunc.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-09 15:06:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_DETFUNC_HXX
#define SC_DETFUNC_HXX

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

class SdrObject;
class SdrPage;
class String;

class ScCommentData;
class ScDetectiveData;
class ScDocument;
class ScTripel;
class ScAddress;
class ScRange;

#define SC_DET_MAXCIRCLE    1000

enum ScDetectiveDelete { SC_DET_ALL, SC_DET_DETECTIVE, SC_DET_CIRCLES, SC_DET_COMMENTS };

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
    ScDocument*     pDoc;
    USHORT          nTab;

    Point       GetDrawPos( USHORT nCol, USHORT nRow, BOOL bArrow );
    BOOL        HasArrow( USHORT nStartCol, USHORT nStartRow, USHORT nStartTab,
                            USHORT nEndCol, USHORT nEndRow, USHORT nEndTab );

    void        DeleteArrowsAt( USHORT nCol, USHORT nRow, BOOL bDestPnt );
    void        DeleteBox( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2 );

    BOOL        HasError( const ScTripel& rStart, const ScTripel& rEnd, ScTripel& rErrPos );

    void        FillAttributes( ScDetectiveData& rData );

                // called from DrawEntry/DrawAlienEntry and InsertObject
    BOOL        InsertArrow( USHORT nCol, USHORT nRow,
                                USHORT nRefStartCol, USHORT nRefStartRow,
                                USHORT nRefEndCol, USHORT nRefEndRow,
                                BOOL bFromOtherTab, BOOL bRed,
                                ScDetectiveData& rData );
    BOOL        InsertToOtherTab( USHORT nStartCol, USHORT nStartRow,
                                USHORT nEndCol, USHORT nEndRow, BOOL bRed,
                                ScDetectiveData& rData );

                // DrawEntry / DrawAlienEntry check for existing arrows and errors
    BOOL        DrawEntry( USHORT nCol, USHORT nRow, const ScTripel& rRefStart, const ScTripel& rRefEnd,
                                ScDetectiveData& rData );
    BOOL        DrawAlienEntry( const ScTripel& rRefStart, const ScTripel& rRefEnd,
                                ScDetectiveData& rData );

    void        DrawCircle( USHORT nCol, USHORT nRow, ScDetectiveData& rData );

    SdrObject*  DrawCaption( USHORT nCol, USHORT nRow, const String& rText,
                                ScCommentData& rData, SdrPage* pDestPage,
                                BOOL bHasUserText, BOOL bLeft,
                                const Rectangle& rVisible );

    USHORT      InsertPredLevel( USHORT nCol, USHORT nRow, ScDetectiveData& rData, USHORT nLevel );
    USHORT      InsertPredLevelArea( const ScTripel& rRefStart, const ScTripel& rRefEnd,
                                        ScDetectiveData& rData, USHORT nLevel );
    USHORT      FindPredLevel( USHORT nCol, USHORT nRow, USHORT nLevel, USHORT nDeleteLevel );
    USHORT      FindPredLevelArea( const ScTripel& rRefStart, const ScTripel& rRefEnd,
                                    USHORT nLevel, USHORT nDeleteLevel );

    USHORT      InsertErrorLevel( USHORT nCol, USHORT nRow, ScDetectiveData& rData, USHORT nLevel );

    USHORT      InsertSuccLevel( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                        ScDetectiveData& rData, USHORT nLevel );
    USHORT      FindSuccLevel( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                USHORT nLevel, USHORT nDeleteLevel );

    BOOL        FindFrameForObject( SdrObject* pObject, ScRange& rRange );


public:
                ScDetectiveFunc(ScDocument* pDocument, USHORT nTable) : pDoc(pDocument),nTab(nTable) {}

    BOOL        ShowSucc( USHORT nCol, USHORT nRow );
    BOOL        ShowPred( USHORT nCol, USHORT nRow );
    BOOL        ShowError( USHORT nCol, USHORT nRow );

    BOOL        DeleteSucc( USHORT nCol, USHORT nRow );
    BOOL        DeletePred( USHORT nCol, USHORT nRow );
    BOOL        DeleteAll( ScDetectiveDelete eWhat );

    BOOL        MarkInvalid(BOOL& rOverflow);

    SdrObject*  ShowComment( USHORT nCol, USHORT nRow, BOOL bForce, SdrPage* pDestPage = NULL );
    SdrObject*  ShowCommentUser( USHORT nCol, USHORT nRow, const String& rUserText,
                                    const Rectangle& rVisible, BOOL bLeft,
                                    BOOL bForce, SdrPage* pDestPage );
    BOOL        HideComment( USHORT nCol, USHORT nRow );

    void        UpdateAllComments();        // on all tables

    static BOOL IsNonAlienArrow( SdrObject* pObject );

    ScDetectiveObjType GetDetectiveObjectType( SdrObject* pObject,
                                ScAddress& rPosition, ScRange& rSource, BOOL& rRedLine );
    void        InsertObject( ScDetectiveObjType eType, const ScAddress& rPosition,
                                const ScRange& rSource, BOOL bRedLine );
};



#endif
