/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sbxcore.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-07 08:44:34 $
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

#ifndef _SBXCORE_HXX
#define _SBXCORE_HXX

#ifndef _RTTI_HXX //autogen
#include <tools/rtti.hxx>
#endif
#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include <basic/sbxdef.hxx>

class SvStream;
class String;
class UniString;

// Das nachfolgende Makro definiert die vier  (fuenf) notwendigen Methoden
// innerhalb eines SBX-Objekts. LoadPrivateData() und StorePrivateData()
// muessen selbst implementiert werden. Sie sind fuer das Laden/Speichern
// der Daten der abgeleiteten Klasse notwendig. Load() und Store() duerfen
// nicht ueberlagert werden.

// Diese Version des Makros definiert keine Load/StorePrivateData()-Methoden

#define SBX_DECL_PERSIST_NODATA( nCre, nSbxId, nVer )       \
    virtual UINT32 GetCreator() const { return nCre;   }    \
    virtual UINT16 GetVersion() const { return nVer;   }    \
    virtual UINT16 GetSbxId() const   { return nSbxId; }

#define SBX_DECL_PERSIST_NODATA_()                          \
    virtual UINT32 GetCreator() const;                      \
    virtual UINT16 GetVersion() const;                      \
    virtual UINT16 GetSbxId() const;

// Diese Version des Makros definiert Load/StorePrivateData()-Methoden

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

    // Setzen der Factory fuer Load/Store/Create
    static void AddFactory( SbxFactory* );
    static void RemoveFactory( SbxFactory* );

    static SbxBase* Create( UINT16, UINT32=SBXCR_SBX );
    static SbxObject* CreateObject( const String& );
    // Sbx-Loesung als Ersatz fuer SfxBroadcaster::Enable()
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
