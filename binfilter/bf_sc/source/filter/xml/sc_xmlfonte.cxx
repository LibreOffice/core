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

#ifdef PRECOMPILED
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif

#define ITEMID_FIELD EE_FEATURE_FIELD

#ifndef _SVX_FONTITEM_HXX
#include <bf_svx/fontitem.hxx>
#endif
#ifndef _EEITEM_HXX
#include <bf_svx/eeitem.hxx>
#endif
#ifndef _MyEDITENG_HXX
#include <bf_svx/editeng.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_SCDOCPOL_HXX
#include "docpool.hxx"
#endif
#ifndef SC_XMLEXPRT_HXX
#include "xmlexprt.hxx"
#endif
#ifndef SC_STLPOOL_HXX
#include "stlpool.hxx"
#endif
#ifndef SC_SCATTR_HXX
#include "attrib.hxx"
#endif
namespace binfilter {

class ScXMLFontAutoStylePool_Impl: public XMLFontAutoStylePool
{
    void AddFontItems(sal_uInt16* pWhichIds, sal_uInt8 nIdCount, const SfxItemPool* pPool, const sal_Bool bExportDefaults);
    public:

    ScXMLFontAutoStylePool_Impl( ScXMLExport& rExport );

};

void ScXMLFontAutoStylePool_Impl::AddFontItems(sal_uInt16* pWhichIds, sal_uInt8 nIdCount, const SfxItemPool* pPool, const sal_Bool bExportDefaults)
{
    const SfxPoolItem* pItem;
    for( sal_uInt16 i=0; i < nIdCount; i++ )
    {
        sal_uInt16 nWhichId = pWhichIds[i];
        if (bExportDefaults && (0 != (pItem = &pPool->GetDefaultItem(nWhichId))))
        {
            const SvxFontItem *pFont =
                        (const SvxFontItem *)pItem;
            Add( pFont->GetFamilyName(), pFont->GetStyleName(),
                    pFont->GetFamily(), pFont->GetPitch(),
                    pFont->GetCharSet() );
        }
        sal_uInt16 nItems = pPool->GetItemCount( nWhichId );
        for( sal_uInt16 j = 0; j < nItems; ++j )
        {
            if( 0 != (pItem = pPool->GetItem( nWhichId, j ) ) )
            {
                const SvxFontItem *pFont =
                            (const SvxFontItem *)pItem;
                Add( pFont->GetFamilyName(), pFont->GetStyleName(),
                     pFont->GetFamily(), pFont->GetPitch(),
                     pFont->GetCharSet() );
            }
        }
    }
}

ScXMLFontAutoStylePool_Impl::ScXMLFontAutoStylePool_Impl(
    ScXMLExport& rExport ) :
    XMLFontAutoStylePool( rExport )
{
    sal_uInt16 aWhichIds[3] = { ATTR_FONT, ATTR_CJK_FONT,
                                ATTR_CTL_FONT };
    sal_uInt16 aEditWhichIds[3] = { EE_CHAR_FONTINFO, EE_CHAR_FONTINFO_CJK,
                                    EE_CHAR_FONTINFO_CTL };
    sal_uInt16 aPageWhichIds[4] = { ATTR_PAGE_HEADERLEFT, ATTR_PAGE_FOOTERLEFT,
                                    ATTR_PAGE_HEADERRIGHT, ATTR_PAGE_FOOTERRIGHT };

    const SfxItemPool* pPool = rExport.GetDocument() ? rExport.GetDocument()->GetPool() : NULL;
    AddFontItems(aWhichIds, 3, pPool, sal_True);
    const SfxItemPool* pEditPool = rExport.GetDocument()->GetEditPool();
    AddFontItems(aEditWhichIds, 3, pEditPool, sal_False);

    SfxStyleSheetIterator* pItr = rExport.GetDocument() ? rExport.GetDocument()->GetStyleSheetPool()->CreateIterator(SFX_STYLE_FAMILY_PAGE, 0xFFFF) : NULL;
    if(pItr)
    {
        SfxStyleSheetBase* pStyle = pItr->First();
        SfxItemPool* pPageEditPool = EditEngine::CreatePool();
        EditEngine aEditEngine(pPageEditPool);
        while (pStyle)
        {
            const SfxItemPool& rPagePool = pStyle->GetPool().GetPool();
            for (sal_uInt8 j = 0; j < 4; j++)
            {
                sal_uInt16 nPageWhichId = aPageWhichIds[j];
                sal_uInt16 nPageHFItems = rPagePool.GetItemCount(nPageWhichId);
                const ScPageHFItem* pPageItem;
                for (sal_uInt16 k = 0; k < nPageHFItems; k++)
                {
                    if (0 != (pPageItem = static_cast<const ScPageHFItem*>(rPagePool.GetItem(nPageWhichId, k))))
                    {
                        const EditTextObject* pLeftArea = pPageItem->GetLeftArea();
                        if (pLeftArea)
                        {
                            aEditEngine.SetText(*pLeftArea);
                            AddFontItems(aEditWhichIds, 3, pPageEditPool, sal_False);
                        }
                        const EditTextObject* pCenterArea = pPageItem->GetCenterArea();
                        if (pCenterArea)
                        {
                            aEditEngine.SetText(*pCenterArea);
                            AddFontItems(aEditWhichIds, 3, pPageEditPool, sal_False);
                        }
                        const EditTextObject* pRightArea = pPageItem->GetRightArea();
                        if (pRightArea)
                        {
                            aEditEngine.SetText(*pRightArea);
                            AddFontItems(aEditWhichIds, 3, pPageEditPool, sal_False);
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
}
