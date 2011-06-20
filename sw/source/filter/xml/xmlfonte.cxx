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
#include "precompiled_sw.hxx"

#include "hintids.hxx"
#include <com/sun/star/text/XTextDocument.hpp>
#include <xmloff/XMLFontAutoStylePool.hxx>
#include <editeng/fontitem.hxx>
#include <unotext.hxx>
#include <doc.hxx>
#include <xmlexp.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::text;

class SwXMLFontAutoStylePool_Impl: public XMLFontAutoStylePool
{
    public:

    SwXMLFontAutoStylePool_Impl( SwXMLExport& rExport );

};

SwXMLFontAutoStylePool_Impl::SwXMLFontAutoStylePool_Impl(
    SwXMLExport& _rExport ) :
    XMLFontAutoStylePool( _rExport )
{
    sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT,
                                RES_CHRATR_CTL_FONT };

    Reference < XTextDocument > xTextDoc( _rExport.GetModel(), UNO_QUERY );
    Reference < XText > xText = xTextDoc->getText();
    Reference<XUnoTunnel> xTextTunnel( xText, UNO_QUERY);
    OSL_ENSURE( xTextTunnel.is(), "missing XUnoTunnel for Cursor" );
    if( !xTextTunnel.is() )
        return;

    SwXText *pText = reinterpret_cast< SwXText *>(
            sal::static_int_cast< sal_IntPtr >( xTextTunnel->getSomething( SwXText::getUnoTunnelId() )));
    OSL_ENSURE( pText, "SwXText missing" );
    if( !pText )
        return;

    const SfxItemPool& rPool = pText->GetDoc()->GetAttrPool();
    const SfxPoolItem* pItem;
    for( sal_uInt16 i=0; i<3; i++ )
    {
        sal_uInt16 nWhichId = aWhichIds[i];

        const SvxFontItem& rFont =
            (const SvxFontItem&)rPool.GetDefaultItem( nWhichId );
        Add( rFont.GetFamilyName(), rFont.GetStyleName(),
             static_cast< sal_uInt16 >(rFont.GetFamily()), static_cast< sal_uInt16 >(rFont.GetPitch()),
             rFont.GetCharSet() );
        sal_uInt32 nItems = rPool.GetItemCount2( nWhichId );
        for( sal_uInt32 j = 0; j < nItems; ++j )
        {
            if( 0 != (pItem = rPool.GetItem2( nWhichId, j ) ) )
            {
                const SvxFontItem *pFont =
                            (const SvxFontItem *)pItem;
                Add( pFont->GetFamilyName(), pFont->GetStyleName(),
                     static_cast< sal_uInt16 >(pFont->GetFamily()), static_cast< sal_uInt16 >(pFont->GetPitch()),
                     pFont->GetCharSet() );
            }
        }
    }
}


XMLFontAutoStylePool* SwXMLExport::CreateFontAutoStylePool()
{
    return new SwXMLFontAutoStylePool_Impl( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
