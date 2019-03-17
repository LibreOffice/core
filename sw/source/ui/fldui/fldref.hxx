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
#ifndef INCLUDED_SW_SOURCE_UI_FLDUI_FLDREF_HXX
#define INCLUDED_SW_SOURCE_UI_FLDUI_FLDREF_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/edit.hxx>

#include "fldpage.hxx"
#include <IDocumentOutlineNodes.hxx>
#include <IDocumentListItems.hxx>
class SwTextNode;

class SwFieldRefPage : public SwFieldPage
{
    OUString    sBookmarkText;
    OUString    sFootnoteText;
    OUString    sEndnoteText;
    // #i83479#
    OUString    sHeadingText;
    OUString    sNumItemText;

    IDocumentOutlineNodes::tSortedOutlineNodeList maOutlineNodes;
    IDocumentListItems::tSortedNodeNumList maNumItems;

    // selected text node in the listbox for headings and numbered items
    // in order to restore selection after update of selection listbox
    const SwTextNode* mpSavedSelectedTextNode;
    // fallback, if previously selected text node doesn't exist anymore
    size_t mnSavedSelectedPos;

    std::unique_ptr<weld::TreeView> m_xTypeLB;
    std::unique_ptr<weld::Widget> m_xSelection;
    std::unique_ptr<weld::TreeView> m_xSelectionLB;
    // #i83479#
    std::unique_ptr<weld::TreeView> m_xSelectionToolTipLB;
    std::unique_ptr<weld::Widget> m_xFormat;
    std::unique_ptr<weld::TreeView> m_xFormatLB;
    std::unique_ptr<weld::Label> m_xNameFT;
    std::unique_ptr<weld::Entry> m_xNameED;
    std::unique_ptr<weld::Entry> m_xValueED;
    std::unique_ptr<weld::Entry> m_xFilterED;

    DECL_LINK(TypeHdl, weld::TreeView&, void);
    DECL_LINK(SubTypeListBoxHdl, weld::TreeView&, void);
    DECL_LINK(SubTypeTreeListBoxHdl, weld::TreeView&, void);
    DECL_LINK(ModifyHdl, weld::Entry&, void);
    DECL_LINK(ModifyHdl_Impl, weld::Entry&, void);

    void SubTypeHdl();

    void                UpdateSubType(const OUString& filterString);

    static bool                MatchSubstring( const OUString& list_string, const OUString& substr );

    sal_Int32               FillFormatLB(sal_uInt16 nTypeId);

    // #i83479#
    void SaveSelectedTextNode();

protected:
    virtual sal_uInt16      GetGroup() override;

public:
    SwFieldRefPage(TabPageParent pParent, const SfxItemSet* pSet);
    virtual ~SwFieldRefPage() override;

    static VclPtr<SfxTabPage>  Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    virtual void        FillUserData() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
