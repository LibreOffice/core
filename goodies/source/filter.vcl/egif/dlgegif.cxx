/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgegif.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:40:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_goodies.hxx"
#ifndef GCC
#endif
#include <tools/ref.hxx>
#include <svtools/FilterConfigItem.hxx>
#include <vcl/msgbox.hxx>
#include "dlgegif.hxx"
#include "dlgegif.hrc"
#include "strings.hrc"

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

DlgExportEGIF::DlgExportEGIF( FltCallDialogParameter& rPara ) :
                ModalDialog         ( rPara.pWindow, ResId( DLG_EXPORT_GIF, rPara.pResMgr ) ),
                rFltCallPara        ( rPara ),
                aCbxInterlaced      ( this, ResId( CBX_INTERLACED ) ),
                aCbxTranslucent     ( this, ResId( CBX_TRANSLUCENT ) ),
                aGrpMode            ( this, ResId( GRP_MODE ) ),
                aGrpDraw            ( this, ResId( GRP_DRAW ) ),
                aBtnOK              ( this, ResId( BTN_OK ) ),
                aBtnCancel          ( this, ResId( BTN_CANCEL ) ),
                aBtnHelp            ( this, ResId( BTN_HELP ) ),
                pMgr                ( rPara.pResMgr )
{
    FreeResource();

    String  aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/GIF" ) );
    pConfigItem = new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData );

    String aInterlaceStr( ResId( KEY_INTER, pMgr ) );
    String aTranslucentStr( ResId( KEY_TRANS, pMgr ) );
    // Config-Parameter lesen
    sal_Bool bInterlaced = pConfigItem->ReadInt32( aInterlaceStr, 1 ) != 0;
    sal_Bool bTranslucent = pConfigItem->ReadInt32( aTranslucentStr, 1 ) != 0;

    aCbxInterlaced.Check( bInterlaced );
    aCbxTranslucent.Check( bTranslucent );

    aBtnOK.SetClickHdl( LINK( this, DlgExportEGIF, OK ) );
}

DlgExportEGIF::~DlgExportEGIF()
{
    delete pConfigItem;
}

/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\************************************************************************/

IMPL_LINK( DlgExportEGIF, OK, void *, EMPTYARG )
{

    // Config-Parameter schreiben
    String aInterlaceStr( ResId( KEY_INTER, pMgr ) );
    String aTranslucentStr( ResId( KEY_TRANS, pMgr ) );

    sal_Int32 nValue = 0;
    if ( aCbxInterlaced.IsChecked() )
        nValue++;
    pConfigItem->WriteInt32( aInterlaceStr, nValue );

    nValue = 0;
    if ( aCbxTranslucent.IsChecked() )
        nValue++;
    pConfigItem->WriteInt32( aTranslucentStr, nValue );
    rFltCallPara.aFilterData = pConfigItem->GetFilterData();
    EndDialog( RET_OK );

    return 0;
}



