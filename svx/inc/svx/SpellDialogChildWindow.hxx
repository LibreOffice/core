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

#ifndef SVX_SPELL_DIALOG_CHILD_WINDOW_HXX
#define SVX_SPELL_DIALOG_CHILD_WINDOW_HXX

#include <sfx2/childwin.hxx>
#include <editeng/SpellPortions.hxx>
#include "svx/svxdllapi.h"
#include <vcl/image.hxx>

class AbstractSpellDialog;

namespace svx {

/** The child window wrapper of the actual spellin dialog.  To use the
    spelling dialog in an application you have to do the ususal things:
    <ol>
    <li>Call this class' RegisterChildWindow() method with the module
    as second argument that you want the dialog included in.</li>
    <li>In the SFX_IMPL_INTERFACE implementation of the view shell
    that wants to use the dialog call SFX_CHILDWINDOW_REGISTRATION()
    with the id returned by this class' GetChildWindowId()
    method.</li>
    <li>Include the item associated with this child window to the SDI
    description of the view shell.</li>
    </ol>
*/
class SVX_DLLPUBLIC SpellDialogChildWindow
    : public SfxChildWindow
{
    friend class SpellDialog;
    AbstractSpellDialog* m_pAbstractSpellDialog;
public:
    SpellDialogChildWindow (
        Window*pParent,
        sal_uInt16 nId,
        SfxBindings* pBindings,
        SfxChildWinInfo* pInfo);
    virtual ~SpellDialogChildWindow ();

protected:
    /** This abstract method has to be defined by a derived class.  It
        returns the next wrong sentence.
        @return
            returns an empty vector if no error could be found
    */
    virtual SpellPortions GetNextWrongSentence (bool bRecheck) = 0;

    /** This abstract method applies the changes made in the spelling dialog
     to the document.
     The dialog always updates its settings when it gets the focus. The document
     can rely on the fact that the methods ApplyChangedSentence() is called for the
     position that the last GetNextWrongSentence() returned.
     If 'bRecheck' is set to true then the same sentence should be rechecked once from
     the start. This should be used too find errors that the user has introduced by
     manual changes in the edit field, and in order to not miss the still following errors
     in that sentence.
    */
    virtual void ApplyChangedSentence(const SpellPortions& rChanged, bool bRecheck ) = 0;
    /** This methods determines whether the application supports AutoCorrection
     */
    virtual bool HasAutoCorrection();
    /** This method adds a word pair to the AutoCorrection - if available
     */
    virtual void AddAutoCorrection(const String& rOld, const String& rNew, LanguageType eLanguage);
    /** Return the sfx bindings for this child window. They are
        retrieved from the dialog so they do not have to be stored in
        this class as well.  The bindings may be necessary to be used
        by the abstract methods.
    */
    /** This method determines if grammar checking is supported
     */
    virtual bool HasGrammarChecking();
    /** determines if grammar checking is switched on
     */
    virtual bool IsGrammarChecking();
    /** switches grammar checking on/off
     */
    virtual void SetGrammarChecking(bool bOn);

    SfxBindings& GetBindings (void) const;
    /** Set the spell dialog into the 'resume' state. This method should be called
        to notify the SpellDialog about changes in the document that invalidate the
        current state which results in disabling most of the dialog controls and presenting
        a "Resume" button that initiates a reinitialization.
     */
    void    InvalidateSpellDialog();
    /** Notifies the ChildWindow about the get focus event. The ChildWindow should no check if
        the spelling dialog should be set to the 'Resume' state by calling InvalidateSpellDialog()
     */
    virtual void    GetFocus() = 0;
    /** Notifies the ChildWindow about the lose focus event. The ChildWindow should use it to save
        the current selection/state.
     */
    virtual void    LoseFocus() = 0;
};

} // end of namespace ::svx

#endif
