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

#ifndef _BASTYPE_HXX
#define _BASTYPE_HXX


#include <tools/globname.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>

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

    friend SvStream& operator << (SvStream & rStm, const SvUINT32 & r )
                { SvUINT32::Write( rStm, r.nVal ); return rStm; }
    friend SvStream& operator >> (SvStream & rStm, SvUINT32 & r )
                { r.nVal = SvUINT32::Read( rStm ); return rStm; }
};


class Svint
{
    int     nVal;
    sal_Bool    bSet;
public:
                Svint() { nVal = bSet = 0; }
                Svint( int n ) : nVal( n ), bSet( sal_True ) {}
                Svint( int n, sal_Bool bSetP ) : nVal( n ), bSet( bSetP ) {}
    Svint    &  operator = ( int n ) { nVal = n; bSet = sal_True; return *this; }

    operator    int ()const { return nVal; }
    sal_Bool        IsSet() const { return bSet; }

    friend SvStream& operator << (SvStream & rStm, const Svint & r )
                { SvUINT32::Write( rStm, (sal_uInt32)r.nVal ); rStm << r.bSet; return rStm; }
    friend SvStream& operator >> (SvStream & rStm, Svint & r )
                { r.nVal = (int)SvUINT32::Read( rStm ); rStm >> r.bSet ; return rStm; }
};


class SvBOOL
{
    sal_Bool  nVal:1,
          bSet:1;
public:
                SvBOOL() { bSet = nVal = sal_False; }
                SvBOOL( sal_Bool n ) : nVal( n ), bSet( sal_True ) {}
                SvBOOL( sal_Bool n, sal_Bool bSetP ) : nVal( n ), bSet( bSetP ) {}
    SvBOOL &    operator = ( sal_Bool n ) { nVal = n; bSet = sal_True; return *this; }

    operator    sal_Bool() const { return nVal; }
#ifdef STC
    operator    int() const { return nVal; }
#endif
    sal_Bool        Is() const { return nVal; }
    sal_Bool        IsSet() const { return bSet; }

    friend SvStream& operator << (SvStream &, const SvBOOL &);
    friend SvStream& operator >> (SvStream &, SvBOOL &);

    sal_Bool        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    sal_Bool        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm );
    rtl::OString    GetSvIdlString( SvStringHashEntry * pName );
};


class SvIdentifier
{
private:
    rtl::OString m_aStr;
public:
    SvIdentifier()
    {
    }
    void setString(const rtl::OString& rStr)
    {
        m_aStr = rStr;
    }
    const rtl::OString& getString() const
    {
        return m_aStr;
    }
    friend SvStream& operator << (SvStream &, const SvIdentifier &);
    friend SvStream& operator >> (SvStream &, SvIdentifier &);

    sal_Bool IsSet() const
    {
        return !m_aStr.isEmpty();
    }
    sal_Bool        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    sal_Bool        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm,
                            sal_uInt16 nTab );
};


class SvNumberIdentifier : public SvIdentifier
{
    sal_uInt32  nValue;
    // must not be used
    sal_Bool    ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
public:
                SvNumberIdentifier() : nValue( 0 ) {};
    sal_Bool        IsSet() const
                {
                    return SvIdentifier::IsSet() || nValue != 0;
                }
    sal_uInt32      GetValue() const { return nValue; }
    void        SetValue( sal_uInt32 nVal ) { nValue = nVal; }

    friend SvStream& operator << (SvStream &, const SvNumberIdentifier &);
    friend SvStream& operator >> (SvStream &, SvNumberIdentifier &);
    sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    sal_Bool        ReadSvIdl( SvIdlDataBase &, SvStringHashEntry * pName,
                           SvTokenStream & rInStm );
};


class SvString
{
private:
    rtl::OString m_aStr;
public:
    SvString() {}
    void setString(const rtl::OString& rStr)
    {
        m_aStr = rStr;
    }
    const rtl::OString& getString() const
    {
        return m_aStr;
    }
    sal_Bool IsSet() const
    {
        return !m_aStr.isEmpty();
    }
    friend SvStream& operator << (SvStream &, const SvString &);
    friend SvStream& operator >> (SvStream &, SvString &);

    sal_Bool        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    sal_Bool        WriteSvIdl( SvStringHashEntry * pName, SvStream & rOutStm,
                            sal_uInt16 nTab );
};


class SvHelpText : public SvString
{
public:
                SvHelpText() {}
    sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    sal_Bool        WriteSvIdl( SvIdlDataBase & rBase, SvStream & rOutStm,
                            sal_uInt16 nTab );
};


class SvHelpContext : public SvNumberIdentifier
{
};

class SvUUId : public SvGlobalName
{
public:
                SvUUId() {}
    sal_Bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
    sal_Bool        WriteSvIdl( SvStream & rOutStm );
};


class SvVersion
{
    sal_uInt16  nMajorVersion;
    sal_uInt16  nMinorVersion;
public:
                SvVersion() : nMajorVersion( 1 ), nMinorVersion( 0 ) {}
    sal_Bool        operator == ( const SvVersion & r )
                {
                    return (r.nMajorVersion == nMajorVersion)
                             && (r.nMinorVersion == nMinorVersion);
                }
    sal_Bool        operator != ( const SvVersion & r )
                {
                    return !(*this == r);
                }

    sal_uInt16      GetMajorVersion() const { return nMajorVersion; }
    sal_uInt16      GetMinorVersion() const { return nMinorVersion; }

    friend SvStream& operator << (SvStream &, const SvVersion &);
    friend SvStream& operator >> (SvStream &, SvVersion &);
    sal_Bool        ReadSvIdl( SvTokenStream & rInStm );
    sal_Bool        WriteSvIdl( SvStream & rOutStm );
};


#endif // _BASTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
