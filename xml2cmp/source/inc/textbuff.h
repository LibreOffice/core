/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textbuff.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 11:54:54 $
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

