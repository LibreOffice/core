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
#include "dlgeppm.hxx"
#include "dlgeppm.hrc"
#include "strings.hrc"

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

DlgExportEPPM::DlgExportEPPM( FltCallDialogParameter& rPara ) :
                ModalDialog			( rPara.pWindow, ResId( DLG_EXPORT_EPPM, *rPara.pResMgr ) ),
                rFltCallPara		( rPara ),
                aGrpFormat			( this, ResId( GRP_FORMAT, *rPara.pResMgr ) ),
                aRBRaw				( this, ResId( RB_RAW, *rPara.pResMgr ) ),
                aRBASCII			( this, ResId( RB_ASCII, *rPara.pResMgr ) ),
                aBtnOK				( this, ResId( BTN_OK, *rPara.pResMgr ) ),
                aBtnCancel			( this, ResId( BTN_CANCEL, *rPara.pResMgr ) ),
                aBtnHelp			( this, ResId( BTN_HELP, *rPara.pResMgr ) ),
                pMgr				( rPara.pResMgr )
{
    FreeResource();

    // Config-Parameter lesen

    String	aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/PPM" ) );
    pConfigItem = new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData );
    sal_Int32	nFormat = pConfigItem->ReadInt32( String( ResId( KEY_FORMAT, *pMgr ) ), 0 );

    BOOL bCheck = FALSE;
    if ( !nFormat )
        bCheck ^= TRUE;
    aRBRaw.Check( bCheck );
    bCheck ^= TRUE;
    aRBASCII.Check( bCheck );

    aBtnOK.SetClickHdl( LINK( this, DlgExportEPPM, OK ) );
}

DlgExportEPPM::~DlgExportEPPM()
{
    delete pConfigItem;
}

/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\************************************************************************/

IMPL_LINK( DlgExportEPPM, OK, void *, EMPTYARG )
{
    // Config-Parameter schreiben
    sal_Int32	nFormat = 0;
    if ( aRBASCII.IsChecked() )
        nFormat++;
    pConfigItem->WriteInt32( String( ResId( KEY_FORMAT, *pMgr ) ), nFormat );
    rFltCallPara.aFilterData = pConfigItem->GetFilterData();
    EndDialog( RET_OK );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
