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
#include <sal/log.hxx>
#include <osl/file.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <unotools/bootstrap.hxx>
#include <unotools/textsearch.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>

#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/xml/xpath/XPathAPI.hpp>
#include <com/sun/star/xml/xpath/XPathException.hpp>
#include <com/sun/star/xml/dom/DOMException.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <com/sun/star/deployment/XExtensionManager.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace migration
{

// ExtensionMigration


OO3ExtensionMigration::OO3ExtensionMigration(Reference< XComponentContext > const & ctx) :
m_ctx(ctx)
{
}


OO3ExtensionMigration::~OO3ExtensionMigration()
{
}

void OO3ExtensionMigration::scanUserExtensions( const OUString& sSourceDir, TStringVector& aMigrateExtensions )
{
    osl::Directory    aScanRootDir( sSourceDir );
    osl::FileStatus   fs(osl_FileStatus_Mask_Type | osl_FileStatus_Mask_FileURL);
    osl::FileBase::RC nRetCode = aScanRootDir.open();
    if ( nRetCode != osl::Directory::E_None )
        return;

    sal_uInt32    nHint( 0 );
    osl::DirectoryItem aItem;
    while ( aScanRootDir.getNextItem( aItem, nHint ) == osl::Directory::E_None )
    {
        if (( aItem.getFileStatus(fs) == osl::FileBase::E_None ) &&
            ( fs.getFileType() == osl::FileStatus::Directory   ))
        {
            //Check next folder as the "real" extension folder is below a temp folder!
            OUString sExtensionFolderURL = fs.getFileURL();

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

OO3ExtensionMigration::ScanResult OO3ExtensionMigration::scanExtensionFolder( const OUString& sExtFolder )
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
                if (fs.getFileType() == osl::FileStatus::Directory)
                    aDirectories.push_back( fs.getFileURL() );
                else
                {
                    OUString aDirEntryURL = fs.getFileURL();
                    if ( aDirEntryURL.indexOf( "/description.xml" ) > 0 )
                        aResult = scanDescriptionXml( aDirEntryURL ) ? SCANRESULT_MIGRATE_EXTENSION : SCANRESULT_DONTMIGRATE_EXTENSION;
                }
            }
        }

        for (auto const& directory : aDirectories)
        {
            aResult = scanExtensionFolder(directory);
            if (aResult != SCANRESULT_NOTFOUND)
                break;
        }
    }
    return aResult;
}

bool OO3ExtensionMigration::scanDescriptionXml( const OUString& sDescriptionXmlURL )
{
    if ( !m_xDocBuilder.is() )
    {
        m_xDocBuilder.set( xml::dom::DocumentBuilder::create(m_ctx) );
    }

    if ( !m_xSimpleFileAccess.is() )
    {
        m_xSimpleFileAccess = ucb::SimpleFileAccess::create(m_ctx);
    }

    OUString aExtIdentifier;
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
                    uno::Reference< xml::xpath::XXPathAPI > xPath = xml::xpath::XPathAPI::create(m_ctx);

                    xPath->registerNS("desc", xRoot->getNamespaceURI());
                    xPath->registerNS("xlink", "http://www.w3.org/1999/xlink");

                    try
                    {
                        uno::Reference< xml::dom::XNode > xNode(
                            xPath->selectSingleNode(
                                xRoot, "desc:identifier/@value" ));
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
            for (const OUString & i : m_aDenyList)
            {
                utl::SearchParam param(i, utl::SearchParam::SearchType::Regexp);
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
        for (const OUString & i : m_aDenyList)
        {
            utl::SearchParam param(i, utl::SearchParam::SearchType::Regexp);
            utl::TextSearch  ts(param, LANGUAGE_DONTKNOW);

            sal_Int32 start = 0;
            sal_Int32 end   = sDescriptionXmlURL.getLength();
            if (ts.SearchForward(sDescriptionXmlURL, &start, &end))
                return false;
        }
    }

    return true;
}

void OO3ExtensionMigration::migrateExtension( const OUString& sSourceDir )
{
    css::uno::Reference< css::deployment::XExtensionManager > extMgr(
        deployment::ExtensionManager::get( m_ctx ) );
    try
    {
        rtl::Reference<TmpRepositoryCommandEnv> pCmdEnv = new TmpRepositoryCommandEnv();

        uno::Reference< task::XAbortChannel > xAbortChannel;
        extMgr->addExtension(
            sSourceDir, uno::Sequence<beans::NamedValue>(), "user",
            xAbortChannel, pCmdEnv );
    }
    catch ( css::uno::Exception & )
    {
        TOOLS_WARN_EXCEPTION(
            "desktop.migration",
            "Ignoring UNO Exception while migrating extension from <" << sSourceDir << ">");
    }
}


// XServiceInfo


OUString OO3ExtensionMigration::getImplementationName()
{
    return "com.sun.star.comp.desktop.migration.OOo3Extensions";
}


sal_Bool OO3ExtensionMigration::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}


Sequence< OUString > OO3ExtensionMigration::getSupportedServiceNames()
{
    return { "com.sun.star.migration.Extensions" };
}


// XInitialization


void OO3ExtensionMigration::initialize( const Sequence< Any >& aArguments )
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
        else if ( aValue.Name == "ExtensionDenyList" )
        {
            Sequence< OUString > aDenyList;
            if ( (aValue.Value >>= aDenyList ) && aDenyList.hasElements())
            {
                m_aDenyList.resize( aDenyList.getLength() );
                ::comphelper::sequenceToArray< OUString >( m_aDenyList.data(), aDenyList );
            }
        }
    }
}

Any OO3ExtensionMigration::execute( const Sequence< beans::NamedValue >& )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::utl::Bootstrap::PathStatus aStatus = ::utl::Bootstrap::locateUserInstallation( m_sTargetDir );
    if ( aStatus == ::utl::Bootstrap::PATH_EXISTS )
    {
        // copy all extensions
        OUString sSourceDir = m_sSourceDir +
            "/user/uno_packages/cache/uno_packages";
        TStringVector aExtensionToMigrate;
        scanUserExtensions( sSourceDir, aExtensionToMigrate );
        for (auto const& extensionToMigrate : aExtensionToMigrate)
        {
            migrateExtension(extensionToMigrate);
        }
    }

    return Any();
}


// TmpRepositoryCommandEnv


TmpRepositoryCommandEnv::TmpRepositoryCommandEnv()
{
}

TmpRepositoryCommandEnv::~TmpRepositoryCommandEnv()
{
}
// XCommandEnvironment

uno::Reference< task::XInteractionHandler > TmpRepositoryCommandEnv::getInteractionHandler()
{
    return this;
}


uno::Reference< ucb::XProgressHandler > TmpRepositoryCommandEnv::getProgressHandler()
{
    return this;
}

// XInteractionHandler
void TmpRepositoryCommandEnv::handle(
    uno::Reference< task::XInteractionRequest> const & xRequest )
{
    OSL_ASSERT( xRequest->getRequest().getValueTypeClass() == uno::TypeClass_EXCEPTION );

    bool approve = true;

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
    }
}

// XProgressHandler
void TmpRepositoryCommandEnv::push( uno::Any const & /*Status*/ )
{
}


void TmpRepositoryCommandEnv::update( uno::Any const & /*Status */)
{
}

void TmpRepositoryCommandEnv::pop()
{
}


}   // namespace migration


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
desktop_OO3ExtensionMigration_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new migration::OO3ExtensionMigration(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
