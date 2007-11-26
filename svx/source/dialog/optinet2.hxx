/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: optinet2.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 16:39:02 $
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
#ifndef _SVX_OPTINET_HXX
#define _SVX_OPTINET_HXX

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif
#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SVTABBX_HXX //autogen
#include <svtools/svtabbx.hxx>
#endif
#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SVX_SRCHNCFG_HXX
#include "srchcfg.hxx"
#endif

#ifdef _SVX_OPTINET2_CXX
#ifndef _HEADBAR_HXX //autogen
#include <svtools/headbar.hxx>
#endif
#else
class HeaderBar;
#endif

#ifndef _SVX_READONLYIMAGE_HXX
#include <readonlyimage.hxx>
#endif

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
    BOOL            bOnlyNumeric;

public:
    SvxNoSpaceEdit(Window* pParent, ResId rResId, BOOL bNum = FALSE ) :
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
    void EnableControls_Impl(BOOL bEnable);
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
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
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
    void            FillSearchBox_Impl();
    String          GetSearchString_Impl();

    DECL_LINK( NewSearchHdl_Impl, PushButton * );
    DECL_LINK( AddSearchHdl_Impl, PushButton * );
    DECL_LINK( ChangeSearchHdl_Impl, PushButton * );
    DECL_LINK( DeleteSearchHdl_Impl, PushButton * );
    DECL_LINK( SearchEntryHdl_Impl, ListBox * );
    DECL_LINK( SearchModifyHdl_Impl, SvxNoSpaceEdit * );
    DECL_LINK( SearchPartHdl_Impl, RadioButton * );
#endif

    virtual void        ActivatePage( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );
    BOOL                ConfirmLeave( const String& rStringSelection );   //add by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)

    SvxSearchTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual ~SvxSearchTabPage();

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
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
public:
    enum RedliningMode  { RL_NONE, RL_WRITER, RL_CALC };

private:
    FixedLine           maSecurityOptionsFL;
    FixedInfo           maSecurityOptionsFI;
    PushButton          maSecurityOptionsPB;

    FixedLine           maPasswordsFL;
    CheckBox            maSavePasswordsCB;
    PushButton          maMasterPasswordPB;
    FixedInfo           maMasterPasswordFI;
    PushButton          maShowPasswordsPB;

    FixedLine           maMacroSecFL;
    FixedInfo           maMacroSecFI;
    PushButton          maMacroSecPB;

    FixedLine           maFilesharingFL;
    CheckBox            maRecommReadOnlyCB;
    CheckBox            maRecordChangesCB;
    PushButton          maProtectRecordsPB;

    SvtSecurityOptions*         mpSecOptions;
    svx::SecurityOptionsDialog* mpSecOptDlg;

    RedliningMode       meRedlingMode;
    String              msProtectRecordsStr;
    String              msUnprotectRecordsStr;
    String              msPasswordStoringDeactivateStr;

    DECL_LINK(          SecurityOptionsHdl, PushButton* );
    DECL_LINK(          SavePasswordHdl, void* );
    DECL_LINK(          MasterPasswordHdl, PushButton* );
    DECL_LINK(          ShowPasswordsHdl, PushButton* );
    DECL_LINK(          MacroSecPBHdl, void* );
    DECL_LINK(          RecordChangesCBHdl, void* );
    DECL_LINK(          ProtectRecordsPBHdl, void* );

    void                CheckRecordChangesState( void );
    void                InitControls();

                SvxSecurityTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual     ~SvxSecurityTabPage();

protected:
    virtual void        ActivatePage( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

//added by jmeng begin
class MozPluginTabPage : public SfxTabPage
{
    FixedLine       aMSWordGB;
    CheckBox        aWBasicCodeCB;

    BOOL isInstalled(void);
    BOOL installPlugin(void);
    BOOL uninstallPlugin(void);

    MozPluginTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual ~MozPluginTabPage();

public:

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );

};
//added by jmeng end
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

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif // #ifndef _SVX_OPTINET_HXX


