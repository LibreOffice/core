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
// for simplification
#define css ::com::sun::star

//Typedefs
//=============================================================================


typedef ::std::map  <   css::uno::Reference< css::uno::XInterface >
                    ,   css::uno::Reference< css::script::provider::XScriptProvider >
                    ,   ::comphelper::OInterfaceCompare< css::uno::XInterface >
                    >   ScriptComponent_map;

typedef ::boost::unordered_map< ::rtl::OUString,
    css::uno::Reference< css::script::provider::XScriptProvider >,
    ::rtl::OUStringHash,
            ::std::equal_to< ::rtl::OUString > > Msp_hash;

class ActiveMSPList : public ::cppu::WeakImplHelper1< css::lang::XEventListener  >
{

public:

    ActiveMSPList(  const css::uno::Reference<
        css::uno::XComponentContext > & xContext  );
    ~ActiveMSPList();

    css::uno::Reference< css::script::provider::XScriptProvider >
        getMSPFromStringContext( const ::rtl::OUString& context )
            SAL_THROW(( css::lang::IllegalArgumentException, css::uno::RuntimeException ));

    css::uno::Reference< css::script::provider::XScriptProvider >
        getMSPFromAnyContext( const css::uno::Any& context )
            SAL_THROW(( css::lang::IllegalArgumentException, css::uno::RuntimeException ));

    css::uno::Reference< css::script::provider::XScriptProvider >
        getMSPFromInvocationContext( const css::uno::Reference< css::document::XScriptInvocationContext >& context )
            SAL_THROW(( css::lang::IllegalArgumentException, css::uno::RuntimeException ));

    //XEventListener
    //======================================================================

    virtual void SAL_CALL disposing( const css::lang::EventObject& Source )
        throw ( css::uno::RuntimeException );

private:
    void addActiveMSP( const css::uno::Reference< css::uno::XInterface >& xComponent,
                       const css::uno::Reference< css::script::provider::XScriptProvider >& msp );
    css::uno::Reference< css::script::provider::XScriptProvider >
        createNewMSP( const css::uno::Any& context );
    css::uno::Reference< css::script::provider::XScriptProvider >
        createNewMSP( const ::rtl::OUString& context )
    {
        return createNewMSP( css::uno::makeAny( context ) );
    }

    void createNonDocMSPs();
    Msp_hash m_hMsps;
    ScriptComponent_map m_mScriptComponents;
    osl::Mutex m_mutex;
    ::rtl::OUString userDirString;
    ::rtl::OUString shareDirString;
    ::rtl::OUString bundledDirString;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
};
} // func_provider
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
