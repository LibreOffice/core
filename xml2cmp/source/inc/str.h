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
