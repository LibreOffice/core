/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: spelldialog.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:52:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_SPELLDIALOG_HXX
#define SC_SPELLDIALOG_HXX

#include <memory>

#ifndef SVX_SPELL_DIALOG_CHILD_WINDOW_HXX
#include <svx/SpellDialogChildWindow.hxx>
#endif

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

    explicit            ScSpellDialogChildWindow( Window* pParent, USHORT nId,
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
    virtual ::svx::SpellPortions GetNextWrongSentence();

    /** This method is responsible for merging corrections made in the
        spelling dialog back into the document.
    */
    virtual void        ApplyChangedSentence( const ::svx::SpellPortions& rChanged );
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

