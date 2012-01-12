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

