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

#ifndef GLOSBIB_HXX
#define GLOSBIB_HXX

#include <vcl/edit.hxx>
#include <svx/stddlg.hxx>

#include <vcl/lstbox.hxx>
#include <svtools/svtabbx.hxx>

#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include "rtl/ustring.hxx"

#include <vector>

class SwGlossaryHdl;

class FEdit : public Edit
{
public:
    FEdit(Window* pParent, const ResId& rResId)
        : Edit(pParent, rResId)
    {
    }

    FEdit(Window* pParent)
        : Edit(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK)
    {
    }

    virtual void KeyInput( const KeyEvent& rKEvent );
};

struct GlosBibUserData
{
    String sPath;
    String sGroupName;
    String sGroupTitle;
};

class SwGlossaryGroupTLB : public SvTabListBox
{
public:
    SwGlossaryGroupTLB(Window* pParent, const ResId& rResId)
        : SvTabListBox(pParent, rResId)
    {
    }

    SwGlossaryGroupTLB(Window* pParent)
        : SvTabListBox(pParent, WB_BORDER|WB_HSCROLL|WB_CLIPCHILDREN|WB_SORT)
    {
    }

    virtual void RequestHelp( const HelpEvent& rHEvt );
};

class SwGlossaryGroupDlg : public SvxStandardDialog
{
    FEdit*              m_pNameED;
    ListBox*            m_pPathLB;
    SwGlossaryGroupTLB* m_pGroupTLB;

    PushButton*     m_pNewPB;
    PushButton*     m_pDelPB;
    PushButton*     m_pRenamePB;

    typedef std::vector< OUString> OUVector_t;
    OUVector_t m_RemovedArr;
    OUVector_t m_InsertedArr;
    OUVector_t m_RenamedArr;

    SwGlossaryHdl   *pGlosHdl;

    String          sCreatedGroup;

    sal_Bool            IsDeleteAllowed(const OUString &rGroup);

protected:
    virtual void Apply();
    DECL_LINK( SelectHdl, SvTabListBox* );
    DECL_LINK(NewHdl, void *);
    DECL_LINK( DeleteHdl, Button*  );
    DECL_LINK(ModifyHdl, void *);
    DECL_LINK(RenameHdl, void *);

public:
    SwGlossaryGroupDlg(Window * pParent,
                        std::vector<OUString> const& rPathArr,
                        SwGlossaryHdl *pGlosHdl);
    ~SwGlossaryGroupDlg();

    const String&       GetCreatedGroupName() const {return sCreatedGroup;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
