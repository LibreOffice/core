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
#include <unotools/configmgr.hxx>
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
#include <DocumentSettingManager.hxx>
#include <IDocumentDrawModelAccess.hxx>

using namespace com::sun::star;

// Constructor

const OUString GetPalettePath()
{
    if (utl::ConfigManager::IsAvoidConfig())
        return OUString();
    SvtPathOptions aPathOpt;
    return aPathOpt.GetPalettePath();
}

SwDrawModel::SwDrawModel(SwDoc *const pDoc)
    : FmFormModel( ::GetPalettePath(), &pDoc->GetAttrPool(),
                     pDoc->GetDocShell(), true )
    , m_pDoc( pDoc )
{
    SetScaleUnit( MAP_TWIP );
    SetSwapGraphics();

    // use common InitDrawModelAndDocShell which will set the associations as needed,
    // including SvxColorTableItem  with WhichID SID_COLOR_TABLE
    InitDrawModelAndDocShell(m_pDoc->GetDocShell(), this);

    // copy all the default values to the SdrModel
    SfxItemPool* pSdrPool = m_pDoc->GetAttrPool().GetSecondaryPool();
    if( pSdrPool )
    {
        const sal_uInt16 aWhichRanges[] =
            {
                RES_CHRATR_BEGIN, RES_CHRATR_END,
                RES_PARATR_BEGIN, RES_PARATR_END,
                0
            };

        SfxItemPool& rDocPool = m_pDoc->GetAttrPool();
        sal_uInt16 nEdtWhich, nSlotId;
        const SfxPoolItem* pItem;
        for( const sal_uInt16* pRangeArr = aWhichRanges;
            *pRangeArr; pRangeArr += 2 )
            for( sal_uInt16 nW = *pRangeArr, nEnd = *(pRangeArr+1);
                    nW < nEnd; ++nW )
                if( nullptr != (pItem = rDocPool.GetPoolDefaultItem( nW )) &&
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

    SetForbiddenCharsTable(m_pDoc->GetDocumentSettingManager().getForbiddenCharacterTable());
    // Implementation for asian compression
    SetCharCompressType( static_cast<sal_uInt16>(
            m_pDoc->GetDocumentSettingManager().getCharacterCompressionType()));
}

// Destructor

SwDrawModel::~SwDrawModel()
{
    Broadcast(SdrHint(HINT_MODELCLEARED));

    ClearModel(true);
}

/** Create a new page (SdPage) and return a pointer to it back.
 *
 * The drawing engine is using this method while loading for the creating of
 * pages (whose type it not even know, because they are inherited from SdrPage).
 *
 * @return Pointer to the new page.
 */
SdrPage* SwDrawModel::AllocPage(bool bMasterPage)
{
    SwDPage* pPage = new SwDPage(*this, bMasterPage);
    pPage->SetName("Controls");
    return pPage;
}

uno::Reference<embed::XStorage> SwDrawModel::GetDocumentStorage() const
{
    return m_pDoc->GetDocStorage();
}

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
        OSL_FAIL( "<SwDrawModel::createUnoModel()> - could *not* retrieve model at <SwDocShell>" );
    }

    return xModel;
}

void SwDrawModel::PutAreaListItems(SfxItemSet& rSet) const
{
    rSet.Put(SvxColorListItem(GetColorList(), SID_COLOR_TABLE));
    rSet.Put(SvxGradientListItem(GetGradientList(), SID_GRADIENT_LIST));
    rSet.Put(SvxHatchListItem(GetHatchList(), SID_HATCH_LIST));
    rSet.Put(SvxBitmapListItem(GetBitmapList(), SID_BITMAP_LIST));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
