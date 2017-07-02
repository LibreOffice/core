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

#ifndef INCLUDED_XMLOFF_SOURCE_FORMS_IFACECOMPARE_HXX
#define INCLUDED_XMLOFF_SOURCE_FORMS_IFACECOMPARE_HXX

#include <comphelper/stl_types.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

namespace xmloff
{

    //= OInterfaceCompare
    /** is stl-compliant structure for comparing Reference&lt; &lt;iface&gt; &gt; instances
    */
    template < class IAFCE >
    struct OInterfaceCompare
    {
        bool operator() (const css::uno::Reference< IAFCE >& lhs, const css::uno::Reference< IAFCE >& rhs) const
        {
            return lhs.get() < rhs.get();
                // this does not make any sense if you see the semantics of the pointer returned by get:
                // It's a pointer to a point in memory where an interface implementation lies.
                // But for our purpose (provide a reliable less-operator which can be used with the STL), this is
                // sufficient ....
        }
    };

    typedef OInterfaceCompare< css::beans::XPropertySet >  OPropertySetCompare;
    typedef OInterfaceCompare< css::drawing::XDrawPage >   ODrawPageCompare;

}   // namespace xmloff

#endif // INCLUDED_XMLOFF_SOURCE_FORMS_IFACECOMPARE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
