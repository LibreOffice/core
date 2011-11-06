/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SW_SPELL_DIALOG_CHILD_WINDOW_HXX
#define SW_SPELL_DIALOG_CHILD_WINDOW_HXX

#include <svx/SpellDialogChildWindow.hxx>
#ifndef _VIEW_HXX
#include <view.hxx>
#endif

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

