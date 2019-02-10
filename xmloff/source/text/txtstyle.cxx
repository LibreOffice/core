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


#include <com/sun/star/style/ParagraphStyleCategory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/families.hxx>
#include <xmloff/txtparae.hxx>
#include <xmloff/xmlnume.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include "XMLSectionExport.hxx"
#include "XMLLineNumberingExport.hxx"
#include "txtexppr.hxx"
#include <xmloff/txtprmap.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

void XMLTextParagraphExport::exportStyleAttributes(
        const css::uno::Reference< css::style::XStyle > & rStyle )
{
    OUString sName;
    Reference< XPropertySet > xPropSet( rStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo(
            xPropSet->getPropertySetInfo());
    if( xPropSetInfo->hasPropertyByName( gsCategory ) )
    {
        sal_Int16 nCategory = 0;
        xPropSet->getPropertyValue( gsCategory ) >>= nCategory;
        enum XMLTokenEnum eValue = XML_TOKEN_INVALID;
        if( -1 != nCategory )
        {
            switch( nCategory )
            {
            case ParagraphStyleCategory::TEXT:
                eValue = XML_TEXT;
                break;
            case ParagraphStyleCategory::CHAPTER:
                eValue = XML_CHAPTER;
                break;
            case ParagraphStyleCategory::LIST:
                eValue = XML_LIST;
                break;
            case ParagraphStyleCategory::INDEX:
                eValue = XML_INDEX;
                break;
            case ParagraphStyleCategory::EXTRA:
                eValue = XML_EXTRA;
                break;
            case ParagraphStyleCategory::HTML:
                eValue = XML_HTML;
                break;
            }
        }
        if( eValue != XML_TOKEN_INVALID )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_CLASS, eValue);
    }
    if( xPropSetInfo->hasPropertyByName( gsPageDescName ) )
    {
        Reference< XPropertyState > xPropState( xPropSet, uno::UNO_QUERY );
        if( PropertyState_DIRECT_VALUE ==
                xPropState->getPropertyState( gsPageDescName  ) )
        {
            xPropSet->getPropertyValue( gsPageDescName ) >>= sName;
            // fix for #i5551#  if( sName.getLength() > 0 )
            GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_MASTER_PAGE_NAME,
                                          GetExport().EncodeStyleName( sName ) );
        }
    }
    if( bProgress )
    {
        ProgressBarHelper *pProgress = GetExport().GetProgressBarHelper();
        pProgress->SetValue( pProgress->GetValue()+2 );
    }
}

void XMLTextParagraphExport::exportNumStyles( bool bUsed )
{
    SvxXMLNumRuleExport aNumRuleExport( GetExport() );
    aNumRuleExport.exportStyles( bUsed, &maListAutoPool, !IsBlockMode() );
}

void XMLTextParagraphExport::exportTextStyles( bool bUsed, bool bProg )
{
    bool bOldProg = bProgress;
    bProgress = bProg;

    Reference < lang::XMultiServiceFactory > xFactory (GetExport().GetModel(), UNO_QUERY);
    if (xFactory.is())
    {
        Reference < XPropertySet > xPropSet (xFactory->createInstance ( "com.sun.star.text.Defaults" ), UNO_QUERY);
        if (xPropSet.is())
        {
            exportDefaultStyle( xPropSet, GetXMLToken(XML_PARAGRAPH), GetParaPropMapper());

            exportDefaultStyle(
                xPropSet,
                GetXMLToken(XML_TABLE),
                new XMLTextExportPropertySetMapper(
                    new XMLTextPropertySetMapper(
                        TextPropMap::TABLE_DEFAULTS, true ),
                    GetExport() ) );

            exportDefaultStyle(
                xPropSet,
                GetXMLToken(XML_TABLE_ROW),
                new XMLTextExportPropertySetMapper(
                    new XMLTextPropertySetMapper(
                        TextPropMap::TABLE_ROW_DEFAULTS, true ),
                    GetExport() ) );
        }
    }
    exportStyleFamily( "ParagraphStyles", GetXMLToken(XML_PARAGRAPH), GetParaPropMapper(),
                       bUsed, XML_STYLE_FAMILY_TEXT_PARAGRAPH);
    exportStyleFamily( "CharacterStyles", GetXMLToken(XML_TEXT), GetTextPropMapper(),
                       bUsed, XML_STYLE_FAMILY_TEXT_TEXT );
    // get shape export to make sure the frame family is added correctly.
    GetExport().GetShapeExport();
    exportStyleFamily( "FrameStyles", OUString(XML_STYLE_FAMILY_SD_GRAPHICS_NAME), xFramePropMapper,
                       bUsed, XML_STYLE_FAMILY_TEXT_FRAME);
    exportNumStyles( bUsed );
    if( !IsBlockMode() )
    {
        exportTextFootnoteConfiguration();
        XMLSectionExport::ExportBibliographyConfiguration(GetExport());
        XMLLineNumberingExport aLineNumberingExport(GetExport());
        aLineNumberingExport.Export();
    }

    bProgress = bOldProg;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
