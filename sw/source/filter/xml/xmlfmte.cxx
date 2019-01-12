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

#include <com/sun/star/text/XTextDocument.hpp>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/attrlist.hxx>
#include "xmlexpit.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/XMLTextListAutoStylePool.hxx>
#include <xmloff/XMLTextMasterPageExport.hxx>
#include <xmloff/table/XMLTableExport.hxx>

#include <xmloff/txtprmap.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/families.hxx>
#include <xmloff/maptype.hxx>
#include <format.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <unostyle.hxx>
#include <cellatr.hxx>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include "xmlexp.hxx"
#include <SwStyleNameMapper.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::xmloff::token;

void SwXMLExport::ExportFormat( const SwFormat& rFormat, enum XMLTokenEnum eFamily )
{
    // <style:style ...>
    CheckAttrList();

    // style:family="..."
    OSL_ENSURE( RES_FRMFMT==rFormat.Which(), "frame format expected" );
    if( RES_FRMFMT != rFormat.Which() )
        return;
    OSL_ENSURE( eFamily != XML_TOKEN_INVALID, "family must be specified" );
    // style:name="..."
    bool bEncoded = false;
    AddAttribute( XML_NAMESPACE_STYLE, XML_NAME, EncodeStyleName(
                    rFormat.GetName(), &bEncoded ) );
    if( bEncoded )
        AddAttribute( XML_NAMESPACE_STYLE, XML_DISPLAY_NAME, rFormat.GetName() );

    if( eFamily != XML_TOKEN_INVALID )
        AddAttribute( XML_NAMESPACE_STYLE, XML_FAMILY, eFamily );

#if OSL_DEBUG_LEVEL > 0
    // style:parent-style-name="..." (if it's not the default only)
    const SwFormat* pParent = rFormat.DerivedFrom();
    // Only adopt parent name, if it's not the default
    OSL_ENSURE( !pParent || pParent->IsDefault(), "unexpected parent" );

    OSL_ENSURE( USHRT_MAX == rFormat.GetPoolFormatId(), "pool ids aren't supported" );
    OSL_ENSURE( USHRT_MAX == rFormat.GetPoolHelpId(), "help ids aren't supported" );
    OSL_ENSURE( USHRT_MAX == rFormat.GetPoolHelpId() ||
            UCHAR_MAX == rFormat.GetPoolHlpFileId(), "help file ids aren't supported" );
#endif

    // style:master-page-name
    if( RES_FRMFMT == rFormat.Which() && XML_TABLE == eFamily )
    {
        const SfxPoolItem *pItem;
        if( SfxItemState::SET == rFormat.GetAttrSet().GetItemState( RES_PAGEDESC,
                                                            false, &pItem ) )
        {
            OUString sName;
            const SwPageDesc *pPageDesc =
                static_cast<const SwFormatPageDesc *>(pItem)->GetPageDesc();
            if( pPageDesc )
                SwStyleNameMapper::FillProgName(
                                    pPageDesc->GetName(),
                                    sName,
                                    SwGetPoolIdFromName::PageDesc);
            AddAttribute( XML_NAMESPACE_STYLE, XML_MASTER_PAGE_NAME,
                          EncodeStyleName( sName ) );
        }
    }

    if( XML_TABLE_CELL == eFamily )
    {
        OSL_ENSURE(RES_FRMFMT == rFormat.Which(), "only frame format");

        const SfxPoolItem *pItem;
        if( SfxItemState::SET ==
            rFormat.GetAttrSet().GetItemState( RES_BOXATR_FORMAT,
                                            false, &pItem ) )
        {
            sal_Int32 nFormat = static_cast<sal_Int32>(static_cast<const SwTableBoxNumFormat *>(pItem)->GetValue());

            if ( (nFormat != -1) && (nFormat != static_cast<sal_Int32>(getSwDefaultTextFormat())) )
            {
                // if we have a format, register and then export
                // (Careful: here we assume that data styles will be
                // written after cell styles)
                addDataStyle(nFormat);
                OUString sDataStyleName = getDataStyleName(nFormat);
                if( !sDataStyleName.isEmpty() )
                    AddAttribute( XML_NAMESPACE_STYLE, XML_DATA_STYLE_NAME,
                                  sDataStyleName );
            }
        }
    }

    {
        SvXMLElementExport aElem( *this, XML_NAMESPACE_STYLE, XML_STYLE,
                                  true, true );

        SvXMLItemMapEntriesRef xItemMap;
        XMLTokenEnum ePropToken = XML_TABLE_PROPERTIES;
        if( XML_TABLE == eFamily )
        {
            xItemMap = m_xTableItemMap;
        }
        else if( XML_TABLE_ROW == eFamily )
        {
            xItemMap = m_xTableRowItemMap;
            ePropToken = XML_TABLE_ROW_PROPERTIES;
        }
        else if( XML_TABLE_CELL == eFamily )
        {
            xItemMap = m_xTableCellItemMap;
            ePropToken = XML_TABLE_CELL_PROPERTIES;
        }

        if( xItemMap.is() )
        {
            m_pTableItemMapper->setMapEntries( xItemMap );
            m_pTableItemMapper->exportXML( *this,
                                           rFormat.GetAttrSet(),
                                           GetTwipUnitConverter(),
                                           ePropToken );
        }
    }
}

void SwXMLExport::ExportStyles_( bool bUsed )
{
    SvXMLExport::ExportStyles_( bUsed );

    // drawing defaults
    GetShapeExport()->ExportGraphicDefaults();

    GetTextParagraphExport()->exportTextStyles( bUsed
                                             ,IsShowProgress()
                                              );
    GetShapeExport()->GetShapeTableExport()->exportTableStyles();
    //page defaults
    GetPageExport()->exportDefaultStyle();
}

void SwXMLExport::collectAutoStyles()
{
    SvXMLExport::collectAutoStyles();

    if (mbAutoStylesCollected)
        return;

    // The order in which styles are collected *MUST* be the same as
    // the order in which they are exported. Otherwise, caching will
    // fail.
    if( getExportFlags() & (SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::CONTENT) )
    {
        if( !(getExportFlags() & SvXMLExportFlags::CONTENT) )
        {
            // only master pages are exported => styles for frames bound
            // to frames (but none for frames bound to pages) need to be
            // collected.
            // TODO: exclude PageBoundFrames on export
        }
    }

    // exported in _ExportMasterStyles
    if( getExportFlags() & SvXMLExportFlags::MASTERSTYLES )
        GetPageExport()->collectAutoStyles( false );


    // exported in ExportContent_
    if( getExportFlags() & SvXMLExportFlags::CONTENT )
    {
        // collect form autostyle
        // (do this before collectTextAutoStyles, 'cause the shapes need the results of the work
        // done by examineForms)
        Reference<XDrawPageSupplier> xDrawPageSupplier( GetModel(), UNO_QUERY );
        if (xDrawPageSupplier.is() && GetFormExport().is())
        {
            Reference<XDrawPage> xPage = xDrawPageSupplier->getDrawPage();
            if (xPage.is())
                GetFormExport()->examineForms(xPage);
        }

        GetTextParagraphExport()->collectTextAutoStylesOptimized( m_bShowProgress );
    }

    mbAutoStylesCollected = true;
}

void SwXMLExport::ExportAutoStyles_()
{
    collectAutoStyles();

    // if we don't export styles (i.e. in content stream only, but not
    // in single-stream case), then we can save ourselves a bit of
    // work and memory by not collecting field masters
    if( !(getExportFlags() & SvXMLExportFlags::STYLES) )
        GetTextParagraphExport()->exportUsedDeclarations();

    // exported in ExportContent_
    if( getExportFlags() & SvXMLExportFlags::CONTENT )
    {
        GetTextParagraphExport()->exportTrackedChanges( true );
    }

    GetTextParagraphExport()->exportTextAutoStyles();
    GetShapeExport()->exportAutoStyles();
    if( getExportFlags() & SvXMLExportFlags::MASTERSTYLES )
        GetPageExport()->exportAutoStyles();

    // we rely on data styles being written after cell styles in the
    // ExportFormat() method; so be careful when changing order.
    exportAutoDataStyles();

    SvXMLExportFlags nContentAutostyles = SvXMLExportFlags::CONTENT | SvXMLExportFlags::AUTOSTYLES;
    if ( ( getExportFlags() & nContentAutostyles ) == nContentAutostyles )
        GetFormExport()->exportAutoStyles();
}

XMLPageExport* SwXMLExport::CreatePageExport()
{
    return new XMLTextMasterPageExport( *this );
}

void SwXMLExport::ExportMasterStyles_()
{
    // export master styles
    GetPageExport()->exportMasterStyles( false );
}

class SwXMLAutoStylePoolP : public SvXMLAutoStylePoolP
{
    SvXMLExport& rExport;
    const OUString sListStyleName;
    const OUString sMasterPageName;

protected:

    virtual void exportStyleAttributes(
            SvXMLAttributeList& rAttrList,
            sal_Int32 nFamily,
            const std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp
            , const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap
            ) const override;
public:

    explicit SwXMLAutoStylePoolP( SvXMLExport& rExport );
};

void SwXMLAutoStylePoolP::exportStyleAttributes(
            SvXMLAttributeList& rAttrList,
            sal_Int32 nFamily,
            const std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp
            , const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap
            ) const
{
    SvXMLAutoStylePoolP::exportStyleAttributes( rAttrList, nFamily, rProperties, rPropExp, rUnitConverter, rNamespaceMap);

    if( XML_STYLE_FAMILY_TEXT_PARAGRAPH == nFamily )
    {
        for( const auto& rProperty : rProperties )
        {
            if (rProperty.mnIndex != -1) // #i26762#
            {
                switch( rPropExp.getPropertySetMapper()->
                        GetEntryContextId( rProperty.mnIndex ) )
                {
                case CTF_NUMBERINGSTYLENAME:
                    {
                        OUString sStyleName;
                        rProperty.maValue >>= sStyleName;
                        // #i70748# - export also empty list styles
                        if( !sStyleName.isEmpty() )
                        {
                            OUString sTmp = rExport.GetTextParagraphExport()->GetListAutoStylePool().Find( sStyleName );
                            if( !sTmp.isEmpty() )
                                sStyleName = sTmp;
                        }
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                              sListStyleName,
                              GetExport().EncodeStyleName( sStyleName ) );
                    }
                    break;
                case CTF_PAGEDESCNAME:
                    {
                        OUString sStyleName;
                        rProperty.maValue >>= sStyleName;
                        GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                      sMasterPageName,
                                      GetExport().EncodeStyleName( sStyleName ) );
                    }
                    break;
                }
            }
        }
    }
}

SwXMLAutoStylePoolP::SwXMLAutoStylePoolP(SvXMLExport& rExp ) :
    SvXMLAutoStylePoolP( rExp ),
    rExport( rExp ),
    sListStyleName( GetXMLToken( XML_LIST_STYLE_NAME ) ),
    sMasterPageName( GetXMLToken( XML_MASTER_PAGE_NAME ) )
{
}

SvXMLAutoStylePoolP* SwXMLExport::CreateAutoStylePool()
{
    return new SwXMLAutoStylePoolP( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
