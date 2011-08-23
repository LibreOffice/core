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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <hintids.hxx>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RSCSFX_HXX
#include <rsc/rscsfx.hxx>
#endif

#ifndef _XMLITMAP_HXX
#include "xmlitmap.hxx"
#endif
#ifndef _XMLIMPIT_HXX
#include "xmlimpit.hxx"
#endif
#ifndef _XMLITEM_HXX
#include "xmlitem.hxx"
#endif
#ifndef _XMLOFF_I18NMAP_HXX
#include <bf_xmloff/i18nmap.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <bf_xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX
#include <bf_xmloff/families.hxx>
#endif

#ifndef _SVX_UNOMID_HXX
#include <bf_svx/unomid.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX
#include <bf_svx/boxitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <bf_svx/fontitem.hxx>
#endif
#ifndef _SVX_TSPTITEM_HXX
#include <bf_svx/tstpitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX
#include <bf_svx/boxitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <bf_svx/brshitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <bf_svx/langitem.hxx>
#endif

#ifndef _XMLOFF_XMLTABI_HXX
//#include <bf_xmloff/xmltabi.hxx>
#endif

#ifndef _XMLBRSHI_HXX
#include "xmlbrshi.hxx"
#endif

#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX //autogen wg. SwDoc
#include <doc.hxx>
#endif
#ifndef _UNOMID_H
#include <unomid.h>
#endif

#ifndef _XMLIMP_HXX
#include "xmlimp.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

extern SvXMLItemMapEntry aXMLTableItemMap[];
extern SvXMLItemMapEntry aXMLTableColItemMap[];
extern SvXMLItemMapEntry aXMLTableRowItemMap[];
extern SvXMLItemMapEntry aXMLTableCellItemMap[];

class SwXMLImportTableItemMapper_Impl: public SvXMLImportItemMapper
{

public:

    SwXMLImportTableItemMapper_Impl( SvXMLItemMapEntriesRef rMapEntries );
    virtual ~SwXMLImportTableItemMapper_Impl();

    virtual sal_Bool handleSpecialItem( const SvXMLItemMapEntry& rEntry,
                                SfxPoolItem& rItem,
                                SfxItemSet& rSet,
                                const OUString& rValue,
                                const SvXMLUnitConverter& rUnitConverter,
                                const SvXMLNamespaceMap& rNamespaceMap ) const;
    virtual void finished( SfxItemSet& rSet ) const;
};

SwXMLImportTableItemMapper_Impl::SwXMLImportTableItemMapper_Impl(
                                        SvXMLItemMapEntriesRef rMapEntries ) :
    SvXMLImportItemMapper( rMapEntries, RES_UNKNOWNATR_CONTAINER)
{
}

SwXMLImportTableItemMapper_Impl::~SwXMLImportTableItemMapper_Impl()
{
}

sal_Bool SwXMLImportTableItemMapper_Impl::handleSpecialItem(
                                        const SvXMLItemMapEntry& rEntry,
                                        SfxPoolItem& rItem,
                                        SfxItemSet& rItemSet,
                                        const OUString& rValue,
                                        const SvXMLUnitConverter& rUnitConv,
                                        const SvXMLNamespaceMap& ) const
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nMemberId = rEntry.nMemberId & MID_FLAG_MASK;
    switch( rItem.Which() )
    {
    case RES_FRM_SIZE:
        switch( nMemberId )
        {
        case MID_FRMSIZE_COL_WIDTH:
            // If the item is existing already, a relative value has been set
            // already that must be preserved.
            if( SFX_ITEM_SET != rItemSet.GetItemState( RES_FRM_SIZE,
                                                       sal_False ) )
                bRet = SvXMLImportItemMapper::PutXMLValue( 
                    rItem, rValue, nMemberId, rUnitConv );
            break;
        }
    }
    
    return bRet;
}

void SwXMLImportTableItemMapper_Impl::finished( SfxItemSet& rSet ) const
{
#if 0
    const SfxPoolItem *pItem;
    
    // ensure that box item have a distance to a border.
    if( SFX_ITEM_SET == rSet.GetItemState( RES_BOX, sal_False, &pItem ) )
    {
        const SvxBoxItem *pBox = (const SvxBoxItem *)pItem;
        sal_uInt16 aLines[4] = { BOX_LINE_TOP, BOX_LINE_BOTTOM,
                             BOX_LINE_LEFT, BOX_LINE_RIGHT };
        sal_uInt16 i;
        for( i=0; i<4; i++ )
        {
            if( pBox->GetLine( aLines[i] ) &&
                pBox->GetDistance( aLines[i] ) < MIN_BORDER_DIST )
                break;
        }
        if( i < 4 )
        {
            SvxBoxItem aBox( *pBox );
            for( /*i=0*/; i<4; i++ )	// i points to the mod. line
            {
                if( aBox.GetLine( aLines[i] ) &&
                    aBox.GetDistance( aLines[i] ) < MIN_BORDER_DIST )
                    aBox.SetDistance( MIN_BORDER_DIST, aLines[i] );
            }
            rSet.Put( aBox );
        }
    }
#endif
}

// ---------------------------------------------------------------------

class SwXMLItemSetContext_Impl : public SvXMLItemSetContext
{
    SvXMLImportContextRef xBackground;

public:
    SwXMLItemSetContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
                  const OUString& rLName,
                  const Reference< xml::sax::XAttributeList > & xAttrList,
                  SfxItemSet&  rItemSet,
                  const SvXMLImportItemMapper& rIMapper,
                  const SvXMLUnitConverter& rUnitConv );
    virtual ~SwXMLItemSetContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                   const OUString& rLocalName,
                   const Reference< xml::sax::XAttributeList > & xAttrList,
                   SfxItemSet&  rItemSet,
                   const SvXMLItemMapEntry& rEntry,
                   const SvXMLUnitConverter& rUnitConv );
};

SwXMLItemSetContext_Impl::SwXMLItemSetContext_Impl(
                 SwXMLImport& rImport, sal_uInt16 nPrfx,
                 const OUString& rLName,
                 const Reference< xml::sax::XAttributeList > & xAttrList,
                 SfxItemSet&  rItemSet,
                 const SvXMLImportItemMapper& rIMapper,
                 const SvXMLUnitConverter& rUnitConv ) :
    SvXMLItemSetContext( rImport, nPrfx, rLName, xAttrList,
                         rItemSet, rIMapper, rUnitConv )
{
}

SwXMLItemSetContext_Impl::~SwXMLItemSetContext_Impl()
{
    if( xBackground.Is() )
    {
        const SvxBrushItem& rItem =
            ((SwXMLBrushItemImportContext*)&xBackground)->GetItem();
        rItemSet.Put( rItem );
    }
}
    
SvXMLImportContext *SwXMLItemSetContext_Impl::CreateChildContext(
                   sal_uInt16 nPrefix,
                   const OUString& rLocalName,
                   const Reference< xml::sax::XAttributeList > & xAttrList,
                   SfxItemSet&  rItemSet,
                   const SvXMLItemMapEntry& rEntry,
                   const SvXMLUnitConverter& rUnitConv )
{
    SvXMLImportContext *pContext = 0;

    switch( rEntry.nWhichId )
    {
    case RES_BACKGROUND:
        {
            const SfxPoolItem *pItem;
            if( SFX_ITEM_SET == rItemSet.GetItemState( RES_BACKGROUND,
                                                       sal_False, &pItem ) )
            {
                pContext = new SwXMLBrushItemImportContext(
                                GetImport(), nPrefix, rLocalName, xAttrList,
                                rUnitConv, *(const SvxBrushItem *)pItem );
            }
            else
            {
                pContext = new SwXMLBrushItemImportContext(
                                GetImport(), nPrefix, rLocalName, xAttrList,
                                rUnitConv, RES_BACKGROUND );
            }
            xBackground = pContext;
        }
        break;
    }
    
    if( !pContext )
        pContext = SvXMLItemSetContext::CreateChildContext( nPrefix, rLocalName,
                                                            xAttrList, rItemSet,
                                                            rEntry, rUnitConv );
    
    return pContext;
}

// ---------------------------------------------------------------------

void SwXMLImport::_InitItemImport()
{
    // #110680#
    pTwipUnitConv = new SvXMLUnitConverter( MAP_TWIP, MAP_TWIP, getServiceFactory() );

    xTableItemMap = new SvXMLItemMapEntries( aXMLTableItemMap );
    xTableColItemMap = new SvXMLItemMapEntries( aXMLTableColItemMap );
    xTableRowItemMap = new SvXMLItemMapEntries( aXMLTableRowItemMap );
    xTableCellItemMap = new SvXMLItemMapEntries( aXMLTableCellItemMap );

    pTableItemMapper = new SwXMLImportTableItemMapper_Impl( xTableItemMap );
}

void SwXMLImport::_FinitItemImport()
{
    delete pTableItemMapper;
    delete pTwipUnitConv;
}

SvXMLImportContext *SwXMLImport::CreateTableItemImportContext(
                  sal_uInt16 nPrefix,
                  const OUString& rLocalName,
                  const Reference< xml::sax::XAttributeList > & xAttrList,
                  sal_uInt16 nFamily,
                  SfxItemSet& rItemSet )
{
    SvXMLItemMapEntriesRef xItemMap;

    switch( nFamily )
    {
    case XML_STYLE_FAMILY_TABLE_TABLE:
        xItemMap = xTableItemMap;
        break;
    case XML_STYLE_FAMILY_TABLE_COLUMN:
        xItemMap = xTableColItemMap;
        break;
    case XML_STYLE_FAMILY_TABLE_ROW:
        xItemMap = xTableRowItemMap;
        break;
    case XML_STYLE_FAMILY_TABLE_CELL:
        xItemMap = xTableCellItemMap;
        break;
    }

    pTableItemMapper->setMapEntries( xItemMap );

    return new SwXMLItemSetContext_Impl( *this, nPrefix, rLocalName,
                                            xAttrList, rItemSet,
                                            GetTableItemMapper(),
                                            GetTwipUnitConverter() );
}
}
