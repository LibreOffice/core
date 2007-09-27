/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawdoc.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:43:23 $
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
#include "precompiled_sw.hxx"

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#include <sot/storage.hxx>
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _FORBIDDENCHARACTERSTABLE_HXX
#include <svx/forbiddencharacterstable.hxx>
#endif

#include <unotools/ucbstreamhelper.hxx>
#include <svx/xtable.hxx>

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
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif

using namespace com::sun::star;

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

const String GetPalettePath()
{
    SvtPathOptions aPathOpt;
    return aPathOpt.GetPalettePath();
}

SwDrawDocument::SwDrawDocument( SwDoc* pD ) :
    FmFormModel( ::GetPalettePath(), &pD->GetAttrPool(),
                 pD->GetDocShell(), TRUE ),
    pDoc( pD )
{
    SetScaleUnit( MAP_TWIP );
    SetSwapGraphics( TRUE );

    SwDocShell* pDocSh = pDoc->GetDocShell();
    if ( pDocSh )
    {
        SetObjectShell( pDocSh );
        SvxColorTableItem* pColItem = ( SvxColorTableItem* )
                                ( pDocSh->GetItem( SID_COLOR_TABLE ) );
        XColorTable *pXCol = pColItem ? pColItem->GetColorTable() :
                                        XColorTable::GetStdColorTable();
        SetColorTable( pXCol );

        if ( !pColItem )
            pDocSh->PutItem( SvxColorTableItem( pXCol, SID_COLOR_TABLE ) );

        pDocSh->PutItem( SvxGradientListItem( GetGradientList(), SID_GRADIENT_LIST ));
        pDocSh->PutItem( SvxHatchListItem( GetHatchList(), SID_HATCH_LIST ) );
        pDocSh->PutItem( SvxBitmapListItem( GetBitmapList(), SID_BITMAP_LIST ) );
        pDocSh->PutItem( SvxDashListItem( GetDashList(), SID_DASH_LIST ) );
        pDocSh->PutItem( SvxLineEndListItem( GetLineEndList(), SID_LINEEND_LIST ) );
        pDocSh->PutItem( SfxUInt16Item(SID_ATTR_LINEEND_WIDTH_DEFAULT, 111) );
        SetObjectShell( pDocSh );
    }
    else
        SetColorTable( XColorTable::GetStdColorTable() );

    // copy all the default values to the SdrModel
    SfxItemPool* pSdrPool = pD->GetAttrPool().GetSecondaryPool();
    if( pSdrPool )
    {
        const USHORT aWhichRanges[] =
            {
                RES_CHRATR_BEGIN, RES_CHRATR_END,
                RES_PARATR_BEGIN, RES_PARATR_END,
                0
            };

        SfxItemPool& rDocPool = pD->GetAttrPool();
        USHORT nEdtWhich, nSlotId;
        const SfxPoolItem* pItem;
        for( const USHORT* pRangeArr = aWhichRanges;
            *pRangeArr; pRangeArr += 2 )
            for( USHORT nW = *pRangeArr, nEnd = *(pRangeArr+1);
                    nW < nEnd; ++nW )
                if( 0 != (pItem = rDocPool.GetPoolDefaultItem( nW )) &&
                    0 != (nSlotId = rDocPool.GetSlotId( nW ) ) &&
                    nSlotId != nW &&
                    0 != (nEdtWhich = pSdrPool->GetWhich( nSlotId )) &&
                    nSlotId != nEdtWhich )
                {
                    SfxPoolItem* pCpy = pItem->Clone();
                    pCpy->SetWhich( nEdtWhich );
                    pSdrPool->SetPoolDefaultItem( *pCpy );
                    delete pCpy;
                }
    }

    SetForbiddenCharsTable( pD->getForbiddenCharacterTable() );
    // #87795# Implementation for asian compression
    SetCharCompressType( static_cast<UINT16>(pD->getCharacterCompressionType() ));
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/


SwDrawDocument::~SwDrawDocument()
{
    Broadcast(SdrHint(HINT_MODELCLEARED));

    // #116168#
    ClearModel(sal_True);
    //Clear();
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
    SwDPage* pPage = new SwDPage(*this, 0 != bMasterPage);
    pPage->SetName( String::CreateFromAscii(
                                    RTL_CONSTASCII_STRINGPARAM( "Controls" )) );
    return pPage;
}


SvStream* SwDrawDocument::GetDocumentStream( SdrDocumentStreamInfo& rInfo ) const
{
    SvStream* pRet = NULL;
    uno::Reference < embed::XStorage > xRoot( pDoc->GetDocStorage() );
    if( xRoot.is() )
    {
        if( rInfo.maUserData.Len() &&
            ( rInfo.maUserData.GetToken( 0, ':' ) ==
              String( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.Package" ) ) ) )
        {
            const String aPicturePath( rInfo.maUserData.GetToken( 1, ':' ) );

            // graphic from picture stream in picture storage in XML package
            if( aPicturePath.GetTokenCount( '/' ) == 2 )
            {
                const String    aPictureStorageName( aPicturePath.GetToken( 0, '/' ) );
                const String    aPictureStreamName( aPicturePath.GetToken( 1, '/' ) );

                try
                {
                    uno::Reference < embed::XStorage > xPictureStorage = xRoot->openStorageElement(
                            aPictureStorageName, embed::ElementModes::READ );
                    uno::Reference < io::XStream > xStream = xPictureStorage->openStreamElement(
                            aPictureStreamName, embed::ElementModes::READ );
                    pRet = utl::UcbStreamHelper::CreateStream( xStream );
                    if( pRet )
                    {
                        rInfo.mbDeleteAfterUse = TRUE;
                        rInfo.mxStorageRef = xPictureStorage;
                    }
                }
                catch ( uno::Exception& )
                {
                }
            }
        }
    }
    return pRet;
}

SdrLayerID SwDrawDocument::GetControlExportLayerId( const SdrObject & ) const
{
    //fuer Versionen < 5.0, es gab nur Hell und Heaven
    return (SdrLayerID)pDoc->GetHeavenId();
}

// --> OD 2006-03-01 #b6382898#
uno::Reference< uno::XInterface > SwDrawDocument::createUnoModel()
{

    uno::Reference< uno::XInterface > xModel;

    try
    {
        if ( GetDoc().GetDocShell() )
        {
            xModel = GetDoc().GetDocShell()->GetModel();
        }
    }
    catch( uno::RuntimeException& )
    {
        ASSERT( false,
                "<SwDrawDocument::createUnoModel()> - could *not* retrieve model at <SwDocShell>" );
    }

    return xModel;
}

// <--
