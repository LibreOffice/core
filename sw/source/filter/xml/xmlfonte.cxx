/*************************************************************************
 *
 *  $RCSfile: xmlfonte.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mib $ $Date: 2000-11-14 12:03:30 $
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

#ifndef _HINTIDS_HXX
#include "hintids.hxx"
#endif

#ifndef _XMLOFF_XMLFONTAUTOSTYLEPOOL_HXX
#include <xmloff/XMLFontAutoStylePool.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif

#ifndef _XMLEXP_HXX
#include <xmlexp.hxx>
#endif


class SwXMLFontAutoStylePool_Impl: public XMLFontAutoStylePool
{
    public:

    SwXMLFontAutoStylePool_Impl( SwXMLExport& rExport );

};

SwXMLFontAutoStylePool_Impl::SwXMLFontAutoStylePool_Impl(
    SwXMLExport& rExport ) :
    XMLFontAutoStylePool( rExport )
{
    sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT,
                                RES_CHRATR_CTL_FONT };
    const SfxItemPool& rPool = rExport.GetDoc().GetAttrPool();
    const SfxPoolItem* pItem;
    for( sal_uInt16 i=0; i<3; i++ )
    {
        sal_uInt16 nWhichId = aWhichIds[i];
        sal_uInt16 nItems = rPool.GetItemCount( nWhichId );
        for( sal_uInt16 j = 0; j < nItems; ++j )
        {
            if( 0 != (pItem = rPool.GetItem( nWhichId, j ) ) )
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


XMLFontAutoStylePool* SwXMLExport::CreateFontAutoStylePool()
{
    return new SwXMLFontAutoStylePool_Impl( *this );
}
