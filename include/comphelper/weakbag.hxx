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

#ifndef INCLUDED_COMPHELPER_WEAKBAG_HXX
#define INCLUDED_COMPHELPER_WEAKBAG_HXX

#include <sal/config.h>

#include <list>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weakref.hxx>
#include <osl/diagnose.h>

namespace comphelper {

/**
   A bag of UNO weak references.
*/
template< typename T > class WeakBag {
public:
    /**
       Add a new weak reference.

       The implementation keeps the amount of memory consumed linear in the
       number of living references added, not linear in the number of total
       references added.

       @param e
       a non-null reference.
    */
    void add(css::uno::Reference< T > const & e) {
        OSL_ASSERT(e.is());
        for (typename WeakReferenceList::iterator i(m_list.begin()); i != m_list.end();) {
            if (css::uno::Reference< T >(*i).is()) {
                ++i;
            } else {
                i = m_list.erase(i);
            }
        }
        m_list.push_back(css::uno::WeakReference< T >(e));
    }

    /**
       Remove a living reference.

       @return
       a living reference, or null if there are none.
    */
    css::uno::Reference< T > remove() {
        while (!m_list.empty()) {
            css::uno::Reference< T > r(m_list.front());
            m_list.pop_front();
            if (r.is()) {
                return r;
            }
        }
        return css::uno::Reference< T >();
    }

private:
    typedef std::list< css::uno::WeakReference< T > > WeakReferenceList;

    WeakReferenceList m_list;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
