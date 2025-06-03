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

#ifdef _WIN32
#include <prewin.h>
#include <postwin.h>
#include <comutil.h>
#include <oleauto.h>
#endif

#include <com/sun/star/bridge/oleautomation/Decimal.hpp>


// Decimal support
// Implementation only for windows

class SbxDecimal
{
    friend void releaseDecimalPtr( SbxDecimal*& rpDecimal );

#ifdef _WIN32
    DECIMAL     maDec;
#endif
    sal_Int32       mnRefCount;

public:
    SbxDecimal();
    SbxDecimal( const SbxDecimal& rDec );
    explicit SbxDecimal( const css::bridge::oleautomation::Decimal& rAutomationDec );

    void addRef()
        { mnRefCount++; }

    void fillAutomationDecimal( css::bridge::oleautomation::Decimal& rAutomationDec );

    void setChar( sal_Unicode val );
    void setByte( sal_uInt8 val );
    void setShort( sal_Int16 val );
    void setLong( sal_Int32 val );
    bool setHyper( sal_Int64 val );
    void setUShort( sal_uInt16 val );
    void setULong( sal_uInt32 val );
    bool setUHyper( sal_uInt64 val );
    bool setSingle( float val );
    bool setDouble( double val );
    void setInt( int val );
    void setUInt( unsigned int val );
    bool setString( OUString* pOUString );
    void setDecimal( SbxDecimal const * pDecimal )
    {
#ifdef _WIN32
        if( pDecimal )
            maDec = pDecimal->maDec;
#else
        (void)pDecimal;
#endif
    }

    void setWithOverflow(float val) { HandleFailure(setSingle(val)); }
    void setWithOverflow(double val) { HandleFailure(setDouble(val)); }
    void setWithOverflow(sal_Int64 val) { HandleFailure(setHyper(val)); }
    void setWithOverflow(sal_uInt64 val) { HandleFailure(setUHyper(val)); }

    bool getChar( sal_Unicode& rVal );
    bool getByte( sal_uInt8& rVal );
    bool getShort( sal_Int16& rVal );
    bool getLong( sal_Int32& rVal );
    bool getHyper( sal_Int64& rVal );
    bool getUShort( sal_uInt16& rVal );
    bool getULong( sal_uInt32& rVal );
    bool getUHyper( sal_uInt64& rVal );
    bool getSingle( float& rVal );
    bool getDouble( double& rVal );
    void getString( OUString& rString );

    // Only handles types, which have corresponding getWithOverflow_impl
    template <typename T> T getWithOverflow()
    {
        T n = 0;
        HandleFailure(getWithOverflow_impl(n));
        return n;
    }

    bool operator -= ( const SbxDecimal &r );
    bool operator += ( const SbxDecimal &r );
    bool operator /= ( const SbxDecimal &r );
    bool operator *= ( const SbxDecimal &r );
    bool neg();

    bool isZero() const;

    // must match the return values of the Microsoft VarDecCmp Automation function
    enum class CmpResult { LT, EQ, GT };
    friend CmpResult compare( const SbxDecimal &rLeft, const SbxDecimal &rRight );

private:
    bool getWithOverflow_impl(sal_Unicode& n) { return getChar(n); }
    bool getWithOverflow_impl(sal_uInt8& n) { return getByte(n); }
    bool getWithOverflow_impl(sal_Int16& n) { return getShort(n); }
    bool getWithOverflow_impl(sal_uInt16& n) { return getUShort(n); }
    bool getWithOverflow_impl(sal_Int32& n) { return getLong(n); }
    bool getWithOverflow_impl(sal_uInt32& n) { return getULong(n); }
    bool getWithOverflow_impl(sal_Int64& n) { return getHyper(n); }
    bool getWithOverflow_impl(sal_uInt64& n) { return getUHyper(n); }
    bool getWithOverflow_impl(float& n) { return getSingle(n); }
    bool getWithOverflow_impl(double& n) { return getDouble(n); }

    void HandleFailure(bool isSuccess);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
