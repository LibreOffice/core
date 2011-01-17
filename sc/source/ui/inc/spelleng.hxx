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
#ifndef SC_SPELLENG_HXX
#define SC_SPELLENG_HXX

#include "editutil.hxx"
#include "selectionstate.hxx"
#include "spellparam.hxx"

class ScViewData;
class ScDocShell;
class ScDocument;
class SfxItemPool;

// ============================================================================

/** Base class for special type of edit engines, i.e. for spell checker and text conversion. */
class ScConversionEngineBase : public ScEditEngineDefaulter
{
public:
    explicit            ScConversionEngineBase(
                            SfxItemPool* pEnginePool, ScViewData& rViewData,
                            ScDocument* pUndoDoc, ScDocument* pRedoDoc );

    virtual             ~ScConversionEngineBase();

    /** Derived classes implement to convert all cells in the selection or sheet. */
    virtual void        ConvertAll( EditView& rEditView ) = 0;

    /** Returns true, if at least one cell has been modified. */
    inline bool         IsAnyModified() const { return mbIsAnyModified; }
    /** Returns true, if the entire document/selection has been finished. */
    inline bool         IsFinished() const { return mbFinished; }

protected:
    /** Implementation of cell iteration. Finds a cell that needs conversion.
        @return  true = Current cell needs conversion (i.e. spelling error found). */
    bool                FindNextConversionCell();
    /** Restores the initial cursor position. */
    void                RestoreCursorPos();

    /** Derived classes return, if the current text needs conversion (i.e. spelling error found).
        @return  true = Current edit text needs conversion. */
    virtual bool        NeedsConversion() = 0;

    /** Derived classes may show a query box that asks whether to restart at top of the sheet.
        @descr  Default here is no dialog and restart always.
        @return  true = Restart at top, false = Stop the conversion. */
    virtual bool        ShowTableWrapDialog();
    /** Derived classes may show a message box stating that the conversion is finished.
        @descr  Default here is no dialog. */
    virtual void        ShowFinishDialog();

private:
    /** Fills the edit engine from a document cell. */
    void                FillFromCell( SCCOL nCol, SCROW nRow, SCTAB nTab );

protected:  // for usage in derived classes
    ScViewData&         mrViewData;
    ScDocShell&         mrDocShell;
    ScDocument&         mrDoc;

private:
    ScSelectionState    maSelState;         /// Selection data of the document.
    ScDocument*         mpUndoDoc;          /// Document stores all old cells for UNDO action.
    ScDocument*         mpRedoDoc;          /// Document stores all new cells for REDO action.
    LanguageType        meCurrLang;         /// Current cell language.
    SCCOL               mnStartCol;         /// Initial column index.
    SCROW               mnStartRow;         /// Initial row index.
    SCTAB               mnStartTab;         /// Initial sheet index.
    SCCOL               mnCurrCol;          /// Current column index.
    SCROW               mnCurrRow;          /// Current row index.
    bool                mbIsAnyModified;    /// true = At least one cell has been changed.
    bool                mbInitialState;     /// true = Not searched for a cell yet.
    bool                mbWrappedInTable;   /// true = Already restarted at top of the sheet.
    bool                mbFinished;         /// true = Entire document/selection finished.
};

// ============================================================================

/** Edit engine for spell checking. */
class ScSpellingEngine : public ScConversionEngineBase
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1 > XSpellCheckerRef;

    explicit            ScSpellingEngine(
                            SfxItemPool* pEnginePool,
                            ScViewData& rViewData,
                            ScDocument* pUndoDoc,
                            ScDocument* pRedoDoc,
                            XSpellCheckerRef xSpeller );

    /** Checks spelling of all cells in the selection or sheet. */
    virtual void        ConvertAll( EditView& rEditView );

protected:
    /** Callback from edit engine to check the next cell. */
    virtual sal_Bool        SpellNextDocument();

    /** Returns true, if the current text contains a spelling error. */
    virtual bool        NeedsConversion();

    /** Show a query box that asks whether to restart at top of the sheet.
        @return  true = Restart at top, false = Stop the conversion. */
    virtual bool        ShowTableWrapDialog();
    /** Show a message box stating that spell checking is finished. */
    virtual void        ShowFinishDialog();

private:
    /** Returns the spelling dialog if it is open. */
    Window*             GetDialogParent();
};

// ============================================================================

/** Edit engine for text conversion. */
class ScTextConversionEngine : public ScConversionEngineBase
{
public:
    explicit            ScTextConversionEngine(
                            SfxItemPool* pEnginePool,
                            ScViewData& rViewData,
                            const ScConversionParam& rConvParam,
                            ScDocument* pUndoDoc,
                            ScDocument* pRedoDoc );

    /** Converts all cells in the selection or sheet according to set language. */
    virtual void        ConvertAll( EditView& rEditView );

protected:
    /** Callback from edit engine to convert the next cell. */
    virtual sal_Bool        ConvertNextDocument();

    /** Returns true, if the current text contains text to convert. */
    virtual bool        NeedsConversion();

private:
    ScConversionParam   maConvParam;        /// Conversion parameters.
};

// ============================================================================

#endif

