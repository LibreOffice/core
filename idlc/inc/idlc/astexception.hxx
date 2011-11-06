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


#ifndef _IDLC_ASTEXCEPTION_HXX_
#define _IDLC_ASTEXCEPTION_HXX_

#include <idlc/aststruct.hxx>

class AstException : public AstStruct
{
public:
    AstException(const ::rtl::OString& name, AstException* pBaseType, AstScope* pScope)
        : AstStruct(NT_exception, name, pBaseType, pScope)
        {}

    virtual ~AstException() {}
};

#endif // _IDLC_ASTEXCEPTION_HXX_

