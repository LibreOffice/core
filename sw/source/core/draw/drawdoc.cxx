/*************************************************************************
 *
 *  $RCSfile: drawdoc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:18 $
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
#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST       SID_HATCH_LIST
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST
#define ITEMID_DASH_LIST        SID_DASH_LIST
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST
#ifndef _SVX_DRAWITEM_HXX
#include <svx/drawitem.hxx>
#endif

#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _DRAWDOC_HXX
#include <drawdoc.hxx>
#endif
#ifndef _DPAGE_HXX
#include <dpage.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _SW3IO_HXX
#include <sw3io.hxx>
#endif



/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/


SwDrawDocument::SwDrawDocument( SwDoc* pD ) :
    FmFormModel( SFX_APP()->GetAppIniManager()->Get(
                                        SFX_KEY_PALETTE_PATH ),
                 &pD->GetAttrPool(), 0, TRUE ),
    pDoc( pD )
{
    SetScaleUnit( MAP_TWIP );
    SetDefaultFontHeight( 240 );
    SetSwapGraphics( TRUE );

    SwDocShell* pDocSh = pDoc->GetDocShell();
    if ( pDocSh )
    {
        SetObjectShell( pDocSh );
        SvxColorTableItem* pColItem = ( SvxColorTableItem* )
                                ( pDocSh->GetItem( ITEMID_COLOR_TABLE ) );
        XColorTable *pXCol = pColItem ? pColItem->GetColorTable() :
                                        OFF_APP()->GetStdColorTable();
        SetColorTable( pXCol );

        if ( !pColItem )
            pDocSh->PutItem( SvxColorTableItem( pXCol ) );

        pDocSh->PutItem( SvxGradientListItem( GetGradientList() ));
        pDocSh->PutItem( SvxHatchListItem( GetHatchList() ) );
        pDocSh->PutItem( SvxBitmapListItem( GetBitmapList() ) );
        pDocSh->PutItem( SvxDashListItem( GetDashList() ) );
        pDocSh->PutItem( SvxLineEndListItem( GetLineEndList() ) );
        pDocSh->PutItem( SfxUInt16Item(SID_ATTR_LINEEND_WIDTH_DEFAULT, 111) );
        SetObjectShell( pDocSh );
    }
    else
        SetColorTable( OFF_APP()->GetStdColorTable() );
}

/*************************************************************************
|*
|* Konstruktor, fuer einfuegen Document
|*
\************************************************************************/


SwDrawDocument::SwDrawDocument( SfxItemPool *pPool, SwDocShell *pDocSh )
    : FmFormModel( SFX_APP()->GetAppIniManager()->
                                Get( SFX_KEY_PALETTE_PATH ),
                     pPool, 0, TRUE ),
    pDoc( pDocSh->GetDoc() )
{
    SetScaleUnit( MAP_TWIP );
    SetDefaultFontHeight( 240 );
    SetSwapGraphics( TRUE );

    ASSERT( pDocSh, "DocShell not found" );
    SvxColorTableItem* pColItem = ( SvxColorTableItem* )
                                ( pDocSh->GetItem( ITEMID_COLOR_TABLE ) );
    XColorTable *pXCol = pColItem ? pColItem->GetColorTable() :
                                    OFF_APP()->GetStdColorTable();
    SetColorTable( pXCol );

    if ( !pColItem )
        pDocSh->PutItem( SvxColorTableItem( pXCol ) );

    // Bug 35371:
    //  fuers "Datei einfuegen" NIE die anderen Items an der DocShell setzen!!!
    // Diese zeigen sonst immer in das temporaere SdrModel !
    SetObjectShell( pDocSh );
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/


SwDrawDocument::~SwDrawDocument()
{
    Clear();
}

/*************************************************************************
|*
|* Diese Methode erzeugt eine neue Seite (SdPage) und gibt einen Zeiger
|* darauf zurueck. Die Drawing Engine benutzt diese Methode beim Laden
|* zur Erzeugung von Seiten (deren Typ sie ja nicht kennt, da es ABLEITUNGEN
|* der SdrPage sind).
|*
\************************************************************************/


SdrPage* SwDrawDocument::AllocPage(FASTBOOL bMasterPage)
{
    SwDPage* pPage = new SwDPage(*this, bMasterPage);
    pPage->SetName( String::CreateFromAscii(
                                    RTL_CONSTASCII_STRINGPARAM( "Controls" )) );
    return pPage;
}


SvStream* SwDrawDocument::GetDocumentStream( FASTBOOL& rbDeleteAfterUse ) const
{
    SvStream* pRet = 0;
    SvStorageRef xRoot( pDoc->GetDocStorage() );
    String sDrawStrmNm( String::CreateFromAscii(
                    RTL_CONSTASCII_STRINGPARAM( DRAWING_STREAM_NAME )));
    if( xRoot.Is() && SVSTREAM_OK == xRoot->GetError() &&
        xRoot->IsStream( sDrawStrmNm ) )
    {
        long nFFVersion = xRoot->GetVersion();
        ASSERT( nFFVersion == SOFFICE_FILEFORMAT_31 ||
                nFFVersion == SOFFICE_FILEFORMAT_40 ||
                nFFVersion == SOFFICE_FILEFORMAT_NOW,
                "Am Root-Storage ist keine FF-Version gesetzt!" );

        // Wenn eine 3.1-Clipboard-ID gesetzt ist, die Fileformat-Version
        // auf 3.1 setzten.
        if( SOT_FORMATSTR_ID_STARWRITER_30 == xRoot->GetFormat() &&
            nFFVersion != SOFFICE_FILEFORMAT_31 )
        {
            ASSERT( nFFVersion == SOFFICE_FILEFORMAT_31,
                    "Fileformat-Version auf 3.1 umgesetzt" );
            xRoot->SetVersion( nFFVersion = SOFFICE_FILEFORMAT_31 );
        }
        else if( ( SOT_FORMATSTR_ID_STARWRITER_40 == xRoot->GetFormat() ||
                   SOT_FORMATSTR_ID_STARWRITERWEB_40 == xRoot->GetFormat() ||
                   SOT_FORMATSTR_ID_STARWRITERGLOB_40 == xRoot->GetFormat() ) &&
                 nFFVersion != SOFFICE_FILEFORMAT_40 )
        {
            ASSERT( nFFVersion == SOFFICE_FILEFORMAT_40,
                    "Fileformat-Version auf 4.0 umgesetzt" );
            xRoot->SetVersion( nFFVersion = SOFFICE_FILEFORMAT_40 );
        }

        pRet = xRoot->OpenStream( sDrawStrmNm,
                    STREAM_READ | STREAM_SHARE_DENYWRITE | STREAM_NOCREATE );
        if( pRet )
            rbDeleteAfterUse = TRUE;
    }
    return pRet;
}

SdrLayerID SwDrawDocument::GetControlExportLayerId( const SdrObject & ) const
{
    //fuer Versionen < 5.0, es gab nur Hell und Heaven
    return (SdrLayerID)pDoc->GetHeavenId();
}




