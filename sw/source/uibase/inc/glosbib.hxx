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

#include <sfx2/basedlgs.hxx>
#include <rtl/ustring.hxx>
#include <vcl/weld.hxx>

#include <vector>

class SwGlossaryHdl;

struct GlosBibUserData
{
    OUString sPath;
    OUString sGroupName;
    OUString sGroupTitle;
};

class SwGlossaryGroupDlg final : public SfxDialogController
{
    std::vector<OUString> m_RemovedArr;
    std::vector<OUString> m_InsertedArr;
    std::vector<OUString> m_RenamedArr;

    weld::Window* m_pParent;
    SwGlossaryHdl* pGlosHdl;

    OUString sCreatedGroup;

    std::unique_ptr<weld::Entry> m_xNameED;
    std::unique_ptr<weld::ComboBox> m_xPathLB;
    std::unique_ptr<weld::TreeView> m_xGroupTLB;

    std::unique_ptr<weld::Button> m_xNewPB;
    std::unique_ptr<weld::Button> m_xDelPB;
    std::unique_ptr<weld::Button> m_xRenamePB;

    bool IsDeleteAllowed(const OUString& rGroup);

    void Apply();
    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(NewHdl, weld::Button&, void);
    DECL_LINK(DeleteHdl, weld::Button&, void);
    DECL_LINK(ModifyHdl, weld::Entry&, void);
    DECL_LINK(ModifyListBoxHdl, weld::ComboBox&, void);
    DECL_LINK(RenameHdl, weld::Button&, void);
    DECL_STATIC_LINK(SwGlossaryGroupDlg, EditInsertTextHdl, OUString&, bool);
    DECL_LINK(EntrySizeAllocHdl, const Size&, void);

public:
    SwGlossaryGroupDlg(weld::Window* pParent, std::vector<OUString> const& rPathArr,
                       SwGlossaryHdl* pGlosHdl);
    virtual short run() override;
    virtual ~SwGlossaryGroupDlg() override;

    const OUString& GetCreatedGroupName() const { return sCreatedGroup; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
