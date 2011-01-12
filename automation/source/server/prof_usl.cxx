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



