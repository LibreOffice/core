/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: assclass.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:40:55 $
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
    List* mpPages[MAX_PAGES];
        //enthaelt fuer jede Seite die Controls die
        //korrekt geschaltet werden muessen

    int mnPages;
        //gibt die Anzahl der Seiten an

    int mnCurrentPage;
        //gibt die aktuelle Seite an

    bool* mpPageStatus;

public:

    Assistent(int nNoOfPage);

    bool IsEnabled( int nPage );
    void EnablePage( int nPage );
    void DisablePage( int nPage );

    bool InsertControl(int nDestPage,Control* pUsedControl);
        //fuegt einer spezifizierten Seite ein Control hinzu

    bool NextPage();
        //springt zur naechsten Seite

    bool PreviousPage();
        //springt zur vorherigen Seite

    bool GotoPage(const int nPageToGo);
        //springt zu einer angegebenen Seite

    bool IsLastPage();
        //gibt an ob die aktuelle Seite die letzte ist

    bool IsFirstPage();
        //gibt an ob die aktuelle Seite die erste ist

    int  GetCurrentPage();
        //gibt die aktuelle Seite zurueck

    ~Assistent();
};


#endif
