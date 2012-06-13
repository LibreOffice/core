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

#define CSV_USE_CSV_ASSERTIONS
#include <cosv/csv_env.hxx>

#include <cosv/comfunc.hxx>
#include <cosv/string.hxx>
#include <cosv/streamstr.hxx>
#include <cosv/std_outp.hxx>
#include <cosv/tpl/dyn.hxx>
#include <cosv/file.hxx>

// NOT FULLY DECLARED SERVICES
#include <ut.hxx>

using csv::File;


/** @file
    UnitTests for class File.
*/

FUT_DECL( File, Read );
FUT_DECL( File, Write );
FUT_DECL( File, Seek );
FUT_DECL( File, SeekBack );
FUT_DECL( File, SeekRelative );


bool
classtest_File()
{
    csv::File aFile( "bigfile.txt", csv::CFM_RW, 0 );
    csv::File & rFile = aFile;

    rFile.Open();

    bool ret = ftest_Read( rFile );
    ret = ftest_Write( rFile ) AND ret;
    ret = ftest_SeekBack( rFile ) AND ret;
    ret = ftest_SeekRelative( rFile ) AND ret;

    rFile.Close();

    return ret;
}



FUT_DECL( File, Read )
{
    bool ret = true;

    rFile.SeekBack(0);
    uintt nSourceSize = rFile.Position();
    rFile.Seek(0);

    char * pBuf = new char[nSourceSize+1];
    uintt nCount = rFile.Read(pBuf,nSourceSize);

    UT_CHECK( Read, nCount == nSourceSize );

    return ret;
}

FUT_DECL( File, Write )
{
    bool ret = true;



    return ret;
}

FUT_DECL( File, Seek )
{
    bool ret = true;



    return ret;
}

FUT_DECL( File, SeekBack )
{
    bool ret = true;



    return ret;
}

FUT_DECL( File, SeekRelative )
{
    bool ret = true;



    return ret;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
