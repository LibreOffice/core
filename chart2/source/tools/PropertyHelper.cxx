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

#include "PropertyHelper.hxx"
#include "ContainerHelper.hxx"
#include "macros.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <vector>
#include <algorithm>
#include <iterator>
#include <functional>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{
struct lcl_EqualsElement : public ::std::unary_function< OUString, bool >
{
    explicit lcl_EqualsElement( const Any & rValue, const Reference< container::XNameAccess > & xAccess )
            : m_aValue( rValue ), m_xAccess( xAccess )
    {
        OSL_ASSERT( m_xAccess.is());
    }

    bool operator() ( const OUString & rName )
    {
        try
        {
            return (m_xAccess->getByName( rName ) == m_aValue);
        }
        catch( const uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
        return false;
    }

private:
    Any m_aValue;
    Reference< container::XNameAccess > m_xAccess;
};

struct lcl_StringMatches : public ::std::unary_function< OUString ,bool >
{
    lcl_StringMatches( const OUString & rCmpStr ) :
            m_aCmpStr( rCmpStr )
    {}

    bool operator() ( const OUString & rStr )
    {
        return rStr.match( m_aCmpStr );
    }

private:
    OUString m_aCmpStr;
};

struct lcl_OUStringRestToInt32 : public ::std::unary_function< OUString, sal_Int32 >
{
    lcl_OUStringRestToInt32( sal_Int32 nPrefixLength ) :
            m_nPrefixLength( nPrefixLength )
    {}
    sal_Int32 operator() ( const OUString & rStr )
    {
        if( m_nPrefixLength > rStr.getLength() )
            return 0;
        return rStr.copy( m_nPrefixLength ).toInt32();
    }
private:
    sal_Int32 m_nPrefixLength;
};

/** adds a fill gradient, fill hatch, fill bitmap, fill transparency gradient,
    line dash or line marker to the corresponding name container with a unique
    name.

    @param rPrefix
        The prefix used for automated name generation.

    @param rPreferredName
        If this string is not empty it is used as name if it is unique in the
        table. Otherwise a new name is generated using pPrefix.

    @return the new name under which the property was stored in the table
*/
OUString lcl_addNamedPropertyUniqueNameToTable(
    const Any & rValue,
    const Reference< container::XNameContainer > & xNameContainer,
    const OUString & rPrefix,
    const OUString & rPreferredName )
{
    if( ! xNameContainer.is() ||
        ! rValue.hasValue() ||
        ( rValue.getValueType() != xNameContainer->getElementType()))
        return rPreferredName;

    try
    {
        Reference< container::XNameAccess > xNameAccess( xNameContainer, uno::UNO_QUERY_THROW );
        ::std::vector< OUString > aNames( ::chart::ContainerHelper::SequenceToVector( xNameAccess->getElementNames()));
        ::std::vector< OUString >::const_iterator aIt(
            ::std::find_if( aNames.begin(), aNames.end(), lcl_EqualsElement( rValue, xNameAccess )));

        // element not found in container
        if( aIt == aNames.end())
        {
            OUString aUniqueName;

            // check if preferred name is already used
            if( !rPreferredName.isEmpty())
            {
                aIt = ::std::find( aNames.begin(), aNames.end(), rPreferredName );
                if( aIt == aNames.end())
                    aUniqueName = rPreferredName;
            }

            if( aUniqueName.isEmpty())
            {
                // create a unique id using the prefix plus a number
                ::std::vector< sal_Int32 > aNumbers;
                ::std::vector< OUString >::iterator aNonConstIt(
                    ::std::partition( aNames.begin(), aNames.end(), lcl_StringMatches( rPrefix )));
                ::std::transform( aNames.begin(), aNonConstIt,
                                  back_inserter( aNumbers ),
                                  lcl_OUStringRestToInt32( rPrefix.getLength() ));
                ::std::vector< sal_Int32 >::const_iterator aMaxIt(
                    ::std::max_element( aNumbers.begin(), aNumbers.end()));

                sal_Int32 nIndex = 1;
                if( aMaxIt != aNumbers.end())
                    nIndex = (*aMaxIt) + 1;

                aUniqueName = rPrefix + OUString::valueOf( nIndex );
            }

            OSL_ASSERT( !aUniqueName.isEmpty());
            xNameContainer->insertByName( aUniqueName, rValue );
            return aUniqueName;
        }
        else
            // element found => return name
            return *aIt;
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }

    return rPreferredName;
}

} // anonymous namespace

namespace chart
{
namespace PropertyHelper
{

OUString addLineDashUniqueNameToTable(
    const Any & rValue,
    const Reference< lang::XMultiServiceFactory > & xFact,
    const OUString & rPreferredName )
{
    if( xFact.is())
    {
        Reference< container::XNameContainer > xNameCnt(
            xFact->createInstance( "com.sun.star.drawing.DashTable"),
            uno::UNO_QUERY );
        if( xNameCnt.is())
            return lcl_addNamedPropertyUniqueNameToTable(
                rValue, xNameCnt, "ChartDash ", rPreferredName );
    }
    return OUString();
}

OUString addGradientUniqueNameToTable(
    const Any & rValue,
    const Reference< lang::XMultiServiceFactory > & xFact,
    const OUString & rPreferredName )
{
    if( xFact.is())
    {
        Reference< container::XNameContainer > xNameCnt(
            xFact->createInstance( "com.sun.star.drawing.GradientTable"),
            uno::UNO_QUERY );
        if( xNameCnt.is())
            return lcl_addNamedPropertyUniqueNameToTable(
                rValue, xNameCnt, "ChartGradient ", rPreferredName );
    }
    return OUString();
}

OUString addTransparencyGradientUniqueNameToTable(
    const Any & rValue,
    const Reference< lang::XMultiServiceFactory > & xFact,
    const OUString & rPreferredName )
{
    if( xFact.is())
    {
        Reference< container::XNameContainer > xNameCnt(
            xFact->createInstance( "com.sun.star.drawing.TransparencyGradientTable"),
            uno::UNO_QUERY );
        if( xNameCnt.is())
            return lcl_addNamedPropertyUniqueNameToTable(
                rValue, xNameCnt, "ChartTransparencyGradient ", rPreferredName );
    }
    return OUString();
}

OUString addHatchUniqueNameToTable(
    const Any & rValue,
    const Reference< lang::XMultiServiceFactory > & xFact,
    const OUString & rPreferredName )
{
    if( xFact.is())
    {
        Reference< container::XNameContainer > xNameCnt(
            xFact->createInstance( "com.sun.star.drawing.HatchTable"),
            uno::UNO_QUERY );
        if( xNameCnt.is())
            return lcl_addNamedPropertyUniqueNameToTable(
                rValue, xNameCnt, "ChartHatch ", rPreferredName );
    }
    return OUString();
}

OUString addBitmapUniqueNameToTable(
    const Any & rValue,
    const Reference< lang::XMultiServiceFactory > & xFact,
    const OUString & rPreferredName )
{
    if( xFact.is())
    {
        Reference< container::XNameContainer > xNameCnt(
            xFact->createInstance( "com.sun.star.drawing.BitmapTable"),
            uno::UNO_QUERY );
        if( xNameCnt.is())
            return lcl_addNamedPropertyUniqueNameToTable(
                rValue, xNameCnt, "ChartBitmap ", rPreferredName );
    }
    return OUString();
}

void setPropertyValueAny( tPropertyValueMap & rOutMap, tPropertyValueMapKey key, const uno::Any & rAny )
{
    tPropertyValueMap::iterator aIt( rOutMap.find( key ));
    if( aIt == rOutMap.end())
        rOutMap.insert( tPropertyValueMap::value_type( key, rAny ));
    else
        (*aIt).second = rAny;
}

template<>
    void setPropertyValue< ::com::sun::star::uno::Any >( tPropertyValueMap & rOutMap, tPropertyValueMapKey key, const ::com::sun::star::uno::Any & rAny )
{
    setPropertyValueAny( rOutMap, key, rAny );
}

void setPropertyValueDefaultAny( tPropertyValueMap & rOutMap, tPropertyValueMapKey key, const uno::Any & rAny )
{
    OSL_ENSURE( rOutMap.end() == rOutMap.find( key ), "Default already exists for property" );
    setPropertyValue( rOutMap, key, rAny );
}

template<>
    void setPropertyValueDefault< ::com::sun::star::uno::Any >( tPropertyValueMap & rOutMap, tPropertyValueMapKey key, const ::com::sun::star::uno::Any & rAny )
{
    setPropertyValueDefaultAny( rOutMap, key, rAny );
}

void setEmptyPropertyValueDefault( tPropertyValueMap & rOutMap, tPropertyValueMapKey key )
{
    setPropertyValueDefault( rOutMap, key, uno::Any());
}

} // namespace PropertyHelper

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
