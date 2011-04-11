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

#ifndef INCLUDED_UCB_CACHEMAPOBJECTCONTAINER2_HXX
#define INCLUDED_UCB_CACHEMAPOBJECTCONTAINER2_HXX

#include "cppuhelper/weakref.hxx"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "sal/types.h"

#include <boost/unordered_map.hpp>

namespace rtl {
    class OUString;
    struct OUStringHash;
}
namespace ucb { namespace cachemap { class Object2; } }

namespace ucb { namespace cachemap {

class ObjectContainer2
{
public:
    ObjectContainer2();

    ~ObjectContainer2() SAL_THROW(());

    rtl::Reference< Object2 > get(rtl::OUString const & rKey);

private:
    typedef boost::unordered_map< rtl::OUString,
                           com::sun::star::uno::WeakReference< Object2 >,
                           rtl::OUStringHash >
    Map;

    ObjectContainer2(ObjectContainer2 &); // not implemented
    void operator =(ObjectContainer2); // not implemented

    Map m_aMap;
    osl::Mutex m_aMutex;
};

} }

#endif // INCLUDED_UCB_CACHEMAPOBJECTCONTAINER2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
