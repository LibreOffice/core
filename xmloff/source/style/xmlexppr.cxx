/*************************************************************************
 *
 *  $RCSfile: xmlexppr.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: cl $ $Date: 2001-04-03 10:13:07 $
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

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_ATTRIBUTEDATA_HPP_
#include <com/sun/star/xml/AttributeData.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef __SGI_STL_LIST
#include <list>
#endif
#ifndef _CNTRSRT_HXX
#include <svtools/cntnrsrt.hxx>
#endif

#include "xmlexppr.hxx"

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_ATTRLIST_HXX
#include "attrlist.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLCNITM_HXX
#include "xmlcnitm.hxx"
#endif
#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif

#ifndef _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
#endif

using namespace ::rtl;
using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

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

typedef std::list<XMLPropertyState> MyPropertyStateList;

class MyPropertyStates
{
    MyPropertyStateList             aProps;
    MyPropertyStateList::iterator   aLastItr;
    sal_uInt32                      nCount;
public:
    MyPropertyStates();
    void AddPropertyState(const XMLPropertyState& aProp);
    void FillPropertyStateVector(std::vector<XMLPropertyState>& aVector);
};

MyPropertyStates::MyPropertyStates()
    : aProps(),
    nCount(0)
{
    aLastItr = aProps.begin();
}

void MyPropertyStates::AddPropertyState(const XMLPropertyState& aProp)
{
    MyPropertyStateList::iterator aItr = aProps.begin();
    sal_Bool bInserted(sal_False);
    if (nCount)
    {
        if (aLastItr->mnIndex < aProp.mnIndex)
            aItr = ++aLastItr;
    }
    do
    {
        if (aItr == aProps.end())
        {
            aLastItr = aProps.insert(aProps.end(), aProp);
            bInserted = sal_True;
            nCount++;
        }
        else if (aItr->mnIndex > aProp.mnIndex)
        {
            aLastItr = aProps.insert(aItr, aProp);
            bInserted = sal_True;
            nCount++;
        }
    }
    while(!bInserted && (aItr++ != aProps.end()));
}

void MyPropertyStates::FillPropertyStateVector(std::vector<XMLPropertyState>& aVector)
{
    if (nCount)
    {
        aVector.resize(nCount, XMLPropertyState(-1));
        MyPropertyStateList::iterator aItr = aProps.begin();
        sal_uInt32 i (0);
        while (aItr != aProps.end())
        {
            aVector[i] = *aItr;
            aItr++;
            i++;
        }
    }
}

struct FilterPropertyInfo
{
    rtl::OUString           sApiName;
    std::list<sal_uInt32>   aIndexes;
    sal_uInt32              nCount;

    FilterPropertyInfo(const rtl::OUString& sApiName, const sal_uInt32 nIndex);
};

FilterPropertyInfo::FilterPropertyInfo(const rtl::OUString& sApiName, const sal_uInt32 nIndex)
    : aIndexes(),
    nCount(0)
{
    this->sApiName = sApiName;
    this->aIndexes.push_back(nIndex);
    nCount++;
}

typedef std::list<FilterPropertyInfo> FilterPropertyInfoList;

class FilterPropertiesInfo
{
    Sequence < sal_Int8 >               aImplementationId;
    sal_uInt32                          nCount;
    FilterPropertyInfoList              aProps;
    FilterPropertyInfoList::iterator    aLastItr;
public:
    FilterPropertiesInfo();
    FilterPropertiesInfo( const Sequence < sal_Int8 >& rImplId );
    ~FilterPropertiesInfo();
    void setImplementationId( const Sequence < sal_Int8 >& rImplId )
    {
        aImplementationId = rImplId;
        OSL_ENSURE( aImplementationId.getLength()==16, "invalid implementation id" );
    }
    const Sequence < sal_Int8 >& getImplementationId() const
    {
        return aImplementationId;
    }

    void AddProperty(const rtl::OUString& sApiName, const sal_uInt32 nIndex);
    void GetApiNames(uno::Sequence<OUString>& aApiNames);
    void FillPropertyStateArray(vector< XMLPropertyState >& aPropStates, const PropertyState *pStates,
                            const Reference< XPropertySet >& xPropSet,
                            UniReference< XMLPropertySetMapper > maPropMapper, const sal_Bool bDefault = sal_False);
};

int FilterPropertiesInfoCmp_Impl( const FilterPropertiesInfo& r1,
                                  const FilterPropertiesInfo& r2 )
{
    return memcmp( r1.getImplementationId().getConstArray(),
                   r2.getImplementationId().getConstArray(), 16 );
}

DECLARE_CONTAINER_SORT_DEL( FilterPropertiesInfos_Impl,
                         FilterPropertiesInfo );
IMPL_CONTAINER_SORT( FilterPropertiesInfos_Impl,
                          FilterPropertiesInfo,
                          FilterPropertiesInfoCmp_Impl )

FilterPropertiesInfo::FilterPropertiesInfo()
    : aProps(),
    nCount(0)
{
    aLastItr = aProps.begin();
}

FilterPropertiesInfo::FilterPropertiesInfo( const Sequence < sal_Int8 >& rImplId )
    : aProps(),
    aImplementationId( rImplId ),
    nCount(0)
{
    aLastItr = aProps.begin();
    OSL_ENSURE( aImplementationId.getLength()==16, "invalid implementation id" );
}

FilterPropertiesInfo::~FilterPropertiesInfo()
{
}

void FilterPropertiesInfo::AddProperty(const rtl::OUString& sApiName, const sal_uInt32 nIndex)
{
    FilterPropertyInfoList::iterator aItr = aProps.begin();
    sal_Bool bInserted(sal_False);
    if (nCount)
    {
        sal_Int32 nCompResult(aLastItr->sApiName.compareTo(sApiName));
        if (nCompResult == 0)
            aItr = aLastItr;
        else if (nCompResult < 0)
            aItr = ++aLastItr;
    }
    do
    {
        if (aItr == aProps.end())
        {
            aLastItr = aProps.insert(aProps.end(), FilterPropertyInfo(sApiName, nIndex));
            bInserted = sal_True;
            nCount++;
        }
        else if (aItr->sApiName > sApiName)
        {
            aLastItr = aProps.insert(aItr, FilterPropertyInfo(sApiName, nIndex));
            bInserted = sal_True;
            nCount++;
        }
        else if (aItr->sApiName == sApiName)
        {
            aItr->aIndexes.push_back(nIndex);
            aItr->nCount++;
            bInserted = sal_True;
            aLastItr = aItr;
        }
    }
    while(!bInserted && (aItr++ != aProps.end()));
}

void FilterPropertiesInfo::GetApiNames(uno::Sequence<OUString>& aApiNames)
{
    DBG_ASSERT(nCount == aProps.size(), "wrong property count");
    aApiNames.realloc(nCount);
    FilterPropertyInfoList::iterator aItr = aProps.begin();
    for (sal_uInt32 i = 0; i < nCount; i++, aItr++)
        aApiNames[i] = aItr->sApiName;
}

void FilterPropertiesInfo::FillPropertyStateArray(vector< XMLPropertyState >& aPropStates, const PropertyState *pStates,
                                            const Reference< XPropertySet >& xPropSet,
                                            UniReference< XMLPropertySetMapper > maPropMapper, const sal_Bool bDefault)
{
    FilterPropertyInfoList::iterator aItr = aProps.begin();
    MyPropertyStates aPropsList;
    for(sal_uInt32 i = 0; i < nCount; i++, aItr++ )
    {
        std::list<sal_uInt32>::iterator aIndexItr = aItr->aIndexes.begin();
        if( (pStates[i] == PropertyState_DIRECT_VALUE) || (bDefault && ((maPropMapper->GetEntryFlags( *aIndexItr ) &
                    MID_FLAG_DEFAULT_ITEM_EXPORT) != 0)) )
        {
            // The value is stored in the PropertySet itself, add to list.
            XMLPropertyState aNewProperty( *aIndexItr,
                    xPropSet->getPropertyValue( aItr->sApiName ) );
            do
            {
                aNewProperty.mnIndex = *aIndexItr;
                aPropsList.AddPropertyState( aNewProperty );
                aIndexItr++;
            }
            while(aIndexItr != aItr->aIndexes.end());
        }
    }
    aPropsList.FillPropertyStateVector(aPropStates);
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

    if( xInfo.is() )
    {
        Reference< XPropertyState > xPropState( xPropSet, UNO_QUERY );
        sal_Int32 nProps = maPropMapper->GetEntryCount();

        if( xPropState.is() )
        {
            FilterPropertiesInfo aFilterInfo;
            sal_Bool bInfoValid = sal_False;

            FilterPropertiesInfo *pFilterInfo = 0;

/* temporary disabled this optimization
            Reference < XTypeProvider > xTypeProv( xPropSet, UNO_QUERY );
            if( xTypeProv.is() )
            {
                Sequence< sal_Int8  > aImplId( xTypeProv->getImplementationId() );
                if( aImplId.getLength() == 16 )
                {
                    if( pCache )
                    {
                        aFilterInfo.setImplementationId( aImplId );
                        sal_uInt32 nPos = 0;
                        if( pCache->Seek_Entry( &aFilterInfo, &nPos ) )
                        {
                            pFilterInfo = pCache->GetObject( nPos );
                            bInfoValid = sal_True;
                        }
                    }
                    if( !pFilterInfo )
                    {
                        pFilterInfo = new FilterPropertiesInfo( aImplId );
                        if( !pCache )
                            ((SvXMLExportPropertyMapper *)this)->pCache =
                                new FilterPropertiesInfos_Impl( 10, 5 );
                        pCache->Insert( pFilterInfo );
                    }
                }
            }
*/
            if( !pFilterInfo )
                pFilterInfo = &aFilterInfo;

            if( !bInfoValid )
            {
                for( sal_Int32 i=0; i < nProps; i++ )
                {
                    // Are we allowed to ask for the property? (MID_FLAG_NO_PROP..)
                    // Does the PropertySet contain name of mpEntries-array ?
                    const OUString& rAPIName = maPropMapper->GetEntryAPIName( i );
                    const sal_Int32 nFlags = maPropMapper->GetEntryFlags( i );
                    if( (0 == (nFlags & MID_FLAG_NO_PROPERTY_EXPORT)) &&
                        xInfo->hasPropertyByName( rAPIName ) )
                        pFilterInfo->AddProperty(rAPIName, i);
                }
            }

            uno::Sequence<OUString> aApiNames;
            pFilterInfo->GetApiNames(aApiNames);

            Sequence < PropertyState > aStates =
                xPropState->getPropertyStates( aApiNames );

            const PropertyState *pStates = aStates.getArray();
            pFilterInfo->FillPropertyStateArray(aPropStateArray, pStates, xPropSet, maPropMapper, bDefault);
        }
        else
        {
            for( sal_Int32 i=0; i < nProps; i++ )
            {
                // Are we allowed to ask for the property? (MID_FLAG_NO_PROP..)
                // Does the PropertySet contain name of mpEntries-array ?
                const OUString& rAPIName = maPropMapper->GetEntryAPIName( i );
                const sal_Int32 nFlags = maPropMapper->GetEntryFlags( i );
                if( (0 == (nFlags & MID_FLAG_NO_PROPERTY_EXPORT)) &&
                    xInfo->hasPropertyByName( rAPIName ) )
                {
                    // If there isn't a XPropertyState we can't filter by its state
                    if( !xPropState.is() ||
                        xPropState->getPropertyState( rAPIName ) ==
                                PropertyState_DIRECT_VALUE ||
                        (bDefault && ((maPropMapper->GetEntryFlags( i ) &
                    MID_FLAG_DEFAULT_ITEM_EXPORT) != 0) ))
                    {
                        // The value is stored in the PropertySet itself, add to list.
                        XMLPropertyState aNewProperty( i,
                            xPropSet->getPropertyValue( rAPIName ) );
                        aPropStateArray.push_back( aNewProperty );
                    }
                }
            }
        }

        // Call centext-filter
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
    }

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
//  1.Number of elements equal ?
//  2.Index of each element equal ? (So I know whether the propertynames are the same)
//  3.Value of each element equal ?
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


/** fills the given attribute list with the items in the given set */
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


void SvXMLExportPropertyMapper::exportXML( SvXMLAttributeList& rAttrList,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_uInt16 nFlags ) const
{
    if( ( maPropMapper->GetEntryFlags( rProperty.mnIndex ) &
                MID_FLAG_ELEMENT_ITEM_EXPORT ) == 0 )
        _exportXML( rAttrList, rProperty, rUnitConverter, rNamespaceMap,
                    nFlags );
}

void SvXMLExportPropertyMapper::exportXML(
           const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
        const ::std::vector< XMLPropertyState >& rProperties,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_uInt16 nFlags ) const
{
    exportXML( rHandler, rProperties, rUnitConverter, rNamespaceMap,
               -1, -1,  nFlags );
}

void SvXMLExportPropertyMapper::exportXML(
           const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
        const ::std::vector< XMLPropertyState >& rProperties,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_Int32 nPropMapStartIdx, sal_Int32 nPropMapEndIdx,
        sal_uInt16 nFlags ) const
{
    SvXMLAttributeList *pAttrList = new SvXMLAttributeList();
    uno::Reference< xml::sax::XAttributeList > xAttrList( pAttrList );

    SvUShorts aIndexArray;

    _exportXML( *pAttrList, rProperties, rUnitConverter, rNamespaceMap,
                nFlags, &aIndexArray, nPropMapStartIdx, nPropMapEndIdx );

    if( pAttrList->getLength() > 0L || (nFlags & XML_EXPORT_FLAG_EMPTY) != 0 ||
        aIndexArray.Count() != 0 )
    {
        if( (nFlags & XML_EXPORT_FLAG_IGN_WS) != 0 )
        {
            OUString sWS( OUString::createFromAscii(sXML_WS) );
            rHandler->ignorableWhitespace( sWS );
        }

        OUString sLName( OUString::createFromAscii(sXML_properties) );
        OUString sName = rNamespaceMap.GetQNameByKey( XML_NAMESPACE_STYLE,
                                                      sLName );
        rHandler->startElement( sName, xAttrList );

        exportElementItems( rHandler, rUnitConverter, rNamespaceMap,
                            rProperties, nFlags, aIndexArray );

        rHandler->endElement( sName );
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
    DBG_ASSERT( mxNextMapper.is(), "special item not handled in xml export" );
    if( mxNextMapper.is() )
        mxNextMapper->handleSpecialItem( rAttrList, rProperty, rUnitConverter,
                                        rNamespaceMap, pProperties, nIdx );
}

/** this method is called for every item that has the
    MID_FLAG_ELEMENT_EXPORT flag set */
void SvXMLExportPropertyMapper::handleElementItem(
        const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
        const XMLPropertyState& rProperty,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_uInt16 nFlags,
        const ::std::vector< XMLPropertyState > *pProperties,
        sal_uInt32 nIdx ) const
{
    DBG_ASSERT( mxNextMapper.is(), "element item not handled in xml export" );
    if( mxNextMapper.is() )
        mxNextMapper->handleElementItem( rHandler, rProperty, rUnitConverter,
                                           rNamespaceMap, nFlags, pProperties,
                                        nIdx );
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
                    pIndexArray->Insert( nIndex, pIndexArray->Count() );
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
    OUString sCDATA( OUString::createFromAscii( sXML_CDATA ) );

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
                    sal_uInt16 nIdx = pNamespaceMap->GetIndexByPrefix( sPrefix );
                    if( USHRT_MAX == nIdx || pNamespaceMap->GetNameByIndex( nIdx ) != sNamespace )
                    {
                        if( !pNewNamespaceMap )
                        {
                            pNewNamespaceMap = new SvXMLNamespaceMap( rNamespaceMap );
                            pNamespaceMap = pNewNamespaceMap;
                        }
                        pNewNamespaceMap->Add( sPrefix, sNamespace );

                        sName.appendAscii( sXML_xmlns );
                        sName.append( sal_Unicode(':') );
                        sName.append( sPrefix );
                        rAttrList.AddAttribute( sName.makeStringAndClear(), sCDATA,
                                                sNamespace );
                    }
                }

                rAttrList.AddAttribute( *pAttribName, aData.Type, aData.Value );
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
            rAttrList.AddAttribute( sName, sCDATA, aValue );
        }
    }
}

void SvXMLExportPropertyMapper::exportElementItems(
        const uno::Reference< xml::sax::XDocumentHandler > & rHandler,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        const ::std::vector< XMLPropertyState >& rProperties,
        sal_uInt16 nFlags,
        const SvUShorts& rIndexArray ) const
{
    const sal_uInt16 nCount = rIndexArray.Count();

    sal_Bool bItemsExported = sal_False;
    OUString sWS( OUString::createFromAscii(sXML_WS) );
    for( sal_uInt16 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const sal_uInt16 nElement = rIndexArray.GetObject( nIndex );

        DBG_ASSERT( 0 != ( maPropMapper->GetEntryFlags(
                rProperties[nElement].mnIndex ) & MID_FLAG_ELEMENT_ITEM_EXPORT),
                "wrong mid flag!" );

        rHandler->ignorableWhitespace( sWS );
        handleElementItem( rHandler, rProperties[nElement], rUnitConverter,
                           rNamespaceMap, nFlags, &rProperties, nElement );
        bItemsExported = sal_True;
    }

    if( bItemsExported )
        rHandler->ignorableWhitespace( sWS );
}
