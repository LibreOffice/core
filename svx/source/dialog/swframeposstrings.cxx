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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/swframeposstrings.hxx>
#include <tools/rc.hxx>
#include <tools/debug.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

class SvxSwFramePosString_Impl : public Resource
{
    friend class SvxSwFramePosString;
    String aStrings[SvxSwFramePosString::STR_MAX];
public:
    SvxSwFramePosString_Impl();
};
SvxSwFramePosString_Impl::SvxSwFramePosString_Impl() :
    Resource(SVX_RES(RID_SVXSW_FRAMEPOSITIONS))
{
    for(sal_uInt16 i = 0; i < SvxSwFramePosString::STR_MAX; i++)
    {
        //string ids have to start at 1
        aStrings[i] = String(SVX_RES(i + 1));
    }
    FreeResource();
}
/*-- 04.03.2004 13:14:48---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxSwFramePosString::SvxSwFramePosString() :
    pImpl(new SvxSwFramePosString_Impl)
{
}
/*-- 04.03.2004 13:14:48---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxSwFramePosString::~SvxSwFramePosString()
{
    delete pImpl;
}
/*-- 04.03.2004 13:14:48---------------------------------------------------

  -----------------------------------------------------------------------*/
const String& SvxSwFramePosString::GetString(StringId eId)
{
    DBG_ASSERT(eId >= 0 && eId < STR_MAX, "invalid StringId");
    if(!(eId >= 0 && eId < STR_MAX))
        eId = LEFT;
    return pImpl->aStrings[eId];
}
