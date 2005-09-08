/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: future.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:58:17 $
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

#ifndef _SALHELPER_FUTURE_HXX_
#define _SALHELPER_FUTURE_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif

#ifndef _SALHELPER_REFOBJ_HXX_
#include <salhelper/refobj.hxx>
#endif

namespace salhelper
{

//----------------------------------------------------------------------------

#ifndef SALHELPER_COPYCTOR_API
#define SALHELPER_COPYCTOR_API(C) C (const C&); C& operator= (const C&)
#endif

//----------------------------------------------------------------------------

template<class value_type>
class FutureValue : protected osl::Condition
{
    /** Representation.
     */
    value_type m_aValue;

    /** Not implemented.
     */
    SALHELPER_COPYCTOR_API(FutureValue<value_type>);

public:
    inline FutureValue (const value_type& value = value_type()) SAL_THROW(())
        : m_aValue (value)
    {
        Condition::reset();
    }

    inline ~FutureValue() SAL_THROW(())
    {}

    inline sal_Bool is() const SAL_THROW(())
    {
        return const_cast<FutureValue*>(this)->check();
    }

    inline void set (const value_type& value) SAL_THROW(())
    {
        m_aValue = value;
        Condition::set();
    }

    inline value_type& get() SAL_THROW(())
    {
        Condition::wait();
        return m_aValue;
    }
};

//----------------------------------------------------------------------------

template<class value_type>
class Future : public salhelper::ReferenceObject
{
    /** Representation.
     */
    FutureValue<value_type> m_aValue;

    /** Not implemented.
     */
    SALHELPER_COPYCTOR_API(Future<value_type>);

public:
    inline Future (const value_type& value = value_type()) SAL_THROW(())
        : m_aValue (value)
    {}

    inline void set (const value_type& value) SAL_THROW(())
    {
        OSL_PRECOND(!m_aValue.is(), "Future::set(): value already set");
        m_aValue.set (value);
    }

    inline value_type& get() SAL_THROW(())
    {
        return m_aValue.get();
    }
};

//----------------------------------------------------------------------------

} // namespace salhelper

#endif /* !_SALHELPER_FUTURE_HXX_ */
