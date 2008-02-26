/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cfg.hxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 15:11:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFXCFG_HXX
#define _SFXCFG_HXX

#include <vector>
#include <algorithm>

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SVTABBX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

//
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODULEMANAGER_HPP_
#include <com/sun/star/frame/XModuleManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONMANAGER_HPP_
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XACCELERATORCONFIGURATION_HPP_
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLECOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#endif
#include <com/sun/star/script/browse/XBrowseNode.hpp>

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#define _SVSTDARR_USHORTS
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>     // SvUShorts
#include <sfx2/minarray.hxx>
#include <sfx2/tabdlg.hxx>

class SfxMacroInfoArr_Impl;
class SfxMacroInfoItem;
class SfxMacroInfo;
class BasicManager;

#define SFX_CFGGROUP_FUNCTION 1
#define SFX_CFGGROUP_BASICMGR 2
#define SFX_CFGGROUP_DOCBASICMGR 3
#define SFX_CFGGROUP_BASICLIB 4
#define SFX_CFGGROUP_BASICMOD 5
#define SFX_CFGFUNCTION_MACRO 6
#define SFX_CFGFUNCTION_SLOT  7
#define SFX_CFGGROUP_SCRIPTCONTAINER  8
#define SFX_CFGFUNCTION_SCRIPT 9
#define SFX_CFGGROUP_STYLES  10

#define css ::com::sun::star

struct SfxStyleInfo_Impl
{
    public:

    ::rtl::OUString sFamily;
    ::rtl::OUString sStyle;
    ::rtl::OUString sCommand;
    ::rtl::OUString sLabel;

    SfxStyleInfo_Impl()
    {}

    SfxStyleInfo_Impl(const SfxStyleInfo_Impl& rCopy)
    {
        sFamily  = rCopy.sFamily;
        sStyle   = rCopy.sStyle;
        sCommand = rCopy.sCommand;
        sLabel   = rCopy.sLabel;
    }
};

struct SfxStylesInfo_Impl
{
    private:

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > m_xDoc;

    public:

        SfxStylesInfo_Impl();
        void setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel);

        sal_Bool parseStyleCommand(SfxStyleInfo_Impl& aStyle);
        void getLabel4Style(SfxStyleInfo_Impl& aStyle);

        ::std::vector< SfxStyleInfo_Impl > getStyleFamilies();
        ::std::vector< SfxStyleInfo_Impl > getStyles(const ::rtl::OUString& sFamily);

        static ::rtl::OUString generateCommand(const ::rtl::OUString& sFamily, const ::rtl::OUString& sStyle);
};

struct SfxGroupInfo_Impl
{
    USHORT      nKind;
    USHORT      nOrd;
    void*       pObject;
    BOOL        bWasOpened;
    String      sCommand;
    String      sLabel;

                SfxGroupInfo_Impl( USHORT n, USHORT nr, void* pObj = 0 ) :
                    nKind( n ), nOrd( nr ), pObject( pObj ), bWasOpened(FALSE) {}
};

typedef SfxGroupInfo_Impl* SfxGroupInfoPtr;
SV_DECL_PTRARR_DEL(SfxGroupInfoArr_Impl, SfxGroupInfoPtr, 5, 5)

class SfxConfigFunctionListBox_Impl : public SvTreeListBox
{
    friend class SfxConfigGroupListBox_Impl;
    Timer                aTimer;
    SvLBoxEntry*         pCurEntry;
    SfxGroupInfoArr_Impl aArr;
    SfxStylesInfo_Impl*  pStylesInfo;

    DECL_LINK( TimerHdl, Timer* );
    virtual void  MouseMove( const MouseEvent& rMEvt );

public:
                  SfxConfigFunctionListBox_Impl( Window*, const ResId& );
                  ~SfxConfigFunctionListBox_Impl();

    void          ClearAll();
    SvLBoxEntry*  GetEntry_Impl( USHORT nId );
    SvLBoxEntry*  GetEntry_Impl( const String& );
    USHORT        GetId( SvLBoxEntry *pEntry );
    using Window::GetHelpText;
    String        GetHelpText( SvLBoxEntry *pEntry );
    USHORT        GetCurId() { return GetId( FirstSelected() ); }
    String        GetCurCommand();
    String        GetCurLabel();
    SfxMacroInfo* GetMacroInfo();
    void          FunctionSelected();
    void          SetStylesInfo(SfxStylesInfo_Impl* pStyles);
};

class SfxSlotPool;
class SfxConfigGroupListBox_Impl : public SvTreeListBox
{
    SfxSlotPool*                    pSlotPool;
    SfxConfigFunctionListBox_Impl*  pFunctionListBox;
    SfxGroupInfoArr_Impl            aArr;
    ULONG                           nMode;
    String                          aScriptType;
    BOOL                            bShowSF; // show Scripting Framework scripts
    BOOL                            bShowBasic; // show Basic scripts

    Image m_hdImage;
    Image m_hdImage_hc;
    Image m_libImage;
    Image m_libImage_hc;
    Image m_macImage;
    Image m_macImage_hc;
    Image m_docImage;
    Image m_docImage_hc;

    ::rtl::OUString m_sMyMacros;
    ::rtl::OUString m_sProdMacros;

    ::rtl::OUString m_sModuleLongName;
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;
    css::uno::Reference< css::frame::XFrame > m_xFrame;
    css::uno::Reference< css::container::XNameAccess > m_xGlobalCategoryInfo;
    css::uno::Reference< css::container::XNameAccess > m_xModuleCategoryInfo;
    css::uno::Reference< css::container::XNameAccess > m_xUICmdDescription;

    Image GetImage( ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode > node, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xCtx, bool bIsRootNode, bool bHighContrast );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface  > getDocumentModel( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xCtx, ::rtl::OUString& docName );
    ::rtl::OUString parseLocationName( const ::rtl::OUString& location );

    void InitModule();
    void InitBasic();
    void InitStyles();

    ::rtl::OUString MapCommand2UIName(const ::rtl::OUString& sCommand);

    SfxStylesInfo_Impl* pStylesInfo;

protected:
    virtual void        RequestingChilds( SvLBoxEntry *pEntry);
    using SvListView::Expand;
    virtual BOOL        Expand( SvLBoxEntry* pParent );

public:
    SfxConfigGroupListBox_Impl ( Window* pParent,
                                 const ResId&,
                                 ULONG nConfigMode = 0 );
    ~SfxConfigGroupListBox_Impl();
    void                ClearAll();

    void                Init(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR          ,
                             const css::uno::Reference< css::frame::XFrame >&              xFrame         ,
                             const ::rtl::OUString&                                        sModuleLongName);
    void                SetFunctionListBox( SfxConfigFunctionListBox_Impl *pBox )
                        { pFunctionListBox = pBox; }
    void                Open( SvLBoxEntry*, BOOL );
    void                GroupSelected();
    void                SelectMacro( const SfxMacroInfoItem* );
    void                SelectMacro( const String&, const String& );
    String              GetGroup();
    BasicManager*       GetBasicManager( const SvLBoxEntry& _rEntry );
    void                SetScriptType( const String& rScriptType );
    void                SetStylesInfo(SfxStylesInfo_Impl* pStyles);
};
/*
class SfxMenuConfigEntry
{
private:
    USHORT                      nId;
    BOOL                        bPopUp;
    String                      aHelpText;
    String                      aStr;
    String                      aCommand;
    String                      aHelpURL;
    BOOL                        bConfigure;
    BOOL                        bStrEdited;

public:
    SfxMenuConfigEntry( USHORT nInitId, const String& rInitStr,
                        const String& rHelpText, BOOL bPopup = FALSE );
    SfxMenuConfigEntry() : nId( 0 ), bPopUp( FALSE ), bConfigure( TRUE ), bStrEdited( FALSE ) {}
    ~SfxMenuConfigEntry();

    USHORT                      GetId() const { return nId; }
    void                        SetId( USHORT nNew );
    void                        SetCommand( const String& rCmd ) { aCommand = rCmd; }
    const String&               GetName() const { return aStr; }
    void                        SetName( const String& rStr ) { aStr = rStr; bStrEdited = TRUE; }
    const String&               GetHelpText() const { return aHelpText; }
    void                        SetHelpText( const String& rStr ) { aHelpText = rStr; }
    const String&               GetHelpURL() const { return aHelpURL; }
    void                        SetHelpURL( const String& rStr ) { aHelpURL = rStr; }
    void                        SetPopup( BOOL bOn = TRUE ) { bPopUp = bOn; }
    void                        SetConfigurable( BOOL bOn = TRUE ) { bConfigure = bOn; }
    BOOL                        IsBinding() const { return nId != 0 && !bPopUp; }
    BOOL                        IsSeparator() const { return nId == 0; }
    BOOL                        IsPopup() const { return bPopUp; }
    BOOL                        IsConfigurable() const { return bConfigure; }
    BOOL                        HasChangedName() const { return bStrEdited; }
    const String&               GetCommand() const { return aCommand; }
};

// class SfxMenuConfigEntryArr *************************************************

SV_DECL_PTRARR_DEL(SfxMenuConfigEntryArr, SfxMenuConfigEntry *, 16, 16)

class SfxMenuConfigPage;
class SfxMenuCfgTabListBox_Impl : public SvTabListBox
{
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > m_xComponentContext;
    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > m_aSeparatorSeq;
    SfxMenuConfigPage*          pPage;
    SfxMenuConfigEntryArr       aMenuArr;
    Timer                       aTimer;
    SfxMenuConfigEntry*         pCurEntry;
    rtl::OUString               m_aDescriptorCommandURL;
    rtl::OUString               m_aDescriptorType;
    rtl::OUString               m_aDescriptorLabel;
    rtl::OUString               m_aDescriptorHelpURL;
    rtl::OUString               m_aDescriptorContainer;

    DECL_LINK(                  TimerHdl, Timer* );

    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& GetSeparator();
    com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > ConvertSfxMenuConfigEntry( SfxMenuConfigEntry* pEntry );

public :
                                SfxMenuCfgTabListBox_Impl( Window *pParent, const ResId& rResId );
                                ~SfxMenuCfgTabListBox_Impl();

    virtual long                PreNotify( NotifyEvent& );
    virtual void                ModelIsRemoving( SvListEntry* pEntry );
    void                        EntryInserted( SvListEntry* pEntry );
    virtual void                ModelHasCleared();
    virtual void                EditingRequest( SvLBoxEntry* pEntry,
                                                SvLBoxItem* pItem, const Point& rMousePos );
    virtual BOOL                EditedEntry( SvLBoxEntry* pEntry, const String& rNewText );
    virtual void                MouseMove( const MouseEvent& rMEvt );
    virtual sal_Int8            AcceptDrop( const AcceptDropEvent& rEvt );
    virtual BOOL                NotifyMoving(SvLBoxEntry*, SvLBoxEntry*, SvLBoxEntry*&, ULONG& );
    void                        Apply( com::sun::star::uno::Reference< com::sun::star::container::XIndexContainer >& rNewMenuBar, com::sun::star::uno::Reference< com::sun::star::lang::XSingleComponentFactory >& rFactory, SvLBoxEntry *pParent = NULL );
};

// class SfxMenuConfigPage -----------------------------------------------

class SfxMenuConfigPage : public SfxTabPage
{
private:
    SfxMenuCfgTabListBox_Impl       aEntriesBox;
    const SfxMacroInfoItem*         m_pMacroInfoItem;
    FixedLine                       aMenuGroup;
    PushButton                      aNewButton;
    PushButton                      aNewPopupButton;
    PushButton                      aChangeButton;
    PushButton                      aRemoveButton;
    ImageButton                     aUpButton;
    ImageButton                     aDownButton;
    FixedText                       aGroupText;
    SfxConfigGroupListBox_Impl      aGroupLBox;
    FixedText                       aFunctionText;
    SfxConfigFunctionListBox_Impl   aFunctionBox;
    FixedLine                       aFunctionsGroup;
    PushButton                      aLoadButton;
    PushButton                      aSaveButton;
    PushButton                      aResetButton;

    ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager > m_xDocCfgMgr;
    ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIConfigurationManager > m_xModuleCfgMgr;
    ::com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;
    ::com::sun::star::uno::Reference< com::sun::star::frame::XFrame > m_xFrame;
    ::com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess > m_xMenuBarSettings;
    ::com::sun::star::uno::Reference< com::sun::star::container::XNameAccess > m_xCommandToLabelMap;
    String                          m_aMenuResourceURL;
    sal_Bool                        m_bDocConfig;
    sal_Bool                        m_bDocReadOnly;

//  SfxMenuManager*             pMgr;

    BOOL                        m_bModified;
    BOOL                        m_bDefault;

    DECL_LINK(                  MoveHdl, Button * );
    DECL_LINK(                  NewHdl, Button * );
    DECL_LINK(                  NewPopupHdl, Button * );
    DECL_LINK(                  ChangeHdl, Button * );
    DECL_LINK(                  RemoveHdl, Button * );
    DECL_LINK(                  SelectHdl, Control* );

    void                        CheckEntry( Control* );
    USHORT                      GetCurId();
    String                      MakeEntry( const SfxMenuConfigEntry &rEntry ) const;
    String                      Trim( const String &rStr ) const;

    sal_Bool                    GetMenuItemData( const ::com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess >& rItemContainer,
                                                 sal_Int32 nIndex,
                                                 rtl::OUString& rCommandURL,
                                                 rtl::OUString& rHelpURL,
                                                 rtl::OUString& rLabel,
                                                 sal_uInt16& rType,
                                                 ::com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess >& xPopupMenu );

    void                        FillEntriesBox( const ::com::sun::star::uno::Reference< com::sun::star::container::XIndexAccess >& xMenuBarSettings, SfxMenuCfgTabListBox_Impl& aEntriesBox, SvLBoxEntry *pParentEntry );
    void                        Init();
    void                        ResetConfig();

    BOOL                        TryMove_Impl( Button*, SvLBoxEntry** pEntry=NULL, ULONG* pPos=NULL );

    DECL_LINK(                  Default, PushButton * );
    DECL_LINK(                  Save, Button * );
    DECL_LINK(                  Load, Button * );

public:
                                SfxMenuConfigPage( Window *pParent, const SfxItemSet& rItemSet );
    virtual                     ~SfxMenuConfigPage();

    virtual BOOL                FillItemSet( SfxItemSet& );
    virtual void                Reset( const SfxItemSet& );
    void                        Apply( BOOL );
    void                        SetModified( BOOL bSet ) { m_bModified = bSet; }
    void                        SetDefault( BOOL bSet ) { m_bDefault = bSet; }

    void                        SelectMacro(const SfxMacroInfoItem*);

    ::com::sun::star::uno::Reference < com::sun::star::container::XNameAccess >
        GetCommandToLabelMap() { return m_xCommandToLabelMap; }
};


class SfxAcceleratorManager;
*/
// class SfxAcceleratorConfigListBox *************************************************

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

    void                        ReplaceEntry( USHORT nPos, const String &rStr );
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
            // its important to set TRUE as default -
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
    sfx2::FileDialogHelper*         m_pFileDlg;

    SfxAccCfgTabListBox_Impl        aEntriesBox;
    FixedLine                       aKeyboardGroup;
     RadioButton                        aOfficeButton;
    RadioButton                     aModuleButton;
    PushButton                      aChangeButton;
    PushButton                      aRemoveButton;
    FixedText                       aGroupText;
    SfxConfigGroupListBox_Impl      aGroupLBox;
    FixedText                       aFunctionText;
    SfxConfigFunctionListBox_Impl   aFunctionBox;
    FixedText                       aKeyText;
    SvTreeListBox                   aKeyBox;
    FixedLine                       aFunctionsGroup;
    PushButton                      aLoadButton;
    PushButton                      aSaveButton;
    PushButton                      aResetButton;
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
    KeyCode                     MapPosToKeyCode( USHORT nPos ) const;
    USHORT                      MapKeyCodeToPos( const KeyCode &rCode ) const;
    String                      GetFunctionName( KeyFuncType eType ) const;
    css::uno::Reference< css::frame::XModel > SearchForAlreadyLoadedDoc(const String& sName);
    void                        StartFileDialog( WinBits nBits, const String& rTitle );

    void                        Init(const css::uno::Reference< css::ui::XAcceleratorConfiguration >& pAccMgr);
    void                        ResetConfig();

    void                        CreateCustomItems( SvLBoxEntry* pEntry, const String& aCol1, const String& aCol2 );

public:
                                SfxAcceleratorConfigPage( Window *pParent, const SfxItemSet& rItemSet );
    virtual                     ~SfxAcceleratorConfigPage();

    virtual BOOL                FillItemSet( SfxItemSet& );
    virtual void                Reset( const SfxItemSet& );

    void                        SelectMacro(const SfxMacroInfoItem*);
    void                        Apply(const css::uno::Reference< css::ui::XAcceleratorConfiguration >& pAccMgr);
    void                        CopySource2Target(const css::uno::Reference< css::ui::XAcceleratorConfiguration >& xSourceAccMgr,
                                                  const css::uno::Reference< css::ui::XAcceleratorConfiguration >& xTargetAccMgr);
};
/*
class SfxConfigDialog : public SfxTabDialog
{
    const SfxMacroInfoItem*     pMacroInfo;
    SfxViewFrame*               pViewFrame;
    USHORT                      nObjectBar;

public:
                                SfxConfigDialog( Window*, const SfxItemSet*, SfxViewFrame* pFrame );
                                ~SfxConfigDialog();

    void                        ActivateToolBoxConfig( USHORT nId );
    void                        ActivateMacroConfig( const SfxMacroInfoItem* );
    void                        ActivateTabPage( USHORT );

    virtual void                PageCreated( USHORT nId, SfxTabPage &rPage );
    virtual short               Ok();

    static String               FileDialog_Impl( Window *pParent, WinBits nBits, const String& rTitle );
    static SfxConfigManager*    MakeCfgMgr_Impl( const String& rName, BOOL bWriting, BOOL& bCreated );
};

class SfxStatusBarManager;
class SfxStbInfoArr_Impl;

class SfxStatusBarConfigListBox : public SvTreeListBox
{
friend class SfxStatusBarConfigPage;
    SvLBoxButtonData*           pButton;
    BOOL                        bModified;
    BOOL                        bDefault;
    SvLBoxEntry*                pCurEntry;
    Timer                       aTimer;

    DECL_LINK(                  TimerHdl, Timer* );

public:

                                SfxStatusBarConfigListBox( Window* pParent,
                                        const ResId& );
                                ~SfxStatusBarConfigListBox();

    virtual BOOL                NotifyAcceptDrop( SvLBoxEntry* pEntry );
    virtual BOOL                NotifyMoving(SvLBoxEntry*,
                                    SvLBoxEntry*,SvLBoxEntry*&,ULONG&);
    virtual void                CheckButtonHdl();
    virtual void                MouseMove( const MouseEvent& rMEvt );
    virtual void                KeyInput( const KeyEvent& rKeyEvent );
};

// class SfxStatusBarConfigPage ------------------------------------------

class SfxStatusBarConfigPage : public SfxTabPage
{
private:
    SfxStatusBarConfigListBox   aEntriesBox;
    FixedLine                   aStatusBarGroup;
    PushButton                  aLoadButton;
    PushButton                  aSaveButton;
    PushButton                  aResetButton;

    SvUShorts                   aListOfIds;
    SfxStatusBarManager*        pMgr;
    SfxStbInfoArr_Impl*         pArr;
    BOOL                        bMgrCreated;

    DECL_LINK(                  SelectHdl, SvTreeListBox * );
    DECL_LINK(                  Save, Button * );
    DECL_LINK(                  Load, Button * );
    DECL_LINK(                  Default, PushButton * );

    void                        Init();
    void                        ResetConfig();

public:
                                SfxStatusBarConfigPage( Window *pParent, const SfxItemSet& rItemSet );
    virtual                     ~SfxStatusBarConfigPage();

    virtual BOOL                FillItemSet( SfxItemSet& );
    virtual void                Reset( const SfxItemSet& );
    void                        Apply( SfxStatusBarManager*, BOOL );
};
*/

#undef css
#undef css

#endif

