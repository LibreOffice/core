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
#include <docmodel/uno/UnoGradientTools.hxx>
#include <comphelper/sequence.hxx>
#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>
#include <o3tl/string_view.hxx>

#include <utility>
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
    explicit lcl_EqualsElement( Any  rValue, const Reference< container::XNameAccess > & xAccess )
            : m_aValue(std::move( rValue )), m_xAccess( xAccess )
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
    explicit lcl_StringMatches( OUString aCmpStr ) :
            m_aCmpStr(std::move( aCmpStr ))
    {}

    bool operator() ( std::u16string_view rStr )
    {
        return o3tl::starts_with( rStr, m_aCmpStr );
    }

private:
    OUString m_aCmpStr;
};

struct lcl_OUStringRestToInt32
{
    explicit lcl_OUStringRestToInt32( sal_Int32 nPrefixLength ) :
            m_nPrefixLength( nPrefixLength )
    {}
    sal_Int32 operator() ( std::u16string_view rStr )
    {
        if( m_nPrefixLength > static_cast<sal_Int32>(rStr.size()) )
            return 0;
        return o3tl::toInt32(rStr.substr( m_nPrefixLength ));
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
        ! rValue.hasValue() )
        return rPreferredName;

    Any aValue(rValue);

    if ( rValue.has<css::awt::Gradient>())
    {
        // tdf#158421 the lists for Gradients needs awt::Gradient2
        // as type, convert input data if needed (and warn about it,
        // the caller should be changed to offer the needed type)
        SAL_WARN("chart2","input value needs to be awt::Gradient2");
        const basegfx::BGradient aTemp(model::gradient::getFromAny(rValue));
        aValue <<= model::gradient::createUnoGradient2(aTemp);
    }

    if ( aValue.getValueType() != xNameContainer->getElementType())
        return rPreferredName;

    try
    {
        Reference< container::XNameAccess > xNameAccess( xNameContainer, uno::UNO_QUERY_THROW );
        const uno::Sequence<OUString> aElementNames = xNameAccess->getElementNames();
        auto it = std::find_if( aElementNames.begin(), aElementNames.end(), lcl_EqualsElement( aValue, xNameAccess ));

        // element found => return name
        if( it != aElementNames.end())
            return *it;

        // element not found in container
        OUString aUniqueName;

        // check if preferred name is already used
        if( !rPreferredName.isEmpty())
        {
            auto aIt = std::find( aElementNames.begin(), aElementNames.end(), rPreferredName );
            if( aIt == aElementNames.end())
                aUniqueName = rPreferredName;
        }

        if( aUniqueName.isEmpty())
        {
            auto aNames( comphelper::sequenceToContainer<std::vector< OUString >>( aElementNames ));
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
        xNameContainer->insertByName( aUniqueName, aValue );
        return aUniqueName;
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
            xFact->createInstance( u"com.sun.star.drawing.DashTable"_ustr),
            uno::UNO_QUERY );
        if( xNameCnt.is())
            return lcl_addNamedPropertyUniqueNameToTable(
                rValue, xNameCnt, u"ChartDash "_ustr, rPreferredName );
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
            xFact->createInstance( u"com.sun.star.drawing.GradientTable"_ustr),
            uno::UNO_QUERY );
        if( xNameCnt.is())
            return lcl_addNamedPropertyUniqueNameToTable(
                rValue, xNameCnt, u"ChartGradient "_ustr, rPreferredName );
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
            xFact->createInstance( u"com.sun.star.drawing.TransparencyGradientTable"_ustr),
            uno::UNO_QUERY );
        if( xNameCnt.is())
            return lcl_addNamedPropertyUniqueNameToTable(
                rValue, xNameCnt, u"ChartTransparencyGradient "_ustr, rPreferredName );
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
            xFact->createInstance( u"com.sun.star.drawing.HatchTable"_ustr),
            uno::UNO_QUERY );
        if( xNameCnt.is())
            return lcl_addNamedPropertyUniqueNameToTable(
                rValue, xNameCnt, u"ChartHatch "_ustr, rPreferredName );
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
            xFact->createInstance( u"com.sun.star.drawing.BitmapTable"_ustr),
            uno::UNO_QUERY );
        if( xNameCnt.is())
            return lcl_addNamedPropertyUniqueNameToTable(
                rValue, xNameCnt, u"ChartBitmap "_ustr, rPreferredName );
    }
    return OUString();
}

void setPropertyValueAny( tPropertyValueMap & rOutMap, tPropertyValueMapKey key, const uno::Any & rAny )
{
    tPropertyValueMap::iterator aIt( rOutMap.find( key ));
    if( aIt == rOutMap.end())
        rOutMap.emplace( key, rAny );
    else
        (*aIt).second = rAny;
}

template<>
    void setPropertyValue< css::uno::Any >( tPropertyValueMap & rOutMap, tPropertyValueMapKey key, const css::uno::Any & rAny )
{
    setPropertyValueAny( rOutMap, key, rAny );
}

void setPropertyValueDefaultAny( tPropertyValueMap & rOutMap, tPropertyValueMapKey key, const uno::Any & rAny )
{
    OSL_ENSURE( rOutMap.end() == rOutMap.find( key ), "Default already exists for property" );
    setPropertyValue( rOutMap, key, rAny );
}

template<>
    void setPropertyValueDefault< css::uno::Any >( tPropertyValueMap & rOutMap, tPropertyValueMapKey key, const css::uno::Any & rAny )
{
    setPropertyValueDefaultAny( rOutMap, key, rAny );
}

void setEmptyPropertyValueDefault( tPropertyValueMap & rOutMap, tPropertyValueMapKey key )
{
    setPropertyValueDefault( rOutMap, key, uno::Any());
}

} //  namespace chart::PropertyHelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
