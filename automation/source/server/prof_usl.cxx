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
#include "precompiled_automation.hxx"


#include <procfs.h>
#include <tools/stream.hxx>
#include "profiler.hxx"


struct SysdepProfileSnapshot
{
    pstatus mpstatus;
    psinfo mpsinfo;
    prusage mprusage;
};


struct SysdepStaticData
{
    // Hier steht alles, was während des Profiles ständig gebraucht wird
};


void TTProfiler::InitSysdepProfiler()
{
    if ( !pSysDepStatic )   // Sollte immer so sein!!
        pSysDepStatic = new SysdepStaticData;
    // Hier initialisieren

};

void TTProfiler::DeinitSysdepProfiler()
{
    if ( pSysDepStatic )    // Sollte immer so sein!!
    {
        // Hier aufräumen und eventuell Speicher freigeben

        delete pSysDepStatic;
    }
};

SysdepProfileSnapshot *TTProfiler::NewSysdepSnapshotData()
{
    return new SysdepProfileSnapshot;
};

void TTProfiler::DeleteSysdepSnapshotData( SysdepProfileSnapshot *pSysdepProfileSnapshot )
{
    delete pSysdepProfileSnapshot;
};


// Titelzeile für Logdatei
String TTProfiler::GetSysdepProfileHeader()
{
    return String::CreateFromAscii(" Size(Kb) ResidentSZ  rtime  ktime  utime  total");
};


// Zustand merken
void TTProfiler::GetSysdepProfileSnapshot( SysdepProfileSnapshot *pSysdepProfileSnapshot, sal_uInt16 )
{
    SvFileStream aStream( String::CreateFromAscii("/proc/self/psinfo"), STREAM_READ );      // Das ist ein expliziter Pfad für UNXSOL!
    if ( aStream.IsOpen() )
    {
        aStream.Read( &(pSysdepProfileSnapshot->mpsinfo), sizeof( psinfo ) );
        aStream.Close();
    }
    SvFileStream anotherStream( String::CreateFromAscii("/proc/self/status"), STREAM_READ );        // Das ist ein expliziter Pfad für UNXSOL!
    if ( anotherStream.IsOpen() )
    {
        anotherStream.Read( &(pSysdepProfileSnapshot->mpstatus), sizeof( pstatus ) );
        anotherStream.Close();
    }
    SvFileStream YetAnotherStream( String::CreateFromAscii("/proc/self/usage"), STREAM_READ );      // Das ist ein expliziter Pfad für UNXSOL!
    if ( YetAnotherStream.IsOpen() )
    {
        YetAnotherStream.Read( &(pSysdepProfileSnapshot->mprusage), sizeof( prusage ) );
        YetAnotherStream.Close();
    }
};

#define DIFF2( aFirst, aSecond, Membername ) ( aSecond.Membername - aFirst.Membername )
#define CALC_MS( nSec, nNSec ) ( nSec * 1000 + (nNSec+500000) / 1000000 )
#define DIFF_MS( pStart, pEnd, Member ) ( CALC_MS( pEnd->Member.tv_sec, pEnd->Member.tv_nsec ) - CALC_MS( pStart->Member.tv_sec, pStart->Member.tv_nsec ) )
// Informationszeile zusammenbauen
String TTProfiler::GetSysdepProfileLine( SysdepProfileSnapshot *pStart, SysdepProfileSnapshot *pStop )
{
    String aProfile;

    aProfile += Pad( String::CreateFromInt64(pStop->mpsinfo.pr_size), 9);
    aProfile += Pad( String::CreateFromInt64(pStop->mpsinfo.pr_rssize), 11);


    aProfile += Pad( String::CreateFromInt64(DIFF_MS( pStart, pStop, mprusage.pr_rtime ) / AVER( pStart, pStop, mprusage.pr_count )), 7 );


    sal_uLong d_utime = DIFF_MS( pStart, pStop, mpstatus.pr_utime ) + DIFF_MS( pStart, pStop, mpstatus.pr_cutime );
    sal_uLong d_stime = DIFF_MS( pStart, pStop, mpstatus.pr_stime ) + DIFF_MS( pStart, pStop, mpstatus.pr_cstime );

    aProfile += Pad( String::CreateFromInt64(d_utime), 7 );
    aProfile += Pad( String::CreateFromInt64(d_stime), 7 );
    aProfile += Pad( String::CreateFromInt64(d_utime + d_stime), 7 );

    return aProfile;
};



