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


#ifdef _MSC_VER
#pragma hdrstop
#endif


#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _SW3IMP_HXX
#include <sw3imp.hxx>
#endif

#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
namespace binfilter {

#define STREAM_STGREAD  ( STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE )
#define STREAM_STGWRITE ( STREAM_READ | STREAM_WRITE | STREAM_SHARE_DENYWRITE )

sal_Char __FAR_DATA N_BLOCKINFO[] = "AutotextInfo";


/*************************************************************************
*
* 	Enumeration aller Textbausteine
*
*************************************************************************/

// Das Directory enthaelt die Kurz- und Langnamen der Bausteine. Die
// Bausteine selbst sind unter ihrem Kurznamen in eigenen Streams
// abgelegt. Die Namen selbst sind nach folgender Konvention aufgebaut:
// 1. Zeichen: #
// ! == 0x01
// / == 0x0F
// \ == 0x0C
// : == 0x0A
// . == 0x0E


// Der Fehlercode wird zurueckgeliefert. Das Ende der Liste
// wird durch einen leeren Short-Namen angezeigt.


/*************************************************************************
*
* 	I/O einzelner Textbausteine
*
*************************************************************************/


////////////////////////////////////////////////////////////////////////////


// Textbaustein-Konversionsmode ein/ausschalten
// Das Flag verhindert das Commit auf die Root nach dem Schreiben
// eines Textbausteins; daher wird hier committed, wenn das
// Flag geloescht wird.



// ggf. eine SvPersist-Instanz einrichten

/*N*/ BOOL Sw3IoImp::CheckPersist()
/*N*/ {
/*N*/ 	// Haben wir schon einen Persist?
/*N*/ 	if( pDoc->GetPersist() )
/*N*/ 		return TRUE;
        DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 /*?*/ 	pPersist = new Sw3Persist;
/*N*/ }

// ggf. eine SvPersist-Instanz freigeben


//////////////////////////////////////////////////////////////////////////



}
