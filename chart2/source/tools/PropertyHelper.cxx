/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PropertyHelper.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 15:09:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "PropertyHelper.hxx"
#include "ContainerHelper.hxx"
#include "macros.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#include <vector>
#include <algorithm>
#include <functional>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using ::rtl::OUString;
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
        return rStr.copy( m_nPrefixLength ).toInt32( 10 /* radix */ );
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
            if( rPreferredName.getLength())
            {
                aIt = ::std::find( aNames.begin(), aNames.end(), rPreferredName );
                if( aIt == aNames.end())
                    aUniqueName = rPreferredName;
            }

            if( ! aUniqueName.getLength())
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

            OSL_ASSERT( aUniqueName.getLength());
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

void copyProperties(
    const Reference< XPropertySet > & xSource,
    const Reference< XPropertySet > & xDestination )
{
    if( ! (xSource.is() && xDestination.is()))
        return;

    try
    {
        Reference< XPropertySetInfo > xSrcInfo( xSource->getPropertySetInfo(), uno::UNO_QUERY_THROW );
        Reference< XPropertySetInfo > xDestInfo( xDestination->getPropertySetInfo(), uno::UNO_QUERY_THROW );
        Sequence< Property > aProperties( xSrcInfo->getProperties());
        const sal_Int32 nLength = aProperties.getLength();
        for( sal_Int32 i = 0; i < nLength; ++i )
        {
            OUString aName( aProperties[i].Name);
            if( xDestInfo->hasPropertyByName( aName ))
            {
                Property aProp( xDestInfo->getPropertyByName( aName ));
                if( (aProp.Attributes & PropertyAttribute::READONLY) == 0 )
                    xDestination->setPropertyValue(
                        aName, xSource->getPropertyValue( aName ));
            }
        }
    }
    catch( const uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
}

OUString addLineDashUniqueNameToTable(
    const Any & rValue,
    const Reference< lang::XMultiServiceFactory > & xFact,
    const OUString & rPreferredName )
{
    if( xFact.is())
    {
        Reference< container::XNameContainer > xNameCnt(
            xFact->createInstance( C2U( "com.sun.star.drawing.DashTable" )),
            uno::UNO_QUERY );
        if( xNameCnt.is())
            return lcl_addNamedPropertyUniqueNameToTable(
                rValue, xNameCnt, C2U( "ChartDash " ), rPreferredName );
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
            xFact->createInstance( C2U( "com.sun.star.drawing.GradientTable" )),
            uno::UNO_QUERY );
        if( xNameCnt.is())
            return lcl_addNamedPropertyUniqueNameToTable(
                rValue, xNameCnt, C2U( "ChartGradient " ), rPreferredName );
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
            xFact->createInstance( C2U( "com.sun.star.drawing.TransparencyGradientTable" )),
            uno::UNO_QUERY );
        if( xNameCnt.is())
            return lcl_addNamedPropertyUniqueNameToTable(
                rValue, xNameCnt, C2U( "ChartTransparencyGradient " ), rPreferredName );
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
            xFact->createInstance( C2U( "com.sun.star.drawing.HatchTable" )),
            uno::UNO_QUERY );
        if( xNameCnt.is())
            return lcl_addNamedPropertyUniqueNameToTable(
                rValue, xNameCnt, C2U( "ChartHatch " ), rPreferredName );
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
            xFact->createInstance( C2U( "com.sun.star.drawing.BitmapTable" )),
            uno::UNO_QUERY );
        if( xNameCnt.is())
            return lcl_addNamedPropertyUniqueNameToTable(
                rValue, xNameCnt, C2U( "ChartBitmap " ), rPreferredName );
    }
    return OUString();
}

// ----------------------------------------

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
