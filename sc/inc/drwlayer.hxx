/*************************************************************************
 *
 *  $RCSfile: drwlayer.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:49 $
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

#ifndef SC_DRWLAYER_HXX
#define SC_DRWLAYER_HXX

#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif

#ifndef _FM_FMMODEL_HXX
#include <svx/fmmodel.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

class ScDocument;
class SfxViewShell;
class ScDrawObjData;
class ScIMapInfo;
class IMapObject;
class ScMarkData;
class SdrOle2Obj;

// -----------------------------------------------------------------------

class ScTabDeletedHint : public SfxHint
{
private:
    USHORT  nTab;
public:
            TYPEINFO();
            ScTabDeletedHint( USHORT nTabNo = USHRT_MAX );
    virtual ~ScTabDeletedHint();

    USHORT  GetTab()    { return nTab; }
};

class ScTabSizeChangedHint : public SfxHint
{
private:
    USHORT  nTab;
public:
            TYPEINFO();
            ScTabSizeChangedHint( USHORT nTabNo = USHRT_MAX );
    virtual ~ScTabSizeChangedHint();

    USHORT  GetTab()    { return nTab; }
};

// -----------------------------------------------------------------------


class ScDrawLayer: public FmFormModel
{
private:
    String          aName;
    ScDocument*     pDoc;
    SdrUndoGroup*   pUndoGroup;
    BOOL            bRecording;
    BOOL            bAdjustEnabled;

private:
    void            MoveAreaMM( USHORT nTab, const Rectangle& rArea, const Point& rMove,
                                const Point& rTopLeft );
    void            MoveCells( USHORT nTab, USHORT nCol1,USHORT nRow1, USHORT nCol2,USHORT nRow2,
                                short nDx,short nDy );
    void            RecalcPos( SdrObject* pObj, ScDrawObjData* pData );

public:
                    ScDrawLayer( ScDocument* pDocument, const String& rName );
    virtual         ~ScDrawLayer();

    virtual SdrPage*  AllocPage(FASTBOOL bMasterPage);
    virtual SdrModel* AllocModel() const;
    virtual void    SetChanged( FASTBOOL bFlg = TRUE );

    virtual Window* GetCurDocViewWin();
    virtual SvStream* GetDocumentStream(FASTBOOL& rbDeleteAfterUse) const;

    virtual SdrLayerID GetControlExportLayerId( const SdrObject & ) const;

    BOOL            HasObjects() const;

    void            ScAddPage( USHORT nTab );
    void            ScRemovePage( USHORT nTab );
    void            ScRenamePage( USHORT nTab, const String& rNewName );
    void            ScMovePage( USHORT nOldPos, USHORT nNewPos );
                    // inkl. Inhalt, bAlloc=FALSE -> nur Inhalt
    void            ScCopyPage( USHORT nOldPos, USHORT nNewPos, BOOL bAlloc );

    ScDocument*     GetDocument() const { return pDoc; }

    void            UpdateBasic();              // DocShell-Basic in DrawPages setzen

    void            Load( SvStream& rStream );
    void            Store( SvStream& rStream ) const;

    BOOL            GetPrintArea( ScRange& rRange, BOOL bSetHor, BOOL bSetVer ) const;

                    //      automatische Anpassungen

    void            EnableAdjust( BOOL bSet = TRUE )    { bAdjustEnabled = bSet; }

    void            BeginCalcUndo();
    SdrUndoGroup*   GetCalcUndo();
    BOOL            IsRecording()           { return bRecording; }
    void            AddCalcUndo( SdrUndoAction* pUndo );

    void            MoveArea( USHORT nTab, USHORT nCol1,USHORT nRow1, USHORT nCol2,USHORT nRow2,
                                short nDx,short nDy, BOOL bInsDel );
    void            WidthChanged( USHORT nTab, USHORT nCol, long nDifTwips );
    void            HeightChanged( USHORT nTab, USHORT nRow, long nDifTwips );

    BOOL            HasObjectsInRows( USHORT nTab, USHORT nStartRow, USHORT nEndRow );

    void            DeleteObjectsInArea( USHORT nTab, USHORT nCol1,USHORT nRow1,
                                            USHORT nCol2,USHORT nRow2 );
    void            DeleteObjectsInSelection( const ScMarkData& rMark );
    void            DeleteObjects( USHORT nTab );

    void            SetPageSize( USHORT nPageNo, const Size& rSize );
    ULONG           GetDefTextHeight() const;

    SdrObject*      GetNamedObject( const String& rName, USHORT nId, USHORT& rFoundTab ) const;
    String          GetNewGraphicName() const;
    void            EnsureGraphicNames();

    // Verankerung setzen und ermitteln
    static void     SetAnchor( SdrObject*, ScAnchorType );
    static ScAnchorType GetAnchor( const SdrObject* );

    // Positionen fuer Detektivlinien
    static ScDrawObjData* GetObjData( SdrObject* pObj, BOOL bCreate=FALSE );

    // Image-Map
    static ScIMapInfo* GetIMapInfo( SdrObject* pObj );

    static Graphic  GetGraphicFromOle2Obj( const SdrOle2Obj* pOle2Obj );
    static IMapObject* GetHitIMapObject( SdrObject* pObject,
                            const Point& rWinPoint, const Window& rCmpWnd );

private:
    static SvPersist* pGlobalDrawPersist;           // fuer AllocModel
public:
    static void     SetGlobalDrawPersist(SvPersist* pPersist);
};


#endif


