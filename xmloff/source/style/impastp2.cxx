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

#include <rtl/ustrbuf.hxx>
#include "impastpl.hxx"

using namespace std;

// Class SvXMLAutoStylePoolProperties_Impl
// ctor class SvXMLAutoStylePoolProperties_Impl

XMLAutoStylePoolProperties::XMLAutoStylePoolProperties( XMLAutoStyleFamily& rFamilyData, const vector< XMLPropertyState >& rProperties )
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

bool operator<( const XMLAutoStyleFamily& r1, const XMLAutoStyleFamily& r2)
{
    return r1.mnFamily < r2.mnFamily;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
