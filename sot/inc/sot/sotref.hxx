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



#ifndef _SOT_SOTREF_HXX
#define _SOT_SOTREF_HXX

#ifndef _TOOLS_REF_HXX
#include <tools/ref.hxx>
#endif

//========================================================================
#ifndef SVT_DECL_SOTOBJECT_DEFINED
#define SVT_DECL_SOTOBJECT_DEFINED
class SotObject;
class SotObjectRef
{
    PRV_SV_DECL_REF(SotObject)
};
#endif

//========================================================================
#define SO2_DECL_REF(ClassName)                                           \
class ClassName;                                                          \
class ClassName##Ref                                                      \
{                                                                         \
    PRV_SV_DECL_REF(ClassName)                                            \
    inline          ClassName##Ref( const SotObjectRef & );               \
    inline          ClassName##Ref( SotObject * pObjP );                  \
};

#define SO2_IMPL_REF(ClassName)                                           \
SV_IMPL_REF(ClassName)                                                    \
inline ClassName##Ref::ClassName##Ref( const SotObjectRef & r )           \
{                                                                         \
    pObj = (ClassName *)ClassName::ClassFactory()->CastAndAddRef( &r );   \
}                                                                         \
inline ClassName##Ref::ClassName##Ref( SotObject * pObjP )                \
{                                                                         \
    pObj = (ClassName *)ClassName::ClassFactory()->CastAndAddRef( pObjP );\
}


#define SO2_DECL_IMPL_REF(ClassName)                                      \
    SO2_DECL_REF(ClassName)                                               \
    SO2_IMPL_REF(ClassName)

#endif // _SO2REF_HXX
