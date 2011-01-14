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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------
#include <tools/shl.hxx>
#include <tools/config.hxx>
#include <vcl/msgbox.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/slstitm.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/filedlghelper.hxx>
#include <tools/inetdef.hxx>
#include <svl/urihelper.hxx>
#include <svl/cntwids.hrc>
#include <unotools/inetoptions.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/bootstrap.hxx>
#include <vcl/help.hxx>
#include <sfx2/viewfrm.hxx>
#include <tools/urlobj.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/securityoptions.hxx>
#include <unotools/javaoptions.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/extendedsecurityoptions.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#define _SVX_OPTINET2_CXX
#include <dialmgr.hxx>
#include "optinet2.hxx"
#include <svx/svxdlg.hxx> //CHINA001
#include <cuires.hrc>
#include "optinet2.hrc"
#include "helpid.hrc"
#include <svx/ofaitem.hxx>
#include <svx/htmlmode.hxx>
#include <svx/svxids.hrc> // slot ids, mostly for changetracking

// for security TP
#include <comphelper/processfactory.hxx>
#include <com/sun/star/security/XDocumentDigitalSignatures.hpp>

#ifdef UNX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <string.h>
#include <rtl/textenc.h>
#include <rtl/locale.h>
#include <osl/nlsupport.h>
#endif
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <osl/process.h>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <comphelper/processfactory.hxx>

#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include "com/sun/star/task/XMasterPasswordHandling2.hpp"
#include "com/sun/star/task/XPasswordContainer.hpp"
#include "securityoptions.hxx"
#include "webconninfo.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::sfx2;
using ::rtl::OUString;

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

void SvxNoSpaceEdit::KeyInput( const KeyEvent& rKEvent )
{
    if ( bOnlyNumeric )
    {
        const KeyCode& rKeyCode = rKEvent.GetKeyCode();
        sal_uInt16 nGroup = rKeyCode.GetGroup();
        sal_uInt16 nKey = rKeyCode.GetCode();
        sal_Bool bValid = ( KEYGROUP_NUM == nGroup || KEYGROUP_CURSOR == nGroup ||
                        ( KEYGROUP_MISC == nGroup && ( nKey < KEY_ADD || nKey > KEY_EQUAL ) ) );
        if ( !bValid && ( rKeyCode.IsMod1() && (
             KEY_A == nKey || KEY_C == nKey || KEY_V == nKey || KEY_X == nKey || KEY_Z == nKey ) ) )
            // Erase, Copy, Paste, Select All und Undo soll funktionieren
            bValid = sal_True;

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
            // der H�chstwert einer Portnummer ist USHRT_MAX
            ErrorBox( this, CUI_RES( RID_SVXERR_OPT_PROXYPORTS ) ).Execute();
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
    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_INET_PROXY ), rSet ),
    aOptionGB   (this, CUI_RES(GB_SETTINGS)),

    aProxyModeFT  (this, CUI_RES(FT_PROXYMODE)),
    aProxyModeLB  (this, CUI_RES(LB_PROXYMODE)),

    aHttpProxyFT      (this, CUI_RES( FT_HTTP_PROXY   )),
    aHttpProxyED      (this, CUI_RES( ED_HTTP_PROXY     )),
    aHttpPortFT       (this, CUI_RES( FT_HTTP_PORT      )),
    aHttpPortED       (this, CUI_RES( ED_HTTP_PORT      ), sal_True),

    aHttpsProxyFT      (this, CUI_RES( FT_HTTPS_PROXY     )),
    aHttpsProxyED      (this, CUI_RES( ED_HTTPS_PROXY     )),
    aHttpsPortFT       (this, CUI_RES( FT_HTTPS_PORT      )),
    aHttpsPortED       (this, CUI_RES( ED_HTTPS_PORT      ), sal_True),

    aFtpProxyFT       (this, CUI_RES( FT_FTP_PROXY      )),
    aFtpProxyED       (this, CUI_RES( ED_FTP_PROXY      )),
    aFtpPortFT        (this, CUI_RES( FT_FTP_PORT       )),
    aFtpPortED        (this, CUI_RES( ED_FTP_PORT       ), sal_True),

    aNoProxyForFT     (this, CUI_RES( FT_NOPROXYFOR     )),
    aNoProxyForED     (this, CUI_RES( ED_NOPROXYFOR     )),
    aNoProxyDescFT    (this, CUI_RES( ED_NOPROXYDESC    )),
    sFromBrowser        (       CUI_RES( ST_PROXY_FROM_BROWSER ) ),
    aProxyModePN(RTL_CONSTASCII_USTRINGPARAM("ooInetProxyType")),
    aHttpProxyPN(RTL_CONSTASCII_USTRINGPARAM("ooInetHTTPProxyName")),
    aHttpPortPN(RTL_CONSTASCII_USTRINGPARAM("ooInetHTTPProxyPort")),
    aHttpsProxyPN(RTL_CONSTASCII_USTRINGPARAM("ooInetHTTPSProxyName")),
    aHttpsPortPN(RTL_CONSTASCII_USTRINGPARAM("ooInetHTTPSProxyPort")),
    aFtpProxyPN(RTL_CONSTASCII_USTRINGPARAM("ooInetFTPProxyName")),
    aFtpPortPN(RTL_CONSTASCII_USTRINGPARAM("ooInetFTPProxyPort")),
    aNoProxyDescPN(RTL_CONSTASCII_USTRINGPARAM("ooInetNoProxy"))
{
    FreeResource();

    aHttpPortED.SetMaxTextLen(5);
    aHttpsPortED.SetMaxTextLen(5);
    aFtpPortED.SetMaxTextLen(5);
    Link aLink = LINK( this, SvxProxyTabPage, LoseFocusHdl_Impl );
    aHttpPortED.SetLoseFocusHdl( aLink );
    aHttpsPortED.SetLoseFocusHdl( aLink );
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
        catch ( RuntimeException& )
        {
        }
    }

    ArrangeControls_Impl();
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

        sal_Int32 nIntValue = 0;
        OUString  aStringValue;

        if( xNameAccess->getByName(aProxyModePN) >>= nIntValue )
        {
            aProxyModeLB.SelectEntryPos( (sal_uInt16) nIntValue );
        }

        if( xNameAccess->getByName(aHttpProxyPN) >>= aStringValue )
        {
            aHttpProxyED.SetText( aStringValue );
        }

        if( xNameAccess->getByName(aHttpPortPN) >>= nIntValue )
        {
            aHttpPortED.SetText( String::CreateFromInt32( nIntValue ));
        }

        if( xNameAccess->getByName(aHttpsProxyPN) >>= aStringValue )
        {
            aHttpsProxyED.SetText( aStringValue );
        }

        if( xNameAccess->getByName(aHttpsPortPN) >>= nIntValue )
        {
            aHttpsPortED.SetText( String::CreateFromInt32( nIntValue ));
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

        sal_Int32 nIntValue = 0;
        OUString  aStringValue;

        if( xPropertyState->getPropertyDefault(aHttpProxyPN) >>= aStringValue )
        {
            aHttpProxyED.SetText( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(aHttpPortPN) >>= nIntValue )
        {
            aHttpPortED.SetText( String::CreateFromInt32( nIntValue ));
        }

        if( xPropertyState->getPropertyDefault(aHttpsProxyPN) >>= aStringValue )
        {
            aHttpsProxyED.SetText( aStringValue );
        }

        if( xPropertyState->getPropertyDefault(aHttpsPortPN) >>= nIntValue )
        {
            aHttpsPortED.SetText( String::CreateFromInt32( nIntValue ));
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
        xPropertyState->setPropertyToDefault(aHttpsProxyPN);
        xPropertyState->setPropertyToDefault(aHttpsPortPN);
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
    aHttpsProxyED.SaveValue();
    aHttpsPortED.SaveValue();
    aFtpProxyED.SaveValue();
    aFtpPortED.SaveValue();
    aNoProxyForED.SaveValue();

    EnableControls_Impl( aProxyModeLB.GetSelectEntryPos() == 2 );
}

/*-----------------12.08.96 16.34-------------------

--------------------------------------------------*/

sal_Bool SvxProxyTabPage::FillItemSet(SfxItemSet& )
{
    sal_Bool bModified=sal_False;

    try {
        Reference< beans::XPropertySet > xPropertySet(m_xConfigurationUpdateAccess, UNO_QUERY_THROW );

        sal_uInt16 nSelPos = aProxyModeLB.GetSelectEntryPos();
        if(aProxyModeLB.GetSavedValue() != nSelPos)
        {
            if( nSelPos == 1 )
            {
                RestoreConfigDefaults_Impl();
                return sal_True;
            }

            xPropertySet->setPropertyValue(aProxyModePN,
                makeAny((sal_Int32) nSelPos));
            bModified = sal_True;
        }

        if(aHttpProxyED.GetSavedValue() != aHttpProxyED.GetText())
        {
            xPropertySet->setPropertyValue( aHttpProxyPN,
                makeAny(rtl::OUString(aHttpProxyED.GetText())));
            bModified = sal_True;
        }

        if ( aHttpPortED.GetSavedValue() != aHttpPortED.GetText() )
        {
            xPropertySet->setPropertyValue( aHttpPortPN,
                makeAny(aHttpPortED.GetText().ToInt32()));
            bModified = sal_True;
        }

        if(aHttpsProxyED.GetSavedValue() != aHttpsProxyED.GetText())
        {
            xPropertySet->setPropertyValue( aHttpsProxyPN,
                makeAny(rtl::OUString(aHttpsProxyED.GetText())));
            bModified = sal_True;
        }

        if ( aHttpsPortED.GetSavedValue() != aHttpsPortED.GetText() )
        {
            xPropertySet->setPropertyValue( aHttpsPortPN,
                makeAny(aHttpsPortED.GetText().ToInt32()));
            bModified = sal_True;
        }

        if(aFtpProxyED.GetSavedValue() != aFtpProxyED.GetText())
        {
            xPropertySet->setPropertyValue( aFtpProxyPN,
                makeAny( rtl::OUString(aFtpProxyED.GetText())));
            bModified = sal_True;
        }

        if ( aFtpPortED.GetSavedValue() != aFtpPortED.GetText() )
        {
            xPropertySet->setPropertyValue( aFtpPortPN,
                makeAny(aFtpPortED.GetText().ToInt32()));
            bModified = sal_True;
        }

        if ( aNoProxyForED.GetSavedValue() != aNoProxyForED.GetText() )
        {
            xPropertySet->setPropertyValue( aNoProxyDescPN,
                makeAny( rtl::OUString(aNoProxyForED.GetText())));
            bModified = sal_True;
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

void SvxProxyTabPage::ArrangeControls_Impl()
{
    //-->Calculate dynamical width of controls, add buy wuy for i71445 Time: 2007.02.27
    long nWidth = aProxyModeFT.GetCtrlTextWidth( aProxyModeFT.GetText() );
    long nTemp = aHttpProxyFT.GetCtrlTextWidth( aHttpProxyFT.GetText() );
    if ( nTemp > nWidth )
        nWidth = nTemp;
    nTemp = aHttpsProxyFT.GetCtrlTextWidth( aHttpsProxyFT.GetText() );
    if ( nTemp > nWidth )
        nWidth = nTemp;
    nTemp = aFtpProxyFT.GetCtrlTextWidth( aFtpProxyFT.GetText() );
    if ( nTemp > nWidth )
        nWidth = nTemp;
    nTemp = aNoProxyForFT.GetCtrlTextWidth( aNoProxyForFT.GetText() );
    if ( nTemp > nWidth )
        nWidth = nTemp;

    nWidth += 10; // To be sure the length of the FixedText is enough on all platforms
    const long nFTWidth = aProxyModeFT.GetSizePixel().Width();
    if ( nWidth > nFTWidth )
    {
        Size aNewSize = aProxyModeFT.GetSizePixel();
        aNewSize.Width() = nWidth;

        aProxyModeFT.SetSizePixel( aNewSize );
        aHttpProxyFT.SetSizePixel( aNewSize );
        aHttpsProxyFT.SetSizePixel( aNewSize );
        aFtpProxyFT.SetSizePixel( aNewSize );
        aNoProxyForFT.SetSizePixel( aNewSize );

        const long nDelta = nWidth - nFTWidth;
        Point aNewPos = aProxyModeLB.GetPosPixel();
        aNewPos.X() += nDelta;

        aProxyModeLB.SetPosPixel( aNewPos );

        aNewSize = aHttpProxyED.GetSizePixel();
        aNewSize.Width() -= nDelta;

        aNewPos.Y() = aHttpProxyED.GetPosPixel().Y();
        aHttpProxyED.SetPosSizePixel( aNewPos, aNewSize );
        aNewPos.Y() = aHttpsProxyED.GetPosPixel().Y();
        aHttpsProxyED.SetPosSizePixel( aNewPos, aNewSize );
        aNewPos.Y() = aFtpProxyED.GetPosPixel().Y();
        aFtpProxyED.SetPosSizePixel( aNewPos, aNewSize );
        aNewPos.Y() = aNoProxyForED.GetPosPixel().Y();
        aNoProxyForED.SetPosSizePixel( aNewPos, aNewSize );
    }
    //<--End buy wuy for i71445 Time: 2007.02.27
}

/*-----------------12.08.96 13.38-------------------

--------------------------------------------------*/
void SvxProxyTabPage::EnableControls_Impl(sal_Bool bEnable)
{
    aHttpProxyFT.Enable(bEnable);
    aHttpProxyED.Enable(bEnable);
    aHttpPortFT.Enable(bEnable);
    aHttpPortED.Enable(bEnable);

    aHttpsProxyFT.Enable(bEnable);
    aHttpsProxyED.Enable(bEnable);
    aHttpsPortFT.Enable(bEnable);
    aHttpsPortED.Enable(bEnable);

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
    sal_uInt16 nPos = pBox->GetSelectEntryPos();

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

    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_INET_SEARCH ), rSet ),

    aSearchGB       ( this, CUI_RES( GB_SEARCH ) ),
    aSearchLB       ( this, CUI_RES( LB_SEARCH ) ),
    aSearchNameFT   ( this, CUI_RES( FT_SEARCH_NAME ) ),
    aSearchNameED   ( this, CUI_RES( ED_SEARCH_NAME ) ),
    aSearchFT       ( this, CUI_RES( FT_SEARCH ) ),
    aAndRB          ( this, CUI_RES( RB_AND ) ),
    aOrRB           ( this, CUI_RES( RB_OR ) ),
    aExactRB        ( this, CUI_RES( RB_EXACT ) ),

    aURLFT          ( this, CUI_RES( FT_URL ) ),
    aURLED          ( this, CUI_RES( ED_URL ) ),

    aPostFixFT      ( this, CUI_RES( FT_POSTFIX ) ),
    aPostFixED      ( this, CUI_RES( ED_POSTFIX ) ),
    aSeparatorFT    ( this, CUI_RES( FT_SEPARATOR ) ),
    aSeparatorED    ( this, CUI_RES( ED_SEPARATOR ) ),
    aCaseFT         ( this, CUI_RES( FT_CASE ) ),
    aCaseED         ( this, CUI_RES( ED_CASE ) ),

    aNewPB          ( this, CUI_RES( PB_NEW ) ),
    aAddPB          ( this, CUI_RES( PB_ADD ) ),
    aChangePB       ( this, CUI_RES( PB_CHANGE ) ),
    aDeletePB       ( this, CUI_RES( PB_DELETE ) ),

    sModifyMsg(CUI_RES(MSG_MODIFY))
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

    InitControls_Impl();
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

sal_Bool SvxSearchTabPage::FillItemSet( SfxItemSet&  )
{
    if(aSearchConfig.IsModified())
        aSearchConfig.Commit();
    return sal_True;
}
/*--------------------------------------------------------------------*/

void SvxSearchTabPage::ActivatePage( const SfxItemSet&  )
{
}

/*--------------------------------------------------------------------*/

int SvxSearchTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    //Modified by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)
    if(!ConfirmLeave(String()))
        return KEEP_PAGE;

    if ( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

//add by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)
sal_Bool SvxSearchTabPage::ConfirmLeave( const String& rStringSelection)
{
    if(aChangePB.IsEnabled())
    {
        QueryBox aQuery(this, WB_YES_NO_CANCEL|WB_DEF_YES, sModifyMsg);
        sal_uInt16 nRet = aQuery.Execute();
        if(RET_CANCEL == nRet)
        {
            if(rStringSelection.Len())
                aSearchLB.SelectEntry(sLastSelectedEntry);
            return sal_False;
        }
        else if(RET_YES == nRet)
        {
            sal_uInt16 nEntryPos = aSearchLB.GetEntryPos( aSearchNameED.GetText() );
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
            aChangePB.Enable(sal_False);
            aAddPB.Enable(sal_False);
            SearchEntryHdl_Impl(&aSearchLB);
        }
    }
    if(aAddPB.IsEnabled())
    {
        QueryBox aQuery(this, WB_YES_NO_CANCEL|WB_DEF_YES, sModifyMsg);
        sal_uInt16 nRet = aQuery.Execute();
        if(RET_CANCEL == nRet)
        {
            aSearchLB.SetNoSelection();
            return sal_False;
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
            aAddPB.Enable(sal_False);
            aChangePB.Enable(sal_False);
            NewSearchHdl_Impl(0);
        }

    }
    return sal_True;
}

// -----------------------------------------------------------------------

void SvxSearchTabPage::InitControls_Impl()
{
    // detect longest label text
    sal_Int32 i = 0;
    long nLabelTextWidth = 0;
    Window* pLabels[] = { &aSearchNameFT, &aSearchFT, &aURLFT, &aPostFixFT, &aSeparatorFT, &aCaseFT };
    Window** pLabel = pLabels;
    const sal_Int32 nLabelCount = sizeof( pLabels ) / sizeof( pLabels[0] );
    for ( ; i < nLabelCount; ++i, ++pLabel )
    {
        long nTemp = (*pLabel)->GetCtrlTextWidth( (*pLabel)->GetText() );
        if ( nTemp > nLabelTextWidth )
            nLabelTextWidth = nTemp;
    }

    // resize all labels
    nLabelTextWidth = nLabelTextWidth * 120 / 100; // additional space looks better
    const long nLabelWidth = aSearchNameFT.GetSizePixel().Width();
    const long nDelta = nLabelWidth - nLabelTextWidth;
    pLabel = pLabels;
    for ( i = 0; i < nLabelCount; ++i, ++pLabel )
    {
        Size aNewSize = (*pLabel)->GetSizePixel();
        aNewSize.Width() += nDelta;
        (*pLabel)->SetSizePixel( aNewSize );
    }

    // resize and move the edits
    Window* pEdits[] = { &aSearchNameED, &aAndRB, &aOrRB,
        &aExactRB, &aURLED, &aPostFixED, &aSeparatorED, &aCaseED };
    Window** pEdit = pEdits;
    const sal_Int32 nCCount = sizeof( pEdits ) / sizeof( pEdits[ 0 ] );
    for ( i = 0; i < nCCount; ++i, ++pEdit )
    {
        Point aNewPos = (*pEdit)->GetPosPixel();
        aNewPos.X() -= nDelta;
        Size aNewSize = (*pEdit)->GetSizePixel();
        if ( (*pEdit) != &aSeparatorED && (*pEdit) != &aCaseED )
            aNewSize.Width() += nDelta;
        (*pEdit)->SetPosSizePixel( aNewPos, aNewSize );
    }
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
    aAndRB.Check( sal_True );
    SearchEntryHdl_Impl( &aSearchLB );
    SearchPartHdl_Impl( &aAndRB );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchTabPage, AddSearchHdl_Impl, PushButton *, EMPTYARG )
{
    //The following two lines is added by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)
    aAddPB.Enable(sal_False);
    aChangePB.Enable(sal_False);
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
    aChangePB.Enable(sal_False);
    aAddPB.Enable(sal_False);
    sal_uInt16 nPos = aSearchLB.GetSelectEntryPos();
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
        SetUpdateMode(sal_False);
        String sEntry = aSearchNameED.GetText();
        // im AddHdl wird sLastSelectedEntry umgesetzt
        String sTemp(sLastSelectedEntry);
        AddSearchHdl_Impl(0);
        aSearchLB.SelectEntry(sTemp);
        DeleteSearchHdl_Impl(0);
        aSearchLB.SelectEntry(sEntry);
        SearchEntryHdl_Impl(&aSearchLB);
        SetUpdateMode(sal_True);
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchTabPage, DeleteSearchHdl_Impl, PushButton *, EMPTYARG)
{
    aChangePB.Enable(sal_False);     //add by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)
    sal_uInt16 nPos = aSearchLB.GetSelectEntryPos();
    DBG_ASSERT(nPos != LISTBOX_ENTRY_NOTFOUND, "kein Eintrag selektiert!");
    aSearchConfig.RemoveData(aSearchLB.GetSelectEntry());
    aSearchLB.RemoveEntry(nPos);
    aSearchLB.SelectEntryPos(0);
    SearchEntryHdl_Impl(&aSearchLB);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchTabPage, SearchEntryHdl_Impl, ListBox*, pBox )
{
    sal_uInt16 nEntryPos = pBox->GetSelectEntryPos();
    if ( nEntryPos != LISTBOX_ENTRY_NOTFOUND )
    {
        String sSelection(pBox->GetSelectEntry());
        //Modified by BerryJia for fixing Bug102610 Time:2002-8-29 11:00 (China Standard Time GMT+08:00)
        if(!ConfirmLeave(sSelection))
            return 0;

        const SvxSearchEngineData* pData = aSearchConfig.GetData(sSelection);
        DBG_ASSERT(pData, "SearchEngine not available");
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
            aCaseED.SelectEntryPos( (sal_uInt16)nCase );
            aCurrentSrchData = *pData;
        }
        aDeletePB.Enable();
    }
    else
    {
        aDeletePB.Enable(sal_False);
        sLastSelectedEntry.Erase();
    }
    aChangePB.Enable(sal_False);
    aAddPB.Enable(sal_False);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchTabPage, SearchModifyHdl_Impl, SvxNoSpaceEdit*, pEdit )
{
    if ( pEdit == &aSearchNameED )
    {
        sal_Bool bTextLen = ( 0 != pEdit->GetText().Len() );
        sal_Bool bFound = sal_False;
        if ( bTextLen )
        {
            sal_uInt16 nEntryPos = aSearchLB.GetEntryPos( pEdit->GetText() );
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

IMPL_LINK( SvxSearchTabPage, SearchPartHdl_Impl, RadioButton *, EMPTYARG )
{
    sal_Bool bAnd = aAndRB.IsChecked();
    sal_Bool bOr = aOrRB.IsChecked();

    aURLED.SetText(bAnd ? aCurrentSrchData.sAndPrefix : bOr ? aCurrentSrchData.sOrPrefix : aCurrentSrchData.sExactPrefix);
    aSeparatorED.SetText( bAnd ? aCurrentSrchData.sAndSeparator : bOr ? aCurrentSrchData.sOrSeparator : aCurrentSrchData.sExactSeparator);
    aPostFixED.SetText(bAnd ? aCurrentSrchData.sAndSuffix : bOr ? aCurrentSrchData.sOrSuffix : aCurrentSrchData.sExactSuffix );
    sal_Int32 nCase = bAnd ? aCurrentSrchData.nAndCaseMatch : bOr ? aCurrentSrchData.nOrCaseMatch : aCurrentSrchData.nExactCaseMatch;
    aCaseED.SelectEntryPos( (sal_uInt16)nCase );
    return 0;
}

//#98647#----------------------------------------------
void SvxScriptExecListBox::RequestHelp( const HelpEvent& rHEvt )
{   // try to show tips just like as on toolbars
    sal_uInt16 nPos=LISTBOX_ENTRY_NOTFOUND;
    sal_uInt16 nTop = GetTopEntry();
    sal_uInt16 nCount = GetDisplayLineCount(); // Attention: Not GetLineCount()
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
    :SfxTabPage         ( pParent, CUI_RES( RID_SVXPAGE_INET_SECURITY ), rSet )

    ,maSecurityOptionsFL( this, CUI_RES( FL_SEC_SECURITYOPTIONS ) )
    ,maSecurityOptionsFI( this, CUI_RES( FI_SEC_SECURITYOPTIONS ) )
    ,maSecurityOptionsPB( this, CUI_RES( PB_SEC_SECURITYOPTIONS ) )

    ,maPasswordsFL      ( this, CUI_RES( FL_SEC_PASSWORDS ) )
    ,maSavePasswordsCB  ( this, CUI_RES( CB_SEC_SAVEPASSWORDS ) )
    ,maShowConnectionsPB( this, CUI_RES( PB_SEC_CONNECTIONS ) )
    ,maMasterPasswordCB ( this, CUI_RES( CB_SEC_MASTERPASSWORD ) )
    ,maMasterPasswordFI ( this, CUI_RES( FI_SEC_MASTERPASSWORD ) )
    ,maMasterPasswordPB ( this, CUI_RES( PB_SEC_MASTERPASSWORD ) )

    ,maMacroSecFL       ( this, CUI_RES( FL_SEC_MACROSEC ) )
    ,maMacroSecFI       ( this, CUI_RES( FI_SEC_MACROSEC ) )
    ,maMacroSecPB       ( this, CUI_RES( PB_SEC_MACROSEC ) )

    ,mpSecOptions       ( new SvtSecurityOptions )
    ,mpSecOptDlg        ( NULL )

    ,msPasswordStoringDeactivateStr(    CUI_RES( STR_SEC_NOPASSWDSAVE ) )

{
    FreeResource();

    InitControls();

    maSecurityOptionsPB.SetClickHdl( LINK( this, SvxSecurityTabPage, SecurityOptionsHdl ) );
    maSavePasswordsCB.SetClickHdl( LINK( this, SvxSecurityTabPage, SavePasswordHdl ) );
    maMasterPasswordPB.SetClickHdl( LINK( this, SvxSecurityTabPage, MasterPasswordHdl ) );
    maMasterPasswordCB.SetClickHdl( LINK( this, SvxSecurityTabPage, MasterPasswordCBHdl ) );
    maShowConnectionsPB.SetClickHdl( LINK( this, SvxSecurityTabPage, ShowPasswordsHdl ) );
    maMacroSecPB.SetClickHdl( LINK( this, SvxSecurityTabPage, MacroSecPBHdl ) );

    ActivatePage( rSet );
}

SvxSecurityTabPage::~SvxSecurityTabPage()
{
    delete mpSecOptions;
    delete mpSecOptDlg;
}

IMPL_LINK( SvxSecurityTabPage, SecurityOptionsHdl, PushButton*, EMPTYARG )
{
    if ( !mpSecOptDlg )
        mpSecOptDlg = new svx::SecurityOptionsDialog( this, mpSecOptions );
    mpSecOptDlg->Execute();
    return 0;
}

IMPL_LINK( SvxSecurityTabPage, SavePasswordHdl, void*, EMPTYARG )
{
    try
    {
        Reference< task::XMasterPasswordHandling > xMasterPasswd(
            comphelper::getProcessServiceFactory()->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.task.PasswordContainer" ) ) ),
            UNO_QUERY_THROW );

        if ( maSavePasswordsCB.IsChecked() )
        {
            sal_Bool bOldValue = xMasterPasswd->allowPersistentStoring( sal_True );
            xMasterPasswd->removeMasterPassword();
            if ( xMasterPasswd->changeMasterPassword( Reference< task::XInteractionHandler >() ) )
            {
                maMasterPasswordPB.Enable( sal_True );
                maMasterPasswordCB.Check( sal_True );
                maMasterPasswordCB.Enable( sal_True );
                maMasterPasswordFI.Enable( sal_True );
                maShowConnectionsPB.Enable( sal_True );
            }
            else
            {
                xMasterPasswd->allowPersistentStoring( bOldValue );
                maSavePasswordsCB.Check( sal_False );
            }
        }
        else
        {
            QueryBox aQuery( this, WB_YES_NO|WB_DEF_NO, msPasswordStoringDeactivateStr );
            sal_uInt16 nRet = aQuery.Execute();

            if( RET_YES == nRet )
            {
                xMasterPasswd->allowPersistentStoring( sal_False );
                maMasterPasswordCB.Check( sal_True );
                maMasterPasswordPB.Enable( sal_False );
                maMasterPasswordCB.Enable( sal_False );
                maMasterPasswordFI.Enable( sal_False );
                maShowConnectionsPB.Enable( sal_False );
            }
            else
            {
                maSavePasswordsCB.Check( sal_True );
                maMasterPasswordPB.Enable( sal_True );
                maShowConnectionsPB.Enable( sal_True );
            }
        }
    }
    catch( Exception& )
    {
        maSavePasswordsCB.Check( !maSavePasswordsCB.IsChecked() );
    }

    return 0;
}

IMPL_LINK( SvxSecurityTabPage, MasterPasswordHdl, PushButton*, EMPTYARG )
{
    try
    {
        Reference< task::XMasterPasswordHandling > xMasterPasswd(
            comphelper::getProcessServiceFactory()->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.task.PasswordContainer" ) ) ),
            UNO_QUERY );

        if ( xMasterPasswd.is() && xMasterPasswd->isPersistentStoringAllowed() )
            xMasterPasswd->changeMasterPassword( Reference< task::XInteractionHandler >() );
    }
    catch( Exception& )
    {}

    return 0;
}

IMPL_LINK( SvxSecurityTabPage, MasterPasswordCBHdl, void*, EMPTYARG )
{
    try
    {
        Reference< task::XMasterPasswordHandling2 > xMasterPasswd(
            comphelper::getProcessServiceFactory()->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.task.PasswordContainer" ) ) ),
            UNO_QUERY_THROW );

        if ( maMasterPasswordCB.IsChecked() )
        {
            if ( xMasterPasswd->isPersistentStoringAllowed() && xMasterPasswd->changeMasterPassword( Reference< task::XInteractionHandler >() ) )
            {
                maMasterPasswordPB.Enable( sal_True );
                maMasterPasswordFI.Enable( sal_True );
            }
            else
            {
                maMasterPasswordCB.Check( sal_False );
                maMasterPasswordPB.Enable( sal_True );
                maMasterPasswordFI.Enable( sal_True );
            }
        }
        else
        {
            if ( xMasterPasswd->isPersistentStoringAllowed() && xMasterPasswd->useDefaultMasterPassword( Reference< task::XInteractionHandler >() ) )
            {
                maMasterPasswordPB.Enable( sal_False );
                maMasterPasswordFI.Enable( sal_False );
            }
            else
            {
                maMasterPasswordCB.Check( sal_True );
                maMasterPasswordPB.Enable( sal_True );
                maShowConnectionsPB.Enable( sal_True );
            }
        }
    }
    catch( Exception& )
    {
        maSavePasswordsCB.Check( !maSavePasswordsCB.IsChecked() );
    }

    return 0;
}

IMPL_LINK( SvxSecurityTabPage, ShowPasswordsHdl, PushButton*, EMPTYARG )
{
    try
    {
        Reference< task::XMasterPasswordHandling > xMasterPasswd(
            comphelper::getProcessServiceFactory()->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.task.PasswordContainer" ) ) ),
            UNO_QUERY );

        if ( xMasterPasswd.is() && xMasterPasswd->isPersistentStoringAllowed() && xMasterPasswd->authorizateWithMasterPassword( Reference< task::XInteractionHandler>() ) )
        {
            svx::WebConnectionInfoDialog aDlg( this );
            aDlg.Execute();
        }
    }
    catch( Exception& )
    {}
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


void SvxSecurityTabPage::InitControls()
{
    // Hide all controls which belong to the macro security button in case the macro
    // security settings managed by the macro security dialog opened via the button
    // are all readonly or if the macros are disabled in general.
    // @@@ Better would be to query the dialog whether it is 'useful' or not. Exposing
    //     macro security dialog implementations here, which is bad.
    if (    mpSecOptions->IsMacroDisabled()
         || (    mpSecOptions->IsReadOnly( SvtSecurityOptions::E_MACRO_SECLEVEL )
              && mpSecOptions->IsReadOnly( SvtSecurityOptions::E_MACRO_TRUSTEDAUTHORS )
              && mpSecOptions->IsReadOnly( SvtSecurityOptions::E_SECUREURLS ) ) )
    {
        maMacroSecFL.Hide();
        maMacroSecFI.Hide();
        maMacroSecPB.Hide();
    }

    // one button too small for its text?
    sal_Int32 i = 0;
    long nBtnTextWidth = 0;
    Window* pButtons[] = { &maSecurityOptionsPB, &maMasterPasswordPB,
                           &maShowConnectionsPB, &maMacroSecPB };
    Window** pButton = pButtons;
    const sal_Int32 nBCount = sizeof( pButtons ) / sizeof( pButtons[ 0 ] );
    for ( ; i < nBCount; ++i, ++pButton )
    {
        long nTemp = (*pButton)->GetCtrlTextWidth( (*pButton)->GetText() );
        if ( nTemp > nBtnTextWidth )
            nBtnTextWidth = nTemp;
    }

    nBtnTextWidth = nBtnTextWidth * 115 / 100; // a little offset
    const long nButtonWidth = maSecurityOptionsPB.GetSizePixel().Width();
    const long nMaxWidth = nButtonWidth * 140 / 100;
    long nExtra = ( nBtnTextWidth > nMaxWidth ) ? nBtnTextWidth - nMaxWidth : 0;
    nBtnTextWidth = std::min( nBtnTextWidth, nMaxWidth );

    if ( nBtnTextWidth > nButtonWidth )
    {
        // so make the buttons broader and its control in front of it smaller
        long nDelta = nBtnTextWidth - nButtonWidth;
        pButton = pButtons;

        if ( nExtra > 0 )
        {
            long nPos = (*pButton)->GetPosPixel().X() - nDelta;
            long nWidth = (*pButton)->GetSizePixel().Width() + nDelta;
            long nMaxExtra = GetOutputSizePixel().Width() - ( nPos + nWidth ) - 2;
            nExtra = ( nExtra < nMaxExtra ) ? nExtra : nMaxExtra;
        }

        for ( i = 0; i < nBCount; ++i, ++pButton )
        {
            Point aNewPos = (*pButton)->GetPosPixel();
            aNewPos.X() -= nDelta;
            Size aNewSize = (*pButton)->GetSizePixel();
            aNewSize.Width() += ( nDelta + nExtra );
            (*pButton)->SetPosSizePixel( aNewPos, aNewSize );
        }

        Window* pControls[] = { &maSecurityOptionsFI, &maSavePasswordsCB,
                                &maMasterPasswordFI, &maMacroSecFI };
        Window** pControl = pControls;
        const sal_Int32 nCCount = sizeof( pControls ) / sizeof( pControls[ 0 ] );
        for ( i = 0; i < nCCount; ++i, ++pControl )
        {
            Size aNewSize = (*pControl)->GetSizePixel();
            aNewSize.Width() -= nDelta;
            (*pControl)->SetSizePixel( aNewSize );
        }
    }

    maMasterPasswordPB.Enable( sal_False );
    maMasterPasswordCB.Enable( sal_False );
    maMasterPasswordCB.Check( sal_True );
    maMasterPasswordFI.Enable( sal_False );
    maShowConnectionsPB.Enable( sal_False );

    // initialize the password saving checkbox
    try
    {
        Reference< task::XMasterPasswordHandling > xMasterPasswd(
            comphelper::getProcessServiceFactory()->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.task.PasswordContainer" ) ) ),
            UNO_QUERY_THROW );

        if ( xMasterPasswd->isPersistentStoringAllowed() )
        {
            maMasterPasswordCB.Enable( sal_True );
            maShowConnectionsPB.Enable( sal_True );
            maSavePasswordsCB.Check( sal_True );

            Reference< task::XMasterPasswordHandling2 > xMasterPasswd2( xMasterPasswd, UNO_QUERY );
            if ( xMasterPasswd2.is() && xMasterPasswd2->isDefaultMasterPasswordUsed() )
                maMasterPasswordCB.Check( sal_False );
            else
            {
                maMasterPasswordPB.Enable( sal_True );
                maMasterPasswordCB.Check( sal_True );
                maMasterPasswordFI.Enable( sal_True );
            }
        }
    }
    catch( Exception& )
    {
        maSavePasswordsCB.Enable( sal_False );
    }


}

SfxTabPage* SvxSecurityTabPage::Create(Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SvxSecurityTabPage(pParent, rAttrSet);
}

void SvxSecurityTabPage::ActivatePage( const SfxItemSet& )
{
}

int SvxSecurityTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

namespace
{
    bool CheckAndSave( SvtSecurityOptions& _rOpt, SvtSecurityOptions::EOption _eOpt, const bool _bIsChecked, bool& _rModfied )
    {
        bool bModified = false;
        if ( _rOpt.IsOptionEnabled( _eOpt ) )
        {
            bModified = _rOpt.IsOptionSet( _eOpt ) != _bIsChecked;
            if ( bModified )
            {
                _rOpt.SetOption( _eOpt, _bIsChecked );
                _rModfied = true;
            }
        }

        return bModified;
    }
}

sal_Bool SvxSecurityTabPage::FillItemSet( SfxItemSet& )
{
    bool bModified = false;

    if ( mpSecOptDlg )
    {
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_SAVEORSEND, mpSecOptDlg->IsSaveOrSendDocsChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_SIGNING, mpSecOptDlg->IsSignDocsChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_PRINT, mpSecOptDlg->IsPrintDocsChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_CREATEPDF, mpSecOptDlg->IsCreatePdfChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_REMOVEPERSONALINFO, mpSecOptDlg->IsRemovePersInfoChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_DOCWARN_RECOMMENDPASSWORD, mpSecOptDlg->IsRecommPasswdChecked(), bModified );
        CheckAndSave( *mpSecOptions, SvtSecurityOptions::E_CTRLCLICK_HYPERLINK, mpSecOptDlg->IsCtrlHyperlinkChecked(), bModified );
    }

    return bModified;
}

/*--------------------------------------------------------------------*/

void SvxSecurityTabPage::Reset( const SfxItemSet& )
{
    SfxObjectShell* pCurDocShell = SfxObjectShell::Current();
    if( pCurDocShell )
    {
    }
}

//added by jmeng begin
MozPluginTabPage::MozPluginTabPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_INET_MOZPLUGIN ), rSet ),
    aMSWordGB       ( this, CUI_RES( GB_MOZPLUGIN       ) ),
    aWBasicCodeCB   ( this, CUI_RES( CB_MOZPLUGIN_CODE ) )
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
sal_Bool MozPluginTabPage::FillItemSet( SfxItemSet& )
{
    sal_Bool hasInstall = isInstalled();
    sal_Bool hasChecked = aWBasicCodeCB.IsChecked();
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
    return sal_True;
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
inline bool getDllURL(rtl::OString * path)
{
    OSL_ASSERT(path != NULL);
    ::rtl::OUString dirPath/*dllPath, */;
    if (osl_getExecutableFile(&dirPath.pData) != osl_Process_E_None) {
        return false;
    }
    dirPath = dirPath.copy(0, dirPath.lastIndexOf('/'));
//    osl::FileBase::getAbsoluteFileURL(dirPath, libPath, dllPath);
    ::rtl::OUString sysDirPath;
    osl::FileBase::getSystemPathFromFileURL(dirPath, sysDirPath);
    *path = OUStringToOString(sysDirPath, RTL_TEXTENCODING_ASCII_US);
    return true;
}

sal_Bool MozPluginTabPage::isInstalled()
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
    if (!getDllURL(&tempString)) {
        return false;
    }
    strncpy(realFilePath, tempString.getStr(), NPP_PATH_MAX);
    strcat(realFilePath, "/libnpsoplugin" SAL_DLLEXTENSION);

    if (0 != strcmp(lnkReferFilePath, realFilePath))
        return false;
    return true;
#endif
#ifdef WNT
    // get the value from registry
        sal_Bool ret = true;
    ::rtl::OString tempString;
    char realFilePath[NPP_PATH_MAX] = {0};
    if (!getDllURL(&tempString)){
        return false;
    }
    strncpy(realFilePath, tempString.getStr(), NPP_PATH_MAX);
    if(! lc_isInstalled(realFilePath))
        ret =true;
    else
        ret = false;
    return ret;
#endif
}

sal_Bool MozPluginTabPage::installPlugin()
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
    if (!getDllURL(&tempString)) {
        return false;
    }
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
    if (!getDllURL(&tempString)) {
        return false;
    }
    strncpy(realFilePath, tempString.getStr(), NPP_PATH_MAX);
    if( !lc_installPlugin(realFilePath))
        return true;
    else
        return false;
#endif
}

sal_Bool MozPluginTabPage::uninstallPlugin()
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
    if (!getDllURL(&tempString)) {
        return false;
    }
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
    virtual void Notify( const com::sun::star::uno::Sequence< rtl::OUString >& _rPropertyNames);
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

void MailerProgramCfg_Impl::Notify( const com::sun::star::uno::Sequence< rtl::OUString >& )
{
}

/* -------------------------------------------------------------------------*/

struct SvxEMailTabPage_Impl
{
    MailerProgramCfg_Impl aMailConfig;
};

SvxEMailTabPage::SvxEMailTabPage(Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, CUI_RES( RID_SVXPAGE_INET_MAIL ), rSet),
    aMailFL(this,           CUI_RES(FL_MAIL           )),
    aMailerURLFI(this,      CUI_RES(FI_MAILERURL      )),
    aMailerURLFT(this,      CUI_RES(FT_MAILERURL      )),
    aMailerURLED(this,      CUI_RES(ED_MAILERURL      )),
    aMailerURLPB(this,      CUI_RES(PB_MAILERURL      )),
    m_sDefaultFilterName(   CUI_RES(STR_DEFAULT_FILENAME        )),
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

sal_Bool SvxEMailTabPage::FillItemSet( SfxItemSet& )
{
    sal_Bool bMailModified = sal_False;
    if(!pImpl->aMailConfig.bROProgram && aMailerURLED.GetSavedValue() != aMailerURLED.GetText())
    {
        pImpl->aMailConfig.sProgram = aMailerURLED.GetText();
        bMailModified = sal_True;
    }
    if ( bMailModified )
        pImpl->aMailConfig.Commit();

    return sal_False;
}

/* -------------------------------------------------------------------------*/

void SvxEMailTabPage::Reset( const SfxItemSet& )
{
    aMailerURLED.Enable(sal_True );
    aMailerURLPB.Enable(sal_True );

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
        FileDialogHelper aHelper(
            com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            WB_OPEN );
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
