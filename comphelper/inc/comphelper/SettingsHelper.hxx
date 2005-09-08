/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SettingsHelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:24:06 $
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

#ifndef _COMPHELPER_SETTINGSHELPER_HXX_
#define _COMPHELPER_SETTINGSHELPER_HXX_
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
namespace vos
{
    class IMutex;
}

namespace comphelper
{
    class MasterPropertySet;
    class MasterPropertySetInfo;
    class ChainablePropertySet;
    class ChainablePropertySetInfo;

    typedef  cppu::WeakImplHelper3
    <
        ::com::sun::star::beans::XPropertySet,
        ::com::sun::star::beans::XMultiPropertySet,
        ::com::sun::star::lang::XServiceInfo
    >
    HelperBaseNoState;
    template < class ComphelperBase, class ComphelperBaseInfo > class SettingsHelperNoState :
        public HelperBaseNoState,
        public ComphelperBase
    {
    public:
        SettingsHelperNoState ( ComphelperBaseInfo *pInfo, ::vos::IMutex *pMutex = NULL)
        : ComphelperBase ( pInfo, pMutex )
        {}
        virtual ~SettingsHelperNoState () throw( ) {}
        com::sun::star::uno::Any SAL_CALL queryInterface( const com::sun::star::uno::Type& aType ) throw (com::sun::star::uno::RuntimeException)
        { return HelperBaseNoState::queryInterface( aType ); }
        void SAL_CALL acquire(  ) throw ()
        { HelperBaseNoState::acquire( ); }
        void SAL_CALL release(  ) throw ()
        { HelperBaseNoState::release( ); }

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
            throw(::com::sun::star::uno::RuntimeException)
        { return ComphelperBase::getPropertySetInfo(); }
        virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
        { ComphelperBase::setPropertyValue ( aPropertyName, aValue ); }
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
        { return ComphelperBase::getPropertyValue ( PropertyName ); }
        virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
        { ComphelperBase::addPropertyChangeListener ( aPropertyName, xListener ); }
        virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
        { ComphelperBase::removePropertyChangeListener ( aPropertyName, aListener ); }
        virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
        { ComphelperBase::addVetoableChangeListener ( PropertyName, aListener ); }
        virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
        { ComphelperBase::removeVetoableChangeListener ( PropertyName, aListener ); }

        // XMultiPropertySet
        virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues )
            throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException)
        { ComphelperBase::setPropertyValues ( aPropertyNames, aValues ); }
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames )
            throw(::com::sun::star::uno::RuntimeException)
        { return ComphelperBase::getPropertyValues ( aPropertyNames ); }
        virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException)
        { ComphelperBase::addPropertiesChangeListener ( aPropertyNames, xListener ); }
        virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException)
        { ComphelperBase::removePropertiesChangeListener ( xListener ); }
        virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener )
            throw(::com::sun::star::uno::RuntimeException)
        { ComphelperBase::firePropertiesChangeEvent ( aPropertyNames, xListener ); }
    };
    typedef comphelper::SettingsHelperNoState
    <
        ::comphelper::MasterPropertySet,
        ::comphelper::MasterPropertySetInfo
    >
    MasterHelperNoState;
    typedef comphelper::SettingsHelperNoState
    <
        ::comphelper::ChainablePropertySet,
        ::comphelper::ChainablePropertySetInfo
    >
    ChainableHelperNoState;
}

#endif
