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
    virtual sal_uInt32 GetCreator() const { return nCre;   }    \
    virtual sal_uInt16 GetVersion() const { return nVer;   }    \
    virtual sal_uInt16 GetSbxId() const   { return nSbxId; }

#define SBX_DECL_PERSIST_NODATA_()                          \
    virtual sal_uInt32 GetCreator() const;                      \
    virtual sal_uInt16 GetVersion() const;                      \
    virtual sal_uInt16 GetSbxId() const;

// This version of the macro defines Load/StorePrivateData()-methods
#define SBX_DECL_PERSIST( nCre, nSbxId, nVer )              \
    virtual sal_Bool LoadPrivateData( SvStream&, sal_uInt16 );      \
    virtual sal_Bool StorePrivateData( SvStream& ) const;       \
    SBX_DECL_PERSIST_NODATA( nCre, nSbxId, nVer )

#define SBX_DECL_PERSIST_()                                 \
    virtual sal_Bool LoadPrivateData( SvStream&, sal_uInt16 );      \
    virtual sal_Bool StorePrivateData( SvStream& ) const;       \
    SBX_DECL_PERSIST_NODATA_()

#define SBX_IMPL_PERSIST( C, nCre, nSbxId, nVer )           \
    sal_uInt32 C::GetCreator() const { return nCre;   }         \
    sal_uInt16 C::GetVersion() const { return nVer;   }         \
    sal_uInt16 C::GetSbxId() const   { return nSbxId; }

class SbxBase;
class SbxFactory;
class SbxObject;

DBG_NAMEEX(SbxBase)

class SbxBaseImpl;

class SbxBase : virtual public SvRefBase
{
    SbxBaseImpl* mpSbxBaseImpl; // Impl data

    virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
    virtual sal_Bool StoreData( SvStream& ) const;
protected:
    sal_uInt16 nFlags;          // Flag-Bits

    SbxBase();
    SbxBase( const SbxBase& );
    SbxBase& operator=( const SbxBase& );
    virtual ~SbxBase();
    SBX_DECL_PERSIST(0,0,0);
public:
    TYPEINFO();
    inline void     SetFlags( sal_uInt16 n );
    inline sal_uInt16   GetFlags() const;
    inline void     SetFlag( sal_uInt16 n );
    inline void     ResetFlag( sal_uInt16 n );
    inline sal_Bool     IsSet( sal_uInt16 n ) const;
    inline sal_Bool     IsReset( sal_uInt16 n ) const;
    inline sal_Bool     CanRead() const;
    inline sal_Bool     CanWrite() const;
    inline sal_Bool     IsModified() const;
    inline sal_Bool     IsConst() const;
    inline sal_Bool     IsHidden() const;
    inline sal_Bool     IsVisible() const;

    virtual sal_Bool IsFixed() const;
    virtual void SetModified( sal_Bool );

    virtual SbxDataType  GetType()  const;
    virtual SbxClassType GetClass() const;

    virtual void Clear();

    static SbxBase* Load( SvStream& );
    static void Skip( SvStream& );
    sal_Bool Store( SvStream& );
    virtual sal_Bool LoadCompleted();
    virtual sal_Bool StoreCompleted();

    static SbxError GetError();
    static void SetError( SbxError );
    static sal_Bool IsError();
    static void ResetError();

    // Set the factory for Load/Store/Create
    static void AddFactory( SbxFactory* );
    static void RemoveFactory( SbxFactory* );

    static SbxBase* Create( sal_uInt16, sal_uInt32=SBXCR_SBX );
    static SbxObject* CreateObject( const String& );
    // Sbx solution as replacement for SfxBroadcaster::Enable()
    static void StaticEnableBroadcasting( sal_Bool bEnable );
    static sal_Bool StaticIsEnabledBroadcasting( void );
};

#ifndef SBX_BASE_DECL_DEFINED
#define SBX_BASE_DECL_DEFINED
SV_DECL_REF(SbxBase)
#endif

inline void SbxBase::SetFlags( sal_uInt16 n )
{ DBG_CHKTHIS( SbxBase, 0 ); nFlags = n; }

inline sal_uInt16 SbxBase::GetFlags() const
{ DBG_CHKTHIS( SbxBase, 0 ); return nFlags; }

inline void SbxBase::SetFlag( sal_uInt16 n )
{ DBG_CHKTHIS( SbxBase, 0 ); nFlags |= n; }

inline void SbxBase::ResetFlag( sal_uInt16 n )
{ DBG_CHKTHIS( SbxBase, 0 ); nFlags &= ~n; }

inline sal_Bool SbxBase::IsSet( sal_uInt16 n ) const
{ DBG_CHKTHIS( SbxBase, 0 ); return sal_Bool( ( nFlags & n ) != 0 ); }

inline sal_Bool SbxBase::IsReset( sal_uInt16 n ) const
{ DBG_CHKTHIS( SbxBase, 0 ); return sal_Bool( ( nFlags & n ) == 0 ); }

inline sal_Bool SbxBase::CanRead() const
{ DBG_CHKTHIS( SbxBase, 0 ); return IsSet( SBX_READ ); }

inline sal_Bool SbxBase::CanWrite() const
{ DBG_CHKTHIS( SbxBase, 0 ); return IsSet( SBX_WRITE ); }

inline sal_Bool SbxBase::IsModified() const
{ DBG_CHKTHIS( SbxBase, 0 ); return IsSet( SBX_MODIFIED ); }

inline sal_Bool SbxBase::IsConst() const
{ DBG_CHKTHIS( SbxBase, 0 ); return IsSet( SBX_CONST ); }

inline sal_Bool SbxBase::IsHidden() const
{ DBG_CHKTHIS( SbxBase, 0 ); return IsSet( SBX_HIDDEN ); }

inline sal_Bool SbxBase::IsVisible() const
{ DBG_CHKTHIS( SbxBase, 0 ); return IsReset( SBX_INVISIBLE ); }

#endif
