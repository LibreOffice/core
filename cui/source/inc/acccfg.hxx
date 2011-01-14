/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _ACCCFG_HXX
#define _ACCCFG_HXX

// #include *****************************************************************

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/frame/XFrame.hpp>
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
#include <svtools/svtreebx.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/basedlgs.hxx>

#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>

#include <sfx2/minarray.hxx>

#include "cfgutil.hxx"

// class SfxAcceleratorConfigListBox *************************************************

#define css ::com::sun::star

class SfxMacroInfoItem;
class SfxConfigGroupListBox_Impl;
class SfxConfigFunctionListBox_Impl;
class SfxAcceleratorConfigPage;

class SfxAccCfgTabListBox_Impl : public SvTabListBox
{
    SfxAcceleratorConfigPage*   m_pAccelConfigPage;

    void                        KeyInput( const KeyEvent &rKEvt );

protected:
    virtual void                InitEntry( SvLBoxEntry*, const XubString&, const Image&, const Image&, SvLBoxButtonKind eButtonKind );

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
            , m_bIsConfigurable(sal_True )
            , m_sCommand       (         )
            , m_aKey           (aKey     )
            // its important to set sal_True as default -
            // because only fix entries will be disabled later ...
        {}

        sal_Bool isConfigured() const
        {
            return (m_nKeyPos>-1 && m_nListPos>-1 && m_sCommand.getLength());
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

    DECL_LINK(                  ChangeHdl, Button * );
    DECL_LINK(                  RemoveHdl, Button * );
    DECL_LINK(                  SelectHdl, Control * );
    DECL_LINK(                  Save, Button * );
    DECL_LINK(                  Load, Button * );
    DECL_LINK(                  Default, PushButton * );
    DECL_LINK(                  RadioHdl, RadioButton* );

    DECL_LINK(                  LoadHdl, sfx2::FileDialogHelper* );
    DECL_LINK(                  SaveHdl, sfx2::FileDialogHelper* );

    String                      GetLabel4Command(const String& sCommand);
    void                        InitAccCfg();
    sal_uInt16                      MapKeyCodeToPos( const KeyCode &rCode ) const;
    css::uno::Reference< css::frame::XModel > SearchForAlreadyLoadedDoc(const String& sName);
    void                        StartFileDialog( WinBits nBits, const String& rTitle );

    void                        Init(const css::uno::Reference< css::ui::XAcceleratorConfiguration >& pAccMgr);
    void                        ResetConfig();

    void                        CreateCustomItems( SvLBoxEntry* pEntry, const String& aCol1, const String& aCol2 );

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



#undef css

#endif

