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
#ifndef GCC
#endif

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
                ModalDialog			( rPara.pWindow, ResId( DLG_EXPORT_EPS, *rPara.pResMgr ) ),
                rFltCallPara		( rPara ),
                aGrpPreview			( this, ResId( GRP_PREVIEW, *rPara.pResMgr ) ),
                aCBPreviewTiff		( this, ResId( CB_PREVIEW_TIFF, *rPara.pResMgr ) ),
                aCBPreviewEPSI		( this, ResId( CB_PREVIEW_EPSI, *rPara.pResMgr ) ),
                aGrpVersion			( this, ResId( GRP_VERSION, *rPara.pResMgr ) ),
                aRBLevel1			( this, ResId( RB_LEVEL1, *rPara.pResMgr ) ),
                aRBLevel2			( this, ResId( RB_LEVEL2, *rPara.pResMgr ) ),
                aGrpColor			( this, ResId( GRP_COLOR, *rPara.pResMgr ) ),
                aRBColor			( this, ResId( RB_COLOR, *rPara.pResMgr ) ),
                aRBGrayscale		( this, ResId( RB_GRAYSCALE, *rPara.pResMgr ) ),
                aGrpCompression		( this, ResId( GRP_COMPRESSION, *rPara.pResMgr ) ),
                aRBCompressionLZW	( this, ResId( RB_COMPRESSION_LZW, *rPara.pResMgr ) ),
                aRBCompressionNone	( this, ResId( RB_COMPRESSION_NONE, *rPara.pResMgr ) ),
                aBtnOK				( this, ResId( BTN_OK, *rPara.pResMgr ) ),
                aBtnCancel			( this, ResId( BTN_CANCEL, *rPara.pResMgr ) ),
                aBtnHelp			( this, ResId( BTN_HELP, *rPara.pResMgr ) ),
                pMgr				( rPara.pResMgr )
{
    FreeResource();

    String	aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/EPS" ) );
    pConfigItem = new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData );

    // Config-Parameter lesen
    String sPreview( RTL_CONSTASCII_USTRINGPARAM( "Preview" ) );
    String sVersion( RTL_CONSTASCII_USTRINGPARAM( "Version" ) );
    String sColorFormat( RTL_CONSTASCII_USTRINGPARAM( "ColorFormat" ) );   
    String sCompressionMode( RTL_CONSTASCII_USTRINGPARAM( "CompressionMode" ) );
    String sTextMode( RTL_CONSTASCII_USTRINGPARAM( "TextMode" ) );

    sal_Int32	nPreview = pConfigItem->ReadInt32( sPreview, 0 );
    sal_Int32	nVersion = pConfigItem->ReadInt32( sVersion, 2 );
    sal_Int32	nColor = pConfigItem->ReadInt32( sColorFormat, 0 );
    sal_Int32	nCompr = pConfigItem->ReadInt32( sCompressionMode, 2 );

    /* SJ: The following line is not superfluous, reading the item will also	#106652#
       create the corresponding FilterData Property. Since all filter
       are no longer accessing the configuration itself, we have fill the 
       FilterData sequence with all available configuration items */
    pConfigItem->ReadInt32( sTextMode, 0 );

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

    String sPreview( RTL_CONSTASCII_USTRINGPARAM( "Preview" ) );
    pConfigItem->WriteInt32( sPreview, nCheck );

    nCheck = 1;
    if ( aRBLevel2.IsChecked() )
        nCheck++;
    String sVersion( RTL_CONSTASCII_USTRINGPARAM( "Version" ) );
    pConfigItem->WriteInt32( sVersion, nCheck );

    nCheck = 1;
    if ( aRBGrayscale.IsChecked() )
        nCheck++;
    String sColorFormat( RTL_CONSTASCII_USTRINGPARAM( "ColorFormat" ) );   
    pConfigItem->WriteInt32( sColorFormat, nCheck );

    nCheck = 1;
    if ( aRBCompressionNone.IsChecked() )
        nCheck++;
    String sCompressionMode( RTL_CONSTASCII_USTRINGPARAM( "CompressionMode" ) );
    pConfigItem->WriteInt32( sCompressionMode, nCheck );

    rFltCallPara.aFilterData = pConfigItem->GetFilterData();
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
