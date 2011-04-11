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

#ifndef _EHDL_HXX
#define _EHDL_HXX

#ifndef __RSC

#include "svtools/svtdllapi.h"

#include <tools/errinf.hxx>

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

    SVT_DLLPRIVATE sal_Bool             GetClassString(sal_uLong lErrId, String &) const;
    virtual sal_Bool     CreateString(
                         const ErrorInfo *, String &, sal_uInt16 &) const;
};

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
