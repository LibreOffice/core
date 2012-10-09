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

#include <com/sun/star/text/XTextDocument.hpp>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/attrlist.hxx>
#include "xmlexpit.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/XMLTextListAutoStylePool.hxx>
#include <xmloff/XMLTextMasterPageExport.hxx>

#include <xmloff/txtprmap.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/families.hxx>
#include <format.hxx>
#include <fmtpdsc.hxx>
#include <pagedesc.hxx>
#include <unostyle.hxx>
#include <cellatr.hxx>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "xmlexp.hxx"
#include <SwStyleNameMapper.hxx>


using ::rtl::OUString;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::xmloff::token;

using rtl::OUString;

void SwXMLExport::ExportFmt( const SwFmt& rFmt, enum XMLTokenEnum eFamily )
{
    // <style:style ...>
    CheckAttrList();

    // style:family="..."
    OSL_ENSURE( RES_FRMFMT==rFmt.Which(), "frame format expected" );
    if( RES_FRMFMT != rFmt.Which() )
        return;
    OSL_ENSURE( eFamily != XML_TOKEN_INVALID, "family must be specified" );
    // style:name="..."
    sal_Bool bEncoded = sal_False;
    AddAttribute( XML_NAMESPACE_STYLE, XML_NAME, EncodeStyleName(
                    rFmt.GetName(), &bEncoded ) );
    if( bEncoded )
        AddAttribute( XML_NAMESPACE_STYLE, XML_DISPLAY_NAME, rFmt.GetName() );


    if( eFamily != XML_TOKEN_INVALID )
        AddAttribute( XML_NAMESPACE_STYLE, XML_FAMILY, eFamily );

#if OSL_DEBUG_LEVEL > 0
    // style:parent-style-name="..." (if its not the default only)
    const SwFmt* pParent = rFmt.DerivedFrom();
    // Parent-Namen nur uebernehmen, wenn kein Default
    OSL_ENSURE( !pParent || pParent->IsDefault(), "unexpected parent" );

    OSL_ENSURE( USHRT_MAX == rFmt.GetPoolFmtId(), "pool ids arent'supported" );
    OSL_ENSURE( USHRT_MAX == rFmt.GetPoolHelpId(), "help ids arent'supported" );
    OSL_ENSURE( USHRT_MAX == rFmt.GetPoolHelpId() ||
            UCHAR_MAX == rFmt.GetPoolHlpFileId(), "help file ids aren't supported" );
#endif

    // style:master-page-name
    if( RES_FRMFMT == rFmt.Which() && XML_TABLE == eFamily )
    {
        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == rFmt.GetAttrSet().GetItemState( RES_PAGEDESC,
                                                            sal_False, &pItem ) )
        {
            String sName;
            const SwPageDesc *pPageDesc =
                ((const SwFmtPageDesc *)pItem)->GetPageDesc();
            if( pPageDesc )
                SwStyleNameMapper::FillProgName(
                                    pPageDesc->GetName(),
                                    sName,
                                    nsSwGetPoolIdFromName::GET_POOLID_PAGEDESC,
                                    true);
            AddAttribute( XML_NAMESPACE_STYLE, XML_MASTER_PAGE_NAME,
                          EncodeStyleName( sName ) );
        }
    }

    if( XML_TABLE_CELL == eFamily )
    {
        OSL_ENSURE(RES_FRMFMT == rFmt.Which(), "only frame format");

        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET ==
            rFmt.GetAttrSet().GetItemState( RES_BOXATR_FORMAT,
                                            sal_False, &pItem ) )
        {
            sal_Int32 nFormat = (sal_Int32)
                ((const SwTblBoxNumFormat *)pItem)->GetValue();

            if ( (nFormat != -1) && (nFormat != NUMBERFORMAT_TEXT) )
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
                                  sal_True, sal_True );

        SvXMLItemMapEntriesRef xItemMap;
        XMLTokenEnum ePropToken = XML_TABLE_PROPERTIES;
        if( XML_TABLE == eFamily )
        {
            xItemMap = xTableItemMap;
        }
        else if( XML_TABLE_ROW == eFamily )
        {
            xItemMap = xTableRowItemMap;
            ePropToken = XML_TABLE_ROW_PROPERTIES;
        }
        else if( XML_TABLE_CELL == eFamily )
        {
            xItemMap = xTableCellItemMap;
            ePropToken = XML_TABLE_CELL_PROPERTIES;
        }

        if( xItemMap.Is() )
        {
            SvXMLExportItemMapper& rItemMapper = GetTableItemMapper();
            rItemMapper.setMapEntries( xItemMap );

            GetTableItemMapper().exportXML( *this,
                                           rFmt.GetAttrSet(),
                                           GetTwipUnitConverter(),
                                           ePropToken,
                                           XML_EXPORT_FLAG_IGN_WS );
        }
    }
}


void SwXMLExport::_ExportStyles( sal_Bool bUsed )
{
    SvXMLExport::_ExportStyles( bUsed );

    // drawing defaults
    GetShapeExport()->ExportGraphicDefaults();

    GetTextParagraphExport()->exportTextStyles( bUsed
                                             ,IsShowProgress()
                                              );
    //page defaults
    GetPageExport()->exportDefaultStyle();
}

void SwXMLExport::_ExportAutoStyles()
{
    // The order in which styles are collected *MUST* be the same as
    // the order in which they are exported. Otherwise, caching will
    // fail.

    if( (getExportFlags() & (EXPORT_MASTERSTYLES|EXPORT_CONTENT)) != 0 )
    {
        if( (getExportFlags() & EXPORT_CONTENT) == 0 )
        {
            // only master pages are exported => styles for frames bound
            // to frames (but none for frames bound to pages) need to be
            // collected.
            // TODO: exclude PageBoundFrames on export
        }
    }

    // exported in _ExportMasterStyles
    if( (getExportFlags() & EXPORT_MASTERSTYLES) != 0 )
        GetPageExport()->collectAutoStyles( sal_False );

    // if we don't export styles (i.e. in content stream only, but not
    // in single-stream case), then we can save ourselves a bit of
    // work and memory by not collecting field masters
    if( (getExportFlags() & EXPORT_STYLES ) == 0 )
        GetTextParagraphExport()->exportUsedDeclarations( sal_False );

    // exported in _ExportContent
    if( (getExportFlags() & EXPORT_CONTENT) != 0 )
    {
        GetTextParagraphExport()->exportTrackedChanges( sal_True );
        Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
        Reference < XText > xText = xTextDoc->getText();

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

        GetTextParagraphExport()->collectTextAutoStylesOptimized( bShowProgress );
        //GetTextParagraphExport()->collectTextAutoStyles( xText, bShowProgress, sal_True, bPortions );
    }

    GetTextParagraphExport()->exportTextAutoStyles();
    GetShapeExport()->exportAutoStyles();
    if( (getExportFlags() & EXPORT_MASTERSTYLES) != 0 )
        GetPageExport()->exportAutoStyles();

    // we rely on data styles being written after cell styles in the
    // ExportFmt() method; so be careful when changing order.
    exportAutoDataStyles();

    sal_uInt16 nContentAutostyles = EXPORT_CONTENT | EXPORT_AUTOSTYLES;
    if ( ( getExportFlags() & nContentAutostyles ) == nContentAutostyles )
        GetFormExport()->exportAutoStyles();
}

XMLPageExport* SwXMLExport::CreatePageExport()
{
    return new XMLTextMasterPageExport( *this );
}

void SwXMLExport::_ExportMasterStyles()
{
    // export master styles
    GetPageExport()->exportMasterStyles( sal_False );
}

// ---------------------------------------------------------------------
class SwXMLAutoStylePoolP : public SvXMLAutoStylePoolP
{
    SvXMLExport& rExport;
    const OUString sListStyleName;
    const OUString sMasterPageName;
    const OUString sCDATA;

protected:

    virtual void exportStyleAttributes(
            SvXMLAttributeList& rAttrList,
            sal_Int32 nFamily,
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp
            , const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap
            ) const;
public:

    SwXMLAutoStylePoolP( SvXMLExport& rExport );
    virtual ~SwXMLAutoStylePoolP();
};

void SwXMLAutoStylePoolP::exportStyleAttributes(
            SvXMLAttributeList& rAttrList,
            sal_Int32 nFamily,
            const ::std::vector< XMLPropertyState >& rProperties,
            const SvXMLExportPropertyMapper& rPropExp
            , const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap
            ) const
{
    SvXMLAutoStylePoolP::exportStyleAttributes( rAttrList, nFamily, rProperties, rPropExp, rUnitConverter, rNamespaceMap);

    if( XML_STYLE_FAMILY_TEXT_PARAGRAPH == nFamily )
    {
        for( ::std::vector< XMLPropertyState >::const_iterator
                    aProperty = rProperties.begin();
             aProperty != rProperties.end();
              ++aProperty )
        {
            if (aProperty->mnIndex != -1) // #i26762#
            {
                switch( rPropExp.getPropertySetMapper()->
                        GetEntryContextId( aProperty->mnIndex ) )
                {
                case CTF_NUMBERINGSTYLENAME:
                    {
                        OUString sStyleName;
                        aProperty->maValue >>= sStyleName;
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
                        aProperty->maValue >>= sStyleName;
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
    sMasterPageName( GetXMLToken( XML_MASTER_PAGE_NAME ) ),
    sCDATA( GetXMLToken( XML_CDATA ) )
{
}


SwXMLAutoStylePoolP::~SwXMLAutoStylePoolP()
{
}

SvXMLAutoStylePoolP* SwXMLExport::CreateAutoStylePool()
{
    return new SwXMLAutoStylePoolP( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
