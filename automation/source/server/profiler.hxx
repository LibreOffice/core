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



#define AVER( pFirst, pSecond, Membername ) (( pFirst->Membername + pSecond->Membername ) / 2 )
#define DIFF( pFirst, pSecond, Membername ) ( pSecond->Membername - pFirst->Membername )
#define S_SAFEDIV( a,b ) ((b)==0?CUniString("#DIV"):UniString::CreateFromInt32( (ULONG) ((a)/(b))))
#define S_SAFEDIV_DEC( a,b ) ((b)==0?CUniString("#DIV"):Dec((ULONG) ((a)/(b))))

#include <tools/time.hxx>
#include <tools/string.hxx>
#include <vcl/timer.hxx>

#define PROFILE_START   0x01
#define PROFILE_END     0x02


struct SysdepProfileSnapshot;
struct SysdepStaticData;    // not really static but static over several snapshots

struct ProfileSnapshot
{
    Time aTime;
    SysdepProfileSnapshot *pSysdepProfileSnapshot;
    sal_uLong nProcessTicks;
    sal_uLong nSystemTicks;
};


class TTProfiler : private Timer
{
public:
    TTProfiler();
    ~TTProfiler();

    String GetProfileHeader();  // headline for logfile
    void StartProfileInterval( sal_Bool bReadAnyway = sal_False );  // memorize condition
    void EndProfileInterval();  // construct information line
    String GetProfileLine( String &aPrefix );


    void StartProfilingPerCommand();
    void StopProfilingPerCommand();
    sal_Bool IsProfilingPerCommand() { return bIsProfilingPerCommand; }

    void StartPartitioning();
    void StopPartitioning();
    sal_Bool IsPartitioning() { return bIsPartitioning; }
    sal_uLong GetPartitioningTime();

    void StartAutoProfiling( sal_uLong nMSec ); // sample automatically every nMSec milliseconds
    String GetAutoProfiling();  // get current 'collection'
    void StopAutoProfiling();
    sal_Bool IsAutoProfiling() { return bIsAutoProfiling; }

private:

    void GetProfileSnapshot( ProfileSnapshot *pProfileSnapshot );


    String GetProfileLine( ProfileSnapshot *pStart, ProfileSnapshot *pStop );


    ProfileSnapshot *mpStart;
    ProfileSnapshot *mpEnd;
    sal_Bool bIsProfileIntervalStarted;
    sal_Bool bIsProfilingPerCommand;
    sal_Bool bIsPartitioning;


    ProfileSnapshot *pAutoStart;
    ProfileSnapshot *pAutoEnd;
    sal_Bool bIsAutoProfiling;
    String aAutoProfileBuffer;

    virtual void Timeout();



    String Dec( sal_uLong nNr );    // result = nNr / 100 with 2 decimals
    String Pad( const String &aS, xub_StrLen nLen );        // adds blanks links to the string

/*  from now on the methods are implemented system dependent in the respective cxx
    they are called by the methods above
*/

    SysdepStaticData *pSysDepStatic;

    void InitSysdepProfiler();
    void DeinitSysdepProfiler();

    SysdepProfileSnapshot *NewSysdepSnapshotData();
    void DeleteSysdepSnapshotData( SysdepProfileSnapshot *pSysdepProfileSnapshot );


    String GetSysdepProfileHeader();

    void GetSysdepProfileSnapshot( SysdepProfileSnapshot *pSysdepProfileSnapshot, sal_uInt16 nMode = PROFILE_START | PROFILE_END );

    String GetSysdepProfileLine( SysdepProfileSnapshot *pStart, SysdepProfileSnapshot *pStop );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
