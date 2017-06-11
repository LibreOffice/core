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

#include <memory>
#include <svtools/svtdllapi.h>

#include <vcl/errinf.hxx>

typedef std::pair<const char*, ErrCode> ErrMsgCode;
SVT_DLLPUBLIC const ErrMsgCode* getRID_ERRHDL();
SVT_DLLPUBLIC const ErrMsgCode* getRID_ERRCTX();

namespace vcl { class Window; }

class SVT_DLLPUBLIC SfxErrorContext : private ErrorContext
{
public:
    SfxErrorContext(
            sal_uInt16 nCtxIdP, vcl::Window *pWin=nullptr,
            const ErrMsgCode* pIds = nullptr, const std::locale* pResLocaleP = nullptr);
    SfxErrorContext(
            sal_uInt16 nCtxIdP, const OUString &aArg1, vcl::Window *pWin=nullptr,
            const ErrMsgCode* pIds = nullptr, const std::locale* pResLocaleP = nullptr);
    bool GetString(ErrCode nErrId, OUString &rStr) override;

private:
    sal_uInt16 nCtxId;
    const ErrMsgCode* pIds;
    const std::locale *pResLocale;
    OUString aArg1;
};

class SVT_DLLPUBLIC SfxErrorHandler : private ErrorHandler
{
public:
    SfxErrorHandler(const ErrMsgCode* pIds, ErrCode lStart, ErrCode lEnd, const std::locale* pResLocale = nullptr);
    virtual ~SfxErrorHandler() override;

protected:
    bool     GetErrorString(ErrCode lErrId, OUString &) const;

private:

    ErrCode              lStart;
    ErrCode              lEnd;
    const ErrMsgCode*    pIds;
    const std::locale*   pResLocale;
    std::unique_ptr<std::locale> xFreeLocale;

    SVT_DLLPRIVATE static void GetClassString(sal_uLong lErrId, OUString &);
    virtual bool          CreateString(const ErrorInfo *, OUString &) const override;
};

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
