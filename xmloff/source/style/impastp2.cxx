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

#include <rtl/ustrbuf.hxx>
#include "impastpl.hxx"

using namespace std;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

//#############################################################################
//
// Class SvXMLAutoStylePoolProperties_Impl
//

///////////////////////////////////////////////////////////////////////////////
//
// ctor class SvXMLAutoStylePoolProperties_Impl
//

SvXMLAutoStylePoolPropertiesP_Impl::SvXMLAutoStylePoolPropertiesP_Impl( XMLFamilyData_Impl& rFamilyData, const vector< XMLPropertyState >& rProperties )
: maProperties( rProperties ),
  mnPos       ( rFamilyData.mnCount )
{
    // create a name that hasn't been used before. The created name has not
    // to be added to the array, because it will never tried again
    OUStringBuffer sBuffer( 7 );
    do
    {
        rFamilyData.mnName++;
        sBuffer.append( rFamilyData.maStrPrefix );
        sBuffer.append( OUString::valueOf( (sal_Int32)rFamilyData.mnName ) );
        msName = sBuffer.makeStringAndClear();
    }
    while( rFamilyData.mpNameList->find(msName) != rFamilyData.mpNameList->end() );
}

bool operator<( const XMLFamilyData_Impl& r1, const XMLFamilyData_Impl& r2)
{
    return r1.mnFamily < r2.mnFamily;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
