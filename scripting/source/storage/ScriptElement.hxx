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


#ifndef _SCRIPT_FRAMEWORK_STORAGE_SCRIPT_ELEMENT_HXX_
#define _SCRIPT_FRAMEWORK_STORAGE_SCRIPT_ELEMENT_HXX_

#include <osl/mutex.hxx>

#include "ScriptData.hxx"

#include "XMLElement.hxx"

namespace scripting_impl
{
// for simplification
#define css ::com::sun::star
#define dcsssf ::drafts::com::sun::star::script::framework

class ScriptElement : public ::scripting_impl::XMLElement
{
public:
    /**
    Construct a ScriptElement from a ScriptData  object

    @param sII
    the ScriptData Object
    */
    explicit ScriptElement( ScriptData & sII );
    ~ScriptElement() SAL_THROW (());

private:
    ScriptData m_sII;

};

}

#endif
