/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_OPTINET_HXX
#define _SVX_OPTINET_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <vcl/field.hxx>
#include <svl/svarray.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/svtabbx.hxx>
#include <sfx2/tabdlg.hxx>
#include <svl/srchcfg.hxx>

#ifdef _SVX_OPTINET2_CXX
#include <svtools/headbar.hxx>
#else
class HeaderBar;
#endif
#include <readonlyimage.hxx>

class SfxFilter;
class SvtInetOptions;

#ifndef SV_NODIALOG
#define PROXY_CONTROLS  23
#define CACHE_CONTROLS  20
#define INET_SEARCH     19

#if defined(OS2)
#define TYPE_CONTROLS  20
#else
#define TYPE_CONTROLS  18
#endif

namespace svx {
    class SecurityOptionsDialog;
}

namespace lang = ::com::sun::star::lang;
namespace uno = ::com::sun::star::uno;

// class SvxNoSpaceEdit --------------------------------------------------

class SvxNoSpaceEdit : public Edit
{
private:
    sal_Bool            bOnlyNumeric;

public:
    SvxNoSpaceEdit(Window* pParent, ResId rResId, sal_Bool bNum = sal_False ) :
        Edit( pParent, rResId ), bOnlyNumeric( bNum ) {}

    virtual void    KeyInput( const KeyEvent& rKEvent );
    virtual void    Modify();
};

typedef SfxFilter* SfxFilterPtr;
SV_DECL_PTRARR( SfxFilterPtrArr, SfxFilterPtr, 0, 4 )

// class SvxProxyTabPage -------------------------------------------------

class SvxProxyTabPage : public SfxTabPage
{
private:
    FixedLine       aOptionGB;

    FixedText       aProxyModeFT;
    ListBox         aProxyModeLB;

    FixedText       aHttpProxyFT;
    SvxNoSpaceEdit  aHttpProxyED;
    FixedText       aHttpPortFT;
    SvxNoSpaceEdit  aHttpPortED;

    FixedText       aHttpsProxyFT;
    SvxNoSpaceEdit  aHttpsProxyED;
    FixedText       aHttpsPortFT;
    SvxNoSpaceEdit  aHttpsPortED;


    FixedText       aFtpProxyFT;
    SvxNoSpaceEdit  aFtpProxyED;
    FixedText       aFtpPortFT;
    SvxNoSpaceEdit  aFtpPortED;

    FixedText       aNoProxyForFT;
    Edit            aNoProxyForED;
    FixedText       aNoProxyDescFT;

    String          sFromBrowser;

    const rtl::OUString aProxyModePN;
    const rtl::OUString aHttpProxyPN;
    const rtl::OUString aHttpPortPN;
    const rtl::OUString aHttpsProxyPN;
    const rtl::OUString aHttpsPortPN;
    const rtl::OUString aFtpProxyPN;
    const rtl::OUString aFtpPortPN;
    const rtl::OUString aNoProxyDescPN;

    uno::Reference< uno::XInterface > m_xConfigurationUpdateAccess;

#ifdef _SVX_OPTINET2_CXX
    void ArrangeControls_Impl();
    void EnableControls_Impl(sal_Bool bEnable);
    void ReadConfigData_Impl();
    void ReadConfigDefaults_Impl();
    void RestoreConfigDefaults_Impl();

    DECL_LINK( ProxyHdl_Impl, ListBox * );
    DECL_LINK( LoseFocusHdl_Impl, Edit * );
#endif

    SvxProxyTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxProxyTabPage();

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

// class SvxSearchTabPage ------------------------------------------------
class SvxSearchConfig;
class SvxSearchTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    FixedLine       aSearchGB;
    ListBox         aSearchLB;
    FixedText       aSearchNameFT;
    SvxNoSpaceEdit  aSearchNameED;

    FixedText       aSearchFT;
    RadioButton     aAndRB;
    RadioButton     aOrRB;
    RadioButton     aExactRB;

    FixedText       aURLFT;
    SvxNoSpaceEdit  aURLED;

    FixedText       aPostFixFT;
    SvxNoSpaceEdit  aPostFixED;
    FixedText       aSeparatorFT;
    SvxNoSpaceEdit  aSeparatorED;
    FixedText       aCaseFT;
    ListBox         aCaseED;

    PushButton      aNewPB;
    PushButton      aAddPB;
    PushButton      aChangePB;
    PushButton      aDeletePB;

    String          sLastSelectedEntry;
    String          sModifyMsg;

    SvxSearchConfig     aSearchConfig;
    SvxSearchEngineData aCurrentSrchData;

#ifdef _SVX_OPTINET2_CXX
    void        InitControls_Impl();

    DECL_LINK(  NewSearchHdl_Impl, PushButton * );
    DECL_LINK(  AddSearchHdl_Impl, PushButton * );
    DECL_LINK(  ChangeSearchHdl_Impl, PushButton * );
    DECL_LINK(  DeleteSearchHdl_Impl, PushButton * );
    DECL_LINK(  SearchEntryHdl_Impl, ListBox * );
    DECL_LINK(  SearchModifyHdl_Impl, SvxNoSpaceEdit * );
    DECL_LINK(  SearchPartHdl_Impl, RadioButton * );
#endif

    virtual void        ActivatePage( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );
    sal_Bool                ConfirmLeave( const String& rStringSelection );   //add by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)

    SvxSearchTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxSearchTabPage();

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

// #98647# class SvxScriptExecListBox ------------------------------------
class SvxScriptExecListBox : public ListBox
{ // for adding tooltips to ListBox
public:
    SvxScriptExecListBox( Window* pParent, WinBits nStyle = WB_BORDER )
            :ListBox(pParent, nStyle) {}
    SvxScriptExecListBox( Window* pParent, const ResId& rResId )
            :ListBox(pParent, rResId) {}

protected:
    virtual void RequestHelp( const HelpEvent& rHEvt );
};

// class SvxSecurityTabPage ---------------------------------------------

class SvtJavaOptions;
class SvtSecurityOptions;

class SvxSecurityTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    FixedLine           maSecurityOptionsFL;
    FixedInfo           maSecurityOptionsFI;
    PushButton          maSecurityOptionsPB;

    FixedLine           maPasswordsFL;
    CheckBox            maSavePasswordsCB;
    PushButton          maShowConnectionsPB;
    CheckBox            maMasterPasswordCB;
    FixedInfo           maMasterPasswordFI;
    PushButton          maMasterPasswordPB;

    FixedLine           maMacroSecFL;
    FixedInfo           maMacroSecFI;
    PushButton          maMacroSecPB;


    SvtSecurityOptions*         mpSecOptions;
    svx::SecurityOptionsDialog* mpSecOptDlg;

    String              msPasswordStoringDeactivateStr;

    DECL_LINK(          SecurityOptionsHdl, PushButton* );
    DECL_LINK(          SavePasswordHdl, void* );
    DECL_LINK(          MasterPasswordHdl, PushButton* );
    DECL_LINK(          MasterPasswordCBHdl, void* );
    DECL_LINK(          ShowPasswordsHdl, PushButton* );
    DECL_LINK(          MacroSecPBHdl, void* );

    void                InitControls();

                SvxSecurityTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual     ~SvxSecurityTabPage();

protected:
    virtual void        ActivatePage( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif

/* -----------------------------20.06.01 16:32--------------------------------

 ---------------------------------------------------------------------------*/
#ifdef WNT
#else
#define HELPER_PAGE_COMPLETE
#endif

struct SvxEMailTabPage_Impl;
class SvxEMailTabPage : public SfxTabPage
{
    FixedLine       aMailFL;
    ReadOnlyImage   aMailerURLFI;
    FixedText       aMailerURLFT;
    Edit            aMailerURLED;
    PushButton      aMailerURLPB;

    String          m_sDefaultFilterName;

    SvxEMailTabPage_Impl* pImpl;

    DECL_LINK(  FileDialogHdl_Impl, PushButton* ) ;

public:
    SvxEMailTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxEMailTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif // #ifndef _SVX_OPTINET_HXX


