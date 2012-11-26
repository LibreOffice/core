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


#ifndef _SFXHINT_HXX
#define _SFXHINT_HXX

#include "svl/svldllapi.h"

class SVL_DLLPUBLIC SfxHint
{
public:
    virtual ~SfxHint();
};

//--------------------------------------------------------------------

#define DECL_PTRHINT(Visibility, Name, Type) \
        class Visibility Name: public SfxHint \
        { \
            Type* pObj; \
            sal_Bool  bIsOwner; \
        \
        public: \
            Name( Type* Object, sal_Bool bOwnedByHint = sal_False ); \
            ~Name(); \
        \
            Type* GetObject() const { return pObj; } \
            sal_Bool  IsOwner() const { return bIsOwner; } \
        }

#define IMPL_PTRHINT_AUTODELETE(Name, Type) \
        Name::Name( Type* pObject, sal_Bool bOwnedByHint ) \
            { pObj = pObject; bIsOwner = bOwnedByHint; } \
        Name::~Name() { if ( bIsOwner ) delete pObj; }

#define IMPL_PTRHINT(Name, Type) \
        Name::Name( Type* pObject, sal_Bool bOwnedByHint ) \
            { pObj = pObject; bIsOwner = bOwnedByHint; } \
        Name::~Name() {}


#endif

