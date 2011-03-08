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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
