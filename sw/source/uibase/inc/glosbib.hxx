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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_GLOSBIB_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_GLOSBIB_HXX

#include <vcl/edit.hxx>
#include <svx/stddlg.hxx>

#include <vcl/lstbox.hxx>
#include <svtools/svtabbx.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <rtl/ustring.hxx>

#include <vector>

class SwGlossaryHdl;

class FEdit : public Edit
{
public:
    FEdit(vcl::Window* pParent, const ResId& rResId)
        : Edit(pParent, rResId)
    {
    }

    FEdit(vcl::Window* pParent)
        : Edit(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK)
    {
    }

    virtual void KeyInput( const KeyEvent& rKEvent ) SAL_OVERRIDE;
};

struct GlosBibUserData
{
    OUString sPath;
    OUString sGroupName;
    OUString sGroupTitle;
};

class SwGlossaryGroupTLB : public SvTabListBox
{
public:
    SwGlossaryGroupTLB(vcl::Window* pParent)
        : SvTabListBox(pParent, WB_BORDER|WB_HSCROLL|WB_CLIPCHILDREN|WB_SORT)
    {
    }

    virtual void RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;
};

class SwGlossaryGroupDlg : public SvxStandardDialog
{
    VclPtr<FEdit>              m_pNameED;
    VclPtr<ListBox>            m_pPathLB;
    VclPtr<SwGlossaryGroupTLB> m_pGroupTLB;

    VclPtr<PushButton>     m_pNewPB;
    VclPtr<PushButton>     m_pDelPB;
    VclPtr<PushButton>     m_pRenamePB;

    typedef std::vector< OUString> OUVector_t;
    OUVector_t m_RemovedArr;
    OUVector_t m_InsertedArr;
    OUVector_t m_RenamedArr;

    SwGlossaryHdl   *pGlosHdl;

    OUString        sCreatedGroup;

    bool            IsDeleteAllowed(const OUString &rGroup);

protected:
    virtual void Apply() SAL_OVERRIDE;
    DECL_LINK_TYPED( SelectHdl, SvTreeListBox*, void );
    DECL_LINK_TYPED(NewHdl, Button *, void);
    DECL_LINK_TYPED( DeleteHdl, Button*, void  );
    DECL_LINK(ModifyHdl, void *);
    DECL_LINK_TYPED(ModifyListBoxHdl, ListBox&, void);
    DECL_LINK_TYPED(RenameHdl, Button *, void);

public:
    SwGlossaryGroupDlg(vcl::Window * pParent,
                        std::vector<OUString> const& rPathArr,
                        SwGlossaryHdl *pGlosHdl);
    virtual ~SwGlossaryGroupDlg();
    virtual void dispose() SAL_OVERRIDE;

    const OUString&     GetCreatedGroupName() const {return sCreatedGroup;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
