/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fstest.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 22:21:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#define _FSTEST_CXX "$Revision: 1.4 $"

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#include <stdio.h>

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif

#ifndef _DATE_HXX
#include <tools/date.hxx>
#endif

#ifndef _TIME_HXX
#include <tools/time.hxx>
#endif

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

