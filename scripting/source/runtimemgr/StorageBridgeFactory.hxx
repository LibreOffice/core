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




#ifndef _FRAMEWORK_SCRIPT_STORAGEBRIDGEFACTORY_HXX_
#define _FRAMEWORK_SCRIPT_STORAGEBRIDGEFACTORY_HXX_

#include <rtl/ustring>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <drafts/com/sun/star/script/framework/storage/XScriptInfoAccess.hpp>

namespace scripting_runtimemgr
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

class StorageBridgeFactory
{

public:
    explicit StorageBridgeFactory(
        const css::uno::Reference < css::uno::XComponentContext > & xContext );
    css::uno::Reference < dcsssf::storage::XScriptInfoAccess >
        getStorageInstance( sal_Int32 sid );
private:
    StorageBridgeFactory(); // No definition for default ctor
    css::uno::Reference< css::uno::XComponentContext >  m_xContext;
};
} // scripting_runtimemgr
#endif //_FRAMEWORK_SCRIPT_NAMERESOLVER_HXX_
