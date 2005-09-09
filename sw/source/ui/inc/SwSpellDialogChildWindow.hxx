/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwSpellDialogChildWindow.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 08:57:24 $
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

#ifndef SW_SPELL_DIALOG_CHILD_WINDOW_HXX
#define SW_SPELL_DIALOG_CHILD_WINDOW_HXX

#ifndef SVX_SPELL_DIALOG_CHILD_WINDOW_HXX
#include <svx/SpellDialogChildWindow.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif

class SwWrtShell;
struct SpellState;
class SwSpellDialogChildWindow
    : public ::svx::SpellDialogChildWindow
{
    SpellState*     m_pSpellState;

    SwWrtShell*     GetWrtShell_Impl();
    bool            MakeTextSelection_Impl(SwWrtShell& rSh, ShellModes eSelMode);
    bool            FindNextDrawTextError_Impl(SwWrtShell& rSh);
    bool            SpellDrawText_Impl(SwWrtShell& rSh, ::svx::SpellPortions& rPortions);
    void            LockFocusNotification(bool bLock);

protected:
    virtual svx::SpellPortions GetNextWrongSentence (void);
    virtual void ApplyChangedSentence(const svx::SpellPortions& rChanged);
    virtual void AddAutoCorrection(const String& rOld, const String& rNew, LanguageType eLanguage);
    virtual bool HasAutoCorrection();
    virtual void GetFocus();
    virtual void LoseFocus();

public:
    SwSpellDialogChildWindow (
        Window*pParent,
        USHORT nId,
        SfxBindings* pBindings,
        SfxChildWinInfo* pInfo);
    ~SwSpellDialogChildWindow();

    SFX_DECL_CHILDWINDOW(SwSpellDialogChildWindow);

    void    InvalidateSpellDialog();

};

#endif

