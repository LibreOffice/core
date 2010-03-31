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
#ifndef INCLUDED_QNAME_TO_STRING_HXX
#define INCLUDED_QNAME_TO_STRING_HXX

#include <boost/shared_ptr.hpp>
#include <map>
#include <string>
#include <iostream>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace writerfilter
{
using namespace ::std;

class WRITERFILTER_DLLPUBLIC QNameToString
{
    typedef boost::shared_ptr<QNameToString> Pointer_t;
    typedef map < Id, string > Map;

    static Pointer_t pInstance;

    void init_doctok();
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

class WRITERFILTER_DLLPUBLIC SprmIdToString
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
