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

#ifndef GCC
#   pragma hdrstop
#endif

#include <tools/ref.hxx>
#include <svtools/FilterConfigItem.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <vcl/msgbox.hxx>
#include "dlgexpor.hxx"
#include "dlgexpor.hrc"
#include "strings.hrc"

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

DlgExportPix::DlgExportPix( FltCallDialogParameter& rPara ) :
                ModalDialog         ( rPara.pWindow, ResId( DLG_EXPORT_PIX, *rPara.pResMgr ) ),
                rFltCallPara        ( rPara ),
                aBtnOK              ( this, ResId( BTN_OK_PIX, *rPara.pResMgr ) ),
                aBtnCancel          ( this, ResId( BTN_CANCEL_PIX, *rPara.pResMgr ) ),
                aBtnHelp            ( this, ResId( BTN_HELP_PIX, *rPara.pResMgr ) ),
                aLbColors           ( this, ResId( LB_COLORS, *rPara.pResMgr ) ),
                aCbxRLE             ( this, ResId( CBX_RLE, *rPara.pResMgr ) ),
                aGrpColors          ( this, ResId( GRP_COLORS, *rPara.pResMgr ) ),
                aRbOriginal         ( this, ResId( RB_ORIGINAL_PIX, *rPara.pResMgr ) ),
                aRbRes              ( this, ResId( RB_RES_PIX, *rPara.pResMgr ) ),
                aRbSize             ( this, ResId( RB_SIZE_PIX, *rPara.pResMgr ) ),
                aFtSizeX            ( this, ResId( FT_SIZEX_PIX, *rPara.pResMgr ) ),
                aMtfSizeX           ( this, ResId( MTF_SIZEX_PIX, *rPara.pResMgr ) ),
                aFtSizeY            ( this, ResId( FT_SIZEY_PIX, *rPara.pResMgr ) ),
                aMtfSizeY           ( this, ResId( MTF_SIZEY_PIX, *rPara.pResMgr ) ),
                aGrpMode            ( this, ResId( GRP_MODE_PIX, *rPara.pResMgr ) ),
                aCbbRes             ( this, ResId( CBB_RES_PIX, *rPara.pResMgr ) ),
                pMgr                ( rPara.pResMgr ),
                aExt                ( rPara.aFilterExt )
{
    aExt.ToUpperAscii();
    String  aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/" ) );
    aFilterConfigPath.Append( aExt );
    pConfigItem = new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData );

    String  aTitle( aExt );
    FreeResource();

    aBtnOK.SetClickHdl( LINK( this, DlgExportPix, OK ) );
    aRbOriginal.SetClickHdl( LINK( this, DlgExportPix, ClickRbOriginal ) );
    aRbRes.SetClickHdl( LINK( this, DlgExportPix, ClickRbRes ) );
    aRbSize.SetClickHdl( LINK( this, DlgExportPix, ClickRbSize ) );
    aLbColors.SetSelectHdl( LINK( this, DlgExportPix, SelectLbColors ) );

    aTitle.ToUpperAscii();
    aTitle += String( ResId( EXPORT_DIALOG_TITLE, *pMgr ) );
    SetText( aTitle );

    // Config-Parameter lesen
    sal_Int32   nColors = pConfigItem->ReadInt32( String( ResId( KEY_COLORS, *pMgr ) ), 0 );
    sal_Int32   nMode = pConfigItem->ReadInt32( String( ResId( KEY_MODE, *pMgr ) ), 0 );
    sal_Int32   nRes = pConfigItem->ReadInt32( String( ResId( KEY_RES, *pMgr ) ), 75 );
    sal_Bool    bRleCoding = pConfigItem->ReadBool( String( ResId( KEY_RLE_CODING, *pMgr ) ), sal_True );

    aLbColors.SelectEntryPos( Min( (sal_uInt16) 7, (sal_uInt16)nColors ) );

    String aStrRes( String::CreateFromInt32( nRes ) );
    aStrRes.Append( String( RTL_CONSTASCII_USTRINGPARAM( " DPI" ) ) );
    aCbbRes.SetText( aStrRes );

    ::com::sun::star::awt::Size aDefault( 10000, 10000 );
    ::com::sun::star::awt::Size aSize;
    aSize = pConfigItem->ReadSize( String( ResId( KEY_SIZE, *pMgr ) ), aDefault );

    aCbxRLE.Check( bRleCoding );

    aMtfSizeX.SetDefaultUnit( FUNIT_MM );
    aMtfSizeY.SetDefaultUnit( FUNIT_MM );

    aMtfSizeX.SetValue( aSize.Width );
    aMtfSizeY.SetValue( aSize.Height );

    switch ( rPara.eFieldUnit )
    {
//      case FUNIT_NONE :
//      case FUNIT_KM :
//      case FUNIT_PERCENT :
//      case FUNIT_CUSTOM :
//      case FUNIT_MILE :
//      case FUNIT_FOOT :
//      case FUNIT_M :
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
        break;      // -Wall  multiple values not handled.
    }

    switch ( nMode )
    {
        case 2 :
        {
            aRbSize.Check( sal_True );
            ClickRbSize( NULL );
        }
        break;
        case 1 :
        {
            aRbRes.Check( sal_True );
            ClickRbRes( NULL );
        }
        break;
        default :
        {
            aRbOriginal.Check( sal_True );
            ClickRbOriginal( NULL );
        }
        break;
    }
    SelectLbColors( &aLbColors );
}

DlgExportPix::~DlgExportPix()
{
    delete pConfigItem;
}


/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\************************************************************************/

IMPL_LINK( DlgExportPix, OK, void *, EMPTYARG )
{
    // Config-Parameter schreiben

    sal_Int32   nRes = Max( Min( aCbbRes.GetText().ToInt32(), sal_Int32( 600 ) ), sal_Int32( 75 ) );
    ::com::sun::star::awt::Size aSize(
        static_cast<long>(MetricField::ConvertDoubleValue( static_cast<double>(aMtfSizeX.GetValue()), 2, aMtfSizeX.GetUnit(), MAP_100TH_MM )),
            static_cast<long>(MetricField::ConvertDoubleValue( static_cast<double>(aMtfSizeY.GetValue()), 2, aMtfSizeY.GetUnit(), MAP_100TH_MM )) );

    sal_Int32 nMode;
    if ( aRbRes.IsChecked() )
        nMode = 1;
    else if ( aRbSize.IsChecked() )
        nMode = 2;
    else
        nMode = 0;

    pConfigItem->WriteInt32( String( ResId( KEY_MODE, *pMgr ) ), nMode );
    pConfigItem->WriteInt32( String( ResId( KEY_RES, *pMgr ) ), nRes );
    pConfigItem->WriteSize( String( ResId( KEY_SIZE, *pMgr ) ), aSize );
    pConfigItem->WriteInt32( String( ResId( KEY_COLORS, *pMgr ) ), (sal_Int32)aLbColors.GetSelectEntryPos() );
    pConfigItem->WriteBool( String( ResId( KEY_RLE_CODING, *pMgr ) ), aCbxRLE.IsChecked() );
    rFltCallPara.aFilterData = pConfigItem->GetFilterData();
    EndDialog( RET_OK );

    return 0;
}

/*************************************************************************
|*
|* Enabled/Disabled Controls
|*
\************************************************************************/

IMPL_LINK( DlgExportPix, ClickRbOriginal, void*, EMPTYARG )
{
    aCbbRes.Disable();

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

IMPL_LINK( DlgExportPix, ClickRbRes, void*, EMPTYARG )
{
    aCbbRes.Enable();

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

IMPL_LINK( DlgExportPix, ClickRbSize, void*, EMPTYARG )
{
    aFtSizeX.Enable();
    aMtfSizeX.Enable();
    aFtSizeY.Enable();
    aMtfSizeY.Enable();

    aCbbRes.Disable();

    return 0;
}


/*************************************************************************
|*
|* Enabled/Disabled Controls
|*
\************************************************************************/

IMPL_LINK( DlgExportPix, SelectLbColors, void*, EMPTYARG )
{
    const sal_uInt16 nLbPos = aLbColors.GetSelectEntryPos();

    if ( ( nLbPos >= 3 ) && ( nLbPos <= 6 ) )
        aCbxRLE.Enable();
    else
        aCbxRLE.Disable();

    return 0L;
}


/******************************************************************************/


/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

DlgExportVec::DlgExportVec( FltCallDialogParameter& rPara ) :
                ModalDialog         ( rPara.pWindow, ResId( DLG_EXPORT_VEC, *rPara.pResMgr ) ),
                rFltCallPara        ( rPara ),
                aBtnOK              ( this, ResId( BTN_OK_VEC, *rPara.pResMgr ) ),
                aBtnCancel          ( this, ResId( BTN_CANCEL_VEC, *rPara.pResMgr ) ),
                aBtnHelp            ( this, ResId( BTN_HELP_VEC, *rPara.pResMgr ) ),
                aRbOriginal         ( this, ResId( RB_ORIGINAL_VEC, *rPara.pResMgr ) ),
                aRbSize             ( this, ResId( RB_SIZE_VEC, *rPara.pResMgr ) ),
                aGrpMode            ( this, ResId( GRP_MODE_VEC, *rPara.pResMgr ) ),
                aFtSizeX            ( this, ResId( FT_SIZEX_VEC, *rPara.pResMgr ) ),
                aMtfSizeX           ( this, ResId( MTF_SIZEX_VEC, *rPara.pResMgr ) ),
                aFtSizeY            ( this, ResId( FT_SIZEY_VEC, *rPara.pResMgr ) ),
                aMtfSizeY           ( this, ResId( MTF_SIZEY_VEC, *rPara.pResMgr ) ),
                aGrpSize            ( this, ResId( GRP_SIZE_VEC, *rPara.pResMgr ) ),
                pMgr                ( rPara.pResMgr ),
                aExt                ( rPara.aFilterExt )
{
    aExt.ToUpperAscii();
    String  aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/" ) );
    aFilterConfigPath.Append( aExt );
    pConfigItem = new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData );

    String  aTitle( aExt );
    FreeResource();

    aBtnOK.SetClickHdl( LINK( this, DlgExportVec, OK ) );
    aRbOriginal.SetClickHdl( LINK( this, DlgExportVec, ClickRbOriginal ) );
    aRbSize.SetClickHdl( LINK( this, DlgExportVec, ClickRbSize ) );

    aTitle.ToUpperAscii();
    aTitle += String( ResId( EXPORT_DIALOG_TITLE, *pMgr ) );
    SetText( aTitle );

    // reading config-parameter
    sal_Int32   nMode = pConfigItem->ReadInt32( String( ResId( KEY_MODE, *pMgr ) ), 0 );

    ::com::sun::star::awt::Size aDefault( 10000, 10000 );
    ::com::sun::star::awt::Size aSize;
    aSize = pConfigItem->ReadSize( String( ResId( KEY_SIZE, *pMgr ) ), aDefault );

    aMtfSizeX.SetDefaultUnit( FUNIT_MM );
    aMtfSizeY.SetDefaultUnit( FUNIT_MM );
    aMtfSizeX.SetValue( aSize.Width );
    aMtfSizeY.SetValue( aSize.Height );

    switch ( rPara.eFieldUnit )
    {
//      case FUNIT_NONE :
//      case FUNIT_KM :
//      case FUNIT_PERCENT :
//      case FUNIT_CUSTOM :
//      case FUNIT_MILE :
//      case FUNIT_FOOT :
        case FUNIT_MM :
        case FUNIT_CM :
        case FUNIT_M :
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
            break;  // -Wall Multiple values not handled.
    }

    switch ( nMode )
    {
        case 1 :
        {
            aRbSize.Check( sal_True );
            ClickRbSize( NULL );
        }
        break;

        default :
        {
            aRbOriginal.Check( sal_True );
            ClickRbOriginal( NULL );
        }
        break;
    }
}

DlgExportVec::~DlgExportVec()
{
    delete pConfigItem;
}
/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\************************************************************************/

IMPL_LINK( DlgExportVec, OK, void *, EMPTYARG )
{
    // Config-Parameter schreiben
    ::com::sun::star::awt::Size aSize(
        static_cast<long>(MetricField::ConvertDoubleValue( static_cast<double>(aMtfSizeX.GetValue()), 2, aMtfSizeX.GetUnit(), MAP_100TH_MM )),
            static_cast<long>(MetricField::ConvertDoubleValue( static_cast<double>(aMtfSizeY.GetValue()), 2, aMtfSizeY.GetUnit(), MAP_100TH_MM )) );

    sal_Int32 nMode;
    if ( aRbSize.IsChecked() )
        nMode = 1;
    else
        nMode = 0;

    pConfigItem->WriteInt32( String( ResId( KEY_MODE, *pMgr ) ), nMode );
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

IMPL_LINK( DlgExportVec, ClickRbOriginal, void*, EMPTYARG )
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

IMPL_LINK( DlgExportVec, ClickRbSize, void*, EMPTYARG )
{
    aGrpSize.Enable();
    aFtSizeX.Enable();
    aMtfSizeX.Enable();
    aFtSizeY.Enable();
    aMtfSizeY.Enable();

    return 0;
}



