/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _FRAMEWORK_SCRIPT_PROVIDER_OPENDOCUMENTLIST_HXX_
#define _FRAMEWORK_SCRIPT_PROVIDER_OPENDOCUMENTLIST_HXX_

#include <hash_map>
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

typedef ::std::hash_map< ::rtl::OUString,
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
