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
    RadioButton* m_pCharKerningRB;
    RadioButton* m_pCharPunctKerningRB;

    RadioButton* m_pNoCompressionRB;
    RadioButton* m_pPunctCompressionRB;
    RadioButton* m_pPunctKanaCompressionRB;

    FixedText*      m_pLanguageFT;
    SvxLanguageBox* m_pLanguageLB;
    CheckBox*       m_pStandardCB;

    FixedText*   m_pStartFT;
    Edit*        m_pStartED;
    FixedText*   m_pEndFT;
    Edit*        m_pEndED;
    FixedText*   m_pHintFT;

    SvxAsianLayoutPage_Impl* pImpl;

    DECL_LINK(LanguageHdl, void *);
    DECL_LINK(ChangeStandardHdl, CheckBox*);
    DECL_LINK(ModifyHdl, Edit*);

    SvxAsianLayoutPage( Window* pParent, const SfxItemSet& rSet );
public:

    virtual ~SvxAsianLayoutPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    static sal_uInt16*  GetRanges();
    virtual bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
