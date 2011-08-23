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

#include <com/sun/star/container/XNameContainer.hpp>

#include <com/sun/star/xml/AttributeData.hpp>


#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>

#include <list>
#include <hash_map>




#include "nmspmap.hxx"

#include "xmlnmspe.hxx"

#include "xmlexp.hxx"



#ifndef _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTS
#include <bf_svtools/svstdarr.hxx>
#endif
namespace binfilter {

using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::binfilter::xmloff::token;

using rtl::OUString;
using rtl::OUStringBuffer;

///////////////////////////////////////////////////////////////////////////////
//
// public methods
//

///////////////////////////////////////////////////////////////////////////
//
// Take all properties of the XPropertySet which are also found in the
// XMLPropertyMapEntry-array and which are not set to their default-value,
// if a state is available.
//
// After that I call the method 'ContextFilter'.
//

typedef ::std::list<XMLPropertyState> XMLPropertyStateList_Impl;

class XMLPropertyStates_Impl
{
    XMLPropertyStateList_Impl			aPropStates;
    XMLPropertyStateList_Impl::iterator	aLastItr;
    sal_uInt32							nCount;
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
    sal_Bool bInserted(sal_False);
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
            bInserted = sal_True;
            nCount++;
        }
        else if (aItr->mnIndex > rPropState.mnIndex)
        {
            aLastItr = aPropStates.insert(aItr, rPropState);
            bInserted = sal_True;
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
        XMLPropertyStateList_Impl::iterator aItr = aPropStates.begin();
        sal_Int32 i (0);
        while (aItr != aPropStates.end())
        {
            rVector[i] = *aItr;
            aItr++;
            i++;
        }
    }
}

class FilterPropertyInfo_Impl
{
    const ::rtl::OUString		sApiName;
    std::list<sal_uInt32>	aIndexes;
    sal_uInt32				nCount;

public:

    FilterPropertyInfo_Impl( const ::rtl::OUString& rApiName,
                             const sal_uInt32 nIndex);

    const OUString& GetApiName() const { return sApiName; }
    std::list<sal_uInt32>& GetIndexes() { return aIndexes; }

    void AddIndex( sal_uInt32 nIndex )
    {
        aIndexes.push_back(nIndex);
        nCount++;
    }

    // for sort
    sal_Bool operator< ( const FilterPropertyInfo_Impl& rArg ) const
    {
        return (GetApiName() < rArg.GetApiName());
    }
};

FilterPropertyInfo_Impl::FilterPropertyInfo_Impl(
        const ::rtl::OUString& rApiName,
        const sal_uInt32 nIndex ) :
    sApiName( rApiName ),
    aIndexes(),
    nCount(1)
{
    aIndexes.push_back(nIndex);
}

typedef ::std::list<FilterPropertyInfo_Impl> FilterPropertyInfoList_Impl;

// ----------------------------------------------------------------------------

class FilterPropertiesInfo_Impl
{
    sal_uInt32								nCount;
    FilterPropertyInfoList_Impl				aPropInfos;
    FilterPropertyInfoList_Impl::iterator	aLastItr;

    Sequence <OUString> 					*pApiNames;

public:
    FilterPropertiesInfo_Impl();
    ~FilterPropertiesInfo_Impl();

    void AddProperty(const ::rtl::OUString& rApiName, const sal_uInt32 nIndex);
    const uno::Sequence<OUString>& GetApiNames();
    void FillPropertyStateArray(
            vector< XMLPropertyState >& rPropStates,
            const Reference< XPropertySet >& xPropSet,
            const UniReference< XMLPropertySetMapper >& maPropMapper,
            const sal_Bool bDefault = sal_False);
    sal_uInt32 GetPropertyCount() const { return nCount; }
};

// ----------------------------------------------------------------------------

typedef ::std::hash_map
<
    PropertySetInfoKey,
    FilterPropertiesInfo_Impl *,
    PropertySetInfoHash,
    PropertySetInfoHash
>
FilterOropertiesHashMap_Impl;

class FilterPropertiesInfos_Impl : public FilterOropertiesHashMap_Impl
{
public:
    ~FilterPropertiesInfos_Impl ();
};

FilterPropertiesInfos_Impl::~FilterPropertiesInfos_Impl ()
{
    FilterOropertiesHashMap_Impl::iterator aIter = begin();
    FilterOropertiesHashMap_Impl::iterator aEnd = end();
    while( aIter != aEnd )
    {
        delete (*aIter).second;
        (*aIter).second = 0;
        ++aIter;
    }
}

// ----------------------------------------------------------------------------

FilterPropertiesInfo_Impl::FilterPropertiesInfo_Impl() :
    aPropInfos(),
    nCount(0),
    pApiNames( 0 )
{
    aLastItr = aPropInfos.begin();
}

FilterPropertiesInfo_Impl::~FilterPropertiesInfo_Impl()
{
    delete pApiNames;
}

void FilterPropertiesInfo_Impl::AddProperty(
        const ::rtl::OUString& rApiName, const sal_uInt32 nIndex)
{
    aPropInfos.push_back(FilterPropertyInfo_Impl(rApiName, nIndex));
    nCount++;

    OSL_ENSURE( !pApiNames, "perfomance warning: API names already retrieved" );
    if( pApiNames )
    {
        delete pApiNames;
        pApiNames = NULL;
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
            aCurrent++;

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
                    aCurrent++;
                }
            }
        }

        // construct sequence
        pApiNames = new Sequence < OUString >( nCount );
        OUString *pNames = pApiNames->getArray();
        FilterPropertyInfoList_Impl::iterator aItr = aPropInfos.begin();
        FilterPropertyInfoList_Impl::iterator aEnd = aPropInfos.end();
        for ( ; aItr != aEnd; aItr++, pNames++)
            *pNames = aItr->GetApiName();
    }

    return *pApiNames;
}

void FilterPropertiesInfo_Impl::FillPropertyStateArray(
        vector< XMLPropertyState >& rPropStates,
        const Reference< XPropertySet >& rPropSet,
        const UniReference< XMLPropertySetMapper >& rPropMapper,
        const sal_Bool bDefault )
{
    XMLPropertyStates_Impl aPropStates;

    const uno::Sequence<OUString>& rApiNames = GetApiNames();

    Sequence < PropertyState > aStates;
    const PropertyState *pStates = 0;
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

            for( i = 0; i < nCount; i++, pStates++ )
            {
                if( *pStates == PropertyState_DIRECT_VALUE )
                    nValueCount++;
            }

            if( nValueCount )
            {
                // step 2: collect property names
                Sequence < OUString > aAPINames( nValueCount );
                OUString *pAPINames = aAPINames.getArray();

                FilterPropertyInfoList_Impl::iterator *aPropIters =
                    new FilterPropertyInfoList_Impl::iterator[nValueCount];
                FilterPropertyInfoList_Impl::iterator *pPropIter = aPropIters;

                FilterPropertyInfoList_Impl::iterator aItr = aPropInfos.begin();

                pStates = aStates.getConstArray();
                i = 0;
                while( i < nValueCount )
                {
                    if( *pStates == PropertyState_DIRECT_VALUE )
                    {
                        *pAPINames++ = aItr->GetApiName();
                        *pPropIter++ = aItr;
                        i++;
                    }
                    aItr++;
                    pStates++;
                }

                aValues = xMultiPropSet->getPropertyValues( aAPINames );
                const Any *pValues = aValues.getConstArray();
                pPropIter = aPropIters;
                XMLPropertyState aNewProperty( -1 );
                for( i = 0; i < nValueCount; i++ )
                {
                    aNewProperty.mnIndex = -1;
                    aNewProperty.maValue = *pValues;

                    for( ::std::list<sal_uInt32>::iterator aIndexItr =
                                (*pPropIter)->GetIndexes().begin();
                        aIndexItr != (*pPropIter)->GetIndexes().end();
                        aIndexItr++ )
                    {
                        aNewProperty.mnIndex = *aIndexItr;
                        aPropStates.AddPropertyState( aNewProperty );
                    }

                    pPropIter++;
                    pValues++;
                }
                delete[] aPropIters;
            }
        }
        else
        {
            aValues = xMultiPropSet->getPropertyValues( rApiNames );
            const Any *pValues = aValues.getConstArray();

            FilterPropertyInfoList_Impl::iterator aItr = aPropInfos.begin();
            for(sal_uInt32 i = 0; i < nCount; i++ )
            {
                // The value is stored in the PropertySet itself, add to list.
                sal_Bool bGotValue = sal_False;
                XMLPropertyState aNewProperty( -1 );
                aNewProperty.maValue = *pValues;
                for( ::std::list<sal_uInt32>::iterator aIndexItr =
                        aItr->GetIndexes().begin();
                    aIndexItr != aItr->GetIndexes().end();
                    aIndexItr++ )
                {
                    aNewProperty.mnIndex = *aIndexItr;
                    aPropStates.AddPropertyState( aNewProperty );
                }
                aItr++;
            }
        }
    }
    else
    {
        FilterPropertyInfoList_Impl::iterator aItr = aPropInfos.begin();
        for(sal_uInt32 i = 0; i < nCount; i++ )
        {
            sal_Bool bDirectValue =
                !pStates || *pStates == PropertyState_DIRECT_VALUE;
            if( bDirectValue || bDefault )
            {
                // The value is stored in the PropertySet itself, add to list.
                sal_Bool bGotValue = sal_False;
                XMLPropertyState aNewProperty( -1 );
                for( ::std::list<sal_uInt32>::iterator aIndexItr =
                        aItr->GetIndexes().begin();
                    aIndexItr != aItr->GetIndexes().end();
                    aIndexItr++ )
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
                                bGotValue = sal_True;
                            }
                            aNewProperty.mnIndex = *aIndexItr;
                            aPropStates.AddPropertyState( aNewProperty );
                        }
                        catch( UnknownPropertyException& )
                        {
                            // might be a problem of getImplemenetationId
                            OSL_ENSURE( !this, "unknown property in getPropertyValue" );
                        }

                    }
                }
            }

            aItr++;
            if( pStates )
                pStates++;
        }
    }
    aPropStates.FillPropertyStateVector(rPropStates);
}

///////////////////////////////////////////////////////////////////////////////
//
// ctor/dtor , class SvXMLExportPropertyMapper
//

SvXMLExportPropertyMapper::SvXMLExportPropertyMapper(
        const UniReference< XMLPropertySetMapper >& rMapper ) :
    maPropMapper( rMapper ),
    pCache( 0 )
{
}

SvXMLExportPropertyMapper::~SvXMLExportPropertyMapper()
{
    delete pCache;
    mxNextMapper = 0;
}

void SvXMLExportPropertyMapper::ChainExportMapper(
        const UniReference< SvXMLExportPropertyMapper>& rMapper )
{
    // add map entries from rMapper to current map
    maPropMapper->AddMapperEntry( rMapper->getPropertySetMapper() );
    // rMapper uses the same map as 'this'
    rMapper->maPropMapper = maPropMapper;

    // set rMapper as last mapper in current chain
    UniReference< SvXMLExportPropertyMapper > xNext = mxNextMapper;
    if( xNext.is())
    {
        while( xNext->mxNextMapper.is())
            xNext = xNext->mxNextMapper;
        xNext->mxNextMapper = rMapper;
    }
    else
        mxNextMapper = rMapper;

    // if rMapper was already chained, correct
    // map pointer of successors
    xNext = rMapper;

    while( xNext->mxNextMapper.is())
    {
        xNext = xNext->mxNextMapper;
        xNext->maPropMapper = maPropMapper;
    }
}


vector< XMLPropertyState > SvXMLExportPropertyMapper::_Filter(
        const Reference< XPropertySet > xPropSet,
        const sal_Bool bDefault ) const
{
    vector< XMLPropertyState > aPropStateArray;

    // Retrieve XPropertySetInfo and XPropertyState
    Reference< XPropertySetInfo > xInfo( xPropSet->getPropertySetInfo() );
    if( !xInfo.is() )
        return aPropStateArray;

    sal_Int32 nProps = maPropMapper->GetEntryCount();

    FilterPropertiesInfo_Impl *pFilterInfo = 0;

    Reference < XTypeProvider > xTypeProv( xPropSet, UNO_QUERY );
    Sequence< sal_Int8 > aImplId;
    if( xTypeProv.is() )
    {
        aImplId = xTypeProv->getImplementationId();
        if( aImplId.getLength() == 16 )
        {
            if( pCache )
            {
                // The key must not be created outside this block, because it
                // keeps a reference to the property set info.
                PropertySetInfoKey aKey( xInfo, aImplId );
                FilterPropertiesInfos_Impl::iterator aIter =
                    pCache->find( aKey );
                if( aIter != pCache->end() )
                    pFilterInfo = (*aIter).second;
            }
        }
    }

    sal_Bool bDelInfo = sal_False;
    if( !pFilterInfo )
    {
        pFilterInfo = new FilterPropertiesInfo_Impl;
        for( sal_Int32 i=0; i < nProps; i++ )
        {
            // Are we allowed to ask for the property? (MID_FLAG_NO_PROP..)
            // Does the PropertySet contain name of mpEntries-array ?
            const OUString& rAPIName = maPropMapper->GetEntryAPIName( i );
            const sal_Int32 nFlags = maPropMapper->GetEntryFlags( i );
            if( (0 == (nFlags & MID_FLAG_NO_PROPERTY_EXPORT)) &&
                ( (0 != (nFlags & MID_FLAG_MUST_EXIST)) ||
                  xInfo->hasPropertyByName( rAPIName ) ) )
                pFilterInfo->AddProperty(rAPIName, i);
        }

        if( xTypeProv.is() && aImplId.getLength() == 16 )
        {
            // Check whether the property set info is destroyed if it is
            // assigned to a weak reference only. If it is destroyed, then
            // every instance of getPropertySetInfo returns a new object.
            // Such property set infos must not be cached.
            WeakReference < XPropertySetInfo > xWeakInfo( xInfo );
            xInfo = 0;
            xInfo = xWeakInfo;
            if( xInfo.is() )
            {
                if( !pCache )
                    ((SvXMLExportPropertyMapper *)this)->pCache =
                        new FilterPropertiesInfos_Impl;
                PropertySetInfoKey aKey( xInfo, aImplId );
                (*pCache)[aKey] = pFilterInfo;
            }
            else
                bDelInfo = sal_True;
        }
        else
        {
            OSL_ENSURE(sal_False, "here is no TypeProvider or the ImplId is wrong");
            bDelInfo = sal_True;
        }
    }

    if( pFilterInfo->GetPropertyCount() )
    {
        try
        {
            pFilterInfo->FillPropertyStateArray(aPropStateArray,
                                                xPropSet, maPropMapper,
                                                bDefault);
        }
        catch( UnknownPropertyException& )
        {
            // might be a problem of getImplemenetationId
            OSL_ENSURE( !this, "unknown property in getPropertyStates" );
        }
    }

    // Call centext-filter
    if( aPropStateArray.size() > 0 )
        ContextFilter( aPropStateArray, xPropSet );

    // Have to do if we change from a vector to a list or something like that
    /*vector< XMLPropertyState >::iterator aItr = aPropStateArray.begin();
    while (aItr != aPropStateArray.end())
    {
        if (aItr->mnIndex == -1)
            aItr = aPropStateArray.erase(aItr);
        else
            aItr++;
    }*/

    if( bDelInfo )
        delete pFilterInfo;

    return aPropStateArray;
}

void SvXMLExportPropertyMapper::ContextFilter(
        vector< XMLPropertyState >& rProperties,
        Reference< XPropertySet > rPropSet ) const
{
    // Derived class could implement this.
    if( mxNextMapper.is() )
        mxNextMapper->ContextFilter( rProperties, rPropSet );
}

///////////////////////////////////////////////////////////////////////////
//
// Compares two Sequences of XMLPropertyState:
//	1.Number of elements equal ?
//	2.Index of each element equal ? (So I know whether the propertynames are the same)
//	3.Value of each element equal ?
//
sal_Bool SvXMLExportPropertyMapper::Equals(
        const vector< XMLPropertyState >& aProperties1,
        const vector< XMLPropertyState >& aProperties2 ) const
{
    sal_Bool bRet = sal_True;
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
                    if( ( maPropMapper->GetEntryType( rProp1.mnIndex ) &
                          XML_TYPE_BUILDIN_CMP ) != 0 )
                        // simple type ( binary compare )
                        bRet = ( rProp1.maValue == rProp2.maValue );
                    else
                        // complex type ( ask for compare-function )
                        bRet = maPropMapper->GetPropertyHandler(
                                    rProp1.mnIndex )->equals( rProp1.maValue,
                                                              rProp2.maValue );
                }
            }
            else
                bRet = sal_False;

            nIndex++;
        }
    }
    else
        bRet = sal_False;

    return bRet;
}

void SvXMLExportPropertyMapper::exportXML(
        SvXMLExport& rExport,
        const ::std::vector< XMLPropertyState >& rProperties,
        sal_uInt16 nFlags ) const
{
    exportXML( rExport, rProperties, -1, -1,  nFlags );
}

void SvXMLExportPropertyMapper::exportXML(
        SvXMLExport& rExport,
        const ::std::vector< XMLPropertyState >& rProperties,
        sal_Int32 nPropMapStartIdx, sal_Int32 nPropMapEndIdx,
        sal_uInt16 nFlags ) const
{
    SvUShorts aIndexArray;

    _exportXML( rExport.GetAttrList(), rProperties,
                rExport.GetMM100UnitConverter(), rExport.GetNamespaceMap(),
                nFlags, &aIndexArray, nPropMapStartIdx, nPropMapEndIdx );

    if( rExport.GetAttrList().getLength() > 0L ||
        (nFlags & XML_EXPORT_FLAG_EMPTY) != 0 ||
        aIndexArray.Count() != 0 )
    {
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_STYLE, XML_PROPERTIES,
                                  (nFlags & XML_EXPORT_FLAG_IGN_WS) != 0,
                                  sal_False );

        exportElementItems( rExport, rProperties, nFlags, aIndexArray );
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
    OSL_ENSURE( mxNextMapper.is(), "special item not handled in xml export" );
    if( mxNextMapper.is() )
        mxNextMapper->handleSpecialItem( rAttrList, rProperty, rUnitConverter,
                                        rNamespaceMap, pProperties, nIdx );
}

/** this method is called for every item that has the
    MID_FLAG_ELEMENT_EXPORT flag set */
void SvXMLExportPropertyMapper::handleElementItem(
        SvXMLExport& rExport,
        const XMLPropertyState& rProperty,
        sal_uInt16 nFlags,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx ) const
{
    OSL_ENSURE( mxNextMapper.is(), "element item not handled in xml export" );
    if( mxNextMapper.is() )
        mxNextMapper->handleElementItem( rExport, rProperty, nFlags,
                                         pProperties, nIdx );
}

///////////////////////////////////////////////////////////////////////////////
//
// protected methods
//

/** fills the given attribute list with the items in the given set */
void SvXMLExportPropertyMapper::_exportXML(
        SvXMLAttributeList& rAttrList,
        const ::std::vector< XMLPropertyState >& rProperties,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_uInt16 nFlags,
        SvUShorts* pIndexArray,
        sal_Int32 nPropMapStartIdx, sal_Int32 nPropMapEndIdx ) const
{
    const sal_uInt32 nCount = rProperties.size();
    sal_uInt32 nIndex = 0;

    if( -1 == nPropMapStartIdx )
        nPropMapStartIdx = 0;
    if( -1 == nPropMapEndIdx )
        nPropMapEndIdx = maPropMapper->GetEntryCount();

    while( nIndex < nCount )
    {
        sal_Int32 nPropMapIdx = rProperties[nIndex].mnIndex;
        if( nPropMapIdx >= nPropMapStartIdx &&
            nPropMapIdx < nPropMapEndIdx  )// valid entry?
        {
            // we have a valid map entry here, so lets use it...
            if( ( maPropMapper->GetEntryFlags( nPropMapIdx )
                  & MID_FLAG_ELEMENT_ITEM_EXPORT ) != 0 )
            {
                // element items do not add any properties,
                // we export it later
                if( pIndexArray )
                    pIndexArray->Insert( (sal_uInt16)nIndex, pIndexArray->Count() );
            }
            else
            {
                _exportXML( rAttrList, rProperties[nIndex], rUnitConverter,
                            rNamespaceMap, nFlags, &rProperties, nIndex );
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
        sal_uInt16 nFlags,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx ) const
{
    OUString sCDATA( GetXMLToken(XML_CDATA) );

    if ( ( maPropMapper->GetEntryFlags( rProperty.mnIndex ) &
                MID_FLAG_SPECIAL_ITEM_EXPORT ) != 0 )
    {
        uno::Reference< container::XNameContainer > xAttrContainer;
        if( (rProperty.maValue >>= xAttrContainer) && xAttrContainer.is() )
        {
            SvXMLNamespaceMap *pNewNamespaceMap = 0;
            const SvXMLNamespaceMap *pNamespaceMap = &rNamespaceMap;

            uno::Sequence< OUString > aAttribNames( xAttrContainer->getElementNames() );
            const OUString* pAttribName = aAttribNames.getConstArray();

            const sal_Int32 nCount = aAttribNames.getLength();

            OUStringBuffer sName;
            xml::AttributeData aData;
            for( sal_Int32 i=0; i < nCount; i++, pAttribName++ )
            {
                xAttrContainer->getByName( *pAttribName ) >>= aData;

                // extract namespace prefix from attribute name if it exists
                OUString sPrefix;
                const sal_Int32 nPos = pAttribName->indexOf( sal_Unicode(':') );
                if( nPos != -1 )
                    sPrefix = pAttribName->copy( 0, nPos );

                if( sPrefix.getLength() )
                {
                    OUString sNamespace( aData.Namespace );

                    // if the prefix isn't defined yet or has another meaning,
                    // we have to redefine it now.
                    sal_uInt16 nKey = pNamespaceMap->GetKeyByPrefix( sPrefix );
                    if( USHRT_MAX == nKey || pNamespaceMap->GetNameByKey( nKey ) != sNamespace )
                    {
                        if( !pNewNamespaceMap )
                        {
                            pNewNamespaceMap = new SvXMLNamespaceMap( rNamespaceMap );
                            pNamespaceMap = pNewNamespaceMap;
                        }
                        pNewNamespaceMap->Add( sPrefix, sNamespace );

                        sName.append( GetXMLToken(XML_XMLNS) );
                        sName.append( sal_Unicode(':') );
                        sName.append( sPrefix );
                        rAttrList.AddAttribute( sName.makeStringAndClear(),
                                                sNamespace );
                    }
                }
                OSL_ENSURE(aData.Type == GetXMLToken(XML_CDATA), "different type to our default type which should be written out");
                rAttrList.AddAttribute( *pAttribName, aData.Value );
            }

            delete pNewNamespaceMap;
        }
        else
        {
            handleSpecialItem( rAttrList, rProperty, rUnitConverter,
                               rNamespaceMap, pProperties, nIdx );
        }
    }
    else if ( ( maPropMapper->GetEntryFlags( rProperty.mnIndex ) &
                    MID_FLAG_ELEMENT_ITEM_EXPORT ) == 0 )
    {
        OUString aValue;
        const OUString sName( rNamespaceMap.GetQNameByKey(
                    maPropMapper->GetEntryNameSpace( rProperty.mnIndex ),
                    maPropMapper->GetEntryXMLName( rProperty.mnIndex ) ) );

        sal_Bool bRemove = sal_False;
        if( ( maPropMapper->GetEntryFlags( rProperty.mnIndex ) &
                    MID_FLAG_MERGE_ATTRIBUTE ) != 0 )
        {
            aValue = rAttrList.getValueByName( sName );
            bRemove = sal_True; //aValue.getLength() != 0;
        }

        if( maPropMapper->exportXML( aValue, rProperty, rUnitConverter ) )
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
        sal_uInt16 nFlags,
        const SvUShorts& rIndexArray ) const
{
    const sal_uInt16 nCount = rIndexArray.Count();

    sal_Bool bItemsExported = sal_False;
    OUString sWS( GetXMLToken(XML_WS) );
    for( sal_uInt16 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const sal_uInt16 nElement = rIndexArray.GetObject( nIndex );

        OSL_ENSURE( 0 != ( maPropMapper->GetEntryFlags(
                rProperties[nElement].mnIndex ) & MID_FLAG_ELEMENT_ITEM_EXPORT),
                "wrong mid flag!" );

        rExport.IgnorableWhitespace();
        handleElementItem( rExport, rProperties[nElement],
                           nFlags, &rProperties, nElement );
        bItemsExported = sal_True;
    }

    if( bItemsExported )
        rExport.IgnorableWhitespace();
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
