/*************************************************************************
 *
 *  $RCSfile: textfile.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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



