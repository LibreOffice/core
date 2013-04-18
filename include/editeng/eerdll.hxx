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

#ifndef _EERDLL_HXX
#define _EERDLL_HXX

class GlobalEditData;

#include <tools/resid.hxx>
#include <tools/shl.hxx>
#include <editeng/editengdllapi.h>

class EDITENG_DLLPUBLIC EditResId: public ResId
{
public:
    EditResId( sal_uInt16 nId );
};

class EditDLL
{
    ResMgr*         pResMgr;
    GlobalEditData* pGlobalData;

public:
    EditDLL();
    ~EditDLL();

    ResMgr*         GetResMgr() const       { return pResMgr; }
    GlobalEditData* GetGlobalData() const   { return pGlobalData; }
    static EditDLL& Get();
};

#define EE_DLL() EditDLL::Get()

#define EE_RESSTR(x) EditResId(x).toString()

#endif //_EERDLL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
