/*************************************************************************
 *
 *  $RCSfile: optinet2.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 15:52:32 $
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

#pragma hdrstop

// include ---------------------------------------------------------------

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSLSTITM_HXX //autogen
#include <svtools/slstitm.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX //autogen
#include <sfx2/docfilt.hxx>
#endif
#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _TOOLS_INETDEF_HXX
#include <tools/inetdef.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#ifndef _CNTWIDS_HRC
#include <svtools/cntwids.hrc>
#endif
#ifndef _SVTOOLS_INETOPTIONS_HXX_
#include <svtools/inetoptions.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif

#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SECURITIYOPTIONS_HXX
#include <svtools/securityoptions.hxx>
#endif
#ifndef _SVTOOLS_JAVAPTIONS_HXX
#include <svtools/javaoptions.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_EXTENDEDSECURITYOPTIONS_HXX
#include <svtools/extendedsecurityoptions.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#define _SVX_OPTINET2_CXX

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif
#ifndef _SVX_OPTINET_HXX
#include "optinet2.hxx"
#endif
//CHINA001 #ifndef _SVX_MULTIFIL_HXX
//CHINA001 #include "multifil.hxx"
//CHINA001 #endif
#include "svxdlg.hxx" //CHINA001
#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif
#ifndef _SVX_OPTINET_HRC
#include "optinet2.hrc"
#endif
#ifndef _SVX_HELPID_HRC
#include "helpid.hrc"
#endif

using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::sfx2;

//CHINA001 copy from multipat.hxx begin
// define ----------------------------------------------------------------

// different delimiter for Unix (:) and Windows (;)

#ifdef UNX
#define CLASSPATH_DELIMITER ':'
#else
#define CLASSPATH_DELIMITER ';'
#endif
//CHINA001 copy from multipat.hxx end
// static ----------------------------------------------------------------

#define C2U(cChar) OUString::createFromAscii(cChar)

#define INET_SEARCH_TOKEN   '"'
#define RET_ALL             ((short)200)
#define CFG_READONLY_DEFAULT    FALSE

SV_IMPL_PTRARR( SfxFilterPtrArr, SfxFilterPtr )

const char* SEARCHENGINE_INI    = "srcheng.ini";
const char* SEARCHENGINE_GROUP  = "SearchEngines-$(vlang)";

// -----------------------------------------------------------------------

String lcl_MakeTabEntry(const SfxFilter* pFilter)
{
    String sEntry(pFilter->GetMimeType());
    sEntry += '\t';
    sEntry += pFilter->GetWildcard().GetWildCard();
    sEntry += '\t';
    sEntry += pFilter->GetName();
#if defined(OS2) || defined(MAC)
    sEntry += '\t';
    sEntry += pFilter->GetTypeName();
#endif
    return sEntry;
}

// -----------------------------------------------------------------------

BOOL IsJavaInstalled_Impl( /*!!!SfxIniManager* pIniMgr*/ )
{
    BOOL bRet = FALSE;
/*!!! (pb) needs new implementation
    String aIniEntry;
    String aFullName = Config::GetConfigName( pIniMgr->Get( SFX_KEY_USERCONFIG_PATH ),
                                              String::CreateFromAscii("java") );
    INetURLObject aIniFileObj( aFullName, INET_PROT_FILE );
    String aIniPath = aIniFileObj.getName();
    if ( pIniMgr->SearchFile( aIniPath ) )
    {
        Config aJavaCfg( aIniPath );
        aJavaCfg.SetGroup( "Java" );
        ByteString sTemp = aJavaCfg.ReadKey( ByteString(::rtl::OUStringToOString(pIniMgr->GetKeyName( SFX_KEY_JAVA_SYSTEMCLASSPATH ),RTL_TEXTENCODING_UTF8)) );
        String aJavaSystemClassPath = ::rtl::OStringToOUString(sTemp,RTL_TEXTENCODING_UTF8);
        String aJavaRuntimeLib = ::rtl::OStringToOUString(aJavaCfg.ReadKey( "RuntimeLib" ),RTL_TEXTENCODING_UTF8);
        if ( aJavaSystemClassPath.Len() && aJavaRuntimeLib.Len() )
            bRet = TRUE;
    }
*/
    return bRet;
}

// -----------------------------------------------------------------------

void SvxNoSpaceEdit::KeyInput( const KeyEvent& rKEvent )
{
    if ( bOnlyNumeric )
    {
        const KeyCode& rKeyCode = rKEvent.GetKeyCode();
        USHORT nGroup = rKeyCode.GetGroup();
        USHORT nKey = rKeyCode.GetCode();
        BOOL bValid = ( KEYGROUP_NUM == nGroup || KEYGROUP_CURSOR == nGroup ||
                        ( KEYGROUP_MISC == nGroup && ( nKey < KEY_ADD || nKey > KEY_EQUAL ) ) );
        if ( !bValid && ( rKeyCode.IsMod1() && (
             KEY_A == nKey || KEY_C == nKey || KEY_V == nKey || KEY_X == nKey || KEY_Z == nKey ) ) )
            // Erase, Copy, Paste, Select All und Undo soll funktionieren
            bValid = TRUE;

        if ( bValid )
            Edit::KeyInput(rKEvent);
    }
    else if( rKEvent.GetKeyCode().GetCode() != KEY_SPACE )
        Edit::KeyInput(rKEvent);
}

// -----------------------------------------------------------------------

void SvxNoSpaceEdit::Modify()
{
    Edit::Modify();

    if ( bOnlyNumeric )
    {
        XubString aValue = GetText();

        if ( !ByteString(::rtl::OUStringToOString(aValue,RTL_TEXTENCODING_UTF8)).IsNumericAscii() || (long)aValue.ToInt32() > USHRT_MAX )
            // der Höchstwert einer Portnummer ist USHRT_MAX
            ErrorBox( this, SVX_RES( RID_SVXERR_OPT_PROXYPORTS ) ).Execute();
    }
}

/********************************************************************/
/********************************************************************/
/*                                                                  */
/*  SvxProxyTabPage                                                 */
/*                                                                  */
/*                                                                  */
/********************************************************************/
/********************************************************************/

SvxProxyTabPage::SvxProxyTabPage(Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage( pParent, ResId( RID_SVXPAGE_INET_PROXY, DIALOG_MGR() ), rSet ),
    aOptionGB   (this, ResId(GB_SETTINGS)),

    aProxyModeFT  (this, ResId(FT_PROXYMODE)),
    aProxyModeLB  (this, ResId(LB_PROXYMODE)),

    aHttpProxyFT      (this, ResId( FT_HTTP_PROXY     )),
    aHttpProxyED      (this, ResId( ED_HTTP_PROXY     )),
    aHttpPortFT       (this, ResId( FT_HTTP_PORT      )),
    aHttpPortED       (this, ResId( ED_HTTP_PORT      ), TRUE),

    aFtpProxyFT       (this, ResId( FT_FTP_PROXY      )),
    aFtpProxyED       (this, ResId( ED_FTP_PROXY      )),
    aFtpPortFT        (this, ResId( FT_FTP_PORT       )),
    aFtpPortED        (this, ResId( ED_FTP_PORT       ), TRUE),

    aNoProxyForFT     (this, ResId( FT_NOPROXYFOR     )),
    aNoProxyForED     (this, ResId( ED_NOPROXYFOR     )),
    aNoProxyDescFT    (this, ResId( ED_NOPROXYDESC    )),
    sFromBrowser        (       ResId( ST_PROXY_FROM_BROWSER ) ),
    pInetOptions( new SvtInetOptions)
{
    FreeResource();

    aHttpPortED.SetMaxTextLen(5);
    aFtpPortED.SetMaxTextLen(5);
    Link aLink = LINK( this, SvxProxyTabPage, LoseFocusHdl_Impl );
    aHttpPortED.SetLoseFocusHdl( aLink );
    aFtpPortED.SetLoseFocusHdl( aLink );

    aProxyModeLB.SetSelectHdl(LINK( this, SvxProxyTabPage, ProxyHdl_Impl ));

    if(SfxApplication::IsPlugin())
    {
        aProxyModeLB.InsertEntry(sFromBrowser);
        aFtpProxyFT.Show(FALSE);
        aFtpProxyED.Show(FALSE);
        aFtpPortFT.Show(FALSE);
        aFtpPortED.Show(FALSE);

        aNoProxyForFT.Show(FALSE);
        aNoProxyForED.Show(FALSE);
        aNoProxyDescFT.Show(FALSE);

    }
}

/*-----------------12.08.96 14.55-------------------

--------------------------------------------------*/

SvxProxyTabPage::~SvxProxyTabPage()
{
    delete pInetOptions;
}

/*-----------------12.08.96 14.55-------------------

--------------------------------------------------*/

SfxTabPage* SvxProxyTabPage::Create(Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxProxyTabPage(pParent, rAttrSet);
}

/*-----------------12.08.96 14.55-------------------

--------------------------------------------------*/

void SvxProxyTabPage::Reset(const SfxItemSet&)
{
    //none == 0, manual == 2 , automatic only in PluginMode
    USHORT nPos = 0;
    BOOL bEnableProxyControls = FALSE;
    switch(pInetOptions->GetProxyType())
    {
        case SvtInetOptions::NONE:      nPos = 0; break;
        case SvtInetOptions::MANUAL:
            nPos = 1;
            bEnableProxyControls = TRUE;
        break;
        case SvtInetOptions::AUTOMATIC:
            nPos = aProxyModeLB.GetEntryCount() == 3 ? 2 : 0;
        break;
    }
    aProxyModeLB.SelectEntryPos( nPos );
    aProxyModeLB.SaveValue();
    EnableControls_Impl( bEnableProxyControls );

    aHttpProxyED.SetText( pInetOptions->GetProxyHttpName() );
    aHttpProxyED.SaveValue();
    aHttpPortED.SetText(
        String::CreateFromInt32( pInetOptions->GetProxyHttpPort() ));
    aHttpPortED.SaveValue();

    aFtpProxyED.SetText( pInetOptions->GetProxyFtpName() );
    aFtpProxyED.SaveValue();
    aFtpPortED.SetText(
        String::CreateFromInt32( pInetOptions->GetProxyFtpPort() ));
    aFtpPortED.SaveValue();

    aNoProxyForED.SetText( pInetOptions->GetProxyNoProxy() );
    aNoProxyForED.SaveValue();
}

/*-----------------12.08.96 16.34-------------------

--------------------------------------------------*/

BOOL SvxProxyTabPage::FillItemSet(SfxItemSet& rSet)
{
    BOOL bModified=FALSE;
    USHORT nSelPos = aProxyModeLB.GetSelectEntryPos();
    if(aProxyModeLB.GetSavedValue() != nSelPos)
    {
        SvtInetOptions::ProxyType eSet = SvtInetOptions::NONE;
        switch(nSelPos)
        {
//            case  0: eSet  = SvtInetOptions::NONE;     break;
            case  1: eSet  = SvtInetOptions::MANUAL;   break;
            case  2: eSet  = SvtInetOptions::AUTOMATIC;break;
        }
        pInetOptions->SetProxyType(eSet);
        bModified = TRUE;
    }
    if(aHttpProxyED.GetSavedValue() != aHttpProxyED.GetText())
    {
        pInetOptions->SetProxyHttpName(aHttpProxyED.GetText());
        bModified = TRUE;
    }

    if ( aHttpPortED.GetSavedValue() != aHttpPortED.GetText() )
    {
        pInetOptions->SetProxyHttpPort(aHttpPortED.GetText().ToInt32());
        bModified = TRUE;
    }
    if(aFtpProxyED.GetSavedValue() != aFtpProxyED.GetText())
    {
        pInetOptions->SetProxyFtpName(aFtpProxyED.GetText());
        bModified = TRUE;
    }

    if ( aFtpPortED.GetSavedValue() != aFtpPortED.GetText() )
    {
        pInetOptions->SetProxyFtpPort(aFtpPortED.GetText().ToInt32());
        bModified = TRUE;
    }
    if ( aNoProxyForED.GetSavedValue() != aNoProxyForED.GetText() )
    {
        pInetOptions->SetProxyNoProxy(aNoProxyForED.GetText());
        bModified = TRUE;
    }

    return bModified;
}
/*-----------------12.08.96 13.38-------------------

--------------------------------------------------*/
void SvxProxyTabPage::EnableControls_Impl(BOOL bEnable)
{
    aHttpProxyFT.Enable(bEnable);
    aHttpProxyED.Enable(bEnable);
    aHttpPortFT.Enable(bEnable);
    aHttpPortED.Enable(bEnable);

    aFtpProxyFT.Enable(bEnable);
    aFtpProxyED.Enable(bEnable);
    aFtpPortFT.Enable(bEnable);
    aFtpPortED.Enable(bEnable);

    aNoProxyForFT.Enable(bEnable);
    aNoProxyForED.Enable(bEnable);
    aNoProxyDescFT.Enable(bEnable);
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxProxyTabPage, ProxyHdl_Impl, ListBox *, pBox )
{
    BOOL bEnableProxyControls = pBox->GetSelectEntryPos() == 1;
    EnableControls_Impl(bEnableProxyControls);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxProxyTabPage, LoseFocusHdl_Impl, Edit *, pEdit )
{
    XubString aValue = pEdit->GetText();

    if ( !ByteString(::rtl::OUStringToOString(aValue,RTL_TEXTENCODING_UTF8)).IsNumericAscii() || (long)aValue.ToInt32() > USHRT_MAX )
        pEdit->SetText( '0' );
    return 0;
}


/********************************************************************/
/********************************************************************/
/*                                                                  */
/*  SvxSearchTabPage                                                */
/*                                                                  */
/*                                                                  */
/********************************************************************/
/********************************************************************/

SvxSearchTabPage::SvxSearchTabPage(Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_INET_SEARCH ), rSet ),

    aSearchGB       ( this, ResId( GB_SEARCH ) ),
    aSearchFT       ( this, ResId( FT_SEARCH ) ),
    aSearchLB       ( this, ResId( LB_SEARCH ) ),
    aSearchNameFT   ( this, ResId( FT_SEARCH_NAME ) ),
    aSearchNameED   ( this, ResId( ED_SEARCH_NAME ) ),
    aURLFT          ( this, ResId( FT_URL ) ),
    aURLED          ( this, ResId( ED_URL ) ),
    aAndRB          ( this, ResId( RB_AND ) ),
    aOrRB           ( this, ResId( RB_OR ) ),
    aExactRB        ( this, ResId( RB_EXACT ) ),
    aSeparatorFT    ( this, ResId( FT_SEPARATOR ) ),
    aSeparatorED    ( this, ResId( ED_SEPARATOR ) ),
    aPostFixFT      ( this, ResId( FT_POSTFIX ) ),
    aPostFixED      ( this, ResId( ED_POSTFIX ) ),
    aCaseFT         ( this, ResId( FT_CASE ) ),
    aCaseED         ( this, ResId( ED_CASE ) ),
    aNewPB          ( this, ResId( PB_NEW ) ),
    aAddPB          ( this, ResId( PB_ADD ) ),
    aChangePB       ( this, ResId( PB_CHANGE ) ),
    aDeletePB       ( this, ResId( PB_DELETE ) ),
    sModifyMsg(ResId(MSG_MODIFY))
{
    FreeResource();
    SetExchangeSupport();
    aCaseED.SelectEntryPos(0); // falls kein Eintrag vorhanden ist, kann es sonst "Arger geben

    aNewPB.SetClickHdl(LINK( this, SvxSearchTabPage, NewSearchHdl_Impl ) );
    aAddPB.SetClickHdl(LINK( this, SvxSearchTabPage, AddSearchHdl_Impl ) );
    aChangePB.SetClickHdl(LINK( this, SvxSearchTabPage, ChangeSearchHdl_Impl ) );
    aDeletePB.SetClickHdl(LINK( this, SvxSearchTabPage, DeleteSearchHdl_Impl ) );
    aSearchLB.SetSelectHdl(LINK( this, SvxSearchTabPage, SearchEntryHdl_Impl ) );

    Link aLink = LINK( this, SvxSearchTabPage, SearchModifyHdl_Impl );
    aSearchNameED.SetModifyHdl( aLink );
    aURLED.SetModifyHdl( aLink );
    aSeparatorED.SetModifyHdl( aLink );
    aPostFixED.SetModifyHdl( aLink );
    aCaseED.SetSelectHdl( aLink );

    aLink = LINK( this, SvxSearchTabPage, SearchPartHdl_Impl );
    aAndRB.SetClickHdl( aLink );
    aOrRB.SetClickHdl( aLink );
    aExactRB.SetClickHdl( aLink );
}

// -----------------------------------------------------------------------
SvxSearchTabPage::~SvxSearchTabPage()
{
}
// -----------------------------------------------------------------------

SfxTabPage* SvxSearchTabPage::Create(Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxSearchTabPage(pParent, rAttrSet);
}

// -----------------------------------------------------------------------

void SvxSearchTabPage::Reset( const SfxItemSet& )
{
    //The two lines below are moved here from the last part of this method by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)
    aChangePB.Disable();
    aAddPB.Disable();

    sal_uInt16 nCount = aSearchConfig.Count();
    aSearchLB.Clear();  //add by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)
    for(sal_uInt16 i = 0; i < nCount; i++)
    {
        const SvxSearchEngineData& rData = aSearchConfig.GetData(i);
        aSearchLB.InsertEntry(rData.sEngineName);
    }

    if ( nCount )
    {
        aSearchLB.SelectEntryPos(0);
        SearchEntryHdl_Impl( &aSearchLB );
    }
    else
        aDeletePB.Disable();
}

// -----------------------------------------------------------------------

BOOL SvxSearchTabPage::FillItemSet( SfxItemSet& rSet )
{
    if(aSearchConfig.IsModified())
        aSearchConfig.Commit();
    return TRUE;
}
/*--------------------------------------------------------------------*/

void SvxSearchTabPage::ActivatePage( const SfxItemSet& rSet )
{
}

/*--------------------------------------------------------------------*/

int SvxSearchTabPage::DeactivatePage( SfxItemSet* pSet )
{
    //Modified by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)
    if(!ConfirmLeave(String()))
        return KEEP_PAGE;

    if ( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

//add by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)
BOOL SvxSearchTabPage::ConfirmLeave( const String& rStringSelection)
{
    if(aChangePB.IsEnabled())
    {
        QueryBox aQuery(this, WB_YES_NO_CANCEL|WB_DEF_YES, sModifyMsg);
        USHORT nRet = aQuery.Execute();
        if(RET_CANCEL == nRet)
        {
            if(rStringSelection.Len())
                aSearchLB.SelectEntry(sLastSelectedEntry);
            return FALSE;
        }
        else if(RET_YES == nRet)
        {
            USHORT nEntryPos = aSearchLB.GetEntryPos( aSearchNameED.GetText() );
            if ( nEntryPos != LISTBOX_ENTRY_NOTFOUND  )
                aSearchLB.SelectEntryPos(nEntryPos);
            else
                aSearchLB.SetNoSelection();
            ChangeSearchHdl_Impl(0);
            if(rStringSelection.Len())
                aSearchLB.SelectEntry(rStringSelection);
        }
        else if(RET_NO == nRet)
        {
            aChangePB.Enable(FALSE);
            aAddPB.Enable(FALSE);
            SearchEntryHdl_Impl(&aSearchLB);
        }
    }
    if(aAddPB.IsEnabled())
    {
        QueryBox aQuery(this, WB_YES_NO_CANCEL|WB_DEF_YES, sModifyMsg);
        USHORT nRet = aQuery.Execute();
        if(RET_CANCEL == nRet)
        {
            aSearchLB.SetNoSelection();
            return FALSE;
        }
        else if(RET_YES == nRet)
        {
            aSearchLB.SetNoSelection();
            AddSearchHdl_Impl(0);
            if(rStringSelection.Len())
                aSearchLB.SelectEntry(rStringSelection);
        }
        else if(RET_NO == nRet)
        {
            aAddPB.Enable(FALSE);
            aChangePB.Enable(FALSE);
            NewSearchHdl_Impl(0);
        }

    }
    return TRUE;
}
// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchTabPage, NewSearchHdl_Impl, PushButton *, EMPTYARG )
{
    //The 3 lines below is modified by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)
    SearchEntryHdl_Impl(&aSearchLB);
    if(aChangePB.IsEnabled() || aAddPB.IsEnabled())
        return 0;
    aSearchNameED.SetText( String() );
    aSearchLB.SetNoSelection();
    aCurrentSrchData = SvxSearchEngineData();
    aAndRB.Check( TRUE );
    SearchEntryHdl_Impl( &aSearchLB );
    SearchPartHdl_Impl( &aAndRB );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchTabPage, AddSearchHdl_Impl, PushButton *, EMPTYARG )
{
    //The following two lines is added by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)
    aAddPB.Enable(FALSE);
    aChangePB.Enable(FALSE);
    aCurrentSrchData.sEngineName = aSearchNameED.GetText();
    aSearchConfig.SetData(aCurrentSrchData);
    aSearchLB.InsertEntry( aCurrentSrchData.sEngineName );
    aSearchLB.SelectEntry( aCurrentSrchData.sEngineName );
    SearchEntryHdl_Impl( &aSearchLB );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchTabPage, ChangeSearchHdl_Impl, PushButton *, EMPTYARG )
{
    //The following two lines is added by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)
    aChangePB.Enable(FALSE);
    aAddPB.Enable(FALSE);
    USHORT nPos = aSearchLB.GetSelectEntryPos();
    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        String sEngine = aSearchLB.GetSelectEntry();
        aCurrentSrchData.sEngineName = sEngine;
        aSearchConfig.SetData(aCurrentSrchData);
        aSearchLB.SelectEntry(sEngine);
        SearchEntryHdl_Impl(&aSearchLB);
    }
    else
    {
        SetUpdateMode(FALSE);
        String sEntry = aSearchNameED.GetText();
        // im AddHdl wird sLastSelectedEntry umgesetzt
        String sTemp(sLastSelectedEntry);
        AddSearchHdl_Impl(0);
        aSearchLB.SelectEntry(sTemp);
        DeleteSearchHdl_Impl(0);
        aSearchLB.SelectEntry(sEntry);
        SearchEntryHdl_Impl(&aSearchLB);
        SetUpdateMode(TRUE);
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchTabPage, DeleteSearchHdl_Impl, PushButton *, EMPTYARG)
{
    aChangePB.Enable(FALSE);     //add by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)
    USHORT nPos = aSearchLB.GetSelectEntryPos();
    DBG_ASSERT(nPos != LISTBOX_ENTRY_NOTFOUND, "kein Eintrag selektiert!")
    aSearchConfig.RemoveData(aSearchLB.GetSelectEntry());
    aSearchLB.RemoveEntry(nPos);
    aSearchLB.SelectEntryPos(0);
    SearchEntryHdl_Impl(&aSearchLB);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchTabPage, SearchEntryHdl_Impl, ListBox*, pBox )
{
    USHORT nEntryPos = pBox->GetSelectEntryPos();
    if ( nEntryPos != LISTBOX_ENTRY_NOTFOUND )
    {
        String sSelection(pBox->GetSelectEntry());
        //Modified by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)
        if(!ConfirmLeave(sSelection))
            return 0;

        const SvxSearchEngineData* pData = aSearchConfig.GetData(sSelection);
        DBG_ASSERT(pData, "SearchEngine not available")
        if(pData)
        {
            aSearchNameED.SetText(sSelection);
            sLastSelectedEntry = sSelection;
            sal_Bool bAnd = aAndRB.IsChecked();
            sal_Bool bOr = aOrRB.IsChecked();

            aURLED.SetText(bAnd ? pData->sAndPrefix : bOr ? pData->sOrPrefix : pData->sExactPrefix);
            aSeparatorED.SetText( bAnd ? pData->sAndSeparator : bOr ? pData->sOrSeparator : pData->sExactSeparator);
            aPostFixED.SetText(bAnd ? pData->sAndSuffix : bOr ? pData->sOrSuffix : pData->sExactSuffix );
            sal_Int32 nCase = bAnd ? pData->nAndCaseMatch : bOr ? pData->nOrCaseMatch : pData->nExactCaseMatch;
            aCaseED.SelectEntryPos( (USHORT)nCase );
            aCurrentSrchData = *pData;
        }
        aDeletePB.Enable();
    }
    else
    {
        aDeletePB.Enable(FALSE);
        sLastSelectedEntry.Erase();
    }
    aChangePB.Enable(FALSE);
    aAddPB.Enable(FALSE);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchTabPage, SearchModifyHdl_Impl, SvxNoSpaceEdit*, pEdit )
{
    if ( pEdit == &aSearchNameED )
    {
        BOOL bTextLen = ( 0 != pEdit->GetText().Len() );
        BOOL bFound = FALSE;
        if ( bTextLen )
        {
            USHORT nEntryPos = aSearchLB.GetEntryPos( pEdit->GetText() );
            bFound = ( nEntryPos != LISTBOX_ENTRY_NOTFOUND );
            if ( bFound )
                aSearchLB.SelectEntryPos(nEntryPos);
            else
                aSearchLB.SetNoSelection();
        }
        aChangePB.Enable( sLastSelectedEntry.Len() > 0 );
        aDeletePB.Enable(bFound);
        aAddPB.Enable(bTextLen && !bFound);
    }
    else
    {
        if ( aSearchLB.GetSelectEntryCount() && sLastSelectedEntry.Len() > 0 )
            aChangePB.Enable();

        if(aAndRB.IsChecked())
        {
            aCurrentSrchData.sAndPrefix = aURLED.GetText();
            aCurrentSrchData.sAndSeparator = aSeparatorED.GetText();
            aCurrentSrchData.sAndSuffix = aPostFixED.GetText();
            aCurrentSrchData.nAndCaseMatch = aCaseED.GetSelectEntryPos();
        }
        else if(aOrRB.IsChecked())
        {
            aCurrentSrchData.sOrPrefix = aURLED.GetText();
            aCurrentSrchData.sOrSeparator = aSeparatorED.GetText();
            aCurrentSrchData.sOrSuffix = aPostFixED.GetText();
            aCurrentSrchData.nOrCaseMatch = aCaseED.GetSelectEntryPos();
        }
        else
        {
            aCurrentSrchData.sExactPrefix = aURLED.GetText();
            aCurrentSrchData.sExactSeparator = aSeparatorED.GetText();
            aCurrentSrchData.sExactSuffix = aPostFixED.GetText();
            aCurrentSrchData.nExactCaseMatch = aCaseED.GetSelectEntryPos();
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchTabPage, SearchPartHdl_Impl, RadioButton *, pButton )
{
    sal_Bool bAnd = aAndRB.IsChecked();
    sal_Bool bOr = aOrRB.IsChecked();

    aURLED.SetText(bAnd ? aCurrentSrchData.sAndPrefix : bOr ? aCurrentSrchData.sOrPrefix : aCurrentSrchData.sExactPrefix);
    aSeparatorED.SetText( bAnd ? aCurrentSrchData.sAndSeparator : bOr ? aCurrentSrchData.sOrSeparator : aCurrentSrchData.sExactSeparator);
    aPostFixED.SetText(bAnd ? aCurrentSrchData.sAndSuffix : bOr ? aCurrentSrchData.sOrSuffix : aCurrentSrchData.sExactSuffix );
    sal_Int32 nCase = bAnd ? aCurrentSrchData.nAndCaseMatch : bOr ? aCurrentSrchData.nOrCaseMatch : aCurrentSrchData.nExactCaseMatch;
    aCaseED.SelectEntryPos( (USHORT)nCase );
    return 0;
}

// -----------------------------------------------------------------------

/********************************************************************/
/********************************************************************/
/*                                                                  */
/*  SvxOtherTabPage                                                 */
/*                                                                  */
/********************************************************************/
/********************************************************************/

/*-----------------15.05.97 09:51-------------------

--------------------------------------------------*/
/*
SvxPatternField::SvxPatternField( Window* pParent, const ResId& rResId ) :

    PatternField( pParent, rResId ),

    sMsg233 ( ResId( ST_MSG_233 ) ),
    sMsg255 ( ResId( ST_MSG_255 ) )

{
    FreeResource();
    SelectFixedFont();
} */

/*-----------------15.05.97 09:51-------------------

--------------------------------------------------*/

/*void SvxPatternField::KeyInput( const KeyEvent& rKEvt )
{
    PatternField::KeyInput( rKEvt );
    BOOL bDelete = ( rKEvt.GetKeyCode().GetCode() == KEY_DELETE );
    String sEntry( GetText() );
    sEntry[(USHORT)3] = '.';
    sEntry[(USHORT)7] = '.';
    sEntry[(USHORT)11] = '.';
    Selection aSelection( GetSelection() );
    String sPart( sEntry.GetToken( 0, '.' ) );
    USHORT i, nPart( sPart.EraseLeadingChars() );
    BOOL bSet = FALSE;

    if ( sPart.Len() && ( !nPart || nPart > 255 ) )
    {
        // der erste Part darf nicht 0 und nicht gr"osser 255 sein
        String sMsg( sPart );
        sMsg += ' ';
        sMsg += sMsg233;
        InfoBox( this, sMsg ).Execute();

        if ( nPart == 0 )
            sPart = "  1";
        else
            sPart = "255";
        sEntry.SetToken( 0, '.', sPart );
        bSet = TRUE;
    };

    for ( i = 1; i < 4; i++ )
    {
        // die anderen Parts d"urfen nicht gr"osser 255 sein
        sPart = sEntry.GetToken( i, '.' );
        nPart = sPart.EraseLeadingChars();

        if ( nPart > 255 )
        {
            String sMsg( sPart );
            sMsg += ' ';
            sMsg += sMsg255;
            InfoBox( this, sMsg ).Execute();

            if ( nPart == 0 )
                sPart = "  1";
            else
                sPart = "255";
            sEntry.SetToken( i, '.', sPart );
            bSet = TRUE;
        };
    }

    if ( bSet )
    {
        SetText( sEntry );
        SetSelection( aSelection );
    }
}
*/
// -----------------------------------------------------------------------
#if 0
long SvxPatternField::Notify( NotifyEvent& rNEvt )
{
    return PatternField::Notify( rNEvt );
/*! long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYUP )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        KeyInput( *pKEvt );
        nHandled = 1;
    }
    return nHandled;*/
}
#endif

// class JavaScriptDisableQueryBox_Impl --------------------------------------

class JavaScriptDisableQueryBox_Impl : public ModalDialog
{
private:
    FixedImage      aImage;
    FixedText       aWarningFT;
    CheckBox        aDisableCB;
    OKButton        aYesBtn;
    CancelButton    aNoBtn;

public:
    JavaScriptDisableQueryBox_Impl( Window* pParent );

    BOOL        IsWarningDisabled() const { return aDisableCB.IsChecked(); }
};

JavaScriptDisableQueryBox_Impl::JavaScriptDisableQueryBox_Impl( Window* pParent ) :

    ModalDialog( pParent, SVX_RES( RID_SVXDLG_OPT_JAVASCRIPT_DISABLE ) ),

    aImage      ( this, ResId( IMG_JSCPT_WARNING ) ),
    aWarningFT  ( this, ResId( FT_JSCPT_WARNING ) ),
    aDisableCB  ( this, ResId( CB_JSCPT_DISABLE ) ),
    aYesBtn     ( this, ResId( BTN_JSCPT_YES ) ),
    aNoBtn      ( this, ResId( BTN_JSCPT_NO ) )

{
    FreeResource();

    aYesBtn.SetText( Button::GetStandardText( BUTTON_YES ) );
    aNoBtn.SetText( Button::GetStandardText( BUTTON_NO ) );
    aImage.SetImage( WarningBox::GetStandardImage() );
}

//#98647#----------------------------------------------
void SvxScriptExecListBox::RequestHelp( const HelpEvent& rHEvt )
{   // try to show tips just like as on toolbars
    USHORT nPos=LISTBOX_ENTRY_NOTFOUND;
    USHORT nTop = GetTopEntry();
    USHORT nCount = GetDisplayLineCount(); // Attention: Not GetLineCount()
    Point aPt = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
    Rectangle aItemRect;
    if( nCount > 0 ) // if there're some entries, find it.
         for( nPos = nTop ; nPos <= nTop+nCount-1 ; nPos++ ) {
            aItemRect = GetBoundingRectangle(nPos);
            if( aPt.Y() < aItemRect.Top() || aPt.Y() > aItemRect.Bottom() )
                continue;
            else
                break;
        }
     else // if not, nothing happens.
         return;
     String aHelpText;
     if( nPos <= nTop+nCount-1 ) // if find the matching entry, get its content.
         aHelpText = GetEntry(nPos);
    if( aHelpText.Len() && GetTextWidth(aHelpText)<GetOutputSizePixel().Width() )
        aHelpText.Erase(); // if the entry is quite short, clear the helping tip content.
    aItemRect = Rectangle(Point(0,0),GetSizePixel());
    aPt = Point(OutputToScreenPixel( aItemRect.TopLeft() ));
    aItemRect.Left()   = aPt.X();
    aItemRect.Top()    = aPt.Y();
    aPt = OutputToScreenPixel( aItemRect.BottomRight() );
    aItemRect.Right()  = aPt.X();
    aItemRect.Bottom() = aPt.Y();
    if( rHEvt.GetMode() == HELPMODE_BALLOON )
        Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aHelpText);
    else
        Help::ShowQuickHelp( this, aItemRect, aHelpText );
}

/********************************************************************/
/*                                                                  */
/*  SvxScriptingTabPage                                             */
/*                                                                  */
/********************************************************************/

SvxScriptingTabPage::SvxScriptingTabPage( Window* pParent, const SfxItemSet& rSet ) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_INET_SCRIPTING ), rSet ),

    aGrpScriptingStarBasic  ( this, ResId( GB_STARBASIC ) ),
    aExecMacroFT            ( this, ResId( FT_EXEC_MACRO ) ),
    aExecMacroLB            ( this, ResId( LB_EXEC_MACRO ) ),
    aConfirmCB              ( this, ResId( CB_CONFIRM ) ),
    aWarningCB              ( this, ResId( CB_WARNING ) ),
    aPathListFT             ( this, ResId( FT_PATH_LIST ) ),
    aLbScriptExec           ( this, ResId( LB_SCRIPT_EXEC ) ),
    aBtnScriptExecDelete    ( this, ResId( PB_SCRIPT_DELETE ) ),
    aBtnScriptExecDefault   ( this, ResId( PB_SCRIPT_DEFAULT ) ),
    aNewPathFT              ( this, ResId( FT_NEW_PATH ) ),
    aEdtScriptExec          ( this, ResId( ED_SCRIPT_EXEC ) ),
    aBtnScriptExecInsert    ( this, ResId( PB_SCRIPT_INSERT ) ),
    aHyperlinksFL           ( this, ResId( FL_HYPERLINKS ) ),
    aHyperlinksFT           ( this, ResId( FT_HYPERLINKS ) ),
    aHyperlinksLB           ( this, ResId( LB_HYPERLINKS ) ),
    aJavaFL                 ( this, ResId( FL_JAVA ) ),
    aJavaEnableCB           ( this, ResId( CB_J_ENABLE ) ),
    aJavaSecurityCB         ( this, ResId( CB_J_SECURITY ) ),
    aNetAccessFT            ( this, ResId( FT_NETACCESS ) ),
    aNetAccessLB            ( this, ResId( LB_NETACCESS ) ),
    aClassPathFT            ( this, ResId( FT_CLASSPATH ) ),
    aClassPathED            ( this, ResId( ED_CLASSPATH ) ),
    aClassPathPB            ( this, ResId( PB_CLASSPATH ) ),
    aSeparatorFL            ( this, ResId( FL_SEPARATOR ) ),
    aExecuteGB              ( this, ResId( GB_EXECUTE ) ),
    aExePlugCB              ( this, ResId( CB_EXECUTE_PLUGINS ) ),
    aExecAppletsCB          ( this, ResId( CB_EXECUTE_APPLETS ) ),

    aExecMacroFI(       this, ResId( FI_EXECMACRO    )),
    aConfirmFI(         this, ResId( FI_CONFIRM      )),
    aWarningFI(         this, ResId( FI_WARNING      )),
    aScriptExecFI(      this, ResId( FI_SCRIPTEXEC   )),
    aHyperlinksFI(      this, ResId( FI_HYPERLINKS   )),
    aJavaEnableFI(      this, ResId( FI_JAVAENABLE   )),
    aJavaSecurityFI(    this, ResId( FI_JAVASECURITY )),
    aNetAccessFI(       this, ResId( FI_NETACCESS    )),
    aClassPathFI(       this, ResId( FI_CLASSPATH    )),
    aExePlugFI(         this, ResId( FI_EXECPLUG     )),
    aExecAppletsFI(     this, ResId( FI_EXEAPPLETS  )),

    pJavaOptions            ( new SvtJavaOptions ),
    pSecurityOptions        ( new SvtSecurityOptions ),

    bROConfirm              ( CFG_READONLY_DEFAULT),
    bROWarning              ( CFG_READONLY_DEFAULT),
    bROExecMacro            ( CFG_READONLY_DEFAULT),
    bROExePlug              ( CFG_READONLY_DEFAULT),
    bROJavaEnabled          ( CFG_READONLY_DEFAULT),
    bROJavaSecurity         ( CFG_READONLY_DEFAULT),
    bROJavaNetAccess        ( CFG_READONLY_DEFAULT),
    bROJavaUserClassPath    ( CFG_READONLY_DEFAULT),
    bROJavaExecuteApplets   ( CFG_READONLY_DEFAULT)
{
    FreeResource();

    aEdtScriptExec.SetModifyHdl( LINK(this, SvxScriptingTabPage, EditHdl_Impl) );
    aLbScriptExec.SetSelectHdl( LINK(this, SvxScriptingTabPage, LBHdl_Impl) );

    Link aLink( LINK(this, SvxScriptingTabPage, BtnHdl_Impl) );
    aBtnScriptExecInsert.SetClickHdl( aLink );
    aBtnScriptExecDelete.SetClickHdl( aLink );
    aBtnScriptExecDefault.SetClickHdl( aLink );

    aExecMacroLB.SetSelectHdl(LINK( this, SvxScriptingTabPage, RunHdl_Impl ));
    EditHdl_Impl( NULL );
    ActivatePage( rSet );

#ifdef SOLAR_JAVA
    aLink = LINK(this, SvxScriptingTabPage, JavaEnableHdl_Impl);
    aJavaEnableCB.SetClickHdl( aLink );
    aJavaSecurityCB.SetClickHdl( aLink );
    aClassPathPB.SetClickHdl( LINK(this, SvxScriptingTabPage, ClassPathHdl_Impl) );
#endif

    aJavaEnableCB.Hide();
    aClassPathFT.Hide();
    aClassPathED.Hide();
    aClassPathPB.Hide();
}

/*--------------------------------------------------------------------*/
SvxScriptingTabPage::~SvxScriptingTabPage()
{
    delete pJavaOptions;
    delete pSecurityOptions;
}
// -----------------------------------------------------------------------
void SvxScriptingTabPage::EnableJava_Impl( BOOL bEnable, BOOL bOnlySecurity )
{
    if ( !bOnlySecurity )
    {
        aJavaSecurityCB.Enable( bEnable && bROJavaSecurity);
        aClassPathFT.Enable( bEnable && !bROJavaUserClassPath);
        aClassPathED.Enable( bEnable && !bROJavaUserClassPath);
        aClassPathPB.Enable( bEnable && !bROJavaExecuteApplets);
        aExecAppletsCB.Enable( bEnable && !bROJavaExecuteApplets);
    }

    bEnable = ( bEnable && aJavaSecurityCB.IsChecked() && !bROJavaNetAccess);
    aNetAccessFT.Enable( bEnable );
    aNetAccessLB.Enable( bEnable );
}
// -----------------------------------------------------------------------
IMPL_LINK( SvxScriptingTabPage, JavaEnableHdl_Impl, CheckBox*, pBox )
{
    EnableJava_Impl( pBox->IsChecked(), &aJavaSecurityCB == pBox );
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxScriptingTabPage, ClassPathHdl_Impl, PushButton*, EMPTYARG )
{
    //CHINA001 SvxMultiFileDialog aDlg( this, TRUE );
    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
    if(pFact)
    {
        AbstractSvxMultiFileDialog* aDlg = pFact->CreateSvxMultiFileDialog( this, ResId(RID_SVXDLG_MULTIPATH), TRUE );
        DBG_ASSERT(aDlg, "Dialogdiet fail!");//CHINA001
        aDlg->SetClassPathMode(); //CHINA001 aDlg.SetClassPathMode();
        aDlg->SetPath( aClassPathED.GetText() ); //CHINA001 aDlg.SetPath( aClassPathED.GetText() );
        if ( aDlg->Execute() == RET_OK ) //CHINA001 if ( aDlg.Execute() == RET_OK )
        {
            // convert to system path if necessary
            String sNewPath, sPath = aDlg->GetPath(); //CHINA001 String sNewPath, sPath = aDlg.GetPath();
            xub_StrLen nCount = sPath.GetTokenCount( CLASSPATH_DELIMITER );
            for ( USHORT i = 0; i < nCount; ++i )
            {
                String sTmp, sToken = sPath.GetToken( i, CLASSPATH_DELIMITER );
                if ( ::utl::LocalFileHelper::ConvertURLToSystemPath( sToken, sTmp ) )
                    sToken = sTmp;
                if ( sNewPath.Len() > 0 )
                    sNewPath += CLASSPATH_DELIMITER;
                sNewPath += sToken;
            }

            aClassPathED.SetText( sNewPath );
        }
        delete aDlg; //add by CHINA001
    }
    return 1;
}


/*--------------------------------------------------------------------*/

SfxTabPage* SvxScriptingTabPage::Create(Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxScriptingTabPage(pParent, rAttrSet);
}

/*--------------------------------------------------------------------*/

void SvxScriptingTabPage::ActivatePage( const SfxItemSet& rSet )
{
}

/*--------------------------------------------------------------------*/

int SvxScriptingTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

/*--------------------------------------------------------------------*/

void SvxScriptingTabPage::FillListBox_Impl()
{
}

/*--------------------------------------------------------------------*/

IMPL_LINK( SvxScriptingTabPage, EditHdl_Impl, Edit*, EMPTYARG )

/*  [Beschreibung]

    ModifyHandler vom Edit. Der 'Hinzuf"ugen'-Button wird nur enabled,
    wenn der Text im Edit vorhanden ist und nicht nur aus Blanks besteht.
*/

{
    String aTxt = aEdtScriptExec.GetText();
    aTxt.EraseLeadingChars().EraseTrailingChars();
    aBtnScriptExecInsert.Enable( aTxt.Len() > 0 );
    return 1L;
}

/*--------------------------------------------------------------------*/

IMPL_LINK( SvxScriptingTabPage, LBHdl_Impl, ListBox*, EMPTYARG )
{
    USHORT nPos = aLbScriptExec.GetSelectEntryPos();
    aBtnScriptExecDelete.Enable( nPos != LISTBOX_ENTRY_NOTFOUND );
    return 1L;
}

/*--------------------------------------------------------------------*/

IMPL_LINK( SvxScriptingTabPage, BtnHdl_Impl, PushButton*, pBtn )
{
    if( pBtn == &aBtnScriptExecInsert )
    {
        // Insert new entry if not found in listbox
        String aNewEntry = aEdtScriptExec.GetText();
        aNewEntry.EraseLeadingChars().EraseTrailingChars();
        INetURLObject aINetObject(aNewEntry, INET_PROT_HTTP);
        if(INET_PROT_FILE == aINetObject.GetProtocol())
            aNewEntry = aINetObject.GetFull();
        else
            aNewEntry = aINetObject.GetMainURL( INetURLObject::DECODE_TO_IURI );
        // if it's not a URL of any kind the original string is used again
        if(!aNewEntry.Len())
            aNewEntry = aEdtScriptExec.GetText().EraseLeadingChars().EraseTrailingChars();
        aEdtScriptExec.SetText( aNewEntry );
        FASTBOOL bFound = !aNewEntry.Len();

        if ( !bFound )
        {
            for ( USHORT i = 0; i < aLbScriptExec.GetEntryCount(); ++i )
            {
                String aEntry = aLbScriptExec.GetEntry(i);

                if ( aEntry.CompareIgnoreCaseToAscii( aNewEntry ) == COMPARE_EQUAL )
                {
                    bFound = TRUE;
                    break;
                }
            }
        }

        if ( !bFound )
        {
            aLbScriptExec.InsertEntry( aNewEntry );
            aLbScriptExec.SelectEntry( aNewEntry );
        }
    }
    else if( pBtn == &aBtnScriptExecDelete )
    {
        // delete selected entry
        USHORT nPos = aLbScriptExec.GetSelectEntryPos();

        if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        {
            aLbScriptExec.RemoveEntry( nPos );
            USHORT nCount = aLbScriptExec.GetEntryCount();

            if ( nCount > 0 )
            {
                if ( nPos >= nCount )
                    nPos = nCount - 1;
            }
            aLbScriptExec.SelectEntryPos( nPos );
        }
    }
    else if ( pBtn == &aBtnScriptExecDefault )
    {
        // Reset listbox and edit

        aLbScriptExec.Clear();
        SvtPathOptions aPathOpt;

        String sModule = URIHelper::SmartRelToAbs( aPathOpt.GetModulePath() );
        INetURLObject aURLObject(sModule);
        aURLObject.removeSegment();
        aLbScriptExec.InsertEntry( aURLObject.GetFull() );
        // #102041# -------------------------------
        rtl::OUString aString;
        utl::Bootstrap::locateUserInstallation( aString );
        UniString aUniString( aString );
        aUniString.AppendAscii( "/user" );
        aURLObject = aUniString;
        aLbScriptExec.InsertEntry( aURLObject.GetFull() );
        aEdtScriptExec.SetText( String() );
    }
    LBHdl_Impl( NULL );
    EditHdl_Impl( NULL );

    return 1;
}

/*--------------------------------------------------------------------*/

IMPL_LINK( SvxScriptingTabPage, RunHdl_Impl, ListBox*, pListBox )
{
    sal_Bool bConfirm = sal_False;
    sal_Bool bWarning = sal_True;
    switch(pListBox->GetSelectEntryPos())
    {
        case 1 : //according to list
            bConfirm = sal_True;
        break;
        case 0: //never
            bWarning = sal_False;
        break;
//        case 2 : //always
//        break;
    }
    if (!bROConfirm)
        aConfirmCB.Enable(bConfirm);
    if (!bROWarning)
        aWarningCB.Enable(bWarning);
    return 1;
}

/*--------------------------------------------------------------------*/

BOOL SvxScriptingTabPage::FillItemSet(SfxItemSet&)
{
    // ggf. neuen Eintrag einf"ugen
    BtnHdl_Impl( &aBtnScriptExecInsert );

    BOOL bModified = FALSE;

    // Liste fuer Scripting (Execute)
    if (!bROScriptExec)
    {
        Sequence< OUString > aURLs(aLbScriptExec.GetEntryCount());
        OUString* pURLs = aURLs.getArray();
        USHORT nCount = aLbScriptExec.GetEntryCount();
        for( USHORT i = 0; i < nCount; i++ )
        {
            String sURL(URIHelper::SmartRelToAbs(aLbScriptExec.GetEntry( i )));
            pURLs[i] = sURL;
        }

        if(aURLs != pSecurityOptions->GetSecureURLs())
            bModified = TRUE;

        if ( bModified )
        {
            pSecurityOptions->SetSecureURLs( aURLs );
        }
    }
    if( !bROConfirm && aConfirmCB.GetSavedValue() != aConfirmCB.IsChecked() )
    {
        pSecurityOptions->SetConfirmationEnabled(aConfirmCB.IsChecked());
        bModified = TRUE;
    }
    if( !bROWarning && aWarningCB.GetSavedValue() != aWarningCB.IsChecked() )
    {
        pSecurityOptions->SetWarningEnabled(aWarningCB.IsChecked());
        bModified = TRUE;
    }

    // Scripting
    if( !bROExecMacro && aExecMacroLB.GetSelectEntryPos() != aExecMacroLB.GetSavedValue())
    {
        USHORT nSB = aExecMacroLB.GetSelectEntryPos();
        pSecurityOptions->SetBasicMode((EBasicSecurityMode) nSB );
        bModified = TRUE;
    }

    //  Execute
    if( !bROExePlug && aExePlugCB.GetSavedValue() != aExePlugCB.IsChecked() )
    {
        pSecurityOptions->SetExecutePlugins( aExePlugCB.IsChecked());
        bModified = TRUE;
    }
    // Java
    sal_Bool bJavaOptionsModified = sal_False;
    if ( !pJavaOptions->IsReadOnly(SvtJavaOptions::E_ENABLED) && aJavaEnableCB.GetSavedValue() != aJavaEnableCB.IsChecked() )
    {
        pJavaOptions->SetEnabled(aJavaEnableCB.IsChecked());
        bJavaOptionsModified = sal_True;
    }

    // Applets
    if ( aExecAppletsCB.IsEnabled() &&
         aExecAppletsCB.GetSavedValue() != aExecAppletsCB.IsChecked() &&
         !pJavaOptions->IsReadOnly(SvtJavaOptions::E_EXECUTEAPPLETS) )
    {
        pJavaOptions->SetExecuteApplets(aExecAppletsCB.IsChecked());
        bJavaOptionsModified = sal_True;
    }

    USHORT nSelPos = aNetAccessLB.GetSelectEntryPos();
    if( !pJavaOptions->IsReadOnly(SvtJavaOptions::E_NETACCESS) && aNetAccessLB.GetSavedValue() != nSelPos )
    {
        pJavaOptions->SetNetAccess(nSelPos);
        bJavaOptionsModified = sal_True;
    }
    if(!pJavaOptions->IsReadOnly(SvtJavaOptions::E_SECURITY) && aJavaSecurityCB.GetSavedValue() != aJavaSecurityCB.IsChecked() )
    {
        pJavaOptions->SetSecurity(aJavaSecurityCB.IsChecked() );
        bJavaOptionsModified = sal_True;
    }
    if(!pJavaOptions->IsReadOnly(SvtJavaOptions::E_USERCLASSPATH) && aClassPathED.GetSavedValue() != aClassPathED.GetText() )
    {
        String aNewPath = aClassPathED.GetText();
        aNewPath.EraseLeadingChars().EraseTrailingChars();
        pJavaOptions->SetUserClassPath(aNewPath);
        bJavaOptionsModified = sal_True;
    }
    if(bJavaOptionsModified)
        pJavaOptions->Commit();

    // hyperlinks
    if ( aHyperlinksLB.GetSavedValue() != aHyperlinksLB.GetSelectEntryPos() )
    {
        SvtExtendedSecurityOptions::OpenHyperlinkMode eMode =
            (SvtExtendedSecurityOptions::OpenHyperlinkMode)aHyperlinksLB.GetSelectEntryPos();
        SvtExtendedSecurityOptions().SetOpenHyperlinkMode( eMode );
    }

    return bModified;
}

/*--------------------------------------------------------------------*/

void SvxScriptingTabPage::Reset( const SfxItemSet& )
{
    bROConfirm =    pSecurityOptions->IsReadOnly(SvtSecurityOptions::E_CONFIRMATION);
    bROWarning =    pSecurityOptions->IsReadOnly(SvtSecurityOptions::E_WARNING);
    bROScriptExec = pSecurityOptions->IsReadOnly(SvtSecurityOptions::E_SECUREURLS);
    bROExePlug =    pSecurityOptions->IsReadOnly(SvtSecurityOptions::E_EXECUTEPLUGINS);
    bROExecMacro =  pSecurityOptions->IsReadOnly(SvtSecurityOptions::E_BASICMODE);

    // Execute
    aLbScriptExec.Clear();
    Sequence< OUString > aURLs = pSecurityOptions->GetSecureURLs( );
    const OUString* pURLs = aURLs.getConstArray();
    sal_Int32 nCount = aURLs.getLength();

    for ( sal_Int32 i = 0; i < nCount; i++ )
    {
        INetURLObject aURL(pURLs[i]);
        aLbScriptExec.InsertEntry(
            INET_PROT_FILE == aURL.GetProtocol() ?
                aURL.GetFull() : aURL.GetMainURL( INetURLObject::DECODE_TO_IURI ) );
    }
    aScriptExecFI.Show(bROScriptExec);
    aLbScriptExec.Enable(!bROScriptExec);
    aEdtScriptExec.Enable(!bROScriptExec);
    aBtnScriptExecDelete.Enable(!bROScriptExec);
    aBtnScriptExecDefault.Enable(!bROScriptExec);
    aBtnScriptExecInsert.Enable(!bROScriptExec);
    aPathListFT.Enable(!bROScriptExec);
    aNewPathFT.Enable(!bROScriptExec);

    aWarningCB.Check(pSecurityOptions->IsWarningEnabled());
    aWarningCB.SaveValue();
    aWarningCB.Enable(!bROWarning);
    aWarningFI.Show(bROWarning);

    aConfirmCB.Check(pSecurityOptions->IsConfirmationEnabled());
    aConfirmFI.Show(bROConfirm);
    aConfirmCB.SaveValue();
    aConfirmCB.Enable(!bROConfirm);

    aExecMacroLB.SelectEntryPos(pSecurityOptions->GetBasicMode());
    aExecMacroFI.Show(bROExecMacro);
    aExecMacroLB.Enable(!bROExecMacro);
    aExecMacroFT.Enable(!bROExecMacro);
    aExecMacroLB.SaveValue();

    // Execute
    aExePlugCB.Check(pSecurityOptions->IsExecutePlugins());
    aExePlugCB.SaveValue();
    aExePlugCB.Enable(!bROExePlug);
    aExePlugFI.Show(bROExePlug);

    EditHdl_Impl( NULL );
    RunHdl_Impl(&aExecMacroLB);
    LBHdl_Impl( NULL );

    // Java
    bROJavaEnabled = pJavaOptions->IsReadOnly(SvtJavaOptions::E_ENABLED);
    bROJavaSecurity = pJavaOptions->IsReadOnly(SvtJavaOptions::E_SECURITY);
    bROJavaNetAccess = pJavaOptions->IsReadOnly(SvtJavaOptions::E_NETACCESS);
    bROJavaUserClassPath = pJavaOptions->IsReadOnly(SvtJavaOptions::E_USERCLASSPATH);
    bROJavaExecuteApplets = pJavaOptions->IsReadOnly(SvtJavaOptions::E_EXECUTEAPPLETS);

    aJavaEnableCB.Check(pJavaOptions->IsEnabled());
#ifndef SOLAR_JAVA
    aJavaEnableCB.Check(FALSE);
    aJavaEnableCB.Enable(FALSE);
#else
    aJavaEnableCB.SaveValue();
    aJavaEnableFI.Show(bROJavaEnabled);
    aJavaEnableCB.Enable(!bROJavaEnabled);
#endif
    EnableJava_Impl( aJavaEnableCB.IsChecked(), FALSE );

    aNetAccessLB.SelectEntryPos((USHORT)pJavaOptions->GetNetAccess());
    aNetAccessLB.SaveValue();
    aNetAccessFI.Show(bROJavaNetAccess);
    aNetAccessLB.Enable(!bROJavaNetAccess);


    aJavaSecurityCB.Check(pJavaOptions->IsSecurity());
    aJavaSecurityCB.SaveValue();
    aJavaSecurityFI.Show(bROJavaSecurity);
    aJavaSecurityCB.Enable(!bROJavaSecurity);
    EnableJava_Impl( ( aJavaEnableCB.IsChecked() && aJavaSecurityCB.IsChecked() ), TRUE );

    aClassPathED.SetText(pJavaOptions->GetUserClassPath());
    aClassPathED.SaveValue();
    aClassPathFI.Show(bROJavaUserClassPath);
    aClassPathED.Enable(!bROJavaUserClassPath);
    aClassPathPB.Enable(!bROJavaUserClassPath);

    // Execute Applets
    aExecAppletsCB.Check( pJavaOptions->IsExecuteApplets() );
    aExecAppletsCB.SaveValue();
    aExecAppletsFI.Show(bROJavaExecuteApplets);
    aExecAppletsCB.Enable(!bROJavaExecuteApplets);

    // hyperlinks
    SvtExtendedSecurityOptions aExtSecurityOptions;
    SvtExtendedSecurityOptions::OpenHyperlinkMode eMode = aExtSecurityOptions.GetOpenHyperlinkMode();
    aHyperlinksLB.SelectEntryPos( (USHORT)eMode );
    aHyperlinksLB.SelectEntryPos( (USHORT)eMode );
    sal_Bool bROExecuteHyperlinks = aExtSecurityOptions.IsOpenHyperlinkModeReadOnly();
    aHyperlinksFT.Enable(!bROExecuteHyperlinks);
    aHyperlinksLB.Enable(!bROExecuteHyperlinks);
    aHyperlinksFI.Show(bROExecuteHyperlinks);
    aHyperlinksLB.SaveValue();

    // disable groups if all controls of it are disabled
    // But use real "enabled" states of the controls - not the readonly values of the configuration!
    // Because some controls are disabled programmaticly if another control will have a special value ...

    if (!aLbScriptExec.IsEnabled() &&
        !aEdtScriptExec.IsEnabled() &&
        !aBtnScriptExecDelete.IsEnabled() &&
        !aBtnScriptExecDefault.IsEnabled() &&
        !aBtnScriptExecInsert.IsEnabled() &&
        !aPathListFT.IsEnabled() &&
        !aNewPathFT.IsEnabled() &&
        !aExecMacroLB.IsEnabled() &&
        !aExecMacroFT.IsEnabled() &&
        !aWarningCB.IsEnabled() &&
        !aConfirmCB.IsEnabled())
    {
        aGrpScriptingStarBasic.Enable(FALSE);
    }
    else
    {
        aGrpScriptingStarBasic.Enable(TRUE);
    }

    aExecuteGB.Enable(aExePlugCB.IsEnabled() ||
                            aExecAppletsCB.IsEnabled());

    aJavaFL.Enable(aJavaEnableCB.IsEnabled() ||
                    aNetAccessLB.IsEnabled() ||
                    aJavaSecurityCB.IsEnabled() ||
                    aClassPathED.IsEnabled());
}

/* -------------------------------------------------------------------------*/

class MailerProgramCfg_Impl : public utl::ConfigItem
{
    friend class SvxEMailTabPage;
    // variables
    OUString sProgram;
    // readonly states
    sal_Bool bROProgram;

    const Sequence<OUString> GetPropertyNames();
public:
    MailerProgramCfg_Impl();
    virtual ~MailerProgramCfg_Impl();

    virtual void    Commit();

};

/* -------------------------------------------------------------------------*/

MailerProgramCfg_Impl::MailerProgramCfg_Impl() :
    utl::ConfigItem(C2U("Office.Common/ExternalMailer")),
    bROProgram(sal_False)
{
    const Sequence< OUString > aNames = GetPropertyNames();
    const Sequence< Any > aValues = GetProperties(aNames);
    const Sequence< sal_Bool > aROStates = GetReadOnlyStates(aNames);
    const Any* pValues = aValues.getConstArray();
    const sal_Bool* pROStates = aROStates.getConstArray();
    for(sal_Int32 nProp = 0; nProp < aValues.getLength(); nProp++)
    {
        if(pValues[nProp].hasValue())
        {
            switch(nProp)
            {
                case 0 :
                {
                    pValues[nProp] >>= sProgram;
                    bROProgram = pROStates[nProp];
                }
                break;
            }
        }
    }
}

/* -------------------------------------------------------------------------*/

MailerProgramCfg_Impl::~MailerProgramCfg_Impl()
{
}

/* -------------------------------------------------------------------------*/

const Sequence<OUString> MailerProgramCfg_Impl::GetPropertyNames()
{
    Sequence<OUString> aRet(1);
    OUString* pRet = aRet.getArray();
    pRet[0] = C2U("Program");
    return aRet;
}

/* -------------------------------------------------------------------------*/

void MailerProgramCfg_Impl::Commit()
{
    const Sequence< OUString > aOrgNames = GetPropertyNames();
    sal_Int32 nOrgCount = aOrgNames.getLength();

    Sequence< OUString > aNames(nOrgCount);
    Sequence< Any > aValues(nOrgCount);
    sal_Int32 nRealCount = 0;

    for(int nProp = 0; nProp < nOrgCount; nProp++)
    {
    switch(nProp)
    {
            case  0:
            {
                if (!bROProgram)
                {
                    aNames[nRealCount] = aOrgNames[nProp];
                    aValues[nRealCount] <<= sProgram;
                    ++nRealCount;
                }
            }
            break;
        }
    }

    aNames.realloc(nRealCount);
    aValues.realloc(nRealCount);
    PutProperties(aNames, aValues);
}

/* -------------------------------------------------------------------------*/

struct SvxEMailTabPage_Impl
{
    MailerProgramCfg_Impl aMailConfig;
};

SvxEMailTabPage::SvxEMailTabPage(Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, ResId( RID_SVXPAGE_INET_MAIL, DIALOG_MGR() ), rSet),
    aMailFL(this,           ResId(FL_MAIL           )),
    aMailerURLFI(this,      ResId(FI_MAILERURL      )),
    aMailerURLFT(this,      ResId(FT_MAILERURL      )),
    aMailerURLED(this,      ResId(ED_MAILERURL      )),
    aMailerURLPB(this,      ResId(PB_MAILERURL      )),
    m_sDefaultFilterName(   ResId(STR_DEFAULT_FILENAME      )),
    pImpl(new SvxEMailTabPage_Impl)
{
    FreeResource();

    sal_Bool bHighContrast = GetDisplayBackground().GetColor().IsDark();

    Link aLink(LINK(this, SvxEMailTabPage, FileDialogHdl_Impl));
    aMailerURLPB.SetClickHdl(aLink);
}

/* -------------------------------------------------------------------------*/

SvxEMailTabPage::~SvxEMailTabPage()
{
    delete pImpl;
}

/* -------------------------------------------------------------------------*/

SfxTabPage*  SvxEMailTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxEMailTabPage(pParent, rAttrSet);
}

/* -------------------------------------------------------------------------*/

BOOL SvxEMailTabPage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bMailModified = FALSE;
    if(!pImpl->aMailConfig.bROProgram && aMailerURLED.GetSavedValue() != aMailerURLED.GetText())
    {
        pImpl->aMailConfig.sProgram = aMailerURLED.GetText();
        bMailModified = TRUE;
    }
    if ( bMailModified )
        pImpl->aMailConfig.Commit();

    return FALSE;
}

/* -------------------------------------------------------------------------*/

void SvxEMailTabPage::Reset( const SfxItemSet& rSet )
{
    aMailerURLED.Enable(TRUE );
    aMailerURLPB.Enable(TRUE );

    if(pImpl->aMailConfig.bROProgram)
        aMailerURLFI.Show();

    aMailerURLED.SetText(pImpl->aMailConfig.sProgram);
    aMailerURLED.SaveValue();
    aMailerURLED.Enable(!pImpl->aMailConfig.bROProgram);
    aMailerURLPB.Enable(!pImpl->aMailConfig.bROProgram);
    aMailerURLFT.Enable(!pImpl->aMailConfig.bROProgram);

    aMailFL.Enable(aMailerURLFT.IsEnabled() ||
                   aMailerURLED.IsEnabled() ||
                   aMailerURLPB.IsEnabled());
}

/* -------------------------------------------------------------------------*/

IMPL_LINK(  SvxEMailTabPage, FileDialogHdl_Impl, PushButton*, pButton )
{
    if ( &aMailerURLPB == pButton && !pImpl->aMailConfig.bROProgram )
    {
        FileDialogHelper aHelper( FILEOPEN_SIMPLE, WB_OPEN );
        String sPath = aMailerURLED.GetText();
        if ( !sPath.Len() )
            sPath.AppendAscii("/usr/bin");

        String sUrl;
        ::utl::LocalFileHelper::ConvertPhysicalNameToURL(sPath,sUrl);
        aHelper.SetDisplayDirectory(sUrl);
        aHelper.AddFilter( m_sDefaultFilterName, String::CreateFromAscii("*"));

        if ( ERRCODE_NONE == aHelper.Execute() )
        {
            sUrl = aHelper.GetPath();
            ::utl::LocalFileHelper::ConvertURLToPhysicalName(sUrl,sPath);
            aMailerURLED.SetText(sPath);
        }
    }
    return 0;
}

// -----------------------------------------------------------------------------
