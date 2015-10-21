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

#ifndef INCLUDED_SW_SOURCE_CORE_ACCESS_ACCHYPERTEXTDATA_HXX
#define INCLUDED_SW_SOURCE_CORE_ACCESS_ACCHYPERTEXTDATA_HXX

#include <cppuhelper/weakref.hxx>
#include <map>

class SwTextAttr;
namespace com { namespace sun { namespace star {
    namespace accessibility { class XAccessibleHyperlink; }
} } }

class SwAccessibleHyperTextData
{
public:
    typedef const SwTextAttr *                                          key_type;
    typedef css::uno::WeakReference< css::accessibility::XAccessibleHyperlink > mapped_type;
    typedef std::pair<const key_type,mapped_type>                       value_type;
    typedef ::std::less< const SwTextAttr * >                           key_compare;
    typedef std::map<key_type,mapped_type,key_compare>::iterator        iterator;
private:
    std::map<key_type,mapped_type,key_compare> maMap;
public:
    SwAccessibleHyperTextData();
    ~SwAccessibleHyperTextData();

    iterator begin() { return maMap.begin(); }
    iterator end() { return maMap.end(); }
    iterator find(const key_type& key) { return maMap.find(key); }
    std::pair<iterator,bool> insert(const value_type& value ) { return maMap.insert(value); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
