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
#include <vcl/group.hxx>
#include <vcl/edit.hxx>

#include "fldpage.hxx"
#include <IDocumentOutlineNodes.hxx>
#include <IDocumentListItems.hxx>
#include <FldRefTreeListBox.hxx>
class SwTxtNode;

class SwFldRefPage : public SwFldPage
{
    ListBox*        m_pTypeLB;
    VclContainer*   m_pSelection;
    ListBox*        m_pSelectionLB;
    // #i83479#
    SwFldRefTreeListBox* m_pSelectionToolTipLB;
    VclContainer*   m_pFormat;
    ListBox*        m_pFormatLB;
    FixedText*      m_pNameFT;
    Edit*           m_pNameED;
    Edit*           m_pValueED;
    OUString    sBookmarkTxt;
    OUString    sFootnoteTxt;
    OUString    sEndnoteTxt;
    // #i83479#
    OUString    sHeadingTxt;
    OUString    sNumItemTxt;

    IDocumentOutlineNodes::tSortedOutlineNodeList maOutlineNodes;
    IDocumentListItems::tSortedNodeNumList maNumItems;

    // selected text node in the listbox for headings and numbered items
    // in order to restore selection after update of selection listbox
    const SwTxtNode* mpSavedSelectedTxtNode;
    // fallback, if previously selected text node doesn't exist anymore
    size_t mnSavedSelectedPos;

    DECL_LINK(TypeHdl, void *);
    DECL_LINK(SubTypeHdl, void * = 0);
    DECL_LINK(ModifyHdl, void * = 0);

    void                UpdateSubType();
    sal_Int32               FillFormatLB(sal_uInt16 nTypeId);

    // #i83479#
    void SaveSelectedTxtNode();

protected:
    virtual sal_uInt16      GetGroup() SAL_OVERRIDE;

public:
                        SwFldRefPage(vcl::Window* pParent, const SfxItemSet& rSet);

                        virtual ~SwFldRefPage();

    static SfxTabPage*  Create(vcl::Window* pParent, const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;

    virtual void        FillUserData() SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
