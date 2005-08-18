/*************************************************************************
 *
 *  $RCSfile: optinet2.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2005-08-18 08:10:26 $
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

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

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
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
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
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
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
#include "ofaitem.hxx"
#ifndef _SVX_HTMLMODE_HXX
#include "htmlmode.hxx"
#endif

// for security TP
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_SECURITY_XDOCUMENTDIGITALSIGNATURES_HPP_
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>
#endif

#ifdef UNX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <string.h>
#include <osl/process.h>
#include <rtl/textenc.h>
#include <rtl/locale.h>
#include <osl/nlsupport.h>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _OSL_MODULE_HXX_
#include <osl/module.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

using namespace ::com::sun::star;
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

#include <sal/config.h>

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
    aProxyModePN(RTL_CONSTASCII_USTRINGPARAM("ooInetProxyType")),
    aHttpProxyPN(RTL_CONSTASCII_USTRINGPARAM("ooInetHTTPProxyName")),
    aHttpPortPN(RTL_CONSTASCII_USTRINGPARAM("ooInetHTTPProxyPort")),
    aFtpProxyPN(RTL_CONSTASCII_USTRINGPARAM("ooInetFTPProxyName")),
    aFtpPortPN(RTL_CONSTASCII_USTRINGPARAM("ooInetFTPProxyPort")),
    aNoProxyDescPN(RTL_CONSTASCII_USTRINGPARAM("ooInetNoProxy"))
{
    FreeResource();

    aHttpPortED.SetMaxTextLen(5);
    aFtpPortED.SetMaxTextLen(5);
    Link aLink = LINK( this, SvxProxyTabPage, LoseFocusHdl_Impl );
    aHttpPortED.SetLoseFocusHdl( aLink );
    aFtpPortED.SetLoseFocusHdl( aLink );

    aProxyModeLB.SetSelectHdl(LINK( this, SvxProxyTabPage, ProxyHdl_Impl ));

    Reference< com::sun::star::lang::XMultiServiceFactory > xServiceManager(
        ::comphelper::getProcessServiceFactory());

    if( xServiceManager.is() )
    {
        try
        {
            Reference< com::sun::star::lang::XMultiServiceFactory > xConfigurationProvider =
                Reference< com::sun::star::lang::XMultiServiceFactory > ( xServiceManager->createInstance( rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider" ) ) ),
                UNO_QUERY_THROW);

            OUString aConfigRoot(RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings" ) );

            beans::PropertyValue aProperty;
            aProperty.Name  = OUString(RTL_CONSTASCII_USTRINGPARAM( "nodepath" ));
            aProperty.Value = makeAny( aConfigRoot );

            Sequence< Any > aArgumentList( 1 );
            aArgumentList[0] = makeAny( aProperty );

            m_xConfigurationUpdateAccess = xConfigurationProvider->createInstanceWithArguments( rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationUpdateAccess" ) ),
                    aArgumentList );
        }

        catch (RuntimeException e) {
        }
    }

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
}

/*-----------------12.08.96 14.55-------------------

--------------------------------------------------*/

SfxTabPage* SvxProxyTabPage::Create(Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxProxyTabPage(pParent, rAttrSet);
}

/*-----------------02.09.04 14.55-------------------

--------------------------------------------------*/

void SvxProxyTabPage::ReadConfigData_Impl()
{
    try {
        Reference< container::XNameAccess > xNameAccess(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);

        sal_Int32 nIntValue;
        OUString  aStringValue;

        if( xNameAccess->getByName(aProxyModePN) >>= nIntValue )
        {
            aProxyModeLB.SelectEntryPos( (USHORT) nIntValue );
        }

        if( xNameAccess->getByName(aHttpProxyPN) >>= aStringValue )
        {
            aHttpProxyED.SetText( aStringValue );
        }

        if( xNameAccess->getByName(aHttpPortPN) >>= nIntValue )
        {
            aHttpPortED.SetText( String::CreateFromInt32( nIntValue ));
        }

        if( xNameAccess->getByName(aFtpProxyPN) >>= aStringValue )
        {
            aFtpProxyED.SetText( aStringValue );
        }

        if( xNameAccess->getByName(aFtpPortPN) >>= nIntValue )
        {
            aFtpPortED.SetText( String::CreateFromInt32( nIntValue ));
        }

        if( xNameAccess->getByName(aNoProxyDescPN) >>= aStringValue )
        {
            aNoProxyForED.SetText( aStringValue );
        }
    }

    catch(container::NoSuchElementException e) {
        OSL_TRACE( "SvxProxyTabPage::ReadConfigData_Impl: NoSuchElementException caught" );
    }

    catch(com::sun::star::lang::WrappedTargetException e) {
        OSL_TRACE( "SvxProxyTabPage::ReadConfigData_Impl: WrappedTargetException caught" );
    }

    catch(RuntimeException e) {
        OSL_TRACE( "SvxProxyTabPage::ReadConfigData_Impl: RuntimeException caught" );
    }

}

/*-----------------02.09.04 14.55-------------------

--------------------------------------------------*/

void SvxProxyTabPage::ReadConfigDefaults_Impl()
{
    try
    {
        Reference< beans::XPropertyState > xPropertyState(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);

        sal_Int32 nIntValue;
        OUString  aStringValue;

        if( xPropertyState->getPropertyDefault(aHttpProxyPN) >>= aStringValue )
        {
            aHttpProxyED.SetText( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(aHttpPortPN) >>= nIntValue )
        {
            aHttpPortED.SetText( String::CreateFromInt32( nIntValue ));
        }

        if( xPropertyState->getPropertyDefault(aFtpProxyPN) >>= aStringValue )
        {
            aFtpProxyED.SetText( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(aFtpPortPN) >>= nIntValue )
        {
            aFtpPortED.SetText( String::CreateFromInt32( nIntValue ));
        }

        if( xPropertyState->getPropertyDefault(aNoProxyDescPN) >>= aStringValue )
        {
            aNoProxyForED.SetText( aStringValue );
        }
    }
    catch(beans::UnknownPropertyException e)
    {
        OSL_TRACE( "SvxProxyTabPage::RestoreConfigDefaults_Impl: UnknownPropertyException caught" );
    }

    catch(com::sun::star::lang::WrappedTargetException e) {
        OSL_TRACE( "SvxProxyTabPage::RestoreConfigDefaults_Impl: WrappedTargetException caught" );
    }

    catch(RuntimeException e)
    {
        OSL_TRACE( "SvxProxyTabPage::RestoreConfigDefaults_Impl: RuntimeException caught" );
    }
}

/*-----------------02.09.04 14.55-------------------

--------------------------------------------------*/

void SvxProxyTabPage::RestoreConfigDefaults_Impl()
{
    try
    {
        Reference< beans::XPropertyState > xPropertyState(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);

        xPropertyState->setPropertyToDefault(aProxyModePN);
        xPropertyState->setPropertyToDefault(aHttpProxyPN);
        xPropertyState->setPropertyToDefault(aHttpPortPN);
        xPropertyState->setPropertyToDefault(aFtpProxyPN);
        xPropertyState->setPropertyToDefault(aFtpPortPN);
        xPropertyState->setPropertyToDefault(aNoProxyDescPN);

        Reference< util::XChangesBatch > xChangesBatch(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);
        xChangesBatch->commitChanges();
    }

    catch(beans::UnknownPropertyException e)
    {
        OSL_TRACE( "SvxProxyTabPage::RestoreConfigDefaults_Impl: UnknownPropertyException caught" );
    }

    catch(com::sun::star::lang::WrappedTargetException e) {
        OSL_TRACE( "SvxProxyTabPage::RestoreConfigDefaults_Impl: WrappedTargetException caught" );
    }

    catch(RuntimeException e)
    {
        OSL_TRACE( "SvxProxyTabPage::RestoreConfigDefaults_Impl: RuntimeException caught" );
    }
}

/*-----------------12.08.96 14.55-------------------

--------------------------------------------------*/

void SvxProxyTabPage::Reset(const SfxItemSet&)
{
    ReadConfigData_Impl();

    aProxyModeLB.SaveValue();
    aHttpProxyED.SaveValue();
    aHttpPortED.SaveValue();
    aFtpProxyED.SaveValue();
    aFtpPortED.SaveValue();
    aNoProxyForED.SaveValue();

    EnableControls_Impl( aProxyModeLB.GetSelectEntryPos() == 2 );
}

/*-----------------12.08.96 16.34-------------------

--------------------------------------------------*/

BOOL SvxProxyTabPage::FillItemSet(SfxItemSet& rSet)
{
    BOOL bModified=FALSE;

    try {
        Reference< beans::XPropertySet > xPropertySet(m_xConfigurationUpdateAccess, UNO_QUERY_THROW );

        USHORT nSelPos = aProxyModeLB.GetSelectEntryPos();
        if(aProxyModeLB.GetSavedValue() != nSelPos)
        {
            if( nSelPos == 1 )
            {
                RestoreConfigDefaults_Impl();
                return TRUE;
            }

            xPropertySet->setPropertyValue(aProxyModePN,
                makeAny((sal_Int32) nSelPos));
            bModified = TRUE;
        }

        if(aHttpProxyED.GetSavedValue() != aHttpProxyED.GetText())
        {
            xPropertySet->setPropertyValue( aHttpProxyPN,
                makeAny(rtl::OUString(aHttpProxyED.GetText())));
            bModified = TRUE;
        }

        if ( aHttpPortED.GetSavedValue() != aHttpPortED.GetText() )
        {
            xPropertySet->setPropertyValue( aHttpPortPN,
                makeAny(aHttpPortED.GetText().ToInt32()));
            bModified = TRUE;
        }

        if(aFtpProxyED.GetSavedValue() != aFtpProxyED.GetText())
        {
            xPropertySet->setPropertyValue( aFtpProxyPN,
                makeAny( rtl::OUString(aFtpProxyED.GetText())));
            bModified = TRUE;
        }

        if ( aFtpPortED.GetSavedValue() != aFtpPortED.GetText() )
        {
            xPropertySet->setPropertyValue( aFtpPortPN,
                makeAny(aFtpPortED.GetText().ToInt32()));
            bModified = TRUE;
        }

        if ( aNoProxyForED.GetSavedValue() != aNoProxyForED.GetText() )
        {
            xPropertySet->setPropertyValue( aNoProxyDescPN,
                makeAny( rtl::OUString(aNoProxyForED.GetText())));
            bModified = TRUE;
        }

        Reference< util::XChangesBatch > xChangesBatch(m_xConfigurationUpdateAccess, UNO_QUERY_THROW);
        xChangesBatch->commitChanges();
    }

    catch(com::sun::star::lang::IllegalArgumentException e) {
        OSL_TRACE( "SvxProxyTabPage::FillItemSet: IllegalArgumentException caught" );
    }

    catch(beans::UnknownPropertyException e) {
        OSL_TRACE( "SvxProxyTabPage::FillItemSet: UnknownPropertyException caught" );
    }

    catch(beans::PropertyVetoException e) {
        OSL_TRACE( "SvxProxyTabPage::FillItemSet: PropertyVetoException caught" );
    }

    catch(com::sun::star::lang::WrappedTargetException e) {
        OSL_TRACE( "SvxProxyTabPage::FillItemSet: WrappedTargetException caught" );
    }

    catch(RuntimeException e) {
        OSL_TRACE( "SvxProxyTabPage::FillItemSet: RuntimeException caught" );
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
    USHORT nPos = pBox->GetSelectEntryPos();

    // Restore original system values
    if( nPos == 1 )
    {
        ReadConfigDefaults_Impl();
    }

    EnableControls_Impl(nPos == 2);
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
/*  SvxSecurityTabPage                                             */
/*                                                                  */
/********************************************************************/

SvxSecurityTabPage::SvxSecurityTabPage( Window* pParent, const SfxItemSet& rSet )
    :SfxTabPage         ( pParent, SVX_RES( RID_SVXPAGE_INET_SECURITY ), rSet )
    ,maSecOptionsFL     ( this, ResId( FL_SEC_SECOPTIONS ) )
    ,maSecOptionsFI     ( this, ResId( FI_SEC_SECOPTIONS ) )
    ,maSaveOrSendDocsFI ( this, ResId( FI_SEC_SAVEORSENDDOCS ) )
    ,maSaveOrSendDocsCB ( this, ResId( CB_SEC_SAVEORSENDDOCS ) )
    ,maSignDocsFI       ( this, ResId( FI_SEC_SIGNDOCS ) )
    ,maSignDocsCB       ( this, ResId( CB_SEC_SIGNDOCS ) )
    ,maPrintDocsFI      ( this, ResId( FI_SEC_PRINTDOCS ) )
    ,maPrintDocsCB      ( this, ResId( CB_SEC_PRINTDOCS ) )
    ,maCreatePdfFI      ( this, ResId( FI_SEC_CREATEPDF ) )
    ,maCreatePdfCB      ( this, ResId( CB_SEC_CREATEPDF ) )
    ,maRemovePersInfoFI ( this, ResId( FI_SEC_REMOVEPERSINFO ) )
    ,maRemovePersInfoCB ( this, ResId( CB_SEC_REMOVEPERSINFO ) )
    ,maRecommPasswdFI   ( this, ResId( FI_SEC_RECOMMPASSWD ) )
    ,maRecommPasswdCB   ( this, ResId( CB_SEC_RECOMMPASSWD ) )
    ,maMacroSecFL       ( this, ResId( FL_SEC_MACROSEC ) )
    ,maMacroSecFI       ( this, ResId( FI_SEC_MACROSEC ) )
    ,maMacroSecPB       ( this, ResId( PB_SEC_MACROSEC ) )
    ,maFilesharingFL    ( this, ResId( FL_SEC_FILESHARING ) )
    ,maRecommReadOnlyCB ( this, ResId( CB_SEC_RECOMMREADONLY ) )
    ,maRecordChangesCB  ( this, ResId( CB_SEC_RECORDCHANGES ) )
    ,maProtectRecordsPB ( this, ResId( PB_SEC_PROTRECORDS ) )
    ,msProtectRecordsStr(       ResId( STR_SEC_PROTRECORDS ) )
    ,msUnprotectRecordsStr(     ResId( STR_SEC_UNPROTRECORDS ) )
    ,mpSecOptions       ( new SvtSecurityOptions )
    ,meRedlingMode      ( RL_NONE )
{
    FreeResource();

    InitControls();

    maMacroSecPB.SetClickHdl( LINK( this, SvxSecurityTabPage, MacroSecPBHdl ) );
    maProtectRecordsPB.SetClickHdl( LINK( this, SvxSecurityTabPage, ProtectRecordsPBHdl ) );
    maRecordChangesCB.SetClickHdl( LINK( this, SvxSecurityTabPage, RecordChangesCBHdl ) );

    ActivatePage( rSet );
}

SvxSecurityTabPage::~SvxSecurityTabPage()
{
    delete mpSecOptions;
}

IMPL_LINK( SvxSecurityTabPage, AdvancedPBHdl, void*, EMPTYARG )
{
    return 0;
}

IMPL_LINK( SvxSecurityTabPage, MacroSecPBHdl, void*, EMPTYARG )
{
    Reference< security::XDocumentDigitalSignatures > xD(
        comphelper::getProcessServiceFactory()->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.security.DocumentDigitalSignatures" ) ) ), UNO_QUERY );
    if ( xD.is() )
        xD->manageTrustedSources();

    return 0;
}

namespace
{
    enum RedlineFunc    { RF_ON, RF_PROTECT };

    const SfxBoolItem* ExecuteRecordChangesFunc( SvxSecurityTabPage::RedliningMode _eMode, RedlineFunc _eFunc, BOOL _bVal, Window* _pParent = NULL )
    {
        const SfxBoolItem* pRet = NULL;

        if( _eMode != SvxSecurityTabPage::RL_NONE )
        {
            USHORT nSlot;
            if ( _eMode == SvxSecurityTabPage::RL_WRITER )
                nSlot = ( _eFunc == RF_ON )? FN_REDLINE_ON : FN_REDLINE_PROTECT;
            else
                nSlot = ( _eFunc == RF_ON )? FID_CHG_RECORD : SID_CHG_PROTECT;

            // execute
            SfxViewShell* pViewSh = SfxViewShell::Current();
            if( pViewSh )
            {
                bool bNeedItem = ( _eMode == SvxSecurityTabPage::RL_WRITER || _eFunc != RF_ON );
                SfxBoolItem* pItem = bNeedItem ? new SfxBoolItem( nSlot, _bVal ) : NULL;
                SfxDispatcher* pDisp = pViewSh->GetDispatcher();
                if ( _pParent )
                {
                    OfaPtrItem aParentItem( SID_ATTR_PARENTWINDOW, _pParent );
                    pRet = static_cast< const SfxBoolItem* >(
                        pDisp->Execute( nSlot, SFX_CALLMODE_SYNCHRON, &aParentItem, pItem, 0L ) );
                }
                else
                    pRet = static_cast< const SfxBoolItem* >(
                        pDisp->Execute( nSlot, SFX_CALLMODE_SYNCHRON, pItem, 0L ) );
                delete pItem;
            }
        }

        return pRet;
    }

    bool QueryState( USHORT _nSlot, bool& _rValue )
    {
        bool bRet = false;

        SfxViewShell* pViewSh = SfxViewShell::Current();
        if( pViewSh )
        {
            const SfxPoolItem* pItem;
            SfxDispatcher* pDisp = pViewSh->GetDispatcher();
            bRet = SFX_ITEM_AVAILABLE <= pDisp->QueryState( _nSlot, pItem );
            if( bRet )
                _rValue = ( static_cast< const SfxBoolItem* >( pItem ) )->GetValue();
        }

        return bRet;
    }

    bool QueryRecordChangesProtectionState( SvxSecurityTabPage::RedliningMode _eMode, bool& _rValue )
    {
        bool bRet = false;

        if( _eMode != SvxSecurityTabPage::RL_NONE )
        {
            USHORT nSlot = ( _eMode == SvxSecurityTabPage::RL_WRITER )? FN_REDLINE_PROTECT : SID_CHG_PROTECT;
            bRet = QueryState( nSlot, _rValue );
        }

        return bRet;
    }

    bool QueryRecordChangesState( SvxSecurityTabPage::RedliningMode _eMode, bool& _rValue )
    {
        bool bRet = false;

        if( _eMode != SvxSecurityTabPage::RL_NONE )
        {
            USHORT nSlot = ( _eMode == SvxSecurityTabPage::RL_WRITER )? FN_REDLINE_ON : FID_CHG_RECORD;
            bRet = QueryState( nSlot, _rValue );
        }

        return bRet;
    }
}

IMPL_LINK( SvxSecurityTabPage, RecordChangesCBHdl, void*, EMPTYARG )
{
    const SfxBoolItem* pItem =
        ExecuteRecordChangesFunc( meRedlingMode, RF_ON, maRecordChangesCB.IsChecked(), this );
    CheckRecordChangesState();
    return 0;
}

IMPL_LINK( SvxSecurityTabPage, ProtectRecordsPBHdl, void*, EMPTYARG )
{
    bool bProt;
    QueryRecordChangesProtectionState( meRedlingMode, bProt );
    const SfxBoolItem* pItem = ExecuteRecordChangesFunc( meRedlingMode, RF_PROTECT, !bProt, this );
    CheckRecordChangesState();

    if ( QueryRecordChangesProtectionState( meRedlingMode, bProt ) )
    {
        // RecordChangesCB is enabled if protection is off
        maRecordChangesCB.Enable( !bProt );
        // toggle text of button "Protect" <-> "Unprotect"
        String sNewText = bProt ? msUnprotectRecordsStr : msProtectRecordsStr;
        maProtectRecordsPB.SetText( sNewText );
    }
    return 0;
}

void SvxSecurityTabPage::CheckRecordChangesState( void )
{
    bool bVal;
    if( QueryRecordChangesState( meRedlingMode, bVal ) )
    {
        maRecordChangesCB.Enable();
        maRecordChangesCB.Check( bVal );
    }
    else
        maRecordChangesCB.Disable();        // because now we don't know the state!

    maProtectRecordsPB.Enable( QueryRecordChangesProtectionState( meRedlingMode, bVal ) );
}

void SvxSecurityTabPage::InitControls()
{
    // if the button text is too wide, then broaden the button
    const long nOffset = 10;
    String sText = maMacroSecPB.GetText();
    long nTxtW = maMacroSecPB.GetTextWidth( sText );
    if ( sText.Search( '~' ) == STRING_NOTFOUND )
        nTxtW += nOffset;
    long nBtnW = maMacroSecPB.GetSizePixel().Width();
    if ( nTxtW > nBtnW )
    {
        // broaden the button
        long nDelta = nTxtW - nBtnW;
        Size aNewSize = maMacroSecPB.GetSizePixel();
        aNewSize.Width() += nDelta;
        maMacroSecPB.SetSizePixel( aNewSize );
        Point aNewPos = maMacroSecPB.GetPosPixel();
        aNewPos.X() -= nDelta;
        maMacroSecPB.SetPosPixel( aNewPos );
        // and narrow the fixedtext of the left side
        aNewSize = maMacroSecFI.GetSizePixel();
        aNewSize.Width() -= nDelta;
        maMacroSecFI.SetSizePixel( aNewSize );
    }

    long nTxtW1 = maProtectRecordsPB.GetTextWidth( msProtectRecordsStr );
    if ( msProtectRecordsStr.Search( '~' ) == STRING_NOTFOUND )
        nTxtW1 += nOffset;
    long nTxtW2 = maProtectRecordsPB.GetTextWidth( msUnprotectRecordsStr );
    if ( msUnprotectRecordsStr.Search( '~' ) == STRING_NOTFOUND )
        nTxtW2 += nOffset;
    nTxtW = Max( nTxtW1, nTxtW2 );
    nBtnW = maProtectRecordsPB.GetSizePixel().Width();
    if ( nTxtW > nBtnW )
    {
        // broaden the button
        long nDelta = nTxtW - nBtnW;
        Size aNewSize = maProtectRecordsPB.GetSizePixel();
        aNewSize.Width() += nDelta;
        maProtectRecordsPB.SetSizePixel( aNewSize );
    }
}

SfxTabPage* SvxSecurityTabPage::Create(Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxSecurityTabPage(pParent, rAttrSet);
}

void SvxSecurityTabPage::ActivatePage( const SfxItemSet& rSet )
{
}

int SvxSecurityTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

namespace
{
/*    bool Enable( const SvtSecurityOptions& _rOpt, SvtSecurityOptions::EOption _eOpt, Control& _rCtrl, FixedImage& _rImg )
    {
        bool    b = _rOpt.IsOptionEnabled( _eOpt );
        _rCtrl.Enable( b );
        _Img.Show( !b );
        return b;
    }
*/
    bool EnableAndSet( const SvtSecurityOptions& _rOpt, SvtSecurityOptions::EOption _eOpt,
                                                        CheckBox& _rCtrl, FixedImage& _rImg )
    {
//        bool    b = Enable( _rOpt, _eOpt, _rCtrl, _rImg );
        bool    b = _rOpt.IsOptionEnabled( _eOpt );
        _rCtrl.Enable( b );
        _rImg.Show( !b );
        if( b )
            _rCtrl.Check( _rOpt.IsOptionSet( _eOpt ) );
        return b;
    }

    bool CheckAndSave( SvtSecurityOptions& _rOpt, SvtSecurityOptions::EOption _eOpt, const CheckBox& _rCtrl, bool& _rModfied )
    {
        bool    bModified = false;
        if( _rOpt.IsOptionEnabled( _eOpt ) )
        {
            bool    bNew = _rCtrl.IsChecked();
            bModified = _rOpt.IsOptionSet( _eOpt ) != bNew;
            if( bModified )
            {
                _rOpt.SetOption( _eOpt, bNew );
                _rModfied = true;
            }
        }

        return bModified;
    }
}

BOOL SvxSecurityTabPage::FillItemSet( SfxItemSet& _rSet )
{
    bool bModified = false;

    CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_SAVEORSEND, maSaveOrSendDocsCB, bModified );
    CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_SIGNING, maSignDocsCB, bModified );
    CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_PRINT, maPrintDocsCB, bModified );
    CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_CREATEPDF, maCreatePdfCB, bModified );
    CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_REMOVEPERSONALINFO, maRemovePersInfoCB, bModified );
    CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_RECOMMENDPASSWORD, maRecommPasswdCB, bModified );

    // document options
    SfxObjectShell* pCurDocShell = SfxObjectShell::Current();
    if( pCurDocShell )
    {
        if( pCurDocShell->HasSecurityOptOpenReadOnly() )
            pCurDocShell->SetSecurityOptOpenReadOnly( maRecommReadOnlyCB.IsChecked() );

    }

    return bModified;
}

/*--------------------------------------------------------------------*/

void SvxSecurityTabPage::Reset( const SfxItemSet& _rSet )
{
    EnableAndSet( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_SAVEORSEND, maSaveOrSendDocsCB, maSaveOrSendDocsFI );
    EnableAndSet( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_SIGNING, maSignDocsCB, maSignDocsFI );
    EnableAndSet( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_PRINT, maPrintDocsCB, maPrintDocsFI );
    EnableAndSet( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_CREATEPDF, maCreatePdfCB, maCreatePdfFI );
    EnableAndSet( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_REMOVEPERSONALINFO, maRemovePersInfoCB, maRemovePersInfoFI );
    EnableAndSet( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_RECOMMENDPASSWORD, maRecommPasswdCB, maRecommPasswdFI );

    String sNewText = msProtectRecordsStr;
    SfxObjectShell* pCurDocShell = SfxObjectShell::Current();
    if( pCurDocShell )
    {
        bool bIsHTMLDoc = false;
        SfxViewShell* pViewSh = SfxViewShell::Current();
        if( pViewSh )
        {
            const SfxPoolItem* pItem;
            SfxDispatcher* pDisp = pViewSh->GetDispatcher();
            if ( SFX_ITEM_AVAILABLE <= pDisp->QueryState( SID_HTML_MODE, pItem ) )
            {
                USHORT nMode = static_cast< const SfxUInt16Item* >( pItem )->GetValue();
                bIsHTMLDoc = ( ( nMode & HTMLMODE_ON ) != 0 );
            }
        }

        sal_Bool bIsReadonly = pCurDocShell->IsReadOnly();
        if( pCurDocShell->HasSecurityOptOpenReadOnly() && !bIsHTMLDoc )
        {
            maRecommReadOnlyCB.Check( pCurDocShell->IsSecurityOptOpenReadOnly() );
//!         maRecommReadOnlyCB.Enable( !bIsReadonly );
        }
        else
            maRecommReadOnlyCB.Disable();

        bool bVal;
        if ( QueryRecordChangesState( RL_WRITER, bVal ) && !bIsHTMLDoc )
            meRedlingMode = RL_WRITER;
        else if( QueryRecordChangesState( RL_CALC, bVal ) )
            meRedlingMode = RL_CALC;
        else
            meRedlingMode = RL_NONE;

        if ( meRedlingMode != RL_NONE )
        {
            maRecordChangesCB.Check( bVal );
            maRecordChangesCB.Enable( !bVal && !bIsReadonly );
            maProtectRecordsPB.Enable(
                QueryRecordChangesProtectionState( meRedlingMode, bVal ) && !bIsReadonly );
            // set the right text
            if ( bVal )
                sNewText = msUnprotectRecordsStr;
        }
        else
        {
            // only Writer and Calc support redlining
            maRecordChangesCB.Disable();
            maProtectRecordsPB.Disable();
        }
    }
    else
    {   // no doc -> hide document settings
        maRecommReadOnlyCB.Disable();
        maRecordChangesCB.Disable();
        maProtectRecordsPB.Disable();
    }

    maProtectRecordsPB.SetText( sNewText );
}

//added by jmeng begin
MozPluginTabPage::MozPluginTabPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage( pParent, ResId( RID_SVXPAGE_INET_MOZPLUGIN, DIALOG_MGR() ), rSet ),
    aMSWordGB       ( this, ResId( GB_MOZPLUGIN     ) ),
    aWBasicCodeCB   ( this, ResId( CB_MOZPLUGIN_CODE ) )
{
    FreeResource();
}

MozPluginTabPage::~MozPluginTabPage()
{
}

SfxTabPage* MozPluginTabPage::Create( Window* pParent,
                                        const SfxItemSet& rAttrSet )
{
    return new MozPluginTabPage( pParent, rAttrSet );
}
BOOL MozPluginTabPage::FillItemSet( SfxItemSet& )
{
    BOOL hasInstall = isInstalled();
    BOOL hasChecked = aWBasicCodeCB.IsChecked();
    if(hasInstall && (!hasChecked)){
        //try to uninstall
        uninstallPlugin();
    }
    else if((!hasInstall) && hasChecked){
        //try to install
        installPlugin();
    }
    else{
        // do nothing
    }
    return TRUE;
}
void MozPluginTabPage::Reset( const SfxItemSet& )
{
        aWBasicCodeCB.Check( isInstalled());
        aWBasicCodeCB.SaveValue();
}

#ifdef WNT
extern "C" {
    int lc_isInstalled(const  char* realFilePath);
    int lc_installPlugin(const  char* realFilePath);
    int lc_uninstallPlugin(const  char* realFilePath);
}
#endif

#define NPP_PATH_MAX 2048
inline ::rtl::OString getDllURL( void )
{
//    ::rtl::OUString libPath(rtl::OUString::createFromAscii("libcui680li.so"));
    ::rtl::OUString dirPath/*dllPath, */;
    osl::Module::getUrlFromAddress((void*)&getDllURL, dirPath);
    dirPath = dirPath.copy(0, dirPath.lastIndexOf('/'));
//    osl::FileBase::getAbsoluteFileURL(dirPath, libPath, dllPath);
    ::rtl::OUString sysDirPath;
    osl::FileBase::getSystemPathFromFileURL(dirPath, sysDirPath);
    ::rtl::OString oSysDirPath;
    oSysDirPath = OUStringToOString(sysDirPath, RTL_TEXTENCODING_ASCII_US);
    return oSysDirPath;
}

BOOL MozPluginTabPage::isInstalled()
{
#ifdef UNIX
    // get the real file referred by .so lnk file
    char lnkFilePath[NPP_PATH_MAX] = {0};
    char lnkReferFilePath[NPP_PATH_MAX] = {0};
    char* pHome = getpwuid(getuid())->pw_dir;
    strcat(lnkFilePath, pHome);
    strcat(lnkFilePath, "/.mozilla/plugins/libnpsoplugin" SAL_DLLEXTENSION);

    struct stat sBuf;
    if (0 > lstat(lnkFilePath, &sBuf))
        return false;
    if (!S_ISLNK(sBuf.st_mode))
        return false;
    if (0 >= readlink(lnkFilePath, lnkReferFilePath, NPP_PATH_MAX))
        return false;
    // If the link is relative, then we regard it as non-standard
    if (lnkReferFilePath[0] != '/')
        return false;

    // get the real file path
    char realFilePath[NPP_PATH_MAX] = {0};
    ::rtl::OString tempString;
    tempString = getDllURL();
    strncpy(realFilePath, tempString.getStr(), NPP_PATH_MAX);
    strcat(realFilePath, "/libnpsoplugin" SAL_DLLEXTENSION);

    if (0 != strcmp(lnkReferFilePath, realFilePath))
        return false;
    return true;
#endif
#ifdef WNT
    // get the value from registry
        BOOL ret = true;
    ::rtl::OString tempString;
    char realFilePath[NPP_PATH_MAX] = {0};
    tempString = getDllURL();
    strncpy(realFilePath, tempString.getStr(), NPP_PATH_MAX);
    if(! lc_isInstalled(realFilePath))
        ret =true;
    else
        ret = false;
    return ret;
#endif
}

BOOL MozPluginTabPage::installPlugin()
{
#ifdef UNIX
    // get the real file referred by .so lnk file
    char lnkFilePath[NPP_PATH_MAX] = {0};
    char* pHome = getpwuid(getuid())->pw_dir;
    strcat(lnkFilePath, pHome);
    strcat(lnkFilePath, "/.mozilla/plugins/libnpsoplugin" SAL_DLLEXTENSION);
    remove(lnkFilePath);

    // create the dirs if necessary
    struct stat buf;
    char tmpDir[NPP_PATH_MAX] = {0};
    sprintf(tmpDir, "%s/.mozilla", pHome);
    if (0 > stat(lnkFilePath, &buf))
    {
        mkdir(tmpDir, 0755);
        strcat(tmpDir, "/plugins");
        mkdir(tmpDir, 0755);
    }

    // get the real file path
    char realFilePath[NPP_PATH_MAX] = {0};
    ::rtl::OString tempString;
    tempString = getDllURL();
    strncpy(realFilePath, tempString.getStr(), NPP_PATH_MAX);
    strcat(realFilePath, "/libnpsoplugin" SAL_DLLEXTENSION);

    // create the link
    if (0 != symlink(realFilePath, lnkFilePath))
        return false;
    return true;
#endif
#ifdef WNT
    ::rtl::OString tempString;
    char realFilePath[NPP_PATH_MAX] = {0};
    tempString = getDllURL();
    strncpy(realFilePath, tempString.getStr(), NPP_PATH_MAX);
    if( !lc_installPlugin(realFilePath))
        return true;
    else
        return false;
#endif
}

BOOL MozPluginTabPage::uninstallPlugin()
{
#ifdef UNIX
    // get the real file referred by .so lnk file
    char lnkFilePath[NPP_PATH_MAX] = {0};
    char* pHome = getpwuid(getuid())->pw_dir;
    strcat(lnkFilePath, pHome);
    strcat(lnkFilePath, "/.mozilla/plugins/libnpsoplugin" SAL_DLLEXTENSION);

    if(0 > remove(lnkFilePath))
        return false;
    return true;
#endif
#ifdef WNT
    ::rtl::OString tempString;
    char realFilePath[NPP_PATH_MAX] = {0};
    tempString = getDllURL();
    strncpy(realFilePath, tempString.getStr(), NPP_PATH_MAX);
    if(!lc_uninstallPlugin(realFilePath))
        return true;
    else
        return false;
#endif
}
//added by jmeng end

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

    aMailerURLPB.SetClickHdl( LINK( this, SvxEMailTabPage, FileDialogHdl_Impl ) );

    // FixedText not wide enough?
    long nTxtW = aMailerURLFT.GetCtrlTextWidth( aMailerURLFT.GetText() );
    long nCtrlW = aMailerURLFT.GetSizePixel().Width();
    if ( nTxtW >= nCtrlW )
    {
        long nDelta = Max( (long)10, nTxtW - nCtrlW );
        // so FixedText wider
        Size aNewSz = aMailerURLFT.GetSizePixel();
        aNewSz.Width() += nDelta;
        aMailerURLFT.SetSizePixel( aNewSz );
        // and Edit smaller
        aNewSz = aMailerURLED.GetSizePixel();
        aNewSz.Width() -= nDelta;
        Point aNewPt = aMailerURLED.GetPosPixel();
        aNewPt.X() += nDelta;
        aMailerURLED.SetPosSizePixel( aNewPt, aNewSz );
    }
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
