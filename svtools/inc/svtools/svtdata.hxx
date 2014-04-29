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



#ifndef _SVTOOLS_SVTDATA_HXX
#define _SVTOOLS_SVTDATA_HXX

#include "svtools/svtdllapi.h"
#include <tools/resid.hxx>
#include <com/sun/star/lang/Locale.hpp>

class ResMgr;
class SfxItemDesruptorList_Impl;

//============================================================================
class ImpSvtData
{
public:
    SfxItemDesruptorList_Impl * pItemDesruptList;
    ResMgr *        pResMgr;

private:
    ImpSvtData():
        pItemDesruptList(0), pResMgr(0)
    {}

    ~ImpSvtData();

public:
    ResMgr * GetResMgr(const ::com::sun::star::lang::Locale aLocale);
    ResMgr * GetResMgr(); // VCL dependent, only available in SVT, not in SVL!

    static ImpSvtData & GetSvtData();
};

//============================================================================
class SVT_DLLPUBLIC SvtResId: public ResId
{
public:
    SvtResId(sal_uInt16 nId, const ::com::sun::star::lang::Locale aLocale);
    SvtResId(sal_uInt16 nId);
     // VCL dependent, only available in SVT, not in SVL!
};

#endif //  _SVTOOLS_SVTDATA_HXX

