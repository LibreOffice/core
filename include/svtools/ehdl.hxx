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

#ifndef INCLUDED_SVTOOLS_EHDL_HXX
#define INCLUDED_SVTOOLS_EHDL_HXX

#ifndef __RSC

#include <svtools/svtdllapi.h>

#include <tools/errinf.hxx>

namespace vcl { class Window; }
class ResMgr;

class SVT_DLLPUBLIC SfxErrorContext : private ErrorContext
{
public:
    SfxErrorContext(
            sal_uInt16 nCtxIdP, vcl::Window *pWin=nullptr,
            sal_uInt16 nResIdP=USHRT_MAX, ResMgr *pMgrP=nullptr);
    SfxErrorContext(
            sal_uInt16 nCtxIdP, const OUString &aArg1, vcl::Window *pWin=nullptr,
            sal_uInt16 nResIdP=USHRT_MAX, ResMgr *pMgrP=nullptr);
    bool GetString(sal_uLong nErrId, OUString &rStr) override;

private:
    sal_uInt16 nCtxId;
    sal_uInt16 nResId;
    ResMgr *pMgr;
    OUString aArg1;
};

class SVT_DLLPUBLIC SfxErrorHandler : private ErrorHandler
{
public:
    SfxErrorHandler(sal_uInt16 nId, sal_uLong lStart, sal_uLong lEnd, ResMgr *pMgr=nullptr);
    virtual ~SfxErrorHandler();

protected:
    bool     GetErrorString(sal_uLong lErrId, OUString &, sal_uInt16&) const;
    bool     GetMessageString(sal_uLong lErrId, OUString &, sal_uInt16&) const;

private:

    sal_uLong            lStart;
    sal_uLong            lEnd;
    sal_uInt16           nId;
    ResMgr              *pMgr;
    ResMgr              *pFreeMgr;

    SVT_DLLPRIVATE void   GetClassString(sal_uLong lErrId, OUString &) const;
    virtual bool          CreateString( const ErrorInfo *, OUString &, sal_uInt16 &) const override;
};

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
