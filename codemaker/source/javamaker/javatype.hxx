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



#ifndef INCLUDED_CODEMAKER_SOURCE_JAVAMAKER_JAVATYPE_HXX
#define INCLUDED_CODEMAKER_SOURCE_JAVAMAKER_JAVATYPE_HXX

namespace codemaker { class GeneratedTypeSet; }
namespace rtl { class OString; }
class JavaOptions;
class TypeManager;
class RegistryKey;

bool produceType(
    rtl::OString const & type, TypeManager const & manager,
    codemaker::GeneratedTypeSet & generated, JavaOptions * pOptions);

bool produceType(RegistryKey& typeName, bool bIsExtraType, TypeManager const & typeMgr,
                 codemaker::GeneratedTypeSet & generated,
                 JavaOptions* pOptions);

#endif // INCLUDED_CODEMAKER_SOURCE_JAVAMAKER_JAVATYPE_HXX
