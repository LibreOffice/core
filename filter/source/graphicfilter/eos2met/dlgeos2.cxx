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
#include <com/sun/star/awt/Size.hpp>
#include <vcl/msgbox.hxx>
#include <svtools/FilterConfigItem.hxx>
#include "dlgeos2.hxx"
#include "dlgeos2.hrc"
#include "strings.hrc"

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

DlgExportEMET::DlgExportEMET( FltCallDialogParameter& rPara ) :
                ModalDialog			( rPara.pWindow, ResId( DLG_EXPORT_EMET, *rPara.pResMgr ) ),
                rFltCallPara		( rPara ),
                aBtnOK				( this, ResId( BTN_OK, *rPara.pResMgr ) ),
                aBtnCancel			( this, ResId( BTN_CANCEL, *rPara.pResMgr ) ),
                aBtnHelp			( this, ResId( BTN_HELP, *rPara.pResMgr ) ),
                aRbOriginal			( this, ResId( RB_ORIGINAL, *rPara.pResMgr ) ),
                aRbSize				( this, ResId( RB_SIZE, *rPara.pResMgr ) ),
                aGrpMode			( this, ResId( GRP_MODE, *rPara.pResMgr ) ),
                aFtSizeX			( this, ResId( FT_SIZEX, *rPara.pResMgr ) ),
                aMtfSizeX			( this, ResId( MTF_SIZEX, *rPara.pResMgr ) ),
                aFtSizeY			( this, ResId( FT_SIZEY, *rPara.pResMgr ) ),
                aMtfSizeY			( this, ResId( MTF_SIZEY, *rPara.pResMgr ) ),
                aGrpSize			( this, ResId( GRP_SIZE, *rPara.pResMgr ) ),
                pMgr				( rPara.pResMgr )
{
    FreeResource();

    String	aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/MET" ) );
    pConfigItem = new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData );

    aBtnOK.SetClickHdl( LINK( this, DlgExportEMET, OK ) );
    aRbOriginal.SetClickHdl( LINK( this, DlgExportEMET, ClickRbOriginal ) );
    aRbSize.SetClickHdl( LINK( this, DlgExportEMET, ClickRbSize ) );

    // Config-Parameter lesen
    sal_Int32 nMode = pConfigItem->ReadInt32( String( ResId( KEY_MODE, *pMgr ) ), 0 );
    ::com::sun::star::awt::Size aDefault( 10000, 10000 );
    ::com::sun::star::awt::Size aSize;
    aSize = pConfigItem->ReadSize( String( ResId( KEY_SIZE, *pMgr ) ), aDefault );

    aMtfSizeX.SetDefaultUnit( FUNIT_MM );
    aMtfSizeY.SetDefaultUnit( FUNIT_MM );

    aMtfSizeX.SetValue( aSize.Width );
    aMtfSizeY.SetValue( aSize.Height );

    switch ( rPara.eFieldUnit )
    {
//		case FUNIT_NONE :
//		case FUNIT_KM :
//		case FUNIT_PERCENT :
//		case FUNIT_CUSTOM :
//		case FUNIT_MILE :
//		case FUNIT_FOOT :
//		case FUNIT_M :
        case FUNIT_MM :
        case FUNIT_CM :
        case FUNIT_TWIP :
        case FUNIT_POINT :
        case FUNIT_PICA :
        case FUNIT_INCH :
        case FUNIT_100TH_MM :
        {
            aMtfSizeX.SetUnit( rPara.eFieldUnit );
            aMtfSizeY.SetUnit( rPara.eFieldUnit );
        }
        break;	
        default:
            break;  // multiple other value not handled -Wall
    }
    if ( nMode == 1 )
    {
        aRbSize.Check( TRUE );
        ClickRbSize( NULL );
    }
    else
    {
        aRbOriginal.Check( TRUE );
        ClickRbOriginal( NULL );
    }
}

DlgExportEMET::~DlgExportEMET()
{
    delete pConfigItem;
}

/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\************************************************************************/

IMPL_LINK( DlgExportEMET, OK, void *, EMPTYARG )
{
    // Config-Parameter schreiben
    ::com::sun::star::awt::Size aSize(
        (sal_Int32)MetricField::ConvertDoubleValue( aMtfSizeX.GetValue(), 2, aMtfSizeX.GetUnit(), MAP_100TH_MM ),
            (sal_Int32)MetricField::ConvertDoubleValue( aMtfSizeY.GetValue(), 2, aMtfSizeY.GetUnit(), MAP_100TH_MM ) );
    sal_Int32 nStrMode = ( aRbSize.IsChecked() ) ? 1 : 0; 

    pConfigItem->WriteInt32( String( ResId( KEY_MODE, *pMgr ) ), nStrMode );
    pConfigItem->WriteSize( String( ResId( KEY_SIZE, *pMgr ) ), aSize );
    rFltCallPara.aFilterData = pConfigItem->GetFilterData();
    EndDialog( RET_OK );

    return 0;
}

/*************************************************************************
|*
|* Enabled/Disabled Controls
|*
\************************************************************************/

IMPL_LINK( DlgExportEMET, ClickRbOriginal, void*, EMPTYARG )
{
    aGrpSize.Disable();
    aFtSizeX.Disable();
    aMtfSizeX.Disable();
    aFtSizeY.Disable();
    aMtfSizeY.Disable();

    return 0;
}


/*************************************************************************
|*
|* Enabled/Disabled Controls
|*
\************************************************************************/

IMPL_LINK( DlgExportEMET, ClickRbSize, void*, EMPTYARG )
{
    aGrpSize.Enable();
    aFtSizeX.Enable();
    aMtfSizeX.Enable();
    aFtSizeY.Enable();
    aMtfSizeY.Enable();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
