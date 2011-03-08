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

#ifndef _SBXCORE_HXX
#define _SBXCORE_HXX

#include <tools/rtti.hxx>
#include <tools/ref.hxx>
#include <tools/debug.hxx>

#include <basic/sbxdef.hxx>

class SvStream;
class String;
class UniString;

// The following Macro defines four (five) necessary methods within a
// SBX object. LoadPrivateData() and StorePrivateData() must be implemented.
// They are necessary for loading/storing the data of derived classes.
// Load() and Store() must not be overridden.

// This version of the Macros does not define Load/StorePrivateData()-methods
#define SBX_DECL_PERSIST_NODATA( nCre, nSbxId, nVer )       \
    virtual UINT32 GetCreator() const { return nCre;   }    \
    virtual UINT16 GetVersion() const { return nVer;   }    \
    virtual UINT16 GetSbxId() const   { return nSbxId; }

#define SBX_DECL_PERSIST_NODATA_()                          \
    virtual UINT32 GetCreator() const;                      \
    virtual UINT16 GetVersion() const;                      \
    virtual UINT16 GetSbxId() const;

// This version of the macro defines Load/StorePrivateData()-methods
#define SBX_DECL_PERSIST( nCre, nSbxId, nVer )              \
    virtual BOOL LoadPrivateData( SvStream&, USHORT );      \
    virtual BOOL StorePrivateData( SvStream& ) const;       \
    SBX_DECL_PERSIST_NODATA( nCre, nSbxId, nVer )

#define SBX_DECL_PERSIST_()                                 \
    virtual BOOL LoadPrivateData( SvStream&, USHORT );      \
    virtual BOOL StorePrivateData( SvStream& ) const;       \
    SBX_DECL_PERSIST_NODATA_()

#define SBX_IMPL_PERSIST( C, nCre, nSbxId, nVer )           \
    UINT32 C::GetCreator() const { return nCre;   }         \
    UINT16 C::GetVersion() const { return nVer;   }         \
    UINT16 C::GetSbxId() const   { return nSbxId; }

class SbxBase;
class SbxFactory;
class SbxObject;

DBG_NAMEEX(SbxBase)

class SbxBaseImpl;

class SbxBase : virtual public SvRefBase
{
    SbxBaseImpl* mpSbxBaseImpl; // Impl data

    virtual BOOL LoadData( SvStream&, USHORT );
    virtual BOOL StoreData( SvStream& ) const;
protected:
    USHORT nFlags;          // Flag-Bits

    SbxBase();
    SbxBase( const SbxBase& );
    SbxBase& operator=( const SbxBase& );
    virtual ~SbxBase();
    SBX_DECL_PERSIST(0,0,0);
public:
    TYPEINFO();
    inline void     SetFlags( USHORT n );
    inline USHORT   GetFlags() const;
    inline void     SetFlag( USHORT n );
    inline void     ResetFlag( USHORT n );
    inline BOOL     IsSet( USHORT n ) const;
    inline BOOL     IsReset( USHORT n ) const;
    inline BOOL     CanRead() const;
    inline BOOL     CanWrite() const;
    inline BOOL     IsModified() const;
    inline BOOL     IsConst() const;
    inline BOOL     IsHidden() const;
    inline BOOL     IsVisible() const;

    virtual BOOL IsFixed() const;
    virtual void SetModified( BOOL );

    virtual SbxDataType  GetType()  const;
    virtual SbxClassType GetClass() const;

    virtual void Clear();

    static SbxBase* Load( SvStream& );
    static void Skip( SvStream& );
    BOOL Store( SvStream& );
    virtual BOOL LoadCompleted();
    virtual BOOL StoreCompleted();

    static SbxError GetError();
    static void SetError( SbxError );
    static BOOL IsError();
    static void ResetError();

    // Set the factory for Load/Store/Create
    static void AddFactory( SbxFactory* );
    static void RemoveFactory( SbxFactory* );

    static SbxBase* Create( UINT16, UINT32=SBXCR_SBX );
    static SbxObject* CreateObject( const String& );
    // Sbx solution as replacement for SfxBroadcaster::Enable()
    static void StaticEnableBroadcasting( BOOL bEnable );
    static BOOL StaticIsEnabledBroadcasting( void );
};

#ifndef SBX_BASE_DECL_DEFINED
#define SBX_BASE_DECL_DEFINED
SV_DECL_REF(SbxBase)
#endif

inline void SbxBase::SetFlags( USHORT n )
{ DBG_CHKTHIS( SbxBase, 0 ); nFlags = n; }

inline USHORT SbxBase::GetFlags() const
{ DBG_CHKTHIS( SbxBase, 0 ); return nFlags; }

inline void SbxBase::SetFlag( USHORT n )
{ DBG_CHKTHIS( SbxBase, 0 ); nFlags |= n; }

inline void SbxBase::ResetFlag( USHORT n )
{ DBG_CHKTHIS( SbxBase, 0 ); nFlags &= ~n; }

inline BOOL SbxBase::IsSet( USHORT n ) const
{ DBG_CHKTHIS( SbxBase, 0 ); return BOOL( ( nFlags & n ) != 0 ); }

inline BOOL SbxBase::IsReset( USHORT n ) const
{ DBG_CHKTHIS( SbxBase, 0 ); return BOOL( ( nFlags & n ) == 0 ); }

inline BOOL SbxBase::CanRead() const
{ DBG_CHKTHIS( SbxBase, 0 ); return IsSet( SBX_READ ); }

inline BOOL SbxBase::CanWrite() const
{ DBG_CHKTHIS( SbxBase, 0 ); return IsSet( SBX_WRITE ); }

inline BOOL SbxBase::IsModified() const
{ DBG_CHKTHIS( SbxBase, 0 ); return IsSet( SBX_MODIFIED ); }

inline BOOL SbxBase::IsConst() const
{ DBG_CHKTHIS( SbxBase, 0 ); return IsSet( SBX_CONST ); }

inline BOOL SbxBase::IsHidden() const
{ DBG_CHKTHIS( SbxBase, 0 ); return IsSet( SBX_HIDDEN ); }

inline BOOL SbxBase::IsVisible() const
{ DBG_CHKTHIS( SbxBase, 0 ); return IsReset( SBX_INVISIBLE ); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
