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
#include "precompiled_filter.hxx"

#include <tools/ref.hxx>
#include <svtools/FilterConfigItem.hxx>
#include <vcl/msgbox.hxx>
#include "dlgegif.hxx"
#include "dlgegif.hrc"
#include "strings.hrc"

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

DlgExportEGIF::DlgExportEGIF( FltCallDialogParameter& rPara ) :
                ModalDialog			( rPara.pWindow, ResId( DLG_EXPORT_GIF, *rPara.pResMgr ) ),
                rFltCallPara		( rPara ),
                aCbxInterlaced		( this, ResId( CBX_INTERLACED, *rPara.pResMgr ) ),
                aCbxTranslucent		( this, ResId( CBX_TRANSLUCENT, *rPara.pResMgr ) ),
                aGrpMode			( this, ResId( GRP_MODE, *rPara.pResMgr ) ),
                aGrpDraw			( this, ResId( GRP_DRAW, *rPara.pResMgr ) ),
                aBtnOK				( this, ResId( BTN_OK, *rPara.pResMgr ) ),
                aBtnCancel			( this, ResId( BTN_CANCEL, *rPara.pResMgr ) ),
                aBtnHelp			( this, ResId( BTN_HELP, *rPara.pResMgr ) ),
                pMgr				( rPara.pResMgr )
{
    FreeResource();

    String	aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/GIF" ) );
    pConfigItem = new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData );
    
    String aInterlaceStr( ResId( KEY_INTER, *pMgr ) );
    String aTranslucentStr( ResId( KEY_TRANS, *pMgr ) );
    // Config-Parameter lesen
    sal_Bool bInterlaced = pConfigItem->ReadInt32( aInterlaceStr, 1 ) != 0;
    sal_Bool bTranslucent = pConfigItem->ReadInt32( aTranslucentStr, 1 ) != 0;

    aCbxInterlaced.Check( bInterlaced );
    aCbxTranslucent.Check( bTranslucent );

    aBtnOK.SetClickHdl( LINK( this, DlgExportEGIF, OK ) );
}

DlgExportEGIF::~DlgExportEGIF()
{
    delete pConfigItem;
}

/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\************************************************************************/

IMPL_LINK( DlgExportEGIF, OK, void *, EMPTYARG )
{

    // Config-Parameter schreiben
    String aInterlaceStr( ResId( KEY_INTER, *pMgr ) );
    String aTranslucentStr( ResId( KEY_TRANS, *pMgr ) );

    sal_Int32 nValue = 0;
    if ( aCbxInterlaced.IsChecked() )
        nValue++;
    pConfigItem->WriteInt32( aInterlaceStr, nValue );

    nValue = 0;
    if ( aCbxTranslucent.IsChecked() )
        nValue++;
    pConfigItem->WriteInt32( aTranslucentStr, nValue );
    rFltCallPara.aFilterData = pConfigItem->GetFilterData();
    EndDialog( RET_OK );

    return 0;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
