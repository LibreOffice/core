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
#ifndef INCLUDED_QNAME_TO_STRING_HXX
#define INCLUDED_QNAME_TO_STRING_HXX

#include <WriterFilterDllApi.hxx>
#include <boost/shared_ptr.hpp>
#include <map>
#include <string>
#include <iostream>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace writerfilter
{
using namespace ::std;

class QNameToString
{
    typedef boost::shared_ptr<QNameToString> Pointer_t;
    typedef map < Id, string > Map;

    static Pointer_t pInstance;

    void init_ooxml();

    Map mMap;

protected:
    /**
       Generated.
     */
    QNameToString();

public:
    static Pointer_t Instance();

    string operator()(Id qName);
};

class SprmIdToString
{
    typedef boost::shared_ptr<SprmIdToString> Pointer_t;

    static Pointer_t pInstance;

    map<sal_uInt32, string> mMap;

protected:
    /**
       Generated automatically.
    */
    SprmIdToString();

public:
    static Pointer_t Instance();
    string operator()(sal_uInt32 nId);
};



}

#endif // INCLUDED_QNAME_TO_STRING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
