/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SBXCORE_HXX
#define _SBXCORE_HXX

#include <tools/rtti.hxx>
#include <tools/ref.hxx>
#include <tools/debug.hxx>

#include <basic/sbxdef.hxx>
#include "basicdllapi.h"

class SvStream;
class String;
namespace rtl { class OUString; }

// The following Macro defines four (five) necessary methods within a
// SBX object. LoadPrivateData() and StorePrivateData() must be implemented.
// They are necessary for loading/storing the data of derived classes.
// Load() and Store() must not be overridden.

// This version of the Macros does not define Load/StorePrivateData()-methods
#define SBX_DECL_PERSIST_NODATA( nCre, nSbxId, nVer )       \
    virtual sal_uInt32 GetCreator() const { return nCre;   }    \
    virtual sal_uInt16 GetVersion() const { return nVer;   }    \
    virtual sal_uInt16 GetSbxId() const   { return nSbxId; }

// This version of the macro defines Load/StorePrivateData()-methods
#define SBX_DECL_PERSIST( nCre, nSbxId, nVer )              \
    virtual sal_Bool LoadPrivateData( SvStream&, sal_uInt16 );      \
    virtual sal_Bool StorePrivateData( SvStream& ) const;       \
    SBX_DECL_PERSIST_NODATA( nCre, nSbxId, nVer )

class SbxBase;
class SbxFactory;
class SbxObject;

DBG_NAMEEX_VISIBILITY(SbxBase, BASIC_DLLPUBLIC)

class BASIC_DLLPUBLIC SbxBase : virtual public SvRefBase
{
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
    static SbxObject* CreateObject( const rtl::OUString& );
};

SV_DECL_REF(SbxBase)

inline void SbxBase::SetFlags( sal_uInt16 n )
{ //DBG_CHKTHIS( SbxBase, 0 );
 nFlags = n; }

inline sal_uInt16 SbxBase::GetFlags() const
{ DBG_CHKTHIS( SbxBase, 0 ); return nFlags; }

inline void SbxBase::SetFlag( sal_uInt16 n )
{ //DBG_CHKTHIS( SbxBase, 0 );
 nFlags |= n; }

inline void SbxBase::ResetFlag( sal_uInt16 n )
{ //DBG_CHKTHIS( SbxBase, 0 );
 nFlags &= ~n; }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
