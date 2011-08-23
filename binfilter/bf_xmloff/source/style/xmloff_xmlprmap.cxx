/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <tools/debug.hxx>



#include "xmlprmap.hxx"





namespace binfilter {


using namespace ::std;
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using ::binfilter::xmloff::token::GetXMLToken;

XMLPropertySetMapperEntry_Impl::XMLPropertySetMapperEntry_Impl(
    const XMLPropertyMapEntry& rMapEntry,
    const UniReference< XMLPropertyHandlerFactory >& rFactory ) :
    sXMLAttributeName( GetXMLToken(rMapEntry.meXMLName) ),
    sAPIPropertyName( OUString(rMapEntry.msApiName, rMapEntry.nApiNameLength,
                               RTL_TEXTENCODING_ASCII_US ) ),
    nXMLNameSpace( rMapEntry.mnNameSpace ),
    nType( rMapEntry.mnType ),
    nContextId( rMapEntry.mnContextId ),
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
    pHdl( rEntry.pHdl)
{
    DBG_ASSERT( pHdl, "Unknown XML property type handler!" );
}

///////////////////////////////////////////////////////////////////////////
//
// Ctor
//
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
         aFIter++ )
    {
        aHdlFactories.push_back( *aFIter );
    }

    for( vector < XMLPropertySetMapperEntry_Impl >::iterator
            aEIter = rMapper->aMapEntries.begin();
         aEIter != rMapper->aMapEntries.end();
         aEIter++ )
    {
        aMapEntries.push_back( *aEIter );
    }
}

///////////////////////////////////////////////////////////////////////////
//
// Export a Property
//
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

///////////////////////////////////////////////////////////////////////////
//
// Import a Property
//
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

///////////////////////////////////////////////////////////////////////////
//
// Search for the given name and the namespace in the list and return
// the index of the entry
// If there is no matching entry the method returns -1
//
sal_Int32 XMLPropertySetMapper::GetEntryIndex(
        sal_uInt16 nNamespace,
        const OUString& rStrName,
        sal_Int32 nStartAt /* = -1 */ ) const
{
    sal_Int32 nEntries = GetEntryCount();
    sal_Int32 nIndex= nStartAt == - 1? 0 : nStartAt+1;

    do
    {
        const XMLPropertySetMapperEntry_Impl& rEntry = aMapEntries[nIndex];
        if( rEntry.nXMLNameSpace == nNamespace &&
            rStrName == rEntry.sXMLAttributeName )
            return nIndex;
        else
            nIndex++;

    } while( nIndex<nEntries );

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
    sal_Int32 nIndex = 0;
    sal_Int32 nEntries = GetEntryCount();

    do
    {
        const XMLPropertySetMapperEntry_Impl& rEntry = aMapEntries[nIndex];
        if( rEntry.nContextId == nContextId )
            return nIndex;
        else
            nIndex++;

    } while( nIndex < nEntries );

    return -1;
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
