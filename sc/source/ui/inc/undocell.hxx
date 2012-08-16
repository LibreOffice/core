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

#ifndef SC_UNDOCELL_HXX
#define SC_UNDOCELL_HXX

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
                            ScBaseCell* pUndoCell, double nVal, sal_Bool bHeight );
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
    sal_Bool            bNeedHeight;

    void            SetChangeTrack();
};


class ScUndoPutCell: public ScSimpleUndo
{
public:
                    TYPEINFO();
                    ScUndoPutCell( ScDocShell* pNewDocShell,
                            const ScAddress& rNewPos,
                            ScBaseCell* pUndoCell, ScBaseCell* pRedoCell, sal_Bool bHeight );
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
    sal_Bool            bNeedHeight;

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
