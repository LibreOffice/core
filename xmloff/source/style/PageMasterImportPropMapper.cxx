/*************************************************************************
 *
 *  $RCSfile: PageMasterImportPropMapper.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: sab $ $Date: 2000-11-03 11:00:39 $
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

#pragma hdrstop

#ifndef _XMLOFF_PAGEMASTERIMPORTPROPMAPPER_HXX
#include "PageMasterImportPropMapper.hxx"
#endif
#ifndef _XMLOFF_PAGEMASTERPROPMAPPER_HXX
#include "PageMasterPropMapper.hxx"
#endif
#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include "PageMasterStyleMap.hxx"
#endif

#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include "maptype.hxx"
#endif
#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif

#define XML_LINE_LEFT 0
#define XML_LINE_RIGHT 1
#define XML_LINE_TOP 2
#define XML_LINE_BOTTOM 3

using namespace ::com::sun::star;

PageMasterImportPropertyMapper::PageMasterImportPropertyMapper(
        const UniReference< XMLPropertySetMapper >& rMapper ) :
    SvXMLImportPropertyMapper( rMapper )
{
}

PageMasterImportPropertyMapper::~PageMasterImportPropertyMapper()
{
}

/*sal_Bool PageMasterImportPropertyMapper::handleSpecialItem(
        XMLPropertyState& rProperty,
        ::std::vector< XMLPropertyState >& rProperties,
        const ::rtl::OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    return sal_True;
}*/

/*sal_Bool PageMasterImportPropertyMapper::handleNoItem(
        sal_Int32 nIndex,
        ::std::vector< XMLPropertyState >& rProperties,
        const ::rtl::OUString& rValue,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap ) const
{
    return sal_True;
}*/

void PageMasterImportPropertyMapper::finished(::std::vector< XMLPropertyState >& rProperties, sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const
{
    SvXMLImportPropertyMapper::finished(rProperties, nStartIndex, nEndIndex);
    XMLPropertyState* pAllPaddingProperty = NULL;
    XMLPropertyState* pPadding[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pNewPadding[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pAllBorderProperty = NULL;
    XMLPropertyState* pBorders[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pNewBorders[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pAllBorderWidthProperty = NULL;
    XMLPropertyState* pBorderWidths[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pAllHeaderPaddingProperty = NULL;
    XMLPropertyState* pHeaderPadding[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pHeaderNewPadding[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pAllHeaderBorderProperty = NULL;
    XMLPropertyState* pHeaderBorders[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pHeaderNewBorders[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pAllHeaderBorderWidthProperty = NULL;
    XMLPropertyState* pHeaderBorderWidths[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pAllFooterPaddingProperty = NULL;
    XMLPropertyState* pFooterPadding[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pFooterNewPadding[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pAllFooterBorderProperty = NULL;
    XMLPropertyState* pFooterBorders[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pFooterNewBorders[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pAllFooterBorderWidthProperty = NULL;
    XMLPropertyState* pFooterBorderWidths[4] = { NULL, NULL, NULL, NULL };
    XMLPropertyState* pHeaderHeight = NULL;
    XMLPropertyState* pHeaderMinHeight = NULL;
    XMLPropertyState* pHeaderDynamic = NULL;
    XMLPropertyState* pFooterHeight = NULL;
    XMLPropertyState* pFooterMinHeight = NULL;
    XMLPropertyState* pFooterDynamic = NULL;
    ::std::vector< XMLPropertyState >::iterator property = rProperties.begin();
    for (property; property != rProperties.end(); property++)
    {
        sal_Int16 nContextID = getPropertySetMapper()->GetEntryContextId(property->mnIndex);
        if (property->mnIndex >= nStartIndex && property->mnIndex < nEndIndex)
        {
            switch (nContextID)
            {
                case CTF_PM_PADDINGALL                  : pAllPaddingProperty = property; break;
                case CTF_PM_PADDINGLEFT                 : pPadding[XML_LINE_LEFT] = property; break;
                case CTF_PM_PADDINGRIGHT                : pPadding[XML_LINE_RIGHT] = property; break;
                case CTF_PM_PADDINGTOP                  : pPadding[XML_LINE_TOP] = property; break;
                case CTF_PM_PADDINGBOTTOM               : pPadding[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_BORDERALL                   : pAllBorderProperty = property; break;
                case CTF_PM_BORDERLEFT                  : pBorders[XML_LINE_LEFT] = property; break;
                case CTF_PM_BORDERRIGHT                 : pBorders[XML_LINE_RIGHT] = property; break;
                case CTF_PM_BORDERTOP                   : pBorders[XML_LINE_TOP] = property; break;
                case CTF_PM_BORDERBOTTOM                : pBorders[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_BORDERWIDTHALL              : pAllBorderWidthProperty = property; break;
                case CTF_PM_BORDERWIDTHLEFT             : pBorderWidths[XML_LINE_LEFT] = property; break;
                case CTF_PM_BORDERWIDTHRIGHT            : pBorderWidths[XML_LINE_RIGHT] = property; break;
                case CTF_PM_BORDERWIDTHTOP              : pBorderWidths[XML_LINE_TOP] = property; break;
                case CTF_PM_BORDERWIDTHBOTTOM           : pBorderWidths[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_HEADERPADDINGALL            : pAllHeaderPaddingProperty = property; break;
                case CTF_PM_HEADERPADDINGLEFT           : pHeaderPadding[XML_LINE_LEFT] = property; break;
                case CTF_PM_HEADERPADDINGRIGHT          : pHeaderPadding[XML_LINE_RIGHT] = property; break;
                case CTF_PM_HEADERPADDINGTOP            : pHeaderPadding[XML_LINE_TOP] = property; break;
                case CTF_PM_HEADERPADDINGBOTTOM         : pHeaderPadding[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_HEADERBORDERALL             : pAllHeaderBorderProperty = property; break;
                case CTF_PM_HEADERBORDERLEFT            : pHeaderBorders[XML_LINE_LEFT] = property; break;
                case CTF_PM_HEADERBORDERRIGHT           : pHeaderBorders[XML_LINE_RIGHT] = property; break;
                case CTF_PM_HEADERBORDERTOP             : pHeaderBorders[XML_LINE_TOP] = property; break;
                case CTF_PM_HEADERBORDERBOTTOM          : pHeaderBorders[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_HEADERBORDERWIDTHALL        : pAllHeaderBorderWidthProperty = property; break;
                case CTF_PM_HEADERBORDERWIDTHLEFT       : pHeaderBorderWidths[XML_LINE_LEFT] = property; break;
                case CTF_PM_HEADERBORDERWIDTHRIGHT      : pHeaderBorderWidths[XML_LINE_RIGHT] = property; break;
                case CTF_PM_HEADERBORDERWIDTHTOP        : pHeaderBorderWidths[XML_LINE_TOP] = property; break;
                case CTF_PM_HEADERBORDERWIDTHBOTTOM     : pHeaderBorderWidths[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_FOOTERPADDINGALL            : pAllFooterPaddingProperty = property; break;
                case CTF_PM_FOOTERPADDINGLEFT           : pFooterPadding[XML_LINE_LEFT] = property; break;
                case CTF_PM_FOOTERPADDINGRIGHT          : pFooterPadding[XML_LINE_RIGHT] = property; break;
                case CTF_PM_FOOTERPADDINGTOP            : pFooterPadding[XML_LINE_TOP] = property; break;
                case CTF_PM_FOOTERPADDINGBOTTOM         : pFooterPadding[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_FOOTERBORDERALL             : pAllFooterBorderProperty = property; break;
                case CTF_PM_FOOTERBORDERLEFT            : pFooterBorders[XML_LINE_LEFT] = property; break;
                case CTF_PM_FOOTERBORDERRIGHT           : pFooterBorders[XML_LINE_RIGHT] = property; break;
                case CTF_PM_FOOTERBORDERTOP             : pFooterBorders[XML_LINE_TOP] = property; break;
                case CTF_PM_FOOTERBORDERBOTTOM          : pFooterBorders[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_FOOTERBORDERWIDTHALL        : pAllFooterBorderWidthProperty = property; break;
                case CTF_PM_FOOTERBORDERWIDTHLEFT       : pFooterBorderWidths[XML_LINE_LEFT] = property; break;
                case CTF_PM_FOOTERBORDERWIDTHRIGHT      : pFooterBorderWidths[XML_LINE_RIGHT] = property; break;
                case CTF_PM_FOOTERBORDERWIDTHTOP        : pFooterBorderWidths[XML_LINE_TOP] = property; break;
                case CTF_PM_FOOTERBORDERWIDTHBOTTOM     : pFooterBorderWidths[XML_LINE_BOTTOM] = property; break;
                case CTF_PM_HEADERHEIGHT                : pHeaderHeight = property; break;
                case CTF_PM_HEADERMINHEIGHT             : pHeaderMinHeight = property; break;
                case CTF_PM_FOOTERHEIGHT                : pFooterHeight = property; break;
                case CTF_PM_FOOTERMINHEIGHT             : pFooterMinHeight = property; break;
            }
        }
    }

    for (sal_uInt16 i = 0; i < 4; i++)
    {
        if (pAllPaddingProperty && !pPadding[i])
            pNewPadding[i] = new XMLPropertyState(pAllPaddingProperty->mnIndex + 1 + i, pAllPaddingProperty->maValue);
        if (pAllBorderProperty && !pBorders[i])
        {
            pNewBorders[i] = new XMLPropertyState(pAllBorderProperty->mnIndex + 1 + i, pAllBorderProperty->maValue);
            pBorders[i] = pNewBorders[i];
        }
        if( !pBorderWidths[i] )
            pBorderWidths[i] = pAllBorderWidthProperty;
        else
            pBorderWidths[i]->mnIndex = -1;
        if( pBorders[i] )
        {
            table::BorderLine aBorderLine;
            pBorders[i]->maValue >>= aBorderLine;
             if( pBorderWidths[i] )
            {
                table::BorderLine aBorderLineWidth;
                pBorderWidths[i]->maValue >>= aBorderLineWidth;
                aBorderLine.OuterLineWidth = aBorderLineWidth.OuterLineWidth;
                aBorderLine.InnerLineWidth = aBorderLineWidth.InnerLineWidth;
                aBorderLine.LineDistance = aBorderLineWidth.LineDistance;
                pBorders[i]->maValue <<= aBorderLine;
            }
        }
        if (pAllHeaderPaddingProperty && !pHeaderPadding[i])
            pHeaderNewPadding[i] = new XMLPropertyState(pAllHeaderPaddingProperty->mnIndex + 1 + i, pAllHeaderPaddingProperty->maValue);
        if (pAllHeaderBorderProperty && !pHeaderBorders[i])
            pHeaderNewBorders[i] = new XMLPropertyState(pAllHeaderBorderProperty->mnIndex + 1 + i, pAllHeaderBorderProperty->maValue);
        if( !pHeaderBorderWidths[i] )
            pHeaderBorderWidths[i] = pAllHeaderBorderWidthProperty;
        else
            pHeaderBorderWidths[i]->mnIndex = -1;
        if( pHeaderBorders[i] )
        {
            table::BorderLine aBorderLine;
            pHeaderBorders[i]->maValue >>= aBorderLine;
             if( pHeaderBorderWidths[i] )
            {
                table::BorderLine aBorderLineWidth;
                pHeaderBorderWidths[i]->maValue >>= aBorderLineWidth;
                aBorderLine.OuterLineWidth = aBorderLineWidth.OuterLineWidth;
                aBorderLine.InnerLineWidth = aBorderLineWidth.InnerLineWidth;
                aBorderLine.LineDistance = aBorderLineWidth.LineDistance;
                pHeaderBorders[i]->maValue <<= aBorderLine;
            }
        }
        if (pAllFooterPaddingProperty && !pFooterPadding[i])
            pFooterNewPadding[i] = new XMLPropertyState(pAllFooterPaddingProperty->mnIndex + 1 + i, pAllFooterPaddingProperty->maValue);
        if (pAllFooterBorderProperty && !pFooterBorders[i])
            pFooterNewBorders[i] = new XMLPropertyState(pAllFooterBorderProperty->mnIndex + 1 + i, pAllFooterBorderProperty->maValue);
        if( !pFooterBorderWidths[i] )
            pFooterBorderWidths[i] = pAllFooterBorderWidthProperty;
        else
            pFooterBorderWidths[i]->mnIndex = -1;
        if( pFooterBorders[i] )
        {
            table::BorderLine aBorderLine;
            pFooterBorders[i]->maValue >>= aBorderLine;
             if( pFooterBorderWidths[i] )
            {
                table::BorderLine aBorderLineWidth;
                pFooterBorderWidths[i]->maValue >>= aBorderLineWidth;
                aBorderLine.OuterLineWidth = aBorderLineWidth.OuterLineWidth;
                aBorderLine.InnerLineWidth = aBorderLineWidth.InnerLineWidth;
                aBorderLine.LineDistance = aBorderLineWidth.LineDistance;
                pFooterBorders[i]->maValue <<= aBorderLine;
            }
        }
    }

    if (pHeaderHeight)
    {
        sal_Bool bValue(sal_False);
        uno::Any aAny;
        aAny.setValue( &bValue, ::getBooleanCppuType() );
        pHeaderDynamic = new XMLPropertyState(pHeaderHeight->mnIndex + 2, aAny);
    }
    if (pHeaderMinHeight)
    {
        sal_Bool bValue(sal_True);
        uno::Any aAny;
        aAny.setValue( &bValue, ::getBooleanCppuType() );
        pHeaderDynamic = new XMLPropertyState(pHeaderMinHeight->mnIndex + 1, aAny);
    }
    if (pFooterHeight)
    {
        sal_Bool bValue(sal_False);
        uno::Any aAny;
        aAny.setValue( &bValue, ::getBooleanCppuType() );
        pFooterDynamic = new XMLPropertyState(pFooterHeight->mnIndex + 2, aAny);
    }
    if (pFooterMinHeight)
    {
        sal_Bool bValue(sal_True);
        uno::Any aAny;
        aAny.setValue( &bValue, ::getBooleanCppuType() );
        pFooterDynamic = new XMLPropertyState(pFooterMinHeight->mnIndex + 1, aAny);
    }
    for (i = 0; i < 4; i++)
    {
        if (pNewPadding[i])
        {
            rProperties.push_back(*pNewPadding[i]);
            delete pNewPadding[i];
        }
        if (pNewBorders[i])
        {
            rProperties.push_back(*pNewBorders[i]);
            delete pNewBorders[i];
        }
        if (pHeaderNewPadding[i])
        {
            rProperties.push_back(*pHeaderNewPadding[i]);
            delete pHeaderNewPadding[i];
        }
        if (pHeaderNewBorders[i])
        {
            rProperties.push_back(*pHeaderNewBorders[i]);
            delete pHeaderNewBorders[i];
        }
        if (pFooterNewPadding[i])
        {
            rProperties.push_back(*pFooterNewPadding[i]);
            delete pFooterNewPadding[i];
        }
        if (pFooterNewBorders[i])
        {
            rProperties.push_back(*pFooterNewBorders[i]);
            delete pFooterNewBorders[i];
        }
    }
    if(pHeaderDynamic)
    {
        rProperties.push_back(*pHeaderDynamic);
        delete pHeaderDynamic;
    }
    if(pFooterDynamic)
    {
        rProperties.push_back(*pFooterDynamic);
        delete pFooterDynamic;
    }
}

