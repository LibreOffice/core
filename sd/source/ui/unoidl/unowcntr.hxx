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

#ifndef INCLUDED_SD_SOURCE_UI_UNOIDL_UNOWCNTR_HXX
#define INCLUDED_SD_SOURCE_UI_UNOIDL_UNOWCNTR_HXX

#include <cppuhelper/weakref.hxx>
#include <list>

typedef bool (*weakref_searchfunc)( css::uno::WeakReference< css::uno::XInterface > xRef, void* pSearchData );

typedef ::std::list< css::uno::WeakReference< css::uno::XInterface >* > WeakRefList;

class SvUnoWeakContainer
{
private:
    WeakRefList maList;

public:
    SvUnoWeakContainer() throw();
    ~SvUnoWeakContainer() throw();

    /** inserts the given ref into this container */
    void insert( css::uno::WeakReference< css::uno::XInterface > xRef ) throw();

    /** searches the container for a ref that returns true on the given
        search function
    */
    bool findRef( css::uno::WeakReference< css::uno::XInterface >& rRef, void* pSearchData, weakref_searchfunc pSearchFunc );

    void dispose();
};

#endif // INCLUDED_SD_SOURCE_UI_UNOIDL_UNOWCNTR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
