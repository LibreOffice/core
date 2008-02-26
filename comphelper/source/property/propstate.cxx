/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propstate.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 15:16:21 $
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
#include "precompiled_comphelper.hxx"

#ifndef _COMPHELPER_PROPERTY_STATE_HXX_
#include <comphelper/propstate.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_GENFUNC_H_
#include <com/sun/star/uno/genfunc.h>
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::lang::XTypeProvider;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::cpp_queryInterface;
    using ::com::sun::star::uno::cpp_release;
    using ::com::sun::star::beans::PropertyState_DEFAULT_VALUE;
    using ::com::sun::star::beans::PropertyState_DIRECT_VALUE;

    //=====================================================================
    // OPropertyStateHelper
    //=====================================================================

    //---------------------------------------------------------------------
    ::com::sun::star::uno::Any SAL_CALL OPropertyStateHelper::queryInterface(const  ::com::sun::star::uno::Type& _rType) throw( ::com::sun::star::uno::RuntimeException)
    {
        ::com::sun::star::uno::Any aReturn = OPropertySetHelper::queryInterface(_rType);
        // our own ifaces
        if ( !aReturn.hasValue() )
            aReturn = ::cppu::queryInterface(_rType, static_cast< ::com::sun::star::beans::XPropertyState*>(this));

        return aReturn;
    }

    //---------------------------------------------------------------------
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> OPropertyStateHelper::getTypes() throw( ::com::sun::star::uno::RuntimeException)
    {
        static  ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> aTypes;
        if (!aTypes.getLength())
        {
            aTypes.realloc(4);
            ::com::sun::star::uno::Type* pTypes = aTypes.getArray();
            // base class types
            pTypes[0] = getCppuType(( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>*)NULL);
            pTypes[1] = getCppuType(( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet>*)NULL);
            pTypes[2] = getCppuType(( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet>*)NULL);
            // my own type
            pTypes[3] = getCppuType(( ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState>*)NULL);
        }
        return aTypes;
    }

    OPropertyStateHelper::OPropertyStateHelper(
        ::cppu::OBroadcastHelper& rBHlp,
        ::cppu::IEventNotificationHook *i_pFireEvents)
        :   ::cppu::OPropertySetHelper(rBHlp, i_pFireEvents) { }

    OPropertyStateHelper::~OPropertyStateHelper() {}

    //---------------------------------------------------------------------
    void OPropertyStateHelper::firePropertyChange(sal_Int32 nHandle, const  ::com::sun::star::uno::Any& aNewValue, const  ::com::sun::star::uno::Any& aOldValue)
    {
        fire(&nHandle, &aNewValue, &aOldValue, 1, sal_False);
    }

    // XPropertyState
    //---------------------------------------------------------------------
    ::com::sun::star::beans::PropertyState SAL_CALL OPropertyStateHelper::getPropertyState(const ::rtl::OUString& _rsName) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::uno::RuntimeException)
    {
        cppu::IPropertyArrayHelper& rPH = getInfoHelper();
        sal_Int32 nHandle = rPH.getHandleByName(_rsName);

        if (nHandle == -1)
            throw  ::com::sun::star::beans::UnknownPropertyException();

        return getPropertyStateByHandle(nHandle);
    }

    //---------------------------------------------------------------------
    void SAL_CALL OPropertyStateHelper::setPropertyToDefault(const ::rtl::OUString& _rsName) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::uno::RuntimeException)
    {
        cppu::IPropertyArrayHelper& rPH = getInfoHelper();
        sal_Int32 nHandle = rPH.getHandleByName(_rsName);

        if (nHandle == -1)
            throw  ::com::sun::star::beans::UnknownPropertyException();

        setPropertyToDefaultByHandle(nHandle);
    }

    //---------------------------------------------------------------------
    ::com::sun::star::uno::Any SAL_CALL OPropertyStateHelper::getPropertyDefault(const ::rtl::OUString& _rsName) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::lang::WrappedTargetException,  ::com::sun::star::uno::RuntimeException)
    {
        cppu::IPropertyArrayHelper& rPH = getInfoHelper();
        sal_Int32 nHandle = rPH.getHandleByName(_rsName);

        if (nHandle == -1)
            throw  ::com::sun::star::beans::UnknownPropertyException();

        return getPropertyDefaultByHandle(nHandle);
    }

    //---------------------------------------------------------------------
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState> SAL_CALL OPropertyStateHelper::getPropertyStates(const  ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rPropertyNames) throw( ::com::sun::star::beans::UnknownPropertyException,  ::com::sun::star::uno::RuntimeException)
    {
        sal_Int32 nLen = _rPropertyNames.getLength();
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState> aRet(nLen);
        ::com::sun::star::beans::PropertyState* pValues = aRet.getArray();
        const ::rtl::OUString* pNames = _rPropertyNames.getConstArray();

        cppu::IPropertyArrayHelper& rHelper = getInfoHelper();

        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property> aProps = rHelper.getProperties();
        const  ::com::sun::star::beans::Property* pProps = aProps.getConstArray();
        sal_Int32 nPropCount       = aProps.getLength();

        osl::MutexGuard aGuard(rBHelper.rMutex);
        for (sal_Int32 i=0, j=0; i<nPropCount && j<nLen; ++i, ++pProps)
        {
            // get the values only for valid properties
            if (pProps->Name.equals(*pNames))
            {
                *pValues = getPropertyState(*pNames);
                ++pValues;
                ++pNames;
                ++j;
            }
        }

        return aRet;
    }

    //---------------------------------------------------------------------
    ::com::sun::star::beans::PropertyState OPropertyStateHelper::getPropertyStateByHandle( sal_Int32 _nHandle )
    {
        // simply compare the current and the default value
        Any aCurrentValue = getPropertyDefaultByHandle( _nHandle );
        Any aDefaultValue;  getFastPropertyValue( aDefaultValue, _nHandle );

        sal_Bool bEqual = uno_type_equalData(
                const_cast< void* >( aCurrentValue.getValue() ), aCurrentValue.getValueType().getTypeLibType(),
                const_cast< void* >( aDefaultValue.getValue() ), aDefaultValue.getValueType().getTypeLibType(),
                reinterpret_cast< uno_QueryInterfaceFunc >(cpp_queryInterface),
                reinterpret_cast< uno_ReleaseFunc >(cpp_release)
            );
        return bEqual ? PropertyState_DEFAULT_VALUE : PropertyState_DIRECT_VALUE;
    }

    //---------------------------------------------------------------------
    void OPropertyStateHelper::setPropertyToDefaultByHandle( sal_Int32 _nHandle )
    {
        setFastPropertyValue( _nHandle, getPropertyDefaultByHandle( _nHandle ) );
    }

    //---------------------------------------------------------------------
    ::com::sun::star::uno::Any OPropertyStateHelper::getPropertyDefaultByHandle( sal_Int32 ) const
    {
        return  ::com::sun::star::uno::Any();
    }

    //=====================================================================
    // OStatefulPropertySet
    //=====================================================================
    //---------------------------------------------------------------------
    OStatefulPropertySet::OStatefulPropertySet()
        :OPropertyStateHelper( GetBroadcastHelper() )
    {
    }

    //---------------------------------------------------------------------
    OStatefulPropertySet::~OStatefulPropertySet()
    {
    }

    //---------------------------------------------------------------------
    Sequence< Type > SAL_CALL OStatefulPropertySet::getTypes() throw(RuntimeException)
    {
        Sequence< Type > aOwnTypes( 2 );
        aOwnTypes[0] = XWeak::static_type();
        aOwnTypes[1] = XTypeProvider::static_type();

        return concatSequences(
            aOwnTypes,
            OPropertyStateHelper::getTypes()
        );
    }

    //---------------------------------------------------------------------
    Sequence< sal_Int8 > SAL_CALL OStatefulPropertySet::getImplementationId() throw(RuntimeException)
    {
        static ::cppu::OImplementationId * pId = NULL;
        if ( !pId )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !pId )
            {
                static ::cppu::OImplementationId aId;
                pId = &aId;
            }
        }
        return pId->getImplementationId();
    }

    //---------------------------------------------------------------------
    Any SAL_CALL OStatefulPropertySet::queryInterface( const Type& _rType ) throw(RuntimeException)
    {
        Any aReturn = OWeakObject::queryInterface( _rType );
        if ( !aReturn.hasValue() )
            aReturn = ::cppu::queryInterface( _rType, static_cast< XTypeProvider* >( this ) );
        if ( !aReturn.hasValue() )
            aReturn = OPropertyStateHelper::queryInterface( _rType );
        return aReturn;
    }

    //---------------------------------------------------------------------
    void SAL_CALL OStatefulPropertySet::acquire() throw()
    {
        ::cppu::OWeakObject::acquire();
    }

    //---------------------------------------------------------------------
    void SAL_CALL OStatefulPropertySet::release() throw()
    {
        ::cppu::OWeakObject::release();
    }

//.........................................................................
}
//.........................................................................

