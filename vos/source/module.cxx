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




#include <vos/module.hxx>
#include <vos/diagnose.hxx>

using namespace vos;

VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OModule, vos),
                        VOS_NAMESPACE(OModule, vos),
                        VOS_NAMESPACE(OObject, vos), 0);


OModule::OModule()
    :m_Module(0)
{
}

OModule::OModule(const rtl::OUString& ustrModuleName, sal_Int32 nRtldMode) : m_Module(0)
{
    if( !ustrModuleName.isEmpty())
        load(ustrModuleName, nRtldMode);
}

OModule::~OModule()
{
    if (m_Module)
        osl_unloadModule(m_Module);
}

sal_Bool OModule::load(const rtl::OUString& ustrModuleName, sal_Int32 nRtldMode)
{
    VOS_ASSERT( !ustrModuleName.isEmpty());

    unload();

    m_Module = osl_loadModule( ustrModuleName.pData, nRtldMode );

    return (m_Module != 0);
}

void OModule::unload()
{
    if (m_Module)
    {
        osl_unloadModule(m_Module);
        m_Module = 0;
    }
}

sal_Bool OModule::isLoaded()
{
    return m_Module != NULL;
}

void *OModule::getSymbol(const rtl::OUString& strSymbolName)
{
    VOS_ASSERT( !strSymbolName.isEmpty());
    VOS_ASSERT(m_Module);
    return ( osl_getSymbol( m_Module, strSymbolName.pData ) );
}

