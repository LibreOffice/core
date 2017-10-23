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

#ifndef INCLUDED_CPPU_ENTERABLE_HXX
#define INCLUDED_CPPU_ENTERABLE_HXX

#include "uno/Enterable.h"
#include "rtl/ustring.hxx"

namespace cppu
{
/** C++ wrapper for binary C Enterable
    (http://wiki.openoffice.org/wiki/Uno/Cpp/Spec/Environment_Stack)

    @see uno_Enterable
    @since UDK 3.2.7
*/
class Enterable : public uno_Enterable
{
public:
    /* These methods need to be implemented in a derived class.
     */
    virtual void v_enter()                                      = 0;
    virtual void v_leave()                                      = 0;
    virtual void v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam) = 0;
    virtual void v_callOut_v (uno_EnvCallee * pCallee, va_list * pParam) = 0;
    virtual bool v_isValid   (rtl::OUString * pReason)                   = 0;

    virtual ~Enterable() {}

public:
    inline explicit Enterable();

    void enter() {m_enter(this);}
    void leave() {m_leave(this);}

    void callInto_v(uno_EnvCallee * pCallee, va_list * pParam) {m_callInto_v(this, pCallee, pParam);}
    void callOut_v (uno_EnvCallee * pCallee, va_list * pParam) {m_callOut_v (this, pCallee, pParam);}

    inline void callInto(uno_EnvCallee * pCallee, ...);
    inline void callOut (uno_EnvCallee * pCallee, ...);

    int  isValid (rtl::OUString * pReason) {return m_isValid(this, &pReason->pData);}

private:
    Enterable(Enterable const &) SAL_DELETED_FUNCTION;
    Enterable & operator = (Enterable const &) SAL_DELETED_FUNCTION;
};

extern "C" inline void Enterable_call_enter (void * context) { static_cast<Enterable *>(context)->v_enter(); }
extern "C" inline void Enterable_call_leave (void * context) { static_cast<Enterable *>(context)->v_leave(); }
extern "C" inline void Enterable_call_callInto_v(void * context, uno_EnvCallee * pCallee, va_list * pParam)
    { static_cast<Enterable *>(context)->v_callInto_v(pCallee, pParam); }
extern "C" inline void Enterable_call_callOut_v (void * context, uno_EnvCallee * pCallee, va_list * pParam)
    { static_cast<Enterable *>(context)->v_callOut_v(pCallee, pParam); }
extern "C" inline int  Enterable_call_isValid   (void * context, rtl_uString ** pReason)
    {return static_cast<Enterable *>(context)->v_isValid(reinterpret_cast<rtl::OUString *>(pReason));}


Enterable::Enterable()
{
    m_enter      = Enterable_call_enter;
    m_leave      = Enterable_call_leave;
    m_callInto_v = Enterable_call_callInto_v;
    m_callOut_v  = Enterable_call_callOut_v;
    m_isValid    = Enterable_call_isValid;
}

void Enterable::callInto(uno_EnvCallee * pCallee, ...)
{
    va_list param;

    va_start(param, pCallee);
    callInto_v(pCallee, &param);
    va_end(param);
}

void Enterable::callOut(uno_EnvCallee * pCallee, ...)
{
    va_list param;

    va_start(param, pCallee);
    callOut_v(pCallee, &param);
    va_end(param);
}

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
