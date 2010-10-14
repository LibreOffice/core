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
#include "precompiled_tools.hxx"

#define _FSTEST_CXX "$Revision: 1.5 $"
#include <tools/solar.h>

#include <stdio.h>
#include <tools/stream.hxx>
#include <tools/fsys.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>

void CheckTimes(DirEntry aDE);

/*
 * main.
 */
int main (int argc, char **argv)
{
    DirEntry aDir;
    if (aDir.Exists())
    {
        aDir.ToAbs();
        String sTmp(aDir.GetFull(FSYS_STYLE_HOST));
        printf("Directory = %s\n", sTmp.GetStr());
        CheckTimes(aDir);
        DirEntry aFile =
            aDir + DirEntry("testfile.txt", FSYS_STYLE_HOST);
        SvFileStream aStream;
        aStream.Open(aFile.GetFull(FSYS_STYLE_HOST), STREAM_WRITE);
        aStream << "Test";
        aStream.Close();
        ULONG i, nWaitFor = 2000 + Time::GetSystemTicks();
        for (i=Time::GetSystemTicks();
                 i < nWaitFor;
                 i = Time::GetSystemTicks())
            ;
        CheckTimes(aDir);
        nWaitFor = 2000 + Time::GetSystemTicks();
        for (i=Time::GetSystemTicks();
                 i < nWaitFor;
                 i = Time::GetSystemTicks())
            ;
        aFile.Kill();
    }
    else
        puts("MakeDir failed!");
    return 0;
}

void CheckTimes(DirEntry aDE)
{
    FileStat aDirStat(aDE);
    aDirStat.Update(aDE);
    Date aDateCreated(aDirStat.DateCreated());
    Date aDateModified(aDirStat.DateModified());
    Time aTimeCreated(aDirStat.TimeCreated());
    Time aTimeModified(aDirStat.TimeModified());
    printf(
        "DirDateCreated = %i, DirTimeCreated = %i\n",
        aDateCreated.GetDate(), aTimeCreated.GetTime());
    printf(
        "DirDateModified = %i, DirTimeModified = %i\n",
        aDateModified.GetDate(), aTimeModified.GetTime());
    return;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
