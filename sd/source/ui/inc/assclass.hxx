/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
