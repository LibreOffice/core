/*************************************************************************
 *
 *  $RCSfile: optinet2.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 15:52:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVX_OPTINET_HXX
#define _SVX_OPTINET_HXX

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

#if defined(OS2) || defined(MAC)
#define TYPE_CONTROLS  20
#else
#define TYPE_CONTROLS  18
#endif


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

    FixedText       aFtpProxyFT;
    SvxNoSpaceEdit  aFtpProxyED;
    FixedText       aFtpPortFT;
    SvxNoSpaceEdit  aFtpPortED;

    FixedText       aNoProxyForFT;
    Edit            aNoProxyForED;
    FixedText       aNoProxyDescFT;

    String          sFromBrowser;

    SvtInetOptions* pInetOptions;
#ifdef _SVX_OPTINET2_CXX
    void            EnableControls_Impl(BOOL bEnable);

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

// class SvxScriptingTabPage ---------------------------------------------

class SvtJavaOptions;
class SvtSecurityOptions;

class SvxScriptingTabPage : public SfxTabPage
{
private:
    // readonly ------------------------------------
    sal_Bool bROConfirm;
    sal_Bool bROWarning;
    sal_Bool bROScriptExec;
    sal_Bool bROExePlug;
    sal_Bool bROExecMacro;

    sal_Bool bROJavaEnabled;
    sal_Bool bROJavaSecurity;
    sal_Bool bROJavaNetAccess;
    sal_Bool bROJavaUserClassPath;
    sal_Bool bROJavaExecuteApplets;

    // Execute

    FixedLine           aGrpScriptingStarBasic;

    ReadOnlyImage       aExecMacroFI;
    FixedText           aExecMacroFT;
    ListBox             aExecMacroLB;

    ReadOnlyImage       aConfirmFI;
    CheckBox            aConfirmCB;

    ReadOnlyImage       aWarningFI;
    CheckBox            aWarningCB;

    ReadOnlyImage       aScriptExecFI;
    FixedText           aPathListFT;
    // #98647# ------------------------------------
    SvxScriptExecListBox aLbScriptExec;
    PushButton          aBtnScriptExecDelete;
    PushButton          aBtnScriptExecDefault;

    FixedText           aNewPathFT;
    Edit                aEdtScriptExec;
    PushButton          aBtnScriptExecInsert;

    FixedLine           aHyperlinksFL;
    ReadOnlyImage       aHyperlinksFI;
    FixedText           aHyperlinksFT;
    ListBox             aHyperlinksLB;

    FixedLine           aJavaFL;
    ReadOnlyImage       aJavaEnableFI;
    CheckBox            aJavaEnableCB;
    ReadOnlyImage       aJavaSecurityFI;
    CheckBox            aJavaSecurityCB;
    ReadOnlyImage       aNetAccessFI;
    FixedText           aNetAccessFT;
    ListBox             aNetAccessLB;
    FixedText           aClassPathFT;
    ReadOnlyImage       aClassPathFI;
    Edit                aClassPathED;
    PushButton          aClassPathPB;

    FixedLine           aSeparatorFL;

    FixedLine           aExecuteGB;
    ReadOnlyImage       aExePlugFI;
    CheckBox            aExePlugCB;
    ReadOnlyImage       aExecAppletsFI;
    CheckBox            aExecAppletsCB;

    Image               aLockImg;
    Image               aLockHCImg;

    SvtJavaOptions*     pJavaOptions;
    SvtSecurityOptions* pSecurityOptions;
#ifdef _SVX_OPTINET2_CXX
    DECL_LINK( EditHdl_Impl, Edit* );
    DECL_LINK( LBHdl_Impl, ListBox* );
    DECL_LINK( BtnHdl_Impl, PushButton* );
    DECL_LINK( RunHdl_Impl, ListBox* );
    DECL_LINK( JavaEnableHdl_Impl, CheckBox* );
    DECL_LINK( ClassPathHdl_Impl, PushButton* );

    void                FillListBox_Impl();
    void                EnableJava_Impl( BOOL bEnable, BOOL bOnlySecurity );
#endif
                SvxScriptingTabPage( Window* pParent, const SfxItemSet& rSet );
    virtual     ~SvxScriptingTabPage();

protected:
    virtual void        ActivatePage( const SfxItemSet& rSet );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
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

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif // #ifndef _SVX_OPTINET_HXX


