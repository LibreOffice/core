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

#ifndef INCLUDED_SC_INC_FUNCUNO_HXX
#define INCLUDED_SC_INC_FUNCUNO_HXX

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/XFunctionAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/implbase.hxx>
#include <svl/lstner.hxx>

class ScDocument;
class ScDocOptions;

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
    ScFunctionAccess_CreateInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory >& );

class ScTempDocCache
{
private:
    ScDocument*     pDoc;
    bool            bInUse;

public:
                ScTempDocCache();
                ~ScTempDocCache();

    ScDocument* GetDocument() const     { return pDoc; }
    bool        IsInUse() const         { return bInUse; }
    void        SetInUse( bool bSet )   { bInUse = bSet; }

    void        SetDocument( ScDocument* pNew );
    void        Clear();
};

class ScFunctionAccess : public cppu::WeakImplHelper<
                                        com::sun::star::sheet::XFunctionAccess,
                                        com::sun::star::beans::XPropertySet,
                                        com::sun::star::lang::XServiceInfo>,
                         public SfxListener
{
private:
    ScTempDocCache  aDocCache;
    ScDocOptions*   pOptions;
    SfxItemPropertyMap aPropertyMap;
    bool            mbArray;
    bool            mbValid;

public:
                            ScFunctionAccess();
    virtual                 ~ScFunctionAccess();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XFunctionAccess
    virtual ::com::sun::star::uno::Any SAL_CALL callFunction(
                                    const OUString& aName,
                                    const ::com::sun::star::uno::Sequence<
                                        ::com::sun::star::uno::Any >& aArguments )
                                throw (::com::sun::star::container::NoSuchElementException,
                                       ::com::sun::star::lang::IllegalArgumentException,
                                       ::com::sun::star::uno::RuntimeException,
                                       std::exception) override;

                            // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Any& aValue )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::beans::PropertyVetoException,
                                    ::com::sun::star::lang::IllegalArgumentException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue(
                                    const OUString& PropertyName )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& xListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XPropertyChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::beans::XVetoableChangeListener >& aListener )
                                throw(::com::sun::star::beans::UnknownPropertyException,
                                    ::com::sun::star::lang::WrappedTargetException,
                                    ::com::sun::star::uno::RuntimeException, std::exception) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
