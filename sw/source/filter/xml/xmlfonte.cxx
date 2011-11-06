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
    ASSERT( xTextTunnel.is(), "missing XUnoTunnel for Cursor" );
    if( !xTextTunnel.is() )
        return;

    SwXText *pText = reinterpret_cast< SwXText *>(
            sal::static_int_cast< sal_IntPtr >( xTextTunnel->getSomething( SwXText::getUnoTunnelId() )));
    ASSERT( pText, "SwXText missing" );
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
