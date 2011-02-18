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

#include <rtl/ustring.hxx>

#include <rsc/rscsfx.hxx>

#include <xmloff/i18nmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>

#include <editeng/boxitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/memberids.hrc>

#include <svx/unomid.hxx>

#include <hintids.hxx>
#include <paratr.hxx>
#include <doc.hxx>
#include <unomid.h>
#include "xmlbrshi.hxx"
#include "xmlimp.hxx"
#include "xmlitmap.hxx"
#include "xmlimpit.hxx"
#include "xmlitem.hxx"

using ::rtl::OUString;
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
                                const SvXMLNamespaceMap& rNamespaceMap );

    virtual sal_Bool
    handleNoItem(SvXMLItemMapEntry const& rEntry,
                 SfxItemSet & rSet,
                 ::rtl::OUString const& rValue,
                 SvXMLUnitConverter const& rUnitConverter,
                 SvXMLNamespaceMap const& rNamespaceMap);

    virtual void finished(SfxItemSet & rSet,
                          SvXMLUnitConverter const& rUnitConverter) const;

    virtual void setMapEntries( SvXMLItemMapEntriesRef rMapEntries );

private:
    void Reset();

    ::rtl::OUString m_FoMarginValue;
    enum { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3 };
    bool m_bHaveMargin[4];
};

SwXMLImportTableItemMapper_Impl::SwXMLImportTableItemMapper_Impl(
                                        SvXMLItemMapEntriesRef rMapEntries ) :
    SvXMLImportItemMapper( rMapEntries, RES_UNKNOWNATR_CONTAINER)
{
    Reset();
}

SwXMLImportTableItemMapper_Impl::~SwXMLImportTableItemMapper_Impl()
{
}

void SwXMLImportTableItemMapper_Impl::Reset()
{
    m_FoMarginValue = ::rtl::OUString();
    for (int i = 0; i < 3; ++i)
    {
        m_bHaveMargin[i] = false;
    }
}

void SwXMLImportTableItemMapper_Impl::setMapEntries(
        SvXMLItemMapEntriesRef rMapEntries )
{
    Reset();
    SvXMLImportItemMapper::setMapEntries(rMapEntries);
}

sal_Bool SwXMLImportTableItemMapper_Impl::handleSpecialItem(
                                        const SvXMLItemMapEntry& rEntry,
                                        SfxPoolItem& rItem,
                                        SfxItemSet& rItemSet,
                                        const OUString& rValue,
                                        const SvXMLUnitConverter& rUnitConv,
                                        const SvXMLNamespaceMap& )
{
    sal_Bool bRet = sal_False;
    sal_uInt16 nMemberId = static_cast< sal_Int16 >(rEntry.nMemberId & MID_SW_FLAG_MASK);
    switch( rItem.Which() )
    {
    case RES_LR_SPACE:
        switch (nMemberId)
        {
            case MID_L_MARGIN:
                m_bHaveMargin[LEFT] = true;
                break;
            case MID_R_MARGIN:
                m_bHaveMargin[RIGHT] = true;
                break;
        }
        bRet = SvXMLImportItemMapper::PutXMLValue(
                rItem, rValue, nMemberId, rUnitConv);
        break;
    case RES_UL_SPACE:
        switch (nMemberId)
        {
            case MID_UP_MARGIN:
                m_bHaveMargin[TOP] = true;
                break;
            case MID_LO_MARGIN:
                m_bHaveMargin[BOTTOM] = true;
                break;
        }
        bRet = SvXMLImportItemMapper::PutXMLValue(
                rItem, rValue, nMemberId, rUnitConv);
        break;
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


sal_Bool SwXMLImportTableItemMapper_Impl::handleNoItem(
     SvXMLItemMapEntry const& rEntry,
     SfxItemSet & rSet,
     ::rtl::OUString const& rValue,
     SvXMLUnitConverter const& rUnitConverter,
     SvXMLNamespaceMap const& rNamespaceMap)
{
    if ((XML_NAMESPACE_FO == rEntry.nNameSpace) &&
        (xmloff::token::XML_MARGIN == rEntry.eLocalName))
    {
        m_FoMarginValue = rValue;
        return true;
    }
    else
    {
        return SvXMLImportItemMapper::handleNoItem(
                rEntry, rSet, rValue, rUnitConverter, rNamespaceMap);
    }
}

void SwXMLImportTableItemMapper_Impl::finished(
        SfxItemSet & rSet, SvXMLUnitConverter const& rUnitConverter) const
{
    if (m_FoMarginValue.getLength())
    {
        sal_uInt16 const Ids[4][2] = {
            { RES_LR_SPACE, MID_L_MARGIN },
            { RES_LR_SPACE, MID_R_MARGIN },
            { RES_UL_SPACE, MID_UP_MARGIN },
            { RES_UL_SPACE, MID_LO_MARGIN },
        };
        for (int i = 0; i < 4; ++i)
        {
            if (m_bHaveMargin[i])
            {
                continue; // already read fo:margin-top etc.
            }
            // first get item from itemset
            SfxPoolItem const* pItem = 0;
            SfxItemState eState =
                rSet.GetItemState(Ids[i][0], sal_True, &pItem);

            // if not set, try the pool
            if ((SFX_ITEM_SET != eState) && (SFX_WHICH_MAX > Ids[i][0]))
            {
                pItem = &rSet.GetPool()->GetDefaultItem(Ids[i][0]);
            }

            // do we have an item?
            if (eState >= SFX_ITEM_DEFAULT && pItem)
            {
                SfxPoolItem *const pNewItem = pItem->Clone();
                bool const bPut = PutXMLValue(
                        *pNewItem, m_FoMarginValue, Ids[i][1], rUnitConverter);
                if (bPut)
                {
                    rSet.Put(*pNewItem);
                }
            }
            else
            {
                OSL_ENSURE(false, "could not get item");
            }
        }
    }
}

// ---------------------------------------------------------------------

class SwXMLItemSetContext_Impl : public SvXMLItemSetContext
{
    SvXMLImportContextRef xBackground;

    using SvXMLItemSetContext::CreateChildContext;

public:
    SwXMLItemSetContext_Impl( SwXMLImport& rImport, sal_uInt16 nPrfx,
                  const OUString& rLName,
                  const Reference< xml::sax::XAttributeList > & xAttrList,
                  SfxItemSet&  rItemSet,
                  SvXMLImportItemMapper & rIMapper,
                  const SvXMLUnitConverter& rUnitConv );
    virtual ~SwXMLItemSetContext_Impl();

    virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                   const ::rtl::OUString& rLocalName,
                   const ::uno::Reference< xml::sax::XAttributeList > & xAttrList,
                   SfxItemSet&  rItemSet,
                   const SvXMLItemMapEntry& rEntry,
                   const SvXMLUnitConverter& rUnitConv );
};

SwXMLItemSetContext_Impl::SwXMLItemSetContext_Impl(
                 SwXMLImport& rImport, sal_uInt16 nPrfx,
                 const OUString& rLName,
                 const Reference< xml::sax::XAttributeList > & xAttrList,
                 SfxItemSet&  _rItemSet,
                 SvXMLImportItemMapper & _rIMapper,
                 const SvXMLUnitConverter& _rUnitConv ) :
    SvXMLItemSetContext( rImport, nPrfx, rLName, xAttrList,
                         _rItemSet, _rIMapper, _rUnitConv )
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
                   SfxItemSet&  _rItemSet,
                   const SvXMLItemMapEntry& rEntry,
                   const SvXMLUnitConverter& _rUnitConv )
{
    SvXMLImportContext *pContext = 0;

    switch( rEntry.nWhichId )
    {
    case RES_BACKGROUND:
        {
            const SfxPoolItem *pItem;
            if( SFX_ITEM_SET == _rItemSet.GetItemState( RES_BACKGROUND,
                                                       sal_False, &pItem ) )
            {
                pContext = new SwXMLBrushItemImportContext(
                                GetImport(), nPrefix, rLocalName, xAttrList,
                                _rUnitConv, *(const SvxBrushItem *)pItem );
            }
            else
            {
                pContext = new SwXMLBrushItemImportContext(
                                GetImport(), nPrefix, rLocalName, xAttrList,
                                _rUnitConv, RES_BACKGROUND );
            }
            xBackground = pContext;
        }
        break;
    }

    if( !pContext )
        pContext = SvXMLItemSetContext::CreateChildContext( nPrefix, rLocalName,
                                                            xAttrList, _rItemSet,
                                                            rEntry, _rUnitConv );

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
