/*************************************************************************
 *
 *  $RCSfile: spelldialog.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-09-17 13:52:53 $
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

