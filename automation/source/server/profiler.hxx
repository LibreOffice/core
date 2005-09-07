/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: profiler.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:24:45 $
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



#define AVER( pFirst, pSecond, Membername ) (( pFirst->Membername + pSecond->Membername ) / 2 )
#define DIFF( pFirst, pSecond, Membername ) ( pSecond->Membername - pFirst->Membername )
#define S_SAFEDIV( a,b ) ((b)==0?CUniString("#DIV"):UniString::CreateFromInt32( (ULONG) ((a)/(b))))
#define S_SAFEDIV_DEC( a,b ) ((b)==0?CUniString("#DIV"):Dec((ULONG) ((a)/(b))))

#ifndef _TIME_HXX //autogen
#include <tools/time.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

#define PROFILE_START   0x01
#define PROFILE_END     0x02


struct SysdepProfileSnapshot;
struct SysdepStaticData;    // Nicht wirklich statisch, sondern statisch über mehrere Snapshots

struct ProfileSnapshot
{
    Time aTime;
    SysdepProfileSnapshot *pSysdepProfileSnapshot;
    ULONG nProcessTicks;
    ULONG nSystemTicks;
};


class TTProfiler : private Timer
{
public:
    TTProfiler();
    ~TTProfiler();

    String GetProfileHeader();  // Titelzeile für Logdatei
    void StartProfileInterval( BOOL bReadAnyway = FALSE );  // Zustand merken
    void EndProfileInterval();  // Informationszeile zusammenbauen
    String GetProfileLine( String &aPrefix );


    void StartProfilingPerCommand();    // Jeden Befehl mitschneiden
    void StopProfilingPerCommand();
    BOOL IsProfilingPerCommand() { return bIsProfilingPerCommand; }

    void StartPartitioning();
    void StopPartitioning();
    BOOL IsPartitioning() { return bIsPartitioning; }
    ULONG GetPartitioningTime();

    void StartAutoProfiling( ULONG nMSec ); // Automatisch alle nMSec Milisekunden sampeln
    String GetAutoProfiling();  // Aktuelle `Sammlung` abholen
    void StopAutoProfiling();   // Sampeln beenden
    BOOL IsAutoProfiling() { return bIsAutoProfiling; }

private:

    void GetProfileSnapshot( ProfileSnapshot *pProfileSnapshot );

    // Informationszeile zusammenbauen
    String GetProfileLine( ProfileSnapshot *pStart, ProfileSnapshot *pStop );


    ProfileSnapshot *pStart;
    ProfileSnapshot *pEnd;
    BOOL bIsProfileIntervalStarted;



//
    BOOL bIsProfilingPerCommand;
    BOOL bIsPartitioning;


//  Für das Automatische Profiling in festen Intervallen

    ProfileSnapshot *pAutoStart;
    ProfileSnapshot *pAutoEnd;
    BOOL bIsAutoProfiling;
    String aAutoProfileBuffer;

    virtual void Timeout();


// Einige Hilfsfunktionen

//  String Hex( ULONG nNr );
    String Dec( ULONG nNr );    // Ergebnis = nNr / 100 mit 2 Dezimalen
    String Pad( const String aS, xub_StrLen nLen );     // Fügt blanks links an den String an

/*  Ab hier werden die Methoden Systemabhängig in den entsprechenden cxx implementiert
    Sie werden von den oberen Methoden gerufen.
*/

    SysdepStaticData *pSysDepStatic;

    void InitSysdepProfiler();
    void DeinitSysdepProfiler();

    SysdepProfileSnapshot *NewSysdepSnapshotData();
    void DeleteSysdepSnapshotData( SysdepProfileSnapshot *pSysdepProfileSnapshot );

    // Titelzeile für Logdatei
    String GetSysdepProfileHeader();

    // Zustand merken
    void GetSysdepProfileSnapshot( SysdepProfileSnapshot *pSysdepProfileSnapshot, USHORT nMode = PROFILE_START | PROFILE_END );

    // Informationszeile zusammenbauen
    String GetSysdepProfileLine( SysdepProfileSnapshot *pStart, SysdepProfileSnapshot *pStop );
};

