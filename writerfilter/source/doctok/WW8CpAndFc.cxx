/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
