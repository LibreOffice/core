/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "oo3extensionmigration.hxx"
#include <rtl/instance.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <tools/urlobj.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/textsearch.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/processfactory.hxx>
#include <ucbhelper/content.hxx>

#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/xml/xpath/XXPathAPI.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/deployment/XExtensionManager.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace migration
{

static ::rtl::OUString sExtensionSubDir = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/user/uno_packages/" ) );
static ::rtl::OUString sSubDirName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cache" ) );
static ::rtl::OUString sDescriptionXmlFile = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/description.xml" ) );
static ::rtl::OUString sExtensionRootSubDirName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/uno_packages" ) );

// =============================================================================
// component operations
// =============================================================================

::rtl::OUString OO3ExtensionMigration_getImplementationName()
{
    static ::rtl::OUString* pImplName = 0;
    if ( !pImplName )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if ( !pImplName )
        {
            static ::rtl::OUString aImplName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.desktop.migration.OOo3Extensions" ) );
            pImplName = &aImplName;
        }
    }
    return *pImplName;
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > OO3ExtensionMigration_getSupportedServiceNames()
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

OO3ExtensionMigration::OO3ExtensionMigration(Reference< XComponentContext > const & ctx) :
m_ctx(ctx)
{
}

// -----------------------------------------------------------------------------

OO3ExtensionMigration::~OO3ExtensionMigration()
{
}

::osl::FileBase::RC OO3ExtensionMigration::checkAndCreateDirectory( INetURLObject& rDirURL )
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

void OO3ExtensionMigration::scanUserExtensions( const ::rtl::OUString& sSourceDir, TStringVector& aMigrateExtensions )
{
    osl::Directory    aScanRootDir( sSourceDir );
    osl::FileStatus   fs(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL);
    osl::FileBase::RC nRetCode = aScanRootDir.open();
    if ( nRetCode == osl::Directory::E_None )
    {
        sal_uInt32    nHint( 0 );
        osl::DirectoryItem aItem;
        while ( aScanRootDir.getNextItem( aItem, nHint ) == osl::Directory::E_None )
        {
            if (( aItem.getFileStatus(fs) == osl::FileBase::E_None ) &&
                ( fs.getFileType() == osl::FileStatus::Directory   ))
            {
                //Check next folder as the "real" extension folder is below a temp folder!
                ::rtl::OUString sExtensionFolderURL = fs.getFileURL();

                osl::Directory     aExtensionRootDir( sExtensionFolderURL );

                nRetCode = aExtensionRootDir.open();
                if ( nRetCode == osl::Directory::E_None )
                {
                    osl::DirectoryItem aExtDirItem;
                    while ( aExtensionRootDir.getNextItem( aExtDirItem, nHint ) == osl::Directory::E_None )
                    {
                        bool bFileStatus = aExtDirItem.getFileStatus(fs) == osl::FileBase::E_None;
                        bool bIsDir      = fs.getFileType() == osl::FileStatus::Directory;

                        if ( bFileStatus && bIsDir )
                        {
                            sExtensionFolderURL = fs.getFileURL();
                            ScanResult eResult = scanExtensionFolder( sExtensionFolderURL );
                            if ( eResult == SCANRESULT_MIGRATE_EXTENSION )
                                aMigrateExtensions.push_back( sExtensionFolderURL );
                            break;
                        }
                    }
                }
            }
        }
    }
}

OO3ExtensionMigration::ScanResult OO3ExtensionMigration::scanExtensionFolder( const ::rtl::OUString& sExtFolder )
{
    ScanResult     aResult = SCANRESULT_NOTFOUND;
    osl::Directory aDir(sExtFolder);

    // get sub dirs
    if (aDir.open() == osl::FileBase::E_None)
    {
        // work through directory contents...
        osl::DirectoryItem item;
        osl::FileStatus fs(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL);
        TStringVector aDirectories;
        while ((aDir.getNextItem(item) == osl::FileBase::E_None ) &&
               ( aResult == SCANRESULT_NOTFOUND ))
        {
            if (item.getFileStatus(fs) == osl::FileBase::E_None)
            {
                ::rtl::OUString aDirEntryURL;
                if (fs.getFileType() == osl::FileStatus::Directory)
                    aDirectories.push_back( fs.getFileURL() );
                else
                {
                    aDirEntryURL = fs.getFileURL();
                    if ( aDirEntryURL.indexOf( sDescriptionXmlFile ) > 0 )
                        aResult = scanDescriptionXml( aDirEntryURL ) ? SCANRESULT_MIGRATE_EXTENSION : SCANRESULT_DONTMIGRATE_EXTENSION;
                }
            }
        }

        TStringVector::const_iterator pIter = aDirectories.begin();
        while ( pIter != aDirectories.end() && aResult == SCANRESULT_NOTFOUND )
        {
            aResult = scanExtensionFolder( *pIter );
            ++pIter;
        }
    }
    return aResult;
}

bool OO3ExtensionMigration::scanDescriptionXml( const ::rtl::OUString& sDescriptionXmlURL )
{
    if ( !m_xDocBuilder.is() )
    {
        m_xDocBuilder = uno::Reference< xml::dom::XDocumentBuilder >( xml::dom::DocumentBuilder::create(m_ctx) );
    }

    if ( !m_xSimpleFileAccess.is() )
    {
        m_xSimpleFileAccess = ucb::SimpleFileAccess::create(m_ctx);
    }

    ::rtl::OUString aExtIdentifier;
    try
    {
        uno::Reference< io::XInputStream > xIn =
            m_xSimpleFileAccess->openFileRead( sDescriptionXmlURL );

        if ( xIn.is() )
        {
            uno::Reference< xml::dom::XDocument > xDoc = m_xDocBuilder->parse( xIn );
            if ( xDoc.is() )
            {
                uno::Reference< xml::dom::XElement > xRoot = xDoc->getDocumentElement();
                if ( xRoot.is() && xRoot->getTagName() == "description" )
                {
                    uno::Reference< xml::xpath::XXPathAPI > xPath(
                        m_ctx->getServiceManager()->createInstanceWithContext(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.xpath.XPathAPI")),
                            m_ctx),
                        uno::UNO_QUERY);

                    xPath->registerNS(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("desc")),
                        xRoot->getNamespaceURI());
                    xPath->registerNS(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("xlink")),
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("http://www.w3.org/1999/xlink")));

                    try
                    {
                        uno::Reference< xml::dom::XNode > xRootNode( xRoot, uno::UNO_QUERY );
                        uno::Reference< xml::dom::XNode > xNode(
                            xPath->selectSingleNode(
                                xRootNode,
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("desc:identifier/@value")) ));
                        if ( xNode.is() )
                            aExtIdentifier = xNode->getNodeValue();
                    }
                    catch ( const xml::xpath::XPathException& )
                    {
                    }
                    catch ( const xml::dom::DOMException& )
                    {
                    }
                }
            }
        }

        if ( !aExtIdentifier.isEmpty() )
        {
            // scan extension identifier and try to match with our black list entries
            for ( sal_uInt32 i = 0; i < m_aBlackList.size(); i++ )
            {
                utl::SearchParam param(m_aBlackList[i], utl::SearchParam::SRCH_REGEXP);
                utl::TextSearch  ts(param, LANGUAGE_DONTKNOW);

                sal_Int32 start = 0;
                sal_Int32 end = aExtIdentifier.getLength();
                if (ts.SearchForward(aExtIdentifier, &start, &end))
                    return false;
            }
        }
    }
    catch ( const ucb::CommandAbortedException& )
    {
    }
    catch ( const uno::RuntimeException& )
    {
    }

    if ( aExtIdentifier.isEmpty() )
    {
        // Fallback:
        // Try to use the folder name to match our black list
        // as some extensions don't provide an identifier in the
        // description.xml!
        for ( sal_uInt32 i = 0; i < m_aBlackList.size(); i++ )
        {
            utl::SearchParam param(m_aBlackList[i], utl::SearchParam::SRCH_REGEXP);
            utl::TextSearch  ts(param, LANGUAGE_DONTKNOW);

            xub_StrLen start = 0;
            xub_StrLen end   = static_cast<sal_uInt16>(sDescriptionXmlURL.getLength());
            if (ts.SearchFrwrd(sDescriptionXmlURL, &start, &end))
                return false;
        }
    }

    return true;
}

void OO3ExtensionMigration::migrateExtension( const ::rtl::OUString& sSourceDir )
{
    css::uno::Reference< css::deployment::XExtensionManager > extMgr(
        deployment::ExtensionManager::get( m_ctx ) );
    try
    {
        TmpRepositoryCommandEnv* pCmdEnv = new TmpRepositoryCommandEnv();

        uno::Reference< ucb::XCommandEnvironment > xCmdEnv(
            static_cast< cppu::OWeakObject* >( pCmdEnv ), uno::UNO_QUERY );
        uno::Reference< task::XAbortChannel > xAbortChannel;
        extMgr->addExtension(
            sSourceDir, uno::Sequence<beans::NamedValue>(),
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("user")), xAbortChannel, xCmdEnv );
    }
    catch ( css::uno::Exception & e )
    {
        SAL_WARN(
            "desktop.migration",
            "Ignoring UNO Exception while migrating extension from <"
            << sSourceDir << ">: \"" << e.Message << "\"");
    }
}


// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString OO3ExtensionMigration::getImplementationName() throw (RuntimeException)
{
    return OO3ExtensionMigration_getImplementationName();
}

// -----------------------------------------------------------------------------

sal_Bool OO3ExtensionMigration::supportsService( const ::rtl::OUString& rServiceName ) throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames( getSupportedServiceNames() );
    const ::rtl::OUString* pNames = aNames.getConstArray();
    const ::rtl::OUString* pEnd = pNames + aNames.getLength();
    for ( ; pNames != pEnd && !pNames->equals( rServiceName ); ++pNames )
        ;

    return pNames != pEnd;
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > OO3ExtensionMigration::getSupportedServiceNames() throw (RuntimeException)
{
    return OO3ExtensionMigration_getSupportedServiceNames();
}

// -----------------------------------------------------------------------------
// XInitialization
// -----------------------------------------------------------------------------

void OO3ExtensionMigration::initialize( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    const Any* pIter = aArguments.getConstArray();
    const Any* pEnd = pIter + aArguments.getLength();
    for ( ; pIter != pEnd ; ++pIter )
    {
        beans::NamedValue aValue;
        *pIter >>= aValue;
        if ( aValue.Name == "UserData" )
        {
            if ( !(aValue.Value >>= m_sSourceDir) )
            {
                OSL_FAIL( "ExtensionMigration::initialize: argument UserData has wrong type!" );
            }
        }
        else if ( aValue.Name == "ExtensionBlackList" )
        {
            Sequence< ::rtl::OUString > aBlackList;
            if ( (aValue.Value >>= aBlackList ) && ( aBlackList.getLength() > 0 ))
            {
                m_aBlackList.resize( aBlackList.getLength() );
                ::comphelper::sequenceToArray< ::rtl::OUString >( &m_aBlackList[0], aBlackList );
            }
        }
    }
}

Any OO3ExtensionMigration::execute( const Sequence< beans::NamedValue >& )
    throw (lang::IllegalArgumentException, Exception, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::utl::Bootstrap::PathStatus aStatus = ::utl::Bootstrap::locateUserInstallation( m_sTargetDir );
    if ( aStatus == ::utl::Bootstrap::PATH_EXISTS )
    {
        // copy all extensions
        ::rtl::OUString sSourceDir( m_sSourceDir );
        sSourceDir += sExtensionSubDir;
        sSourceDir += sSubDirName;
        sSourceDir += sExtensionRootSubDirName;
        TStringVector aExtensionToMigrate;
        scanUserExtensions( sSourceDir, aExtensionToMigrate );
        if ( aExtensionToMigrate.size() > 0 )
        {
            TStringVector::iterator pIter = aExtensionToMigrate.begin();
            while ( pIter != aExtensionToMigrate.end() )
            {
                migrateExtension( *pIter );
                ++pIter;
            }
        }
    }

    return Any();
}

// -----------------------------------------------------------------------------
// TmpRepositoryCommandEnv
// -----------------------------------------------------------------------------

TmpRepositoryCommandEnv::TmpRepositoryCommandEnv()
{
}

TmpRepositoryCommandEnv::~TmpRepositoryCommandEnv()
{
}
// XCommandEnvironment
//______________________________________________________________________________
uno::Reference< task::XInteractionHandler > TmpRepositoryCommandEnv::getInteractionHandler()
throw ( uno::RuntimeException )
{
    return this;
}

//______________________________________________________________________________
uno::Reference< ucb::XProgressHandler > TmpRepositoryCommandEnv::getProgressHandler()
throw ( uno::RuntimeException )
{
    return this;
}

// XInteractionHandler
void TmpRepositoryCommandEnv::handle(
    uno::Reference< task::XInteractionRequest> const & xRequest )
    throw ( uno::RuntimeException )
{
    uno::Any request( xRequest->getRequest() );
    OSL_ASSERT( request.getValueTypeClass() == uno::TypeClass_EXCEPTION );

    bool approve = true;
    bool abort   = false;

    // select:
    uno::Sequence< Reference< task::XInteractionContinuation > > conts(
        xRequest->getContinuations() );
    Reference< task::XInteractionContinuation > const * pConts =
        conts.getConstArray();
    sal_Int32 len = conts.getLength();
    for ( sal_Int32 pos = 0; pos < len; ++pos )
    {
        if (approve) {
            uno::Reference< task::XInteractionApprove > xInteractionApprove(
                pConts[ pos ], uno::UNO_QUERY );
            if (xInteractionApprove.is()) {
                xInteractionApprove->select();
                // don't query again for ongoing continuations:
                approve = false;
            }
        }
        else if (abort) {
            uno::Reference< task::XInteractionAbort > xInteractionAbort(
                pConts[ pos ], uno::UNO_QUERY );
            if (xInteractionAbort.is()) {
                xInteractionAbort->select();
                // don't query again for ongoing continuations:
                abort = false;
            }
        }
    }
}

// XProgressHandler
void TmpRepositoryCommandEnv::push( uno::Any const & /*Status*/ )
throw (uno::RuntimeException)
{
}


void TmpRepositoryCommandEnv::update( uno::Any const & /*Status */)
throw (uno::RuntimeException)
{
}

void TmpRepositoryCommandEnv::pop() throw (uno::RuntimeException)
{
}

// =============================================================================
// component operations
// =============================================================================

Reference< XInterface > SAL_CALL OO3ExtensionMigration_create(
    Reference< XComponentContext > const & ctx )
    SAL_THROW(())
{
    return static_cast< lang::XTypeProvider * >( new OO3ExtensionMigration(
        ctx) );
}

// -----------------------------------------------------------------------------

}   // namespace migration

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
