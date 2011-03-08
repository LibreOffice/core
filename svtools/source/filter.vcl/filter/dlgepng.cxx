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
#include "precompiled_svtools.hxx"
#include <tools/ref.hxx>
#include <vcl/msgbox.hxx>
#include "dlgepng.hxx"
#include "dlgepng.hrc"

using namespace ::rtl;
using namespace ::com::sun::star::uno;

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

DlgExportEPNG::DlgExportEPNG( FltCallDialogParameter& rPara ) :
                ModalDialog         ( rPara.pWindow, ResId( DLG_EXPORT_EPNG, *rPara.pResMgr ) ),
                FilterConfigItem    ( OUString( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/PNG" ) ), &rPara.aFilterData ),
                rFltCallPara        ( rPara ),
                aGrpCompression     ( this, ResId( GRP_COMPRESSION, *rPara.pResMgr ) ),
                aFiCompression      ( this, ResId( FI_COMPRESSION, *rPara.pResMgr ) ),
                aNumCompression     ( this, ResId( NUM_COMPRESSION, *rPara.pResMgr ) ),
                aCbxInterlaced      ( this, ResId( CBX_INTERLACED, *rPara.pResMgr ) ),
                aBtnOK              ( this, ResId( BTN_OK, *rPara.pResMgr ) ),
                aBtnCancel          ( this, ResId( BTN_CANCEL, *rPara.pResMgr ) ),
                aBtnHelp            ( this, ResId( BTN_HELP, *rPara.pResMgr ) ),
                pMgr                ( rPara.pResMgr )
{
    FreeResource();

    // Config-Parameter lesen
    sal_Int32   nCompression = ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Compression" ) ), 6 );
    if ( ( nCompression < 0 ) || ( nCompression > 9 ) )
        nCompression = 6;
    aNumCompression.SetValue( nCompression );

    sal_Int32   nInterlaced = ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Interlaced" ) ), 0 );
    sal_Bool    bInterlaced = nInterlaced != 0;
    aCbxInterlaced.Check( bInterlaced );

    aBtnOK.SetClickHdl( LINK( this, DlgExportEPNG, OK ) );
}

/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\************************************************************************/

IMPL_LINK( DlgExportEPNG, OK, void *, EMPTYARG )
{
    // Config-Parameter schreiben
    WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Compression" ) ), static_cast<sal_Int32>(aNumCompression.GetValue()) );
    sal_Int32 nInterlace = 0;
    if ( aCbxInterlaced.IsChecked() )
        nInterlace++;
    WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Interlaced" ) ), nInterlace );
    rFltCallPara.aFilterData = GetFilterData();
    EndDialog( RET_OK );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
