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
        ::com::sun::star::script::provider::XScript > BasicScriptImpl_BASE;


    class BasicScriptImpl : public BasicScriptImpl_BASE, public SfxListener,
                                public ::scripting_helper::OMutexHolder,
                                public ::scripting_helper::OBroadcastHelperHolder,
                                public ::comphelper::OPropertyContainer,
                                public ::comphelper::OPropertyArrayUsageHelper< BasicScriptImpl >
    {
    private:
        SbMethodRef         m_xMethod;
        OUString     m_funcName;
        BasicManager*       m_documentBasicManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::document::XScriptInvocationContext >
                            m_xDocumentScriptContext;
        // hack, OPropertyContainer doesn't allow you to define a property of unknown
        // type ( I guess because an Any can't contain an Any... I've always wondered why?
    // as its not unusual to do that in corba )
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > m_caller;
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
            const ::com::sun::star::uno::Reference< ::com::sun::star::document::XScriptInvocationContext >& documentScriptContext
        );
        virtual ~BasicScriptImpl();

        // XInterface
        DECLARE_XINTERFACE()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XScript
        virtual ::com::sun::star::uno::Any SAL_CALL invoke(
            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aParams,
            ::com::sun::star::uno::Sequence< sal_Int16 >& aOutParamIndex,
            ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aOutParam )
            throw (
                    ::com::sun::star::script::provider::ScriptFrameworkErrorException,
                    ::com::sun::star::reflection::InvocationTargetException,
                    ::com::sun::star::uno::RuntimeException, std::exception ) override;
        // XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  )
            throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // SfxListener
        virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    };


}   // namespace basprov


#endif // INCLUDED_SCRIPTING_SOURCE_BASPROV_BASSCRIPT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
