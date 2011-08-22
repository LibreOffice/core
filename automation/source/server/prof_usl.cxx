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
    // everything that is always needed during the profile
};


void TTProfiler::InitSysdepProfiler()
{
    if ( !pSysDepStatic )   // Should always be like this!!
        pSysDepStatic = new SysdepStaticData;
    // initialize here

};

void TTProfiler::DeinitSysdepProfiler()
{
    if ( pSysDepStatic )    // Should always be like this!!
    {
        // tidy up here and free storage if applicable

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


String TTProfiler::GetSysdepProfileHeader()
{
    return String::CreateFromAscii(" Size(Kb) ResidentSZ  rtime  ktime  utime  total");
};



void TTProfiler::GetSysdepProfileSnapshot( SysdepProfileSnapshot *pSysdepProfileSnapshot, sal_uInt16 )
{
    SvFileStream aStream( String::CreateFromAscii("/proc/self/psinfo"), STREAM_READ );      // explicit path for UNXSOL!
    if ( aStream.IsOpen() )
    {
        aStream.Read( &(pSysdepProfileSnapshot->mpsinfo), sizeof( psinfo ) );
        aStream.Close();
    }
    SvFileStream anotherStream( String::CreateFromAscii("/proc/self/status"), STREAM_READ );        // explicit path for UNXSOL!
    if ( anotherStream.IsOpen() )
    {
        anotherStream.Read( &(pSysdepProfileSnapshot->mpstatus), sizeof( pstatus ) );
        anotherStream.Close();
    }
    SvFileStream YetAnotherStream( String::CreateFromAscii("/proc/self/usage"), STREAM_READ );      // explicit path for UNXSOL!
    if ( YetAnotherStream.IsOpen() )
    {
        YetAnotherStream.Read( &(pSysdepProfileSnapshot->mprusage), sizeof( prusage ) );
        YetAnotherStream.Close();
    }
};

#define DIFF2( aFirst, aSecond, Membername ) ( aSecond.Membername - aFirst.Membername )
#define CALC_MS( nSec, nNSec ) ( nSec * 1000 + (nNSec+500000) / 1000000 )
#define DIFF_MS( pStart, pEnd, Member ) ( CALC_MS( pEnd->Member.tv_sec, pEnd->Member.tv_nsec ) - CALC_MS( pStart->Member.tv_sec, pStart->Member.tv_nsec ) )

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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
