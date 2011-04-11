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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"

#include "jvmargs.hxx"
#include <rtl/ustring.hxx>


#define OUSTR(x) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( x ))

using ::rtl::OUString;
namespace stoc_javavm {

JVM::JVM() throw()//: _enabled(sal_False)
{
}

void JVM::pushProp(const OUString & property)
{
    sal_Int32 index = property.indexOf((sal_Unicode)'=');
    if(index > 0)
    {
        OUString left = property.copy(0, index).trim();
        OUString right(property.copy(index + 1).trim());
        _props.push_back(property);
    }
    else
    { // no '=', could be -X
        _props.push_back(property);
    }
}


const ::std::vector< ::rtl::OUString > & JVM::getProperties() const
{
    return _props;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
