/*************************************************************************
 *
 *  $RCSfile: dlgeps.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-07 20:13:45 $
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
#include <vcl/msgbox.hxx>
#include <svtools/FilterConfigItem.hxx>
#include "dlgeps.hxx"
#include "dlgeps.hrc"
#include "strings.hrc"

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

DlgExportEPS::DlgExportEPS( FltCallDialogParameter& rPara ) :
                ModalDialog         ( rPara.pWindow, ResId( DLG_EXPORT_EPS, rPara.pResMgr ) ),
                aGrpPreview         ( this, ResId( GRP_PREVIEW ) ),
                aCBPreviewTiff      ( this, ResId( CB_PREVIEW_TIFF ) ),
                aCBPreviewEPSI      ( this, ResId( CB_PREVIEW_EPSI ) ),
                aGrpVersion         ( this, ResId( GRP_VERSION ) ),
                aRBLevel1           ( this, ResId( RB_LEVEL1 ) ),
                aRBLevel2           ( this, ResId( RB_LEVEL2 ) ),
                aGrpColor           ( this, ResId( GRP_COLOR ) ),
                aRBColor            ( this, ResId( RB_COLOR ) ),
                aRBGrayscale        ( this, ResId( RB_GRAYSCALE ) ),
                aGrpCompression     ( this, ResId( GRP_COMPRESSION ) ),
                aRBCompressionLZW   ( this, ResId( RB_COMPRESSION_LZW ) ),
                aRBCompressionNone  ( this, ResId( RB_COMPRESSION_NONE ) ),
                aBtnOK              ( this, ResId( BTN_OK ) ),
                aBtnCancel          ( this, ResId( BTN_CANCEL ) ),
                aBtnHelp            ( this, ResId( BTN_HELP ) ),
                pMgr                ( rPara.pResMgr )
{
    FreeResource();

    String  aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/EPS" ) );
    pConfigItem = new FilterConfigItem( aFilterConfigPath );

    // Config-Parameter lesen
    String aPrevStr( ResId( KEY_PREVIEW, pMgr ) );
    String aVersionStr( ResId( KEY_VERSION, pMgr ) );
    String aColorStr( ResId( KEY_COLOR, pMgr ) );
    String aComprStr( ResId( KEY_COMPR, pMgr ) );

    sal_Int32   nPreview = pConfigItem->ReadInt32( aPrevStr, 0 );
    sal_Int32   nVersion = pConfigItem->ReadInt32( aVersionStr, 2 );
    sal_Int32   nColor = pConfigItem->ReadInt32( aColorStr, 0 );
    sal_Int32   nCompr = pConfigItem->ReadInt32( aComprStr, 2 );

    BOOL bCheck = FALSE;
    if ( nPreview & 1 )
        bCheck = TRUE;
    aCBPreviewTiff.Check( bCheck );
    if ( nPreview & 2 )
        bCheck = TRUE;
    aCBPreviewEPSI.Check( bCheck );

    bCheck = FALSE;
    if ( nVersion == 1 )
        bCheck ^= TRUE;
    aRBLevel1.Check( bCheck );
    bCheck ^= TRUE;
    aRBLevel2.Check( bCheck );

    bCheck = FALSE;
    if ( nColor == 1 )
        bCheck ^= TRUE;
    aRBColor.Check( bCheck );
    bCheck ^= TRUE;
    aRBGrayscale.Check( bCheck );

    bCheck = FALSE;
    if ( nCompr == 1 )
        bCheck ^= TRUE;
    aRBCompressionLZW.Check( bCheck );
    bCheck ^= TRUE;
    aRBCompressionNone.Check( bCheck );

    if ( aRBLevel1.IsChecked() )
    {
        aRBColor.Disable();
        aRBGrayscale.Disable();
        aRBCompressionNone.Disable();
        aRBCompressionLZW.Disable();
        aRBCompressionNone.Disable();
    }

    aBtnOK.SetClickHdl( LINK( this, DlgExportEPS, OK ) );
    aRBLevel1.SetClickHdl( LINK( this, DlgExportEPS, LEVEL1 ) );
    aRBLevel2.SetClickHdl( LINK( this, DlgExportEPS, LEVEL2 ) );
}

DlgExportEPS::~DlgExportEPS()
{
    delete pConfigItem;
}

/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\************************************************************************/

IMPL_LINK( DlgExportEPS, OK, void *, EMPTYARG )
{

    // Config-Parameter schreiben
    sal_Int32 nCheck = 0;
    if ( aCBPreviewTiff.IsChecked() )
        nCheck++;
    if ( aCBPreviewEPSI.IsChecked() )
        nCheck += 2;

    String aPrevStr( ResId( KEY_PREVIEW, pMgr ) );
    pConfigItem->WriteInt32( aPrevStr, nCheck );

    nCheck = 1;
    if ( aRBLevel2.IsChecked() )
        nCheck++;
    String aVersionStr( ResId( KEY_VERSION, pMgr ) );
    pConfigItem->WriteInt32( aVersionStr, nCheck );

    nCheck = 1;
    if ( aRBGrayscale.IsChecked() )
        nCheck++;
    String aColorStr( ResId( KEY_COLOR, pMgr ) );
    pConfigItem->WriteInt32( aColorStr, nCheck );

    nCheck = 1;
    if ( aRBCompressionNone.IsChecked() )
        nCheck++;
    String aComprStr( ResId( KEY_COMPR, pMgr ) );
    pConfigItem->WriteInt32( aComprStr, nCheck );

    EndDialog( RET_OK );

    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( DlgExportEPS, LEVEL1, void*, EMPTYARG )
{
    if ( aRBLevel1.IsChecked() )
    {
        aRBColor.Disable();
        aRBGrayscale.Disable();
        aRBCompressionLZW.Disable();
        aRBCompressionNone.Disable();
    }
    return 0;
}

//------------------------------------------------------------------------

IMPL_LINK( DlgExportEPS, LEVEL2, void*, EMPTYARG )
{
    if ( aRBLevel2.IsChecked() )
    {
        aRBColor.Enable();
        aRBGrayscale.Enable();
        aRBCompressionLZW.Enable();
        aRBCompressionNone.Enable();
    }
    return 0;
}

