/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

