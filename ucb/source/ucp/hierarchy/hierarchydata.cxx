/*************************************************************************
 *
 *  $RCSfile: hierarchydata.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:54:18 $
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

/**************************************************************************
                                TODO
 **************************************************************************

 - HierarchyEntry::move
   --> Umstellen auf Benutzung von XNamed ( wenn es von config db api
       unterstuetzt wird ).

 *************************************************************************/

#ifndef _HIERARCHYDATA_HXX
#include "hierarchydata.hxx"
#endif

#ifndef __VECTOR__
#include <stl/vector>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGESCAPE_HPP_
#include <com/sun/star/util/XStringEscape.hpp>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif
#ifndef _HIERARCHYPROVIDER_HXX
#include "hierarchyprovider.hxx"
#endif

using namespace com::sun::star::container;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace rtl;
using namespace hierarchy_ucp;

namespace hierarchy_ucp
{

struct HierarchyEntry::iterator_Impl
{
    HierarchyEntryData                   entry;
    Reference< XHierarchicalNameAccess > dir;
    Sequence< OUString>                  names;
    sal_Int32                            pos;

    iterator_Impl() : pos( -1 /* before first */ ) {};
};

} // hierarchy_ucp

//=========================================================================
//=========================================================================
//
// HierarchyEntry Implementation.
//
//=========================================================================
//=========================================================================

#if SUPD>604

#define HIERARCHY_ROOT_DB_KEY           "/org.openoffice.ucb.Hierarchy/Root"
#define HIERARCHY_ROOT_DB_KEY_LENGTH    34

#else

#define HIERARCHY_ROOT_DB_KEY           "/com.sun.star.ucb.Hierarchy/Root"
#define HIERARCHY_ROOT_DB_KEY_LENGTH    32

#endif

//=========================================================================
HierarchyEntry::HierarchyEntry( const Reference< XMultiServiceFactory >& rSMgr,
                                const OUString& rURL )
: m_xSMgr( rSMgr )
{
    // Note: do not init m_aPath init list. createPathFromHierarchyURL
    //       needs m_xSMgr and m_aMutex.
    m_aPath = createPathFromHierarchyURL( rURL );
}

//=========================================================================
sal_Bool HierarchyEntry::hasData()
{
    try
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( !m_xConfigProvider.is() )
            m_xConfigProvider = Reference< XMultiServiceFactory >(
                m_xSMgr->createInstance(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationProvider" ) ),
                UNO_QUERY );

        if ( m_xConfigProvider.is() )
        {
            // Create Root object.

            Sequence< Any > aArguments( 1 );
            aArguments[ 0 ]
                    <<= OUString::createFromAscii( HIERARCHY_ROOT_DB_KEY );

            Reference< XHierarchicalNameAccess > xRootHierAccess(
                m_xConfigProvider->createInstanceWithArguments(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationAccess" ),
                    aArguments ),
                UNO_QUERY );

            VOS_ENSURE( xRootHierAccess.is(),
                        "HierarchyEntry::hasData - No root!" );

            if ( xRootHierAccess.is() )
                return xRootHierAccess->hasByHierarchicalName( m_aPath );
        }
    }
    catch ( RuntimeException& )
    {
        throw;
    }
    catch ( Exception& )
    {
        // createInstance, createInstanceWithArguments

        VOS_ENSURE( sal_False,
                    "HierarchyEntry::hasData - caught Exception!" );
    }

    return sal_False;
}

//=========================================================================
sal_Bool HierarchyEntry::getData( Any& rTitle,
                                    Any& rTargetURL,
                                  sal_Bool bChildren,
                                  Any& rChildren )
{
    try
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( !m_xConfigProvider.is() )
            m_xConfigProvider = Reference< XMultiServiceFactory >(
                m_xSMgr->createInstance(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationProvider" ) ),
                UNO_QUERY );

        if ( m_xConfigProvider.is() )
        {
            // Create Root object.

            Sequence< Any > aArguments( 1 );
            aArguments[ 0 ]
                    <<= OUString::createFromAscii( HIERARCHY_ROOT_DB_KEY );

            Reference< XHierarchicalNameAccess > xRootHierAccess(
                m_xConfigProvider->createInstanceWithArguments(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationAccess" ),
                    aArguments ),
                UNO_QUERY );

            VOS_ENSURE( xRootHierAccess.is(),
                        "HierarchyEntry::getData - No root!" );

            if ( xRootHierAccess.is() )
            {
                OUString aTitlePath     = m_aPath;
                OUString aTargetURLPath = m_aPath;
                OUString aChildrenPath  = m_aPath;

                aTitlePath     += OUString::createFromAscii( "/Title" );
                aTargetURLPath += OUString::createFromAscii( "/TargetURL" );
                aChildrenPath  += OUString::createFromAscii( "/Children" );

                // Get Title value.
                rTitle
                    = xRootHierAccess->getByHierarchicalName( aTitlePath );

                // Get TargetURL value.
                rTargetURL
                    = xRootHierAccess->getByHierarchicalName( aTargetURLPath );

                // Get Children value.
                if ( bChildren )
                    rChildren
                        = xRootHierAccess->getByHierarchicalName(
                                                            aChildrenPath );
                return sal_True;
            }
        }
    }
    catch ( RuntimeException& )
    {
        throw;
    }
    catch ( NoSuchElementException& )
    {
        // getByHierarchicalName

        VOS_ENSURE( sal_False,
                    "HierarchyEntry::getData - caught NoSuchElementException!" );
    }
    catch ( Exception& )
    {
        // createInstance, createInstanceWithArguments

        VOS_ENSURE( sal_False,
                    "HierarchyEntry::getData - caught Exception!" );
    }
    return sal_False;
}

//=========================================================================
sal_Bool HierarchyEntry::getData( HierarchyEntryData& rData )
{
    Any aTitle;
    Any aTargetURL;
    if ( getData( aTitle, aTargetURL, sal_False, Any() ) )
    {
        if ( ( aTitle >>= rData.aTitle )
             && ( aTargetURL >>= rData.aTargetURL ) )
            return sal_True;
    }
    return sal_False;
}

//=========================================================================
sal_Bool HierarchyEntry::setData( const OUString& rOldPath,
                                  const OUString& rPath,
                                  const Any& rTitle,
                                  const Any& rTargetURL,
                                  sal_Bool bCreate,
                                   sal_Bool bFailIfExists,
                                   sal_Bool bChildren,
                                  const Any& rChildren,
                                  const Reference< XChangesBatch >& rxBatch,
                                  const OUString& rBatchPath )
{
    try
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( !m_xConfigProvider.is() )
            m_xConfigProvider = Reference< XMultiServiceFactory >(
                m_xSMgr->createInstance(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationProvider" ) ),
                UNO_QUERY );

        if ( m_xConfigProvider.is() )
        {
            // Create parent's key. It must exist!

            OUString aParentPath
                        = OUString::createFromAscii( HIERARCHY_ROOT_DB_KEY );
            OUString aKey  = rPath;
            sal_Bool bRoot = sal_True;

            sal_Int32 nPos = rPath.lastIndexOf( '/' );
            if ( nPos != -1 )
            {
                aKey = rPath.copy( nPos + 1 );

                // Skip "/Children" segment of the path, too.
                nPos = rPath.lastIndexOf( '/', nPos - 1 );

                VOS_ENSURE( nPos != -1,
                            "HierarchyEntry::setData - Wrong path!" );

                aParentPath += OUString::createFromAscii( "/" );
                aParentPath += rPath.copy( 0, nPos );
                bRoot = sal_False;
            }

            OUString aBatchPath
                = ( rBatchPath.getLength() > 0 ) ? rBatchPath : aParentPath;

            Reference< XChangesBatch > xBatch = rxBatch;
            if ( !xBatch.is() )
            {
                Sequence< Any > aArguments( 1 );
                aArguments[ 0 ] <<= aParentPath;

                xBatch = Reference< XChangesBatch >(
                    m_xConfigProvider->createInstanceWithArguments(
                        OUString::createFromAscii(
                            "com.sun.star.configuration.ConfigurationUpdateAccess" ),
                        aArguments ),
                    UNO_QUERY );
            }

            VOS_ENSURE( xBatch.is(),
                        "HierarchyEntry::setData - No batch!" );

            Reference< XNameAccess > xParentNameAccess( xBatch, UNO_QUERY );

            if ( rxBatch.is() )
            {
                Reference< XHierarchicalNameAccess > xTmp(
                                            xParentNameAccess, UNO_QUERY );

                VOS_ENSURE( xTmp.is(),
                            "HierarchyEntry::setData - No hier name access!" );

                if ( xTmp.is() )
                {
                    OUString aName = rPath.copy(
                                        aBatchPath.getLength()
                                              - HIERARCHY_ROOT_DB_KEY_LENGTH );
                    sal_Int32 nPos1 = aName.lastIndexOf( '/' );
                    if ( nPos1 != -1 )
                    {
                        // Skip "/Children" segment of the path, too.
                        nPos1 = aName.lastIndexOf( '/', nPos1 - 1 );

                        VOS_ENSURE( nPos1 != -1,
                                    "HierarchyEntry::setData - Wrong path!" );

                        aName = aName.replaceAt( nPos1,
                                                 aName.getLength() - nPos1,
                                                 OUString() );
                    }

                    xTmp->getByHierarchicalName( aName ) >>= xParentNameAccess;
                }
            }

            VOS_ENSURE( xParentNameAccess.is(),
                        "HierarchyEntry::setData - No name access!" );

            if ( xBatch.is() && xParentNameAccess.is() )
            {
                // Try to create own key. It must not exist!

                sal_Bool bExists = sal_True;
                Any aMyKey;

                try
                {
                    Reference< XNameAccess > xNameAccess;

                    if ( bRoot )
                    {
                        xNameAccess = xParentNameAccess;
                    }
                    else
                    {
                        xParentNameAccess->getByName(
                            OUString::createFromAscii( "Children" ) )
                                >>= xNameAccess;
                    }

                    if ( xNameAccess->hasByName( aKey ) )
                        aMyKey = xNameAccess->getByName( aKey );
                    else
                        bExists = sal_False;
                }
                catch ( NoSuchElementException& )
                {
                    bExists = sal_False;
                }

                Reference< XNameReplace >   xNameReplace;
                Reference< XNameContainer > xContainer;

                if ( bExists )
                {
                    if ( bFailIfExists )
                        return sal_False;

                    // Key exists. Replace values.

                    aMyKey >>= xNameReplace;

                    VOS_ENSURE( xNameReplace.is(),
                                "HierarchyEntry::setData - No name replace!" );
                }
                else
                {
                    if ( !bCreate )
                        return sal_True;

                    // Key does not exist. Create / fill / insert it.

                    Reference< XSingleServiceFactory > xFac;

                    if ( bRoot )
                    {
                        // Special handling for children of root,
                        // which is not an entry. It's only a set
                        // of entries.
                        xFac = Reference< XSingleServiceFactory >(
                                            xParentNameAccess, UNO_QUERY );
                    }
                    else
                    {
                        // Append new entry to parents child list,
                        // which is a set of entries.
                        xParentNameAccess->getByName(
                                        OUString::createFromAscii(
                                            "Children" ) ) >>= xFac;
                    }

                    VOS_ENSURE( xFac.is(),
                                "HierarchyEntry::setData - No factory!" );

                    if ( xFac.is() )
                    {
                        xNameReplace = Reference< XNameReplace >(
                                        xFac->createInstance(), UNO_QUERY );

                        VOS_ENSURE( xNameReplace.is(),
                                "HierarchyEntry::setData - No name replace!" );

                        if ( xNameReplace.is() )
                        {
                            xContainer = Reference< XNameContainer >(
                                                        xFac, UNO_QUERY );

                            VOS_ENSURE( xContainer.is(),
                                "HierarchyEntry::setData - No container!" );
                        }
                    }
                }

                if ( xNameReplace.is() )
                {
                    // Set Title value.
                    xNameReplace->replaceByName(
                                OUString::createFromAscii( "Title" ),
                                rTitle );

                    // Set TargetURL value.
                    xNameReplace->replaceByName(
                                OUString::createFromAscii( "TargetURL" ),
                                rTargetURL );

                    if ( xContainer.is() )
                        xContainer->insertByName(
                                            aKey, makeAny( xNameReplace ) );

        // SEE BELOW! At the moment, each single change must be committed.

                    // Commit changes.
                    xBatch->commitChanges();

#if 0
    // Doesn't work. According to FS possibly never will ...

                    // Set Children, if given.
                    if ( bChildren )
                        xNameReplace->replaceByName(
                                OUString::createFromAscii( "Children" ),
                                rChildren );
#else
    // Copy children manually :-(

                    if ( bChildren && ( rOldPath.getLength() > 0  ) )
                    {
                        Reference< XNameAccess > xChildrenNameAccess;
                        rChildren >>= xChildrenNameAccess;

                        if ( xChildrenNameAccess.is() )
                        {
                            Sequence< Any > aArguments( 1 );
                            aArguments[ 0 ]
                                <<= OUString::createFromAscii(
                                                HIERARCHY_ROOT_DB_KEY );

                            Reference< XHierarchicalNameAccess >
                                xRootHierAccess(
                                    m_xConfigProvider->createInstanceWithArguments(
                                        OUString::createFromAscii(
                                            "com.sun.star.configuration.ConfigurationAccess" ),
                                        aArguments ),
                                    UNO_QUERY );

                            VOS_ENSURE( xRootHierAccess.is(),
                                        "HierarchyEntry::getData - No root!" );

                            if ( xRootHierAccess.is() )
                            {
                                Sequence< OUString >aElems
                                    = xChildrenNameAccess->getElementNames();
                                const OUString* pElems = aElems.getConstArray();
                                sal_Int32 nCount = aElems.getLength();

                                if ( nCount > 0 )
                                {
                                    OUString aTitleKey =
                                        OUString::createFromAscii(
                                                            "/Title" );
                                    OUString aTargetURLKey =
                                        OUString::createFromAscii(
                                                            "/TargetURL" );
                                    OUString aChildrenKey =
                                        OUString::createFromAscii(
                                                            "/Children" );

                                    OUString aFullChildPath = rPath;
                                    aFullChildPath += aChildrenKey;
                                    aFullChildPath
                                        += OUString::createFromAscii( "/" );

                                    OUString aOldFullChildPath = rOldPath;
                                    aOldFullChildPath += aChildrenKey;
                                    aOldFullChildPath
                                        += OUString::createFromAscii( "/" );

                                    // Iterate over children.
                                    for ( sal_Int32 n = 0; n < nCount; ++n )
                                    {
                                        const OUString& rElem = pElems[ n ];

                                        // Get data from old child...

                                        OUString aOldChildPath
                                                    = aOldFullChildPath;
                                        aOldChildPath += rElem;

                                        OUString aTitle     = aOldChildPath;
                                        OUString aTargetURL = aOldChildPath;
                                        OUString aChildren  = aOldChildPath;

                                        aTitle     += aTitleKey;
                                        aTargetURL += aTargetURLKey;
                                        aChildren  += aChildrenKey;

                                        Any aTitleValue
                                            = xRootHierAccess
                                                ->getByHierarchicalName(
                                                                aTitle );

                                        Any aTargetURLValue
                                            = xRootHierAccess
                                                ->getByHierarchicalName(
                                                                aTargetURL );
                                        Any aChildrenValue
                                            = xRootHierAccess
                                                ->getByHierarchicalName(
                                                                aChildren );

                                        OUString aChildPath = aFullChildPath;
                                        aChildPath += rElem;

                                        aOldChildPath = aOldFullChildPath;
                                        aOldChildPath += rElem;

                                        // Recurse...
                                        if ( !setData( aOldChildPath,
                                                       aChildPath,
                                                       aTitleValue,
                                                       aTargetURLValue,
                                                       sal_True, // create
                                                       sal_True, // fail, if exists
                                                       sal_True, // copy children
                                                       aChildrenValue,
                                                       xBatch,   // no commit
                                                       aBatchPath ) )
                                            return sal_False;
                                    }
                                }
                            }
                        }
                    }
#endif

#if 0
    // Transaction with more then one change do not work at the moment!!!

                    if ( !rxBatch.is() )
                    {
                        // Commit changes.
                        xBatch->commitChanges();
                    }
#endif
                    return sal_True;
                }
            }
        }
    }
    catch ( RuntimeException& )
    {
        throw;
    }
    catch ( IllegalArgumentException& )
    {
        // escapeString, replaceByName, insertByName

        VOS_ENSURE( sal_False,
                "HierarchyEntry::setData - caught IllegalArgumentException!" );
    }
    catch ( NoSuchElementException& )
    {
        // replaceByName, getByName, getByHierarchicalName

        VOS_ENSURE( sal_False,
                "HierarchyEntry::setData - caught NoSuchElementException!" );
    }
    catch ( ElementExistException& )
    {
        // insertByName

        VOS_ENSURE( sal_False,
                    "HierarchyEntry::setData - caught ElementExistException!" );
    }
    catch ( WrappedTargetException& )
    {
        // replaceByName, insertByName, getByName, commitChanges

        VOS_ENSURE( sal_False,
                "HierarchyEntry::setData - caught WrappedTargetException!" );
    }
    catch ( Exception& )
    {
        // createInstance, createInstanceWithArguments

        VOS_ENSURE( sal_False,
                    "HierarchyEntry::setData - caught Exception!" );
    }

    return sal_False;
}

//=========================================================================
sal_Bool HierarchyEntry::setData(
                    const HierarchyEntryData& rData, sal_Bool bCreate )
{
    return setData( OUString(), // no old path
                    m_aPath,
                    makeAny( rData.aTitle ),
                    makeAny( rData.aTargetURL ),
                    bCreate,
                    sal_False,  // don't fail if exists
                    sal_False,  // no children data given
                    Any(),
                    Reference< XChangesBatch >(), // commit data
                    OUString() );
}

//=========================================================================
sal_Bool HierarchyEntry::move( const OUString& rNewURL )
{
    OUString aNewPath = createPathFromHierarchyURL( rNewURL );

    if ( aNewPath == m_aPath )
        return sal_True;

    if ( aNewPath.compareTo( m_aPath, aNewPath.lastIndexOf( '/' ) + 1 ) != 0 )
    {
        VOS_ENSURE( sal_False,
                    "HierarchyEntry::move - "
                    "Only move inside one directory allowed!" );
        return sal_False;
    }

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

#if 0
    // In the near future...

    - get update access for m_aPath
    - update access -> XNamed
    - xNamed::setName( newName )
    - updateaccess commit
#else
    //////////////////////////////////////////////////////////////////////
    // (1) Get old entry...
    //////////////////////////////////////////////////////////////////////
    Any aTitle;
    Any aTargetURL;
    Any aChildren;
    if ( getData( aTitle, aTargetURL, sal_True, aChildren ) )
    {
        //////////////////////////////////////////////////////////////////
        // (2) Insert entry at new location. Fail, if another entry with
        //     same key already exists...
        //////////////////////////////////////////////////////////////////
        if ( setData( m_aPath,     // old path
                      aNewPath,    // new path
                      aTitle,
                      aTargetURL,
                      sal_True,    // create, if not exists
                      sal_True,    // fail, if exists
                      sal_True,    // process aChildren
                      aChildren,
                      Reference< XChangesBatch >(),  // commit data
                      OUString() ) )
        {
            //////////////////////////////////////////////////////////////
            // (3) Remove old entry.
            //////////////////////////////////////////////////////////////
            return remove();
        }
    }

    return sal_False;
#endif
}

//=========================================================================
sal_Bool HierarchyEntry::remove()
{
    try
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( !m_xConfigProvider.is() )
            m_xConfigProvider = Reference< XMultiServiceFactory >(
                m_xSMgr->createInstance(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationProvider" ) ),
                UNO_QUERY );

        if ( m_xConfigProvider.is() )
        {
            // Create parent's key. It must exist!

            OUString aParentPath
                        = OUString::createFromAscii( HIERARCHY_ROOT_DB_KEY );
            OUString aKey
                        = m_aPath;
            sal_Bool bRoot = sal_True;

            sal_Int32 nPos = m_aPath.lastIndexOf( '/' );
            if ( nPos != -1 )
            {
                aKey = m_aPath.copy( nPos + 1 );

                // Skip "/Children" segment of the path, too.
                nPos = m_aPath.lastIndexOf( '/', nPos - 1 );

                VOS_ENSURE( nPos != -1,
                            "HierarchyEntry::remove - Wrong path!" );

                aParentPath += OUString::createFromAscii( "/" );
                aParentPath += m_aPath.copy( 0, nPos );
                bRoot = sal_False;
            }

            Sequence< Any > aArguments( 1 );
            aArguments[ 0 ] <<= aParentPath;

            Reference< XChangesBatch > xBatch(
                m_xConfigProvider->createInstanceWithArguments(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationUpdateAccess" ),
                    aArguments ),
                UNO_QUERY );

            VOS_ENSURE( xBatch.is(),
                        "HierarchyEntry::remove - No batch!" );

            Reference< XNameAccess > xParentNameAccess( xBatch, UNO_QUERY );

            VOS_ENSURE( xParentNameAccess.is(),
                        "HierarchyEntry::remove - No name access!" );

            if ( xBatch.is() && xParentNameAccess.is() )
            {
                Reference< XNameContainer > xContainer;

                if ( bRoot )
                {
                    // Special handling for children of root,
                    // which is not an entry. It's only a set
                    // of entries.
                    xContainer = Reference< XNameContainer >(
                                        xParentNameAccess, UNO_QUERY );
                }
                else
                {
                    // Append new entry to parents child list,
                    // which is a set of entries.
                     xParentNameAccess->getByName(
                                 OUString::createFromAscii( "Children" ) )
                        >>= xContainer;
                }

                VOS_ENSURE( xContainer.is(),
                            "HierarchyEntry::remove - No container!" );

                if ( xContainer.is() )
                {
                    xContainer->removeByName( aKey );
                    xBatch->commitChanges();
                    return sal_True;
                }
            }
        }
    }
    catch ( RuntimeException& )
    {
        throw;
    }
    catch ( NoSuchElementException& )
    {
        // getByName, removeByName

        VOS_ENSURE( sal_False,
                "HierarchyEntry::remove - caught NoSuchElementException!" );
    }
    catch ( WrappedTargetException& )
    {
        // getByName, commitChanges

        VOS_ENSURE( sal_False,
                "HierarchyEntry::remove - caught WrappedTargetException!" );
    }
    catch ( Exception& )
    {
        // createInstance, createInstanceWithArguments

        VOS_ENSURE( sal_False,
                    "HierarchyEntry::remove - caught Exception!" );
    }

    return sal_False;
}

//=========================================================================
sal_Bool HierarchyEntry::first( iterator& it )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( it.m_pImpl->pos == -1 )
    {
        // Init...

        try
        {
            if ( !m_xConfigProvider.is() )
                m_xConfigProvider = Reference< XMultiServiceFactory >(
                    m_xSMgr->createInstance(
                        OUString::createFromAscii(
                            "com.sun.star.configuration.ConfigurationProvider" ) ),
                    UNO_QUERY );

            if ( m_xConfigProvider.is() )
            {
                OUString aPath
                        = OUString::createFromAscii( HIERARCHY_ROOT_DB_KEY );

                // No slash in path -> root entry -> special handling needed.
                if ( m_aPath.getLength() > 0 )
                {
                    aPath += OUString::createFromAscii( "/" );
                    aPath += m_aPath;
                    aPath += OUString::createFromAscii( "/Children" );
                }

                Sequence< Any > aArguments( 1 );
                aArguments[ 0 ] <<= aPath;

                Reference< XNameAccess > xNameAccess(
                        m_xConfigProvider->createInstanceWithArguments(
                            OUString::createFromAscii(
                                "com.sun.star.configuration.ConfigurationAccess" ),
                            aArguments ),
                        UNO_QUERY );

                VOS_ENSURE( xNameAccess.is(),
                            "HierarchyEntry::first - No name access!" );

                if ( xNameAccess.is() )
                    it.m_pImpl->names = xNameAccess->getElementNames();

                Reference< XHierarchicalNameAccess > xHierNameAccess(
                                                xNameAccess, UNO_QUERY );

                VOS_ENSURE( xHierNameAccess.is(),
                            "HierarchyEntry::first - No hier. name access!" );

                if ( xHierNameAccess.is() )
                    it.m_pImpl->dir = xHierNameAccess;
            }
        }
        catch ( RuntimeException& )
        {
            throw;
        }
        catch ( Exception& )
        {
            VOS_ENSURE( sal_False,
                        "HierarchyEntry::first - caught Exception!" );
        }
    }

    if ( it.m_pImpl->names.getLength() == 0 )
        return sal_False;

    it.m_pImpl->pos = 0;
    return sal_True;
}

//=========================================================================
sal_Bool HierarchyEntry::next( iterator& it )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( it.m_pImpl->pos == -1 )
        return first( it );

    ++(it.m_pImpl->pos);

    return ( it.m_pImpl->pos < it.m_pImpl->names.getLength() );
}

//=========================================================================
OUString HierarchyEntry::createPathFromHierarchyURL( const OUString& rURL )
{
    try
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        if ( !m_xConfigProvider.is() )
            m_xConfigProvider = Reference< XMultiServiceFactory >(
                m_xSMgr->createInstance(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationProvider" ) ),
                UNO_QUERY );

        if ( m_xConfigProvider.is() )
        {
            // Create Root object.

            Sequence< Any > aArguments( 1 );
            aArguments[ 0 ]
                    <<= OUString::createFromAscii( HIERARCHY_ROOT_DB_KEY );

            m_xEscaper = Reference< XStringEscape >(
                m_xConfigProvider->createInstanceWithArguments(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationUpdateAccess" ),
                    aArguments ),
                UNO_QUERY );
        }
    }
    catch ( RuntimeException& )
    {
        throw;
    }
    catch ( Exception& )
    {
        // createInstance, createInstanceWithArguments

        VOS_ENSURE( sal_False,
                    "HierarchyEntry::createPathFromHierarchyURL - "
                    "caught Exception!" );
    }

    VOS_ENSURE( m_xEscaper.is(),
                "HierarchyEntry::createPathFromHierarchyURL - No escaper!" );

    // Transform path....
    // folder/subfolder/subsubfolder
    //      --> folder/Children/subfolder/Children/subsubfolder

    // Erase URL scheme and ":/" from rURL to get the path.
    OUString aPath = rURL.copy( HIERARCHY_URL_SCHEME_LENGTH + 2 );
    sal_Int32 nLen = aPath.getLength();

    OUString aNewPath;
    if ( nLen )
    {
        const OUString aChildren = OUString::createFromAscii( "/Children/" );
        sal_Int32 nStart = 0;
        sal_Int32 nEnd   = aPath.indexOf( '/' );

        do
        {
            if ( nEnd == -1 )
                nEnd = nLen;

            OUString aToken = aPath.copy( nStart, nEnd - nStart );

            if ( m_xEscaper.is() )
            {
                try
                {
                    aToken = m_xEscaper->escapeString( aToken );
                }
                catch ( IllegalArgumentException& )
                {
                    VOS_ENSURE( sal_False,
                                "HierarchyEntry::createPathFromHierarchyURL - "
                                "caught IllegalArgumentException!" );
                }
            }

            aNewPath += aToken;

            if ( nEnd != nLen )
            {
                aNewPath += aChildren;
                nStart = nEnd + 1;
                nEnd = aPath.indexOf( '/', nStart );
            }
        }
        while ( nEnd != nLen );
    }

    return aNewPath;
}

//=========================================================================
//=========================================================================
//
// HierarchyEntry::iterator Implementation.
//
//=========================================================================
//=========================================================================

HierarchyEntry::iterator::iterator()
{
    m_pImpl = new iterator_Impl;
}

//=========================================================================
HierarchyEntry::iterator::~iterator()
{
    delete m_pImpl;
}

//=========================================================================
const HierarchyEntryData& HierarchyEntry::iterator::operator*() const
{
    if ( ( m_pImpl->pos != -1 )
         && ( m_pImpl->dir.is() )
         && ( m_pImpl->pos < m_pImpl->names.getLength() ) )
    {
        try
        {
            OUString aKey = m_pImpl->names.getConstArray()[ m_pImpl->pos ];
            OUString aTitle     = aKey;
            OUString aTargetURL = aKey;
            aTitle     += OUString::createFromAscii( "/Title" );
            aTargetURL += OUString::createFromAscii( "/TargetURL" );

            m_pImpl->dir->getByHierarchicalName( aTitle )
                >>= m_pImpl->entry.aTitle;
            m_pImpl->dir->getByHierarchicalName( aTargetURL )
                >>= m_pImpl->entry.aTargetURL;

        }
        catch ( NoSuchElementException& )
        {
            m_pImpl->entry = HierarchyEntryData();
        }
    }

    return m_pImpl->entry;
}

