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

#ifndef SC_DRWLAYER_HXX
#define SC_DRWLAYER_HXX

#include <vcl/graph.hxx>
#include <svx/fmmodel.hxx>
#include <svx/svdundo.hxx>
#include "global.hxx"

class ScDocument;
class SfxObjectShell;
class ScDrawObjData;
class ScIMapInfo;
class ScMacroInfo;
class IMapObject;
class ScMarkData;
class ScRange;
class ScAddress;

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

//  Adjusting of detective UserData and draw undo's both have to be in SdrUndoGroup;
//  therefore derived from SdrUndoAction

class ScUndoObjData : public SdrUndoObj
{
private:
    ScAddress   aOldStt;
    ScAddress   aOldEnd;
    ScAddress   aNewStt;
    ScAddress   aNewEnd;
public:
                ScUndoObjData( SdrObject* pObj, const ScAddress& rOS, const ScAddress& rOE,
                                                const ScAddress& rNS, const ScAddress& rNE );
                ~ScUndoObjData();

    virtual void     Undo();
    virtual void     Redo();
};

class SC_DLLPUBLIC ScDrawLayer : public FmFormModel
{
private:
    String          aName;
    ScDocument*     pDoc;
    SdrUndoGroup*   pUndoGroup;
    sal_Bool            bRecording;
    sal_Bool            bAdjustEnabled;
    sal_Bool            bHyphenatorSet;

private:
    void            MoveCells( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
                                SCsCOL nDx,SCsROW nDy, bool bUpdateNoteCaptionPos );

    void            RecalcPos( SdrObject* pObj, ScDrawObjData& rData, bool bNegativePage, bool bUpdateNoteCaptionPos );
    void            ResizeLastRectFromAnchor( SdrObject* pObj, ScDrawObjData& rData, bool bUseLogicRect, bool bNegativePage, bool bCanResize, bool bHiddenAsZero = true );

public:
                    ScDrawLayer( ScDocument* pDocument, const String& rName );
    virtual         ~ScDrawLayer();

    virtual SdrPage*  AllocPage(bool bMasterPage);
    virtual SdrModel* AllocModel() const;
    virtual void    SetChanged( sal_Bool bFlg = sal_True );

    virtual Window* GetCurDocViewWin();

    virtual SdrLayerID GetControlExportLayerId( const SdrObject & ) const;

    sal_Bool            HasObjects() const;

    sal_Bool            ScAddPage( SCTAB nTab );
    void            ScRemovePage( SCTAB nTab );
    void            ScRenamePage( SCTAB nTab, const String& rNewName );
    void            ScMovePage( sal_uInt16 nOldPos, sal_uInt16 nNewPos );
                    // incl. content, bAlloc=FALSE -> only content
    void            ScCopyPage( sal_uInt16 nOldPos, sal_uInt16 nNewPos, sal_Bool bAlloc );
    void            ResetTab( SCTAB nStart, SCTAB nEnd );

    ScDocument*     GetDocument() const { return pDoc; }

    void            UseHyphenator();

    sal_Bool            GetPrintArea( ScRange& rRange, sal_Bool bSetHor, sal_Bool bSetVer ) const;

                    //      automatic adjustments

    void            EnableAdjust( sal_Bool bSet = sal_True )    { bAdjustEnabled = bSet; }

    void            BeginCalcUndo();
    SdrUndoGroup*   GetCalcUndo();
    sal_Bool            IsRecording() const         { return bRecording; }
    void            AddCalcUndo( SdrUndoAction* pUndo );

    void            MoveArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
                                SCsCOL nDx,SCsROW nDy, sal_Bool bInsDel, bool bUpdateNoteCaptionPos = true );

    sal_Bool            HasObjectsInRows( SCTAB nTab, SCROW nStartRow, SCROW nEndRow );

    void            DeleteObjectsInArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1,
                                            SCCOL nCol2,SCROW nRow2 );
    void            DeleteObjectsInSelection( const ScMarkData& rMark );

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

    static bool IsCellAnchored( const SdrObject& rObj );
    static void             SetPageAnchored( SdrObject& );
    static void             SetCellAnchored( SdrObject&, const ScDrawObjData &rAnchor );
    static void             SetVisualCellAnchored( SdrObject&, const ScDrawObjData &rAnchor );
    // Updates rAnchor based on position of rObj
    static void             GetCellAnchorFromPosition( SdrObject &rObj, ScDrawObjData &rAnchor, const ScDocument &rDoc, SCTAB nTab, bool bUseLogicRect = true, bool bHiddenAsZero = true );
    static void             SetCellAnchoredFromPosition( SdrObject &rObj, const ScDocument &rDoc, SCTAB nTab );
    static void             UpdateCellAnchorFromPositionEnd( SdrObject &rObj, ScDrawObjData &rAnchor, const ScDocument &rDoc, SCTAB nTab, bool bUseLogicRect = true );
    static ScAnchorType     GetAnchorType( const SdrObject& );

    // positions for detektive lines
    static ScDrawObjData* GetObjData( SdrObject* pObj, sal_Bool bCreate=false );
    static ScDrawObjData* GetNonRotatedObjData( SdrObject* pObj, sal_Bool bCreate=false );

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

    static ScMacroInfo* GetMacroInfo( SdrObject* pObj, sal_Bool bCreate = false );

private:
    static SfxObjectShell* pGlobalDrawPersist;          // for AllocModel
public:
    static void     SetGlobalDrawPersist(SfxObjectShell* pPersist);
protected:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoModel();
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
