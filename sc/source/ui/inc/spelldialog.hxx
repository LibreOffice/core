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

#ifndef SC_SPELLDIALOG_HXX
#define SC_SPELLDIALOG_HXX

#include <memory>
#include <svx/SpellDialogChildWindow.hxx>

// ============================================================================

class ScConversionEngineBase;
class ScSelectionState;
class ScTabViewShell;
class ScViewData;
class ScDocShell;
class ScDocument;

/** Specialized spell check dialog child window for Calc.

    This derivation of the ::svx::SpellDialogChildWindow base class provides
    Calc specific implementations of the virtual functions GetNextWrongSentence()
    and ApplyChangedSentence().
 */
class ScSpellDialogChildWindow : public ::svx::SpellDialogChildWindow
{
public:
    SFX_DECL_CHILDWINDOW( ScSpellDialogChildWindow );

    explicit            ScSpellDialogChildWindow( Window* pParent, sal_uInt16 nId,
                            SfxBindings* pBindings, SfxChildWinInfo* pInfo );
    virtual             ~ScSpellDialogChildWindow();

    /** This method makes the one from the base class public so that
        it can be called from the view shell when one is created.
    */
    virtual void        InvalidateSpellDialog();

protected:
    /** Iterate over the sentences in all text shapes and stop at the
        next sentence with spelling errors. While doing so the view
        mode may be changed and text shapes are set into edit mode.
    */
    virtual ::svx::SpellPortions GetNextWrongSentence( bool bRecheck );

    /** This method is responsible for merging corrections made in the
        spelling dialog back into the document.
    */
    virtual void        ApplyChangedSentence( const ::svx::SpellPortions& rChanged, bool bRecheck );
    virtual void        GetFocus();
    virtual void        LoseFocus();

private:
    void                Reset();
    void                Init();

    bool                IsSelectionChanged();

private:
    typedef ::std::auto_ptr< ScConversionEngineBase >   ScConvEnginePtr;
    typedef ::std::auto_ptr< ScDocument >               ScDocumentPtr;
    typedef ::std::auto_ptr< ScSelectionState >         ScSelectionStatePtr;

    ScConvEnginePtr     mxEngine;
    ScDocumentPtr       mxUndoDoc;
    ScDocumentPtr       mxRedoDoc;
    ScSelectionStatePtr mxOldSel;           /// Old selection state for comparison.
    ScTabViewShell*     mpViewShell;
    ScViewData*         mpViewData;
    ScDocShell*         mpDocShell;
    ScDocument*         mpDoc;
    bool                mbNeedNextObj;
    bool                mbOldIdleDisabled;
};

// ============================================================================

#endif

