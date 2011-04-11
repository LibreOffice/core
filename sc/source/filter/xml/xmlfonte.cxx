/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"
#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif


#include "scitems.hxx"

#include <editeng/eeitem.hxx>


#include <xmloff/XMLFontAutoStylePool.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editeng.hxx>
#include "document.hxx"
#include "docpool.hxx"
#include "xmlexprt.hxx"
#include "stlpool.hxx"
#include "attrib.hxx"

class ScXMLFontAutoStylePool_Impl: public XMLFontAutoStylePool
{
    void AddFontItems(sal_uInt16* pWhichIds, sal_uInt8 nIdCount, const SfxItemPool* pItemPool, const sal_Bool bExportDefaults);
    public:

    ScXMLFontAutoStylePool_Impl( ScXMLExport& rExport );

};

void ScXMLFontAutoStylePool_Impl::AddFontItems(sal_uInt16* pWhichIds, sal_uInt8 nIdCount, const SfxItemPool* pItemPool, const sal_Bool bExportDefaults)
{
    const SfxPoolItem* pItem;
    for( sal_uInt16 i=0; i < nIdCount; ++i )
    {
        sal_uInt16 nWhichId(pWhichIds[i]);
        if (bExportDefaults && (0 != (pItem = &pItemPool->GetDefaultItem(nWhichId))))
        {
            const SvxFontItem *pFont((const SvxFontItem *)pItem);
            Add( pFont->GetFamilyName(), pFont->GetStyleName(),
                    sal::static_int_cast<sal_Int16>(pFont->GetFamily()),
                    sal::static_int_cast<sal_Int16>(pFont->GetPitch()),
                    pFont->GetCharSet() );
        }
        sal_uInt32 nItems(pItemPool->GetItemCount2( nWhichId ));
        for( sal_uInt32 j = 0; j < nItems; ++j )
        {
            if( 0 != (pItem = pItemPool->GetItem2( nWhichId, j ) ) )
            {
                const SvxFontItem *pFont((const SvxFontItem *)pItem);
                Add( pFont->GetFamilyName(), pFont->GetStyleName(),
                     sal::static_int_cast<sal_Int16>(pFont->GetFamily()),
                     sal::static_int_cast<sal_Int16>(pFont->GetPitch()),
                     pFont->GetCharSet() );
            }
        }
    }
}

ScXMLFontAutoStylePool_Impl::ScXMLFontAutoStylePool_Impl(
    ScXMLExport& rExportP ) :
    XMLFontAutoStylePool( rExportP )
{
    sal_uInt16 aWhichIds[3] = { ATTR_FONT, ATTR_CJK_FONT,
                                ATTR_CTL_FONT };
    sal_uInt16 aEditWhichIds[3] = { EE_CHAR_FONTINFO, EE_CHAR_FONTINFO_CJK,
                                    EE_CHAR_FONTINFO_CTL };
    sal_uInt16 aPageWhichIds[4] = { ATTR_PAGE_HEADERLEFT, ATTR_PAGE_FOOTERLEFT,
                                    ATTR_PAGE_HEADERRIGHT, ATTR_PAGE_FOOTERRIGHT };

    const SfxItemPool* pItemPool(rExportP.GetDocument() ? rExportP.GetDocument()->GetPool() : NULL);
    AddFontItems(aWhichIds, 3, pItemPool, sal_True);
    const SfxItemPool* pEditPool(rExportP.GetDocument()->GetEditPool());
    AddFontItems(aEditWhichIds, 3, pEditPool, false);

    SfxStyleSheetIterator* pItr(rExportP.GetDocument() ? rExportP.GetDocument()->GetStyleSheetPool()->CreateIterator(SFX_STYLE_FAMILY_PAGE, 0xFFFF) : NULL);
    if(pItr)
    {
        SfxStyleSheetBase* pStyle(pItr->First());
        SfxItemPool* pPageEditPool(EditEngine::CreatePool());
        EditEngine aEditEngine(pPageEditPool);
        while (pStyle)
        {
            const SfxItemPool& rPagePool(pStyle->GetPool().GetPool());
            for (sal_uInt8 j = 0; j < 4; ++j)
            {
                sal_uInt16 nPageWhichId(aPageWhichIds[j]);
                sal_uInt32 nPageHFItems(rPagePool.GetItemCount2(nPageWhichId));
                const ScPageHFItem* pPageItem;
                for (sal_uInt32 k = 0; k < nPageHFItems; ++k)
                {
                    if (0 != (pPageItem = static_cast<const ScPageHFItem*>(rPagePool.GetItem2(nPageWhichId, k))))
                    {
                        const EditTextObject* pLeftArea(pPageItem->GetLeftArea());
                        if (pLeftArea)
                        {
                            aEditEngine.SetText(*pLeftArea);
                            AddFontItems(aEditWhichIds, 3, pPageEditPool, false);
                        }
                        const EditTextObject* pCenterArea(pPageItem->GetCenterArea());
                        if (pCenterArea)
                        {
                            aEditEngine.SetText(*pCenterArea);
                            AddFontItems(aEditWhichIds, 3, pPageEditPool, false);
                        }
                        const EditTextObject* pRightArea(pPageItem->GetRightArea());
                        if (pRightArea)
                        {
                            aEditEngine.SetText(*pRightArea);
                            AddFontItems(aEditWhichIds, 3, pPageEditPool, false);
                        }
                    }
                }
            }
            pStyle = pItr->Next();
        }
    }
}


XMLFontAutoStylePool* ScXMLExport::CreateFontAutoStylePool()
{
    return new ScXMLFontAutoStylePool_Impl( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
