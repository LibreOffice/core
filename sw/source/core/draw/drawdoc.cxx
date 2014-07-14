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

SwDrawModel::SwDrawModel( SwDoc* pD )
:   FmFormModel( ::GetPalettePath(), &pD->GetAttrPool(), pD->GetDocShell() ),
    pDoc( pD )
{
    SetExchangeObjectUnit( MAP_TWIP );
    SetSwapGraphics( true );

    // use common InitDrawModelAndDocShell which will set the associations as needed,
    // including SvxColorTableItem  with WhichID SID_COLOR_TABLE
    InitDrawModelAndDocShell(pDoc ? pDoc->GetDocShell() : 0, this);

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


SwDrawModel::~SwDrawModel()
{
    Broadcast(SdrBaseHint(HINT_MODELCLEARED));

    // #116168#
    ClearModel(true);
}

/*************************************************************************
|*
|* Diese Methode erzeugt eine neue Seite (SdPage) und gibt einen Zeiger
|* darauf zurueck. Die Drawing Engine benutzt diese Methode beim Laden
|* zur Erzeugung von Seiten (deren Typ sie ja nicht kennt, da es ABLEITUNGEN
|* der SdrPage sind).
|*
\************************************************************************/


SdrPage* SwDrawModel::AllocPage(bool bMasterPage)
{
    SwDPage* pPage = new SwDPage(*this, 0 != bMasterPage);
    pPage->SetName( String::CreateFromAscii(
                                    RTL_CONSTASCII_STRINGPARAM( "Controls" )) );
    return pPage;
}


SvStream* SwDrawModel::GetDocumentStream( SdrDocumentStreamInfo& rInfo ) const
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
                        rInfo.mbDeleteAfterUse = true;
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

SdrLayerID SwDrawModel::GetControlExportLayerId( const SdrObject & ) const
{
    //fuer Versionen < 5.0, es gab nur Hell und Heaven
    return (SdrLayerID)pDoc->GetHeavenId();
}

// --> OD 2006-03-01 #b6382898#
uno::Reference< uno::XInterface > SwDrawModel::createUnoModel()
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
        ASSERT( false, "<SwDrawModel::createUnoModel()> - could *not* retrieve model at <SwDocShell>" );
    }

    return xModel;
}

bool SwDrawModel::IsWriter() const
{
    return true;
}

// <--
