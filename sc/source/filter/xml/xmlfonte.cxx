/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlfonte.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 12:53:13 $
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
#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif


#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif

#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#ifndef _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX
#include <xmloff/XMLFontAutoStylePool.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#ifndef _MyEDITENG_HXX
#include <svx/editeng.hxx>
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

class ScXMLFontAutoStylePool_Impl: public XMLFontAutoStylePool
{
    void AddFontItems(sal_uInt16* pWhichIds, sal_uInt8 nIdCount, const SfxItemPool* pPool, const sal_Bool bExportDefaults);
    public:

    ScXMLFontAutoStylePool_Impl( ScXMLExport& rExport );

};

void ScXMLFontAutoStylePool_Impl::AddFontItems(sal_uInt16* pWhichIds, sal_uInt8 nIdCount, const SfxItemPool* pPool, const sal_Bool bExportDefaults)
{
    const SfxPoolItem* pItem;
    for( sal_uInt16 i=0; i < nIdCount; ++i )
    {
        sal_uInt16 nWhichId(pWhichIds[i]);
        if (bExportDefaults && (0 != (pItem = &pPool->GetDefaultItem(nWhichId))))
        {
            const SvxFontItem *pFont((const SvxFontItem *)pItem);
            Add( pFont->GetFamilyName(), pFont->GetStyleName(),
                    pFont->GetFamily(), pFont->GetPitch(),
                    pFont->GetCharSet() );
        }
        sal_uInt16 nItems(pPool->GetItemCount( nWhichId ));
        for( sal_uInt16 j = 0; j < nItems; ++j )
        {
            if( 0 != (pItem = pPool->GetItem( nWhichId, j ) ) )
            {
                const SvxFontItem *pFont((const SvxFontItem *)pItem);
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

    const SfxItemPool* pPool(rExport.GetDocument() ? rExport.GetDocument()->GetPool() : NULL);
    AddFontItems(aWhichIds, 3, pPool, sal_True);
    const SfxItemPool* pEditPool(rExport.GetDocument()->GetEditPool());
    AddFontItems(aEditWhichIds, 3, pEditPool, sal_False);

    SfxStyleSheetIterator* pItr(rExport.GetDocument() ? rExport.GetDocument()->GetStyleSheetPool()->CreateIterator(SFX_STYLE_FAMILY_PAGE, 0xFFFF) : NULL);
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
                sal_uInt16 nPageHFItems(rPagePool.GetItemCount(nPageWhichId));
                const ScPageHFItem* pPageItem;
                for (sal_uInt16 k = 0; k < nPageHFItems; ++k)
                {
                    if (0 != (pPageItem = static_cast<const ScPageHFItem*>(rPagePool.GetItem(nPageWhichId, k))))
                    {
                        const EditTextObject* pLeftArea(pPageItem->GetLeftArea());
                        if (pLeftArea)
                        {
                            aEditEngine.SetText(*pLeftArea);
                            AddFontItems(aEditWhichIds, 3, pPageEditPool, sal_False);
                        }
                        const EditTextObject* pCenterArea(pPageItem->GetCenterArea());
                        if (pCenterArea)
                        {
                            aEditEngine.SetText(*pCenterArea);
                            AddFontItems(aEditWhichIds, 3, pPageEditPool, sal_False);
                        }
                        const EditTextObject* pRightArea(pPageItem->GetRightArea());
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
