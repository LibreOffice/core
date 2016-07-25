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

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/xml/AttributeData.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XTolerantMultiPropertySet.hpp>
#include <com/sun/star/beans/TolerantPropertySetResultType.hpp>
#include <rtl/ustrbuf.hxx>
#include <list>
#include <map>

#include <xmloff/xmlexppr.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmlprhdl.hxx>

using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::xmloff::token;

#define GET_PROP_TYPE( f ) static_cast<sal_uInt16>((f & XML_TYPE_PROP_MASK) >> XML_TYPE_PROP_SHIFT)
#define ENTRY(t) { GET_PROP_TYPE(XML_TYPE_PROP_##t), XML_##t##_PROPERTIES }

namespace {

struct XMLPropTokens_Impl
{
    sal_uInt16 nType;
    XMLTokenEnum eToken;
};

const sal_uInt16 MAX_PROP_TYPES =
    (XML_TYPE_PROP_END >> XML_TYPE_PROP_SHIFT) -
    (XML_TYPE_PROP_START >> XML_TYPE_PROP_SHIFT);

XMLPropTokens_Impl aPropTokens[MAX_PROP_TYPES] =
{
    ENTRY(CHART),
    ENTRY(GRAPHIC),
    ENTRY(TABLE),
    ENTRY(TABLE_COLUMN),
    ENTRY(TABLE_ROW),
    ENTRY(TABLE_CELL),
    ENTRY(LIST_LEVEL),
    ENTRY(PARAGRAPH),
    ENTRY(TEXT),
    ENTRY(DRAWING_PAGE),
    ENTRY(PAGE_LAYOUT),
    ENTRY(HEADER_FOOTER),
    ENTRY(RUBY),
    ENTRY(SECTION)
};

// public methods

// Take all properties of the XPropertySet which are also found in the
// XMLPropertyMapEntry-array and which are not set to their default-value,
// if a state is available.
// After that I call the method 'ContextFilter'.

typedef std::list<XMLPropertyState> XMLPropertyStateList_Impl;

class XMLPropertyStates_Impl
{
    XMLPropertyStateList_Impl           aPropStates;
    XMLPropertyStateList_Impl::iterator aLastItr;
    sal_uInt32                          nCount;
public:
    XMLPropertyStates_Impl();
    void AddPropertyState(const XMLPropertyState& rPropState);
    void FillPropertyStateVector(std::vector<XMLPropertyState>& rVector);
};

XMLPropertyStates_Impl::XMLPropertyStates_Impl() :
    aPropStates(),
    nCount(0)
{
    aLastItr = aPropStates.begin();
}

void XMLPropertyStates_Impl::AddPropertyState(
        const XMLPropertyState& rPropState)
{
    XMLPropertyStateList_Impl::iterator aItr = aPropStates.begin();
    bool bInserted(false);
    if (nCount)
    {
        if (aLastItr->mnIndex < rPropState.mnIndex)
            aItr = ++aLastItr;
    }
    do
    {
        // TODO: one path required only
        if (aItr == aPropStates.end())
        {
            aLastItr = aPropStates.insert(aPropStates.end(), rPropState);
            bInserted = true;
            nCount++;
        }
        else if (aItr->mnIndex > rPropState.mnIndex)
        {
            aLastItr = aPropStates.insert(aItr, rPropState);
            bInserted = true;
            nCount++;
        }
    }
    while(!bInserted && (aItr++ != aPropStates.end()));
}

void XMLPropertyStates_Impl::FillPropertyStateVector(
        std::vector<XMLPropertyState>& rVector)
{
    if (nCount)
    {
        rVector.resize(nCount, XMLPropertyState(-1));
        ::std::copy( aPropStates.begin(), aPropStates.end(), rVector.begin() );
    }
}

class FilterPropertyInfo_Impl
{
    const OUString     sApiName;
    std::list<sal_uInt32>   aIndexes;

public:

    FilterPropertyInfo_Impl( const OUString& rApiName,
                             const sal_uInt32 nIndex);

    const OUString& GetApiName() const { return sApiName; }
    std::list<sal_uInt32>& GetIndexes() { return aIndexes; }

    // for sort
    bool operator< ( const FilterPropertyInfo_Impl& rArg ) const
    {
        return (GetApiName() < rArg.GetApiName());
    }
};

FilterPropertyInfo_Impl::FilterPropertyInfo_Impl(
        const OUString& rApiName,
        const sal_uInt32 nIndex ) :
    sApiName( rApiName )
{
    aIndexes.push_back(nIndex);
}

typedef std::list<FilterPropertyInfo_Impl> FilterPropertyInfoList_Impl;

class FilterPropertiesInfo_Impl
{
    sal_uInt32                              nCount;
    FilterPropertyInfoList_Impl             aPropInfos;
    FilterPropertyInfoList_Impl::iterator   aLastItr;

    Sequence <OUString>                     *pApiNames;

public:
    FilterPropertiesInfo_Impl();
    ~FilterPropertiesInfo_Impl();

    void AddProperty(const OUString& rApiName, const sal_uInt32 nIndex);
    const uno::Sequence<OUString>& GetApiNames();
    void FillPropertyStateArray(
            vector< XMLPropertyState >& rPropStates,
            const Reference< XPropertySet >& xPropSet,
            const rtl::Reference< XMLPropertySetMapper >& maPropMapper,
            const bool bDefault);
    sal_uInt32 GetPropertyCount() const { return nCount; }
};

FilterPropertiesInfo_Impl::FilterPropertiesInfo_Impl() :
    nCount(0),
    aPropInfos(),
    pApiNames( nullptr )
{
    aLastItr = aPropInfos.begin();
}

FilterPropertiesInfo_Impl::~FilterPropertiesInfo_Impl()
{
    delete pApiNames;
}

void FilterPropertiesInfo_Impl::AddProperty(
        const OUString& rApiName, const sal_uInt32 nIndex)
{
    aPropInfos.push_back(FilterPropertyInfo_Impl(rApiName, nIndex));
    nCount++;

    OSL_ENSURE( !pApiNames, "performance warning: API names already retrieved" );
    if( pApiNames )
    {
        delete pApiNames;
        pApiNames = nullptr;
    }
}

const uno::Sequence<OUString>& FilterPropertiesInfo_Impl::GetApiNames()
{
    OSL_ENSURE(nCount == aPropInfos.size(), "wrong property count");
    if( !pApiNames )
    {
        // we have to do three things:
        // 1) sort API names,
        // 2) merge duplicates,
        // 3) construct sequence

        // sort names
        aPropInfos.sort();

        // merge duplicates
        if ( nCount > 1 )
        {
            FilterPropertyInfoList_Impl::iterator aOld = aPropInfos.begin();
            FilterPropertyInfoList_Impl::iterator aEnd = aPropInfos.end();
            FilterPropertyInfoList_Impl::iterator aCurrent = aOld;
            ++aCurrent;

            while ( aCurrent != aEnd )
            {
                // equal to next element?
                if ( aOld->GetApiName().equals( aCurrent->GetApiName() ) )
                {
                    // if equal: merge index lists
                    aOld->GetIndexes().merge( aCurrent->GetIndexes() );
                    // erase element, and continue with next
                    aCurrent = aPropInfos.erase( aCurrent );
                    nCount--;
                }
                else
                {
                    // remember old element and continue with next
                    aOld = aCurrent;
                    ++aCurrent;
                }
            }
        }

        // construct sequence
        pApiNames = new Sequence < OUString >( nCount );
        OUString *pNames = pApiNames->getArray();
        FilterPropertyInfoList_Impl::iterator aItr = aPropInfos.begin();
        FilterPropertyInfoList_Impl::iterator aEnd = aPropInfos.end();
        for ( ; aItr != aEnd; ++aItr, ++pNames)
            *pNames = aItr->GetApiName();
    }

    return *pApiNames;
}

void FilterPropertiesInfo_Impl::FillPropertyStateArray(
        vector< XMLPropertyState >& rPropStates,
        const Reference< XPropertySet >& rPropSet,
        const rtl::Reference< XMLPropertySetMapper >& rPropMapper,
        const bool bDefault )
{
    XMLPropertyStates_Impl aPropStates;

    const uno::Sequence<OUString>& rApiNames = GetApiNames();

    Reference < XTolerantMultiPropertySet > xTolPropSet( rPropSet, UNO_QUERY );
    if (xTolPropSet.is())
    {
        if (!bDefault)
        {
            Sequence < beans::GetDirectPropertyTolerantResult > aResults(xTolPropSet->getDirectPropertyValuesTolerant(rApiNames));
            sal_Int32 nResultCount(aResults.getLength());
            if (nResultCount > 0)
            {
                const beans::GetDirectPropertyTolerantResult *pResults = aResults.getConstArray();
                FilterPropertyInfoList_Impl::iterator aPropIter(aPropInfos.begin());
                XMLPropertyState aNewProperty( -1 );
                sal_uInt32 i = 0;
                while (nResultCount > 0 && i < nCount)
                {
                    if (pResults->Name == aPropIter->GetApiName())
                    {
                        aNewProperty.mnIndex = -1;
                        aNewProperty.maValue = pResults->Value;

                        for( std::list<sal_uInt32>::iterator aIndexItr(aPropIter->GetIndexes().begin());
                            aIndexItr != aPropIter->GetIndexes().end();
                            ++aIndexItr )
                        {
                            aNewProperty.mnIndex = *aIndexItr;
                            aPropStates.AddPropertyState( aNewProperty );
                        }
                        ++pResults;
                        --nResultCount;
                    }
                    ++aPropIter;
                    ++i;
                }
            }
        }
        else
        {
            Sequence < beans::GetPropertyTolerantResult > aResults(xTolPropSet->getPropertyValuesTolerant(rApiNames));
            OSL_ENSURE( rApiNames.getLength() == aResults.getLength(), "wrong implemented XTolerantMultiPropertySet" );
            const beans::GetPropertyTolerantResult *pResults = aResults.getConstArray();
            FilterPropertyInfoList_Impl::iterator aPropIter(aPropInfos.begin());
            XMLPropertyState aNewProperty( -1 );
            sal_uInt32 nResultCount(aResults.getLength());
            OSL_ENSURE( nCount == nResultCount, "wrong implemented XTolerantMultiPropertySet??" );
            for( sal_uInt32 i = 0; i < nResultCount; ++i )
            {
                if ((pResults->Result == beans::TolerantPropertySetResultType::SUCCESS) &&
                    ((pResults->State == PropertyState_DIRECT_VALUE) || (pResults->State == PropertyState_DEFAULT_VALUE)))
                {
                    aNewProperty.mnIndex = -1;
                    aNewProperty.maValue = pResults->Value;

                    for( std::list<sal_uInt32>::iterator aIndexItr(aPropIter->GetIndexes().begin());
                        aIndexItr != aPropIter->GetIndexes().end();
                        ++aIndexItr )
                    {
                        aNewProperty.mnIndex = *aIndexItr;
                        aPropStates.AddPropertyState( aNewProperty );
                    }
                }
                ++pResults;
                ++aPropIter;
            }
        }
    }
    else
    {
        Sequence < PropertyState > aStates;
        const PropertyState *pStates = nullptr;
        Reference< XPropertyState > xPropState( rPropSet, UNO_QUERY );
        if( xPropState.is() )
        {
            aStates = xPropState->getPropertyStates( rApiNames );
            pStates = aStates.getConstArray();
        }

        Reference < XMultiPropertySet > xMultiPropSet( rPropSet, UNO_QUERY );
        if( xMultiPropSet.is() && !bDefault )
        {
            Sequence < Any > aValues;
            if( pStates )
            {
                // step 1: get value count
                sal_uInt32 nValueCount = 0;
                sal_uInt32 i;

                for( i = 0; i < nCount; ++i, ++pStates )
                {
                    if( (*pStates == PropertyState_DIRECT_VALUE)/* || (bDefault && (*pStates == PropertyState_DEFAULT_VALUE))*/ )
                        nValueCount++;
                }

                if( nValueCount )
                {
                    // step 2: collect property names
                    Sequence < OUString > aAPINames( nValueCount );
                    OUString *pAPINames = aAPINames.getArray();

                    ::std::vector< FilterPropertyInfoList_Impl::iterator > aPropIters;
                    aPropIters.reserve( nValueCount );

                    FilterPropertyInfoList_Impl::iterator aItr = aPropInfos.begin();
                    OSL_ENSURE(aItr != aPropInfos.end(),"Invalid iterator!");

                    pStates = aStates.getConstArray();
                    i = 0;
                    while( i < nValueCount )
                    {
                        if( (*pStates == PropertyState_DIRECT_VALUE)/* || (bDefault && (*pStates == PropertyState_DEFAULT_VALUE))*/ )
                        {
                            *pAPINames++ = aItr->GetApiName();
                            aPropIters.push_back( aItr );
                            ++i;
                        }
                        ++aItr;
                        ++pStates;
                    }

                    aValues = xMultiPropSet->getPropertyValues( aAPINames );
                    const Any *pValues = aValues.getConstArray();

                    ::std::vector< FilterPropertyInfoList_Impl::iterator >::const_iterator
                        pPropIter = aPropIters.begin();

                    XMLPropertyState aNewProperty( -1 );
                    for( i = 0; i < nValueCount; ++i )
                    {
                        aNewProperty.mnIndex = -1;
                        aNewProperty.maValue = *pValues;

                        const ::std::list< sal_uInt32 >& rIndexes( (*pPropIter)->GetIndexes() );
                        for (   std::list<sal_uInt32>::const_iterator aIndexItr = rIndexes.begin();
                                aIndexItr != rIndexes.end();
                                ++aIndexItr
                            )
                        {
                            aNewProperty.mnIndex = *aIndexItr;
                            aPropStates.AddPropertyState( aNewProperty );
                        }

                        ++pPropIter;
                        ++pValues;
                    }
                }
            }
            else
            {
                aValues = xMultiPropSet->getPropertyValues( rApiNames );
                const Any *pValues = aValues.getConstArray();

                FilterPropertyInfoList_Impl::iterator aItr = aPropInfos.begin();
                for(sal_uInt32 i = 0; i < nCount; ++i)
                {
                    // The value is stored in the PropertySet itself, add to list.
                    XMLPropertyState aNewProperty( -1 );
                    aNewProperty.maValue = *pValues;
                    ++pValues;
                    for( std::list<sal_uInt32>::iterator aIndexItr =
                            aItr->GetIndexes().begin();
                        aIndexItr != aItr->GetIndexes().end();
                        ++aIndexItr )
                    {
                        aNewProperty.mnIndex = *aIndexItr;
                        aPropStates.AddPropertyState( aNewProperty );
                    }
                    ++aItr;
                }
            }
        }
        else
        {
            FilterPropertyInfoList_Impl::iterator aItr = aPropInfos.begin();
            for(sal_uInt32 i = 0; i < nCount; ++i)
            {
                bool bDirectValue =
                    !pStates || *pStates == PropertyState_DIRECT_VALUE;
                if( bDirectValue || bDefault )
                {
                    // The value is stored in the PropertySet itself, add to list.
                    bool bGotValue = false;
                    XMLPropertyState aNewProperty( -1 );
                    for( std::list<sal_uInt32>::const_iterator aIndexItr =
                            aItr->GetIndexes().begin();
                        aIndexItr != aItr->GetIndexes().end();
                        ++aIndexItr )
                    {
                        if( bDirectValue ||
                            (rPropMapper->GetEntryFlags( *aIndexItr ) &
                                            MID_FLAG_DEFAULT_ITEM_EXPORT) != 0 )
                        {
                            try
                            {
                                if( !bGotValue )
                                {
                                    aNewProperty.maValue =
                                        rPropSet->getPropertyValue( aItr->GetApiName() );
                                    bGotValue = true;
                                }
                                aNewProperty.mnIndex = *aIndexItr;
                                aPropStates.AddPropertyState( aNewProperty );
                            }
                            catch( UnknownPropertyException& )
                            {
                                // might be a problem of getImplemenetationId
                                OSL_ENSURE( false, "unknown property in getPropertyValue" );
                            }

                        }
                    }
                }

                ++aItr;
                if( pStates )
                    ++pStates;
            }
        }
    }
    aPropStates.FillPropertyStateVector(rPropStates);
}

}

struct SvXMLExportPropertyMapper::Impl
{
    typedef std::map<css::uno::Reference<css::beans::XPropertySetInfo>, FilterPropertiesInfo_Impl*> CacheType;
    CacheType maCache;

    rtl::Reference<SvXMLExportPropertyMapper> mxNextMapper;
    rtl::Reference<XMLPropertySetMapper> mxPropMapper;

    OUString maStyleName;

    ~Impl()
    {
        CacheType::iterator it = maCache.begin(), itEnd = maCache.end();
        for (; it != itEnd; ++it)
            delete it->second;
    }
};

// ctor/dtor , class SvXMLExportPropertyMapper

SvXMLExportPropertyMapper::SvXMLExportPropertyMapper(
        const rtl::Reference< XMLPropertySetMapper >& rMapper ) :
    mpImpl(new Impl)
{
    mpImpl->mxPropMapper = rMapper;
}

SvXMLExportPropertyMapper::~SvXMLExportPropertyMapper()
{
}

void SvXMLExportPropertyMapper::ChainExportMapper(
        const rtl::Reference< SvXMLExportPropertyMapper>& rMapper )
{
    // add map entries from rMapper to current map
    mpImpl->mxPropMapper->AddMapperEntry( rMapper->getPropertySetMapper() );
    // rMapper uses the same map as 'this'
    rMapper->mpImpl->mxPropMapper = mpImpl->mxPropMapper;

    // set rMapper as last mapper in current chain
    rtl::Reference< SvXMLExportPropertyMapper > xNext = mpImpl->mxNextMapper;
    if( xNext.is())
    {
        while (xNext->mpImpl->mxNextMapper.is())
            xNext = xNext->mpImpl->mxNextMapper;
        xNext->mpImpl->mxNextMapper = rMapper;
    }
    else
        mpImpl->mxNextMapper = rMapper;

    // if rMapper was already chained, correct
    // map pointer of successors
    xNext = rMapper;

    while (xNext->mpImpl->mxNextMapper.is())
    {
        xNext = xNext->mpImpl->mxNextMapper;
        xNext->mpImpl->mxPropMapper = mpImpl->mxPropMapper;
    }
}

std::vector<XMLPropertyState> SvXMLExportPropertyMapper::Filter(
    const uno::Reference<beans::XPropertySet>& rPropSet, bool bEnableFoFontFamily ) const
{
    return Filter_(rPropSet, false, bEnableFoFontFamily);
}

std::vector<XMLPropertyState> SvXMLExportPropertyMapper::FilterDefaults(
    const uno::Reference<beans::XPropertySet>& rPropSet ) const
{
    return Filter_(rPropSet, true, false/*bEnableFoFontFamily*/);
}

vector<XMLPropertyState> SvXMLExportPropertyMapper::Filter_(
    const Reference<XPropertySet>& xPropSet, bool bDefault, bool bEnableFoFontFamily ) const
{
    vector< XMLPropertyState > aPropStateArray;

    // Retrieve XPropertySetInfo and XPropertyState
    Reference< XPropertySetInfo > xInfo( xPropSet->getPropertySetInfo() );
    if( !xInfo.is() )
        return aPropStateArray;

    sal_Int32 nProps = mpImpl->mxPropMapper->GetEntryCount();

    FilterPropertiesInfo_Impl *pFilterInfo = nullptr;

    Impl::CacheType::iterator aIter = mpImpl->maCache.find(xInfo);
    if (aIter != mpImpl->maCache.end())
        pFilterInfo = (*aIter).second;

    bool bDelInfo = false;
    if( !pFilterInfo )
    {
        const SvtSaveOptions::ODFDefaultVersion nCurrentVersion( SvtSaveOptions().GetODFDefaultVersion() );
        pFilterInfo = new FilterPropertiesInfo_Impl;
        for( sal_Int32 i=0; i < nProps; i++ )
        {
            // Are we allowed to ask for the property? (MID_FLAG_NO_PROP..)
            // Does the PropertySet contain name of mpEntries-array ?
            const OUString& rAPIName = mpImpl->mxPropMapper->GetEntryAPIName( i );
            const sal_Int32 nFlags = mpImpl->mxPropMapper->GetEntryFlags( i );
            if( (0 == (nFlags & MID_FLAG_NO_PROPERTY_EXPORT)) &&
                ( (0 != (nFlags & MID_FLAG_MUST_EXIST)) ||
                  xInfo->hasPropertyByName( rAPIName ) ) )
            {
                const SvtSaveOptions::ODFDefaultVersion nEarliestODFVersionForExport(
                        mpImpl->mxPropMapper->GetEarliestODFVersionForExport(i));
                if( nCurrentVersion >= nEarliestODFVersionForExport
                        || nCurrentVersion == SvtSaveOptions::ODFVER_UNKNOWN
                        || nEarliestODFVersionForExport == SvtSaveOptions::ODFVER_UNKNOWN )
                    pFilterInfo->AddProperty(rAPIName, i);
            }
        }

        // Check whether the property set info is destroyed if it is assigned to
        // a weak reference only; If it is destroyed, then every instance of
        // getPropertySetInfo returns a new object; such property set infos must
        // not be cached:
        WeakReference < XPropertySetInfo > xWeakInfo( xInfo );
        xInfo.clear();
        xInfo = xWeakInfo;
        if( xInfo.is() )
        {
            mpImpl->maCache.insert(Impl::CacheType::value_type(xInfo, pFilterInfo));
        }
        else
            bDelInfo = true;
    }

    if( pFilterInfo->GetPropertyCount() )
    {
        try
        {
            pFilterInfo->FillPropertyStateArray(
                aPropStateArray, xPropSet, mpImpl->mxPropMapper, bDefault);
        }
        catch( UnknownPropertyException& )
        {
            // might be a problem of getImplemenetationId
            OSL_ENSURE( false, "unknown property in getPropertyStates" );
        }
    }

    // Call context-filter
    if( !aPropStateArray.empty() )
        ContextFilter(bEnableFoFontFamily, aPropStateArray, xPropSet);

    // Have to do if we change from a vector to a list or something like that

    if( bDelInfo )
        delete pFilterInfo;

    return aPropStateArray;
}

void SvXMLExportPropertyMapper::ContextFilter(
        bool bEnableFoFontFamily,
        vector< XMLPropertyState >& rProperties,
        const Reference< XPropertySet >& rPropSet ) const
{
    // Derived class could implement this.
    if (mpImpl->mxNextMapper.is())
        mpImpl->mxNextMapper->ContextFilter(bEnableFoFontFamily, rProperties, rPropSet);
}

// Compares two Sequences of XMLPropertyState:
//  1.Number of elements equal ?
//  2.Index of each element equal ? (So I know whether the propertynames are the same)
//  3.Value of each element equal ?
bool SvXMLExportPropertyMapper::Equals(
        const vector< XMLPropertyState >& aProperties1,
        const vector< XMLPropertyState >& aProperties2 ) const
{
    bool bRet = true;
    sal_uInt32 nCount = aProperties1.size();

    if( nCount == aProperties2.size() )
    {
        sal_uInt32 nIndex = 0;
        while( bRet && nIndex < nCount )
        {
            const XMLPropertyState& rProp1 = aProperties1[ nIndex ];
            const XMLPropertyState& rProp2 = aProperties2[ nIndex ];

            // Compare index. If equal, compare value
            if( rProp1.mnIndex == rProp2.mnIndex )
            {
                if( rProp1.mnIndex != -1 )
                {
                    // Now compare values
                    if ( (mpImpl->mxPropMapper->GetEntryType( rProp1.mnIndex ) &
                          XML_TYPE_BUILDIN_CMP ) != 0 )
                        // simple type ( binary compare )
                        bRet = ( rProp1.maValue == rProp2.maValue );
                    else
                        // complex type ( ask for compare-function )
                        bRet = mpImpl->mxPropMapper->GetPropertyHandler(
                                    rProp1.mnIndex )->equals( rProp1.maValue,
                                                              rProp2.maValue );
                }
            }
            else
                bRet = false;

            nIndex++;
        }
    }
    else
        bRet = false;

    return bRet;
}

/** fills the given attribute list with the items in the given set
void SvXMLExportPropertyMapper::exportXML( SvXMLAttributeList& rAttrList,
        const ::std::vector< XMLPropertyState >& rProperties,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_uInt16 nFlags ) const
{
    _exportXML( rAttrList, rProperties, rUnitConverter, rNamespaceMap,
                nFlags, 0, -1, -1 );
}

void SvXMLExportPropertyMapper::exportXML( SvXMLAttributeList& rAttrList,
        const ::std::vector< XMLPropertyState >& rProperties,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_Int32 nPropMapStartIdx, sal_Int32 nPropMapEndIdx,
        sal_uInt16 nFlags ) const
{
    _exportXML( rAttrList, rProperties, rUnitConverter, rNamespaceMap,
                nFlags, 0, nPropMapStartIdx, nPropMapEndIdx );
}
*/

void SvXMLExportPropertyMapper::exportXML(
        SvXMLExport& rExport,
        const ::std::vector< XMLPropertyState >& rProperties,
        SvXmlExportFlags nFlags,
        bool bUseExtensionNamespaceForGraphicProperties) const
{
    exportXML(rExport, rProperties, -1, -1,  nFlags, bUseExtensionNamespaceForGraphicProperties);
}


void SvXMLExportPropertyMapper::exportXML(
        SvXMLExport& rExport,
        const ::std::vector< XMLPropertyState >& rProperties,
        sal_Int32 nPropMapStartIdx, sal_Int32 nPropMapEndIdx,
        SvXmlExportFlags nFlags, bool bUseExtensionNamespaceForGraphicProperties) const
{
    sal_uInt16 nPropTypeFlags = 0;
    for( sal_uInt16 i=0; i<MAX_PROP_TYPES; ++i )
    {
        sal_uInt16 nPropType = aPropTokens[i].nType;
        if( 0==i || (nPropTypeFlags & (1 << nPropType)) != 0 )
        {
            sal_uInt16 nNamespace = XML_NAMESPACE_STYLE;
            if (bUseExtensionNamespaceForGraphicProperties &&
                aPropTokens[i].eToken == xmloff::token::XML_GRAPHIC_PROPERTIES)
            {
                nNamespace = XML_NAMESPACE_LO_EXT;
                if (rExport.getDefaultVersion() <= SvtSaveOptions::ODFVER_012)
                {
                    continue; // don't write for ODF <= 1.2
                }
            }

            std::vector<sal_uInt16> aIndexArray;

            _exportXML( nPropType, nPropTypeFlags,
                        rExport.GetAttrList(), rProperties,
                        rExport.GetMM100UnitConverter(),
                        rExport.GetNamespaceMap(),
                        nFlags, &aIndexArray,
                        nPropMapStartIdx, nPropMapEndIdx );

            if( rExport.GetAttrList().getLength() > 0L ||
                (nFlags & SvXmlExportFlags::EMPTY) ||
                !aIndexArray.empty() )
            {
                SvXMLElementExport aElem( rExport, nNamespace,
                                  aPropTokens[i].eToken,
                                  bool(nFlags & SvXmlExportFlags::IGN_WS),
                                  false );

                exportElementItems( rExport, rProperties, nFlags, aIndexArray );
            }
        }
    }
}

/** this method is called for every item that has the
    MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
void SvXMLExportPropertyMapper::handleSpecialItem(
        SvXMLAttributeList& rAttrList,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx ) const
{
    OSL_ENSURE(mpImpl->mxNextMapper.is(), "special item not handled in xml export");
    if (mpImpl->mxNextMapper.is())
        mpImpl->mxNextMapper->handleSpecialItem(
            rAttrList, rProperty, rUnitConverter, rNamespaceMap, pProperties, nIdx);
}

/** this method is called for every item that has the
    MID_FLAG_ELEMENT_EXPORT flag set */
void SvXMLExportPropertyMapper::handleElementItem(
        SvXMLExport& rExport,
        const XMLPropertyState& rProperty,
        SvXmlExportFlags nFlags,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx ) const
{
    OSL_ENSURE(mpImpl->mxNextMapper.is(), "element item not handled in xml export");
    if (mpImpl->mxNextMapper.is())
        mpImpl->mxNextMapper->handleElementItem(rExport, rProperty, nFlags, pProperties, nIdx);
}

// protected methods

/** fills the given attribute list with the items in the given set */
void SvXMLExportPropertyMapper::_exportXML(
        sal_uInt16 nPropType, sal_uInt16& rPropTypeFlags,
        SvXMLAttributeList& rAttrList,
        const ::std::vector< XMLPropertyState >& rProperties,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        SvXmlExportFlags nFlags,
        std::vector<sal_uInt16>* pIndexArray,
        sal_Int32 nPropMapStartIdx, sal_Int32 nPropMapEndIdx ) const
{
    const sal_uInt32 nCount = rProperties.size();
    sal_uInt32 nIndex = 0;

    if( -1 == nPropMapStartIdx )
        nPropMapStartIdx = 0;
    if( -1 == nPropMapEndIdx )
        nPropMapEndIdx = mpImpl->mxPropMapper->GetEntryCount();

    while( nIndex < nCount )
    {
        sal_Int32 nPropMapIdx = rProperties[nIndex].mnIndex;
        if( nPropMapIdx >= nPropMapStartIdx &&
            nPropMapIdx < nPropMapEndIdx  )// valid entry?
        {
            sal_uInt32 nEFlags = mpImpl->mxPropMapper->GetEntryFlags(nPropMapIdx);
            sal_uInt16 nEPType = GET_PROP_TYPE(nEFlags);
            OSL_ENSURE(nEPType >= (XML_TYPE_PROP_START >> XML_TYPE_PROP_SHIFT),
                       "no prop type specified");
            rPropTypeFlags |= (1 << nEPType);
            if( nEPType == nPropType )
            {
                // we have a valid map entry here, so lets use it...
                if( ( nEFlags & MID_FLAG_ELEMENT_ITEM_EXPORT ) != 0 )
                {
                    // element items do not add any properties,
                    // we export it later
                    if( pIndexArray )
                    {
                        pIndexArray->push_back( (sal_uInt16)nIndex );
                    }
                }
                else
                {
                    _exportXML( rAttrList, rProperties[nIndex], rUnitConverter,
                                rNamespaceMap, nFlags, &rProperties, nIndex );
                }
            }
        }

        nIndex++;
    }
}

void SvXMLExportPropertyMapper::_exportXML(
        SvXMLAttributeList& rAttrList,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        SvXmlExportFlags /*nFlags*/,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx ) const
{
    if ((mpImpl->mxPropMapper->GetEntryFlags(rProperty.mnIndex) & MID_FLAG_SPECIAL_ITEM_EXPORT) != 0)
    {
        uno::Reference< container::XNameContainer > xAttrContainer;
        if( (rProperty.maValue >>= xAttrContainer) && xAttrContainer.is() )
        {
            SvXMLNamespaceMap *pNewNamespaceMap = nullptr;
            const SvXMLNamespaceMap *pNamespaceMap = &rNamespaceMap;

            uno::Sequence< OUString > aAttribNames( xAttrContainer->getElementNames() );
            const OUString* pAttribName = aAttribNames.getConstArray();

            const sal_Int32 nCount = aAttribNames.getLength();

            OUStringBuffer sNameBuffer;
            xml::AttributeData aData;
            for( sal_Int32 i=0; i < nCount; i++, pAttribName++ )
            {
                xAttrContainer->getByName( *pAttribName ) >>= aData;
                OUString sAttribName( *pAttribName );

                // extract namespace prefix from attribute name if it exists
                OUString sPrefix;
                const sal_Int32 nColonPos =
                    pAttribName->indexOf( ':' );
                if( nColonPos != -1 )
                    sPrefix = pAttribName->copy( 0, nColonPos );

                if( !sPrefix.isEmpty() )
                {
                    OUString sNamespace( aData.Namespace );

                    // if the prefix isn't defined yet or has another meaning,
                    // we have to redefine it now.
                    sal_uInt16 nKey = pNamespaceMap->GetKeyByPrefix( sPrefix );
                    if( USHRT_MAX == nKey || pNamespaceMap->GetNameByKey( nKey ) != sNamespace )
                    {
                        bool bAddNamespace = false;
                        if( USHRT_MAX == nKey )
                        {
                            // The prefix is unused, so it is sufficient
                            // to add it to the namespace map.
                            bAddNamespace = true;
                        }
                        else
                        {
                            // check if there is a prefix registered for the
                            // namepsace URI
                            nKey = pNamespaceMap->GetKeyByName( sNamespace );
                            if( XML_NAMESPACE_UNKNOWN == nKey )
                            {
                                // There is no prefix for the namespace, so
                                // we have to generate one and have to add it.
                                sal_Int32 n=0;
                                OUString sOrigPrefix( sPrefix );
                                do
                                {
                                    sNameBuffer.append( sOrigPrefix );
                                    sNameBuffer.append( ++n );
                                    sPrefix = sNameBuffer.makeStringAndClear();
                                    nKey = pNamespaceMap->GetKeyByPrefix( sPrefix );
                                }
                                while( nKey != USHRT_MAX );

                                bAddNamespace = true;
                            }
                            else
                            {
                                // If there is a prefix for the namespace,
                                // we reuse that.
                                sPrefix = pNamespaceMap->GetPrefixByKey( nKey );
                            }
                            // In any case, the attribute name has to be adapted.
                            sNameBuffer.append( sPrefix + ":" + pAttribName->copy( nColonPos+1 ) );
                            sAttribName = sNameBuffer.makeStringAndClear();
                        }

                        if( bAddNamespace )
                        {
                            if( !pNewNamespaceMap )
                            {
                                pNewNamespaceMap = new SvXMLNamespaceMap( rNamespaceMap );
                                pNamespaceMap = pNewNamespaceMap;
                            }
                            pNewNamespaceMap->Add( sPrefix, sNamespace );
                            sNameBuffer.append( GetXMLToken(XML_XMLNS) + ":" + sPrefix );
                            rAttrList.AddAttribute( sNameBuffer.makeStringAndClear(),
                                                    sNamespace );
                        }
                    }
                }
                OUString sOldValue( rAttrList.getValueByName( sAttribName ) );
                OSL_ENSURE( sOldValue.isEmpty(), "alien attribute exists already" );
                OSL_ENSURE(aData.Type == GetXMLToken(XML_CDATA), "different type to our default type which should be written out");
                if( sOldValue.isEmpty() )
                    rAttrList.AddAttribute( sAttribName, aData.Value );
            }

            delete pNewNamespaceMap;
        }
        else
        {
            handleSpecialItem( rAttrList, rProperty, rUnitConverter,
                               rNamespaceMap, pProperties, nIdx );
        }
    }
    else if ((mpImpl->mxPropMapper->GetEntryFlags(rProperty.mnIndex) & MID_FLAG_ELEMENT_ITEM_EXPORT ) == 0)
    {
        OUString aValue;
        const OUString sName = rNamespaceMap.GetQNameByKey(
            mpImpl->mxPropMapper->GetEntryNameSpace(rProperty.mnIndex),
            mpImpl->mxPropMapper->GetEntryXMLName(rProperty.mnIndex));

        bool bRemove = false;
        if ((mpImpl->mxPropMapper->GetEntryFlags( rProperty.mnIndex ) & MID_FLAG_MERGE_ATTRIBUTE) != 0)
        {
            aValue = rAttrList.getValueByName( sName );
            bRemove = true;
        }

        if (mpImpl->mxPropMapper->exportXML(aValue, rProperty, rUnitConverter))
        {
            if( bRemove )
                rAttrList.RemoveAttribute( sName );
            rAttrList.AddAttribute( sName, aValue );
        }
    }
}

void SvXMLExportPropertyMapper::exportElementItems(
        SvXMLExport& rExport,
        const ::std::vector< XMLPropertyState >& rProperties,
        SvXmlExportFlags nFlags,
        const std::vector<sal_uInt16>& rIndexArray ) const
{
    const sal_uInt16 nCount = rIndexArray.size();

    bool bItemsExported = false;
    for( sal_uInt16 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const sal_uInt16 nElement = rIndexArray[nIndex];

        OSL_ENSURE( 0 != (mpImpl->mxPropMapper->GetEntryFlags(
                rProperties[nElement].mnIndex ) & MID_FLAG_ELEMENT_ITEM_EXPORT),
                "wrong mid flag!" );

        rExport.IgnorableWhitespace();
        handleElementItem( rExport, rProperties[nElement],
                           nFlags, &rProperties, nElement );
        bItemsExported = true;
    }

    if( bItemsExported )
        rExport.IgnorableWhitespace();
}

const rtl::Reference<XMLPropertySetMapper>& SvXMLExportPropertyMapper::getPropertySetMapper() const
{
    return mpImpl->mxPropMapper;
}

void SvXMLExportPropertyMapper::SetStyleName( const OUString& rStyleName )
{
    mpImpl->maStyleName = rStyleName;
}

const OUString& SvXMLExportPropertyMapper::GetStyleName() const
{
    return mpImpl->maStyleName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
