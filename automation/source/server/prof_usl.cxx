/*************************************************************************
 *
 *  $RCSfile: prof_usl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-11-09 16:52:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


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
void TTProfiler::GetSysdepProfileSnapshot( SysdepProfileSnapshot *pSysdepProfileSnapshot, USHORT nMode )
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


    ULONG d_utime = DIFF_MS( pStart, pStop, mpstatus.pr_utime ) + DIFF_MS( pStart, pStop, mpstatus.pr_cutime );
    ULONG d_stime = DIFF_MS( pStart, pStop, mpstatus.pr_stime ) + DIFF_MS( pStart, pStop, mpstatus.pr_cstime );

    aProfile += Pad( String::CreateFromInt64(d_utime), 7 );
    aProfile += Pad( String::CreateFromInt64(d_stime), 7 );
    aProfile += Pad( String::CreateFromInt64(d_utime + d_stime), 7 );

    return aProfile;
};



