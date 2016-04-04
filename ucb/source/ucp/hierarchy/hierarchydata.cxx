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


/**************************************************************************
                                TODO
 **************************************************************************

 - HierarchyEntry::move
   --> Rewrite to use XNamed ( once this is supported by config db api ).

 *************************************************************************/
#include "hierarchydata.hxx"

#include <vector>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/XOfficeInstallationDirectories.hpp>
#include "hierarchyprovider.hxx"
#include "hierarchyuri.hxx"

using namespace com::sun::star;

namespace hierarchy_ucp
{


struct HierarchyEntry::iterator_Impl
{
    HierarchyEntryData                                     entry;
    uno::Reference< container::XHierarchicalNameAccess >   dir;
    uno::Reference< util::XOfficeInstallationDirectories > officeDirs;
    uno::Sequence< OUString>                          names;
    sal_Int32                                              pos;
    iterator_Impl()
    : officeDirs( nullptr ), pos( -1 /* before first */ ) {};
};


void makeXMLName( const OUString & rIn, OUStringBuffer & rBuffer  )
{
    sal_Int32 nCount = rIn.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const sal_Unicode c = rIn[ n ];
        switch ( c )
        {
            case '&':
                rBuffer.append( "&amp;" );
                break;

            case '"':
                rBuffer.append( "&quot;" );
                break;

            case '\'':
                rBuffer.append( "&apos;" );
                break;

            case '<':
                rBuffer.append( "&lt;" );
                break;

            case '>':
                rBuffer.append( "&gt;" );
                break;

            default:
                rBuffer.append( c );
                break;
        }
    }
}


// HierarchyEntry Implementation.


#define READ_SERVICE_NAME      "com.sun.star.ucb.HierarchyDataReadAccess"
#define READWRITE_SERVICE_NAME "com.sun.star.ucb.HierarchyDataReadWriteAccess"

// describe path of cfg entry
#define CFGPROPERTY_NODEPATH    "nodepath"


HierarchyEntry::HierarchyEntry(
                const uno::Reference< uno::XComponentContext >& rxContext,
                HierarchyContentProvider* pProvider,
                const OUString& rURL )
: m_xContext( rxContext ),
  m_xOfficeInstDirs( pProvider->getOfficeInstallationDirectories() ),
  m_bTriedToGetRootReadAccess( false )
{
    HierarchyUri aUri( rURL );
    m_aServiceSpecifier = aUri.getService();

    m_xConfigProvider
        = pProvider->getConfigProvider( m_aServiceSpecifier );
    m_xRootReadAccess
        = pProvider->getRootConfigReadNameAccess( m_aServiceSpecifier );

    // Note: do not init m_aPath in init list. createPathFromHierarchyURL
    //       needs m_xContext and m_aMutex.
    m_aPath = createPathFromHierarchyURL( aUri );

    // Extract language independent name from URL.
    sal_Int32 nPos = rURL.lastIndexOf( '/' );
    if ( nPos > HIERARCHY_URL_SCHEME_LENGTH )
        m_aName = rURL.copy( nPos + 1 );
    else
        OSL_FAIL( "HierarchyEntry - Invalid URL!" );
}


bool HierarchyEntry::hasData()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    uno::Reference< container::XHierarchicalNameAccess > xRootReadAccess
        = getRootReadAccess();

    OSL_ENSURE( xRootReadAccess.is(), "HierarchyEntry::hasData - No root!" );

    if ( xRootReadAccess.is() )
        return xRootReadAccess->hasByHierarchicalName( m_aPath );

    return false;
}


bool HierarchyEntry::getData( HierarchyEntryData& rData )
{
    try
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        uno::Reference< container::XHierarchicalNameAccess > xRootReadAccess
            = getRootReadAccess();

        OSL_ENSURE( xRootReadAccess.is(),
                    "HierarchyEntry::getData - No root!" );

        if ( xRootReadAccess.is() )
        {
            OUString aTitlePath = m_aPath;
            aTitlePath += "/Title";

            // Note: Avoid NoSuchElementExceptions, because exceptions are
            //       relatively 'expensive'. Checking for availability of
            //       title value is sufficient here, because if it is
            //       there, the other values will be available too.
            if ( !xRootReadAccess->hasByHierarchicalName( aTitlePath ) )
                return false;

            OUString aValue;

            // Get Title value.
            if ( !( xRootReadAccess->getByHierarchicalName( aTitlePath )
                    >>= aValue ) )
            {
                OSL_FAIL( "HierarchyEntry::getData - "
                            "Got no Title value!" );
                return false;
            }

            rData.setTitle( aValue );

            // Get TargetURL value.
            OUString aTargetURLPath = m_aPath;
            aTargetURLPath += "/TargetURL";
            if ( !( xRootReadAccess->getByHierarchicalName( aTargetURLPath )
                    >>= aValue ) )
            {
                OSL_FAIL( "HierarchyEntry::getData - "
                            "Got no TargetURL value!" );
                return false;
            }

            // TargetURL property may contain a reference to the Office
            // installation directory. To ensure a reloctable office
            // installation, the path to the office installtion directory must
            // never be stored directly. A placeholder is used instead. Replace
            // it by actual installation directory.
            if ( m_xOfficeInstDirs.is() &&  !aValue.isEmpty()  )
                aValue = m_xOfficeInstDirs->makeAbsoluteURL( aValue );
            rData.setTargetURL( aValue );

            OUString aTypePath = m_aPath;
            aTypePath += "/Type";
            if ( xRootReadAccess->hasByHierarchicalName( aTypePath ) )
            {
                // Might not be present since it was introduced long after
                // Title and TargetURL (#82433#)... So not getting it is
                // not an error.

                // Get Type value.
                sal_Int32 nType = 0;
                if ( xRootReadAccess->getByHierarchicalName( aTypePath )
                     >>= nType )
                {
                    if ( nType == 0 )
                    {
                        rData.setType( HierarchyEntryData::LINK );
                    }
                    else if ( nType == 1 )
                    {
                        rData.setType( HierarchyEntryData::FOLDER );
                    }
                    else
                    {
                        OSL_FAIL( "HierarchyEntry::getData - "
                                    "Unknown Type value!" );
                        return false;
                    }
                }
            }

            rData.setName( m_aName );
            return true;
        }
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( container::NoSuchElementException const & )
    {
        // getByHierarchicalName

        OSL_FAIL( "HierarchyEntry::getData - caught NoSuchElementException!" );
    }
    return false;
}


bool HierarchyEntry::setData( const HierarchyEntryData& rData )
{
    try
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( !m_xConfigProvider.is() )
            m_xConfigProvider.set(
                m_xContext->getServiceManager()->createInstanceWithContext(m_aServiceSpecifier, m_xContext),
                uno::UNO_QUERY );

        if ( m_xConfigProvider.is() )
        {
            // Create parent's key. It must exist!

            OUString aParentPath;
            bool bRoot = true;

            sal_Int32 nPos = m_aPath.lastIndexOf( '/' );
            if ( nPos != -1 )
            {
                // Skip "/Children" segment of the path, too.
                nPos = m_aPath.lastIndexOf( '/', nPos - 1 );

                OSL_ENSURE( nPos != -1,
                            "HierarchyEntry::setData - Wrong path!" );

                aParentPath += m_aPath.copy( 0, nPos );
                bRoot = false;
            }

            uno::Sequence< uno::Any > aArguments( 1 );
            beans::PropertyValue      aProperty;

            aProperty.Name    = CFGPROPERTY_NODEPATH;
            aProperty.Value <<= aParentPath;
            aArguments[ 0 ] <<= aProperty;

            uno::Reference< util::XChangesBatch > xBatch(
                    m_xConfigProvider->createInstanceWithArguments(
                        READWRITE_SERVICE_NAME,
                        aArguments ),
                    uno::UNO_QUERY );

            OSL_ENSURE( xBatch.is(),
                        "HierarchyEntry::setData - No batch!" );

            uno::Reference< container::XNameAccess > xParentNameAccess(
                xBatch, uno::UNO_QUERY );

            OSL_ENSURE( xParentNameAccess.is(),
                        "HierarchyEntry::setData - No name access!" );

            if ( xBatch.is() && xParentNameAccess.is() )
            {
                // Try to create own key. It must not exist!

                bool bExists = true;
                uno::Any aMyKey;

                try
                {
                    uno::Reference< container::XNameAccess > xNameAccess;

                    if ( bRoot )
                    {
                        xNameAccess = xParentNameAccess;
                    }
                    else
                    {
                        xParentNameAccess->getByName("Children") >>= xNameAccess;
                    }

                    if ( xNameAccess->hasByName( m_aName ) )
                        aMyKey = xNameAccess->getByName( m_aName );
                    else
                        bExists = false;
                }
                catch ( container::NoSuchElementException const & )
                {
                    bExists = false;
                }

                uno::Reference< container::XNameReplace >   xNameReplace;
                uno::Reference< container::XNameContainer > xContainer;

                if ( bExists )
                {
                    // Key exists. Replace values.

                    aMyKey >>= xNameReplace;

                    OSL_ENSURE( xNameReplace.is(),
                                "HierarchyEntry::setData - No name replace!" );
                }
                else
                {
                    // Key does not exist. Create / fill / insert it.

                    uno::Reference< lang::XSingleServiceFactory > xFac;

                    if ( bRoot )
                    {
                        // Special handling for children of root,
                        // which is not an entry. It's only a set
                        // of entries.
                        xFac.set( xParentNameAccess, uno::UNO_QUERY );
                    }
                    else
                    {
                        // Append new entry to parents child list,
                        // which is a set of entries.
                        xParentNameAccess->getByName("Children") >>= xFac;
                    }

                    OSL_ENSURE( xFac.is(),
                                "HierarchyEntry::setData - No factory!" );

                    if ( xFac.is() )
                    {
                        xNameReplace.set( xFac->createInstance(), uno::UNO_QUERY );

                        OSL_ENSURE( xNameReplace.is(),
                                "HierarchyEntry::setData - No name replace!" );

                        if ( xNameReplace.is() )
                        {
                            xContainer.set( xFac, uno::UNO_QUERY );

                            OSL_ENSURE( xContainer.is(),
                                "HierarchyEntry::setData - No container!" );
                        }
                    }
                }

                if ( xNameReplace.is() )
                {
                    // Set Title value.
                    xNameReplace->replaceByName(
                        "Title",
                        uno::makeAny( rData.getTitle() ) );

                    // Set TargetURL value.

                    // TargetURL property may contain a reference to the Office
                    // installation directory. To ensure a reloctable office
                    // installation, the path to the office installtion
                    // directory must never be stored directly. Use a
                    // placeholder instead.
                    OUString aValue( rData.getTargetURL() );
                    if ( m_xOfficeInstDirs.is() &&  !aValue.isEmpty() )
                        aValue
                            = m_xOfficeInstDirs->makeRelocatableURL( aValue );

                    xNameReplace->replaceByName(
                        "TargetURL",
                        uno::makeAny( aValue ) );

                    // Set Type value.
                    sal_Int32 nType
                        = rData.getType() == HierarchyEntryData::LINK ? 0 : 1;
                    xNameReplace->replaceByName(
                        "Type",
                        uno::makeAny( nType ) );

                    if ( xContainer.is() )
                        xContainer->insertByName(
                            m_aName, uno::makeAny( xNameReplace ) );

                    // Commit changes.
                    xBatch->commitChanges();
                    return true;
                }
            }
        }
    }
    catch ( lang::IllegalArgumentException const & )
    {
        // replaceByName, insertByName

        OSL_FAIL(
            "HierarchyEntry::setData - caught IllegalArgumentException!" );
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( container::NoSuchElementException const & )
    {
        // replaceByName, getByName

        OSL_FAIL(
            "HierarchyEntry::setData - caught NoSuchElementException!" );
    }
    catch ( container::ElementExistException const & )
    {
        // insertByName

        OSL_FAIL(
            "HierarchyEntry::setData - caught ElementExistException!" );
    }
    catch ( lang::WrappedTargetException const & )
    {
        // replaceByName, insertByName, getByName, commitChanges

        OSL_FAIL(
            "HierarchyEntry::setData - caught WrappedTargetException!" );
    }
    catch ( uno::Exception const & )
    {
        // createInstance, createInstanceWithArguments

        OSL_FAIL(
            "HierarchyEntry::setData - caught Exception!" );
    }

    return false;
}


bool HierarchyEntry::move(
    const OUString& rNewURL, const HierarchyEntryData& rData )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    OUString aNewPath = createPathFromHierarchyURL( HierarchyUri(rNewURL) );

    if ( aNewPath == m_aPath )
        return true;

    bool bOldRoot = true;
    uno::Reference< util::XChangesBatch > xOldParentBatch;

    OUString aNewKey;
    sal_Int32 nURLPos = rNewURL.lastIndexOf( '/' );
    if ( nURLPos > HIERARCHY_URL_SCHEME_LENGTH )
        aNewKey = rNewURL.copy( nURLPos + 1 );
    else
    {
        OSL_FAIL( "HierarchyEntry::move - Invalid URL!" );
        return false;
    }

    bool bNewRoot = true;
    uno::Reference< util::XChangesBatch > xNewParentBatch;

    bool bDifferentParents = true;

    try
    {
        if ( !m_xConfigProvider.is() )
            m_xConfigProvider.set(
                m_xContext->getServiceManager()->createInstanceWithContext(m_aServiceSpecifier, m_xContext),
                uno::UNO_QUERY );

        if ( !m_xConfigProvider.is() )
            return false;

        OUString aOldParentPath;
        sal_Int32 nPos = m_aPath.lastIndexOf( '/' );
        if ( nPos != -1 )
        {
            // Skip "/Children" segment of the path, too.
            nPos = m_aPath.lastIndexOf( '/', nPos - 1 );

            OSL_ENSURE( nPos != -1, "HierarchyEntry::move - Wrong path!" );

            aOldParentPath += m_aPath.copy( 0, nPos );
            bOldRoot = false;
        }

        OUString aNewParentPath;
        nPos = aNewPath.lastIndexOf( '/' );
        if ( nPos != -1 )
        {
            // Skip "/Children" segment of the path, too.
            nPos = aNewPath.lastIndexOf( '/', nPos - 1 );

            OSL_ENSURE( nPos != -1, "HierarchyEntry::move - Wrong path!" );

            aNewParentPath += aNewPath.copy( 0, nPos );
            bNewRoot = false;
        }

        uno::Sequence< uno::Any > aArguments( 1 );
        beans::PropertyValue      aProperty;

        aProperty.Name  = CFGPROPERTY_NODEPATH;
        aProperty.Value <<= aOldParentPath;
        aArguments[ 0 ] <<= aProperty;

        xOldParentBatch.set(
            m_xConfigProvider->createInstanceWithArguments(
                READWRITE_SERVICE_NAME,
                aArguments ),
            uno::UNO_QUERY );

        OSL_ENSURE( xOldParentBatch.is(), "HierarchyEntry::move - No batch!" );

        if ( !xOldParentBatch.is() )
            return false;

        if ( aOldParentPath == aNewParentPath )
        {
            bDifferentParents = false;
            xNewParentBatch = xOldParentBatch;
        }
        else
        {
            bDifferentParents = true;

            aProperty.Name    = CFGPROPERTY_NODEPATH;
            aProperty.Value <<= aNewParentPath;
            aArguments[ 0 ] <<= aProperty;

            xNewParentBatch.set(
                m_xConfigProvider->createInstanceWithArguments(
                    READWRITE_SERVICE_NAME,
                    aArguments ),
                uno::UNO_QUERY );

            OSL_ENSURE(
                xNewParentBatch.is(), "HierarchyEntry::move - No batch!" );

            if ( !xNewParentBatch.is() )
                return false;
        }
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( uno::Exception const & )
    {
        // createInstance, createInstanceWithArguments

        OSL_FAIL( "HierarchyEntry::move - caught Exception!" );
        return false;
    }


    // (1) Get entry...


    uno::Any aEntry;
    uno::Reference< container::XNameAccess >    xOldParentNameAccess;
    uno::Reference< container::XNameContainer > xOldNameContainer;

    try
    {
        xOldParentNameAccess.set( xOldParentBatch, uno::UNO_QUERY );

        OSL_ENSURE( xOldParentNameAccess.is(),
                    "HierarchyEntry::move - No name access!" );

        if ( !xOldParentNameAccess.is() )
            return false;

        if ( bOldRoot )
        {
            xOldNameContainer.set( xOldParentNameAccess, uno::UNO_QUERY );
        }
        else
        {
            xOldParentNameAccess->getByName("Children") >>= xOldNameContainer;
        }

        aEntry = xOldNameContainer->getByName( m_aName );
    }
    catch ( container::NoSuchElementException const & )
    {
        // getByName

        OSL_FAIL( "HierarchyEntry::move - caught NoSuchElementException!" );
        return false;
    }
    catch ( lang::WrappedTargetException const & )
    {
        // getByName

        OSL_FAIL( "HierarchyEntry::move - caught WrappedTargetException!" );
        return false;
    }


    // (2) Remove entry... Note: Insert BEFORE remove does not work!


    try
    {
        xOldNameContainer->removeByName( m_aName );
        xOldParentBatch->commitChanges();
    }
    catch ( container::NoSuchElementException const & )
    {
        // getByName, removeByName

        OSL_FAIL( "HierarchyEntry::move - caught NoSuchElementException!" );
        return false;
    }


    // (3) Insert entry at new parent...


    try
    {
        uno::Reference< container::XNameReplace > xNewNameReplace;
        aEntry >>= xNewNameReplace;

        OSL_ENSURE( xNewNameReplace.is(),
                    "HierarchyEntry::move - No name replace!" );

        if ( !xNewNameReplace.is() )
            return false;

        uno::Reference< container::XNameAccess > xNewParentNameAccess;
        if ( bDifferentParents )
            xNewParentNameAccess.set( xNewParentBatch, uno::UNO_QUERY );
        else
            xNewParentNameAccess = xOldParentNameAccess;

        OSL_ENSURE( xNewParentNameAccess.is(),
                    "HierarchyEntry::move - No name access!" );

        if ( !xNewParentNameAccess.is() )
            return false;

        uno::Reference< container::XNameContainer > xNewNameContainer;
        if ( bDifferentParents )
        {
            if ( bNewRoot )
            {
                xNewNameContainer.set( xNewParentNameAccess, uno::UNO_QUERY );
            }
            else
            {
                xNewParentNameAccess->getByName("Children") >>= xNewNameContainer;
            }
        }
        else
            xNewNameContainer = xOldNameContainer;

        if ( !xNewNameContainer.is() )
            return false;

        xNewNameReplace->replaceByName(
            "Title",
            uno::makeAny( rData.getTitle() ) );

        // TargetURL property may contain a reference to the Office
        // installation directory. To ensure a reloctable office
        // installation, the path to the office installtion
        // directory must never be stored directly. Use a placeholder
        // instead.
        OUString aValue( rData.getTargetURL() );
        if ( m_xOfficeInstDirs.is() &&  !aValue.isEmpty() )
            aValue = m_xOfficeInstDirs->makeRelocatableURL( aValue );
        xNewNameReplace->replaceByName(
            "TargetURL",
            uno::makeAny( aValue ) );
        sal_Int32 nType = rData.getType() == HierarchyEntryData::LINK ? 0 : 1;
        xNewNameReplace->replaceByName(
            "Type",
            uno::makeAny( nType ) );

        xNewNameContainer->insertByName( aNewKey, aEntry );
        xNewParentBatch->commitChanges();
    }
    catch ( container::NoSuchElementException const & )
    {
        // replaceByName, insertByName, getByName

        OSL_FAIL( "HierarchyEntry::move - caught NoSuchElementException!" );
        return false;
    }
    catch ( lang::IllegalArgumentException const & )
    {
        // replaceByName, insertByName

        OSL_FAIL(
            "HierarchyEntry::move - caught IllegalArgumentException!" );
        return false;
    }
    catch ( container::ElementExistException const & )
    {
        // insertByName

        OSL_FAIL( "HierarchyEntry::move - caught ElementExistException!" );
        return false;
    }
    catch ( lang::WrappedTargetException const & )
    {
        // replaceByName, insertByName, getByName

        OSL_FAIL( "HierarchyEntry::move - caught WrappedTargetException!" );
        return false;
    }

    return true;
}


bool HierarchyEntry::remove()
{
    try
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( !m_xConfigProvider.is() )
            m_xConfigProvider.set(
                m_xContext->getServiceManager()->createInstanceWithContext(m_aServiceSpecifier, m_xContext),
                uno::UNO_QUERY );

        if ( m_xConfigProvider.is() )
        {
            // Create parent's key. It must exist!

            OUString aParentPath;
            bool bRoot = true;

            sal_Int32 nPos = m_aPath.lastIndexOf( '/' );
            if ( nPos != -1 )
            {
                // Skip "/Children" segment of the path, too.
                nPos = m_aPath.lastIndexOf( '/', nPos - 1 );

                OSL_ENSURE( nPos != -1,
                            "HierarchyEntry::remove - Wrong path!" );

                aParentPath += m_aPath.copy( 0, nPos );
                bRoot = false;
            }

            uno::Sequence< uno::Any > aArguments( 1 );
            beans::PropertyValue      aProperty;

            aProperty.Name    = CFGPROPERTY_NODEPATH;
            aProperty.Value <<= aParentPath;
            aArguments[ 0 ] <<= aProperty;

            uno::Reference< util::XChangesBatch > xBatch(
                m_xConfigProvider->createInstanceWithArguments(
                    READWRITE_SERVICE_NAME,
                    aArguments ),
                uno::UNO_QUERY );

            OSL_ENSURE( xBatch.is(),
                        "HierarchyEntry::remove - No batch!" );

            uno::Reference< container::XNameAccess > xParentNameAccess(
                xBatch, uno::UNO_QUERY );

            OSL_ENSURE( xParentNameAccess.is(),
                        "HierarchyEntry::remove - No name access!" );

            if ( xBatch.is() && xParentNameAccess.is() )
            {
                uno::Reference< container::XNameContainer > xContainer;

                if ( bRoot )
                {
                    // Special handling for children of root,
                    // which is not an entry. It's only a set
                    // of entries.
                    xContainer.set( xParentNameAccess, uno::UNO_QUERY );
                }
                else
                {
                    // Append new entry to parents child list,
                    // which is a set of entries.
                     xParentNameAccess->getByName("Children") >>= xContainer;
                }

                OSL_ENSURE( xContainer.is(),
                            "HierarchyEntry::remove - No container!" );

                if ( xContainer.is() )
                {
                    xContainer->removeByName( m_aName );
                    xBatch->commitChanges();
                    return true;
                }
            }
        }
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( container::NoSuchElementException const & )
    {
        // getByName, removeByName

        OSL_FAIL(
            "HierarchyEntry::remove - caught NoSuchElementException!" );
    }
    catch ( lang::WrappedTargetException const & )
    {
        // getByName, commitChanges

        OSL_FAIL(
            "HierarchyEntry::remove - caught WrappedTargetException!" );
    }
    catch ( uno::Exception const & )
    {
        // createInstance, createInstanceWithArguments

        OSL_FAIL( "HierarchyEntry::remove - caught Exception!" );
    }

    return false;
}


bool HierarchyEntry::first( iterator& it )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( it.m_pImpl->pos == -1 )
    {
        // Init...

        try
        {
            uno::Reference< container::XHierarchicalNameAccess >
                xRootHierNameAccess = getRootReadAccess();

            if ( xRootHierNameAccess.is() )
            {
                uno::Reference< container::XNameAccess > xNameAccess;

                if ( !m_aPath.isEmpty() )
                {
                    OUString aPath = m_aPath;
                    aPath += "/Children";

                    xRootHierNameAccess->getByHierarchicalName( aPath )
                        >>= xNameAccess;
                }
                else
                    xNameAccess.set( xRootHierNameAccess, uno::UNO_QUERY );

                OSL_ENSURE( xNameAccess.is(),
                            "HierarchyEntry::first - No name access!" );

                if ( xNameAccess.is() )
                    it.m_pImpl->names = xNameAccess->getElementNames();

                uno::Reference< container::XHierarchicalNameAccess >
                    xHierNameAccess( xNameAccess, uno::UNO_QUERY );

                OSL_ENSURE( xHierNameAccess.is(),
                            "HierarchyEntry::first - No hier. name access!" );

                it.m_pImpl->dir = xHierNameAccess;

                it.m_pImpl->officeDirs = m_xOfficeInstDirs;
            }
        }
        catch ( uno::RuntimeException const & )
        {
            throw;
        }
        catch ( container::NoSuchElementException const& )
        {
            // getByHierarchicalName

            OSL_FAIL(
                "HierarchyEntry::first - caught NoSuchElementException!" );
        }
        catch ( uno::Exception const & )
        {
            OSL_FAIL( "HierarchyEntry::first - caught Exception!" );
        }
    }

    if ( it.m_pImpl->names.getLength() == 0 )
        return false;

    it.m_pImpl->pos = 0;
    return true;
}


bool HierarchyEntry::next( iterator& it )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( it.m_pImpl->pos == -1 )
        return first( it );

    ++(it.m_pImpl->pos);

    return ( it.m_pImpl->pos < it.m_pImpl->names.getLength() );
}


OUString HierarchyEntry::createPathFromHierarchyURL(
    const HierarchyUri& rURI )
{
    // Transform path....
    // folder/subfolder/subsubfolder
    //      --> ['folder']/Children/['subfolder']/Children/['subsubfolder']

    const OUString aPath = rURI.getPath().copy( 1 ); // skip leading slash.
    sal_Int32 nLen = aPath.getLength();

    if ( nLen )
    {
        OUStringBuffer aNewPath;
        aNewPath.append( "['" );

        sal_Int32 nStart = 0;
        sal_Int32 nEnd   = aPath.indexOf( '/' );

        do
        {
            if ( nEnd == -1 )
                nEnd = nLen;

            OUString aToken = aPath.copy( nStart, nEnd - nStart );
            makeXMLName( aToken, aNewPath );

            if ( nEnd != nLen )
            {
                aNewPath.append( "']/Children/['" );
                nStart = nEnd + 1;
                nEnd   = aPath.indexOf( '/', nStart );
            }
            else
                aNewPath.append( "']" );
        }
        while ( nEnd != nLen );

        return aNewPath.makeStringAndClear();
    }

    return aPath;
}


uno::Reference< container::XHierarchicalNameAccess >
HierarchyEntry::getRootReadAccess()
{
    if ( !m_xRootReadAccess.is() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if ( !m_xRootReadAccess.is() )
        {
            if ( m_bTriedToGetRootReadAccess )
            {
                OSL_FAIL( "HierarchyEntry::getRootReadAccess - "
                            "Unable to read any config data! -> #82494#" );
                return uno::Reference< container::XHierarchicalNameAccess >();
            }

            try
            {
                if ( !m_xConfigProvider.is() )
                    m_xConfigProvider.set(
                            m_xContext->getServiceManager()->createInstanceWithContext(m_aServiceSpecifier, m_xContext),
                            uno::UNO_QUERY );

                if ( m_xConfigProvider.is() )
                {
                    // Create Root object.

                    uno::Sequence< uno::Any > aArguments( 1 );
                    beans::PropertyValue      aProperty;
                    aProperty.Name = CFGPROPERTY_NODEPATH;
                    aProperty.Value <<= OUString(); // root path
                    aArguments[ 0 ] <<= aProperty;

                    m_bTriedToGetRootReadAccess = true;

                    m_xRootReadAccess.set(
                            m_xConfigProvider->createInstanceWithArguments(
                                READ_SERVICE_NAME,
                                aArguments ),
                            uno::UNO_QUERY );
                }
            }
            catch ( uno::RuntimeException const & )
            {
                throw;
            }
            catch ( uno::Exception const & )
            {
                // createInstance, createInstanceWithArguments

                OSL_FAIL( "HierarchyEntry::getRootReadAccess - "
                            "caught Exception!" );
            }
        }
    }
    return m_xRootReadAccess;
}


// HierarchyEntry::iterator Implementation.


HierarchyEntry::iterator::iterator()
    : m_pImpl( new iterator_Impl )
{
}


HierarchyEntry::iterator::~iterator()
{
}


const HierarchyEntryData& HierarchyEntry::iterator::operator*() const
{
    if ( ( m_pImpl->pos != -1 )
         && ( m_pImpl->dir.is() )
         && ( m_pImpl->pos < m_pImpl->names.getLength() ) )
    {
        try
        {
            OUStringBuffer aKey;
            aKey.append( "['" );
            makeXMLName( m_pImpl->names.getConstArray()[ m_pImpl->pos ], aKey );
            aKey.append( "']" );

            OUString aTitle     = aKey.makeStringAndClear();
            OUString aTargetURL = aTitle;
            OUString aType      = aTitle;

            aTitle     += "/Title";
            aTargetURL += "/TargetURL";
            aType      += "/Type";

            OUString aValue;
            m_pImpl->dir->getByHierarchicalName( aTitle ) >>= aValue;
            m_pImpl->entry.setTitle( aValue );

            m_pImpl->dir->getByHierarchicalName( aTargetURL ) >>= aValue;

            // TargetURL property may contain a reference to the Office
            // installation directory. To ensure a reloctable office
            // installation, the path to the office installtion directory must
            // never be stored directly. A placeholder is used instead. Replace
            // it by actual installation directory.
            if ( m_pImpl->officeDirs.is() && !aValue.isEmpty() )
                aValue = m_pImpl->officeDirs->makeAbsoluteURL( aValue );
            m_pImpl->entry.setTargetURL( aValue );

            if ( m_pImpl->dir->hasByHierarchicalName( aType ) )
            {
                // Might not be present since it was introduced long
                // after Title and TargetURL (#82433#)... So not getting
                // it is not an error.

                // Get Type value.
                sal_Int32 nType = 0;
                if ( m_pImpl->dir->getByHierarchicalName( aType ) >>= nType )
                {
                    if ( nType == 0 )
                    {
                        m_pImpl->entry.setType( HierarchyEntryData::LINK );
                    }
                    else if ( nType == 1 )
                    {
                        m_pImpl->entry.setType( HierarchyEntryData::FOLDER );
                    }
                    else
                    {
                        OSL_FAIL( "HierarchyEntry::getData - "
                                    "Unknown Type value!" );
                    }
                }
            }

            m_pImpl->entry.setName(
                m_pImpl->names.getConstArray()[ m_pImpl->pos ] );
        }
        catch ( container::NoSuchElementException const & )
        {
            m_pImpl->entry = HierarchyEntryData();
        }
    }

    return m_pImpl->entry;
}

} // namespace hierarchy_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
