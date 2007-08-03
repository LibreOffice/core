/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WrappedNumberFormatProperty.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 12:32:48 $
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

#include "WrappedNumberFormatProperty.hxx"
#include "macros.hxx"

// header for define DBG_ERROR
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;

//.............................................................................
namespace chart
{
namespace wrapper
{
//.............................................................................

WrappedNumberFormatProperty::WrappedNumberFormatProperty( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact )
        : WrappedDirectStateProperty( C2U("NumberFormat"), C2U("NumberFormat") )
        , m_spChart2ModelContact(spChart2ModelContact)
{
    m_aOuterValue = getPropertyDefault( 0 );
}

WrappedNumberFormatProperty::~WrappedNumberFormatProperty()
{
    if( m_pWrappedLinkNumberFormatProperty )
    {
        if( m_pWrappedLinkNumberFormatProperty->m_pWrappedNumberFormatProperty == this )
            m_pWrappedLinkNumberFormatProperty->m_pWrappedNumberFormatProperty = 0;
    }
}

void WrappedNumberFormatProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    sal_Int32 nFormat = 0;
    if( ! (rOuterValue >>= nFormat) )
        throw lang::IllegalArgumentException( C2U("Property 'NumberFormat' requires value of type sal_Int32"), 0, 0 );

    m_aOuterValue = rOuterValue;
    if(xInnerPropertySet.is())
    {
        bool bUseSourceFormat = !xInnerPropertySet->getPropertyValue( C2U("NumberFormat" )).hasValue();
        if( bUseSourceFormat )
        {
            uno::Reference< chart2::XChartDocument > xChartDoc( m_spChart2ModelContact->getChart2Document() );
            if( xChartDoc.is() && xChartDoc->hasInternalDataProvider() )
                bUseSourceFormat = false;
        }
        if( !bUseSourceFormat )
            xInnerPropertySet->setPropertyValue( m_aInnerName, this->convertOuterToInnerValue( rOuterValue ) );
    }
}

Any WrappedNumberFormatProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR("missing xInnerPropertySet in WrappedNumberFormatProperty::getPropertyValue");
        return Any();
    }
    Any aRet( xInnerPropertySet->getPropertyValue( m_aInnerName ));
    if( !aRet.hasValue() )
    {
        Reference< chart2::XAxis > xAxis( xInnerPropertySet, uno::UNO_QUERY );
        sal_Int32 nKey = m_spChart2ModelContact->getExplicitNumberFormatKeyForAxis( xAxis );
        aRet <<= nKey;
    }
    return aRet;
}

Any WrappedNumberFormatProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny( sal_Int32( 0 ) );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

WrappedLinkNumberFormatProperty::WrappedLinkNumberFormatProperty( WrappedNumberFormatProperty* pWrappedNumberFormatProperty )
        : WrappedProperty( C2U("LinkNumberFormatToSource"), C2U("") )
        , m_pWrappedNumberFormatProperty( pWrappedNumberFormatProperty )
{
    if( m_pWrappedNumberFormatProperty )
    {
        m_pWrappedNumberFormatProperty->m_pWrappedLinkNumberFormatProperty = this;
    }
}

WrappedLinkNumberFormatProperty::~WrappedLinkNumberFormatProperty()
{
    if( m_pWrappedNumberFormatProperty )
    {
        if( m_pWrappedNumberFormatProperty->m_pWrappedLinkNumberFormatProperty == this )
            m_pWrappedNumberFormatProperty->m_pWrappedLinkNumberFormatProperty = 0;
    }
}

void WrappedLinkNumberFormatProperty::setPropertyValue( const Any& rOuterValue, const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if( !xInnerPropertySet.is() )
    {
        DBG_ERROR("missing xInnerPropertySet in WrappedNumberFormatProperty::setPropertyValue");
        return;
    }

    bool bLinkFormat = false;
    if( rOuterValue >>= bLinkFormat )
    {
        Any aValue;
        if( bLinkFormat )
        {
            if( m_pWrappedNumberFormatProperty )
            {
                uno::Reference< chart2::XChartDocument > xChartDoc( m_pWrappedNumberFormatProperty->m_spChart2ModelContact->getChart2Document() );
                if( xChartDoc.is() && xChartDoc->hasInternalDataProvider() )
                    return;
            }
        }
        else
        {
            if( m_pWrappedNumberFormatProperty )
            {
                aValue = m_pWrappedNumberFormatProperty->getPropertyValue( xInnerPropertySet );
            }
            else
                aValue <<= sal_Int32( 0 );
        }

        xInnerPropertySet->setPropertyValue( C2U("NumberFormat"), aValue );
    }
}

Any WrappedLinkNumberFormatProperty::getPropertyValue( const Reference< beans::XPropertySet >& xInnerPropertySet ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    bool bLink = ! xInnerPropertySet->getPropertyValue( C2U("NumberFormat" )).hasValue();
    return uno::makeAny( bLink );
}

Any WrappedLinkNumberFormatProperty::getPropertyDefault( const Reference< beans::XPropertyState >& /*xInnerPropertyState*/ ) const
                        throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    bool bLink = true;
    return uno::makeAny( bLink );
}

//.............................................................................
} //namespace wrapper
} //namespace chart
//.............................................................................
