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
#if 1

#include "undobase.hxx"
#include "postit.hxx"

#include <boost/shared_ptr.hpp>

class ScDocShell;
class ScBaseCell;
class ScPatternAttr;
class EditTextObject;
class SdrUndoAction;
class ScDetOpList;
class ScDetOpData;
class ScRangeName;

//----------------------------------------------------------------------------

class ScUndoCursorAttr: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoCursorAttr( ScDocShell* pNewDocShell,
                            SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
                            const ScPatternAttr* pOldPat, const ScPatternAttr* pNewPat,
                            const ScPatternAttr* pApplyPat, sal_Bool bAutomatic );
    virtual         ~ScUndoCursorAttr();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual rtl::OUString GetComment() const;

    /** once the objects are passed to this class, their life-cycle is
        managed by this class; the calling function must pass new'ed
        objects to this method. */
    void            SetEditData( EditTextObject* pOld, EditTextObject* pNew );

private:
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    ScPatternAttr*  pOldPattern;
    ScPatternAttr*  pNewPattern;
    ScPatternAttr*  pApplyPattern;
    ::boost::shared_ptr<EditTextObject> pOldEditData;
    ::boost::shared_ptr<EditTextObject> pNewEditData;
    sal_Bool            bIsAutomatic;

    void            DoChange( const ScPatternAttr* pWhichPattern, const ::boost::shared_ptr<EditTextObject>& pEditData ) const;
};


class ScUndoEnterData: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoEnterData( ScDocShell* pNewDocShell,
                            SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
                            SCTAB nNewCount, SCTAB* pNewTabs,
                            ScBaseCell** ppOldData, sal_Bool* pHasForm, sal_uLong* pOldForm,
                            const String& rNewStr, EditTextObject* pObj = NULL );
    virtual         ~ScUndoEnterData();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual rtl::OUString GetComment() const;

private:
    String          aNewString;
    SCTAB*          pTabs;
    ScBaseCell**    ppOldCells;
    sal_Bool*           pHasFormat;
    sal_uLong*          pOldFormats;
    EditTextObject* pNewEditData;
    sal_uLong           nEndChangeAction;
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    SCTAB           nCount;             //  Marked sheet

    void            DoChange() const;
    void            SetChangeTrack();
};


class ScUndoEnterValue: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoEnterValue( ScDocShell* pNewDocShell,
                            const ScAddress& rNewPos,
                            ScBaseCell* pUndoCell, double nVal );
    virtual         ~ScUndoEnterValue();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual rtl::OUString GetComment() const;

private:
    ScAddress       aPos;
    ScBaseCell*     pOldCell;
    double          nValue;
    sal_uLong           nEndChangeAction;

    void            SetChangeTrack();
};


class ScUndoPutCell: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoPutCell( ScDocShell* pNewDocShell,
                            const ScAddress& rNewPos,
                            ScBaseCell* pUndoCell, ScBaseCell* pRedoCell );
    virtual         ~ScUndoPutCell();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual rtl::OUString GetComment() const;

private:
    ScAddress       aPos;
    ScBaseCell*     pOldCell;
    ScBaseCell*     pEnteredCell;
    sal_uLong           nEndChangeAction;

    void            SetChangeTrack();
};


class ScUndoPageBreak: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoPageBreak( ScDocShell* pNewDocShell,
                            SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
                            sal_Bool bNewColumn, sal_Bool bNewInsert );
    virtual         ~ScUndoPageBreak();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual rtl::OUString GetComment() const;

private:
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    sal_Bool            bColumn;        // Column or row break
    sal_Bool            bInsert;        // Insert or Delete

    void            DoChange( sal_Bool bInsert ) const;
};

class ScUndoPrintZoom: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoPrintZoom( ScDocShell* pNewDocShell, SCTAB nT,
                                    sal_uInt16 nOS, sal_uInt16 nOP, sal_uInt16 nNS, sal_uInt16 nNP );
    virtual         ~ScUndoPrintZoom();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual rtl::OUString GetComment() const;

private:
    SCTAB           nTab;
    sal_uInt16          nOldScale;
    sal_uInt16          nOldPages;
    sal_uInt16          nNewScale;
    sal_uInt16          nNewPages;

    void            DoChange( sal_Bool bUndo );
};

class ScUndoThesaurus: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoThesaurus( ScDocShell* pNewDocShell,
                            SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
                            const String& rNewUndoStr, const EditTextObject* pUndoTObj,
                            const String& rNewRedoStr, const EditTextObject* pRedoTObj);
    virtual         ~ScUndoThesaurus();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual rtl::OUString GetComment() const;

private:
    SCCOL           nCol;
    SCROW           nRow;
    SCTAB           nTab;
    String          aUndoStr;           // Data at String cell
    EditTextObject* pUndoTObject;       //      at Edit cell
    String          aRedoStr;
    EditTextObject* pRedoTObject;
    sal_uLong           nEndChangeAction;

    void            DoChange( sal_Bool bUndo, const String& rStr,
                                const EditTextObject* pTObj );
    void            SetChangeTrack( ScBaseCell* pOldCell );
};

// ============================================================================

/** Undo action for inserting, removing, and replacing a cell note. */
class ScUndoReplaceNote : public ScSimpleUndo
{
public:
                    TYPEINFO();

    /** Constructs an undo action for inserting or removing a cell note. */
                    ScUndoReplaceNote(
                        ScDocShell& rDocShell,
                        const ScAddress& rPos,
                        const ScNoteData& rNoteData,
                        bool bInsert,
                        SdrUndoAction* pDrawUndo );

    /** Constructs an undo action for replacing a cell note with another. */
                    ScUndoReplaceNote(
                        ScDocShell& rDocShell,
                        const ScAddress& rPos,
                        const ScNoteData& rOldData,
                        const ScNoteData& rNewData,
                        SdrUndoAction* pDrawUndo );

    virtual         ~ScUndoReplaceNote();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat( SfxRepeatTarget& rTarget );
    virtual sal_Bool    CanRepeat( SfxRepeatTarget& rTarget ) const;

    virtual rtl::OUString GetComment() const;

private:
    void            DoInsertNote( const ScNoteData& rNoteData );
    void            DoRemoveNote( const ScNoteData& rNoteData );

private:
    ScAddress       maPos;
    ScNoteData      maOldData;
    ScNoteData      maNewData;
    SdrUndoAction*  mpDrawUndo;
};

// ============================================================================

/** Undo action for showing or hiding a cell note caption. */
class ScUndoShowHideNote : public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoShowHideNote( ScDocShell& rDocShell, const ScAddress& rPos, bool bShow );
    virtual         ~ScUndoShowHideNote();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat( SfxRepeatTarget& rTarget );
    virtual sal_Bool    CanRepeat( SfxRepeatTarget& rTarget ) const;

    virtual rtl::OUString GetComment() const;

private:
    ScAddress       maPos;
    bool            mbShown;
};

// ============================================================================

class ScUndoDetective: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoDetective( ScDocShell* pNewDocShell,
                                    SdrUndoAction* pDraw, const ScDetOpData* pOperation,
                                    ScDetOpList* pUndoList = NULL );
    virtual         ~ScUndoDetective();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual rtl::OUString GetComment() const;

private:
    sal_Bool            bIsDelete;
    ScDetOpList*    pOldList;
    sal_uInt16          nAction;
    ScAddress       aPos;
    SdrUndoAction*  pDrawUndo;
};


class ScUndoRangeNames: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    //use nTab = -1 for global range names
                    ScUndoRangeNames( ScDocShell* pNewDocShell,
                                        ScRangeName* pOld, ScRangeName* pNew , SCTAB nTab = -1);
    virtual         ~ScUndoRangeNames();

    virtual void    Undo();
    virtual void    Redo();
    virtual void    Repeat(SfxRepeatTarget& rTarget);
    virtual sal_Bool    CanRepeat(SfxRepeatTarget& rTarget) const;

    virtual rtl::OUString GetComment() const;

private:
    ScRangeName*    pOldRanges;
    ScRangeName*    pNewRanges;
    SCTAB           mnTab;

    void            DoChange( sal_Bool bUndo );
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
