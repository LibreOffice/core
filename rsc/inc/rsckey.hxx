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


#ifndef _RSCKEY_HXX
#define _RSCKEY_HXX

/****************** C l a s s   F o r w a r d s **************************/
class RscTop;

#include <rscall.h>

typedef struct {
    Atom          nName;
    sal_uInt32          nTyp;
    long            yylval;
} KEY_STRUCT;

class RscNameTable {
    sal_Bool            bSort;      //soll bei jedem einfuegen sortiert werden?
    sal_uInt32          nEntries;   //Anzahl der Eintr�ge
    KEY_STRUCT *    pTable;
public:
            RscNameTable();
            ~RscNameTable();
    void    SetSort( sal_Bool bSorted = sal_True );
    Atom  Put( Atom nName, sal_uInt32 nTyp, long nValue );
    Atom  Put( Atom nName, sal_uInt32 nTyp );
    Atom  Put( const char * pName, sal_uInt32 nTyp, long nValue );
    Atom  Put( const char * pName, sal_uInt32 nTyp );
    Atom  Put( Atom nName, sal_uInt32 nTyp, RscTop * pClass );
    Atom  Put( const char * pName, sal_uInt32 nTyp, RscTop * pClass );

            // sal_True, wurde gefunden
    sal_Bool    Get( Atom nName, KEY_STRUCT * pEle );
};


#endif// _RSCKEY_HXX
