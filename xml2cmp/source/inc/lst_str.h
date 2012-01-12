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



#ifndef CPV_LST_STR_H
#define CPV_LST_STR_H

#include "str.h"


typedef struct LSElem
{
    Cstring *           pData;
    struct LSElem * pNext;
} LSElem;


typedef struct LSIterator
{
    LSElem *        pElement;
} LSIterator;


typedef struct ListCstring
{
    LSElem *            dpStart;
    LSElem *            pEnd;
    Bool                bAutoDeleteData;
} ListCstring;


#define ListCstring_THIS    ListCstring * pThis
#define LSIterator_THIS LSIterator * pThis
#define LSElem_THIS     LSElem * pThis



void                ListCstring_CTOR( ListCstring_THIS,
                        Bool                i_bAutoDeleteData );
void                ListCstring_DTOR( ListCstring_THIS );

void                LS_Add( ListCstring_THIS,
                        Cstring *           i_pData );
void                LS_Empty( ListCstring_THIS,
                        Bool                i_bDeleteData );
void                LS_Append( ListCstring_THIS,
                        char *              i_sStrings[],
                        intt                i_nNrOfStrings );
Bool                LS_IsEmpty( ListCstring_THIS );

LSIterator          LS_Begin( ListCstring_THIS );


void                LSIterator_CTOR( LSIterator_THIS,
                        LSElem *            i_pElement );
void                LSI_opp( LSIterator_THIS );     /** operator++() */

Bool                LSI_obool( LSIterator_THIS );
Cstring *           LSI_optr( LSIterator_THIS );    /** operator->() */


void                LSElem_CTOR( LSElem_THIS,
                        Cstring *           i_pData );
void                LSElem_DTOR( LSElem_THIS );

Cstring *           LSE_Data( LSElem_THIS );
LSElem *            LSE_Next( LSElem_THIS );

void                LSE_SetNext( LSElem_THIS,
                        LSElem *            i_pNext );
void                LSE_DeleteData( LSElem_THIS );



#endif

