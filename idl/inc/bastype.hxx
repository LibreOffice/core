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

#ifndef INCLUDED_IDL_INC_BASTYPE_HXX
#define INCLUDED_IDL_INC_BASTYPE_HXX


#include <sal/types.h>
#include <tools/globname.hxx>
#include <tools/stream.hxx>

class SvStringHashEntry;
class SvIdlDataBase;
class SvTokenStream;

class SvUINT32
{
    sal_uInt32  nVal;
public:
                SvUINT32() { nVal = 0; }
                SvUINT32( sal_uInt32 n ) : nVal( n ) {}
    SvUINT32 &  operator = ( sal_uInt32 n ) { nVal = n; return *this; }

    operator    sal_uInt32 &() { return nVal; }

    static sal_uInt32  Read( SvStream & rStm );
    static void    Write( SvStream & rStm, sal_uInt32 nVal );

    friend SvStream& WriteSvUINT32(SvStream & rStm, const SvUINT32 & r )
                { SvUINT32::Write( rStm, r.nVal ); return rStm; }
    friend SvStream& operator >> (SvStream & rStm, SvUINT32 & r )
                { r.nVal = SvUINT32::Read( rStm ); return rStm; }
};


class Svint
{
    int     nVal;
    bool    bSet;
public:
                Svint() { nVal = 0; bSet = false; }
                Svint( int n ) : nVal( n ), bSet( true ) {}
                Svint( int n, bool bSetP ) : nVal( n ), bSet( bSetP ) {}
    Svint    &  operator = ( int n ) { nVal = n; bSet = true; return *this; }

    operator    int ()const { return nVal; }
    bool        IsSet() const { return bSet; }

    friend SvStream& WriteSvint(SvStream & rStm, const Svint & r )
                { SvUINT32::Write( rStm, (sal_uInt32)r.nVal ); rStm.WriteUInt8( r.bSet ); return rStm; }
    friend SvStream& operator >> (SvStream & rStm, Svint & r )
                { r.nVal = (int)SvUINT32::Read( rStm ); rStm.ReadCharAsBool( r.bSet ); return rStm; }
};


class SvBOOL
{
    bool  nVal:1,
          bSet:1;
public:
                SvBOOL() { bSet = nVal = false; }
                SvBOOL( bool n ) : nVal( n ), bSet( true ) {}
                SvBOOL( bool n, bool bSetP ) : nVal( n ), bSet( bSetP ) {}
    SvBOOL &    operator = ( bool n ) { nVal = n; bSet = true; return *this; }

    operator    bool() const { return nVal; }
#ifdef STC
    operator    int() const { return nVal; }
#endif
    bool        Is() const { return nVal; }
    bool        IsSet() const { return bSet; }

    friend SvStream& WriteSvBOOL(SvStream &, const SvBOOL &);
    friend SvStream& operator >> (SvStream &, SvBOOL &);

    bool        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    bool        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm );
    OString    GetSvIdlString( SvStringHashEntry * pName );
};


class SvIdentifier
{
private:
    OString m_aStr;
public:
    SvIdentifier()
    {
    }
    void setString(const OString& rStr)
    {
        m_aStr = rStr;
    }
    const OString& getString() const
    {
        return m_aStr;
    }
    friend SvStream& WriteSvIdentifier(SvStream &, const SvIdentifier &);
    friend SvStream& operator >> (SvStream &, SvIdentifier &);

    bool IsSet() const
    {
        return !m_aStr.isEmpty();
    }
    bool        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    bool        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm,
                            sal_uInt16 nTab );
};


class SvNumberIdentifier : public SvIdentifier
{
    sal_uInt32  nValue;
    // must not be used
    bool    ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
public:
                SvNumberIdentifier() : nValue( 0 ) {};
    bool        IsSet() const
                {
                    return SvIdentifier::IsSet() || nValue != 0;
                }
    sal_uInt32      GetValue() const { return nValue; }
    void        SetValue( sal_uInt32 nVal ) { nValue = nVal; }

    friend SvStream& WriteSvNumberIdentifier(SvStream &, const SvNumberIdentifier &);
    friend SvStream& operator >> (SvStream &, SvNumberIdentifier &);
    bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    bool        ReadSvIdl( SvIdlDataBase &, SvStringHashEntry * pName,
                           SvTokenStream & rInStm );
};


class SvString
{
private:
    OString m_aStr;
public:
    SvString() {}
    void setString(const OString& rStr)
    {
        m_aStr = rStr;
    }
    const OString& getString() const
    {
        return m_aStr;
    }
    bool IsSet() const
    {
        return !m_aStr.isEmpty();
    }
    friend SvStream& WriteSvString(SvStream &, const SvString &);
    friend SvStream& operator >> (SvStream &, SvString &);

    bool        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    bool        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm,
                            sal_uInt16 nTab );
};


class SvHelpText : public SvString
{
public:
                SvHelpText() {}
    bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    bool        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                            sal_uInt16 nTab );
};


class SvHelpContext : public SvNumberIdentifier
{
};

class SvUUId : public SvGlobalName
{
public:
                SvUUId() {}
    bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    bool        WriteSvIdl( SvStream & rOutStm );
};


class SvVersion
{
    sal_uInt16  nMajorVersion;
    sal_uInt16  nMinorVersion;
public:
                SvVersion() : nMajorVersion( 1 ), nMinorVersion( 0 ) {}
    bool        operator == ( const SvVersion & r )
                {
                    return (r.nMajorVersion == nMajorVersion)
                             && (r.nMinorVersion == nMinorVersion);
                }
    bool        operator != ( const SvVersion & r )
                {
                    return !(*this == r);
                }

    sal_uInt16      GetMajorVersion() const { return nMajorVersion; }
    sal_uInt16      GetMinorVersion() const { return nMinorVersion; }

    friend SvStream& WriteSvVersion(SvStream &, const SvVersion &);
    friend SvStream& operator >> (SvStream &, SvVersion &);
    bool        ReadSvIdl( SvTokenStream & rInStm );
    bool        WriteSvIdl( SvStream & rOutStm );
};


#endif // INCLUDED_IDL_INC_BASTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
