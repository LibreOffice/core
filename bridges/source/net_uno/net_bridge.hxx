/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "net_base.hxx"

#include <bridges/net_uno/net_types.hxx>
#include <uno/dispatcher.hxx>
#include <uno/environment.hxx>
#include <uno/mapping.hxx>

namespace net_uno
{
struct Bridge;

struct Mapping : public uno_Mapping
{
    Bridge* m_bridge;
};

struct Bridge
{
    mutable oslInterlockedCount m_ref;
    uno_ExtEnvironment* m_uno_env;
    uno_Environment* m_net_env;

    Mapping m_net2uno;
    Mapping m_uno2net;
    bool m_registered_net2uno;

    Bridge(uno_Environment* net_env, uno_ExtEnvironment* uno_env, bool registered_net2uno);
    ~Bridge();

    void acquire();
    void release();

    void map_uno_to_net_value(void* pUnoData, Value* pValue,
                              typelib_TypeDescriptionReference* pTDRef, bool destructValue);
    void map_net_value_to_uno(void* pUnoData, Value* pValue,
                              typelib_TypeDescriptionReference* pTDRef, bool destructObject,
                              bool assignObject);

    bool call_uno_func(uno_Interface* pUnoI, const typelib_TypeDescription* pMethodTD,
                       typelib_TypeDescriptionReference* pReturnTDRef, int nParams,
                       typelib_MethodParameter* pParams, Value* pArgs, Value* pRet, Value* pExc);
    void call_net_func(IntPtr pNetI, const typelib_TypeDescription* pMethodTD,
                       typelib_TypeDescriptionReference* pReturnTDRef, int nParams,
                       typelib_MethodParameter* pParams, void** pArgs, void* pRet, uno_Any** pExc);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
