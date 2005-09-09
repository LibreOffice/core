/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textfile.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 11:55:12 $
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

#ifndef CPV_TEXTFILE_H
#define CPV_TEXTFILE_H

#include <stdio.h>
#include "str.h"



typedef struct TextFile
{
    Cstring             sName;
    FILE *              hFile;
    intt                nLastAction;
} TextFile;

#define TextFile_THIS       TextFile * pThis



void                TextFile_CTOR( TextFile_THIS,
                        char *              i_pName );
void                TextFile_DTOR( TextFile_THIS );

Bool                TF_Open( TextFile_THIS,
                        char *              i_sOptions );    /* options for second parameter of fopen */
Bool                TF_Create( TextFile_THIS );
void                TF_Close( TextFile_THIS );

void                TF_Goto( TextFile_THIS,
                        intt                i_nPosition );
intt                TF_Read( TextFile_THIS,
                        char *              o_pBuffer,
                        intt                i_nNrOfBytes );
intt                TF_Write( TextFile_THIS,
                        char *              i_pBuffer,
                        intt                i_nNrOfBytes );
intt                TF_WriteStr( TextFile_THIS,
                        char *              i_pString );

intt                TF_Position( TextFile_THIS );
intt                TF_Size( TextFile_THIS );




#endif



