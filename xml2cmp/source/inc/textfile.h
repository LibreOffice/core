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
