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
void TTProfiler::GetSysdepProfileSnapshot( SysdepProfileSnapshot *, sal_uInt16 )
{};


// Informationszeile zusammenbauen
String TTProfiler::GetSysdepProfileLine( SysdepProfileSnapshot *, SysdepProfileSnapshot * )
{
    return String();
};



