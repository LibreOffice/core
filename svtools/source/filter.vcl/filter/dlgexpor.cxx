/*************************************************************************
 *
 *  $RCSfile: dlgexpor.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:59 $
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

#pragma hdrstop

#include <tools/ref.hxx>
#include <vcl/config.hxx>
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
                ModalDialog         ( rPara.pWindow, ResId( DLG_EXPORT_PIX, rPara.pResMgr ) ),
                aBtnOK              ( this, ResId( BTN_OK_PIX ) ),
                aBtnCancel          ( this, ResId( BTN_CANCEL_PIX ) ),
                aBtnHelp            ( this, ResId( BTN_HELP_PIX ) ),
                aGrpMode            ( this, ResId( GRP_MODE_PIX ) ),
                aCbbRes             ( this, ResId( CBB_RES_PIX ) ),
                aRbOriginal         ( this, ResId( RB_ORIGINAL_PIX ) ),
                aRbRes              ( this, ResId( RB_RES_PIX ) ),
                aRbSize             ( this, ResId( RB_SIZE_PIX ) ),
                aFtSizeX            ( this, ResId( FT_SIZEX_PIX ) ),
                aMtfSizeX           ( this, ResId( MTF_SIZEX_PIX ) ),
                aFtSizeY            ( this, ResId( FT_SIZEY_PIX ) ),
                aMtfSizeY           ( this, ResId( MTF_SIZEY_PIX ) ),
                aGrpColors          ( this, ResId( GRP_COLORS ) ),
                aLbColors           ( this, ResId( LB_COLORS ) ),
                aCbxRLE             ( this, ResId( CBX_RLE ) ),
                pConfig             ( rPara.pCfg ),
                pMgr                ( rPara.pResMgr ),
                rExt                ( rPara.aFilterExt )

{
    String  aTitle( rExt );

    FreeResource();

    aBtnOK.SetClickHdl( LINK( this, DlgExportPix, OK ) );
    aRbOriginal.SetClickHdl( LINK( this, DlgExportPix, ClickRbOriginal ) );
    aRbRes.SetClickHdl( LINK( this, DlgExportPix, ClickRbRes ) );
    aRbSize.SetClickHdl( LINK( this, DlgExportPix, ClickRbSize ) );
    aLbColors.SetSelectHdl( LINK( this, DlgExportPix, SelectLbColors ) );

    aTitle.ToUpperAscii();
    aTitle += String( ResId( EXPORT_DIALOG_TITLE, pMgr ) );
    SetText( aTitle );


    // Config-Parameter lesen
    ByteString aStrColors( rExt, RTL_TEXTENCODING_UTF8 );
    aStrColors.Append( ByteString( String( ResId( KEY_COLORS, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    aStrColors = pConfig->ReadKey( aStrColors );
    ByteString aStrMode( rExt, RTL_TEXTENCODING_UTF8 );
    aStrMode.Append( ByteString( String( ResId( KEY_MODE, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    aStrMode = pConfig->ReadKey( aStrMode );
    ByteString aStrRes( rExt, RTL_TEXTENCODING_UTF8 );
    aStrRes.Append( ByteString( String( ResId( KEY_RES, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    aStrRes = pConfig->ReadKey( aStrRes );
    ByteString aStrSizeX( rExt, RTL_TEXTENCODING_UTF8 );
    aStrSizeX.Append( ByteString( String( ResId( KEY_SIZEX, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    aStrSizeX = pConfig->ReadKey( aStrSizeX );
    ByteString aStrSizeY( rExt, RTL_TEXTENCODING_UTF8 );
    aStrSizeY.Append( ByteString( String( ResId( KEY_SIZEY, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    aStrSizeY = pConfig->ReadKey( aStrSizeY );
    ByteString aStrRLE( rExt, RTL_TEXTENCODING_UTF8 );
    aStrRLE.Append( ByteString( String( ResId( KEY_RLE_CODING, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    aStrRLE = pConfig->ReadKey( aStrRLE );

    aLbColors.SelectEntryPos( Min( (USHORT) 7, (USHORT)aStrColors.ToInt32() ) );
    aCbxRLE.Check( aStrRLE.CompareIgnoreCaseToAscii( "false", 5 ) == COMPARE_EQUAL );

    if ( !aStrRes.Len() )
        aStrRes = 75;

    aStrRes += " DPI";
    aCbbRes.SetText( UniString( aStrRes, RTL_TEXTENCODING_UTF8 ) );

    aMtfSizeX.SetDefaultUnit( FUNIT_MM );
    aMtfSizeY.SetDefaultUnit( FUNIT_MM );

    if ( !aStrSizeX.Len() )
        aMtfSizeX.SetValue( 10000 );
    else
        aMtfSizeX.SetValue( aStrSizeX.ToInt32() );

    if ( !aStrSizeY.Len() )
        aMtfSizeY.SetValue( 10000 );
    else
        aMtfSizeY.SetValue( aStrSizeY.ToInt32() );

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
    }

    switch ( aStrMode.ToInt32() )
    {
        case 2 :
        {
            aRbSize.Check( TRUE );
            ClickRbSize( NULL );
        }
        break;
        case 1 :
        {
            aRbRes.Check( TRUE );
            ClickRbRes( NULL );
        }
        break;
        default :
        {
            aRbOriginal.Check( TRUE );
            ClickRbOriginal( NULL );
        }
        break;
    }
    SelectLbColors( &aLbColors );
}

/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\************************************************************************/

IMPL_LINK( DlgExportPix, OK, void *, EMPTYARG )
{
    // Config-Parameter schreiben

    ByteString  aStrMode;
    ByteString  aRLEStr;

    long    nRes = Max( Min( aCbbRes.GetText().ToInt32(), 600L), 75L );
    long nSizeX = (long)MetricField::ConvertDoubleValue( aMtfSizeX.GetValue(), 2, aMtfSizeX.GetUnit(), MAP_100TH_MM );
    long nSizeY = (long)MetricField::ConvertDoubleValue( aMtfSizeY.GetValue(), 2, aMtfSizeY.GetUnit(), MAP_100TH_MM );

    if ( aRbRes.IsChecked() )
        aStrMode = '1';
    else if ( aRbSize.IsChecked() )
        aStrMode = '2';
    else
        aStrMode = '0';

    if ( aCbxRLE.IsChecked() )
        aRLEStr = ByteString( "TRUE", 4 );
    else
        aRLEStr = ByteString( "FALSE", 5 );

    ByteString aExt0( rExt, RTL_TEXTENCODING_UTF8 );
    aExt0.Append( ByteString( String( ResId( KEY_MODE, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    pConfig->WriteKey( aExt0, aStrMode );

    ByteString aExt1( rExt, RTL_TEXTENCODING_UTF8 );
    aExt1.Append( ByteString( String( ResId( KEY_RES, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    pConfig->WriteKey( aExt1, ByteString::CreateFromInt32( nRes )  );

    ByteString aExt2( rExt, RTL_TEXTENCODING_UTF8 );
    aExt2.Append( ByteString( String( ResId( KEY_SIZEX, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    pConfig->WriteKey( aExt2, ByteString::CreateFromInt32( nSizeX )  );

    ByteString aExt3( rExt, RTL_TEXTENCODING_UTF8 );
    aExt3.Append( ByteString( String( ResId( KEY_SIZEY, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    pConfig->WriteKey( aExt3, ByteString::CreateFromInt32( nSizeY )  );

    ByteString aExt4( rExt, RTL_TEXTENCODING_UTF8 );
    aExt4.Append( ByteString( String( ResId( KEY_COLORS, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    pConfig->WriteKey( aExt4, ByteString::CreateFromInt32( (sal_Int32)aLbColors.GetSelectEntryPos() ) );

    ByteString aExt5( rExt, RTL_TEXTENCODING_UTF8  );
    aExt5.Append( ByteString( String( ResId( KEY_RLE_CODING, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    pConfig->WriteKey( aExt5, ByteString::CreateFromInt32( (sal_Int32)aLbColors.GetSelectEntryPos() ) );

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
    const USHORT nLbPos = aLbColors.GetSelectEntryPos();

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
                ModalDialog         ( rPara.pWindow, ResId( DLG_EXPORT_VEC, rPara.pResMgr ) ),
                aBtnOK              ( this, ResId( BTN_OK_VEC ) ),
                aBtnCancel          ( this, ResId( BTN_CANCEL_VEC ) ),
                aBtnHelp            ( this, ResId( BTN_HELP_VEC ) ),
                aRbOriginal         ( this, ResId( RB_ORIGINAL_VEC ) ),
                aRbSize             ( this, ResId( RB_SIZE_VEC ) ),
                aGrpMode            ( this, ResId( GRP_MODE_VEC ) ),
                aFtSizeX            ( this, ResId( FT_SIZEX_VEC ) ),
                aMtfSizeX           ( this, ResId( MTF_SIZEX_VEC ) ),
                aFtSizeY            ( this, ResId( FT_SIZEY_VEC ) ),
                aMtfSizeY           ( this, ResId( MTF_SIZEY_VEC ) ),
                aGrpSize            ( this, ResId( GRP_SIZE_VEC ) ),
                pConfig             ( rPara.pCfg ),
                pMgr                ( rPara.pResMgr ),
                rExt                ( rPara.aFilterExt )

{
    String  aTitle( rExt );

    FreeResource();

    aBtnOK.SetClickHdl( LINK( this, DlgExportVec, OK ) );
    aRbOriginal.SetClickHdl( LINK( this, DlgExportVec, ClickRbOriginal ) );
    aRbSize.SetClickHdl( LINK( this, DlgExportVec, ClickRbSize ) );

    aTitle.ToUpperAscii();
    aTitle += String( ResId( EXPORT_DIALOG_TITLE, pMgr ) );
    SetText( aTitle );

    // Config-Parameter lesen
    //  String aStrMode = pConfig->ReadKey( rExt + String( ResId( KEY_MODE, pMgr ) ) );
    ByteString aExt( rExt, RTL_TEXTENCODING_UTF8 );
    ByteString aStrMode( aExt );
    aStrMode.Append( ByteString( String( ResId( KEY_MODE, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    aStrMode = pConfig->ReadKey( aStrMode );

    ByteString aStrSizeX( aExt );
    aStrSizeX.Append( ByteString( String( ResId( KEY_SIZEX, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    aStrSizeX = pConfig->ReadKey( aStrSizeX );

    ByteString aStrSizeY( aExt );
    aStrSizeY.Append( ByteString( String( ResId( KEY_SIZEY, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    aStrSizeY = pConfig->ReadKey( aStrSizeY );

    aMtfSizeX.SetDefaultUnit( FUNIT_MM );
    aMtfSizeY.SetDefaultUnit( FUNIT_MM );

    if ( !aStrSizeX.Len() )
        aMtfSizeX.SetValue( 10000 );
    else
        aMtfSizeX.SetValue( aStrSizeX.ToInt32() );

    if ( !aStrSizeY.Len() )
        aMtfSizeY.SetValue( 10000 );
    else
        aMtfSizeY.SetValue( aStrSizeY.ToInt32() );

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
    }

    switch ( aStrMode.ToInt32() )
    {
        case 1 :
        {
            aRbSize.Check( TRUE );
            ClickRbSize( NULL );
        }
        break;

        default :
        {
            aRbOriginal.Check( TRUE );
            ClickRbOriginal( NULL );
        }
        break;
    }
}

/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\************************************************************************/

IMPL_LINK( DlgExportVec, OK, void *, EMPTYARG )
{
    // Config-Parameter schreiben
    long nSizeX = (long)MetricField::ConvertDoubleValue( aMtfSizeX.GetValue(), 2, aMtfSizeX.GetUnit(), MAP_100TH_MM );
    long nSizeY = (long)MetricField::ConvertDoubleValue( aMtfSizeY.GetValue(), 2, aMtfSizeY.GetUnit(), MAP_100TH_MM );

    ByteString aStrMode;
    if ( aRbSize.IsChecked() )
        aStrMode = '1';
    else
        aStrMode = '0';

    ByteString aExt( rExt, RTL_TEXTENCODING_UTF8 );
    ByteString aDest( aExt );
    aDest.Append( ByteString( String( ResId( KEY_MODE, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    pConfig->WriteKey( aDest, aStrMode );

    aDest = aExt;
    aDest.Append( ByteString( String( ResId( KEY_SIZEX, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    pConfig->WriteKey( aDest, ByteString::CreateFromInt32( nSizeX ) );

    aDest = aExt;
    aDest.Append( ByteString( String( ResId( KEY_SIZEY, pMgr ) ), RTL_TEXTENCODING_UTF8 ) );
    pConfig->WriteKey( aDest, ByteString::CreateFromInt32( nSizeY ) );

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



