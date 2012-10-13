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
