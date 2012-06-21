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
