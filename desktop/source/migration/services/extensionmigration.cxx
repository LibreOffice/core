/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: extensionmigration.cxx,v $
 * $Revision: 1.2 $
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
#include "precompiled_desktop.hxx"
#include "extensionmigration.hxx"
#include <tools/urlobj.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <ucbhelper/content.hxx>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include "comphelper/processfactory.hxx"
#include "com/sun/star/deployment/XPackageManagerFactory.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/xml/sax/XParser.hpp"
#include "rtl/instance.hxx"
#include "osl/file.hxx"
#include "osl/thread.h"

#include "xmlscript/xmllib_imexp.hxx"
#include "../../deployment/inc/dp_ucb.h"

#ifdef SYSTEM_DB
#include <db.h>
#else
#include <berkeleydb/db.h>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {

    struct LibDescriptor :
        public rtl::StaticWithInit<const ::xmlscript::LibDescriptorArray, LibDescriptor> {
    const ::xmlscript::LibDescriptorArray operator () () {


        return ::xmlscript::LibDescriptorArray();
    }
};
}
//.........................................................................
namespace migration
{
//.........................................................................


    static ::rtl::OUString sExtensionSubDir = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/uno_packages/" ) );
    static ::rtl::OUString sSubDirName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cache" ) );
    static ::rtl::OUString sConfigDir = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/registry/data" ) );
    static ::rtl::OUString sOrgDir = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/registry/data/org" ) );
    static ::rtl::OUString sExcludeDir1 = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/registry/data/org" ) );
    static ::rtl::OUString sExcludeDir2 = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/registry/data/org/openoffice" ) );

    static ::rtl::OUString sBasicType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("application/vnd.sun.star.basic-library"));
    static ::rtl::OUString sDialogType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("application/vnd.sun.star.dialog-library"));

    static ::rtl::OUString sConfigurationDataType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("application/vnd.sun.star.configuration-data"));
    static ::rtl::OUString sConfigurationSchemaType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("application/vnd.sun.star.configuration-schema"));

    // =============================================================================
    // component operations
    // =============================================================================

    ::rtl::OUString ExtensionMigration_getImplementationName()
    {
        static ::rtl::OUString* pImplName = 0;
        if ( !pImplName )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pImplName )
            {
                static ::rtl::OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.desktop.migration.Extensions" ) );
                pImplName = &aImplName;
            }
        }
        return *pImplName;
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > ExtensionMigration_getSupportedServiceNames()
    {
        static Sequence< ::rtl::OUString >* pNames = 0;
        if ( !pNames )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pNames )
            {
                static Sequence< ::rtl::OUString > aNames(1);
                aNames.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.migration.Extensions" ) );
                pNames = &aNames;
            }
        }
        return *pNames;
    }

    // =============================================================================
    // ExtensionMigration
    // =============================================================================

    ExtensionMigration::ExtensionMigration(Reference< XComponentContext > const & ctx) :
    m_ctx(ctx)
    {
    }

    // -----------------------------------------------------------------------------

    ExtensionMigration::~ExtensionMigration()
    {
    }

    ::osl::FileBase::RC ExtensionMigration::checkAndCreateDirectory( INetURLObject& rDirURL )
    {
        ::osl::FileBase::RC aResult = ::osl::Directory::create( rDirURL.GetMainURL( INetURLObject::DECODE_TO_IURI ) );
        if ( aResult == ::osl::FileBase::E_NOENT )
        {
            INetURLObject aBaseURL( rDirURL );
            aBaseURL.removeSegment();
            checkAndCreateDirectory( aBaseURL );
            return ::osl::Directory::create( rDirURL.GetMainURL( INetURLObject::DECODE_TO_IURI ) );
        }
        else
        {
            return aResult;
        }
    }

    void ExtensionMigration::prepareBasicLibs()
    {
        prepareBasicLibs(m_sSourceDir + ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("/user/basic/script.xlc")), m_scriptElements);
        prepareBasicLibs(m_sSourceDir + ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("/user/basic/dialog.xlc")), m_dialogElements);
    }

    void ExtensionMigration::prepareBasicLibs(const ::rtl::OUString & sURL,
        ::xmlscript::LibDescriptorArray & out_elements)
    {

        ::ucbhelper::Content ucb_content;
        if (dp_misc::create_ucb_content( &ucb_content, sURL,
            uno::Reference< ucb::XCommandEnvironment>(), false /* no throw */ ))
        {
            uno::Reference<xml::sax::XParser> xParser(
                m_ctx->getServiceManager()->createInstanceWithContext(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.sax.Parser")),
                m_ctx ), UNO_QUERY_THROW );

            xParser->setDocumentHandler( ::xmlscript::importLibraryContainer( &out_elements ) );
            xml::sax::InputSource source;
            source.aInputStream = ucb_content.openStream();
            source.sSystemId = ucb_content.getURL();
            xParser->parseStream( source );
        }
       //else
       //The file need not exists
    }
    /* Checks if basic package is enabled in StarOffice 8. This is the case when the dialog.xlc or
    the script.xlc in the user installation contains an entry for this package.
    The passed package MUST be a basic package.
    */
    bool ExtensionMigration::isBasicPackageEnabled( const uno::Reference< deployment::XPackage > & xPkg)
    {
        ::rtl::OUString sScriptURL = xPkg->getURL();
        if ( sScriptURL[ sScriptURL.getLength()-1 ] != '/' )
            sScriptURL += ::rtl::OUString::createFromAscii("/");
        sScriptURL += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("script.xlb") );

        bool bEntryFound = false;
        for ( sal_Int32 nPos = m_scriptElements.mnLibCount; nPos--; )
        {
            ::xmlscript::LibDescriptor const & descr =
                m_scriptElements.mpLibs[ nPos ];

            if (descr.aStorageURL.equals(sScriptURL))
            {
               bEntryFound = true;
               break;
            }
        }

        ::rtl::OUString sDialogURL = xPkg->getURL();
        if ( sDialogURL[ sDialogURL.getLength()-1 ] != '/' )
            sDialogURL += ::rtl::OUString::createFromAscii("/");
         sScriptURL += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("dialog.xlb") );

        if (!bEntryFound)
        {
            for ( sal_Int32 nPos = m_dialogElements.mnLibCount; nPos--; )
            {
                ::xmlscript::LibDescriptor const & descr =
                    m_dialogElements.mpLibs[ nPos ];

                if (descr.aStorageURL.equals(sDialogURL))
                {
                    bEntryFound = true;
                    break;
                }
            }
        }
        return bEntryFound;
    }
    /* This function only registers basic and dialog packages.
    */
    void ExtensionMigration::registerBasicPackage( const uno::Reference< deployment::XPackage > & xPkg)
    {
        const ::rtl::OUString sMediaType = xPkg->getPackageType()->getMediaType();
        if ( (sMediaType.equals(sBasicType) || sMediaType.equals(sDialogType))
            && isBasicPackageEnabled(xPkg))
        {
            xPkg->registerPackage(uno::Reference< task::XAbortChannel >(),
                uno::Reference< ucb::XCommandEnvironment> ());
        }
    }

    void ExtensionMigration::registerConfigurationPackage( const uno::Reference< deployment::XPackage > & xPkg)
    {
        const ::rtl::OUString sMediaType = xPkg->getPackageType()->getMediaType();
        if ( (sMediaType.equals(sConfigurationDataType) || sMediaType.equals(sConfigurationSchemaType) ) )
        {
            xPkg->revokePackage(uno::Reference< task::XAbortChannel >(), uno::Reference< ucb::XCommandEnvironment> ());
            xPkg->registerPackage(uno::Reference< task::XAbortChannel >(), uno::Reference< ucb::XCommandEnvironment> ());
        }
    }

    bool ExtensionMigration::processExtensions( const ::rtl::OUString& sSourceDir, const ::rtl::OUString& sTargetDir )
    {
        if (!copy(sSourceDir, sTargetDir))
            return false;

        // Find all basic and script packages and reregister them
        uno::Reference< deployment::XPackageManagerFactory > xPMF;
        if (! ( m_ctx->getValueByName( ::rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.deployment.thePackageManagerFactory")))
            >>= xPMF))
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "ExtensionsMigration: could not get thePackageManagerFactory")), 0);

        const uno::Reference< deployment::XPackageManager > xPackageMgr =
            xPMF->getPackageManager(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("user")));

        if (!xPackageMgr.is())
            throw uno::RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
            "ExtensionsMigration: could not get XPackageManager")), 0);

        const uno::Sequence< uno::Reference< deployment::XPackage > > allPackages =
            xPackageMgr->getDeployedPackages(
            uno::Reference< task::XAbortChannel >(), uno::Reference< ucb::XCommandEnvironment> ());

        for (int i = 0; i < allPackages.getLength(); i ++)
        {
            const uno::Reference< deployment::XPackage > aPackage = allPackages[i];
            if ( aPackage->isBundle() )
            {
                const uno::Sequence< uno::Reference < deployment::XPackage > > seqPkg =
                    aPackage->getBundle(
                        uno::Reference< task::XAbortChannel >(),
                        uno::Reference< ucb::XCommandEnvironment> ());

                for ( int k = 0; k < seqPkg.getLength(); k++ )
                    registerBasicPackage(seqPkg[k]);

                for (int l = 0; l < seqPkg.getLength(); l++)
                {
                    const ::rtl::OUString sMediaType = seqPkg[l]->getPackageType()->getMediaType();
                    beans::Optional<beans::Ambiguous<sal_Bool> > opt =
                        seqPkg[l]->isRegistered(uno::Reference< task::XAbortChannel >(), uno::Reference< ucb::XCommandEnvironment> ());
                    bool bRegistered = opt.IsPresent && opt.Value.IsAmbiguous == sal_False && opt.Value.Value == sal_True ? true : false;

                    if ( bRegistered && !sMediaType.equals(sBasicType) && !sMediaType.equals(sDialogType) )
                    {
                        seqPkg[l]->revokePackage(uno::Reference< task::XAbortChannel >(), uno::Reference< ucb::XCommandEnvironment> ());
                        seqPkg[l]->registerPackage(uno::Reference< task::XAbortChannel >(), uno::Reference< ucb::XCommandEnvironment> ());
                    }
                }
            }
            else
            {
                registerBasicPackage(aPackage);
                {
                    aPackage->revokePackage(uno::Reference< task::XAbortChannel >(), uno::Reference< ucb::XCommandEnvironment> ());
                    aPackage->registerPackage(uno::Reference< task::XAbortChannel >(), uno::Reference< ucb::XCommandEnvironment> ());
                }
            }
        }


        return true;

    }

bool ExtensionMigration::isCompatibleBerkleyDb(const ::rtl::OUString& sSourceDir)
{
    try
    {
    ::rtl::OUString sDb(sSourceDir + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "/uno_packages.db")));
    //check if the db exist at all. If not then the call to db_create would create
    //the file.
    ::osl::File f(sDb);
    if (::osl::File::E_None != f.open(OpenFlag_Read))
    {
        f.close();
        return false;
    }
    f.close();

    //create a system path
    ::rtl::OUString sSysPath;
    if (::osl::File::getSystemPathFromFileURL(sDb, sSysPath ) != ::osl::File::E_None)
        return false;

    ::rtl::OString cstr_sysPath(
        ::rtl::OUStringToOString( sSysPath, osl_getThreadTextEncoding() ) );
    char const * pcstr_sysPath = cstr_sysPath.getStr();

    //Open the db. If it works then we assume that the file was written with a
    //compatible version of Berkeley Db
    DB* pDB = NULL;
    //using  DB_RDONLY will return an "Invalid argument" error.
    //DB_CREATE: only creates the file if it does not exist.
    //An existing db is not modified.
    if (0 != db_create(& pDB, 0, DB_CREATE))
        return false;

    if (0 != pDB->open(pDB, 0, pcstr_sysPath , 0, DB_HASH, DB_RDONLY, 0664 /* fs mode */))
        return false;

    pDB->close(pDB, 0);
    }
    catch (uno::Exception& )
    {
        return false;
    }

    return true;
}

bool ExtensionMigration::copy( const ::rtl::OUString& sSourceDir, const ::rtl::OUString& sTargetDir )
{
    bool bRet = false;
    if (! isCompatibleBerkleyDb(sSourceDir))
        return false;

    INetURLObject aSourceObj( sSourceDir );
    INetURLObject aDestObj( sTargetDir );
    String aName = aDestObj.getName();
    aDestObj.removeSegment();
    aDestObj.setFinalSlash();

    try
    {
        ::ucbhelper::Content aDestPath( aDestObj.GetMainURL( INetURLObject::NO_DECODE ), uno::Reference< ucb::XCommandEnvironment > () );
        uno::Reference< ucb::XCommandInfo > xInfo = aDestPath.getCommands();
        ::rtl::OUString aTransferName = ::rtl::OUString::createFromAscii( "transfer" );
        if ( xInfo->hasCommandByName( aTransferName ) )
        {
            aDestPath.executeCommand( aTransferName, uno::makeAny(
                ucb::TransferInfo( sal_False, aSourceObj.GetMainURL( INetURLObject::NO_DECODE ), aName, ucb::NameClash::OVERWRITE ) ) );
            bRet = true;
        }
    }
    catch( uno::Exception& )
    {
    }

    return bRet;
}


    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------

    ::rtl::OUString ExtensionMigration::getImplementationName() throw (RuntimeException)
    {
        return ExtensionMigration_getImplementationName();
    }

    // -----------------------------------------------------------------------------

    sal_Bool ExtensionMigration::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
        const ::rtl::OUString* pNames = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pNames + aNames.getLength();
        for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
            ;

        return pNames != pEnd;
    }

    // -----------------------------------------------------------------------------

    Sequence< ::rtl::OUString > ExtensionMigration::getSupportedServiceNames() throw (RuntimeException)
    {
        return ExtensionMigration_getSupportedServiceNames();
    }

    // -----------------------------------------------------------------------------
    // XInitialization
    // -----------------------------------------------------------------------------

    void ExtensionMigration::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        const Any* pIter = aArguments.getConstArray();
        const Any* pEnd = pIter + aArguments.getLength();
        for ( ; pIter != pEnd ; ++pIter )
        {
            beans::NamedValue aValue;
            *pIter >>= aValue;
            if ( aValue.Name.equalsAscii( "UserData" ) )
            {
                if ( !(aValue.Value >>= m_sSourceDir) )
                {
                    OSL_ENSURE( false, "ExtensionMigration::initialize: argument UserData has wrong type!" );
                }
                break;
            }
        }
        prepareBasicLibs();
    }

    TStringVectorPtr getContent( const ::rtl::OUString& rBaseURL )
    {
        TStringVectorPtr aResult( new TStringVector );
        ::osl::Directory aDir( rBaseURL);
        if ( aDir.open() == ::osl::FileBase::E_None )
        {
            // iterate over directory content
            TStringVector aSubDirs;
            ::osl::DirectoryItem aItem;
            while ( aDir.getNextItem( aItem ) == ::osl::FileBase::E_None )
            {
                ::osl::FileStatus aFileStatus( FileStatusMask_Type | FileStatusMask_FileURL );
                if ( aItem.getFileStatus( aFileStatus ) == ::osl::FileBase::E_None )
                    aResult->push_back( aFileStatus.getFileURL() );
            }
        }

        return aResult;
    }

    // -----------------------------------------------------------------------------
    // XJob
    // -----------------------------------------------------------------------------

void ExtensionMigration::copyConfig( const ::rtl::OUString& sSourceDir, const ::rtl::OUString& sTargetDir )
{
    ::rtl::OUString sEx1( m_sSourceDir );
    sEx1 += sExcludeDir1;
    ::rtl::OUString sEx2( m_sSourceDir );
    sEx2 += sExcludeDir2;

    TStringVectorPtr aList = getContent( sSourceDir );
    TStringVector::const_iterator aI = aList->begin();
    while ( aI != aList->end() )
    {
        ::rtl::OUString sSourceLocalName = aI->copy( sSourceDir.getLength() );
        ::rtl::OUString aTemp = aI->copy( m_sSourceDir.getLength() );
        if ( aTemp != sExcludeDir1 && aTemp != sExcludeDir2 )
        {
            ::rtl::OUString sTargetName = sTargetDir + sSourceLocalName;
            copy( (*aI), sTargetName );
        }
        ++aI;
    }
}

    Any ExtensionMigration::execute( const Sequence< beans::NamedValue >& )
        throw (lang::IllegalArgumentException, Exception, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        ::utl::Bootstrap::PathStatus aStatus = ::utl::Bootstrap::locateUserInstallation( m_sTargetDir );
        if ( aStatus == ::utl::Bootstrap::PATH_EXISTS )
        {
            // copy all extensions
            ::rtl::OUString sTargetDir(m_sTargetDir), sSourceDir( m_sSourceDir );
            sTargetDir += sExtensionSubDir;
            sSourceDir += sExtensionSubDir;
            sSourceDir += sSubDirName;
            sTargetDir += sSubDirName;
            processExtensions( sSourceDir, sTargetDir );

            // copy all user config settings in user/registry/data (except user/registry/data/org)
            sSourceDir = m_sSourceDir;
            sSourceDir += sConfigDir;
            sTargetDir = m_sTargetDir;
            sTargetDir += sConfigDir;
            copyConfig( sSourceDir, sTargetDir );

            // copy all user config settings in user/registry/data/org (except user/registry/data/org/openoffice)
            sSourceDir = m_sSourceDir;
            sSourceDir += sOrgDir;
            sTargetDir = m_sTargetDir;
            sTargetDir += sOrgDir;
            copyConfig( sSourceDir, sTargetDir );
        }

        return Any();
    }

    // =============================================================================
    // component operations
    // =============================================================================

    Reference< XInterface > SAL_CALL ExtensionMigration_create(
        Reference< XComponentContext > const & ctx )
        SAL_THROW( () )
    {
        return static_cast< lang::XTypeProvider * >( new ExtensionMigration(
            ctx) );
    }

    // -----------------------------------------------------------------------------

//.........................................................................
}   // namespace migration
//.........................................................................
