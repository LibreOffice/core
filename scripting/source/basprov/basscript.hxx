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

#ifndef INCLUDED_SCRIPTING_SOURCE_BASPROV_BASSCRIPT_HXX
#define INCLUDED_SCRIPTING_SOURCE_BASPROV_BASSCRIPT_HXX

#include "bcholder.hxx"
#include <com/sun/star/script/provider/XScript.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <basic/sbmeth.hxx>
#include <svl/lstner.hxx>

class BasicManager;


namespace basprov
{



    //  class BasicScriptImpl


    typedef ::cppu::WeakImplHelper<
        css::script::provider::XScript > BasicScriptImpl_BASE;


    class BasicScriptImpl : public BasicScriptImpl_BASE, public SfxListener,
                                public ::scripting_helper::OMutexHolder,
                                public ::scripting_helper::OBroadcastHelperHolder,
                                public ::comphelper::OPropertyContainer,
                                public ::comphelper::OPropertyArrayUsageHelper< BasicScriptImpl >
    {
    private:
        SbMethodRef         m_xMethod;
        OUString            m_funcName;
        BasicManager*       m_documentBasicManager;
        css::uno::Reference< css::document::XScriptInvocationContext >
                            m_xDocumentScriptContext;
        // hack, OPropertyContainer doesn't allow you to define a property of unknown
        // type ( I guess because an Any can't contain an Any... I've always wondered why?
    // as its not unusual to do that in corba )
        css::uno::Sequence< css::uno::Any > m_caller;
    protected:
        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper(  ) override;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper(  ) const override;

    public:
        BasicScriptImpl(
            const OUString& funcName,
            SbMethodRef xMethod
        );
        BasicScriptImpl(
            const OUString& funcName,
            SbMethodRef xMethod,
            BasicManager& documentBasicManager,
            const css::uno::Reference< css::document::XScriptInvocationContext >& documentScriptContext
        );
        virtual ~BasicScriptImpl();

        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XScript
        virtual css::uno::Any SAL_CALL invoke(
            const css::uno::Sequence< css::uno::Any >& aParams,
            css::uno::Sequence< sal_Int16 >& aOutParamIndex,
            css::uno::Sequence< css::uno::Any >& aOutParam )
            throw (
                    css::script::provider::ScriptFrameworkErrorException,
                    css::reflection::InvocationTargetException,
                    css::uno::RuntimeException, std::exception ) override;
        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
            throw (css::uno::RuntimeException, std::exception) override;

        // SfxListener
        virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    };


}   // namespace basprov


#endif // INCLUDED_SCRIPTING_SOURCE_BASPROV_BASSCRIPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
