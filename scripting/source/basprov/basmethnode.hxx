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

#ifndef INCLUDED_SCRIPTING_SOURCE_BASPROV_BASMETHNODE_HXX
#define INCLUDED_SCRIPTING_SOURCE_BASPROV_BASMETHNODE_HXX

#include "bcholder.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
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



    //  class BasicMethodNodeImpl


    typedef ::cppu::WeakImplHelper<
        ::com::sun::star::script::browse::XBrowseNode,
        ::com::sun::star::script::XInvocation > BasicMethodNodeImpl_BASE;

    class BasicMethodNodeImpl : public BasicMethodNodeImpl_BASE,
                                public ::scripting_helper::OMutexHolder,
                                public ::scripting_helper::OBroadcastHelperHolder,
                                public ::comphelper::OPropertyContainer,
                                public ::comphelper::OPropertyArrayUsageHelper< BasicMethodNodeImpl >
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    m_xContext;
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
        BasicMethodNodeImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
            const OUString& sScriptingContext,
            SbMethod* pMethod, bool isAppScript = true );
        virtual ~BasicMethodNodeImpl();

        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XBrowseNode
        virtual OUString SAL_CALL getName(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::script::browse::XBrowseNode > > SAL_CALL getChildNodes(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasChildNodes(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Int16 SAL_CALL getType(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XInvocation
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess > SAL_CALL getIntrospection(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Any SAL_CALL invoke(
            const OUString& aFunctionName,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aParams,
            ::com::sun::star::uno::Sequence< sal_Int16 >& aOutParamIndex,
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aOutParam )
            throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::script::CannotConvertException,
                   ::com::sun::star::reflection::InvocationTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue )
            throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::script::CannotConvertException,
                   ::com::sun::star::reflection::InvocationTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual ::com::sun::star::uno::Any SAL_CALL getValue( const OUString& aPropertyName )
            throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasMethod( const OUString& aName )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasProperty( const OUString& aName )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    };


}   // namespace basprov


#endif // INCLUDED_SCRIPTING_SOURCE_BASPROV_BASMETHNODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
