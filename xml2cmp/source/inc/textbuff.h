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

#ifndef CTLS_TEXTBUFF_H
#define CTLS_TEXTBUFF_H

#include "textfile.h"


typedef struct TextBuffer
{
    char *              dpText;
    intt                nSize;

    intt                nPosition;

} TextBuffer;

#define TextBuffer_THIS     TextBuffer * pThis


#define ENDS    '\0'
#define ENDL    '\n'

typedef enum E_TB_Relation
{
    tb_begin,
    tb_cur,
    tb_end
} E_TB_Relation;


void                TextBuffer_CTOR( TextBuffer_THIS,
                        intt                i_nSize );
void                TextBuffer_DTOR( TextBuffer_THIS );
void                TB_Resize( TextBuffer_THIS,
                        intt                i_nNewSize );

Bool                TB_oin( TextBuffer_THIS,                /** operator>>(char *) */
                        char *              i_pText );
Bool                TB_oinChar( TextBuffer_THIS,            /** operator>>(char) */
                        char                i_cChar );
Bool                TB_oinFile( TextBuffer_THIS,            /** operator>>(TextFile*)  */
                        TextFile *          i_pFile );
void                TB_opp( TextBuffer_THIS );              /** operator++ */

intt                TB_Goto( TextBuffer_THIS,
                        intt                i_nPosition,
                        E_TB_Relation       i_nRelation );      /** tb_begin, tb_cur, tb_end */


char *              TB_Text( TextBuffer_THIS );
char                TB_CurChar( TextBuffer_THIS );
char *              TB_CurCharPtr( TextBuffer_THIS );
intt                TB_Size( TextBuffer_THIS );
intt                TB_Position( TextBuffer_THIS );
Bool                TB_EndOfBuffer( TextBuffer_THIS );


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
