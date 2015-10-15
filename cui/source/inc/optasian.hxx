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
#ifndef INCLUDED_CUI_SOURCE_INC_OPTASIAN_HXX
#define INCLUDED_CUI_SOURCE_INC_OPTASIAN_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/group.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <svx/langbox.hxx>
struct SvxAsianLayoutPage_Impl;
class SvxAsianLayoutPage : public SfxTabPage
{
    VclPtr<RadioButton> m_pCharKerningRB;
    VclPtr<RadioButton> m_pCharPunctKerningRB;

    VclPtr<RadioButton> m_pNoCompressionRB;
    VclPtr<RadioButton> m_pPunctCompressionRB;
    VclPtr<RadioButton> m_pPunctKanaCompressionRB;

    VclPtr<FixedText>      m_pLanguageFT;
    VclPtr<SvxLanguageBox> m_pLanguageLB;
    VclPtr<CheckBox>       m_pStandardCB;

    VclPtr<FixedText>   m_pStartFT;
    VclPtr<Edit>        m_pStartED;
    VclPtr<FixedText>   m_pEndFT;
    VclPtr<Edit>        m_pEndED;
    VclPtr<FixedText>   m_pHintFT;

    SvxAsianLayoutPage_Impl* pImpl;

    DECL_LINK_TYPED(LanguageHdl, ListBox&, void);
    DECL_LINK_TYPED(ChangeStandardHdl, Button*, void);
    DECL_LINK_TYPED(ModifyHdl, Edit&, void);

public:
    SvxAsianLayoutPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxAsianLayoutPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );
    static const sal_uInt16*  GetRanges();
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
