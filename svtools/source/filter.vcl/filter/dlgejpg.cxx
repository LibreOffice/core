/*************************************************************************
 *
 *  $RCSfile: dlgejpg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sj $ $Date: 2001-03-07 19:57:06 $
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

#include "dlgejpg.hxx"
#include "dlgejpg.hrc"
#include "strings.hrc"
#include <FilterConfigItem.hxx>

#define KEY_QUALITY     "Quality"
#define KEY_GRAYSCALES  "ColorMode"

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

DlgExportEJPG::DlgExportEJPG( FltCallDialogParameter& rPara ) :
                ModalDialog         ( rPara.pWindow, ResId( DLG_EXPORT_JPG, rPara.pResMgr ) ),
                aFiDescr            ( this, ResId( FI_DESCR ) ),
                aNumFldQuality      ( this, ResId( NUM_FLD_QUALITY ) ),
                aGrpQuality         ( this, ResId( GRP_QUALITY ) ),
                aRbGray             ( this, ResId( RB_GRAY ) ),
                aRbRGB              ( this, ResId( RB_RGB ) ),
                aGrpColors          ( this, ResId( GRP_COLORS ) ),
                aBtnOK              ( this, ResId( BTN_OK ) ),
                aBtnCancel          ( this, ResId( BTN_CANCEL ) ),
                aBtnHelp            ( this, ResId( BTN_HELP ) )
{
    FreeResource();
    String  aFilterConfigPath( RTL_CONSTASCII_USTRINGPARAM( "Office.Common/Filter/Graphic/Export/JPG" ) );
    pConfigItem = new FilterConfigItem( aFilterConfigPath );

    // reading filter options
    sal_Int32 nQuality = pConfigItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( KEY_QUALITY ) ), 75 );
    sal_Int32 nColorMode = pConfigItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( KEY_GRAYSCALES ) ), 0 );
    aNumFldQuality.SetValue( nQuality );

    if ( nColorMode  )
        aRbGray.Check( sal_True );
    else
        aRbRGB.Check( sal_True );

    aBtnOK.SetClickHdl( LINK( this, DlgExportEJPG, OK ) );
}


/*************************************************************************
|*
|* Speichert eingestellte Werte in ini-Datei
|*
\************************************************************************/

IMPL_LINK( DlgExportEJPG, OK, void *, EMPTYARG )
{
    // Config-Parameter schreiben
    pConfigItem->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( KEY_QUALITY ) ), (sal_Int32)aNumFldQuality.GetValue() );
    pConfigItem->WriteInt32( String( RTL_CONSTASCII_USTRINGPARAM( KEY_GRAYSCALES ) ), aRbGray.IsChecked() ? 1 : 0 );
    EndDialog( RET_OK );
    return 0;
}

DlgExportEJPG::~DlgExportEJPG()
{
    delete pConfigItem;
}


