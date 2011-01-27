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
#include "dlgejpg.hxx"
#include "dlgejpg.hrc"
#include "strings.hrc"
#include <svtools/FilterConfigItem.hxx>

#define KEY_QUALITY     "Quality"
#define KEY_GRAYSCALES  "ColorMode"

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

DlgExportEJPG::DlgExportEJPG( FltCallDialogParameter& rPara ) :
                ModalDialog         ( rPara.pWindow, ResId( DLG_EXPORT_JPG, *rPara.pResMgr ) ),
                rFltCallPara        ( rPara ),
                aFiDescr            ( this, ResId( FI_DESCR, *rPara.pResMgr ) ),
                aNumFldQuality      ( this, ResId( NUM_FLD_QUALITY, *rPara.pResMgr ) ),
                aGrpQuality         ( this, ResId( GRP_QUALITY, *rPara.pResMgr ) ),
                aRbGray             ( this, ResId( RB_GRAY, *rPara.pResMgr ) ),
                aRbRGB              ( this, ResId( RB_RGB, *rPara.pResMgr ) ),
                aGrpColors          ( this, ResId( GRP_COLORS, *rPara.pResMgr ) ),
                aBtnOK              ( this, ResId( BTN_OK, *rPara.pResMgr ) ),
                aBtnCancel          ( this, ResId( BTN_CANCEL, *rPara.pResMgr ) ),
                aBtnHelp            ( this, ResId( BTN_HELP, *rPara.pResMgr ) )
{
    FreeResource();
    String  aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/JPG" ) );
    pConfigItem = new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData );

    // reading filter options
    sal_Int32 nQuality = pConfigItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( KEY_QUALITY ) ), 75 );
    sal_Int32 nColorMode = pConfigItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( KEY_GRAYSCALES ) ), 0 );
    aNumFldQuality.SetValue( nQuality );

    if ( nColorMode  )
        aRbGray.Check( sal_True );
    else
        aRbRGB.Check( sal_True );

    aBtnOK.SetClickHdl( LINK( this, DlgExportEJPG, OK ) );
}


/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\************************************************************************/

IMPL_LINK( DlgExportEJPG, OK, void *, EMPTYARG )
{
    // Config-Parameter schreiben
    pConfigItem->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( KEY_QUALITY ) ), (sal_Int32)aNumFldQuality.GetValue() );
    pConfigItem->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( KEY_GRAYSCALES ) ), aRbGray.IsChecked() ? 1 : 0 );
    rFltCallPara.aFilterData = pConfigItem->GetFilterData();
    EndDialog( RET_OK );
    return 0;
}

DlgExportEJPG::~DlgExportEJPG()
{
    delete pConfigItem;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
