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

#include "hintids.hxx"
#include <com/sun/star/text/XTextDocument.hpp>
#include <xmloff/XMLFontAutoStylePool.hxx>
#include <editeng/fontitem.hxx>
#include <unotext.hxx>
#include <doc.hxx>
#include <xmlexp.hxx>
#include <xmlimp.hxx>
#include <IDocumentSettingAccess.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;

class SwXMLFontAutoStylePool_Impl: public XMLFontAutoStylePool
{
    public:
    SwXMLFontAutoStylePool_Impl( SwXMLExport& rExport, bool blockFontEmbedding );
};

SwXMLFontAutoStylePool_Impl::SwXMLFontAutoStylePool_Impl(
    SwXMLExport& _rExport, bool blockFontEmbedding ) :
    XMLFontAutoStylePool( _rExport, blockFontEmbedding )
{
    sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT,
                                RES_CHRATR_CTL_FONT };

    const SfxItemPool& rPool = _rExport.getDoc()->GetAttrPool();
    const SfxPoolItem* pItem;
    for( size_t i=0; i < SAL_N_ELEMENTS(aWhichIds); ++i )
    {
        const sal_uInt16 nWhichId = aWhichIds[i];

        const SvxFontItem& rFont =
            static_cast<const SvxFontItem&>(rPool.GetDefaultItem( nWhichId ));
        Add( rFont.GetFamilyName(), rFont.GetStyleName(),
             rFont.GetFamily(), rFont.GetPitch(),
             rFont.GetCharSet() );
        sal_uInt32 nItems = rPool.GetItemCount2( nWhichId );
        for( sal_uInt32 j = 0; j < nItems; ++j )
        {
            if( 0 != (pItem = rPool.GetItem2( nWhichId, j ) ) )
            {
                const SvxFontItem *pFont =
                            static_cast<const SvxFontItem *>(pItem);
                Add( pFont->GetFamilyName(), pFont->GetStyleName(),
                     pFont->GetFamily(), pFont->GetPitch(),
                     pFont->GetCharSet() );
            }
        }
    }
}

XMLFontAutoStylePool* SwXMLExport::CreateFontAutoStylePool()
{
    bool blockFontEmbedding = false;
    // We write font info to both content.xml and styles.xml, but they are both
    // written by different SwXMLExport instance, and would therefore write each
    // font file twice without complicated checking for duplicates, so handle
    // the embedding only in one of them.
    if( !( getExportFlags() & SvXMLExportFlags::CONTENT) )
        blockFontEmbedding = true;
    if( !getDoc()->getIDocumentSettingAccess().get( DocumentSettingId::EMBED_FONTS ))
        blockFontEmbedding = true;
    return new SwXMLFontAutoStylePool_Impl( *this, !blockFontEmbedding );
}

void SwXMLImport::NotifyEmbeddedFontRead()
{
    getDoc()->getIDocumentSettingAccess().set( DocumentSettingId::EMBED_FONTS, true );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

