/*************************************************************************
 *
 *  $RCSfile: hierarchydata.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: kso $ $Date: 2001-03-21 08:56:23 $
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

 - HierarchyEntry::move
   --> Umstellen auf Benutzung von XNamed ( wenn es von config db api
       unterstuetzt wird ).

 *************************************************************************/

// Commit every single write operation vs. commit multiple write operations
#define MULTI_COMMIT

#ifndef _HIERARCHYDATA_HXX
#include "hierarchydata.hxx"
#endif

#include <vector>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
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

using namespace com::sun::star::beans;
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
    Reference< XStringEscape >           esc;
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

#define HIERARCHY_ROOT_DB_KEY   "/org.openoffice.ucb.Hierarchy/Root"

// describe path of cfg entry
#define CFGPROPERTY_NODEPATH    "nodepath"
// true->async. update; false->sync. update
#define CFGPROPERTY_LAZYWRITE   "lazywrite"

//=========================================================================
HierarchyEntry::HierarchyEntry(
                const Reference< XMultiServiceFactory >& rSMgr,
                HierarchyContentProvider* pProvider,
                const OUString& rURL )
: m_xSMgr( rSMgr ),
  m_bTriedToGetRootReadAccess( sal_False )
{
    if ( pProvider )
    {
        m_xConfigProvider = pProvider->getConfigProvider();
          m_xRootReadAccess = pProvider->getRootConfigReadNameAccess();
    }

    // Note: do not init m_aPath in init list. createPathFromHierarchyURL
    //       needs m_xSMgr and m_aMutex.
    m_aPath = createPathFromHierarchyURL( rURL );

    // Extract language independent name from URL.
    sal_Int32 nPos = rURL.lastIndexOf( '/' );
    if ( nPos > HIERARCHY_URL_SCHEME_LENGTH )
        m_aName = rURL.copy( nPos + 1 );
    else
        VOS_ENSURE( sal_False, "HierarchyEntry - Invalid URL!" );
}

//=========================================================================
sal_Bool HierarchyEntry::hasData()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    Reference< XHierarchicalNameAccess > xRootReadAccess = getRootReadAccess();

    VOS_ENSURE( xRootReadAccess.is(), "HierarchyEntry::hasData - No root!" );

    if ( xRootReadAccess.is() )
        return xRootReadAccess->hasByHierarchicalName( m_aPath );

    return sal_False;
}

//=========================================================================
sal_Bool HierarchyEntry::getData( HierarchyEntryData& rData )
{
    try
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );

        Reference< XHierarchicalNameAccess > xRootReadAccess
            = getRootReadAccess();

        VOS_ENSURE( xRootReadAccess.is(),
                    "HierarchyEntry::getData - No root!" );

        if ( xRootReadAccess.is() )
        {
            OUString aTitlePath = m_aPath;
            aTitlePath += OUString::createFromAscii( "/Title" );

            // Note: Avoid NoSuchElementExceptions, because exceptions are
            //       relatively 'expensive'. Checking for availability of
            //       title value is sufficient here, because if it is
            //       there, the other values will be available too.
            if ( !xRootReadAccess->hasByHierarchicalName( aTitlePath ) )
                return sal_False;

            // Get Title value.
            if ( !( xRootReadAccess->getByHierarchicalName(
                            aTitlePath ) >>= rData.aTitle ) )
            {
                VOS_ENSURE( sal_False,
                            "HierarchyEntry::getData - "
                            "Got no Title value!" );
                return sal_False;
            }

            // Get TargetURL value.
            OUString aTargetURLPath = m_aPath;
            aTargetURLPath += OUString::createFromAscii( "/TargetURL" );
            if ( !( xRootReadAccess->getByHierarchicalName(
                            aTargetURLPath ) >>= rData.aTargetURL ) )
            {
                VOS_ENSURE( sal_False,
                            "HierarchyEntry::getData - "
                            "Got no TargetURL value!" );
                return sal_False;
            }

            rData.aName = m_aName;
            return sal_True;
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
    return sal_False;
}

//=========================================================================
sal_Bool HierarchyEntry::setData(
                    const HierarchyEntryData& rData, sal_Bool bCreate )
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

            OUString aParentPath(
                RTL_CONSTASCII_USTRINGPARAM( HIERARCHY_ROOT_DB_KEY ) );
            OUString aKey  = m_aPath;
            sal_Bool bRoot = sal_True;

            sal_Int32 nPos = m_aPath.lastIndexOf( '/' );
            if ( nPos != -1 )
            {
                aKey = m_aPath.copy( nPos + 1 );

                // Skip "/Children" segment of the path, too.
                nPos = m_aPath.lastIndexOf( '/', nPos - 1 );

                VOS_ENSURE( nPos != -1,
                            "HierarchyEntry::setData - Wrong path!" );

                aParentPath += OUString::createFromAscii( "/" );
                aParentPath += m_aPath.copy( 0, nPos );
                bRoot = sal_False;
            }

            Sequence< Any > aArguments( 2 );
            PropertyValue   aProperty;

            aProperty.Name    = OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                    CFGPROPERTY_NODEPATH ) );
            aProperty.Value <<= aParentPath;
            aArguments[ 0 ] <<= aProperty;

            aProperty.Name    = OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                    CFGPROPERTY_LAZYWRITE ) );
            aProperty.Value <<= sal_True;
            aArguments[ 1 ] <<= aProperty;

            Reference< XChangesBatch > xBatch(
                    m_xConfigProvider->createInstanceWithArguments(
                        OUString::createFromAscii(
                            "com.sun.star.configuration.ConfigurationUpdateAccess" ),
                        aArguments ),
                    UNO_QUERY );

            VOS_ENSURE( xBatch.is(),
                        "HierarchyEntry::setData - No batch!" );

            Reference< XNameAccess > xParentNameAccess( xBatch, UNO_QUERY );

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
                                makeAny( rData.aTitle ) );

                    // Set TargetURL value.
                    xNameReplace->replaceByName(
                                OUString::createFromAscii( "TargetURL" ),
                                makeAny( rData.aTargetURL ) );

                    if ( xContainer.is() )
                        xContainer->insertByName(
                                            aKey, makeAny( xNameReplace ) );

                    // Commit changes.
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
    catch ( IllegalArgumentException& )
    {
        // replaceByName, insertByName

        VOS_ENSURE( sal_False,
                "HierarchyEntry::setData - caught IllegalArgumentException!" );
    }
    catch ( NoSuchElementException& )
    {
        // replaceByName, getByName

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
sal_Bool HierarchyEntry::move(
                const OUString& rNewURL, const HierarchyEntryData& rData )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    OUString aNewPath = createPathFromHierarchyURL( rNewURL );

    if ( aNewPath == m_aPath )
        return sal_True;

#if 0
       // In the "near future"... ( not yet implemented in config db )

       - get update access for m_aPath
       - update access -> XNamed
       - xNamed::setName( newName )
       - updateaccess commit
#else

    OUString aOldKey  = m_aPath;
    sal_Bool bOldRoot = sal_True;
    Reference< XChangesBatch > xOldParentBatch;

    OUString aNewKey  = aNewPath;
    sal_Bool bNewRoot = sal_True;
    Reference< XChangesBatch > xNewParentBatch;

    sal_Bool bDifferentParents = sal_True;

    try
    {
        if ( !m_xConfigProvider.is() )
            m_xConfigProvider = Reference< XMultiServiceFactory >(
                m_xSMgr->createInstance(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationProvider" ) ),
                UNO_QUERY );

        if ( !m_xConfigProvider.is() )
            return sal_False;

        OUString aOldParentPath(
            RTL_CONSTASCII_USTRINGPARAM( HIERARCHY_ROOT_DB_KEY ) );
        sal_Int32 nPos = m_aPath.lastIndexOf( '/' );
        if ( nPos != -1 )
        {
            aOldKey = m_aPath.copy( nPos + 1 );

            // Skip "/Children" segment of the path, too.
            nPos = m_aPath.lastIndexOf( '/', nPos - 1 );

            VOS_ENSURE( nPos != -1, "HierarchyEntry::move - Wrong path!" );

            aOldParentPath += OUString::createFromAscii( "/" );
            aOldParentPath += m_aPath.copy( 0, nPos );
            bOldRoot = sal_False;
        }

        OUString aNewParentPath(
                    RTL_CONSTASCII_USTRINGPARAM( HIERARCHY_ROOT_DB_KEY ) );
        nPos = aNewPath.lastIndexOf( '/' );
        if ( nPos != -1 )
        {
            aNewKey = aNewPath.copy( nPos + 1 );

            // Skip "/Children" segment of the path, too.
            nPos = aNewPath.lastIndexOf( '/', nPos - 1 );

            VOS_ENSURE( nPos != -1, "HierarchyEntry::move - Wrong path!" );

            aNewParentPath += OUString::createFromAscii( "/" );
            aNewParentPath += aNewPath.copy( 0, nPos );
            bNewRoot = sal_False;
        }

        Sequence< Any > aArguments( 2 );
        PropertyValue   aProperty;

        aProperty.Name    = OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                    CFGPROPERTY_NODEPATH ) );
        aProperty.Value <<= aOldParentPath;
        aArguments[ 0 ] <<= aProperty;

        aProperty.Name    = OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                    CFGPROPERTY_LAZYWRITE ) );
        aProperty.Value <<= sal_True;
        aArguments[ 1 ] <<= aProperty;

        xOldParentBatch = Reference< XChangesBatch >(
            m_xConfigProvider->createInstanceWithArguments(
                OUString::createFromAscii(
                    "com.sun.star.configuration.ConfigurationUpdateAccess" ),
                aArguments ),
            UNO_QUERY );

        VOS_ENSURE( xOldParentBatch.is(), "HierarchyEntry::move - No batch!" );

        if ( !xOldParentBatch.is() )
            return sal_False;

        if ( aOldParentPath == aNewParentPath )
        {
            bDifferentParents = sal_False;
            xNewParentBatch = xOldParentBatch;
        }
        else
        {
            bDifferentParents = sal_True;

            aProperty.Name    = OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                    CFGPROPERTY_NODEPATH ) );
            aProperty.Value <<= aNewParentPath;
            aArguments[ 0 ] <<= aProperty;

            xNewParentBatch = Reference< XChangesBatch >(
                m_xConfigProvider->createInstanceWithArguments(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationUpdateAccess" ),
                    aArguments ),
                UNO_QUERY );

            VOS_ENSURE( xNewParentBatch.is(), "HierarchyEntry::move - No batch!" );

            if ( !xNewParentBatch.is() )
                return sal_False;
        }
    }
    catch ( RuntimeException& )
    {
        throw;
    }
    catch ( Exception& )
    {
        // createInstance, createInstanceWithArguments

        VOS_ENSURE( sal_False, "HierarchyEntry::move - caught Exception!" );
        return sal_False;
    }

    //////////////////////////////////////////////////////////////////////
    // (1) Get entry...
    //////////////////////////////////////////////////////////////////////

    Any aEntry;
    Reference< XNameAccess >    xOldParentNameAccess;
    Reference< XNameContainer > xOldNameContainer;

    try
    {
        xOldParentNameAccess
            = Reference< XNameAccess >( xOldParentBatch, UNO_QUERY );

        VOS_ENSURE( xOldParentNameAccess.is(),
                    "HierarchyEntry::move - No name access!" );

        if ( !xOldParentNameAccess.is() )
            return sal_False;

        if ( bOldRoot )
        {
            xOldNameContainer = Reference< XNameContainer >(
                                        xOldParentNameAccess, UNO_QUERY );
        }
        else
        {
            xOldParentNameAccess->getByName(
                 OUString::createFromAscii( "Children" ) ) >>= xOldNameContainer;
        }

        aEntry = xOldNameContainer->getByName( aOldKey );
    }
    catch ( NoSuchElementException& )
    {
        // getByName

        VOS_ENSURE( sal_False,
                    "HierarchyEntry::move - caught NoSuchElementException!" );
        return sal_False;
    }
    catch ( WrappedTargetException& )
    {
        // getByName

        VOS_ENSURE( sal_False,
                    "HierarchyEntry::move - caught WrappedTargetException!" );
        return sal_False;
    }

    //////////////////////////////////////////////////////////////////////
    // (2) Remove entry... Note: Insert BEFORE remove does not work!
    //////////////////////////////////////////////////////////////////////

    try
    {
        xOldNameContainer->removeByName( aOldKey );
        xOldParentBatch->commitChanges();
    }
    catch ( NoSuchElementException& )
    {
        // getByName, removeByName

        VOS_ENSURE( sal_False,
                    "HierarchyEntry::move - caught NoSuchElementException!" );
        return sal_False;
    }

    //////////////////////////////////////////////////////////////////////
    // (3) Insert entry at new parent...
    //////////////////////////////////////////////////////////////////////

    try
    {
        Reference< XNameReplace > xNewNameReplace;
        aEntry >>= xNewNameReplace;

        VOS_ENSURE( xNewNameReplace.is(),
                    "HierarchyEntry::move - No name replace!" );

        if ( !xNewNameReplace.is() )
            return sal_False;

        Reference< XNameAccess > xNewParentNameAccess;
        if ( bDifferentParents )
            xNewParentNameAccess
                = Reference< XNameAccess >( xNewParentBatch, UNO_QUERY );
        else
            xNewParentNameAccess = xOldParentNameAccess;

        VOS_ENSURE( xNewParentNameAccess.is(),
                    "HierarchyEntry::move - No name access!" );

        if ( !xNewParentNameAccess.is() )
            return sal_False;

        Reference< XNameContainer > xNewNameContainer;
        if ( bDifferentParents )
        {
            if ( bNewRoot )
            {
                xNewNameContainer = Reference< XNameContainer >(
                                            xNewParentNameAccess, UNO_QUERY );
            }
            else
            {
                xNewParentNameAccess->getByName(
                     OUString::createFromAscii( "Children" ) )
                        >>= xNewNameContainer;
            }
        }
        else
            xNewNameContainer = xOldNameContainer;

        if ( !xNewNameContainer.is() )
            return sal_False;

        xNewNameReplace->replaceByName(
                            OUString::createFromAscii( "Title" ),
                            makeAny( rData.aTitle ) );
        xNewNameReplace->replaceByName(
                            OUString::createFromAscii( "TargetURL" ),
                            makeAny( rData.aTargetURL ) );

        xNewNameContainer->insertByName( aNewKey, aEntry );
        xNewParentBatch->commitChanges();
    }
    catch ( NoSuchElementException& )
    {
        // replaceByName, insertByName, getByName

        VOS_ENSURE( sal_False,
                    "HierarchyEntry::move - caught NoSuchElementException!" );
        return sal_False;
    }
    catch ( IllegalArgumentException& )
    {
        // replaceByName, insertByName

        VOS_ENSURE( sal_False,
                    "HierarchyEntry::move - caught IllegalArgumentException!" );
        return sal_False;
    }
    catch ( ElementExistException& )
    {
        // insertByName

        VOS_ENSURE( sal_False,
                    "HierarchyEntry::move - caught ElementExistException!" );
        return sal_False;
    }
    catch ( WrappedTargetException& )
    {
        // replaceByName, insertByName, getByName

        VOS_ENSURE( sal_False,
                    "HierarchyEntry::move - caught WrappedTargetException!" );
        return sal_False;
    }

#if 0
    //////////////////////////////////////////////////////////////////////
    // (4) Commit changes...
    //////////////////////////////////////////////////////////////////////

    try
    {
        xNewParentBatch->commitChanges();

        if ( bDifferentParents )
            xOldParentBatch->commitChanges();
    }
    catch ( WrappedTargetException& )
    {
        // commitChanges

        VOS_ENSURE( sal_False,
                    "HierarchyEntry::move - caught WrappedTargetException!" );
        return sal_False;
    }
#endif

    return sal_True;
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

            OUString aParentPath(
                RTL_CONSTASCII_USTRINGPARAM( HIERARCHY_ROOT_DB_KEY ) );
            OUString aKey  = m_aPath;
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

            Sequence< Any > aArguments( 2 );
            PropertyValue   aProperty;

            aProperty.Name    = OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                    CFGPROPERTY_NODEPATH ) );
            aProperty.Value <<= aParentPath;
            aArguments[ 0 ] <<= aProperty;

            aProperty.Name    = OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                    CFGPROPERTY_LAZYWRITE ) );
            aProperty.Value <<= sal_True;
            aArguments[ 1 ] <<= aProperty;

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
            Reference< XHierarchicalNameAccess > xRootHierNameAccess
                = getRootReadAccess();

            if ( xRootHierNameAccess.is() )
            {
                Reference< XNameAccess > xNameAccess;

                if ( m_aPath.getLength() > 0 )
                {
                    OUString aPath = m_aPath;
                    aPath += OUString::createFromAscii( "/Children" );

                    xRootHierNameAccess->getByHierarchicalName( aPath )
                        >>= xNameAccess;
                }
                else
                    xNameAccess
                        = Reference< XNameAccess >(
                                xRootHierNameAccess, UNO_QUERY );

                VOS_ENSURE( xNameAccess.is(),
                            "HierarchyEntry::first - No name access!" );

                if ( xNameAccess.is() )
                    it.m_pImpl->names = xNameAccess->getElementNames();

                Reference< XHierarchicalNameAccess > xHierNameAccess(
                                                xNameAccess, UNO_QUERY );

                VOS_ENSURE( xHierNameAccess.is(),
                            "HierarchyEntry::first - No hier. name access!" );

                Reference< XStringEscape > xEscaper(
                                            xRootHierNameAccess, UNO_QUERY );

                VOS_ENSURE( xEscaper.is(),
                            "HierarchyEntry::first - No string escaper!" );

                it.m_pImpl->dir = xHierNameAccess;
                it.m_pImpl->esc = xEscaper;
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
                    "HierarchyEntry::first - caught NoSuchElementException!" );
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
    Reference< XStringEscape > xEscaper( getRootReadAccess(), UNO_QUERY );

    VOS_ENSURE( xEscaper.is(),
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

            if ( xEscaper.is() )
            {
                try
                {
                    aToken = xEscaper->escapeString( aToken );
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
Reference< XHierarchicalNameAccess > HierarchyEntry::getRootReadAccess()
{
    if ( !m_xRootReadAccess.is() )
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if ( !m_xRootReadAccess.is() )
        {
            if ( m_bTriedToGetRootReadAccess ) // #82494#
            {
                OSL_ENSURE( sal_False,
                            "HierarchyEntry::getRootReadAccess - "
                            "Unable to read any config data! -> #82494#" );
                return Reference< XHierarchicalNameAccess >();
            }

            try
            {
                if ( !m_xConfigProvider.is() )
                    m_xConfigProvider = Reference< XMultiServiceFactory >(
                        m_xSMgr->createInstance(
                            OUString::createFromAscii(
                                "com.sun.star.configuration."
                                "ConfigurationProvider" ) ),
                        UNO_QUERY );

                if ( m_xConfigProvider.is() )
                {
                    // Create Root object.

                    Sequence< Any > aArguments( 1 );
                    aArguments[ 0 ]
                        <<= OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                HIERARCHY_ROOT_DB_KEY ) );

                    m_bTriedToGetRootReadAccess = sal_True;

                    m_xRootReadAccess = Reference< XHierarchicalNameAccess >(
                        m_xConfigProvider->createInstanceWithArguments(
                            OUString::createFromAscii(
                                "com.sun.star.configuration."
                                "ConfigurationAccess" ),
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
                            "HierarchyEntry::getRootReadAccess - "
                            "caught Exception!" );
            }
        }
    }
    return m_xRootReadAccess;
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

            // key may be encoded!
            if ( m_pImpl->esc.is() )
            {
                 try
                 {
                    aKey = m_pImpl->esc->unescapeString( aKey );
                }
                catch ( IllegalArgumentException& )
                {
                }
                catch ( Exception& )
                {
                }
            }

            m_pImpl->entry.aName = aKey;
        }
        catch ( NoSuchElementException& )
        {
            m_pImpl->entry = HierarchyEntryData();
        }
    }

    return m_pImpl->entry;
}

