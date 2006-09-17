/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgeppm.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:43:40 $
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
#include "dlgeppm.hxx"
#include "dlgeppm.hrc"
#include "strings.hrc"

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

DlgExportEPPM::DlgExportEPPM( FltCallDialogParameter& rPara ) :
                ModalDialog         ( rPara.pWindow, ResId( DLG_EXPORT_EPPM, rPara.pResMgr ) ),
                rFltCallPara        ( rPara ),
                aGrpFormat          ( this, ResId( GRP_FORMAT ) ),
                aRBRaw              ( this, ResId( RB_RAW ) ),
                aRBASCII            ( this, ResId( RB_ASCII ) ),
                aBtnOK              ( this, ResId( BTN_OK ) ),
                aBtnCancel          ( this, ResId( BTN_CANCEL ) ),
                aBtnHelp            ( this, ResId( BTN_HELP ) ),
                pMgr                ( rPara.pResMgr )
{
    FreeResource();

    // Config-Parameter lesen

    String  aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/PPM" ) );
    pConfigItem = new FilterConfigItem( aFilterConfigPath, &rPara.aFilterData );
    sal_Int32   nFormat = pConfigItem->ReadInt32( String( ResId( KEY_FORMAT, pMgr ) ), 0 );

    BOOL bCheck = FALSE;
    if ( !nFormat )
        bCheck ^= TRUE;
    aRBRaw.Check( bCheck );
    bCheck ^= TRUE;
    aRBASCII.Check( bCheck );

    aBtnOK.SetClickHdl( LINK( this, DlgExportEPPM, OK ) );
}

DlgExportEPPM::~DlgExportEPPM()
{
    delete pConfigItem;
}

/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\************************************************************************/

IMPL_LINK( DlgExportEPPM, OK, void *, EMPTYARG )
{
    // Config-Parameter schreiben
    sal_Int32   nFormat = 0;
    if ( aRBASCII.IsChecked() )
        nFormat++;
    pConfigItem->WriteInt32( String( ResId( KEY_FORMAT, pMgr ) ), nFormat );
    rFltCallPara.aFilterData = pConfigItem->GetFilterData();
    EndDialog( RET_OK );

    return 0;
}
