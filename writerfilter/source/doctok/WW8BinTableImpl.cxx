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



#include "WW8BinTableImpl.hxx"

namespace writerfilter {
namespace doctok
{
using namespace ::std;

void PageNumber::dump(OutputWithDepth<string> & /*output*/) const
{
}

sal_uInt32 WW8BinTableImpl::getPageNumber(const Fc & rFc) const
{
    sal_uInt32 nResult = 0;

    if (mPageMap.find(rFc) == mPageMap.end())
    {
#if 0
        sal_uInt32 n = getEntryCount();

        while (rFc < getFc(n))
        {
            --n;
        }

        nResult = getPageNumber(n);
        mPageMap[rFc] = nResult;
#else
        sal_uInt32 left = 0;
        sal_uInt32 right = getEntryCount();

        while (right - left > 1)
        {
            sal_uInt32 middle = (right + left) / 2;

            Fc aFc = getFc(middle);

            if (rFc < aFc)
                right = middle;
            else
                left = middle;

        }

        nResult = getPageNumber(left);
        mPageMap[rFc] = nResult;
#endif

    }
    else
        nResult = mPageMap[rFc];

    return nResult;
}

string WW8BinTableImpl::toString() const
{
    string aResult;
    char sBuffer[255];

    aResult += "(";

    for (sal_uInt32 n = 0; n < getEntryCount(); n++)
    {
        if (n > 0)
            aResult += ", ";

        snprintf(sBuffer, 255, "%" SAL_PRIxUINT32, getFc(n).get());
        aResult += sBuffer;
        aResult += "->";
        snprintf(sBuffer, 255, "%" SAL_PRIxUINT32, getPageNumber(n));
        aResult += sBuffer;
    }

    aResult += ")";

    return aResult;
}
}}
