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

#include <WW8CpAndFc.hxx>

#include <iterator>
#include <algorithm>
#include <string>
#include <map>

namespace writerfilter {
namespace doctok
{
using namespace ::std;

bool operator < (const Cp & rA, const Cp & rB)
{
    return rA.nCp < rB.nCp;
}

bool operator == (const Cp & rA, const Cp & rB)
{
    return rA.nCp == rB.nCp;
}

string Cp::toString() const
{
    char sBuffer[256];

    snprintf(sBuffer, 255, "%" SAL_PRIxUINT32 "", get());

    return string(sBuffer);
}

ostream & operator << (ostream & o, const Cp & rCp)
{
    return o << rCp.toString();
}

bool operator < (const Fc & rA, const Fc & rB)
{
    return rA.mnFc < rB.mnFc;
}

bool operator == (const Fc & rA, const Fc & rB)
{
    return rA.mnFc == rB.mnFc;
}

string Fc::toString() const
{
    char sBuffer[256];

    snprintf(sBuffer, 255, "(%" SAL_PRIxUINT32 ", %s)", static_cast<sal_uInt32>(get()),
             isComplex() ? "true" : "false");

    return string(sBuffer);
}

ostream & operator << (ostream & o, const Fc & rFc)
{

    return o << rFc.toString();
}

bool operator < (const CpAndFc & rA, const CpAndFc & rB)
{
    bool bResult = false;

    if (rA.mCp < rB.mCp)
        bResult = true;
    else if (rA.mCp == rB.mCp && rA.mType < rB.mType)
        bResult = true;

    return bResult;
}

bool operator == (const CpAndFc & rA, const CpAndFc & rB)
{
    return rA.mCp == rB.mCp;
}

ostream & operator << (ostream & o, const CpAndFc & /*rCpAndFc*/)
{
    return o;
}

ostream & operator << (ostream & o, const CpAndFcs & rCpAndFcs)
{
    copy(rCpAndFcs.begin(), rCpAndFcs.end(),
         ostream_iterator<CpAndFc>(o, ", "));

    char sBuffer[256];

    snprintf(sBuffer, 255, "%" SAL_PRI_SIZET "u", rCpAndFcs.size());
    o << sBuffer;

    return o;
}

CpAndFc::CpAndFc(const Cp & rCp, const Fc & rFc, PropertyType eType_)
: mCp(rCp), mFc(rFc), mType(eType_)
{
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
