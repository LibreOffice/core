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
#ifndef INCLUDED_SC_SOURCE_UI_INC_SPELLENG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SPELLENG_HXX

#include "editutil.hxx"
#include "selectionstate.hxx"
#include "spellparam.hxx"

class ScViewData;
class ScDocShell;
class ScDocument;
class SfxItemPool;

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
    virtual void        ConvertAll( EditView& rEditView ) SAL_OVERRIDE;

protected:
    /** Callback from edit engine to check the next cell. */
    virtual bool        SpellNextDocument() SAL_OVERRIDE;

    /** Returns true, if the current text contains a spelling error. */
    virtual bool        NeedsConversion() SAL_OVERRIDE;

    /** Show a query box that asks whether to restart at top of the sheet.
        @return  true = Restart at top, false = Stop the conversion. */
    virtual bool        ShowTableWrapDialog() SAL_OVERRIDE;
    /** Show a message box stating that spell checking is finished. */
    virtual void        ShowFinishDialog() SAL_OVERRIDE;

private:
    /** Returns the spelling dialog if it is open. */
    vcl::Window*             GetDialogParent();
};

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
    virtual void        ConvertAll( EditView& rEditView ) SAL_OVERRIDE;

protected:
    /** Callback from edit engine to convert the next cell. */
    virtual bool        ConvertNextDocument() SAL_OVERRIDE;

    /** Returns true, if the current text contains text to convert. */
    virtual bool        NeedsConversion() SAL_OVERRIDE;

private:
    ScConversionParam   maConvParam;        /// Conversion parameters.
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
