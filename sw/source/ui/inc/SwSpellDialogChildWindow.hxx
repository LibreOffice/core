/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SW_SPELL_DIALOG_CHILD_WINDOW_HXX
#define SW_SPELL_DIALOG_CHILD_WINDOW_HXX

#include <svx/SpellDialogChildWindow.hxx>
#include <view.hxx>

class SwWrtShell;
struct SpellState;
class SwSpellDialogChildWindow
    : public ::svx::SpellDialogChildWindow
{
    bool            m_bIsGrammarCheckingOn;
    SpellState*     m_pSpellState;

    SwWrtShell*     GetWrtShell_Impl();
    bool            MakeTextSelection_Impl(SwWrtShell& rSh, ShellModes eSelMode);
    bool            FindNextDrawTextError_Impl(SwWrtShell& rSh);
    bool            SpellDrawText_Impl(SwWrtShell& rSh, ::svx::SpellPortions& rPortions);
    void            LockFocusNotification(bool bLock);

protected:
    virtual svx::SpellPortions GetNextWrongSentence(bool bRecheck);
    virtual void ApplyChangedSentence(const svx::SpellPortions& rChanged, bool bRecheck);
    virtual void AddAutoCorrection(const String& rOld, const String& rNew, LanguageType eLanguage);
    virtual bool HasAutoCorrection();
    virtual bool HasGrammarChecking();
    virtual bool IsGrammarChecking();
    virtual void SetGrammarChecking(bool bOn);
    virtual void GetFocus();
    virtual void LoseFocus();

public:
    SwSpellDialogChildWindow (
        Window*pParent,
        sal_uInt16 nId,
        SfxBindings* pBindings,
        SfxChildWinInfo* pInfo);
    ~SwSpellDialogChildWindow();

    SFX_DECL_CHILDWINDOW(SwSpellDialogChildWindow);

    void    InvalidateSpellDialog();

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
