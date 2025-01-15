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

#pragma once

#include <svtools/svtdllapi.h>
#include <svtools/svtresid.hxx>
#include <vcl/errinf.hxx>

typedef std::pair<TranslateId, ErrCode> ErrMsgCode;
SVT_DLLPUBLIC extern const ErrMsgCode RID_ERRHDL[];
SVT_DLLPUBLIC extern const ErrMsgCode RID_ERRCTX[];

namespace weld { class Window; }

class SVT_DLLPUBLIC SfxErrorContext : private ErrorContext
{
public:
    SfxErrorContext(
            sal_uInt16 nCtxIdP, weld::Window *pWin=nullptr,
            const ErrMsgCode* pIds = nullptr);
    SfxErrorContext(
            sal_uInt16 nCtxIdP, OUString aArg1, weld::Window *pWin=nullptr,
            const ErrMsgCode* pIds = nullptr);
    bool GetString(const ErrCodeMsg& nErrId, OUString &rStr) override;
private:
    virtual OUString Translate(TranslateId aId) const;
private:
    sal_uInt16 nCtxId;
    const ErrMsgCode* pIds;
    OUString aArg1;
};

class SVT_DLLPUBLIC SfxErrorHandler : private ErrorHandler
{
public:
    SfxErrorHandler(const ErrMsgCode* pIds, ErrCodeArea lStart, ErrCodeArea lEnd, const std::locale& rResLocale = SvtResLocale());
    virtual ~SfxErrorHandler() override;

protected:
    bool     GetErrorString(ErrCode lErrId, OUString &) const;

private:

    ErrCodeArea          lStart;
    ErrCodeArea          lEnd;
    const ErrMsgCode*    pIds;
    std::locale aResLocale;

    SVT_DLLPRIVATE static void GetClassString(ErrCodeClass lErrId, OUString &);
    virtual bool          CreateString(const ErrCodeMsg&, OUString &) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
