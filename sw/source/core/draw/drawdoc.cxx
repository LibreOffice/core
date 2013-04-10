/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#include <tools/stream.hxx>
#include <unotools/pathoptions.hxx>
#include <sot/storage.hxx>
#include <svl/intitem.hxx>
#include <editeng/forbiddencharacterstable.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <viewsh.hxx>
#include <doc.hxx>
#include <rootfrm.hxx>
#include <drawdoc.hxx>
#include <dpage.hxx>
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#include <shellio.hxx>
#include <hintids.hxx>
#include <com/sun/star/embed/ElementModes.hpp>

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
                 pD->GetDocShell(), sal_True ),
    pDoc( pD )
{
    SetScaleUnit( MAP_TWIP );
    SetSwapGraphics( sal_True );

    SwDocShell* pDocSh = pDoc->GetDocShell();
    if ( pDocSh )
    {
        SetObjectShell( pDocSh );
        SvxColorTableItem* pColItem = ( SvxColorTableItem* )
                                ( pDocSh->GetItem( SID_COLOR_TABLE ) );
        XColorList *pXCol = pColItem ? pColItem->GetColorTable() :
                                        XColorList::GetStdColorList();
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
        SetColorTable( XColorList::GetStdColorList() );

    // copy all the default values to the SdrModel
    SfxItemPool* pSdrPool = pD->GetAttrPool().GetSecondaryPool();
    if( pSdrPool )
    {
        const sal_uInt16 aWhichRanges[] =
            {
                RES_CHRATR_BEGIN, RES_CHRATR_END,
                RES_PARATR_BEGIN, RES_PARATR_END,
                0
            };

        SfxItemPool& rDocPool = pD->GetAttrPool();
        sal_uInt16 nEdtWhich, nSlotId;
        const SfxPoolItem* pItem;
        for( const sal_uInt16* pRangeArr = aWhichRanges;
            *pRangeArr; pRangeArr += 2 )
            for( sal_uInt16 nW = *pRangeArr, nEnd = *(pRangeArr+1);
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
    SetCharCompressType( static_cast<sal_uInt16>(pD->getCharacterCompressionType() ));
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
                        rInfo.mbDeleteAfterUse = sal_True;
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
