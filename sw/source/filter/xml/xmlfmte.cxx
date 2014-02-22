/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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


using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::xmloff::token;


void SwXMLExport::ExportFmt( const SwFmt& rFmt, enum XMLTokenEnum eFamily )
{
    
    CheckAttrList();

    
    OSL_ENSURE( RES_FRMFMT==rFmt.Which(), "frame format expected" );
    if( RES_FRMFMT != rFmt.Which() )
        return;
    OSL_ENSURE( eFamily != XML_TOKEN_INVALID, "family must be specified" );
    
    sal_Bool bEncoded = sal_False;
    AddAttribute( XML_NAMESPACE_STYLE, XML_NAME, EncodeStyleName(
                    rFmt.GetName(), &bEncoded ) );
    if( bEncoded )
        AddAttribute( XML_NAMESPACE_STYLE, XML_DISPLAY_NAME, rFmt.GetName() );


    if( eFamily != XML_TOKEN_INVALID )
        AddAttribute( XML_NAMESPACE_STYLE, XML_FAMILY, eFamily );

#if OSL_DEBUG_LEVEL > 0
    
    const SwFmt* pParent = rFmt.DerivedFrom();
    
    OSL_ENSURE( !pParent || pParent->IsDefault(), "unexpected parent" );

    OSL_ENSURE( USHRT_MAX == rFmt.GetPoolFmtId(), "pool ids arent'supported" );
    OSL_ENSURE( USHRT_MAX == rFmt.GetPoolHelpId(), "help ids arent'supported" );
    OSL_ENSURE( USHRT_MAX == rFmt.GetPoolHelpId() ||
            UCHAR_MAX == rFmt.GetPoolHlpFileId(), "help file ids aren't supported" );
#endif

    
    if( RES_FRMFMT == rFmt.Which() && XML_TABLE == eFamily )
    {
        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == rFmt.GetAttrSet().GetItemState( RES_PAGEDESC,
                                                            false, &pItem ) )
        {
            OUString sName;
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
                                            false, &pItem ) )
        {
            sal_Int32 nFormat = (sal_Int32)
                ((const SwTblBoxNumFormat *)pItem)->GetValue();

            if ( (nFormat != -1) && (nFormat != NUMBERFORMAT_TEXT) )
            {
                
                
                
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


void SwXMLExport::_ExportStyles( bool bUsed )
{
    SvXMLExport::_ExportStyles( bUsed );

    
    GetShapeExport()->ExportGraphicDefaults();

    GetTextParagraphExport()->exportTextStyles( bUsed
                                             ,IsShowProgress()
                                              );
    
    GetPageExport()->exportDefaultStyle();
}

void SwXMLExport::_ExportAutoStyles()
{
    
    
    

    if( (getExportFlags() & (EXPORT_MASTERSTYLES|EXPORT_CONTENT)) != 0 )
    {
        if( (getExportFlags() & EXPORT_CONTENT) == 0 )
        {
            
            
            
            
        }
    }

    
    if( (getExportFlags() & EXPORT_MASTERSTYLES) != 0 )
        GetPageExport()->collectAutoStyles( sal_False );

    
    
    
    if( (getExportFlags() & EXPORT_STYLES ) == 0 )
        GetTextParagraphExport()->exportUsedDeclarations( sal_False );

    
    if( (getExportFlags() & EXPORT_CONTENT) != 0 )
    {
        GetTextParagraphExport()->exportTrackedChanges( sal_True );
        Reference < XTextDocument > xTextDoc( GetModel(), UNO_QUERY );
        Reference < XText > xText = xTextDoc->getText();

        
        
        
        Reference<XDrawPageSupplier> xDrawPageSupplier( GetModel(), UNO_QUERY );
        if (xDrawPageSupplier.is() && GetFormExport().is())
        {
            Reference<XDrawPage> xPage = xDrawPageSupplier->getDrawPage();
            if (xPage.is())
                GetFormExport()->examineForms(xPage);
        }

        GetTextParagraphExport()->collectTextAutoStylesOptimized( bShowProgress );
        
    }

    GetTextParagraphExport()->exportTextAutoStyles();
    GetShapeExport()->exportAutoStyles();
    if( (getExportFlags() & EXPORT_MASTERSTYLES) != 0 )
        GetPageExport()->exportAutoStyles();

    
    
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
    
    GetPageExport()->exportMasterStyles( sal_False );
}

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
            if (aProperty->mnIndex != -1) 
            {
                switch( rPropExp.getPropertySetMapper()->
                        GetEntryContextId( aProperty->mnIndex ) )
                {
                case CTF_NUMBERINGSTYLENAME:
                    {
                        OUString sStyleName;
                        aProperty->maValue >>= sStyleName;
                        
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
