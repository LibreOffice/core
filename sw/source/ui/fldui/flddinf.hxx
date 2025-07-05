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
#include <numfmtlb.hxx>
#include "fldpage.hxx"

namespace com::sun::star::beans { class XPropertySet; }
enum class SwDocInfoSubType : sal_uInt16;

class SwFieldDokInfPage : public SwFieldPage
{
    std::unique_ptr<weld::TreeIter> m_xSelEntry;
    css::uno::Reference < css::beans::XPropertySet > m_xCustomPropertySet;

    sal_Int32               m_nOldSel;
    sal_uInt32               m_nOldFormat;
    OUString                m_sOldCustomFieldName;

    std::unique_ptr<weld::TreeView> m_xTypeList;
    std::unique_ptr<weld::TreeView> m_xTypeTree;
    weld::TreeView* m_pTypeView;
    std::unique_ptr<weld::Widget> m_xSelection;
    std::unique_ptr<weld::TreeView> m_xSelectionLB;
    std::unique_ptr<weld::Widget> m_xFormat;
    std::unique_ptr<SwNumFormatTreeView> m_xFormatLB;
    std::unique_ptr<weld::CheckButton> m_xFixedCB;

    DECL_LINK(TypeHdl, weld::TreeView&, void);
    DECL_LINK(SubTypeHdl, weld::TreeView&, void);

    sal_Int32               FillSelectionLB(SwDocInfoSubType nSubTypeId);

protected:
    virtual sal_uInt16      GetGroup() override;

public:
    SwFieldDokInfPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* pSet);
    virtual ~SwFieldDokInfPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    virtual void        FillUserData() override;
};

void FillFieldSelect(weld::TreeView& rListBox);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
