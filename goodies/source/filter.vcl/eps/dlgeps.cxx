/*************************************************************************
 *
 *  $RCSfile: dlgeps.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: sj $ $Date: 2001-04-26 13:17:07 $
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
                aGrpText            ( this, ResId( GRP_TEXT ) ),
                aLBTextMode         ( this, ResId( LB_TEXT_MODE ) ),
                aLBTextResolution   ( this, ResId( LB_TEXT_RESOLUTION ) ),
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
    String sPreview( ResId( KEY_PREVIEW, pMgr ) );
    String sVersion( ResId( KEY_VERSION, pMgr ) );
    String sColorFormat( ResId( KEY_COLOR, pMgr ) );
    String sCompressionMode( ResId( KEY_COMPR, pMgr ) );
    String sTextMode( ResId( KEY_TEXTMODE, pMgr ) );
    String sTextToBitmapResolutions( ResId( KEY_TEXTRESOLUTIONS, pMgr ) );
    String sSelectedTextToBitmapResolution( ResId( KEY_SELECTEDTEXTRESOLUTION, pMgr ) );

    sal_Int32   nPreview = pConfigItem->ReadInt32( sPreview, 0 );
    sal_Int32   nVersion = pConfigItem->ReadInt32( sVersion, 2 );
    sal_Int32   nColor = pConfigItem->ReadInt32( sColorFormat, 0 );
    sal_Int32   nCompr = pConfigItem->ReadInt32( sCompressionMode, 2 );
    sal_uInt16  nTextMode = (sal_Int16)pConfigItem->ReadInt32( sTextMode, 0 );

    if ( nTextMode > 2 )
        nTextMode = 0;
    aLBTextMode.SelectEntryPos( nTextMode, sal_True );

    ::rtl::OUString sDefault( RTL_CONSTASCII_USTRINGPARAM( "150 300 600 1200" ) );
    String      sEntries( String( pConfigItem->ReadString( sTextToBitmapResolutions, sDefault ) ) );
    sal_Int16   nSelection = (sal_Int16)pConfigItem->ReadInt32( sSelectedTextToBitmapResolution, 0 );
    String      sTextResDPI( ResId( KEY_TEXTRESOLUTION_DPI, pMgr ) );

    sal_uInt16  i, nIndex, nTokenCount = sEntries.GetTokenCount( ' ' );
    for ( i = 0; i < 2; i++ )
    {
        for ( nIndex = 0; nIndex < nTokenCount; nIndex++ )
        {
            String sToken( sEntries.GetToken( nIndex, ' ' ) );
            sal_Int32 nVal = sToken.ToInt32();
            if ( nVal )
            {
                sToken.Append( sTextResDPI );
                aLBTextResolution.InsertEntry( sToken );
            }
        }
        if ( aLBTextResolution.GetEntryCount() )
            break;
        sEntries = sDefault;
    }
    if ( aLBTextResolution.GetEntryCount() <= nSelection )
        nSelection = 0;
    aLBTextResolution.SelectEntryPos( nSelection, sal_True );
    if ( nTextMode == 1 )
        aLBTextResolution.Disable();

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
    aLBTextMode.SetSelectHdl( LINK( this, DlgExportEPS, TEXTMODE ) );
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

    String sPreview( ResId( KEY_PREVIEW, pMgr ) );
    pConfigItem->WriteInt32( sPreview, nCheck );

    nCheck = 1;
    if ( aRBLevel2.IsChecked() )
        nCheck++;
    String sVersion( ResId( KEY_VERSION, pMgr ) );
    pConfigItem->WriteInt32( sVersion, nCheck );

    nCheck = 1;
    if ( aRBGrayscale.IsChecked() )
        nCheck++;
    String sColorFormat( ResId( KEY_COLOR, pMgr ) );
    pConfigItem->WriteInt32( sColorFormat, nCheck );

    nCheck = 1;
    if ( aRBCompressionNone.IsChecked() )
        nCheck++;
    String sCompressionMode( ResId( KEY_COMPR, pMgr ) );
    pConfigItem->WriteInt32( sCompressionMode, nCheck );

    String sTextMode( ResId( KEY_TEXTMODE, pMgr ) );
    pConfigItem->WriteInt32( sTextMode, aLBTextMode.GetSelectEntryPos() );

    String sSelectedTextToBitmapResolution( ResId( KEY_SELECTEDTEXTRESOLUTION, pMgr ) );
    pConfigItem->WriteInt32( sSelectedTextToBitmapResolution, aLBTextResolution.GetSelectEntryPos() );

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

//------------------------------------------------------------------------

IMPL_LINK( DlgExportEPS, TEXTMODE, void*, EMPTYARG )
{
    if ( aLBTextMode.GetSelectEntryPos() == 1 )
        aLBTextResolution.Disable();
    else
        aLBTextResolution.Enable();
    return 0;
}

