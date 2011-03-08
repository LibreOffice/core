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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
