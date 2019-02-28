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

#ifndef INCLUDED_BASIC_SBXCORE_HXX
#define INCLUDED_BASIC_SBXCORE_HXX

#include <basic/basicdllapi.h>
#include <basic/sbxdef.hxx>
#include <rtl/ustring.hxx>
#include <tools/ref.hxx>

class SvStream;
class ErrCode;

// The following Macro defines four (five) necessary methods within a
// SBX object. LoadPrivateData() and StorePrivateData() must be implemented.
// They are necessary for loading/storing the data of derived classes.
// Load() and Store() must not be overridden.

// This version of the Macros does not define Load/StorePrivateData()-methods
#define SBX_DECL_PERSIST_NODATA( nSbxId, nVer )       \
    virtual sal_uInt16 GetVersion() const override { return nVer;   }    \
    virtual sal_uInt16 GetSbxId() const override   { return nSbxId; }

class SbxFactory;
class SbxObject;

class BASIC_DLLPUBLIC SbxBase : virtual public SvRefBase
{
    virtual bool LoadData( SvStream&, sal_uInt16 ) = 0;
    virtual bool StoreData( SvStream& ) const = 0;
protected:
    SbxFlagBits nFlags;          // Flag-Bits

    SbxBase();
    SbxBase( const SbxBase& );
    SbxBase& operator=( const SbxBase& );
    virtual ~SbxBase() override;

    virtual sal_uInt16 GetVersion() const = 0;
    virtual sal_uInt16 GetSbxId() const = 0;

public:
    inline void         SetFlags( SbxFlagBits n );
    inline SbxFlagBits  GetFlags() const;
    inline void         SetFlag( SbxFlagBits n );
    inline void         ResetFlag( SbxFlagBits n );
    inline bool         IsSet( SbxFlagBits n ) const;
    inline bool         IsReset( SbxFlagBits n ) const;
    inline bool         CanRead() const;
    inline bool         CanWrite() const;
    inline bool         IsModified() const;
    inline bool         IsHidden() const;
    inline bool         IsVisible() const;

    virtual bool        IsFixed() const;
    virtual void        SetModified( bool );

    virtual SbxDataType GetType()  const;

    virtual void    Clear() = 0;

    static SbxBase* Load( SvStream& );
    bool            Store( SvStream& );
    virtual bool    LoadCompleted();

    static ErrCode const & GetError();
    static void SetError( ErrCode );
    static bool IsError();
    static void ResetError();

    // Set the factory for Load/Store/Create
    static void AddFactory( SbxFactory* );
    static void RemoveFactory( SbxFactory const * );

    static SbxBase* Create( sal_uInt16, sal_uInt32 );
    static SbxObject* CreateObject( const OUString& );
};

typedef tools::SvRef<SbxBase> SbxBaseRef;

inline void SbxBase::SetFlags( SbxFlagBits n )
{ nFlags = n; }

inline SbxFlagBits SbxBase::GetFlags() const
{ return nFlags; }

inline void SbxBase::SetFlag( SbxFlagBits n )
{ nFlags |= n; }

inline void SbxBase::ResetFlag( SbxFlagBits n )
{ nFlags &= ~n; }

inline bool SbxBase::IsSet( SbxFlagBits n ) const
{ return ( nFlags & n ) != SbxFlagBits::NONE; }

inline bool SbxBase::IsReset( SbxFlagBits n ) const
{ return ( nFlags & n ) == SbxFlagBits::NONE; }

inline bool SbxBase::CanRead() const
{ return IsSet( SbxFlagBits::Read ); }

inline bool SbxBase::CanWrite() const
{ return IsSet( SbxFlagBits::Write ); }

inline bool SbxBase::IsModified() const
{ return IsSet( SbxFlagBits::Modified ); }

inline bool SbxBase::IsHidden() const
{ return IsSet( SbxFlagBits::Hidden ); }

inline bool SbxBase::IsVisible() const
{ return IsReset( SbxFlagBits::Invisible ); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
