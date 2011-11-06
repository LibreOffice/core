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
