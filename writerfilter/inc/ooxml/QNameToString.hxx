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
#ifndef INCLUDED_WRITERFILTER_INC_OOXML_QNAMETOSTRING_HXX
#define INCLUDED_WRITERFILTER_INC_OOXML_QNAMETOSTRING_HXX

#include <memory>
#include <map>
#include <string>
#include <iostream>
#include <dmapper/resourcemodel.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace writerfilter
{

class QNameToString
{
    typedef std::shared_ptr<QNameToString> Pointer_t;
    typedef std::map < Id, std::string > Map;

    static Pointer_t pInstance;

    void init();

    Map mMap;

protected:
    /**
       Generated.
     */
    QNameToString();

public:
    static Pointer_t const & Instance();

    std::string operator()(Id qName);
};

}

#endif // INCLUDED_WRITERFILTER_INC_OOXML_QNAMETOSTRING_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
