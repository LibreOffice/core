/*************************************************************************
 *
 *  $RCSfile: LDriver.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:01:28 $
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

#ifndef _CONNECTIVITY_EVOAB_LDRIVER_HXX_
#include "LDriver.hxx"
#endif
#ifndef _CONNECTIVITY_EVOAB_LCONNECTION_HXX_
#include "LConnection.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif
#ifndef _CONNECTIVITY_EVOAB_LCONFIGACCESS_HXX_
#include "LConfigAccess.hxx"
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _OSL_SECURITY_HXX_
#include "osl/security.hxx"
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDENVIRONMENT_HPP_
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef CONNECTIVITY_EVOAB_DEBUG_HELPER_HXX
#include "LDebug.hxx"
#endif

using namespace osl;
using namespace connectivity::evoab;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;

// --------------------------------------------------------------------------------
OEvoabDriver::OEvoabDriver(const Reference< XMultiServiceFactory >& _rxFactory) : OFileDriver(_rxFactory)
    ,m_aTempDir(NULL, sal_True)
    ,m_bWorkingDirCreated(sal_False)
    ,m_aFolderListName(::rtl::OUString::createFromAscii(getEVOAB_FOLDERLIST_FILE_NAME()))
    ,m_aVersionName(::rtl::OUString::createFromAscii(getEVOAB_VERSION_FILE_NAME()))
    ,m_aFileExt(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(getEVOAB_META_FILE_EXT())))
{
    m_aEvoab_CLI_FullPathCommand = getFullPathExportingCommand(_rxFactory);

    if ( m_aEvoab_CLI_FullPathCommand.getLength() == 0 )
        m_aEvoab_CLI_FullPathCommand = ::rtl::OUString::createFromAscii(getEVOAB_CLI_FULLPATHCOMMAND());
    if ( m_aEvoab_CLI_FullPathCommand.copy(0,7) != ::rtl::OUString::createFromAscii("file://") && m_aEvoab_CLI_FullPathCommand.copy(0,1) == ::rtl::OUString::createFromAscii("/"))
        m_aEvoab_CLI_FullPathCommand = ::rtl::OUString::createFromAscii("file://") + m_aEvoab_CLI_FullPathCommand;
    m_aEvoab_CLI_EffectiveCommand = m_aEvoab_CLI_FullPathCommand;
    m_aTempDir.EnableKillingFile();

    EVO_TRACE_STRING("OEvoabDriver::OEvoabDriver()::m_aEvoab_CLI_FullPathCommand = %s\n", m_aEvoab_CLI_FullPathCommand );
}
// static ServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OEvoabDriver::getImplementationName_Static(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii(EVOAB_DRIVER_IMPL_NAME);
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL OEvoabDriver::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  SAL_CALL connectivity::evoab::OEvoabDriver_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory) throw( ::com::sun::star::uno::Exception )
{
    return *(new OEvoabDriver(_rxFactory));
}
// --------------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OEvoabDriver::connect( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODriver_BASE::rBHelper.bDisposed)
       throw DisposedException();

    if ( ! acceptsURL(url) )
        return NULL;

    OEvoabConnection* pCon = new OEvoabConnection(this);
    pCon->construct(url,info);
        Reference< XConnection > xCon = pCon;
        m_xConnections.push_back(WeakReferenceHelper(*pCon));

    return xCon;
}
// --------------------------------------------------------------------------------
sal_Bool SAL_CALL OEvoabDriver::acceptsURL( const ::rtl::OUString& url )
                throw(SQLException, RuntimeException)
{
    sal_Bool bRet;
    bRet = sal_False;
    // here we have to look if we support this url format
    if(acceptsURL_Stat(url))
    {
        if(!m_bWorkingDirCreated)
        {
            String sTempDirURL = getTempDirURL();
            //EVO_TRACE_STRING("OEvoabDriver::acceptsURL()::sTempDirURL = %s\n", sTempDirURL );

            ::rtl::OUString aTempDirURL(sTempDirURL);
            m_aWorkingDirURL = aTempDirURL;
            m_bWorkingDirCreated = sal_True;
        }

        ::rtl::OUString aCLICommand = getEvoab_CLI_EffectiveCommand();
        ::rtl::OUString aWorkingDirURL = getWorkingDirURL();
        ::rtl::OUString aArgVersion = ::rtl::OUString::createFromAscii(getEVOAB_CLI_ARG_VERSION());
        EVO_TRACE_STRING("OEvoabDriver::acceptsURL()::aCLICommand = %s\n", aCLICommand );
        EVO_TRACE_STRING("OEvoabDriver::acceptsURL()::aWorkingDirURL = %s\n", aWorkingDirURL );
        EVO_TRACE_STRING("OEvoabDriver::acceptsURL()::aArgVersion = %s\n", aArgVersion );
        //::rtl::OUString aVerFileName = getEvoVersionFileName();
        //::rtl::OUString aFullFileName = getWorkingDirURL() + aVerFileName;
        //EVO_TRACE_STRING("OEvoabDriver::acceptsURL()::aVerFileName = %s\n", aVerFileName );

        oslFileHandle aStdout = NULL;

        sal_Bool bSuccess;
        bSuccess = sal_False;
        oslProcess aProcess( 0 );
        oslProcessError nProcErr = osl_Process_E_None;
        ::rtl::OUString sErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" no.error!"));
        nProcErr = osl_executeProcess_WithRedirectedIO(aCLICommand.pData,
                                                    &aArgVersion.pData,
                                                    1,
                                                    //osl_Process_SEARCHPATH
                                                    osl_Process_WAIT,
                                                        //| osl_Process_HIDDEN,
                                                    //osl::Security().getHandle(),
                                                    0,
                                                    aWorkingDirURL.pData,
                                                    0,
                                                    0,
                                                    &aProcess,
                                                    NULL,
                                                    &aStdout,
                                                    NULL);
        sErr = aCLICommand + translateProcessErrorMessage( nProcErr);
        EVO_TRACE_STRING("%s \n", sErr);
        if(nProcErr != osl_Process_E_None)
        {
            if(doesEvoab_CLI_HavePath())
                aCLICommand = getEvoab_CLI_Command();
            else
                aCLICommand = getEvoab_CLI_Path() + getEvoab_CLI_Command();
            EVO_TRACE_STRING("OEvoabDriver::acceptsURL()::aCLICommand = %s\n", aCLICommand );

            nProcErr = osl_executeProcess_WithRedirectedIO(aCLICommand.pData,
                                                        &aArgVersion.pData,
                                                        1,
                                                        osl_Process_SEARCHPATH | osl_Process_WAIT | osl_Process_HIDDEN,
                                                        0 ,
                                                        aWorkingDirURL.pData,
                                                        0,
                                                        0,
                                                        &aProcess,
                                                        NULL,
                                                        &aStdout,
                                                        NULL);
            if ( nProcErr == osl_Process_E_None )
                m_aEvoab_CLI_EffectiveCommand = aCLICommand;
            sErr = aCLICommand + translateProcessErrorMessage( nProcErr);
            EVO_TRACE_STRING("%s \n", sErr);
        }
        if(nProcErr == osl_Process_E_None)
        {
            OSL_ASSERT( aStdout );
            sal_Char  pBuffer[256];
            sal_uInt64  nBytesRead;
            OSL_ASSERT( pBuffer );
            oslFileError nFileErr = osl_File_E_None;
            nFileErr = osl_readFile( aStdout, pBuffer, 256, &nBytesRead);
            if ( nFileErr != osl_File_E_None )
            {
                sErr = translateFileErrorMessage( nFileErr);
                OSL_ENSURE(false, ::rtl::OUStringToOString( sErr, RTL_TEXTENCODING_ASCII_US ).getStr());
            }
            ::rtl::OUString aVersionInfo;
            if ( nFileErr == osl_File_E_None && nBytesRead > 0 && nBytesRead <= 256)
            {
                OSL_TRACE("OEvoabDriver::acceptsURL()::osl_readFile(),nBytesRead = %u",nBytesRead);
                aVersionInfo =
                        ::rtl::OUString( ( sal_Char * )pBuffer,
                        nBytesRead,
                        RTL_TEXTENCODING_UTF8 );
                EVO_TRACE_STRING("OEvoabDriver::acceptsURL()::aVersionInfo = %s\n", aVersionInfo );
                sal_Int32 nIndex = 0;
                sal_Bool bNumRetrieved = sal_False;
                ::rtl::OUString aToken;
                sal_Int32 aVer[4];
                sal_Int32 i;
                for ( i = 0; i < 4; i++ )
                    aVer[i] = 0;
                do
                {
                    aToken = aVersionInfo.getToken( 0, ' ', nIndex );
                    //OSL_TRACE("OEvoabDriver::acceptsURL()::Token:%d", nIndex );
                    //EVO_TRACE_STRING("OEvoabDriver::acceptsURL()::aToken = %s\n", aToken );
                    if( aToken.toChar() >= '0' && aToken.toChar() <= '9' )
                    {
                        bNumRetrieved = sal_True;
                        sal_Int32 nIndex1 = 0;
                        ::rtl::OUString aNum;
                        for(i = 0; i < 4 ; i++)
                        {
                            if( nIndex1 >= 0 )
                            {
                                aNum = aToken.getToken( 0, '.', nIndex1);
                                aVer[i] = aNum.toInt32();
                                OSL_TRACE("OEvoabDriver::acceptsURL()::Ver[%u]=%u", i, aVer[i] );
                            }
                        }
                    }
                }
                while ( nIndex >= 0 && !bNumRetrieved );

                if((aVer[0]>1)||(aVer[0]==1 && aVer[1]>3)||(aVer[0]==1 && aVer[1]==3 && aVer[2]>2)||(aVer[0]==1 && aVer[1]==3 && aVer[2]==2 && aVer[3]>=99))
                    bRet = sal_True;
                else
                    bRet = sal_False;
            }
            else
                bRet = sal_False;
        }
        osl_closeFile( aStdout );
    }
    EVO_TRACE_STRING("OEvoabDriver::acceptsURL()::return, return value = %s\n", ::rtl::OUString::valueOf(bRet) );
    return bRet;
}


// --------------------------------------------------------------------------------
sal_Bool OEvoabDriver::acceptsURL_Stat( const ::rtl::OUString& url )
{

    EVO_TRACE_STRING("OEvoabDriver::acceptsURL_Stat()::Scheme = %s\n", url );
    // Skip 'sdbc:address: part of URL
    //
    sal_Int32 nLen = url.indexOf(':');
    nLen = url.indexOf(':',nLen+1);
    ::rtl::OUString aAddrbookURI(url.copy(nLen+1));

    // Get Scheme
    nLen = aAddrbookURI.indexOf(':');
    ::rtl::OUString aAddrbookScheme;
    if ( nLen == -1 )
    {
        // There isn't any subschema: - but could be just subschema
        if ( aAddrbookURI.getLength() > 0 )
            aAddrbookScheme= aAddrbookURI;
        else if(url == ::rtl::OUString::createFromAscii("sdbc:address:") )
            return sal_True; // special case here
        else
            return sal_False;
    }
    else
        aAddrbookScheme = aAddrbookURI.copy(0, nLen);


    EVO_TRACE_STRING("OEvoabDriver::acceptsURL_Stat()::URI = %s\n", aAddrbookURI );
    EVO_TRACE_STRING("OEvoabDriver::acceptsURL_Stat()::Scheme = %s\n", aAddrbookScheme );

    return  aAddrbookScheme.compareToAscii( getSDBC_SCHEME_EVOLUTION() ) == 0 ;
}
// -------------------------------------------------------------------------
const rtl::OUString OEvoabDriver::getEvoab_CLI_Command() const
{
    rtl::OUString aEvoab_CLI_Command;
    sal_Int32 nLen = m_aEvoab_CLI_FullPathCommand.lastIndexOf('/');

    if ( nLen == -1 )
        aEvoab_CLI_Command = m_aEvoab_CLI_FullPathCommand;
    else
        aEvoab_CLI_Command = m_aEvoab_CLI_FullPathCommand.copy(nLen+1);

    EVO_TRACE_STRING( "OEvoabDriver::getEvoab_CLI_Command()::aEvoab_CLI_Command = %s\n", aEvoab_CLI_Command );

    return  aEvoab_CLI_Command;
}
// -------------------------------------------------------------------------
const rtl::OUString OEvoabDriver::getEvoab_CLI_Path() const
{
    rtl::OUString aEvoab_CLI_Path;
    sal_Int32 nLen = m_aEvoab_CLI_FullPathCommand.lastIndexOf('/');

    if ( nLen == -1 )
    {
        rtl::OUString aDefault_CLI_FullPathCommand;
        aDefault_CLI_FullPathCommand = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(OEvoabDriver::getEVOAB_CLI_FULLPATHCOMMAND()));
        sal_Int32 nLength = aDefault_CLI_FullPathCommand.lastIndexOf('/');
        aEvoab_CLI_Path = aDefault_CLI_FullPathCommand.copy(0, nLength+1);
    }
    else
        aEvoab_CLI_Path = m_aEvoab_CLI_FullPathCommand.copy(0, nLen+1);
    EVO_TRACE_STRING( "OEvoabDriver::getEvoab_CLI_Path()::aEvoab_CLI_Path = %s\n", aEvoab_CLI_Path );

    return  aEvoab_CLI_Path;
}
// -------------------------------------------------------------------------
const rtl::OUString OEvoabDriver::getWorkingDirPath() const
{
    ::rtl::OUString aWorkingDirPath;
    if(m_bWorkingDirCreated)
        osl::File::getSystemPathFromFileURL( m_aWorkingDirURL, aWorkingDirPath );
    return  aWorkingDirPath;
}
// -------------------------------------------------------------------------
const String OEvoabDriver::getEvoFolderListFileURL() const
{

    ::rtl::OUString aEvoFolderListFileURL;
    aEvoFolderListFileURL = getWorkingDirURL() + getEvoFolderListFileName();

    EVO_TRACE_STRING("OEvoabDriver::getEvoFolderListFileURL(): aEvoFolderListFileURL = %s\n", aEvoFolderListFileURL );
    return aEvoFolderListFileURL.getStr();
}

// -------------------------------------------------------------------------
String OEvoabDriver::getTempDirURL() const
{
    ::rtl::OUString aTempDirURL;
    aTempDirURL = m_aTempDir.GetURL();
    if((aTempDirURL.lastIndexOf( '/')) != (aTempDirURL.getLength( ) - 1))
        aTempDirURL += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));

    EVO_TRACE_STRING("OEvoabDriver::getTempDirURL(): aTempDirURL = %s\n", aTempDirURL );
    return aTempDirURL.getStr();
}
//-------------------------------------------------------------------------
sal_Bool OEvoabDriver::fileExists(const ::rtl::OUString& _rURL, sal_Bool _bIsDir) const
{
    ::ucb::Content aCheckExistence;
    sal_Bool bExists = sal_False;
    try
    {
        aCheckExistence = ::ucb::Content(_rURL, Reference< XCommandEnvironment >());
        if(_bIsDir)
            bExists = aCheckExistence.isFolder();
        else
            bExists = aCheckExistence.isDocument();
    }
    catch(const Exception&) { }
    return bExists;
}

// -----------------------------------------------------------------------------
const sal_Char* OEvoabDriver::getSDBC_SCHEME_EVOLUTION()
{
    static sal_Char*    SDBC_SCHEME_EVOLUTION = EVOAB_EVOLUTION_SCHEMA;
    return SDBC_SCHEME_EVOLUTION;
}
const sal_Char* OEvoabDriver::getEVOAB_FOLDERLIST_FILE_NAME()
{
    static sal_Char*    EVOAB_FOLDERLIST_FILE_NAME = "FolderList";
    return EVOAB_FOLDERLIST_FILE_NAME;
}
const sal_Char* OEvoabDriver::getEVOAB_VERSION_FILE_NAME()
{
    static sal_Char*    EVOAB_VERSION_FILE_NAME = "EvoVersion";
    return EVOAB_VERSION_FILE_NAME;
}
const sal_Char* OEvoabDriver::getEVOAB_META_FILE_EXT()
{
    static sal_Char*    EVOAB_META_FILE_EXT = "csv";
    return EVOAB_META_FILE_EXT;
}
const sal_Char* OEvoabDriver::getEVOAB_CLI_FULLPATHCOMMAND()
{
    static sal_Char*    EVOAB_CLI_FULLPATHCOMMAND = "file:///home/evoab/extra/share/evolution/*/tools/evolution-addressbook-export";
    return EVOAB_CLI_FULLPATHCOMMAND;
}
const sal_Char* OEvoabDriver::getEVOAB_CLI_ARG_LIST_FOLDERS()
{
    static sal_Char*    EVOAB_CLI_ARG_LIST_FOLDERS = "-l";
    return EVOAB_CLI_ARG_LIST_FOLDERS;
}
const sal_Char* OEvoabDriver::getEVOAB_CLI_ARG_OUTPUT_FILE_PREFIX()
{
    static sal_Char*    EVOAB_CLI_ARG_OUTPUT_FILE_PREFIX = "--output=";
    return EVOAB_CLI_ARG_OUTPUT_FILE_PREFIX;
}
const sal_Char* OEvoabDriver::getEVOAB_CLI_ARG_OUTPUT_FORMAT()
{
    static sal_Char*    EVOAB_CLI_ARG_OUTPUT_FORMAT = "--format=csv";
    return EVOAB_CLI_ARG_OUTPUT_FORMAT;
}
const sal_Char* OEvoabDriver::getEVOAB_CLI_ARG_VERSION()
{
    static sal_Char*    EVOAB_CLI_ARG_VERSION = "--version";
    return EVOAB_CLI_ARG_VERSION;
}
const sal_Char* OEvoabDriver::getEVOAB_CLI_ARG_OUTPUT_REDIRECT()
{
    static sal_Char*    EVOAB_CLI_ARG_OUTPUT_REDIRECT = ">";
    return EVOAB_CLI_ARG_OUTPUT_REDIRECT;
}
rtl::OUString OEvoabDriver::translateProcessErrorMessage( oslProcessError nProcErr)
{
    ::rtl::OUString sProcErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" not executed!."));
    switch (nProcErr)
    {
        case osl_Process_E_None:
            sProcErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" executed sucessful!"));
            break;
        case osl_Process_E_NotFound:
            sProcErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" executed failed: not found!"));
            break;
        case osl_Process_E_NoPermission:
            sProcErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" executed failed: has no permission!"));
            break;
        case osl_Process_E_TimedOut:
            sProcErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" executed failed: time out!"));
            break;
        case osl_Process_E_Unknown:
            sProcErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" executed failed: unkown reason!"));
            break;
        case osl_Process_E_InvalidError:
            sProcErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" executed failed: invalid error!"));
            break;
        default:
            sProcErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" executed failed: other reason!"));
    }
    return sProcErr;
}
rtl::OUString OEvoabDriver::translateFileErrorMessage( oslFileError nFileErr)
{
    ::rtl::OUString sFileErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" no file operation!."));
    switch (nFileErr)
    {
        case osl_File_E_None:
            sFileErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" File operation succeeded!"));
            break;
        case osl_File_E_INVAL:
            sFileErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" File operation failed: the format of the parameters was not valid!"));
            break;
        case osl_File_E_INTR:
            sFileErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" File operation failed: function call was interrupted!"));
            break;
        case osl_File_E_IO:
            sFileErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" File operation failed: I/O errors!"));
            break;
        case osl_File_E_ISDIR:
            sFileErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" File operation failed: is a directory!"));
            break;
        case osl_File_E_BADF:
            sFileErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" File operation failed: bad file!"));
            break;
        case osl_File_E_FAULT:
            sFileErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" File operation failed: bad address!"));
            break;
        case osl_File_E_AGAIN:
            sFileErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" File operation failed: operation would block!"));
            break;
        case osl_File_E_NOLINK:
            sFileErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" File operation failed: link has been severed!"));
            break;
        default:
            sFileErr = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" File operation failed: other reason!"));
    }
    return sFileErr;
}
// -----------------------------------------------------------------------------
Sequence< DriverPropertyInfo > SAL_CALL OEvoabDriver::getPropertyInfo( const ::rtl::OUString& url, const Sequence< PropertyValue >& info ) throw(SQLException, RuntimeException)
{
    if ( !acceptsURL(url) )
        ::dbtools::throwGenericSQLException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Invalid URL!")) ,*this);
    return Sequence< DriverPropertyInfo >();
}
