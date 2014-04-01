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
#ifndef _FRAMEWORK_SCRIPT_PROVIDER_OPENDOCUMENTLIST_HXX_
#define _FRAMEWORK_SCRIPT_PROVIDER_OPENDOCUMENTLIST_HXX_

#include <boost/unordered_map.hpp>
#include <map>

#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/lang/XEventListener.hpp>

#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/script/browse/XBrowseNode.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>

#include <comphelper/stl_types.hxx>

namespace func_provider
{

//Typedefs



typedef ::std::map  <   css::uno::Reference< css::uno::XInterface >
                    ,   css::uno::Reference< css::script::provider::XScriptProvider >
                    ,   ::comphelper::OInterfaceCompare< css::uno::XInterface >
                    >   ScriptComponent_map;

typedef ::boost::unordered_map< OUString,
    css::uno::Reference< css::script::provider::XScriptProvider >,
    OUStringHash,
            ::std::equal_to< OUString > > Msp_hash;

class ActiveMSPList : public ::cppu::WeakImplHelper1< css::lang::XEventListener  >
{

public:

    ActiveMSPList(  const css::uno::Reference<
        css::uno::XComponentContext > & xContext  );
    virtual ~ActiveMSPList();

    css::uno::Reference< css::script::provider::XScriptProvider >
        getMSPFromStringContext( const OUString& context )
            SAL_THROW(( css::lang::IllegalArgumentException, css::uno::RuntimeException ));

    css::uno::Reference< css::script::provider::XScriptProvider >
        getMSPFromAnyContext( const css::uno::Any& context )
            SAL_THROW(( css::lang::IllegalArgumentException, css::uno::RuntimeException ));

    css::uno::Reference< css::script::provider::XScriptProvider >
        getMSPFromInvocationContext( const css::uno::Reference< css::document::XScriptInvocationContext >& context )
            SAL_THROW(( css::lang::IllegalArgumentException, css::uno::RuntimeException ));

    //XEventListener


    virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
        throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

private:
    void addActiveMSP( const css::uno::Reference< css::uno::XInterface >& xComponent,
                       const css::uno::Reference< css::script::provider::XScriptProvider >& msp );
    css::uno::Reference< css::script::provider::XScriptProvider >
        createNewMSP( const css::uno::Any& context );
    css::uno::Reference< css::script::provider::XScriptProvider >
        createNewMSP( const OUString& context )
    {
        return createNewMSP( css::uno::makeAny( context ) );
    }

    void createNonDocMSPs();
    Msp_hash m_hMsps;
    ScriptComponent_map m_mScriptComponents;
    osl::Mutex m_mutex;
    OUString userDirString;
    OUString shareDirString;
    OUString bundledDirString;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};
} // func_provider
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
