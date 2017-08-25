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

#include <memory>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/XFunctionAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/implbase.hxx>
#include <svl/lstner.hxx>
#include "document.hxx"

class ScDocOptions;

css::uno::Reference< css::uno::XInterface > SAL_CALL
    ScFunctionAccess_CreateInstance(
        const css::uno::Reference< css::lang::XMultiServiceFactory >& );

class ScTempDocCache
{
private:
    ScDocumentUniquePtr xDoc;
    bool            bInUse;

public:
    ScTempDocCache();

    ScDocument* GetDocument() const     { return xDoc.get(); }
    bool        IsInUse() const         { return bInUse; }
    void        SetInUse( bool bSet )   { bInUse = bSet; }

    void        SetDocument( ScDocument* pNew );
    void        Clear();
};

class ScFunctionAccess : public cppu::WeakImplHelper<
                                        css::sheet::XFunctionAccess,
                                        css::beans::XPropertySet,
                                        css::lang::XServiceInfo>,
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
    virtual                 ~ScFunctionAccess() override;

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            // XFunctionAccess
    virtual css::uno::Any SAL_CALL callFunction(
                                    const OUString& aName,
                                    const css::uno::Sequence< css::uno::Any >& aArguments ) override;

                            // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo >
                            SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL   setPropertyValue( const OUString& aPropertyName,
                                    const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL   addPropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL   removePropertyChangeListener( const OUString& aPropertyName,
                                    const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL   addVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL   removeVetoableChangeListener( const OUString& PropertyName,
                                    const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

                            // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
