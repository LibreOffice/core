/*************************************************************************
 *
 *  $RCSfile: datatypes_impl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:51:39 $
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

#ifndef DATATYPES_INCLUDED_BY_MASTER_HEADER
    #error "not to be included directly!"
#endif

//--------------------------------------------------------------------
template< typename CONCRETE_DATA_TYPE_IMPL, typename SUPERCLASS >
ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::ODerivedDataType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass )
    :SUPERCLASS( _rName, _nTypeClass )
    ,m_bPropertiesRegistered( false )
{
}

//--------------------------------------------------------------------
template< typename CONCRETE_DATA_TYPE_IMPL, typename SUPERCLASS >
::cppu::IPropertyArrayHelper* ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::createArrayHelper( ) const
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > aProps;
    ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}

//--------------------------------------------------------------------
template< typename CONCRETE_DATA_TYPE_IMPL, typename SUPERCLASS >
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException )
{
    return createPropertySetInfo( getInfoHelper() );
}

//--------------------------------------------------------------------
template< typename CONCRETE_DATA_TYPE_IMPL, typename SUPERCLASS >
::cppu::IPropertyArrayHelper& SAL_CALL ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::getInfoHelper()
{
    if ( !m_bPropertiesRegistered )
    {
        const_cast< ODerivedDataType* >( this )->registerProperties();
        const_cast< ODerivedDataType* >( this )->m_bPropertiesRegistered = true;
    }

    return *ODerivedDataType< CONCRETE_DATA_TYPE_IMPL, SUPERCLASS >::getArrayHelper();
}


//--------------------------------------------------------------------
template< typename VALUE_TYPE >
OComparableType< VALUE_TYPE >::OComparableType( const ::rtl::OUString& _rName, sal_Int16 _nTypeClass )
    :OXSDDataType( _rName, _nTypeClass )
{
}

//--------------------------------------------------------------------
template< typename VALUE_TYPE >
void OComparableType< VALUE_TYPE >::initializeClone( const OXSDDataType& _rCloneSource )
{
    OXSDDataType::initializeClone( _rCloneSource );
    initializeTypedClone( static_cast< const OComparableType& >( _rCloneSource ) );
}

//--------------------------------------------------------------------
template< typename VALUE_TYPE >
void OComparableType< VALUE_TYPE >::initializeTypedClone( const OComparableType& _rCloneSource )
{
    m_aMaxInclusive   = _rCloneSource.m_aMaxInclusive;
    m_aMaxExclusive   = _rCloneSource.m_aMaxExclusive;
    m_aMinInclusive   = _rCloneSource.m_aMinInclusive;
    m_aMinExclusive   = _rCloneSource.m_aMinExclusive;
}

//--------------------------------------------------------------------
template< typename VALUE_TYPE >
void OComparableType< VALUE_TYPE >::registerProperties()
{
    OXSDDataType::registerProperties();

    REGISTER_VOID_PROP( XSD_MAX_INCLUSIVE,   m_aMaxInclusive,   VALUE_TYPE );
    REGISTER_VOID_PROP( XSD_MAX_EXCLUSIVE,   m_aMaxExclusive,   VALUE_TYPE );
    REGISTER_VOID_PROP( XSD_MIN_INCLUSIVE,   m_aMinInclusive,   VALUE_TYPE );
    REGISTER_VOID_PROP( XSD_MIN_EXCLUSIVE,   m_aMinExclusive,   VALUE_TYPE );
}

//--------------------------------------------------------------------
template< typename VALUE_TYPE >
bool OComparableType< VALUE_TYPE >::_getValue( const ::rtl::OUString& rValue, double& fValue )
{
    // convert to double
    rtl_math_ConversionStatus eStatus;
    sal_Int32 nEnd;
    double f = ::rtl::math::stringToDouble(
        rValue, sal_Unicode('.'), sal_Unicode(0), &eStatus, &nEnd );

    // error checking...
    bool bReturn = false;
    if( eStatus == rtl_math_ConversionStatus_Ok
        && nEnd == rValue.getLength() )
    {
        bReturn = true;
        fValue = f;
    }
    return bReturn;
}

//--------------------------------------------------------------------
// validate min-/max facets
// to be used by validate(..) and explainInvalid(..) methods
template< typename VALUE_TYPE >
sal_uInt16 OComparableType< VALUE_TYPE >::_validate( const ::rtl::OUString& rValue )
{
    sal_uInt16 nReason = OXSDDataType::_validate( rValue );
    if( nReason == 0 )
    {

        // convert value and check format
        double nDoubleLimit = 0;
        double f;
        if( ! _getValue( rValue, f ) )
            nReason = RID_STR_XFORMS_VALUE_IS_NOT_A;

        // check range
        else if( ( m_aMaxInclusive >>= nDoubleLimit ) && f > nDoubleLimit )
            nReason = RID_STR_XFORMS_VALUE_MAX_INCL;
        else if( ( m_aMaxExclusive >>= nDoubleLimit ) && f >= nDoubleLimit)
            nReason = RID_STR_XFORMS_VALUE_MAX_EXCL;
        else if( ( m_aMinInclusive >>= nDoubleLimit ) && f < nDoubleLimit )
            nReason = RID_STR_XFORMS_VALUE_MIN_INCL;
        else if( ( m_aMinExclusive >>= nDoubleLimit ) && f <= nDoubleLimit)
            nReason = RID_STR_XFORMS_VALUE_MIN_EXCL;
    }
    return nReason;
}

//--------------------------------------------------------------------
template< typename VALUE_TYPE >
::rtl::OUString OComparableType< VALUE_TYPE >::_explainInvalid( sal_uInt16 nReason )
{
    ::rtl::OUStringBuffer sInfo;
    switch( nReason )
    {
    case 0:
        // nothing to do!
        break;

    case RID_STR_XFORMS_VALUE_IS_NOT_A:
        sInfo.append( getName() );
        break;

    case RID_STR_XFORMS_VALUE_MAX_INCL:
        sInfo.append( typedValueAsString( m_aMaxInclusive ) );
        break;

    case RID_STR_XFORMS_VALUE_MAX_EXCL:
        sInfo.append( typedValueAsString( m_aMaxExclusive ) );
        break;

    case RID_STR_XFORMS_VALUE_MIN_INCL:
        sInfo.append( typedValueAsString( m_aMinInclusive ) );
        break;

    case RID_STR_XFORMS_VALUE_MIN_EXCL:
        sInfo.append( typedValueAsString( m_aMinExclusive ) );
        break;

    default:
        OSL_ENSURE( false, "OComparableType::_explainInvalid: unknown reason!" );
        break;
    }

    return sInfo.makeStringAndClear();
}

