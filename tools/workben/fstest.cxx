/*************************************************************************
 *
 *  $RCSfile: fstest.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _FSTEST_CXX "$Revision: 1.1.1.1 $"

#ifndef _SOLAR_H
#include <solar.h>
#endif

#include <stdio.h>

#ifndef _STREAM_HXX
#include <stream.hxx>
#endif

#ifndef _FSYS_HXX
#include <fsys.hxx>
#endif

#ifndef _DATE_HXX
#include <date.hxx>
#endif

#ifndef _TIME_HXX
#include <time.hxx>
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

