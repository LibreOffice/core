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

#include "swtypes.hxx"
#include <numprevw.hxx>
#include <numberingtypelistbox.hxx>
#include <rtl/ustring.hxx>

class SwWrtShell;
class SwNumRule;
class SwChapterNumRules;

class SwOutlineTabDialog : public SfxTabDialog
{
    static     sal_uInt16    nNumLevel;

    sal_uInt16 m_nNumPosId;
    sal_uInt16 m_nOutlineId;

    OUString            aCollNames[MAXLEVEL];

    SwWrtShell&         rWrtSh;
    std::unique_ptr<SwNumRule>  xNumRule;
    SwChapterNumRules*  pChapterNumRules;

    bool                bModified : 1;

protected:
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
    VclPtr<ListBox>        m_pLevelLB;

    VclPtr<ListBox>        m_pCollBox;
    VclPtr<SwNumberingTypeListBox> m_pNumberBox;
    VclPtr<ListBox>        m_pCharFormatLB;
    VclPtr<FixedText>      m_pAllLevelFT;
    VclPtr<NumericField>   m_pAllLevelNF;
    VclPtr<Edit>           m_pPrefixED;
    VclPtr<Edit>           m_pSuffixED;
    VclPtr<NumericField>   m_pStartEdit;
    VclPtr<NumberingPreview> m_pPreviewWIN;

    OUString            aNoFormatName;
    OUString            aSaveCollNames[MAXLEVEL];
    SwWrtShell*         pSh;
    SwNumRule*          pNumRule;
    OUString*           pCollNames;
    sal_uInt16              nActLevel;

    DECL_LINK( LevelHdl, ListBox&, void );
    DECL_LINK( ToggleComplete, Edit&, void );
    DECL_LINK( CollSelect, ListBox&, void );
    DECL_LINK( CollSelectGetFocus, Control&, void );
    DECL_LINK( NumberSelect, ListBox&, void );
    DECL_LINK( DelimModify, Edit&, void );
    DECL_LINK( StartModified, Edit&, void );
    DECL_LINK( CharFormatHdl, ListBox&, void );

    void    Update();

    void    SetModified(){m_pPreviewWIN->Invalidate();}
    void    CheckForStartValue_Impl(sal_uInt16 nNumberingType);

    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;

public:
    SwOutlineSettingsTabPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwOutlineSettingsTabPage() override;
    virtual void dispose() override;

    void SetWrtShell(SwWrtShell* pShell);

    virtual void        ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC   DeactivatePage(SfxItemSet *pSet) override;

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent,
                                       const SfxItemSet* rAttrSet);
    void SetNumRule(SwNumRule *pRule)
    {
        pNumRule = pRule;
        m_pPreviewWIN->SetNumRule(pNumRule);
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
