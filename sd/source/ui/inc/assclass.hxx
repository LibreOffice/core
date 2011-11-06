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



#ifndef INC_ASSCLASS
#define INC_ASSCLASS

#include <tools/solar.h>
#include "sddllapi.h"

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
