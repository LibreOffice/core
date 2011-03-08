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

#ifndef _SALHELPER_FUTURE_HXX_
#define _SALHELPER_FUTURE_HXX_

#include <sal/types.h>
#include <osl/diagnose.h>
#include <osl/conditn.hxx>
#include <salhelper/refobj.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
