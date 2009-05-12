/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: prof_nul.cxx,v $
 * $Revision: 1.5 $
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


#include "profiler.hxx"
#include "rcontrol.hxx"
#include <basic/ttstrhlp.hxx>

struct SysdepProfileSnapshot
{
    // Hier stehen alle Felder zum Speichern eines Snapshot
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
    return String(CUniString("(noch) keine weiteren Daten Implementiert"));
};


// Zustand merken
void TTProfiler::GetSysdepProfileSnapshot( SysdepProfileSnapshot *, USHORT )
{};


// Informationszeile zusammenbauen
String TTProfiler::GetSysdepProfileLine( SysdepProfileSnapshot *, SysdepProfileSnapshot * )
{
    return String();
};



