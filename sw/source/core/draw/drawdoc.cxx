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

#include <com/sun/star/frame/XModel.hpp>
#include <svx/drawitem.hxx>
#include <osl/diagnose.h>
#include <doc.hxx>
#include <drawdoc.hxx>
#include <dpage.hxx>
#include <docsh.hxx>
#include <hintids.hxx>
#include <dcontact.hxx>
#include <DocumentSettingManager.hxx>

using namespace com::sun::star;

// Constructor
SwDrawModel::SwDrawModel(SwDoc& rDoc)
    : FmFormModel(&rDoc.GetAttrPool(), rDoc.GetDocShell())
    , m_rDoc(rDoc)
{
    m_bThemedControls = false;
    SetScaleUnit( MapUnit::MapTwip );
    SetSwapGraphics();

    // use common InitDrawModelAndDocShell which will set the associations as needed,
    // including SvxColorTableItem  with WhichID SID_COLOR_TABLE
    InitDrawModelAndDocShell(m_rDoc.GetDocShell(), this);

    // copy all the default values to the SdrModel
    SfxItemPool* pSdrPool = m_rDoc.GetAttrPool().GetSecondaryPool();
    if( pSdrPool )
    {
        const sal_uInt16 aWhichRanges[] =
            {
                RES_CHRATR_BEGIN, RES_CHRATR_END,
                RES_PARATR_BEGIN, RES_PARATR_END,
                0
            };

        SfxItemPool& rDocPool = m_rDoc.GetAttrPool();
        sal_uInt16 nEdtWhich, nSlotId;
        const SfxPoolItem* pItem;
        for( const sal_uInt16* pRangeArr = aWhichRanges;
            *pRangeArr; pRangeArr += 2 )
            for( sal_uInt16 nW = *pRangeArr, nEnd = *(pRangeArr+1);
                    nW < nEnd; ++nW )
                if( nullptr != (pItem = rDocPool.GetUserDefaultItem( nW )) &&
                    0 != (nSlotId = rDocPool.GetSlotId( nW ) ) &&
                    nSlotId != nW &&
                    0 != (nEdtWhich = pSdrPool->GetWhichIDFromSlotID( nSlotId )) &&
                    nSlotId != nEdtWhich )
                {
                    std::unique_ptr<SfxPoolItem> pCpy(pItem->Clone());
                    pCpy->SetWhich( nEdtWhich );
                    pSdrPool->SetUserDefaultItem( *pCpy );
                }
    }

    SetForbiddenCharsTable(m_rDoc.GetDocumentSettingManager().getForbiddenCharacterTable());
    // Implementation for asian compression
    SetCharCompressType( m_rDoc.GetDocumentSettingManager().getCharacterCompressionType() );
}

// Destructor

SwDrawModel::~SwDrawModel()
{
    Broadcast(SdrHint(SdrHintKind::ModelCleared));

    // We have a nasty situation, where SwDrawVirtObj has a SwAnchoredObject field,
    // which points back to the SwDrawVirtObj via the rtl::Reference<SdrObject> mpDrawObj field.
    // Which creates a reference loop.
    // But other code also uses SwAnchoredObject, and does
    // so in a way which expects the mpDrawObj to keep things alive.
    // So we cannot change that.
    // So to prevent leaks on shutdown, we have to remove all of the SwDrawVirtObj objects.
    sal_uInt16 nPageCount=GetPageCount();
    for (sal_uInt16 i=0; i < nPageCount; ++i)
    {
        SdrPage* pPage = GetPage(i);
        for (const rtl::Reference<SdrObject>& pSdrObj : *pPage)
        {
            SwDrawContact* pContact = dynamic_cast<SwDrawContact*>(pSdrObj->GetUserCall());
            if (pContact)
                pContact->RemoveAllVirtObjs();
        }
    }

    ClearModel(true);
}

/** Create a new page (SdPage) and return a pointer to it back.
 *
 * The drawing engine is using this method while loading for the creating of
 * pages (whose type it not even know, because they are inherited from SdrPage).
 *
 * @return Pointer to the new page.
 */
rtl::Reference<SdrPage> SwDrawModel::AllocPage(bool bMasterPage)
{
    rtl::Reference<SwDPage> pPage = new SwDPage(*this, bMasterPage);
    pPage->SetName(u"Controls"_ustr);
    return pPage;
}

uno::Reference<embed::XStorage> SwDrawModel::GetDocumentStorage() const
{
    return m_rDoc.GetDocStorage();
}

uno::Reference< frame::XModel > SwDrawModel::createUnoModel()
{
    uno::Reference< frame::XModel > xModel;

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
    rSet.Put(SvxPatternListItem(GetPatternList(), SID_PATTERN_LIST));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
