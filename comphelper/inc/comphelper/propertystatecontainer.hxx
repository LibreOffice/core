/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: propertystatecontainer.hxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef COMPHELPER_PROPERTYSTATECONTAINER_HXX
#define COMPHELPER_PROPERTYSTATECONTAINER_HXX

#include <comphelper/propertycontainer.hxx>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <osl/diagnose.h>
#include "comphelper/comphelperdllapi.h"

#include <map>

//.........................................................................
namespace comphelper
{
//.........................................................................

    //=====================================================================
    //= OPropertyStateContainer
    //=====================================================================
    typedef ::cppu::ImplHelper1 <   ::com::sun::star::beans::XPropertyState
                                >   OPropertyStateContainer_TBase;

    /** helper implementation for components which have properties with a default

        <p>This class is not intended for direct use, you need to derive from it.</p>

        @see com.sun.star.beans.XPropertyState
    */
    class COMPHELPER_DLLPUBLIC OPropertyStateContainer
                :public  OPropertyContainer
                ,public  OPropertyStateContainer_TBase
    {
    protected:
        /** ctor
            @param _rBHelper
                help to be used for broadcasting events
        */
        OPropertyStateContainer( ::cppu::OBroadcastHelper&  _rBHelper );

        // ................................................................
        // XPropertyState
        virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // ................................................................
        // own overridables
        // these are the impl-methods for the XPropertyState members - they are implemented already by this class,
        // but you may want to override them for whatever reasons (for instance, if your derived class
        // supports the AMBIGUOUS state for properties)

        /** get the PropertyState of the property denoted by the given handle

            <p>Already implemented by this base class, no need to override</p>
            @precond <arg>_nHandle</arg> is a valid property handle
        */
        virtual ::com::sun::star::beans::PropertyState  getPropertyStateByHandle( sal_Int32 _nHandle );

        /** set the property denoted by the given handle to it's default value

            <p>Already implemented by this base class, no need to override</p>
            @precond <arg>_nHandle</arg> is a valid property handle
        */
        virtual void                                    setPropertyToDefaultByHandle( sal_Int32 _nHandle );

        /** get the default value for the property denoted by the given handle

            @precond
                <arg>_nHandle</arg> is a valid property handle
        */
        virtual void getPropertyDefaultByHandle( sal_Int32 _nHandle, ::com::sun::star::uno::Any& _rDefault ) const = 0;

    protected:
        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException);
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

    protected:
        /** returns the handle for the given name

            @throw UnknownPropertyException if the given name is not a registered property
        */
        sal_Int32   getHandleForName( const ::rtl::OUString& _rPropertyName ) SAL_THROW( ( ::com::sun::star::beans::UnknownPropertyException ) );
    };

//.........................................................................
}   // namespace comphelper
//.........................................................................

#endif // COMPHELPER_PROPERTYSTATECONTAINER_HXX
