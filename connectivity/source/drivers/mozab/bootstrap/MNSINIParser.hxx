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
#ifndef __MNSINIPARSER_HXX__
#define __MNSINIPARSER_HXX__

#include <rtl/ustring.hxx>
#include <com/sun/star/io/IOException.hpp>
#include <osl/process.h>

#include <map>
#include <list>

#if OSL_DEBUG_LEVEL > 0
#include <stdio.h>
#endif

using ::rtl::OUString;
using ::rtl::OString;

struct ini_NameValue
{
    rtl::OUString sName;
    rtl::OUString sValue;

    inline ini_NameValue() SAL_THROW(())
        {}
    inline ini_NameValue(
        OUString const & name, OUString const & value ) SAL_THROW(())
        : sName( name ),
          sValue( value )
        {}
};

typedef std::list<
    ini_NameValue
> NameValueList;

struct ini_Section
{
    rtl::OUString sName;
    NameValueList lList;
};
typedef std::map<rtl::OUString,
                ini_Section
                >IniSectionMap;


class IniParser
{
    IniSectionMap mAllSection;
public:
    IniSectionMap * getAllSection(){return &mAllSection;};
    IniParser(OUString const & rIniName) throw(com::sun::star::io::IOException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
