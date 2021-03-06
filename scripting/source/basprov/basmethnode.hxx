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

#pragma once

#include <bcholder.hxx>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase.hxx>


class SbMethod;


namespace basprov
{




    typedef ::cppu::WeakImplHelper<
        css::script::browse::XBrowseNode,
        css::script::XInvocation > BasicMethodNodeImpl_BASE;

    class BasicMethodNodeImpl : public BasicMethodNodeImpl_BASE,
                                public ::scripting_helper::OMutexHolder,
                                public ::scripting_helper::OBroadcastHelperHolder,
                                public ::comphelper::OPropertyContainer,
                                public ::comphelper::OPropertyArrayUsageHelper< BasicMethodNodeImpl >
    {
    private:
        css::uno::Reference< css::uno::XComponentContext >    m_xContext;
        OUString m_sScriptingContext;
        SbMethod* m_pMethod;
        bool m_bIsAppScript;

        // properties
        OUString m_sURI;
        bool m_bEditable;

    protected:
        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper(  ) override;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper(  ) const override;

    public:
        BasicMethodNodeImpl( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            const OUString& sScriptingContext,
            SbMethod* pMethod, bool isAppScript );
        virtual ~BasicMethodNodeImpl() override;

        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XBrowseNode
        virtual OUString SAL_CALL getName(  ) override;
        virtual css::uno::Sequence< css::uno::Reference< css::script::browse::XBrowseNode > > SAL_CALL getChildNodes(  ) override;
        virtual sal_Bool SAL_CALL hasChildNodes(  ) override;
        virtual sal_Int16 SAL_CALL getType(  ) override;

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

        // XInvocation
        virtual css::uno::Reference< css::beans::XIntrospectionAccess > SAL_CALL getIntrospection(  ) override;
        virtual css::uno::Any SAL_CALL invoke(
            const OUString& aFunctionName,
            const css::uno::Sequence< css::uno::Any >& aParams,
            css::uno::Sequence< sal_Int16 >& aOutParamIndex,
            css::uno::Sequence< css::uno::Any >& aOutParam ) override;
        virtual void SAL_CALL setValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
        virtual css::uno::Any SAL_CALL getValue( const OUString& aPropertyName ) override;
        virtual sal_Bool SAL_CALL hasMethod( const OUString& aName ) override;
        virtual sal_Bool SAL_CALL hasProperty( const OUString& aName ) override;
    };


}   // namespace basprov


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
