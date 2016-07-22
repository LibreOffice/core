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
#ifndef INCLUDED_SFX2_MGETEMPL_HXX
#define INCLUDED_SFX2_MGETEMPL_HXX

#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/vclmedit.hxx>
#include <sfx2/tabdlg.hxx>

/* expected:
    SID_TEMPLATE_NAME   :   In: StringItem, Name of Template
    SID_TEMPLATE_FAMILY :   In: Family of Template
*/

class SfxStyleFamilies;
class SfxStyleFamilyItem;
class SfxStyleSheetBase;

class SfxManageStyleSheetPage : public SfxTabPage
{
    VclPtr<VclMultiLineEdit> m_pNameRo;
    VclPtr<Edit>             m_pNameRw;

    VclPtr<CheckBox>         m_pAutoCB;

    VclPtr<FixedText>        m_pFollowFt;
    VclPtr<ListBox>          m_pFollowLb;
    VclPtr<PushButton>       m_pEditStyleBtn;

    VclPtr<FixedText>        m_pBaseFt;
    VclPtr<ListBox>          m_pBaseLb;
    VclPtr<PushButton>       m_pEditLinkStyleBtn;

    VclPtr<FixedText>        m_pFilterFt;
    VclPtr<ListBox>          m_pFilterLb;

    VclPtr<FixedText>        m_pDescFt;

    SfxStyleSheetBase *pStyle;
    SfxStyleFamilies *pFamilies;
    const SfxStyleFamilyItem *pItem;
    OUString aBuf;
    bool bModified;

    // initial data for the style
    OUString aName;
    OUString aFollow;
    OUString aParent;
    sal_uInt16 nFlags;

private:
friend class SfxStyleDialog;

    DECL_LINK_TYPED( GetFocusHdl, Control&, void );
    DECL_LINK_TYPED( LoseFocusHdl, Control&, void );
    DECL_LINK_TYPED( EditStyleSelectHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( EditStyleHdl_Impl, Button*, void );
    DECL_LINK_TYPED( EditLinkStyleSelectHdl_Impl, ListBox&, void );
    DECL_LINK_TYPED( EditLinkStyleHdl_Impl, Button*, void );

    void    UpdateName_Impl(ListBox *, const OUString &rNew);
    void    SetDescriptionText_Impl();

    virtual ~SfxManageStyleSheetPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( vcl::Window* pParent, const SfxItemSet* );

protected:
    virtual bool        FillItemSet(SfxItemSet *) override;
    virtual void        Reset(const SfxItemSet *) override;

    static bool    Execute_Impl( sal_uInt16 nId, const OUString& rStr, const OUString& rRefStr,
                          sal_uInt16 nFamily, sal_uInt16 nMask = 0 );
    using TabPage::ActivatePage;
        virtual void        ActivatePage(const SfxItemSet &) override;
        using TabPage::DeactivatePage;
    virtual DeactivateRC DeactivatePage(SfxItemSet *) override;

public:
    SfxManageStyleSheetPage(vcl::Window *pParent, const SfxItemSet &rAttrSet );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
