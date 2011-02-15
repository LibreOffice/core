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

#ifndef SC_DRWLAYER_HXX
#define SC_DRWLAYER_HXX

#include <vcl/graph.hxx>
#include <svx/fmmodel.hxx>
#include <svx/svdundo.hxx>
#include "global.hxx"

class ScDocument;
class SfxViewShell;
class SfxObjectShell;
class ScDrawObjData;
class ScIMapInfo;
class ScMacroInfo;
class IMapObject;
class ScMarkData;
class SdrOle2Obj;
class ScRange;
class ScAddress;

// -----------------------------------------------------------------------

class ScTabDeletedHint : public SfxHint
{
private:
    SCTAB   nTab;
public:
            TYPEINFO();
            ScTabDeletedHint( SCTAB nTabNo = SCTAB_MAX );
    virtual ~ScTabDeletedHint();

    SCTAB   GetTab()    { return nTab; }
};

class ScTabSizeChangedHint : public SfxHint
{
private:
    SCTAB   nTab;
public:
            TYPEINFO();
            ScTabSizeChangedHint( SCTAB nTabNo = SCTAB_MAX );
    virtual ~ScTabSizeChangedHint();

    SCTAB   GetTab()    { return nTab; }
};

// -----------------------------------------------------------------------
//
//  Das Anpassen der Detektiv-UserData muss zusammen mit den Draw-Undo's
//  in der SdrUndoGroup liegen, darum von SdrUndoAction abgeleitet:

class ScUndoObjData : public SdrUndoObj
{
private:
    ScAddress   aOldStt;
    ScAddress   aOldEnd;
    ScAddress   aNewStt;
    ScAddress   aNewEnd;
    sal_Bool        bHasNew;
public:
                ScUndoObjData( SdrObject* pObj, const ScAddress& rOS, const ScAddress& rOE,
                                                const ScAddress& rNS, const ScAddress& rNE );
                ~ScUndoObjData();

    virtual void     Undo();
    virtual void     Redo();
};

// -----------------------------------------------------------------------

class SC_DLLPUBLIC ScDrawLayer : public FmFormModel
{
private:
//REMOVE        SotStorageRef   xPictureStorage;
    String          aName;
    ScDocument*     pDoc;
    SdrUndoGroup*   pUndoGroup;
    sal_Bool            bRecording;
    sal_Bool            bAdjustEnabled;
    sal_Bool            bHyphenatorSet;

private:
    void            MoveAreaTwips( SCTAB nTab, const Rectangle& rArea, const Point& rMove,
                                const Point& rTopLeft );
    void            MoveCells( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
                                SCsCOL nDx,SCsROW nDy, bool bUpdateNoteCaptionPos );

    void            RecalcPos( SdrObject* pObj, const ScDrawObjData& rData, bool bNegativePage, bool bUpdateNoteCaptionPos );

public:
                    ScDrawLayer( ScDocument* pDocument, const String& rName );
    virtual         ~ScDrawLayer();

    virtual SdrPage*  AllocPage(FASTBOOL bMasterPage);
    virtual SdrModel* AllocModel() const;
    virtual void    SetChanged( sal_Bool bFlg = sal_True );

    virtual Window* GetCurDocViewWin();
    virtual SvStream* GetDocumentStream(SdrDocumentStreamInfo& rStreamInfo) const;

    virtual SdrLayerID GetControlExportLayerId( const SdrObject & ) const;

//REMOVE        void            ReleasePictureStorage();

    sal_Bool            HasObjects() const;

    sal_Bool            ScAddPage( SCTAB nTab );
    void            ScRemovePage( SCTAB nTab );
    void            ScRenamePage( SCTAB nTab, const String& rNewName );
    void            ScMovePage( sal_uInt16 nOldPos, sal_uInt16 nNewPos );
                    // inkl. Inhalt, bAlloc=sal_False -> nur Inhalt
    void            ScCopyPage( sal_uInt16 nOldPos, sal_uInt16 nNewPos, sal_Bool bAlloc );

    ScDocument*     GetDocument() const { return pDoc; }

    void            UpdateBasic();              // DocShell-Basic in DrawPages setzen
    void            UseHyphenator();

    sal_Bool            GetPrintArea( ScRange& rRange, sal_Bool bSetHor, sal_Bool bSetVer ) const;

                    //      automatische Anpassungen

    void            EnableAdjust( sal_Bool bSet = sal_True )    { bAdjustEnabled = bSet; }

    void            BeginCalcUndo();
    SdrUndoGroup*   GetCalcUndo();
    sal_Bool            IsRecording() const         { return bRecording; }
    void            AddCalcUndo( SdrUndoAction* pUndo );

    void            MoveArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
                                SCsCOL nDx,SCsROW nDy, sal_Bool bInsDel, bool bUpdateNoteCaptionPos = true );
    void            WidthChanged( SCTAB nTab, SCCOL nCol, long nDifTwips );
    void            HeightChanged( SCTAB nTab, SCROW nRow, long nDifTwips );

        sal_Bool            HasObjectsInRows( SCTAB nTab, SCROW nStartRow, SCROW nEndRow, bool bIncludeNotes = true );

    void            DeleteObjectsInArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1,
                                            SCCOL nCol2,SCROW nRow2 );
    void            DeleteObjectsInSelection( const ScMarkData& rMark );
#if 0
    void            DeleteObjects( SCTAB nTab );
#endif

    void            CopyToClip( ScDocument* pClipDoc, SCTAB nTab, const Rectangle& rRange );
    void            CopyFromClip( ScDrawLayer* pClipModel,
                                    SCTAB nSourceTab, const Rectangle& rSourceRange,
                                    const ScAddress& rDestPos, const Rectangle& rDestRange );

    void            SetPageSize( sal_uInt16 nPageNo, const Size& rSize, bool bUpdateNoteCaptionPos = true );

                    //  mirror or move between positive and negative positions for RTL
    void            MirrorRTL( SdrObject* pObj );
    static void     MirrorRectRTL( Rectangle& rRect );      // for bounding rectangles etc.

    /** Returns the rectangle for the passed cell address in 1/100 mm.
        @param bMergedCell  True = regards merged cells. False = use single column/row size. */
    static Rectangle GetCellRect( ScDocument& rDoc, const ScAddress& rPos, bool bMergedCell );

                    //  GetVisibleName: name for navigator etc: GetPersistName or GetName
                    //  (ChartListenerCollection etc. must use GetPersistName directly)
    static String   GetVisibleName( SdrObject* pObj );

    SdrObject*      GetNamedObject( const String& rName, sal_uInt16 nId, SCTAB& rFoundTab ) const;
                    // if pnCounter != NULL, the search for a name starts with this index + 1,
                    // and the index really used is returned.
    String          GetNewGraphicName( long* pnCounter = NULL ) const;
    void            EnsureGraphicNames();

    // Verankerung setzen und ermitteln
    static void     SetAnchor( SdrObject*, ScAnchorType );
    static ScAnchorType GetAnchor( const SdrObject* );

    // Positionen fuer Detektivlinien
    static ScDrawObjData* GetObjData( SdrObject* pObj, sal_Bool bCreate=sal_False );

    // The sheet information in ScDrawObjData isn't updated when sheets are inserted/deleted.
    // Use this method to get an object with positions on the specified sheet (should be the
    // sheet on which the object is inserted).
    static ScDrawObjData* GetObjDataTab( SdrObject* pObj, SCTAB nTab );

    /** Returns true, if the passed object is the caption of a cell note. */
    static bool     IsNoteCaption( SdrObject* pObj );

    /** Returns the object data, if the passed object is a cell note caption. */
    static ScDrawObjData* GetNoteCaptionData( SdrObject* pObj, SCTAB nTab );

    // Image-Map
    static ScIMapInfo* GetIMapInfo( SdrObject* pObj );

    static IMapObject* GetHitIMapObject( SdrObject* pObject,
                            const Point& rWinPoint, const Window& rCmpWnd );

    static ScMacroInfo* GetMacroInfo( SdrObject* pObj, sal_Bool bCreate = sal_False );

private:
    static SfxObjectShell* pGlobalDrawPersist;          // fuer AllocModel
public:
    static void     SetGlobalDrawPersist(SfxObjectShell* pPersist);
protected:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoModel();
};


#endif


