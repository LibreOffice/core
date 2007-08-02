/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgepng.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-02 18:18:36 $
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
#include "precompiled_svtools.hxx"
#include <tools/ref.hxx>
#include <vcl/msgbox.hxx>
#include "dlgepng.hxx"
#include "dlgepng.hrc"

using namespace ::rtl;
using namespace ::com::sun::star::uno;

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

DlgExportEPNG::DlgExportEPNG( FltCallDialogParameter& rPara ) :
                ModalDialog         ( rPara.pWindow, ResId( DLG_EXPORT_EPNG, *rPara.pResMgr ) ),
                FilterConfigItem    ( OUString( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/PNG" ) ), &rPara.aFilterData ),
                rFltCallPara        ( rPara ),
                aGrpCompression     ( this, ResId( GRP_COMPRESSION, *rPara.pResMgr ) ),
                aFiCompression      ( this, ResId( FI_COMPRESSION, *rPara.pResMgr ) ),
                aNumCompression     ( this, ResId( NUM_COMPRESSION, *rPara.pResMgr ) ),
                aCbxInterlaced      ( this, ResId( CBX_INTERLACED, *rPara.pResMgr ) ),
                aBtnOK              ( this, ResId( BTN_OK, *rPara.pResMgr ) ),
                aBtnCancel          ( this, ResId( BTN_CANCEL, *rPara.pResMgr ) ),
                aBtnHelp            ( this, ResId( BTN_HELP, *rPara.pResMgr ) ),
                pMgr                ( rPara.pResMgr )
{
    FreeResource();

    // Config-Parameter lesen
    sal_Int32   nCompression = ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Compression" ) ), 6 );
    if ( ( nCompression < 0 ) || ( nCompression > 9 ) )
        nCompression = 6;
    aNumCompression.SetValue( nCompression );

    sal_Int32   nInterlaced = ReadInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Interlaced" ) ), 0 );
    sal_Bool    bInterlaced = nInterlaced != 0;
    aCbxInterlaced.Check( bInterlaced );

    aBtnOK.SetClickHdl( LINK( this, DlgExportEPNG, OK ) );
}

/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\************************************************************************/

IMPL_LINK( DlgExportEPNG, OK, void *, EMPTYARG )
{
    // Config-Parameter schreiben
    WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Compression" ) ), static_cast<sal_Int32>(aNumCompression.GetValue()) );
    sal_Int32 nInterlace = 0;
    if ( aCbxInterlaced.IsChecked() )
        nInterlace++;
    WriteInt32( OUString( RTL_CONSTASCII_USTRINGPARAM( "Interlaced" ) ), nInterlace );
    rFltCallPara.aFilterData = GetFilterData();
    EndDialog( RET_OK );

    return 0;
}
