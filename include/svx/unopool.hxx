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

#ifndef INCLUDED_SVX_UNOPOOL_HXX
#define INCLUDED_SVX_UNOPOOL_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/propertysethelper.hxx>
#include <cppuhelper/implbase4.hxx>
#include <svx/svxdllapi.h>

class SdrModel;
class SfxItemPool;

/** This class implements the service com.sun.star.drawing.Defaults.
    It works on the SfxItemPool from the given model and the global
    draw object item pool.
    The class can work in a read only mode without a model. Derivated
    classes can set a model on demand by overiding getModelPool().
*/
class SVX_DLLPUBLIC SvxUnoDrawPool :    public ::cppu::OWeakAggObject,
                        public css::lang::XServiceInfo,
                        public css::lang::XTypeProvider,
                        public comphelper::PropertySetHelper
{
public:
    SvxUnoDrawPool(SdrModel* pModel, sal_Int32 nServiceId);

    /** deprecated */
    SvxUnoDrawPool(SdrModel* pModel);
    virtual ~SvxUnoDrawPool() throw();

    /** This returns the item pool from the given model, or the default pool if there is no model and bReadOnly is true.
        If bReadOnly is false and there is no model the default implementation returns NULL.
    */
    virtual SfxItemPool* getModelPool( bool bReadOnly ) throw();

    // overriden helpers from comphelper::PropertySetHelper
    virtual void _setPropertyValues( const comphelper::PropertyMapEntry** ppEntries, const css::uno::Any* pValues ) throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException ) override;
    virtual void _getPropertyValues( const comphelper::PropertyMapEntry** ppEntries, css::uno::Any* pValue ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException ) override;

    virtual void _getPropertyStates( const comphelper::PropertyMapEntry** ppEntries, css::beans::PropertyState* pStates ) throw(css::beans::UnknownPropertyException ) override;
    virtual void _setPropertyToDefault( const comphelper::PropertyMapEntry* pEntry )  throw(css::beans::UnknownPropertyException ) override;
    virtual css::uno::Any _getPropertyDefault( const comphelper::PropertyMapEntry* pEntry ) throw(css::beans::UnknownPropertyException, css::lang::WrappedTargetException ) override;

    // XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

protected:
    void init();

    static void getAny( SfxItemPool* pPool, const comphelper::PropertyMapEntry* pEntry, css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException);
    virtual void putAny( SfxItemPool* pPool, const comphelper::PropertyMapEntry* pEntry, const css::uno::Any& rValue ) throw(css::beans::UnknownPropertyException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception);

protected:
    SdrModel* mpModel;
    SfxItemPool* mpDefaultsPool;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
