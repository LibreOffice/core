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

#pragma once

#include <sal/types.h>
#include <rtl/string.hxx>

class SvStringHashEntry;
class SvIdlDataBase;
class SvStream;
class SvTokenStream;

class SvBOOL
{
    bool  bVal:1,
          bSet:1;
public:
                SvBOOL() { bSet = bVal = false; }
                SvBOOL( bool b ) : bVal( b ), bSet( false) {}
    SvBOOL &    operator = ( bool n ) { bVal = n; bSet = true; return *this; }

    operator    bool() const { return bVal; }
    bool        IsSet() const { return bSet; }

    bool        ReadSvIdl( SvStringHashEntry const * pName, SvTokenStream & rInStm );
};


class SvIdentifier
{
    OString     m_aStr;
    sal_uInt32  nValue;
public:
                SvIdentifier() : nValue( 0 ) {};
                SvIdentifier(sal_uInt32 n) : nValue( n ) {};

    void        setString(const OString& rStr) { m_aStr = rStr; }
    const OString& getString() const { return m_aStr; }

    void        ReadSvIdl( SvStringHashEntry const * pName, SvTokenStream & rInStm );
    bool        IsSet() const
                {
                    return !m_aStr.isEmpty() || nValue != 0;
                }
    sal_uInt32  GetValue() const { return nValue; }
    void        SetValue( sal_uInt32 bVal ) { nValue = bVal; }

    void        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm );
};


bool        ReadStringSvIdl( SvStringHashEntry const * pName, SvTokenStream & rInStm, OString& aString );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
