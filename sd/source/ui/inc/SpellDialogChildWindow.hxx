/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SpellDialogChildWindow.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:16:25 $
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

#ifndef SD_SPELL_DIALOG_CHILD_WINDOW_HXX
#define SD_SPELL_DIALOG_CHILD_WINDOW_HXX

#ifndef SVX_SPELL_DIALOG_CHILD_WINDOW_HXX
#include <svx/SpellDialogChildWindow.hxx>
#endif


namespace sd {

class Outliner;

/** This derivation of the ::svx::SpellDialogChildWindow base class
    provides Draw and Impress specific implementations of
    GetNextWrongSentence() and ApplyChangedSentence().
*/
class SpellDialogChildWindow
    : public ::svx::SpellDialogChildWindow
{
public:
    SpellDialogChildWindow (
        ::Window* pParent,
        USHORT nId,
        SfxBindings* pBindings,
        SfxChildWinInfo* pInfo);
    virtual ~SpellDialogChildWindow (void);

    /** This method makes the one from the base class public so that
        it can be called from the view shell when one is created.
    */
    virtual void InvalidateSpellDialog (void);

    SFX_DECL_CHILDWINDOW(SpellDialogChildWindow);

protected:
    /** Iterate over the sentences in all text shapes and stop at the
        next sentence with spelling errors. While doing so the view
        mode may be changed and text shapes are set into edit mode.
    */
    virtual ::svx::SpellPortions GetNextWrongSentence (void);

    /** This method is responsible for merging corrections made in the
        spelling dialog back into the document.
    */
    virtual void ApplyChangedSentence (const ::svx::SpellPortions& rChanged);
    virtual void GetFocus (void);
    virtual void LoseFocus (void);

private:
    /** This outliner is used to do the main work of iterating over a
        document and finding sentences with spelling errors.
    */
    Outliner* mpSdOutliner;

    /** When this flag is <TRUE/> then eventually we have to destroy
        the outliner in mpSdOutliner.
    */
    bool mbOwnOutliner;

    /** Provide an outliner in the mpSdOutliner data member.  When the
        view shell has changed since the last call this include the
        deletion/release of formerly created/obtained one prior to
        construction/obtaining of a new one.
    */
    void ProvideOutliner (void);
};

} // end of namespace ::sd

#endif
