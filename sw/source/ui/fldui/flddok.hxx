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
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/edit.hxx>
#include <vcl/field.hxx>

#include "numfmtlb.hxx"
#include "fldpage.hxx"

class SwFieldDokPage : public SwFieldPage
{
    VclPtr<ListBox>            m_pTypeLB;
    VclPtr<VclContainer>       m_pSelection;
    VclPtr<ListBox>            m_pSelectionLB;
    VclPtr<FixedText>          m_pValueFT;
    VclPtr<Edit>               m_pValueED;
    VclPtr<FixedText>          m_pLevelFT;
    VclPtr<NumericField>       m_pLevelED;
    VclPtr<FixedText>          m_pDateFT;
    VclPtr<FixedText>          m_pTimeFT;
    VclPtr<NumericField>       m_pDateOffsetED;
    VclPtr<VclContainer>       m_pFormat;
    VclPtr<ListBox>            m_pFormatLB;
    VclPtr<NumFormatListBox>   m_pNumFormatLB;
    VclPtr<CheckBox>           m_pFixedCB;

    sal_Int32               nOldSel;
    sal_uLong               nOldFormat;

    DECL_LINK_TYPED(TypeHdl, ListBox&, void);
    DECL_LINK_TYPED(FormatHdl, ListBox&, void);
    DECL_LINK_TYPED(SubTypeHdl, ListBox&, void);

    void                AddSubType(sal_uInt16 nTypeId);
    sal_Int32           FillFormatLB(sal_uInt16 nTypeId);

protected:
    virtual sal_uInt16      GetGroup() SAL_OVERRIDE;

public:
                        SwFieldDokPage(vcl::Window* pWindow, const SfxItemSet& rSet);

                        virtual ~SwFieldDokPage();
    virtual void        dispose() SAL_OVERRIDE;

    static VclPtr<SfxTabPage>  Create(vcl::Window* pParent, const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet* rSet ) SAL_OVERRIDE;

    virtual void        FillUserData() SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
