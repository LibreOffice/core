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
#if 1

#include <sal/types.h>
#include <doctok/WW8Document.hxx>
#include <set>
#include <boost/unordered_map.hpp>
#include <iostream>

namespace writerfilter {
namespace doctok
{
using namespace ::std;

template <class T>
bool operator <= (const T & rA, const T & rB)
{
    return ! ( rB < rA );
}

/**
   A character position.

   This is a wrapper to make the type for WW8 character position (CP)
   distinct from WW8 file character positions (FC).
*/
struct Cp
{
    sal_uInt32 nCp; // the WW8 character position

    Cp() : nCp(0) {}

    Cp(sal_uInt32 nCp_) : nCp(nCp_) {}

    /**
       Returns the WW8 character position.

       @return the WW8 character position
    */
    sal_uInt32 get() const { return nCp; }

    /**
       Sets the WW8 character position.

       @param nCp_    the WW8 character position to set
    */
    void set(sal_uInt32 nCp_) { nCp = nCp_; }

    /**
       Calculate CP moved backward.

       None of the involved CPs is changed.

       @param n     amount of movement

       @return CP moved @n steps backward
     */
    sal_uInt32 operator - (const Cp & rCp) const
    { return nCp - rCp.nCp; }

    /**
       Calculate CP moved forward.

       None of the involved CPs is changed.

       @param n     amount of movement

       @return CP moved @n steps forward
     */
    Cp operator + (sal_uInt32 n) const
    { return Cp(nCp + n); }

    /**
       Advance CP forward.

       @attention The CP is altered.

       @param n      amount of movement

       @return  CP moved @n steps forward
     */
    Cp & operator += (sal_uInt32 n)
    {
        nCp += n;

        return *this;
    }

    /**
       Return string representation of CP.
     */
    string toString() const;

    friend bool operator < (const Cp & rA, const Cp & rB);
    friend bool operator == (const Cp & rA, const Cp & rB);
    friend ostream & operator << (ostream & o, const Cp & rCp);
};

/**
   A file character position.

   This is a wrapper to make the type for WW8 character position (CP)
   distinct from WW8 file character positions (FC).

   \see{Cp}
*/
struct Fc
{
    sal_uInt32 mnFc; // the WW8 character position
    bool mbComplex;

    Fc() : mnFc(0), mbComplex(false) {}

    Fc(sal_uInt32 nFc, bool bComplex = true)
    : mnFc(nFc), mbComplex(bComplex)
    {}

    sal_uInt32 complexFactor() const { return mbComplex ? 1 : 2; }

    /**
       Returns the WW8 character position.

       @return the WW8 character position
    */
    sal_uInt32 get() const { return mnFc; }

    /**
       Sets the WW8 file character position.

       @param nFc    the WW8 file character position to set
    */
    void set(sal_uInt32 nFc) { mnFc = nFc; }

    /**
       Set if the FC is complex.

       @param bComplex      true if FC is set to be complex
     */
    void setComplex(bool bComplex) { mbComplex = bComplex; }

    /**
       Return if FC is complex.

       @retval true   FC is complex
       @retval false  else
     */
    bool isComplex() const { return mbComplex; }

    /**
       Distance of FCs.

       None of the involved FCs is changed.

       @param  rFc      FC to calculate distance to

       @return Distance from @a rFc to this FC
     */
    sal_uInt32 operator - (const Fc & rFc) const
    { return (mnFc - rFc.mnFc) / complexFactor(); }

    /**
       Calculate FC moved backward.

       None of the involved FCs is changed.

       @param n     amount of movement

       @return FC moved @n steps backward
     */
    Fc operator - (sal_uInt32 n) const
    { return Fc(mnFc - n * complexFactor(), mbComplex); }

    /**
       Calculate FC moved forward.

       None of the involved FCs is changed.

       @param n     amount of movement

       @return FC moved @n steps Forward
     */
    Fc operator + (sal_uInt32 n) const
    { return Fc(mnFc + n * complexFactor(), mbComplex); }

    /**
       Return string representation of FC.
     */
    string toString() const;

    friend bool operator < (const Fc & rA, const Fc & rB);
    friend bool operator == (const Fc & rA, const Fc & rB);
    friend ostream & operator << (ostream & o, const Fc & rFc);
};

/**
   A character position and a corresponding file character position
   paired.
 */
class CpAndFc
{
private:
    /**
       character position
    */
    Cp mCp;

    /**
       file character position
    */
    Fc mFc;

    /**
       property type
    */
    PropertyType mType;

public:
    CpAndFc() {}
    CpAndFc(const Cp & rCp, const Fc & rFc, PropertyType eType_);

    /**
       Return character position.
    */
    const Cp & getCp() const { return mCp; }

    /**
       Return file character position.
    */
    const Fc & getFc() const { return mFc; }

    /**
       Return property type.
    */
    PropertyType getType() const { return mType; }

    /**
       Return if FC is complex.

       @retval true    FC is complex
       @retval false   else
     */
    bool isComplex() const { return mFc.isComplex(); }

    /**
       Return the distance to other CpAndFc.

       @param  rCpAndFc    the other CpAndFc

       @return the distance from the CP in @a rCpAndFc to the CP in
       CpAndFc.
     */
    sal_uInt32 operator-(const CpAndFc & rCpAndFc) const
    { return mCp - rCpAndFc.mCp; }

    /**
       Return string representation of the CpAndFc.
    */
    string toString() const;

    friend bool operator < (const CpAndFc & rA, const CpAndFc & rB);
    friend bool operator == (const CpAndFc & rA, const CpAndFc & rB);
    friend ostream & operator << (ostream & o, const CpAndFc & rCpAndFc);
};

struct CpAndFcLess
{
    CpAndFcLess()
    {
    }

    bool operator()(const CpAndFc & rA, const CpAndFc & rB) const
    {
        return rA < rB;
    }

    bool operator()(const CpAndFc & rA, const Cp & rB) const
    {
        return rA.getCp() < rB;
    }

    bool operator()(const Cp & rA, const CpAndFc & rB) const
    {
        return rA < rB.getCp();
    }
};


typedef set<CpAndFc, CpAndFcLess> CpAndFcs;

ostream & operator << (ostream & o, const CpAndFcs & rCpAndFcs);

struct CpHash
{
    size_t operator()(const Cp & rCp) const
    {
        return rCp.get();
    }
};

struct FcHash
{
    size_t operator()(const Fc & rFc) const
    {
        return rFc.get();
    }
};

struct CpEq
{
    bool operator() (const Cp & rA, const Cp &rB) const
    {
        return rA == rB;
    }
};

struct CpAndFcHash
{
    size_t operator()(const CpAndFc & rCpAndFc) const
    {
        CpHash aHash;

        return aHash(rCpAndFc.getCp());
    }
};

typedef boost::unordered_map<Cp, Fc, CpHash, CpEq> Cp2FcHashMap_t;

} // namespace doctok
} // namespace writerfilter

#endif // INCLUDED_WW8_CP_AND_FC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
