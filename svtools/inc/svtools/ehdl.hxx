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



#ifndef _EHDL_HXX
#define _EHDL_HXX

#ifndef __RSC

#include "svtools/svtdllapi.h"

#ifndef _EINF_HXX
#include <tools/errinf.hxx>
#endif

class Window;
class ResMgr;

class SVT_DLLPUBLIC SfxErrorContext : private ErrorContext
{
public:
    SfxErrorContext(
            sal_uInt16 nCtxIdP, Window *pWin=0,
            sal_uInt16 nResIdP=USHRT_MAX, ResMgr *pMgrP=0);
    SfxErrorContext(
            sal_uInt16 nCtxIdP, const String &aArg1, Window *pWin=0,
            sal_uInt16 nResIdP=USHRT_MAX, ResMgr *pMgrP=0);
    virtual sal_Bool GetString(sal_uLong nErrId, String &rStr);

private:
    sal_uInt16 nCtxId;
    sal_uInt16 nResId;
    ResMgr *pMgr;
    String aArg1;
};

class SVT_DLLPUBLIC SfxErrorHandler : private ErrorHandler
{
public:
    SfxErrorHandler(sal_uInt16 nId, sal_uLong lStart, sal_uLong lEnd, ResMgr *pMgr=0);
    ~SfxErrorHandler();

protected:
    virtual sal_Bool     GetErrorString(sal_uLong lErrId, String &, sal_uInt16&) const;
    virtual sal_Bool     GetMessageString(sal_uLong lErrId, String &, sal_uInt16&) const;

private:

    sal_uLong            lStart;
    sal_uLong            lEnd;
    sal_uInt16           nId;
    ResMgr          *pMgr;
    ResMgr          *pFreeMgr;

//#if 0 // _SOLAR__PRIVATE
    SVT_DLLPRIVATE sal_Bool             GetClassString(sal_uLong lErrId, String &) const;
//#endif
    virtual sal_Bool     CreateString(
                         const ErrorInfo *, String &, sal_uInt16 &) const;
};

#endif

#endif

