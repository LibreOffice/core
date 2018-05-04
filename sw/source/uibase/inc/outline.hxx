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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_OUTLINE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_OUTLINE_HXX

#include <memory>
#include <sfx2/tabdlg.hxx>

#include <vcl/menu.hxx>

#include <vcl/button.hxx>

#include <vcl/fixed.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/edit.hxx>

#include <vcl/field.hxx>

#include <swtypes.hxx>
#include "numprevw.hxx"
#include "numberingtypelistbox.hxx"
#include <rtl/ustring.hxx>

class SwWrtShell;
class SwNumRule;
class SwChapterNumRules;

class SwOutlineTabDialog final : public SfxTabDialog
{
    static     sal_uInt16    nNumLevel;

    sal_uInt16 m_nNumPosId;
    sal_uInt16 m_nOutlineId;

    OUString            aCollNames[MAXLEVEL];

    SwWrtShell&         rWrtSh;
    std::unique_ptr<SwNumRule>  xNumRule;
    SwChapterNumRules*  pChapterNumRules;

    bool                bModified : 1;

    DECL_LINK(CancelHdl, Button*, void);
    DECL_LINK( FormHdl, Button *, void );
    DECL_LINK( MenuSelectHdl, Menu *, bool );

    virtual void    PageCreated(sal_uInt16 nPageId, SfxTabPage& rPage) override;
    virtual short   Ok() override;

public:
    SwOutlineTabDialog(vcl::Window* pParent,
                    const SfxItemSet* pSwItemSet,
                    SwWrtShell &);
    virtual ~SwOutlineTabDialog() override;
    virtual void        dispose() override;

    SwNumRule*          GetNumRule() { return xNumRule.get(); }
    sal_uInt16          GetLevel(const OUString &rFormatName) const;
    OUString*           GetCollNames() {return aCollNames;}

    static sal_uInt16   GetActNumLevel() {return nNumLevel;}
    static void         SetActNumLevel(sal_uInt16 nSet) {nNumLevel = nSet;}
};

class SwOutlineSettingsTabPage : public SfxTabPage
{
    OUString            aNoFormatName;
    OUString            aSaveCollNames[MAXLEVEL];
    SwWrtShell*         pSh;
    SwNumRule*          pNumRule;
    OUString*           pCollNames;
    sal_uInt16          nActLevel;

    std::unique_ptr<weld::TreeView> m_xLevelLB;
    std::unique_ptr<weld::ComboBoxText> m_xCollBox;
    std::unique_ptr<NumberingTypeListBox> m_xNumberBox;
    std::unique_ptr<weld::ComboBoxText> m_xCharFormatLB;
    std::unique_ptr<weld::Label> m_xAllLevelFT;
    std::unique_ptr<weld::SpinButton>  m_xAllLevelNF;
    std::unique_ptr<weld::Entry> m_xPrefixED;
    std::unique_ptr<weld::Entry> m_xSuffixED;
    std::unique_ptr<weld::SpinButton> m_xStartEdit;
    std::unique_ptr<SwNumberingPreview> m_xPreviewWIN;

    DECL_LINK( LevelHdl, weld::TreeView&, void );
    DECL_LINK( ToggleComplete, weld::SpinButton&, void );
    DECL_LINK( CollSelect, weld::ComboBoxText&, void );
    DECL_LINK( CollSelectGetFocus, weld::Widget&, void );
    DECL_LINK( NumberSelect, weld::ComboBoxText&, void );
    DECL_LINK( DelimModify, weld::Entry&, void );
    DECL_LINK( StartModified, weld::SpinButton&, void );
    DECL_LINK( CharFormatHdl, weld::ComboBoxText&, void );

    void    Update();

    void    SetModified() { m_xPreviewWIN->queue_draw(); }
    void    CheckForStartValue_Impl(sal_uInt16 nNumberingType);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwOutlineSettingsTabPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SwOutlineSettingsTabPage() override;

    void SetWrtShell(SwWrtShell* pShell);

    virtual void        ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                       const SfxItemSet* rAttrSet);
    void SetNumRule(SwNumRule *pRule)
    {
        pNumRule = pRule;
        m_xPreviewWIN->SetNumRule(pNumRule);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
