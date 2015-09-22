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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_GLOSSARY_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_GLOSSARY_HXX

#include <vcl/edit.hxx>
#include <svtools/treelistbox.hxx>
#include <svx/stddlg.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

#include <vcl/combobox.hxx>

#include <vcl/menubtn.hxx>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/text/XAutoTextContainer2.hpp>

#include <rtl/ustring.hxx>

#include <actctrl.hxx>

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

    virtual DragDropMode NotifyStartDrag( TransferDataContainer& rContainer,
                                            SvTreeListEntry* ) SAL_OVERRIDE;
    virtual bool         NotifyAcceptDrop( SvTreeListEntry* ) SAL_OVERRIDE;

    virtual TriState     NotifyMoving(   SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                ) SAL_OVERRIDE;
    virtual TriState     NotifyCopying(  SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos) SAL_OVERRIDE;

    TriState NotifyCopyingOrMoving( SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    bool              bIsMove);
public:
    SwGlTreeListBox(vcl::Window* pParent, WinBits nBits);

    virtual void RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;
    virtual Size GetOptimalSize() const SAL_OVERRIDE;
    void Clear();
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

    OUString        sReadonlyPath;

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XAutoTextContainer2 > m_xAutoText;
    SwOneExampleFrame*  pExampleFrame;

    SwGlossaryHdl*  pGlossaryHdl;

    OUString        sResumeGroup;
    OUString        sResumeShortName;
    bool            bResume;

    const bool      bSelection : 1;
    bool            bReadOnly : 1;
    bool            bIsOld : 1;
    bool            bIsDocReadOnly:1;

    SwWrtShell*     pSh;

    void EnableShortName(bool bOn = true);
    void ShowPreview();

    DECL_LINK( NameModify, Edit * );
    DECL_LINK_TYPED( NameDoubleClick, SvTreeListBox*, bool );
    DECL_LINK_TYPED( GrpSelect, SvTreeListBox *, void );
    DECL_LINK_TYPED( MenuHdl, Menu *, bool );
    DECL_LINK_TYPED( EnableHdl, Menu *, bool );
    DECL_LINK_TYPED(BibHdl, Button *, void);
    DECL_LINK_TYPED(EditHdl, MenuButton *, void);
    DECL_LINK_TYPED(InsertHdl, Button *, void);
    DECL_LINK_TYPED( PathHdl, Button *, void );
    DECL_LINK_TYPED( CheckBoxHdl, Button*, void );
    DECL_LINK_TYPED( PreviewLoadedHdl, SwOneExampleFrame&, void );

    virtual void    Apply() SAL_OVERRIDE;
    void            Init();
    SvTreeListEntry*    DoesBlockExist(const OUString& sBlock, const OUString& rShort);
    void            ShowAutoText(const OUString& rGroup, const OUString& rShortName);
    void            ResumeShowAutoText();

    bool            GetResumeData(OUString& rGroup, OUString& rShortName)
                        {rGroup = sResumeGroup; rShortName = sResumeShortName; return bResume;}
    void            SetResumeData(const OUString& rGroup, const OUString& rShortName)
                        {sResumeGroup = rGroup; sResumeShortName = rShortName; bResume = true;}
    void            ResetResumeData() {bResume = false;}
public:
    SwGlossaryDlg(SfxViewFrame* pViewFrame, SwGlossaryHdl* pGlosHdl, SwWrtShell *pWrtShell);
    virtual ~SwGlossaryDlg();
    virtual void dispose() SAL_OVERRIDE;
    OUString GetCurrGrpName() const;
    OUString GetCurrShortName() const
    {
        return m_pShortNameEdit->GetText();
    }
    static OUString GetCurrGroup();
    static void     SetActGroup(const OUString& rNewGroup);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
