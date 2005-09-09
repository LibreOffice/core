/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: str.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 11:54:36 $
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

#ifndef CPV_STR_H
#define CPV_STR_H


#ifdef UNX
#define stricmp(str1,str2)      strcasecmp(str1, str2)
#define strnicmp(str1,str2,n)   strncasecmp(str1, str2, n)
#endif


typedef struct Cstring
{
    char *              dpText;
    intt                nLength;

} Cstring;

#define Cstring_THIS        Cstring * pThis


void                Cstring_CTOR( Cstring_THIS,
                        char *              pText );
void                Cstring_DTOR( Cstring * pThis );

void                Cs_Assign( Cstring_THIS,
                        char *              i_pNewText );
void                Cs_AssignPart( Cstring_THIS,
                        char *              i_pNewText,
                        intt                i_nLength );

void                Cs_AddCs( Cstring_THIS,
                        Cstring *           i_pAddedText );
void                Cs_Add( Cstring_THIS,
                        char *              i_pAddedText );

char *              Cs_Str( Cstring_THIS );
intt                Cs_Length( Cstring_THIS );

void                Cs_ToUpper( Cstring_THIS );



#endif

