/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lst_str.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 11:53:38 $
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

