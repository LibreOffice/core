/*************************************************************************
 *
 *  $RCSfile: xmlfonte.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-27 16:05:20 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
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

    const SfxItemPool* pPool = rExport.GetDocument()->GetPool();
    AddFontItems(aWhichIds, 3, pPool, sal_True);
    const SfxItemPool* pEditPool = rExport.GetDocument()->GetEditPool();
    AddFontItems(aEditWhichIds, 3, pEditPool, sal_False);

    SfxStyleSheetIterator* pItr = rExport.GetDocument()->GetStyleSheetPool()->CreateIterator(SFX_STYLE_FAMILY_PAGE, 0xFFFF);
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
