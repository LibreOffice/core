/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlfonte.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 10:11:03 $
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
#include "precompiled_sw.hxx"


#ifndef _HINTIDS_HXX
#include "hintids.hxx"
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP_
#include <com/sun/star/text/XTextDocument.hpp>
#endif
#ifndef _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX
#include <xmloff/XMLFontAutoStylePool.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif

#ifndef _XMLEXP_HXX
#include <xmlexp.hxx>
#endif

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
        sal_uInt16 nItems = rPool.GetItemCount( nWhichId );
        for( sal_uInt16 j = 0; j < nItems; ++j )
        {
            if( 0 != (pItem = rPool.GetItem( nWhichId, j ) ) )
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
