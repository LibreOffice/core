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

#ifndef _ACCCFG_HXX
#define _ACCCFG_HXX

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
#include <sfx2/minarray.hxx>
#include "cfgutil.hxx"

namespace css = ::com::sun::star;

class SfxMacroInfoItem;
class SfxConfigGroupListBox_Impl;
class SfxConfigFunctionListBox_Impl;
class SfxAcceleratorConfigPage;
class SfxStringItem;

class SfxAccCfgTabListBox_Impl : public SvTabListBox
{
    SfxAcceleratorConfigPage*   m_pAccelConfigPage;

    void                        KeyInput( const KeyEvent &rKEvt );

protected:
    virtual void                InitEntry( SvTreeListEntry*, const XubString&, const Image&, const Image&, SvLBoxButtonKind eButtonKind );

public:
                                SfxAccCfgTabListBox_Impl(
                                        SfxAcceleratorConfigPage* pAccelConfigPage,
                                        Window *pParent,
                                        const ResId &rResId ) :
                                     SvTabListBox( pParent, rResId ),
                                    m_pAccelConfigPage( pAccelConfigPage )
                                {}

    void                        ReplaceEntry( sal_uInt16 nPos, const String &rStr );
};

// class SfxAcceleratorConfigPage ----------------------------------------

struct TAccInfo
{
    public:
        TAccInfo(      sal_Int32 nKeyPos ,
                       sal_Int32 nListPos,
                 const KeyCode&  aKey    )
            : m_nKeyPos        (nKeyPos  )
            , m_nListPos       (nListPos )
            , m_bIsConfigurable(sal_True ) /**< its important to set sal_True as default -
                                                because only fix entries will be disabled later ... */
            , m_sCommand       (         )
            , m_aKey           (aKey     )
        {}

        sal_Bool isConfigured() const
        {
            return (m_nKeyPos>-1 && m_nListPos>-1 && !m_sCommand.isEmpty());
        }

        sal_Int32 m_nKeyPos;
        sal_Int32 m_nListPos;
        sal_Bool m_bIsConfigurable;
        ::rtl::OUString m_sCommand;
        KeyCode m_aKey;
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

    SfxAccCfgTabListBox_Impl        aEntriesBox;
    FixedLine                       aKeyboardGroup;
     RadioButton                        aOfficeButton;
    RadioButton                     aModuleButton;
    PushButton                      aChangeButton;
    PushButton                      aRemoveButton;
    FixedText                       aGroupText;
    SfxConfigGroupListBox_Impl*     pGroupLBox;
    FixedText                       aFunctionText;
    SfxConfigFunctionListBox_Impl*  pFunctionBox;
    FixedText                       aKeyText;
    SvTreeListBox                   aKeyBox;
    FixedLine                       aFunctionsGroup;
    PushButton                      aLoadButton;
    PushButton                      aSaveButton;
    PushButton                      aResetButton;
    String              aLoadAccelConfigStr;
    String              aSaveAccelConfigStr;
    String              aFilterAllStr;
    String              aFilterCfgStr;
    SfxStylesInfo_Impl              m_aStylesInfo;
    sal_Bool                        m_bStylesInfoInitialized;

    css::uno::Reference< css::lang::XMultiServiceFactory >     m_xSMGR;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xGlobal;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xModule;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xAct;
    css::uno::Reference< css::container::XNameAccess > m_xUICmdDescription;
    css::uno::Reference< css::frame::XFrame > m_xFrame;

    ::rtl::OUString m_sModuleLongName;
    ::rtl::OUString m_sModuleShortName;
    ::rtl::OUString m_sModuleUIName;

    DECL_LINK(ChangeHdl, void *);
    DECL_LINK(RemoveHdl, void *);
    DECL_LINK(                  SelectHdl, Control * );
    DECL_LINK(Save, void *);
    DECL_LINK(Load, void *);
    DECL_LINK(Default, void *);
    DECL_LINK(RadioHdl, void *);

    DECL_LINK(LoadHdl, void *);
    DECL_LINK(SaveHdl, void *);

    String                      GetLabel4Command(const String& sCommand);
    void                        InitAccCfg();
    sal_uInt16                      MapKeyCodeToPos( const KeyCode &rCode ) const;
    css::uno::Reference< css::frame::XModel > SearchForAlreadyLoadedDoc(const String& sName);
    void                        StartFileDialog( WinBits nBits, const String& rTitle );

    void                        Init(const css::uno::Reference< css::ui::XAcceleratorConfiguration >& pAccMgr);
    void                        ResetConfig();

    void                        CreateCustomItems( SvTreeListEntry* pEntry, const String& aCol1, const String& aCol2 );

public:
                                SfxAcceleratorConfigPage( Window *pParent, const SfxItemSet& rItemSet );
    virtual                     ~SfxAcceleratorConfigPage();

    virtual sal_Bool                FillItemSet( SfxItemSet& );
    virtual void                Reset( const SfxItemSet& );

    void                        Apply(const css::uno::Reference< css::ui::XAcceleratorConfiguration >& pAccMgr);
    static SfxTabPage*          Create( Window* pParent, const SfxItemSet& rAttrSet );
};


class SfxAcceleratorConfigListBox : public ListBox
{
    void KeyInput( const KeyEvent &rKEvt );

public:

    SfxAcceleratorConfigListBox( Window *pParent, ResId &rResId ) :
        ListBox( pParent, rResId ) {}

    void ReplaceEntry( sal_uInt16 nPos, const String &rStr );
    void ExpandEntry ( sal_uInt16 nPos, const String &rStr );
};

class SvxShortcutAssignDlg : public SfxSingleTabDialog
{
public:
    SvxShortcutAssignDlg(
        Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxDocumentFrame,
        const SfxItemSet& rSet );
    virtual ~SvxShortcutAssignDlg();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
