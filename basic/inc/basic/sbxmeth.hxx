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



#ifndef __SBX_SBXMETHOD_HXX
#define __SBX_SBXMETHOD_HXX

#include <basic/sbxvar.hxx>

class SbxMethodImpl;

class SbxMethod : public SbxVariable
{
    SbxMethodImpl* mpSbxMethodImpl; // Impl data

public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_METHOD,1);
    SbxMethod( const String& r, SbxDataType t )
    : SbxVariable( t ) { SetName( r ); }
    SbxMethod( const SbxMethod& r ) : SvRefBase( r ), SbxVariable( r ) {}
    SbxMethod& operator=( const SbxMethod& r )
    { SbxVariable::operator=( r ); return *this; }
    sal_Bool Run( SbxValues* pValues = NULL );
    virtual SbxClassType GetClass() const;
};

#ifndef __SBX_SBXMETHODREF_HXX
#define __SBX_SBXMETHODREF_HXX

#ifndef SBX_METHOD_DECL_DEFINED
#define SBX_METHOD_DECL_DEFINED
SV_DECL_REF(SbxMethod)
#endif
SV_IMPL_REF(SbxMethod)

#endif
#endif

