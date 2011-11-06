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



#ifndef _RDBMAKER_RDBTYPE_HXX_
#define _RDBMAKER_RDBTYPE_HXX_

#include    <codemaker/typemanager.hxx>
#include    <codemaker/dependency.hxx>

sal_Bool    checkFilterTypes(const ::rtl::OString& type);
void        cleanUp(sal_Bool);

class RdbOptions;
class FileStream;
class RegistryKey;

sal_Bool produceType(const ::rtl::OString& typeName,
                     TypeManager& typeMgr,
                     TypeDependency& typeDependencies,
                     RdbOptions* pOptions,
                     FileStream& o,
                     RegistryKey& regKey,
                     StringSet& filterTypes,
                     sal_Bool bDepend = sal_False)
                 throw( CannotDumpException );

#endif // _RDBMAKER_RDBTYPE_HXX_

