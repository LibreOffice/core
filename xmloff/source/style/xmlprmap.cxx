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

#include <tools/debug.hxx>
#include <xmloff/xmlprhdl.hxx>
#include "xmlbahdl.hxx"
#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmltypes.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <xmloff/xmltoken.hxx>

using namespace ::std;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using ::xmloff::token::GetXMLToken;

XMLPropertySetMapperEntry_Impl::XMLPropertySetMapperEntry_Impl(
    const XMLPropertyMapEntry& rMapEntry,
    const UniReference< XMLPropertyHandlerFactory >& rFactory ) :
    sXMLAttributeName( GetXMLToken(rMapEntry.meXMLName) ),
    sAPIPropertyName( OUString(rMapEntry.msApiName, rMapEntry.nApiNameLength,
                               RTL_TEXTENCODING_ASCII_US ) ),
    nXMLNameSpace( rMapEntry.mnNameSpace ),
    nType( rMapEntry.mnType ),
    nContextId( rMapEntry.mnContextId ),
    nEarliestODFVersionForExport( rMapEntry.mnEarliestODFVersionForExport ),
    pHdl( rFactory->GetPropertyHandler( rMapEntry.mnType & MID_FLAG_MASK ) )
{
}

XMLPropertySetMapperEntry_Impl::XMLPropertySetMapperEntry_Impl(
        const XMLPropertySetMapperEntry_Impl& rEntry ) :
    sXMLAttributeName( rEntry.sXMLAttributeName),
    sAPIPropertyName( rEntry.sAPIPropertyName),
    nXMLNameSpace( rEntry.nXMLNameSpace),
    nType( rEntry.nType),
    nContextId( rEntry.nContextId),
    nEarliestODFVersionForExport( rEntry.nEarliestODFVersionForExport ),
    pHdl( rEntry.pHdl)
{
    DBG_ASSERT( pHdl, "Unknown XML property type handler!" );
}

// Ctor
XMLPropertySetMapper::XMLPropertySetMapper(
        const XMLPropertyMapEntry* pEntries,
        const UniReference< XMLPropertyHandlerFactory >& rFactory )
{
    aHdlFactories.push_back( rFactory );
    if( pEntries )
    {
        const XMLPropertyMapEntry* pIter = pEntries;

        // count entries
        while( pIter->msApiName )
        {
            XMLPropertySetMapperEntry_Impl aEntry( *pIter, rFactory );
            aMapEntries.push_back( aEntry );
            pIter++;
        }
    }
}

XMLPropertySetMapper::~XMLPropertySetMapper()
{
}

void XMLPropertySetMapper::AddMapperEntry(
    const UniReference < XMLPropertySetMapper >& rMapper )
{
    for( vector < UniReference < XMLPropertyHandlerFactory > >::iterator
            aFIter = rMapper->aHdlFactories.begin();
         aFIter != rMapper->aHdlFactories.end();
         ++aFIter )
    {
        aHdlFactories.push_back( *aFIter );
    }

    for( vector < XMLPropertySetMapperEntry_Impl >::iterator
            aEIter = rMapper->aMapEntries.begin();
         aEIter != rMapper->aMapEntries.end();
         ++aEIter )
    {
        aMapEntries.push_back( *aEIter );
    }
}

// Export a Property
sal_Bool XMLPropertySetMapper::exportXML(
        OUString& rStrExpValue,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    const XMLPropertyHandler* pHdl = GetPropertyHandler( rProperty.mnIndex );

    DBG_ASSERT( pHdl, "Unknown XML Type!" );
    if( pHdl )
        bRet = pHdl->exportXML( rStrExpValue, rProperty.maValue,
                                rUnitConverter );

    return bRet;
}

// Import a Property
sal_Bool XMLPropertySetMapper::importXML(
        const OUString& rStrImpValue,
        XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    sal_Bool bRet = sal_False;

    const XMLPropertyHandler* pHdl = GetPropertyHandler( rProperty.mnIndex );

    if( pHdl )
        bRet = pHdl->importXML( rStrImpValue, rProperty.maValue,
                                rUnitConverter );

    return bRet;
}

// Search for the given name and the namespace in the list and return
// the index of the entry
// If there is no matching entry the method returns -1
sal_Int32 XMLPropertySetMapper::GetEntryIndex(
        sal_uInt16 nNamespace,
        const OUString& rStrName,
        sal_uInt32 nPropType,
        sal_Int32 nStartAt /* = -1 */ ) const
{
    sal_Int32 nEntries = GetEntryCount();
    sal_Int32 nIndex= nStartAt == - 1? 0 : nStartAt+1;

    if ( nEntries )
    {
        do
        {
            const XMLPropertySetMapperEntry_Impl& rEntry = aMapEntries[nIndex];
            if( (!nPropType || nPropType == rEntry.GetPropType()) &&
                rEntry.nXMLNameSpace == nNamespace &&
                rStrName == rEntry.sXMLAttributeName )
                return nIndex;
            else
                nIndex++;

        } while( nIndex<nEntries );
    }

    return -1;
}

/** searches for an entry that matches the given api name, namespace and local name or -1 if nothing found */
sal_Int32 XMLPropertySetMapper::FindEntryIndex(
        const sal_Char* sApiName,
        sal_uInt16 nNameSpace,
        const OUString& sXMLName ) const
{
    sal_Int32 nIndex = 0;
    sal_Int32 nEntries = GetEntryCount();

    do
    {
        const XMLPropertySetMapperEntry_Impl& rEntry = aMapEntries[nIndex];
        if( rEntry.nXMLNameSpace == nNameSpace &&
            rEntry.sXMLAttributeName.equals( sXMLName ) &&
            0 == rEntry.sAPIPropertyName.compareToAscii( sApiName ) )
            return nIndex;
        else
            nIndex++;

    } while( nIndex < nEntries );

    return -1;
}

sal_Int32 XMLPropertySetMapper::FindEntryIndex( const sal_Int16 nContextId ) const
{
    const sal_Int32 nEntries = GetEntryCount();

    if ( nEntries )
    {
        sal_Int32 nIndex = 0;
        do
        {
            const XMLPropertySetMapperEntry_Impl& rEntry = aMapEntries[nIndex];
            if( rEntry.nContextId == nContextId )
                return nIndex;
            else
                nIndex++;

        } while( nIndex < nEntries );
    }

    return -1;
}

void XMLPropertySetMapper::RemoveEntry( sal_Int32 nIndex )
{
    const sal_Int32 nEntries = GetEntryCount();
    if( nIndex>=nEntries || nIndex<0 )
        return;
    vector < XMLPropertySetMapperEntry_Impl >::iterator aEIter = aMapEntries.begin();
    for( sal_Int32 nN=0; nN<nIndex; nN++ )
        ++aEIter;
    aMapEntries.erase( aEIter );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
