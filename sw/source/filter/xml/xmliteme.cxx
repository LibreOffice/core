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

#include <com/sun/star/util/MeasureUnit.hpp>

#include <hintids.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <utility>
#include <xmloff/xmluconv.hxx>
#include "xmlexpit.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltabe.hxx>
#include "xmlbrshe.hxx"
#include <editeng/tstpitem.hxx>
#include <editeng/brushitem.hxx>
#include <tools/fldunit.hxx>
#include <swmodule.hxx>
#include <doc.hxx>
#include <fmtornt.hxx>
#include <unomid.h>
#include <frmfmt.hxx>
#include <fmtfsize.hxx>
#include <swrect.hxx>
#include "xmlexp.hxx"
#include <editeng/memberids.h>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

class SwXMLTableItemMapper_Impl: public SvXMLExportItemMapper
{
    SwXMLBrushItemExport aBrushItemExport;

protected:

    sal_uInt32 nAbsWidth;

    static void AddAttribute( sal_uInt16 nPrefix, enum XMLTokenEnum eLName,
                       const OUString& rValue,
                       const SvXMLNamespaceMap& rNamespaceMap,
                       SvXMLAttributeList& rAttrList );

public:

    SwXMLTableItemMapper_Impl(
            SvXMLItemMapEntriesRef rMapEntries,
            SwXMLExport& rExp );

    virtual void handleSpecialItem( SvXMLAttributeList& rAttrList,
                                    const SvXMLItemMapEntry& rEntry,
                                    const SfxPoolItem& rItem,
                                    const SvXMLUnitConverter& rUnitConverter,
                                    const SvXMLNamespaceMap& rNamespaceMap,
                                    const SfxItemSet *pSet ) const override;

    virtual void handleElementItem(
            SvXMLExport& rExport,
            const SvXMLItemMapEntry& rEntry,
            const SfxPoolItem& rItem,
            const SvXMLUnitConverter& rUnitConverter,
            const SfxItemSet& rSet,
            SvXmlExportFlags nFlags ) const override;

    inline void SetAbsWidth( sal_uInt32 nAbs );
};

SwXMLTableItemMapper_Impl::SwXMLTableItemMapper_Impl(
        SvXMLItemMapEntriesRef rMapEntries,
        SwXMLExport& rExp ) :
    SvXMLExportItemMapper( std::move(rMapEntries) ),
    aBrushItemExport( rExp ),
    nAbsWidth( USHRT_MAX )
{
}

void SwXMLTableItemMapper_Impl::AddAttribute( sal_uInt16 nPrefix,
        enum XMLTokenEnum eLName,
        const OUString& rValue,
        const SvXMLNamespaceMap& rNamespaceMap,
        SvXMLAttributeList& rAttrList )
{
    OUString sName( rNamespaceMap.GetQNameByKey( nPrefix,
                                                 GetXMLToken(eLName) ) );
    rAttrList.AddAttribute( sName, rValue );
}

void SwXMLTableItemMapper_Impl::handleSpecialItem(
        SvXMLAttributeList& rAttrList,
        const SvXMLItemMapEntry& rEntry,
        const SfxPoolItem& rItem,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap,
        const SfxItemSet *pSet ) const
{
    switch( rEntry.nWhichId )
    {
    case RES_LR_SPACE:
        {
            const SfxPoolItem *pItem;
            if( pSet &&
                SfxItemState::SET == pSet->GetItemState( RES_HORI_ORIENT, true,
                                                    &pItem ) )
            {
                sal_Int16 eHoriOrient =
                    static_cast<const SwFormatHoriOrient *>(pItem)->GetHoriOrient();
                bool bExport = false;
                sal_uInt16 nMemberId =
                    static_cast<sal_uInt16>( rEntry.nMemberId & MID_SW_FLAG_MASK );
                switch( nMemberId )
                {
                case MID_L_MARGIN:
                    bExport = text::HoriOrientation::NONE == eHoriOrient ||
                              text::HoriOrientation::LEFT_AND_WIDTH == eHoriOrient;
                    break;
                case MID_R_MARGIN:
                    bExport = text::HoriOrientation::NONE == eHoriOrient;
                    break;
                }
                OUString sValue;
                if( bExport && SvXMLExportItemMapper::QueryXMLValue(
                    rItem, sValue, nMemberId, rUnitConverter ) )
                {
                    AddAttribute( rEntry.nNameSpace, rEntry.eLocalName, sValue,
                                  rNamespaceMap, rAttrList );
                }
            }
        }
        break;

    case RES_FRM_SIZE:
        {
            sal_uInt16 nMemberId =
                static_cast<sal_uInt16>( rEntry.nMemberId & MID_SW_FLAG_MASK );
            switch( nMemberId )
            {
            case MID_FRMSIZE_WIDTH:
                if( nAbsWidth )
                {
                    OUStringBuffer sBuffer;
                    rUnitConverter.convertMeasureToXML( sBuffer, nAbsWidth );
                    AddAttribute( rEntry.nNameSpace, rEntry.eLocalName,
                                  sBuffer.makeStringAndClear(),
                                  rNamespaceMap, rAttrList );
                }
                break;
            case MID_FRMSIZE_REL_WIDTH:
                {
                    OUString sValue;
                    if( SvXMLExportItemMapper::QueryXMLValue(
                        rItem, sValue, nMemberId, rUnitConverter ) )
                    {
                        AddAttribute( rEntry.nNameSpace, rEntry.eLocalName,
                                      sValue, rNamespaceMap, rAttrList );
                    }
                }
                break;
            }
        }
        break;
    }
}

/** this method is called for every item that has the
    MID_SW_FLAG_ELEMENT_EXPORT flag set */
void SwXMLTableItemMapper_Impl::handleElementItem(
        SvXMLExport& /*rExport*/,
        const SvXMLItemMapEntry& rEntry,
        const SfxPoolItem& rItem,
        const SvXMLUnitConverter& /*rUnitConverter*/,
        const SfxItemSet&,
        SvXmlExportFlags ) const
{
    switch( rEntry.nWhichId )
    {
    case RES_BACKGROUND:
        {
            const_cast<SwXMLTableItemMapper_Impl *>(this)->aBrushItemExport.exportXML(
                                                static_cast<const SvxBrushItem&>(rItem) );
        }
        break;
    }
}

inline void SwXMLTableItemMapper_Impl::SetAbsWidth( sal_uInt32 nAbs )
{
    nAbsWidth = nAbs;
}

void SwXMLExport::InitItemExport()
{
    m_pTwipUnitConverter.reset(new SvXMLUnitConverter(getComponentContext(),
        util::MeasureUnit::TWIP, GetMM100UnitConverter().GetXMLMeasureUnit()));

    m_xTableItemMap = new SvXMLItemMapEntries( aXMLTableItemMap );
    m_xTableRowItemMap = new SvXMLItemMapEntries( aXMLTableRowItemMap );
    m_xTableCellItemMap = new SvXMLItemMapEntries( aXMLTableCellItemMap );

    m_pTableItemMapper.reset(new SwXMLTableItemMapper_Impl( m_xTableItemMap, *this ));
}

void SwXMLExport::FinitItemExport()
{
    m_pTableItemMapper.reset();
    m_pTwipUnitConverter.reset();
}

void SwXMLExport::ExportTableFormat( const SwFrameFormat& rFormat, sal_uInt32 nAbsWidth )
{
    static_cast<SwXMLTableItemMapper_Impl *>(m_pTableItemMapper.get())
        ->SetAbsWidth( nAbsWidth );
    ExportFormat( rFormat, XML_TABLE );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
