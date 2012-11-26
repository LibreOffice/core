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



#ifndef _SB_SBMETH_HXX
#define _SB_SBMETH_HXX

#include <tools/errcode.hxx>
#include <basic/sbxmeth.hxx>
#include <basic/sbdef.hxx>

class SbModule;
class SbMethodImpl;

class SbMethod : public SbxMethod
{
    friend class SbiRuntime;
    friend class SbiFactory;
    friend class SbModule;
    friend class SbClassModuleObject;
    friend class SbiCodeGen;
    friend class SbJScriptMethod;
    friend class SbIfaceMapperMethod;

    SbMethodImpl* mpSbMethodImpl;           // Impl data
    SbModule* pMod;
    sal_uInt16    nDebugFlags;
    sal_uInt16    nLine1, nLine2;
    sal_uInt32    nStart;
    sal_Bool      bInvalid;
    SbxArrayRef refStatics;
    SbMethod( const String&, SbxDataType, SbModule* );
    SbMethod( const SbMethod& );
    virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
    virtual sal_Bool StoreData( SvStream& ) const;
    virtual ~SbMethod();

public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_BASICMETHOD,2);
    virtual SbxInfo* GetInfo();
    SbxArray* GetLocals();
    SbxArray* GetStatics();
    void      ClearStatics();
    SbModule* GetModule()                { return pMod;        }
    sal_uInt32    GetId() const              { return nStart;      }
    sal_uInt16    GetDebugFlags()            { return nDebugFlags; }
    void      SetDebugFlags( sal_uInt16 n )  { nDebugFlags = n;    }
    void      GetLineRange( sal_uInt16&, sal_uInt16& );

    // Schnittstelle zum Ausfuehren einer Methode aus den Applikationen
    virtual ErrCode Call( SbxValue* pRet = NULL );
    virtual void Broadcast( sal_uIntPtr nHintId );
};

#ifndef __SB_SBMETHODREF_HXX
#define __SB_SBMETHODREF_HXX
SV_DECL_IMPL_REF(SbMethod)
#endif

class SbIfaceMapperMethod : public SbMethod
{
    friend class SbiRuntime;

    SbMethodRef mxImplMeth;

public:
    SbIfaceMapperMethod( const String& rName, SbMethod* pImplMeth )
        : SbMethod( rName, pImplMeth->GetType(), NULL )
        , mxImplMeth( pImplMeth )
    {}
    virtual ~SbIfaceMapperMethod();
    SbMethod* getImplMethod( void )
        { return mxImplMeth; }
};

#endif
