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

#include <rtl/ustring.hxx>

#include <com/sun/star/util/MeasureUnit.hpp>

#include <svl/style.hxx>

#include <xmloff/i18nmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>

#include <editeng/boxitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/memberids.h>

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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

class SwXMLImportTableItemMapper_Impl: public SvXMLImportItemMapper
{

public:

    explicit SwXMLImportTableItemMapper_Impl(const SvXMLItemMapEntriesRef& rMapEntries);

    virtual bool handleSpecialItem( const SvXMLItemMapEntry& rEntry,
                                SfxPoolItem& rItem,
                                SfxItemSet& rSet,
                                const OUString& rValue,
                                const SvXMLUnitConverter& rUnitConverter,
                                const SvXMLNamespaceMap& rNamespaceMap ) override;

    virtual bool
    handleNoItem(SvXMLItemMapEntry const& rEntry,
                 SfxItemSet & rSet,
                 OUString const& rValue,
                 SvXMLUnitConverter const& rUnitConverter,
                 SvXMLNamespaceMap const& rNamespaceMap) override;

    virtual void finished(SfxItemSet & rSet,
                          SvXMLUnitConverter const& rUnitConverter) const override;

    virtual void setMapEntries( SvXMLItemMapEntriesRef rMapEntries ) override;

private:
    void Reset();

    OUString m_FoMarginValue;
    enum { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3 };
    bool m_bHaveMargin[4];
};

SwXMLImportTableItemMapper_Impl::SwXMLImportTableItemMapper_Impl(
                                        const SvXMLItemMapEntriesRef& rMapEntries ) :
    SvXMLImportItemMapper( rMapEntries )
{
    Reset();
}

void SwXMLImportTableItemMapper_Impl::Reset()
{
    m_FoMarginValue.clear();
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

bool SwXMLImportTableItemMapper_Impl::handleSpecialItem(
                                        const SvXMLItemMapEntry& rEntry,
                                        SfxPoolItem& rItem,
                                        SfxItemSet& rItemSet,
                                        const OUString& rValue,
                                        const SvXMLUnitConverter& rUnitConv,
                                        const SvXMLNamespaceMap& )
{
    bool bRet = false;
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
            if( SfxItemState::SET != rItemSet.GetItemState( RES_FRM_SIZE,
                                                       false ) )
                bRet = SvXMLImportItemMapper::PutXMLValue(
                    rItem, rValue, nMemberId, rUnitConv );
            break;
        }
    }

    return bRet;
}

bool SwXMLImportTableItemMapper_Impl::handleNoItem(
     SvXMLItemMapEntry const& rEntry,
     SfxItemSet & rSet,
     OUString const& rValue,
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
    if (!m_FoMarginValue.isEmpty())
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
            SfxPoolItem const* pItem = nullptr;
            SfxItemState eState =
                rSet.GetItemState(Ids[i][0], true, &pItem);

            // if not set, try the pool
            if ((SfxItemState::SET != eState) && SfxItemPool::IsWhich(Ids[i][0]))
            {
                pItem = &rSet.GetPool()->GetDefaultItem(Ids[i][0]);
            }

            // do we have an item?
            if (eState >= SfxItemState::DEFAULT && pItem)
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
    virtual ~SwXMLItemSetContext_Impl() override;

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix,
                   const OUString& rLocalName,
                   const ::uno::Reference< xml::sax::XAttributeList > & xAttrList,
                   SfxItemSet&  rItemSet,
                   const SvXMLItemMapEntry& rEntry,
                   const SvXMLUnitConverter& rUnitConv ) override;
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
    if( xBackground.is() )
    {
        const SvxBrushItem& rItem =
            static_cast<SwXMLBrushItemImportContext*>(xBackground.get())->GetItem();
        rItemSet.Put( rItem );
    }
}

SvXMLImportContextRef SwXMLItemSetContext_Impl::CreateChildContext(
                   sal_uInt16 nPrefix,
                   const OUString& rLocalName,
                   const Reference< xml::sax::XAttributeList > & xAttrList,
                   SfxItemSet&  _rItemSet,
                   const SvXMLItemMapEntry& rEntry,
                   const SvXMLUnitConverter& _rUnitConv )
{
    SvXMLImportContextRef xContext;

    switch( rEntry.nWhichId )
    {
    case RES_BACKGROUND:
        {
            const SfxPoolItem *pItem;
            if( SfxItemState::SET == _rItemSet.GetItemState( RES_BACKGROUND,
                                                       false, &pItem ) )
            {
                xContext = new SwXMLBrushItemImportContext(
                                GetImport(), nPrefix, rLocalName, xAttrList,
                                _rUnitConv, *static_cast<const SvxBrushItem *>(pItem) );
            }
            else
            {
                xContext = new SwXMLBrushItemImportContext(
                                GetImport(), nPrefix, rLocalName, xAttrList,
                                _rUnitConv, RES_BACKGROUND );
            }
            xBackground = xContext;
        }
        break;
    }

    if (!xContext)
        xContext = SvXMLItemSetContext::CreateChildContext( nPrefix, rLocalName,
                                                            xAttrList, _rItemSet,
                                                            rEntry, _rUnitConv );

    return xContext;
}

void SwXMLImport::InitItemImport()
{
    m_pTwipUnitConv.reset( new SvXMLUnitConverter( GetComponentContext(),
            util::MeasureUnit::TWIP, util::MeasureUnit::TWIP ) );

    m_xTableItemMap = new SvXMLItemMapEntries( aXMLTableItemMap );
    m_xTableColItemMap = new SvXMLItemMapEntries( aXMLTableColItemMap );
    m_xTableRowItemMap = new SvXMLItemMapEntries( aXMLTableRowItemMap );
    m_xTableCellItemMap = new SvXMLItemMapEntries( aXMLTableCellItemMap );

    m_pTableItemMapper.reset( new SwXMLImportTableItemMapper_Impl( m_xTableItemMap ) );
}

void SwXMLImport::FinitItemImport()
{
    m_pTableItemMapper.reset();
    m_pTwipUnitConv.reset();
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
        xItemMap = m_xTableItemMap;
        break;
    case XML_STYLE_FAMILY_TABLE_COLUMN:
        xItemMap = m_xTableColItemMap;
        break;
    case XML_STYLE_FAMILY_TABLE_ROW:
        xItemMap = m_xTableRowItemMap;
        break;
    case XML_STYLE_FAMILY_TABLE_CELL:
        xItemMap = m_xTableCellItemMap;
        break;
    }

    m_pTableItemMapper->setMapEntries( xItemMap );

    return new SwXMLItemSetContext_Impl( *this, nPrefix, rLocalName,
                                            xAttrList, rItemSet,
                                            GetTableItemMapper(),
                                            *m_pTwipUnitConv );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
