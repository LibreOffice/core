/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WW8CpAndFc.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:45:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

    snprintf(sBuffer, 255, "%lx", get());

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

    snprintf(sBuffer, 255, "(%lx, %s)", static_cast<sal_uInt32>(get()),
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

    snprintf(sBuffer, 255, "%d", rCpAndFcs.size());
    o << sBuffer;

    return o;
}

CpAndFc::CpAndFc(const Cp & rCp, const Fc & rFc, PropertyType eType_)
: mCp(rCp), mFc(rFc), mType(eType_)
{
}

}}
