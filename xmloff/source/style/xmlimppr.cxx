/*************************************************************************
 *
 *  $RCSfile: xmlimppr.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:27:03 $
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

#ifndef _COM_SUN_STAR_XML_ATTRIBUTEDATA_HPP_
#include <com/sun/star/xml/AttributeData.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_WRAPPEDTARGETEXCEPTION_HPP_
#include <com/sun/star/lang/WrappedTargetException.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_UNKNOWNPROPERTYEXCEPTION_HPP_
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVETOEXCEPTION_HPP_
#include <com/sun/star/beans/PropertyVetoException.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include "xmlprmap.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLIMPPR_HXX
#include "xmlimppr.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#include "xmlkywd.hxx"
#include "unoatrcn.hxx"
#include "xmlnmspe.hxx"

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif
#ifndef _XMLOFF_XMLERROR_HXX
#include "xmlerror.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

// STL includes
#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::rtl;
using namespace ::std;
using namespace ::xmloff::token;
using ::com::sun::star::lang::IllegalArgumentException;
using ::com::sun::star::lang::WrappedTargetException;
using ::com::sun::star::beans::UnknownPropertyException;
using ::com::sun::star::beans::PropertyVetoException;


SvXMLImportPropertyMapper::SvXMLImportPropertyMapper(
        const UniReference< XMLPropertySetMapper >& rMapper,
        SvXMLImport& rImp ):
    maPropMapper  ( rMapper ),
    rImport(rImp)
{
}

SvXMLImportPropertyMapper::~SvXMLImportPropertyMapper()
{
    mxNextMapper = 0;
}

void SvXMLImportPropertyMapper::ChainImportMapper(
        const UniReference< SvXMLImportPropertyMapper>& rMapper )
{
    // add map entries from rMapper to current map
    maPropMapper->AddMapperEntry( rMapper->getPropertySetMapper() );
    // rMapper uses the same map as 'this'
    rMapper->maPropMapper = maPropMapper;

    // set rMapper as last mapper in current chain
    UniReference< SvXMLImportPropertyMapper > xNext = mxNextMapper;
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

void SvXMLImportPropertyMapper::importXML(
        vector< XMLPropertyState >& rProperties,
           Reference< XAttributeList > xAttrList,
           const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        sal_uInt32 nPropType ) const
{
    importXML( rProperties, xAttrList, rUnitConverter, rNamespaceMap,
               nPropType,-1, -1 );
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
    INT16 nAttr = xAttrList->getLength();

    Reference< XNameContainer > xAttrContainer;

    if( -1 == nStartIdx )
        nStartIdx = 0;
    if( -1 == nEndIdx )
        nEndIdx = maPropMapper->GetEntryCount();
    for( INT16 i=0; i < nAttr; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName, aPrefix, aNamespace;
        USHORT nPrefix = rNamespaceMap.GetKeyByAttrName( rAttrName, &aPrefix,
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
        sal_Bool bFound = sal_False;

        // for better error reporting: this should be set true if no
        // warning is needed
        sal_Bool bNoWarning = sal_False;

        do
        {
            // find an entry for this attribute
            nIndex = maPropMapper->GetEntryIndex( nPrefix, aLocalName,
                                                  nPropType, nIndex );

            if( nIndex > -1 && nIndex < nEndIdx  )
            {
                // create a XMLPropertyState with an empty value

                nFlags = maPropMapper->GetEntryFlags( nIndex );
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

                    sal_Bool bSet = sal_False;
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
                bFound = sal_True;
            }
            else if( !bFound )
            {
                if( (XML_NAMESPACE_UNKNOWN_FLAG & nPrefix) || (XML_NAMESPACE_NONE == nPrefix) )
                {
                    OSL_ENSURE( XML_NAMESPACE_NONE == nPrefix ||
                                (XML_NAMESPACE_UNKNOWN_FLAG & nPrefix),
                                "unknown attribute - might be a new feature?" );
                    if( !xAttrContainer.is() )
                    {
                        // add an unknown attribute container to the properties
                        Reference< XNameContainer > xNew( SvUnoAttributeContainer_CreateInstance(), UNO_QUERY );
                        xAttrContainer = xNew;

                        // find map entry and create new property state
                        nIndex = maPropMapper->FindEntryIndex( "UserDefinedAttributes", XML_NAMESPACE_TEXT, GetXMLToken(XML_XMLNS) );
                        if( -1 == nIndex )
                            nIndex = maPropMapper->FindEntryIndex( "ParaUserDefinedAttributes", XML_NAMESPACE_TEXT, GetXMLToken(XML_XMLNS) );
                        if( -1 == nIndex )
                            nIndex = maPropMapper->FindEntryIndex( "TextUserDefinedAttributes", XML_NAMESPACE_TEXT, GetXMLToken(XML_XMLNS) );

                        OSL_ENSURE( nIndex != -1,
                                    "not able to store alien attribute");

                        // #106963#; use userdefined attribute only if it is in the specified property range
                        if( nIndex != -1 && nIndex >= nStartIdx && nIndex < nEndIdx)
                        {
                            Any aAny;
                            aAny <<= xAttrContainer;
                            XMLPropertyState aNewProperty( nIndex, aAny );

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
                            sName.append( sal_Unicode(':') );
                            aData.Namespace = aNamespace;
                        }

                        sName.append( aLocalName );

                        Any aAny;
                        aAny <<= aData;
                        xAttrContainer->insertByName( sName.makeStringAndClear(), aAny );
                    }
                }
            }
        }
        while( ( nIndex >= 0 ) && (( nFlags & MID_FLAG_MULTI_PROPERTY ) != 0 ) );
    }

    finished( rProperties, nStartIdx, nEndIdx );

    // Have to do if we change from a vector to a list or something like that
    /*std::vector <XMLPropertyState>::iterator aItr = rProperties.begin();
    while (aItr != rProperties.end())
    {
        if (aItr->mnIndex == -1)
            aItr = rProperties.erase(aItr);
        else
            aItr++;
    }*/
}

/** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_IMPORT flag set */
BOOL SvXMLImportPropertyMapper::handleSpecialItem(
        XMLPropertyState& rProperty,
        vector< XMLPropertyState >& rProperties,
        const OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    OSL_ENSURE( mxNextMapper.is(), "unsuported special item in xml import" );
    if( mxNextMapper.is() )
        return mxNextMapper->handleSpecialItem( rProperty, rProperties, rValue,
                                               rUnitConverter, rNamespaceMap );
    else
        return FALSE;
}


sal_Bool SvXMLImportPropertyMapper::FillPropertySet(
            const vector< XMLPropertyState >& aProperties,
            const Reference< XPropertySet > rPropSet,
            struct _ContextID_Index_Pair* pSpecialContextIds ) const
{
    sal_Bool bSet = sal_False;

    // get property set info
    Reference< XPropertySetInfo > xInfo = rPropSet->getPropertySetInfo();

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

    return bSet;
}

sal_Bool SvXMLImportPropertyMapper::_FillPropertySet(
    const vector<XMLPropertyState> & rProperties,
    const Reference<XPropertySet> & rPropSet,
    const Reference<XPropertySetInfo> & rPropSetInfo,
    const UniReference<XMLPropertySetMapper> & rPropMapper,
    SvXMLImport& rImport,
    struct _ContextID_Index_Pair* pSpecialContextIds )
{
    OSL_ENSURE( rPropSet.is(), "need an XPropertySet" );
    OSL_ENSURE( rPropSetInfo.is(), "need an XPropertySetInfo" );

    // preliminaries
    sal_Bool bSet = sal_False;
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
                bSet = sal_True;
            }
            catch ( IllegalArgumentException& e )
            {
                // illegal value: check whether this property is
                // allowed to throw this exception
                if ( 0 == ( nPropFlags & MID_FLAG_PROPERTY_MAY_EXCEPT ) )
                {
                    Sequence<OUString> aSeq(1);
                    aSeq[0] = rPropName;
                    rImport.SetError(
                        XMLERROR_STYLE_PROP_VALUE | XMLERROR_FLAG_ERROR,
                        aSeq, e.Message, NULL );
                }
            }
            catch ( UnknownPropertyException& e )
            {
                // unknown property: This is always an error!
                Sequence<OUString> aSeq(1);
                aSeq[0] = rPropName;
                rImport.SetError(
                    XMLERROR_STYLE_PROP_UNKNOWN | XMLERROR_FLAG_ERROR,
                    aSeq, e.Message, NULL );
            }
            catch ( PropertyVetoException& e )
            {
                // property veto: this shouldn't happen
                Sequence<OUString> aSeq(1);
                aSeq[0] = rPropName;
                rImport.SetError(
                    XMLERROR_STYLE_PROP_OTHER | XMLERROR_FLAG_ERROR,
                    aSeq, e.Message, NULL );
            }
            catch ( WrappedTargetException& e )
            {
                // wrapped target: this shouldn't happen either
                Sequence<OUString> aSeq(1);
                aSeq[0] = rPropName;
                rImport.SetError(
                    XMLERROR_STYLE_PROP_OTHER | XMLERROR_FLAG_ERROR,
                    aSeq, e.Message, NULL );
            }
        }

        // handle no-property and special items
        if( ( pSpecialContextIds != NULL ) &&
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
    public binary_function<PropertyPair, PropertyPair, bool>
{
    bool operator()( const PropertyPair& a, const PropertyPair& b ) const
    {
        return (*a.first < *b.first ? true : false);
    }
};

sal_Bool SvXMLImportPropertyMapper::_FillMultiPropertySet(
    const vector<XMLPropertyState> & rProperties,
    const Reference<XMultiPropertySet> & rMultiPropSet,
    const Reference<XPropertySetInfo> & rPropSetInfo,
    const UniReference<XMLPropertySetMapper> & rPropMapper,
    struct _ContextID_Index_Pair* pSpecialContextIds )
{
    sal_Int32 i;

    OSL_ENSURE( rMultiPropSet.is(), "Need multi property set. ");
    OSL_ENSURE( rPropSetInfo.is(), "Need property set info." );

    sal_Bool bSuccessful = sal_False;
    sal_Int32 nCount = rProperties.size();

    // property pairs structure stores names + values of properties to be set.
    PropertyPairs aPropertyPairs;
    aPropertyPairs.reserve( nCount );

    // iterate over property states that we want to set
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
               rPropSetInfo->hasPropertyByName( rPropName ) )    )
        {
            // save property into property pair structure
            PropertyPair aPair( &rPropName, &rProp.maValue );
            aPropertyPairs.push_back( aPair );
        }

        // handle no-property and special items
        if( ( pSpecialContextIds != NULL ) &&
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
    Sequence<OUString> aNames( aPropertyPairs.size() );
    OUString* pNamesArray = aNames.getArray();
    Sequence<Any> aValues( aPropertyPairs.size() );
    Any* pValuesArray = aValues.getArray();

    // copy values into sequences
    i = 0;
    for( PropertyPairs::iterator aIter = aPropertyPairs.begin();
         aIter != aPropertyPairs.end();
         aIter++ )
    {
        pNamesArray[i] = *(aIter->first);
        pValuesArray[i++] = *(aIter->second);
    }

    // and, finally, try to set the values
    try
    {
        rMultiPropSet->setPropertyValues( aNames, aValues );
        bSuccessful = sal_True;
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
