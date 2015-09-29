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
    bool        IsSet() const { return bSet; }

    friend SvStream& operator >> (SvStream &, SvBOOL &);

    bool        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
};


class SvIdentifier
{
    OString     m_aStr;
    sal_uInt32  nValue;
public:
                SvIdentifier() : nValue( 0 ) {};

    void        setString(const OString& rStr) { m_aStr = rStr; }
    const OString& getString() const { return m_aStr; }

    bool        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
    bool        IsSet() const
                {
                    return !m_aStr.isEmpty() || nValue != 0;
                }
    sal_uInt32  GetValue() const { return nValue; }
    void        SetValue( sal_uInt32 nVal ) { nValue = nVal; }

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
    friend SvStream& operator >> (SvStream &, SvString &);

    bool        ReadSvIdl( SvStringHashEntry * pName, SvTokenStream & rInStm );
};


class SvHelpText : public SvString
{
public:
                SvHelpText() {}
    bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
};


class SvHelpContext : public SvIdentifier
{
};

class SvUUId : public SvGlobalName
{
public:
                SvUUId() {}
    bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
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

    friend SvStream& operator >> (SvStream &, SvVersion &);
    bool        ReadSvIdl( SvTokenStream & rInStm );
};


#endif // INCLUDED_IDL_INC_BASTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
