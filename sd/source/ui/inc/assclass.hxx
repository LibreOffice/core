/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: assclass.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:22:10 $
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

#ifndef INC_ASSCLASS
#define INC_ASSCLASS

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef INCLUDED_SDDLLAPI_H
#include "sddllapi.h"
#endif

#define MAX_PAGES 10

class List;
class Control;

class SD_DLLPUBLIC Assistent
{
    List* pPages[MAX_PAGES];
        //enthaelt fuer jede Seite die Controls die
        //korrekt geschaltet werden muessen

    UINT8 nPages;
        //gibt die Anzahl der Seiten an

    UINT8 nCurrentPage;
        //gibt die aktuelle Seite an

    BOOL* pPageStatus;

public:

    Assistent(UINT8 nNoOfPage);

    BOOL IsEnabled( UINT8 nPage );
    void EnablePage( UINT8 nPage );
    void DisablePage( UINT8 nPage );

    BOOL InsertControl(UINT8 nDestPage,Control* pUsedControl);
        //fuegt einer spezifizierten Seite ein Control hinzu

    BOOL NextPage();
        //springt zur naechsten Seite

    BOOL PreviousPage();
        //springt zur vorherigen Seite

    BOOL GotoPage(const UINT8 nPageToGo);
        //springt zu einer angegebenen Seite

    BOOL IsLastPage();
        //gibt an ob die aktuelle Seite die letzte ist

    BOOL IsFirstPage();
        //gibt an ob die aktuelle Seite die erste ist

    UINT8  GetCurrentPage();
        //gibt die aktuelle Seite zurueck

    ~Assistent();
};


#endif
