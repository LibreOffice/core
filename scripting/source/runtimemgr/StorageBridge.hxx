/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
************************************************************************/


#ifndef _FRAMEWORK_SCRIPT_STORAGEBRIDGE_HXX_
#define _FRAMEWORK_SCRIPT_STORAGEBRIDGE_HXX_

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <drafts/com/sun/star/script/framework/storage/XScriptInfoAccess.hpp>
#include "StorageBridgeFactory.hxx"

namespace scripting_runtimemgr
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

class StorageBridge : public ::cppu::WeakImplHelper1< dcsssf::storage::XScriptInfoAccess >
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
    virtual css::uno::Sequence< css::uno::Reference< dcsssf::storage::XScriptInfo > >
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
    css::uno::Reference< dcsssf::storage::XScriptInfoAccess > m_xScriptInfoAccess;
    sal_Int32 m_sid;
};
}

#endif //_COM_SUN_STAR_SCRIPTING_STORAGEBRIDGE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
