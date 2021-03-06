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

#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XEventListener.hpp>

#include <com/sun/star/script/provider/XScriptProvider.hpp>
#include <com/sun/star/document/XScriptInvocationContext.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <map>
#include <unordered_map>

namespace func_provider
{

//Typedefs
typedef std::map  <   css::uno::Reference< css::uno::XInterface >
                  ,   css::uno::Reference< css::script::provider::XScriptProvider >
                  >   ScriptComponent_map;

typedef std::unordered_map< OUString,
    css::uno::Reference< css::script::provider::XScriptProvider > > Msp_hash;

class NonDocMSPCreator;

class ActiveMSPList : public ::cppu::WeakImplHelper< css::lang::XEventListener  >
{

public:

    explicit ActiveMSPList(  const css::uno::Reference<
        css::uno::XComponentContext > & xContext  );
    virtual ~ActiveMSPList() override;

    css::uno::Reference< css::script::provider::XScriptProvider >
        getMSPFromStringContext( const OUString& context );

    css::uno::Reference< css::script::provider::XScriptProvider >
        getMSPFromAnyContext( const css::uno::Any& context );

    css::uno::Reference< css::script::provider::XScriptProvider >
        getMSPFromInvocationContext( const css::uno::Reference< css::document::XScriptInvocationContext >& context );

    //XEventListener


    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

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

    friend class NonDocMSPCreator;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
