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



#ifndef __SBX_SBXPROPERTY_HXX
#define __SBX_SBXPROPERTY_HXX

#include <basic/sbxvar.hxx>

class SbxPropertyImpl;

class SbxProperty : public SbxVariable
{
    SbxPropertyImpl* mpSbxPropertyImpl; // Impl data

public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_PROPERTY,1);
    SbxProperty( const String& r, SbxDataType t )
    : SbxVariable( t ) { SetName( r ); }
    SbxProperty( const SbxProperty& r ) : SvRefBase( r ), SbxVariable( r ) {}
    SbxProperty& operator=( const SbxProperty& r )
    { SbxVariable::operator=( r ); return *this; }
    virtual SbxClassType GetClass() const;
};

#ifndef __SBX_SBXPROPERTYREF_HXX
#define __SBX_SBXPROPERTYREF_HXX

#ifndef SBX_PROPERTY_DECL_DEFINED
#define SBX_PROPERTY_DECL_DEFINED
SV_DECL_REF(SbxProperty)
#endif
SV_IMPL_REF(SbxProperty)

#endif

#endif
