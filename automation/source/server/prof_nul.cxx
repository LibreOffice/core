/*************************************************************************
 *
 *  $RCSfile: prof_nul.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mh $ $Date: 2002-11-18 15:27:54 $
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
void TTProfiler::GetSysdepProfileSnapshot( SysdepProfileSnapshot *pSysdepProfileSnapshot, USHORT nMode )
{};


// Informationszeile zusammenbauen
String TTProfiler::GetSysdepProfileLine( SysdepProfileSnapshot *pStart, SysdepProfileSnapshot *pStop )
{
    return String();
};



