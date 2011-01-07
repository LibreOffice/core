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
#include "precompiled_xmloff.hxx"

#include <com/sun/star/style/ParagraphStyleCategory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/families.hxx>
#include <xmloff/txtparae.hxx>
#include <xmloff/xmlnume.hxx>
#include <xmloff/xmlexp.hxx>
#include "XMLSectionExport.hxx"
#include "XMLLineNumberingExport.hxx"
#include "txtexppr.hxx"
#include <xmloff/txtprmap.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::xmloff::token;

void XMLTextParagraphExport::exportStyleAttributes(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle )
{
    OUString sName;
    Any aAny;
    Reference< XPropertySet > xPropSet( rStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo(
            xPropSet->getPropertySetInfo());
    if( xPropSetInfo->hasPropertyByName( sCategory ) )
    {
        sal_Int16 nCategory = 0;
        xPropSet->getPropertyValue( sCategory ) >>= nCategory;
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
    if( xPropSetInfo->hasPropertyByName( sPageDescName ) )
    {
        Reference< XPropertyState > xPropState( xPropSet, uno::UNO_QUERY );
        if( PropertyState_DIRECT_VALUE ==
                xPropState->getPropertyState( sPageDescName  ) )
        {
            xPropSet->getPropertyValue( sPageDescName ) >>= sName;
            // fix for #i5551#  if( sName.getLength() > 0 )
                GetExport().AddAttribute( XML_NAMESPACE_STYLE,
                                          XML_MASTER_PAGE_NAME,
                                          GetExport().EncodeStyleName( sName ) );
        }
    }
    //sal_Int32 nOutlineLevel = //#outline level, zhaojianwei, moved to styleexp.cxx - XMLStyleExport::exportStyle(бнбн)
    //  GetExport().GetTextParagraphExport()->GetHeadingLevel( rStyle->getName() );
    //if( nOutlineLevel != -1 )
    //{
    //  OUStringBuffer sTmp;
    //              sTmp.append( static_cast<sal_Int32>(nOutlineLevel+1L) );
    //  GetExport().AddAttribute( XML_NAMESPACE_STYLE,
    //                      XML_DEFAULT_OUTLINE_LEVEL,
    //                      sTmp.makeStringAndClear() );
    //}

    if( bProgress )
    {
        ProgressBarHelper *pProgress = GetExport().GetProgressBarHelper();
            pProgress->SetValue( pProgress->GetValue()+2 );
    }
}

void XMLTextParagraphExport::exportNumStyles( sal_Bool bUsed )
{
    SvxXMLNumRuleExport aNumRuleExport( GetExport() );
    aNumRuleExport.exportStyles( bUsed, pListAutoPool, !IsBlockMode() );
}

void XMLTextParagraphExport::exportTextStyles( sal_Bool bUsed, sal_Bool bProg )
{
    sal_Bool bOldProg = bProgress;
    bProgress = bProg;

    Reference < lang::XMultiServiceFactory > xFactory (GetExport().GetModel(), UNO_QUERY);
    if (xFactory.is())
    {
        OUString sTextDefaults ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.text.Defaults" ) );
        Reference < XPropertySet > xPropSet (xFactory->createInstance ( sTextDefaults ), UNO_QUERY);
        if (xPropSet.is())
        {
            exportDefaultStyle( xPropSet, GetXMLToken(XML_PARAGRAPH), GetParaPropMapper());

            exportDefaultStyle(
                xPropSet,
                GetXMLToken(XML_TABLE),
                new XMLTextExportPropertySetMapper(
                    new XMLTextPropertySetMapper(
                        TEXT_PROP_MAP_TABLE_DEFAULTS ),
                    GetExport() ) );

            exportDefaultStyle(
                xPropSet,
                GetXMLToken(XML_TABLE_ROW),
                new XMLTextExportPropertySetMapper(
                    new XMLTextPropertySetMapper(
                        TEXT_PROP_MAP_TABLE_ROW_DEFAULTS ),
                    GetExport() ) );
        }
    }
    exportStyleFamily( "ParagraphStyles", GetXMLToken(XML_PARAGRAPH), GetParaPropMapper(),
                       bUsed, XML_STYLE_FAMILY_TEXT_PARAGRAPH, 0);
    exportStyleFamily( "CharacterStyles", GetXMLToken(XML_TEXT), GetTextPropMapper(),
                       bUsed, XML_STYLE_FAMILY_TEXT_TEXT );
    // get shape export to make sure the the frame family is added correctly.
    GetExport().GetShapeExport();
    exportStyleFamily( "FrameStyles", OUString(RTL_CONSTASCII_USTRINGPARAM(XML_STYLE_FAMILY_SD_GRAPHICS_NAME)), GetFramePropMapper(),
                       bUsed, XML_STYLE_FAMILY_TEXT_FRAME, 0);
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
