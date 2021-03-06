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

//for MAccessible.cxx
struct ltComp
{
    bool operator()(REFGUID  rguid1, REFGUID  rguid2) const
    {
        if(reinterpret_cast<LONG const *>(&rguid1)[0] < reinterpret_cast<LONG const *>(&rguid2)[0])
            return true;
        else if(reinterpret_cast<LONG const *>(&rguid1)[0] > reinterpret_cast<LONG const *>(&rguid2)[0])
            return false;
        if(reinterpret_cast<LONG const *>(&rguid1)[1] < reinterpret_cast<LONG const *>(&rguid2)[1])
            return true;
        else if(reinterpret_cast<LONG const *>(&rguid1)[1] > reinterpret_cast<LONG const *>(&rguid2)[1])
            return false;
        if(reinterpret_cast<LONG const *>(&rguid1)[2] < reinterpret_cast<LONG const *>(&rguid2)[2])
            return true;
        else if(reinterpret_cast<LONG const *>(&rguid1)[2] > reinterpret_cast<LONG const *>(&rguid2)[2])
            return false;
        if(reinterpret_cast<LONG const *>(&rguid1)[3] < reinterpret_cast<LONG const *>(&rguid2)[3])
            return true;
        else if(reinterpret_cast<LONG const *>(&rguid1)[3] > reinterpret_cast<LONG const *>(&rguid2)[3])
            return false;
        return false;
    }
};

enum DM_NIR {
    DM_FIRSTCHILD = 0x00,
    DM_LASTCHILD = 0x01,
    DM_NEXTCHILD = 0x02,
    DM_PREVCHILD = 0x03
};


#define SELECT_STR       L"Select"
#define PRESS_STR        L"Press"
#define UNCHECK_STR      L"UnCheck"
#define CHECK_STR        L"Check"
//End

template<typename T, typename Ifc> HRESULT
createInstance(REFIID iid, Ifc ** ppIfc)
{
    return CComCreator< CComObject<T> >::CreateInstance(0, iid, reinterpret_cast<void**>(ppIfc));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
