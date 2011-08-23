/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

ostream & operator << (ostream & o, const CpAndFc & rCpAndFc)
{
    return o << rCpAndFc.toString(); 
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
