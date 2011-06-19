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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sot.hxx"

#include <sot/storage.hxx>

/*
 * main.
 */
int main (int argc, sal_Char **argv)
{
     SotStorageRef xStor = new SotStorage( "c:\\temp\\65254.ppt" );
/*
    SotStorageRef xStor = new SotStorage( "c:\\temp\\video.sdb" );
    SotStorageRef xStor = new SotStorage( "c:\\temp\\video.sdb" );
    SotStorageRef x2Stor = xStor->OpenSotStorage( "1117" );

    SotStorageStreamRef xStm = x2Stor->OpenSotStream( "Genres", STREAM_STD_READWRITE | STREAM_TRUNC);
    //BYTE szData[100];
    //xStm->Write( szData, 100 );
    UINT32 nSize = xStm->GetSize();
    xStm->SetSize( 0 );
    nSize = xStm->GetSize();
    x2Stor->Commit();
    xStor->Commit();
*/
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
