/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Enterable.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:32:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_cppu_Enterable_hxx
#define INCLUDED_cppu_Enterable_hxx

#include "uno/Enterable.h"
#include "rtl/ustring.hxx"

namespace cppu
{
/** C++ wrapper for binary C Enterable
    (http://wiki.services.openoffice.org/wiki/Uno/Cpp/Spec/Environment_Stack)

    @see uno_Enterable
    @since UDK 3.2.7
*/
class Enterable : public uno_Enterable
{
public:
    /* These methods need to be implemented in a derived class.
     */
    virtual void v_enter     (void)                                   = 0;
    virtual void v_leave     (void)                                   = 0;
    virtual void v_callInto_v(uno_EnvCallee * pCallee, va_list param) = 0;
    virtual void v_callOut_v (uno_EnvCallee * pCallee, va_list param) = 0;
    virtual int  v_isValid   (rtl::OUString * pReason)                = 0;

    virtual ~Enterable() {};

public:
    inline explicit Enterable(void);

    inline void enter(void) {m_enter(this);};
    inline void leave(void) {m_leave(this);};

    inline void callInto_v(uno_EnvCallee * pCallee, va_list param) {m_callInto_v(this, pCallee, param);};
    inline void callOut_v (uno_EnvCallee * pCallee, va_list param) {m_callOut_v (this, pCallee, param);};

    inline void callInto(uno_EnvCallee * pCallee, ...);
    inline void callOut (uno_EnvCallee * pCallee, ...);

    inline int  isValid (rtl::OUString * pReason) {return m_isValid(this, (rtl_uString **)pReason);}

private:
    Enterable(Enterable const &);
    Enterable & operator = (Enterable const &);
};

extern "C" inline void Enterable_call_enter (void * context) { ((Enterable *)context)->v_enter(); };
extern "C" inline void Enterable_call_leave (void * context) { ((Enterable *)context)->v_leave(); };
extern "C" inline void Enterable_call_callInto_v(void * context, uno_EnvCallee * pCallee, va_list param)
    { ((Enterable *)context)->v_callInto_v(pCallee, param); };
extern "C" inline void Enterable_call_callOut_v (void * context, uno_EnvCallee * pCallee, va_list param)
    { ((Enterable *)context)->v_callOut_v(pCallee, param); };
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
    callInto_v(pCallee, param);
    va_end(param);
}

void Enterable::callOut(uno_EnvCallee * pCallee, ...)
{
    va_list param;

    va_start(param, pCallee);
    callOut_v(pCallee, param);
    va_end(param);
}

}


#endif
