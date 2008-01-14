/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xsddatatypes.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 15:01:48 $
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
#include "precompiled_extensions.hxx"

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_XSDDATATYPES_HXX
#include "xsddatatypes.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_XSD_DATATYPECLASS_HPP_
#include <com/sun/star/xsd/DataTypeClass.hpp>
#endif
#ifndef _COM_SUN_STAR_XSD_XDATATYPE_HPP_
#include <com/sun/star/xsd/XDataType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xsd;
    using namespace ::com::sun::star::beans;

    //====================================================================
    //= helper
    //====================================================================
    //--------------------------------------------------------------------
    template< typename INTERFACE, typename ARGUMENT >
    void setSave( INTERFACE* pObject, void ( SAL_CALL INTERFACE::*pSetter )( ARGUMENT ), ARGUMENT _rArg )
    {
        try
        {
            (pObject->*pSetter)( _rArg );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "XSDDataType: setSave: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    template< typename INTERFACE, typename ARGUMENT >
    ARGUMENT getSave( INTERFACE* pObject, ARGUMENT ( SAL_CALL INTERFACE::*pGetter )( ) )
    {
        ARGUMENT aReturn = ARGUMENT();
        try
        {
            aReturn = (pObject->*pGetter)( );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "XSDDataType: getSave: caught an exception!" );
        }
        return aReturn;
    }

    template< typename FACETTYPE >
    FACETTYPE getFacet( const Reference< XPropertySet >& _rxFacets, const ::rtl::OUString& _rFacetName ) SAL_THROW(())
    {
        FACETTYPE aReturn;
        try
        {
            OSL_VERIFY( _rxFacets->getPropertyValue( _rFacetName ) >>= aReturn );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "XSDDataType: getFacet: caught an exception!" );
        }
        return aReturn;
    }

    //====================================================================
    //= XSDDataType
    //====================================================================
    //--------------------------------------------------------------------
    XSDDataType::XSDDataType( const Reference< XDataType >& _rxDataType )
        :m_xDataType( _rxDataType )
        ,m_refCount( 0 )
    {
        DBG_ASSERT( m_xDataType.is(), "XSDDataType::XSDDataType: invalid UNO object!" );
        if ( m_xDataType.is() )
            m_xFacetInfo = m_xDataType->getPropertySetInfo();
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL XSDDataType::acquire()
    {
        return osl_incrementInterlockedCount( &m_refCount );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL XSDDataType::release()
    {
        if ( 0 == osl_decrementInterlockedCount( &m_refCount ) )
        {
           delete this;
           return 0;
        }
        return m_refCount;
    }

    //--------------------------------------------------------------------
    XSDDataType::~XSDDataType()
    {
    }

    //--------------------------------------------------------------------
    sal_Int16 XSDDataType::classify() const SAL_THROW(())
    {
        sal_Int16 nTypeClass = DataTypeClass::STRING;
        try
        {
            if ( m_xDataType.is() )
                nTypeClass = m_xDataType->getTypeClass();
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "XSDDataType::classify: caught an exception!" );
        }
        return nTypeClass;
    }

    //--------------------------------------------------------------------
    bool XSDDataType::isBasicType() const SAL_THROW(())
    {
        return getSave( m_xDataType.get(), &XDataType::getIsBasic );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString XSDDataType::getName() const SAL_THROW(())
    {
        return getSave( m_xDataType.get(), &XDataType::getName );
    }

     //--------------------------------------------------------------------
    void XSDDataType::setFacet( const ::rtl::OUString& _rFacetName, const Any& _rValue ) SAL_THROW(())
    {
        try
        {
            m_xDataType->setPropertyValue( _rFacetName, _rValue );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "XSDDataType::setFacet: caught an exception - sure this is the right data type class for this property?" );
        }
    }

    //--------------------------------------------------------------------
    bool XSDDataType::hasFacet( const ::rtl::OUString& _rFacetName ) const SAL_THROW(())
    {
        bool bReturn = false;
        try
        {
            bReturn = m_xFacetInfo.is() && m_xFacetInfo->hasPropertyByName( _rFacetName );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "XSDDataType::hasFacet: caught an exception!" );
        }
        return bReturn;
    }

    //--------------------------------------------------------------------
    Type XSDDataType::getFacetType( const ::rtl::OUString& _rFacetName ) const SAL_THROW(())
    {
        Type aReturn;
        try
        {
            if ( m_xFacetInfo.is() )
                aReturn = m_xFacetInfo->getPropertyByName( _rFacetName ).Type;
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "XSDDataType::getFacetType: caught an exception (did you check with 'hasFacet' before?)!" );
        }
        return aReturn;
    }

    //--------------------------------------------------------------------
    Any XSDDataType::getFacet( const ::rtl::OUString& _rFacetName ) SAL_THROW(())
    {
        Any aReturn;
        try
        {
            aReturn = m_xDataType->getPropertyValue( _rFacetName );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "XSDDataType::getFacet: caught an exception - sure this is the right data type class for this property?" );
        }
        return aReturn;
    }

    //--------------------------------------------------------------------
    namespace
    {
        void lcl_copyProperties( const Reference< XPropertySet >& _rxSource, const Reference< XPropertySet >& _rxDest )
        {
            Reference< XPropertySetInfo > xSourceInfo;
            if ( _rxSource.is() )
                xSourceInfo = _rxSource->getPropertySetInfo();
            Reference< XPropertySetInfo > xDestInfo;
            if ( _rxDest.is() )
                xDestInfo = _rxDest->getPropertySetInfo();
            OSL_ENSURE( xSourceInfo.is() && xDestInfo.is(), "lcl_copyProperties: invalid property set( info)s!" );
            if ( !xSourceInfo.is() || !xDestInfo.is() )
                return;

            Sequence< Property > aProperties( xSourceInfo->getProperties() );
            const Property* pProperties = aProperties.getConstArray();
            const Property* pPropertiesEnd = pProperties + aProperties.getLength();
            for ( ; pProperties != pPropertiesEnd; ++pProperties )
            {
                if ( xDestInfo->hasPropertyByName( pProperties->Name ) )
                    _rxDest->setPropertyValue( pProperties->Name, _rxSource->getPropertyValue( pProperties->Name ) );
            }
        }
    }

    //--------------------------------------------------------------------
    void XSDDataType::copyFacetsFrom( const ::rtl::Reference< XSDDataType >& _pSourceType )
    {
        OSL_ENSURE( _pSourceType.is(), "XSDDataType::copyFacetsFrom: invalid source type!" );
        if ( !_pSourceType.is() )
            return;

        try
        {
            Reference< XPropertySet > xSource( _pSourceType->getUnoDataType(), UNO_QUERY );
            Reference< XPropertySet > xDest( getUnoDataType(), UNO_QUERY );
            lcl_copyProperties( xSource, xDest );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "XSDDataType::copyFacetsFrom: caught an exception!" );
        }
    }

//........................................................................
} // namespace pcr
//........................................................................

