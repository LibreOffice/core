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

#ifndef INCLUDED_cppu_Enterable_hxx
#define INCLUDED_cppu_Enterable_hxx

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
    virtual void v_enter     (void)                                      = 0;
    virtual void v_leave     (void)                                      = 0;
    virtual void v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam) = 0;
    virtual void v_callOut_v (uno_EnvCallee * pCallee, va_list * pParam) = 0;
    virtual int  v_isValid   (rtl::OUString * pReason)                   = 0;

    virtual ~Enterable() {}

public:
    inline explicit Enterable(void);

    inline void enter(void) {m_enter(this);}
    inline void leave(void) {m_leave(this);}

    inline void callInto_v(uno_EnvCallee * pCallee, va_list * pParam) {m_callInto_v(this, pCallee, pParam);}
    inline void callOut_v (uno_EnvCallee * pCallee, va_list * pParam) {m_callOut_v (this, pCallee, pParam);}

    inline void callInto(uno_EnvCallee * pCallee, ...);
    inline void callOut (uno_EnvCallee * pCallee, ...);

    inline int  isValid (rtl::OUString * pReason) {return m_isValid(this, (rtl_uString **)pReason);}

private:
    Enterable(Enterable const &);
    Enterable & operator = (Enterable const &);
};

extern "C" inline void Enterable_call_enter (void * context) { ((Enterable *)context)->v_enter(); }
extern "C" inline void Enterable_call_leave (void * context) { ((Enterable *)context)->v_leave(); }
extern "C" inline void Enterable_call_callInto_v(void * context, uno_EnvCallee * pCallee, va_list * pParam)
    { ((Enterable *)context)->v_callInto_v(pCallee, pParam); }
extern "C" inline void Enterable_call_callOut_v (void * context, uno_EnvCallee * pCallee, va_list * pParam)
    { ((Enterable *)context)->v_callOut_v(pCallee, pParam); }
extern "C" inline int  Enterable_call_isValid   (void * context, rtl_uString ** pReason)
    {return ((Enterable *)context)->v_isValid((rtl::OUString *)pReason);}


Enterable::Enterable(void)
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
