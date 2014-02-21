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
#include "PageBordersHandler.hxx"

#include <resourcemodel/ResourceModelHelper.hxx>
#include <SdtHelper.hxx>
#include <TDefTableHandler.hxx>
#include <DomainMapper_Impl.hxx>
#include <ConversionHelper.hxx>
#include <ModelEventListener.hxx>
#include <MeasureHandler.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nutil/paper.hxx>
#include <ooxml/OOXMLFastTokens.hxx>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XOOXMLDocumentPropertiesImporter.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XLineNumberingProperties.hpp>
#include <com/sun/star/awt/FontRelief.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/text/FootnoteNumbering.hpp>
#include <com/sun/star/text/TextGridMode.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/types.hxx>
#include <comphelper/storagehelper.hxx>
#include <filter/msfilter/util.hxx>

#include <TextEffectsHandler.hxx>
#include <CellColorHandler.hxx>
#include <SectionColumnHandler.hxx>
#include <GraphicHelpers.hxx>

using namespace ::com::sun::star;
using namespace ::rtl;

namespace writerfilter {

using resourcemodel::resolveSprmProps;

namespace dmapper{

TagLogger::Pointer_t dmapper_logger(TagLogger::getInstance("DOMAINMAPPER"));

struct _PageSz
{
    sal_Int32 code;
    sal_Int32 h;
    bool      orient;
    sal_Int32 w;
} CT_PageSz;


DomainMapper::DomainMapper( const uno::Reference< uno::XComponentContext >& xContext,
                            uno::Reference< io::XInputStream > xInputStream,
                            uno::Reference< lang::XComponent > xModel,
                            bool bRepairStorage,
                            SourceDocumentType eDocumentType,
                            uno::Reference< text::XTextRange > xInsertTextRange,
                            bool bIsNewDoc ) :
LoggedProperties(dmapper_logger, "DomainMapper"),
LoggedTable(dmapper_logger, "DomainMapper"),
LoggedStream(dmapper_logger, "DomainMapper"),
    m_pImpl( new DomainMapper_Impl( *this, xContext, xModel, eDocumentType, xInsertTextRange, bIsNewDoc )),
    mnBackgroundColor(0), mbIsHighlightSet(false), mbIsSplitPara(false)
{
    // #i24363# tab stops relative to indent
    m_pImpl->SetDocumentSettingsProperty(
        PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_TABS_RELATIVE_TO_INDENT ),
        uno::makeAny( false ) );
    m_pImpl->SetDocumentSettingsProperty(
        PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_SURROUND_TEXT_WRAP_SMALL ),
        uno::makeAny( true ) );

    //import document properties
    try
    {
        uno::Reference< embed::XStorage > xDocumentStorage =
            (comphelper::OStorageHelper::GetStorageOfFormatFromInputStream(OFOPXML_STORAGE_FORMAT_STRING, xInputStream, xContext, bRepairStorage ));

        uno::Reference< uno::XInterface > xTemp = xContext->getServiceManager()->createInstanceWithContext(
                                "com.sun.star.document.OOXMLDocumentPropertiesImporter",
                                xContext);

        uno::Reference< document::XOOXMLDocumentPropertiesImporter > xImporter( xTemp, uno::UNO_QUERY_THROW );
        uno::Reference< document::XDocumentPropertiesSupplier > xPropSupplier( xModel, uno::UNO_QUERY_THROW);
        xImporter->importProperties( xDocumentStorage, xPropSupplier->getDocumentProperties() );
    }
    catch( const uno::Exception& rEx )
    {
        (void)rEx;
    }
}

DomainMapper::~DomainMapper()
{
    try
    {
        uno::Reference< text::XDocumentIndexesSupplier> xIndexesSupplier( m_pImpl->GetTextDocument(), uno::UNO_QUERY );
        sal_Int32 nIndexes = 0;
        if( xIndexesSupplier.is() )
        {
            uno::Reference< container::XIndexAccess > xIndexes = xIndexesSupplier->getDocumentIndexes();
            nIndexes = xIndexes->getCount();
        }
        // If we have page references, those need updating as well, similar to the indexes.
        uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(m_pImpl->GetTextDocument(), uno::UNO_QUERY);
        if(xTextFieldsSupplier.is())
        {
            uno::Reference<container::XEnumeration> xEnumeration = xTextFieldsSupplier->getTextFields()->createEnumeration();
            while(xEnumeration->hasMoreElements())
            {
                ++nIndexes;
                xEnumeration->nextElement();
            }
        }
        if( nIndexes || m_pImpl->m_pSdtHelper->hasElements())
        {
            //index update has to wait until first view is created
            uno::Reference< document::XEventBroadcaster > xBroadcaster(xIndexesSupplier, uno::UNO_QUERY);
            if (xBroadcaster.is())
                xBroadcaster->addEventListener(uno::Reference< document::XEventListener >(new ModelEventListener(nIndexes, m_pImpl->m_pSdtHelper->hasElements())));
        }


        // Apply the document settings after everything else
        m_pImpl->GetSettingsTable()->ApplyProperties( m_pImpl->GetTextDocument( ) );
    }
    catch( const uno::Exception& rEx )
    {
        (void)rEx;
    }

    delete m_pImpl;
}

void DomainMapper::lcl_attribute(Id nName, Value & val)
{
    static OUString sLocalBookmarkName;
    sal_Int32 nIntValue = val.getInt();
    OUString sStringValue = val.getString();

    SectionPropertyMap * pSectionContext = m_pImpl->GetSectionContext();
    PropertyMap::iterator oldPropValue;
        switch( nName )
        {
        case NS_ooxml::LN_CT_Lvl_start:
            break;
        case NS_ooxml::LN_CT_Lvl_numFmt:
            break;
        case NS_ooxml::LN_CT_Lvl_isLgl:
            break;
        case NS_ooxml::LN_CT_Lvl_legacy:
            break;
        case NS_ooxml::LN_CT_AbstractNum_nsid:
            break;
        case NS_ooxml::LN_CT_AbstractNum_tmpl:
            break;
        case NS_ooxml::LN_CT_Border_sz:
            break;
        case NS_ooxml::LN_CT_Border_val:
            break;
        case NS_ooxml::LN_CT_Border_space:
            break;
        case NS_ooxml::LN_CT_Border_shadow:
            break;
        case NS_ooxml::LN_CT_Border_frame:
            break;
        case NS_ooxml::LN_headerr:
            break;
        case NS_ooxml::LN_footerr:
            break;
        case NS_ooxml::LN_endnote:
            break;
        case NS_ooxml::LN_CT_Bookmark_name:
            // sStringValue contains the bookmark name
            sLocalBookmarkName = sStringValue;
        break;
        case NS_ooxml::LN_CT_MarkupRangeBookmark_id:
            //contains the bookmark identifier - has to be added to the bookmark name imported before
            //if it is already available then the bookmark should be inserted
            m_pImpl->AddBookmark( sLocalBookmarkName, sStringValue );
            sLocalBookmarkName = OUString();
        break;
        case NS_ooxml::LN_CT_MarkupRange_displacedByCustomXml:
            break;
        case NS_ooxml::LN_NUMBERING:
            break;
        case NS_ooxml::LN_FONTTABLE:
            break;
        case NS_ooxml::LN_STYLESHEET:
            break;

        case NS_ooxml::LN_CT_Sym_char:
        if( m_pImpl->GetTopContext() && m_pImpl->GetTopContext()->GetFootnote().is())
        {
            m_pImpl->GetTopContext()->GetFootnote()->setLabel(OUString( sal_Unicode(nIntValue)));
            break;
        }
        else //it's a _real_ symbol
        {
            utext( reinterpret_cast < const sal_uInt8 * >( &nIntValue ), 1 );
        }
        break;
        case NS_ooxml::LN_CT_Sym_font:
            //the footnote symbol and font are provided after the footnote is already inserted
        if( m_pImpl->GetTopContext() && m_pImpl->GetTopContext()->GetFootnote().is())
        {
            uno::Reference< beans::XPropertySet > xAnchorProps( m_pImpl->GetTopContext()->GetFootnote()->getAnchor(), uno::UNO_QUERY );
            xAnchorProps->setPropertyValue(
                PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_CHAR_FONT_NAME),
                uno::makeAny( sStringValue ));
        }
        else //a real symbol
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME, uno::makeAny( sStringValue ));
        break;
        case NS_ooxml::LN_CT_Underline_val:
            handleUnderlineType(nIntValue, m_pImpl->GetTopContext());
            break;
        case NS_ooxml::LN_CT_Color_val:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COLOR, uno::makeAny( nIntValue ) );
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "val", OStringToOUString(msfilter::util::ConvertColor(nIntValue, /*bAutoColor=*/true), RTL_TEXTENCODING_UTF8));
            break;
        case NS_ooxml::LN_CT_Underline_color:
            if (m_pImpl->GetTopContext())
            {
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_UNDERLINE_HAS_COLOR, uno::makeAny( true ) );
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_UNDERLINE_COLOR, uno::makeAny( nIntValue ) );
            }
            break;

        case NS_ooxml::LN_CT_TabStop_val:
            if (sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_ST_TabJc_clear)
            {
                m_pImpl->m_aCurrentTabStop.bDeleted = true;
            }
            else
            {
                m_pImpl->m_aCurrentTabStop.bDeleted = false;
                m_pImpl->m_aCurrentTabStop.Alignment = getTabAlignFromValue(nIntValue);
            }
            break;
        case NS_ooxml::LN_CT_TabStop_leader:
            m_pImpl->m_aCurrentTabStop.FillChar = getFillCharFromValue(nIntValue);
            break;
        case NS_ooxml::LN_CT_TabStop_pos:
            m_pImpl->m_aCurrentTabStop.Position = ConversionHelper::convertTwipToMM100(nIntValue);
            break;

        case NS_ooxml::LN_CT_Fonts_ascii:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME, uno::makeAny( sStringValue ));
            break;
        case NS_ooxml::LN_CT_Fonts_asciiTheme:
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "asciiTheme", ThemeTable::getStringForTheme(nIntValue));
            if (m_pImpl->GetTopContext())
            {
                uno::Any aPropValue = uno::makeAny( m_pImpl->GetThemeTable()->getFontNameForTheme( nIntValue ) );
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME, aPropValue );
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_THEME_FONT_NAME_ASCII, aPropValue, true, CHAR_GRAB_BAG );
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_THEME_NAME_ASCII, uno::makeAny( ThemeTable::getStringForTheme(nIntValue) ), true, CHAR_GRAB_BAG);
            }
            break;
        case NS_ooxml::LN_CT_Fonts_hAnsi:
            break;//unsupported
        case NS_ooxml::LN_CT_Fonts_hAnsiTheme:
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "hAnsiTheme", ThemeTable::getStringForTheme(nIntValue));
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_THEME_NAME_H_ANSI, uno::makeAny( ThemeTable::getStringForTheme(nIntValue) ), true, CHAR_GRAB_BAG);
            break;
        case NS_ooxml::LN_CT_Fonts_eastAsia:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME_ASIAN, uno::makeAny( sStringValue ));
            break;
        case NS_ooxml::LN_CT_Fonts_eastAsiaTheme:
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "eastAsiaTheme", ThemeTable::getStringForTheme(nIntValue));
            if (m_pImpl->GetTopContext())
            {
                uno::Any aPropValue = uno::makeAny( m_pImpl->GetThemeTable()->getFontNameForTheme( nIntValue ) );
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME_ASIAN, aPropValue );
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_THEME_FONT_NAME_EAST_ASIA, aPropValue, true, CHAR_GRAB_BAG );
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_THEME_NAME_EAST_ASIA, uno::makeAny( ThemeTable::getStringForTheme(nIntValue) ), true, CHAR_GRAB_BAG);
            }
            break;
        case NS_ooxml::LN_CT_Fonts_cs:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME_COMPLEX, uno::makeAny( sStringValue ));
            break;
        case NS_ooxml::LN_CT_Fonts_cstheme:
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "cstheme", ThemeTable::getStringForTheme(nIntValue));
            if (m_pImpl->GetTopContext())
            {
                uno::Any aPropValue = uno::makeAny( m_pImpl->GetThemeTable()->getFontNameForTheme( nIntValue ) );
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME_COMPLEX, aPropValue );
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_THEME_FONT_NAME_CS, aPropValue, true, CHAR_GRAB_BAG );
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_THEME_NAME_CS, uno::makeAny( ThemeTable::getStringForTheme(nIntValue) ), true, CHAR_GRAB_BAG);
            }
        break;
        case NS_ooxml::LN_CT_Spacing_before:
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "before", OUString::number(nIntValue));
            if (m_pImpl->GetTopContext())
                // Don't overwrite NS_ooxml::LN_CT_Spacing_beforeAutospacing.
                m_pImpl->GetTopContext()->Insert(PROP_PARA_TOP_MARGIN, uno::makeAny( ConversionHelper::convertTwipToMM100( nIntValue ) ), false);
            break;
        case NS_ooxml::LN_CT_Spacing_beforeLines:
                m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "beforeLines", OUString::number(nIntValue));
            break;
        case NS_ooxml::LN_CT_Spacing_after:
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "after", OUString::number(nIntValue));
            if (m_pImpl->GetTopContext())
                // Don't overwrite NS_ooxml::LN_CT_Spacing_afterAutospacing.
                m_pImpl->GetTopContext()->Insert(PROP_PARA_BOTTOM_MARGIN, uno::makeAny( ConversionHelper::convertTwipToMM100( nIntValue ) ), false);
            break;
        case NS_ooxml::LN_CT_Spacing_afterLines:
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "afterLines", OUString::number(nIntValue));
            break;
        case NS_ooxml::LN_CT_Spacing_line: //91434
        case NS_ooxml::LN_CT_Spacing_lineRule: //91435
        {
#define SINGLE_LINE_SPACING 240
            style::LineSpacing aSpacing;
            PropertyMapPtr pTopContext = m_pImpl->GetTopContext();
            bool bFound = false;
            PropertyMap::iterator aLineSpacingIter;
            if (pTopContext)
            {
                aLineSpacingIter = pTopContext->find(PROP_PARA_LINE_SPACING);
                bFound = aLineSpacingIter != pTopContext->end();
            }
            if (bFound)
            {
                aLineSpacingIter->second.getValue() >>= aSpacing;
            }
            else
            {
                //default to single line spacing
                aSpacing.Mode = style::LineSpacingMode::FIX;
                aSpacing.Height = sal_Int16(ConversionHelper::convertTwipToMM100( SINGLE_LINE_SPACING ));
            }
            if( nName == NS_ooxml::LN_CT_Spacing_line )
            {
                m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "line", OUString::number(nIntValue));
                //now set the value depending on the Mode
                if( aSpacing.Mode == style::LineSpacingMode::PROP )
                    aSpacing.Height = sal_Int16(sal_Int32(nIntValue) * 100 / SINGLE_LINE_SPACING );
                else
                    aSpacing.Height = sal_Int16(ConversionHelper::convertTwipToMM100( nIntValue ));
            }
            else //NS_ooxml::LN_CT_Spacing_lineRule:
            {
                    // exactly, atLeast, auto
                    if( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_wordprocessingml_ST_LineSpacingRule_auto)
                    {
                        m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "lineRule", "auto");
                        aSpacing.Mode = style::LineSpacingMode::PROP;
                        //reinterpret the already set value
                        aSpacing.Height = sal_Int16( aSpacing.Height * 100 /  ConversionHelper::convertTwipToMM100( SINGLE_LINE_SPACING ));
                    }
                    else if( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_wordprocessingml_ST_LineSpacingRule_atLeast)
                    {
                        m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "lineRule", "atLeast");
                        aSpacing.Mode = style::LineSpacingMode::MINIMUM;
                    }
                    else // NS_ooxml::LN_Value_wordprocessingml_ST_LineSpacingRule_exact
                    {
                        m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "lineRule", "exact");
                        aSpacing.Mode = style::LineSpacingMode::FIX;
                    }
            }
            if (pTopContext)
                pTopContext->Insert(PROP_PARA_LINE_SPACING, uno::makeAny( aSpacing ));
        }
        break;
        case NS_ooxml::LN_CT_Ind_start:
        case NS_ooxml::LN_CT_Ind_left:
            if (m_pImpl->GetTopContext())
            {
                // Word inherits FirstLineIndent property of the numbering, even if ParaLeftMargin is set, Writer does not.
                // So copy it explicitly, if necessary.
                sal_Int32 nFirstLineIndent = m_pImpl->getCurrentNumberingProperty("FirstLineIndent");

                if (nFirstLineIndent != 0)
                    m_pImpl->GetTopContext()->Insert(PROP_PARA_FIRST_LINE_INDENT, uno::makeAny(nFirstLineIndent));

                m_pImpl->GetTopContext()->Insert(
                    PROP_PARA_LEFT_MARGIN, uno::makeAny( ConversionHelper::convertTwipToMM100(nIntValue ) ));
            }
            break;
        case NS_ooxml::LN_CT_Ind_end:
        case NS_ooxml::LN_CT_Ind_right:
            if (m_pImpl->GetTopContext())
            {
                // Word inherits FirstLineIndent/ParaLeftMargin property of the numbering, even if ParaRightMargin is set, Writer does not.
                // So copy it explicitly, if necessary.
                sal_Int32 nFirstLineIndent = m_pImpl->getCurrentNumberingProperty("FirstLineIndent");
                sal_Int32 nParaLeftMargin = m_pImpl->getCurrentNumberingProperty("IndentAt");

                if (nFirstLineIndent != 0)
                    m_pImpl->GetTopContext()->Insert(PROP_PARA_FIRST_LINE_INDENT, uno::makeAny(nFirstLineIndent));
                if (nParaLeftMargin != 0)
                    m_pImpl->GetTopContext()->Insert(PROP_PARA_LEFT_MARGIN, uno::makeAny(nParaLeftMargin));

                m_pImpl->GetTopContext()->Insert(
                    PROP_PARA_RIGHT_MARGIN, uno::makeAny( ConversionHelper::convertTwipToMM100(nIntValue ) ));
            }
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "right", OUString::number(nIntValue));
            break;
        case NS_ooxml::LN_CT_Ind_hanging:
            if (m_pImpl->GetTopContext())
            {
                sal_Int32 nValue = ConversionHelper::convertTwipToMM100( nIntValue );
                m_pImpl->GetTopContext()->Insert(
                    PROP_PARA_FIRST_LINE_INDENT, uno::makeAny( - nValue ));
            }
            break;
        case NS_ooxml::LN_CT_Ind_firstLine:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(
                    PROP_PARA_FIRST_LINE_INDENT, uno::makeAny( ConversionHelper::convertTwipToMM100(nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Ind_rightChars:
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "rightChars", OUString::number(nIntValue));
            break;

        case NS_ooxml::LN_CT_EastAsianLayout_id:
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_combine:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_IS_ON, uno::makeAny ( nIntValue ? true : false ));
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_combineBrackets:
            if (m_pImpl->GetTopContext())
            {
                OUString sCombinePrefix = getBracketStringFromEnum(nIntValue);
                OUString sCombineSuffix = getBracketStringFromEnum(nIntValue, false);
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_PREFIX, uno::makeAny ( sCombinePrefix ));
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_SUFFIX, uno::makeAny ( sCombineSuffix ));
            }
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_vert:
            if (m_pImpl->GetTopContext())
            {
                sal_Int16 nRotationAngle = (nIntValue ? 900 : 0);
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_ROTATION, uno::makeAny ( nRotationAngle ));
            }
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_vertCompress:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_ROTATION_IS_FIT_TO_LINE, uno::makeAny ( nIntValue ? true : false));
            break;

        case NS_ooxml::LN_CT_PageSz_code:
            CT_PageSz.code = nIntValue;
            break;
        case NS_ooxml::LN_CT_PageSz_h:
            {
                sal_Int32 nHeight = ConversionHelper::convertTwipToMM100(nIntValue);
                CT_PageSz.h = PaperInfo::sloppyFitPageDimension(nHeight);
            }
            break;
        case NS_ooxml::LN_CT_PageSz_orient:
            CT_PageSz.orient = (nIntValue != 0);
            break;
        case NS_ooxml::LN_CT_PageSz_w:
            {
                sal_Int32 nWidth = ConversionHelper::convertTwipToMM100(nIntValue);
                CT_PageSz.w = PaperInfo::sloppyFitPageDimension(nWidth);
            }
            break;

        case NS_ooxml::LN_CT_PageMar_top:
            m_pImpl->SetPageMarginTwip( PAGE_MAR_TOP, nIntValue );
        break;
        case NS_ooxml::LN_CT_PageMar_right:
            m_pImpl->SetPageMarginTwip( PAGE_MAR_RIGHT, nIntValue );
        break;
        case NS_ooxml::LN_CT_PageMar_bottom:
            m_pImpl->SetPageMarginTwip( PAGE_MAR_BOTTOM, nIntValue );
        break;
        case NS_ooxml::LN_CT_PageMar_left:
            m_pImpl->SetPageMarginTwip( PAGE_MAR_LEFT, nIntValue );
        break;
        case NS_ooxml::LN_CT_PageMar_header:
            m_pImpl->SetPageMarginTwip( PAGE_MAR_HEADER, nIntValue );
        break;
        case NS_ooxml::LN_CT_PageMar_footer:
            m_pImpl->SetPageMarginTwip( PAGE_MAR_FOOTER, nIntValue );
        break;
        case NS_ooxml::LN_CT_PageMar_gutter:
            m_pImpl->SetPageMarginTwip( PAGE_MAR_GUTTER, nIntValue );
        break;
        case NS_ooxml::LN_CT_Language_val: //90314
        case NS_ooxml::LN_CT_Language_eastAsia: //90315
        case NS_ooxml::LN_CT_Language_bidi: //90316
        {
            if (nName == NS_ooxml::LN_CT_Language_eastAsia)
                m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "eastAsia", sStringValue);
            else if (nName == NS_ooxml::LN_CT_Language_val)
                m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "val", sStringValue);
            else if (nName == NS_ooxml::LN_CT_Language_bidi)
                m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "bidi", sStringValue);
            lang::Locale aLocale( LanguageTag::convertToLocale( sStringValue));
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(NS_ooxml::LN_CT_Language_val== nName ? PROP_CHAR_LOCALE :
                             NS_ooxml::LN_CT_Language_eastAsia == nName ? PROP_CHAR_LOCALE_ASIAN : PROP_CHAR_LOCALE_COMPLEX,
                             uno::makeAny( aLocale ) );
        }
        break;
        // See SwWW8ImplReader::GetParagraphAutoSpace() on why these are 100 and 280
        case NS_ooxml::LN_CT_Spacing_beforeAutospacing:
        {
            sal_Int32 default_spacing = 100;
            if (!m_pImpl->GetSettingsTable()->GetDoNotUseHTMLParagraphAutoSpacing())
            {
                // 49 is just the old value that should be removed, once the
                // root cause in SwTabFrm::MakeAll() is fixed.
                if (m_pImpl->GetSettingsTable()->GetView() == NS_ooxml::LN_Value_wordprocessingml_ST_View_web)
                    default_spacing = 49;
                else
                    default_spacing = 280;
            }
            m_pImpl->GetTopContext()->Insert( PROP_PARA_TOP_MARGIN, uno::makeAny( ConversionHelper::convertTwipToMM100(default_spacing) ) );
            if  (nIntValue) // If auto spacing is set, then only store set value in InteropGrabBag
            {
                m_pImpl->GetTopContext()->Insert( PROP_PARA_TOP_MARGIN_BEFORE_AUTO_SPACING, uno::makeAny( ConversionHelper::convertTwipToMM100(default_spacing) ),true, PARA_GRAB_BAG );
            }
        }
        break;
        case NS_ooxml::LN_CT_Spacing_afterAutospacing:
        {
            sal_Int32 default_spacing = 100;

            if (!m_pImpl->GetSettingsTable()->GetDoNotUseHTMLParagraphAutoSpacing())
            {
                if (m_pImpl->GetSettingsTable()->GetView() == NS_ooxml::LN_Value_wordprocessingml_ST_View_web)
                    default_spacing = 49;
                else
                    default_spacing = 280;
            }
            m_pImpl->GetTopContext()->Insert( PROP_PARA_BOTTOM_MARGIN, uno::makeAny( ConversionHelper::convertTwipToMM100(default_spacing) ) );
            if  (nIntValue) // If auto spacing is set, then only store set value in InteropGrabBag
            {
                m_pImpl->GetTopContext()->Insert( PROP_PARA_BOTTOM_MARGIN_AFTER_AUTO_SPACING, uno::makeAny( ConversionHelper::convertTwipToMM100(default_spacing) ),true, PARA_GRAB_BAG );
            }
        }
        break;
        case NS_ooxml::LN_CT_SmartTagRun_uri:
        case NS_ooxml::LN_CT_SmartTagRun_element:
            //TODO: add handling of SmartTags
        break;
        case NS_ooxml::LN_CT_Br_type :
            //TODO: attributes for break (0x12) are not supported
        break;
        case NS_ooxml::LN_CT_Fonts_hint :
            /*  assigns script type to ambigous characters, values can be:
                NS_ooxml::LN_Value_ST_Hint_default
                NS_ooxml::LN_Value_ST_Hint_eastAsia
                NS_ooxml::LN_Value_ST_Hint_cs
             */
            //TODO: unsupported?
        break;
        case NS_ooxml::LN_CT_TblCellMar_right: // 92375;
        case NS_ooxml::LN_CT_TblBorders_top: // 92377;
        case NS_ooxml::LN_CT_TblBorders_left: // 92378;
        case NS_ooxml::LN_CT_TblBorders_bottom: // 92379;
        //todo: handle cell mar
        break;
        case NS_ooxml::LN_blip: // contains the binary graphic
        case NS_ooxml::LN_shape:
        {
            //looks a bit like a hack - and it is. The graphic import is split into the inline_inline part and
            //afterwards the adding of the binary data.
            m_pImpl->GetGraphicImport( IMPORT_AS_DETECTED_INLINE )->attribute(nName, val);
            m_pImpl->ImportGraphic( val.getProperties(), IMPORT_AS_DETECTED_INLINE );
        }
        break;
        case NS_ooxml::LN_starmath:
            m_pImpl->appendStarMath( val );
            break;
        case NS_ooxml::LN_CT_FramePr_dropCap:
        case NS_ooxml::LN_CT_FramePr_lines:
        case NS_ooxml::LN_CT_FramePr_hAnchor:
        case NS_ooxml::LN_CT_FramePr_vAnchor:
        case NS_ooxml::LN_CT_FramePr_x:
        case NS_ooxml::LN_CT_FramePr_xAlign:
        case NS_ooxml::LN_CT_FramePr_y:
        case NS_ooxml::LN_CT_FramePr_yAlign:
        case NS_ooxml::LN_CT_FramePr_hRule:
        case NS_ooxml::LN_CT_FramePr_w:
        case NS_ooxml::LN_CT_FramePr_h:
        case NS_ooxml::LN_CT_FramePr_wrap:
        case NS_ooxml::LN_CT_FramePr_hSpace:
        case NS_ooxml::LN_CT_FramePr_vSpace:
        {
            ParagraphProperties* pParaProperties = dynamic_cast< ParagraphProperties*>(
                    m_pImpl->GetTopContextOfType( CONTEXT_PARAGRAPH ).get() );
            if( pParaProperties )
            {
                switch( nName )
                {
                    case NS_ooxml::LN_CT_FramePr_dropCap:
                        pParaProperties->SetDropCap( nIntValue );
                    break;
                    case NS_ooxml::LN_CT_FramePr_lines:
                        pParaProperties->SetLines( nIntValue );
                    break;
                    case NS_ooxml::LN_CT_FramePr_hAnchor:
                        switch(nIntValue)
                        {
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_HAnchor_text:   //relative to column
                                nIntValue = text::RelOrientation::FRAME; break;
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_HAnchor_margin: nIntValue = text::RelOrientation::PAGE_PRINT_AREA; break;
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_HAnchor_page:   nIntValue = text::RelOrientation::PAGE_FRAME; break;
                            default:;
                        }
                        pParaProperties->SethAnchor( nIntValue );
                    break;
                    case NS_ooxml::LN_CT_FramePr_vAnchor:
                        switch(nIntValue)
                        {
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_VAnchor_text:  //relative to paragraph
                                    nIntValue = text::RelOrientation::FRAME; break;
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_VAnchor_margin:nIntValue = text::RelOrientation::PAGE_PRINT_AREA ; break;
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_VAnchor_page: nIntValue = text::RelOrientation::PAGE_FRAME; break;
                            default:;
                        }
                        pParaProperties->SetvAnchor( nIntValue );
                    break;
                    case NS_ooxml::LN_CT_FramePr_x:
                        pParaProperties->Setx( ConversionHelper::convertTwipToMM100(nIntValue ));
                    break;
                    case NS_ooxml::LN_CT_FramePr_xAlign:
                        switch( nIntValue )
                        {
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_XAlign_center  : nIntValue = text::HoriOrientation::CENTER; break;
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_XAlign_right   : nIntValue = text::HoriOrientation::RIGHT; break;
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_XAlign_inside  : nIntValue = text::HoriOrientation::INSIDE; break;
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_XAlign_outside : nIntValue = text::HoriOrientation::OUTSIDE; break;
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_XAlign_left    : nIntValue = text::HoriOrientation::LEFT; break;
                            default:    nIntValue = text::HoriOrientation::NONE;
                        }
                        pParaProperties->SetxAlign( nIntValue );
                    break;
                    case NS_ooxml::LN_CT_FramePr_y:
                        pParaProperties->Sety( ConversionHelper::convertTwipToMM100(nIntValue ));
                    break;
                    case NS_ooxml::LN_CT_FramePr_yAlign:
                        switch( nIntValue )
                        {
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_top     :
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_inside  :nIntValue = text::VertOrientation::TOP; break;
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_center  :nIntValue = text::VertOrientation::CENTER;break;
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_bottom  :
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_outside :nIntValue = text::VertOrientation::BOTTOM;break;
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_inline  :
                            {
                            // HACK: This is for bnc#780851, where a table has one cell that has w:framePr,
                            // which causes that paragraph to be converted to a text frame, and the original
                            // paragraph object no longer exists, which makes table creation fail and futhermore
                            // it would be missing in the table layout anyway. So actually no letting that paragraph
                            // be a text frame "fixes" it. I'm not sure what "inline" is supposed to mean in practice
                            // anyway, so as long as this doesn't cause trouble elsewhere ...
                                PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH);
                                if( pContext.get() )
                                {
                                    ParagraphPropertyMap* pParaContext = dynamic_cast< ParagraphPropertyMap* >( pContext.get() );
                                    pParaContext->SetFrameMode(false);
                                }
                                nIntValue = text::VertOrientation::NONE;
                                break;
                            }
                            default:
                                nIntValue = text::VertOrientation::NONE;
                                break;
                        }
                        pParaProperties->SetyAlign( nIntValue );
                    break;
                    case NS_ooxml::LN_CT_FramePr_hRule:
                         switch( nIntValue )
                         {
                            case NS_ooxml::LN_Value_wordprocessingml_ST_HeightRule_exact:
                                nIntValue = text::SizeType::FIX;
                            break;
                            case NS_ooxml::LN_Value_wordprocessingml_ST_HeightRule_atLeast:
                                nIntValue = text::SizeType::MIN;
                            break;
                            case NS_ooxml::LN_Value_wordprocessingml_ST_HeightRule_auto:
                            //no break;
                            default:;
                                nIntValue = text::SizeType::VARIABLE;
                         }
                        pParaProperties->SethRule( nIntValue );
                    break;
                    case NS_ooxml::LN_CT_FramePr_wrap:
                    {
                        //should be either LN_Value_wordprocessingml_ST_Wrap_notBeside or LN_Value_wordprocessingml_ST_Wrap_around
                        OSL_ENSURE( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_wordprocessingml_ST_Wrap_around ||
                                    sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_wordprocessingml_ST_Wrap_notBeside,
                            "wrap not around or not_Beside?");
                        pParaProperties->SetWrap(sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_wordprocessingml_ST_Wrap_around ?
                                                 text::WrapTextMode_DYNAMIC : text::WrapTextMode_NONE );
                    }
                    break;
                    case NS_ooxml::LN_CT_FramePr_w:
                        pParaProperties->Setw(ConversionHelper::convertTwipToMM100(nIntValue));
                    break;
                    case NS_ooxml::LN_CT_FramePr_h:
                        pParaProperties->Seth(ConversionHelper::convertTwipToMM100(nIntValue));
                    break;
                    case NS_ooxml::LN_CT_FramePr_hSpace:
                        pParaProperties->SethSpace( ConversionHelper::convertTwipToMM100(nIntValue ));
                    break;
                    case NS_ooxml::LN_CT_FramePr_vSpace:
                        pParaProperties->SetvSpace( ConversionHelper::convertTwipToMM100(nIntValue ));
                    break;
                    default:;
                }
            }
            else
            {
                //TODO: how to handle frame properties at styles
            }
        }
        break;
        case NS_ooxml::LN_CT_TrackChange_author:
            m_pImpl->SetCurrentRedlineAuthor( sStringValue );
        break;
        case NS_ooxml::LN_CT_TrackChange_date:
            m_pImpl->SetCurrentRedlineDate( sStringValue );
        break;
        case NS_ooxml::LN_CT_Markup_id:
            m_pImpl->SetCurrentRedlineId( nIntValue );
        break;
        case NS_ooxml::LN_EG_RangeMarkupElements_commentRangeStart:
            m_pImpl->AddAnnotationPosition( true, nIntValue );
        break;
        case NS_ooxml::LN_EG_RangeMarkupElements_commentRangeEnd:
            m_pImpl->AddAnnotationPosition( false, nIntValue );
        break;
        case NS_ooxml::LN_CT_Comment_initials:
            m_pImpl->SetCurrentRedlineInitials(sStringValue);
        break;
        case NS_ooxml::LN_token:
            m_pImpl->SetCurrentRedlineToken( nIntValue );
        break;
        case NS_ooxml::LN_CT_LineNumber_start:
        case NS_ooxml::LN_CT_LineNumber_distance:
        case NS_ooxml::LN_CT_LineNumber_countBy:
        case NS_ooxml::LN_CT_LineNumber_restart:
        {
            //line numbering in Writer is a global document setting
            //in Word is a section setting
            //if line numbering is switched on anywhere in the document it's set at the global settings
            LineNumberSettings aSettings = m_pImpl->GetLineNumberSettings();
            switch( nName )
            {
                case NS_ooxml::LN_CT_LineNumber_countBy:
                    aSettings.nInterval = nIntValue;
                    OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
                    if( pSectionContext )
                        pSectionContext->SetLnnMod( nIntValue );
                break;
                case NS_ooxml::LN_CT_LineNumber_start:
                    aSettings.nStartValue = nIntValue; // todo: has to be set at (each) first paragraph
                    OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
                    if( pSectionContext )
                        pSectionContext->SetLnnMin( nIntValue );
                break;
                case NS_ooxml::LN_CT_LineNumber_distance:
                    aSettings.nDistance = ConversionHelper::convertTwipToMM100( nIntValue );
                    OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
                    if( pSectionContext )
                        pSectionContext->SetdxaLnn( nIntValue );
                break;
                case NS_ooxml::LN_CT_LineNumber_restart:
                    //page:empty, probably 0,section:1,continuous:2;
                    aSettings.bRestartAtEachPage = nIntValue < 1;
                    OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
                    if( pSectionContext )
                        pSectionContext->SetLnc( nIntValue );
                break;
                default:;
            }
            m_pImpl->SetLineNumberSettings( aSettings );
        }
        break;
        case NS_ooxml::LN_CT_FtnEdnRef_customMarkFollows:
            m_pImpl->SetCustomFtnMark( true );
        break;
        case NS_ooxml::LN_CT_FtnEdnRef_id:
            // footnote or endnote reference id - not needed
        case NS_ooxml::LN_CT_Color_themeColor:
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "themeColor", TDefTableHandler::getThemeColorTypeString(nIntValue));
        break;
        case NS_ooxml::LN_CT_Color_themeTint:
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "themeTint", OUString::number(nIntValue, 16));
        break;
        case NS_ooxml::LN_CT_Color_themeShade:
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "themeShade", OUString::number(nIntValue, 16));
        break;
        case NS_ooxml::LN_CT_DocGrid_linePitch:
        {
            //see SwWW8ImplReader::SetDocumentGrid
            OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
            if(pSectionContext)
            {
                pSectionContext->SetGridLinePitch( ConversionHelper::convertTwipToMM100( nIntValue ) );
            }
        }
        break;
        case NS_ooxml::LN_CT_DocGrid_charSpace:
        {
            OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
            if(pSectionContext)
            {
                pSectionContext->SetDxtCharSpace( nIntValue );
            }
        }
        break;
        case NS_ooxml::LN_CT_DocGrid_type:
        {
            if (pSectionContext != NULL)
            {
                pSectionContext->SetGridType(nIntValue);
            }
        }
        break;
        case NS_ooxml::LN_CT_SdtBlock_sdtContent:
            m_pImpl->SetSdt(true);
        break;
        case NS_ooxml::LN_CT_SdtBlock_sdtEndContent:
            m_pImpl->SetSdt(false);
            if (!m_pImpl->m_pSdtHelper->getDropDownItems().empty())
                m_pImpl->m_pSdtHelper->createDropDownControl();
        break;
        case NS_ooxml::LN_CT_SdtListItem_displayText:
            // TODO handle when this is != value
        break;
        case NS_ooxml::LN_CT_SdtListItem_value:
            m_pImpl->m_pSdtHelper->getDropDownItems().push_back(sStringValue);
        break;
        case NS_ooxml::LN_CT_Background_color:
            m_pImpl->m_oBackgroundColor.reset(nIntValue);
        break;
        case NS_ooxml::LN_CT_PageNumber_start:
            if (pSectionContext != NULL)
                pSectionContext->SetPageNumber(nIntValue);
        break;
        case NS_ooxml::LN_CT_FtnEdn_type:
            // This is the "separator" footnote, ignore its linebreak.
            if (static_cast<sal_uInt32>(nIntValue) == NS_ooxml::LN_Value_wordprocessingml_ST_FtnEdn_separator)
                m_pImpl->m_bIgnoreNextPara = true;
        break;
        default:
            {
#if OSL_DEBUG_LEVEL > 0
            OString sMessage("DomainMapper::attribute() - Id: ");
            sMessage += OString::number( nName, 10 );
            sMessage += " / 0x";
            sMessage += OString::number( nName, 16 );
            sMessage += " value: ";
            sMessage += OString::number( nIntValue, 10 );
            sMessage += " / 0x";
            sMessage += OString::number( nIntValue, 16 );
            SAL_WARN("writerfilter", sMessage.getStr());
#endif
            }
        }
}

void DomainMapper::lcl_sprm(Sprm & rSprm)
{
    if( !m_pImpl->getTableManager().sprm(rSprm))
        sprmWithProps( rSprm, m_pImpl->GetTopContext() );
}

sal_Int32 lcl_getCurrentNumberingProperty(uno::Reference<container::XIndexAccess> xNumberingRules, sal_Int32 nNumberingLevel, OUString aProp)
{
    sal_Int32 nRet = 0;

    try
    {
        if (nNumberingLevel < 0) // It seems it's valid to omit numbering level, and in that case it means zero.
            nNumberingLevel = 0;
        if (xNumberingRules.is())
        {
            uno::Sequence<beans::PropertyValue> aProps;
            xNumberingRules->getByIndex(nNumberingLevel) >>= aProps;
            for (int i = 0; i < aProps.getLength(); ++i)
            {
                const beans::PropertyValue& rProp = aProps[i];

                if (rProp.Name == aProp)
                {
                    rProp.Value >>= nRet;
                    break;
                }
            }
        }
    }
    catch( const uno::Exception& )
    {
        // This can happen when the doc contains some hand-crafted invalid list level.
    }

    return nRet;
}

// In rtl-paragraphs the meaning of left/right are to be exchanged
static bool ExchangeLeftRight( const PropertyMapPtr rContext, DomainMapper_Impl* m_pImpl )
{
    bool bExchangeLeftRight = false;
    PropertyMap::const_iterator aPropParaIte = rContext->find(PROP_WRITING_MODE);
    if( aPropParaIte != rContext->end())
    {
        sal_Int32 aAdjust ;
        if( (aPropParaIte->second.getValue() >>= aAdjust) && aAdjust == text::WritingMode2::RL_TB )
            bExchangeLeftRight = true;
    }
    else
    {
        // check if there RTL <bidi> in default style for the paragraph
        StyleSheetEntryPtr pTable = m_pImpl->GetStyleSheetTable()->FindDefaultParaStyle();
        if ( pTable )
        {
            PropertyMap::const_iterator aPropStyle = pTable->pProperties->find(PROP_WRITING_MODE);
            if( aPropStyle != pTable->pProperties->end())
            {
                sal_Int32 aDirect;
                if( (aPropStyle->second.getValue() >>= aDirect) && aDirect == text::WritingMode2::RL_TB )
                    bExchangeLeftRight = true;
            }
        }
    }
    return bExchangeLeftRight;
}

void DomainMapper::sprmWithProps( Sprm& rSprm, PropertyMapPtr rContext, SprmType eSprmType )
{
    OSL_ENSURE(rContext.get(), "PropertyMap has to be valid!");
    if(!rContext.get())
        return ;

    sal_uInt32 nSprmId = rSprm.getId();
    //needed for page properties
    SectionPropertyMap * pSectionContext = m_pImpl->GetSectionContext();
    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue->getInt();
    OUString sStringValue = pValue->getString();
    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

    switch(nSprmId)
    {
    case 2:  // sprmPIstd
    case 0x4600:
        break;  // sprmPIstd - style code
    case NS_ooxml::LN_CT_PPrBase_jc:
        handleParaJustification(nIntValue, rContext, ExchangeLeftRight( rContext, m_pImpl ));
        break;
    case NS_ooxml::LN_CT_PPrBase_keepLines:
        rContext->Insert(PROP_PARA_SPLIT, uno::makeAny(nIntValue ? false : true));
        break;
    case NS_ooxml::LN_CT_PPrBase_keepNext:
        rContext->Insert(PROP_PARA_KEEP_TOGETHER, uno::makeAny( nIntValue ? true : false) );
        break;
    case NS_ooxml::LN_CT_PPrBase_pageBreakBefore:
        rContext->Insert(PROP_BREAK_TYPE, uno::makeAny( com::sun::star::style::BreakType_PAGE_BEFORE ) );
    break;
    case NS_ooxml::LN_CT_NumPr_ilvl:
            if (nIntValue < 0 || 10 <= nIntValue) // Writer can't do everything
            {
                SAL_INFO("writerfilter",
                        "unsupported numbering level " << nIntValue);
                break;
            }
            if( m_pImpl->IsStyleSheetImport() )
            {
                //style sheets cannot have a numbering rule attached
                StyleSheetPropertyMap* pStyleSheetPropertyMap = dynamic_cast< StyleSheetPropertyMap* >( rContext.get() );
                if (pStyleSheetPropertyMap)
                    pStyleSheetPropertyMap->SetListLevel( (sal_Int16)nIntValue );
            }
            else
                rContext->Insert( PROP_NUMBERING_LEVEL, uno::makeAny( (sal_Int16)nIntValue ));
        break;
    case NS_ooxml::LN_CT_NumPr_numId:
        {
            //convert the ListTable entry to a NumberingRules propery and apply it
            ListsManager::Pointer pListTable = m_pImpl->GetListTable();
            ListDef::Pointer pList = pListTable->GetList( nIntValue );
            if( m_pImpl->IsStyleSheetImport() )
            {
                //style sheets cannot have a numbering rule attached
                StyleSheetPropertyMap* pStyleSheetPropertyMap = dynamic_cast< StyleSheetPropertyMap* >( rContext.get() );
                if (pStyleSheetPropertyMap)
                    pStyleSheetPropertyMap->SetListId( nIntValue );
            }
            if( pList.get( ) )
            {
                if( !m_pImpl->IsStyleSheetImport() )
                {
                    uno::Any aRules = uno::makeAny( pList->GetNumberingRules( ) );
                    rContext->Insert( PROP_NUMBERING_RULES, aRules );
                    // erase numbering from pStyle if already set
                    rContext->erase(PROP_NUMBERING_STYLE_NAME);
                }
            }
            else
            {
                if( m_pImpl->IsStyleSheetImport() )
                {
                    // set the number id for AbstractNum references
                    StyleSheetPropertyMap* pStyleSheetPropertyMap = dynamic_cast< StyleSheetPropertyMap* >( rContext.get() );
                    if (pStyleSheetPropertyMap)
                        pStyleSheetPropertyMap->SetNumId( nIntValue );
                }
                else
                {
                    rContext->Insert( PROP_NUMBERING_STYLE_NAME, uno::makeAny( OUString() ) );
                }
            }
        }
        break;
    case NS_ooxml::LN_CT_PPrBase_suppressLineNumbers:
        rContext->Insert(PROP_PARA_LINE_NUMBER_COUNT, uno::makeAny( nIntValue ? false : true) );
        break;
    case 0x845d:    //right margin Asian - undocumented
    case 0x845e:    //left margin Asian - undocumented
    case 16:      // sprmPDxaRight - right margin
    case NS_sprm::LN_PDxaRight:   // sprmPDxaRight - right margin
    case 17:
    case NS_sprm::LN_PDxaLeft:   // sprmPDxaLeft
    {
        bool bExchangeLeftRight = ExchangeLeftRight( rContext, m_pImpl );
        if( NS_sprm::LN_PDxaLeft == nSprmId || 0x17 == nSprmId|| (bExchangeLeftRight && nSprmId == 0x845d) || ( !bExchangeLeftRight && nSprmId == 0x845e))
            rContext->Insert(
                             eSprmType == SPRM_DEFAULT ? PROP_PARA_LEFT_MARGIN : PROP_LEFT_MARGIN,
                             uno::makeAny( ConversionHelper::convertTwipToMM100( nIntValue ) ));
        else if(eSprmType == SPRM_DEFAULT)
            rContext->Insert(
                             PROP_PARA_RIGHT_MARGIN,
                             uno::makeAny( ConversionHelper::convertTwipToMM100(nIntValue ) ));
    }
        //TODO: what happens to the right margins in numberings?
        break;
    case NS_sprm::LN_PDxaLeft1:    // sprmPDxaLeft1
        rContext->Insert(
                         eSprmType == SPRM_DEFAULT ? PROP_PARA_FIRST_LINE_INDENT : PROP_FIRST_LINE_OFFSET,
                         uno::makeAny( ConversionHelper::convertTwipToMM100(nIntValue ) ));
        break;
    case 24: // "sprmPFInTable"
    case NS_sprm::LN_PFInTable:
        break;  // sprmPFInTable
    case NS_sprm::LN_PTableDepth: //sprmPTableDepth
        //not handled via sprm but via text( 0x07 )
    break;
    case NS_ooxml::LN_CT_FramePr_w:
        break;
    case NS_ooxml::LN_CT_FramePr_wrap:
        break;

    case NS_ooxml::LN_CT_PrBase_pBdr: //paragraph border
        resolveSprmProps(*this, rSprm);
    break;
    case NS_ooxml::LN_CT_PBdr_top:
    case NS_ooxml::LN_CT_PBdr_left:
    case NS_ooxml::LN_CT_PBdr_bottom:
    case NS_ooxml::LN_CT_PBdr_right:
    case NS_ooxml::LN_CT_PBdr_between:
        {
            //in binary format the borders are directly provided in OOXML they are inside of properties
            if( IsOOXMLImport() || IsRTFImport() )
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
                if( pProperties.get())
                {
                    BorderHandlerPtr pBorderHandler( new BorderHandler( true ) );
                    pProperties->resolve(*pBorderHandler);
                    PropertyIds eBorderId = PropertyIds( 0 );
                    PropertyIds eBorderDistId = PropertyIds( 0 );
                    switch( nSprmId )
                    {
                        case NS_ooxml::LN_CT_PBdr_top:
                            eBorderId = PROP_TOP_BORDER;
                            eBorderDistId = PROP_TOP_BORDER_DISTANCE;
                        break;
                        case NS_ooxml::LN_CT_PBdr_left:
                            eBorderId = PROP_LEFT_BORDER;
                            eBorderDistId = PROP_LEFT_BORDER_DISTANCE;
                        break;
                        case NS_ooxml::LN_CT_PBdr_bottom:
                            eBorderId = PROP_BOTTOM_BORDER         ;
                            eBorderDistId = PROP_BOTTOM_BORDER_DISTANCE;
                        break;
                        case NS_ooxml::LN_CT_PBdr_right:
                            eBorderId = PROP_RIGHT_BORDER;
                            eBorderDistId = PROP_RIGHT_BORDER_DISTANCE ;
                        break;
                        case NS_ooxml::LN_CT_PBdr_between:
                            //not supported
                        break;
                        default:;
                    }
                    if( eBorderId )
                        rContext->Insert( eBorderId, uno::makeAny( pBorderHandler->getBorderLine()) , true);
                    if(eBorderDistId)
                        rContext->Insert(eBorderDistId, uno::makeAny( pBorderHandler->getLineDistance()), true);
                    if (nSprmId == NS_ooxml::LN_CT_PBdr_right && pBorderHandler->getShadow())
                    {
                        table::ShadowFormat aFormat = rContext->getShadowFromBorder(pBorderHandler->getBorderLine());
                        rContext->Insert(PROP_PARA_SHADOW_FORMAT, uno::makeAny(aFormat));
                    }
                }
            }
            else
            {
                table::BorderLine2 aBorderLine;
                sal_Int32 nLineDistance = ConversionHelper::MakeBorderLine( nIntValue, aBorderLine );
                PropertyIds eBorderId = PROP_LEFT_BORDER;
                PropertyIds eBorderDistId = PROP_LEFT_BORDER_DISTANCE  ;
                switch( nSprmId )
                {
                case NS_ooxml::LN_CT_PBdr_between:
                    OSL_FAIL( "TODO: inner border is not handled");
                    break;
                case NS_ooxml::LN_CT_PBdr_left:
                    eBorderId = PROP_LEFT_BORDER;
                    eBorderDistId = PROP_LEFT_BORDER_DISTANCE  ;
                    break;
                case NS_ooxml::LN_CT_PBdr_right:
                    eBorderId = PROP_RIGHT_BORDER          ;
                    eBorderDistId = PROP_RIGHT_BORDER_DISTANCE ;
                    break;
                case NS_ooxml::LN_CT_PBdr_top:
                    eBorderId = PROP_TOP_BORDER            ;
                    eBorderDistId = PROP_TOP_BORDER_DISTANCE;
                    break;
                case NS_ooxml::LN_CT_PBdr_bottom:
                default:
                    eBorderId = PROP_BOTTOM_BORDER         ;
                    eBorderDistId = PROP_BOTTOM_BORDER_DISTANCE;
                }
                rContext->Insert(eBorderId, uno::makeAny( aBorderLine ));
                rContext->Insert(eBorderDistId, uno::makeAny( nLineDistance ));
            }
        }
    break;
    case NS_ooxml::LN_CT_PBdr_bar:
        break;
    case NS_ooxml::LN_CT_PPrBase_suppressAutoHyphens:
        rContext->Insert(PROP_PARA_IS_HYPHENATION, uno::makeAny( nIntValue ? false : true ));
        break;
    case NS_ooxml::LN_CT_FramePr_h:
        break;
    case NS_ooxml::LN_CT_PrBase_shd:
    {
        //contains fore color, back color and shadow percentage, results in a brush
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
        {
            CellColorHandlerPtr pCellColorHandler( new CellColorHandler );
            pCellColorHandler->setOutputFormat( CellColorHandler::Paragraph );
            sal_Bool bEnableTempGrabBag = !pCellColorHandler->isInteropGrabBagEnabled();
            if( bEnableTempGrabBag )
                pCellColorHandler->enableInteropGrabBag( "TempShdPropsGrabBag" );

            pProperties->resolve(*pCellColorHandler);
            rContext->InsertProps(pCellColorHandler->getProperties());

            rContext->Insert(PROP_CHAR_THEME_FILL,  pCellColorHandler->getInteropGrabBag().Value, true, PARA_GRAB_BAG);
            if(bEnableTempGrabBag)
                pCellColorHandler->disableInteropGrabBag();
        }
    }
    break;
    case NS_ooxml::LN_CT_FramePr_vSpace:
        break;  // sprmPDyaFromText
    case NS_ooxml::LN_CT_FramePr_hSpace:
        break;  // sprmPDxaFromText
    case NS_ooxml::LN_CT_FramePr_anchorLock:
        break;
    case NS_sprm::LN_PFWidowControl:
    case NS_ooxml::LN_CT_PPrBase_widowControl:
    {
        uno::Any aVal( uno::makeAny( sal_Int8(nIntValue ? 2 : 0 )));
        rContext->Insert( PROP_PARA_WIDOWS, aVal );
        rContext->Insert( PROP_PARA_ORPHANS, aVal );
    }
    break;  // sprmPFWidowControl
    case NS_ooxml::LN_CT_PPrBase_overflowPunct:
        rContext->Insert(PROP_PARA_IS_HANGING_PUNCTUATION, uno::makeAny( nIntValue ? false : true ));
        break;
    case NS_ooxml::LN_CT_PPrBase_topLinePunct:
        break;
    case NS_ooxml::LN_CT_PPrBase_autoSpaceDE:
        break;
    case NS_ooxml::LN_CT_PPrBase_autoSpaceDN:
        break;
    case NS_ooxml::LN_CT_PPrBase_textAlignment:
        {
            sal_Int16 nAlignment = 0;
            switch (nIntValue)
            {
                case NS_ooxml::LN_Value_wordprocessingml_ST_TextAlignment_top:
                    nAlignment = 2;
                    break;
                case NS_ooxml::LN_Value_wordprocessingml_ST_TextAlignment_center:
                    nAlignment = 3;
                    break;
                case NS_ooxml::LN_Value_wordprocessingml_ST_TextAlignment_baseline:
                    nAlignment = 1;
                    break;
                case NS_ooxml::LN_Value_wordprocessingml_ST_TextAlignment_bottom:
                    nAlignment = 4;
                    break;
                case NS_ooxml::LN_Value_wordprocessingml_ST_TextAlignment_auto:
                default:
                    break;
            }
            rContext->Insert( PROP_PARA_VERT_ALIGNMENT, uno::makeAny( nAlignment) );
        }
        break;
    case NS_ooxml::LN_CT_PPrBase_textDirection:
        break;
    case NS_ooxml::LN_CT_PPrBase_outlineLvl:
        {
            sal_Int16 nLvl = static_cast< sal_Int16 >( nIntValue );
            if( m_pImpl->IsStyleSheetImport() )
            {

                StyleSheetPropertyMap* pStyleSheetPropertyMap = dynamic_cast< StyleSheetPropertyMap* >( rContext.get() );
                pStyleSheetPropertyMap->SetOutlineLevel( nLvl );
            }
            else
            {
                nLvl = nLvl >= WW_OUTLINE_MIN && nLvl < WW_OUTLINE_MAX? nLvl+1 : 0; //0 means no outline level set on
                rContext->Insert(PROP_OUTLINE_LEVEL, uno::makeAny ( nLvl ));
            }
        }
        break;
    case NS_ooxml::LN_CT_PPrBase_bidi:
        {
            if (nIntValue != 0)
            {
                rContext->Insert(PROP_WRITING_MODE, uno::makeAny( text::WritingMode2::RL_TB ));
                rContext->Insert(PROP_PARA_ADJUST, uno::makeAny( style::ParagraphAdjust_RIGHT ));
            }
            else
            {
                rContext->Insert(PROP_WRITING_MODE, uno::makeAny( text::WritingMode2::LR_TB ));
                rContext->Insert(PROP_PARA_ADJUST, uno::makeAny( style::ParagraphAdjust_LEFT ));
            }
        }

        break;
    case NS_ooxml::LN_EG_SectPrContents_bidi:
        if (pSectionContext != NULL)
            pSectionContext->Insert(PROP_WRITING_MODE, uno::makeAny( text::WritingMode2::RL_TB));
        break;
    case NS_sprm::LN_CHighlight:
        {
            sal_Int32 nColor = 0;
            if( (mbIsHighlightSet = getColorFromIndex(nIntValue, nColor)) )
                rContext->Insert(PROP_CHAR_HIGHLIGHT, uno::makeAny( nColor ));
            else if (mnBackgroundColor)
                rContext->Insert(PROP_CHAR_BACK_COLOR, uno::makeAny( mnBackgroundColor ));
        }
        break;  // sprmCHighlight
    case NS_sprm::LN_CKcd:
        rContext->Insert(PROP_CHAR_EMPHASIS, uno::makeAny ( getEmphasisValue (nIntValue)));
        break;  // sprmCKcd
    case NS_sprm::LN_CFEmboss:// sprmCFEmboss
    case 60:// sprmCFBold
    case NS_sprm::LN_CFBoldBi:// sprmCFBoldBi    (offset 0x27 to normal bold)
    case NS_sprm::LN_CFItalicBi:// sprmCFItalicBi  (offset 0x27 to normal italic)
    case NS_sprm::LN_CFBold: //sprmCFBold
    case 61: /*sprmCFItalic*/
    case NS_sprm::LN_CFItalic: //sprmCFItalic
    case NS_sprm::LN_CFStrike: //sprmCFStrike
    case NS_sprm::LN_CFOutline: //sprmCFOutline
    case NS_sprm::LN_CFShadow: //sprmCFShadow
    case NS_sprm::LN_CFSmallCaps: //sprmCFSmallCaps
    case NS_sprm::LN_CFCaps: //sprmCFCaps
    case NS_sprm::LN_CFVanish: //sprmCFVanish
    case NS_sprm::LN_CFDStrike:   // sprmCFDStrike
        {
            PropertyIds ePropertyId = PROP_CHAR_WEIGHT; //initialized to prevent warning!
            switch( nSprmId )
            {
            case 60:// sprmCFBold
            case NS_sprm::LN_CFBoldBi: // sprmCFBoldBi
            case NS_sprm::LN_CFBold: /*sprmCFBold*/
                ePropertyId = nSprmId != NS_sprm::LN_CFBoldBi ? PROP_CHAR_WEIGHT : PROP_CHAR_WEIGHT_COMPLEX;
                break;
            case 61: /*sprmCFItalic*/
            case NS_sprm::LN_CFItalicBi: // sprmCFItalicBi
            case NS_sprm::LN_CFItalic: /*sprmCFItalic*/
                ePropertyId = nSprmId == 0x836 ? PROP_CHAR_POSTURE : PROP_CHAR_POSTURE_COMPLEX;
                break;
            case NS_sprm::LN_CFStrike: /*sprmCFStrike*/
            case NS_sprm::LN_CFDStrike : /*sprmCFDStrike double strike through*/
                ePropertyId = PROP_CHAR_STRIKEOUT;
                break;
            case NS_sprm::LN_CFOutline: /*sprmCFOutline*/
                ePropertyId = PROP_CHAR_CONTOURED;
                break;
            case NS_sprm::LN_CFShadow: /*sprmCFShadow*/
                ePropertyId = PROP_CHAR_SHADOWED;
                break;
            case NS_sprm::LN_CFSmallCaps: /*sprmCFSmallCaps*/
            case NS_sprm::LN_CFCaps: /*sprmCFCaps*/
                ePropertyId = PROP_CHAR_CASE_MAP;
                break;
            case NS_sprm::LN_CFVanish: /*sprmCFVanish*/
                ePropertyId = PROP_CHAR_HIDDEN;
                break;
            case NS_sprm::LN_CFEmboss: /*sprmCFEmboss*/
                ePropertyId = PROP_CHAR_RELIEF;
                break;
            }
            //expected: 0,1,128,129
            if(nIntValue != 128) //inherited from paragraph - ignore
            {
                if( nIntValue == 129) //inverted style sheet value
                {
                    //get value from style sheet and invert it
                    sal_Int16 nStyleValue = 0;
                    uno::Any aStyleVal = m_pImpl->GetPropertyFromStyleSheet(ePropertyId);
                    if( !aStyleVal.hasValue() )
                    {
                        nIntValue = 0x83a == nSprmId ?
                            4 : 1;
                    }
                    else if(aStyleVal.getValueTypeClass() == uno::TypeClass_FLOAT )
                    {
                        double fDoubleValue = 0;
                        //only in case of awt::FontWeight
                        aStyleVal >>= fDoubleValue;
                        nIntValue = fDoubleValue  > 100. ?  0 : 1;
                    }
                    else if((aStyleVal >>= nStyleValue) ||
                            (nStyleValue = (sal_Int16)comphelper::getEnumAsINT32(aStyleVal)) >= 0 )
                    {
                        nIntValue = 0x83a == nSprmId ?
                            nStyleValue ? 0 : 4 :
                            nStyleValue ? 0 : 1;
                    }
                    else
                    {
                        OSL_FAIL( "what type was it");
                    }
                }

                switch( nSprmId )
                {
                    case 60:/*sprmCFBold*/
                    case NS_sprm::LN_CFBold: /*sprmCFBold*/
                    case NS_sprm::LN_CFBoldBi: // sprmCFBoldBi
                    {
                        uno::Any aBold( uno::makeAny( nIntValue ? awt::FontWeight::BOLD : awt::FontWeight::NORMAL ) );

                        rContext->Insert(ePropertyId, aBold );
                        if( nSprmId != NS_sprm::LN_CFBoldBi ) // sprmCFBoldBi
                            rContext->Insert(PROP_CHAR_WEIGHT_ASIAN, aBold );

                        uno::Reference<beans::XPropertySet> xCharStyle(m_pImpl->GetCurrentNumberingCharStyle());
                        if (xCharStyle.is())
                            xCharStyle->setPropertyValue(rPropNameSupplier.GetName(PROP_CHAR_WEIGHT), aBold);
                        if (nSprmId == NS_sprm::LN_CFBold)
                            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "b", OUString::number(nIntValue));
                        else if (nSprmId == NS_sprm::LN_CFBoldBi)
                            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "bCs", OUString::number(nIntValue));
                    }
                    break;
                    case 61: /*sprmCFItalic*/
                    case NS_sprm::LN_CFItalic: /*sprmCFItalic*/
                    case NS_sprm::LN_CFItalicBi: // sprmCFItalicBi
                    {
                        uno::Any aPosture( uno::makeAny( nIntValue ? awt::FontSlant_ITALIC : awt::FontSlant_NONE ) );
                        rContext->Insert( ePropertyId, aPosture );
                        if( nSprmId != NS_sprm::LN_CFItalicBi ) // sprmCFItalicBi
                            rContext->Insert(PROP_CHAR_POSTURE_ASIAN, aPosture );
                        if (nSprmId == NS_sprm::LN_CFItalic)
                            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "i", OUString::number(nIntValue));
                    }
                    break;
                    case NS_sprm::LN_CFStrike: /*sprmCFStrike*/
                        rContext->Insert(ePropertyId,
                                         uno::makeAny( nIntValue ? awt::FontStrikeout::SINGLE : awt::FontStrikeout::NONE ) );
                    break;
                    case NS_sprm::LN_CFDStrike : /*sprmCFDStrike double strike through*/
                        rContext->Insert(ePropertyId,
                                         uno::makeAny( nIntValue ? awt::FontStrikeout::DOUBLE : awt::FontStrikeout::NONE ) );
                    break;
                    case NS_sprm::LN_CFOutline: /*sprmCFOutline*/
                    case NS_sprm::LN_CFShadow: /*sprmCFShadow*/
                    case NS_sprm::LN_CFVanish: /*sprmCFVanish*/
                        rContext->Insert(ePropertyId, uno::makeAny( nIntValue ? true : false ));
                    break;
                    case NS_sprm::LN_CFSmallCaps: /*sprmCFSmallCaps*/
                        // If smallcaps would be just disabled and an other casemap is already inserted, don't do anything.
                        if (nIntValue || rContext->find(ePropertyId) == rContext->end())
                            rContext->Insert(ePropertyId, uno::makeAny( nIntValue ? style::CaseMap::SMALLCAPS : style::CaseMap::NONE));
                        m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "smallCaps", OUString::number(nIntValue));
                    break;
                    case NS_sprm::LN_CFCaps: /*sprmCFCaps*/
                        rContext->Insert(ePropertyId,
                                         uno::makeAny( nIntValue ? style::CaseMap::UPPERCASE : style::CaseMap::NONE));
                        m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "caps", OUString::number(nIntValue));
                    break;
                    case NS_sprm::LN_CFEmboss: /*sprmCFEmboss*/
                        rContext->Insert(ePropertyId,
                                         uno::makeAny( nIntValue ? awt::FontRelief::EMBOSSED : awt::FontRelief::NONE ));
                    break;

                }
            }
        }
        break;
    case NS_sprm::LN_CKul: // sprmCKul
        {
            // Parameter:  0 = none,    1 = single,  2 = by Word,
            // 3 = double,  4 = dotted,  5 = hidden
            // 6 = thick,   7 = dash,    8 = dot(not used)
            // 9 = dotdash 10 = dotdotdash 11 = wave
            handleUnderlineType(nIntValue, rContext);
        }
        break;
    case NS_sprm::LN_CLid:
        break;  // sprmCLid
    case NS_sprm::LN_CIco:
        {
            sal_Int32 nColor = 0;
            if (getColorFromIndex(nIntValue, nColor))
                rContext->Insert(PROP_CHAR_COLOR, uno::makeAny( nColor ) );
        }
        break;  // sprmCIco
    case NS_sprm::LN_CHpsBi:    // sprmCHpsBi
    case NS_sprm::LN_CHps:    // sprmCHps
        {
            //multiples of half points (12pt == 24)
            double fVal = double(nIntValue) / 2.;
            uno::Any aVal = uno::makeAny( fVal );
            if( NS_sprm::LN_CHpsBi == nSprmId )
            {
                rContext->Insert( PROP_CHAR_HEIGHT_COMPLEX, aVal );
            }
            else
            {
                bool bIgnore = false;
                if (m_pImpl->m_bInTableStyleRunProps)
                {
                    // If the default para style contains PROP_CHAR_HEIGHT, that should have priority over the table style.
                    StyleSheetEntryPtr pTable = m_pImpl->GetStyleSheetTable()->FindDefaultParaStyle();
                    if (pTable && pTable->pProperties->find(PROP_CHAR_HEIGHT) != pTable->pProperties->end())
                        bIgnore = true;
                }
                if (!bIgnore)
                {
                    //Asian get the same value as Western
                    rContext->Insert( PROP_CHAR_HEIGHT, aVal );
                    rContext->Insert( PROP_CHAR_HEIGHT_ASIAN, aVal );

                    uno::Reference<beans::XPropertySet> xCharStyle(m_pImpl->GetCurrentNumberingCharStyle());
                    if (xCharStyle.is())
                        xCharStyle->setPropertyValue(rPropNameSupplier.GetName(PROP_CHAR_HEIGHT), aVal);
                }
            }
            // Make sure char sizes defined in the stylesheets don't affect char props from direct formatting.
            if (!m_pImpl->IsStyleSheetImport())
                m_pImpl->deferCharacterProperty( nSprmId, uno::makeAny( nIntValue ));
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, (nSprmId == NS_sprm::LN_CHps ? OUString("sz") : OUString("szCs")), OUString::number(nIntValue));
        }
        break;
    case NS_sprm::LN_CHpsPos:
        // The spec says 0 is the same as the lack of the value, so don't parse that.
        if (nIntValue)
            m_pImpl->deferCharacterProperty( nSprmId, uno::makeAny( nIntValue ));
        break;  // sprmCHpsPos
    case 71 : //"sprmCDxaSpace"
    case 96 : //"sprmCDxaSpace"
    case NS_sprm::LN_CDxaSpace:  // sprmCDxaSpace
        {
            //Kerning half point values
            //TODO: there are two kerning values -
            // in ww8par6.cxx NS_sprm::LN_CHpsKern is used as boolean AutoKerning
            sal_Int16 nResult = static_cast<sal_Int16>(ConversionHelper::convertTwipToMM100(nIntValue));
            if (m_pImpl->IsInComments())
            {
                nResult = static_cast<sal_Int16>(nIntValue);
            }
            rContext->Insert(PROP_CHAR_CHAR_KERNING, uno::makeAny(nResult));
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "spacing", OUString::number(nIntValue));
        }
        break;
    case NS_sprm::LN_CHpsKern:  // sprmCHpsKern    auto kerning is bound to a minimum font size in Word - but not in Writer :-(
        rContext->Insert(PROP_CHAR_AUTO_KERNING, uno::makeAny( sal_Bool(nIntValue) ) );
        break;
    case NS_sprm::LN_CRgFtc0:  // sprmCRgFtc0     //ascii font index
    case NS_sprm::LN_CRgFtc1:  // sprmCRgFtc1     //Asian font index
    case NS_sprm::LN_CRgFtc2:  // sprmCRgFtc2     //CTL font index
        {
            FontTablePtr pFontTable = m_pImpl->GetFontTable();
            if(nIntValue >= 0 && pFontTable->size() > sal_uInt32(nIntValue))
            {
                PropertyIds eFontName    = PROP_CHAR_FONT_NAME;
                PropertyIds eFontStyle   = PROP_CHAR_FONT_STYLE;
                PropertyIds eFontFamily  = PROP_CHAR_FONT_FAMILY;
                PropertyIds eFontCharSet = PROP_CHAR_FONT_CHAR_SET;
                PropertyIds eFontPitch   = PROP_CHAR_FONT_PITCH;
                switch(nSprmId)
                {
                case NS_sprm::LN_CRgFtc0:
                    //already initialized
                    break;
                case NS_sprm::LN_CRgFtc1:
                    eFontName =     PROP_CHAR_FONT_NAME_ASIAN;
                    eFontStyle =    PROP_CHAR_FONT_STYLE_ASIAN;
                    eFontFamily =   PROP_CHAR_FONT_FAMILY_ASIAN;
                    eFontCharSet =  PROP_CHAR_FONT_CHAR_SET_ASIAN;
                    eFontPitch =    PROP_CHAR_FONT_PITCH_ASIAN;
                    break;
                case NS_sprm::LN_CRgFtc2:
                    eFontName =     PROP_CHAR_FONT_NAME_COMPLEX;
                    eFontStyle =    PROP_CHAR_FONT_STYLE_COMPLEX;
                    eFontFamily =   PROP_CHAR_FONT_FAMILY_COMPLEX;
                    eFontCharSet =  PROP_CHAR_FONT_CHAR_SET_COMPLEX;
                    eFontPitch =    PROP_CHAR_FONT_PITCH_COMPLEX;
                    break;
                }
                (void)eFontFamily;
                (void)eFontStyle;
                const FontEntry::Pointer_t pFontEntry(pFontTable->getFontEntry(sal_uInt32(nIntValue)));
                rContext->Insert(eFontName, uno::makeAny( pFontEntry->sFontName  ));
                rContext->Insert(eFontCharSet, uno::makeAny( (sal_Int16)pFontEntry->nTextEncoding  ));
                rContext->Insert(eFontPitch, uno::makeAny( pFontEntry->nPitchRequest  ));
            }
        }
        break;
    case NS_sprm::LN_CCharScale:  // sprmCCharScale
        rContext->Insert(PROP_CHAR_SCALE_WIDTH,
                         uno::makeAny( sal_Int16(nIntValue) ));
        break;
    case NS_sprm::LN_CFImprint: // sprmCFImprint   1 or 0
        // FontRelief: NONE, EMBOSSED, ENGRAVED
        rContext->Insert(PROP_CHAR_RELIEF,
                         uno::makeAny( nIntValue ? awt::FontRelief::ENGRAVED : awt::FontRelief::NONE ));
        break;
    case NS_sprm::LN_CSfxText:
        // The file-format has many character animations. We have only
        // one, so we use it always. Suboptimal solution though.
        if (nIntValue)
            rContext->Insert(PROP_CHAR_FLASH, uno::makeAny( true ));
        else
            rContext->Insert(PROP_CHAR_FLASH, uno::makeAny( false ));
        break;  // sprmCSfxText
    case NS_sprm::LN_CFBiDi:
        break;  // sprmCFBiDi
    case NS_sprm::LN_CShd:
        {
            //contains fore color, back color and shadow percentage, results in a brush
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
            {
                CellColorHandlerPtr pCellColorHandler( new CellColorHandler );
                pCellColorHandler->setOutputFormat( CellColorHandler::Character );
                pProperties->resolve(*pCellColorHandler);
                rContext->InsertProps(pCellColorHandler->getProperties());
            }
            break;
        }
    case NS_sprm::LN_CLidBi:     // sprmCLidBi     language complex
    case NS_sprm::LN_CRgLid0:    // sprmCRgLid0    language Western
    case NS_sprm::LN_CRgLid1:    // sprmCRgLid1    language Asian
        {
            lang::Locale aLocale( LanguageTag( (LanguageType)nIntValue).getLocale());

            PropertyIds aPropId;
            switch (nSprmId)
            {
                case NS_sprm::LN_CRgLid0:
                    aPropId = PROP_CHAR_LOCALE;
                    break;
                case NS_sprm::LN_CRgLid1:
                    aPropId = PROP_CHAR_LOCALE_ASIAN;
                    break;
                default:
                    aPropId = PROP_CHAR_LOCALE_COMPLEX;
                    break;
            }

            rContext->Insert(aPropId, uno::makeAny( aLocale ) );
        }
        break;

    case NS_sprm::LN_SBkc: // sprmSBkc
        /* break type
          0 - No break
          1 - New Column
          2 - New page
          3 - Even page
          4 - odd page
        */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            // Ignore continuous section break at the end of the document, if the previous section had the same type as well
            // It makes the importer loose margin settings with no benefit
            SectionPropertyMap* pLastContext = m_pImpl->GetLastSectionContext();
            int nPrevBreakType = 0;
            bool bHasPrevSection = false;
            if (pLastContext)
            {
                bHasPrevSection = true;
                nPrevBreakType = pLastContext->GetBreakType();
            }
            if (m_pImpl->GetParaSectpr() || nIntValue != 0 || (bHasPrevSection && nPrevBreakType != nIntValue))
                pSectionContext->SetBreakType( nIntValue );
        }
        break;
    case NS_ooxml::LN_EG_SectPrContents_titlePg:
    {
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetTitlePage( nIntValue > 0 ? true : false );//section has title page
    }
    break;
    case 165:
    {
        //page height, rounded to default values, default: 0x3dc0 twip
        sal_Int32 nHeight = ConversionHelper::convertTwipToMM100( nIntValue );
        rContext->Insert( PROP_HEIGHT, uno::makeAny( PaperInfo::sloppyFitPageDimension( nHeight ) ) );
    }
    break;
    case 0x703a: //undocumented, grid related?
        OSL_FAIL( "TODO: not handled yet"); //nIntValue like 0x008a2373 ?
        break;
    case NS_sprm::LN_STextFlow:
    case NS_ooxml::LN_EG_SectPrContents_textDirection:
    {
        /* 0 HoriLR 1 Vert TR 2 Vert TR 3 Vert TT 4 HoriLT
            only 0 and 1 can be imported correctly
          */
        sal_Int16 nDirection = text::WritingMode_LR_TB;
        switch( nIntValue )
        {
            case 0:
            case 4:
                nDirection = text::WritingMode_LR_TB;
            break;
            case 1:
            case 2:
            case 3:
                nDirection = text::WritingMode_TB_RL;
            break;
            default:;
        }

        PropertyMap * pTargetContext = rContext.get();

        if (pSectionContext != NULL &&
            nSprmId == NS_ooxml::LN_EG_SectPrContents_textDirection)
        {
            pTargetContext = pSectionContext;
        }

        pTargetContext->Insert(PROP_WRITING_MODE, uno::makeAny( nDirection ) );
    }
    break;  // sprmSTextFlow
    case 164: // sprmTSetShd
    case NS_sprm::LN_TTextFlow:
        break;  // sprmTTextFlow
        // the following are not part of the official documentation
    case 0x6870: //TxtForeColor
        {
            //contains a color as 0xTTRRGGBB while SO uses 0xTTRRGGBB
            sal_Int32 nColor = ConversionHelper::ConvertColor(nIntValue);
            rContext->Insert(PROP_CHAR_COLOR, uno::makeAny( nColor ) );
        }
        break;
    case 0x6877: //underlining color
        {
            rContext->Insert(PROP_CHAR_UNDERLINE_HAS_COLOR, uno::makeAny( true ) );
            rContext->Insert(PROP_CHAR_UNDERLINE_COLOR, uno::makeAny( nIntValue ) );
        }
        break;
    case 0x6815:
        break; //undocumented
    case 0x6467:
        break; //undocumented
    case 0xF617:
        break; //undocumented
    case 0xd634: // sprmTNewSpacing - table spacing ( see WW8TabBandDesc::ProcessSpacing() )
        break;
    case 0x4888:
    case 0x6887:
        //properties of list levels - undocumented
        break;
    case 0xd234:
    case 0xd235:
    case 0xd236:
    case 0xd237:
        break;//undocumented section properties
    case NS_ooxml::LN_CT_Tabs_tab:
        resolveSprmProps(*this, rSprm);
        m_pImpl->IncorporateTabStop(m_pImpl->m_aCurrentTabStop);
        m_pImpl->m_aCurrentTabStop = DeletableTabStop();
    break;
    case NS_ooxml::LN_CT_PPrBase_tabs:
    {
        // Initialize tab stop vector from style sheet
        if( !m_pImpl->IsStyleSheetImport() )
        {
            uno::Any aValue = m_pImpl->GetPropertyFromStyleSheet(PROP_PARA_TAB_STOPS);
            uno::Sequence< style::TabStop > aStyleTabStops;
            if(aValue >>= aStyleTabStops)
            {
                m_pImpl->InitTabStopFromStyle( aStyleTabStops );
            }
        }
        resolveSprmProps(*this, rSprm);
        rContext->Insert(PROP_PARA_TAB_STOPS, uno::makeAny( m_pImpl->GetCurrentTabStopAndClear()));
    }
    break;

    case NS_ooxml::LN_CT_DocDefaults_pPrDefault:
    case NS_ooxml::LN_CT_DocDefaults_rPrDefault:
        GetStyleSheetTable()->sprm( rSprm );
    break;
    case NS_ooxml::LN_EG_RPrBase_bdr:
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
            {
                BorderHandlerPtr pBorderHandler( new BorderHandler( true ) );
                pProperties->resolve(*pBorderHandler);

                rContext->Insert( PROP_CHAR_TOP_BORDER, uno::makeAny( pBorderHandler->getBorderLine()));
                rContext->Insert( PROP_CHAR_BOTTOM_BORDER, uno::makeAny( pBorderHandler->getBorderLine()));
                rContext->Insert( PROP_CHAR_LEFT_BORDER, uno::makeAny( pBorderHandler->getBorderLine()));
                rContext->Insert( PROP_CHAR_RIGHT_BORDER, uno::makeAny( pBorderHandler->getBorderLine()));

                rContext->Insert( PROP_CHAR_TOP_BORDER_DISTANCE, uno::makeAny( pBorderHandler->getLineDistance()));
                rContext->Insert( PROP_CHAR_BOTTOM_BORDER_DISTANCE, uno::makeAny( pBorderHandler->getLineDistance()));
                rContext->Insert( PROP_CHAR_LEFT_BORDER_DISTANCE, uno::makeAny( pBorderHandler->getLineDistance()));
                rContext->Insert( PROP_CHAR_RIGHT_BORDER_DISTANCE, uno::makeAny( pBorderHandler->getLineDistance()));

                if( pBorderHandler->getShadow() )
                {
                    table::ShadowFormat aFormat = rContext->getShadowFromBorder(pBorderHandler->getBorderLine());
                    rContext->Insert(PROP_CHAR_SHADOW_FORMAT, uno::makeAny(aFormat));
                }
            }
        }
        break;
    case NS_ooxml::LN_CT_PPr_sectPr:
    case NS_ooxml::LN_EG_RPrBase_color:
    case NS_ooxml::LN_EG_RPrBase_rFonts:
    case NS_ooxml::LN_EG_RPrBase_eastAsianLayout:
    case NS_ooxml::LN_EG_RPrBase_u:
    case NS_ooxml::LN_EG_RPrBase_lang:
    case NS_ooxml::LN_CT_PPrBase_spacing:
    case NS_ooxml::LN_CT_PPrBase_ind:
    case NS_ooxml::LN_CT_RPrDefault_rPr:
    case NS_ooxml::LN_CT_PPrDefault_pPr:
    case NS_ooxml::LN_CT_Style_pPr:
    case NS_ooxml::LN_CT_Style_rPr:
    case NS_ooxml::LN_CT_PPr_rPr:
    case NS_ooxml::LN_CT_PPrBase_numPr:
    {
        bool bTempGrabBag = !m_pImpl->isInteropGrabBagEnabled();
        if (nSprmId == NS_ooxml::LN_CT_PPr_sectPr)
            m_pImpl->SetParaSectpr(true);
        else if (nSprmId == NS_ooxml::LN_EG_RPrBase_color && bTempGrabBag)
            // if DomainMapper grab bag is not enabled, enable it temporarily
            m_pImpl->enableInteropGrabBag("TempColorPropsGrabBag");
        resolveSprmProps(*this, rSprm);
        if (nSprmId == NS_ooxml::LN_CT_PPrBase_spacing)
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "spacing", m_pImpl->m_aSubInteropGrabBag);
        else if (nSprmId == NS_ooxml::LN_EG_RPrBase_rFonts)
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "rFonts", m_pImpl->m_aSubInteropGrabBag);
        else if (nSprmId == NS_ooxml::LN_EG_RPrBase_lang)
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "lang", m_pImpl->m_aSubInteropGrabBag);
        else if (nSprmId == NS_ooxml::LN_EG_RPrBase_color)
        {
            std::vector<beans::PropertyValue>::iterator aIter = m_pImpl->m_aSubInteropGrabBag.begin();
            for (; aIter != m_pImpl->m_aSubInteropGrabBag.end(); ++aIter)
            {
                if (aIter->Name == "val")
                    m_pImpl->GetTopContext()->Insert(PROP_CHAR_THEME_ORIGINAL_COLOR, aIter->Value, true, CHAR_GRAB_BAG);
                else if (aIter->Name == "themeColor")
                    m_pImpl->GetTopContext()->Insert(PROP_CHAR_THEME_COLOR, aIter->Value, true, CHAR_GRAB_BAG);
                else if (aIter->Name == "themeShade")
                    m_pImpl->GetTopContext()->Insert(PROP_CHAR_THEME_COLOR_SHADE, aIter->Value, true, CHAR_GRAB_BAG);
                else if (aIter->Name == "themeTint")
                    m_pImpl->GetTopContext()->Insert(PROP_CHAR_THEME_COLOR_TINT, aIter->Value, true, CHAR_GRAB_BAG);
            }
            if (bTempGrabBag)
                //disable and clear DomainMapper grab bag if it wasn't enabled before
                m_pImpl->disableInteropGrabBag();

            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "color", m_pImpl->m_aSubInteropGrabBag);
        }
        else if (nSprmId == NS_ooxml::LN_CT_PPrBase_ind)
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "ind", m_pImpl->m_aSubInteropGrabBag);
    }
    break;
    case NS_ooxml::LN_CT_PPrBase_wordWrap:
        m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "wordWrap", "");
    break;
    case NS_ooxml::LN_EG_SectPrContents_footnotePr:
    case NS_ooxml::LN_EG_SectPrContents_endnotePr:
        m_pImpl->SetInFootnoteProperties( NS_ooxml::LN_EG_SectPrContents_footnotePr == nSprmId );
        resolveSprmProps(*this, rSprm);
    break;
    case NS_ooxml::LN_EG_SectPrContents_lnNumType:
    {
        resolveSprmProps(*this, rSprm);
        LineNumberSettings aSettings = m_pImpl->GetLineNumberSettings();
        aSettings.bIsOn = true;
        m_pImpl->SetLineNumberSettings( aSettings );
        //apply settings at XLineNumberingProperties
        try
        {
            uno::Reference< text::XLineNumberingProperties > xLineNumberingProperties( m_pImpl->GetTextDocument(), uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xLineNumberingPropSet = xLineNumberingProperties->getLineNumberingProperties();
            PropertyNameSupplier& rNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
            xLineNumberingPropSet->setPropertyValue(rNameSupplier.GetName( PROP_IS_ON ), uno::makeAny(true) );
            if( aSettings.nInterval )
                xLineNumberingPropSet->setPropertyValue(rNameSupplier.GetName( PROP_INTERVAL ), uno::makeAny((sal_Int16)aSettings.nInterval) );
            if( aSettings.nDistance )
                xLineNumberingPropSet->setPropertyValue(rNameSupplier.GetName( PROP_DISTANCE ), uno::makeAny(aSettings.nDistance) );
            xLineNumberingPropSet->setPropertyValue(rNameSupplier.GetName( PROP_RESTART_AT_EACH_PAGE ), uno::makeAny(aSettings.bRestartAtEachPage) );
        }
        catch( const uno::Exception& )
        {
        }

    }
    break;
    case NS_ooxml::LN_CT_PPrBase_framePr:
    // Avoid frames if we're inside a structured document tag, would just cause outer tables fail to create.
    if (!m_pImpl->GetSdt())
    {
        PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH);
        if( pContext.get() )
        {
            ParagraphPropertyMap* pParaContext = dynamic_cast< ParagraphPropertyMap* >( pContext.get() );
            pParaContext->SetFrameMode();
        }
        else
        {
            //TODO: What about style sheet import of frame properties
        }
        resolveSprmProps(*this, rSprm);
    }
    break;
    case NS_ooxml::LN_EG_SectPrContents_pgSz:
        CT_PageSz.code = 0;
        {
            PaperInfo aLetter(PAPER_LETTER);
            CT_PageSz.w = aLetter.getWidth();
            CT_PageSz.h = aLetter.getHeight();
        }
        CT_PageSz.orient = false;
        resolveSprmProps(*this, rSprm);
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            pSectionContext->Insert( PROP_HEIGHT, uno::makeAny( CT_PageSz.h ) );
            pSectionContext->Insert( PROP_IS_LANDSCAPE, uno::makeAny( CT_PageSz.orient ));
            pSectionContext->Insert( PROP_WIDTH, uno::makeAny( CT_PageSz.w ) );
            pSectionContext->SetLandscape( CT_PageSz.orient );
        }
        break;

    case NS_ooxml::LN_EG_SectPrContents_pgMar:
        m_pImpl->InitPageMargins();
        resolveSprmProps(*this, rSprm);
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            const _PageMar& rPageMar = m_pImpl->GetPageMargins();
            pSectionContext->SetTopMargin( rPageMar.top );
            pSectionContext->SetRightMargin( rPageMar.right );
            pSectionContext->SetBottomMargin( rPageMar.bottom );
            pSectionContext->SetLeftMargin( rPageMar.left );
            pSectionContext->SetHeaderTop( rPageMar.header );
            pSectionContext->SetHeaderBottom( rPageMar.footer );
        }
        break;

    case NS_ooxml::LN_EG_SectPrContents_cols:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
        {

            SectionColumnHandlerPtr pSectHdl( new SectionColumnHandler );
            pProperties->resolve(*pSectHdl);
            if(pSectionContext)
            {
                if( pSectHdl->IsEqualWidth() )
                {
                    pSectionContext->SetEvenlySpaced( true );
                    pSectionContext->SetColumnCount( (sal_Int16) (pSectHdl->GetNum() - 1) );
                    pSectionContext->SetColumnDistance( pSectHdl->GetSpace() );
                    pSectionContext->SetSeparatorLine( pSectHdl->IsSeparator() );
                }
                else if( !pSectHdl->GetColumns().empty() )
                {
                    pSectionContext->SetEvenlySpaced( false );
                    pSectionContext->SetColumnDistance( pSectHdl->GetSpace() );
                    pSectionContext->SetColumnCount( (sal_Int16)(pSectHdl->GetColumns().size() -1));
                    std::vector<_Column>::const_iterator tmpIter = pSectHdl->GetColumns().begin();
                    for (; tmpIter != pSectHdl->GetColumns().end(); ++tmpIter)
                    {
                        pSectionContext->AppendColumnWidth( tmpIter->nWidth );
                        if ((tmpIter != pSectHdl->GetColumns().end() - 1) || (tmpIter->nSpace > 0))
                            pSectionContext->AppendColumnSpacing( tmpIter->nSpace );
                    }
                    pSectionContext->SetSeparatorLine( pSectHdl->IsSeparator() );
                }
                else if( pSectHdl->GetNum() > 0 )
                {
                    pSectionContext->SetColumnCount( (sal_Int16)pSectHdl->GetNum() - 1 );
                    pSectionContext->SetColumnDistance( pSectHdl->GetSpace() );
                    pSectionContext->SetSeparatorLine( pSectHdl->IsSeparator() );
                }
            }
        }
    }
    break;
    case NS_ooxml::LN_EG_SectPrContents_docGrid:
        resolveSprmProps(*this, rSprm);
    break;
    case NS_ooxml::LN_EG_SectPrContents_pgBorders:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get( ) && pSectionContext )
        {
            PageBordersHandlerPtr pHandler( new PageBordersHandler );
            pProperties->resolve( *pHandler );

            // Set the borders to the context and apply them to the styles
            pHandler->SetBorders( pSectionContext );
            pSectionContext->SetBorderParams( pHandler->GetDisplayOffset( ) );
        }
    }
    break;

    case NS_ooxml::LN_CT_PPrBase_snapToGrid:
        m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "snapToGrid", OUString::number(nIntValue));
    break;
    case NS_ooxml::LN_CT_PPrBase_pStyle:
    {
        m_pImpl->SetCurrentParaStyleId( sStringValue );
        StyleSheetTablePtr pStyleTable = m_pImpl->GetStyleSheetTable();
        const OUString sConvertedStyleName = pStyleTable->ConvertStyleName( sStringValue, true );
        if (m_pImpl->GetTopContext() && m_pImpl->GetTopContextType() != CONTEXT_SECTION)
            m_pImpl->GetTopContext()->Insert( PROP_PARA_STYLE_NAME, uno::makeAny( sConvertedStyleName ));
        //apply numbering to paragraph if it was set at the style, but only if the paragraph itself
        //does not specify the numbering
        if( rContext->find(PROP_NUMBERING_RULES) == rContext->end()) // !contains
        {
            const StyleSheetEntryPtr pEntry = pStyleTable->FindStyleSheetByISTD(sStringValue);
            OSL_ENSURE( pEntry.get(), "no style sheet found" );
            const StyleSheetPropertyMap* pStyleSheetProperties = dynamic_cast<const StyleSheetPropertyMap*>(pEntry ? pEntry->pProperties.get() : 0);

            if( pStyleSheetProperties && pStyleSheetProperties->GetListId() >= 0 )
            {
                rContext->Insert( PROP_NUMBERING_STYLE_NAME, uno::makeAny(
                            ListDef::GetStyleName( pStyleSheetProperties->GetListId( ) ) ), false);

                // We're inheriting properties from a numbering style. Make sure a possible right margin is inherited from the base style.
                sal_Int32 nParaRightMargin = 0;
                if (!pEntry->sBaseStyleIdentifier.isEmpty())
                {
                    const StyleSheetEntryPtr pParent = pStyleTable->FindStyleSheetByISTD(pEntry->sBaseStyleIdentifier);
                    const StyleSheetPropertyMap* pParentProperties = dynamic_cast<const StyleSheetPropertyMap*>(pParent ? pParent->pProperties.get() : 0);
                    if (pParentProperties && pParentProperties->find(PROP_PARA_RIGHT_MARGIN) != pParentProperties->end())
                        nParaRightMargin = pParentProperties->find(PROP_PARA_RIGHT_MARGIN)->second.getValue().get<sal_Int32>();
                }
                if (nParaRightMargin != 0)
                {
                    // If we're setting the right margin, we should set the first / left margin as well from the numbering style.
                    sal_Int32 nFirstLineIndent = lcl_getCurrentNumberingProperty(m_pImpl->GetCurrentNumberingRules(), pStyleSheetProperties->GetListLevel(), "FirstLineIndent");
                    sal_Int32 nParaLeftMargin = lcl_getCurrentNumberingProperty(m_pImpl->GetCurrentNumberingRules(), pStyleSheetProperties->GetListLevel(), "IndentAt");
                    if (nFirstLineIndent != 0)
                        rContext->Insert(PROP_PARA_FIRST_LINE_INDENT, uno::makeAny(nFirstLineIndent));
                    if (nParaLeftMargin != 0)
                        rContext->Insert(PROP_PARA_LEFT_MARGIN, uno::makeAny(nParaLeftMargin));

                    rContext->Insert(PROP_PARA_RIGHT_MARGIN, uno::makeAny(nParaRightMargin));
                }
            }

            if( pStyleSheetProperties && pStyleSheetProperties->GetListLevel() >= 0 )
                rContext->Insert( PROP_NUMBERING_LEVEL, uno::makeAny(pStyleSheetProperties->GetListLevel()), false);
        }
    }
    break;
    case NS_ooxml::LN_EG_RPrBase_rStyle:
        {
            OUString sConvertedName( m_pImpl->GetStyleSheetTable()->ConvertStyleName( sStringValue, true ) );
            // First check if the style exists in the document.
            StyleSheetEntryPtr pEntry = m_pImpl->GetStyleSheetTable( )->FindStyleSheetByStyleName( sConvertedName );
            bool bExists = pEntry.get( ) && ( pEntry->nStyleTypeCode == STYLE_TYPE_CHAR );

            // Add the property if the style exists
            if ( bExists && m_pImpl->GetTopContext() )
                m_pImpl->GetTopContext()->Insert( PROP_CHAR_STYLE_NAME, uno::makeAny( sConvertedName ) );
        }
    break;
    case NS_ooxml::LN_CT_TblPrBase_tblCellMar: //cell margins
    {
        resolveSprmProps(*this, rSprm);//contains LN_CT_TblCellMar_top, LN_CT_TblCellMar_left, LN_CT_TblCellMar_bottom, LN_CT_TblCellMar_right
    }
    break;
    case NS_ooxml::LN_CT_TblCellMar_top:
    case NS_ooxml::LN_CT_TblCellMar_start:
    case NS_ooxml::LN_CT_TblCellMar_left:
    case NS_ooxml::LN_CT_TblCellMar_bottom:
    case NS_ooxml::LN_CT_TblCellMar_end:
    case NS_ooxml::LN_CT_TblCellMar_right:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
        {
            MeasureHandlerPtr pMeasureHandler( new MeasureHandler );
            pProperties->resolve(*pMeasureHandler);
            sal_Int32 nMeasureValue = pMeasureHandler->getMeasureValue();
            PropertyIds eId = META_PROP_CELL_MAR_TOP;
            bool rtl = false; // TODO
            switch(nSprmId)
            {
                case NS_ooxml::LN_CT_TblCellMar_top:
                break;
                case NS_ooxml::LN_CT_TblCellMar_start:
                    eId = rtl ? META_PROP_CELL_MAR_RIGHT : META_PROP_CELL_MAR_LEFT;
                break;
                case NS_ooxml::LN_CT_TblCellMar_left:
                    eId = META_PROP_CELL_MAR_LEFT;
                break;
                case NS_ooxml::LN_CT_TblCellMar_bottom:
                    eId = META_PROP_CELL_MAR_BOTTOM;
                break;
                case NS_ooxml::LN_CT_TblCellMar_end:
                    eId = rtl ? META_PROP_CELL_MAR_LEFT : META_PROP_CELL_MAR_RIGHT;
                break;
                case NS_ooxml::LN_CT_TblCellMar_right:
                    eId = META_PROP_CELL_MAR_RIGHT;
                break;
                default:;
            }
            rContext->Insert( eId, uno::makeAny(nMeasureValue), false);
        }
    }
    break;
    case NS_sprm::LN_CFNoProof: //0x875 no grammar and spell checking, unsupported
    break;
    case NS_ooxml::LN_anchor_anchor: // at_character drawing
    case NS_ooxml::LN_inline_inline: // as_character drawing
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
        {
            GraphicImportType eGraphicType =
                (NS_ooxml::LN_anchor_anchor ==
                 sal::static_int_cast<Id>(nSprmId)) ?
                IMPORT_AS_DETECTED_ANCHOR :
                IMPORT_AS_DETECTED_INLINE;
            GraphicImportPtr pGraphicImport =
                m_pImpl->GetGraphicImport(eGraphicType);
            pProperties->resolve(*pGraphicImport);
            m_pImpl->ImportGraphic(pProperties, eGraphicType);
            if( !pGraphicImport->IsGraphic() )
            {
                m_pImpl->ResetGraphicImport();
                // todo: It's a shape, now start shape import
            }
        }
    }
    break;
    case NS_ooxml::LN_EG_RPrBase_vertAlign:
    {
        sal_Int16 nEscapement = 0;
        sal_Int8 nProp  = 58;
        if ( sStringValue == "superscript" )
                nEscapement = 101;
        else if ( sStringValue == "subscript" )
                nEscapement = -101;
        else
            nProp = 100;

        rContext->Insert(PROP_CHAR_ESCAPEMENT,         uno::makeAny( nEscapement ) );
        rContext->Insert(PROP_CHAR_ESCAPEMENT_HEIGHT,  uno::makeAny( nProp ) );
    }
    break;
    case NS_ooxml::LN_CT_FtnProps_pos:
    //footnotes in word can be at page end or beneath text - writer supports only the first
    //endnotes in word can be at section end or document end - writer supports only the latter
    // -> so this property can be ignored
    break;
    case NS_ooxml::LN_EG_FtnEdnNumProps_numStart:
    case NS_ooxml::LN_EG_FtnEdnNumProps_numRestart:
    case NS_ooxml::LN_CT_FtnProps_numFmt:
    case NS_ooxml::LN_CT_EdnProps_numFmt:
    {
        try
        {
            uno::Reference< beans::XPropertySet >  xFtnEdnSettings;
            if( m_pImpl->IsInFootnoteProperties() )
            {
                uno::Reference< text::XFootnotesSupplier> xFootnotesSupplier( m_pImpl->GetTextDocument(), uno::UNO_QUERY );
                if (xFootnotesSupplier.is())
                    xFtnEdnSettings = xFootnotesSupplier->getFootnoteSettings();
            }
            else
            {
                uno::Reference< text::XEndnotesSupplier> xEndnotesSupplier( m_pImpl->GetTextDocument(), uno::UNO_QUERY );
                if (xEndnotesSupplier.is())
                    xFtnEdnSettings = xEndnotesSupplier->getEndnoteSettings();
            }
            if( NS_ooxml::LN_EG_FtnEdnNumProps_numStart == nSprmId && xFtnEdnSettings.is())
            {
                xFtnEdnSettings->setPropertyValue(
                    PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_START_AT),
                                                                    uno::makeAny( sal_Int16( nIntValue - 1 )));
            }
            else if( NS_ooxml::LN_EG_FtnEdnNumProps_numRestart == nSprmId && xFtnEdnSettings.is())
            {
                sal_Int16 nFootnoteCounting = 0;
                switch (nIntValue)
                {
                    case NS_ooxml::LN_Value_ST_RestartNumber_continuous: nFootnoteCounting = text::FootnoteNumbering::PER_DOCUMENT; break;
                    case NS_ooxml::LN_Value_ST_RestartNumber_eachPage: nFootnoteCounting = text::FootnoteNumbering::PER_PAGE; break;
                    case NS_ooxml::LN_Value_ST_RestartNumber_eachSect: nFootnoteCounting = text::FootnoteNumbering::PER_CHAPTER; break;
                    default: break;
                }
                xFtnEdnSettings->setPropertyValue(
                        PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_FOOTNOTE_COUNTING ),
                        uno::makeAny( nFootnoteCounting ));
            }
            else if (xFtnEdnSettings.is())
            {
                sal_Int16 nNumType = ConversionHelper::ConvertNumberingType( nIntValue );
                xFtnEdnSettings->setPropertyValue(
                    PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_NUMBERING_TYPE),
                                                                    uno::makeAny( nNumType ));
            }
        }
        catch( const uno::Exception& )
        {
        }
    }
    break;
    case NS_ooxml::LN_paratrackchange:
        m_pImpl->StartParaMarkerChange( );
        /* fallthrough */
    case NS_ooxml::LN_CT_PPr_pPrChange:
    case NS_ooxml::LN_trackchange:
    case NS_ooxml::LN_EG_RPrContent_rPrChange:
    {
        HandleRedline( rSprm );
    }
    break;
    case NS_ooxml::LN_endtrackchange:
        m_pImpl->RemoveCurrentRedline( );
    break;
    case NS_ooxml::LN_CT_RPrChange_rPr:
        // Push all the current 'Character' properties to the stack, so that we don't store them
        // as 'tracked changes' by mistake
        m_pImpl->PushProperties(CONTEXT_CHARACTER);

        // Resolve all the properties that are under the 'rPrChange'->'rPr' XML node
        resolveSprmProps(*this, rSprm );

        if (m_pImpl->GetTopContext())
        {
            // Get all the properties that were processed in the 'rPrChange'->'rPr' XML node
            uno::Sequence< beans::PropertyValue > currentRedlineRevertProperties = m_pImpl->GetTopContext()->GetPropertyValues();

            // Store these properties in the current redline object
            m_pImpl->SetCurrentRedlineRevertProperties( currentRedlineRevertProperties );
        }

        // Pop back out the character properties that were on the run
        m_pImpl->PopProperties(CONTEXT_CHARACTER);
    break;
    case NS_ooxml::LN_CT_PPrChange_pPr:
        // Push all the current 'Paragraph' properties to the stack, so that we don't store them
        // as 'tracked changes' by mistake
        m_pImpl->PushProperties(CONTEXT_PARAGRAPH);

        // Resolve all the properties that are under the 'pPrChange'->'pPr' XML node
        resolveSprmProps(*this, rSprm );

        if (m_pImpl->GetTopContext())
        {
            // Get all the properties that were processed in the 'pPrChange'->'pPr' XML node
            uno::Sequence< beans::PropertyValue > currentRedlineRevertProperties = m_pImpl->GetTopContext()->GetPropertyValues();

            // Store these properties in the current redline object
            m_pImpl->SetCurrentRedlineRevertProperties( currentRedlineRevertProperties );
        }

        // Pop back out the character properties that were on the run
        m_pImpl->PopProperties(CONTEXT_PARAGRAPH);
    break;
    case NS_ooxml::LN_object:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get( ) )
        {
            OLEHandlerPtr pOLEHandler( new OLEHandler );
            pProperties->resolve(*pOLEHandler);
            if ( pOLEHandler->isOLEObject( ) )
            {
                OUString sStreamName = pOLEHandler->copyOLEOStream( m_pImpl->GetTextDocument() );
                if( !sStreamName.isEmpty() )
                {
                    m_pImpl->appendOLE( sStreamName, pOLEHandler );
                }
            }
        }
    }
    break;
    case NS_ooxml::LN_EG_HdrFtrReferences_headerReference: // header reference - not needed
    case NS_ooxml::LN_EG_HdrFtrReferences_footerReference: // footer reference - not needed
    break;
    case NS_ooxml::LN_EG_RPrBase_snapToGrid: // "Use document grid  settings for inter-paragraph spacing"
    break;
    case NS_ooxml::LN_CT_PPrBase_contextualSpacing:
        rContext->Insert(PROP_PARA_CONTEXT_MARGIN, uno::makeAny( sal_Bool( nIntValue ) ));
    break;
    case NS_ooxml::LN_CT_PPrBase_mirrorIndents: // mirrorIndents
        rContext->Insert(PROP_MIRROR_INDENTS, uno::makeAny(sal_Bool(nIntValue)), true, PARA_GRAB_BAG);
    break;
    case NS_ooxml::LN_EG_SectPrContents_formProt: //section protection, only form editing is enabled - unsupported
    case NS_ooxml::LN_EG_SectPrContents_vAlign:
    case NS_ooxml::LN_EG_RPrBase_fitText:
    break;
    case NS_ooxml::LN_ffdata:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties.get() != NULL)
        {
            FFDataHandler::Pointer_t pFFDataHandler(new FFDataHandler());

            pProperties->resolve(*pFFDataHandler);
            m_pImpl->SetFieldFFData(pFFDataHandler);
        }
    }
    break;
    case NS_ooxml::LN_CT_SdtPr_dropDownList:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties.get() != NULL)
            pProperties->resolve(*this);
    }
    break;
    case NS_ooxml::LN_CT_SdtDropDownList_listItem:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties.get() != NULL)
            pProperties->resolve(*this);
    }
    break;
    case NS_ooxml::LN_CT_SdtPr_date:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties.get() != NULL)
            pProperties->resolve(*this);
    }
    break;
    case NS_ooxml::LN_CT_SdtDate_dateFormat:
    {
        // See com/sun/star/awt/UnoControlDateFieldModel.idl, DateFormat; sadly there are no constants for this.
        if (sStringValue == "M/d/yyyy" || sStringValue == "M.d.yyyy")
            // Approximate with MM.dd.yyy
            m_pImpl->m_pSdtHelper->getDateFormat().reset(8);
        else
        {
            // Set default format, so at least the date picker is created.
            m_pImpl->m_pSdtHelper->getDateFormat().reset(0);
            SAL_WARN("writerfilter", "unhandled w:dateFormat value");
        }
    }
    break;
    case NS_ooxml::LN_EG_SectPrContents_pgNumType:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
        {
            pProperties->resolve(*this);
        }
    }
    break;
    case NS_ooxml::LN_tblStart:
        m_pImpl->m_nTableDepth++;
    break;
    case NS_ooxml::LN_tblEnd:
        m_pImpl->m_nTableDepth--;
    break;
    case NS_ooxml::LN_glow_glow:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
        {
            TextEffectsHandlerPtr pTextEffectsHandlerPtr( new TextEffectsHandler );
            sal_Bool bEnableTempGrabBag = !pTextEffectsHandlerPtr->isInteropGrabBagEnabled();
            if( bEnableTempGrabBag )
                pTextEffectsHandlerPtr->enableInteropGrabBag( "glow" );

            pProperties->resolve(*pTextEffectsHandlerPtr);

            rContext->Insert(PROP_CHAR_GLOW_TEXT_EFFECT,  pTextEffectsHandlerPtr->getInteropGrabBag().Value, true, CHAR_GRAB_BAG);
            if(bEnableTempGrabBag)
                pTextEffectsHandlerPtr->disableInteropGrabBag();
        }
    }
    break;
    default:
        {
#ifdef DEBUG_DOMAINMAPPER
            dmapper_logger->startElement("unhandled");
            dmapper_logger->attribute("id", nSprmId);
            dmapper_logger->attribute("name", rSprm.getName());
            dmapper_logger->endElement();
#endif
        }
    }
}

void DomainMapper::setInTableStyleRunProps(bool bInTableStyleRunProps)
{
    m_pImpl->m_bInTableStyleRunProps = bInTableStyleRunProps;
}

void DomainMapper::processDeferredCharacterProperties( const std::map< sal_Int32, uno::Any >& deferredCharacterProperties )
{
    assert( m_pImpl->GetTopContextType() == CONTEXT_CHARACTER );
    PropertyMapPtr rContext = m_pImpl->GetTopContext();
    for( std::map< sal_Int32, uno::Any >::const_iterator it = deferredCharacterProperties.begin();
         it != deferredCharacterProperties.end();
         ++it )
    {
        sal_Int32 Id = it->first;
        sal_Int32 nIntValue = 0;
        OUString sStringValue;
        it->second >>= nIntValue;
        it->second >>= sStringValue;
        switch( Id )
        {
        case NS_sprm::LN_CHps:
        case NS_sprm::LN_CHpsBi:
        break; // only for use by other properties, ignore here
        case NS_sprm::LN_CHpsPos:
        {
            sal_Int16 nEscapement = 0;
            sal_Int8 nProp  = 100;
            if(nIntValue == 0)
                nProp = 0;
            else
            {
                std::map< sal_Int32, uno::Any >::const_iterator font = deferredCharacterProperties.find( NS_sprm::LN_CHps );
                PropertyMapPtr pDefaultCharProps = m_pImpl->GetStyleSheetTable()->GetDefaultCharProps();
                PropertyMap::iterator aDefaultFont = pDefaultCharProps->find(PROP_CHAR_HEIGHT);
                if( font != deferredCharacterProperties.end())
                {
                    double fontSize = 0;
                    font->second >>= fontSize;
                    nEscapement = nIntValue * 100 / fontSize;
                }
                // TODO if not direct formatting, check the style first, not directly the default char props.
                else if (aDefaultFont != pDefaultCharProps->end())
                {
                    double fHeight = 0;
                    aDefaultFont->second.getValue() >>= fHeight;
                    // fHeight is in points, nIntValue is in half points, nEscapement is in percents.
                    nEscapement = nIntValue * 100 / fHeight / 2;
                }
                else
                { // TODO: Find out the font size. The 58/-58 values were here previous, but I have
                  // no idea what they are (they are probably some random guess that did fit whatever
                  // specific case somebody was trying to fix).
                    nEscapement = ( nIntValue > 0 ) ? 58: -58;
                }
            }
            rContext->Insert(PROP_CHAR_ESCAPEMENT,         uno::makeAny( nEscapement ) );
            rContext->Insert(PROP_CHAR_ESCAPEMENT_HEIGHT,  uno::makeAny( nProp ) );
        }
        break;  // sprmCHpsPos
        default:
            SAL_WARN( "writerfilter", "Unhandled property in processDeferredCharacterProperty()" );
            break;
        }
    }
}

void DomainMapper::lcl_entry(int /*pos*/,
                         writerfilter::Reference<Properties>::Pointer_t ref)
{
    ref->resolve(*this);
}

void DomainMapper::data(const sal_uInt8* /*buf*/, size_t /*len*/,
                        writerfilter::Reference<Properties>::Pointer_t /*ref*/)
{
}

void DomainMapper::lcl_startSectionGroup()
{
    if (!m_pImpl->isInIndexContext())
    {
        m_pImpl->PushProperties(CONTEXT_SECTION);
    }
}

void DomainMapper::lcl_endSectionGroup()
{
    if (!m_pImpl->isInIndexContext())
    {
        m_pImpl->CheckUnregisteredFrameConversion();
        m_pImpl->ExecuteFrameConversion();
        PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_SECTION);
        SectionPropertyMap* pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->CloseSectionGroup( *m_pImpl );
        m_pImpl->PopProperties(CONTEXT_SECTION);
    }
}

void DomainMapper::lcl_startParagraphGroup()
{
    m_pImpl->getTableManager().startParagraphGroup();
    /*
     * Add new para properties only if paragraph is not split
     * or the top context is not of paragraph properties
     * Set mbIsSplitPara to false as it has been handled
     */
    if (!mbIsSplitPara)
        m_pImpl->PushProperties(CONTEXT_PARAGRAPH);
    mbIsSplitPara = false;
    if (!(m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH) == m_pImpl->GetTopContext()))
        m_pImpl->PushProperties(CONTEXT_PARAGRAPH);

    static OUString sDefault("Standard");
    if (m_pImpl->GetTopContext())
    {
        if (!m_pImpl->IsInShape())
        {
            m_pImpl->GetTopContext()->Insert( PROP_PARA_STYLE_NAME, uno::makeAny( sDefault ) );
            m_pImpl->SetCurrentParaStyleId(sDefault);
        }
        if (m_pImpl->isBreakDeferred(PAGE_BREAK))
               m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, uno::makeAny( com::sun::star::style::BreakType_PAGE_BEFORE) );
        else if (m_pImpl->isBreakDeferred(COLUMN_BREAK))
            m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, uno::makeAny( com::sun::star::style::BreakType_COLUMN_BEFORE) );
    }
    m_pImpl->SetIsFirstRun(true);
    m_pImpl->clearDeferredBreaks();
}

void DomainMapper::lcl_endParagraphGroup()
{
    m_pImpl->PopProperties(CONTEXT_PARAGRAPH);
    m_pImpl->getTableManager().endParagraphGroup();
    //frame conversion has to be executed after table conversion
    m_pImpl->ExecuteFrameConversion();
}

void DomainMapper::markLastParagraphInSection( )
{
    m_pImpl->SetIsLastParagraphInSection( true );
}

void DomainMapper::lcl_startShape( uno::Reference< drawing::XShape > xShape )
{
    if (m_pImpl->GetTopContext())
    {
        // If there is a deferred page break, handle it now, so that the
        // started shape will be on the correct page.
        if (m_pImpl->isBreakDeferred(PAGE_BREAK))
        {
            m_pImpl->clearDeferredBreak(PAGE_BREAK);
            lcl_startCharacterGroup();
            sal_uInt8 sBreak[] = { 0xd };
            lcl_text(sBreak, 1);
            lcl_endCharacterGroup();
            lcl_endParagraphGroup();
            lcl_startParagraphGroup();
            m_pImpl->GetTopContext()->Insert(PROP_BREAK_TYPE, uno::makeAny( com::sun::star::style::BreakType_PAGE_BEFORE));
        }
        m_pImpl->PushShapeContext( xShape );
        lcl_startParagraphGroup();
    }
    else
        // No context? Then this image should not appear directly inside the
        // document, just save it for later usage.
        m_pImpl->PushPendingShape(xShape);
}

void DomainMapper::lcl_endShape( )
{
    if (m_pImpl->GetTopContext())
    {
        // End the current table, if there are any. Otherwise the unavoidable
        // empty paragraph at the end of the shape text will cause problems: if
        // the shape text ends with a table, the extra paragraph will be
        // handled as an additional row of the ending table.
        m_pImpl->getTableManager().endTable();

        lcl_endParagraphGroup();
        m_pImpl->PopShapeContext( );
    }
}

void DomainMapper::PushStyleSheetProperties( PropertyMapPtr pStyleProperties, bool bAffectTableMngr )
{
    m_pImpl->PushStyleProperties( pStyleProperties );
    if ( bAffectTableMngr )
        m_pImpl->getTableManager( ).SetStyleProperties( pStyleProperties );
}

void DomainMapper::PopStyleSheetProperties( bool bAffectTableMngr )
{
    m_pImpl->PopProperties( CONTEXT_STYLESHEET );
    if ( bAffectTableMngr )
    {
        PropertyMapPtr emptyPtr;
        m_pImpl->getTableManager( ).SetStyleProperties( emptyPtr );
    }
}

void DomainMapper::PushListProperties( ::boost::shared_ptr<PropertyMap> pListProperties )
{
    m_pImpl->PushListProperties( pListProperties );
}

void DomainMapper::PopListProperties()
{
    m_pImpl->PopProperties( CONTEXT_LIST );
}

void DomainMapper::lcl_startCharacterGroup()
{
    m_pImpl->PushProperties(CONTEXT_CHARACTER);
    if (m_pImpl->m_bFrameBtLr)
        // No support for this in core, work around by char rotation, as we do so for table cells already.
        m_pImpl->GetTopContext()->Insert(PROP_CHAR_ROTATION, uno::makeAny(sal_Int16(900)));
}

void DomainMapper::lcl_endCharacterGroup()
{
    m_pImpl->PopProperties(CONTEXT_CHARACTER);
}

void DomainMapper::lcl_text(const sal_uInt8 * data_, size_t len)
{
    //TODO: Determine the right text encoding (FIB?)
    OUString sText( (const sal_Char*) data_, len, RTL_TEXTENCODING_MS_1252 );
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("text");
    dmapper_logger->chars(sText);
    dmapper_logger->endElement();
#endif

    try
    {
        if(len == 1)
        {
            switch(*data_)
            {
                case 0x02: return; //footnote character
                case 0x0c: //page break
                    m_pImpl->deferBreak(PAGE_BREAK);
                    return;
                case 0x0e: //column break
                    m_pImpl->deferBreak(COLUMN_BREAK);
                    return;
                case 0x07:
                    m_pImpl->getTableManager().text(data_, len);
                case 0x0d:
                    m_pImpl->finishParagraph(m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH));
                    return;
                case 0x13:
                    m_pImpl->PushFieldContext();
                    return;
                case 0x14:
                    // delimiter not necessarily available
                    // appears only if field contains further content
                    m_pImpl->CloseFieldCommand();
                    return;
                case 0x15: /* end of field */
                    m_pImpl->PopFieldContext();
                    return;
                default:
                    break;
            }
        }

        PropertyMapPtr pContext = m_pImpl->GetTopContext();
    if ( pContext && !pContext->GetFootnote().is() )
    {
        if (m_pImpl->isBreakDeferred(PAGE_BREAK))
                m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, uno::makeAny( com::sun::star::style::BreakType_PAGE_BEFORE) );
            else if (m_pImpl->isBreakDeferred(COLUMN_BREAK))
                m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, uno::makeAny( com::sun::star::style::BreakType_COLUMN_BEFORE) );
            m_pImpl->clearDeferredBreaks();
        }

        if( pContext->GetFootnote().is() && m_pImpl->IsCustomFtnMark() )
        {
            pContext->GetFootnote()->setLabel( sText );
            m_pImpl->SetCustomFtnMark( false );
            //otherwise ignore sText
        }
        else if( m_pImpl->IsOpenFieldCommand() )
            m_pImpl->AppendFieldCommand(sText);
        else if( m_pImpl->IsOpenField() && m_pImpl->IsFieldResultAsString())
             /*depending on the success of the field insert operation this result will be
              set at the field or directly inserted into the text*/
            m_pImpl->SetFieldResult( sText );
        else
        {
            if (pContext == 0)
                pContext.reset(new PropertyMap());

            m_pImpl->appendTextPortion( sText, pContext );
        }
    }
    catch( const uno::RuntimeException& e )
    {
        SAL_WARN("writerfilter", "failed. Message :" << e.Message);
    }
}

void DomainMapper::lcl_positivePercentage(const OUString& rText)
{
    m_pImpl->m_aPositivePercentages.push(rText);
}

void DomainMapper::lcl_utext(const sal_uInt8 * data_, size_t len)
{
    OUString sText;
    OUStringBuffer aBuffer = OUStringBuffer(len);
    aBuffer.append( (const sal_Unicode *) data_, len);
    sText = aBuffer.makeStringAndClear();

    if (!m_pImpl->m_pSdtHelper->getDropDownItems().empty())
    {
        m_pImpl->m_pSdtHelper->getSdtTexts().append(sText);
        return;
    }
    else if (m_pImpl->m_pSdtHelper->getDateFormat())
    {
        /*
         * Here we assume w:sdt only contains a single text token. We need to
         * create the control early, as in Writer, it's part of the cell, but
         * in OOXML, the sdt contains the cell.
         */
        m_pImpl->m_pSdtHelper->createDateControl(sText);
        return;
    }
    else if (len == 1 && sText[0] == 0x03)
    {
        // This is the uFtnEdnSep, remember that the document has a separator.
        m_pImpl->m_bHasFtnSep = true;
        return;
    }
    else if (len == 1 && sText[0] == '\t' && m_pImpl->m_bIgnoreNextTab)
    {
        m_pImpl->m_bIgnoreNextTab = false;
        return;
    }

    try
    {
        m_pImpl->getTableManager().utext(data_, len);

        if(len == 1 && (sText[0] == 0x0d || sText[0] == 0x07))
        {
            if (m_pImpl->m_bIgnoreNextPara)
            {
                m_pImpl->m_bIgnoreNextPara = false;
                return;
            }
            PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH);
            if (pContext && m_pImpl->GetSettingsTable()->GetSplitPgBreakAndParaMark())
            {
                if (m_pImpl->isBreakDeferred(PAGE_BREAK))
                    pContext->Insert(PROP_BREAK_TYPE, uno::makeAny( com::sun::star::style::BreakType_PAGE_BEFORE));
                else if (m_pImpl->isBreakDeferred(COLUMN_BREAK))
                    pContext->Insert(PROP_BREAK_TYPE, uno::makeAny( com::sun::star::style::BreakType_COLUMN_BEFORE));
                m_pImpl->clearDeferredBreaks();
            }

            bool bSingleParagraph = m_pImpl->GetIsFirstParagraphInSection() && m_pImpl->GetIsLastParagraphInSection();
            // If the paragraph contains only the section properties and it has
            // no runs, we should not create a paragraph for it in Writer, unless that would remove the whole section.
            bool bRemove = !m_pImpl->GetParaChanged() && m_pImpl->GetParaSectpr() && !bSingleParagraph;
            m_pImpl->SetParaSectpr(false);
            m_pImpl->finishParagraph(m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH));
            if (bRemove)
                m_pImpl->RemoveLastParagraph();
        }
        else
        {

            PropertyMapPtr pContext = m_pImpl->GetTopContext();
            if ( pContext && !pContext->GetFootnote().is() )
            {
                if (m_pImpl->isBreakDeferred(PAGE_BREAK))
                    m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, uno::makeAny( com::sun::star::style::BreakType_PAGE_BEFORE) );
                else if (m_pImpl->isBreakDeferred(COLUMN_BREAK))
                {
                    if (!m_pImpl->IsFirstRun())
                    {
                        mbIsSplitPara = true;
                        m_pImpl->finishParagraph(m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH));
                        lcl_startParagraphGroup();
                    }
                    m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, uno::makeAny( com::sun::star::style::BreakType_COLUMN_BEFORE) );
                }
                m_pImpl->clearDeferredBreaks();
            }

            if( pContext && pContext->GetFootnote().is() )
            {
                if( !pContext->GetFootnoteSymbol() )
                    pContext->GetFootnote()->setLabel( sText );
                //otherwise ignore sText
            }
            else if( m_pImpl->IsOpenFieldCommand() )
                m_pImpl->AppendFieldCommand(sText);
            else if( m_pImpl->IsOpenField() && m_pImpl->IsFieldResultAsString())
                /*depending on the success of the field insert operation this result will be
                  set at the field or directly inserted into the text*/
                m_pImpl->SetFieldResult( sText );
            else
            {
                if (pContext == 0)
                    pContext.reset(new PropertyMap());

                m_pImpl->appendTextPortion( sText, pContext );
            }

        }
        m_pImpl->SetIsFirstRun(false);
    }
    catch( const uno::RuntimeException& )
    {
    }
}

void DomainMapper::lcl_props(writerfilter::Reference<Properties>::Pointer_t ref)
{
    string sType = ref->getType();
    if( sType == "PICF" )
    {
        m_pImpl->ImportGraphic(ref, IMPORT_AS_GRAPHIC);
    }
    else if( sType == "FSPA" )
    {
        m_pImpl->ImportGraphic(ref, IMPORT_AS_SHAPE);
    }
    else
        ref->resolve(*this);
}

void DomainMapper::lcl_table(Id name, writerfilter::Reference<Table>::Pointer_t ref)
{
    m_pImpl->SetAnyTableImport(true);
    switch(name)
    {
    case NS_ooxml::LN_FONTTABLE:

        // create a font table object that listens to the attributes
        // each entry call inserts a new font entry
        ref->resolve( *m_pImpl->GetFontTable() );
        break;
    case NS_ooxml::LN_STYLESHEET:
        //same as above to import style sheets
        m_pImpl->SetStyleSheetImport( true );
        ref->resolve( *m_pImpl->GetStyleSheetTable() );
        m_pImpl->GetStyleSheetTable()->ApplyStyleSheets(m_pImpl->GetFontTable());
        m_pImpl->SetStyleSheetImport( false );
        break;
    case NS_ooxml::LN_NUMBERING:
        {

            //the same for list tables
            ref->resolve( *m_pImpl->GetListTable() );
            m_pImpl->GetListTable( )->CreateNumberingRules( );
        }
        break;
    case NS_ooxml::LN_THEMETABLE:
        m_pImpl->GetThemeTable()->setThemeFontLangProperties(
                m_pImpl->GetSettingsTable()->GetThemeFontLangProperties() );
        ref->resolve ( *m_pImpl->GetThemeTable() );
    break;
    case NS_ooxml::LN_settings_settings:
        ref->resolve ( *m_pImpl->GetSettingsTable() );
        m_pImpl->ApplySettingsTable();
    break;
    default:
        OSL_FAIL( "which table is to be filled here?");
    }
    m_pImpl->SetAnyTableImport(false);
}

void DomainMapper::lcl_substream(Id rName, ::writerfilter::Reference<Stream>::Pointer_t ref)
{
    m_pImpl->appendTableManager( );
    // Appending a TableManager resets its TableHandler, so we need to append
    // that as well, or tables won't be imported properly in headers/footers.
    m_pImpl->appendTableHandler( );
    m_pImpl->getTableManager().startLevel();

    //import of page header/footer

    switch( rName )
    {
    case NS_ooxml::LN_headerl:

        m_pImpl->PushPageHeader(SectionPropertyMap::PAGE_LEFT);
        break;
    case NS_ooxml::LN_headerr:

        m_pImpl->PushPageHeader(SectionPropertyMap::PAGE_RIGHT);
        break;
    case NS_ooxml::LN_headerf:

        m_pImpl->PushPageHeader(SectionPropertyMap::PAGE_FIRST);
        break;
    case NS_ooxml::LN_footerl:

        m_pImpl->PushPageFooter(SectionPropertyMap::PAGE_LEFT);
        break;
    case NS_ooxml::LN_footerr:

        m_pImpl->PushPageFooter(SectionPropertyMap::PAGE_RIGHT);
        break;
    case NS_ooxml::LN_footerf:

        m_pImpl->PushPageFooter(SectionPropertyMap::PAGE_FIRST);
        break;
    case NS_ooxml::LN_footnote:
    case NS_ooxml::LN_endnote:
        m_pImpl->PushFootOrEndnote( NS_ooxml::LN_footnote == rName );
    break;
    case NS_ooxml::LN_annotation :
        m_pImpl->PushAnnotation();
    break;
    }
    ref->resolve(*this);
    switch( rName )
    {
    case NS_ooxml::LN_headerl:
    case NS_ooxml::LN_headerr:
    case NS_ooxml::LN_headerf:
    case NS_ooxml::LN_footerl:
    case NS_ooxml::LN_footerr:
    case NS_ooxml::LN_footerf:
        m_pImpl->PopPageHeaderFooter();
    break;
    case NS_ooxml::LN_footnote:
    case NS_ooxml::LN_endnote:
        m_pImpl->PopFootOrEndnote();
    break;
    case NS_ooxml::LN_annotation :
        m_pImpl->PopAnnotation();
    break;
    }

    m_pImpl->getTableManager().endLevel();
    m_pImpl->popTableManager( );
}

void DomainMapper::lcl_info(const string & /*info_*/)
{
}

void DomainMapper::handleUnderlineType(const sal_Int32 nIntValue, const ::boost::shared_ptr<PropertyMap> pContext)
{
    sal_Int16 eUnderline = awt::FontUnderline::NONE;

    switch(nIntValue)
    {
    case 0: eUnderline = awt::FontUnderline::NONE; break;
    case 2: pContext->Insert(PROP_CHAR_WORD_MODE, uno::makeAny( true ) ); // TODO: how to get rid of it?
    case 1: eUnderline = awt::FontUnderline::SINGLE;       break;
    case 3: eUnderline = awt::FontUnderline::DOUBLE;       break;
    case 4: eUnderline = awt::FontUnderline::DOTTED;       break;
    case 7: eUnderline = awt::FontUnderline::DASH;         break;
    case 9: eUnderline = awt::FontUnderline::DASHDOT;      break;
    case 10:eUnderline = awt::FontUnderline::DASHDOTDOT;   break;
    case 6: eUnderline = awt::FontUnderline::BOLD;         break;
    case 11:eUnderline = awt::FontUnderline::WAVE;         break;
    case 20:eUnderline = awt::FontUnderline::BOLDDOTTED;   break;
    case 23:eUnderline = awt::FontUnderline::BOLDDASH;     break;
    case 39:eUnderline = awt::FontUnderline::LONGDASH;     break;
    case 55:eUnderline = awt::FontUnderline::BOLDLONGDASH; break;
    case 25:eUnderline = awt::FontUnderline::BOLDDASHDOT;  break;
    case 26:eUnderline = awt::FontUnderline::BOLDDASHDOTDOT;break;
    case 27:eUnderline = awt::FontUnderline::BOLDWAVE;     break;
    case 43:eUnderline = awt::FontUnderline::DOUBLEWAVE;   break;
    default: ;
    }
    pContext->Insert(PROP_CHAR_UNDERLINE, uno::makeAny( eUnderline ) );
}

void DomainMapper::handleParaJustification(const sal_Int32 nIntValue, const ::boost::shared_ptr<PropertyMap> pContext, const bool bExchangeLeftRight)
{
    sal_Int16 nAdjust = 0;
    sal_Int16 nLastLineAdjust = 0;
    OUString aStringValue = "left";
    switch(nIntValue)
    {
    case 1:
        nAdjust = style::ParagraphAdjust_CENTER;
        aStringValue = "center";
        break;
    case 2:
        nAdjust = static_cast< sal_Int16 > (bExchangeLeftRight ? style::ParagraphAdjust_LEFT : style::ParagraphAdjust_RIGHT);
        aStringValue = "right";
        break;
    case 4:
        nLastLineAdjust = style::ParagraphAdjust_BLOCK;
        //no break;
    case 3:
        nAdjust = style::ParagraphAdjust_BLOCK;
        aStringValue = "both";
        break;
    case 0:
    default:
        nAdjust = static_cast< sal_Int16 > (bExchangeLeftRight ? style::ParagraphAdjust_RIGHT : style::ParagraphAdjust_LEFT);
        break;
    }
    pContext->Insert( PROP_PARA_ADJUST, uno::makeAny( nAdjust ) );
    pContext->Insert( PROP_PARA_LAST_LINE_ADJUST, uno::makeAny( nLastLineAdjust ) );
    m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "jc", aStringValue);
}

bool DomainMapper::getColorFromIndex(const sal_Int32 nIndex, sal_Int32 &nColor)
{
    nColor = 0;
    if ((nIndex < 1) || (nIndex > 16))
        return false;

    switch (nIndex)
    {
    case 1: nColor=0x000000; break; //black
    case 2: nColor=0x0000ff; break; //blue
    case 3: nColor=0x00ffff; break; //cyan
    case 4: nColor=0x00ff00; break; //green
    case 5: nColor=0xff00ff; break; //magenta
    case 6: nColor=0xff0000; break; //red
    case 7: nColor=0xffff00; break; //yellow
    case 8: nColor=0xffffff; break; //white
    case 9: nColor=0x000080;  break;//dark blue
    case 10: nColor=0x008080; break; //dark cyan
    case 11: nColor=0x008000; break; //dark green
    case 12: nColor=0x800080; break; //dark magenta
    case 13: nColor=0x800000; break; //dark red
    case 14: nColor=0x808000; break; //dark yellow
    case 15: nColor=0x808080; break; //dark gray
    case 16: nColor=0xC0C0C0; break; //light gray
    default:
        return false;
    }
    return true;
}

sal_Int16 DomainMapper::getEmphasisValue(const sal_Int32 nIntValue)
{
    switch (nIntValue)
    {
    case 1:
        return com::sun::star::text::FontEmphasis::DOT_ABOVE;
    case 2:
        return com::sun::star::text::FontEmphasis::ACCENT_ABOVE;
    case 3:
        return com::sun::star::text::FontEmphasis::CIRCLE_ABOVE;
    case 4:
        return com::sun::star::text::FontEmphasis::DOT_BELOW;
    default:
        return com::sun::star::text::FontEmphasis::NONE;
    }
}

OUString DomainMapper::getBracketStringFromEnum(const sal_Int32 nIntValue, const bool bIsPrefix)
{
    switch(nIntValue)
    {
    case 1:
        if (bIsPrefix)
            return OUString( "(" );
        return OUString( ")" );

    case 2:
        if (bIsPrefix)
            return OUString( "[" );
        return OUString( "]" );

    case 3:
        if (bIsPrefix)
            return OUString( "<" );
        return OUString( ">" );

    case 4:
        if (bIsPrefix)
            return OUString( "{" );
        return OUString( "}" );

    case 0:
    default:
        return OUString();
    }
}

com::sun::star::style::TabAlign DomainMapper::getTabAlignFromValue(const sal_Int32 nIntValue)
{
    switch (nIntValue)
    {
    case 0:
    case 4: // bar not supported
    case 5: // num not supported
        return com::sun::star::style::TabAlign_LEFT;
    case 1:
        return com::sun::star::style::TabAlign_CENTER;
    case 2:
        return com::sun::star::style::TabAlign_RIGHT;
    case 3:
        return com::sun::star::style::TabAlign_DECIMAL;
    }
    return com::sun::star::style::TabAlign_LEFT;
}

sal_Unicode DomainMapper::getFillCharFromValue(const sal_Int32 nIntValue)
{
    switch (nIntValue)
    {
    case 1: // dot
        return sal_Unicode(0x002e);
    case 2: // hyphen
        return sal_Unicode(0x002d);
    case 3: // underscore
    case 4: // heavy FIXME ???
        return sal_Unicode(0x005f);
    case NS_ooxml::LN_Value_ST_TabTlc_middleDot: // middleDot
        return sal_Unicode(0x00b7);
    case 0: // none
    default:
        return sal_Unicode(0x0020); // blank space
    }
}

bool DomainMapper::IsOOXMLImport() const
{
    return m_pImpl->IsOOXMLImport();
}

bool DomainMapper::IsRTFImport() const
{
    return m_pImpl->IsRTFImport();
}

uno::Reference < lang::XMultiServiceFactory > DomainMapper::GetTextFactory() const
{
    return m_pImpl->GetTextFactory();
}

uno::Reference< text::XTextRange > DomainMapper::GetCurrentTextRange()
{
    return m_pImpl->GetTopTextAppend()->getEnd();
}

OUString DomainMapper::getOrCreateCharStyle( PropertyValueVector_t& rCharProperties )
{
    StyleSheetTablePtr pStyleSheets = m_pImpl->GetStyleSheetTable();
    return pStyleSheets->getOrCreateCharStyle( rCharProperties );
}

StyleSheetTablePtr DomainMapper::GetStyleSheetTable( )
{
    return m_pImpl->GetStyleSheetTable( );
}

GraphicZOrderHelper* DomainMapper::graphicZOrderHelper()
{
    if( zOrderHelper.get() == NULL )
        zOrderHelper.reset( new GraphicZOrderHelper );
    return zOrderHelper.get();
}

uno::Reference<drawing::XShape> DomainMapper::PopPendingShape()
{
    return m_pImpl->PopPendingShape();
}

bool DomainMapper::IsInHeaderFooter() const
{
    return m_pImpl->IsInHeaderFooter();
}

void DomainMapper::enableInteropGrabBag(OUString aName)
{
    m_pImpl->m_aInteropGrabBagName = aName;
}

beans::PropertyValue DomainMapper::getInteropGrabBag()
{
    beans::PropertyValue aRet;
    aRet.Name = m_pImpl->m_aInteropGrabBagName;

    uno::Sequence<beans::PropertyValue> aSeq(m_pImpl->m_aInteropGrabBag.size());
    beans::PropertyValue* pSeq = aSeq.getArray();
    for (std::vector<beans::PropertyValue>::iterator i = m_pImpl->m_aInteropGrabBag.begin(); i != m_pImpl->m_aInteropGrabBag.end(); ++i)
        *pSeq++ = *i;

    m_pImpl->m_aInteropGrabBag.clear();
    m_pImpl->m_aInteropGrabBagName = "";
    aRet.Value = uno::makeAny(aSeq);
    return aRet;
}

uno::Sequence<beans::PropertyValue> DomainMapper::GetThemeFontLangProperties() const
{
    return m_pImpl->GetSettingsTable()->GetThemeFontLangProperties();
}

void DomainMapper::HandleRedline( Sprm& rSprm )
{
    sal_uInt32 nSprmId = rSprm.getId();

    m_pImpl->AddNewRedline( );

    if (nSprmId == NS_ooxml::LN_CT_PPr_pPrChange)
    {
        m_pImpl->SetCurrentRedlineToken(OOXML_ParagraphFormat);
    }
    else if (nSprmId == NS_ooxml::LN_CT_TrPr_ins)
    {
        m_pImpl->SetCurrentRedlineToken(OOXML_tableRowInsert);
    }
    else if (nSprmId == NS_ooxml::LN_CT_TrPr_del)
    {
        m_pImpl->SetCurrentRedlineToken(OOXML_tableRowDelete);
    }
    else if (nSprmId == NS_ooxml::LN_CT_TcPrBase_cellIns)
    {
        m_pImpl->SetCurrentRedlineToken(OOXML_tableCellInsert);
    }
    else if (nSprmId == NS_ooxml::LN_CT_TcPrBase_cellDel)
    {
        m_pImpl->SetCurrentRedlineToken(OOXML_tableCellDelete);
    }

    resolveSprmProps(*this, rSprm );
    // now the properties author, date and id should be available
    sal_Int32 nToken = m_pImpl->GetCurrentRedlineToken();
    switch( nToken & 0xffff )
    {
        case OOXML_mod:
        case OOXML_ins:
        case OOXML_del:
        case OOXML_ParagraphFormat:
        case OOXML_tableRowInsert:
        case OOXML_tableRowDelete:
        case OOXML_tableCellInsert:
        case OOXML_tableCellDelete:
            break;
        default: OSL_FAIL( "redline token other than mod, ins, del or table row" ); break;
    }
    m_pImpl->EndParaMarkerChange( );
}

} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
