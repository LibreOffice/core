/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <svx/svxids.hrc>
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
#include <docsh.hxx>
#include <shellio.hxx>
#include <hintids.hxx>
#include <com/sun/star/embed/ElementModes.hpp>

using namespace com::sun::star;

/*************************************************************************
|*
|* Constructor
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
        SvxColorListItem* pColItem = ( SvxColorListItem* )
                                ( pDocSh->GetItem( SID_COLOR_TABLE ) );
        XColorListRef pXCol = pColItem ? pColItem->GetColorList() :
                                         XColorList::GetStdColorList();
        SetPropertyList( static_cast<XPropertyList *> (pXCol.get()) );

        if ( !pColItem )
            pDocSh->PutItem( SvxColorListItem( pXCol, SID_COLOR_TABLE ) );

        pDocSh->PutItem( SvxGradientListItem( GetGradientList(), SID_GRADIENT_LIST ));
        pDocSh->PutItem( SvxHatchListItem( GetHatchList(), SID_HATCH_LIST ) );
        pDocSh->PutItem( SvxBitmapListItem( GetBitmapList(), SID_BITMAP_LIST ) );
        pDocSh->PutItem( SvxDashListItem( GetDashList(), SID_DASH_LIST ) );
        pDocSh->PutItem( SvxLineEndListItem( GetLineEndList(), SID_LINEEND_LIST ) );
        pDocSh->PutItem( SfxUInt16Item(SID_ATTR_LINEEND_WIDTH_DEFAULT, 111) );
        SetObjectShell( pDocSh );
    }
    else
        SetPropertyList( static_cast<XPropertyList *> (XColorList::GetStdColorList().get()) );

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
    // Implementation for asian compression
    SetCharCompressType( static_cast<sal_uInt16>(pD->getCharacterCompressionType() ));
}

/*************************************************************************
|*
|* Destructor
|*
\************************************************************************/


SwDrawDocument::~SwDrawDocument()
{
    Broadcast(SdrHint(HINT_MODELCLEARED));

    ClearModel(sal_True);
}

/*************************************************************************
|*
|* This method creates a new page (SdPage) and thereupon returns a pointer
|* to it back. The drawing engine is using this method while loading for
|* the creating of pages (whose type it not even know, because they are
|* inherited from SdrPage).
|*
\************************************************************************/


SdrPage* SwDrawDocument::AllocPage(bool bMasterPage)
{
    SwDPage* pPage = new SwDPage(*this, 0 != bMasterPage);
    pPage->SetName(rtl::OUString("Controls"));
    return pPage;
}

uno::Reference<embed::XStorage> SwDrawDocument::GetDocumentStorage() const
{
    return pDoc->GetDocStorage();
}

SdrLayerID SwDrawDocument::GetControlExportLayerId( const SdrObject & ) const
{
    //for versions < 5.0, there was only Hell and Heaven
    return (SdrLayerID)pDoc->GetHeavenId();
}

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
        OSL_FAIL( "<SwDrawDocument::createUnoModel()> - could *not* retrieve model at <SwDocShell>" );
    }

    return xModel;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
