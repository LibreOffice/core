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

#include <salhelper/dynload.hxx>
#include <rtl/ustring>
#include "samplelib.hxx"


using namespace salhelper;

using ::rtl::OUString;


class SampleLibLoader
    : public ::salhelper::ODynamicLoader<SampleLib_Api>
{
public:
    SampleLibLoader():
        ::salhelper::ODynamicLoader<SampleLib_Api>
            (::rtl::OUString( SAL_MODULENAME( "samplelib") ),
             ::rtl::OUString( SAMPLELIB_INIT_FUNCTION_NAME) )
        {}

};


int main( int argc, char *argv[ ], char *envp[ ] )
{
    SampleLibLoader Loader;
    SampleLibLoader Loader2;
    Loader= Loader2;
    SampleLib_Api *pApi= Loader.getApi();

    sal_Int32 retint= pApi->funcA( 10);
    double retdouble= pApi->funcB( 3.14);


    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
