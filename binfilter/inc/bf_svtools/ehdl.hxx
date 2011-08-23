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

#include "bf_svtools/svtdllapi.h"

#include <tools/errinf.hxx>

class Window;
class ResMgr;

namespace binfilter
{

class  SfxErrorContext : private ErrorContext
{
public:
    SfxErrorContext(
            USHORT nCtxIdP, const String &aArg1, Window *pWin=0,
            USHORT nResIdP=USHRT_MAX, ResMgr *pMgrP=0);
    virtual BOOL GetString(ULONG nErrId, String &rStr);

private:
    USHORT nCtxId;
    USHORT nResId;
    ResMgr *pMgr;
    String aArg1;
};

class  SfxErrorHandler : private ErrorHandler
{
public:
    SfxErrorHandler(USHORT nId, ULONG lStart, ULONG lEnd, ResMgr *pMgr=0);
    ~SfxErrorHandler();

protected:
    virtual BOOL     GetErrorString(ULONG lErrId, String &, USHORT&) const;
    virtual BOOL     GetMessageString(ULONG lErrId, String &, USHORT&) const;

private:

    ULONG            lStart;
    ULONG            lEnd;
    USHORT           nId;
    ResMgr          *pMgr;
    ResMgr          *pFreeMgr;

    BOOL             GetClassString(ULONG lErrId, String &) const;
    virtual BOOL     CreateString(
                         const ErrorInfo *, String &, USHORT &) const;
};


}

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
