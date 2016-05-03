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

#include <com/sun/star/xml/AttributeData.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <com/sun/star/beans/TolerantPropertySetResultType.hpp>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <xmloff/xmlprmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlimp.hxx>

#include <xmloff/unoatrcn.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/contextid.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/maptype.hxx>

#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;

using namespace ::std;
using namespace ::xmloff::token;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::lang::WrappedTargetException;
using ::com::sun::star::beans::UnknownPropertyException;
using ::com::sun::star::beans::PropertyVetoException;


SvXMLImportPropertyMapper::SvXMLImportPropertyMapper(
        const rtl::Reference< XMLPropertySetMapper >& rMapper,
        SvXMLImport& rImp ):
    rImport(rImp),
    maPropMapper  ( rMapper )
{
}

SvXMLImportPropertyMapper::~SvXMLImportPropertyMapper()
{
    mxNextMapper = nullptr;
}

void SvXMLImportPropertyMapper::ChainImportMapper(
        const rtl::Reference< SvXMLImportPropertyMapper>& rMapper )
{
    // add map entries from rMapper to current map
    maPropMapper->AddMapperEntry( rMapper->getPropertySetMapper() );
    // rMapper uses the same map as 'this'
    rMapper->maPropMapper = maPropMapper;

    // set rMapper as last mapper in current chain
    rtl::Reference< SvXMLImportPropertyMapper > xNext = mxNextMapper;
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

/** fills the given itemset with the attributes in the given list */
void SvXMLImportPropertyMapper::importXML(
        vector< XMLPropertyState >& rProperties,
           Reference< XAttributeList > xAttrList,
           const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_uInt32 nPropType,
        sal_Int32 nStartIdx,
        sal_Int32 nEndIdx ) const
{
    sal_Int16 nAttr = xAttrList->getLength();

    Reference< XNameContainer > xAttrContainer;

    if( -1 == nStartIdx )
        nStartIdx = 0;
    if( -1 == nEndIdx )
        nEndIdx = maPropMapper->GetEntryCount();
    for( sal_Int16 i=0; i < nAttr; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName, aPrefix, aNamespace;
        sal_uInt16 nPrefix = rNamespaceMap.GetKeyByAttrName( rAttrName, &aPrefix,
                                                    &aLocalName, &aNamespace );

        if( XML_NAMESPACE_XMLNS == nPrefix )
            continue;

        const OUString& rValue = xAttrList->getValueByIndex( i );

        // index of actual property map entry
        // This looks very strange, but it works well:
        // If the start index is 0, the new value will become -1, and
        // GetEntryIndex will start searching with position 0.
        // Otherwise GetEntryIndex will start with the next position specified.
        sal_Int32 nIndex =  nStartIdx - 1;
        sal_uInt32 nFlags = 0;  // flags of actual property map entry
        bool bFound = false;

        // for better error reporting: this should be set true if no
        // warning is needed
        bool bNoWarning = false;
        bool bAlienImport = false;

        do
        {
            // find an entry for this attribute
            nIndex = maPropMapper->GetEntryIndex( nPrefix, aLocalName,
                                                  nPropType, nIndex );

            if( nIndex > -1 && nIndex < nEndIdx  )
            {
                // create a XMLPropertyState with an empty value

                nFlags = maPropMapper->GetEntryFlags( nIndex );
                if( (( nFlags & MID_FLAG_NO_PROPERTY ) == MID_FLAG_NO_PROPERTY) && (maPropMapper->GetEntryContextId( nIndex ) == CTF_ALIEN_ATTRIBUTE_IMPORT) )
                {
                    bAlienImport = true;
                    nIndex = -1;
                }
                else
                {
                    if( ( nFlags & MID_FLAG_ELEMENT_ITEM_IMPORT ) == 0 )
                    {
                        XMLPropertyState aNewProperty( nIndex );
                        sal_Int32 nReference = -1;

                        // if this is a multi attribute check if another attribute already set
                        // this any. If so use this as a initial value
                        if( ( nFlags & MID_FLAG_MERGE_PROPERTY ) != 0 )
                        {
                            const OUString aAPIName( maPropMapper->GetEntryAPIName( nIndex ) );
                            const sal_Int32 nSize = rProperties.size();
                            for( nReference = 0; nReference < nSize; nReference++ )
                            {
                                sal_Int32 nRefIdx = rProperties[nReference].mnIndex;
                                if( (nRefIdx != -1) && (nIndex != nRefIdx) &&
                                    (maPropMapper->GetEntryAPIName( nRefIdx ) == aAPIName ))
                                {
                                    aNewProperty = rProperties[nReference];
                                    aNewProperty.mnIndex = nIndex;
                                    break;
                                }
                            }

                            if( nReference == nSize )
                                nReference = -1;
                        }

                        bool bSet = false;
                        if( ( nFlags & MID_FLAG_SPECIAL_ITEM_IMPORT ) == 0 )
                        {
                            // let the XMLPropertySetMapper decide how to import the value
                            bSet = maPropMapper->importXML( rValue, aNewProperty,
                                                     rUnitConverter );
                        }
                        else
                        {
                            sal_uInt32 nOldSize = rProperties.size();

                            bSet = handleSpecialItem( aNewProperty, rProperties,
                                                      rValue, rUnitConverter,
                                                         rNamespaceMap );

                            // no warning if handleSpecialItem added properties
                            bNoWarning |= ( nOldSize != rProperties.size() );
                        }

                        // no warning if we found could set the item. This
                        // 'remembers' bSet across multi properties.
                        bNoWarning |= bSet;

                        // store the property in the given vector
                        if( bSet )
                        {
                            if( nReference == -1 )
                                rProperties.push_back( aNewProperty );
                            else
                                rProperties[nReference] = aNewProperty;
                        }
                        else
                        {
                            // warn about unknown value. Unless it's a
                            // multi property: Then we get another chance
                            // to set the value.
                            if( !bNoWarning &&
                                ((nFlags & MID_FLAG_MULTI_PROPERTY) == 0) )
                            {
                                Sequence<OUString> aSeq(2);
                                aSeq[0] = rAttrName;
                                aSeq[1] = rValue;
                                rImport.SetError( XMLERROR_FLAG_WARNING |
                                                  XMLERROR_STYLE_ATTR_VALUE,
                                                  aSeq );
                            }
                        }
                    }
                    bFound = true;
                    continue;
                }
            }

            if( !bFound )
            {
                SAL_INFO_IF((XML_NAMESPACE_NONE != nPrefix) &&
                            !(XML_NAMESPACE_UNKNOWN_FLAG & nPrefix) &&
                            !bAlienImport, "xmloff.style",
                            "unknown attribute: \"" << rAttrName << "\"");
                if( (XML_NAMESPACE_UNKNOWN_FLAG & nPrefix) || (XML_NAMESPACE_NONE == nPrefix) || bAlienImport )
                {
                    if( !xAttrContainer.is() )
                    {
                        // add an unknown attribute container to the properties
                        Reference< XNameContainer > xNew( SvUnoAttributeContainer_CreateInstance(), UNO_QUERY );
                        xAttrContainer = xNew;

                        // find map entry and create new property state
                        if( -1 == nIndex )
                        {
                            switch( nPropType )
                            {
                                case XML_TYPE_PROP_CHART:
                                    nIndex = maPropMapper->FindEntryIndex( "ChartUserDefinedAttributes", XML_NAMESPACE_TEXT, GetXMLToken(XML_XMLNS) );
                                    break;
                                case XML_TYPE_PROP_PARAGRAPH:
                                    nIndex = maPropMapper->FindEntryIndex( "ParaUserDefinedAttributes", XML_NAMESPACE_TEXT, GetXMLToken(XML_XMLNS) );
                                    break;
                                case  XML_TYPE_PROP_TEXT:
                                    nIndex = maPropMapper->FindEntryIndex( "TextUserDefinedAttributes", XML_NAMESPACE_TEXT, GetXMLToken(XML_XMLNS) );
                                    break;
                                default:
                                    break;
                            }
                            // other property type or property not found
                            if( -1 == nIndex )
                                nIndex = maPropMapper->FindEntryIndex( "UserDefinedAttributes", XML_NAMESPACE_TEXT, GetXMLToken(XML_XMLNS) );
                        }

                        // #106963#; use userdefined attribute only if it is in the specified property range
                        if( nIndex != -1 && nIndex >= nStartIdx && nIndex < nEndIdx)
                        {
                            XMLPropertyState aNewProperty( nIndex, Any(xAttrContainer) );

                            // push it on our stack so we export it later
                            rProperties.push_back( aNewProperty );
                        }
                    }

                    if( xAttrContainer.is() )
                    {
                        AttributeData aData;
                        aData.Type = GetXMLToken( XML_CDATA );
                        aData.Value = rValue;

                        OUStringBuffer sName;
                        if( XML_NAMESPACE_NONE != nPrefix )
                        {
                            sName.append( aPrefix );
                            sName.append( ':' );
                            aData.Namespace = aNamespace;
                        }

                        sName.append( aLocalName );

                        xAttrContainer->insertByName( sName.makeStringAndClear(), Any(aData) );
                    }
                }
            }
        }
        while( ( nIndex >= 0 && nIndex + 1 < nEndIdx ) && (( nFlags & MID_FLAG_MULTI_PROPERTY ) != 0 ) );
    }

    finished( rProperties, nStartIdx, nEndIdx );
}

/** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_IMPORT flag set */
bool SvXMLImportPropertyMapper::handleSpecialItem(
        XMLPropertyState& rProperty,
        vector< XMLPropertyState >& rProperties,
        const OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    OSL_ENSURE( mxNextMapper.is(), "unsupported special item in xml import" );
    if( mxNextMapper.is() )
        return mxNextMapper->handleSpecialItem( rProperty, rProperties, rValue,
                                               rUnitConverter, rNamespaceMap );
    else
        return false;
}

void SvXMLImportPropertyMapper::FillPropertySequence(
            const ::std::vector< XMLPropertyState >& rProperties,
            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rValues )
            const
{
    sal_Int32 nCount = rProperties.size();
    sal_Int32 nValueCount = 0;
    rValues.realloc( nCount );
    PropertyValue *pProps = rValues.getArray();
    for( sal_Int32 i=0; i < nCount; i++ )
    {
        const XMLPropertyState& rProp = rProperties[i];
        sal_Int32 nIdx = rProp.mnIndex;
        if( nIdx == -1 )
            continue;
        pProps->Name = maPropMapper->GetEntryAPIName( nIdx );
        if( !pProps->Name.isEmpty() )
        {
            pProps->Value <<= rProp.maValue;
            ++pProps;
            ++nValueCount;
        }
    }
    if( nValueCount < nCount )
        rValues.realloc( nValueCount );
}

void SvXMLImportPropertyMapper::CheckSpecialContext(
            const ::std::vector< XMLPropertyState >& aProperties,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet >& rPropSet,
            _ContextID_Index_Pair* pSpecialContextIds ) const
{
    OSL_ENSURE( rPropSet.is(), "need an XPropertySet" );
    sal_Int32 nCount = aProperties.size();

    Reference< XPropertySetInfo > xInfo(rPropSet->getPropertySetInfo());

    for( sal_Int32 i=0; i < nCount; i++ )
    {
        const XMLPropertyState& rProp = aProperties[i];
        sal_Int32 nIdx = rProp.mnIndex;

        // disregard property state if it has an invalid index
        if( -1 == nIdx )
            continue;

        const sal_Int32 nPropFlags = maPropMapper->GetEntryFlags( nIdx );

        // handle no-property and special items
        if( ( pSpecialContextIds != nullptr ) &&
            ( ( 0 != ( nPropFlags & MID_FLAG_NO_PROPERTY_IMPORT ) ) ||
              ( 0 != ( nPropFlags & MID_FLAG_SPECIAL_ITEM_IMPORT ) )   ) )
        {
            // maybe it's one of our special context ids?
            sal_Int16 nContextId = maPropMapper->GetEntryContextId(nIdx);

            for ( sal_Int32 n = 0;
                  pSpecialContextIds[n].nContextID != -1;
                  n++ )
            {
                // found: set index in pSpecialContextIds array
                if ( pSpecialContextIds[n].nContextID == nContextId )
                {
                    pSpecialContextIds[n].nIndex = i;
                    break; // early out
                }
            }
        }
    }

}

bool SvXMLImportPropertyMapper::FillPropertySet(
            const vector< XMLPropertyState >& aProperties,
            const Reference< XPropertySet >& rPropSet,
            _ContextID_Index_Pair* pSpecialContextIds ) const
{
    bool bSet = false;

    Reference< XTolerantMultiPropertySet > xTolPropSet( rPropSet, UNO_QUERY );
    if (xTolPropSet.is())
        bSet = _FillTolerantMultiPropertySet( aProperties, xTolPropSet, maPropMapper, rImport,
                                            pSpecialContextIds );

    if (!bSet)
    {
        // get property set info
        Reference< XPropertySetInfo > xInfo(rPropSet->getPropertySetInfo());

        // check for multi-property set
        Reference<XMultiPropertySet> xMultiPropSet( rPropSet, UNO_QUERY );
        if ( xMultiPropSet.is() )
        {
            // Try XMultiPropertySet. If that fails, try the regular route.
            bSet = _FillMultiPropertySet( aProperties, xMultiPropSet,
                                        xInfo, maPropMapper,
                                        pSpecialContextIds );
            if ( !bSet )
                bSet = _FillPropertySet( aProperties, rPropSet,
                                        xInfo, maPropMapper, rImport,
                                        pSpecialContextIds);
        }
        else
            bSet = _FillPropertySet( aProperties, rPropSet, xInfo,
                                    maPropMapper, rImport,
                                    pSpecialContextIds );
    }

    return bSet;
}

bool SvXMLImportPropertyMapper::_FillPropertySet(
    const vector<XMLPropertyState> & rProperties,
    const Reference<XPropertySet> & rPropSet,
    const Reference<XPropertySetInfo> & rPropSetInfo,
    const rtl::Reference<XMLPropertySetMapper> & rPropMapper,
    SvXMLImport& rImport,
    _ContextID_Index_Pair* pSpecialContextIds )
{
    OSL_ENSURE( rPropSet.is(), "need an XPropertySet" );
    OSL_ENSURE( rPropSetInfo.is(), "need an XPropertySetInfo" );

    // preliminaries
    bool bSet = false;
    sal_Int32 nCount = rProperties.size();

    // iterate over property states that we want to set
    for( sal_Int32 i=0; i < nCount; i++ )
    {
        const XMLPropertyState& rProp = rProperties[i];
        sal_Int32 nIdx = rProp.mnIndex;

        // disregard property state if it has an invalid index
        if( -1 == nIdx )
            continue;

        const OUString& rPropName = rPropMapper->GetEntryAPIName( nIdx );
        const sal_Int32 nPropFlags = rPropMapper->GetEntryFlags( nIdx );

        if ( ( 0 == ( nPropFlags & MID_FLAG_NO_PROPERTY ) ) &&
             ( ( 0 != ( nPropFlags & MID_FLAG_MUST_EXIST ) ) ||
               rPropSetInfo->hasPropertyByName( rPropName ) )    )
        {
            // try setting the property
            try
            {
                rPropSet->setPropertyValue( rPropName, rProp.maValue );
                bSet = true;
            }
            catch ( const IllegalArgumentException& e )
            {
                // illegal value: check whether this property is
                // allowed to throw this exception
                if ( 0 == ( nPropFlags & MID_FLAG_PROPERTY_MAY_THROW ) )
                {
                    Sequence<OUString> aSeq { rPropName };
                    rImport.SetError(
                        XMLERROR_STYLE_PROP_VALUE | XMLERROR_FLAG_ERROR,
                        aSeq, e.Message, nullptr );
                }
            }
            catch ( const UnknownPropertyException& e )
            {
                // unknown property: This is always an error!
                Sequence<OUString> aSeq { rPropName };
                rImport.SetError(
                    XMLERROR_STYLE_PROP_UNKNOWN | XMLERROR_FLAG_ERROR,
                    aSeq, e.Message, nullptr );
            }
            catch ( const PropertyVetoException& e )
            {
                // property veto: this shouldn't happen
                Sequence<OUString> aSeq { rPropName };
                rImport.SetError(
                    XMLERROR_STYLE_PROP_OTHER | XMLERROR_FLAG_ERROR,
                    aSeq, e.Message, nullptr );
            }
            catch ( const WrappedTargetException& e )
            {
                // wrapped target: this shouldn't happen either
                Sequence<OUString> aSeq { rPropName };
                rImport.SetError(
                    XMLERROR_STYLE_PROP_OTHER | XMLERROR_FLAG_ERROR,
                    aSeq, e.Message, nullptr );
            }
        }

        // handle no-property and special items
        if( ( pSpecialContextIds != nullptr ) &&
            ( ( 0 != ( nPropFlags & MID_FLAG_NO_PROPERTY_IMPORT ) ) ||
              ( 0 != ( nPropFlags & MID_FLAG_SPECIAL_ITEM_IMPORT ) )   ) )
        {
            // maybe it's one of our special context ids?
            sal_Int16 nContextId = rPropMapper->GetEntryContextId(nIdx);

            for ( sal_Int32 n = 0;
                  pSpecialContextIds[n].nContextID != -1;
                  n++ )
            {
                // found: set index in pSpecialContextIds array
                if ( pSpecialContextIds[n].nContextID == nContextId )
                {
                    pSpecialContextIds[n].nIndex = i;
                    break; // early out
                }
            }
        }
    }

    return bSet;
}


typedef pair<const OUString*, const Any* > PropertyPair;
typedef vector<PropertyPair> PropertyPairs;

struct PropertyPairLessFunctor :
    public std::binary_function<PropertyPair, PropertyPair, bool>
{
    bool operator()( const PropertyPair& a, const PropertyPair& b ) const
    {
        return (*a.first < *b.first);
    }
};

void SvXMLImportPropertyMapper::_PrepareForMultiPropertySet(
    const vector<XMLPropertyState> & rProperties,
    const Reference<XPropertySetInfo> & rPropSetInfo,
    const rtl::Reference<XMLPropertySetMapper> & rPropMapper,
    _ContextID_Index_Pair* pSpecialContextIds,
    Sequence<OUString>& rNames,
    Sequence<Any>& rValues)
{
    sal_Int32 nCount = rProperties.size();

    // property pairs structure stores names + values of properties to be set.
    PropertyPairs aPropertyPairs;
    aPropertyPairs.reserve( nCount );

    // iterate over property states that we want to set
    sal_Int32 i;
    for( i = 0; i < nCount; i++ )
    {
        const XMLPropertyState& rProp = rProperties[i];
        sal_Int32 nIdx = rProp.mnIndex;

        // disregard property state if it has an invalid index
        if( -1 == nIdx )
            continue;

        const OUString& rPropName = rPropMapper->GetEntryAPIName( nIdx );
        const sal_Int32 nPropFlags = rPropMapper->GetEntryFlags( nIdx );

        if ( ( 0 == ( nPropFlags & MID_FLAG_NO_PROPERTY ) ) &&
             ( ( 0 != ( nPropFlags & MID_FLAG_MUST_EXIST ) ) ||
               !rPropSetInfo.is() ||
               (rPropSetInfo.is() && rPropSetInfo->hasPropertyByName( rPropName )) ) )
        {
            // save property into property pair structure
            aPropertyPairs.push_back( PropertyPair( &rPropName, &rProp.maValue ) );
        }

        // handle no-property and special items
        if( ( pSpecialContextIds != nullptr ) &&
            ( ( 0 != ( nPropFlags & MID_FLAG_NO_PROPERTY_IMPORT ) ) ||
              ( 0 != ( nPropFlags & MID_FLAG_SPECIAL_ITEM_IMPORT ) )   ) )
        {
            // maybe it's one of our special context ids?
            sal_Int16 nContextId = rPropMapper->GetEntryContextId(nIdx);
            for ( sal_Int32 n = 0;
                  pSpecialContextIds[n].nContextID != -1;
                  n++ )
            {
                // found: set index in pSpecialContextIds array
                if ( pSpecialContextIds[n].nContextID == nContextId )
                {
                    pSpecialContextIds[n].nIndex = i;
                    break; // early out
                }
            }
        }
    }

    // We now need to construct the sequences and actually the set
    // values.

    // sort the property pairs
    sort( aPropertyPairs.begin(), aPropertyPairs.end(),
          PropertyPairLessFunctor());

    // create sequences
    rNames.realloc( aPropertyPairs.size() );
    OUString* pNamesArray = rNames.getArray();
    rValues.realloc( aPropertyPairs.size() );
    Any* pValuesArray = rValues.getArray();

    // copy values into sequences
    i = 0;
    for( PropertyPairs::iterator aIter = aPropertyPairs.begin();
         aIter != aPropertyPairs.end();
         ++aIter )
    {
        pNamesArray[i] = *(aIter->first);
        pValuesArray[i++] = *(aIter->second);
    }
}

bool SvXMLImportPropertyMapper::_FillMultiPropertySet(
    const vector<XMLPropertyState> & rProperties,
    const Reference<XMultiPropertySet> & rMultiPropSet,
    const Reference<XPropertySetInfo> & rPropSetInfo,
    const rtl::Reference<XMLPropertySetMapper> & rPropMapper,
    _ContextID_Index_Pair* pSpecialContextIds )
{
    OSL_ENSURE( rMultiPropSet.is(), "Need multi property set. ");
    OSL_ENSURE( rPropSetInfo.is(), "Need property set info." );

    bool bSuccessful = false;

    Sequence<OUString> aNames;
    Sequence<Any> aValues;

    _PrepareForMultiPropertySet(rProperties, rPropSetInfo, rPropMapper, pSpecialContextIds,
        aNames, aValues);

    // and, finally, try to set the values
    try
    {
        rMultiPropSet->setPropertyValues( aNames, aValues );
        bSuccessful = true;
    }
    catch ( ... )
    {
        OSL_ENSURE(bSuccessful, "Exception caught; style may not be imported correctly.");
    }

    return bSuccessful;
}

bool SvXMLImportPropertyMapper::_FillTolerantMultiPropertySet(
    const vector<XMLPropertyState> & rProperties,
    const Reference<XTolerantMultiPropertySet> & rTolMultiPropSet,
    const rtl::Reference<XMLPropertySetMapper> & rPropMapper,
    SvXMLImport& rImport,
    _ContextID_Index_Pair* pSpecialContextIds )
{
    OSL_ENSURE( rTolMultiPropSet.is(), "Need tolerant multi property set. ");

    bool bSuccessful = false;

    Sequence<OUString> aNames;
    Sequence<Any> aValues;

    _PrepareForMultiPropertySet(rProperties, Reference<XPropertySetInfo>(nullptr), rPropMapper, pSpecialContextIds,
        aNames, aValues);

    // and, finally, try to set the values
    try
    {
        Sequence< SetPropertyTolerantFailed > aResults(rTolMultiPropSet->setPropertyValuesTolerant( aNames, aValues ));
        if (aResults.getLength() == 0)
            bSuccessful = true;
        else
        {
            sal_Int32 nCount(aResults.getLength());
            for( sal_Int32 i = 0; i < nCount; ++i)
            {
                Sequence<OUString> aSeq { aResults[i].Name };
                OUString sMessage;
                switch (aResults[i].Result)
                {
                case TolerantPropertySetResultType::UNKNOWN_PROPERTY :
                    sMessage = "UNKNOWN_PROPERTY";
                    break;
                case TolerantPropertySetResultType::ILLEGAL_ARGUMENT :
                    sMessage = "ILLEGAL_ARGUMENT";
                    break;
                case TolerantPropertySetResultType::PROPERTY_VETO :
                    sMessage = "PROPERTY_VETO";
                    break;
                case TolerantPropertySetResultType::WRAPPED_TARGET :
                    sMessage = "WRAPPED_TARGET";
                    break;
                };
                rImport.SetError(
                    XMLERROR_STYLE_PROP_OTHER | XMLERROR_FLAG_ERROR,
                    aSeq, sMessage, nullptr );
            }
        }
    }
    catch ( ... )
    {
        OSL_ENSURE(bSuccessful, "Exception caught; style may not be imported correctly.");
    }

    return bSuccessful;
}

void SvXMLImportPropertyMapper::finished(
        vector< XMLPropertyState >& rProperties,
        sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const
{
    // nothing to do here
    if( mxNextMapper.is() )
        mxNextMapper->finished( rProperties, nStartIndex, nEndIndex );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
