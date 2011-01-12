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
struct SysdepStaticData;    // Nicht wirklich statisch, sondern statisch über mehrere Snapshots

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

    String GetProfileHeader();  // Titelzeile für Logdatei
    void StartProfileInterval( sal_Bool bReadAnyway = sal_False );  // Zustand merken
    void EndProfileInterval();  // Informationszeile zusammenbauen
    String GetProfileLine( String &aPrefix );


    void StartProfilingPerCommand();    // Jeden Befehl mitschneiden
    void StopProfilingPerCommand();
    sal_Bool IsProfilingPerCommand() { return bIsProfilingPerCommand; }

    void StartPartitioning();
    void StopPartitioning();
    sal_Bool IsPartitioning() { return bIsPartitioning; }
    sal_uLong GetPartitioningTime();

    void StartAutoProfiling( sal_uLong nMSec ); // Automatisch alle nMSec Milisekunden sampeln
    String GetAutoProfiling();  // Aktuelle `Sammlung` abholen
    void StopAutoProfiling();   // Sampeln beenden
    sal_Bool IsAutoProfiling() { return bIsAutoProfiling; }

private:

    void GetProfileSnapshot( ProfileSnapshot *pProfileSnapshot );

    // Informationszeile zusammenbauen
    String GetProfileLine( ProfileSnapshot *pStart, ProfileSnapshot *pStop );


    ProfileSnapshot *mpStart;
    ProfileSnapshot *mpEnd;
    sal_Bool bIsProfileIntervalStarted;



//
    sal_Bool bIsProfilingPerCommand;
    sal_Bool bIsPartitioning;


//  Für das Automatische Profiling in festen Intervallen

    ProfileSnapshot *pAutoStart;
    ProfileSnapshot *pAutoEnd;
    sal_Bool bIsAutoProfiling;
    String aAutoProfileBuffer;

    virtual void Timeout();


// Einige Hilfsfunktionen

//  String Hex( sal_uLong nNr );
    String Dec( sal_uLong nNr );    // Ergebnis = nNr / 100 mit 2 Dezimalen
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
    void GetSysdepProfileSnapshot( SysdepProfileSnapshot *pSysdepProfileSnapshot, sal_uInt16 nMode = PROFILE_START | PROFILE_END );

    // Informationszeile zusammenbauen
    String GetSysdepProfileLine( SysdepProfileSnapshot *pStart, SysdepProfileSnapshot *pStop );
};

