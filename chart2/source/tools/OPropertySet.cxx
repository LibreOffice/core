/*************************************************************************
 *
 *  $RCSfile: OPropertySet.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:32 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "OPropertySet.hxx"
#include "ImplOPropertySet.hxx"
#include "algohelper.hxx"

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;

using ::drafts::com::sun::star::style::XStyleSupplier;
// using ::drafts::com::sun::star::beans::XFastPropertyState;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;
using ::rtl::OUString;
using ::osl::MutexGuard;

// needed for MS compiler
using ::cppu::OBroadcastHelper;
using ::cppu::OPropertySetHelper;
using ::cppu::OWeakObject;

namespace property
{

OPropertySet::OPropertySet( ::osl::Mutex & _rMutex ) :
        OBroadcastHelper( _rMutex ),
        // the following causes a warning; there seems to be no way to avoid it
        OPropertySetHelper( static_cast< OBroadcastHelper & >( *this )),
        m_rMutex( _rMutex ),
        m_pImplProperties( new impl::ImplOPropertySet() )
{
}

OPropertySet::~OPropertySet()
{}

Any SAL_CALL OPropertySet::queryInterface( const uno::Type& aType )
    throw (uno::RuntimeException)
{
    return ::cppu::queryInterface(
        aType,
//         static_cast< uno::XInterface * >(
//             static_cast< uno::XWeak * >( this )),
//         static_cast< uno::XWeak * >( this ),
//         static_cast< lang::XServiceInfo * >( this ),
        static_cast< lang::XTypeProvider * >( this ),
        static_cast< beans::XPropertySet * >( this ),
        static_cast< beans::XMultiPropertySet * >( this ),
        static_cast< beans::XFastPropertySet * >( this ),
        static_cast< beans::XPropertyState * >( this ),
        static_cast< beans::XMultiPropertyStates * >( this ),
        static_cast< XStyleSupplier * >( this ) );
//         static_cast< XFastPropertyState * >( this ) );
}

// void SAL_CALL OPropertySet::acquire() throw ()
// {
//     OWeakObject::acquire();
// }

// void SAL_CALL OPropertySet::release() throw ()
// {
//     OWeakObject::release();
// }


// ____ XServiceInfo ____
// OUString SAL_CALL
//     OPropertySet::getImplementationName()
//     throw (uno::RuntimeException)
// {
//     return OUString( RTL_CONSTASCII_USTRINGPARAM( "property::OPropertySet" ));
// }

// sal_Bool SAL_CALL
//     OPropertySet::supportsService( const OUString& ServiceName )
//     throw (uno::RuntimeException)
// {
//     return ( 0 == ServiceName.reverseCompareToAsciiL(
//                  RTL_CONSTASCII_STRINGPARAM( "com.sun.star.beans.PropertySet" )));
// }

// Sequence< OUString > SAL_CALL
//     OPropertySet::getSupportedServiceNames()
//     throw (uno::RuntimeException)
// {
//     Sequence< OUString > aServiceNames( 1 );
//     aServiceNames[ 0 ] = OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.beans.PropertySet" ));
//     return aServiceNames;
// }

#define LCL_PROP_CPPUTYPE(t) (::getCppuType( reinterpret_cast< const Reference<t> *>(0)))

// // ____ XTypeProvider ____
Sequence< uno::Type > SAL_CALL
    OPropertySet::getTypes()
    throw (uno::RuntimeException)
{
    static Sequence< uno::Type > aTypeList;

    // /--
    MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

    if( aTypeList.getLength() == 0 )
    {
        ::std::vector< uno::Type > aTypes;

//         aTypes.push_back( LCL_PROP_CPPUTYPE( uno::XWeak ));
//         aTypes.push_back( LCL_PROP_CPPUTYPE( lang::XServiceInfo ));
        aTypes.push_back( LCL_PROP_CPPUTYPE( lang::XTypeProvider ));
        aTypes.push_back( LCL_PROP_CPPUTYPE( beans::XPropertySet ));
        aTypes.push_back( LCL_PROP_CPPUTYPE( beans::XMultiPropertySet ));
        aTypes.push_back( LCL_PROP_CPPUTYPE( beans::XFastPropertySet ));
        aTypes.push_back( LCL_PROP_CPPUTYPE( beans::XPropertyState ));
        aTypes.push_back( LCL_PROP_CPPUTYPE( beans::XMultiPropertyStates ));
        aTypes.push_back( LCL_PROP_CPPUTYPE( XStyleSupplier ));
//         aTypes.push_back( LCL_PROP_CPPUTYPE( XFastPropertyState ));

        aTypeList = ::chart::helper::VectorToSequence( aTypes );
    }

    return aTypeList;
    // \--
}

Sequence< sal_Int8 > SAL_CALL
    OPropertySet::getImplementationId()
    throw (uno::RuntimeException)
{
    static uno::Sequence< sal_Int8 > aId;
    if( aId.getLength() == 0 )
    {
        aId.realloc( 16 );
        rtl_createUuid( (sal_uInt8 *)aId.getArray(), 0, sal_True );
    }
    return aId;
}


// ____ XPropertyState ____
beans::PropertyState SAL_CALL
    OPropertySet::getPropertyState( const OUString& PropertyName )
    throw (beans::UnknownPropertyException,
           uno::RuntimeException)
{
    cppu::IPropertyArrayHelper & rPH = getInfoHelper();

    return m_pImplProperties->GetPropertyStateByHandle(
        rPH.getHandleByName( PropertyName ));
}

Sequence< beans::PropertyState > SAL_CALL
    OPropertySet::getPropertyStates( const Sequence< OUString >& aPropertyName )
    throw (beans::UnknownPropertyException,
           uno::RuntimeException)
{
    cppu::IPropertyArrayHelper & rPH = getInfoHelper();

    sal_Int32 * pHandles = new sal_Int32[ aPropertyName.getLength() ];
    rPH.fillHandles( pHandles, aPropertyName );

    ::std::vector< sal_Int32 > aHandles( pHandles, pHandles + aPropertyName.getLength());
    delete[] pHandles;

    return m_pImplProperties->GetPropertyStatesByHandle( aHandles );
}

void SAL_CALL
    OPropertySet::setPropertyToDefault( const OUString& PropertyName )
    throw (beans::UnknownPropertyException,
           uno::RuntimeException)
{
    cppu::IPropertyArrayHelper & rPH = getInfoHelper();

    m_pImplProperties->SetPropertyToDefault( rPH.getHandleByName( PropertyName ));
}

Any SAL_CALL
    OPropertySet::getPropertyDefault( const OUString& aPropertyName )
    throw (beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    cppu::IPropertyArrayHelper & rPH = getInfoHelper();

    return GetDefaultValue( rPH.getHandleByName( aPropertyName ) );
}


// ____ XMultiPropertyStates ____

// Note: getPropertyStates() is already implemented in XPropertyState with the
// same signature

void SAL_CALL
    OPropertySet::setAllPropertiesToDefault()
    throw (uno::RuntimeException)
{
    m_pImplProperties->SetAllPropertiesToDefault();
}

void SAL_CALL
    OPropertySet::setPropertiesToDefault( const Sequence< OUString >& aPropertyNames )
    throw (beans::UnknownPropertyException,
           uno::RuntimeException)
{
    cppu::IPropertyArrayHelper & rPH = getInfoHelper();

    sal_Int32 * pHandles = new sal_Int32[ aPropertyNames.getLength() ];
    rPH.fillHandles( pHandles, aPropertyNames );

    ::std::vector< sal_Int32 > aHandles( pHandles, pHandles + aPropertyNames.getLength());
    delete[] pHandles;

    m_pImplProperties->SetPropertiesToDefault( aHandles );
}

Sequence< Any > SAL_CALL
    OPropertySet::getPropertyDefaults( const Sequence< OUString >& aPropertyNames )
    throw (beans::UnknownPropertyException,
           lang::WrappedTargetException,
           uno::RuntimeException)
{
    ::cppu::IPropertyArrayHelper & rPH = getInfoHelper();
    const sal_Int32 nElements = aPropertyNames.getLength();

    Sequence< Any > aResult( nElements );
    Any * pResultArray = aResult.getArray();
    sal_Int32 nI = 0;

    for( ; nI < nElements; ++nI )
    {
        pResultArray[ nI ] = GetDefaultValue(
            rPH.getHandleByName( aPropertyNames[ nI ] ));
    }

    return aResult;
}

sal_Bool SAL_CALL OPropertySet::convertFastPropertyValue
    ( Any & rConvertedValue,
      Any & rOldValue,
      sal_Int32 nHandle,
      const Any& rValue )
    throw (lang::IllegalArgumentException)
{
    getFastPropertyValue( rOldValue, nHandle );
    rConvertedValue = rValue;
    return sal_True;
}

void SAL_CALL OPropertySet::setFastPropertyValue_NoBroadcast
    ( sal_Int32 nHandle,
      const Any& rValue )
    throw (uno::Exception)
{
    m_pImplProperties->SetPropertyValueByHandle( nHandle, rValue );
}

void SAL_CALL OPropertySet::getFastPropertyValue
    ( Any& rValue,
      sal_Int32 nHandle ) const
{
    if( ! m_pImplProperties->GetPropertyValueByHandle( rValue, nHandle ))
    {
//         OSL_TRACE( "OPropertySet: asking style for property" );
        // property was not set -> try style
        uno::Reference< beans::XFastPropertySet > xStylePropSet( m_pImplProperties->GetStyle(), uno::UNO_QUERY );
        if( xStylePropSet.is() )
        {
#ifndef NDEBUG
            {
                // check if the handle of the style points to the same property
                // name as the handle in this property set
                uno::Reference< beans::XPropertySet > xPropSet( xStylePropSet, uno::UNO_QUERY );
                if( xPropSet.is())
                {
                    uno::Reference< beans::XPropertySetInfo > xInfo( xPropSet->getPropertySetInfo(),
                                                                     uno::UNO_QUERY );
                    if( xInfo.is() )
                    {
                        // for some reason the virtual method getInfoHelper() is
                        // not const
                        ::cppu::IPropertyArrayHelper & rPH =
                              const_cast< OPropertySet * >( this )->getInfoHelper();

                        // find the Property with Handle nHandle in Style
                        Sequence< beans::Property > aProps( xInfo->getProperties() );
                        sal_Int32 nI = aProps.getLength() - 1;
                        while( ( nI >= 0 ) && nHandle != aProps[ nI ].Handle )
                            --nI;

                        if( nI >= 0 ) // => nHandle == aProps[nI].Handle
                        {
                            // check whether the handle in this property set is
                            // the same as the one in the style
                            beans::Property aProp( rPH.getPropertyByName( aProps[ nI ].Name ) );
                            OSL_ENSURE( nHandle == aProp.Handle,
                                        "HandleCheck: Handles for same property differ!" );

                            if( nHandle == aProp.Handle )
                            {
                                OSL_ENSURE( aProp.Type == aProps[nI].Type,
                                            "HandleCheck: Types differ!" );
                                OSL_ENSURE( aProp.Attributes == aProps[nI].Attributes,
                                            "HandleCheck: Attributes differ!" );
                            }
                        }
                        else
                        {
                            OSL_ENSURE( false,  "HandleCheck: Handle not found in Style" );
                        }
                    }
                    else
                        OSL_ENSURE( false, "HandleCheck: Invalid XPropertySetInfo returned" );
                }
                else
                    OSL_ENSURE( false, "HandleCheck: XPropertySet not supported" );
            }
#endif
            rValue = xStylePropSet->getFastPropertyValue( nHandle );
        }
        else
        {
//             OSL_TRACE( "OPropertySet: no style => getting default for property" );
            // there is no style (or the style does not support XFastPropertySet)
            // => take the default value
            try
            {
                rValue = GetDefaultValue( nHandle );
            }
            catch( beans::UnknownPropertyException ex )
            {
                rValue.clear();
            }
        }
    }
}

// ____ XStyleSupplier ____
Reference< style::XStyle > SAL_CALL OPropertySet::getStyle()
    throw (uno::RuntimeException)
{
    return m_pImplProperties->GetStyle();
}

void SAL_CALL OPropertySet::setStyle( const Reference< style::XStyle >& xStyle )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    if( ! m_pImplProperties->SetStyle( xStyle ))
        throw lang::IllegalArgumentException(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "Empty Style" )),
            static_cast< beans::XPropertySet * >( this ),
            0 );
}

// ____ XFastPropertyState ____
// beans::PropertyState OPropertySet::SAL_CALL getFastPropertyState( sal_Int32 nHandle )
//     throw (beans::UnknownPropertyException,
//            uno::RuntimeException)
// {
//     return m_pImplProperties->GetPropertyStateByHandle( nHandle );
// }

// uno::Sequence< beans::PropertyState > OPropertySet::SAL_CALL getFastPropertyStates(
//     const uno::Sequence< sal_Int32 >& aHandles )
//     throw (beans::UnknownPropertyException,
//            uno::RuntimeException)
// {
//     ::std::vector< sal_Int32 > aHandleVec(
//         aHandles.getConstArray(),
//         aHandles.getConstArray() + aHandles.getLength() );

//     return m_pImplProperties->GetPropertyStatesByHandle( aHandleVec );
// }

// void OPropertySet::SAL_CALL setFastPropertyToDefault( sal_Int32 nHandle )
//     throw (beans::UnknownPropertyException,
//            uno::RuntimeException)
// {
//     m_pImplProperties->SetPropertyToDefault( nHandle );
// }

// uno::Any OPropertySet::SAL_CALL getFastPropertyDefault( sal_Int32 nHandle )
//     throw (beans::UnknownPropertyException,
//            lang::WrappedTargetException,
//            uno::RuntimeException)
// {
//     return GetDefaultValue( nHandle );
// }


} //  namespace property
