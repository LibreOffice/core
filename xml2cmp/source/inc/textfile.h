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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
