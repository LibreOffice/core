/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SpellDialogChildWindow.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:37:11 $
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

#ifndef SVX_SPELL_DIALOG_CHILD_WINDOW_HXX
#define SVX_SPELL_DIALOG_CHILD_WINDOW_HXX

#ifndef _SFX_CHILDWIN_HXX
#include <sfx2/childwin.hxx>
#endif
#ifndef SVX_SPELL_PORTIONS_HXX
#include <svx/SpellPortions.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
        USHORT nId,
        SfxBindings* pBindings,
        SfxChildWinInfo* pInfo);
    virtual ~SpellDialogChildWindow ();

protected:
    /** This abstract method has to be defined by a derived class.  It
        returns the next wrong sentence.
        @return
            returns an empty vector if no error could be found
    */
    virtual SpellPortions GetNextWrongSentence (void) = 0;

    /** This abstract method applies the changes made in the spelling dialog
     to the document.
     The dialog always updates its settings when it gets the focus. The document
     can rely on the fact that the methods ApplyChangedSentence() is called for the
     position that the last GetNextWrongSentence() returned.
    */
    virtual void ApplyChangedSentence(const SpellPortions& rChanged) = 0;
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
