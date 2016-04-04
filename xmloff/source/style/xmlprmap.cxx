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

#include <xmloff/xmlprmap.hxx>
#include <xmloff/xmlprhdl.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/prhdlfac.hxx>
#include <tools/debug.hxx>

#include "xmlbahdl.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <vector>

using namespace ::std;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using ::xmloff::token::GetXMLToken;

/** Helper-class for XML-im/export:
    - Holds a pointer to a given array of XMLPropertyMapEntry
    - Provides several methods to access data from this array
    - Holds a Sequence of XML-names (for properties)
    - The filter takes all properties of the XPropertySet which are also
      in the XMLPropertyMapEntry and which are have not a default value
      and put them into a vector of XMLPropertyStae
    - this class knows how to compare, im/export properties

    Attention: At all methods, which get an index as parameter, there is no
               range validation to save runtime !!
*/
struct XMLPropertySetMapperEntry_Impl
{
    OUString                           sXMLAttributeName;
    OUString                           sAPIPropertyName;
    sal_Int32                          nType;
    sal_uInt16                         nXMLNameSpace;
    sal_Int16                          nContextId;
    SvtSaveOptions::ODFDefaultVersion  nEarliestODFVersionForExport;
    bool                               bImportOnly;
    const XMLPropertyHandler          *pHdl;

    XMLPropertySetMapperEntry_Impl(
        const XMLPropertyMapEntry& rMapEntry,
        const rtl::Reference< XMLPropertyHandlerFactory >& rFactory );

    XMLPropertySetMapperEntry_Impl(
        const XMLPropertySetMapperEntry_Impl& rEntry );

    sal_uInt32 GetPropType() const { return nType & XML_TYPE_PROP_MASK; }
};

XMLPropertySetMapperEntry_Impl::XMLPropertySetMapperEntry_Impl(
    const XMLPropertyMapEntry& rMapEntry,
    const rtl::Reference< XMLPropertyHandlerFactory >& rFactory ) :
    sXMLAttributeName( GetXMLToken(rMapEntry.meXMLName) ),
    sAPIPropertyName( OUString(rMapEntry.msApiName, rMapEntry.nApiNameLength,
                               RTL_TEXTENCODING_ASCII_US ) ),
    nType( rMapEntry.mnType ),
    nXMLNameSpace( rMapEntry.mnNameSpace ),
    nContextId( rMapEntry.mnContextId ),
    nEarliestODFVersionForExport( rMapEntry.mnEarliestODFVersionForExport ),
    bImportOnly( rMapEntry.mbImportOnly),
    pHdl( rFactory->GetPropertyHandler( rMapEntry.mnType & MID_FLAG_MASK ) )
{
    assert(pHdl);
}

XMLPropertySetMapperEntry_Impl::XMLPropertySetMapperEntry_Impl(
        const XMLPropertySetMapperEntry_Impl& rEntry ) :
    sXMLAttributeName( rEntry.sXMLAttributeName),
    sAPIPropertyName( rEntry.sAPIPropertyName),
    nType( rEntry.nType),
    nXMLNameSpace( rEntry.nXMLNameSpace),
    nContextId( rEntry.nContextId),
    nEarliestODFVersionForExport( rEntry.nEarliestODFVersionForExport ),
    bImportOnly( rEntry.bImportOnly),
    pHdl( rEntry.pHdl)
{
    assert(pHdl);
}

struct XMLPropertySetMapper::Impl
{
    std::vector<XMLPropertySetMapperEntry_Impl> maMapEntries;
    std::vector<rtl::Reference <XMLPropertyHandlerFactory> > maHdlFactories;

    bool mbOnlyExportMappings;

    explicit Impl( bool bForExport ) : mbOnlyExportMappings(bForExport) {}
};

// Ctor
XMLPropertySetMapper::XMLPropertySetMapper(
    const XMLPropertyMapEntry* pEntries, const rtl::Reference<XMLPropertyHandlerFactory>& rFactory,
    bool bForExport ) :
    mpImpl(new Impl(bForExport))
{
    mpImpl->maHdlFactories.push_back(rFactory);
    if( pEntries )
    {
        const XMLPropertyMapEntry* pIter = pEntries;

        if (mpImpl->mbOnlyExportMappings)
        {
            while( pIter->msApiName )
            {
                if (!pIter->mbImportOnly)
                {
                    XMLPropertySetMapperEntry_Impl aEntry( *pIter, rFactory );
                    mpImpl->maMapEntries.push_back( aEntry );
                }
                pIter++;
            }
        }
        else
        {
            while( pIter->msApiName )
            {
                XMLPropertySetMapperEntry_Impl aEntry( *pIter, rFactory );
                mpImpl->maMapEntries.push_back( aEntry );
                pIter++;
            }
        }
    }
}

XMLPropertySetMapper::~XMLPropertySetMapper()
{
}

void XMLPropertySetMapper::AddMapperEntry(
    const rtl::Reference < XMLPropertySetMapper >& rMapper )
{
    for( vector < rtl::Reference < XMLPropertyHandlerFactory > >::iterator
            aFIter = rMapper->mpImpl->maHdlFactories.begin();
         aFIter != rMapper->mpImpl->maHdlFactories.end();
         ++aFIter )
    {
        mpImpl->maHdlFactories.push_back(*aFIter);
    }

    for( vector < XMLPropertySetMapperEntry_Impl >::iterator
            aEIter = rMapper->mpImpl->maMapEntries.begin();
         aEIter != rMapper->mpImpl->maMapEntries.end();
         ++aEIter )
    {
        if (!mpImpl->mbOnlyExportMappings || !(*aEIter).bImportOnly)
            mpImpl->maMapEntries.push_back( *aEIter );
    }
}

sal_Int32 XMLPropertySetMapper::GetEntryCount() const
{
    return mpImpl->maMapEntries.size();
}

sal_uInt32 XMLPropertySetMapper::GetEntryFlags( sal_Int32 nIndex ) const
{
    assert((0 <= nIndex) && (nIndex < static_cast<sal_Int32>(mpImpl->maMapEntries.size())));
    return mpImpl->maMapEntries[nIndex].nType & ~MID_FLAG_MASK;
}

sal_uInt32 XMLPropertySetMapper::GetEntryType( sal_Int32 nIndex ) const
{
    assert((0 <= nIndex) && (nIndex < static_cast<sal_Int32>(mpImpl->maMapEntries.size())));
    sal_uInt32 nType = mpImpl->maMapEntries[nIndex].nType;
    return nType;
}

sal_uInt16 XMLPropertySetMapper::GetEntryNameSpace( sal_Int32 nIndex ) const
{
    assert((0 <= nIndex) && (nIndex < static_cast<sal_Int32>(mpImpl->maMapEntries.size())));
    return mpImpl->maMapEntries[nIndex].nXMLNameSpace;
}

const OUString& XMLPropertySetMapper::GetEntryXMLName( sal_Int32 nIndex ) const
{
    assert((0 <= nIndex) && (nIndex < static_cast<sal_Int32>(mpImpl->maMapEntries.size())));
    return mpImpl->maMapEntries[nIndex].sXMLAttributeName;
}

const OUString& XMLPropertySetMapper::GetEntryAPIName( sal_Int32 nIndex ) const
{
    assert((0 <= nIndex) && (nIndex < static_cast<sal_Int32>(mpImpl->maMapEntries.size())));
    return mpImpl->maMapEntries[nIndex].sAPIPropertyName;
}

sal_Int16 XMLPropertySetMapper::GetEntryContextId( sal_Int32 nIndex ) const
{
    assert((-1 <= nIndex) && (nIndex < static_cast<sal_Int32>(mpImpl->maMapEntries.size())));
    return nIndex == -1 ? 0 : mpImpl->maMapEntries[nIndex].nContextId;
}

SvtSaveOptions::ODFDefaultVersion XMLPropertySetMapper::GetEarliestODFVersionForExport( sal_Int32 nIndex ) const
{
    assert((-1 <= nIndex) && (nIndex < static_cast<sal_Int32>(mpImpl->maMapEntries.size())));
    return nIndex == -1 ? SvtSaveOptions::ODFVER_UNKNOWN : mpImpl->maMapEntries[nIndex].nEarliestODFVersionForExport;
}

const XMLPropertyHandler* XMLPropertySetMapper::GetPropertyHandler( sal_Int32 nIndex ) const
{
    assert((0 <= nIndex) && (nIndex < static_cast<sal_Int32>(mpImpl->maMapEntries.size())));
    return mpImpl->maMapEntries[nIndex].pHdl;
}

// Export a Property
bool XMLPropertySetMapper::exportXML(
        OUString& rStrExpValue,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    bool bRet = false;

    const XMLPropertyHandler* pHdl = GetPropertyHandler( rProperty.mnIndex );

    assert(pHdl);
    if( pHdl )
        bRet = pHdl->exportXML( rStrExpValue, rProperty.maValue,
                                rUnitConverter );

    return bRet;
}

// Import a Property
bool XMLPropertySetMapper::importXML(
        const OUString& rStrImpValue,
        XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter ) const
{
    bool bRet = false;

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

    if ( nEntries && nIndex < nEntries )
    {
        do
        {
            const XMLPropertySetMapperEntry_Impl& rEntry = mpImpl->maMapEntries[nIndex];
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
        const XMLPropertySetMapperEntry_Impl& rEntry = mpImpl->maMapEntries[nIndex];
        if( rEntry.nXMLNameSpace == nNameSpace &&
            rEntry.sXMLAttributeName.equals( sXMLName ) &&
            rEntry.sAPIPropertyName.equalsAscii( sApiName ) )
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
            const XMLPropertySetMapperEntry_Impl& rEntry = mpImpl->maMapEntries[nIndex];
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
    vector < XMLPropertySetMapperEntry_Impl >::iterator aEIter = mpImpl->maMapEntries.begin();
    for( sal_Int32 nN=0; nN<nIndex; nN++ )
        ++aEIter;
    mpImpl->maMapEntries.erase( aEIter );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
