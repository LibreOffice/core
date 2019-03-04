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
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <algorithm>

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/svtabbx.hxx>
#include <vcl/treelistbox.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/basedlgs.hxx>
#include <i18nutil/searchopt.hxx>
#include "cfgutil.hxx"

class SfxMacroInfoItem;
class CuiConfigFunctionListBox;
class SfxAcceleratorConfigPage;
class SfxStringItem;

class SfxAccCfgTabListBox_Impl : public SvTabListBox
{
    void                        KeyInput( const KeyEvent &rKEvt ) override;

public:
    SfxAccCfgTabListBox_Impl(vcl::Window *pParent, WinBits nStyle)
        : SvTabListBox(pParent, nStyle)
    {
    }
    virtual ~SfxAccCfgTabListBox_Impl() override;
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
            , m_bIsConfigurable(true ) /**< it's important to set true as default -
                                                because only fix entries will be disabled later... */
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

enum class StartFileDialogType { Open, SaveAs };

class SfxAcceleratorConfigPage : public SfxTabPage
{
    friend class SfxAccCfgTabListBox_Impl;
private:
    const SfxMacroInfoItem*         m_pMacroInfoItem;
    std::unique_ptr<sfx2::FileDialogHelper> m_pFileDlg;

    OUString                            aLoadAccelConfigStr;
    OUString                            aSaveAccelConfigStr;
    OUString                            aFilterAllStr;
    OUString                            aFilterCfgStr;
    SfxStylesInfo_Impl                  m_aStylesInfo;
    bool                                m_bStylesInfoInitialized;

    css::uno::Reference< css::uno::XComponentContext >          m_xContext;
    css::uno::Reference< css::ui::XAcceleratorConfiguration >   m_xGlobal;
    css::uno::Reference< css::ui::XAcceleratorConfiguration >   m_xModule;
    css::uno::Reference< css::ui::XAcceleratorConfiguration >   m_xAct;
    css::uno::Reference< css::container::XNameAccess >          m_xUICmdDescription;
    css::uno::Reference< css::frame::XFrame >                   m_xFrame;

    OUString m_sModuleLongName;
    OUString m_sModuleUIName;

    // For search
    Timer m_aUpdateDataTimer;
    i18nutil::SearchOptions2 m_options;

    Idle m_aFillGroupIdle;

    std::unique_ptr<weld::TreeView>     m_xEntriesBox;
    std::unique_ptr<weld::RadioButton>  m_xOfficeButton;
    std::unique_ptr<weld::RadioButton>  m_xModuleButton;
    std::unique_ptr<weld::Button>       m_xChangeButton;
    std::unique_ptr<weld::Button>       m_xRemoveButton;
    std::unique_ptr<CuiConfigGroupListBox> m_xGroupLBox;
    std::unique_ptr<CuiConfigFunctionListBox> m_xFunctionBox;
    std::unique_ptr<weld::TreeView>     m_xKeyBox;
    std::unique_ptr<weld::Entry>        m_xSearchEdit;
    std::unique_ptr<weld::Button>       m_xLoadButton;
    std::unique_ptr<weld::Button>       m_xSaveButton;
    std::unique_ptr<weld::Button>       m_xResetButton;

    DECL_LINK(ChangeHdl,           weld::Button&,   void);
    DECL_LINK(RemoveHdl,           weld::Button&,   void);
    DECL_LINK(SelectHdl,           weld::TreeView&, void);
    DECL_LINK(SearchUpdateHdl,     weld::Entry&,    void);
    DECL_LINK(Save,                weld::Button&,   void);
    DECL_LINK(Load,                weld::Button&,   void);
    DECL_LINK(Default,             weld::Button&,   void);
    DECL_LINK(RadioHdl,            weld::Button&,   void);
    DECL_LINK(ImplUpdateDataHdl,   Timer*,          void);
    DECL_LINK(FocusOut_Impl,       weld::Widget&,   void);
    DECL_LINK(TimeOut_Impl,        Timer*,          void);

    DECL_LINK(LoadHdl, sfx2::FileDialogHelper *, void);
    DECL_LINK(SaveHdl, sfx2::FileDialogHelper *, void);

    OUString                    GetLabel4Command(const OUString& rCommand);
    int                         applySearchFilter(OUString const & rSearchTerm);
    void                        InitAccCfg();
    sal_Int32                   MapKeyCodeToPos(const vcl::KeyCode &rCode) const;
    void                        StartFileDialog( StartFileDialogType nType, const OUString& rTitle );

    void                        Init(const css::uno::Reference< css::ui::XAcceleratorConfiguration >& pAccMgr);
    void                        ResetConfig();

public:
                                SfxAcceleratorConfigPage(TabPageParent pParent, const SfxItemSet& rItemSet);
    virtual                     ~SfxAcceleratorConfigPage() override;
    virtual void                dispose() override;

    virtual bool                FillItemSet( SfxItemSet* ) override;
    virtual void                Reset( const SfxItemSet* ) override;

    void                        Apply(const css::uno::Reference< css::ui::XAcceleratorConfiguration >& pAccMgr);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
