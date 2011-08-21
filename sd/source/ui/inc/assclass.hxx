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

#ifndef INC_ASSCLASS
#define INC_ASSCLASS

#include <vector>
#include <boost/scoped_array.hpp>

#include <tools/solar.h>
#include "sddllapi.h"

#define MAX_PAGES 10

class Control;

class SD_DLLPUBLIC Assistent
{
    std::vector<Control*> maPages[MAX_PAGES];
        //enthaelt fuer jede Seite die Controls die
        //korrekt geschaltet werden muessen

    int mnPages;
        //gibt die Anzahl der Seiten an

    int mnCurrentPage;
        //gibt die aktuelle Seite an

    boost::scoped_array<bool> mpPageStatus;

public:

    Assistent(int nNoOfPage);

    bool IsEnabled ( int nPage ) const;
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

    bool IsLastPage() const;
        //gibt an ob die aktuelle Seite die letzte ist

    bool IsFirstPage() const;
        //gibt an ob die aktuelle Seite die erste ist

    int  GetCurrentPage() const;
        //gibt die aktuelle Seite zurueck
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
