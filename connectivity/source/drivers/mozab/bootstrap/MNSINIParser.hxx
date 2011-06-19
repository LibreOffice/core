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

    inline ini_NameValue() SAL_THROW( () )
        {}
    inline ini_NameValue(
        OUString const & name, OUString const & value ) SAL_THROW( () )
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
    IniParser(OUString const & rIniName) throw(com::sun::star::io::IOException );
#if OSL_DEBUG_LEVEL > 0
    void Dump();
#endif

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
