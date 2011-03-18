/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    SbxVariable* mCaller;                   // caller
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
    TYPEINFO();
    virtual SbxInfo* GetInfo();
    SbxArray* GetLocals();
    SbxArray* GetStatics();
    void      ClearStatics();
    SbModule* GetModule()                { return pMod;        }
    sal_uInt32    GetId() const              { return nStart;      }
    sal_uInt16    GetDebugFlags()            { return nDebugFlags; }
    void      SetDebugFlags( sal_uInt16 n )  { nDebugFlags = n;    }
    void      GetLineRange( sal_uInt16&, sal_uInt16& );

    // Interface to execute a method from the applications
    virtual ErrCode Call( SbxValue* pRet = NULL,  SbxVariable* pCaller = NULL );
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
    TYPEINFO();
    SbIfaceMapperMethod( const String& rName, SbMethod* pImplMeth )
        : SbMethod( rName, pImplMeth->GetType(), NULL )
        , mxImplMeth( pImplMeth )
    {}
    virtual ~SbIfaceMapperMethod();
    SbMethod* getImplMethod( void )
        { return mxImplMeth; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
