/*************************************************************************
 *
 *  $RCSfile: dlgepct.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:12 $
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

#include <vcl/msgbox.hxx>
#include <vcl/config.hxx>
#include "dlgepct.hxx"
#include "dlgepct.hrc"
#include "strings.hrc"

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

DlgExportEPCT::DlgExportEPCT( FltCallDialogParameter& rPara ) :
                ModalDialog         ( rPara.pWindow, ResId( DLG_EXPORT_EPCT, rPara.pResMgr ) ),
                aBtnOK              ( this, ResId( BTN_OK ) ),
                aBtnCancel          ( this, ResId( BTN_CANCEL ) ),
                aBtnHelp            ( this, ResId( BTN_HELP ) ),
                aRbOriginal         ( this, ResId( RB_ORIGINAL ) ),
                aRbSize             ( this, ResId( RB_SIZE ) ),
                aGrpMode            ( this, ResId( GRP_MODE ) ),
                aFtSizeX            ( this, ResId( FT_SIZEX ) ),
                aMtfSizeX           ( this, ResId( MTF_SIZEX ) ),
                aFtSizeY            ( this, ResId( FT_SIZEY ) ),
                aMtfSizeY           ( this, ResId( MTF_SIZEY ) ),
                aGrpSize            ( this, ResId( GRP_SIZE ) ),
                pConfig             ( rPara.pCfg ),
                pMgr                ( rPara.pResMgr )

{
    FreeResource();

    aBtnOK.SetClickHdl( LINK( this, DlgExportEPCT, OK ) );
    aRbOriginal.SetClickHdl( LINK( this, DlgExportEPCT, ClickRbOriginal ) );
    aRbSize.SetClickHdl( LINK( this, DlgExportEPCT, ClickRbSize ) );

    // Config-Parameter lesen
    String aMStr( ResId( KEY_MODE, pMgr ) );
    String aSXStr( ResId( KEY_SIZEX, pMgr ) );
    String aSYStr( ResId( KEY_SIZEY, pMgr ) );

    sal_Int32 nStrMode( pConfig->ReadKey( ByteString( aMStr, RTL_TEXTENCODING_UTF8 ) ).ToInt32() );
    ByteString aStrSizeX( pConfig->ReadKey( ByteString( aSXStr, RTL_TEXTENCODING_UTF8 ) ) );
    ByteString aStrSizeY( pConfig->ReadKey( ByteString( aSYStr, RTL_TEXTENCODING_UTF8 ) ) );

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

    if ( nStrMode == 1 )
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

/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\****************************************   ********************************/

IMPL_LINK( DlgExportEPCT, OK, void *, EMPTYARG )
{
    // Config-Parameter schreiben
    sal_Int32 nSizeX = (sal_Int32)MetricField::ConvertDoubleValue( aMtfSizeX.GetValue(), 2, aMtfSizeX.GetUnit(), MAP_100TH_MM );
    sal_Int32 nSizeY = (sal_Int32)MetricField::ConvertDoubleValue( aMtfSizeY.GetValue(), 2, aMtfSizeY.GetUnit(), MAP_100TH_MM );
    sal_Int32 nStrMode;

    if ( aRbSize.IsChecked() )
        nStrMode = 1;
    else
        nStrMode = 0;

    String aMStr( ResId( KEY_MODE, pMgr ) );
    String aSXStr( ResId( KEY_SIZEX, pMgr ) );
    String aSYStr( ResId( KEY_SIZEY, pMgr ) );
    pConfig->WriteKey( ByteString( aMStr, RTL_TEXTENCODING_UTF8 ), ByteString::CreateFromInt32( nStrMode ) );
    pConfig->WriteKey( ByteString( aSXStr, RTL_TEXTENCODING_UTF8 ), ByteString::CreateFromInt32( nSizeX ) );
    pConfig->WriteKey( ByteString( aSYStr, RTL_TEXTENCODING_UTF8 ), ByteString::CreateFromInt32( nSizeY ) );

    EndDialog( RET_OK );

    return 0;
}

/*************************************************************************
|*
|* Enabled/Disabled Controls
|*
\************************************************************************/

IMPL_LINK( DlgExportEPCT, ClickRbOriginal, void*, EMPTYARG )
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

IMPL_LINK( DlgExportEPCT, ClickRbSize, void*, EMPTYARG )
{
    aGrpSize.Enable();
    aFtSizeX.Enable();
    aMtfSizeX.Enable();
    aFtSizeY.Enable();
    aMtfSizeY.Enable();

    return 0;
}



