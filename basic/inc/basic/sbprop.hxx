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



#ifndef _SB_SBPROPERTY_HXX
#define _SB_SBPROPERTY_HXX

#include <basic/sbxprop.hxx>
#include <basic/sbdef.hxx>

class SbModule;

class SbProperty : public SbxProperty
{
    friend class SbiFactory;
    friend class SbModule;
    friend class SbProcedureProperty;
    SbModule* pMod;
    sal_Bool     bInvalid;
    SbProperty( const String&, SbxDataType, SbModule* );
    virtual ~SbProperty();
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_BASICPROP,1);
    SbModule* GetModule() { return pMod; }
};

#ifndef __SB_SBPROPERTYREF_HXX
#define __SB_SBPROPERTYREF_HXX
SV_DECL_IMPL_REF(SbProperty)
#endif

class SbProcedureProperty : public SbxProperty
{
    bool mbSet;     // Flag for set command

    virtual ~SbProcedureProperty();

public:
    SbProcedureProperty( const String& r, SbxDataType t )
            : SbxProperty( r, t ) // , pMod( p )
            , mbSet( false )
    {}

    bool isSet( void )
        { return mbSet; }
    void setSet( bool bSet )
        { mbSet = bSet; }
};

#ifndef __SB_SBPROCEDUREPROPERTYREF_HXX
#define __SB_SBPROCEDUREPROPERTYREF_HXX
SV_DECL_IMPL_REF(SbProcedureProperty)
#endif

#endif
