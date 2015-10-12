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

#ifndef INCLUDED_COMPHELPER_PROPERTYSTATECONTAINER_HXX
#define INCLUDED_COMPHELPER_PROPERTYSTATECONTAINER_HXX

#include <comphelper/propertycontainer.hxx>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <osl/diagnose.h>
#include <comphelper/comphelperdllapi.h>

#include <map>


namespace comphelper
{



    //= OPropertyStateContainer

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


        // XPropertyState
        virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;


        // own overridables
        // these are the impl-methods for the XPropertyState members - they are implemented already by this class,
        // but you may want to override them for whatever reasons (for instance, if your derived class
        // supports the AMBIGUOUS state for properties)

        /** get the PropertyState of the property denoted by the given handle

            <p>Already implemented by this base class, no need to override</p>
            @precond <arg>_nHandle</arg> is a valid property handle
        */
        ::com::sun::star::beans::PropertyState  getPropertyStateByHandle( sal_Int32 _nHandle );

        /** set the property denoted by the given handle to it's default value

            <p>Already implemented by this base class, no need to override</p>
            @precond <arg>_nHandle</arg> is a valid property handle
        */
        void                                    setPropertyToDefaultByHandle( sal_Int32 _nHandle );

        /** get the default value for the property denoted by the given handle

            @precond
                <arg>_nHandle</arg> is a valid property handle
        */
        virtual void getPropertyDefaultByHandle( sal_Int32 _nHandle, ::com::sun::star::uno::Any& _rDefault ) const = 0;

    protected:
        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& _rType ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        // XTypeProvider
        DECLARE_XTYPEPROVIDER( )

    protected:
        /** returns the handle for the given name

            @throw UnknownPropertyException if the given name is not a registered property
        */
        sal_Int32   getHandleForName( const OUString& _rPropertyName );
    };


}   // namespace comphelper


#endif // INCLUDED_COMPHELPER_PROPERTYSTATECONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
