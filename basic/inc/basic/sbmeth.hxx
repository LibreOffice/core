/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbmeth.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 12:52:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SB_SBMETH_HXX
#define _SB_SBMETH_HXX

#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif
#ifndef __SBX_SBXMETHOD_HXX //autogen
#include <basic/sbxmeth.hxx>
#endif
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
    USHORT    nDebugFlags;
    USHORT    nLine1, nLine2;
    UINT32    nStart;
    BOOL      bInvalid;
    SbMethod( const String&, SbxDataType, SbModule* );
    SbMethod( const SbMethod& );
    virtual BOOL LoadData( SvStream&, USHORT );
    virtual BOOL StoreData( SvStream& ) const;
    virtual ~SbMethod();

public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_BASICMETHOD,2);
    TYPEINFO();
    virtual SbxInfo* GetInfo();
    SbxArray* GetLocals();
    SbxArray* GetStatics();
    SbModule* GetModule()                { return pMod;        }
    UINT32    GetId() const              { return nStart;      }
    USHORT    GetDebugFlags()            { return nDebugFlags; }
    void      SetDebugFlags( USHORT n )  { nDebugFlags = n;    }
    void      GetLineRange( USHORT&, USHORT& );

    // Schnittstelle zum Ausfuehren einer Methode aus den Applikationen
    virtual ErrCode Call( SbxValue* pRet = NULL );
    virtual void Broadcast( ULONG nHintId );
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
