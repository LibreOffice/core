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

#ifndef INCLUDED_CUI_SOURCE_INC_ACCCFG_HXX
#define INCLUDED_CUI_SOURCE_INC_ACCCFG_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <algorithm>

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/treelistbox.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/basedlgs.hxx>
#include "cfgutil.hxx"

class SfxMacroInfoItem;
class SfxConfigGroupListBox;
class SfxConfigFunctionListBox;
class SfxAcceleratorConfigPage;
class SfxStringItem;

class SfxAccCfgTabListBox_Impl : public SvTabListBox
{
    VclPtr<SfxAcceleratorConfigPage>   m_pAccelConfigPage;

    void                        KeyInput( const KeyEvent &rKEvt ) override;

protected:
    virtual void                InitEntry(SvTreeListEntry*, const OUString&, const Image&,
                                          const Image&, SvLBoxButtonKind eButtonKind) override;

public:
    SfxAccCfgTabListBox_Impl(vcl::Window *pParent, WinBits nStyle)
        : SvTabListBox(pParent, nStyle)
        , m_pAccelConfigPage(nullptr)
    {
    }
    virtual ~SfxAccCfgTabListBox_Impl();
    virtual void dispose() override;

    void SetAccelConfigPage(SfxAcceleratorConfigPage* pAccelConfigPage)
    {
        m_pAccelConfigPage = pAccelConfigPage;
    }
};

// class SfxAcceleratorConfigPage ----------------------------------------

struct TAccInfo
{
    public:
        TAccInfo(      sal_Int32 nKeyPos ,
                       sal_Int32 nListPos,
                 const vcl::KeyCode& aKey )
            : m_nKeyPos        (nKeyPos  )
            , m_nListPos       (nListPos )
            , m_bIsConfigurable(true ) /**< its important to set sal_True as default -
                                                because only fix entries will be disabled later ... */
            , m_sCommand       (         )
            , m_aKey           (aKey     )
        {}

        bool isConfigured() const
        {
            return (m_nKeyPos>-1 && m_nListPos>-1 && !m_sCommand.isEmpty());
        }

        sal_Int32 m_nKeyPos;
        sal_Int32 m_nListPos;
        bool m_bIsConfigurable;
        OUString m_sCommand;
        vcl::KeyCode m_aKey;
};

namespace sfx2
{
    class FileDialogHelper;
}

class SfxAcceleratorConfigPage : public SfxTabPage
{
    friend class SfxAccCfgTabListBox_Impl;
private:
    const SfxMacroInfoItem*         m_pMacroInfoItem;
    const SfxStringItem*            m_pStringItem;
    const SfxStringItem*            m_pFontItem;
    sfx2::FileDialogHelper*         m_pFileDlg;

    VclPtr<SfxAccCfgTabListBox_Impl>       m_pEntriesBox;
    VclPtr<RadioButton>                    m_pOfficeButton;
    VclPtr<RadioButton>                    m_pModuleButton;
    VclPtr<PushButton>                     m_pChangeButton;
    VclPtr<PushButton>                     m_pRemoveButton;
    VclPtr<SfxConfigGroupListBox>          m_pGroupLBox;
    VclPtr<SfxConfigFunctionListBox>       m_pFunctionBox;
    VclPtr<SvTreeListBox>                  m_pKeyBox;
    VclPtr<PushButton>                     m_pLoadButton;
    VclPtr<PushButton>                     m_pSaveButton;
    VclPtr<PushButton>                     m_pResetButton;
    OUString                        aLoadAccelConfigStr;
    OUString                        aSaveAccelConfigStr;
    OUString                        aFilterAllStr;
    OUString                        aFilterCfgStr;
    SfxStylesInfo_Impl              m_aStylesInfo;
    bool                        m_bStylesInfoInitialized;

    css::uno::Reference< css::uno::XComponentContext >     m_xContext;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xGlobal;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xModule;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xAct;
    css::uno::Reference< css::container::XNameAccess > m_xUICmdDescription;
    css::uno::Reference< css::frame::XFrame > m_xFrame;

    OUString m_sModuleLongName;
    OUString m_sModuleShortName;
    OUString m_sModuleUIName;

    DECL_LINK_TYPED(ChangeHdl, Button *, void);
    DECL_LINK_TYPED(RemoveHdl, Button *, void);
    DECL_LINK_TYPED(SelectHdl, SvTreeListBox*, void );
    DECL_LINK_TYPED(Save, Button *, void);
    DECL_LINK_TYPED(Load, Button *, void);
    DECL_LINK_TYPED(Default, Button *, void);
    DECL_LINK_TYPED(RadioHdl, Button *, void);

    DECL_LINK_TYPED(LoadHdl, sfx2::FileDialogHelper *, void);
    DECL_LINK_TYPED(SaveHdl, sfx2::FileDialogHelper *, void);

    OUString                    GetLabel4Command(const OUString& rCommand);
    void                        InitAccCfg();
    sal_uLong                   MapKeyCodeToPos( const vcl::KeyCode &rCode ) const;
    static css::uno::Reference< css::frame::XModel > SearchForAlreadyLoadedDoc(const OUString& sName);
    void                        StartFileDialog( WinBits nBits, const OUString& rTitle );

    void                        Init(const css::uno::Reference< css::ui::XAcceleratorConfiguration >& pAccMgr);
    void                        ResetConfig();

    static void                 CreateCustomItems( SvTreeListEntry* pEntry, const OUString& aCol1, const OUString& aCol2 );

public:
                                SfxAcceleratorConfigPage( vcl::Window *pParent, const SfxItemSet& rItemSet );
    virtual                     ~SfxAcceleratorConfigPage();
    virtual void                dispose() override;

    virtual bool                FillItemSet( SfxItemSet* ) override;
    virtual void                Reset( const SfxItemSet* ) override;

    void                        Apply(const css::uno::Reference< css::ui::XAcceleratorConfiguration >& pAccMgr);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
