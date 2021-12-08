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

#include <PropertyHelper.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/sequence.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>

#include <vector>
#include <algorithm>
#include <iterator>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;

namespace
{
struct lcl_EqualsElement
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
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
        return false;
    }

private:
    Any m_aValue;
    Reference< container::XNameAccess > m_xAccess;
};

struct lcl_StringMatches
{
    explicit lcl_StringMatches( const OUString & rCmpStr ) :
            m_aCmpStr( rCmpStr )
    {}

    bool operator() ( const OUString & rStr )
    {
        return rStr.match( m_aCmpStr );
    }

private:
    OUString m_aCmpStr;
};

struct lcl_OUStringRestToInt32
{
    explicit lcl_OUStringRestToInt32( sal_Int32 nPrefixLength ) :
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
        auto aNames( comphelper::sequenceToContainer<std::vector< OUString >>( xNameAccess->getElementNames()));
        std::vector< OUString >::const_iterator aIt(
            std::find_if( aNames.begin(), aNames.end(), lcl_EqualsElement( rValue, xNameAccess )));

        // element not found in container
        if( aIt == aNames.end())
        {
            OUString aUniqueName;

            // check if preferred name is already used
            if( !rPreferredName.isEmpty())
            {
                aIt = std::find( aNames.begin(), aNames.end(), rPreferredName );
                if( aIt == aNames.end())
                    aUniqueName = rPreferredName;
            }

            if( aUniqueName.isEmpty())
            {
                // create a unique id using the prefix plus a number
                std::vector< sal_Int32 > aNumbers;
                std::vector< OUString >::iterator aNonConstIt(
                    std::partition( aNames.begin(), aNames.end(), lcl_StringMatches( rPrefix )));
                std::transform( aNames.begin(), aNonConstIt,
                                  back_inserter( aNumbers ),
                                  lcl_OUStringRestToInt32( rPrefix.getLength() ));
                std::vector< sal_Int32 >::const_iterator aMaxIt(
                    std::max_element( aNumbers.begin(), aNumbers.end()));

                sal_Int32 nIndex = 1;
                if( aMaxIt != aNumbers.end())
                    nIndex = (*aMaxIt) + 1;

                aUniqueName = rPrefix + OUString::number( nIndex );
            }

            OSL_ASSERT( !aUniqueName.isEmpty());
            xNameContainer->insertByName( aUniqueName, rValue );
            return aUniqueName;
        }
        else
            // element found => return name
            return *aIt;
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return rPreferredName;
}

} // anonymous namespace

namespace chart::PropertyHelper
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

} //  namespace chart::PropertyHelper

namespace chart
{

void tPropertyValueMap::setPropertyValueAny( sal_Int32 key, const uno::Any & rAny )
{
    maValues.try_emplace(key, rAny);
}

void tPropertyValueMap::setPropertyValueAny( sal_Int32 key, uno::Any && rAny )
{
    maValues.try_emplace(key, std::move(rAny));
}

void tPropertyValueMap::get(sal_Int32 nHandle, uno::Any& rAny) const
{
    auto it = maValues.find(nHandle);
    if( it == maValues.end() )
        rAny.clear();
    else
        rAny = it->second;
}

uno::Any tPropertyValueMap::get(sal_Int32 nHandle) const
{
    auto it = maValues.find(nHandle);
    if( it == maValues.end() )
        return uno::Any();
    else
        return it->second;
}

void tPropertyValueMap::setPropertyValueDefaultAny( sal_Int32 key, const uno::Any & rAny )
{
    OSL_ENSURE( maValues.find(key) == maValues.end(), "Default already exists for property" );
    setPropertyValue( key, rAny );
}

void tPropertyValueMap::setEmptyPropertyValueDefault( sal_Int32 key )
{
    setPropertyValueDefault(key, uno::Any());
}

void tPropertyValueMap::setPropertyValueDefault( sal_Int32 key, const css::uno::Any & rAny )
{
    setPropertyValueDefaultAny( key, rAny );
}

void tPropertyValueMap::setPropertyValueDefault( sal_Int32 key, css::uno::Any && rAny )
{
    setPropertyValueDefaultAny( key, std::move(rAny) );
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
