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
#include "BorderHandler.hxx"
#include "PageBordersHandler.hxx"

#include "util.hxx"
#include "SdtHelper.hxx"
#include "TagLogger.hxx"
#include "TDefTableHandler.hxx"
#include "DomainMapper_Impl.hxx"
#include "ConversionHelper.hxx"
#include "ModelEventListener.hxx"
#include "MeasureHandler.hxx"
#include <i18nlangtag/languagetag.hxx>
#include <i18nutil/paper.hxx>
#include <ooxml/resourceids.hxx>
#include <oox/token/tokens.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XOOXMLDocumentPropertiesImporter.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
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
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/RubyPosition.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/types.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/sequence.hxx>
#include <editeng/escapementitem.hxx>
#include <filter/msfilter/util.hxx>
#include <sfx2/DocumentMetadataAccess.hxx>
#include <unotools/mediadescriptor.hxx>

#include "TextEffectsHandler.hxx"
#include "CellColorHandler.hxx"
#include "SectionColumnHandler.hxx"
#include "GraphicHelpers.hxx"
#include <dmapper/GraphicZOrderHelper.hxx>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>
#include <tools/UnitConversion.hxx>

using namespace ::com::sun::star;
using namespace oox;

namespace writerfilter::dmapper{

struct
{
    sal_Int32 h;
    bool      orient;
    sal_Int32 w;
} CT_PageSz;


DomainMapper::DomainMapper( const uno::Reference< uno::XComponentContext >& xContext,
                            uno::Reference<io::XInputStream> const& xInputStream,
                            uno::Reference<lang::XComponent> const& xModel,
                            bool bRepairStorage,
                            SourceDocumentType eDocumentType,
                            utl::MediaDescriptor const & rMediaDesc) :
    LoggedProperties("DomainMapper"),
    LoggedTable("DomainMapper"),
    LoggedStream("DomainMapper"),
    m_pImpl(new DomainMapper_Impl(*this, xContext, xModel, eDocumentType, rMediaDesc)),
    mbIsSplitPara(false),
    mbHasControls(false),
    mbWasShapeInPara(false)
{
    // #i24363# tab stops relative to indent
    m_pImpl->SetDocumentSettingsProperty(
        getPropertyName( PROP_TABS_RELATIVE_TO_INDENT ),
        uno::makeAny( false ) );
    m_pImpl->SetDocumentSettingsProperty(
        getPropertyName( PROP_SURROUND_TEXT_WRAP_SMALL ),
        uno::makeAny( true ) );
    m_pImpl->SetDocumentSettingsProperty(
        getPropertyName( PROP_APPLY_PARAGRAPH_MARK_FORMAT_TO_NUMBERING ),
        uno::makeAny( true ) );

    // Don't load the default style definitions to avoid weird mix
    m_pImpl->SetDocumentSettingsProperty("StylesNoDefault", uno::makeAny(true));
    m_pImpl->SetDocumentSettingsProperty("MsWordCompTrailingBlanks", uno::makeAny(true));
    m_pImpl->SetDocumentSettingsProperty("HeaderSpacingBelowLastPara",
                                         uno::makeAny(true));
    m_pImpl->SetDocumentSettingsProperty("FrameAutowidthWithMorePara", uno::makeAny(true));
    m_pImpl->SetDocumentSettingsProperty("FootnoteInColumnToPageEnd", uno::makeAny(true));

    m_pImpl->SetDocumentSettingsProperty("TabAtLeftIndentForParagraphsInList", uno::makeAny(true));

    // Initialize RDF metadata, to be able to add statements during the import.
    try
    {
        uno::Reference<rdf::XDocumentMetadataAccess> xDocumentMetadataAccess(xModel, uno::UNO_QUERY_THROW);
        uno::Reference<embed::XStorage> xStorage = comphelper::OStorageHelper::GetTemporaryStorage();
        OUString aBaseURL = rMediaDesc.getUnpackedValueOrDefault("URL", OUString());
        const uno::Reference<frame::XModel> xModel_(xModel,
            uno::UNO_QUERY_THROW);
        const uno::Reference<rdf::XURI> xBaseURI(sfx2::createBaseURI(xContext, xModel_, aBaseURL, u""));
        const uno::Reference<task::XInteractionHandler> xHandler;
        xDocumentMetadataAccess->loadMetadataFromStorage(xStorage, xBaseURI, xHandler);
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("writerfilter", "failed to initialize RDF metadata");
    }

    if (eDocumentType == SourceDocumentType::OOXML) {
        // tdf#108350
        // In Word since version 2007, the default document font is Calibri 11 pt.
        // If a DOCX document doesn't contain font information, we should assume
        // the intended font to provide best layout match.
        try
        {
            uno::Reference< beans::XPropertySet > xDefProps(GetTextFactory()->createInstance("com.sun.star.text.Defaults"),
                uno::UNO_QUERY_THROW);
            xDefProps->setPropertyValue(getPropertyName(PROP_CHAR_FONT_NAME), css::uno::Any(OUString("Calibri")));
            xDefProps->setPropertyValue(getPropertyName(PROP_CHAR_HEIGHT), css::uno::Any(double(11)));
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("writerfilter", "failed to initialize default font");
        }
    }

    //import document properties
    try
    {
        m_pImpl->m_xDocumentStorage = comphelper::OStorageHelper::GetStorageOfFormatFromInputStream(
            OFOPXML_STORAGE_FORMAT_STRING, xInputStream, xContext, bRepairStorage);

        uno::Reference< uno::XInterface > xTemp = xContext->getServiceManager()->createInstanceWithContext(
                                "com.sun.star.document.OOXMLDocumentPropertiesImporter",
                                xContext);

        uno::Reference< document::XOOXMLDocumentPropertiesImporter > xImporter( xTemp, uno::UNO_QUERY_THROW );
        uno::Reference< document::XDocumentPropertiesSupplier > xPropSupplier( xModel, uno::UNO_QUERY_THROW);
        xImporter->importProperties(m_pImpl->m_xDocumentStorage,
                                    xPropSupplier->getDocumentProperties());
    }
    catch( const uno::Exception& ) {}
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

        mbHasControls |= m_pImpl->m_pSdtHelper->hasElements();
        if ( nIndexes || mbHasControls )
        {
            //index update has to wait until first view is created
            uno::Reference< document::XEventBroadcaster > xBroadcaster(xIndexesSupplier, uno::UNO_QUERY);
            if (xBroadcaster.is())
                xBroadcaster->addEventListener(uno::Reference< document::XEventListener >(new ModelEventListener(nIndexes, mbHasControls)));
        }


        // Apply the document settings for both DOCX and RTF after everything else
        m_pImpl->GetSettingsTable()->ApplyProperties( m_pImpl->GetTextDocument( ) );

        // now that importing is finished, re-enable default styles for any that were never defined/imported.
        m_pImpl->SetDocumentSettingsProperty("StylesNoDefault", uno::makeAny(false));

        // Grab-bag handling
        comphelper::SequenceAsHashMap aProperties;

        // Add the saved w:themeFontLang setting
        aProperties["ThemeFontLangProps"] <<= m_pImpl->GetSettingsTable()->GetThemeFontLangProperties();

        // Add the saved compat settings
        aProperties["CompatSettings"] <<= m_pImpl->GetSettingsTable()->GetCompatSettings();

        // Add the saved DocumentProtection settings
        aProperties["DocumentProtection"] <<= m_pImpl->GetSettingsTable()->GetDocumentProtectionSettings();

        // Add the saved w:hyphenationZone setting
        aProperties["HyphenationZone"] <<= m_pImpl->GetSettingsTable()->GetHyphenationZone();

        // Add the saved w:doNotHyphenateCaps setting
        aProperties["NoHyphenateCaps"] <<= m_pImpl->GetSettingsTable()->GetNoHyphenateCaps();

        uno::Reference<beans::XPropertySet> xDocProps(m_pImpl->GetTextDocument(), uno::UNO_QUERY);
        if (xDocProps.is())
        {
            comphelper::SequenceAsHashMap aGrabBag(xDocProps->getPropertyValue("InteropGrabBag"));
            aGrabBag.update(aProperties);
            xDocProps->setPropertyValue("InteropGrabBag", uno::Any(aGrabBag.getAsConstPropertyValueList()));
        }

        // tdf#138782: for docs created in MS Word 2010 and older (compatibilityMode <= 14)
        m_pImpl->SetDocumentSettingsProperty(
            "AddFrameOffsets",
            uno::Any(14 >= m_pImpl->GetSettingsTable()->GetWordCompatibilityMode()));
    }
    catch( const uno::Exception& ) {}

#ifdef DBG_UTIL
    TagLogger::getInstance().endDocument();
#endif
}

void DomainMapper::lcl_attribute(Id nName, Value & val)
{
    if (m_pImpl->hasTableManager() && m_pImpl->getTableManager().attribute(nName, val))
        return;

    static const int nSingleLineSpacing = 240;
    sal_Int32 nIntValue = val.getInt();
    OUString sStringValue = val.getString();

    SectionPropertyMap * pSectionContext = m_pImpl->GetSectionContext();
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
            m_pImpl->SetBookmarkName( sStringValue );
        break;
        case NS_ooxml::LN_CT_MarkupRangeBookmark_id:
            // add a bookmark range -- this remembers a bookmark starting here
            // or, if the bookmark was already started or, if the bookmark was
            // already started before, writes out the bookmark
            m_pImpl->StartOrEndBookmark( sStringValue );
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
            m_pImpl->SetSymbolChar(nIntValue);
        break;
        case NS_ooxml::LN_CT_Sym_font:
            m_pImpl->SetSymbolFont(sStringValue);
        break;
        case NS_ooxml::LN_CT_Underline_val:
            if (m_pImpl->GetTopContext())
                handleUnderlineType(nIntValue, m_pImpl->GetTopContext());
            break;
        case NS_ooxml::LN_CT_Color_val:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COLOR, uno::makeAny( nIntValue ) );
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "val", msfilter::util::ConvertColorOU(Color(ColorTransparency,nIntValue)));
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
            {
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME, uno::makeAny( sStringValue ));
                if (m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH) && m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH)->isSet(PROP_NUMBERING_RULES))
                {
                    // Font of the paragraph mark should be used for the numbering as well.
                    uno::Reference<beans::XPropertySet> xCharStyle(m_pImpl->GetCurrentNumberingCharStyle());
                    if (xCharStyle.is())
                        xCharStyle->setPropertyValue("CharFontName", uno::makeAny(sStringValue));
                }
            }
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
                m_pImpl->GetTopContext()->Insert(
                    PROP_PARA_TOP_MARGIN,
                    uno::makeAny(static_cast<sal_Int32>(convertTwipToMm100(nIntValue))), false);
            break;
        case NS_ooxml::LN_CT_Spacing_beforeLines:
                m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "beforeLines", OUString::number(nIntValue));
                // We would need to make sure that this doesn't overwrite any
                // NS_ooxml::LN_CT_Spacing_before in parent styles before style
                // sheet support can be enabled.
                if (m_pImpl->GetTopContext() && !IsStyleSheetImport())
                    m_pImpl->GetTopContext()->Insert(PROP_PARA_TOP_MARGIN, uno::makeAny(ConversionHelper::convertTwipToMM100(nIntValue * nSingleLineSpacing / 100)), false);
            break;
        case NS_ooxml::LN_CT_Spacing_after:
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "after", OUString::number(nIntValue));
            if (m_pImpl->GetTopContext())
            {
                // Don't overwrite NS_ooxml::LN_CT_Spacing_afterAutospacing.
                m_pImpl->GetTopContext()->Insert(PROP_PARA_BOTTOM_MARGIN, uno::makeAny( ConversionHelper::convertTwipToMM100( nIntValue ) ), false);

                uno::Any aContextualSpacingFromStyle = m_pImpl->GetPropertyFromParaStyleSheet(PROP_PARA_CONTEXT_MARGIN);
                if (aContextualSpacingFromStyle.hasValue())
                    // Setting "after" spacing means Writer doesn't inherit
                    // contextual spacing anymore from style, but Word does.
                    m_pImpl->GetTopContext()->Insert(PROP_PARA_CONTEXT_MARGIN, aContextualSpacingFromStyle);
            }
            break;
        case NS_ooxml::LN_CT_Spacing_afterLines:
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "afterLines", OUString::number(nIntValue));
            // We would need to make sure that this doesn't overwrite any
            // NS_ooxml::LN_CT_Spacing_after in parent styles before style
            // sheet support can be enabled.
            if (m_pImpl->GetTopContext() && !IsStyleSheetImport())
                m_pImpl->GetTopContext()->Insert(PROP_PARA_BOTTOM_MARGIN, uno::makeAny(ConversionHelper::convertTwipToMM100(nIntValue * nSingleLineSpacing / 100)), false);
            break;
        case NS_ooxml::LN_CT_Spacing_line: //91434
        case NS_ooxml::LN_CT_Spacing_lineRule: //91435
        {
            style::LineSpacing aSpacing;
            PropertyMapPtr pTopContext = m_pImpl->GetTopContext();
            std::optional<PropertyMap::Property> aLineSpacingVal;
            if (pTopContext && (aLineSpacingVal = pTopContext->getProperty(PROP_PARA_LINE_SPACING)) )
            {
                aLineSpacingVal->second >>= aSpacing;
            }
            else
            {
                //default to single line spacing
                aSpacing.Mode = style::LineSpacingMode::FIX;
                aSpacing.Height = sal_Int16(ConversionHelper::convertTwipToMM100( nSingleLineSpacing ));
            }
            if( nName == NS_ooxml::LN_CT_Spacing_line )
            {
                m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "line", OUString::number(nIntValue));
                //now set the value depending on the Mode
                if( aSpacing.Mode == style::LineSpacingMode::PROP )
                    aSpacing.Height = sal_Int16(nIntValue * 100 / nSingleLineSpacing );
                else
                    aSpacing.Height = sal_Int16(ConversionHelper::convertTwipToMM100( nIntValue ));
            }
            else //NS_ooxml::LN_CT_Spacing_lineRule:
            {
                    // exactly, atLeast, auto
                    if( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_doc_ST_LineSpacingRule_auto)
                    {
                        m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "lineRule", "auto");
                        if (aSpacing.Height >= 0)
                        {
                            aSpacing.Mode = style::LineSpacingMode::PROP;
                            //reinterpret the already set value
                            aSpacing.Height = sal_Int16( aSpacing.Height * 100 /  ConversionHelper::convertTwipToMM100( nSingleLineSpacing ));
                        }
                        else
                        {
                            // Negative value still means a positive height,
                            // just the mode is "exact".
                            aSpacing.Mode = style::LineSpacingMode::FIX;
                            aSpacing.Height *= -1;
                        }
                    }
                    else if( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_doc_ST_LineSpacingRule_atLeast)
                    {
                        m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "lineRule", "atLeast");
                        aSpacing.Mode = style::LineSpacingMode::MINIMUM;
                    }
                    else // NS_ooxml::LN_Value_doc_ST_LineSpacingRule_exact
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
                sal_Int32 nIndentAt = m_pImpl->getCurrentNumberingProperty("IndentAt");

                sal_Int32 nParaLeftMargin = ConversionHelper::convertTwipToMM100(nIntValue);
                if (nParaLeftMargin != 0 && nIndentAt == nParaLeftMargin)
                    // Avoid direct left margin when it's the same as from the
                    // numbering.
                    break;

                if (nFirstLineIndent != 0)
                    m_pImpl->GetTopContext()->Insert(PROP_PARA_FIRST_LINE_INDENT, uno::makeAny(nFirstLineIndent), /*bOverwrite=*/false);

                m_pImpl->GetTopContext()->Insert(PROP_PARA_LEFT_MARGIN,
                                                 uno::makeAny(nParaLeftMargin));
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
                    m_pImpl->GetTopContext()->Insert(PROP_PARA_FIRST_LINE_INDENT, uno::makeAny(nFirstLineIndent), /*bOverwrite=*/false);
                if (nParaLeftMargin != 0)
                    m_pImpl->GetTopContext()->Insert(PROP_PARA_LEFT_MARGIN, uno::makeAny(nParaLeftMargin), /*bOverwrite=*/false);

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

                // See above, need to inherit left margin from list style when first is set.
                sal_Int32 nParaLeftMargin = m_pImpl->getCurrentNumberingProperty("IndentAt");
                if (nParaLeftMargin != 0)
                    m_pImpl->GetTopContext()->Insert(PROP_PARA_LEFT_MARGIN, uno::makeAny(nParaLeftMargin), /*bOverwrite=*/false);
            }
            break;
        case NS_ooxml::LN_CT_Ind_firstLine:
            if (m_pImpl->GetTopContext())
            {
                sal_Int32 nFirstLineIndent
                    = m_pImpl->getCurrentNumberingProperty("FirstLineIndent");
                sal_Int32 nParaFirstLineIndent = ConversionHelper::convertTwipToMM100(nIntValue);
                if (nParaFirstLineIndent != 0 && nFirstLineIndent == nParaFirstLineIndent)
                    // Avoid direct first margin when it's the same as from the
                    // numbering.
                    break;
                m_pImpl->GetTopContext()->Insert(PROP_PARA_FIRST_LINE_INDENT,
                                                 uno::makeAny(nParaFirstLineIndent));
            }
            break;
        case NS_ooxml::LN_CT_Ind_rightChars:
            m_pImpl->appendGrabBag(m_pImpl->m_aSubInteropGrabBag, "rightChars", OUString::number(nIntValue));
            break;

        case NS_ooxml::LN_CT_EastAsianLayout_id:
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_combine:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_IS_ON, uno::makeAny ( nIntValue != 0 ));
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
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_ROTATION_IS_FIT_TO_LINE, uno::makeAny ( nIntValue != 0 ));
            break;

        case NS_ooxml::LN_CT_PageSz_code:
            break;
        case NS_ooxml::LN_CT_PageSz_h:
            {
                sal_Int32 nHeight = ConversionHelper::convertTwipToMM100WithoutLimit(nIntValue);
                CT_PageSz.h = PaperInfo::sloppyFitPageDimension(nHeight);
            }
            break;
        case NS_ooxml::LN_CT_PageSz_orient:
            CT_PageSz.orient = (nIntValue != static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_PageOrientation_portrait));
            break;
        case NS_ooxml::LN_CT_PageSz_w:
            {
                sal_Int32 nWidth = ConversionHelper::convertTwipToMM100WithoutLimit(nIntValue);
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
            lang::Locale aLocale;
            if (sStringValue.getLength() <= 3 && sStringValue.getLength() >= 1)
            {
                // Cheesy Google Docs is known to tag language-only even for
                // "en" or others that need some region to distinguish language
                // variants for spell-checker and hyphenation. Obtain our known
                // fallback to clarify and match. The original value/context is
                // unknown anyway.
                LanguageTag aLanguageTag( sStringValue);
                aLanguageTag.makeFallback();
                if (aLanguageTag.getLanguage() == sStringValue)
                    aLocale = aLanguageTag.getLocale();
                else
                {
                    // Do not fallback for an unknown language, which usually
                    // results in "en-US", or any other non-matching case.
                    aLocale = LanguageTag::convertToLocale( sStringValue);
                }
            }
            else
            {
                aLocale = LanguageTag::convertToLocale( sStringValue);
            }
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(NS_ooxml::LN_CT_Language_val== nName ? PROP_CHAR_LOCALE :
                             NS_ooxml::LN_CT_Language_eastAsia == nName ? PROP_CHAR_LOCALE_ASIAN : PROP_CHAR_LOCALE_COMPLEX,
                             uno::makeAny( aLocale ) );
        }
        break;
        // See SwWW8ImplReader::GetParagraphAutoSpace() on why these are 100 and 280
        case NS_ooxml::LN_CT_Spacing_beforeAutospacing:
        {
            sal_Int32 default_spacing = -1;
            if (nIntValue)
            {
                m_pImpl->SetParaAutoBefore(true);

                default_spacing = 100;
                if (!m_pImpl->GetSettingsTable()->GetDoNotUseHTMLParagraphAutoSpacing())
                {
                    // 49 is just the old value that should be removed, once the
                    // root cause in SwTabFrm::MakeAll() is fixed.
                    if (m_pImpl->GetSettingsTable()->GetView() == NS_ooxml::LN_Value_doc_ST_View_web)
                        default_spacing = 49;
                    else
                        default_spacing = 280;
                }
                // required at export (here mainly for StyleSheets) to determine if the setting has changed from grab_bag
                m_pImpl->GetTopContext()->Insert(PROP_PARA_TOP_MARGIN, uno::makeAny(ConversionHelper::convertTwipToMM100(default_spacing)));
            }
            m_pImpl->GetTopContext()->Insert( PROP_PARA_TOP_MARGIN_BEFORE_AUTO_SPACING, uno::makeAny( ConversionHelper::convertTwipToMM100(default_spacing) ),true, PARA_GRAB_BAG );
        }
        break;
        case NS_ooxml::LN_CT_Spacing_afterAutospacing:
        {
            sal_Int32 default_spacing = -1;
            if  (nIntValue)
            {
                default_spacing = 100;
                if (!m_pImpl->GetSettingsTable()->GetDoNotUseHTMLParagraphAutoSpacing())
                {
                    if (m_pImpl->GetSettingsTable()->GetView() == NS_ooxml::LN_Value_doc_ST_View_web)
                        default_spacing = 49;
                    else
                        default_spacing = 280;
                }
                m_pImpl->GetTopContext()->Insert(PROP_PARA_BOTTOM_MARGIN, uno::makeAny(ConversionHelper::convertTwipToMM100(default_spacing)));
            }
            m_pImpl->GetTopContext()->Insert( PROP_PARA_BOTTOM_MARGIN_AFTER_AUTO_SPACING, uno::makeAny( ConversionHelper::convertTwipToMM100(default_spacing) ),true, PARA_GRAB_BAG );
        }
        break;
        case NS_ooxml::LN_CT_SmartTagRun_uri:
            m_pImpl->getSmartTagHandler().setURI(val.getString());
        break;
        case NS_ooxml::LN_CT_SmartTagRun_element:
            m_pImpl->getSmartTagHandler().setElement(val.getString());
        break;
        case NS_ooxml::LN_CT_Br_type :
            //TODO: attributes for break (0x12) are not supported
        break;
        case NS_ooxml::LN_CT_Fonts_hint :
            /*  assigns script type to ambiguous characters, values can be:
                NS_ooxml::LN_Value_ST_Hint_default
                NS_ooxml::LN_Value_ST_Hint_eastAsia
                NS_ooxml::LN_Value_ST_Hint_cs
             */
            //TODO: unsupported?
        break;
        case NS_ooxml::LN_CT_TblBorders_right:
        case NS_ooxml::LN_CT_TblBorders_top:
        case NS_ooxml::LN_CT_TblBorders_left:
        case NS_ooxml::LN_CT_TblBorders_bottom:
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
        case NS_ooxml::LN_Value_math_ST_Jc_centerGroup:
        case NS_ooxml::LN_Value_math_ST_Jc_center:
            m_pImpl->appendStarMath(val);
            m_pImpl->adjustLastPara(sal_Int8(style::ParagraphAdjust::ParagraphAdjust_CENTER));
            break;
        case NS_ooxml::LN_Value_math_ST_Jc_left:
            m_pImpl->appendStarMath(val);
            m_pImpl->adjustLastPara(sal_Int8(style::ParagraphAdjust::ParagraphAdjust_LEFT));
            break;
        case NS_ooxml::LN_Value_math_ST_Jc_right:
            m_pImpl->appendStarMath(val);
            m_pImpl->adjustLastPara(sal_Int8(style::ParagraphAdjust::ParagraphAdjust_RIGHT));
            break;
        case NS_ooxml::LN_starmath:
            m_pImpl->appendStarMath(val);
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
            ParagraphProperties* pParaProperties = nullptr;
            // handle frame properties at styles
            if( m_pImpl->GetTopContextType() == CONTEXT_STYLESHEET )
                pParaProperties = dynamic_cast< ParagraphProperties*>( m_pImpl->GetTopContextOfType( CONTEXT_STYLESHEET ).get() );
            else
                pParaProperties = dynamic_cast< ParagraphProperties*>( m_pImpl->GetTopContextOfType( CONTEXT_PARAGRAPH ).get() );

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
                            case  NS_ooxml::LN_Value_doc_ST_HAnchor_text:   //relative to column
                                nIntValue = text::RelOrientation::FRAME; break;
                            case  NS_ooxml::LN_Value_doc_ST_HAnchor_margin: nIntValue = text::RelOrientation::PAGE_PRINT_AREA; break;
                            case  NS_ooxml::LN_Value_doc_ST_HAnchor_page:   nIntValue = text::RelOrientation::PAGE_FRAME; break;
                            default:;
                        }
                        pParaProperties->SethAnchor( nIntValue );
                    break;
                    case NS_ooxml::LN_CT_FramePr_vAnchor:
                        switch(nIntValue)
                        {
                            case  NS_ooxml::LN_Value_doc_ST_VAnchor_text:  //relative to paragraph
                                    nIntValue = text::RelOrientation::FRAME; break;
                            case  NS_ooxml::LN_Value_doc_ST_VAnchor_margin:nIntValue = text::RelOrientation::PAGE_PRINT_AREA ; break;
                            case  NS_ooxml::LN_Value_doc_ST_VAnchor_page: nIntValue = text::RelOrientation::PAGE_FRAME; break;
                            default:;
                        }
                        pParaProperties->SetvAnchor( nIntValue );
                    break;
                    case NS_ooxml::LN_CT_FramePr_x:
                        pParaProperties->Setx( ConversionHelper::convertTwipToMM100(nIntValue ));
                        pParaProperties->SetxAlign( text::HoriOrientation::NONE );
                    break;
                    case NS_ooxml::LN_CT_FramePr_xAlign:
                        switch( nIntValue )
                        {
                            case  NS_ooxml::LN_Value_doc_ST_XAlign_center  : nIntValue = text::HoriOrientation::CENTER; break;
                            case  NS_ooxml::LN_Value_doc_ST_XAlign_right   : nIntValue = text::HoriOrientation::RIGHT; break;
                            case  NS_ooxml::LN_Value_doc_ST_XAlign_inside  : nIntValue = text::HoriOrientation::INSIDE; break;
                            case  NS_ooxml::LN_Value_doc_ST_XAlign_outside : nIntValue = text::HoriOrientation::OUTSIDE; break;
                            case  NS_ooxml::LN_Value_doc_ST_XAlign_left    : nIntValue = text::HoriOrientation::LEFT; break;
                            default:    nIntValue = text::HoriOrientation::NONE;
                        }
                        pParaProperties->SetxAlign( nIntValue );
                    break;
                    case NS_ooxml::LN_CT_FramePr_y:
                        pParaProperties->Sety( ConversionHelper::convertTwipToMM100(nIntValue ));
                        pParaProperties->SetyAlign( text::VertOrientation::NONE );
                    break;
                    case NS_ooxml::LN_CT_FramePr_yAlign:
                        switch( nIntValue )
                        {
                            case  NS_ooxml::LN_Value_doc_ST_YAlign_top     :
                            case  NS_ooxml::LN_Value_doc_ST_YAlign_inside  :nIntValue = text::VertOrientation::TOP; break;
                            case  NS_ooxml::LN_Value_doc_ST_YAlign_center  :nIntValue = text::VertOrientation::CENTER;break;
                            case  NS_ooxml::LN_Value_doc_ST_YAlign_bottom  :
                            case  NS_ooxml::LN_Value_doc_ST_YAlign_outside :nIntValue = text::VertOrientation::BOTTOM;break;
                            case  NS_ooxml::LN_Value_doc_ST_YAlign_inline  :
                            {
                            // HACK: This is for bnc#780851, where a table has one cell that has w:framePr,
                            // which causes that paragraph to be converted to a text frame, and the original
                            // paragraph object no longer exists, which makes table creation fail and furthermore
                            // it would be missing in the table layout anyway. So actually no letting that paragraph
                            // be a text frame "fixes" it. I'm not sure what "inline" is supposed to mean in practice
                            // anyway, so as long as this doesn't cause trouble elsewhere ...
                                PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH);
                                if( pContext )
                                {
                                    ParagraphPropertyMap* pParaContext = dynamic_cast< ParagraphPropertyMap* >( pContext.get() );
                                    if (pParaContext)
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
                            case NS_ooxml::LN_Value_doc_ST_HeightRule_exact:
                                nIntValue = text::SizeType::FIX;
                            break;
                            case NS_ooxml::LN_Value_doc_ST_HeightRule_atLeast:
                                nIntValue = text::SizeType::MIN;
                            break;
                            case NS_ooxml::LN_Value_doc_ST_HeightRule_auto:
                            //no break;
                            default:;
                                nIntValue = text::SizeType::VARIABLE;
                         }
                        pParaProperties->SethRule( nIntValue );
                    break;
                    case NS_ooxml::LN_CT_FramePr_wrap:
                    {
                        //should be either LN_Value_doc_ST_Wrap_notBeside or LN_Value_doc_ST_Wrap_around or LN_Value_doc_ST_Wrap_auto
                        OSL_ENSURE( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_doc_ST_Wrap_around ||
                                    sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_doc_ST_Wrap_notBeside ||
                                    sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_doc_ST_Wrap_through ||
                                    sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_doc_ST_Wrap_none ||
                                    sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_doc_ST_Wrap_auto,
                            "wrap not around, not_Beside, through, none or auto?");
                        if( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_doc_ST_Wrap_through ||
                            sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_doc_ST_Wrap_auto )
                            pParaProperties->SetWrap ( text::WrapTextMode_DYNAMIC ) ;
                        else if (sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_doc_ST_Wrap_around)
                            pParaProperties->SetWrap(text::WrapTextMode_PARALLEL);
                        else if (sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_doc_ST_Wrap_none)
                            pParaProperties->SetWrap ( text::WrapTextMode_THROUGH ) ;
                        else
                            pParaProperties->SetWrap ( text::WrapTextMode_NONE ) ;
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
                    aSettings.bRestartAtEachPage = nIntValue == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_LineNumberRestart_newPage);
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
            m_pImpl->StartCustomFootnote(m_pImpl->GetTopContext());
        break;
        case NS_ooxml::LN_CT_FtnEdnRef_id:
            // footnote or endnote reference id - not needed
        break;
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
            if (pSectionContext != nullptr)
            {
                switch( nIntValue )
                {
                    case NS_ooxml::LN_Value_doc_ST_DocGrid_default:
                        pSectionContext->SetGridType(text::TextGridMode::NONE);
                        break;
                    case NS_ooxml::LN_Value_doc_ST_DocGrid_lines:
                        pSectionContext->SetGridType(text::TextGridMode::LINES);
                        break;
                    case NS_ooxml::LN_Value_doc_ST_DocGrid_linesAndChars:
                        pSectionContext->SetGridType(text::TextGridMode::LINES_AND_CHARS);
                        pSectionContext->SetGridSnapToChars( false );
                        break;
                    case NS_ooxml::LN_Value_doc_ST_DocGrid_snapToChars:
                        pSectionContext->SetGridType(text::TextGridMode::LINES_AND_CHARS);
                        pSectionContext->SetGridSnapToChars( true );
                        break;
                    default :
                        OSL_FAIL("unknown SwTextGrid value");
                }
            }
        }
        break;
        case NS_ooxml::LN_CT_SdtBlock_sdtContent:
            m_pImpl->SetSdt(true);
        break;
        case NS_ooxml::LN_CT_SdtBlock_sdtEndContent:
            m_pImpl->SetSdt(false);

            // It's not possible to insert the relevant property to the character context here:
            // the previous, already sent character context may be still active, so the property would be lost.
            if (m_pImpl->m_pSdtHelper->isOutsideAParagraph())
                m_pImpl->setParaSdtEndDeferred(true);
            else
                m_pImpl->setSdtEndDeferred(true);

            switch (m_pImpl->m_pSdtHelper->getControlType())
            {
                case SdtControlType::dropDown:
                    m_pImpl->m_pSdtHelper->createDropDownControl();
                    break;
                case SdtControlType::datePicker:
                    m_pImpl->m_pSdtHelper->createDateContentControl();
                    break;
                case SdtControlType::unknown:
                default:;
            }
        break;
        case NS_ooxml::LN_CT_SdtListItem_displayText:
            // TODO handle when this is != value
        break;
        case NS_ooxml::LN_CT_SdtListItem_value:
            m_pImpl->m_pSdtHelper->getDropDownItems().push_back(sStringValue);
        break;
        case NS_ooxml::LN_CT_SdtDate_fullDate:
            m_pImpl->m_pSdtHelper->getDate().append(sStringValue);
        break;
        case NS_ooxml::LN_CT_Background_color:
            if (m_pImpl->GetSettingsTable()->GetDisplayBackgroundShape())
                m_pImpl->m_oBackgroundColor = nIntValue;
        break;
        case NS_ooxml::LN_CT_PageNumber_start:
            if (pSectionContext != nullptr && !m_pImpl->IsAltChunk())
                pSectionContext->SetPageNumber(nIntValue);
        break;
        case NS_ooxml::LN_CT_PageNumber_fmt:
            if (pSectionContext)
            {
                sal_Int16 nNumberType = ConversionHelper::ConvertNumberingType(nIntValue, -1);
                if (nNumberType != -1)
                    pSectionContext->SetPageNumberType(nNumberType);
            }
        break;
        case NS_ooxml::LN_CT_FtnEdn_type:
            // This is the "separator" footnote, ignore its linebreaks/text.
            if (static_cast<sal_uInt32>(nIntValue) == NS_ooxml::LN_Value_doc_ST_FtnEdn_separator)
                m_pImpl->SetSkipFootnoteState( SkipFootnoteSeparator::ON );
            else
                m_pImpl->SetSkipFootnoteState( SkipFootnoteSeparator::OFF );
        break;
        case NS_ooxml::LN_CT_FtnEdn_id:
        {
            SkipFootnoteSeparator eSkip = m_pImpl->GetSkipFootnoteState();
            if ( eSkip == SkipFootnoteSeparator::ON )
                m_pImpl->SetSkipFootnoteState( SkipFootnoteSeparator::SKIPPING );
            else if ( eSkip == SkipFootnoteSeparator::SKIPPING )
                m_pImpl->SetSkipFootnoteState( SkipFootnoteSeparator::OFF );
        }
        break;
        case NS_ooxml::LN_CT_DataBinding_prefixMappings:
            m_pImpl->m_pSdtHelper->setDataBindingPrefixMapping(sStringValue);
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "ooxml:CT_DataBinding_prefixMappings", sStringValue);
            break;
        case NS_ooxml::LN_CT_DataBinding_xpath:
            m_pImpl->m_pSdtHelper->setDataBindingXPath(sStringValue);
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "ooxml:CT_DataBinding_xpath", sStringValue);
            break;
        case NS_ooxml::LN_CT_DataBinding_storeItemID:
            m_pImpl->m_pSdtHelper->setDataBindingStoreItemID(sStringValue);
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "ooxml:CT_DataBinding_storeItemID", sStringValue);
            break;
        case NS_ooxml::LN_CT_SdtPlaceholder_docPart_val:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "ooxml:CT_SdtPlaceholder_docPart_val", sStringValue);
            break;
        case NS_ooxml::LN_CT_SdtColor_val:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "ooxml:CT_SdtColor_val", sStringValue);
            break;
        case NS_ooxml::LN_CT_SdtText_multiLine:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "ooxml:CT_SdtText_multiLine", sStringValue);
            break;
        case NS_ooxml::LN_CT_PTab_leader:
        case NS_ooxml::LN_CT_PTab_relativeTo:
        case NS_ooxml::LN_CT_PTab_alignment:
            break;
        case NS_ooxml::LN_CT_Cnf_lastRowLastColumn:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "lastRowLastColumn", OUString::number(nIntValue));
            break;
        case NS_ooxml::LN_CT_Cnf_lastRowFirstColumn:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "lastRowFirstColumn", OUString::number(nIntValue));
            break;
        case NS_ooxml::LN_CT_Cnf_firstRowLastColumn:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "firstRowLastColumn", OUString::number(nIntValue));
            break;
        case NS_ooxml::LN_CT_Cnf_oddHBand:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "oddHBand", OUString::number(nIntValue));
            break;
        case NS_ooxml::LN_CT_Cnf_firstRowFirstColumn:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "firstRowFirstColumn", OUString::number(nIntValue));
            break;
        case NS_ooxml::LN_CT_Cnf_evenVBand:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "evenVBand", OUString::number(nIntValue));
            break;
        case NS_ooxml::LN_CT_Cnf_evenHBand:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "evenHBand", OUString::number(nIntValue));
            break;
        case NS_ooxml::LN_CT_Cnf_lastColumn:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "lastColumn", OUString::number(nIntValue));
            break;
        case NS_ooxml::LN_CT_Cnf_firstColumn:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "firstColumn", OUString::number(nIntValue));
            break;
        case NS_ooxml::LN_CT_Cnf_oddVBand:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "oddVBand", OUString::number(nIntValue));
            break;
        case NS_ooxml::LN_CT_Cnf_lastRow:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "lastRow", OUString::number(nIntValue));
            break;
        case NS_ooxml::LN_CT_Cnf_firstRow:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "firstRow", OUString::number(nIntValue));
            break;
        case NS_ooxml::LN_CT_Cnf_val:
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "val", sStringValue);
            break;
        case NS_ooxml::LN_CT_DocPartName_val:
        {
            m_sGlossaryEntryName = sStringValue;
            break;
        }
        case NS_ooxml::LN_CT_DocPartGallery_val:
        {
            const OUString& sGlossaryEntryGallery = sStringValue;
            if(m_pImpl->GetTopContext())
            {
                OUString sName = sGlossaryEntryGallery + ":" + m_sGlossaryEntryName;
                // Add glossary entry name as a first paragraph in section
                m_pImpl->appendTextPortion(sName, m_pImpl->GetTopContext());
            }
            break;
        }
        case NS_ooxml::LN_CT_PermStart_ed:
        {
            m_pImpl->setPermissionRangeEd(sStringValue);
            break;
        }
        case NS_ooxml::LN_CT_PermStart_edGrp:
        {
            m_pImpl->setPermissionRangeEdGrp(sStringValue);
            break;
        }
        case NS_ooxml::LN_CT_PermStart_id:
        {
            m_pImpl->startOrEndPermissionRange(nIntValue);
            break;
        }
        case NS_ooxml::LN_CT_PermEnd_id:
        {
            m_pImpl->startOrEndPermissionRange(nIntValue);
            break;
        }
        case NS_ooxml::LN_CT_NumFmt_val:
        {
            try
            {
                uno::Reference<beans::XPropertySet> xFtnEdnSettings;
                if (m_pImpl->IsInFootnoteProperties())
                {
                    uno::Reference<text::XFootnotesSupplier> xFootnotesSupplier(
                        m_pImpl->GetTextDocument(), uno::UNO_QUERY);
                    if (xFootnotesSupplier.is())
                        xFtnEdnSettings = xFootnotesSupplier->getFootnoteSettings();
                }
                else
                {
                    uno::Reference<text::XEndnotesSupplier> xEndnotesSupplier(
                        m_pImpl->GetTextDocument(), uno::UNO_QUERY);
                    if (xEndnotesSupplier.is())
                        xFtnEdnSettings = xEndnotesSupplier->getEndnoteSettings();
                }
                if (xFtnEdnSettings.is())
                {
                    sal_Int16 nNumType = ConversionHelper::ConvertNumberingType(nIntValue);
                    xFtnEdnSettings->setPropertyValue(getPropertyName(PROP_NUMBERING_TYPE),
                                                      uno::makeAny(nNumType));
                }
            }
            catch (const uno::Exception&)
            {
            }
        }
        break;
        case NS_ooxml::LN_CT_AltChunk:
        {
            m_pImpl->HandleAltChunk(sStringValue);
        }
        break;
        case NS_ooxml::LN_AG_Parids_paraId:
            if (ParagraphPropertyMap* pParaContext
                = dynamic_cast<ParagraphPropertyMap*>(m_pImpl->GetTopContext().get()))
            {
                pParaContext->SetParaId(sStringValue);
            }
            break;
        default:
            SAL_WARN("writerfilter", "DomainMapper::lcl_attribute: unhandled token: " << nName);
    }
}

void DomainMapper::lcl_sprm(Sprm & rSprm)
{
    if (!m_pImpl->hasTableManager() || !m_pImpl->getTableManager().sprm(rSprm))
        sprmWithProps(rSprm, m_pImpl->GetTopContext());
}

// In rtl-paragraphs the meaning of left/right are to be exchanged
static bool ExchangeLeftRight(const PropertyMapPtr& rContext, DomainMapper_Impl& rImpl)
{
    bool bExchangeLeftRight = false;
    sal_Int32 aAdjust;
    uno::Any aPropPara = rImpl.GetAnyProperty(PROP_WRITING_MODE, rContext);
    if( (aPropPara >>= aAdjust) && aAdjust == text::WritingMode2::RL_TB )
        bExchangeLeftRight = true;
    return bExchangeLeftRight;
}

void DomainMapper::sprmWithProps( Sprm& rSprm, const PropertyMapPtr& rContext )
{
    // These SPRM's are not specific to any section, so it's expected that there is no context yet.
    switch (rSprm.getId())
    {
    case NS_ooxml::LN_background_background:
        return;
    default:
        break;
    }

    OSL_ENSURE(rContext, "PropertyMap has to be valid!");
    if(!rContext)
        return ;

    sal_uInt32 nSprmId = rSprm.getId();
    //needed for page properties
    SectionPropertyMap * pSectionContext = m_pImpl->GetSectionContext();
    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue->getInt();
    const OUString sStringValue = pValue->getString();

    switch(nSprmId)
    {
    case NS_ooxml::LN_CT_PPrBase_jc:
    {
        bool bExchangeLeftRight = !IsRTFImport() && ExchangeLeftRight(rContext, *m_pImpl);
        handleParaJustification(nIntValue, rContext, bExchangeLeftRight);
        break;
    }
    case NS_ooxml::LN_CT_PPrBase_keepLines:
        rContext->Insert(PROP_PARA_SPLIT, uno::makeAny(nIntValue == 0));
        break;
    case NS_ooxml::LN_CT_PPrBase_keepNext:
        rContext->Insert(PROP_PARA_KEEP_TOGETHER, uno::makeAny( nIntValue != 0 ) );
        break;
    case NS_ooxml::LN_CT_PPrBase_pageBreakBefore:
        rContext->Insert(PROP_BREAK_TYPE, uno::makeAny(nIntValue ? style::BreakType_PAGE_BEFORE : style::BreakType_NONE), /*bOverwrite=*/bool(nIntValue));
    break;
    case NS_ooxml::LN_CT_NumPr_ilvl:
            if (nIntValue < 0 || 10 <= nIntValue)
            {
                SAL_INFO("writerfilter",
                        "unsupported numbering level " << nIntValue);
                break;
            }
            if( IsStyleSheetImport() )
            {
                //style sheets cannot have a numbering rule attached
                StyleSheetPropertyMap* pStyleSheetPropertyMap = dynamic_cast< StyleSheetPropertyMap* >( rContext.get() );
                if (pStyleSheetPropertyMap)
                    pStyleSheetPropertyMap->SetListLevel( static_cast<sal_Int16>(nIntValue) );
            }
            // 0-8 are the 9 levels that Microsoft supports. (LO supports 10 levels).
            // 9 indicates "no numbering", for which LO has no corresponding concept,
            // and so it will be treated as the 10th level.
            // finishParagraph() will convert the 9 into "no numbering" for direct formatting.
            // (Styles only use this PROP for round-tripping and UI, but cannot trust it for import)
            if (!IsStyleSheetImport() || nIntValue != 9)
                rContext->Insert(PROP_NUMBERING_LEVEL, uno::makeAny(static_cast<sal_Int16>(nIntValue)));
        break;
    case NS_ooxml::LN_CT_NumPr_numId:
        {
            //convert the ListTable entry to a NumberingRules property and apply it
            ListsManager::Pointer pListTable = m_pImpl->GetListTable();
            ListDef::Pointer pList = pListTable->GetList( nIntValue );
            if( IsStyleSheetImport() )
            {
                //style sheets cannot have a numbering rule attached
                StyleSheetPropertyMap* pStyleSheetPropertyMap = dynamic_cast< StyleSheetPropertyMap* >( rContext.get() );
                if (pStyleSheetPropertyMap)
                    pStyleSheetPropertyMap->SetListId( nIntValue );
            }
            if( pList )
            {
                if( !IsStyleSheetImport() )
                {
                    uno::Any aRules = uno::makeAny( pList->GetNumberingRules( ) );
                    rContext->Insert( PROP_NUMBERING_RULES, aRules );
                    PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH);
                    if (pContext)
                    {
                        assert(dynamic_cast<ParagraphPropertyMap*>(pContext.get()));
                        static_cast<ParagraphPropertyMap*>(pContext.get())->SetListId(pList->GetId());
                    }

                    // Indentation can came from:
                    // 1) Paragraph style's numbering's indentation: the current non-style numId has priority over it.
                    // 2) Numbering's indentation: Writer handles that natively, so it should not be set on rContext.
                    // 3) Paragraph style's indentation: ditto.
                    // 4) Direct paragraph formatting: that will came later.
                    // So no situation where keeping indentation at this point would make sense -> erase.
                    rContext->Erase(PROP_PARA_FIRST_LINE_INDENT);
                    rContext->Erase(PROP_PARA_LEFT_MARGIN);
                    rContext->Erase(PROP_PARA_RIGHT_MARGIN);
                }
            }
            else
            {
                if( !IsStyleSheetImport() )
                {
                    // eg. disabled numbering using non-existent numId "0"
                    rContext->Insert( PROP_NUMBERING_STYLE_NAME, uno::makeAny( OUString() ) );
                    // disable inheritance of indentation of parent styles
                    rContext->Insert( PROP_PARA_LEFT_MARGIN, uno::makeAny( sal_Int32(0) ), /*bOverwrite=*/false);
                    rContext->Insert( PROP_PARA_FIRST_LINE_INDENT,
                                                 uno::makeAny( sal_Int32(0) ), /*bOverwrite=*/false);
                }
            }
        }
        break;
    case NS_ooxml::LN_CT_PPrBase_suppressLineNumbers:
        rContext->Insert(PROP_PARA_LINE_NUMBER_COUNT, uno::makeAny( nIntValue == 0 ) );
        break;
    case NS_ooxml::LN_inTbl:
        break;
    case NS_ooxml::LN_tblDepth:
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
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties )
        {
            auto pBorderHandler = std::make_shared<BorderHandler>( true );
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
                rContext->Insert( eBorderId, uno::makeAny( pBorderHandler->getBorderLine()) );
            if(eBorderDistId)
                rContext->Insert(eBorderDistId, uno::makeAny( pBorderHandler->getLineDistance()));
            if ( nSprmId == NS_ooxml::LN_CT_PBdr_right )
            {
                table::ShadowFormat aFormat;
                // Word only allows shadows on visible borders
                if ( pBorderHandler->getShadow() && pBorderHandler->getBorderLine().LineStyle != table::BorderLineStyle::NONE )
                    aFormat = writerfilter::dmapper::PropertyMap::getShadowFromBorder(pBorderHandler->getBorderLine());
                rContext->Insert(PROP_PARA_SHADOW_FORMAT, uno::makeAny(aFormat));
            }
        }
    }
    break;
    case NS_ooxml::LN_CT_PBdr_bar:
        break;
    case NS_ooxml::LN_CT_PPrBase_suppressAutoHyphens:
        rContext->Insert(PROP_PARA_IS_HYPHENATION, uno::makeAny( nIntValue == 0 ));
        break;
    case NS_ooxml::LN_CT_FramePr_h:
        break;
    case NS_ooxml::LN_CT_PrBase_shd:
    {
        //contains fore color, back color and shadow percentage, results in a brush
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties )
        {
            auto pCellColorHandler = std::make_shared<CellColorHandler>();
            pCellColorHandler->setOutputFormat( CellColorHandler::Paragraph );
            bool bEnableTempGrabBag = !pCellColorHandler->isInteropGrabBagEnabled();
            if( bEnableTempGrabBag )
                pCellColorHandler->enableInteropGrabBag( "TempShdPropsGrabBag" );

            pProperties->resolve(*pCellColorHandler);
            rContext->InsertProps(pCellColorHandler->getProperties().get());

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
    case NS_ooxml::LN_CT_PPrBase_widowControl:
    {
        uno::Any aVal( uno::makeAny( sal_Int8(nIntValue ? 2 : 0 )));
        rContext->Insert( PROP_PARA_WIDOWS, aVal );
        rContext->Insert( PROP_PARA_ORPHANS, aVal );
    }
    break;  // sprmPFWidowControl
    case NS_ooxml::LN_CT_PPrBase_overflowPunct:
        rContext->Insert(PROP_PARA_IS_HANGING_PUNCTUATION, uno::makeAny( nIntValue == 0 ));
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
                case NS_ooxml::LN_Value_doc_ST_TextAlignment_top:
                    nAlignment = 2;
                    break;
                case NS_ooxml::LN_Value_doc_ST_TextAlignment_center:
                    nAlignment = 3;
                    break;
                case NS_ooxml::LN_Value_doc_ST_TextAlignment_baseline:
                    nAlignment = 1;
                    break;
                case NS_ooxml::LN_Value_doc_ST_TextAlignment_bottom:
                    nAlignment = 4;
                    break;
                case NS_ooxml::LN_Value_doc_ST_TextAlignment_auto:
                default:
                    break;
            }
            rContext->Insert( PROP_PARA_VERT_ALIGNMENT, uno::makeAny( nAlignment) );
        }
        break;
    case NS_ooxml::LN_CT_PPrBase_textDirection:
        {
            switch (nIntValue)
            {
                case NS_ooxml::LN_Value_ST_TextDirection_tbRl:
                {
                    m_pImpl->SetFrameDirection(text::WritingMode2::TB_RL);
                    break;
                }
                case NS_ooxml::LN_Value_ST_TextDirection_btLr:
                {
                    m_pImpl->SetFrameDirection(text::WritingMode2::BT_LR);
                    break;
                }
                case NS_ooxml::LN_Value_ST_TextDirection_lrTbV:
                {
                    m_pImpl->SetFrameDirection(text::WritingMode2::LR_TB);
                    break;
                }
                case NS_ooxml::LN_Value_ST_TextDirection_tbRlV:
                {
                    m_pImpl->SetFrameDirection(text::WritingMode2::TB_RL);
                    break;
                }
                case NS_ooxml::LN_Value_ST_TextDirection_lrTb:
                case NS_ooxml::LN_Value_ST_TextDirection_tbLrV:
                default:
                    SAL_WARN("writerfilter", "DomainMapper::sprmWithProps: unhandled textDirection");
            }
        }
        break;
    case NS_ooxml::LN_CT_PPrBase_outlineLvl:
        {
            if (nIntValue < WW_OUTLINE_MIN || nIntValue > WW_OUTLINE_MAX)
                break; // invalid value is ignored by MS Word

            if( IsStyleSheetImport() )
            {
                StyleSheetPropertyMap* pStyleSheetPropertyMap = dynamic_cast< StyleSheetPropertyMap* >( rContext.get() );
                if (pStyleSheetPropertyMap)
                    pStyleSheetPropertyMap->SetOutlineLevel(nIntValue);
            }
            else
            {
                // convert MS body level (9) to LO body level (0) and equivalent outline levels
                sal_Int16 nLvl = nIntValue == WW_OUTLINE_MAX ? 0 : nIntValue + 1;
                rContext->Insert(PROP_OUTLINE_LEVEL, uno::makeAny ( nLvl ));
            }
        }
        break;
    case NS_ooxml::LN_CT_PPrBase_bidi:
        {
            // Four situations to handle:
            // 1.) bidi same as previous setting: no adjust change
            // 2.) no previous adjust: set appropriate default for this bidi
            // 3.) previous adjust and bidi different from previous: swap adjusts
            // 4.) previous adjust and no previous bidi: RTL swaps adjust

            const sal_Int16 nWritingMode = nIntValue ? text::WritingMode2::RL_TB : text::WritingMode2::LR_TB;
            sal_Int16 nParentBidi = -1;
            m_pImpl->GetPropertyFromParaStyleSheet(PROP_WRITING_MODE) >>= nParentBidi;
            // Paragraph justification reverses its meaning in an RTL context.
            // 1. Only make adjustments if the BiDi changes.
            if ( nParentBidi != nWritingMode && !IsRTFImport() )
            {
                style::ParagraphAdjust eAdjust = style::ParagraphAdjust(-1);
                // 2. no adjust property exists yet
                if ( !(m_pImpl->GetAnyProperty(PROP_PARA_ADJUST, rContext) >>= eAdjust) )
                {
                    // RTL defaults to right adjust
                    eAdjust = nIntValue ? style::ParagraphAdjust_RIGHT : style::ParagraphAdjust_LEFT;
                    rContext->Insert(PROP_PARA_ADJUST, uno::makeAny( eAdjust ), /*bOverwrite=*/false);
                }
                // 3,4. existing adjust: if RTL, then swap. If LTR, but previous was RTL, also swap.
                else if ( nIntValue || nParentBidi == sal_Int16(text::WritingMode2::RL_TB) )
                {
                    if ( eAdjust == style::ParagraphAdjust_RIGHT )
                        rContext->Insert(PROP_PARA_ADJUST, uno::makeAny( style::ParagraphAdjust_LEFT ));
                    else if ( eAdjust == style::ParagraphAdjust_LEFT )
                        rContext->Insert(PROP_PARA_ADJUST, uno::makeAny( style::ParagraphAdjust_RIGHT ));
                }
            }
            rContext->Insert(PROP_WRITING_MODE, uno::makeAny( nWritingMode ));
        }

        break;
    case NS_ooxml::LN_EG_SectPrContents_bidi:
        if (pSectionContext != nullptr)
        {
            const sal_Int16 writingMode = (nIntValue != 0) ? sal_Int16(text::WritingMode2::RL_TB) : sal_Int16(text::WritingMode2::LR_TB);
            pSectionContext->Insert(PROP_WRITING_MODE, uno::makeAny(writingMode));
        }
        break;
    case NS_ooxml::LN_EG_SectPrContents_rtlGutter:
        if (pSectionContext != nullptr)
        {
            bool bRtlGutter = nIntValue != 0;
            pSectionContext->Insert(PROP_RTL_GUTTER, uno::makeAny(bRtlGutter));
        }
        break;
    case NS_ooxml::LN_EG_RPrBase_highlight:
        {
            // MS Word completely ignores character highlighting in character styles.
            if ( IsStyleSheetImport() )
            {
                const StyleSheetEntryPtr pCurrStyle = GetStyleSheetTable()->GetCurrentEntry();
                if ( pCurrStyle && pCurrStyle->nStyleTypeCode == STYLE_TYPE_CHAR )
                    break;
            }

            // OOXML import uses an ID
            if( IsOOXMLImport() )
            {
                sal_Int32 nColor = 0;
                if( getColorFromId(nIntValue, nColor) )
                    rContext->Insert(PROP_CHAR_HIGHLIGHT, uno::makeAny( nColor ));
            }
            // RTF import uses the actual color value
            else if( IsRTFImport() )
            {
                rContext->Insert(PROP_CHAR_HIGHLIGHT, uno::makeAny( nIntValue ));
            }
        }
        break;
    case NS_ooxml::LN_EG_RPrBase_em:
        rContext->Insert(PROP_CHAR_EMPHASIS, uno::makeAny ( getEmphasisValue (nIntValue)));
        break;
    case NS_ooxml::LN_EG_RPrBase_emboss:
    case NS_ooxml::LN_EG_RPrBase_b:
    case NS_ooxml::LN_EG_RPrBase_bCs:
    case NS_ooxml::LN_EG_RPrBase_i:
    case NS_ooxml::LN_EG_RPrBase_iCs:
    case NS_ooxml::LN_EG_RPrBase_strike:
    case NS_ooxml::LN_EG_RPrBase_dstrike:
    case NS_ooxml::LN_EG_RPrBase_outline:
    case NS_ooxml::LN_EG_RPrBase_shadow:
    case NS_ooxml::LN_EG_RPrBase_caps:
    case NS_ooxml::LN_EG_RPrBase_smallCaps:
    case NS_ooxml::LN_EG_RPrBase_vanish:
    case NS_ooxml::LN_EG_RPrBase_webHidden:
        {
            PropertyIds ePropertyId = PROP_CHAR_WEIGHT; //initialized to prevent warning!
            switch( nSprmId )
            {
            case NS_ooxml::LN_EG_RPrBase_b:
            case NS_ooxml::LN_EG_RPrBase_bCs:
                ePropertyId = nSprmId != NS_ooxml::LN_EG_RPrBase_bCs ? PROP_CHAR_WEIGHT : PROP_CHAR_WEIGHT_COMPLEX;
                break;
            case NS_ooxml::LN_EG_RPrBase_i:
            case NS_ooxml::LN_EG_RPrBase_iCs:
                ePropertyId = nSprmId == NS_ooxml::LN_EG_RPrBase_i ? PROP_CHAR_POSTURE : PROP_CHAR_POSTURE_COMPLEX;
                break;
            case NS_ooxml::LN_EG_RPrBase_strike:
            case NS_ooxml::LN_EG_RPrBase_dstrike:
                ePropertyId = PROP_CHAR_STRIKEOUT;
                break;
            case NS_ooxml::LN_EG_RPrBase_outline:
                ePropertyId = PROP_CHAR_CONTOURED;
                break;
            case NS_ooxml::LN_EG_RPrBase_shadow:
                ePropertyId = PROP_CHAR_SHADOWED;
                break;
            case NS_ooxml::LN_EG_RPrBase_caps:
            case NS_ooxml::LN_EG_RPrBase_smallCaps:
                ePropertyId = PROP_CHAR_CASE_MAP;
                break;
            case NS_ooxml::LN_EG_RPrBase_vanish:
            case NS_ooxml::LN_EG_RPrBase_webHidden:
                ePropertyId = PROP_CHAR_HIDDEN;
                break;
            case NS_ooxml::LN_EG_RPrBase_emboss:
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
                    uno::Any aStyleVal = m_pImpl->GetPropertyFromParaStyleSheet(ePropertyId);
                    if( !aStyleVal.hasValue() )
                    {
                        nIntValue = NS_ooxml::LN_EG_RPrBase_smallCaps == nSprmId ?
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
                            (nStyleValue = static_cast<sal_Int16>(comphelper::getEnumAsINT32(aStyleVal))) >= 0 )
                    {
                        nIntValue = NS_ooxml::LN_EG_RPrBase_smallCaps == nSprmId ?
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
                    case NS_ooxml::LN_EG_RPrBase_b:
                    case NS_ooxml::LN_EG_RPrBase_bCs:
                    {
                        uno::Any aBold( uno::makeAny( nIntValue ? awt::FontWeight::BOLD : awt::FontWeight::NORMAL ) );

                        rContext->Insert(ePropertyId, aBold );
                        if( nSprmId != NS_ooxml::LN_EG_RPrBase_bCs )
                            rContext->Insert(PROP_CHAR_WEIGHT_ASIAN, aBold );

                        uno::Reference<beans::XPropertySet> xCharStyle(m_pImpl->GetCurrentNumberingCharStyle());
                        if (xCharStyle.is())
                            xCharStyle->setPropertyValue(getPropertyName(PROP_CHAR_WEIGHT), aBold);
                        if (nSprmId == NS_ooxml::LN_EG_RPrBase_b)
                            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "b", OUString::number(nIntValue));
                        else if (nSprmId == NS_ooxml::LN_EG_RPrBase_bCs)
                            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "bCs", OUString::number(nIntValue));
                    }
                    break;
                    case NS_ooxml::LN_EG_RPrBase_i:
                    case NS_ooxml::LN_EG_RPrBase_iCs:
                    {
                        uno::Any aPosture( uno::makeAny( nIntValue ? awt::FontSlant_ITALIC : awt::FontSlant_NONE ) );
                        rContext->Insert( ePropertyId, aPosture );
                        if (nSprmId != NS_ooxml::LN_EG_RPrBase_iCs)
                            rContext->Insert(PROP_CHAR_POSTURE_ASIAN, aPosture );
                        if (nSprmId == NS_ooxml::LN_EG_RPrBase_i)
                            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "i", OUString::number(nIntValue));
                    }
                    break;
                    case NS_ooxml::LN_EG_RPrBase_strike:
                        rContext->Insert(ePropertyId,
                                         uno::makeAny( nIntValue ? awt::FontStrikeout::SINGLE : awt::FontStrikeout::NONE ) );
                    break;
                    case NS_ooxml::LN_EG_RPrBase_dstrike:
                        rContext->Insert(ePropertyId,
                                         uno::makeAny( nIntValue ? awt::FontStrikeout::DOUBLE : awt::FontStrikeout::NONE ) );
                    break;
                    case NS_ooxml::LN_EG_RPrBase_outline:
                    case NS_ooxml::LN_EG_RPrBase_shadow:
                    case NS_ooxml::LN_EG_RPrBase_vanish:
                    case NS_ooxml::LN_EG_RPrBase_webHidden:
                        rContext->Insert(ePropertyId, uno::makeAny( nIntValue != 0 ));
                    break;
                    case NS_ooxml::LN_EG_RPrBase_smallCaps:
                        // If smallcaps would be just disabled and another casemap is already inserted, don't do anything.
                        if (nIntValue || !rContext->isSet(ePropertyId) )
                            rContext->Insert(ePropertyId, uno::makeAny( nIntValue ? style::CaseMap::SMALLCAPS : style::CaseMap::NONE));
                        m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "smallCaps", OUString::number(nIntValue));
                    break;
                    case NS_ooxml::LN_EG_RPrBase_caps:
                        rContext->Insert(ePropertyId,
                                         uno::makeAny( nIntValue ? style::CaseMap::UPPERCASE : style::CaseMap::NONE));
                        m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "caps", OUString::number(nIntValue));
                    break;
                    case NS_ooxml::LN_EG_RPrBase_emboss:
                        rContext->Insert(ePropertyId,
                                         uno::makeAny( nIntValue ? awt::FontRelief::EMBOSSED : awt::FontRelief::NONE ));
                    break;

                }
            }
        }
        break;
    case NS_ooxml::LN_EG_RPrBase_sz:
    case NS_ooxml::LN_EG_RPrBase_szCs:
        {
            //multiples of half points (12pt == 24)
            double fVal = double(nIntValue) / 2.;
            uno::Any aVal = uno::makeAny( fVal );
            if( NS_ooxml::LN_EG_RPrBase_szCs == nSprmId )
            {
                rContext->Insert( PROP_CHAR_HEIGHT_COMPLEX, aVal );
            }
            else
            {
                const   RubyInfo    &aInfo = m_pImpl->GetRubyInfo();
                if (aInfo.nSprmId == NS_ooxml::LN_CT_Ruby_rt && aInfo.nHps > 0 )
                {
                    fVal = double(aInfo.nHps) / 2.;
                    aVal <<= fVal;
                }
                else if (aInfo.nSprmId  == NS_ooxml::LN_CT_Ruby_rubyBase && aInfo.nHpsBaseText > 0 )
                {
                    fVal = double(aInfo.nHpsBaseText) / 2.;
                    aVal <<= fVal;
                }
                //Asian get the same value as Western
                rContext->Insert( PROP_CHAR_HEIGHT, aVal );
                rContext->Insert( PROP_CHAR_HEIGHT_ASIAN, aVal );

                uno::Reference<beans::XPropertySet> xCharStyle(m_pImpl->GetCurrentNumberingCharStyle());
                if (xCharStyle.is())
                    xCharStyle->setPropertyValue(getPropertyName(PROP_CHAR_HEIGHT), aVal);
            }
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, (nSprmId == NS_ooxml::LN_EG_RPrBase_sz ? OUString("sz") : OUString("szCs")), OUString::number(nIntValue));
        }
        break;
    case NS_ooxml::LN_EG_RPrBase_position:
        // The spec says 0 is the same as the lack of the value, so don't parse that.
        if ( nIntValue )
        {
            if (!IsStyleSheetImport() && !IsNumberingImport())
                m_pImpl->deferCharacterProperty( nSprmId, uno::makeAny( nIntValue ));
            else if (!m_pImpl->IsDocDefaultsImport())
            {
                // For some undocumented reason, MS Word seems to ignore this in docDefaults

                // DON'T FIXME: Truly calculating this for Character Styles will be tricky,
                // because it depends on the final fontsize - regardless of
                // where it is set. So at the style level,
                // the escapement value would need to be grabbagged.
                // At appendText time the final fontsize needs to be determined, and then
                // the escapement can be calculated from the grabbag'd half-point value
                // and directly applied. Yuck.
                // It seems best to just treat charstyle escapement like
                // pre-commit e70df84352d3670508a4666c97df44f82c1ce934
                // which just assigned default values and ignored the actual/given escapement.
                sal_Int16 nEscapement = nIntValue > 0 ? DFLT_ESC_AUTO_SUPER : DFLT_ESC_AUTO_SUB;
                sal_Int8 nProp = DFLT_ESC_PROP;
                rContext->Insert(PROP_CHAR_ESCAPEMENT, uno::makeAny( nEscapement ) );
                rContext->Insert(PROP_CHAR_ESCAPEMENT_HEIGHT, uno::makeAny( nProp ) );
            }
        }
        break;
    case NS_ooxml::LN_EG_RPrBase_spacing:
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
    case NS_ooxml::LN_EG_RPrBase_kern: // auto kerning is bound to a minimum font size in Word - but not in Writer :-(
        rContext->Insert(PROP_CHAR_AUTO_KERNING, uno::makeAny( nIntValue != 0 ) );
        break;
    case NS_ooxml::LN_EG_RPrBase_w:
        // ST_TextScale must fall between 1% and 600% according to spec, otherwise resets to 100% according to experience
        if ((1 <= nIntValue) && (nIntValue <= 600))
        {
            rContext->Insert(PROP_CHAR_SCALE_WIDTH,
                             uno::makeAny( sal_Int16(nIntValue) ));
        }
        else
        {
            rContext->Insert(PROP_CHAR_SCALE_WIDTH,
                             uno::makeAny( sal_Int16(100) ));
        }
        break;
    case NS_ooxml::LN_EG_RPrBase_imprint:
        // FontRelief: NONE, EMBOSSED, ENGRAVED
        rContext->Insert(PROP_CHAR_RELIEF,
                         uno::makeAny( nIntValue ? awt::FontRelief::ENGRAVED : awt::FontRelief::NONE ));
        break;
    case NS_ooxml::LN_EG_RPrBase_effect:
        // The file-format has many character animations. We have only
        // one, so we use it always. Suboptimal solution though.
        if (nIntValue != NS_ooxml::LN_Value_ST_TextEffect_none)
            rContext->Insert(PROP_CHAR_FLASH, uno::makeAny( true ));
        else
            rContext->Insert(PROP_CHAR_FLASH, uno::makeAny( false ));
        break;
    case NS_ooxml::LN_EG_RPrBase_rtl:
        break;
    case NS_ooxml::LN_EG_RPrBase_shd:
        {
            //contains fore color, back color and shadow percentage, results in a brush
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties )
            {
                auto pCellColorHandler = std::make_shared<CellColorHandler>();
                pCellColorHandler->setOutputFormat( CellColorHandler::Character );
                pProperties->resolve(*pCellColorHandler);
                rContext->InsertProps(pCellColorHandler->getProperties().get());
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_SHADING_MARKER, uno::makeAny(true), true, CHAR_GRAB_BAG );
            }
            break;
        }
    case NS_ooxml::LN_EG_SectPrContents_type:
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
            //continuous break only allowed if it is not the only section break
            SectionPropertyMap* pLastContext = m_pImpl->GetLastSectionContext();
            if ( nIntValue != static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_SectionMark_continuous) || pLastContext || m_pImpl->GetParaSectpr() )
                pSectionContext->SetBreakType( nIntValue );
        }
        break;
    case NS_ooxml::LN_EG_SectPrContents_titlePg:
    {
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetTitlePage( nIntValue > 0 );//section has title page
    }
    break;
    case 165:
    {
        //page height, rounded to default values, default: 0x3dc0 twip
        sal_Int32 nHeight = ConversionHelper::convertTwipToMM100( nIntValue );
        rContext->Insert( PROP_HEIGHT, uno::makeAny( PaperInfo::sloppyFitPageDimension( nHeight ) ) );
    }
    break;
    case NS_ooxml::LN_EG_SectPrContents_textDirection:
    {
        /* 0 HoriLR 1 Vert TR 2 Vert TR 3 Vert TT 4 HoriLT
            only 0 and 1 can be imported correctly
          */
        text::WritingMode nDirection = text::WritingMode_LR_TB;
        switch( nIntValue )
        {
            case NS_ooxml::LN_Value_ST_TextDirection_lrTb:
            case NS_ooxml::LN_Value_ST_TextDirection_lrTbV:
                nDirection = text::WritingMode_LR_TB;
            break;
            case NS_ooxml::LN_Value_ST_TextDirection_tbRl:
            case NS_ooxml::LN_Value_ST_TextDirection_btLr:
                nDirection = text::WritingMode_TB_RL;
            break;
            default:;
        }

        PropertyMap * pTargetContext = rContext.get();

        if (pSectionContext)
        {
            pTargetContext = pSectionContext;
        }

        pTargetContext->Insert(PROP_WRITING_MODE, uno::makeAny( sal_Int16(nDirection) ) );
    }
    break;  // sprmSTextFlow
        // the following are not part of the official documentation
    case NS_ooxml::LN_CT_Tabs_tab:
        resolveSprmProps(*this, rSprm);
        m_pImpl->IncorporateTabStop(m_pImpl->m_aCurrentTabStop);
        m_pImpl->m_aCurrentTabStop = DeletableTabStop();
    break;
    case NS_ooxml::LN_CT_PPrBase_tabs:
    {
        // Initialize tab stop vector from style sheet
        // fdo#81033: for RTF, a tab stop is inherited from the style if it
        // is also applied to the paragraph directly, and cleared if it is
        // not applied to the paragraph directly => don't InitTabStopFromStyle
        if ( !IsRTFImport() )
        {
            uno::Any aValue = m_pImpl->GetPropertyFromParaStyleSheet(PROP_PARA_TAB_STOPS);
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
            if( pProperties )
            {
                auto pBorderHandler = std::make_shared<BorderHandler>( true );
                pProperties->resolve(*pBorderHandler);

                rContext->Insert( PROP_CHAR_TOP_BORDER, uno::makeAny( pBorderHandler->getBorderLine()));
                rContext->Insert( PROP_CHAR_BOTTOM_BORDER, uno::makeAny( pBorderHandler->getBorderLine()));
                rContext->Insert( PROP_CHAR_LEFT_BORDER, uno::makeAny( pBorderHandler->getBorderLine()));
                rContext->Insert( PROP_CHAR_RIGHT_BORDER, uno::makeAny( pBorderHandler->getBorderLine()));

                rContext->Insert( PROP_CHAR_TOP_BORDER_DISTANCE, uno::makeAny( pBorderHandler->getLineDistance()));
                rContext->Insert( PROP_CHAR_BOTTOM_BORDER_DISTANCE, uno::makeAny( pBorderHandler->getLineDistance()));
                rContext->Insert( PROP_CHAR_LEFT_BORDER_DISTANCE, uno::makeAny( pBorderHandler->getLineDistance()));
                rContext->Insert( PROP_CHAR_RIGHT_BORDER_DISTANCE, uno::makeAny( pBorderHandler->getLineDistance()));

                table::ShadowFormat aFormat;
                // Word only allows shadows on visible borders
                if ( pBorderHandler->getShadow() && pBorderHandler->getBorderLine().LineStyle != table::BorderLineStyle::NONE )
                    aFormat = writerfilter::dmapper::PropertyMap::getShadowFromBorder(pBorderHandler->getBorderLine());
                rContext->Insert(PROP_CHAR_SHADOW_FORMAT, uno::makeAny(aFormat));
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
            for (const auto& rItem : m_pImpl->m_aSubInteropGrabBag)
            {
                if (rItem.Name == "val")
                    m_pImpl->GetTopContext()->Insert(PROP_CHAR_THEME_ORIGINAL_COLOR, rItem.Value, true, CHAR_GRAB_BAG);
                else if (rItem.Name == "themeColor")
                    m_pImpl->GetTopContext()->Insert(PROP_CHAR_THEME_COLOR, rItem.Value, true, CHAR_GRAB_BAG);
                else if (rItem.Name == "themeShade")
                    m_pImpl->GetTopContext()->Insert(PROP_CHAR_THEME_COLOR_SHADE, rItem.Value, true, CHAR_GRAB_BAG);
                else if (rItem.Name == "themeTint")
                    m_pImpl->GetTopContext()->Insert(PROP_CHAR_THEME_COLOR_TINT, rItem.Value, true, CHAR_GRAB_BAG);
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
        m_pImpl->SetLineNumberSettings( aSettings );
        //apply settings at XLineNumberingProperties
        try
        {
            uno::Reference< text::XLineNumberingProperties > xLineNumberingProperties( m_pImpl->GetTextDocument(), uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xLineNumberingPropSet = xLineNumberingProperties->getLineNumberingProperties();
            if( aSettings.nInterval == 0 )
                xLineNumberingPropSet->setPropertyValue(getPropertyName( PROP_IS_ON ), uno::makeAny(false) );
            else
            {
                xLineNumberingPropSet->setPropertyValue(getPropertyName( PROP_IS_ON ), uno::makeAny(true) );
                if( aSettings.nInterval )
                    xLineNumberingPropSet->setPropertyValue(getPropertyName( PROP_INTERVAL ), uno::makeAny(static_cast<sal_Int16>(aSettings.nInterval)) );
                if( aSettings.nDistance != -1 )
                    xLineNumberingPropSet->setPropertyValue(getPropertyName( PROP_DISTANCE ), uno::makeAny(aSettings.nDistance) );
                else
                {
                    // set Auto value (0.5 cm)
                    xLineNumberingPropSet->setPropertyValue(getPropertyName( PROP_DISTANCE ), uno::makeAny(static_cast<sal_Int32>(500)) );
                    if( pSectionContext )
                        pSectionContext->SetdxaLnn( static_cast<sal_Int32>(283) );
                }
                xLineNumberingPropSet->setPropertyValue(getPropertyName( PROP_RESTART_AT_EACH_PAGE ), uno::makeAny(aSettings.bRestartAtEachPage) );
            }
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
        if( pContext )
        {
            // If there is a deferred page break applied to this framed paragraph,
            // create a dummy paragraph without extra properties,
            // so that the anchored frame will be on the correct page (similar to shapes).
            if (pContext->isSet(PROP_BREAK_TYPE))
            {
                pContext->Erase(PROP_BREAK_TYPE);

                lcl_startParagraphGroup();
                m_pImpl->GetTopContext()->Insert(PROP_BREAK_TYPE, uno::makeAny(style::BreakType_PAGE_BEFORE));
                lcl_startCharacterGroup();
                sal_uInt8 const sBreak[] = { 0xd };
                lcl_text(sBreak, 1);
                lcl_endCharacterGroup();
                lcl_endParagraphGroup();
            }

            ParagraphPropertyMap* pParaContext = dynamic_cast< ParagraphPropertyMap* >( pContext.get() );
            if (pParaContext)
                pParaContext->SetFrameMode();

            if (!IsInHeaderFooter())
                m_pImpl->m_bIsActualParagraphFramed = true;
        }
        else
        {
            //TODO: What about style sheet import of frame properties
        }
        m_pImpl->NewFrameDirection();
        resolveSprmProps(*this, rSprm);
    }
    break;
    case NS_ooxml::LN_EG_SectPrContents_pgSz:
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
            if (!m_pImpl->IsAltChunk())
            {
                pSectionContext->Insert(PROP_HEIGHT, uno::makeAny(CT_PageSz.h));
            }
            pSectionContext->Insert( PROP_IS_LANDSCAPE, uno::makeAny( CT_PageSz.orient ));
            if (!m_pImpl->IsAltChunk())
            {
                pSectionContext->Insert(PROP_WIDTH, uno::makeAny(CT_PageSz.w));
            }
        }
        break;

    case NS_ooxml::LN_EG_SectPrContents_pgMar:
        m_pImpl->InitPageMargins();
        resolveSprmProps(*this, rSprm);
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            const PageMar& rPageMar = m_pImpl->GetPageMargins();
            pSectionContext->SetTopMargin( rPageMar.top );
            pSectionContext->SetRightMargin( rPageMar.right );
            pSectionContext->SetBottomMargin( rPageMar.bottom );
            pSectionContext->SetLeftMargin( rPageMar.left );
            pSectionContext->SetHeaderTop( rPageMar.header );
            pSectionContext->SetHeaderBottom( rPageMar.footer );
            pSectionContext->SetGutterMargin(rPageMar.gutter);
        }
        break;

    case NS_ooxml::LN_EG_SectPrContents_cols:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties )
        {

            tools::SvRef< SectionColumnHandler > pSectHdl( new SectionColumnHandler );
            pProperties->resolve(*pSectHdl);
            if(pSectionContext && !m_pImpl->isInIndexContext())
            {
                sal_Int16 nColumnCount = pSectHdl->GetNum() == 1 ? 0 : pSectHdl->GetNum();
                if( pSectHdl->IsEqualWidth() )
                {
                    pSectionContext->SetEvenlySpaced( true );
                    pSectionContext->SetColumnCount( nColumnCount );
                    pSectionContext->SetColumnDistance( pSectHdl->GetSpace() );
                    pSectionContext->SetSeparatorLine( pSectHdl->IsSeparator() );
                }
                else if( !pSectHdl->GetColumns().empty() )
                {
                    pSectionContext->SetEvenlySpaced( false );
                    pSectionContext->SetColumnDistance( pSectHdl->GetSpace() );
                    nColumnCount = pSectHdl->GetColumns().size();
                    pSectionContext->SetColumnCount( nColumnCount == 1 ? 0 : nColumnCount );
                    std::vector<Column_>::const_iterator tmpIter = pSectHdl->GetColumns().begin();
                    for (; tmpIter != pSectHdl->GetColumns().end(); ++tmpIter)
                    {
                        pSectionContext->AppendColumnWidth( tmpIter->nWidth );
                        if ((tmpIter != pSectHdl->GetColumns().end() - 1) || (tmpIter->nSpace > 0))
                            pSectionContext->AppendColumnSpacing( tmpIter->nSpace );
                    }
                    pSectionContext->SetSeparatorLine( pSectHdl->IsSeparator() );
                }
                else if( nColumnCount )
                {
                    pSectionContext->SetColumnCount( nColumnCount );
                    pSectionContext->SetColumnDistance( pSectHdl->GetSpace() );
                    pSectionContext->SetSeparatorLine( pSectHdl->IsSeparator() );
                }
            }

            else if ( pSectionContext )
            {
                FieldContextPtr pContext = m_pImpl->GetTopFieldContext();
                uno::Reference< beans::XPropertySet > xTOC = pContext->GetTOC();
                if( xTOC.is() )
                {
                    uno::Reference<text::XTextColumns> xTextColumns;
                    xTOC->getPropertyValue(getPropertyName( PROP_TEXT_COLUMNS )) >>= xTextColumns;
                    if (xTextColumns.is())
                    {
                        uno::Reference< beans::XPropertySet > xColumnPropSet( xTextColumns, uno::UNO_QUERY_THROW );
                        xColumnPropSet->setPropertyValue( getPropertyName( PROP_AUTOMATIC_DISTANCE ), uno::makeAny( pSectHdl->GetSpace() ));
                        xTOC->setPropertyValue( getPropertyName( PROP_TEXT_COLUMNS ), uno::makeAny( xTextColumns ) );
                    }
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
        if( pProperties && pSectionContext )
        {
            tools::SvRef< PageBordersHandler > pHandler( new PageBordersHandler );
            pProperties->resolve( *pHandler );

            // Set the borders to the context and apply them to the styles
            pHandler->SetBorders( pSectionContext );
        }
    }
    break;

    case NS_ooxml::LN_CT_PPrBase_snapToGrid:
        if (!IsStyleSheetImport()||!m_pImpl->isInteropGrabBagEnabled())
        {
            rContext->Insert( PROP_SNAP_TO_GRID, uno::makeAny(bool(nIntValue)));
        }
        else
        {
            m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "snapToGrid", OUString::number(nIntValue));
        }
    break;
    case NS_ooxml::LN_CT_PPrBase_pStyle:
    {
        StyleSheetTablePtr pStyleTable = m_pImpl->GetStyleSheetTable();
        const OUString sConvertedStyleName = pStyleTable->ConvertStyleName( sStringValue, true );
        m_pImpl->SetCurrentParaStyleName( sConvertedStyleName );
        if (m_pImpl->GetTopContext() && m_pImpl->GetTopContextType() != CONTEXT_SECTION)
            m_pImpl->GetTopContext()->Insert( PROP_PARA_STYLE_NAME, uno::makeAny( sConvertedStyleName ));
    }
    break;
    case NS_ooxml::LN_EG_RPrBase_rStyle:
        {
            OUString sConvertedName( m_pImpl->GetStyleSheetTable()->ConvertStyleName( sStringValue, true ) );
            if (m_pImpl->CheckFootnoteStyle() && m_pImpl->GetFootnoteContext())
                m_pImpl->SetHasFootnoteStyle(m_pImpl->GetFootnoteContext()->GetFootnoteStyle() == sConvertedName);

            // First check if the style exists in the document.
            StyleSheetEntryPtr pEntry = m_pImpl->GetStyleSheetTable( )->FindStyleSheetByConvertedStyleName( sConvertedName );
            bool bExists = pEntry && ( pEntry->nStyleTypeCode == STYLE_TYPE_CHAR );
            // Add the property if the style exists, but do not add it elements in TOC:
            // they will receive later another style references from TOC
            if ( bExists && m_pImpl->GetTopContext() && !m_pImpl->IsInTOC())
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
        if( pProperties )
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
    case NS_ooxml::LN_EG_RPrBase_noProof: // no grammar and spell checking, unsupported
    break;
    case NS_ooxml::LN_anchor_anchor: // at_character drawing
    case NS_ooxml::LN_inline_inline: // as_character drawing
    {
        if ( m_pImpl->IsDiscardHeaderFooter() )
            break;
        //tdf112342: Break before images as well, if there are page break
        if (m_pImpl->isBreakDeferred(BreakType::PAGE_BREAK))
        {
            m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH)
                ->Insert(PROP_BREAK_TYPE, uno::makeAny(style::BreakType_PAGE_BEFORE));
            m_pImpl->clearDeferredBreak(PAGE_BREAK);
        }
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties )
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
        sal_Int8 nProp = DFLT_ESC_PROP;
        if ( sStringValue == "superscript" )
                nEscapement = DFLT_ESC_AUTO_SUPER;
        else if ( sStringValue == "subscript" )
                nEscapement = DFLT_ESC_AUTO_SUB;
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
    case NS_ooxml::LN_CT_FtnProps_numFmt:
    case NS_ooxml::LN_CT_EdnProps_numFmt:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties)
        {
            pProperties->resolve(*this);
        }
    }
    break;
    case NS_ooxml::LN_EG_FtnEdnNumProps_numStart:
    case NS_ooxml::LN_EG_FtnEdnNumProps_numRestart:
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
                    getPropertyName( PROP_START_AT),
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
                        getPropertyName( PROP_FOOTNOTE_COUNTING ),
                        uno::makeAny( nFootnoteCounting ));
            }
            else if (xFtnEdnSettings.is())
            {
                sal_Int16 nNumType = ConversionHelper::ConvertNumberingType( nIntValue );
                xFtnEdnSettings->setPropertyValue(
                    getPropertyName( PROP_NUMBERING_TYPE),
                                                                    uno::makeAny( nNumType ));
            }
        }
        catch( const uno::Exception& )
        {
        }
    }
    break;
    case NS_ooxml::LN_EG_RangeMarkupElements_moveFromRangeStart:
        m_pImpl->SetMoveBookmark(/*bIsFrom=*/true);
        if (m_pImpl->hasTableManager())
            m_pImpl->getTableManager().setMoved( getPropertyName(PROP_TABLE_ROW_DELETE) );
    break;
    case NS_ooxml::LN_EG_RangeMarkupElements_moveToRangeStart:
        m_pImpl->SetMoveBookmark(/*bIsFrom=*/false);
        if (m_pImpl->hasTableManager())
            m_pImpl->getTableManager().setMoved( getPropertyName(PROP_TABLE_ROW_INSERT) );
    break;
    case NS_ooxml::LN_EG_RangeMarkupElements_moveFromRangeEnd:
    case NS_ooxml::LN_EG_RangeMarkupElements_moveToRangeEnd:
         if (m_pImpl->hasTableManager())
            m_pImpl->getTableManager().setMoved( OUString() );
    break;
    case NS_ooxml::LN_paratrackchange:
        m_pImpl->StartParaMarkerChange( );
        [[fallthrough]];
    case NS_ooxml::LN_CT_PPr_pPrChange:
    case NS_ooxml::LN_CT_ParaRPr_rPrChange:
    case NS_ooxml::LN_trackchange:
    case NS_ooxml::LN_EG_RPrContent_rPrChange:
    case NS_ooxml::LN_EG_RangeMarkupElements_customXmlDelRangeStart:
    case NS_ooxml::LN_EG_RangeMarkupElements_customXmlDelRangeEnd:
    case NS_ooxml::LN_EG_RangeMarkupElements_customXmlMoveFromRangeStart:
    case NS_ooxml::LN_EG_RangeMarkupElements_customXmlMoveFromRangeEnd:
    case NS_ooxml::LN_EG_RangeMarkupElements_customXmlMoveToRangeStart:
    case NS_ooxml::LN_EG_RangeMarkupElements_customXmlMoveToRangeEnd:
    {
        HandleRedline( rSprm );
    }
    break;
    case NS_ooxml::LN_endtrackchange:
        m_pImpl->RemoveTopRedline();
    break;
    case NS_ooxml::LN_CT_RPrChange_rPr:
    {
        // Push all the current 'Character' properties to the stack, so that we don't store them
        // as 'tracked changes' by mistake
        m_pImpl->PushProperties(CONTEXT_CHARACTER);

        // Resolve all the properties that are under the 'rPrChange'->'rPr' XML node
        resolveSprmProps(*this, rSprm );

        // Get all the properties that were processed in the 'rPrChange'->'rPr' XML node
        uno::Sequence< beans::PropertyValue > currentRedlineRevertProperties = m_pImpl->GetTopContext()->GetPropertyValues();

        // Pop back out the character properties that were on the run
        m_pImpl->PopProperties(CONTEXT_CHARACTER);

        // Store these properties in the current redline object (do it after the PopProperties() above, since
        // otherwise it'd be stored in the content dropped there).
        m_pImpl->SetCurrentRedlineRevertProperties( currentRedlineRevertProperties );
    }
    break;
    case NS_ooxml::LN_CT_PPrChange_pPr:
    {
        // Push all the current 'Paragraph' properties to the stack, so that we don't store them
        // as 'tracked changes' by mistake
        m_pImpl->PushProperties(CONTEXT_PARAGRAPH);

        // Resolve all the properties that are under the 'pPrChange'->'pPr' XML node
        resolveSprmProps(*this, rSprm );

        // Get all the properties that were processed in the 'pPrChange'->'pPr' XML node
        uno::Sequence< beans::PropertyValue > currentRedlineRevertProperties = m_pImpl->GetTopContext()->GetPropertyValues();

        // Pop back out the character properties that were on the run
        m_pImpl->PopProperties(CONTEXT_PARAGRAPH);

        // Store these properties in the current redline object (do it after the PopProperties() above, since
        // otherwise it'd be stored in the content dropped there).
        m_pImpl->SetCurrentRedlineRevertProperties( currentRedlineRevertProperties );
    }
    break;
    case NS_ooxml::LN_object:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties )
        {
            auto pOLEHandler = std::make_shared<OLEHandler>(*this);
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
        rContext->Insert(PROP_PARA_CONTEXT_MARGIN, uno::makeAny( nIntValue != 0 ));
    break;
    case NS_ooxml::LN_CT_PPrBase_mirrorIndents: // mirrorIndents
        rContext->Insert(PROP_MIRROR_INDENTS, uno::makeAny( nIntValue != 0 ), true, PARA_GRAB_BAG);
    break;
    case NS_ooxml::LN_EG_SectPrContents_formProt: //section protection
    {
        if( pSectionContext )
            pSectionContext->Insert( PROP_IS_PROTECTED, uno::makeAny( bool(nIntValue) ) );
    }
    break;
    case NS_ooxml::LN_EG_SectPrContents_vAlign:
    {
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if( pSectionContext )
        {
            drawing::TextVerticalAdjust nVA = drawing::TextVerticalAdjust_TOP;
            switch( nIntValue )
            {
                case NS_ooxml::LN_Value_ST_VerticalJc_center: //92367
                    nVA = drawing::TextVerticalAdjust_CENTER;
                    break;
                case NS_ooxml::LN_Value_ST_VerticalJc_both:   //92368 - justify
                    nVA = drawing::TextVerticalAdjust_BLOCK;
                    break;
                case NS_ooxml::LN_Value_ST_VerticalJc_bottom: //92369
                    nVA = drawing::TextVerticalAdjust_BOTTOM;
                    break;
                default:
                    break;
            }
            pSectionContext->Insert( PROP_TEXT_VERTICAL_ADJUST, uno::makeAny( nVA ), true, PARA_GRAB_BAG );
        }
    }
    break;
    case NS_ooxml::LN_EG_RPrBase_fitText:
    break;
    case NS_ooxml::LN_ffdata:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties)
        {
            FFDataHandler::Pointer_t pFFDataHandler(new FFDataHandler());

            pProperties->resolve(*pFFDataHandler);
            m_pImpl->SetFieldFFData(pFFDataHandler);
        }
    }
    break;
    case NS_ooxml::LN_CT_SdtPr_dropDownList:
    case NS_ooxml::LN_CT_SdtPr_comboBox:
    {
        m_pImpl->m_pSdtHelper->setControlType(SdtControlType::dropDown);
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties)
            pProperties->resolve(*this);
    }
    break;
    case NS_ooxml::LN_CT_SdtDropDownList_listItem:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties)
            pProperties->resolve(*this);
    }
    break;
    case NS_ooxml::LN_CT_SdtPr_placeholder:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties)
            pProperties->resolve(*this);
    }
    break;
    break;
    case NS_ooxml::LN_CT_SdtPr_date:
    {
        m_pImpl->m_pSdtHelper->setControlType(SdtControlType::datePicker);
        resolveSprmProps(*this, rSprm);
        m_pImpl->m_pSdtHelper->setDateFieldStartRange(GetCurrentTextRange()->getEnd());
    }
    break;
    case NS_ooxml::LN_CT_SdtDate_dateFormat:
    {
        m_pImpl->m_pSdtHelper->getDateFormat().append(sStringValue);
    }
    break;
    case NS_ooxml::LN_CT_SdtDate_storeMappedDataAs:
    {
    }
    break;
    case NS_ooxml::LN_CT_SdtDate_calendar:
    {
    }
    break;
    case NS_ooxml::LN_CT_SdtDate_lid:
    {
        m_pImpl->m_pSdtHelper->getLocale().append(sStringValue);
    }
    break;
    case NS_ooxml::LN_CT_SdtPr_dataBinding:
    case NS_ooxml::LN_CT_SdtPr_equation:
    case NS_ooxml::LN_CT_SdtPr_checkbox:
    case NS_ooxml::LN_CT_SdtPr_docPartObj:
    case NS_ooxml::LN_CT_SdtPr_docPartList:
    case NS_ooxml::LN_CT_SdtPr_picture:
    case NS_ooxml::LN_CT_SdtPr_citation:
    case NS_ooxml::LN_CT_SdtPr_group:
    case NS_ooxml::LN_CT_SdtPr_text:
    case NS_ooxml::LN_CT_SdtPr_id:
    case NS_ooxml::LN_CT_SdtPr_alias:
    case NS_ooxml::LN_CT_SdtPlaceholder_docPart:
    case NS_ooxml::LN_CT_SdtPr_color:
    {
        // this is an unsupported SDT property, create a grab bag for it
        OUString sName;
        switch (nSprmId)
        {
            case NS_ooxml::LN_CT_SdtPr_dataBinding: sName = "ooxml:CT_SdtPr_dataBinding"; break;
            case NS_ooxml::LN_CT_SdtPr_equation:    sName = "ooxml:CT_SdtPr_equation"; break;
            case NS_ooxml::LN_CT_SdtPr_checkbox:    sName = "ooxml:CT_SdtPr_checkbox"; break;
            case NS_ooxml::LN_CT_SdtPr_docPartObj:  sName = "ooxml:CT_SdtPr_docPartObj"; break;
            case NS_ooxml::LN_CT_SdtPr_docPartList: sName = "ooxml:CT_SdtPr_docPartList"; break;
            case NS_ooxml::LN_CT_SdtPr_picture:     sName = "ooxml:CT_SdtPr_picture"; break;
            case NS_ooxml::LN_CT_SdtPr_citation:    sName = "ooxml:CT_SdtPr_citation"; break;
            case NS_ooxml::LN_CT_SdtPr_group:       sName = "ooxml:CT_SdtPr_group"; break;
            case NS_ooxml::LN_CT_SdtPr_text:        sName = "ooxml:CT_SdtPr_text"; break;
            case NS_ooxml::LN_CT_SdtPr_id:          sName = "ooxml:CT_SdtPr_id"; break;
            case NS_ooxml::LN_CT_SdtPr_alias:       sName = "ooxml:CT_SdtPr_alias"; break;
            case NS_ooxml::LN_CT_SdtPlaceholder_docPart: sName = "ooxml:CT_SdtPlaceholder_docPart"; break;
            case NS_ooxml::LN_CT_SdtPr_color:       sName = "ooxml:CT_SdtPr_color"; break;
            default: assert(false);
        };
        enableInteropGrabBag(sName);

        // process subitems
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties)
            pProperties->resolve(*this);

        if (nSprmId == NS_ooxml::LN_CT_SdtPr_alias)
        {
            beans::PropertyValue aValue;
            aValue.Name = sName;
            aValue.Value <<= sStringValue;
            m_pImpl->m_pSdtHelper->appendToInteropGrabBag(aValue);
        }
        else
            m_pImpl->m_pSdtHelper->appendToInteropGrabBag(getInteropGrabBag());
        m_pImpl->m_pSdtHelper->setOutsideAParagraph(m_pImpl->IsOutsideAParagraph());
        m_pImpl->disableInteropGrabBag();
    }
    break;
    case NS_ooxml::LN_CT_SdtCheckbox_checked:
        m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "ooxml:CT_SdtCheckbox_checked", sStringValue);
        break;
    case NS_ooxml::LN_CT_SdtCheckbox_checkedState:
        m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "ooxml:CT_SdtCheckbox_checkedState", sStringValue);
        break;
    case NS_ooxml::LN_CT_SdtCheckbox_uncheckedState:
        m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "ooxml:CT_SdtCheckbox_uncheckedState", sStringValue);
        break;
    case NS_ooxml::LN_CT_SdtDocPart_docPartGallery:
        m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "ooxml:CT_SdtDocPart_docPartGallery", sStringValue);
        break;
    case NS_ooxml::LN_CT_SdtDocPart_docPartCategory:
        m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "ooxml:CT_SdtDocPart_docPartCategory", sStringValue);
        break;
    case NS_ooxml::LN_CT_SdtDocPart_docPartUnique:
        m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "ooxml:CT_SdtDocPart_docPartUnique", sStringValue);
        break;
    case NS_ooxml::LN_EG_SectPrContents_pgNumType:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties )
        {
            pProperties->resolve(*this);
        }
    }
    break;
    case NS_ooxml::LN_tblStart:
    {
        if (m_pImpl->hasTableManager())
        {
            bool bTableStartsAtCellStart = m_pImpl->m_nTableDepth > 0 && m_pImpl->m_nTableCellDepth > m_pImpl->m_nLastTableCellParagraphDepth + 1;
            m_pImpl->getTableManager().setTableStartsAtCellStart(bTableStartsAtCellStart);
        }
        /*
         * Hack for Importing Section Properties
         * LO is not able to import section properties if first element in the
         * section is a table. So in case first element is a table add a dummy para
         * and remove it again when lcl_endSectionGroup is called
         */
        if(m_pImpl->m_nTableDepth == 0 && m_pImpl->GetIsFirstParagraphInSection()
                && !m_pImpl->GetIsDummyParaAddedForTableInSection() && !m_pImpl->GetIsTextFrameInserted()
                && !IsInHeaderFooter())
        {
            m_pImpl->AddDummyParaForTableInSection();
        }

        // if first paragraph style in table has break-before-page, transfer that setting to the table itself.
        if( m_pImpl->m_nTableDepth == 0 )
        {
            const uno::Any aBreakType = uno::makeAny(style::BreakType_PAGE_BEFORE);
            const PropertyMapPtr pParagraphProps = m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH);
            if( pParagraphProps && pParagraphProps->isSet(PROP_PARA_STYLE_NAME) )
            {
                StyleSheetEntryPtr pStyle;
                OUString sStyleName;
                pParagraphProps->getProperty(PROP_PARA_STYLE_NAME)->second >>= sStyleName;
                if( !sStyleName.isEmpty() && GetStyleSheetTable() )
                    pStyle = GetStyleSheetTable()->FindStyleSheetByConvertedStyleName( sStyleName );

                if( pStyle && pStyle->pProperties
                    && pStyle->pProperties->isSet(PROP_BREAK_TYPE)
                    && pStyle->pProperties->getProperty(PROP_BREAK_TYPE)->second == aBreakType )
                {
                    pParagraphProps->Insert(PROP_BREAK_TYPE, aBreakType);
                }
            }
        }

        m_pImpl->m_nTableDepth++;
    }
    break;
    case NS_ooxml::LN_tblEnd:
        m_pImpl->m_nTableDepth--;
    break;
    case NS_ooxml::LN_tcStart:
        m_pImpl->m_nTableCellDepth++;
    break;
    case NS_ooxml::LN_tcEnd:
        m_pImpl->m_nTableCellDepth--;
        m_pImpl->m_nLastTableCellParagraphDepth = 0;
    break;
    case NS_ooxml::LN_glow_glow:
    case NS_ooxml::LN_shadow_shadow:
    case NS_ooxml::LN_reflection_reflection:
    case NS_ooxml::LN_textOutline_textOutline:
    case NS_ooxml::LN_textFill_textFill:
    case NS_ooxml::LN_scene3d_scene3d:
    case NS_ooxml::LN_props3d_props3d:
    case NS_ooxml::LN_ligatures_ligatures:
    case NS_ooxml::LN_numForm_numForm:
    case NS_ooxml::LN_numSpacing_numSpacing:
    case NS_ooxml::LN_stylisticSets_stylisticSets:
    case NS_ooxml::LN_cntxtAlts_cntxtAlts:
    {
        tools::SvRef<TextEffectsHandler> pTextEffectsHandlerPtr( new TextEffectsHandler(nSprmId) );
        std::optional<PropertyIds> aPropertyId = pTextEffectsHandlerPtr->getGrabBagPropertyId();
        if(aPropertyId)
        {
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties )
            {
                pProperties->resolve(*pTextEffectsHandlerPtr);

                beans::PropertyValue aGrabBag = pTextEffectsHandlerPtr->getInteropGrabBag();
                rContext->Insert(*aPropertyId, uno::makeAny(aGrabBag), true, CHAR_GRAB_BAG);

                sal_Int16 nTransparency = TextEffectsHandler::GetTextFillSolidFillAlpha(aGrabBag);
                if (nTransparency != 0)
                {
                    rContext->Insert(PROP_CHAR_TRANSPARENCE, uno::makeAny(nTransparency));
                }
            }
        }
    }
    break;
    case NS_ooxml::LN_CT_SdtPr_rPr:
    {
        // Make sure properties from a previous SDT are not merged with the current ones.
        m_pImpl->m_pSdtHelper->getInteropGrabBagAndClear();
    }
    break;
    case NS_ooxml::LN_CT_TblPrBase_tblLook:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties)
        {
            pProperties->resolve(*this);
            m_pImpl->getTableManager().finishTableLook();
        }
    }
    break;
    case NS_ooxml::LN_CT_TrPrBase_cnfStyle:
    {
        m_pImpl->enableInteropGrabBag("cnfStyle");
        resolveSprmProps(*this, rSprm);

        TablePropertyMapPtr pPropMap(new TablePropertyMap());
        pPropMap->Insert(PROP_ROW_CNF_STYLE, uno::makeAny(comphelper::containerToSequence(m_pImpl->m_aInteropGrabBag)), true, ROW_GRAB_BAG);
        m_pImpl->getTableManager().insertRowProps(pPropMap);

        m_pImpl->disableInteropGrabBag();
    }
    break;
    case NS_ooxml::LN_CT_TcPrBase_cnfStyle:
    {
        m_pImpl->enableInteropGrabBag("cnfStyle");
        resolveSprmProps(*this, rSprm);

        TablePropertyMapPtr pPropMap(new TablePropertyMap());
        pPropMap->Insert(PROP_CELL_CNF_STYLE, uno::makeAny(comphelper::containerToSequence(m_pImpl->m_aInteropGrabBag)), true, CELL_GRAB_BAG);
        m_pImpl->getTableManager().cellProps(pPropMap);

        m_pImpl->disableInteropGrabBag();
    }
    break;
    case NS_ooxml::LN_CT_PPrBase_cnfStyle:
    {
        m_pImpl->enableInteropGrabBag("cnfStyle");
        resolveSprmProps(*this, rSprm);
        rContext->Insert(PROP_PARA_CNF_STYLE, uno::makeAny(comphelper::containerToSequence(m_pImpl->m_aInteropGrabBag)), true, PARA_GRAB_BAG);
        m_pImpl->disableInteropGrabBag();
    }
    break;
    case NS_ooxml::LN_EG_RunInnerContent_sym:
    {
        resolveSprmProps(*this, rSprm);
        SymbolData  aSymbolData = m_pImpl->GetSymbolData();
        uno::Any    aVal = uno::makeAny( aSymbolData.sFont );
        auto xFootnote = rContext->GetFootnote();
        if (!xFootnote.is() && m_pImpl->IsInCustomFootnote())
            xFootnote = m_pImpl->GetFootnoteContext()->GetFootnote();
        if (xFootnote.is())
        {
            // DOCX can have different labels for the footnote reference and the footnote area.
            // This skips the one from the footnote area and just uses the reference one.
            if (!m_pImpl->IsInFootOrEndnote())
            {
                auto xAnchorRange = xFootnote->getAnchor();
                auto xAnchorCursor(xAnchorRange->getText()->createTextCursorByRange(xAnchorRange));

                // append a dummy character, so the following properties will be set as
                // as SwpHints::SwTextAttr instead of the SwAttrSet of the paragraph,
                // which would be removed by SwXText::Impl::finishOrAppendParagraph
                xAnchorCursor->collapseToEnd();
                uno::Reference<text::XTextRange> xHackRange(xAnchorCursor, uno::UNO_QUERY);
                xHackRange->setString("x");

                uno::Reference<beans::XPropertySet> xAnchorProps(xAnchorRange, uno::UNO_QUERY);
                xAnchorProps->setPropertyValue(getPropertyName(PROP_CHAR_FONT_NAME), aVal);
                xAnchorProps->setPropertyValue(getPropertyName(PROP_CHAR_FONT_NAME_ASIAN), aVal);
                xAnchorProps->setPropertyValue(getPropertyName(PROP_CHAR_FONT_NAME_COMPLEX), aVal);
                xAnchorProps->setPropertyValue(getPropertyName(PROP_CHAR_FONT_CHAR_SET), uno::makeAny(awt::CharSet::SYMBOL));

                // remove the dummy char
                xHackRange->setString("");

                OUString sLabel = xFootnote->getLabel() + OUStringChar(aSymbolData.cSymbol);
                xFootnote->setLabel(sLabel);
            }
        }
        else //it's a _real_ symbol
        {
            rContext->Insert(PROP_CHAR_FONT_NAME, aVal);
            rContext->Insert(PROP_CHAR_FONT_NAME_ASIAN, aVal);
            rContext->Insert(PROP_CHAR_FONT_NAME_COMPLEX, aVal);
            rContext->Insert(PROP_CHAR_FONT_CHAR_SET, uno::makeAny(awt::CharSet::SYMBOL));
            utext( reinterpret_cast < const sal_uInt8 * >( &(aSymbolData.cSymbol) ), 1 );
        }
    }
    break;
    case NS_ooxml::LN_EG_RunInnerContent_ruby:
    {
        RubyInfo    aInfo ;
        m_pImpl->SetRubyInfo(aInfo);
    }
    break;
    case NS_ooxml::LN_CT_RubyPr:
    case NS_ooxml::LN_CT_Ruby_rt:
    case NS_ooxml::LN_CT_Ruby_rubyBase:
    {
        m_pImpl->SetRubySprmId(nSprmId);
        if (nSprmId == NS_ooxml::LN_CT_RubyPr)
        {
            resolveSprmProps(*this, rSprm);
        }
    }
    break;
    case NS_ooxml::LN_EG_RubyContent_r:
    {
        const RubyInfo & aInfo = m_pImpl->GetRubyInfo();
        if (aInfo.nSprmId == NS_ooxml::LN_CT_Ruby_rubyBase)
        {
            rContext->Insert(PROP_RUBY_TEXT, uno::makeAny(aInfo.sRubyText));
            rContext->Insert(PROP_RUBY_STYLE, uno::makeAny(aInfo.sRubyStyle));
            rContext->Insert(PROP_RUBY_ADJUST, uno::makeAny(static_cast<sal_Int16>(ConversionHelper::convertRubyAlign(aInfo.nRubyAlign))));
            if ( aInfo.nRubyAlign == NS_ooxml::LN_Value_ST_RubyAlign_rightVertical )
                rContext->Insert(PROP_RUBY_POSITION, uno::makeAny(css::text::RubyPosition::INTER_CHARACTER));

            m_pImpl->SetRubySprmId(0);
        }
    }
    break;
    case NS_ooxml::LN_CT_RubyPr_rubyAlign:
    case NS_ooxml::LN_CT_RubyPr_hps:
    case NS_ooxml::LN_CT_RubyPr_hpsBaseText:
    {
        RubyInfo    aInfo = m_pImpl->GetRubyInfo();
        switch(nSprmId)
        {
            case NS_ooxml::LN_CT_RubyPr_rubyAlign:
                aInfo.nRubyAlign = nIntValue;
                break;
            case NS_ooxml::LN_CT_RubyPr_hps:
                aInfo.nHps= nIntValue;
                break;
            case NS_ooxml::LN_CT_RubyPr_hpsBaseText:
                aInfo.nHpsBaseText = nIntValue;
                break;
        }
        m_pImpl->SetRubyInfo(aInfo);
    }
    break;
    case NS_ooxml::LN_CT_SmartTagRun_smartTagPr:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties && m_pImpl->GetTopContextType() == CONTEXT_PARAGRAPH)
            pProperties->resolve(m_pImpl->getSmartTagHandler());
    }
    break;
    case NS_ooxml::LN_CT_DocPartPr_name:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties)
            pProperties->resolve(*this);
    }
    break;
    case NS_ooxml::LN_CT_DocPartPr_category:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties)
            pProperties->resolve(*this);
    }
    break;
    case NS_ooxml::LN_CT_DocPartCategory_gallery:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if (pProperties)
            pProperties->resolve(*this);
    }
    break;
    default:
        {
#ifdef DBG_UTIL
            TagLogger::getInstance().startElement("unhandled");
            TagLogger::getInstance().attribute("id", nSprmId);
            TagLogger::getInstance().attribute("name", rSprm.getName());
            TagLogger::getInstance().endElement();
#endif
        }
    }
}

void DomainMapper::processDeferredCharacterProperties( const std::map< sal_Int32, uno::Any >& deferredCharacterProperties )
{
    assert( m_pImpl->GetTopContextType() == CONTEXT_CHARACTER );
    PropertyMapPtr rContext = m_pImpl->GetTopContext();
    for( const auto& rProp : deferredCharacterProperties )
    {
        sal_Int32 Id = rProp.first;
        sal_Int32 nIntValue = 0;
        OUString sStringValue;
        rProp.second >>= nIntValue;
        rProp.second >>= sStringValue;
        switch( Id )
        {
        case NS_ooxml::LN_EG_RPrBase_position:
        {
            double nEscapement = 0;
            sal_Int8 nProp = 0;
            if ( nIntValue )
            {
                nProp = 100;
                double fFontSize = 0;
                m_pImpl->GetAnyProperty(PROP_CHAR_HEIGHT, rContext) >>= fFontSize;
                if ( fFontSize )
                    // nIntValue is in half-points, fontsize is in points, escapement is a percentage.
                    nEscapement = round( nIntValue/2.0 / fFontSize * 100 );
                else
                    nEscapement = nIntValue > 0 ? DFLT_ESC_SUPER : DFLT_ESC_SUB;
            }
            if ( nEscapement > MAX_ESC_POS )
                nEscapement = MAX_ESC_POS;
            else if ( nEscapement < -MAX_ESC_POS )
                nEscapement = -MAX_ESC_POS;

            rContext->Insert(PROP_CHAR_ESCAPEMENT,         uno::makeAny( sal_Int16(nEscapement) ) );
            rContext->Insert(PROP_CHAR_ESCAPEMENT_HEIGHT,  uno::makeAny( nProp ) );
        }
        break;
        default:
            SAL_WARN( "writerfilter", "Unhandled property in processDeferredCharacterProperty()" );
            break;
        }
    }
}

void DomainMapper::lcl_entry(writerfilter::Reference<Properties>::Pointer_t ref)
{
    ref->resolve(*this);
}

void DomainMapper::data(const sal_uInt8* /*buf*/, size_t /*len*/)
{
}

void DomainMapper::lcl_startSectionGroup()
{
    if (!m_pImpl->isInIndexContext() && !m_pImpl->isInBibliographyContext())
    {
        m_pImpl->PushProperties(CONTEXT_SECTION);
    }
    m_pImpl->SetIsFirstParagraphInSection(true);
    m_pImpl->SetIsFirstParagraphInSectionAfterRedline(true);
}

void DomainMapper::lcl_endSectionGroup()
{
    if (m_pImpl->isInIndexContext() || m_pImpl->isInBibliographyContext())
        return;

    m_pImpl->CheckUnregisteredFrameConversion();
    m_pImpl->ExecuteFrameConversion();
    // When pasting, it's fine to not have any paragraph inside the document at all.
    if (m_pImpl->GetIsFirstParagraphInSection() && m_pImpl->IsNewDoc())
    {
        // This section has no paragraph at all (e.g. they are all actually in a frame).
        // If this section has a page break, there would be nothing to apply to the page
        // style, so force a dummy paragraph.
        lcl_startParagraphGroup();
        lcl_startCharacterGroup();
        sal_uInt8 const sBreak[] = { 0xd };
        lcl_text(sBreak, 1);
        lcl_endCharacterGroup();
        lcl_endParagraphGroup();
    }
    PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_SECTION);
    SectionPropertyMap* pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
    OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
    if(pSectionContext)
    {
        pSectionContext->CloseSectionGroup( *m_pImpl );
        // Remove the dummy paragraph if added for
        // handling the section properties if section starts with a table
        if (m_pImpl->GetIsDummyParaAddedForTableInSection())
            m_pImpl->RemoveDummyParaForTableInSection();
    }
    m_pImpl->SetIsTextFrameInserted( false );
    m_pImpl->PopProperties(CONTEXT_SECTION);
}

void DomainMapper::lcl_startParagraphGroup()
{
    if (m_pImpl->hasTableManager())
        m_pImpl->getTableManager().startParagraphGroup();
    /*
     * Add new para properties only if paragraph is not split
     * or the top context is not of paragraph properties
     * Set mbIsSplitPara to false as it has been handled
     */
    if (!mbIsSplitPara)
        m_pImpl->PushProperties(CONTEXT_PARAGRAPH);
    mbIsSplitPara = false;
    if (m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH) != m_pImpl->GetTopContext())
        m_pImpl->PushProperties(CONTEXT_PARAGRAPH);

    if (m_pImpl->GetTopContext())
    {
        if (!m_pImpl->IsInShape())
        {
            const OUString& sDefaultParaStyle = m_pImpl->GetDefaultParaStyleName();
            m_pImpl->GetTopContext()->Insert( PROP_PARA_STYLE_NAME, uno::makeAny( sDefaultParaStyle ) );
            m_pImpl->SetCurrentParaStyleName( sDefaultParaStyle );

            if (m_pImpl->isBreakDeferred(PAGE_BREAK))
                m_pImpl->GetTopContext()->Insert(PROP_BREAK_TYPE, uno::makeAny(style::BreakType_PAGE_BEFORE));
            else if (m_pImpl->isBreakDeferred(COLUMN_BREAK))
                m_pImpl->GetTopContext()->Insert(PROP_BREAK_TYPE, uno::makeAny(style::BreakType_COLUMN_BEFORE));
            mbWasShapeInPara = false;
        }

        if (m_pImpl->isParaSdtEndDeferred())
            m_pImpl->GetTopContext()->Insert(PROP_PARA_SDT_END_BEFORE, uno::makeAny(true), true, PARA_GRAB_BAG);
    }
    m_pImpl->SetIsFirstRun(true);
    m_pImpl->SetIsOutsideAParagraph(false);
    if (!m_pImpl->IsInShape())
        m_pImpl->clearDeferredBreaks();
    m_pImpl->setParaSdtEndDeferred(false);
}

void DomainMapper::lcl_endParagraphGroup()
{
    m_pImpl->PopProperties(CONTEXT_PARAGRAPH);
    if (m_pImpl->hasTableManager())
       m_pImpl->getTableManager().endParagraphGroup();
    //frame conversion has to be executed after table conversion
    m_pImpl->ExecuteFrameConversion();
    m_pImpl->SetIsOutsideAParagraph(true);
}

void DomainMapper::markLastParagraphInSection( )
{
    m_pImpl->SetIsLastParagraphInSection( true );
}

void DomainMapper::markLastSectionGroup( )
{
    m_pImpl->SetIsLastSectionGroup( true );
}

void DomainMapper::lcl_startShape(uno::Reference<drawing::XShape> const& xShape)
{
    assert(xShape.is());

    if (m_pImpl->GetTopContext())
    {
        // If there is a deferred page break, handle it now, so that the
        // started shape will be on the correct page.
        if (m_pImpl->isBreakDeferred(PAGE_BREAK))
        {
            m_pImpl->clearDeferredBreak(PAGE_BREAK);
            lcl_startCharacterGroup();
            sal_uInt8 const sBreak[] = { 0xd };
            lcl_text(sBreak, 1);
            lcl_endCharacterGroup();
            lcl_endParagraphGroup();
            lcl_startParagraphGroup();
            m_pImpl->GetTopContext()->Insert(PROP_BREAK_TYPE, uno::makeAny(style::BreakType_PAGE_BEFORE));
        }
        m_pImpl->PushShapeContext( xShape );
        lcl_startParagraphGroup();
    }
    else
    {
        // No context? Then this image should not appear directly inside the
        // document, just save it for later usage.
        m_pImpl->PushPendingShape(xShape);
    }

    m_pImpl->SetIsFirstParagraphInShape(true);
    mbWasShapeInPara = true;
}

void DomainMapper::lcl_endShape( )
{
    if (!m_pImpl->GetTopContext())
        return;

    // End the current table, if there are any. Otherwise the unavoidable
    // empty paragraph at the end of the shape text will cause problems: if
    // the shape text ends with a table, the extra paragraph will be
    // handled as an additional row of the ending table.
    if (m_pImpl->hasTableManager())
        m_pImpl->getTableManager().endTable();

    lcl_endParagraphGroup();
    m_pImpl->PopShapeContext( );
    // A shape is always inside a paragraph (anchored or inline).
    m_pImpl->SetIsOutsideAParagraph(false);
}

void DomainMapper::PushStyleSheetProperties( const PropertyMapPtr& pStyleProperties, bool bAffectTableMngr )
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

void DomainMapper::PushListProperties( const ::tools::SvRef<PropertyMap>& pListProperties )
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
    if (m_pImpl->isSdtEndDeferred())
    {
        // Fields have an empty character group before the real one, so don't
        // call setSdtEndDeferred(false) here, that will happen only in lcl_utext().
        m_pImpl->GetTopContext()->Insert(PROP_SDT_END_BEFORE, uno::makeAny(true), true, CHAR_GRAB_BAG);
    }
}

void DomainMapper::lcl_endCharacterGroup()
{
    if (m_pImpl->CheckFootnoteStyle())
    {
        m_pImpl->SetCheckFootnoteStyle(m_pImpl->IsInCustomFootnote());
        m_pImpl->SetHasFootnoteStyle(false);
    }
    m_pImpl->PopProperties(CONTEXT_CHARACTER);
}

void DomainMapper::lcl_text(const sal_uInt8 * data_, size_t len)
{
    //TODO: Determine the right text encoding (FIB?)
    OUString sText( reinterpret_cast<const char*>(data_), len, RTL_TEXTENCODING_MS_1252 );
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("text");
    TagLogger::getInstance().chars(sText);
    TagLogger::getInstance().endElement();
#endif

    try
    {
        if(len == 1)
        {
            switch(*data_)
            {
                case 0x02: return; //footnote character
                case 0x08: // Lock field if in field context
                    if (m_pImpl->IsOpenField())
                        m_pImpl->SetFieldLocked();
                    return;
                case 0x0c: //page break
                    // page breaks aren't supported in footnotes and endnotes
                    if (!m_pImpl->IsInFootOrEndnote())
                    {
                        m_pImpl->deferBreak(PAGE_BREAK);
                        m_pImpl->SetIsDummyParaAddedForTableInSectionPage(false);
                    }
                    return;
                case 0x0e: //column break
                    m_pImpl->deferBreak(COLUMN_BREAK);
                    return;
                case 0x07:
                    m_pImpl->getTableManager().text(data_, len);
                    return;
                case 0x0d:
                {
                    PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH);
                    if (pContext && m_pImpl->isBreakDeferred(COLUMN_BREAK))
                    {
                        pContext->Insert(PROP_BREAK_TYPE, uno::makeAny(style::BreakType_COLUMN_BEFORE));
                        m_pImpl->clearDeferredBreak(COLUMN_BREAK);
                    }
                    finishParagraph();
                    return;
                }
                case cFieldStart:
                    m_pImpl->PushFieldContext();
                    return;
                case cFieldSep:
                    // delimiter not necessarily available
                    // appears only if field contains further content
                    m_pImpl->CloseFieldCommand();
                    return;
                case cFieldEnd:
                    m_pImpl->PopFieldContext();
                    return;
                default:
                    break;
            }
        }

        // GetTopContext() is changed by inserted breaks, but we want to keep the current context
        PropertyMapPtr pContext = m_pImpl->GetTopContext();
        if (!m_pImpl->GetFootnoteContext())
        {
            if (m_pImpl->isBreakDeferred(PAGE_BREAK))
                m_pImpl->GetTopContext()->Insert(PROP_BREAK_TYPE, uno::makeAny(style::BreakType_PAGE_BEFORE));
            else if (m_pImpl->isBreakDeferred(COLUMN_BREAK))
                m_pImpl->GetTopContext()->Insert(PROP_BREAK_TYPE, uno::makeAny(style::BreakType_COLUMN_BEFORE));
            m_pImpl->clearDeferredBreaks();
        }

        if (pContext && pContext->GetFootnote().is() && m_pImpl->IsInCustomFootnote())
        {
            pContext->GetFootnote()->setLabel(sText);
            m_pImpl->EndCustomFootnote();
            //otherwise ignore sText
        }
        else if (m_pImpl->IsOpenFieldCommand() && !m_pImpl->IsForceGenericFields())
        {
            m_pImpl->AppendFieldCommand(sText);
        }
        else if( m_pImpl->IsOpenField() && m_pImpl->IsFieldResultAsString())
             /*depending on the success of the field insert operation this result will be
              set at the field or directly inserted into the text*/
            m_pImpl->AppendFieldResult(sText);
        else
        {
            if (pContext == nullptr)
                pContext = new PropertyMap();

            m_pImpl->appendTextPortion( sText, pContext );
        }
    }
    catch( const uno::RuntimeException& )
    {
        TOOLS_WARN_EXCEPTION("writerfilter", "");
    }
}

void DomainMapper::lcl_positionOffset(const OUString& rText, bool bVertical)
{
    if (bVertical)
        m_pImpl->m_aPositionOffsets.second = rText;
    else
        m_pImpl->m_aPositionOffsets.first = rText;
}

awt::Point DomainMapper::getPositionOffset()
{
    awt::Point aRet;
    aRet.X = oox::drawingml::convertEmuToHmm(m_pImpl->m_aPositionOffsets.first.toInt32());
    aRet.Y = oox::drawingml::convertEmuToHmm(m_pImpl->m_aPositionOffsets.second.toInt32());
    return aRet;
}

void DomainMapper::lcl_align(const OUString& rText, bool bVertical)
{
    if (bVertical)
        m_pImpl->m_aAligns.second = rText;
    else
        m_pImpl->m_aAligns.first = rText;
}

void DomainMapper::lcl_positivePercentage(const OUString& rText)
{
    m_pImpl->m_aPositivePercentages.push(rText);
}

void DomainMapper::lcl_utext(const sal_uInt8 * data_, size_t len)
{
    // All these fixed values are defined as static const sal_Unicode codepoints in the fast parser,
    // like uFtnEdnRef = 0x2, uFtnEdnSep = 0x3, ... and have a len of 1, if they aren't valid unicode.

    OUString sText(reinterpret_cast<const sal_Unicode *>(data_), len);
    const RubyInfo & aInfo = m_pImpl->GetRubyInfo();
    if (aInfo.nSprmId == NS_ooxml::LN_CT_Ruby_rt)
    {
        PropertyMapPtr pContext = m_pImpl->GetTopContext();
        PropertyValueVector_t aProps = comphelper::sequenceToContainer< PropertyValueVector_t >(pContext->GetPropertyValues());
        OUString sStyle = getOrCreateCharStyle(aProps, /*bAlwaysCreate=*/false);
        m_pImpl->SetRubyText(sText,sStyle);
        return;
    }

    if (len == 1)
    {
        // preload all footnotes in separated footnotes
        if (sText[0] == 0x5)
        {
            if (m_pImpl->IsInFootnote())
            {
                if (m_pImpl->GetFootnoteCount() > -1)
                {
                    m_pImpl->PopFootOrEndnote();
                    m_pImpl->PushFootOrEndnote(/*bIsFootnote=*/true);
                }
                m_pImpl->IncrementFootnoteCount();
            }
            else
            {
                if (m_pImpl->GetEndnoteCount() > -1)
                {
                    m_pImpl->PopFootOrEndnote();
                    m_pImpl->PushFootOrEndnote(/*bIsFootnote=*/false);
                }
                m_pImpl->IncrementEndnoteCount();
            }
        }

        // If the footnote contains a Footnote Reference Mark, it can't be a custom footnote
        // ******
        // This code block is wrong, as it should also be in m_pImpl->IsInFootOrEndnote().
        // The main problem is that
        //
        //  assert(len != 1 || sText[0] != 0x2)
        //
        // is triggered by the unit test SwLayoutWriter::testForcepoint75, so all these pseudo
        // value handling is broken.
        // But this is just a symptom, as I guess it's possible to generate broken DOCX documents,
        // which might be problematic, triggering *funny* code paths left and right.
        // ******
        if (sText[0] == 0x2)
        {
            m_pImpl->EndCustomFootnote();
            return;
        }

        if (m_pImpl->IsInCustomFootnote())
        {
            if (sText[0] != 0xd && sText[0] != 0x3)
            {
                // DOCX can have different labels for the footnote reference and the footnote area.
                // This skips the one from the footnote area and just uses the reference one.
                if (!m_pImpl->IsInFootOrEndnote())
                {
                    if (PropertyMapPtr pFootnoteContext = m_pImpl->GetFootnoteContext())
                    {
                        auto xFootnote = pFootnoteContext->GetFootnote();
                        xFootnote->setLabel(xFootnote->getLabel() + sText);
                    }
                }
                return;
            }
            else
                m_pImpl->SetHasFootnoteStyle(true);
        }
    }

    if (m_pImpl->isSdtEndDeferred())
    {
        // In case we have a field context, then save the property there, so
        // SDT's ending right before a field start are handled as well.
        PropertyMapPtr pContext = m_pImpl->GetTopContext();
        if (m_pImpl->IsOpenField())
            pContext = m_pImpl->GetTopFieldContext()->getProperties();
        pContext->Insert(PROP_SDT_END_BEFORE, uno::makeAny(true), true, CHAR_GRAB_BAG);
        m_pImpl->setSdtEndDeferred(false);
    }

    bool bNewLine = len == 1 && (sText[0] == 0x0d || sText[0] == 0x07);
    if (m_pImpl->m_pSdtHelper->getControlType() == SdtControlType::dropDown)
    {
        if (bNewLine)
            // Dropdown control has single-line texts, so in case of newline, create the control.
            m_pImpl->m_pSdtHelper->createDropDownControl();
        else
        {
            m_pImpl->m_pSdtHelper->getSdtTexts().append(sText);
            return;
        }
    }
    else if (m_pImpl->m_pSdtHelper->getControlType() == SdtControlType::datePicker)
    {
        if (IsInHeaderFooter() && m_pImpl->IsDiscardHeaderFooter())
        {
            m_pImpl->m_pSdtHelper->getDateFormat().truncate();
            m_pImpl->m_pSdtHelper->getLocale().truncate();
            return;
        }
    }
    else if (!m_pImpl->m_pSdtHelper->isInteropGrabBagEmpty())
    {
        // there are unsupported SDT properties in the document
        // save them in the paragraph interop grab bag
        if (m_pImpl->IsDiscardHeaderFooter())
        {
            // Unless we're supposed to ignore this header/footer.
            m_pImpl->m_pSdtHelper->getInteropGrabBagAndClear();
            return;
        }
        if((m_pImpl->m_pSdtHelper->containedInInteropGrabBag("ooxml:CT_SdtPr_checkbox") ||
                m_pImpl->m_pSdtHelper->containedInInteropGrabBag("ooxml:CT_SdtPr_text") ||
                m_pImpl->m_pSdtHelper->containedInInteropGrabBag("ooxml:CT_SdtPr_dataBinding") ||
                m_pImpl->m_pSdtHelper->containedInInteropGrabBag("ooxml:CT_SdtPr_citation") ||
                (m_pImpl->m_pSdtHelper->containedInInteropGrabBag("ooxml:CT_SdtPr_id") &&
                        m_pImpl->m_pSdtHelper->getInteropGrabBagSize() == 1)) && !m_pImpl->m_pSdtHelper->isOutsideAParagraph())
        {
            PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_CHARACTER);

            if (m_pImpl->IsOpenField())
                // We have a field, insert the SDT properties to the field's grab-bag, so they won't be lost.
                pContext = m_pImpl->GetTopFieldContext()->getProperties();

            pContext->Insert(PROP_SDTPR, uno::makeAny(m_pImpl->m_pSdtHelper->getInteropGrabBagAndClear()), true, CHAR_GRAB_BAG);
        }
        else
            m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH)->Insert(PROP_SDTPR,
                    uno::makeAny(m_pImpl->m_pSdtHelper->getInteropGrabBagAndClear()), true, PARA_GRAB_BAG);
    }
    else if (len == 1 && sText[0] == 0x03)
    {
        // This is the uFtnEdnSep, remember that the document has a separator.
        m_pImpl->m_bHasFtnSep = true;
        return;
    }
    else if (len == 1 && sText[0] == '\t' )
    {
        if ( m_pImpl->m_bCheckFirstFootnoteTab && m_pImpl->IsInFootOrEndnote() )
        {
            // Allow MSO to emulate LO footnote text starting at left margin - only meaningful with hanging indent
            m_pImpl->m_bCheckFirstFootnoteTab = false;
            sal_Int32 nFirstLineIndent = 0;
            m_pImpl->GetAnyProperty(PROP_PARA_FIRST_LINE_INDENT, m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH)) >>= nFirstLineIndent;
            if ( nFirstLineIndent < 0 )
                m_pImpl->m_bIgnoreNextTab = true;
        }

        if ( m_pImpl->m_bIgnoreNextTab )
        {
            m_pImpl->m_bIgnoreNextTab = false;
            return;
        }
    }
    if (!m_pImpl->hasTableManager())
        return;

    SkipFootnoteSeparator eSkip = m_pImpl->GetSkipFootnoteState();
    if ( eSkip == SkipFootnoteSeparator::ON || eSkip == SkipFootnoteSeparator::SKIPPING )
    {
        m_pImpl->SetSkipFootnoteState( SkipFootnoteSeparator::SKIPPING );
        return;
    }

    try
    {
        m_pImpl->getTableManager().utext(data_, len);

        if (bNewLine)
        {
            const bool bSingleParagraph = m_pImpl->GetIsFirstParagraphInSection() && m_pImpl->GetIsLastParagraphInSection();
            const bool bSingleParagraphAfterRedline = m_pImpl->GetIsFirstParagraphInSection(/*bAfterRedline=*/true) &&
                    m_pImpl->GetIsLastParagraphInSection();
            PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH);
            if (!m_pImpl->GetFootnoteContext())
            {
                if (m_pImpl->isBreakDeferred(PAGE_BREAK))
                {
                    if (m_pImpl->GetSettingsTable()->GetSplitPgBreakAndParaMark())
                    {
                        if ( m_pImpl->GetIsFirstParagraphInSection() || !m_pImpl->IsFirstRun() )
                        {
                            m_pImpl->m_bIsSplitPara = true;
                            finishParagraph();
                            lcl_startParagraphGroup();
                        }


                        pContext->Insert(PROP_BREAK_TYPE, uno::makeAny(style::BreakType_PAGE_BEFORE));
                        m_pImpl->clearDeferredBreaks();
                    }
                }
                else if (m_pImpl->isBreakDeferred(COLUMN_BREAK))
                {
                    if ( m_pImpl->GetIsFirstParagraphInSection() || !m_pImpl->IsFirstRun() )
                    {
                        mbIsSplitPara = true;
                        finishParagraph();
                        lcl_startParagraphGroup();
                    }

                    pContext->Insert(PROP_BREAK_TYPE, uno::makeAny(style::BreakType_COLUMN_BEFORE));
                    m_pImpl->clearDeferredBreaks();
                }
            }

            // If the paragraph contains only the section properties and it has
            // no runs, we should not create a paragraph for it in Writer, unless that would remove the whole section.
            SectionPropertyMap* pSectionContext = m_pImpl->GetSectionContext();
            bool bRemove = (!m_pImpl->GetParaChanged() && m_pImpl->GetRemoveThisPara()) ||
                           (!m_pImpl->GetParaChanged() && m_pImpl->GetParaSectpr()
                            && !bSingleParagraphAfterRedline
                            && !m_pImpl->GetParaHadField()
                            && (!m_pImpl->GetIsDummyParaAddedForTableInSectionPage())
                            && !( pSectionContext && pSectionContext->GetBreakType() != -1 && pContext && pContext->isSet(PROP_BREAK_TYPE) )
                            && !m_pImpl->GetIsPreviousParagraphFramed()
                            && !m_pImpl->HasTopAnchoredObjects()
                            && !m_pImpl->IsParaWithInlineObject());

            const bool bNoNumbering = bRemove || (!m_pImpl->GetParaChanged() && m_pImpl->GetParaSectpr() && bSingleParagraph);
            PropertyMapPtr xContext = bNoNumbering ? m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH) : PropertyMapPtr();
            if (xContext)
            {
                // tdf#97417 delete numbering of the paragraph
                // it will be deleted anyway, and the numbering would be copied
                // to the next paragraph in sw SplitNode and then be applied to
                // every following paragraph
                xContext->Erase(PROP_NUMBERING_RULES);
                static_cast<ParagraphPropertyMap*>(xContext.get())->SetListId(-1);;
                xContext->Erase(PROP_NUMBERING_LEVEL);
            }
            m_pImpl->SetParaSectpr(false);
            finishParagraph(bRemove, bNoNumbering);
            if (bRemove)
                m_pImpl->RemoveLastParagraph();
        }
        else
        {
            // GetTopContext() is changed by inserted breaks, but we want to keep the current context
            PropertyMapPtr pContext = m_pImpl->GetTopContext();
            if (!m_pImpl->GetFootnoteContext())
            {
                if (m_pImpl->isBreakDeferred(PAGE_BREAK))
                {
                    /* If PAGEBREAK appears in first paragraph of the section or
                     * after first run of any paragraph then need to split paragraph
                     * to handle it properly.
                     */
                    if (m_pImpl->GetIsFirstParagraphInSection() || !m_pImpl->IsFirstRun())
                    {
                        m_pImpl->m_bIsSplitPara = true;
                        finishParagraph();
                        lcl_startParagraphGroup();
                    }
                    m_pImpl->GetTopContext()->Insert(PROP_BREAK_TYPE, uno::makeAny(style::BreakType_PAGE_BEFORE));
                }
                else if (m_pImpl->isBreakDeferred(COLUMN_BREAK))
                {
                    if (m_pImpl->GetIsFirstParagraphInSection() || !m_pImpl->IsFirstRun() || mbWasShapeInPara)
                    {
                        mbWasShapeInPara = false;
                        mbIsSplitPara = true;
                        finishParagraph();
                        lcl_startParagraphGroup();
                    }
                    m_pImpl->GetTopContext()->Insert(PROP_BREAK_TYPE, uno::makeAny(style::BreakType_COLUMN_BEFORE));
                }
                m_pImpl->clearDeferredBreaks();
            }

            if (pContext && pContext->GetFootnote().is())
            {
                pContext->GetFootnote()->setLabel( sText );
                // tdf#141548 don't lose footnote/endnote text of the run with uFtnEdnRef
                // (i.e. when footnoteRef/endnoteRef is followed by some text in the same run)
                m_pImpl->appendTextPortion( sText, pContext );
            }
            else if (m_pImpl->IsOpenFieldCommand() && !m_pImpl->IsForceGenericFields())
            {
                m_pImpl->AppendFieldCommand(sText);
            }
            else if( m_pImpl->IsOpenField() && m_pImpl->IsFieldResultAsString())
                /*depending on the success of the field insert operation this result will be
                  set at the field or directly inserted into the text*/
                m_pImpl->AppendFieldResult(sText);
            else
            {
                if (pContext == nullptr)
                    pContext = new PropertyMap();

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
            m_pImpl->SetNumberingImport(true);
            //the same for list tables
            ref->resolve( *m_pImpl->GetListTable() );
            m_pImpl->GetListTable( )->CreateNumberingRules( );
            m_pImpl->SetNumberingImport(false);
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
    m_pImpl->substream(rName, ref);
}

void DomainMapper::lcl_startGlossaryEntry()
{
    uno::Reference< text::XTextRange > xTextRange =  GetCurrentTextRange();
    m_pImpl->setGlossaryEntryStart(xTextRange);
}

void DomainMapper::lcl_endGlossaryEntry()
{
    m_pImpl->appendGlossaryEntry();
}

void DomainMapper::handleUnderlineType(const Id nId, const ::tools::SvRef<PropertyMap>& rContext)
{
    sal_Int16 nUnderline = awt::FontUnderline::NONE;

    switch (nId)
    {
    case NS_ooxml::LN_Value_ST_Underline_none:
        nUnderline = awt::FontUnderline::NONE;
        break;
    case NS_ooxml::LN_Value_ST_Underline_words:
        rContext->Insert(PROP_CHAR_WORD_MODE, uno::makeAny(true));
        [[fallthrough]];
    case NS_ooxml::LN_Value_ST_Underline_single:
        nUnderline = awt::FontUnderline::SINGLE;
        break;
    case NS_ooxml::LN_Value_ST_Underline_double:
        nUnderline = awt::FontUnderline::DOUBLE;
        break;
    case NS_ooxml::LN_Value_ST_Underline_dotted:
        nUnderline = awt::FontUnderline::DOTTED;
        break;
    case NS_ooxml::LN_Value_ST_Underline_dash:
        nUnderline = awt::FontUnderline::DASH;
        break;
    case NS_ooxml::LN_Value_ST_Underline_dotDash:
        nUnderline = awt::FontUnderline::DASHDOT;
        break;
    case NS_ooxml::LN_Value_ST_Underline_dotDotDash:
        nUnderline = awt::FontUnderline::DASHDOTDOT;
        break;
    case NS_ooxml::LN_Value_ST_Underline_thick:
        nUnderline = awt::FontUnderline::BOLD;
        break;
    case NS_ooxml::LN_Value_ST_Underline_wave:
        nUnderline = awt::FontUnderline::WAVE;
        break;
    case NS_ooxml::LN_Value_ST_Underline_dottedHeavy:
        nUnderline = awt::FontUnderline::BOLDDOTTED;
        break;
    case NS_ooxml::LN_Value_ST_Underline_dashedHeavy:
        nUnderline = awt::FontUnderline::BOLDDASH;
        break;
    case NS_ooxml::LN_Value_ST_Underline_dashLong:
        nUnderline = awt::FontUnderline::LONGDASH;
        break;
    case NS_ooxml::LN_Value_ST_Underline_dashLongHeavy:
        nUnderline = awt::FontUnderline::BOLDLONGDASH;
        break;
    case NS_ooxml::LN_Value_ST_Underline_dashDotHeavy:
        nUnderline = awt::FontUnderline::BOLDDASHDOT;
        break;
    case NS_ooxml::LN_Value_ST_Underline_dashDotDotHeavy:
        nUnderline = awt::FontUnderline::BOLDDASHDOTDOT;
        break;
    case NS_ooxml::LN_Value_ST_Underline_wavyHeavy:
        nUnderline = awt::FontUnderline::BOLDWAVE;
        break;
    case NS_ooxml::LN_Value_ST_Underline_wavyDouble:
        nUnderline = awt::FontUnderline::DOUBLEWAVE;
        break;
    }
    rContext->Insert(PROP_CHAR_UNDERLINE, uno::makeAny(nUnderline));
}

void DomainMapper::handleParaJustification(const sal_Int32 nIntValue, const ::tools::SvRef<PropertyMap>& rContext, const bool bExchangeLeftRight)
{
    style::ParagraphAdjust nAdjust = style::ParagraphAdjust_LEFT;
    style::ParagraphAdjust nLastLineAdjust = style::ParagraphAdjust_LEFT;
    OUString aStringValue = "left";
    switch(nIntValue)
    {
    case NS_ooxml::LN_Value_ST_Jc_center:
        nAdjust = style::ParagraphAdjust_CENTER;
        aStringValue = "center";
        break;
    case NS_ooxml::LN_Value_ST_Jc_right:
    case NS_ooxml::LN_Value_ST_Jc_end:
        nAdjust = bExchangeLeftRight ? style::ParagraphAdjust_LEFT : style::ParagraphAdjust_RIGHT;
        aStringValue = "right";
        break;
    case NS_ooxml::LN_Value_ST_Jc_distribute:
        nLastLineAdjust = style::ParagraphAdjust_BLOCK;
        [[fallthrough]];
    case NS_ooxml::LN_Value_ST_Jc_both:
        nAdjust = style::ParagraphAdjust_BLOCK;
        aStringValue = "both";
        break;
    case NS_ooxml::LN_Value_ST_Jc_left:
    case NS_ooxml::LN_Value_ST_Jc_start:
    default:
        nAdjust = bExchangeLeftRight ? style::ParagraphAdjust_RIGHT : style::ParagraphAdjust_LEFT;
        break;
    }
    rContext->Insert( PROP_PARA_ADJUST, uno::makeAny( nAdjust ) );
    rContext->Insert( PROP_PARA_LAST_LINE_ADJUST, uno::makeAny( nLastLineAdjust ) );
    m_pImpl->appendGrabBag(m_pImpl->m_aInteropGrabBag, "jc", aStringValue);
}

bool DomainMapper::getColorFromId(const Id nId, sal_Int32 &nColor)
{
    nColor = 0;
    if ((nId < NS_ooxml::LN_Value_ST_HighlightColor_black) || (nId > NS_ooxml::LN_Value_ST_HighlightColor_none))
        return false;

    switch (nId)
    {
    case NS_ooxml::LN_Value_ST_HighlightColor_black: nColor=0x000000; break;
    case NS_ooxml::LN_Value_ST_HighlightColor_blue: nColor=0x0000ff; break;
    case NS_ooxml::LN_Value_ST_HighlightColor_cyan: nColor=0x00ffff; break;
    case NS_ooxml::LN_Value_ST_HighlightColor_green: nColor=0x00ff00; break;
    case NS_ooxml::LN_Value_ST_HighlightColor_magenta: nColor=0xff00ff; break;
    case NS_ooxml::LN_Value_ST_HighlightColor_red: nColor=0xff0000; break;
    case NS_ooxml::LN_Value_ST_HighlightColor_yellow: nColor=0xffff00; break;
    case NS_ooxml::LN_Value_ST_HighlightColor_white: nColor=0xffffff; break;
    case NS_ooxml::LN_Value_ST_HighlightColor_darkBlue: nColor=0x000080;  break;
    case NS_ooxml::LN_Value_ST_HighlightColor_darkCyan: nColor=0x008080; break;
    case NS_ooxml::LN_Value_ST_HighlightColor_darkGreen: nColor=0x008000; break;
    case NS_ooxml::LN_Value_ST_HighlightColor_darkMagenta: nColor=0x800080; break;
    case NS_ooxml::LN_Value_ST_HighlightColor_darkRed: nColor=0x800000; break;
    case NS_ooxml::LN_Value_ST_HighlightColor_darkYellow: nColor=0x808000; break;
    case NS_ooxml::LN_Value_ST_HighlightColor_darkGray: nColor=0x808080; break;
    case NS_ooxml::LN_Value_ST_HighlightColor_lightGray: nColor=0xC0C0C0; break;
    case NS_ooxml::LN_Value_ST_HighlightColor_none: nColor=0xFFFFFFFF; break; //COL_AUTO
    default:
        return false;
    }
    return true;
}

sal_Int16 DomainMapper::getEmphasisValue(const sal_Int32 nIntValue)
{
    switch (nIntValue)
    {
    case NS_ooxml::LN_Value_ST_Em_dot:
        return text::FontEmphasis::DOT_ABOVE;
    case NS_ooxml::LN_Value_ST_Em_comma:
        return text::FontEmphasis::ACCENT_ABOVE;
    case NS_ooxml::LN_Value_ST_Em_circle:
        return text::FontEmphasis::CIRCLE_ABOVE;
    case NS_ooxml::LN_Value_ST_Em_underDot:
        return text::FontEmphasis::DOT_BELOW;
    default:
        return text::FontEmphasis::NONE;
    }
}

OUString DomainMapper::getBracketStringFromEnum(const sal_Int32 nIntValue, const bool bIsPrefix)
{
    switch(nIntValue)
    {
    case NS_ooxml::LN_Value_ST_CombineBrackets_round:
        if (bIsPrefix)
            return "(";
        return ")";

    case NS_ooxml::LN_Value_ST_CombineBrackets_square:
        if (bIsPrefix)
            return "[";
        return "]";

    case NS_ooxml::LN_Value_ST_CombineBrackets_angle:
        if (bIsPrefix)
            return "<";
        return ">";

    case NS_ooxml::LN_Value_ST_CombineBrackets_curly:
        if (bIsPrefix)
            return "{";
        return "}";

    case NS_ooxml::LN_Value_ST_CombineBrackets_none:
    default:
        return OUString();
    }
}

style::TabAlign DomainMapper::getTabAlignFromValue(const sal_Int32 nIntValue)
{
    switch (nIntValue)
    {
    case NS_ooxml::LN_Value_ST_TabJc_start:
    case NS_ooxml::LN_Value_ST_TabJc_left:
    case NS_ooxml::LN_Value_ST_TabJc_bar: // bar not supported
    case NS_ooxml::LN_Value_ST_TabJc_num: // num not supported
        return style::TabAlign_LEFT;
    case NS_ooxml::LN_Value_ST_TabJc_center:
        return style::TabAlign_CENTER;
    case NS_ooxml::LN_Value_ST_TabJc_end:
    case NS_ooxml::LN_Value_ST_TabJc_right:
        return style::TabAlign_RIGHT;
    case NS_ooxml::LN_Value_ST_TabJc_decimal:
        return style::TabAlign_DECIMAL;
    }
    return style::TabAlign_LEFT;
}

sal_Unicode DomainMapper::getFillCharFromValue(const sal_Int32 nIntValue)
{
    switch (nIntValue)
    {
    case NS_ooxml::LN_Value_ST_TabTlc_dot:
        return u'.';
    case NS_ooxml::LN_Value_ST_TabTlc_hyphen:
        return u'-';
    case NS_ooxml::LN_Value_ST_TabTlc_underscore:
    case NS_ooxml::LN_Value_ST_TabTlc_heavy: // FIXME ???
        return u'_';
    case NS_ooxml::LN_Value_ST_TabTlc_middleDot: // middleDot
        return u'\x00b7';
    case NS_ooxml::LN_Value_ST_TabTlc_none:
    default:
        return u' '; // blank space
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

uno::Reference < lang::XMultiServiceFactory > const & DomainMapper::GetTextFactory() const
{
    return m_pImpl->GetTextFactory();
}

uno::Reference< text::XTextRange > DomainMapper::GetCurrentTextRange()
{
    if (m_pImpl->HasTopText())
        return m_pImpl->GetTopTextAppend()->getEnd();
    return m_pImpl->m_xInsertTextRange;
}

OUString DomainMapper::getOrCreateCharStyle( PropertyValueVector_t& rCharProperties, bool bAlwaysCreate )
{
    StyleSheetTablePtr pStyleSheets = m_pImpl->GetStyleSheetTable();
    return pStyleSheets->getOrCreateCharStyle( rCharProperties, bAlwaysCreate );
}

StyleSheetTablePtr const & DomainMapper::GetStyleSheetTable( )
{
    return m_pImpl->GetStyleSheetTable( );
}

SettingsTablePtr const & DomainMapper::GetSettingsTable()
{
    return m_pImpl->GetSettingsTable();
}

GraphicZOrderHelper* DomainMapper::graphicZOrderHelper()
{
    if (zOrderHelper == nullptr)
        zOrderHelper.reset( new GraphicZOrderHelper );
    return zOrderHelper.get();
}

GraphicNamingHelper& DomainMapper::GetGraphicNamingHelper()
{
    if (m_pGraphicNamingHelper == nullptr)
        m_pGraphicNamingHelper.reset(new GraphicNamingHelper());
    return *m_pGraphicNamingHelper;
}

uno::Reference<drawing::XShape> DomainMapper::PopPendingShape()
{
    return m_pImpl->PopPendingShape();
}

bool DomainMapper::IsInHeaderFooter() const
{
    return m_pImpl->IsInHeaderFooter();
}

bool DomainMapper::IsInShape() const { return m_pImpl->IsInShape(); }

bool DomainMapper::IsInTable() const
{
    return m_pImpl->hasTableManager() && m_pImpl->getTableManager().isInCell();
}

OUString DomainMapper::GetListStyleName(sal_Int32 nListId) const
{
    return m_pImpl->GetListStyleName( nListId );
}

void DomainMapper::ValidateListLevel(const OUString& sStyleIdentifierD)
{
    m_pImpl->ValidateListLevel(sStyleIdentifierD);
}

void DomainMapper::SetDocDefaultsImport(bool bSet)
{
    m_pImpl->SetDocDefaultsImport(bSet);
}

bool DomainMapper::IsStyleSheetImport() const
{
    return m_pImpl->IsStyleSheetImport();
}

bool DomainMapper::IsNumberingImport() const
{
    return m_pImpl->IsNumberingImport();
}

void DomainMapper::enableInteropGrabBag(const OUString& aName)
{
    m_pImpl->m_aInteropGrabBagName = aName;
}

beans::PropertyValue DomainMapper::getInteropGrabBag()
{
    beans::PropertyValue aRet;
    aRet.Name = m_pImpl->m_aInteropGrabBagName;
    aRet.Value <<= comphelper::containerToSequence(m_pImpl->m_aInteropGrabBag);

    m_pImpl->m_aInteropGrabBag.clear();
    m_pImpl->m_aInteropGrabBagName.clear();
    return aRet;
}

void DomainMapper::HandleRedline( Sprm& rSprm )
{
    sal_uInt32 nSprmId = rSprm.getId();

    m_pImpl->AddNewRedline( nSprmId );

    if (nSprmId == NS_ooxml::LN_CT_PPr_pPrChange)
    {
        m_pImpl->SetCurrentRedlineToken(XML_ParagraphFormat);
    }
    else if (nSprmId == NS_ooxml::LN_CT_TrPr_ins)
    {
        m_pImpl->SetCurrentRedlineToken(XML_tableRowInsert);
    }
    else if (nSprmId == NS_ooxml::LN_CT_TrPr_del)
    {
        m_pImpl->SetCurrentRedlineToken(XML_tableRowDelete);
    }
    else if (nSprmId == NS_ooxml::LN_CT_TcPrBase_cellIns)
    {
        m_pImpl->SetCurrentRedlineToken(XML_tableCellInsert);
    }
    else if (nSprmId == NS_ooxml::LN_CT_TcPrBase_cellDel)
    {
        m_pImpl->SetCurrentRedlineToken(XML_tableCellDelete);
    }

    resolveSprmProps(*this, rSprm );
    // now the properties author, date and id should be available
    sal_Int32 nToken = m_pImpl->GetCurrentRedlineToken();
    switch( nToken & 0xffff )
    {
        case XML_mod:
        case XML_ins:
        case XML_del:
        case XML_moveTo:
        case XML_moveFrom:
        case XML_ParagraphFormat:
        case XML_tableRowInsert:
        case XML_tableRowDelete:
        case XML_tableCellInsert:
        case XML_tableCellDelete:
            break;
        default: OSL_FAIL( "redline token other than mod, ins, del, moveTo, moveFrom or table row" ); break;
    }
    m_pImpl->EndParaMarkerChange( );
    m_pImpl->SetCurrentRedlineIsRead();
}

void DomainMapper::finishParagraph(const bool bRemove, const bool bNoNumbering)
{
    if (m_pImpl->m_pSdtHelper->getControlType() == SdtControlType::datePicker)
        m_pImpl->m_pSdtHelper->createDateContentControl();
    m_pImpl->finishParagraph(m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH), bRemove, bNoNumbering);
}

void DomainMapper::commentProps(const OUString& sId, const CommentProperties& rProps)
{
    m_pImpl->commentProps(sId, rProps);
}

css::uno::Reference<css::container::XNameContainer> const & DomainMapper::GetCharacterStyles()
{
    return m_pImpl->GetCharacterStyles();
}

OUString DomainMapper::GetUnusedCharacterStyleName()
{
    return m_pImpl->GetUnusedCharacterStyleName();
}

} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
