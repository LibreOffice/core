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
#ifndef INCLUDED_SW_SOURCE_UI_FLDUI_FLDDOK_HXX
#define INCLUDED_SW_SOURCE_UI_FLDUI_FLDDOK_HXX

#include <sfx2/tabdlg.hxx>

#include <numfmtlb.hxx>
#include "fldpage.hxx"

class SwFieldDokPage : public SwFieldPage
{
    sal_Int32               nOldSel;
    sal_uLong               nOldFormat;

    std::unique_ptr<weld::TreeView> m_xTypeLB;
    std::unique_ptr<weld::Widget> m_xSelection;
    std::unique_ptr<weld::TreeView> m_xSelectionLB;
    std::unique_ptr<weld::Label> m_xValueFT;
    std::unique_ptr<weld::Entry> m_xValueED;
    std::unique_ptr<weld::Label> m_xLevelFT;
    std::unique_ptr<weld::SpinButton> m_xLevelED;
    std::unique_ptr<weld::Label> m_xDateFT;
    std::unique_ptr<weld::Label> m_xTimeFT;
    std::unique_ptr<weld::SpinButton> m_xDateOffsetED;
    std::unique_ptr<weld::Widget> m_xFormat;
    std::unique_ptr<weld::TreeView> m_xFormatLB;
    std::unique_ptr<SwNumFormatTreeView> m_xNumFormatLB;
    std::unique_ptr<weld::CheckButton> m_xFixedCB;

    DECL_LINK(TypeHdl, weld::TreeView&, void);
    DECL_LINK(FormatHdl, weld::TreeView&, void);
    DECL_LINK(SubTypeHdl, weld::TreeView&, void);

    void                AddSubType(sal_uInt16 nTypeId);
    sal_Int32           FillFormatLB(sal_uInt16 nTypeId);

protected:
    virtual sal_uInt16      GetGroup() override;

public:
    SwFieldDokPage(TabPageParent pWindow, const SfxItemSet * pSet);

    virtual ~SwFieldDokPage() override;

    static VclPtr<SfxTabPage>  Create(TabPageParent pParent, const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;

    virtual void        FillUserData() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
