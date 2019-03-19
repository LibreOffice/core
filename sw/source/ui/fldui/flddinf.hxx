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
#ifndef INCLUDED_SW_SOURCE_UI_FLDUI_FLDDINF_HXX
#define INCLUDED_SW_SOURCE_UI_FLDUI_FLDDINF_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/treelistbox.hxx>

#include <numfmtlb.hxx>
#include "fldpage.hxx"

namespace com{namespace sun{ namespace star{ namespace beans{
    class XPropertySet;
}}}}

class SwFieldDokInfPage : public SwFieldPage
{
    std::unique_ptr<weld::TreeIter> m_xSelEntry;
    css::uno::Reference < css::beans::XPropertySet > xCustomPropertySet;

    sal_Int32               nOldSel;
    sal_uLong               nOldFormat;
    OUString                m_sOldCustomFieldName;

    std::unique_ptr<weld::TreeView> m_xTypeTLB;
    std::unique_ptr<weld::Widget> m_xSelection;
    std::unique_ptr<weld::TreeView> m_xSelectionLB;
    std::unique_ptr<weld::Widget> m_xFormat;
    std::unique_ptr<SwNumFormatTreeView> m_xFormatLB;
    std::unique_ptr<weld::CheckButton> m_xFixedCB;

    DECL_LINK(TypeHdl, weld::TreeView&, void);
    DECL_LINK(SubTypeHdl, weld::TreeView&, void);

    sal_Int32               FillSelectionLB(sal_uInt16 nSubTypeId);

protected:
    virtual sal_uInt16      GetGroup() override;

public:
    SwFieldDokInfPage(TabPageParent pWindow, const SfxItemSet* pSet);
    virtual ~SwFieldDokInfPage() override;

    static VclPtr<SfxTabPage>  Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    virtual void        FillUserData() override;
};

void FillFieldSelect(ListBox& rListBox);
void FillFieldSelect(weld::TreeView& rListBox);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
