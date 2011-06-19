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
