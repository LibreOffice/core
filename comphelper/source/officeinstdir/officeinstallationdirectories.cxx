/*************************************************************************
 *
 *  $RCSfile: officeinstallationdirectories.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-05 10:36:13 $
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
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include "osl/file.hxx"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/util/XMacroExpander.hpp"

#include "officeinstallationdirectories.hxx"

using namespace com::sun::star;

using namespace comphelper;

//=========================================================================
// helpers
//=========================================================================

uno::Sequence< rtl::OUString > SAL_CALL
OfficeInstallationDirectories_getSupportedServiceNames()
    throw()
{
    const rtl::OUString aServiceName(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.util.OfficeInstallationDirectories" ) );
    return uno::Sequence< rtl::OUString >( &aServiceName, 1 );
}

//=========================================================================
rtl::OUString SAL_CALL OfficeInstallationDirectories_getImplementationName()
    throw()
{
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.comp.util.OfficeInstallationDirectories" ) );
}

//=========================================================================
rtl::OUString SAL_CALL OfficeInstallationDirectories_getSingletonName()
    throw()
{
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.util.theOfficeInstallationDirectories" ) );
}

//=========================================================================
rtl::OUString SAL_CALL OfficeInstallationDirectories_getSingletonServiceName()
    throw()
{
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.util.OfficeInstallationDirectories" ) );
}

//=========================================================================
uno::Reference< uno::XInterface > SAL_CALL
OfficeInstallationDirectories_createInstance(
        const uno::Reference< lang::XMultiServiceFactory > & xSMgr )
    throw( uno::Exception )
{
    OSL_ENSURE( xSMgr.is(), "No service manager!" );

    uno::Reference< uno::XComponentContext > xCtx;
    uno::Reference< beans::XPropertySet > xPropSet( xSMgr, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        xPropSet->getPropertyValue(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ) ) )
        >>= xCtx;
    }

    if ( !xCtx.is() )
    {
        throw uno::RuntimeException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                "Unable to obtain component context from service manager!" ) ),
            uno::Reference< uno::XInterface >() );
    }

    return static_cast< cppu::OWeakObject * >(
        new OfficeInstallationDirectories( xCtx ) );
}

//=========================================================================
static bool makeCanonicalFileURL( rtl::OUString & rURL )
{
    OSL_ENSURE( rURL.matchAsciiL( "file:", sizeof( "file:" ) - 1 , 0 ) ,
                "File URL expected!" );

    rtl::OUString aNormalizedURL;
    if ( osl::FileBase::getAbsoluteFileURL( rtl::OUString(),
                                            rURL,
                                            aNormalizedURL )
            == osl::DirectoryItem::E_None )
    {
        osl::DirectoryItem aDirItem;
        if ( osl::DirectoryItem::get( aNormalizedURL, aDirItem )
                == osl::DirectoryItem::E_None )
        {
            osl::FileStatus aFileStatus( FileStatusMask_FileURL );

            if ( aDirItem.getFileStatus( aFileStatus )
                    == osl::DirectoryItem::E_None )
            {
                aNormalizedURL = aFileStatus.getFileURL();

                if ( aNormalizedURL.getLength() > 0 )
                {
                    if ( aNormalizedURL
                            .getStr()[ aNormalizedURL.getLength() - 1 ]
                                != sal_Unicode( '/' ) )
                        rURL = aNormalizedURL;
                    else
                        rURL = aNormalizedURL
                                .copy( 0, aNormalizedURL.getLength() - 1 );

                    return true;
                }
            }
        }
    }
    return false;
}

//=========================================================================
//=========================================================================
//
// OfficeInstallationDirectories Implementation.
//
//=========================================================================
//=========================================================================

OfficeInstallationDirectories::OfficeInstallationDirectories(
        const uno::Reference< uno::XComponentContext > & xCtx )
: m_aOfficeDirMacro( RTL_CONSTASCII_USTRINGPARAM( "$(baseinsturl)" ) ),
  m_aUserDirMacro( RTL_CONSTASCII_USTRINGPARAM( "$(userdataurl)" ) ),
  m_xCtx( xCtx ),
  m_pOfficeDir( 0 ),
  m_pUserDir( 0 )
{
}

//=========================================================================
// virtual
OfficeInstallationDirectories::~OfficeInstallationDirectories()
{
}

//=========================================================================
// util::XOfficeInstallationDirectories
//=========================================================================

// virtual
rtl::OUString SAL_CALL
OfficeInstallationDirectories::getOfficeInstallationDirectoryURL()
    throw ( uno::RuntimeException )
{
    // late init m_pOfficeDir and m_pUserDir
    initDirs();
    return rtl::OUString( *m_pOfficeDir );
}

//=========================================================================
// virtual
rtl::OUString SAL_CALL
OfficeInstallationDirectories::getOfficeUserDataDirectoryURL()
    throw ( uno::RuntimeException )
{
    // late init m_pOfficeDir and m_pUserDir
    initDirs();
    return rtl::OUString( *m_pUserDir );
}


//=========================================================================
// virtual
rtl::OUString SAL_CALL
OfficeInstallationDirectories::makeRelocatableURL( const rtl::OUString& URL )
    throw ( uno::RuntimeException )
{
    if ( URL.getLength() > 0 )
    {
        // late init m_pOfficeDir and m_pUserDir
        initDirs();

        rtl::OUString aCanonicalURL( URL );
        makeCanonicalFileURL( aCanonicalURL );

        sal_Int32 nIndex = aCanonicalURL.indexOf( *m_pOfficeDir );
        if ( nIndex  != -1 )
        {
            return rtl::OUString(
                URL.replaceAt( nIndex,
                               m_pOfficeDir->getLength(),
                               m_aOfficeDirMacro ) );
        }
        else
        {
            sal_Int32 nIndex = aCanonicalURL.indexOf( *m_pUserDir );
            if ( nIndex  != -1 )
            {
                return rtl::OUString(
                    URL.replaceAt( nIndex,
                                   m_pUserDir->getLength(),
                                   m_aUserDirMacro ) );
            }
        }
    }
    return rtl::OUString( URL );
}

//=========================================================================
// virtual
rtl::OUString SAL_CALL
OfficeInstallationDirectories::makeAbsoluteURL( const rtl::OUString& URL )
    throw ( uno::RuntimeException )
{
    if ( URL.getLength() > 0 )
    {
        sal_Int32 nIndex = URL.indexOf( m_aOfficeDirMacro );
        if ( nIndex != -1 )
        {
            // late init m_pOfficeDir and m_pUserDir
            initDirs();

            return rtl::OUString(
                URL.replaceAt( nIndex,
                               m_aOfficeDirMacro.getLength(),
                               *m_pOfficeDir ) );
        }
        else
        {
            sal_Int32 nIndex = URL.indexOf( m_aUserDirMacro );
            if ( nIndex != -1 )
            {
                // late init m_pOfficeDir and m_pUserDir
                initDirs();

                return rtl::OUString(
                    URL.replaceAt( nIndex,
                                   m_aUserDirMacro.getLength(),
                                   *m_pUserDir ) );
            }
        }
    }
    return rtl::OUString( URL );
}

//=========================================================================
// lang::XServiceInfo
//=========================================================================

// virtual
rtl::OUString SAL_CALL
OfficeInstallationDirectories::getImplementationName()
    throw ( uno::RuntimeException )
{
    return OfficeInstallationDirectories_getImplementationName();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL
OfficeInstallationDirectories::supportsService( const rtl::OUString& ServiceName )
    throw ( uno::RuntimeException )
{
    const uno::Sequence< rtl::OUString > & aNames
        = OfficeInstallationDirectories_getSupportedServiceNames();
    const rtl::OUString * p = aNames.getConstArray();
    for ( sal_Int32 nPos = 0; nPos < aNames.getLength(); nPos++ )
    {
        if ( p[ nPos ].equals( ServiceName ) )
            return sal_True;
    }
    return sal_False;

}

//=========================================================================
// virtual
uno::Sequence< ::rtl::OUString > SAL_CALL
OfficeInstallationDirectories::getSupportedServiceNames()
    throw ( uno::RuntimeException )
{
    return OfficeInstallationDirectories_getSupportedServiceNames();
}

//=========================================================================
// non-UNO
//=========================================================================

void OfficeInstallationDirectories::initDirs()
{
    if ( m_pOfficeDir == 0 )
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_pOfficeDir == 0 )
        {
            m_pOfficeDir = new rtl::OUString;
            m_pUserDir   = new rtl::OUString;

            uno::Reference< util::XMacroExpander > xExpander;

            m_xCtx->getValueByName(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "/singletons/com.sun.star.util.theMacroExpander" ) ) )
            >>= xExpander;

            OSL_ENSURE( xExpander.is(),
                        "Unable to obtain macro expander singleton!" );

            if ( xExpander.is() )
            {
                *m_pOfficeDir =
                    xExpander->expandMacros(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                            "${$SYSBINDIR/" SAL_CONFIGFILE( "bootstrap" ) ":BaseInstallation}" ) ) );

                OSL_ENSURE( m_pOfficeDir->getLength() > 0,
                        "Unable to obtain office installation directory!" );

                makeCanonicalFileURL( *m_pOfficeDir );

                *m_pUserDir =
                    xExpander->expandMacros(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                            "${$SYSBINDIR/" SAL_CONFIGFILE( "bootstrap" ) ":UserInstallation}" ) ) );

                OSL_ENSURE( m_pUserDir->getLength() > 0,
                        "Unable to obtain office user data directory!" );

                makeCanonicalFileURL( *m_pUserDir );
            }
        }
    }
}

