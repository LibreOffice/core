/*************************************************************************
 *
 *  $RCSfile: lotus.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-10-05 17:07:27 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <sfx2/docfile.hxx>
#include <sfx2/inimgr.hxx>
#include <tools/urlobj.hxx>

#include "flttools.hxx"
#include "scerrors.hxx"
#include "lotimpop.hxx"
#include "root.hxx"

//------------------------------------------------------------------------

extern FltError ScImportLotus123old( SvStream&, ScDocument*, CharSet eSrc );
        // alter Krempel in filter.cxx!



FltError ScImportLotus123( SfxMedium& rMedium, ScDocument* pDocument, CharSet eSrc )
{
    SfxIniManager*      pIniManager = SFX_INIMANAGER();
    BOOL                bWithWK3 = pIniManager->Get(
                                    SFX_GROUP_COMMON, _STRINGCONST( "WK3" ) ).
                                    EqualsAscii( "1" );

    SvStream*           pStream = rMedium.GetInStream();

    if( !pStream )
        return eERR_OPEN;

    FltError            eRet;

    pStream->Seek( 0UL );

    pStream->SetBufferSize( 32768 );

    ImportLotus         aLotusImport( *pStream, pDocument, eSrc );

    if( bWithWK3 )
        eRet = aLotusImport.Read();
    else
        eRet = 0xFFFFFFFF;  // WK1 /WKS erzwingen

    // ACHTUNG: QUICK-HACK fuer WK1 / WKS  <->  WK3 / WK4
    if( eRet == 0xFFFFFFFF )
    {
        SvStream*       pStream = rMedium.GetInStream();

        if( !pStream )
            return eERR_OPEN;

        pStream->Seek( 0UL );

        pStream->SetBufferSize( 32768 );

        eRet = ScImportLotus123old( *pStream, pDocument, eSrc );

        pStream->SetBufferSize( 0 );

        return eRet;
    }

    if( eRet != eERR_OK )
        return eRet;

    if( pLotusRoot->eFirstType == Lotus_WK3 )
    {// versuchen *.FM3-File zu laden
        INetURLObject aURL( rMedium.GetURLObject() );
        aURL.setExtension( _STRINGCONST( "FM3" ) );
        SfxMedium aMedium( aURL.GetMainURL(), STREAM_STD_READ, TRUE );
        SvStream* pStream = aMedium.GetInStream();
        if ( pStream )
        {
            if( aLotusImport.Read( *pStream ) != eERR_OK )
                eRet = SCWARN_IMPORT_WRONG_FM3;
        }
        else
            eRet = SCWARN_IMPORT_OPEN_FM3;
    }

    return eRet;
}




