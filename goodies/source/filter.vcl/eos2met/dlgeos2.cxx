/*************************************************************************
 *
 *  $RCSfile: dlgeos2.cxx,v $
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
#include "dlgeos2.hxx"
#include "dlgeos2.hrc"
#include "strings.hrc"

#include <segmentc.hxx>

SEG_EOFGLOBALS()

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

DlgExportEMET::DlgExportEMET( FltCallDialogParameter& rPara ) :
                ModalDialog         ( rPara.pWindow, ResId( DLG_EXPORT_EMET, rPara.pResMgr ) ),
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

    aBtnOK.SetClickHdl( LINK( this, DlgExportEMET, OK ) );
    aRbOriginal.SetClickHdl( LINK( this, DlgExportEMET, ClickRbOriginal ) );
    aRbSize.SetClickHdl( LINK( this, DlgExportEMET, ClickRbSize ) );

    // Config-Parameter lesen
    String aModeStr( ResId( KEY_MODE, pMgr ) );
    String aSizeXStr( ResId( KEY_SIZEX, pMgr ) );
    String aSizeYStr( ResId( KEY_SIZEY, pMgr ) );
    ByteString aStrMode( pConfig->ReadKey( ByteString( aModeStr, RTL_TEXTENCODING_UTF8 ) ) );
    ByteString aStrSizeX( pConfig->ReadKey( ByteString( aSizeXStr, RTL_TEXTENCODING_UTF8 ) ) );
    ByteString aStrSizeY( pConfig->ReadKey( ByteString( aSizeYStr, RTL_TEXTENCODING_UTF8 ) ) );

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

    if ( aStrMode.Len() && ( aStrMode.GetChar( 0 ) == '1' ) )
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
\************************************************************************/

IMPL_LINK( DlgExportEMET, OK, void *, EMPTYARG )
{
    // Config-Parameter schreiben
    sal_Int32 nSizeX = (sal_Int32)MetricField::ConvertDoubleValue( aMtfSizeX.GetValue(), 2, aMtfSizeX.GetUnit(), MAP_100TH_MM );
    sal_Int32 nSizeY = (sal_Int32)MetricField::ConvertDoubleValue( aMtfSizeY.GetValue(), 2, aMtfSizeY.GetUnit(), MAP_100TH_MM );
    sal_Int32 nStrMode = ( aRbSize.IsChecked() ) ? 1 : 0;

    String aModeStr( ResId( KEY_MODE, pMgr ) );
    String aSizeXStr( ResId( KEY_SIZEX, pMgr ) );
    String aSizeYStr( ResId( KEY_SIZEY, pMgr ) );
    pConfig->WriteKey( ByteString( aModeStr, RTL_TEXTENCODING_UTF8 ), ByteString::CreateFromInt32( nStrMode ) );
    pConfig->WriteKey( ByteString( aSizeXStr, RTL_TEXTENCODING_UTF8 ), ByteString::CreateFromInt32( nSizeX ) );
    pConfig->WriteKey( ByteString( aSizeYStr, RTL_TEXTENCODING_UTF8 ), ByteString::CreateFromInt32( nSizeY ) );

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
#pragma SEG_FUNCDEF(dlgeos2_09)

IMPL_LINK( DlgExportEMET, ClickRbSize, void*, EMPTYARG )
{
    aGrpSize.Enable();
    aFtSizeX.Enable();
    aMtfSizeX.Enable();
    aFtSizeY.Enable();
    aMtfSizeY.Enable();

    return 0;
}


/*************************************************************************

      Source Code Control System - History

      $Log: not supported by cvs2svn $
      Revision 1.6  2000/09/15 15:27:06  willem.vandorp
      OpenOffice header added.

      Revision 1.5  2000/07/26 17:24:45  willem.vandorp
      Headers/footers replaced

      Revision 1.4  2000/04/16 11:17:23  sj
      #75046# displaying different metrics

      Revision 1.3  2000/04/07 11:31:47  sj
      unicode changes

      Revision 1.2  1998/01/17 11:56:14  KA
      Includes


      Rev 1.1   17 Jan 1998 12:56:14   KA
   Includes

      Rev 1.0   26 Aug 1997 11:36:48   KA
   Initial revision.

      Rev 1.2   06 Feb 1996 12:38:06   KA
   #25026#: Defaultgroesse fuer Export auf 10*10 cm angehoben

      Rev 1.1   22 Jan 1996 15:25:10   KA
   Dialoge umgestellt / erweitert

      Rev 1.0   19 Jan 1996 20:09:16   KA
   Dialoganbindung

*************************************************************************/

#pragma SEG_EOFMODULE
