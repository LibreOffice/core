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


#ifndef _FRAMEWORK_SCRIPT_STORAGEBRIDGE_HXX_
#define _FRAMEWORK_SCRIPT_STORAGEBRIDGE_HXX_

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <drafts/com/sun/star/script/framework/storage/XScriptInfoAccess.hpp>
#include "StorageBridgeFactory.hxx"

namespace css = ::com::sun::star;

namespace scripting_runtimemgr
{

class StorageBridge : public ::cppu::WeakImplHelper1< ::drafts::com::sun::star::script::framework::storage::XScriptInfoAccess >
{
    friend class StorageBridgeFactory;
public:
    //XScriptInfoAccess
    //=========================================================================
    /**
     * Get the implementations for a given URI
     *
     * @param queryURI
     *      The URI to get the implementations for
     *
     * @return XScriptURI
     *      The URIs of the implementations
     */
    virtual css::uno::Sequence< css::uno::Reference< ::drafts::com::sun::star::script::framework::storage::XScriptInfo > >
        SAL_CALL getImplementations(
            const ::rtl::OUString& queryURI )
        throw ( css::lang::IllegalArgumentException,
            css::uno::RuntimeException );
    //=========================================================================
    /**
     * Get the all logical names stored in this storage
     *
     * @return sequence < ::rtl::OUString >
     *      The logical names
     */
    virtual css::uno::Sequence< ::rtl::OUString >
        SAL_CALL getScriptLogicalNames()
        throw ( css::lang::IllegalArgumentException, css::uno::RuntimeException );
private:
    StorageBridge( const css::uno::Reference< css::uno::XComponentContext >& xContext,
        sal_Int32 sid );

    void initStorage() throw ( css::uno::RuntimeException );
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< ::drafts::com::sun::star::script::framework::storage::XScriptInfoAccess > m_xScriptInfoAccess;
    sal_Int32 m_sid;
};
}

#endif //_COM_SUN_STAR_SCRIPTING_STORAGEBRIDGE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
