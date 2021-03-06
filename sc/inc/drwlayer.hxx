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

#pragma once

#include <svx/fmmodel.hxx>
#include <svx/svdundo.hxx>
#include "global.hxx"

class ScDocument;
class SfxObjectShell;
class ScDrawObjData;
class ScMacroInfo;
class ScMarkData;

class ScTabDeletedHint final : public SfxHint
{
private:
    SCTAB   nTab;
public:
            ScTabDeletedHint( SCTAB nTabNo );
    virtual ~ScTabDeletedHint() override;

    SCTAB   GetTab() const { return nTab; }
};

class ScTabSizeChangedHint final : public SfxHint
{
private:
    SCTAB   nTab;
public:
            ScTabSizeChangedHint( SCTAB nTabNo );
    virtual ~ScTabSizeChangedHint() override;

    SCTAB   GetTab() const  { return nTab; }
};

//  Adjusting of detective UserData and draw undo's both have to be in SdrUndoGroup;
//  therefore derived from SdrUndoAction

class ScUndoObjData final : public SdrUndoObj
{
private:
    ScAddress   aOldStt;
    ScAddress   aOldEnd;
    ScAddress   aNewStt;
    ScAddress   aNewEnd;
public:
                ScUndoObjData( SdrObject* pObj, const ScAddress& rOS, const ScAddress& rOE,
                                                const ScAddress& rNS, const ScAddress& rNE );
                virtual ~ScUndoObjData() override;

    virtual void     Undo() override;
    virtual void     Redo() override;
};

class ScUndoAnchorData final : public SdrUndoObj
{
private:
    bool                    mbWasCellAnchored;
    bool                    mbWasResizeWithCell;
    ScDocument*             mpDoc;
    SCTAB                   mnTab;
public:
                ScUndoAnchorData( SdrObject* pObj, ScDocument* pDoc, SCTAB nTab );
                virtual ~ScUndoAnchorData() override;

    virtual void     Undo() override;
    virtual void     Redo() override;
};

// for ScDrawLayer::SetPageSize
enum class ScObjectHandling
{
    RecalcPosMode, // used for row height or col width changes
    MoveRTLMode, // used for switch to RTL during import of right-to-left sheet
    MirrorRTLMode // used for switch between RTL and LTR by .uno:SheetRightToLeft
};

class SC_DLLPUBLIC ScDrawLayer final : public FmFormModel
{
private:
    OUString        aName;
    ScDocument*     pDoc;
    std::unique_ptr<SdrUndoGroup> pUndoGroup;
    bool            bRecording;
    bool            bAdjustEnabled;
    bool            bHyphenatorSet;

private:
    void            MoveCells( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
                                SCCOL nDx,SCROW nDy, bool bUpdateNoteCaptionPos );

    void            ResizeLastRectFromAnchor( const SdrObject* pObj, ScDrawObjData& rData, bool bNegativePage, bool bCanResize );

public:
                    ScDrawLayer( ScDocument* pDocument, const OUString& rName );
    virtual         ~ScDrawLayer() override;

    virtual rtl::Reference<SdrPage> AllocPage(bool bMasterPage) override;
    virtual SdrModel* AllocModel() const override;
    virtual void    SetChanged( bool bFlg = true ) override;

    bool            HasObjects() const;

    bool            ScAddPage( SCTAB nTab );
    void            ScRemovePage( SCTAB nTab );
    void            ScRenamePage( SCTAB nTab, const OUString& rNewName );
    void            ScMovePage( sal_uInt16 nOldPos, sal_uInt16 nNewPos );
    void            ScCopyPage( sal_uInt16 nOldPos, sal_uInt16 nNewPos );
    void            ResetTab( SCTAB nStart, SCTAB nEnd );

    ScDocument*     GetDocument() const { return pDoc; }

    void            UseHyphenator();

    bool            GetPrintArea( ScRange& rRange, bool bSetHor, bool bSetVer ) const;

                    //      automatic adjustments

    void            EnableAdjust( bool bSet )    { bAdjustEnabled = bSet; }

    void            BeginCalcUndo(bool bDisableTextEditUsesCommonUndoManager);
    std::unique_ptr<SdrUndoGroup> GetCalcUndo();
    bool            IsRecording() const         { return bRecording; }
    void            AddCalcUndo( std::unique_ptr<SdrUndoAction> pUndo );

    void            MoveArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1, SCCOL nCol2,SCROW nRow2,
                                SCCOL nDx,SCROW nDy, bool bInsDel, bool bUpdateNoteCaptionPos );
    void InitializeCellAnchoredObj(SdrObject* pObj, ScDrawObjData& rData);
    void            RecalcPos( SdrObject* pObj, ScDrawObjData& rData, bool bNegativePage, bool bUpdateNoteCaptionPos );

    bool            HasObjectsInRows( SCTAB nTab, SCROW nStartRow, SCROW nEndRow );

    void            DeleteObjectsInArea( SCTAB nTab, SCCOL nCol1,SCROW nRow1,
                                            SCCOL nCol2,SCROW nRow2, bool bAnchored = false );
    void            DeleteObjectsInSelection( const ScMarkData& rMark );

    void            CopyToClip( ScDocument* pClipDoc, SCTAB nTab, const tools::Rectangle& rRange );
    void            CopyFromClip( ScDrawLayer* pClipModel,
                                    SCTAB nSourceTab, const tools::Rectangle& rSourceRange,
                                    const ScAddress& rDestPos, const tools::Rectangle& rDestRange );

    void            SetPageSize(sal_uInt16 nPageNo, const Size& rSize, bool bUpdateNoteCaptionPos,
                                const ScObjectHandling eObjectHandling = ScObjectHandling::RecalcPosMode);

                    //  mirror or move between positive and negative positions for RTL
    void            MirrorRTL( SdrObject* pObj );
    void            MoveRTL(SdrObject* pObj);
    static void     MirrorRectRTL( tools::Rectangle& rRect );      // for bounding rectangles etc.

    /** Returns the rectangle for the passed cell address in 1/100 mm.
        @param bMergedCell  True = regards merged cells. False = use single column/row size. */
    static tools::Rectangle GetCellRect( const ScDocument& rDoc, const ScAddress& rPos, bool bMergedCell );

                    //  GetVisibleName: name for navigator etc: GetPersistName or GetName
                    //  (ChartListenerCollection etc. must use GetPersistName directly)
    static OUString GetVisibleName( const SdrObject* pObj );

    SdrObject*      GetNamedObject( std::u16string_view rName, sal_uInt16 nId, SCTAB& rFoundTab ) const;
                    // if pnCounter != NULL, the search for a name starts with this index + 1,
                    // and the index really used is returned.
    OUString        GetNewGraphicName( tools::Long* pnCounter = nullptr ) const;
    void            EnsureGraphicNames();

    static bool IsCellAnchored( const SdrObject& rObj );
    static bool IsResizeWithCell( const SdrObject& rObj );
    static void             SetPageAnchored( SdrObject& );
    static void             SetCellAnchored( SdrObject&, const ScDrawObjData &rAnchor );
    static void             SetNonRotatedAnchor( SdrObject&, const ScDrawObjData &rAnchor );

    // Updates rAnchor based on position of rObj
    static void GetCellAnchorFromPosition(
        const tools::Rectangle &rRectangle,
        ScDrawObjData &rAnchor,
        const ScDocument &rDoc,
        SCTAB nTab,
        bool bHiddenAsZero = true);

    static void             SetCellAnchoredFromPosition( SdrObject &rObj, const ScDocument &rDoc, SCTAB nTab, bool bResizeWithCell );
    static void             UpdateCellAnchorFromPositionEnd( const SdrObject &rObj, ScDrawObjData &rAnchor, const ScDocument &rDoc, SCTAB nTab, bool bUseLogicRect = true );
    static ScAnchorType     GetAnchorType( const SdrObject& );
    std::vector<SdrObject*> GetObjectsAnchoredToRows(SCTAB nTab, SCROW nStartRow, SCROW nEndRow);
    std::map<SCROW, std::vector<SdrObject*>> GetObjectsAnchoredToRange(SCTAB nTab, SCCOL nCol, SCROW nStartRow, SCROW nEndRow);
    bool HasObjectsAnchoredInRange(const ScRange& rRange);
    std::vector<SdrObject*> GetObjectsAnchoredToCols(SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol);
    void MoveObject(SdrObject* pObj, const ScAddress& rNewPosition);

    // positions for detective lines
    static ScDrawObjData* GetObjData( SdrObject* pObj, bool bCreate=false );
    static ScDrawObjData* GetNonRotatedObjData( SdrObject* pObj, bool bCreate=false );

    // The sheet information in ScDrawObjData isn't updated when sheets are inserted/deleted.
    // Use this method to get an object with positions on the specified sheet (should be the
    // sheet on which the object is inserted).
    static ScDrawObjData* GetObjDataTab( SdrObject* pObj, SCTAB nTab );

    /** Returns true, if the passed object is the caption of a cell note. */
    static bool     IsNoteCaption( SdrObject* pObj );

    /** Returns the object data, if the passed object is a cell note caption. */
    static ScDrawObjData* GetNoteCaptionData( SdrObject* pObj, SCTAB nTab );

    static ScMacroInfo* GetMacroInfo( SdrObject* pObj, bool bCreate = false );

private:
    static SfxObjectShell* pGlobalDrawPersist;          // for AllocModel
public:
    static void     SetGlobalDrawPersist(SfxObjectShell* pPersist);
private:
    virtual css::uno::Reference< css::uno::XInterface > createUnoModel() override;
};

extern bool bDrawIsInUndo; // somewhere as member!

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
