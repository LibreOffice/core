/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lotus.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:28:56 $
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
#include "precompiled_sc.hxx"
#include "lotimpop.hxx"

#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>

#include "scerrors.hxx"
#include "root.hxx"
#include "filtopt.hxx"

//------------------------------------------------------------------------

extern FltError ScImportLotus123old( SvStream&, ScDocument*, CharSet eSrc );
        // alter Krempel in filter.cxx!



FltError ScImportLotus123( SfxMedium& rMedium, ScDocument* pDocument, CharSet eSrc )
{
        ScFilterOptions aFilterOpt;
    BOOL bWithWK3 = aFilterOpt.GetWK3Flag();

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
        aURL.setExtension( CREATE_STRING( "FM3" ) );
        SfxMedium aMedium( aURL.GetMainURL(INetURLObject::NO_DECODE), STREAM_STD_READ, TRUE );
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




