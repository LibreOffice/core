/*************************************************************************
 *
 *  $RCSfile: spelleng.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 13:36:48 $
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

#ifndef SC_SPELLENG_HXX
#define SC_SPELLENG_HXX

#include "editutil.hxx"

class ScViewData;
class ScDocShell;
class ScDocument;
class SfxItemPool;

// ============================================================================

/** Base class for special type of edit engines, i.e. for spell checker and text conversion. */
class ScConversionEngineBase : public ScEditEngineDefaulter
{
public:
    explicit                ScConversionEngineBase(
                                SfxItemPool* pEnginePool,
                                ScViewData& rViewData,
                                ScDocument* pUndoDoc,
                                ScDocument* pRedoDoc,
                                ESelection* pEdSelection,
                                SCCOL nCol, SCROW nRow, SCTAB nTab,
                                bool bCellSelection );

    virtual                 ~ScConversionEngineBase();

    /** Derived classes implement to convert all cells in the selection or sheet. */
    virtual void            ConvertAll( EditView& rEditView ) = 0;

    /** Returns true, if at least one cell has been modified. */
    inline bool             IsAnyModified() const { return mbIsAnyModified; }

protected:
    /** Implementation of cell iteration. Finds a cell that needs conversion.
        @return  true = Current cell needs conversion (i.e. spelling error found). */
    bool                    FindNextConversionCell();

    /** Derived classes return, if the current text needs conversion (i.e. spelling error found).
        @return  true = Current edit text needs conversion. */
    virtual bool            NeedsConversion() = 0;

    /** Derived classes may show a query box that asks whether to restart at top of the sheet.
        @descr  Default here is no dialog and restart always.
        @return  true = Restart at top, false = Stop the conversion. */
    virtual bool            ShowTableWrapDialog();
    /** Derived classes may show a message box stating that the conversion is finished.
        @descr  Default here is no dialog. */
    virtual void            ShowFinishDialog();

private:
    /** Fills the edit engine from a document cell. */
    void                    FillFromCell( SCCOL nCol, SCROW nRow, SCTAB nTab );

protected:  // for usage in derived classes
    ScViewData&             mrViewData;
    ScDocShell&             mrDocShell;
    ScDocument&             mrDoc;

private:
    ScDocument*             mpUndoDoc;          /// Document stores all old cells for UNDO action.
    ScDocument*             mpRedoDoc;          /// Document stores all new cells for REDO action.
    ESelection*             mpEditSel;          /// Cell selection from edit mode or NULL.
    LanguageType            meCurrLang;         /// Current cell language.
    SCCOL                   mnStartCol;         /// Initial column index.
    SCROW                   mnStartRow;         /// Initial row index.
    SCTAB                   mnStartTab;         /// Initial sheet index.
    SCCOL                   mnCurrCol;          /// Current column index.
    SCROW                   mnCurrRow;          /// Current row index.
    bool                    mbCellSelect;       /// true = Cell selection; false = Entire sheet.
    bool                    mbIsAnyModified;    /// true = At least one cell has been changed.
    bool                    mbInitialState;     /// true = Not searched for a cell yet.
    bool                    mbWrappedInTable;   /// true = Already restarted at top of the sheet.
};

// ============================================================================

/** Edit engine for spell checking. */
class ScSpellingEngine : public ScConversionEngineBase
{
public:
    typedef ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XSpellChecker1 > XSpellCheckerRef;

    explicit                ScSpellingEngine(
                                SfxItemPool* pEnginePool,
                                ScViewData& rViewData,
                                ScDocument* pUndoDoc,
                                ScDocument* pRedoDoc,
                                ESelection* pEdSelection,
                                SCCOL nCol, SCROW nRow, SCTAB nTab,
                                bool bCellSelection,
                                XSpellCheckerRef xSpeller );

    /** Checks spelling of all cells in the selection or sheet. */
    virtual void            ConvertAll( EditView& rEditView );

protected:
    /** Callback from edit engine to check the next cell. */
    virtual BOOL            SpellNextDocument();

    /** Returns true, if the current text contains a spelling error. */
    virtual bool            NeedsConversion();

    /** Show a query box that asks whether to restart at top of the sheet.
        @return  true = Restart at top, false = Stop the conversion. */
    virtual bool            ShowTableWrapDialog();
    /** Show a message box stating that spell checking is finished. */
    virtual void            ShowFinishDialog();
};

// ============================================================================

/** Edit engine for text conversion. */
class ScTextConversionEngine : public ScConversionEngineBase
{
public:
    explicit                ScTextConversionEngine(
                                SfxItemPool* pEnginePool,
                                ScViewData& rViewData,
                                ScDocument* pUndoDoc,
                                ScDocument* pRedoDoc,
                                ESelection* pEdSelection,
                                SCCOL nCol, SCROW nRow, SCTAB nTab,
                                bool bCellSelection,
                                LanguageType eConvLanguage );

    /** Converts all cells in the selection or sheet according to set language. */
    virtual void            ConvertAll( EditView& rEditView );

protected:
    /** Callback from edit engine to convert the next cell. */
    virtual BOOL            ConvertNextDocument();

    /** Returns true, if the current text contains text to convert. */
    virtual bool            NeedsConversion();

private:
    LanguageType            meConvLang;     /// Language for text conversion.
};

// ============================================================================

#endif

