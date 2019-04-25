/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#pragma once

#include <vcl/edit.hxx>
#include <vcl/treelistbox.hxx>
#include <svx/stddlg.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

#include <vcl/menubtn.hxx>
#include <com/sun/star/text/XAutoTextContainer2.hpp>

#include <rtl/ustring.hxx>

#include "actctrl.hxx"

class SwGlossaryHdl;
class SwNewGlosNameDlg;
class SwWrtShell;
class SfxViewFrame;
class PopupMenu;
class Menu;

const short RET_EDIT = 100;

class SwGlTreeListBox : public SvTreeListBox
{
    const OUString    sReadonly;

    SvTreeListEntry*  pDragEntry;

    Link<SwGlTreeListBox*,void> m_aDeleteHdl;

    virtual DragDropMode NotifyStartDrag( TransferDataContainer& rContainer,
                                            SvTreeListEntry* ) override;
    virtual bool         NotifyAcceptDrop( SvTreeListEntry* ) override;

    virtual TriState     NotifyMoving(   SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                ) override;
    virtual TriState     NotifyCopying(  SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos) override;

    TriState NotifyCopyingOrMoving( SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    bool              bIsMove);
public:
    SwGlTreeListBox(vcl::Window* pParent, WinBits nBits);

    virtual void RequestHelp( const HelpEvent& rHEvt ) override;
    virtual Size GetOptimalSize() const override;
    void Clear();

    virtual void ExpandedHdl() override;

    virtual void KeyInput( const KeyEvent& rKEvt ) override;

    void SetDeleteHdl( const Link<SwGlTreeListBox*,void>& rLink ) { m_aDeleteHdl = rLink; }
};

class SwOneExampleFrame;
class SwGlossaryDlg : public SvxStandardDialog
{
    friend class SwNewGlosNameDlg;
    friend class SwGlTreeListBox;

    VclPtr<CheckBox>       m_pInsertTipCB;
    VclPtr<Edit>           m_pNameED;
    VclPtr<FixedText>      m_pShortNameLbl;
    TextFilter      m_aNoSpaceFilter;
    VclPtr<Edit>           m_pShortNameEdit;
    VclPtr<SwGlTreeListBox> m_pCategoryBox;
    VclPtr<CheckBox>       m_pFileRelCB;
    VclPtr<CheckBox>       m_pNetRelCB;
    VclPtr<vcl::Window>    m_pExampleWIN;
    VclPtr<PushButton>     m_pInsertBtn;
    VclPtr<MenuButton>     m_pEditBtn;
    VclPtr<PushButton>     m_pBibBtn;
    VclPtr<PushButton>     m_pPathBtn;

    OUString const        m_sReadonlyPath;

    css::uno::Reference< css::text::XAutoTextContainer2 > m_xAutoText;
    std::unique_ptr<SwOneExampleFrame>  m_pExampleFrame;

    SwGlossaryHdl*  m_pGlossaryHdl;

    OUString        m_sResumeGroup;
    OUString        m_sResumeShortName;
    bool            m_bResume;

    const bool      m_bSelection : 1;
    bool            m_bReadOnly : 1;
    bool            m_bIsOld : 1;
    bool            m_bIsDocReadOnly:1;

    SwWrtShell*     m_pShell;

    void EnableShortName(bool bOn = true);
    void ShowPreview();

    DECL_LINK( NameModify, Edit&, void );
    DECL_LINK( NameDoubleClick, SvTreeListBox*, bool );
    DECL_LINK( GrpSelect, SvTreeListBox *, void );
    DECL_LINK( MenuHdl, Menu *, bool );
    DECL_LINK( EnableHdl, Menu *, bool );
    DECL_LINK( BibHdl, Button *, void );
    DECL_LINK( EditHdl, MenuButton *, void );
    DECL_LINK( InsertHdl, Button *, void );
    DECL_LINK( PathHdl, Button *, void );
    DECL_LINK( CheckBoxHdl, Button*, void );
    DECL_LINK( PreviewLoadedHdl, SwOneExampleFrame&, void );
    DECL_LINK( DeleteHdl, SwGlTreeListBox*, void );

    virtual void    Apply() override;
    void            Init();
    SvTreeListEntry*    DoesBlockExist(const OUString& sBlock, const OUString& rShort);
    void            ShowAutoText(const OUString& rGroup, const OUString& rShortName);
    void            ResumeShowAutoText();

    bool            GetResumeData(OUString& rGroup, OUString& rShortName)
                        {rGroup = m_sResumeGroup; rShortName = m_sResumeShortName; return m_bResume;}
    void            SetResumeData(const OUString& rGroup, const OUString& rShortName)
                        {m_sResumeGroup = rGroup; m_sResumeShortName = rShortName; m_bResume = true;}

    void            DeleteEntry();
public:
    SwGlossaryDlg(SfxViewFrame const * pViewFrame, SwGlossaryHdl* pGlosHdl, SwWrtShell *pWrtShell);
    virtual ~SwGlossaryDlg() override;
    virtual void dispose() override;
    OUString GetCurrGrpName() const;
    OUString GetCurrShortName() const
    {
        return m_pShortNameEdit->GetText();
    }
    static OUString GetCurrGroup();
    static void     SetActGroup(const OUString& rNewGroup);
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
