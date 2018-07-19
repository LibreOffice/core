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

#ifndef INCLUDED_COMPHELPER_MASTERPROPERTYSET_HXX
#define INCLUDED_COMPHELPER_MASTERPROPERTYSET_HXX
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <comphelper/PropertyInfoHash.hxx>
#include <comphelper/comphelperdllapi.h>
#include <comphelper/solarmutex.hxx>
#include <rtl/ref.hxx>
#include <map>

namespace comphelper
{
    class MasterPropertySetInfo;
    class ChainablePropertySet;
    struct SlaveData
    {
        rtl::Reference < ChainablePropertySet > mxSlave;
        bool                                    mbInit;

        SlaveData ( ChainablePropertySet *pSlave);
        bool IsInit () const { return mbInit;}
        void SetInit ( bool bInit) { mbInit = bInit; }
    };
}

/*
 * A MasterPropertySet implements all of the features of a ChainablePropertySet
 * (it is not inherited from ChainablePropertySet to prevent MasterPropertySets
 * being chained to each other), but also allows properties implemented in
 * other ChainablePropertySets to be included as 'native' properties in a
 * given MasterPropertySet implementation. These are registered using the
 * 'registerSlave' method, and require that the implementation of the
 * ChainablePropertySet and the implementation of the ChainablePropertySetInfo
 * both declare the implementation of the MasterPropertySet as a friend.
 */
namespace comphelper
{
    class COMPHELPER_DLLPUBLIC MasterPropertySet : public css::beans::XPropertySet,
                              public css::beans::XPropertyState,
                              public css::beans::XMultiPropertySet
    {
    protected:
        SolarMutex* const mpMutex;
        sal_uInt8 mnLastId;
        std::map< sal_uInt8, comphelper::SlaveData* >  maSlaveMap;
        rtl::Reference< MasterPropertySetInfo >        mxInfo;

        /// @throws css::beans::UnknownPropertyException
        /// @throws css::beans::PropertyVetoException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::lang::WrappedTargetException
        virtual void _preSetValues () = 0;
        /// @throws css::beans::UnknownPropertyException
        /// @throws css::beans::PropertyVetoException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::lang::WrappedTargetException
        /// @throws css::uno::RuntimeException
        virtual void _setSingleValue( const comphelper::PropertyInfo & rInfo, const css::uno::Any &rValue ) = 0;
        /// @throws css::beans::UnknownPropertyException
        /// @throws css::beans::PropertyVetoException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::lang::WrappedTargetException
        virtual void _postSetValues () = 0;

        /// @throws css::beans::UnknownPropertyException
        /// @throws css::beans::PropertyVetoException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::lang::WrappedTargetException
        virtual void _preGetValues () = 0;
        /// @throws css::beans::UnknownPropertyException
        /// @throws css::lang::WrappedTargetException
        /// @throws css::uno::RuntimeException
        virtual void _getSingleValue( const comphelper::PropertyInfo & rInfo, css::uno::Any & rValue ) = 0;
        /// @throws css::beans::UnknownPropertyException
        /// @throws css::beans::PropertyVetoException
        /// @throws css::lang::IllegalArgumentException
        /// @throws css::lang::WrappedTargetException
        virtual void _postGetValues () = 0;

    public:
        MasterPropertySet( comphelper::MasterPropertySetInfo* pInfo, SolarMutex* pMutex )
            throw();
        virtual ~MasterPropertySet()
            throw();
        void registerSlave ( ChainablePropertySet *pNewSet )
            throw();

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
        virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
        virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
        virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
        virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
        virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
        virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

        // XMultiPropertySet
        virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;
        virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;
        virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
        virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
        virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

        // XPropertyState
        virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
        virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
        virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
        virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
