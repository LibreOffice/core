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

#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <ooxml/resourceids.hxx>
#include "DomainMapper_Impl.hxx"
#include "ConversionHelper.hxx"
#include "SdtHelper.hxx"
#include "DomainMapperTableHandler.hxx"
#include "TagLogger.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/i18n/NumberFormatMapper.hpp>
#include <com/sun/star/i18n/NumberFormatIndex.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/LineNumberPosition.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/text/ChapterFormat.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/text/SetVariableType.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XLineNumberingProperties.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/text/LabelFollow.hpp>
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XChapterNumberingSupplier.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/text/XRedline.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/RubyPosition.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/style/DropCapFormat.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontRelief.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <comphelper/indexedpropertyvalues.hxx>
#include <editeng/flditem.hxx>
#include <editeng/unotext.hxx>
#include <o3tl/deleter.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/temporary.hxx>
#include <oox/mathml/imexport.hxx>
#include <utility>
#include <xmloff/odffields.hxx>
#include <rtl/uri.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/string.hxx>

#include <dmapper/GraphicZOrderHelper.hxx>

#include <oox/token/tokens.hxx>

#include <cmath>
#include <optional>
#include <map>
#include <tuple>
#include <unordered_map>
#include <regex>
#include <algorithm>

#include <officecfg/Office/Common.hxx>
#include <filter/msfilter/util.hxx>
#include <filter/msfilter/ww8fields.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/propertysequence.hxx>
#include <unotools/mediadescriptor.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>
#include <com/sun/star/drawing/FillStyle.hpp>

#include <unicode/errorcode.h>
#include <unicode/regex.h>
#include <unotxdoc.hxx>
#include <SwXDocumentSettings.hxx>
#include <SwXTextDefaults.hxx>
#include <unobookmark.hxx>

#define REFFLDFLAG_STYLE_FROM_BOTTOM 0xc100
#define REFFLDFLAG_STYLE_HIDE_NON_NUMERICAL 0xc200

using namespace ::com::sun::star;
using namespace oox;
namespace writerfilter::dmapper{

//line numbering for header/footer
static void lcl_linenumberingHeaderFooter( const uno::Reference<container::XNameContainer>& xStyles, const OUString& rname, DomainMapper_Impl* dmapper )
{
    const StyleSheetEntryPtr pEntry = dmapper->GetStyleSheetTable()->FindStyleSheetByISTD( rname );
    if (!pEntry)
        return;
    const StyleSheetPropertyMap* pStyleSheetProperties = pEntry->m_pProperties.get();
    if ( !pStyleSheetProperties )
        return;
    sal_Int32 nListId = pStyleSheetProperties->props().GetListId();
    if( xStyles.is() )
    {
        if( xStyles->hasByName( rname ) )
        {
            uno::Reference< style::XStyle > xStyle;
            xStyles->getByName( rname ) >>= xStyle;
            if( !xStyle.is() )
                return;
            uno::Reference<beans::XPropertySet> xPropertySet( xStyle, uno::UNO_QUERY );
            xPropertySet->setPropertyValue( getPropertyName( PROP_PARA_LINE_NUMBER_COUNT ), uno::Any( nListId >= 0 ) );
        }
    }
}

// Populate Dropdown Field properties from FFData structure
static void lcl_handleDropdownField( const uno::Reference< beans::XPropertySet >& rxFieldProps, const FFDataHandler::Pointer_t& pFFDataHandler )
{
    if ( !rxFieldProps.is() )
        return;

    if ( !pFFDataHandler->getName().isEmpty() )
        rxFieldProps->setPropertyValue( "Name", uno::Any( pFFDataHandler->getName() ) );

    const FFDataHandler::DropDownEntries_t& rEntries = pFFDataHandler->getDropDownEntries();
    uno::Sequence< OUString > sItems( rEntries.size() );
    ::std::copy( rEntries.begin(), rEntries.end(), sItems.getArray());
    if ( sItems.hasElements() )
        rxFieldProps->setPropertyValue( "Items", uno::Any( sItems ) );

    sal_Int32 nResult = pFFDataHandler->getDropDownResult().toInt32();
    if (nResult > 0 && o3tl::make_unsigned(nResult) < sItems.size())
        rxFieldProps->setPropertyValue("SelectedItem", uno::Any(sItems[nResult]));
    if ( !pFFDataHandler->getHelpText().isEmpty() )
         rxFieldProps->setPropertyValue( "Help", uno::Any( pFFDataHandler->getHelpText() ) );
}

static void lcl_handleTextField( const uno::Reference< beans::XPropertySet >& rxFieldProps, const FFDataHandler::Pointer_t& pFFDataHandler )
{
    if ( rxFieldProps.is() && pFFDataHandler )
    {
        rxFieldProps->setPropertyValue
            (getPropertyName(PROP_HINT),
            uno::Any(pFFDataHandler->getStatusText()));
        rxFieldProps->setPropertyValue
            (getPropertyName(PROP_HELP),
            uno::Any(pFFDataHandler->getHelpText()));
        rxFieldProps->setPropertyValue
            (getPropertyName(PROP_CONTENT),
            uno::Any(pFFDataHandler->getTextDefault()));
    }
}

/**
 Very similar to DomainMapper_Impl::GetPropertyFromStyleSheet
 It is focused on paragraph properties search in current & parent stylesheet entries.
 But it will not take into account properties with listid: these are "list paragraph styles" and
 not used in some cases.
*/
static uno::Any lcl_GetPropertyFromParaStyleSheetNoNum(PropertyIds eId, StyleSheetEntryPtr pEntry, const StyleSheetTablePtr& rStyleSheet)
{
    while (pEntry)
    {
        if (pEntry->m_pProperties)
        {
            std::optional<PropertyMap::Property> aProperty =
                pEntry->m_pProperties->getProperty(eId);
            if (aProperty)
            {
                if (pEntry->m_pProperties->props().GetListId())
                    // It is a paragraph style with list. Paragraph list styles are not taken into account
                    return uno::Any();
                else
                    return aProperty->second;
            }
        }
        //search until the property is set or no parent is available
        StyleSheetEntryPtr pNewEntry;
        if (!pEntry->m_sBaseStyleIdentifier.isEmpty())
            pNewEntry = rStyleSheet->FindStyleSheetByISTD(pEntry->m_sBaseStyleIdentifier);

        SAL_WARN_IF(pEntry == pNewEntry, "writerfilter.dmapper", "circular loop in style hierarchy?");

        if (pEntry == pNewEntry) //fdo#49587
            break;

        pEntry = pNewEntry;
    }
    return uno::Any();
}


namespace {

struct FieldConversion
{
    const char*     cFieldServiceName;
    FieldId         eFieldId;
};

}

typedef std::unordered_map<OUString, FieldConversion> FieldConversionMap_t;

/// Gives access to the parent field context of the topmost one, if there is any.
static FieldContextPtr GetParentFieldContext(const std::deque<FieldContextPtr>& rFieldStack)
{
    if (rFieldStack.size() < 2)
    {
        return nullptr;
    }

    return rFieldStack[rFieldStack.size() - 2];
}

/// Decides if the pInner field inside pOuter is allowed in Writer core, depending on their type.
static bool IsFieldNestingAllowed(const FieldContextPtr& pOuter, const FieldContextPtr& pInner)
{
    if (!pInner->GetFieldId())
    {
        return true;
    }

    std::optional<FieldId> oOuterFieldId = pOuter->GetFieldId();
    if (!oOuterFieldId)
    {
        OUString aCommand = pOuter->GetCommand();

        // Ignore leading space before the field name, but don't accept IFF when we check for IF.
        while (aCommand.getLength() > 3 && aCommand[0] == ' ')
            aCommand = aCommand.subView(1);

        if (aCommand.startsWith("IF "))
        {
            // This will be FIELD_IF once the command is closed.
            oOuterFieldId = FIELD_IF;
        }
    }

    if (!oOuterFieldId)
    {
        return true;
    }

    switch (*oOuterFieldId)
    {
        case FIELD_IF:
        {
            switch (*pInner->GetFieldId())
            {
                case FIELD_DOCVARIABLE:
                case FIELD_FORMTEXT:
                case FIELD_FORMULA:
                case FIELD_IF:
                case FIELD_MERGEFIELD:
                case FIELD_REF:
                case FIELD_PAGE:
                case FIELD_NUMPAGES:
                {
                    // LO does not currently know how to evaluate these as conditions or results
                    return false;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }

    return true;
}

DomainMapper_Impl::DomainMapper_Impl(
            DomainMapper& rDMapper,
            uno::Reference<uno::XComponentContext> xContext,
            rtl::Reference<SwXTextDocument> const& xModel,
            SourceDocumentType eDocumentType,
            utl::MediaDescriptor const & rMediaDesc) :
        m_eDocumentType( eDocumentType ),
        m_rDMapper( rDMapper ),
        m_pOOXMLDocument(nullptr),
        m_xTextDocument( xModel ),
        m_xComponentContext(std::move( xContext )),
        m_bForceGenericFields(officecfg::Office::Common::Filter::Microsoft::Import::ForceImportWWFieldsAsGenericFields::get()),
        m_bIsDecimalComma( false ),
        m_bIsFirstSection( true ),
        m_bStartTOC(false),
        m_bStartTOCHeaderFooter(false),
        m_bStartedTOC(false),
        m_bStartIndex(false),
        m_bStartBibliography(false),
        m_nStartGenericField(0),
        m_bTextDeleted(false),
        m_nLastRedlineMovedID(1),
        m_sCurrentPermId(0),
        m_bFrameDirectionSet(false),
        m_bInDocDefaultsImport(false),
        m_bInStyleSheetImport( false ),
        m_bInNumberingImport(false),
        m_bInAnyTableImport( false ),
        m_bDiscardHeaderFooter( false ),
        m_eSkipFootnoteState(SkipFootnoteSeparator::OFF),
        m_nFootnotes(-1),
        m_nEndnotes(-1),
        m_nFirstFootnoteIndex(-1),
        m_nFirstEndnoteIndex(-1),
        m_bLineNumberingSet( false ),
        m_bIsParaMarkerChange( false ),
        m_bIsParaMarkerMove( false ),
        m_bRedlineImageInPreviousRun( false ),
        m_bDummyParaAddedForTableInSection( false ),
        m_bIsLastSectionGroup( false ),
        m_bUsingEnhancedFields( false ),
        m_nAnnotationId( -1 ),
        m_aSmartTagHandler(m_xComponentContext, m_xTextDocument),
        m_xInsertTextRange(rMediaDesc.getUnpackedValueOrDefault("TextInsertModeRange", uno::Reference<text::XTextRange>())),
        m_xAltChunkStartingRange(rMediaDesc.getUnpackedValueOrDefault("AltChunkStartingRange", uno::Reference<text::XTextRange>())),
        m_bIsNewDoc(!rMediaDesc.getUnpackedValueOrDefault("InsertMode", false)),
        m_bIsAltChunk(rMediaDesc.getUnpackedValueOrDefault("AltChunkMode", false)),
        m_bIsReadGlossaries(rMediaDesc.getUnpackedValueOrDefault("ReadGlossaries", false)),
        m_bHasFtnSep(false),
        m_bIsSplitPara(false),
        m_bIsActualParagraphFramed( false ),
        m_bSaxError(false)
{
    m_StreamStateStack.emplace(); // add state for document body
    m_aBaseUrl = rMediaDesc.getUnpackedValueOrDefault(
        utl::MediaDescriptor::PROP_DOCUMENTBASEURL, OUString());
    if (m_aBaseUrl.isEmpty()) {
        m_aBaseUrl = rMediaDesc.getUnpackedValueOrDefault(
            utl::MediaDescriptor::PROP_URL, OUString());
    }

    appendTableManager( );
    GetBodyText();
    if (!m_bIsNewDoc && !m_xBodyText)
    {
        throw uno::Exception("failed to find body text of the insert position", nullptr);
    }

    uno::Reference< text::XTextAppend > xBodyTextAppend( m_xBodyText, uno::UNO_QUERY );
    m_aTextAppendStack.push(TextAppendContext(xBodyTextAppend,
                m_bIsNewDoc ? uno::Reference<text::XTextCursor>() : m_xBodyText->createTextCursorByRange(m_xInsertTextRange)));

    //todo: does it makes sense to set the body text as static text interface?
    uno::Reference< text::XTextAppendAndConvert > xBodyTextAppendAndConvert( m_xBodyText, uno::UNO_QUERY );
    m_pTableHandler = new DomainMapperTableHandler(xBodyTextAppendAndConvert, *this);
    getTableManager( ).setHandler(m_pTableHandler);

    getTableManager( ).startLevel();
    m_bUsingEnhancedFields = !comphelper::IsFuzzing() && officecfg::Office::Common::Filter::Microsoft::Import::ImportWWFieldsAsEnhancedFields::get();

    m_pSdtHelper = new SdtHelper(*this, m_xComponentContext);

    m_aRedlines.push(std::vector<RedlineParamsPtr>());

    if (m_bIsAltChunk)
    {
        m_bIsFirstSection = false;
    }
}


DomainMapper_Impl::~DomainMapper_Impl()
{
    assert(!m_StreamStateStack.empty());
    ChainTextFrames();
    // Don't remove last paragraph when pasting, sw expects that empty paragraph.
    if (m_bIsNewDoc)
    {
        RemoveLastParagraph();
        suppress_fun_call_w_exception(GetStyleSheetTable()->ApplyClonedTOCStyles());
    }
    if (hasTableManager())
    {
        getTableManager().endLevel();
        popTableManager();
    }
}

writerfilter::ooxml::OOXMLDocument* DomainMapper_Impl::getDocumentReference() const
{
    return m_pOOXMLDocument;
}

uno::Reference< container::XNameContainer > const &  DomainMapper_Impl::GetPageStyles()
{
    if(!m_xPageStyles1.is() && m_xTextDocument)
        m_xTextDocument->getStyleFamilies()->getByName("PageStyles") >>= m_xPageStyles1;
    return m_xPageStyles1;
}

OUString DomainMapper_Impl::GetUnusedPageStyleName()
{
    static const char DEFAULT_STYLE[] = "Converted";
    if (!m_xNextUnusedPageStyleNo)
    {
        const uno::Sequence< OUString > aPageStyleNames = GetPageStyles()->getElementNames();
        sal_Int32         nMaxIndex       = 0;
        // find the highest number x in each style with the name "DEFAULT_STYLE+x" and
        // return an incremented name

        for ( const auto& rStyleName : aPageStyleNames )
        {
            if ( rStyleName.startsWith( DEFAULT_STYLE ) )
            {
                sal_Int32 nIndex = o3tl::toInt32(rStyleName.subView( strlen( DEFAULT_STYLE ) ));
                if ( nIndex > nMaxIndex )
                    nMaxIndex = nIndex;
            }
        }
        m_xNextUnusedPageStyleNo = nMaxIndex + 1;
    }

    OUString sPageStyleName = DEFAULT_STYLE + OUString::number( *m_xNextUnusedPageStyleNo );
    *m_xNextUnusedPageStyleNo = *m_xNextUnusedPageStyleNo + 1;
    return sPageStyleName;
}

uno::Reference< container::XNameContainer > const &  DomainMapper_Impl::GetCharacterStyles()
{
    if(!m_xCharacterStyles.is() && m_xTextDocument)
        m_xTextDocument->getStyleFamilies()->getByName("CharacterStyles") >>= m_xCharacterStyles;
    return m_xCharacterStyles;
}

uno::Reference<container::XNameContainer> const& DomainMapper_Impl::GetParagraphStyles()
{
    if (!m_xParagraphStyles.is() && m_xTextDocument)
        m_xTextDocument->getStyleFamilies()->getByName("ParagraphStyles") >>= m_xParagraphStyles;
    return m_xParagraphStyles;
}

OUString DomainMapper_Impl::GetUnusedCharacterStyleName()
{
    static const char cListLabel[] = "ListLabel ";
    if (!m_xNextUnusedCharacterStyleNo)
    {
        //search for all character styles with the name sListLabel + <index>
        const uno::Sequence< OUString > aCharacterStyleNames = GetCharacterStyles()->getElementNames();
        sal_Int32         nMaxIndex       = 0;
        for ( const auto& rStyleName : aCharacterStyleNames )
        {
            OUString sSuffix;
            if ( rStyleName.startsWith( cListLabel, &sSuffix ) )
            {
                sal_Int32 nSuffix = sSuffix.toInt32();
                if( nSuffix > 0 && nSuffix > nMaxIndex )
                    nMaxIndex = nSuffix;
            }
        }
        m_xNextUnusedCharacterStyleNo = nMaxIndex + 1;
    }

    OUString sPageStyleName = cListLabel + OUString::number( *m_xNextUnusedCharacterStyleNo );
    *m_xNextUnusedCharacterStyleNo = *m_xNextUnusedCharacterStyleNo + 1;
    return sPageStyleName;
}

uno::Reference< text::XText > const & DomainMapper_Impl::GetBodyText()
{
    if(!m_xBodyText.is())
    {
        if (m_xInsertTextRange.is())
            m_xBodyText = m_xInsertTextRange->getText();
        else if (m_xTextDocument.is())
            m_xBodyText = m_xTextDocument->getText();
    }
    return m_xBodyText;
}


rtl::Reference<SwXDocumentSettings> const & DomainMapper_Impl::GetDocumentSettings()
{
    if( !m_xDocumentSettings.is() && m_xTextDocument.is())
    {
        m_xDocumentSettings = m_xTextDocument->createDocumentSettings();
    }
    return m_xDocumentSettings;
}


void DomainMapper_Impl::SetDocumentSettingsProperty( const OUString& rPropName, const uno::Any& rValue )
{
    uno::Reference< beans::XPropertySet > xSettings = GetDocumentSettings();
    if( xSettings.is() )
    {
        try
        {
            xSettings->setPropertyValue( rPropName, rValue );
        }
        catch( const uno::Exception& )
        {
        }
    }
}

namespace
{
void CopyPageDescNameToNextParagraph(const uno::Reference<lang::XComponent>& xParagraph,
                                     const uno::Reference<text::XTextCursor>& xCursor)
{
    // First check if xParagraph has a non-empty page style name to copy from.
    uno::Reference<beans::XPropertySet> xParagraphProps(xParagraph, uno::UNO_QUERY);
    if (!xParagraphProps.is())
    {
        return;
    }

    uno::Any aPageDescName = xParagraphProps->getPropertyValue("PageDescName");
    OUString sPageDescName;
    aPageDescName >>= sPageDescName;
    if (sPageDescName.isEmpty())
    {
        return;
    }

    // If so, search for the next paragraph.
    uno::Reference<text::XParagraphCursor> xParaCursor(xCursor, uno::UNO_QUERY);
    if (!xParaCursor.is())
    {
        return;
    }

    // Create a range till the next paragraph and then enumerate on the range.
    if (!xParaCursor->gotoNextParagraph(/*bExpand=*/true))
    {
        return;
    }

    uno::Reference<container::XEnumerationAccess> xEnumerationAccess(xParaCursor, uno::UNO_QUERY);
    if (!xEnumerationAccess.is())
    {
        return;
    }

    uno::Reference<container::XEnumeration> xEnumeration = xEnumerationAccess->createEnumeration();
    if (!xEnumeration.is())
    {
        return;
    }

    xEnumeration->nextElement();
    if (!xEnumeration->hasMoreElements())
    {
        return;
    }

    // We found a next item in the enumeration: it's usually a paragraph, but may be a table as
    // well.
    uno::Reference<beans::XPropertySet> xNextParagraph(xEnumeration->nextElement(), uno::UNO_QUERY);
    if (!xNextParagraph.is())
    {
        return;
    }

    // See if there is page style set already: if so, don't touch it.
    OUString sNextPageDescName;
    xNextParagraph->getPropertyValue("PageDescName") >>= sNextPageDescName;
    if (!sNextPageDescName.isEmpty())
    {
        return;
    }

    // Finally copy it over, so it's not lost.
    xNextParagraph->setPropertyValue("PageDescName", aPageDescName);
}
}

void DomainMapper_Impl::RemoveDummyParaForTableInSection()
{
    SetIsDummyParaAddedForTableInSection(false);
    PropertyMapPtr pContext = GetTopContextOfType(CONTEXT_SECTION);
    SectionPropertyMap* pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
    if (!pSectionContext)
        return;

    if (m_aTextAppendStack.empty())
        return;
    uno::Reference< text::XTextAppend > xTextAppend = m_aTextAppendStack.top().xTextAppend;
    if (!xTextAppend.is())
        return;

    uno::Reference< text::XTextCursor > xCursor = xTextAppend->createTextCursorByRange(pSectionContext->GetStartingRange());

    // Remove the extra NumPicBullets from the document,
    // which get attached to the first paragraph in the
    // document
    ListsManager::Pointer pListTable = GetListTable();
    pListTable->DisposeNumPicBullets();

    uno::Reference<container::XEnumerationAccess> xEnumerationAccess(xCursor, uno::UNO_QUERY);
    if (xEnumerationAccess.is() && m_aTextAppendStack.size() == 1 )
    {
        uno::Reference<container::XEnumeration> xEnumeration = xEnumerationAccess->createEnumeration();
        uno::Reference<lang::XComponent> xParagraph(xEnumeration->nextElement(), uno::UNO_QUERY);
        // Make sure no page breaks are lost.
        CopyPageDescNameToNextParagraph(xParagraph, xCursor);
        xParagraph->dispose();
    }
}
void DomainMapper_Impl::AddDummyParaForTableInSection()
{
    // Shapes and textboxes can't have sections.
    if (IsInShape() || m_StreamStateStack.top().bIsInTextBox)
        return;

    if (!m_aTextAppendStack.empty())
    {
        uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
        if (xTextAppend.is())
        {
            xTextAppend->finishParagraph(  uno::Sequence< beans::PropertyValue >() );
            SetIsDummyParaAddedForTableInSection(true);
        }
    }
}

 static OUString lcl_FindLastBookmark(const uno::Reference<text::XTextCursor>& xCursor,
                                     bool bAlreadyExpanded)
 {
     OUString sName;
     if (!xCursor.is())
         return sName;

     // Select 1 previous element
     if (!bAlreadyExpanded)
         xCursor->goLeft(1, true);
     comphelper::ScopeGuard unselectGuard(
         [xCursor, bAlreadyExpanded]()
         {
             if (!bAlreadyExpanded)
                 xCursor->goRight(1, true);
         });

     uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCursor, uno::UNO_QUERY);
     if (!xParaEnumAccess.is())
         return sName;

     // Iterate through selection paragraphs
     uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
     if (!xParaEnum->hasMoreElements())
         return sName;

     // Iterate through first para portions
     uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(),
                                                                  uno::UNO_QUERY_THROW);
     uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
     while (xRunEnum->hasMoreElements())
     {
         uno::Reference<beans::XPropertySet> xProps(xRunEnum->nextElement(), uno::UNO_QUERY_THROW);
         uno::Any aType(xProps->getPropertyValue("TextPortionType"));
         OUString sType;
         aType >>= sType;
         if (sType == "Bookmark")
         {
             uno::Reference<container::XNamed> xBookmark(xProps->getPropertyValue("Bookmark"),
                                                         uno::UNO_QUERY_THROW);
             sName = xBookmark->getName();
             // Do not stop the scan here. Maybe there are 2 bookmarks?
         }
     }

     return sName;
 }

static void reanchorObjects(const uno::Reference<uno::XInterface>& xFrom,
                            const uno::Reference<text::XTextRange>& xTo,
                            const uno::Reference<drawing::XDrawPage>& xDrawPage)
{
    std::vector<uno::Reference<text::XTextContent>> aShapes;
    bool bFastPathDone = false;
    if (uno::Reference<beans::XPropertySet> xProps{ xFrom, uno::UNO_QUERY })
    {
        try
        {
            // See SwXParagraph::Impl::GetPropertyValues_Impl
            uno::Sequence<uno::Reference<text::XTextContent>> aSeq;
            xProps->getPropertyValue(u"OOXMLImport_AnchoredShapes"_ustr) >>= aSeq;
            aShapes.insert(aShapes.end(), aSeq.begin(), aSeq.end());
            bFastPathDone = true;
        }
        catch (const uno::Exception&)
        {
        }
    }

    if (!bFastPathDone)
    {
        // Can this happen? Fallback to slow DrawPage iteration and range comparison
        uno::Reference<text::XTextRange> xRange(xFrom, uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextRangeCompare> xCompare(xRange->getText(), uno::UNO_QUERY_THROW);

        const sal_Int32 count = xDrawPage->getCount();
        for (sal_Int32 i = 0; i < count; ++i)
        {
            try
            {
                uno::Reference<text::XTextContent> xShape(xDrawPage->getByIndex(i),
                                                          uno::UNO_QUERY_THROW);
                uno::Reference<text::XTextRange> xAnchor(xShape->getAnchor(), uno::UNO_SET_THROW);
                if (xCompare->compareRegionStarts(xAnchor, xRange) <= 0
                    && xCompare->compareRegionEnds(xAnchor, xRange) >= 0)
                {
                    aShapes.push_back(xShape);
                }
            }
            catch (const uno::Exception&)
            {
                // Can happen e.g. in compareRegion*, when the shape is in a header,
                // and paragraph in body
            }
        }
    }

    for (const auto& xShape : aShapes)
        xShape->attach(xTo);
}

void DomainMapper_Impl::RemoveLastParagraph( )
{
    if (m_bDiscardHeaderFooter)
        return;

    if (m_aTextAppendStack.empty())
        return;

    uno::Reference< text::XTextAppend > xTextAppend = m_aTextAppendStack.top().xTextAppend;
    if (!xTextAppend.is())
        return;

    if (hasTableManager() && getTableManager().getCurrentTablePosition().getLength() != 0)
    {
        // If we have an open floating table, then don't remove this paragraph, since that'll be the
        // anchor of the floating table. Otherwise we would lose the table.
        return;
    }

    try
    {
        uno::Reference< text::XTextCursor > xCursor;
        if (m_bIsNewDoc)
        {
            xCursor = xTextAppend->createTextCursor();
            xCursor->gotoEnd(false);
        }
        else
            xCursor.set(m_aTextAppendStack.top().xCursor, uno::UNO_SET_THROW);

        // Keep the character properties of the last but one paragraph, even if
        // it's empty. This works for headers/footers, and maybe in other cases
        // as well, but surely not in textboxes.
        // fdo#58327: also do this at the end of the document: when pasting,
        // a table before the cursor position would be deleted
        bool const bEndOfDocument(m_aTextAppendStack.size() == 1);

        uno::Reference<lang::XComponent> xParagraph;
        if (IsInHeaderFooter() || bEndOfDocument)
        {
            if (uno::Reference<container::XEnumerationAccess> xEA{ xCursor, uno::UNO_QUERY })
            {
                xParagraph.set(xEA->createEnumeration()->nextElement(), uno::UNO_QUERY);
            }
        }

        xCursor->goLeft(1, true);
        // If this is a text on a shape, possibly the text has the trailing
        // newline removed already. RTF may also not have the trailing newline.
        if (!(xCursor->getString() == SAL_NEWLINE_STRING ||
              // tdf#105444 comments need an exception, if SAL_NEWLINE_STRING defined as "\r\n"
              (sizeof(SAL_NEWLINE_STRING) - 1 == 2 && xCursor->getString() == "\n")))
            return;

        if (!m_xTextDocument)
            return;

        static constexpr OUString RecordChanges(u"RecordChanges"_ustr);

        comphelper::ScopeGuard redlineRestore(
            [this, aPreviousValue = m_xTextDocument->getPropertyValue(RecordChanges)]()
            { m_xTextDocument->setPropertyValue(RecordChanges, aPreviousValue); });

        // disable redlining, otherwise we might end up with an unwanted recorded operations
        m_xTextDocument->setPropertyValue(RecordChanges, uno::Any(false));

        if (xParagraph)
        {
            // move all anchored objects to the previous paragraph
            auto xDrawPage = m_xTextDocument->getDrawPage();
            if (xDrawPage && xDrawPage->hasElements())
            {
                // Cursor already spans two paragraphs
                uno::Reference<container::XEnumerationAccess> xEA(xCursor,
                                                                  uno::UNO_QUERY_THROW);
                auto xEnumeration = xEA->createEnumeration();
                uno::Reference<text::XTextRange> xPrevParagraph(xEnumeration->nextElement(),
                                                                uno::UNO_QUERY_THROW);
                reanchorObjects(xParagraph, xPrevParagraph, xDrawPage);
            }

            xParagraph->dispose();
        }
        else
        {
            // Try to find and remember last bookmark in document: it potentially
            // can be deleted by xCursor->setString() but not by xParagraph->dispose()
            OUString sLastBookmarkName;
            if (bEndOfDocument)
                sLastBookmarkName = lcl_FindLastBookmark(xCursor, true);

            // The cursor already selects across the paragraph break
            // delete
            xCursor->setString(OUString());

            // call to xCursor->setString possibly did remove final bookmark
            // from previous paragraph. We need to restore it, if there was any.
            if (sLastBookmarkName.getLength())
            {
                OUString sBookmarkNameAfterRemoval = lcl_FindLastBookmark(xCursor, false);
                if (sBookmarkNameAfterRemoval.isEmpty())
                {
                    // Yes, it was removed. Restore
                    rtl::Reference<SwXBookmark> xBookmark(m_xTextDocument->createBookmark());
                    xBookmark->setName(sLastBookmarkName);
                    xTextAppend->insertTextContent(xCursor, xBookmark, !xCursor->isCollapsed());
                }
            }
        }
    }
    catch( const uno::Exception& )
    {
    }
}


void DomainMapper_Impl::SetIsLastSectionGroup( bool bIsLast )
{
    m_bIsLastSectionGroup = bIsLast;
}

void DomainMapper_Impl::SetIsLastParagraphInSection( bool bIsLast )
{
    m_StreamStateStack.top().bIsLastParaInSection = bIsLast;
}


void DomainMapper_Impl::SetIsFirstParagraphInSection( bool bIsFirst )
{
    m_StreamStateStack.top().bIsFirstParaInSection = bIsFirst;
}

void DomainMapper_Impl::SetIsFirstParagraphInSectionAfterRedline( bool bIsFirstAfterRedline )
{
    m_StreamStateStack.top().bIsFirstParaInSectionAfterRedline = bIsFirstAfterRedline;
}

bool DomainMapper_Impl::GetIsFirstParagraphInSection( bool bAfterRedline ) const
{
    // Anchored objects may include multiple paragraphs,
    // and none of them should be considered the first para in section.
    return (bAfterRedline
                    ? m_StreamStateStack.top().bIsFirstParaInSectionAfterRedline
                    : m_StreamStateStack.top().bIsFirstParaInSection)
                && !IsInShape()
                && !IsInComments()
                && !IsInFootOrEndnote();
}

void DomainMapper_Impl::SetIsFirstParagraphInShape(bool bIsFirst)
{
    m_StreamStateStack.top().bIsFirstParaInShape = bIsFirst;
}

void DomainMapper_Impl::SetIsDummyParaAddedForTableInSection( bool bIsAdded )
{
    m_bDummyParaAddedForTableInSection = bIsAdded;
}


void DomainMapper_Impl::SetIsTextFrameInserted( bool bIsInserted )
{
    m_StreamStateStack.top().bTextFrameInserted = bIsInserted;
}

void DomainMapper_Impl::SetParaSectpr(bool bParaSectpr)
{
    m_StreamStateStack.top().bParaSectpr = bParaSectpr;
}

void DomainMapper_Impl::SetSdt(bool bSdt)
{
    m_StreamStateStack.top().bSdt = bSdt;

    if (m_StreamStateStack.top().bSdt && !m_aTextAppendStack.empty())
    {
        m_StreamStateStack.top().xSdtEntryStart = GetTopTextAppend()->getEnd();
    }
    else
    {
        m_StreamStateStack.top().xSdtEntryStart.clear();
    }
}

void DomainMapper_Impl::PushSdt()
{
    if (m_aTextAppendStack.empty())
    {
        return;
    }

    uno::Reference<text::XTextAppend> xTextAppend = m_aTextAppendStack.top().xTextAppend;
    if (!xTextAppend.is())
    {
        return;
    }

    // This may delete text, so call it early, before we would set our start position, which may be
    // invalidated by a delete.
    MergeAtContentImageRedlineWithNext(xTextAppend);

    uno::Reference<text::XText> xText = xTextAppend->getText();
    if (!xText.is())
    {
        return;
    }

    uno::Reference<text::XTextCursor> xCursor
        = xText->createTextCursorByRange(xTextAppend->getEnd());
    // Offset so the cursor is not adjusted as we import the SDT's content.
    bool bStart = !xCursor->goLeft(1, /*bExpand=*/false);
    m_xSdtStarts.push({bStart, OUString(), xCursor->getStart()});
}

const std::stack<BookmarkInsertPosition>& DomainMapper_Impl::GetSdtStarts() const
{
    return m_xSdtStarts;
}

void DomainMapper_Impl::PopSdt()
{
    if (m_xSdtStarts.empty())
    {
        return;
    }

    BookmarkInsertPosition aPosition = m_xSdtStarts.top();
    m_xSdtStarts.pop();
    uno::Reference<text::XTextRange> xStart = aPosition.m_xTextRange;
    uno::Reference<text::XTextRange> xEnd = GetTopTextAppend()->getEnd();
    uno::Reference<text::XText> xText = xEnd->getText();

    uno::Reference<text::XTextCursor> xCursor;
    try
    {
        xCursor = xText->createTextCursorByRange(xStart);
    }
    catch (const uno::RuntimeException&)
    {
        TOOLS_WARN_EXCEPTION("writerfilter", "DomainMapper_Impl::DomainMapper_Impl::PopSdt: createTextCursorByRange() failed");
        // We redline form controls and that gets us confused when
        // we process the SDT around the placeholder. What seems to
        // happen is we lose the text-range when we pop the SDT position.
        // Here, we reset the text-range when we fail to create the
        // cursor from the top SDT position.
        if (m_aTextAppendStack.empty())
        {
            return;
        }

        uno::Reference<text::XTextAppend> xTextAppend = m_aTextAppendStack.top().xTextAppend;
        if (!xTextAppend.is())
        {
            return;
        }

        uno::Reference<text::XText> xText2 = xTextAppend->getText();
        if (!xText2.is())
        {
            return;
        }

        // Reset to the start.
        xCursor = xText2->createTextCursorByRange(xTextAppend->getStart());
    }

    if (!xCursor)
    {
        SAL_WARN("writerfilter.dmapper", "DomainMapper_Impl::PopSdt: no start position");
        return;
    }

    if (aPosition.m_bIsStartOfText)
    {
        // Go to the start of the end's paragraph. This helps in case
        // DomainMapper_Impl::AddDummyParaForTableInSection() would make our range multi-paragraph,
        // while the intention is to keep start/end inside the same paragraph for run SDTs.
        uno::Reference<text::XParagraphCursor> xParagraphCursor(xCursor, uno::UNO_QUERY);
        if (xParagraphCursor.is()
            && m_pSdtHelper->GetSdtType() == NS_ooxml::LN_CT_SdtRun_sdtContent)
        {
            xCursor->gotoRange(xEnd, /*bExpand=*/false);
            xParagraphCursor->gotoStartOfParagraph(/*bExpand=*/false);
        }
    }
    else
    {
        // Undo the goLeft() in DomainMapper_Impl::PushSdt();
        xCursor->goRight(1, /*bExpand=*/false);
    }
    xCursor->gotoRange(xEnd, /*bExpand=*/true);

    std::optional<OUString> oData = m_pSdtHelper->getValueFromDataBinding();
    if (oData.has_value())
    {
        // Data binding has a value for us, prefer that over the in-document value.
        xCursor->setString(*oData);

        // Such value is always a plain text string, remove the char style of the placeholder.
        uno::Reference<beans::XPropertyState> xPropertyState(xCursor, uno::UNO_QUERY);
        if (xPropertyState.is())
        {
            xPropertyState->setPropertyToDefault("CharStyleName");
        }
    }

    uno::Reference<text::XTextContent> xContentControl(
        m_xTextDocument->createInstance("com.sun.star.text.ContentControl"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    if (m_pSdtHelper->GetShowingPlcHdr())
    {
        xContentControlProps->setPropertyValue("ShowingPlaceHolder",
                                               uno::Any(m_pSdtHelper->GetShowingPlcHdr()));
    }

    if (!m_pSdtHelper->GetPlaceholderDocPart().isEmpty())
    {
        xContentControlProps->setPropertyValue("PlaceholderDocPart",
                                               uno::Any(m_pSdtHelper->GetPlaceholderDocPart()));
    }

    if (!m_pSdtHelper->GetDataBindingPrefixMapping().isEmpty())
    {
        xContentControlProps->setPropertyValue("DataBindingPrefixMappings",
                                               uno::Any(m_pSdtHelper->GetDataBindingPrefixMapping()));
    }
    if (!m_pSdtHelper->GetDataBindingXPath().isEmpty())
    {
        xContentControlProps->setPropertyValue("DataBindingXpath",
                                               uno::Any(m_pSdtHelper->GetDataBindingXPath()));
    }
    if (!m_pSdtHelper->GetDataBindingStoreItemID().isEmpty())
    {
        xContentControlProps->setPropertyValue("DataBindingStoreItemID",
                                               uno::Any(m_pSdtHelper->GetDataBindingStoreItemID()));
    }

    if (!m_pSdtHelper->GetColor().isEmpty())
    {
        xContentControlProps->setPropertyValue("Color",
                                               uno::Any(m_pSdtHelper->GetColor()));
    }

    if (!m_pSdtHelper->GetAppearance().isEmpty())
    {
        xContentControlProps->setPropertyValue("Appearance",
                                               uno::Any(m_pSdtHelper->GetAppearance()));
    }

    if (!m_pSdtHelper->GetAlias().isEmpty())
    {
        xContentControlProps->setPropertyValue("Alias",
                                               uno::Any(m_pSdtHelper->GetAlias()));
    }

    if (!m_pSdtHelper->GetTag().isEmpty())
    {
        xContentControlProps->setPropertyValue("Tag",
                                               uno::Any(m_pSdtHelper->GetTag()));
    }

    if (m_pSdtHelper->GetId())
    {
        xContentControlProps->setPropertyValue("Id", uno::Any(m_pSdtHelper->GetId()));
    }

    if (m_pSdtHelper->GetTabIndex())
    {
        xContentControlProps->setPropertyValue("TabIndex", uno::Any(m_pSdtHelper->GetTabIndex()));
    }

    if (!m_pSdtHelper->GetLock().isEmpty())
    {
        xContentControlProps->setPropertyValue("Lock", uno::Any(m_pSdtHelper->GetLock()));
    }

    if (m_pSdtHelper->getControlType() == SdtControlType::checkBox)
    {
        xContentControlProps->setPropertyValue("Checkbox", uno::Any(true));

        xContentControlProps->setPropertyValue("Checked", uno::Any(m_pSdtHelper->GetChecked()));

        xContentControlProps->setPropertyValue("CheckedState",
                                               uno::Any(m_pSdtHelper->GetCheckedState()));

        xContentControlProps->setPropertyValue("UncheckedState",
                                               uno::Any(m_pSdtHelper->GetUncheckedState()));
    }

    if (m_pSdtHelper->getControlType() == SdtControlType::dropDown
        || m_pSdtHelper->getControlType() == SdtControlType::comboBox)
    {
        std::vector<OUString>& rDisplayTexts = m_pSdtHelper->getDropDownDisplayTexts();
        std::vector<OUString>& rValues = m_pSdtHelper->getDropDownItems();
        if (rDisplayTexts.size() == rValues.size())
        {
            uno::Sequence<beans::PropertyValues> aItems(rValues.size());
            beans::PropertyValues* pItems = aItems.getArray();
            for (size_t i = 0; i < rValues.size(); ++i)
            {
                pItems[i] = {
                    comphelper::makePropertyValue("DisplayText", rDisplayTexts[i]),
                    comphelper::makePropertyValue("Value", rValues[i])
                };
            }
            xContentControlProps->setPropertyValue("ListItems", uno::Any(aItems));
            if (m_pSdtHelper->getControlType() == SdtControlType::dropDown)
            {
                xContentControlProps->setPropertyValue("DropDown", uno::Any(true));
            }
            else
            {
                xContentControlProps->setPropertyValue("ComboBox", uno::Any(true));
            }
        }
    }

    if (m_pSdtHelper->getControlType() == SdtControlType::picture)
    {
        xContentControlProps->setPropertyValue("Picture", uno::Any(true));
    }

    bool bDateFromDataBinding = false;
    if (m_pSdtHelper->getControlType() == SdtControlType::datePicker)
    {
        xContentControlProps->setPropertyValue("Date", uno::Any(true));
        OUString aDateFormat = m_pSdtHelper->getDateFormat().makeStringAndClear();
        xContentControlProps->setPropertyValue("DateFormat",
                                               uno::Any(aDateFormat.replaceAll("'", "\"")));
        xContentControlProps->setPropertyValue("DateLanguage",
                                               uno::Any(m_pSdtHelper->getLocale().makeStringAndClear()));
        OUString aCurrentDate = m_pSdtHelper->getDate().makeStringAndClear();
        if (oData.has_value())
        {
            aCurrentDate = *oData;
            bDateFromDataBinding = true;
        }
        xContentControlProps->setPropertyValue("CurrentDate",
                                               uno::Any(aCurrentDate));
    }

    if (m_pSdtHelper->getControlType() == SdtControlType::plainText)
    {
        xContentControlProps->setPropertyValue("PlainText", uno::Any(true));
    }

    xText->insertTextContent(xCursor, xContentControl, /*bAbsorb=*/true);

    if (bDateFromDataBinding)
    {
        OUString aDateString;
        xContentControlProps->getPropertyValue("DateString") >>= aDateString;
        xCursor->setString(aDateString);
    }

    m_pSdtHelper->clear();
}

void    DomainMapper_Impl::PushProperties(ContextType eId)
{
    PropertyMapPtr pInsert(eId == CONTEXT_SECTION ?
        (new SectionPropertyMap( m_bIsFirstSection )) :
        eId == CONTEXT_PARAGRAPH ? new ParagraphPropertyMap :  new PropertyMap);
    if(eId == CONTEXT_SECTION)
    {
        if( m_bIsFirstSection )
            m_bIsFirstSection = false;
        // beginning with the second section group a section has to be inserted
        // into the document
        SectionPropertyMap* pSectionContext_ = dynamic_cast< SectionPropertyMap* >( pInsert.get() );
        if (!m_aTextAppendStack.empty())
        {
            uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
            if (xTextAppend.is() && pSectionContext_)
                pSectionContext_->SetStart( xTextAppend->getEnd() );
        }
    }
    if(eId == CONTEXT_PARAGRAPH && m_bIsSplitPara)
    {
        // Some paragraph properties only apply at the beginning of the paragraph - apply only once.
        if (!IsFirstRun())
        {
            auto pParaContext = static_cast<ParagraphPropertyMap*>(GetTopContextOfType(eId).get());
            pParaContext->props().SetListId(-1);
            pParaContext->Erase(PROP_NUMBERING_RULES); // only true with column, not page break
            pParaContext->Erase(PROP_NUMBERING_LEVEL);
            pParaContext->Erase(PROP_NUMBERING_TYPE);
            pParaContext->Erase(PROP_START_WITH);

            pParaContext->Insert(PROP_PARA_TOP_MARGIN, uno::Any(sal_uInt32(0)));
            pParaContext->Erase(PROP_PARA_TOP_MARGIN_BEFORE_AUTO_SPACING);
            pParaContext->Insert(PROP_PARA_FIRST_LINE_INDENT, uno::Any(sal_uInt32(0)));
        }

        m_aPropertyStacks[eId].push( GetTopContextOfType(eId));
        m_bIsSplitPara = false;
    }
    else
    {
        m_aPropertyStacks[eId].push( pInsert );
    }
    m_aContextStack.push(eId);

    m_pTopContext = m_aPropertyStacks[eId].top();
}


void DomainMapper_Impl::PushStyleProperties( const PropertyMapPtr& pStyleProperties )
{
    m_aPropertyStacks[CONTEXT_STYLESHEET].push( pStyleProperties );
    m_aContextStack.push(CONTEXT_STYLESHEET);

    m_pTopContext = m_aPropertyStacks[CONTEXT_STYLESHEET].top();
}


void DomainMapper_Impl::PushListProperties(const PropertyMapPtr& pListProperties)
{
    m_aPropertyStacks[CONTEXT_LIST].push( pListProperties );
    m_aContextStack.push(CONTEXT_LIST);
    m_pTopContext = m_aPropertyStacks[CONTEXT_LIST].top();
}


void    DomainMapper_Impl::PopProperties(ContextType eId)
{
    OSL_ENSURE(!m_aPropertyStacks[eId].empty(), "section stack already empty");
    if ( m_aPropertyStacks[eId].empty() )
        return;

    if ( eId == CONTEXT_SECTION )
    {
        if (m_aPropertyStacks[eId].size() == 1) // tdf#112202 only top level !!!
        {
            m_pLastSectionContext = dynamic_cast< SectionPropertyMap* >( m_aPropertyStacks[eId].top().get() );
            assert(m_pLastSectionContext);
        }
    }
    else if (eId == CONTEXT_CHARACTER)
    {
        m_pLastCharacterContext = m_aPropertyStacks[eId].top();
        // Sadly an assert about deferredCharacterProperties being empty is not possible
        // here, because appendTextPortion() may not be called for every character section.
        m_StreamStateStack.top().deferredCharacterProperties.clear();
    }

    if (!IsInFootOrEndnote() && IsInCustomFootnote() && !m_aPropertyStacks[eId].empty())
    {
        PropertyMapPtr pRet = m_aPropertyStacks[eId].top();
        if (pRet->GetFootnote().is() && m_pFootnoteContext.is())
            EndCustomFootnote();
    }

    m_aPropertyStacks[eId].pop();
    m_aContextStack.pop();
    if(!m_aContextStack.empty() && !m_aPropertyStacks[m_aContextStack.top()].empty())

            m_pTopContext = m_aPropertyStacks[m_aContextStack.top()].top();
    else
    {
        // OSL_ENSURE(eId == CONTEXT_SECTION, "this should happen at a section context end");
        m_pTopContext.clear();
    }
}


PropertyMapPtr DomainMapper_Impl::GetTopContextOfType(ContextType eId)
{
    PropertyMapPtr pRet;
    if(!m_aPropertyStacks[eId].empty())
        pRet = m_aPropertyStacks[eId].top();
    return pRet;
}

bool DomainMapper_Impl::HasTopText() const
{
    return !m_aTextAppendStack.empty();
}

uno::Reference< text::XTextAppend > const &  DomainMapper_Impl::GetTopTextAppend()
{
    OSL_ENSURE(!m_aTextAppendStack.empty(), "text append stack is empty" );
    return m_aTextAppendStack.top().xTextAppend;
}

FieldContextPtr const &  DomainMapper_Impl::GetTopFieldContext()
{
    SAL_WARN_IF(m_aFieldStack.empty(), "writerfilter.dmapper", "Field stack is empty");
    return m_aFieldStack.back();
}

bool DomainMapper_Impl::HasTopAnchoredObjects() const
{
    return !m_aTextAppendStack.empty() && !m_aTextAppendStack.top().m_aAnchoredObjects.empty();
}

void DomainMapper_Impl::InitTabStopFromStyle( const uno::Sequence< style::TabStop >& rInitTabStops )
{
    OSL_ENSURE(m_aCurrentTabStops.empty(), "tab stops already initialized");
    for( const auto& rTabStop : rInitTabStops)
    {
        m_aCurrentTabStops.emplace_back(rTabStop);
    }
}

void DomainMapper_Impl::IncorporateTabStop( const DeletableTabStop &  rTabStop )
{
    sal_Int32 nConverted = rTabStop.Position;
    auto aIt = std::find_if(m_aCurrentTabStops.begin(), m_aCurrentTabStops.end(),
        [&nConverted](const DeletableTabStop& rCurrentTabStop) { return rCurrentTabStop.Position == nConverted; });
    if( aIt != m_aCurrentTabStops.end() )
    {
        if( rTabStop.bDeleted )
            m_aCurrentTabStops.erase( aIt );
        else
            *aIt = rTabStop;
    }
    else
        m_aCurrentTabStops.push_back( rTabStop );
}


uno::Sequence< style::TabStop > DomainMapper_Impl::GetCurrentTabStopAndClear()
{
    std::vector<style::TabStop> aRet;
    for (const DeletableTabStop& rStop : m_aCurrentTabStops)
    {
        if (!rStop.bDeleted)
            aRet.push_back(rStop);
    }
    m_aCurrentTabStops.clear();
    return comphelper::containerToSequence(aRet);
}

OUString DomainMapper_Impl::GetCurrentParaStyleName()
{
    OUString sName;
    // use saved currParaStyleName as a fallback, in case no particular para style name applied.
    // tdf#134784 except in the case of first paragraph of shapes to avoid bad fallback.
    // TODO fix this "highly inaccurate" m_sCurrentParaStyleName
    if ( !IsInShape() )
        sName = m_StreamStateStack.top().sCurrentParaStyleName;

    PropertyMapPtr pParaContext = GetTopContextOfType(CONTEXT_PARAGRAPH);
    if ( pParaContext && pParaContext->isSet(PROP_PARA_STYLE_NAME) )
        pParaContext->getProperty(PROP_PARA_STYLE_NAME)->second >>= sName;

    // In rare situations the name might still be blank, so use the default style,
    // despite documentation that states, "If this attribute is not specified for any style,
    // then no properties shall be applied to objects of the specified type."
    // Word, however, assigns "Normal" style even in these situations.
    if ( !m_bInStyleSheetImport && sName.isEmpty() )
        sName = GetDefaultParaStyleName();

    return sName;
}

OUString DomainMapper_Impl::GetDefaultParaStyleName()
{
    // After import the default style won't change and is frequently requested: cache the LO style name.
    // TODO assert !InStyleSheetImport? This function really only makes sense once import is finished anyway.
    if ( m_sDefaultParaStyleName.isEmpty() )
    {
        const StyleSheetEntryPtr pEntry = GetStyleSheetTable()->FindDefaultParaStyle();
        if ( pEntry && !pEntry->m_sConvertedStyleName.isEmpty() )
        {
            if ( !m_bInStyleSheetImport )
                m_sDefaultParaStyleName = pEntry->m_sConvertedStyleName;
            return pEntry->m_sConvertedStyleName;
        }
        else
            return "Standard";
    }
    return m_sDefaultParaStyleName;
}

uno::Any DomainMapper_Impl::GetPropertyFromStyleSheet(PropertyIds eId, StyleSheetEntryPtr pEntry, const bool bDocDefaults, const bool bPara, bool* pIsDocDefault)
{
    while(pEntry)
    {
        if(pEntry->m_pProperties)
        {
            std::optional<PropertyMap::Property> aProperty =
                    pEntry->m_pProperties->getProperty(eId);
            if( aProperty )
            {
                if (pIsDocDefault)
                    *pIsDocDefault = pEntry->m_pProperties->isDocDefault(eId);

                return aProperty->second;
            }
        }
        //search until the property is set or no parent is available
        StyleSheetEntryPtr pNewEntry;
        if ( !pEntry->m_sBaseStyleIdentifier.isEmpty() )
            pNewEntry = GetStyleSheetTable()->FindStyleSheetByISTD(pEntry->m_sBaseStyleIdentifier);

        SAL_WARN_IF( pEntry == pNewEntry, "writerfilter.dmapper", "circular loop in style hierarchy?");

        if (pEntry == pNewEntry) //fdo#49587
            break;

        pEntry = pNewEntry;
    }
    // not found in style, try the document's DocDefault properties
    if ( bDocDefaults && bPara )
    {
        const PropertyMapPtr& pDefaultParaProps = GetStyleSheetTable()->GetDefaultParaProps();
        if ( pDefaultParaProps )
        {
            std::optional<PropertyMap::Property> aProperty = pDefaultParaProps->getProperty(eId);
            if ( aProperty )
            {
                if (pIsDocDefault)
                    *pIsDocDefault = true;

                return aProperty->second;
            }
        }
    }
    if ( bDocDefaults && isCharacterProperty(eId) )
    {
        const PropertyMapPtr& pDefaultCharProps = GetStyleSheetTable()->GetDefaultCharProps();
        if ( pDefaultCharProps )
        {
            std::optional<PropertyMap::Property> aProperty = pDefaultCharProps->getProperty(eId);
            if ( aProperty )
            {
                if (pIsDocDefault)
                    *pIsDocDefault = true;

                return aProperty->second;
            }
        }
    }

    if (pIsDocDefault)
        *pIsDocDefault = false;

    return uno::Any();
}

uno::Any DomainMapper_Impl::GetPropertyFromParaStyleSheet(PropertyIds eId)
{
    StyleSheetEntryPtr pEntry;
    if ( m_bInStyleSheetImport )
        pEntry = GetStyleSheetTable()->GetCurrentEntry();
    else
        pEntry = GetStyleSheetTable()->FindStyleSheetByConvertedStyleName(GetCurrentParaStyleName());
    return GetPropertyFromStyleSheet(eId, pEntry, /*bDocDefaults=*/true, /*bPara=*/true);
}

uno::Any DomainMapper_Impl::GetPropertyFromCharStyleSheet(PropertyIds eId, const PropertyMapPtr& rContext)
{
    if ( m_bInStyleSheetImport || eId == PROP_CHAR_STYLE_NAME || !isCharacterProperty(eId) )
        return uno::Any();

    StyleSheetEntryPtr pEntry;
    OUString sCharStyleName;
    if ( GetAnyProperty(PROP_CHAR_STYLE_NAME, rContext) >>= sCharStyleName )
        pEntry = GetStyleSheetTable()->FindStyleSheetByConvertedStyleName(sCharStyleName);
    return GetPropertyFromStyleSheet(eId, pEntry, /*bDocDefaults=*/false, /*bPara=*/false);
}

uno::Any DomainMapper_Impl::GetAnyProperty(PropertyIds eId, const PropertyMapPtr& rContext)
{
    // first look in directly applied attributes
    if ( rContext )
    {
        std::optional<PropertyMap::Property> aProperty = rContext->getProperty(eId);
        if ( aProperty )
            return aProperty->second;
    }

    // then look whether it was directly applied as a paragraph property
    PropertyMapPtr pParaContext = GetTopContextOfType(CONTEXT_PARAGRAPH);
    if (pParaContext && rContext != pParaContext)
    {
        std::optional<PropertyMap::Property> aProperty = pParaContext->getProperty(eId);
        if (aProperty)
            return aProperty->second;
    }

    // then look whether it was inherited from a directly applied character style
    if ( eId != PROP_CHAR_STYLE_NAME && isCharacterProperty(eId) )
    {
        uno::Any aRet = GetPropertyFromCharStyleSheet(eId, rContext);
        if ( aRet.hasValue() )
            return aRet;
    }

    // then look in current paragraph style, and docDefaults
    return GetPropertyFromParaStyleSheet(eId);
}

OUString DomainMapper_Impl::GetListStyleName(sal_Int32 nListId)
{
    auto const pList(GetListTable()->GetList( nListId ));
    return pList ? pList->GetStyleName() : OUString();
}

ListsManager::Pointer const & DomainMapper_Impl::GetListTable()
{
    if(!m_pListTable)
        m_pListTable =
            new ListsManager( m_rDMapper, m_xTextDocument );
    return m_pListTable;
}


void DomainMapper_Impl::deferBreak( BreakType deferredBreakType)
{
    assert(!m_StreamStateStack.empty());
    switch (deferredBreakType)
    {
    case LINE_BREAK:
        m_StreamStateStack.top().nLineBreaksDeferred++;
        break;
    case COLUMN_BREAK:
        m_StreamStateStack.top().bIsColumnBreakDeferred = true;
    break;
    case PAGE_BREAK:
            // See SwWW8ImplReader::HandlePageBreakChar(), page break should be
            // ignored inside tables.
            if (0 < m_StreamStateStack.top().nTableDepth)
                return;

            m_StreamStateStack.top().bIsPageBreakDeferred = true;
        break;
    default:
        return;
    }
}

bool DomainMapper_Impl::isBreakDeferred( BreakType deferredBreakType )
{
    assert(!m_StreamStateStack.empty());
    switch (deferredBreakType)
    {
    case LINE_BREAK:
        return 0 < m_StreamStateStack.top().nLineBreaksDeferred;
    case COLUMN_BREAK:
        return m_StreamStateStack.top().bIsColumnBreakDeferred;
    case PAGE_BREAK:
        return m_StreamStateStack.top().bIsPageBreakDeferred;
    default:
        return false;
    }
}

void DomainMapper_Impl::clearDeferredBreak(BreakType deferredBreakType)
{
    assert(!m_StreamStateStack.empty());
    switch (deferredBreakType)
    {
    case LINE_BREAK:
        assert(0 < m_StreamStateStack.top().nLineBreaksDeferred);
        m_StreamStateStack.top().nLineBreaksDeferred--;
        break;
    case COLUMN_BREAK:
        m_StreamStateStack.top().bIsColumnBreakDeferred = false;
        break;
    case PAGE_BREAK:
        m_StreamStateStack.top().bIsPageBreakDeferred = false;
        break;
    default:
        break;
    }
}

void DomainMapper_Impl::clearDeferredBreaks()
{
    assert(!m_StreamStateStack.empty());
    m_StreamStateStack.top().nLineBreaksDeferred = 0;
    m_StreamStateStack.top().bIsColumnBreakDeferred = false;
    m_StreamStateStack.top().bIsPageBreakDeferred = false;
}

void DomainMapper_Impl::setSdtEndDeferred(bool bSdtEndDeferred)
{
    m_StreamStateStack.top().bSdtEndDeferred = bSdtEndDeferred;
}

bool DomainMapper_Impl::isSdtEndDeferred() const
{
    return m_StreamStateStack.top().bSdtEndDeferred;
}

void DomainMapper_Impl::setParaSdtEndDeferred(bool bParaSdtEndDeferred)
{
    m_StreamStateStack.top().bParaSdtEndDeferred = bParaSdtEndDeferred;
}

bool DomainMapper_Impl::isParaSdtEndDeferred() const
{
    return m_StreamStateStack.top().bParaSdtEndDeferred;
}

static void lcl_MoveBorderPropertiesToFrame(std::vector<beans::PropertyValue>& rFrameProperties,
    uno::Reference<text::XTextRange> const& xStartTextRange,
    uno::Reference<text::XTextRange> const& xEndTextRange,
    bool bIsRTFImport)
{
    try
    {
        if (!xStartTextRange.is())   //rhbz#1077780
            return;
        uno::Reference<text::XTextCursor> xRangeCursor = xStartTextRange->getText()->createTextCursorByRange( xStartTextRange );
        xRangeCursor->gotoRange( xEndTextRange, true );

        uno::Reference<beans::XPropertySet> xTextRangeProperties(xRangeCursor, uno::UNO_QUERY);
        if(!xTextRangeProperties.is())
            return ;

        static PropertyIds const aBorderProperties[] =
        {
            PROP_LEFT_BORDER,
            PROP_RIGHT_BORDER,
            PROP_TOP_BORDER,
            PROP_BOTTOM_BORDER,
            PROP_LEFT_BORDER_DISTANCE,
            PROP_RIGHT_BORDER_DISTANCE,
            PROP_TOP_BORDER_DISTANCE,
            PROP_BOTTOM_BORDER_DISTANCE
        };

        // The frame width specified does not include border spacing,
        // so the frame needs to be increased by the left/right para border spacing amount
        sal_Int32 nWidth = 0;
        sal_Int32 nIndexOfWidthProperty = -1;
        sal_Int16 nType = text::SizeType::FIX;
        for (size_t i = 0; nType == text::SizeType::FIX && i < rFrameProperties.size(); ++i)
        {
            if (rFrameProperties[i].Name == "WidthType")
                rFrameProperties[i].Value >>= nType;
            else if (rFrameProperties[i].Name == "Width")
                nIndexOfWidthProperty = i;
        }
        if (nIndexOfWidthProperty > -1 && nType == text::SizeType::FIX)
            rFrameProperties[nIndexOfWidthProperty].Value >>= nWidth;

        for( size_t nProperty = 0; nProperty < SAL_N_ELEMENTS( aBorderProperties ); ++nProperty)
        {
            const OUString & sPropertyName = getPropertyName(aBorderProperties[nProperty]);
            beans::PropertyValue aValue;
            aValue.Name = sPropertyName;
            aValue.Value = xTextRangeProperties->getPropertyValue(sPropertyName);
            if( nProperty < 4 )
            {
                xTextRangeProperties->setPropertyValue( sPropertyName, uno::Any(table::BorderLine2()));
                if (!aValue.Value.hasValue())
                    aValue.Value <<= table::BorderLine2();
            }
            else // border spacing
            {
                sal_Int32 nDistance = 0;
                aValue.Value >>= nDistance;

                // left4/right5 need to be duplicated because of INVERT_BORDER_SPACING (DOCX only)
                // Do not duplicate the top6/bottom7 border spacing.
                if (nProperty > 5 || bIsRTFImport)
                    aValue.Value <<= sal_Int32(0);

                // frames need to be increased by the left/right para border spacing amount
                // This is needed for RTF as well, but that requires other export/import fixes.
                if (!bIsRTFImport && nProperty < 6 && nWidth && nDistance)
                {
                    nWidth += nDistance;
                    rFrameProperties[nIndexOfWidthProperty].Value <<= nWidth;
                }
            }
            if (aValue.Value.hasValue())
                rFrameProperties.push_back(aValue);
        }
    }
    catch( const uno::Exception& )
    {
    }
}


static void lcl_AddRange(
    ParagraphPropertiesPtr const & pToBeSavedProperties,
    uno::Reference< text::XTextAppend > const& xTextAppend,
    TextAppendContext const & rAppendContext)
{
    uno::Reference<text::XParagraphCursor> xParaCursor(
        xTextAppend->createTextCursorByRange( rAppendContext.xInsertPosition.is() ? rAppendContext.xInsertPosition : xTextAppend->getEnd()), uno::UNO_QUERY_THROW );
    pToBeSavedProperties->SetEndingRange(xParaCursor->getStart());
    xParaCursor->gotoStartOfParagraph( false );

    pToBeSavedProperties->SetStartingRange(xParaCursor->getStart());
}


//define some default frame width - 0cm ATM: this allow the frame to be wrapped around the text
constexpr sal_Int32 DEFAULT_FRAME_MIN_WIDTH = 0;
constexpr sal_Int32 DEFAULT_FRAME_MIN_HEIGHT = 0;
constexpr sal_Int32 DEFAULT_VALUE = 0;

std::vector<css::beans::PropertyValue>
DomainMapper_Impl::MakeFrameProperties(const ParagraphProperties& rProps)
{
    std::vector<beans::PropertyValue> aFrameProperties;

    try
    {
        // A paragraph's properties come from direct formatting or somewhere in the style hierarchy
        std::vector<const ParagraphProperties*> vProps;
        vProps.emplace_back(&rProps);
        sal_Int8 nSafetyLimit = 16;
        StyleSheetEntryPtr pStyle
            = GetStyleSheetTable()->FindStyleSheetByConvertedStyleName(rProps.GetParaStyleName());
        while (nSafetyLimit-- && pStyle && pStyle->m_pProperties)
        {
            vProps.emplace_back(&pStyle->m_pProperties->props());
            assert(pStyle->m_sBaseStyleIdentifier != pStyle->m_sStyleName);
            if (pStyle->m_sBaseStyleIdentifier.isEmpty())
                break;
            pStyle = GetStyleSheetTable()->FindStyleSheetByISTD(pStyle->m_sBaseStyleIdentifier);
        }
        SAL_WARN_IF(!nSafetyLimit, "writerfilter.dmapper", "Inheritance loop likely: early exit");


        sal_Int32 nWidth = -1;
        for (const auto pProp : vProps)
        {
            if (pProp->Getw() < 0)
                continue;
            nWidth = pProp->Getw();
            break;
        }
        bool bAutoWidth = nWidth < 1;
        if (bAutoWidth)
            nWidth = DEFAULT_FRAME_MIN_WIDTH;
        aFrameProperties.push_back(
            comphelper::makePropertyValue(getPropertyName(PROP_WIDTH), nWidth));
        aFrameProperties.push_back(
            comphelper::makePropertyValue(getPropertyName(PROP_WIDTH_TYPE),
                                          bAutoWidth ? text::SizeType::MIN : text::SizeType::FIX));

        bool bValidH = false;
        sal_Int32 nHeight = DEFAULT_FRAME_MIN_HEIGHT;
        for (const auto pProp : vProps)
        {
            if (pProp->Geth() < 0)
                continue;
            nHeight = pProp->Geth();
            bValidH = true;
            break;
        }
        aFrameProperties.push_back(
            comphelper::makePropertyValue(getPropertyName(PROP_HEIGHT), nHeight));

        sal_Int16 nhRule = -1;
        for (const auto pProp : vProps)
        {
            if (pProp->GethRule() < 0)
                continue;
            nhRule = pProp->GethRule();
            break;
        }
        if (nhRule < 0)
        {
            if (bValidH && nHeight)
            {
                // [MS-OE376] Word uses a default value of "atLeast" for
                // this attribute when the value of the h attribute is not 0.
                nhRule = text::SizeType::MIN;
            }
            else
            {
                nhRule = text::SizeType::VARIABLE;
            }
        }
        aFrameProperties.push_back(
            comphelper::makePropertyValue(getPropertyName(PROP_SIZE_TYPE), nhRule));

        bool bValidX = false;
        sal_Int32 nX = DEFAULT_VALUE;
        for (const auto pProp : vProps)
        {
            bValidX = pProp->IsxValid();
            if (!bValidX)
                continue;
            nX = pProp->Getx();
            break;
        }
        aFrameProperties.push_back(
            comphelper::makePropertyValue(getPropertyName(PROP_HORI_ORIENT_POSITION), nX));

        sal_Int16 nHoriOrient = text::HoriOrientation::NONE;
        for (const auto pProp : vProps)
        {
            if (pProp->GetxAlign() < 0)
                continue;
            nHoriOrient = pProp->GetxAlign();
            break;
        }
        aFrameProperties.push_back(
            comphelper::makePropertyValue(getPropertyName(PROP_HORI_ORIENT), nHoriOrient));

        //Default the anchor in case FramePr_hAnchor is missing ECMA 17.3.1.11
        sal_Int16 nHAnchor = text::RelOrientation::FRAME; // 'text'
        for (const auto pProp : vProps)
        {
            if (pProp->GethAnchor() < 0)
                continue;
            nHAnchor = pProp->GethAnchor();
            break;
        }
        aFrameProperties.push_back(
            comphelper::makePropertyValue(getPropertyName(PROP_HORI_ORIENT_RELATION), nHAnchor));

        bool bValidY = false;
        sal_Int32 nY = DEFAULT_VALUE;
        for (const auto pProp : vProps)
        {
            bValidY = pProp->IsyValid();
            if (!bValidY)
                continue;
            nY = pProp->Gety();
            break;
        }
        aFrameProperties.push_back(
            comphelper::makePropertyValue(getPropertyName(PROP_VERT_ORIENT_POSITION), nY));

        sal_Int16 nVertOrient = text::VertOrientation::NONE;
        // Testing indicates that yAlign should be ignored if there is any specified w:y
        if (!bValidY)
        {
            for (const auto pProp : vProps)
            {
                if (pProp->GetyAlign() < 0)
                    continue;
                nVertOrient = pProp->GetyAlign();
                break;
            }
        }

        // Default the anchor in case FramePr_vAnchor is missing.
        // ECMA 17.3.1.11 says "page",
        // but errata documentation MS-OE376 2.1.48 Section 2.3.1.11 says "text"
        // while actual testing usually indicates "margin" tdf#157572 tdf#112287
        sal_Int16 nVAnchor = text::RelOrientation::PAGE_PRINT_AREA; // 'margin'
        if (!nY && (bValidY || nVertOrient == text::VertOrientation::NONE))
        {
            // special cases? "auto" position defaults to "paragraph" based on testing when w:y=0
            nVAnchor = text::RelOrientation::FRAME; // 'text'
        }
        for (const auto pProp : vProps)
        {
            if (pProp->GetvAnchor() < 0)
                continue;
            nVAnchor = pProp->GetvAnchor();
            // vAlign is ignored if vAnchor is set to 'text'
            if (nVAnchor == text::RelOrientation::FRAME)
                nVertOrient = text::VertOrientation::NONE;
            break;
        }
        aFrameProperties.push_back(
            comphelper::makePropertyValue(getPropertyName(PROP_VERT_ORIENT_RELATION), nVAnchor));
        aFrameProperties.push_back(
            comphelper::makePropertyValue(getPropertyName(PROP_VERT_ORIENT), nVertOrient));

        text::WrapTextMode nWrap = text::WrapTextMode_NONE;
        for (const auto pProp : vProps)
        {
            if (pProp->GetWrap() == text::WrapTextMode::WrapTextMode_MAKE_FIXED_SIZE)
                continue;
            nWrap = pProp->GetWrap();
            break;
        }
        aFrameProperties.push_back(
            comphelper::makePropertyValue(getPropertyName(PROP_SURROUND), nWrap));

        sal_Int32 nRightDist = 0;
        sal_Int32 nLeftDist = 0;
        for (const auto pProp : vProps)
        {
            if (pProp->GethSpace() < 0)
                continue;
            nLeftDist = nRightDist = pProp->GethSpace();
            break;
        }
        aFrameProperties.push_back(comphelper::makePropertyValue(
            getPropertyName(PROP_LEFT_MARGIN),
            nHoriOrient == text::HoriOrientation::LEFT ? 0 : nLeftDist));
        aFrameProperties.push_back(comphelper::makePropertyValue(
            getPropertyName(PROP_RIGHT_MARGIN),
            nHoriOrient == text::HoriOrientation::RIGHT ? 0 : nRightDist));

        sal_Int32 nBottomDist = 0;
        sal_Int32 nTopDist = 0;
        for (const auto pProp : vProps)
        {
            if (pProp->GetvSpace() < 0)
                continue;
            nTopDist = nBottomDist = pProp->GetvSpace();
            break;
        }
        aFrameProperties.push_back(comphelper::makePropertyValue(
            getPropertyName(PROP_TOP_MARGIN),
            nVertOrient == text::VertOrientation::TOP ? 0 : nTopDist));
        aFrameProperties.push_back(comphelper::makePropertyValue(
            getPropertyName(PROP_BOTTOM_MARGIN),
            nVertOrient == text::VertOrientation::BOTTOM ? 0 : nBottomDist));
    }
    catch (const uno::Exception&)
    {
    }

    return aFrameProperties;
}

void DomainMapper_Impl::CheckUnregisteredFrameConversion(bool bPreventOverlap)
{
    if (m_aTextAppendStack.empty())
        return;
    TextAppendContext& rAppendContext = m_aTextAppendStack.top();
    // n#779642: ignore fly frame inside table as it could lead to messy situations
    if (!rAppendContext.pLastParagraphProperties)
        return;
    if (!rAppendContext.pLastParagraphProperties->IsFrameMode())
        return;
    if (!hasTableManager())
        return;
    if (getTableManager().isInTable())
        return;

    std::vector<beans::PropertyValue> aFrameProperties
        = MakeFrameProperties(*rAppendContext.pLastParagraphProperties);

    if (const std::optional<sal_Int16> nDirection = PopFrameDirection())
    {
        aFrameProperties.push_back(
            comphelper::makePropertyValue(getPropertyName(PROP_FRM_DIRECTION), *nDirection));
    }

    if (bPreventOverlap)
        aFrameProperties.push_back(comphelper::makePropertyValue("AllowOverlap", uno::Any(false)));

    // If there is no fill, the Word default is 100% transparency.
    // Otherwise CellColorHandler has priority, and this setting
    // will be ignored.
    aFrameProperties.push_back(comphelper::makePropertyValue(
        getPropertyName(PROP_BACK_COLOR_TRANSPARENCY), sal_Int32(100)));

    uno::Sequence<beans::PropertyValue> aGrabBag(comphelper::InitPropertySequence(
        { { "ParaFrameProperties", uno::Any(true) } }));
    aFrameProperties.push_back(comphelper::makePropertyValue("FrameInteropGrabBag", aGrabBag));

    lcl_MoveBorderPropertiesToFrame(aFrameProperties,
                                    rAppendContext.pLastParagraphProperties->GetStartingRange(),
                                    rAppendContext.pLastParagraphProperties->GetEndingRange(),
                                    IsRTFImport());

    //frame conversion has to be executed after table conversion, not now
    RegisterFrameConversion(rAppendContext.pLastParagraphProperties->GetStartingRange(),
                            rAppendContext.pLastParagraphProperties->GetEndingRange(),
                            std::move(aFrameProperties));
}

/// Check if the style or its parent has a list id, recursively.
static sal_Int32 lcl_getListId(const StyleSheetEntryPtr& rEntry, const StyleSheetTablePtr& rStyleTable, bool & rNumberingFromBaseStyle)
{
    const StyleSheetPropertyMap* pEntryProperties = rEntry->m_pProperties.get();
    if (!pEntryProperties)
        return -1;

    sal_Int32 nListId = pEntryProperties->props().GetListId();
    // The style itself has a list id.
    if (nListId >= 0)
        return nListId;

    // The style has no parent.
    if (rEntry->m_sBaseStyleIdentifier.isEmpty())
        return -1;

    const StyleSheetEntryPtr pParent = rStyleTable->FindStyleSheetByISTD(rEntry->m_sBaseStyleIdentifier);
    // No such parent style or loop in the style hierarchy.
    if (!pParent || pParent == rEntry)
        return -1;

    rNumberingFromBaseStyle = true;

    return lcl_getListId(pParent, rStyleTable, rNumberingFromBaseStyle);
}

/// Return the paragraph's list level (from styles, unless pParacontext is provided).
/// -1 indicates the level is not set anywhere. [In that case, with a numId, use 0 (level 1)]
///  9 indicates that numbering should be at body level (aka disabled) - rarely used by MSWord.
///  0-8 are the nine valid numbering levels.
sal_Int16 DomainMapper_Impl::GetListLevel(const StyleSheetEntryPtr& pEntry,
                                  const PropertyMapPtr& pParaContext)
{
    sal_Int16 nListLevel = -1;
    if (pParaContext)
    {
        // Deliberately ignore inherited PROP_NUMBERING_LEVEL. Only trust StyleSheetEntry for that.
        std::optional<PropertyMap::Property> aLvl = pParaContext->getProperty(PROP_NUMBERING_LEVEL);
        if (aLvl)
            aLvl->second >>= nListLevel;

        if (nListLevel != -1)
            return nListLevel;
    }

    if (!pEntry)
        return -1;

    const StyleSheetPropertyMap* pEntryProperties = pEntry->m_pProperties.get();
    if (!pEntryProperties)
        return -1;

    nListLevel = pEntryProperties->GetListLevel();
    // The style itself has a list level.
    if (nListLevel >= 0)
        return nListLevel;

    // The style has no parent.
    if (pEntry->m_sBaseStyleIdentifier.isEmpty())
        return -1;

    const StyleSheetEntryPtr pParent = GetStyleSheetTable()->FindStyleSheetByISTD(pEntry->m_sBaseStyleIdentifier);
    // No such parent style or loop in the style hierarchy.
    if (!pParent || pParent == pEntry)
        return -1;

    return GetListLevel(pParent);
}

void DomainMapper_Impl::ValidateListLevel(const OUString& sStyleIdentifierD)
{
    StyleSheetEntryPtr pMyStyle = GetStyleSheetTable()->FindStyleSheetByISTD(sStyleIdentifierD);
    if (!pMyStyle)
        return;

    sal_Int8 nListLevel = GetListLevel(pMyStyle);
    if (nListLevel < 0 || nListLevel >= WW_OUTLINE_MAX)
        return;

    bool bDummy = false;
    sal_Int16 nListId = lcl_getListId(pMyStyle, GetStyleSheetTable(), bDummy);
    if (nListId < 1)
        return;

    auto const pList(GetListTable()->GetList(nListId));
    if (!pList)
        return;

    auto pLevel = pList->GetLevel(nListLevel);
    if (!pLevel && pList->GetAbstractDefinition())
        pLevel = pList->GetAbstractDefinition()->GetLevel(nListLevel);
    if (!pLevel)
        return;

    if (!pLevel->GetParaStyle())
    {
        // First come, first served, and it hasn't been claimed yet, so claim it now.
        pLevel->SetParaStyle(pMyStyle);
    }
    else if (pLevel->GetParaStyle() != pMyStyle)
    {
        // This level is already used by another style, so prevent numbering via this style
        // by setting to body level (9).
        pMyStyle->m_pProperties->SetListLevel(WW_OUTLINE_MAX);
        // WARNING: PROP_NUMBERING_LEVEL is now out of sync with GetListLevel()
    }
}

void DomainMapper_Impl::finishParagraph( const PropertyMapPtr& pPropertyMap, const bool bRemove, const bool bNoNumbering )
{
    if (m_bDiscardHeaderFooter)
        return;

    if (!m_aFieldStack.empty())
    {
        FieldContextPtr pFieldContext = m_aFieldStack.back();
        if (pFieldContext && !pFieldContext->IsCommandCompleted())
        {
            std::vector<OUString> aCommandParts = pFieldContext->GetCommandParts();
            if (!aCommandParts.empty() && aCommandParts[0] == "IF")
            {
                // Conditional text field conditions don't support linebreaks in Writer.
                return;
            }
        }

        if (pFieldContext && pFieldContext->IsCommandCompleted())
        {
            if (pFieldContext->GetFieldId() == FIELD_IF)
            {
                // Conditional text fields can't contain newlines, finish the paragraph later.
                FieldParagraph aFinish{pPropertyMap, bRemove};
                pFieldContext->GetParagraphsToFinish().push_back(aFinish);
                return;
            }
        }
    }

#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("finishParagraph");
#endif

    ParagraphPropertyMap* pParaContext = dynamic_cast< ParagraphPropertyMap* >( pPropertyMap.get() );
    if (m_aTextAppendStack.empty())
        return;
    TextAppendContext& rAppendContext = m_aTextAppendStack.top();
    uno::Reference< text::XTextAppend > xTextAppend(rAppendContext.xTextAppend);
#ifdef DBG_UTIL
    TagLogger::getInstance().attribute("isTextAppend", sal_uInt32(xTextAppend.is()));
#endif

    const StyleSheetEntryPtr pEntry = GetStyleSheetTable()->FindStyleSheetByConvertedStyleName( GetCurrentParaStyleName() );
    SAL_WARN_IF(!pEntry, "writerfilter.dmapper", "no style sheet found");
    const StyleSheetPropertyMap* pStyleSheetProperties = pEntry ? pEntry->m_pProperties.get() : nullptr;
    sal_Int32 nListId = pParaContext ? pParaContext->props().GetListId() : -1;
    bool isNumberingViaStyle(false);
    bool isNumberingViaRule = nListId > -1;
    if ( !bRemove && pStyleSheetProperties && pParaContext )
    {
        if (!pEntry || pEntry->m_nStyleTypeCode != StyleType::STYLE_TYPE_PARA) {
            // We could not resolve paragraph style or it is not a paragraph style
            // Remove this style reference, otherwise it will cause exceptions during further
            // processing and not all paragraph styles will be initialized.
            SAL_WARN("writerfilter.dmapper", "Paragraph style is incorrect. Ignored");
            pParaContext->Erase(PROP_PARA_STYLE_NAME);
        }

        bool bNumberingFromBaseStyle = false;
        if (!isNumberingViaRule)
            nListId = lcl_getListId(pEntry, GetStyleSheetTable(), bNumberingFromBaseStyle);

        //apply numbering level/style to paragraph if it was set at the style, but only if the paragraph itself
        //does not specify the numbering
        sal_Int16 nListLevel = GetListLevel(pEntry, pParaContext);
        // Undefined listLevel with a valid numId is treated as a first level numbering.
        if (nListLevel == -1 && nListId > (IsOOXMLImport() ? 0 : -1))
            nListLevel = 0;

        if (!bNoNumbering && nListLevel >= 0 && nListLevel < 9)
            pParaContext->Insert( PROP_NUMBERING_LEVEL, uno::Any(nListLevel), false );

        auto const pList(GetListTable()->GetList(nListId));
        if (pList && !pParaContext->isSet(PROP_NUMBERING_STYLE_NAME))
        {
            // ListLevel 9 means Body Level/no numbering.
            if (bNoNumbering || nListLevel == 9)
            {
                pParaContext->Insert(PROP_NUMBERING_STYLE_NAME, uno::Any(OUString()), true);
                pParaContext->Erase(PROP_NUMBERING_LEVEL);
            }
            else if ( !isNumberingViaRule )
            {
                isNumberingViaStyle = true;
                // Since LO7.0/tdf#131321 fixed the loss of numbering in styles, this OUGHT to be obsolete,
                // but now other new/critical LO7.0 code expects it, and perhaps some corner cases still need it as well.
                pParaContext->Insert(PROP_NUMBERING_STYLE_NAME, uno::Any(pList->GetStyleName()), true);
            }
            else
            {
                // we have direct numbering, as well as paragraph-style numbering.
                // Apply the style if it uses the same list as the direct numbering,
                // otherwise the directly-applied-to-paragraph status will be lost,
                // and the priority of the numbering-style-indents will be lowered. tdf#133000
                bool bDummy;
                if (nListId == lcl_getListId(pEntry, GetStyleSheetTable(), bDummy))
                    pParaContext->Insert( PROP_NUMBERING_STYLE_NAME, uno::Any(pList->GetStyleName()), true );
            }
        }

        if ( isNumberingViaStyle )
        {
            // When numbering is defined by the paragraph style, then the para-style indents have priority.
            // But since import has just copied para-style's PROP_NUMBERING_STYLE_NAME directly onto the paragraph,
            // the numbering indents now have the priority.
            // So now import must also copy the para-style indents directly onto the paragraph to compensate.
            std::optional<PropertyMap::Property> oProperty;
            const StyleSheetEntryPtr pParent = (!pEntry->m_sBaseStyleIdentifier.isEmpty()) ? GetStyleSheetTable()->FindStyleSheetByISTD(pEntry->m_sBaseStyleIdentifier) : nullptr;
            const StyleSheetPropertyMap* pParentProperties = pParent ? pParent->m_pProperties.get() : nullptr;
            if (!pEntry->m_sBaseStyleIdentifier.isEmpty())
            {
                oProperty = pStyleSheetProperties->getProperty(PROP_PARA_FIRST_LINE_INDENT);
                if ( oProperty
                    // If the numbering comes from a base style, indent of the base style has also priority.
                    || (bNumberingFromBaseStyle && pParentProperties && (oProperty = pParentProperties->getProperty(PROP_PARA_FIRST_LINE_INDENT))) )
                    pParaContext->Insert(PROP_PARA_FIRST_LINE_INDENT, oProperty->second, /*bOverwrite=*/false);
            }
            oProperty = pStyleSheetProperties->getProperty(PROP_PARA_LEFT_MARGIN);
            if ( oProperty
                || (bNumberingFromBaseStyle && pParentProperties && (oProperty = pParentProperties->getProperty(PROP_PARA_LEFT_MARGIN))) )
                pParaContext->Insert(PROP_PARA_LEFT_MARGIN, oProperty->second, /*bOverwrite=*/false);

            // We're inheriting properties from a numbering style. Make sure a possible right margin is inherited from the base style.
            sal_Int32 nParaRightMargin;
            if  ( pParentProperties && (oProperty = pParentProperties->getProperty(PROP_PARA_RIGHT_MARGIN)) && (nParaRightMargin = oProperty->second.get<sal_Int32>()) != 0 )
            {
                // If we're setting the right margin, we should set the first / left margin as well from the numbering style.
                const sal_Int32 nFirstLineIndent = getNumberingProperty(nListId, nListLevel, "FirstLineIndent");
                const sal_Int32 nParaLeftMargin  = getNumberingProperty(nListId, nListLevel, "IndentAt");
                if (nFirstLineIndent != 0)
                    pParaContext->Insert(PROP_PARA_FIRST_LINE_INDENT, uno::Any(nFirstLineIndent), /*bOverwrite=*/false);
                if (nParaLeftMargin != 0)
                    pParaContext->Insert(PROP_PARA_LEFT_MARGIN, uno::Any(nParaLeftMargin), /*bOverwrite=*/false);

                // Override right margin value with value from current style, if any
                if (pStyleSheetProperties && pStyleSheetProperties->isSet(PROP_PARA_RIGHT_MARGIN))
                    nParaRightMargin = pStyleSheetProperties->getProperty(PROP_PARA_RIGHT_MARGIN)->second.get<sal_Int32>();

                pParaContext->Insert(PROP_PARA_RIGHT_MARGIN, uno::Any(nParaRightMargin), /*bOverwrite=*/false);
            }
        }
        // Paragraph style based right paragraph indentation affects not paragraph style based lists in DOCX.
        // Apply it as direct formatting, also left and first line indentation of numbering to keep them.
        else if (isNumberingViaRule)
        {
            uno::Any aRightMargin = GetPropertyFromParaStyleSheet(PROP_PARA_RIGHT_MARGIN);
            if ( aRightMargin != uno::Any() )
            {
                pParaContext->Insert(PROP_PARA_RIGHT_MARGIN, aRightMargin, /*bOverwrite=*/false);

                const sal_Int32 nFirstLineIndent = getNumberingProperty(nListId, nListLevel, "FirstLineIndent");
                const sal_Int32 nParaLeftMargin  = getNumberingProperty(nListId, nListLevel, "IndentAt");
                if (nFirstLineIndent != 0)
                    pParaContext->Insert(PROP_PARA_FIRST_LINE_INDENT, uno::Any(nFirstLineIndent), /*bOverwrite=*/false);
                if (nParaLeftMargin != 0)
                    pParaContext->Insert(PROP_PARA_LEFT_MARGIN, uno::Any(nParaLeftMargin), /*bOverwrite=*/false);
            }
        }

        if (nListId == 0 && !pList)
        {
            // listid = 0 and no list definition is used in DOCX to stop numbering
            // defined somewhere in parent styles
            // And here we should explicitly set left margin and first-line margin.
            // They can be taken from referred style, but not from styles with listid!
            uno::Any aProp = lcl_GetPropertyFromParaStyleSheetNoNum(PROP_PARA_FIRST_LINE_INDENT, pEntry, m_pStyleSheetTable);
            if (aProp.hasValue())
                pParaContext->Insert(PROP_PARA_FIRST_LINE_INDENT, aProp, false);
            else
                pParaContext->Insert(PROP_PARA_FIRST_LINE_INDENT, uno::Any(sal_uInt32(0)), false);

            aProp = lcl_GetPropertyFromParaStyleSheetNoNum(PROP_PARA_LEFT_MARGIN, pEntry, m_pStyleSheetTable);
            if (aProp.hasValue())
                pParaContext->Insert(PROP_PARA_LEFT_MARGIN, aProp, false);
            else
                pParaContext->Insert(PROP_PARA_LEFT_MARGIN, uno::Any(sal_uInt32(0)), false);
        }
    }

    // apply AutoSpacing: it has priority over all other margin settings
    // (note that numbering with autoSpacing is handled separately later on)
    const bool bAllowAdjustments = !GetSettingsTable()->GetDoNotUseHTMLParagraphAutoSpacing();
    sal_Int32 nBeforeAutospacing = -1;
    bool bIsAutoSet = pParaContext && pParaContext->isSet(PROP_PARA_TOP_MARGIN_BEFORE_AUTO_SPACING);
    const bool bNoTopmargin = pParaContext && !pParaContext->isSet(PROP_PARA_TOP_MARGIN);
    // apply INHERITED autospacing only if top margin is not set
    if ( bIsAutoSet || bNoTopmargin )
    {
        GetAnyProperty(PROP_PARA_TOP_MARGIN_BEFORE_AUTO_SPACING, pPropertyMap) >>= nBeforeAutospacing;
        // tdf#137655 only w:beforeAutospacing=0 was specified, but not PARA_TOP_MARGIN
        // (see default_spacing = -1 in processing of LN_CT_Spacing_beforeAutospacing)
        if ( bNoTopmargin && nBeforeAutospacing == ConversionHelper::convertTwipToMM100(-1) )
        {
            sal_Int32 nStyleAuto = -1;
            GetPropertyFromParaStyleSheet(PROP_PARA_TOP_MARGIN_BEFORE_AUTO_SPACING) >>= nStyleAuto;
            if (nStyleAuto > 0)
                nBeforeAutospacing = 0;
        }
    }
    if ( nBeforeAutospacing > -1 && pParaContext )
    {
        if (bAllowAdjustments)
        {
            if ( GetIsFirstParagraphInShape() ||
                 (GetIsFirstParagraphInSection() && GetSectionContext() && GetSectionContext()->IsFirstSection()) ||
                (m_StreamStateStack.top().bFirstParagraphInCell
                 && 0 < m_StreamStateStack.top().nTableDepth
                 && m_StreamStateStack.top().nTableDepth == m_StreamStateStack.top().nTableCellDepth))
            {
                // export requires grabbag to match top_margin, so keep them in sync
                if (nBeforeAutospacing && bIsAutoSet)
                    pParaContext->Insert( PROP_PARA_TOP_MARGIN_BEFORE_AUTO_SPACING, uno::Any( sal_Int32(0) ),true, PARA_GRAB_BAG );
                nBeforeAutospacing = 0;
            }
        }
        pParaContext->Insert(PROP_PARA_TOP_MARGIN, uno::Any(nBeforeAutospacing));
    }

    sal_Int32 nAfterAutospacing = -1;
    bIsAutoSet = pParaContext && pParaContext->isSet(PROP_PARA_BOTTOM_MARGIN_AFTER_AUTO_SPACING);
    const bool bNoBottomMargin = pParaContext && !pParaContext->isSet(PROP_PARA_BOTTOM_MARGIN);
    bool bAppliedBottomAutospacing = false;
    if (bIsAutoSet || bNoBottomMargin)
    {
        GetAnyProperty(PROP_PARA_BOTTOM_MARGIN_AFTER_AUTO_SPACING, pPropertyMap) >>= nAfterAutospacing;
        if (bNoBottomMargin && nAfterAutospacing == ConversionHelper::convertTwipToMM100(-1))
        {
            sal_Int32 nStyleAuto = -1;
            GetPropertyFromParaStyleSheet(PROP_PARA_BOTTOM_MARGIN_AFTER_AUTO_SPACING) >>= nStyleAuto;
            if (nStyleAuto > 0)
                nAfterAutospacing = 0;
        }
    }
    if ( nAfterAutospacing > -1 && pParaContext )
    {
        pParaContext->Insert(PROP_PARA_BOTTOM_MARGIN, uno::Any(nAfterAutospacing));
        bAppliedBottomAutospacing = bAllowAdjustments;
    }

    // tell TableManager to reset the bottom margin if it determines that this is the cell's last paragraph.
    if ( hasTableManager() && getTableManager().isInCell() )
        getTableManager().setCellLastParaAfterAutospacing(bAppliedBottomAutospacing);

    if (xTextAppend.is() && pParaContext && hasTableManager() && !getTableManager().isIgnore())
    {
        try
        {
            /*the following combinations of previous and current frame settings can occur:
                (1) - no old frame and no current frame     -> no special action
                (2) - no old frame and current DropCap      -> save DropCap for later use, don't call finishParagraph
                                                            remove character properties of the DropCap?
                (3) - no old frame and current Frame        -> save Frame for later use
                (4) - old DropCap and no current frame      -> add DropCap to the properties of the finished paragraph, delete previous setting
                (5) - old DropCap and current frame         -> add DropCap to the properties of the finished paragraph, save current frame settings
                (6) - old Frame and new DropCap             -> add old Frame, save DropCap for later use
                (7) - old Frame and new same Frame          -> continue
                (8) - old Frame and new different Frame     -> add old Frame, save new Frame for later use
                (9) - old Frame and no current frame        -> add old Frame, delete previous settings

              old _and_ new DropCap must not occur
             */

            // The paragraph style is vital to knowing all the frame properties.
            std::optional<PropertyMap::Property> aParaStyle
                = pPropertyMap->getProperty(PROP_PARA_STYLE_NAME);
            if (aParaStyle)
            {
                OUString sName;
                aParaStyle->second >>= sName;
                pParaContext->props().SetParaStyleName(sName);
            }

            bool bIsDropCap =
                pParaContext->props().IsFrameMode() &&
                sal::static_int_cast<Id>(pParaContext->props().GetDropCap()) != NS_ooxml::LN_Value_doc_ST_DropCap_none;

            style::DropCapFormat aDrop;
            ParagraphPropertiesPtr pToBeSavedProperties;
            bool bKeepLastParagraphProperties = false;
            if( bIsDropCap )
            {
                uno::Reference<text::XParagraphCursor> xParaCursor(
                    xTextAppend->createTextCursorByRange(xTextAppend->getEnd()), uno::UNO_QUERY_THROW);
                //select paragraph
                xParaCursor->gotoStartOfParagraph( true );
                uno::Reference< beans::XPropertyState > xParaProperties( xParaCursor, uno::UNO_QUERY_THROW );
                xParaProperties->setPropertyToDefault(getPropertyName(PROP_CHAR_ESCAPEMENT));
                xParaProperties->setPropertyToDefault(getPropertyName(PROP_CHAR_HEIGHT));
                //handles (2) and part of (6)
                pToBeSavedProperties = new ParagraphProperties(pParaContext->props());
                sal_Int32 nCount = xParaCursor->getString().getLength();
                pToBeSavedProperties->SetDropCapLength(nCount > 0 && nCount < 255 ? static_cast<sal_Int8>(nCount) : 1);
            }
            if( rAppendContext.pLastParagraphProperties )
            {
                if( sal::static_int_cast<Id>(rAppendContext.pLastParagraphProperties->GetDropCap()) != NS_ooxml::LN_Value_doc_ST_DropCap_none)
                {
                    //handles (4) and part of (5)
                    //create a DropCap property, add it to the property sequence of finishParagraph
                    sal_Int32 nLines = rAppendContext.pLastParagraphProperties->GetLines();
                    aDrop.Lines = nLines > 0 && nLines < SAL_MAX_INT8 ? static_cast<sal_Int8>(nLines) : 2;
                    aDrop.Count = rAppendContext.pLastParagraphProperties->GetDropCapLength();
                    sal_Int32 nHSpace = rAppendContext.pLastParagraphProperties->GethSpace();
                    aDrop.Distance  = nHSpace > 0 && nHSpace < SAL_MAX_INT16 ? static_cast<sal_Int16>(nHSpace) : 0;
                    //completes (5)
                    if( pParaContext->props().IsFrameMode() )
                        pToBeSavedProperties = new ParagraphProperties(pParaContext->props());
                }
                else
                {
                    const bool bIsFrameMode(pParaContext->props().IsFrameMode());
                    std::vector<beans::PropertyValue> aCurrFrameProperties;
                    std::vector<beans::PropertyValue> aPrevFrameProperties;
                    if (bIsFrameMode)
                    {
                        aCurrFrameProperties = MakeFrameProperties(pParaContext->props());
                        aPrevFrameProperties
                            = MakeFrameProperties(*rAppendContext.pLastParagraphProperties);
                    }

                    if (bIsFrameMode && aPrevFrameProperties == aCurrFrameProperties)
                    {
                        //handles (7)
                        rAppendContext.pLastParagraphProperties->SetEndingRange(
                            rAppendContext.xInsertPosition.is() ? rAppendContext.xInsertPosition
                                                                : xTextAppend->getEnd());
                        bKeepLastParagraphProperties = true;
                    }
                    else
                    {
                        // handles (8)(9) and completes (6)

                        // RTF has an \overlap flag (which we ignore so far)
                        // but DOCX has nothing like that for framePr
                        // Always allow overlap in the RTF case - so there can be no regression.

                        // In MSO UI, there is no setting for AllowOverlap for this kind of frame.
                        // Although they CAN overlap with other anchored things,
                        // they do not _easily_ overlap with other framePr's,
                        // so when one frame follows another (8), don't let the first be overlapped.
                        bool bPreventOverlap = !IsRTFImport() && bIsFrameMode && !bIsDropCap;

                        // Preventing overlap is emulation - so deny overlap as little as possible.
                        sal_Int16 nVertOrient = text::VertOrientation::NONE;
                        sal_Int16 nVertOrientRelation = text::RelOrientation::FRAME;
                        sal_Int32 nCurrVertPos = 0;
                        sal_Int32 nPrevVertPos = 0;
                        for (size_t i = 0; bPreventOverlap && i < aCurrFrameProperties.size(); ++i)
                        {
                            if (aCurrFrameProperties[i].Name == "VertOrientRelation")
                            {
                                aCurrFrameProperties[i].Value >>= nVertOrientRelation;
                                if (nVertOrientRelation != text::RelOrientation::FRAME)
                                    bPreventOverlap = false;
                            }
                            else if (aCurrFrameProperties[i].Name == "VertOrient")
                            {
                                aCurrFrameProperties[i].Value >>= nVertOrient;
                                if (nVertOrient != text::VertOrientation::NONE)
                                    bPreventOverlap = false;
                            }
                            else if (aCurrFrameProperties[i].Name == "VertOrientPosition")
                            {
                                aCurrFrameProperties[i].Value >>= nCurrVertPos;
                                // arbitrary value. Assume it must be less than 1st line height
                                if (nCurrVertPos > 20 || nCurrVertPos < -20)
                                    bPreventOverlap = false;
                            }
                        }
                        for (size_t i = 0; bPreventOverlap && i < aPrevFrameProperties.size(); ++i)
                        {
                            if (aPrevFrameProperties[i].Name == "VertOrientRelation")
                            {
                                aPrevFrameProperties[i].Value >>= nVertOrientRelation;
                                if (nVertOrientRelation != text::RelOrientation::FRAME)
                                    bPreventOverlap = false;
                            }
                            else if (aPrevFrameProperties[i].Name == "VertOrient")
                            {
                                aPrevFrameProperties[i].Value >>= nVertOrient;
                                if (nVertOrient != text::VertOrientation::NONE)
                                    bPreventOverlap = false;
                            }
                            else if (aPrevFrameProperties[i].Name == "VertOrientPosition")
                            {
                                aPrevFrameProperties[i].Value >>= nPrevVertPos;
                                if (nPrevVertPos != nCurrVertPos)
                                    bPreventOverlap = false;
                            }
                        }

                        CheckUnregisteredFrameConversion(bPreventOverlap);

                        // If different frame properties are set on this paragraph, keep them.
                        if (!bIsDropCap && bIsFrameMode)
                        {
                            pToBeSavedProperties = new ParagraphProperties(pParaContext->props());
                            lcl_AddRange(pToBeSavedProperties, xTextAppend, rAppendContext);
                        }
                    }
                }
            }
            else
            {
                // (1) doesn't need handling

                if( !bIsDropCap && pParaContext->props().IsFrameMode() )
                {
                    pToBeSavedProperties = new ParagraphProperties(pParaContext->props());
                    lcl_AddRange(pToBeSavedProperties, xTextAppend, rAppendContext);
                }
            }
            applyToggleAttributes(pPropertyMap); // for paragraph marker formatting
            std::vector<beans::PropertyValue> aProperties;
            if (pPropertyMap)
            {
                aProperties = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(pPropertyMap->GetPropertyValues());

                // tdf#64222 filter out the "paragraph marker" formatting and
                // set it as a separate paragraph property, not a empty hint at
                // end of paragraph
                std::vector<beans::NamedValue> charProperties;
                for (auto it = aProperties.begin(); it != aProperties.end(); )
                {
                    // this condition isn't ideal but as it happens all
                    // RES_CHRATR_* have names that start with "Char"
                    if (it->Name.startsWith("Char"))
                    {
                        charProperties.emplace_back(it->Name, it->Value);
                        // as testN793262 demonstrates, font size in rPr must
                        // affect the paragraph size => also insert empty hint!
//                        it = aProperties.erase(it);
                    }
                    ++it;
                }
                if (!charProperties.empty())
                {
                    aProperties.push_back(beans::PropertyValue("ListAutoFormat",
                        0, uno::Any(comphelper::containerToSequence(charProperties)), beans::PropertyState_DIRECT_VALUE));
                }
            }
            if( !bIsDropCap )
            {
                if( aDrop.Lines > 1 )
                {
                    beans::PropertyValue aValue;
                    aValue.Name = getPropertyName(PROP_DROP_CAP_FORMAT);
                    aValue.Value <<= aDrop;
                    aProperties.push_back(aValue);
                }
                uno::Reference< text::XTextRange > xTextRange;
                if (rAppendContext.xInsertPosition.is())
                {
                    xTextRange = xTextAppend->finishParagraphInsert( comphelper::containerToSequence(aProperties), rAppendContext.xInsertPosition );
                    rAppendContext.xCursor->gotoNextParagraph(false);
                    if (rAppendContext.pLastParagraphProperties)
                        rAppendContext.pLastParagraphProperties->SetEndingRange(xTextRange->getEnd());
                }
                else
                {
                    uno::Reference<text::XTextCursor> xCursor;
                    if (m_StreamStateStack.top().bParaHadField
                        && !IsInComments() && !m_xTOCMarkerCursor.is())
                    {
                        // Workaround to make sure char props of the field are not lost.
                        // Not relevant for editeng-based comments.
                        // Not relevant for fields inside a TOC field.
                        xCursor = xTextAppend->getText()->createTextCursor();
                        if (xCursor.is())
                            xCursor->gotoEnd(false);
                        PropertyMapPtr pEmpty(new PropertyMap());
                        appendTextPortion("X", pEmpty);
                    }

                    // Check if top / bottom margin has to be updated, now that we know the numbering status of both the previous and
                    // the current text node.
                    auto itNumberingRules = std::find_if(aProperties.begin(), aProperties.end(), [](const beans::PropertyValue& rValue)
                    {
                        return rValue.Name == "NumberingRules";
                    });

                    assert( isNumberingViaRule == (itNumberingRules != aProperties.end()) );
                    isNumberingViaRule = (itNumberingRules != aProperties.end());
                    if (m_StreamStateStack.top().xPreviousParagraph.is()
                        && (isNumberingViaRule || isNumberingViaStyle))
                    {
                        // This textnode has numbering. Look up the numbering style name of the current and previous paragraph.
                        OUString aCurrentNumberingName;
                        OUString aPreviousNumberingName;
                        if (isNumberingViaRule)
                        {
                            assert(itNumberingRules != aProperties.end() && "by definition itNumberingRules is valid if isNumberingViaRule is true");
                            uno::Reference<container::XNamed> xCurrentNumberingRules(itNumberingRules->Value, uno::UNO_QUERY);
                            if (xCurrentNumberingRules.is())
                                aCurrentNumberingName = xCurrentNumberingRules->getName();
                            try
                            {
                                uno::Reference<container::XNamed> xPreviousNumberingRules(
                                    m_StreamStateStack.top().xPreviousParagraph->getPropertyValue("NumberingRules"),
                                    uno::UNO_QUERY_THROW);
                                aPreviousNumberingName = xPreviousNumberingRules->getName();
                            }
                            catch (const uno::Exception&)
                            {
                                TOOLS_WARN_EXCEPTION("writerfilter", "DomainMapper_Impl::finishParagraph NumberingRules");
                            }
                        }
                        else if (m_StreamStateStack.top().xPreviousParagraph->getPropertySetInfo()->hasPropertyByName("NumberingStyleName")
                                // don't update before tables
                            && (m_StreamStateStack.top().nTableDepth == 0
                                || !m_StreamStateStack.top().bFirstParagraphInCell))
                        {
                            aCurrentNumberingName = GetListStyleName(nListId);
                            m_StreamStateStack.top().xPreviousParagraph->getPropertyValue("NumberingStyleName") >>= aPreviousNumberingName;
                        }

                        // tdf#133363: remove extra auto space even for mixed list styles
                        if (!aPreviousNumberingName.isEmpty()
                            && (aCurrentNumberingName == aPreviousNumberingName
                                || !isNumberingViaRule))
                        {
                            uno::Sequence<beans::PropertyValue> aPrevPropertiesSeq;
                            m_StreamStateStack.top().xPreviousParagraph->getPropertyValue("ParaInteropGrabBag") >>= aPrevPropertiesSeq;
                            const auto & rPrevProperties = aPrevPropertiesSeq;
                            bool bParaAutoBefore = m_StreamStateStack.top().bParaAutoBefore
                                || std::any_of(rPrevProperties.begin(), rPrevProperties.end(), [](const beans::PropertyValue& rValue)
                            {
                                    return rValue.Name == "ParaTopMarginBeforeAutoSpacing";
                            });
                            // if style based spacing was set to auto in the previous paragraph, style of the actual paragraph must be the same
                            if (bParaAutoBefore && !m_StreamStateStack.top().bParaAutoBefore
                                && m_StreamStateStack.top().xPreviousParagraph->getPropertySetInfo()->hasPropertyByName("ParaStyleName"))
                            {
                               auto itParaStyle = std::find_if(aProperties.begin(), aProperties.end(), [](const beans::PropertyValue& rValue)
                               {
                                   return rValue.Name == "ParaStyleName";
                               });
                               bParaAutoBefore = itParaStyle != aProperties.end() &&
                                   m_StreamStateStack.top().xPreviousParagraph->getPropertyValue("ParaStyleName") == itParaStyle->Value;
                            }
                            // There was a previous textnode and it had the same numbering.
                            if (bParaAutoBefore)
                            {
                                // This before spacing is set to auto, set before space to 0.
                                auto itParaTopMargin = std::find_if(aProperties.begin(), aProperties.end(), [](const beans::PropertyValue& rValue)
                                {
                                    return rValue.Name == "ParaTopMargin";
                                });
                                if (itParaTopMargin != aProperties.end())
                                    itParaTopMargin->Value <<= static_cast<sal_Int32>(0);
                                else
                                    aProperties.push_back(comphelper::makePropertyValue("ParaTopMargin", static_cast<sal_Int32>(0)));
                            }

                            bool bPrevParaAutoAfter = std::any_of(rPrevProperties.begin(), rPrevProperties.end(), [](const beans::PropertyValue& rValue)
                            {
                                return rValue.Name == "ParaBottomMarginAfterAutoSpacing";
                            });
                            if (bPrevParaAutoAfter)
                            {
                                // Previous after spacing is set to auto, set previous after space to 0.
                                m_StreamStateStack.top().xPreviousParagraph->setPropertyValue("ParaBottomMargin", uno::Any(static_cast<sal_Int32>(0)));
                            }
                        }
                    }

                    // apply redlines for inline images
                    if (IsParaWithInlineObject())
                    {
                        for (const auto& rAnchored : rAppendContext.m_aAnchoredObjects)
                        {
                            // process only inline objects with redlining
                            if (!rAnchored.m_xRedlineForInline)
                                continue;

                            // select the inline image and set its redline
                            auto xAnchorRange = rAnchored.m_xAnchoredObject->getAnchor();
                            uno::Reference< text::XTextCursor > xCursorOnImage =
                                    xAnchorRange->getText()->createTextCursorByRange(xAnchorRange);
                            xCursorOnImage->goRight(1, true);
                            CreateRedline( xCursorOnImage, rAnchored.m_xRedlineForInline );
                        }
                    }

                    xTextRange = xTextAppend->finishParagraph( comphelper::containerToSequence(aProperties) );
                    m_StreamStateStack.top().xPreviousParagraph.set(xTextRange, uno::UNO_QUERY);

                    if (m_StreamStateStack.top().xPreviousParagraph.is() && // null for SvxUnoTextBase
                        (isNumberingViaStyle || isNumberingViaRule))
                    {
                        assert(pParaContext);
                        if (ListDef::Pointer const& pList = m_pListTable->GetList(nListId))
                        {   // styles could refer to non-existing lists...
                            AbstractListDef::Pointer const& pAbsList =
                                    pList->GetAbstractDefinition();
                            if (pAbsList &&
                                // SvxUnoTextRange doesn't have ListId
                                m_StreamStateStack.top().xPreviousParagraph->getPropertySetInfo()->hasPropertyByName("ListId"))
                            {
                                OUString paraId;
                                m_StreamStateStack.top().xPreviousParagraph->getPropertyValue("ListId") >>= paraId;
                                if (!paraId.isEmpty()) // must be on some list?
                                {
                                    OUString const listId = pAbsList->MapListId(paraId);
                                    if (listId != paraId)
                                    {
                                        m_StreamStateStack.top().xPreviousParagraph->setPropertyValue("ListId", uno::Any(listId));
                                    }
                                }
                            }

                            sal_Int16 nCurrentLevel = GetListLevel(pEntry, pPropertyMap);
                            if (nCurrentLevel == -1)
                                nCurrentLevel = 0;

                            const ListLevel::Pointer pListLevel = pList->GetLevel(nCurrentLevel);
                            if (pListLevel)
                            {
                                sal_Int16 nOverrideLevel = pListLevel->GetStartOverride();
                                if (nOverrideLevel != -1 && m_aListOverrideApplied.find(nListId) == m_aListOverrideApplied.end())
                                {
                                    // Apply override: we have override instruction for this level
                                    // And this was not done for this list before: we can do this only once on first occurrence
                                    // of list with override
                                    // TODO: Not tested variant with different levels override in different lists.
                                    // Probably m_aListOverrideApplied as a set of overridden listids is not sufficient
                                    // and we need to register level overrides separately.
                                    m_StreamStateStack.top().xPreviousParagraph->setPropertyValue("ParaIsNumberingRestart", uno::Any(true));
                                    m_StreamStateStack.top().xPreviousParagraph->setPropertyValue("NumberingStartValue", uno::Any(nOverrideLevel));
                                    m_aListOverrideApplied.insert(nListId);
                                }
                            }
                        }
                    }

                    if (!rAppendContext.m_aAnchoredObjects.empty() && !IsInHeaderFooter())
                    {
                        // Remember what objects are anchored to this paragraph.
                        // That list is only used for Word compat purposes, and
                        // it is only relevant for body text.
                        AnchoredObjectsInfo aInfo;
                        aInfo.m_xParagraph = xTextRange;
                        aInfo.m_aAnchoredObjects = rAppendContext.m_aAnchoredObjects;
                        m_aAnchoredObjectAnchors.push_back(aInfo);
                        rAppendContext.m_aAnchoredObjects.clear();
                    }

                    if (xCursor.is())
                    {
                        xCursor->goLeft(1, true);
                        xCursor->setString(OUString());
                    }
                }
                getTableManager( ).handle(xTextRange);
                m_aSmartTagHandler.handle(xTextRange);

                if (xTextRange.is())
                {
                    // Get the end of paragraph character inserted
                    uno::Reference< text::XTextCursor > xCur = xTextRange->getText( )->createTextCursor( );
                    if (rAppendContext.xInsertPosition.is())
                        xCur->gotoRange( rAppendContext.xInsertPosition, false );
                    else
                        xCur->gotoEnd( false );

                    // tdf#77417 trim right white spaces in table cells in 2010 compatibility mode
                    sal_Int32 nMode = GetSettingsTable()->GetWordCompatibilityMode();
                    if (0 < m_StreamStateStack.top().nTableDepth && 0 < nMode && nMode <= 14)
                    {
                        // skip new line
                        xCur->goLeft(1, false);
                        while ( xCur->goLeft(1, true) )
                        {
                            OUString sChar = xCur->getString();
                            if ( sChar == " " || sChar == "\t" || sChar == OUStringChar(u'\x00A0') )
                                xCur->setString("");
                            else
                                break;
                        }

                        if (rAppendContext.xInsertPosition.is())
                            xCur->gotoRange(rAppendContext.xInsertPosition, false);
                        else
                            xCur->gotoEnd(false);
                    }

                    xCur->goLeft( 1 , true );
                    // Extend the redline ranges for empty paragraphs
                    if (!m_StreamStateStack.top().bParaChanged && m_previousRedline)
                        CreateRedline( xCur, m_previousRedline );
                    CheckParaMarkerRedline( xCur );
                }

                css::uno::Reference<css::beans::XPropertySet> xParaProps(xTextRange, uno::UNO_QUERY);

                // table style precedence and not hidden shapes anchored to hidden empty table paragraphs
                if (xParaProps && !IsInComments()
                    && (0 < m_StreamStateStack.top().nTableDepth
                        || !m_aAnchoredObjectAnchors.empty()))
                {
                    // table style has got bigger precedence than docDefault style
                    // collect these pending paragraph properties to process in endTable()
                    uno::Reference<text::XTextCursor> xCur = xTextRange->getText( )->createTextCursor( );
                    xCur->gotoEnd(false);
                    xCur->goLeft(1, false);
                    uno::Reference<text::XTextCursor> xCur2 =  xTextRange->getText()->createTextCursorByRange(xCur);
                    uno::Reference<text::XParagraphCursor> xParaCursor(xCur2, uno::UNO_QUERY_THROW);
                    xParaCursor->gotoStartOfParagraph(false);
                    if (0 < m_StreamStateStack.top().nTableDepth)
                    {
                        TableParagraph aPending{xParaCursor, xCur, pParaContext, xParaProps};
                        getTableManager().getCurrentParagraphs()->push_back(aPending);
                    }

                    // hidden empty paragraph with a not hidden shape, set as not hidden
                    std::optional<PropertyMap::Property> pHidden;
                    if ( !m_aAnchoredObjectAnchors.empty() && (pHidden = pParaContext->getProperty(PROP_CHAR_HIDDEN)) )
                    {
                        bool bIsHidden = {}; // -Werror=maybe-uninitialized
                        pHidden->second >>= bIsHidden;
                        if (bIsHidden)
                        {
                            bIsHidden = false;
                            pHidden = GetTopContext()->getProperty(PROP_CHAR_HIDDEN);
                            if (pHidden)
                                pHidden->second >>= bIsHidden;
                            if (!bIsHidden)
                            {
                                uno::Reference<text::XTextCursor> xCur3 =  xTextRange->getText()->createTextCursorByRange(xParaCursor);
                                xCur3->goRight(1, true);
                                if (xCur3->getString() == SAL_NEWLINE_STRING)
                                {
                                    uno::Reference< beans::XPropertySet > xProp( xCur3, uno::UNO_QUERY );
                                    xProp->setPropertyValue(getPropertyName(PROP_CHAR_HIDDEN), uno::Any(false));
                                }
                            }
                        }
                    }
                }

                // tdf#118521 set paragraph top or bottom margin based on the paragraph style
                // if we already set the other margin with direct formatting
                if (xParaProps)
                {
                    const bool bTopSet = pParaContext->isSet(PROP_PARA_TOP_MARGIN);
                    const bool bBottomSet = pParaContext->isSet(PROP_PARA_BOTTOM_MARGIN);
                    const bool bContextSet = pParaContext->isSet(PROP_PARA_CONTEXT_MARGIN);
                    if ( bTopSet != bBottomSet || bBottomSet != bContextSet )
                    {

                        if ( !bTopSet )
                        {
                            uno::Any aMargin = GetPropertyFromParaStyleSheet(PROP_PARA_TOP_MARGIN);
                            if ( aMargin != uno::Any() )
                                xParaProps->setPropertyValue("ParaTopMargin", aMargin);
                        }
                        if ( !bBottomSet )
                        {
                            uno::Any aMargin = GetPropertyFromParaStyleSheet(PROP_PARA_BOTTOM_MARGIN);
                            if ( aMargin != uno::Any() )
                                xParaProps->setPropertyValue("ParaBottomMargin", aMargin);
                        }
                        if ( !bContextSet )
                        {
                            uno::Any aMargin = GetPropertyFromParaStyleSheet(PROP_PARA_CONTEXT_MARGIN);
                            if ( aMargin != uno::Any() )
                                xParaProps->setPropertyValue("ParaContextMargin", aMargin);
                        }
                    }
                }

                // Left, Right, and Hanging settings are also grouped. Ensure that all or none are set.
                if (xParaProps)
                {
                    const bool bLeftSet  = pParaContext->isSet(PROP_PARA_LEFT_MARGIN);
                    const bool bRightSet = pParaContext->isSet(PROP_PARA_RIGHT_MARGIN);
                    const bool bFirstSet = pParaContext->isSet(PROP_PARA_FIRST_LINE_INDENT);
                    if (bLeftSet != bRightSet || bRightSet != bFirstSet)
                    {
                        if ( !bLeftSet )
                        {
                            uno::Any aMargin = GetPropertyFromParaStyleSheet(PROP_PARA_LEFT_MARGIN);
                            if ( aMargin != uno::Any() )
                                xParaProps->setPropertyValue("ParaLeftMargin", aMargin);
                            else if (isNumberingViaStyle)
                            {
                                const sal_Int32 nParaLeftMargin = getNumberingProperty(nListId, GetListLevel(pEntry, pPropertyMap), "IndentAt");
                                if (nParaLeftMargin != 0)
                                    xParaProps->setPropertyValue("ParaLeftMargin", uno::Any(nParaLeftMargin));
                            }
                        }
                        if ( !bRightSet )
                        {
                            uno::Any aMargin = GetPropertyFromParaStyleSheet(PROP_PARA_RIGHT_MARGIN);
                            if ( aMargin != uno::Any() )
                                xParaProps->setPropertyValue("ParaRightMargin", aMargin);
                        }
                        if ( !bFirstSet )
                        {
                            uno::Any aMargin = GetPropertyFromParaStyleSheet(PROP_PARA_FIRST_LINE_INDENT);
                            if ( aMargin != uno::Any() )
                                xParaProps->setPropertyValue("ParaFirstLineIndent", aMargin);
                            else if (isNumberingViaStyle)
                            {
                                const sal_Int32 nFirstLineIndent = getNumberingProperty(nListId, GetListLevel(pEntry, pPropertyMap), "FirstLineIndent");
                                if (nFirstLineIndent != 0)
                                    xParaProps->setPropertyValue("ParaFirstLineIndent", uno::Any(nFirstLineIndent));
                            }
                        }
                    }
                }
            }
            if( !bKeepLastParagraphProperties )
                rAppendContext.pLastParagraphProperties = pToBeSavedProperties;
        }
        catch(const lang::IllegalArgumentException&)
        {
            TOOLS_WARN_EXCEPTION( "writerfilter", "DomainMapper_Impl::finishParagraph" );
        }
        catch(const uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION( "writerfilter.dmapper", "finishParagraph()" );
        }

    }

    bool bIgnoreFrameState = IsInHeaderFooter();
    if( (!bIgnoreFrameState && pParaContext && pParaContext->props().IsFrameMode()) || (bIgnoreFrameState && GetIsPreviousParagraphFramed()) )
        SetIsPreviousParagraphFramed(true);
    else
        SetIsPreviousParagraphFramed(false);

    m_StreamStateStack.top().bRemoveThisParagraph = false;
    if( !IsInHeaderFooter() && !IsInShape()
        && (!pParaContext || !pParaContext->props().IsFrameMode()) )
    { // If the paragraph is in a frame, shape or header/footer, it's not a paragraph of the section itself.
        SetIsFirstParagraphInSection(false);
        // don't count an empty deleted paragraph as first paragraph in section to avoid of
        // the deletion of the next empty paragraph later, resulting loss of the associated page break
        if (!m_previousRedline || m_StreamStateStack.top().bParaChanged)
        {
            SetIsFirstParagraphInSectionAfterRedline(false);
            SetIsLastParagraphInSection(false);
        }
    }
    m_previousRedline.clear();
    m_StreamStateStack.top().bParaChanged = false;

    if (IsInComments() && pParaContext)
    {
        if (const OUString sParaId = pParaContext->props().GetParaId(); !sParaId.isEmpty())
        {
            if (const auto& item = m_aCommentProps.find(sParaId); item != m_aCommentProps.end())
            {
                m_bAnnotationResolved = item->second.bDone;
                m_sAnnotationParent = item->second.sParaIdParent;
            }
            m_sAnnotationImportedParaId = sParaId;
        }
    }

    if (m_StreamStateStack.top().bIsFirstParaInShape)
        m_StreamStateStack.top().bIsFirstParaInShape = false;

    if (pParaContext)
    {
        // Reset the frame properties for the next paragraph
        pParaContext->props().ResetFrameProperties();
    }

    SetIsOutsideAParagraph(true);
    m_StreamStateStack.top().bParaHadField = false;

    // don't overwrite m_bFirstParagraphInCell in table separator nodes
    // and in text boxes anchored to the first paragraph of table cells
    if (0 < m_StreamStateStack.top().nTableDepth
        && m_StreamStateStack.top().nTableDepth == m_StreamStateStack.top().nTableCellDepth
        && !IsInShape() && !IsInComments())
    {
        m_StreamStateStack.top().bFirstParagraphInCell = false;
    }

    m_StreamStateStack.top().bParaAutoBefore = false;
    m_StreamStateStack.top().bParaWithInlineObject = false;

#ifdef DBG_UTIL
    TagLogger::getInstance().endElement();
#endif

}

// TODO this does not yet take table styles into account
void DomainMapper_Impl::applyToggleAttributes(const PropertyMapPtr& pPropertyMap)
{
    std::optional<PropertyMap::Property> charStyleProperty = pPropertyMap->getProperty(PROP_CHAR_STYLE_NAME);
    if (charStyleProperty.has_value())
    {
        OUString sCharStyleName;
        charStyleProperty->second >>= sCharStyleName;
        float fCharStyleBold = css::awt::FontWeight::NORMAL;
        float fCharStyleBoldComplex = css::awt::FontWeight::NORMAL;
        css::awt::FontSlant eCharStylePosture = css::awt::FontSlant_NONE;
        css::awt::FontSlant eCharStylePostureComplex = css::awt::FontSlant_NONE;
        sal_Int16 nCharStyleCaseMap = css::style::CaseMap::NONE;
        sal_Int16 nCharStyleRelief = css::awt::FontRelief::NONE;
        bool bCharStyleContoured = false;//Outline;
        bool bCharStyleShadowed = false;
        sal_Int16 nCharStyleStrikeThrough = awt::FontStrikeout::NONE;
        bool bCharStyleHidden = false;

        uno::Reference<beans::XPropertySet> xCharStylePropertySet = GetCharacterStyles()->getByName(sCharStyleName).get<uno::Reference<beans::XPropertySet>>();
        xCharStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_WEIGHT)) >>= fCharStyleBold;
        xCharStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_WEIGHT_COMPLEX)) >>= fCharStyleBoldComplex;
        xCharStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_POSTURE)) >>= eCharStylePosture;
        xCharStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_POSTURE_COMPLEX)) >>= eCharStylePostureComplex;
        xCharStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_CASE_MAP)) >>= nCharStyleCaseMap;
        xCharStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_RELIEF)) >>= nCharStyleRelief;
        xCharStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_CONTOURED)) >>= bCharStyleContoured;
        xCharStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_SHADOWED)) >>= bCharStyleShadowed;
        xCharStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_STRIKEOUT)) >>= nCharStyleStrikeThrough;
        xCharStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_HIDDEN)) >>= bCharStyleHidden;
        if (fCharStyleBold > css::awt::FontWeight::NORMAL || eCharStylePosture != css::awt::FontSlant_NONE|| nCharStyleCaseMap != css::style::CaseMap::NONE ||
            nCharStyleRelief != css::awt::FontRelief::NONE || bCharStyleContoured || bCharStyleShadowed ||
            nCharStyleStrikeThrough == awt::FontStrikeout::SINGLE || bCharStyleHidden)
        {
            uno::Reference<beans::XPropertySet> const xParaStylePropertySet =
                GetParagraphStyles()->getByName(m_StreamStateStack.top().sCurrentParaStyleName).get<uno::Reference<beans::XPropertySet>>();
            float fParaStyleBold = css::awt::FontWeight::NORMAL;
            float fParaStyleBoldComplex = css::awt::FontWeight::NORMAL;
            css::awt::FontSlant eParaStylePosture = css::awt::FontSlant_NONE;
            css::awt::FontSlant eParaStylePostureComplex = css::awt::FontSlant_NONE;
            sal_Int16 nParaStyleCaseMap = css::style::CaseMap::NONE;
            sal_Int16 nParaStyleRelief = css::awt::FontRelief::NONE;
            bool bParaStyleContoured = false;
            bool bParaStyleShadowed = false;
            sal_Int16 nParaStyleStrikeThrough = awt::FontStrikeout::NONE;
            bool bParaStyleHidden = false;
            xParaStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_WEIGHT)) >>= fParaStyleBold;
            xParaStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_WEIGHT_COMPLEX)) >>= fParaStyleBoldComplex;
            xParaStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_POSTURE)) >>= eParaStylePosture;
            xParaStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_POSTURE_COMPLEX)) >>= eParaStylePostureComplex;
            xParaStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_CASE_MAP)) >>= nParaStyleCaseMap;
            xParaStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_RELIEF)) >>= nParaStyleRelief;
            xParaStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_SHADOWED)) >>= bParaStyleShadowed;
            xParaStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_CONTOURED)) >>= bParaStyleContoured;
            xParaStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_STRIKEOUT)) >>= nParaStyleStrikeThrough;
            xParaStylePropertySet->getPropertyValue(getPropertyName(PROP_CHAR_HIDDEN)) >>= bParaStyleHidden;
            if (fCharStyleBold > css::awt::FontWeight::NORMAL && fParaStyleBold > css::awt::FontWeight::NORMAL)
            {
                std::optional<PropertyMap::Property> charBoldProperty = pPropertyMap->getProperty(PROP_CHAR_WEIGHT);
                if (!charBoldProperty.has_value())
                {
                    pPropertyMap->Insert(PROP_CHAR_WEIGHT, uno::Any(css::awt::FontWeight::NORMAL));
                }
            }
            if (fCharStyleBoldComplex > css::awt::FontWeight::NORMAL && fParaStyleBoldComplex > css::awt::FontWeight::NORMAL)
            {
                std::optional<PropertyMap::Property> charBoldPropertyComplex = pPropertyMap->getProperty(PROP_CHAR_WEIGHT_COMPLEX);
                if (!charBoldPropertyComplex.has_value())
                {
                    pPropertyMap->Insert(PROP_CHAR_WEIGHT_COMPLEX, uno::Any(css::awt::FontWeight::NORMAL));
                    pPropertyMap->Insert(PROP_CHAR_WEIGHT_ASIAN, uno::Any(css::awt::FontWeight::NORMAL));
                }
            }
            if (eCharStylePosture != css::awt::FontSlant_NONE && eParaStylePosture != css::awt::FontSlant_NONE)
            {
                std::optional<PropertyMap::Property> charItalicProperty = pPropertyMap->getProperty(PROP_CHAR_POSTURE);
                if (!charItalicProperty.has_value())
                {
                    pPropertyMap->Insert(PROP_CHAR_POSTURE, uno::Any(css::awt::FontSlant_NONE));
                }
            }
            if (eCharStylePostureComplex != css::awt::FontSlant_NONE && eParaStylePostureComplex != css::awt::FontSlant_NONE)
            {
                std::optional<PropertyMap::Property> charItalicPropertyComplex = pPropertyMap->getProperty(PROP_CHAR_POSTURE_COMPLEX);
                if (!charItalicPropertyComplex.has_value())
                {
                    pPropertyMap->Insert(PROP_CHAR_POSTURE_COMPLEX, uno::Any(css::awt::FontSlant_NONE));
                    pPropertyMap->Insert(PROP_CHAR_POSTURE_ASIAN, uno::Any(css::awt::FontSlant_NONE));
                }
            }
            if (nCharStyleCaseMap == nParaStyleCaseMap && nCharStyleCaseMap != css::style::CaseMap::NONE)
            {
                std::optional<PropertyMap::Property> charCaseMap = pPropertyMap->getProperty(PROP_CHAR_CASE_MAP);
                if (!charCaseMap.has_value())
                {
                    pPropertyMap->Insert(PROP_CHAR_CASE_MAP, uno::Any(css::style::CaseMap::NONE));
                }
            }
            if (nParaStyleRelief != css::awt::FontRelief::NONE && nCharStyleRelief == nParaStyleRelief)
            {
                std::optional<PropertyMap::Property> charRelief = pPropertyMap->getProperty(PROP_CHAR_RELIEF);
                if (!charRelief.has_value())
                {
                    pPropertyMap->Insert(PROP_CHAR_RELIEF, uno::Any(css::awt::FontRelief::NONE));
                }
            }
            if (bParaStyleContoured && bCharStyleContoured)
            {
                std::optional<PropertyMap::Property> charContoured = pPropertyMap->getProperty(PROP_CHAR_CONTOURED);
                if (!charContoured.has_value())
                {
                    pPropertyMap->Insert(PROP_CHAR_CONTOURED, uno::Any(false));
                }
            }
            if (bParaStyleShadowed && bCharStyleShadowed)
            {
                std::optional<PropertyMap::Property> charShadow = pPropertyMap->getProperty(PROP_CHAR_SHADOWED);
                if (!charShadow.has_value())
                {
                    pPropertyMap->Insert(PROP_CHAR_SHADOWED, uno::Any(false));
                }
            }
            if (nParaStyleStrikeThrough == css::awt::FontStrikeout::SINGLE && nParaStyleStrikeThrough == nCharStyleStrikeThrough)
            {
                std::optional<PropertyMap::Property> charStrikeThrough = pPropertyMap->getProperty(PROP_CHAR_STRIKEOUT);
                if (!charStrikeThrough.has_value())
                {
                    pPropertyMap->Insert(PROP_CHAR_STRIKEOUT, uno::Any(css::awt::FontStrikeout::NONE));
                }
            }
            if (bParaStyleHidden && bCharStyleHidden)
            {
                std::optional<PropertyMap::Property> charHidden = pPropertyMap->getProperty(PROP_CHAR_HIDDEN);
                if (!charHidden.has_value())
                {
                    pPropertyMap->Insert(PROP_CHAR_HIDDEN, uno::Any(false));
                }
            }
        }
    }
}

void DomainMapper_Impl::MergeAtContentImageRedlineWithNext(const css::uno::Reference<css::text::XTextAppend>& xTextAppend)
{
    // remove workaround for change tracked images, if they are part of a redline,
    // i.e. if the next run is a tracked change with the same type, author and date,
    // as in the change tracking of the image.
    if ( m_bRedlineImageInPreviousRun )
    {
        auto pCurrentRedline = m_aRedlines.top().size() > 0
            ? m_aRedlines.top().back()
            : GetTopContextOfType(CONTEXT_CHARACTER) &&
            GetTopContextOfType(CONTEXT_CHARACTER)->Redlines().size() > 0
            ? GetTopContextOfType(CONTEXT_CHARACTER)->Redlines().back()
            : nullptr;
        if ( m_previousRedline && pCurrentRedline &&
                // same redline
                (m_previousRedline->m_nToken & 0xffff) == (pCurrentRedline->m_nToken & 0xffff) &&
                m_previousRedline->m_sAuthor == pCurrentRedline->m_sAuthor &&
                m_previousRedline->m_sDate == pCurrentRedline->m_sDate )
        {
            uno::Reference< text::XTextCursor > xCursor = xTextAppend->getEnd()->getText( )->createTextCursor( );
            assert(xCursor.is());
            xCursor->gotoEnd(false);
            xCursor->goLeft(2, true);
            if ( xCursor->getString() == u"​​" )
            {
                xCursor->goRight(1, true);
                xCursor->setString("");
                xCursor->gotoEnd(false);
                xCursor->goLeft(1, true);
                xCursor->setString("");
            }
        }

        m_bRedlineImageInPreviousRun = false;
    }
}

    void DomainMapper_Impl::appendTextPortion( const OUString& rString, const PropertyMapPtr& pPropertyMap )
{
    if (m_bDiscardHeaderFooter)
        return;

    if (m_aTextAppendStack.empty())
        return;
    // Before placing call to processDeferredCharacterProperties(), TopContextType should be CONTEXT_CHARACTER
    // processDeferredCharacterProperties() invokes only if character inserted
    if (pPropertyMap == m_pTopContext
        && !m_StreamStateStack.top().deferredCharacterProperties.empty()
        && (GetTopContextType() == CONTEXT_CHARACTER))
    {
        processDeferredCharacterProperties();
    }
    uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
    if (!xTextAppend.is() || !hasTableManager() || getTableManager().isIgnore())
        return;

    try
    {
        applyToggleAttributes(pPropertyMap);
        // If we are in comments, then disable CharGrabBag, comment text doesn't support that.
        uno::Sequence<beans::PropertyValue> aValues = pPropertyMap->GetPropertyValues(/*bCharGrabBag=*/!IsInComments());

        if (IsInTOC() || m_bStartIndex || m_bStartBibliography)
            for( auto& rValue : asNonConstRange(aValues) )
            {
                if (rValue.Name == "CharHidden")
                    rValue.Value <<= false;
            }

        MergeAtContentImageRedlineWithNext(xTextAppend);

        uno::Reference< text::XTextRange > xTextRange;
        if (m_aTextAppendStack.top().xInsertPosition.is())
        {
            xTextRange = xTextAppend->insertTextPortion(rString, aValues, m_aTextAppendStack.top().xInsertPosition);
            m_aTextAppendStack.top().xCursor->gotoRange(xTextRange->getEnd(), true);
        }
        else
        {
            if (IsInTOC() || m_bStartIndex || m_bStartBibliography || m_nStartGenericField != 0)
            {
                if (IsInHeaderFooter() && !m_bStartTOCHeaderFooter)
                {
                    xTextRange = xTextAppend->appendTextPortion(rString, aValues);
                }
                else
                {
                    m_bStartedTOC = true;
                    uno::Reference< text::XTextCursor > xTOCTextCursor = xTextAppend->getEnd()->getText( )->createTextCursor( );
                    assert(xTOCTextCursor.is());
                    xTOCTextCursor->gotoEnd(false);
                    if (m_nStartGenericField != 0)
                    {
                        xTOCTextCursor->goLeft(1, false);
                    }
                    if (IsInComments())
                        xTextRange = xTextAppend->finishParagraphInsert(aValues, xTOCTextCursor);
                    else
                        xTextRange = xTextAppend->insertTextPortion(rString, aValues, xTOCTextCursor);
                    SAL_WARN_IF(!xTextRange.is(), "writerfilter.dmapper", "insertTextPortion failed");
                    if (!xTextRange.is())
                        throw uno::Exception("insertTextPortion failed", nullptr);
                    m_StreamStateStack.top().bTextInserted = true;
                    xTOCTextCursor->gotoRange(xTextRange->getEnd(), true);
                    if (m_nStartGenericField == 0)
                    {
                        m_aTextAppendStack.push(TextAppendContext(xTextAppend, xTOCTextCursor));
                    }
                }
            }
            else
            {
                if (IsOpenField() && GetTopFieldContext()->GetFieldId() == FIELD_HYPERLINK)
                {
                    // It is content of hyperlink field. We need to create and remember
                    // character style for later applying to hyperlink
                    PropertyValueVector_t aProps = comphelper::sequenceToContainer< PropertyValueVector_t >(GetTopContext()->GetPropertyValues());
                    OUString sHyperlinkStyleName = GetStyleSheetTable()->getOrCreateCharStyle(aProps, /*bAlwaysCreate=*/false);
                    GetTopFieldContext()->SetHyperlinkStyle(sHyperlinkStyleName);
                }

#if !defined(MACOSX) // TODO: check layout differences and support all platforms, if needed
                sal_Int32 nPos = 0;
                OUString sFontName;
                OUString sDoubleSpace("  ");
                PropertyMapPtr pContext = GetTopContextOfType(CONTEXT_CHARACTER);
                // tdf#123703 workaround for longer space sequences of the old or compatible RTF documents
                if (GetSettingsTable()->GetLongerSpaceSequence() && !IsOpenFieldCommand() && (nPos = rString.indexOf(sDoubleSpace)) != -1 &&
                    // monospaced fonts have no longer space sequences, regardless of \fprq2 (not monospaced) font setting
                    // fix for the base monospaced font Courier
                    (!pContext || !pContext->isSet(PROP_CHAR_FONT_NAME) ||
                        ((pContext->getProperty(PROP_CHAR_FONT_NAME)->second >>= sFontName) && sFontName.indexOf("Courier") == -1)))
                {
                    // an RTF space character is longer by an extra six-em-space in an old-style RTF space sequence,
                    // insert them to keep RTF document layout formatted by consecutive spaces
                    const sal_Unicode aExtraSpace[5] = { 0x2006, 0x20, 0x2006, 0x20, 0 };
                    const sal_Unicode aExtraSpace2[4] = { 0x20, 0x2006, 0x20, 0 };
                    xTextRange = xTextAppend->appendTextPortion(rString.replaceAll(sDoubleSpace, aExtraSpace, nPos)
                                                                       .replaceAll(sDoubleSpace, aExtraSpace2, nPos), aValues);
                }
                else
#endif
                    xTextRange = xTextAppend->appendTextPortion(rString, aValues);
            }
        }

        // reset moveFrom/moveTo data of non-terminating runs of the paragraph
        if ( m_pParaMarkerRedlineMove )
        {
            m_pParaMarkerRedlineMove.clear();
        }
        CheckRedline( xTextRange );
        m_StreamStateStack.top().bParaChanged = true;

        //getTableManager( ).handle(xTextRange);
    }
    catch(const lang::IllegalArgumentException&)
    {
        TOOLS_WARN_EXCEPTION( "writerfilter", "DomainMapper_Impl::appendTextPortion" );
    }
    catch(const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION( "writerfilter", "DomainMapper_Impl::appendTextPortion" );
    }
}

void DomainMapper_Impl::appendTextContent(
    const uno::Reference< text::XTextContent >& xContent,
    const uno::Sequence< beans::PropertyValue >& xPropertyValues
    )
{
    SAL_WARN_IF(m_aTextAppendStack.empty(), "writerfilter.dmapper", "no text append stack");
    if (m_aTextAppendStack.empty())
        return;
    uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert( m_aTextAppendStack.top().xTextAppend, uno::UNO_QUERY );
    OSL_ENSURE( xTextAppendAndConvert.is(), "trying to append a text content without XTextAppendAndConvert" );
    if (!xTextAppendAndConvert.is() || !hasTableManager() || getTableManager().isIgnore())
        return;

    try
    {
        if (m_aTextAppendStack.top().xInsertPosition.is())
            xTextAppendAndConvert->insertTextContentWithProperties( xContent, xPropertyValues, m_aTextAppendStack.top().xInsertPosition );
        else
            xTextAppendAndConvert->appendTextContent( xContent, xPropertyValues );
    }
    catch(const lang::IllegalArgumentException&)
    {
    }
    catch(const uno::Exception&)
    {
    }
}

void DomainMapper_Impl::appendOLE( const OUString& rStreamName, const std::shared_ptr<OLEHandler>& pOLEHandler )
{
    try
    {
        uno::Reference< text::XTextContent > xOLE( m_xTextDocument->createInstance("com.sun.star.text.TextEmbeddedObject"), uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xOLEProperties(xOLE, uno::UNO_QUERY_THROW);

        OUString aCLSID = pOLEHandler->getCLSID();
        if (aCLSID.isEmpty())
            xOLEProperties->setPropertyValue(getPropertyName( PROP_STREAM_NAME ),
                            uno::Any( rStreamName ));
        else
            xOLEProperties->setPropertyValue("CLSID", uno::Any(aCLSID));

        OUString aDrawAspect = pOLEHandler->GetDrawAspect();
        if(!aDrawAspect.isEmpty())
            xOLEProperties->setPropertyValue("DrawAspect", uno::Any(aDrawAspect));

        awt::Size aSize = pOLEHandler->getSize();
        if( !aSize.Width )
            aSize.Width = 1000;
        if( !aSize.Height )
            aSize.Height = 1000;
        xOLEProperties->setPropertyValue(getPropertyName( PROP_WIDTH ),
                        uno::Any(aSize.Width));
        xOLEProperties->setPropertyValue(getPropertyName( PROP_HEIGHT ),
                        uno::Any(aSize.Height));

        OUString aVisAreaWidth = pOLEHandler->GetVisAreaWidth();
        if(!aVisAreaWidth.isEmpty())
            xOLEProperties->setPropertyValue("VisibleAreaWidth", uno::Any(aVisAreaWidth));

        OUString aVisAreaHeight = pOLEHandler->GetVisAreaHeight();
        if(!aVisAreaHeight.isEmpty())
            xOLEProperties->setPropertyValue("VisibleAreaHeight", uno::Any(aVisAreaHeight));

        uno::Reference< graphic::XGraphic > xGraphic = pOLEHandler->getReplacement();
        xOLEProperties->setPropertyValue(getPropertyName( PROP_GRAPHIC ),
                        uno::Any(xGraphic));
        uno::Reference<beans::XPropertySet> xReplacementProperties(pOLEHandler->getShape(), uno::UNO_QUERY);
        if (xReplacementProperties.is())
        {
            table::BorderLine2 aBorderProps;
            xReplacementProperties->getPropertyValue("LineColor") >>= aBorderProps.Color;
            xReplacementProperties->getPropertyValue("LineWidth") >>= aBorderProps.LineWidth;
            xReplacementProperties->getPropertyValue("LineStyle") >>= aBorderProps.LineStyle;

            if (aBorderProps.LineStyle) // Set line props only if LineStyle is set
            {
                xOLEProperties->setPropertyValue("RightBorder", uno::Any(aBorderProps));
                xOLEProperties->setPropertyValue("TopBorder", uno::Any(aBorderProps));
                xOLEProperties->setPropertyValue("LeftBorder", uno::Any(aBorderProps));
                xOLEProperties->setPropertyValue("BottomBorder", uno::Any(aBorderProps));
            }
            OUString pProperties[] = {
                "AnchorType",
                "Surround",
                "SurroundContour",
                "HoriOrient",
                "HoriOrientPosition",
                "VertOrient",
                "VertOrientPosition",
                "VertOrientRelation",
                "HoriOrientRelation",
                "LeftMargin",
                "RightMargin",
                "TopMargin",
                "BottomMargin"
            };
            for (const OUString& s : pProperties)
            {
                const uno::Any aVal = xReplacementProperties->getPropertyValue(s);
                xOLEProperties->setPropertyValue(s, aVal);
            }

            if (xReplacementProperties->getPropertyValue("FillStyle").get<css::drawing::FillStyle>()
                != css::drawing::FillStyle::FillStyle_NONE) // Apply fill props if style is set
            {
                xOLEProperties->setPropertyValue(
                    "FillStyle", xReplacementProperties->getPropertyValue("FillStyle"));
                xOLEProperties->setPropertyValue(
                    "FillColor", xReplacementProperties->getPropertyValue("FillColor"));
                xOLEProperties->setPropertyValue(
                    "FillColor2", xReplacementProperties->getPropertyValue("FillColor2"));
            }
        }
        else
            // mimic the treatment of graphics here... it seems anchoring as character
            // gives a better ( visually ) result
            xOLEProperties->setPropertyValue(getPropertyName( PROP_ANCHOR_TYPE ),  uno::Any( text::TextContentAnchorType_AS_CHARACTER ) );
        // remove ( if valid ) associated shape ( used for graphic replacement )
        SAL_WARN_IF(m_aAnchoredStack.empty(), "writerfilter.dmapper", "no anchor stack");
        if (!m_aAnchoredStack.empty())
            m_aAnchoredStack.top( ).bToRemove = true;
        RemoveLastParagraph();
        SAL_WARN_IF(m_aTextAppendStack.empty(), "writerfilter.dmapper", "no text stack");
        if (!m_aTextAppendStack.empty())
            m_aTextAppendStack.pop();

        appendTextContent( xOLE, uno::Sequence< beans::PropertyValue >() );

        if (!aCLSID.isEmpty())
            pOLEHandler->importStream(m_xComponentContext, GetTextDocument(), xOLE);

    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION( "writerfilter", "in creation of OLE object" );
    }

}

void DomainMapper_Impl::appendStarMath( const Value& val )
{
    uno::Reference< embed::XEmbeddedObject > formula;
    val.getAny() >>= formula;
    if( !formula.is() )
        return;

    try
    {
        uno::Reference< text::XTextContent > xStarMath( m_xTextDocument->createInstance("com.sun.star.text.TextEmbeddedObject"), uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xStarMathProperties(xStarMath, uno::UNO_QUERY_THROW);

        xStarMathProperties->setPropertyValue(getPropertyName( PROP_EMBEDDED_OBJECT ),
            val.getAny());
        // tdf#66405: set zero margins for embedded object
        xStarMathProperties->setPropertyValue(getPropertyName( PROP_LEFT_MARGIN ),
            uno::Any(sal_Int32(0)));
        xStarMathProperties->setPropertyValue(getPropertyName( PROP_RIGHT_MARGIN ),
            uno::Any(sal_Int32(0)));
        xStarMathProperties->setPropertyValue(getPropertyName( PROP_TOP_MARGIN ),
            uno::Any(sal_Int32(0)));
        xStarMathProperties->setPropertyValue(getPropertyName( PROP_BOTTOM_MARGIN ),
            uno::Any(sal_Int32(0)));

        uno::Reference< uno::XInterface > xInterface( formula->getComponent(), uno::UNO_QUERY );
        // set zero margins for object's component
        uno::Reference< beans::XPropertySet > xComponentProperties( xInterface, uno::UNO_QUERY_THROW );
        xComponentProperties->setPropertyValue(getPropertyName( PROP_LEFT_MARGIN ),
            uno::Any(sal_Int32(0)));
        xComponentProperties->setPropertyValue(getPropertyName( PROP_RIGHT_MARGIN ),
            uno::Any(sal_Int32(0)));
        xComponentProperties->setPropertyValue(getPropertyName( PROP_TOP_MARGIN ),
            uno::Any(sal_Int32(0)));
        xComponentProperties->setPropertyValue(getPropertyName( PROP_BOTTOM_MARGIN ),
            uno::Any(sal_Int32(0)));
        Size size( 1000, 1000 );
        if( oox::FormulaImExportBase* formulaimport = dynamic_cast< oox::FormulaImExportBase* >( xInterface.get()))
            size = formulaimport->getFormulaSize();
        xStarMathProperties->setPropertyValue(getPropertyName( PROP_WIDTH ),
            uno::Any( sal_Int32(size.Width())));
        xStarMathProperties->setPropertyValue(getPropertyName( PROP_HEIGHT ),
            uno::Any( sal_Int32(size.Height())));
        xStarMathProperties->setPropertyValue(getPropertyName(PROP_ANCHOR_TYPE),
                uno::Any(text::TextContentAnchorType_AS_CHARACTER));
        // mimic the treatment of graphics here... it seems anchoring as character
        // gives a better ( visually ) result
        appendTextContent(xStarMath, uno::Sequence<beans::PropertyValue>());
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION( "writerfilter", "in creation of StarMath object" );
    }
}

void DomainMapper_Impl::adjustLastPara(sal_Int8 nAlign)
{
    PropertyMapPtr pLastPara = GetTopContextOfType(dmapper::CONTEXT_PARAGRAPH);
    pLastPara->Insert(PROP_PARA_ADJUST, uno::Any(nAlign), true);
}

static void checkAndAddPropVal(const OUString& prop, const css::uno::Any& val,
                               std::vector<OUString>& props, std::vector<css::uno::Any>& values)
{
    // Avoid well-known reasons for exceptions when setting property values
    if (!val.hasValue())
        return;
    if (prop == "CharStyleName" || prop == "DropCapCharStyleName")
        if (OUString val_string; (val >>= val_string) && val_string.isEmpty())
            return;

    props.push_back(prop);
    values.push_back(val);
}

static uno::Reference<lang::XComponent>
getParagraphOfRange(const css::uno::Reference<css::text::XTextRange>& xRange)
{
    uno::Reference<container::XEnumerationAccess> xEA{ xRange, uno::UNO_QUERY_THROW };
    return { xEA->createEnumeration()->nextElement(), uno::UNO_QUERY_THROW };
}

static void copyAllProps(const css::uno::Reference<css::uno::XInterface>& from,
                         const css::uno::Reference<css::uno::XInterface>& to)
{
    css::uno::Reference<css::beans::XPropertySet> xFromProps(from, css::uno::UNO_QUERY_THROW);
    css::uno::Reference<css::beans::XPropertySetInfo> xFromInfo(xFromProps->getPropertySetInfo(),
                                                                css::uno::UNO_SET_THROW);
    css::uno::Sequence<css::beans::Property> rawProps(xFromInfo->getProperties());
    std::vector<OUString> props;
    props.reserve(rawProps.getLength());
    for (const auto& prop : rawProps)
        if ((prop.Attributes & css::beans::PropertyAttribute::READONLY) == 0)
            props.push_back(prop.Name);

    if (css::uno::Reference<css::beans::XPropertyState> xFromState{ from, css::uno::UNO_QUERY })
    {
        const auto propsSeq = comphelper::containerToSequence(props);
        const auto statesSeq = xFromState->getPropertyStates(propsSeq);
        assert(propsSeq.getLength() == statesSeq.getLength());
        for (sal_Int32 i = 0; i < propsSeq.getLength(); ++i)
            if (statesSeq[i] != css::beans::PropertyState_DIRECT_VALUE)
                std::erase(props, propsSeq[i]);
    }

    std::vector<css::uno::Any> values;
    values.reserve(props.size());
    if (css::uno::Reference<css::beans::XMultiPropertySet> xFromMulti{ xFromProps,
                                                                       css::uno::UNO_QUERY })
    {
        const auto propsSeq = comphelper::containerToSequence(props);
        const auto valuesSeq = xFromMulti->getPropertyValues(propsSeq);
        assert(propsSeq.getLength() == valuesSeq.getLength());
        props.clear();
        for (size_t i = 0; i < propsSeq.size(); ++i)
            checkAndAddPropVal(propsSeq[i], valuesSeq[i], props, values);
    }
    else
    {
        std::vector<OUString> filtered_props;
        filtered_props.reserve(props.size());
        for (const auto& prop : props)
            checkAndAddPropVal(prop, xFromProps->getPropertyValue(prop), filtered_props, values);
        filtered_props.swap(props);
    }
    assert(props.size() == values.size());

    css::uno::Reference<css::beans::XPropertySet> xToProps(to, css::uno::UNO_QUERY_THROW);
    if (css::uno::Reference<css::beans::XMultiPropertySet> xToMulti{ xToProps,
                                                                     css::uno::UNO_QUERY })
    {
        try
        {
            xToMulti->setPropertyValues(comphelper::containerToSequence(props),
                                        comphelper::containerToSequence(values));
            return;
        }
        catch (css::uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("writerfilter.dmapper");
        }
        // Fallback to property-by-property iteration
    }

    for (size_t i = 0; i < props.size(); ++i)
    {
        try
        {
            xToProps->setPropertyValue(props[i], values[i]);
        }
        catch (css::uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("writerfilter.dmapper");
        }
    }
}

uno::Reference< beans::XPropertySet > DomainMapper_Impl::appendTextSectionAfter(
                                    uno::Reference< text::XTextRange > const & xBefore )
{
    uno::Reference< beans::XPropertySet > xRet;
    if (m_aTextAppendStack.empty())
        return xRet;
    uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
    if(xTextAppend.is())
    {
        try
        {
            uno::Reference< text::XParagraphCursor > xCursor(
                xTextAppend->createTextCursorByRange( xBefore ), uno::UNO_QUERY_THROW);
            //the cursor has been moved to the end of the paragraph because of the appendTextPortion() calls
            xCursor->gotoStartOfParagraph( false );
            if (m_aTextAppendStack.top().xInsertPosition.is())
                xCursor->gotoRange( m_aTextAppendStack.top().xInsertPosition, true );
            else
                xCursor->gotoEnd( true );
            // The paragraph after this new section is already inserted. The previous node may be a
            // table; then trying to go left would skip the whole table. Split the trailing
            // paragraph; let the section span over the first of the two resulting paragraphs;
            // destroy the last section's paragraph afterwards.
            xTextAppend->insertControlCharacter(
                xCursor->getEnd(), css::text::ControlCharacter::PARAGRAPH_BREAK, false);
            auto xNewPara = getParagraphOfRange(xCursor->getEnd());
            xCursor->gotoPreviousParagraph(true);
            auto xEndPara = getParagraphOfRange(xCursor->getEnd());
            // xEndPara may already have properties (like page break); make sure to apply them
            // to the newly appended paragraph, which will be kept in the end.
            copyAllProps(xEndPara, xNewPara);

            uno::Reference< text::XTextContent > xSection( m_xTextDocument->createInstance("com.sun.star.text.TextSection"), uno::UNO_QUERY_THROW );
            xSection->attach(xCursor);

            // Remove the extra paragraph (last inside the section)
            xEndPara->dispose();

            xRet.set(xSection, uno::UNO_QUERY );
        }
        catch(const uno::Exception&)
        {
        }

    }

    return xRet;
}

void DomainMapper_Impl::appendGlossaryEntry()
{
    appendTextSectionAfter(m_xGlossaryEntryStart);
}

void DomainMapper_Impl::fillEmptyFrameProperties(std::vector<beans::PropertyValue>& rFrameProperties, bool bSetAnchorToChar)
{
    if (bSetAnchorToChar)
        rFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_ANCHOR_TYPE), text::TextContentAnchorType_AS_CHARACTER));

    uno::Any aEmptyBorder{table::BorderLine2()};
    static const std::vector<PropertyIds> aBorderIds
        = { PROP_BOTTOM_BORDER, PROP_LEFT_BORDER, PROP_RIGHT_BORDER, PROP_TOP_BORDER };
    for (size_t i = 0; i < aBorderIds.size(); ++i)
        rFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(aBorderIds[i]), aEmptyBorder));

    static const std::vector<PropertyIds> aMarginIds
        = { PROP_BOTTOM_MARGIN, PROP_BOTTOM_BORDER_DISTANCE,
            PROP_LEFT_MARGIN,   PROP_LEFT_BORDER_DISTANCE,
            PROP_RIGHT_MARGIN,  PROP_RIGHT_BORDER_DISTANCE,
            PROP_TOP_MARGIN,    PROP_TOP_BORDER_DISTANCE };
    for (size_t i = 0; i < aMarginIds.size(); ++i)
        rFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(aMarginIds[i]), static_cast<sal_Int32>(0)));
}

bool DomainMapper_Impl::IsInTOC() const
{
    if (IsInHeaderFooter())
        return m_bStartTOCHeaderFooter;
    else
        return m_bStartTOC;
}

void DomainMapper_Impl::ConvertHeaderFooterToTextFrame(bool bDynamicHeightTop, bool bDynamicHeightBottom)
{
    while (!m_aHeaderFooterTextAppendStack.empty())
    {
        auto& [aTextAppendContext, ePagePartType] = m_aHeaderFooterTextAppendStack.top();
        if ((ePagePartType == PagePartType::Header && !bDynamicHeightTop) || (ePagePartType == PagePartType::Footer && !bDynamicHeightBottom))
        {
            uno::Reference< text::XTextAppend > xTextAppend = aTextAppendContext.xTextAppend;
            uno::Reference< text::XTextCursor > xCursor = xTextAppend->createTextCursor();
            uno::Reference< text::XTextRange > xRangeStart, xRangeEnd;

            xRangeStart = xCursor->getStart();
            xCursor->gotoEnd(false);
            xRangeEnd = xCursor->getStart();

            std::vector<beans::PropertyValue> aFrameProperties
            {
                comphelper::makePropertyValue("TextWrap", css::text::WrapTextMode_THROUGH),
                comphelper::makePropertyValue(getPropertyName(PROP_HORI_ORIENT), text::HoriOrientation::LEFT),
                comphelper::makePropertyValue(getPropertyName(PROP_OPAQUE), false),
                comphelper::makePropertyValue(getPropertyName(PROP_WIDTH_TYPE), text::SizeType::MIN),
                comphelper::makePropertyValue(getPropertyName(PROP_SIZE_TYPE), text::SizeType::MIN),
            // tdf#143384 If the header/footer started with a table, convertToTextFrame could not
            // convert the table, because it used createTextCursor() -which ignore tables-
            // to set the conversion range.
            // This dummy property is set to make convertToTextFrame to use another CreateTextCursor
            // method that can be parameterized to not ignore tables.
                comphelper::makePropertyValue(getPropertyName(PROP_CURSOR_NOT_IGNORE_TABLES_IN_HF), true)
            };

            fillEmptyFrameProperties(aFrameProperties, false);

            // If it is a footer, then orient the frame to the bottom
            if (ePagePartType == PagePartType::Footer)
            {
                aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_VERT_ORIENT), text::VertOrientation::BOTTOM));
            }
            uno::Reference<text::XTextAppendAndConvert> xBodyText(xRangeStart->getText(), uno::UNO_QUERY);
            xBodyText->convertToTextFrame(xTextAppend, xRangeEnd, comphelper::containerToSequence(aFrameProperties));
        }
        m_aHeaderFooterTextAppendStack.pop();
    }
}

namespace
{
// Determines if the XText content is empty (no text, no shapes, no tables)
bool isContentEmpty(uno::Reference<text::XText> const& xText)
{
    if (!xText.is())
        return true; // no XText means it's empty

    uno::Reference<css::lang::XServiceInfo> xTextServiceInfo(xText, uno::UNO_QUERY);
    if (xTextServiceInfo && xTextServiceInfo->getImplementationName() == "SwXHeadFootText")
        return false;

    uno::Reference<container::XEnumerationAccess> xEnumAccess(xText->getText(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xEnum = xEnumAccess->createEnumeration();
    while (xEnum->hasMoreElements())
    {
        auto xObject = xEnum->nextElement();
        uno::Reference<text::XTextTable> const xTextTable(xObject, uno::UNO_QUERY);
        if (xTextTable.is())
            return false;

        uno::Reference<text::XTextRange> const xParagraph(xObject, uno::UNO_QUERY);
        if (xParagraph.is() && !xParagraph->getString().isEmpty())
            return false;
    }
    return true;
}

} // end anonymous namespace

void DomainMapper_Impl::PushPageHeaderFooter(PagePartType ePagePartType, PageType eType)
{
    bool bHeader = ePagePartType == PagePartType::Header;

    const PropertyIds ePropIsOn = bHeader ? PROP_HEADER_IS_ON: PROP_FOOTER_IS_ON;
    const PropertyIds ePropShared = bHeader ? PROP_HEADER_IS_SHARED: PROP_FOOTER_IS_SHARED;
    const PropertyIds ePropTextLeft = bHeader ? PROP_HEADER_TEXT_LEFT: PROP_FOOTER_TEXT_LEFT;
    const PropertyIds ePropTextFirst = bHeader ? PROP_HEADER_TEXT_FIRST: PROP_FOOTER_TEXT_FIRST;
    const PropertyIds ePropTextRight = bHeader ? PROP_HEADER_TEXT: PROP_FOOTER_TEXT;

    m_bDiscardHeaderFooter = true;
    m_StreamStateStack.top().eSubstreamType = bHeader ? SubstreamType::Header : SubstreamType::Footer;

    //get the section context
    SectionPropertyMap* pSectionContext = GetSectionContext();;
    if (!pSectionContext)
        return;

    if (!m_bIsNewDoc)
        return; // TODO sw cannot Undo insert header/footer without crashing

    uno::Reference<beans::XPropertySet> xPageStyle = pSectionContext->GetPageStyle(*this);
    if (!xPageStyle.is())
        return;

    bool bEvenAndOdd = GetSettingsTable()->GetEvenAndOddHeaders();

    try
    {
        // Turn on the headers
        xPageStyle->setPropertyValue(getPropertyName(ePropIsOn), uno::Any(true));

        // Set both sharing left and first to off so we can import the content regardless of what value
        // the "titlePage" or "evenAndOdd" flags are set (which decide what the sharing is set to in the document).
        xPageStyle->setPropertyValue(getPropertyName(ePropShared), uno::Any(false));
        xPageStyle->setPropertyValue(getPropertyName(PROP_FIRST_IS_SHARED), uno::Any(false));

        if (eType == PageType::LEFT)
        {
            if (bHeader)
            {
                pSectionContext->m_bLeftHeader = true;
                pSectionContext->m_bHadLeftHeader = true;
            }
            else
                pSectionContext->m_bLeftFooter = true;

            prepareHeaderFooterContent(xPageStyle, ePagePartType, ePropTextLeft, bEvenAndOdd);
        }
        else if (eType == PageType::FIRST)
        {
            if (bHeader)
            {
                pSectionContext->m_bFirstHeader = true;
                pSectionContext->m_bHadFirstHeader = true;
            }
            else
                pSectionContext->m_bFirstFooter = true;

            prepareHeaderFooterContent(xPageStyle, ePagePartType, ePropTextFirst, true);
        }
        else
        {
            if (bHeader)
            {
                pSectionContext->m_bRightHeader = true;
                pSectionContext->m_bHadRightHeader = true;
            }
            else
                pSectionContext->m_bRightFooter = true;

            prepareHeaderFooterContent(xPageStyle, ePagePartType, ePropTextRight, true);
        }

        m_bDiscardHeaderFooter = false; // set only on success!
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("writerfilter.dmapper");
    }
}

/** Prepares the header/footer text content by first removing the existing
 *  content and adding it to the text append stack. */
void DomainMapper_Impl::prepareHeaderFooterContent(uno::Reference<beans::XPropertySet> const& xPageStyle,
                                                   PagePartType ePagePartType, PropertyIds ePropertyID,
                                                   bool bAppendToHeaderAndFooterTextStack)
{
    uno::Reference<text::XText> xText;
    xPageStyle->getPropertyValue(getPropertyName(ePropertyID)) >>= xText;

    //remove the existing content first
    SectionPropertyMap::removeXTextContent(xText);

    auto xTextCursor = m_bIsNewDoc ? uno::Reference<text::XTextCursor>() : xText->createTextCursorByRange(xText->getStart());
    uno::Reference<text::XTextAppend> xTextAppend(xText, uno::UNO_QUERY_THROW);
    m_aTextAppendStack.push(TextAppendContext(xTextAppend, xTextCursor));
    if (bAppendToHeaderAndFooterTextStack)
        m_aHeaderFooterTextAppendStack.push(std::make_pair(TextAppendContext(xTextAppend, xTextCursor), ePagePartType));
}

bool DomainMapper_Impl::SeenHeaderFooter(PagePartType const partType, PageType const pageType) const
{
    return m_HeaderFooterSeen.find({partType, pageType}) != m_HeaderFooterSeen.end();
}

/** Checks if the header and footer content on the text appended stack is empty.
 */
void DomainMapper_Impl::checkIfHeaderFooterIsEmpty(PagePartType ePagePartType, PageType eType)
{
    if (m_bDiscardHeaderFooter)
        return;

    if (m_aTextAppendStack.empty())
        return;

    SectionPropertyMap* pSectionContext = GetSectionContext();
    if (!pSectionContext)
        return;

    bool bHeader = ePagePartType == PagePartType::Header;

    uno::Reference<beans::XPropertySet> xPageStyle(pSectionContext->GetPageStyle(*this));

    if (!xPageStyle.is())
        return;

    bool bEmpty = isContentEmpty(m_aTextAppendStack.top().xTextAppend);

    if (eType == PageType::FIRST && bEmpty)
    {
        if (bHeader)
            pSectionContext->m_bFirstHeader = false;
        else
            pSectionContext->m_bFirstFooter = false;
    }
    else if (eType == PageType::LEFT && bEmpty)
    {
        if (bHeader)
            pSectionContext->m_bLeftHeader = false;
        else
            pSectionContext->m_bLeftFooter = false;
    }
    else if (eType == PageType::RIGHT && bEmpty)
    {
        if (bHeader)
            pSectionContext->m_bRightHeader = false;
        else
            pSectionContext->m_bRightFooter = false;
    }
}

void DomainMapper_Impl::PopPageHeaderFooter(PagePartType ePagePartType, PageType eType)
{
    //header and footer always have an empty paragraph at the end
    //this has to be removed
    RemoveLastParagraph();

    checkIfHeaderFooterIsEmpty(ePagePartType, eType);

    // clear the "Link To Previous" flag so that the header/footer
    // content is not copied from the previous section
    SectionPropertyMap* pSectionContext = GetSectionContext();
    if (pSectionContext)
    {
        pSectionContext->clearHeaderFooterLinkToPrevious(ePagePartType, eType);
        m_HeaderFooterSeen.emplace(ePagePartType, eType);
    }

    if (!m_aTextAppendStack.empty())
    {
        if (!m_bDiscardHeaderFooter)
        {
            m_aTextAppendStack.pop();
        }
        m_bDiscardHeaderFooter = false;
    }
}

void DomainMapper_Impl::PushFootOrEndnote( bool bIsFootnote )
{
    SAL_WARN_IF(m_StreamStateStack.top().eSubstreamType != SubstreamType::Body, "writerfilter.dmapper", "PushFootOrEndnote() is called from another foot or endnote");
    m_StreamStateStack.top().eSubstreamType = bIsFootnote ? SubstreamType::Footnote : SubstreamType::Endnote;
    m_StreamStateStack.top().bCheckFirstFootnoteTab = true;
    try
    {
        // Redlines outside the footnote should not affect footnote content
        m_aRedlines.push(std::vector< RedlineParamsPtr >());

        // IMHO character styles from footnote labels should be ignored in the edit view of Writer.
        // This adds a hack on top of the following hack to save the style name in the context.
        PropertyMapPtr pTopContext = GetTopContext();
        OUString sFootnoteCharStyleName;
        std::optional< PropertyMap::Property > aProp = pTopContext->getProperty(PROP_CHAR_STYLE_NAME);
        if (aProp)
            aProp->second >>= sFootnoteCharStyleName;

        // Remove style reference, if any. This reference did appear here as a side effect of tdf#43017
        // Seems it is not required by LO, but causes side effects during editing. So remove it
        // for footnotes/endnotes to restore original LO behavior here.
        pTopContext->Erase(PROP_CHAR_STYLE_NAME);

        uno::Reference< text::XText > xFootnoteText;
        if (m_xTextDocument)
            xFootnoteText.set( m_xTextDocument->createInstance(
            bIsFootnote ?
                OUString( "com.sun.star.text.Footnote" ) : OUString( "com.sun.star.text.Endnote" )),
            uno::UNO_QUERY_THROW );
        uno::Reference< text::XFootnote > xFootnote( xFootnoteText, uno::UNO_QUERY_THROW );
        pTopContext->SetFootnote(xFootnote, sFootnoteCharStyleName);
        uno::Sequence< beans::PropertyValue > aFontProperties;
        if (GetTopContextOfType(CONTEXT_CHARACTER))
            aFontProperties = GetTopContextOfType(CONTEXT_CHARACTER)->GetPropertyValues();
        appendTextContent( uno::Reference< text::XTextContent >( xFootnoteText, uno::UNO_QUERY_THROW ), aFontProperties );
        m_aTextAppendStack.push(TextAppendContext(uno::Reference< text::XTextAppend >( xFootnoteText, uno::UNO_QUERY_THROW ),
                    xFootnoteText->createTextCursorByRange(xFootnoteText->getStart())));

        // Redlines for the footnote anchor in the main text content
        std::vector< RedlineParamsPtr > aFootnoteRedline = std::move(m_aRedlines.top());
        m_aRedlines.pop();
        CheckRedline( xFootnote->getAnchor( ) );
        m_aRedlines.push( aFootnoteRedline );

        // Try scanning for custom footnote labels
        if (!sFootnoteCharStyleName.isEmpty())
            StartCustomFootnote(pTopContext);
        else
            EndCustomFootnote();
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("writerfilter.dmapper", "PushFootOrEndnote");
    }
}

void DomainMapper_Impl::CreateRedline(uno::Reference<text::XTextRange> const& xRange,
        const RedlineParamsPtr& pRedline)
{
    if ( !pRedline )
        return;

    bool bRedlineMoved = false;
    try
    {
        OUString sType;
        switch ( pRedline->m_nToken & 0xffff )
        {
        case XML_mod:
            sType = getPropertyName( PROP_FORMAT );
            break;
        case XML_moveTo:
            bRedlineMoved = true;
            m_pParaMarkerRedlineMove = pRedline.get();
            [[fallthrough]];
        case XML_ins:
            sType = getPropertyName( PROP_INSERT );
            break;
        case XML_moveFrom:
            bRedlineMoved = true;
            m_pParaMarkerRedlineMove = pRedline.get();
            [[fallthrough]];
        case XML_del:
            sType = getPropertyName( PROP_DELETE );
            break;
        case XML_ParagraphFormat:
            sType = getPropertyName( PROP_PARAGRAPH_FORMAT );
            break;
        default:
            throw lang::IllegalArgumentException("illegal redline token type", nullptr, 0);
        }
        beans::PropertyValues aRedlineProperties( 4 );
        beans::PropertyValue * pRedlineProperties = aRedlineProperties.getArray(  );
        pRedlineProperties[0].Name = getPropertyName( PROP_REDLINE_AUTHOR );
        pRedlineProperties[0].Value <<= pRedline->m_sAuthor;
        pRedlineProperties[1].Name = getPropertyName( PROP_REDLINE_DATE_TIME );
        util::DateTime aDateTime = ConversionHelper::ConvertDateStringToDateTime( pRedline->m_sDate );
        // tdf#146171 import not specified w:date (or specified as zero date "0-00-00")
        // as Epoch time to avoid of losing change tracking data during ODF roundtrip
        if ( aDateTime.Year == 0 && aDateTime.Month == 0 && aDateTime.Day == 0 )
        {
            aDateTime.Year = 1970;
            aDateTime.Month = 1;
            aDateTime.Day = 1;
        }
        pRedlineProperties[1].Value <<= aDateTime;
        pRedlineProperties[2].Name = getPropertyName( PROP_REDLINE_REVERT_PROPERTIES );
        pRedlineProperties[2].Value <<= pRedline->m_aRevertProperties;

        sal_uInt32 nRedlineMovedID = 0;
        if (bRedlineMoved)
        {
            if (!m_sCurrentBkmkId.isEmpty())
            {
                nRedlineMovedID = 1;
                BookmarkMap_t::iterator aBookmarkIter = m_aBookmarkMap.find(m_sCurrentBkmkId);
                if (aBookmarkIter != m_aBookmarkMap.end())
                {
                    OUString sMoveID = aBookmarkIter->second.m_sBookmarkName;
                    auto aIter = m_aRedlineMoveIDs.end();

                    if (sMoveID.indexOf("__RefMoveFrom__") >= 0)
                    {
                        aIter = std::find(m_aRedlineMoveIDs.begin(), m_aRedlineMoveIDs.end(),
                                          sMoveID.subView(15));
                    }
                    else if (sMoveID.indexOf("__RefMoveTo__") >= 0)
                    {
                        aIter = std::find(m_aRedlineMoveIDs.begin(), m_aRedlineMoveIDs.end(),
                                          sMoveID.subView(13));
                    };

                    if (aIter != m_aRedlineMoveIDs.end())
                    {
                        nRedlineMovedID = aIter - m_aRedlineMoveIDs.begin() + 2;
                        m_nLastRedlineMovedID = nRedlineMovedID;
                    }
                }
            }
            else
                nRedlineMovedID = m_nLastRedlineMovedID;
        }
        pRedlineProperties[3].Name = "RedlineMoved";
        pRedlineProperties[3].Value <<= nRedlineMovedID;

        if (!m_bIsActualParagraphFramed)
        {
            uno::Reference < text::XRedline > xRedline( xRange, uno::UNO_QUERY_THROW );
            xRedline->makeRedline( sType, aRedlineProperties );
        }
        // store frame and (possible floating) table redline data for restoring them after frame conversion
        enum StoredRedlines eType;
        if (m_bIsActualParagraphFramed || 0 < m_StreamStateStack.top().nTableDepth)
            eType = StoredRedlines::FRAME;
        else if (IsInFootOrEndnote())
            eType = IsInFootnote() ? StoredRedlines::FOOTNOTE : StoredRedlines::ENDNOTE;
        else
            eType = StoredRedlines::NONE;

        if (eType != StoredRedlines::NONE)
        {
            m_aStoredRedlines[eType].emplace_back(xRange);
            m_aStoredRedlines[eType].emplace_back(sType);
            m_aStoredRedlines[eType].emplace_back(aRedlineProperties);
        }
    }
    catch( const uno::Exception & )
    {
        TOOLS_WARN_EXCEPTION( "writerfilter", "in makeRedline" );
    }
}

void DomainMapper_Impl::CheckParaMarkerRedline( uno::Reference< text::XTextRange > const& xRange )
{
    if ( m_pParaMarkerRedline )
    {
        CreateRedline( xRange, m_pParaMarkerRedline );
        if ( m_pParaMarkerRedline )
        {
            m_pParaMarkerRedline.clear();
            m_currentRedline.clear();
        }
    }
    else if ( m_pParaMarkerRedlineMove && m_bIsParaMarkerMove )
    {
        // terminating moveFrom/moveTo redline removes also the paragraph mark
        CreateRedline( xRange, m_pParaMarkerRedlineMove );
    }
    if ( m_pParaMarkerRedlineMove )
    {
        m_pParaMarkerRedlineMove.clear();
        EndParaMarkerMove();
    }
}

void DomainMapper_Impl::CheckRedline( uno::Reference< text::XTextRange > const& xRange )
{
    // Writer core "officially" does not like overlapping redlines, and its UNO interface is stupid enough
    // to not prevent that. However, in practice in fact everything appears to work fine (except for the debug warnings
    // about redline table corruption, which may possibly be harmless in reality). So leave this as it is, since this
    // is a better representation of how the changes happened. If this will ever become a problem, overlapping redlines
    // will need to be merged into one, just like doing the changes in the UI does, which will lose some information
    // (and so if that happens, it may be better to fix Writer).
    // Create the redlines here from lowest (formats) to highest (inserts/removals) priority, since the last one is
    // what Writer presents graphically, so this will show deletes as deleted text and not as just formatted text being there.
    bool bUsedRange = m_aRedlines.top().size() > 0 || (GetTopContextOfType(CONTEXT_CHARACTER) &&
        GetTopContextOfType(CONTEXT_CHARACTER)->Redlines().size() > 0);

    // only export ParagraphFormat, when there is no other redline in the same text portion to avoid missing redline compression,
    // but always export the first ParagraphFormat redline in a paragraph to keep the paragraph style change data for rejection
    if ((!bUsedRange || !m_StreamStateStack.top().bParaChanged)
        && GetTopContextOfType(CONTEXT_PARAGRAPH))
    {
        std::vector<RedlineParamsPtr>& avRedLines = GetTopContextOfType(CONTEXT_PARAGRAPH)->Redlines();
        for( const auto& rRedline : avRedLines )
            CreateRedline( xRange, rRedline );
    }
    if( GetTopContextOfType(CONTEXT_CHARACTER) )
    {
        std::vector<RedlineParamsPtr>& avRedLines = GetTopContextOfType(CONTEXT_CHARACTER)->Redlines();
        for( const auto& rRedline : avRedLines )
            CreateRedline( xRange, rRedline );
    }
    for (const auto& rRedline : m_aRedlines.top() )
        CreateRedline( xRange, rRedline );
}

void DomainMapper_Impl::StartParaMarkerChange( )
{
    m_bIsParaMarkerChange = true;
}

void DomainMapper_Impl::EndParaMarkerChange( )
{
    m_bIsParaMarkerChange = false;
    m_previousRedline = m_currentRedline;
    m_currentRedline.clear();
}

void DomainMapper_Impl::StartParaMarkerMove( )
{
    m_bIsParaMarkerMove = true;
}

void DomainMapper_Impl::EndParaMarkerMove( )
{
    m_bIsParaMarkerMove = false;
}

void DomainMapper_Impl::StartCustomFootnote(const PropertyMapPtr pContext)
{
    if (pContext == m_pFootnoteContext)
        return;

    assert(pContext->GetFootnote().is());
    m_StreamStateStack.top().bHasFootnoteStyle = true;
    m_StreamStateStack.top().bCheckFootnoteStyle = !pContext->GetFootnoteStyle().isEmpty();
    m_pFootnoteContext = pContext;
}

void DomainMapper_Impl::EndCustomFootnote()
{
    m_StreamStateStack.top().bHasFootnoteStyle = false;
    m_StreamStateStack.top().bCheckFootnoteStyle = false;
}

void DomainMapper_Impl::PushAnnotation()
{
    try
    {
        m_StreamStateStack.top().eSubstreamType = SubstreamType::Annotation;
        if (!m_xTextDocument)
            return;
        m_xAnnotationField.set( m_xTextDocument->createInstance( "com.sun.star.text.TextField.Annotation" ),
                                uno::UNO_QUERY_THROW );
        uno::Reference< text::XText > xAnnotationText;
        m_xAnnotationField->getPropertyValue("TextRange") >>= xAnnotationText;
        m_aTextAppendStack.push(TextAppendContext(uno::Reference< text::XTextAppend >( xAnnotationText, uno::UNO_QUERY_THROW ),
                    m_bIsNewDoc ? uno::Reference<text::XTextCursor>() : xAnnotationText->createTextCursorByRange(xAnnotationText->getStart())));
    }
    catch( const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("writerfilter.dmapper");
    }
}

static void lcl_CopyRedlines(
                uno::Reference< text::XText > const& xSrc,
                std::deque<css::uno::Any>& rRedlines,
                std::vector<sal_Int32>& redPos,
                std::vector<sal_Int32>& redLen,
                sal_Int32& redIdx)
{
    redIdx = -1;
    for( size_t i = 0; i < rRedlines.size(); i+=3)
    {
        uno::Reference< text::XTextRange > xRange;
        rRedlines[i] >>= xRange;

        // is this a redline of the temporary footnote?
        uno::Reference<text::XTextCursor> xRangeCursor;
        try
        {
            xRangeCursor = xSrc->createTextCursorByRange( xRange );
        }
        catch( const uno::Exception& )
        {
        }
        if (xRangeCursor.is())
        {
            redIdx = i;
            sal_Int32 nLen = xRange->getString().getLength();
            redLen.push_back(nLen);
            xRangeCursor->gotoRange(xSrc->getStart(), true);
            redPos.push_back(xRangeCursor->getString().getLength() - nLen);
        }
        else
        {
            // we have already found all redlines of the footnote,
            // skip checking the redlines of the other footnotes
            if (redIdx > -1)
                break;
            // failed createTextCursorByRange(), for example, table inside the frame
            redLen.push_back(-1);
            redPos.push_back(-1);
        }
    }
}

static void lcl_PasteRedlines(
                uno::Reference< text::XText > const& xDest,
                std::deque<css::uno::Any>& rRedlines,
                std::vector<sal_Int32>& redPos,
                std::vector<sal_Int32>& redLen,
                sal_Int32 redIdx)
{
    // create redlines in the copied footnote
    for( size_t i = 0; redIdx > -1 && i <= sal::static_int_cast<size_t>(redIdx); i+=3)
    {
        OUString sType;
        beans::PropertyValues aRedlineProperties( 3 );
        // skip failed createTextCursorByRange()
        if (redPos[i/3] == -1)
            continue;
        rRedlines[i+1] >>= sType;
        rRedlines[i+2] >>= aRedlineProperties;
        uno::Reference< text::XTextCursor > xCrsr = xDest->getText()->createTextCursor();
        xCrsr->goRight(redPos[i/3], false);
        xCrsr->goRight(redLen[i/3], true);
        uno::Reference < text::XRedline > xRedline( xCrsr, uno::UNO_QUERY_THROW );
        try {
            xRedline->makeRedline( sType, aRedlineProperties );
        }
        catch(const uno::Exception&)
        {
            // ignore (footnotes of tracked deletions)
        }
    }
}

bool DomainMapper_Impl::CopyTemporaryNotes(
        uno::Reference< text::XFootnote > xNoteSrc,
        uno::Reference< text::XFootnote > xNoteDest )
{
    if (!m_bSaxError && xNoteSrc != xNoteDest)
    {
        uno::Reference< text::XText > xSrc( xNoteSrc, uno::UNO_QUERY_THROW );
        uno::Reference< text::XText > xDest( xNoteDest, uno::UNO_QUERY_THROW );
        uno::Reference< text::XTextCopy > xTxt, xTxt2;
        xTxt.set(  xSrc, uno::UNO_QUERY_THROW );
        xTxt2.set( xDest, uno::UNO_QUERY_THROW );
        xTxt2->copyText( xTxt );

        // copy its redlines
        std::vector<sal_Int32> redPos, redLen;
        sal_Int32 redIdx;
        enum StoredRedlines eType = IsInFootnote() ? StoredRedlines::FOOTNOTE : StoredRedlines::ENDNOTE;
        lcl_CopyRedlines(xSrc, m_aStoredRedlines[eType], redPos, redLen, redIdx);
        lcl_PasteRedlines(xDest, m_aStoredRedlines[eType], redPos, redLen, redIdx);

        // remove processed redlines
        for( size_t i = 0; redIdx > -1 && i <= sal::static_int_cast<size_t>(redIdx) + 2; i++)
            m_aStoredRedlines[eType].pop_front();

        return true;
    }

    return false;
}

void DomainMapper_Impl::RemoveTemporaryFootOrEndnotes()
{
    rtl::Reference< SwXTextDocument> xTextDoc( GetTextDocument() );
    uno::Reference< text::XFootnote > xNote;
    if  (GetFootnoteCount() > 0)
    {
        auto xFootnotes = xTextDoc->getFootnotes();
        if ( m_nFirstFootnoteIndex > 0 )
        {
            uno::Reference< text::XFootnote > xFirstNote;
            xFootnotes->getByIndex(0) >>= xFirstNote;
            uno::Reference< text::XText > xText( xFirstNote, uno::UNO_QUERY_THROW );
            xText->setString("");
            xFootnotes->getByIndex(m_nFirstFootnoteIndex) >>= xNote;
            CopyTemporaryNotes(xNote, xFirstNote);
        }
        for (sal_Int32 i = GetFootnoteCount(); i > 0; --i)
        {
            xFootnotes->getByIndex(i) >>= xNote;
            xNote->getAnchor()->setString("");
        }
    }
    if  (GetEndnoteCount() > 0)
    {
        auto xEndnotes = xTextDoc->getEndnotes();
        if ( m_nFirstEndnoteIndex > 0 )
        {
            uno::Reference< text::XFootnote > xFirstNote;
            xEndnotes->getByIndex(0) >>= xFirstNote;
            uno::Reference< text::XText > xText( xFirstNote, uno::UNO_QUERY_THROW );
            xText->setString("");
            xEndnotes->getByIndex(m_nFirstEndnoteIndex) >>= xNote;
            CopyTemporaryNotes(xNote, xFirstNote);
        }
        for (sal_Int32 i = GetEndnoteCount(); i > 0; --i)
        {
            xEndnotes->getByIndex(i) >>= xNote;
            xNote->getAnchor()->setString("");
        }
    }
}

static void lcl_convertToNoteIndices(std::deque<sal_Int32>& rNoteIds, sal_Int32& rFirstNoteIndex)
{
    // rNoteIds contains XML footnote identifiers in the loaded order of the footnotes
    // (the same order as in footnotes.xml), i.e. it maps temporary footnote positions to the
    // identifiers. For example: Ids[0] = 100; Ids[1] = -1, Ids[2] = 5.
    // To copy the footnotes in their final place, create an array, which map the (normalized)
    // footnote identifiers to the temporary footnote positions. Using the previous example,
    // Pos[0] = 1; Pos[1] = 2; Pos[2] = 0 (where [0], [1], [2] are the normalized
    // -1, 5 and 100 identifiers).
    std::deque<sal_Int32> aSortedIds = rNoteIds;
    std::sort(aSortedIds.begin(), aSortedIds.end());
    std::map<sal_Int32, size_t> aMapIds;
    // normalize footnote identifiers to 0, 1, 2 ...
    for (size_t i = 0; i < aSortedIds.size(); ++i)
        aMapIds[aSortedIds[i]] = i;
    // reusing rNoteIds, create the Pos array to map normalized identifiers to the loaded positions
    std::deque<sal_Int32> aOrigNoteIds = rNoteIds;
    for (size_t i = 0; i < rNoteIds.size(); ++i)
        rNoteIds[aMapIds[aOrigNoteIds[i]]] = i;
    rFirstNoteIndex = rNoteIds.front();
    rNoteIds.pop_front();
}

void DomainMapper_Impl::PopFootOrEndnote()
{
    // content of the footnotes were inserted after the first footnote in temporary footnotes,
    // restore the content of the actual footnote by copying its content from the first
    // (remaining) temporary footnote and remove the temporary footnote.
    bool bCopied = false;
    if ( m_xTextDocument && IsInFootOrEndnote() && ( ( IsInFootnote() && GetFootnoteCount() > -1 ) ||
         ( !IsInFootnote() && GetEndnoteCount() > -1 ) ) )
    {
        uno::Reference< text::XFootnote > xNoteFirst, xNoteLast;
        auto xFootnotes = m_xTextDocument->getFootnotes();
        auto xEndnotes = m_xTextDocument->getEndnotes();
        if ( ( ( IsInFootnote() && xFootnotes->getCount() > 1 &&
                       ( xFootnotes->getByIndex(xFootnotes->getCount()-1) >>= xNoteLast ) ) ||
               ( !IsInFootnote() && xEndnotes->getCount() > 1 &&
                       ( xEndnotes->getByIndex(xEndnotes->getCount()-1) >>= xNoteLast ) )
             ) && xNoteLast->getLabel().isEmpty() )
        {
            // copy content of the next temporary footnote
            try
            {
                if ( IsInFootnote() && !m_aFootnoteIds.empty() )
                {
                    if ( m_nFirstFootnoteIndex == -1 )
                        lcl_convertToNoteIndices(m_aFootnoteIds, m_nFirstFootnoteIndex);
                    if (m_aFootnoteIds.empty()) // lcl_convertToNoteIndices pops m_aFootnoteIds
                        m_bSaxError = true;
                    else
                    {
                        xFootnotes->getByIndex(m_aFootnoteIds.front()) >>= xNoteFirst;
                        m_aFootnoteIds.pop_front();
                    }
                }
                else if ( !IsInFootnote() && !m_aEndnoteIds.empty() )
                {
                    if ( m_nFirstEndnoteIndex == -1 )
                        lcl_convertToNoteIndices(m_aEndnoteIds, m_nFirstEndnoteIndex);
                    if (m_aEndnoteIds.empty()) // lcl_convertToNoteIndices pops m_aEndnoteIds
                        m_bSaxError = true;
                    else
                    {
                        xEndnotes->getByIndex(m_aEndnoteIds.front()) >>= xNoteFirst;
                        m_aEndnoteIds.pop_front();
                    }
                }
                else
                    m_bSaxError = true;
            }
            catch (uno::Exception const&)
            {
                TOOLS_WARN_EXCEPTION("writerfilter.dmapper", "Cannot insert footnote/endnote");
                m_bSaxError = true;
            }

            bCopied = CopyTemporaryNotes(xNoteFirst, xNoteLast);
        }
    }

    if (!IsRTFImport() && !bCopied)
        RemoveLastParagraph();

    if (!m_aTextAppendStack.empty())
        m_aTextAppendStack.pop();

    if (m_aRedlines.size() == 1)
    {
        SAL_WARN("writerfilter.dmapper", "PopFootOrEndnote() is called without PushFootOrEndnote()?");
        return;
    }
    m_aRedlines.pop();
    m_eSkipFootnoteState = SkipFootnoteSeparator::OFF;
    m_pFootnoteContext = nullptr;
}

void DomainMapper_Impl::PopAnnotation()
{
    RemoveLastParagraph();

    m_aTextAppendStack.pop();

    try
    {
        if (m_bAnnotationResolved)
            m_xAnnotationField->setPropertyValue("Resolved", uno::Any(true));

        m_xAnnotationField->setPropertyValue("ParaIdParent", uno::Any(m_sAnnotationParent));
        m_xAnnotationField->setPropertyValue("ParaId", uno::Any(m_sAnnotationImportedParaId));

        // See if the annotation will be a single position or a range.
        if (m_nAnnotationId == -1 || !m_aAnnotationPositions[m_nAnnotationId].m_xStart.is() || !m_aAnnotationPositions[m_nAnnotationId].m_xEnd.is())
        {
            uno::Sequence< beans::PropertyValue > aEmptyProperties;
            uno::Reference< text::XTextContent > xContent( m_xAnnotationField, uno::UNO_QUERY_THROW );
            appendTextContent( xContent, aEmptyProperties );
            CheckRedline( xContent->getAnchor( ) );
        }
        else
        {
            AnnotationPosition& aAnnotationPosition = m_aAnnotationPositions[m_nAnnotationId];
            // Create a range that points to the annotation start/end.
            uno::Reference<text::XText> const xText = aAnnotationPosition.m_xStart->getText();
            uno::Reference<text::XTextCursor> const xCursor = xText->createTextCursorByRange(aAnnotationPosition.m_xStart);

            bool bMarker = false;
            uno::Reference<text::XTextRangeCompare> xTextRangeCompare(xText, uno::UNO_QUERY);
            if (xTextRangeCompare->compareRegionStarts(aAnnotationPosition.m_xStart, aAnnotationPosition.m_xEnd) == 0)
            {
                // Insert a marker so that comment around an anchored image is not collapsed during
                // insertion.
                xText->insertString(xCursor, "x", false);
                bMarker = true;
            }

            xCursor->gotoRange(aAnnotationPosition.m_xEnd, true);
            uno::Reference<text::XTextRange> const xTextRange(xCursor, uno::UNO_QUERY_THROW);

            // Attach the annotation to the range.
            uno::Reference<text::XTextAppend> const xTextAppend = m_aTextAppendStack.top().xTextAppend;
            xTextAppend->insertTextContent(xTextRange, uno::Reference<text::XTextContent>(m_xAnnotationField, uno::UNO_QUERY_THROW), !xCursor->isCollapsed());

            if (bMarker)
            {
                // Remove the marker.
                xCursor->goLeft(1, true);
                xCursor->setString(OUString());
            }
        }
        m_aAnnotationPositions.erase( m_nAnnotationId );
    }
    catch (uno::Exception const&)
    {
        TOOLS_WARN_EXCEPTION("writerfilter.dmapper", "Cannot insert annotation field");
    }

    m_xAnnotationField.clear();
    m_sAnnotationParent.clear();
    m_sAnnotationImportedParaId.clear();
    m_nAnnotationId = -1;
    m_bAnnotationResolved = false;
}

void DomainMapper_Impl::PushPendingShape( const uno::Reference< drawing::XShape > & xShape )
{
    m_aPendingShapes.push_back(xShape);
}

uno::Reference<drawing::XShape> DomainMapper_Impl::PopPendingShape()
{
    uno::Reference<drawing::XShape> xRet;
    if (!m_aPendingShapes.empty())
    {
        xRet = m_aPendingShapes.front();
        m_aPendingShapes.pop_front();
    }
    return xRet;
}

void DomainMapper_Impl::PushShapeContext( const uno::Reference< drawing::XShape > & xShape )
{
    // Append these early, so the context and the table manager stack will be
    // in sync, even if the text append stack is empty.
    appendTableManager();
    appendTableHandler();
    getTableManager().startLevel();

    if (m_aTextAppendStack.empty())
        return;
    uno::Reference<text::XTextAppend> xTextAppend = m_aTextAppendStack.top().xTextAppend;

    try
    {
        uno::Reference< lang::XServiceInfo > xSInfo( xShape, uno::UNO_QUERY_THROW );
        if (xSInfo->supportsService("com.sun.star.drawing.GroupShape"))
        {
            // Textboxes in shapes do not support styles, so check saved style information and apply properties directly to the child shapes.
            const uno::Reference<drawing::XShapes> xShapes(xShape, uno::UNO_QUERY);
            const sal_uInt32 nShapeCount = xShapes.is() ? xShapes->getCount() : 0;
            for ( sal_uInt32 i = 0; i < nShapeCount; ++i )
            {
                try
                {
                    uno::Reference<text::XTextRange> xFrame(xShapes->getByIndex(i), uno::UNO_QUERY);
                    uno::Reference<beans::XPropertySet> xFramePropertySet;
                    if (xFrame)
                        xFramePropertySet.set(xFrame, uno::UNO_QUERY_THROW);
                    uno::Reference<beans::XPropertySet> xShapePropertySet(xShapes->getByIndex(i), uno::UNO_QUERY_THROW);

                    comphelper::SequenceAsHashMap aGrabBag( xShapePropertySet->getPropertyValue("CharInteropGrabBag") );

                    // only VML import has checked for style. Don't apply default parastyle properties to other imported shapes
                    // - except for fontsize - to maintain compatibility with previous versions of LibreOffice.
                    const bool bOnlyApplyCharHeight = !aGrabBag["mso-pStyle"].hasValue();

                    OUString sStyleName;
                    aGrabBag["mso-pStyle"] >>= sStyleName;
                    StyleSheetEntryPtr pEntry = GetStyleSheetTable()->FindStyleSheetByISTD( sStyleName );
                    if ( !pEntry )
                    {
                        // Use default style even in ambiguous cases (where multiple styles were defined) since MOST styles inherit
                        // MOST of their properties from the default style. In the ambiguous case, we have to accept some kind of compromise
                        // and the default paragraph style ought to be the safest one... (compared to DocDefaults or program defaults)
                        pEntry = GetStyleSheetTable()->FindStyleSheetByConvertedStyleName( GetDefaultParaStyleName() );
                    }
                    if ( pEntry )
                    {
                        // The Ids here come from oox/source/vml/vmltextbox.cxx.
                        // It probably could safely expand to all Ids that shapes support.
                        const PropertyIds eIds[] = {
                            PROP_CHAR_HEIGHT,
                            PROP_CHAR_FONT_NAME,
                            PROP_CHAR_WEIGHT,
                            PROP_CHAR_CHAR_KERNING,
                            PROP_CHAR_COLOR,
                            PROP_PARA_ADJUST
                        };
                        const uno::Reference<beans::XPropertyState> xShapePropertyState(xShapePropertySet, uno::UNO_QUERY_THROW);
                        for ( const auto& eId : eIds )
                        {
                            try
                            {
                                if ( bOnlyApplyCharHeight && eId != PROP_CHAR_HEIGHT )
                                    continue;

                                const OUString & sPropName = getPropertyName(eId);
                                if ( beans::PropertyState_DEFAULT_VALUE == xShapePropertyState->getPropertyState(sPropName) )
                                {
                                    const uno::Any aProp = GetPropertyFromStyleSheet(eId, pEntry, /*bDocDefaults=*/true, /*bPara=*/true);
                                    if (aProp.hasValue())
                                    {
                                        if (xFrame)
                                            xFramePropertySet->setPropertyValue(sPropName, aProp);
                                        else
                                            xShapePropertySet->setPropertyValue(sPropName, aProp);
                                    }
                                }
                            }
                            catch (const uno::Exception&)
                            {
                                TOOLS_WARN_EXCEPTION( "writerfilter.dmapper", "PushShapeContext() text stylesheet property exception" );
                            }
                        }
                    }
                }
                catch (const uno::Exception&)
                {
                    TOOLS_WARN_EXCEPTION( "writerfilter.dmapper", "PushShapeContext()" );
                }
            }

            uno::Reference<beans::XPropertySet> xShapePropertySet(xShape, uno::UNO_QUERY);
            uno::Sequence<beans::PropertyValue> aGrabBag;
            xShapePropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;

            for (const auto& rProp : aGrabBag)
            {
                if (rProp.Name == "VML-Z-ORDER")
                {
                    sal_Int64 zOrder(0);
                    rProp.Value >>= zOrder;

                    text::TextContentAnchorType nAnchorType
                        = text::TextContentAnchorType_AT_PARAGRAPH;
                    xShapePropertySet->getPropertyValue(getPropertyName(PROP_ANCHOR_TYPE))
                        >>= nAnchorType;

                    const uno::Any aOpaque(nAnchorType == text::TextContentAnchorType_AS_CHARACTER
                                           || (zOrder >= 0 && !IsInHeaderFooter()));
                    xShapePropertySet->setPropertyValue(getPropertyName(PROP_OPAQUE), aOpaque);
                }
            }
            // A GroupShape doesn't implement text::XTextRange, but appending
            // an empty reference to the stacks still makes sense, because this
            // way bToRemove can be set, and we won't end up with duplicated
            // shapes for OLE objects.
            m_aTextAppendStack.push(TextAppendContext(uno::Reference<text::XTextAppend>(xShape, uno::UNO_QUERY), uno::Reference<text::XTextCursor>()));
            uno::Reference<text::XTextContent> xTxtContent(xShape, uno::UNO_QUERY);
            m_aAnchoredStack.push(AnchoredContext(xTxtContent));
        }
        else if (xSInfo->supportsService("com.sun.star.drawing.OLE2Shape"))
        {
            // OLE2Shape from oox should be converted to a TextEmbeddedObject for sw.
            m_aTextAppendStack.push(TextAppendContext(uno::Reference<text::XTextAppend>(xShape, uno::UNO_QUERY), uno::Reference<text::XTextCursor>()));
            uno::Reference<text::XTextContent> xTextContent(xShape, uno::UNO_QUERY);
            m_aAnchoredStack.push(AnchoredContext(xTextContent));
            uno::Reference<beans::XPropertySet> xShapePropertySet(xShape, uno::UNO_QUERY);

            m_StreamStateStack.top().xEmbedded.set(m_xTextDocument->createInstance("com.sun.star.text.TextEmbeddedObject"), uno::UNO_QUERY_THROW);
            uno::Reference<beans::XPropertySet> xEmbeddedProperties(m_StreamStateStack.top().xEmbedded, uno::UNO_QUERY_THROW);
            xEmbeddedProperties->setPropertyValue(getPropertyName(PROP_EMBEDDED_OBJECT), xShapePropertySet->getPropertyValue(getPropertyName(PROP_EMBEDDED_OBJECT)));
            xEmbeddedProperties->setPropertyValue(getPropertyName(PROP_ANCHOR_TYPE), uno::Any(text::TextContentAnchorType_AS_CHARACTER));
            // So that the original bitmap-only shape will be replaced by the embedded object.
            m_aAnchoredStack.top().bToRemove = true;
            m_aTextAppendStack.pop();
            appendTextContent(m_StreamStateStack.top().xEmbedded, uno::Sequence<beans::PropertyValue>());
        }
        else
        {
            uno::Reference<text::XTextRange> xShapeTextRange(xShape, uno::UNO_QUERY_THROW);
            // Add the shape to the text append stack
            uno::Reference<text::XTextAppend> xShapeTextAppend(xShape, uno::UNO_QUERY_THROW);
            uno::Reference<text::XTextCursor> xTextCursor;
            if (!m_bIsNewDoc)
            {
                xTextCursor = xShapeTextRange->getText()->createTextCursorByRange(
                    xShapeTextRange->getStart());
            }
            TextAppendContext aContext(xShapeTextAppend, xTextCursor);
            m_aTextAppendStack.push(aContext);

            // Add the shape to the anchored objects stack
            uno::Reference< text::XTextContent > xTxtContent( xShape, uno::UNO_QUERY_THROW );
            m_aAnchoredStack.push( AnchoredContext(xTxtContent) );

            uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY_THROW );
#ifdef DBG_UTIL
            TagLogger::getInstance().unoPropertySet(xProps);
#endif
            text::TextContentAnchorType nAnchorType(text::TextContentAnchorType_AT_PARAGRAPH);
            xProps->getPropertyValue(getPropertyName( PROP_ANCHOR_TYPE )) >>= nAnchorType;
            bool checkZOrderStatus = false;
            if (xSInfo->supportsService("com.sun.star.text.TextFrame"))
            {
                SetIsTextFrameInserted(true);
                // Extract the special "btLr text frame" mode, requested by oox, if needed.
                // Extract vml ZOrder from FrameInteropGrabBag
                uno::Reference<beans::XPropertySet> xShapePropertySet(xShape, uno::UNO_QUERY);
                uno::Sequence<beans::PropertyValue> aGrabBag;
                xShapePropertySet->getPropertyValue("FrameInteropGrabBag") >>= aGrabBag;

                for (const auto& rProp : aGrabBag)
                {
                    if (rProp.Name == "VML-Z-ORDER")
                    {
                        GraphicZOrderHelper& rZOrderHelper = m_rDMapper.graphicZOrderHelper();
                        sal_Int64 zOrder(0);
                        rProp.Value >>= zOrder;
                        GraphicZOrderHelper::adjustRelativeHeight(zOrder, /*IsZIndex=*/true,
                                                                  zOrder < 0, IsInHeaderFooter());
                        xShapePropertySet->setPropertyValue("ZOrder",
                            uno::Any(rZOrderHelper.findZOrder(zOrder, /*LastDuplicateWins*/true)));
                        rZOrderHelper.addItem(xShapePropertySet, zOrder);
                        xShapePropertySet->setPropertyValue(getPropertyName( PROP_OPAQUE ), uno::Any( zOrder >= 0 ) );
                        checkZOrderStatus = true;
                    }
                    else if ( rProp.Name == "TxbxHasLink" )
                    {
                        //Chaining of textboxes will happen in ~DomainMapper_Impl
                        //i.e when all the textboxes are read and all its attributes
                        //have been set ( basically the Name/LinkedDisplayName )
                        //which is set in Graphic Import.
                        m_vTextFramesForChaining.push_back(xShape);
                    }
                }

                uno::Reference<text::XTextContent> xTextContent(xShape, uno::UNO_QUERY_THROW);
                uno::Reference<text::XTextRange> xTextRange(xTextAppend->createTextCursorByRange(xTextAppend->getEnd()), uno::UNO_QUERY_THROW);
                xTextAppend->insertTextContent(xTextRange, xTextContent, false);

                uno::Reference<beans::XPropertySet> xPropertySet(xTextContent, uno::UNO_QUERY);
                // we need to re-set this value to xTextContent, then only values are preserved.
                xPropertySet->setPropertyValue("FrameInteropGrabBag",uno::Any(aGrabBag));
            }
            else if (nAnchorType == text::TextContentAnchorType_AS_CHARACTER)
            {
                // Fix spacing for as-character objects. If the paragraph has CT_Spacing_after set,
                // it needs to be set on the object too, as that's what object placement code uses.
                PropertyMapPtr paragraphContext = GetTopContextOfType( CONTEXT_PARAGRAPH );
                std::optional<PropertyMap::Property> aPropMargin = paragraphContext->getProperty(PROP_PARA_BOTTOM_MARGIN);
                if(aPropMargin)
                    xProps->setPropertyValue( getPropertyName( PROP_BOTTOM_MARGIN ), aPropMargin->second );
            }
            else
            {
                uno::Reference<beans::XPropertySet> xShapePropertySet(xShape, uno::UNO_QUERY);
                uno::Sequence<beans::PropertyValue> aGrabBag;
                xShapePropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
                for (const auto& rProp : aGrabBag)
                {
                    if (rProp.Name == "VML-Z-ORDER")
                    {
                        GraphicZOrderHelper& rZOrderHelper = m_rDMapper.graphicZOrderHelper();
                        sal_Int64 zOrder(0);
                        rProp.Value >>= zOrder;
                        GraphicZOrderHelper::adjustRelativeHeight(zOrder, /*IsZIndex=*/true,
                                                                  zOrder < 0, IsInHeaderFooter());
                        xShapePropertySet->setPropertyValue("ZOrder",
                            uno::Any(rZOrderHelper.findZOrder(zOrder, /*LastDuplicateWins*/true)));
                        rZOrderHelper.addItem(xShapePropertySet, zOrder);
                        xShapePropertySet->setPropertyValue(getPropertyName( PROP_OPAQUE ), uno::Any( zOrder >= 0 ) );
                        checkZOrderStatus = true;
                    }
                    else if ( rProp.Name == "TxbxHasLink" )
                    {
                        //Chaining of textboxes will happen in ~DomainMapper_Impl
                        //i.e when all the textboxes are read and all its attributes
                        //have been set ( basically the Name/LinkedDisplayName )
                        //which is set in Graphic Import.
                        m_vTextFramesForChaining.push_back(xShape);
                    }
                }

                if(IsSdtEndBefore())
                {
                    uno::Reference< beans::XPropertySetInfo > xPropSetInfo;
                    if(xShapePropertySet.is())
                    {
                        xPropSetInfo = xShapePropertySet->getPropertySetInfo();
                        if (xPropSetInfo.is() && xPropSetInfo->hasPropertyByName("InteropGrabBag"))
                        {
                            uno::Sequence<beans::PropertyValue> aShapeGrabBag( comphelper::InitPropertySequence({
                                { "SdtEndBefore", uno::Any(true) }
                            }));
                            xShapePropertySet->setPropertyValue("InteropGrabBag",uno::Any(aShapeGrabBag));
                        }
                    }
                }
            }
            if (!IsInHeaderFooter() && !checkZOrderStatus)
                xProps->setPropertyValue(
                        getPropertyName( PROP_OPAQUE ),
                        uno::Any( true ) );
        }
        m_StreamStateStack.top().bParaChanged = true;
        getTableManager().setIsInShape(true);
    }
    catch ( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("writerfilter.dmapper", "Exception when adding shape");
    }
}
/*
 * Updating chart height and width after reading the actual values from wp:extent
*/
void DomainMapper_Impl::UpdateEmbeddedShapeProps(const uno::Reference< drawing::XShape > & xShape)
{
    if (!xShape.is())
        return;

    uno::Reference<beans::XPropertySet> const xEmbeddedProperties(m_StreamStateStack.top().xEmbedded, uno::UNO_QUERY_THROW);
    awt::Size aSize = xShape->getSize( );
    xEmbeddedProperties->setPropertyValue(getPropertyName(PROP_WIDTH), uno::Any(sal_Int32(aSize.Width)));
    xEmbeddedProperties->setPropertyValue(getPropertyName(PROP_HEIGHT), uno::Any(sal_Int32(aSize.Height)));
    uno::Reference<beans::XPropertySet> const xShapeProps(xShape, uno::UNO_QUERY);
    // tdf#130782 copy a11y related properties
    xEmbeddedProperties->setPropertyValue(getPropertyName(PROP_DESCRIPTION),
        xShapeProps->getPropertyValue(getPropertyName(PROP_DESCRIPTION)));
    xEmbeddedProperties->setPropertyValue(getPropertyName(PROP_TITLE),
        xShapeProps->getPropertyValue(getPropertyName(PROP_TITLE)));
    uno::Reference<container::XNamed> const xEmbedName(m_StreamStateStack.top().xEmbedded, uno::UNO_QUERY);
    uno::Reference<container::XNamed> const xShapeName(xShape, uno::UNO_QUERY);
    OUString const name(xShapeName->getName());
    if (!name.isEmpty()) // setting empty name will throw
    {
        try
        {
            xEmbedName->setName(name);
        }
        catch (uno::RuntimeException const&)
        {
            // ignore - document may contain duplicates (testchartoleobjectembeddings.docx)
        }
    }
}

void DomainMapper_Impl::PopShapeContext()
{
    if (hasTableManager())
    {
        getTableManager().endLevel();
        popTableManager();
    }
    if ( m_aAnchoredStack.empty() )
        return;

    // For OLE object replacement shape, the text append context was already removed
    // or the OLE object couldn't be inserted.
    if ( !m_aAnchoredStack.top().bToRemove )
    {
        RemoveLastParagraph();
        if (!m_aTextAppendStack.empty())
            m_aTextAppendStack.pop();
    }

    uno::Reference< text::XTextContent > xObj = m_aAnchoredStack.top( ).xTextContent;
    try
    {
        appendTextContent( xObj, uno::Sequence< beans::PropertyValue >() );
    }
    catch ( const uno::RuntimeException& )
    {
        // this is normal: the shape is already attached
    }

    const uno::Reference<drawing::XShape> xShape( xObj, uno::UNO_QUERY_THROW );
    // Remove the shape if required (most likely replacement shape for OLE object)
    // or anchored to a discarded header or footer
    if ( m_xTextDocument && (m_aAnchoredStack.top().bToRemove || m_bDiscardHeaderFooter) )
    {
        try
        {
            uno::Reference<drawing::XDrawPage> xDrawPage = m_xTextDocument->getDrawPage();
            if ( xDrawPage.is() )
                xDrawPage->remove( xShape );
        }
        catch( const uno::Exception& )
        {
        }
    }

    // Relative width calculations deferred until section's margins are defined.
    // Being cautious: only deferring undefined/minimum-width shapes in order to avoid causing potential regressions
    css::awt::Size aShapeSize;
    try
    {
        aShapeSize = xShape->getSize();
    }
    catch (const css::uno::RuntimeException& e)
    {
        // May happen e.g. when text frame has no frame format
        // See sw/qa/extras/ooxmlimport/data/n779627.docx
        SAL_WARN("writerfilter.dmapper", "getSize failed. " << e.Message);
    }
    if( aShapeSize.Width <= 2 )
    {
        const uno::Reference<beans::XPropertySet> xShapePropertySet( xShape, uno::UNO_QUERY );
        SectionPropertyMap* pSectionContext = GetSectionContext();
        if ( pSectionContext && (!hasTableManager() || !getTableManager().isInTable()) &&
             xShapePropertySet->getPropertySetInfo()->hasPropertyByName(getPropertyName(PROP_RELATIVE_WIDTH)) )
        {
            pSectionContext->addRelativeWidthShape(xShape);
        }
    }

    m_aAnchoredStack.pop();
}

bool DomainMapper_Impl::IsSdtEndBefore()
{
    bool bIsSdtEndBefore = false;
    PropertyMapPtr pContext = GetTopContextOfType(CONTEXT_CHARACTER);
    if(pContext)
    {
        const uno::Sequence< beans::PropertyValue > currentCharProps = pContext->GetPropertyValues();
        for (const auto& rCurrentCharProp : currentCharProps)
        {
            if (rCurrentCharProp.Name == "CharInteropGrabBag")
            {
                uno::Sequence<beans::PropertyValue> aCharGrabBag;
                rCurrentCharProp.Value >>= aCharGrabBag;
                for (const auto& rProp : aCharGrabBag)
                {
                    if(rProp.Name == "SdtEndBefore")
                    {
                        rProp.Value >>= bIsSdtEndBefore;
                    }
                }
            }
        }
    }
    return bIsSdtEndBefore;
}

bool DomainMapper_Impl::IsDiscardHeaderFooter() const
{
    return m_bDiscardHeaderFooter;
}

// called from TableManager::closeCell()
void DomainMapper_Impl::ClearPreviousParagraph()
{
    // in table cells, set bottom auto margin of last paragraph to 0, except in paragraphs with numbering
    if ((m_StreamStateStack.top().nTableDepth == (m_StreamStateStack.top().nTableCellDepth + 1))
        && m_StreamStateStack.top().xPreviousParagraph.is()
        && hasTableManager() && getTableManager().isCellLastParaAfterAutospacing())
    {
        uno::Reference<container::XNamed> xPreviousNumberingRules(m_StreamStateStack.top().xPreviousParagraph->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
        if ( !xPreviousNumberingRules.is() || xPreviousNumberingRules->getName().isEmpty() )
            m_StreamStateStack.top().xPreviousParagraph->setPropertyValue("ParaBottomMargin", uno::Any(static_cast<sal_Int32>(0)));
    }

    m_StreamStateStack.top().xPreviousParagraph.clear();

    // next table paragraph will be first paragraph in a cell
    m_StreamStateStack.top().bFirstParagraphInCell = true;
}

void DomainMapper_Impl::HandleAltChunk(const OUString& rStreamName)
{
    try
    {
        // Create the import filter.
        uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(
            comphelper::getProcessServiceFactory());
        uno::Reference<uno::XInterface> xDocxFilter
            = xMultiServiceFactory->createInstance("com.sun.star.comp.Writer.WriterFilter");

        // Set the target document.
        uno::Reference<document::XImporter> xImporter(xDocxFilter, uno::UNO_QUERY);
        xImporter->setTargetDocument(static_cast<SfxBaseModel*>(m_xTextDocument.get()));

        // Set the import parameters.
        uno::Reference<embed::XHierarchicalStorageAccess> xStorageAccess(m_xDocumentStorage,
                                                                         uno::UNO_QUERY);
        if (!xStorageAccess.is())
        {
            return;
        }
        // Turn the ZIP stream into a seekable one, as the importer only works with such streams.
        uno::Reference<io::XStream> xStream = xStorageAccess->openStreamElementByHierarchicalName(
            rStreamName, embed::ElementModes::READ);
        std::unique_ptr<SvStream> pStream = utl::UcbStreamHelper::CreateStream(xStream, true);
        SvMemoryStream aMemory;
        aMemory.WriteStream(*pStream);
        uno::Reference<io::XStream> xInputStream = new utl::OStreamWrapper(aMemory);
        // Not handling AltChunk during paste for now.
        uno::Reference<text::XTextRange> xInsertTextRange = GetCurrentXText()->getEnd();
        uno::Reference<text::XTextRange> xSectionStartingRange;
        SectionPropertyMap* pSectionContext = GetSectionContext();
        if (pSectionContext)
        {
            xSectionStartingRange = pSectionContext->GetStartingRange();
        }
        uno::Sequence<beans::PropertyValue> aDescriptor(comphelper::InitPropertySequence({
            { "InputStream", uno::Any(xInputStream) },
            { "InsertMode", uno::Any(true) },
            { "TextInsertModeRange", uno::Any(xInsertTextRange) },
            { "AltChunkMode", uno::Any(true) },
            { "AltChunkStartingRange", uno::Any(xSectionStartingRange) },
        }));

        // Do the actual import.
        uno::Reference<document::XFilter> xFilter(xDocxFilter, uno::UNO_QUERY);
        xFilter->filter(aDescriptor);
    }
    catch (const uno::Exception& rException)
    {
        SAL_WARN("writerfilter", "DomainMapper_Impl::HandleAltChunk: failed to handle alt chunk: "
                                     << rException.Message);
    }
}

void DomainMapper_Impl::HandlePTab(sal_Int32 nAlignment)
{
    // We only handle the case when the line already has content, so the left-aligned ptab is
    // equivalent to a line break.
    if (nAlignment != NS_ooxml::LN_Value_ST_PTabAlignment_left)
    {
        return;
    }

    if (m_aTextAppendStack.empty())
    {
        return;
    }

    uno::Reference<text::XTextAppend> xTextAppend = m_aTextAppendStack.top().xTextAppend;
    if (!xTextAppend.is())
    {
        return;
    }

    uno::Reference<css::text::XTextRange> xInsertPosition
        = m_aTextAppendStack.top().xInsertPosition;
    if (!xInsertPosition.is())
    {
        xInsertPosition = xTextAppend->getEnd();
    }
    uno::Reference<text::XTextCursor> xCursor
        = xTextAppend->createTextCursorByRange(xInsertPosition);

    // Assume that we just inserted a tab character.
    xCursor->goLeft(1, true);
    if (xCursor->getString() != "\t")
    {
        return;
    }

    // Assume that there is some content before the tab character.
    uno::Reference<text::XParagraphCursor> xParagraphCursor(xCursor, uno::UNO_QUERY);
    if (!xParagraphCursor.is())
    {
        return;
    }

    xCursor->collapseToStart();
    xParagraphCursor->gotoStartOfParagraph(true);
    if (xCursor->isCollapsed())
    {
        return;
    }

    // Then select the tab again and replace with a line break.
    xCursor->collapseToEnd();
    xCursor->goRight(1, true);
    xTextAppend->insertControlCharacter(xCursor, text::ControlCharacter::LINE_BREAK, true);
}

void DomainMapper_Impl::HandleLineBreakClear(sal_Int32 nClear)
{
    switch (nClear)
    {
        case NS_ooxml::LN_Value_ST_BrClear_left:
            // SwLineBreakClear::LEFT
            m_StreamStateStack.top().oLineBreakClear = 1;
            break;
        case NS_ooxml::LN_Value_ST_BrClear_right:
            // SwLineBreakClear::RIGHT
            m_StreamStateStack.top().oLineBreakClear = 2;
            break;
        case NS_ooxml::LN_Value_ST_BrClear_all:
            // SwLineBreakClear::ALL
            m_StreamStateStack.top().oLineBreakClear = 3;
            break;
    }
}

void DomainMapper_Impl::HandleLineBreak(const PropertyMapPtr& pPropertyMap)
{
    if (!m_StreamStateStack.top().oLineBreakClear.has_value())
    {
        appendTextPortion("\n", pPropertyMap);
        return;
    }

    if (m_xTextDocument)
    {
        uno::Reference<text::XTextContent> xLineBreak(
            m_xTextDocument->createInstance("com.sun.star.text.LineBreak"), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xLineBreakProps(xLineBreak, uno::UNO_QUERY);
        xLineBreakProps->setPropertyValue("Clear", uno::Any(*m_StreamStateStack.top().oLineBreakClear));
        appendTextContent(xLineBreak, pPropertyMap->GetPropertyValues());
    }
    m_StreamStateStack.top().oLineBreakClear.reset();
}

static sal_Int16 lcl_ParseNumberingType( std::u16string_view rCommand )
{
    sal_Int16 nRet = style::NumberingType::PAGE_DESCRIPTOR;

    //  The command looks like: " PAGE \* Arabic "
    // tdf#132185: but may as well be "PAGE \* Arabic"
    OUString sNumber;
    constexpr OUString rSeparator(u"\\* "_ustr);
    if (size_t nStartIndex = rCommand.find(rSeparator); nStartIndex != std::u16string_view::npos)
    {
        sal_Int32 nStartIndex2 = nStartIndex + rSeparator.getLength();
        sNumber = o3tl::getToken(rCommand, 0, ' ', nStartIndex2);
    }

    if( !sNumber.isEmpty() )
    {
        //todo: might make sense to hash this list, too
        struct NumberingPairs
        {
            const char* cWordName;
            sal_Int16 nType;
        };
        static const NumberingPairs aNumberingPairs[] =
        {
            {"Arabic", style::NumberingType::ARABIC}
            ,{"ROMAN", style::NumberingType::ROMAN_UPPER}
            ,{"roman", style::NumberingType::ROMAN_LOWER}
            ,{"ALPHABETIC", style::NumberingType::CHARS_UPPER_LETTER}
            ,{"alphabetic", style::NumberingType::CHARS_LOWER_LETTER}
            ,{"CircleNum", style::NumberingType::CIRCLE_NUMBER}
            ,{"ThaiArabic", style::NumberingType::CHARS_THAI}
            ,{"ThaiCardText", style::NumberingType::CHARS_THAI}
            ,{"ThaiLetter", style::NumberingType::CHARS_THAI}
//            ,{"SBCHAR", style::NumberingType::}
//            ,{"DBCHAR", style::NumberingType::}
//            ,{"DBNUM1", style::NumberingType::}
//            ,{"DBNUM2", style::NumberingType::}
//            ,{"DBNUM3", style::NumberingType::}
//            ,{"DBNUM4", style::NumberingType::}
            ,{"Aiueo", style::NumberingType::AIU_FULLWIDTH_JA}
            ,{"Iroha", style::NumberingType::IROHA_FULLWIDTH_JA}
//            ,{"ZODIAC1", style::NumberingType::}
//            ,{"ZODIAC2", style::NumberingType::}
//            ,{"ZODIAC3", style::NumberingType::}
//            ,{"CHINESENUM1", style::NumberingType::}
//            ,{"CHINESENUM2", style::NumberingType::}
//            ,{"CHINESENUM3", style::NumberingType::}
            ,{"ArabicAlpha", style::NumberingType::CHARS_ARABIC}
            ,{"ArabicAbjad", style::NumberingType::FULLWIDTH_ARABIC}
            ,{"Ganada", style::NumberingType::HANGUL_JAMO_KO}
            ,{"Chosung", style::NumberingType::HANGUL_SYLLABLE_KO}
            ,{"KoreanCounting", style::NumberingType::NUMBER_HANGUL_KO}
            ,{"KoreanLegal", style::NumberingType::NUMBER_LEGAL_KO}
            ,{"KoreanDigital", style::NumberingType::NUMBER_DIGITAL_KO}
            ,{"KoreanDigital2", style::NumberingType::NUMBER_DIGITAL2_KO}
/* possible values:
style::NumberingType::

    CHARS_UPPER_LETTER_N
    CHARS_LOWER_LETTER_N
    TRANSLITERATION
    NATIVE_NUMBERING
    CIRCLE_NUMBER
    NUMBER_LOWER_ZH
    NUMBER_UPPER_ZH
    NUMBER_UPPER_ZH_TW
    TIAN_GAN_ZH
    DI_ZI_ZH
    NUMBER_TRADITIONAL_JA
    AIU_HALFWIDTH_JA
    IROHA_HALFWIDTH_JA
    NUMBER_UPPER_KO
    NUMBER_HANGUL_KO
    HANGUL_JAMO_KO
    HANGUL_SYLLABLE_KO
    HANGUL_CIRCLED_JAMO_KO
    HANGUL_CIRCLED_SYLLABLE_KO
    CHARS_HEBREW
    CHARS_NEPALI
    CHARS_KHMER
    CHARS_LAO
    CHARS_TIBETAN
    CHARS_CYRILLIC_UPPER_LETTER_BG
    CHARS_CYRILLIC_LOWER_LETTER_BG
    CHARS_CYRILLIC_UPPER_LETTER_N_BG
    CHARS_CYRILLIC_LOWER_LETTER_N_BG
    CHARS_CYRILLIC_UPPER_LETTER_RU
    CHARS_CYRILLIC_LOWER_LETTER_RU
    CHARS_CYRILLIC_UPPER_LETTER_N_RU
    CHARS_CYRILLIC_LOWER_LETTER_N_RU
    CHARS_CYRILLIC_UPPER_LETTER_SR
    CHARS_CYRILLIC_LOWER_LETTER_SR
    CHARS_CYRILLIC_UPPER_LETTER_N_SR
    CHARS_CYRILLIC_LOWER_LETTER_N_SR
    CHARS_CYRILLIC_UPPER_LETTER_UK
    CHARS_CYRILLIC_LOWER_LETTER_UK
    CHARS_CYRILLIC_UPPER_LETTER_N_UK
    CHARS_CYRILLIC_LOWER_LETTER_N_UK*/

        };
        for(const NumberingPairs& rNumberingPair : aNumberingPairs)
        {
            if( /*sCommand*/sNumber.equalsAscii(rNumberingPair.cWordName ))
            {
                nRet = rNumberingPair.nType;
                break;
            }
        }

    }
    return nRet;
}


static OUString lcl_ParseFormat( const OUString& rCommand )
{
    //  The command looks like: " DATE \@"dd MMMM yyyy" or "09/02/2014"
    OUString command;
    sal_Int32 delimPos = rCommand.indexOf("\\@");
    if (delimPos != -1)
    {
        // Remove whitespace permitted by standard between \@ and "
        const sal_Int32 nQuoteIndex = rCommand.indexOf('\"');
        if (nQuoteIndex != -1)
        {
            sal_Int32 wsChars = nQuoteIndex - delimPos - 2;
            command = rCommand.replaceAt(delimPos+2, wsChars, u"");
        }
        else
        {
            // turn date \@ MM into date \@"MM"
            command = OUString::Concat(rCommand.subView(0, delimPos + 2)) + "\"" + o3tl::trim(rCommand.subView(delimPos + 2)) + "\"";
        }
        return OUString(msfilter::util::findQuotedText(command, u"\\@\"", '\"'));
    }

    return OUString();
}
/*-------------------------------------------------------------------------
extract a parameter (with or without quotes) between the command and the following backslash
  -----------------------------------------------------------------------*/
static OUString lcl_ExtractToken(std::u16string_view rCommand,
        size_t & rIndex, bool & rHaveToken, bool & rIsSwitch)
{
    rHaveToken = false;
    rIsSwitch = false;

    OUStringBuffer token;
    bool bQuoted(false);
    for (; rIndex < rCommand.size(); ++rIndex)
    {
        sal_Unicode const currentChar(rCommand[rIndex]);
        switch (currentChar)
        {
            case '\\':
            {
                if (rIndex == rCommand.size() - 1)
                {
                    SAL_INFO("writerfilter.dmapper", "field: trailing escape");
                    ++rIndex;
                    return OUString();
                }
                sal_Unicode const nextChar(rCommand[rIndex+1]);
                if (bQuoted || '\\' == nextChar)
                {
                    ++rIndex; // read 2 chars
                    token.append(nextChar);
                }
                else // field switch (case insensitive)
                {
                    rHaveToken = true;
                    if (token.isEmpty())
                    {
                        rIsSwitch = true;
                        rIndex += 2; // read 2 chars
                        return OUString(rCommand.substr(rIndex - 2, 2)).toAsciiUpperCase();
                    }
                    else
                    {   // leave rIndex, read it again next time
                        return token.makeStringAndClear();
                    }
                }
            }
            break;
            case '\"':
                if (bQuoted || !token.isEmpty())
                {
                    rHaveToken = true;
                    if (bQuoted)
                    {
                        ++rIndex;
                    }
                    return token.makeStringAndClear();
                }
                else
                {
                    bQuoted = true;
                }
            break;
            case ' ':
                if (bQuoted)
                {
                    token.append(' ');
                }
                else
                {
                    if (!token.isEmpty())
                    {
                        rHaveToken = true;
                        ++rIndex;
                        return token.makeStringAndClear();
                    }
                }
            break;
            case '=':
                if (token.isEmpty())
                {
                    rHaveToken = true;
                    ++rIndex;
                    return "FORMULA";
                }
                else
                    token.append('=');
            break;
            default:
                token.append(currentChar);
            break;
        }
    }
    assert(rIndex == rCommand.size());
    if (bQuoted)
    {
        // MS Word allows this, so just emit a debug message
        SAL_INFO("writerfilter.dmapper",
                    "field argument with unterminated quote");
    }
    rHaveToken = !token.isEmpty();
    return token.makeStringAndClear();
}

std::tuple<OUString, std::vector<OUString>, std::vector<OUString> > splitFieldCommand(std::u16string_view rCommand)
{
    OUString sType;
    std::vector<OUString> arguments;
    std::vector<OUString> switches;
    size_t nStartIndex(0);
    // tdf#54584: Field may be prepended by a backslash
    // This is not an escapement, but already escaped literal "\"
    // MS Word allows this, so just skip it
    if ((rCommand.size() >= nStartIndex + 2) &&
        (rCommand[nStartIndex] == L'\\') &&
        (rCommand[nStartIndex + 1] != L'\\') &&
        (rCommand[nStartIndex + 1] != L' '))
    {
        ++nStartIndex;
    }

    do
    {
        bool bHaveToken;
        bool bIsSwitch;
        OUString const token =
            lcl_ExtractToken(rCommand, nStartIndex, bHaveToken, bIsSwitch);
        assert(nStartIndex <= rCommand.size());
        if (bHaveToken)
        {
            if (sType.isEmpty())
            {
                sType = token.toAsciiUpperCase();
            }
            else if (bIsSwitch || !switches.empty())
            {
                switches.push_back(token);
            }
            else
            {
                arguments.push_back(token);
            }
        }
    } while (nStartIndex < rCommand.size());

    return std::make_tuple(sType, arguments, switches);
}

static OUString lcl_ExtractVariableAndHint( std::u16string_view rCommand, OUString& rHint )
{
    // the first word after "ASK " is the variable
    // the text after the variable and before a '\' is the hint
    // if no hint is set the variable is used as hint
    // the quotes of the hint have to be removed
    size_t nIndex = rCommand.find( ' ', 2); //find last space after 'ASK'
    if (nIndex == std::u16string_view::npos)
        return OUString();
    while (nIndex < rCommand.size() && rCommand[nIndex] == ' ')
        ++nIndex;
    std::u16string_view sShortCommand( rCommand.substr( nIndex ) ); //cut off the " ASK "

    sShortCommand = o3tl::getToken(sShortCommand, 0, '\\');
    sal_Int32 nIndex2 = 0;
    std::u16string_view sRet = o3tl::getToken(sShortCommand, 0, ' ', nIndex2);
    if( nIndex2 > 0)
        rHint = sShortCommand.substr( nIndex2 );
    if( rHint.isEmpty() )
        rHint = sRet;
    return OUString(sRet);
}

static size_t nextCode(std::u16string_view rCommand, size_t pos)
{
    bool inQuotes = false;
    for (; pos < rCommand.size(); ++pos)
    {
        switch (rCommand[pos])
        {
        case '"':
            inQuotes = !inQuotes;
            break;
        case '\\':
            ++pos;
            if (!inQuotes)
                return pos;
            break;
        }
    }
    return std::u16string_view::npos;
}

// Returns the position of the field code
static size_t findCode(std::u16string_view rCommand, sal_Unicode cSwitch)
{
    for (size_t i = nextCode(rCommand, 0); i < rCommand.size(); i = nextCode(rCommand, i))
        if (rCommand[i] == cSwitch)
            return i;

    return std::u16string_view::npos;
}

static bool lcl_FindInCommand(
    std::u16string_view rCommand,
    sal_Unicode cSwitch,
    OUString& rValue )
{
    if (size_t i = findCode(rCommand, cSwitch); i < rCommand.size())
    {
        ++i;
        size_t next = nextCode(rCommand, i);
        if (next < rCommand.size())
            --next; // get back before the next '\\'
        rValue = o3tl::trim(rCommand.substr(i, next - i));
        return true;
    }

    return false;
}

static OUString lcl_trim(std::u16string_view sValue)
{
    // it seems, all kind of quotation marks are allowed around index type identifiers
    // TODO apply this on bookmarks, too, if needed
    return OUString(o3tl::trim(sValue)).replaceAll("\"","").replaceAll(u"“", "").replaceAll(u"”", "");
}

/*-------------------------------------------------------------------------
    extract the number format from the command and apply the resulting number
    format to the XPropertySet
  -----------------------------------------------------------------------*/
void DomainMapper_Impl::SetNumberFormat( const OUString& rCommand,
        uno::Reference< beans::XPropertySet > const& xPropertySet,
        bool const bDetectFormat)
{
    OUString sFormatString = lcl_ParseFormat( rCommand );
    // find \h - hijri/luna calendar todo: what about saka/era calendar?
    bool bHijri = 0 < rCommand.indexOf("\\h ");
    lang::Locale aUSLocale;
    aUSLocale.Language = "en";
    aUSLocale.Country = "US";

    lang::Locale aCurrentLocale;
    GetAnyProperty(PROP_CHAR_LOCALE, GetTopContext()) >>= aCurrentLocale;

    if (sFormatString.isEmpty())
    {
        // No format specified. MS Word uses different formats depending on w:lang,
        // "M/d/yyyy h:mm:ss AM/PM" for en-US, and "dd/MM/yyyy hh:mm:ss AM/PM" for en-GB.
        // ALSO SEE: ww8par5's GetWordDefaultDateStringAsUS.
        sal_Int32 nPos = rCommand.indexOf(" \\");
        OUString sCommand = nPos == -1 ? rCommand.trim()
                                       : OUString(o3tl::trim(rCommand.subView(0, nPos)));
        if (sCommand == "CREATEDATE" || sCommand == "PRINTDATE" || sCommand == "SAVEDATE")
        {
            try
            {
                css::uno::Reference<css::i18n::XNumberFormatCode> const& xNumberFormatCode =
                    i18n::NumberFormatMapper::create(m_xComponentContext);
                sFormatString = xNumberFormatCode->getFormatCode(
                    css::i18n::NumberFormatIndex::DATE_SYSTEM_SHORT, aCurrentLocale).Code;
                nPos = sFormatString.indexOf("YYYY");
                if (nPos == -1)
                    sFormatString = sFormatString.replaceFirst("YY", "YYYY");
                if (aCurrentLocale == aUSLocale)
                    sFormatString += " h:mm:ss AM/PM";
                else
                    sFormatString += " hh:mm:ss AM/PM";
            }
            catch(const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("writerfilter.dmapper");
            }
        }
    }
    OUString sFormat = ConversionHelper::ConvertMSFormatStringToSO( sFormatString, aCurrentLocale, bHijri);
    //get the number formatter and convert the string to a format value
    try
    {
        sal_Int32 nKey = 0;
        uno::Reference< util::XNumberFormatsSupplier > xNumberSupplier( static_cast<cppu::OWeakObject*>(m_xTextDocument.get()), uno::UNO_QUERY_THROW );
        if( bDetectFormat )
        {
            uno::Reference< util::XNumberFormatter> xFormatter(util::NumberFormatter::create(m_xComponentContext), uno::UNO_QUERY_THROW);
            xFormatter->attachNumberFormatsSupplier( xNumberSupplier );
            nKey = xFormatter->detectNumberFormat( 0, rCommand );
        }
        else
        {
            nKey = xNumberSupplier->getNumberFormats()->addNewConverted( sFormat, aUSLocale, aCurrentLocale );
        }
        xPropertySet->setPropertyValue(
            getPropertyName(PROP_NUMBER_FORMAT),
            uno::Any( nKey ));
    }
    catch(const uno::Exception&)
    {
    }
}

static uno::Any lcl_getGrabBagValue( const uno::Sequence<beans::PropertyValue>& grabBag, OUString const & name )
{
    auto pProp = std::find_if(grabBag.begin(), grabBag.end(),
        [&name](const beans::PropertyValue& rProp) { return rProp.Name == name; });
    if (pProp != grabBag.end())
        return pProp->Value;
    return uno::Any();
}

//Link the text frames.
void DomainMapper_Impl::ChainTextFrames()
{
    //can't link textboxes if there are not even two of them...
    if( 2 > m_vTextFramesForChaining.size() )
        return ;

    struct TextFramesForChaining {
        css::uno::Reference< css::drawing::XShape > xShape;
        sal_Int32 nId;
        sal_Int32 nSeq;
        OUString s_mso_next_textbox;
        OUString shapeName;
        TextFramesForChaining() : nId(0), nSeq(0) {}
    } ;
    typedef std::map <OUString, TextFramesForChaining> ChainMap;

    try
    {
        ChainMap aTextFramesForChainingHelper;
        ::std::vector<TextFramesForChaining> chainingWPS;
        OUString sChainNextName("ChainNextName");

        //learn about ALL of the textboxes and their chaining values first - because frames are processed in no specific order.
        for( const auto& rTextFrame : m_vTextFramesForChaining )
        {
            uno::Reference<text::XTextContent>  xTextContent(rTextFrame, uno::UNO_QUERY_THROW);
            uno::Reference<beans::XPropertySet> xPropertySet(xTextContent, uno::UNO_QUERY);
            uno::Reference<beans::XPropertySetInfo> xPropertySetInfo;
            if( xPropertySet.is() )
                xPropertySetInfo = xPropertySet->getPropertySetInfo();
            uno::Sequence<beans::PropertyValue> aGrabBag;
            uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);

            TextFramesForChaining aChainStruct;
            OUString sShapeName;
            OUString sLinkChainName;

            //The chaining name and the shape name CAN be different in .docx.
            //MUST use LinkDisplayName/ChainName as the shape name for establishing links.
            if ( xServiceInfo->supportsService("com.sun.star.text.TextFrame") )
            {
                xPropertySet->getPropertyValue("FrameInteropGrabBag") >>= aGrabBag;
                xPropertySet->getPropertyValue("LinkDisplayName") >>= sShapeName;
            }
            else
            {
                xPropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
                xPropertySet->getPropertyValue("ChainName") >>= sShapeName;
            }

            lcl_getGrabBagValue( aGrabBag, "Txbx-Id")  >>= aChainStruct.nId;
            lcl_getGrabBagValue( aGrabBag, "Txbx-Seq") >>= aChainStruct.nSeq;
            lcl_getGrabBagValue( aGrabBag, "LinkChainName") >>= sLinkChainName;
            lcl_getGrabBagValue( aGrabBag, "mso-next-textbox") >>= aChainStruct.s_mso_next_textbox;

            //Sometimes the shape names have not been imported.  If not, we may have a fallback name.
            //Set name later, only if required for linking.
            aChainStruct.shapeName = sShapeName;

            if (!sLinkChainName.isEmpty())
            {
                aChainStruct.xShape = rTextFrame;
                aTextFramesForChainingHelper[sLinkChainName] = aChainStruct;
            }
            if (aChainStruct.s_mso_next_textbox.isEmpty())
            {   // no VML chaining => try to chain DrawingML via IDs
                aChainStruct.xShape = rTextFrame;
                if (!sLinkChainName.isEmpty())
                {   // for member of group shapes, TestTdf73499
                    aChainStruct.shapeName = sLinkChainName;
                }
                chainingWPS.emplace_back(aChainStruct);
            }
        }

        //if mso-next-textbox tags are provided, create those vml-style links first. Afterwards we will make dml-style id/seq links.
        for (auto& msoItem : aTextFramesForChainingHelper)
        {
            //if no mso-next-textbox, we are done.
            //if it points to itself, we are done.
            if( !msoItem.second.s_mso_next_textbox.isEmpty()
                && msoItem.second.s_mso_next_textbox != msoItem.first )
            {
                ChainMap::iterator nextFinder=aTextFramesForChainingHelper.find(msoItem.second.s_mso_next_textbox);
                if( nextFinder != aTextFramesForChainingHelper.end() )
                {
                    //if the frames have no name yet, then set them.  LinkDisplayName / ChainName are read-only.
                    if (msoItem.second.shapeName.isEmpty())
                    {
                        uno::Reference< container::XNamed > xNamed( msoItem.second.xShape, uno::UNO_QUERY );
                        if ( xNamed.is() )
                        {
                            xNamed->setName( msoItem.first );
                            msoItem.second.shapeName = msoItem.first;
                        }
                    }
                    if (nextFinder->second.shapeName.isEmpty())
                    {
                        uno::Reference< container::XNamed > xNamed( nextFinder->second.xShape, uno::UNO_QUERY );
                        if ( xNamed.is() )
                        {
                            xNamed->setName( nextFinder->first );
                            nextFinder->second.shapeName = msoItem.first;
                        }
                    }

                    uno::Reference<text::XTextContent>  xTextContent(msoItem.second.xShape, uno::UNO_QUERY_THROW);
                    uno::Reference<beans::XPropertySet> xPropertySet(xTextContent, uno::UNO_QUERY);

                    //The reverse chaining happens automatically, so only one direction needs to be set
                    xPropertySet->setPropertyValue(sChainNextName, uno::Any(nextFinder->second.shapeName));

                    //the last item in an mso-next-textbox chain is indistinguishable from id/seq items.  Now that it is handled, remove it.
                    if( nextFinder->second.s_mso_next_textbox.isEmpty() )
                        aTextFramesForChainingHelper.erase(nextFinder->first);
                }
            }
        }

        //TODO: Perhaps allow reverse sequences when mso-layout-flow-alt = "bottom-to-top"
        const sal_Int32 nDirection = 1;

        //Finally - go through and attach the chains based on matching ID and incremented sequence number (dml-style).
        for (const auto& rOuter : chainingWPS)
        {
                for (const auto& rInner : chainingWPS)
                {
                    if (rInner.nId == rOuter.nId)
                    {
                        if (rInner.nSeq == (rOuter.nSeq + nDirection))
                        {
                            uno::Reference<text::XTextContent> const xTextContent(rOuter.xShape, uno::UNO_QUERY_THROW);
                            uno::Reference<beans::XPropertySet> xPropertySet(xTextContent, uno::UNO_QUERY);

                            //The reverse chaining happens automatically, so only one direction needs to be set
                            xPropertySet->setPropertyValue(sChainNextName, uno::Any(rInner.shapeName));
                            break ; //there cannot be more than one next frame
                        }
                    }
                }
        }
        m_vTextFramesForChaining.clear(); //clear the vector
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("writerfilter.dmapper");
    }
}

void DomainMapper_Impl::PushTextBoxContent()
{
    if (m_StreamStateStack.top().bIsInTextBox)
        return;

    try
    {
        uno::Reference<text::XTextFrame> xTBoxFrame(
            m_xTextDocument->createInstance("com.sun.star.text.TextFrame"), uno::UNO_QUERY_THROW);
        uno::Reference<container::XNamed>(xTBoxFrame, uno::UNO_QUERY_THROW)
            ->setName("textbox" + OUString::number(m_xPendingTextBoxFrames.size() + 1));
        uno::Reference<text::XTextAppendAndConvert>(m_aTextAppendStack.top().xTextAppend,
            uno::UNO_QUERY_THROW)
            ->appendTextContent(xTBoxFrame, beans::PropertyValues());
        m_xPendingTextBoxFrames.push(xTBoxFrame);

        m_aTextAppendStack.push(TextAppendContext(uno::Reference<text::XTextAppend>(xTBoxFrame, uno::UNO_QUERY_THROW), {}));
        m_StreamStateStack.top().bIsInTextBox = true;

        appendTableManager();
        appendTableHandler();
        getTableManager().startLevel();
    }
    catch (uno::Exception& e)
    {
        SAL_WARN("writerfilter.dmapper", "Exception during creating textbox (" + e.Message + ")!");
    }
}

void DomainMapper_Impl::PopTextBoxContent()
{
    if (!m_StreamStateStack.top().bIsInTextBox || m_xPendingTextBoxFrames.empty())
        return;

    if (uno::Reference<text::XTextFrame>(m_aTextAppendStack.top().xTextAppend, uno::UNO_QUERY).is())
    {
        if (hasTableManager())
        {
            getTableManager().endLevel();
            popTableManager();
        }
        RemoveLastParagraph();

        m_aTextAppendStack.pop();
        m_StreamStateStack.top().bIsInTextBox = false;
    }
}

void DomainMapper_Impl::AttachTextBoxContentToShape(css::uno::Reference<css::drawing::XShape> xShape)
{
    // Without textbox or shape pointless to continue
    if (m_xPendingTextBoxFrames.empty() || !xShape)
        return;

    uno::Reference< drawing::XShapes >xGroup(xShape, uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet >xProps(xShape, uno::UNO_QUERY);

    // If this is a group go inside
    if (xGroup)
        for (sal_Int32 i = 0; i < xGroup->getCount(); ++i)
            AttachTextBoxContentToShape(
                uno::Reference<drawing::XShape>(xGroup->getByIndex(i), uno::UNO_QUERY));

    // if this shape has to be a textbox, attach the frame
    if (!xProps->getPropertyValue("TextBox").get<bool>())
        return;

    // if this is a textbox there must be a waiting frame
    auto xTextBox = m_xPendingTextBoxFrames.front();
    if (!xTextBox)
        return;

    // Pop the pending frames
    m_xPendingTextBoxFrames.pop();

    // Attach the textbox to the shape
    try
    {
        xProps->setPropertyValue("TextBoxContent", uno::Any(xTextBox));
    }
    catch (...)
    {
        SAL_WARN("writerfilter.dmapper", "Exception while trying to attach textboxes!");
        return;
    }

    // If attaching is successful, then do the linking
    try
    {
        // Get the name of the textbox
        OUString sTextBoxName;
        uno::Reference<container::XNamed> xName(xTextBox, uno::UNO_QUERY);
        if (xName && !xName->getName().isEmpty())
            sTextBoxName = xName->getName();

        // Try to get the grabbag
        uno::Sequence<beans::PropertyValue> aOldGrabBagSeq;
        if (xProps->getPropertySetInfo()->hasPropertyByName("InteropGrabBag"))
            xProps->getPropertyValue("InteropGrabBag") >>= aOldGrabBagSeq;

        // If the grabbag successfully get...
        if (!aOldGrabBagSeq.hasElements())
            return;

        // Check for the existing linking information
        bool bSuccess = false;
        beans::PropertyValues aNewGrabBagSeq;
        const auto& aHasLink = lcl_getGrabBagValue(aOldGrabBagSeq, "TxbxHasLink");

        // If there must be a link, do it
        if (aHasLink.hasValue() && aHasLink.get<bool>())
        {
            auto aLinkProp = comphelper::makePropertyValue("LinkChainName", sTextBoxName);
            for (sal_uInt32 i = 0; i < aOldGrabBagSeq.size(); ++i)
            {
                aNewGrabBagSeq.realloc(i + 1);
                // If this is the link name replace it
                if (!aOldGrabBagSeq[i].Name.isEmpty() && !aLinkProp.Name.isEmpty()
                    && (aOldGrabBagSeq[i].Name == aLinkProp.Name))
                {
                    aNewGrabBagSeq.getArray()[i] = aLinkProp;
                    bSuccess = true;
                }
                // else copy
                else
                    aNewGrabBagSeq.getArray()[i] = aOldGrabBagSeq[i];
            }

            // If there was no replacement, append the linking data
            if (!bSuccess)
            {
                aNewGrabBagSeq.realloc(aNewGrabBagSeq.size() + 1);
                aNewGrabBagSeq.getArray()[aNewGrabBagSeq.size() - 1] = aLinkProp;
                bSuccess = true;
            }
        }

        // If the linking changed the grabbag, apply the modifications
        if (aNewGrabBagSeq.hasElements() && bSuccess)
        {
            xProps->setPropertyValue("InteropGrabBag", uno::Any(aNewGrabBagSeq));
            m_vTextFramesForChaining.push_back(xShape);
        }
    }
    catch (...)
    {
        SAL_WARN("writerfilter.dmapper", "Exception while trying to link textboxes!");
    }
}

uno::Reference<beans::XPropertySet> DomainMapper_Impl::FindOrCreateFieldMaster(const char* pFieldMasterService, const OUString& rFieldMasterName)
{
    // query master, create if not available
    if (!m_xTextDocument)
        throw uno::RuntimeException();
    uno::Reference< container::XNameAccess > xFieldMasterAccess = m_xTextDocument->getTextFieldMasters();
    uno::Reference< beans::XPropertySet > xMaster;
    OUString sFieldMasterService( OUString::createFromAscii(pFieldMasterService) );
    OUStringBuffer aFieldMasterName;
    OUString sDatabaseDataSourceName = GetSettingsTable()->GetCurrentDatabaseDataSource();
    bool bIsMergeField = sFieldMasterService.endsWith("Database");
    aFieldMasterName.appendAscii( pFieldMasterService );
    aFieldMasterName.append('.');
    if ( bIsMergeField && !sDatabaseDataSourceName.isEmpty() )
    {
        aFieldMasterName.append(sDatabaseDataSourceName + ".");
    }
    aFieldMasterName.append(rFieldMasterName);
    OUString sFieldMasterName = aFieldMasterName.makeStringAndClear();
    if(xFieldMasterAccess->hasByName(sFieldMasterName))
    {
        //get the master
        xMaster.set(xFieldMasterAccess->getByName(sFieldMasterName), uno::UNO_QUERY_THROW);
    }
    else if( m_xTextDocument )
    {
        //create the master
        xMaster.set( m_xTextDocument->createInstance(sFieldMasterService), uno::UNO_QUERY_THROW);
        if ( !bIsMergeField || sDatabaseDataSourceName.isEmpty() )
        {
            //set the master's name
            xMaster->setPropertyValue(
                    getPropertyName(PROP_NAME),
                    uno::Any(rFieldMasterName));
        } else {
           // set database data, based on the "databasename.tablename" of sDatabaseDataSourceName
           xMaster->setPropertyValue(
                    getPropertyName(PROP_DATABASE_NAME),
                    uno::Any(sDatabaseDataSourceName.copy(0, sDatabaseDataSourceName.indexOf('.'))));
           xMaster->setPropertyValue(
                    getPropertyName(PROP_COMMAND_TYPE),
                    uno::Any(sal_Int32(0)));
           xMaster->setPropertyValue(
                    getPropertyName(PROP_DATATABLE_NAME),
                    uno::Any(sDatabaseDataSourceName.copy(sDatabaseDataSourceName.indexOf('.') + 1)));
           xMaster->setPropertyValue(
                    getPropertyName(PROP_DATACOLUMN_NAME),
                    uno::Any(rFieldMasterName));
        }
    }
    return xMaster;
}

void DomainMapper_Impl::PushFieldContext()
{
    m_StreamStateStack.top().bParaHadField = true;
    if(m_bDiscardHeaderFooter)
        return;
#ifdef DBG_UTIL
    TagLogger::getInstance().element("pushFieldContext");
#endif

    uno::Reference<text::XTextCursor> xCrsr;
    if (!m_aTextAppendStack.empty())
    {
        uno::Reference<text::XTextAppend> xTextAppend = m_aTextAppendStack.top().xTextAppend;
        if (xTextAppend.is())
            xCrsr = xTextAppend->createTextCursorByRange(
                        m_aTextAppendStack.top().xInsertPosition.is()
                            ? m_aTextAppendStack.top().xInsertPosition
                            : xTextAppend->getEnd());
    }

    uno::Reference< text::XTextRange > xStart;
    if (xCrsr.is())
        xStart = xCrsr->getStart();
    m_aFieldStack.push_back(new FieldContext(xStart));
}
/*-------------------------------------------------------------------------
//the current field context waits for the completion of the command
  -----------------------------------------------------------------------*/
bool DomainMapper_Impl::IsOpenFieldCommand() const
{
    return !m_aFieldStack.empty() && !m_aFieldStack.back()->IsCommandCompleted();
}
/*-------------------------------------------------------------------------
//the current field context waits for the completion of the command
  -----------------------------------------------------------------------*/
bool DomainMapper_Impl::IsOpenField() const
{
    return !m_aFieldStack.empty();
}

// Mark top field context as containing a fixed field
void DomainMapper_Impl::SetFieldLocked()
{
    if (IsOpenField())
        m_aFieldStack.back()->SetFieldLocked();
}


FieldContext::FieldContext(uno::Reference< text::XTextRange > xStart)
    : m_bFieldCommandCompleted(false)
    , m_xStartRange(std::move( xStart ))
    , m_bFieldLocked( false )
    , m_bCommandType(false)
{
    m_pProperties = new PropertyMap();
}


FieldContext::~FieldContext()
{
}

void FieldContext::SetTextField(uno::Reference<text::XTextField> const& xTextField)
{
#ifndef NDEBUG
    if (xTextField.is())
    {
        uno::Reference<lang::XServiceInfo> const xServiceInfo(xTextField, uno::UNO_QUERY);
        assert(xServiceInfo.is());
        // those must be set by SetFormField()
        assert(!xServiceInfo->supportsService("com.sun.star.text.Fieldmark")
            && !xServiceInfo->supportsService("com.sun.star.text.FormFieldmark"));
    }
#endif
    m_xTextField = xTextField;
}

void FieldContext::CacheVariableValue(const uno::Any& rAny)
{
        rAny >>= m_sVariableValue;
}

void FieldContext::AppendCommand(std::u16string_view rPart)
{
    m_sCommand[m_bCommandType] += rPart;
}

::std::vector<OUString> FieldContext::GetCommandParts() const
{
    ::std::vector<OUString> aResult;
    sal_Int32 nIndex = 0;
    bool bInString = false;
    OUString sPart;
    while (nIndex != -1)
    {
        OUString sToken = GetCommand().getToken(0, ' ', nIndex);
        bool bInStringNext = bInString;

        if (sToken.isEmpty())
            continue;

        if (sToken[0] == '"')
        {
            bInStringNext = true;
            sToken = sToken.copy(1);
        }
        if (sToken.endsWith("\""))
        {
            bInStringNext = false;
            sToken = sToken.copy(0, sToken.getLength() - 1);
        }

        if (bInString)
        {
            sPart += " " + sToken;
            if (!bInStringNext)
            {
                aResult.push_back(sPart);
            }
        }
        else
        {
            if (bInStringNext)
            {
                sPart = sToken;
            }
            else
            {
                aResult.push_back(sToken);
            }
        }

        bInString = bInStringNext;
    }

    return aResult;
}

/*-------------------------------------------------------------------------
//collect the pieces of the command
  -----------------------------------------------------------------------*/
void DomainMapper_Impl::AppendFieldCommand(OUString const & rPartOfCommand)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("appendFieldCommand");
    TagLogger::getInstance().chars(rPartOfCommand);
    TagLogger::getInstance().endElement();
#endif

    FieldContextPtr pContext = m_aFieldStack.back();
    OSL_ENSURE( pContext, "no field context available");
    if( pContext )
    {
        // Set command line type: normal or deleted
        pContext->SetCommandType(m_bTextDeleted);
        pContext->AppendCommand( rPartOfCommand );
    }
}


typedef std::multimap < sal_Int32, OUString > TOCStyleMap;


static ww::eField GetWW8FieldId(OUString const& rType)
{
    std::unordered_map<OUString, ww::eField> mapID
    {
        {"ADDRESSBLOCK",    ww::eADDRESSBLOCK},
        {"ADVANCE",         ww::eADVANCE},
        {"ASK",             ww::eASK},
        {"AUTONUM",         ww::eAUTONUM},
        {"AUTONUMLGL",      ww::eAUTONUMLGL},
        {"AUTONUMOUT",      ww::eAUTONUMOUT},
        {"AUTOTEXT",        ww::eAUTOTEXT},
        {"AUTOTEXTLIST",    ww::eAUTOTEXTLIST},
        {"AUTHOR",          ww::eAUTHOR},
        {"BARCODE",         ww::eBARCODE},
        {"BIDIOUTLINE",     ww::eBIDIOUTLINE},
        {"DATE",            ww::eDATE},
        {"COMMENTS",        ww::eCOMMENTS},
        {"COMPARE",         ww::eCOMPARE},
        {"CONTROL",         ww::eCONTROL},
        {"CREATEDATE",      ww::eCREATEDATE},
        {"DATABASE",        ww::eDATABASE},
        {"DDEAUTOREF",      ww::eDDEAUTOREF},
        {"DDEREF",          ww::eDDEREF},
        {"DOCPROPERTY",     ww::eDOCPROPERTY},
        {"DOCVARIABLE",     ww::eDOCVARIABLE},
        {"EDITTIME",        ww::eEDITTIME},
        {"EMBED",           ww::eEMBED},
        {"EQ",              ww::eEQ},
        {"FILLIN",          ww::eFILLIN},
        {"FILENAME",        ww::eFILENAME},
        {"FILESIZE",        ww::eFILESIZE},
        {"FOOTREF",         ww::eFOOTREF},
//        {"FORMULA",         ww::},
        {"FORMCHECKBOX",    ww::eFORMCHECKBOX},
        {"FORMDROPDOWN",    ww::eFORMDROPDOWN},
        {"FORMTEXT",        ww::eFORMTEXT},
        {"GLOSSREF",        ww::eGLOSSREF},
        {"GOTOBUTTON",      ww::eGOTOBUTTON},
        {"GREETINGLINE",    ww::eGREETINGLINE},
        {"HTMLCONTROL",     ww::eHTMLCONTROL},
        {"HYPERLINK",       ww::eHYPERLINK},
        {"IF",              ww::eIF},
        {"INFO",            ww::eINFO},
        {"INCLUDEPICTURE",  ww::eINCLUDEPICTURE},
        {"INCLUDETEXT",     ww::eINCLUDETEXT},
        {"INCLUDETIFF",     ww::eINCLUDETIFF},
        {"KEYWORDS",        ww::eKEYWORDS},
        {"LASTSAVEDBY",     ww::eLASTSAVEDBY},
        {"LINK",            ww::eLINK},
        {"LISTNUM",         ww::eLISTNUM},
        {"MACRO",           ww::eMACRO},
        {"MACROBUTTON",     ww::eMACROBUTTON},
        {"MERGEDATA",       ww::eMERGEDATA},
        {"MERGEFIELD",      ww::eMERGEFIELD},
        {"MERGEINC",        ww::eMERGEINC},
        {"MERGEREC",        ww::eMERGEREC},
        {"MERGESEQ",        ww::eMERGESEQ},
        {"NEXT",            ww::eNEXT},
        {"NEXTIF",          ww::eNEXTIF},
        {"NOTEREF",         ww::eNOTEREF},
        {"PAGE",            ww::ePAGE},
        {"PAGEREF",         ww::ePAGEREF},
        {"PLUGIN",          ww::ePLUGIN},
        {"PRINT",           ww::ePRINT},
        {"PRINTDATE",       ww::ePRINTDATE},
        {"PRIVATE",         ww::ePRIVATE},
        {"QUOTE",           ww::eQUOTE},
        {"RD",              ww::eRD},
        {"REF",             ww::eREF},
        {"REVNUM",          ww::eREVNUM},
        {"SAVEDATE",        ww::eSAVEDATE},
        {"SECTION",         ww::eSECTION},
        {"SECTIONPAGES",    ww::eSECTIONPAGES},
        {"SEQ",             ww::eSEQ},
        {"SET",             ww::eSET},
        {"SKIPIF",          ww::eSKIPIF},
        {"STYLEREF",        ww::eSTYLEREF},
        {"SUBSCRIBER",      ww::eSUBSCRIBER},
        {"SUBJECT",         ww::eSUBJECT},
        {"SYMBOL",          ww::eSYMBOL},
        {"TA",              ww::eTA},
        {"TEMPLATE",        ww::eTEMPLATE},
        {"TIME",            ww::eTIME},
        {"TITLE",           ww::eTITLE},
        {"TOA",             ww::eTOA},
        {"USERINITIALS",    ww::eUSERINITIALS},
        {"USERADDRESS",     ww::eUSERADDRESS},
        {"USERNAME",        ww::eUSERNAME},

        {"TOC",             ww::eTOC},
        {"TC",              ww::eTC},
        {"NUMCHARS",        ww::eNUMCHARS},
        {"NUMWORDS",        ww::eNUMWORDS},
        {"NUMPAGES",        ww::eNUMPAGES},
        {"INDEX",           ww::eINDEX},
        {"XE",              ww::eXE},
        {"BIBLIOGRAPHY",    ww::eBIBLIOGRAPHY},
        {"CITATION",        ww::eCITATION},
    };
    auto const it = mapID.find(rType);
    return (it == mapID.end()) ? ww::eNONE : it->second;
}

static const FieldConversionMap_t & lcl_GetFieldConversion()
{
    static const FieldConversionMap_t aFieldConversionMap
    {
//      {"ADDRESSBLOCK",    {"",                        FIELD_ADDRESSBLOCK  }},
//      {"ADVANCE",         {"",                        FIELD_ADVANCE       }},
        {"ASK",             {"SetExpression",           FIELD_ASK           }},
        {"AUTONUM",         {"SetExpression",           FIELD_AUTONUM       }},
        {"AUTONUMLGL",      {"SetExpression",           FIELD_AUTONUMLGL    }},
        {"AUTONUMOUT",      {"SetExpression",           FIELD_AUTONUMOUT    }},
        {"AUTHOR",          {"DocInfo.CreateAuthor",    FIELD_AUTHOR        }},
        {"DATE",            {"DateTime",                FIELD_DATE          }},
        {"COMMENTS",        {"DocInfo.Description",     FIELD_COMMENTS      }},
        {"CREATEDATE",      {"DocInfo.CreateDateTime",  FIELD_CREATEDATE    }},
        {"DOCPROPERTY",     {"",                        FIELD_DOCPROPERTY   }},
        {"DOCVARIABLE",     {"User",                    FIELD_DOCVARIABLE   }},
        {"EDITTIME",        {"DocInfo.EditTime",        FIELD_EDITTIME      }},
        {"EQ",              {"",                        FIELD_EQ            }},
        {"FILLIN",          {"Input",                   FIELD_FILLIN        }},
        {"FILENAME",        {"FileName",                FIELD_FILENAME      }},
//      {"FILESIZE",        {"",                        FIELD_FILESIZE      }},
        {"FORMULA",         {"TableFormula",            FIELD_FORMULA       }},
        {"FORMCHECKBOX",    {"",                        FIELD_FORMCHECKBOX  }},
        {"FORMDROPDOWN",    {"DropDown",                FIELD_FORMDROPDOWN  }},
        {"FORMTEXT",        {"Input",                   FIELD_FORMTEXT      }},
        {"GOTOBUTTON",      {"",                        FIELD_GOTOBUTTON    }},
        {"HYPERLINK",       {"",                        FIELD_HYPERLINK     }},
        {"IF",              {"ConditionalText",         FIELD_IF            }},
//      {"INFO",            {"",                        FIELD_INFO          }},
        {"INCLUDEPICTURE",  {"",                        FIELD_INCLUDEPICTURE}},
        {"KEYWORDS",        {"DocInfo.KeyWords",        FIELD_KEYWORDS      }},
        {"LASTSAVEDBY",     {"DocInfo.ChangeAuthor",    FIELD_LASTSAVEDBY   }},
        {"MACROBUTTON",     {"Macro",                   FIELD_MACROBUTTON   }},
        {"MERGEFIELD",      {"Database",                FIELD_MERGEFIELD    }},
        {"MERGEREC",        {"DatabaseNumberOfSet",     FIELD_MERGEREC      }},
//      {"MERGESEQ",        {"",                        FIELD_MERGESEQ      }},
        {"NEXT",            {"DatabaseNextSet",         FIELD_NEXT          }},
        {"NEXTIF",          {"DatabaseNextSet",         FIELD_NEXTIF        }},
        {"PAGE",            {"PageNumber",              FIELD_PAGE          }},
        {"PAGEREF",         {"GetReference",            FIELD_PAGEREF       }},
        {"PRINTDATE",       {"DocInfo.PrintDateTime",   FIELD_PRINTDATE     }},
        {"REF",             {"GetReference",            FIELD_REF           }},
        {"REVNUM",          {"DocInfo.Revision",        FIELD_REVNUM        }},
        {"SAVEDATE",        {"DocInfo.ChangeDateTime",  FIELD_SAVEDATE      }},
//      {"SECTION",         {"",                        FIELD_SECTION       }},
//      {"SECTIONPAGES",    {"",                        FIELD_SECTIONPAGES  }},
        {"SEQ",             {"SetExpression",           FIELD_SEQ           }},
        {"SET",             {"SetExpression",           FIELD_SET           }},
//      {"SKIPIF",          {"",                        FIELD_SKIPIF        }},
        {"STYLEREF",        {"GetReference",            FIELD_STYLEREF      }},
        {"SUBJECT",         {"DocInfo.Subject",         FIELD_SUBJECT       }},
        {"SYMBOL",          {"",                        FIELD_SYMBOL        }},
        {"TEMPLATE",        {"TemplateName",            FIELD_TEMPLATE      }},
        {"TIME",            {"DateTime",                FIELD_TIME          }},
        {"TITLE",           {"DocInfo.Title",           FIELD_TITLE         }},
        {"USERINITIALS",    {"Author",                  FIELD_USERINITIALS  }},
//      {"USERADDRESS",     {"",                        FIELD_USERADDRESS   }},
        {"USERNAME",        {"Author",                  FIELD_USERNAME      }},


        {"TOC",         {"com.sun.star.text.ContentIndex",          FIELD_TOC           }},
        {"TC",          {"com.sun.star.text.ContentIndexMark",      FIELD_TC            }},
        {"NUMCHARS",    {"CharacterCount",                          FIELD_NUMCHARS      }},
        {"NUMWORDS",    {"WordCount",                               FIELD_NUMWORDS      }},
        {"NUMPAGES",    {"PageCount",                               FIELD_NUMPAGES      }},
        {"INDEX",       {"com.sun.star.text.DocumentIndex",         FIELD_INDEX         }},
        {"XE",          {"com.sun.star.text.DocumentIndexMark",     FIELD_XE            }},
        {"BIBLIOGRAPHY",{"com.sun.star.text.Bibliography",          FIELD_BIBLIOGRAPHY  }},
        {"CITATION",    {"com.sun.star.text.TextField.Bibliography",FIELD_CITATION      }},
    };

    return aFieldConversionMap;
}

static const FieldConversionMap_t & lcl_GetEnhancedFieldConversion()
{
    static const FieldConversionMap_t aEnhancedFieldConversionMap =
    {
        {"FORMCHECKBOX", {"FormFieldmark", FIELD_FORMCHECKBOX}},
        {"FORMDROPDOWN", {"FormFieldmark", FIELD_FORMDROPDOWN}},
        {"FORMTEXT",     {"Fieldmark",     FIELD_FORMTEXT}},
    };

    return aEnhancedFieldConversionMap;
}

void DomainMapper_Impl::handleFieldSet
    (const FieldContextPtr& pContext,
     uno::Reference< uno::XInterface > const & xFieldInterface,
     uno::Reference< beans::XPropertySet > const& xFieldProperties)
{
    OUString sVariable, sHint;

    sVariable = lcl_ExtractVariableAndHint(pContext->GetCommand(), sHint);

    // remove surrounding "" if exists
    if(sHint.getLength() >= 2)
    {
        std::u16string_view sTmp = o3tl::trim(sHint);
        if (o3tl::starts_with(sTmp, u"\"") && o3tl::ends_with(sTmp, u"\""))
        {
            sHint = sTmp.substr(1, sTmp.size() - 2);
        }
    }

    // determine field master name
    uno::Reference< beans::XPropertySet > xMaster =
        FindOrCreateFieldMaster
        ("com.sun.star.text.FieldMaster.SetExpression", sVariable);

    // a set field is a string
    xMaster->setPropertyValue(getPropertyName(PROP_SUB_TYPE), uno::Any(text::SetVariableType::STRING));

    // attach the master to the field
    uno::Reference< text::XDependentTextField > xDependentField
        ( xFieldInterface, uno::UNO_QUERY_THROW );
    xDependentField->attachTextFieldMaster( xMaster );

    uno::Any aAnyHint(sHint);
    xFieldProperties->setPropertyValue(getPropertyName(PROP_HINT), aAnyHint);
    xFieldProperties->setPropertyValue(getPropertyName(PROP_CONTENT), aAnyHint);
    xFieldProperties->setPropertyValue(getPropertyName(PROP_SUB_TYPE), uno::Any(text::SetVariableType::STRING));

    // Mimic MS Word behavior (hide the SET)
    xFieldProperties->setPropertyValue(getPropertyName(PROP_IS_VISIBLE), uno::Any(false));
}

void DomainMapper_Impl::handleFieldAsk
    (const FieldContextPtr& pContext,
     uno::Reference< uno::XInterface > & xFieldInterface,
     uno::Reference< beans::XPropertySet > const& xFieldProperties)
{
    //doesn the command contain a variable name?
    OUString sVariable, sHint;

    sVariable = lcl_ExtractVariableAndHint( pContext->GetCommand(),
        sHint );
    if(!sVariable.isEmpty())
    {
        // determine field master name
        uno::Reference< beans::XPropertySet > xMaster =
            FindOrCreateFieldMaster
            ("com.sun.star.text.FieldMaster.SetExpression", sVariable );
        // An ASK field is always a string of characters
        xMaster->setPropertyValue(getPropertyName(PROP_SUB_TYPE), uno::Any(text::SetVariableType::STRING));

        // attach the master to the field
        uno::Reference< text::XDependentTextField > xDependentField
            ( xFieldInterface, uno::UNO_QUERY_THROW );
        xDependentField->attachTextFieldMaster( xMaster );

        // set input flag at the field
        xFieldProperties->setPropertyValue(
            getPropertyName(PROP_IS_INPUT), uno::Any( true ));
        // set the prompt
        xFieldProperties->setPropertyValue(
            getPropertyName(PROP_HINT),
            uno::Any( sHint ));
        xFieldProperties->setPropertyValue(getPropertyName(PROP_SUB_TYPE), uno::Any(text::SetVariableType::STRING));
        // The ASK has no field value to display
        xFieldProperties->setPropertyValue(getPropertyName(PROP_IS_VISIBLE), uno::Any(false));
    }
    else
    {
        //don't insert the field
        //todo: maybe import a 'normal' input field here?
        xFieldInterface = nullptr;
    }
}

/**
 * Converts a Microsoft Word field formula into LibreOffice syntax
 * @param input The Microsoft Word field formula, with no leading '=' sign
 * @return An equivalent LibreOffice field formula
 */
OUString DomainMapper_Impl::convertFieldFormula(const OUString& input) {

    if (!m_pSettingsTable)
    {
        return input;
    }

    OUString listSeparator = m_pSettingsTable->GetListSeparator();

    /* Replace logical condition functions with LO equivalent operators */
    OUString changed = input.replaceAll(" <> ", " NEQ ");
    changed = changed.replaceAll(" <= ", " LEQ ");
    changed = changed.replaceAll(" >= ", " GEQ ");
    changed = changed.replaceAll(" = " , " EQ ");
    changed = changed.replaceAll(" < " , " L ");
    changed = changed.replaceAll(" > " , " G ");

    changed = changed.replaceAll("<>", " NEQ ");
    changed = changed.replaceAll("<=", " LEQ ");
    changed = changed.replaceAll(">=", " GEQ ");
    changed = changed.replaceAll("=" , " EQ ");
    changed = changed.replaceAll("<" , " L ");
    changed = changed.replaceAll(">" , " G ");

    /* Replace function calls with infix keywords for AND(), OR(), and ROUND(). Nothing needs to be
     * done for NOT(). This simple regex will work properly with most common cases. However, it may
     * not work correctly when the arguments are nested subcalls to other functions, like
     * ROUND(MIN(1,2),MAX(3,4)). See TDF#134765.  */
    icu::ErrorCode status;
    icu::UnicodeString usInput(changed.getStr());
    const uint32_t rMatcherFlags = UREGEX_CASE_INSENSITIVE;
    OUString regex = "\\b(AND|OR|ROUND)\\s*\\(\\s*([^" + listSeparator + "]+)\\s*" + listSeparator + "\\s*([^)]+)\\s*\\)";
    icu::UnicodeString usRegex(regex.getStr());
    icu::RegexMatcher rmatch1(usRegex, usInput, rMatcherFlags, status);
    usInput = rmatch1.replaceAll(icu::UnicodeString("(($2) $1 ($3))"), status);

    /* Assumes any remaining list separators separate arguments to functions that accept lists
     * (SUM, MIN, MAX, MEAN, etc.) */
    usInput.findAndReplace(icu::UnicodeString(listSeparator.getStr()), "|");

    /* Surround single cell references with angle brackets.
     * If there is ever added a function name that ends with a digit, this regex will need to be revisited. */
    icu::RegexMatcher rmatch2("\\b([A-Z]{1,3}[0-9]+)\\b(?![(])", usInput, rMatcherFlags, status);
    usInput = rmatch2.replaceAll(icu::UnicodeString("<$1>"), status);

    /* Cell references must be upper case
     * TODO: convert reference to other tables, e.g. SUM(Table1 A1:B2), where "Table1" is a bookmark of the table,
     * TODO: also column range A:A */
    icu::RegexMatcher rmatch3("(<[a-z]{1,3}[0-9]+>|\\b(above|below|left|right)\\b)", usInput, rMatcherFlags, status);
    icu::UnicodeString replacedCellRefs;
    while (rmatch3.find(status) && status.isSuccess()) {
        rmatch3.appendReplacement(replacedCellRefs, rmatch3.group(status).toUpper(), status);
    }
    rmatch3.appendTail(replacedCellRefs);

    /* Fix up cell ranges */
    icu::RegexMatcher rmatch4("<([A-Z]{1,3}[0-9]+)>:<([A-Z]{1,3}[0-9]+)>", replacedCellRefs, rMatcherFlags, status);
    usInput = rmatch4.replaceAll(icu::UnicodeString("<$1:$2>"), status);

    /* Fix up user defined names */
    icu::RegexMatcher rmatch5("\\bDEFINED\\s*\\(<([A-Z]+[0-9]+)>\\)", usInput, rMatcherFlags, status);
    usInput = rmatch5.replaceAll(icu::UnicodeString("DEFINED($1)"), status);

    /* Prepare replace of ABOVE/BELOW/LEFT/RIGHT by adding spaces around them */
    icu::RegexMatcher rmatch6("\\b(ABOVE|BELOW|LEFT|RIGHT)\\b", usInput, rMatcherFlags, status);
    usInput = rmatch6.replaceAll(icu::UnicodeString(" $1 "), status);

    /* DOCX allows to set decimal symbol independently from the locale of the document, so if
     * needed, convert decimal comma to get working formula in a document language (locale),
     * which doesn't use decimal comma */
    if ( m_pSettingsTable->GetDecimalSymbol() == "," && !m_bIsDecimalComma )
    {
        icu::RegexMatcher rmatch7("\\b([0-9]+),([0-9]+([eE][-]?[0-9]+)?)\\b", usInput, rMatcherFlags, status);
        usInput = rmatch7.replaceAll(icu::UnicodeString("$1.$2"), status);
    }

    return OUString(usInput.getTerminatedBuffer());
}

void DomainMapper_Impl::handleFieldFormula
    (const FieldContextPtr& pContext,
     uno::Reference< beans::XPropertySet > const& xFieldProperties)
{
    OUString command = pContext->GetCommand().trim();

    //  Remove number formatting from \# to end of command
    //  TODO: handle custom number formatting
    sal_Int32 delimPos = command.indexOf("\\#");
    if (delimPos != -1)
    {
        command = command.replaceAt(delimPos, command.getLength() - delimPos, u"").trim();
    }

    // command must contains = and at least another char
    if (command.getLength() < 2)
        return;

    // we don't copy the = symbol from the command
    OUString formula = convertFieldFormula(command.copy(1));

    xFieldProperties->setPropertyValue(getPropertyName(PROP_CONTENT), uno::Any(formula));
    xFieldProperties->setPropertyValue(getPropertyName(PROP_NUMBER_FORMAT), uno::Any(sal_Int32(0)));
    xFieldProperties->setPropertyValue("IsShowFormula", uno::Any(false));

    // grab-bag the original and converted formula
    if (hasTableManager())
    {
        TablePropertyMapPtr pPropMap(new TablePropertyMap());
        pPropMap->Insert(PROP_CELL_FORMULA, uno::Any(command.copy(1)), true, CELL_GRAB_BAG);
        pPropMap->Insert(PROP_CELL_FORMULA_CONVERTED, uno::Any(formula), true, CELL_GRAB_BAG);
        getTableManager().cellProps(pPropMap);
    }
}

void  DomainMapper_Impl::handleRubyEQField( const FieldContextPtr& pContext)
{
    const OUString & rCommand(pContext->GetCommand());
    sal_Int32 nIndex = 0, nEnd = 0;
    RubyInfo aInfo ;
    nIndex = rCommand.indexOf("\\* jc" );
    if (nIndex != -1)
    {
        nIndex += 5;
        sal_uInt32  nJc = o3tl::toInt32(o3tl::getToken(rCommand, 0, ' ',nIndex));
        const   sal_Int32   aRubyAlignValues[] =
        {
            NS_ooxml::LN_Value_ST_RubyAlign_center,
            NS_ooxml::LN_Value_ST_RubyAlign_distributeLetter,
            NS_ooxml::LN_Value_ST_RubyAlign_distributeSpace,
            NS_ooxml::LN_Value_ST_RubyAlign_left,
            NS_ooxml::LN_Value_ST_RubyAlign_right,
            NS_ooxml::LN_Value_ST_RubyAlign_rightVertical,
        };
        aInfo.nRubyAlign = aRubyAlignValues[(nJc<SAL_N_ELEMENTS(aRubyAlignValues))?nJc:0];
    }

    // we don't parse or use the font field in rCommand

    nIndex = rCommand.indexOf("\\* hps" );
    if (nIndex != -1)
    {
        nIndex += 6;
        aInfo.nHps = o3tl::toInt32(o3tl::getToken(rCommand, 0, ' ',nIndex));
    }

    nIndex = rCommand.indexOf("\\o");
    if (nIndex == -1)
        return;
    nIndex = rCommand.indexOf('(', nIndex);
    if (nIndex == -1)
        return;
    nEnd = rCommand.lastIndexOf(')');
    if (nEnd == -1)
        return;
    if (nEnd <= nIndex)
        return;

    std::u16string_view sRubyParts = rCommand.subView(nIndex+1,nEnd-nIndex-1);
    nIndex = 0;
    std::u16string_view sPart1 = o3tl::getToken(sRubyParts, 0, ',', nIndex);
    std::u16string_view sPart2 = o3tl::getToken(sRubyParts, 0, ',', nIndex);
    size_t nIndex2 = 0;
    size_t nEnd2 = 0;
    if ((nIndex2 = sPart1.find('(')) != std::u16string_view::npos && (nEnd2 = sPart1.rfind(')')) != std::u16string_view::npos  && nEnd2 > nIndex2)
    {
        aInfo.sRubyText = sPart1.substr(nIndex2+1,nEnd2-nIndex2-1);
    }

    PropertyMapPtr pRubyContext(new PropertyMap());
    pRubyContext->InsertProps(GetTopContext());
    if (aInfo.nHps > 0)
    {
        double fVal = double(aInfo.nHps) / 2.;
        uno::Any aVal( fVal );

        pRubyContext->Insert(PROP_CHAR_HEIGHT, aVal);
        pRubyContext->Insert(PROP_CHAR_HEIGHT_ASIAN, aVal);
    }
    PropertyValueVector_t aProps = comphelper::sequenceToContainer< PropertyValueVector_t >(pRubyContext->GetPropertyValues());
    aInfo.sRubyStyle = m_rDMapper.getOrCreateCharStyle(aProps, /*bAlwaysCreate=*/false);
    PropertyMapPtr pCharContext(new PropertyMap());
    if (m_pLastCharacterContext)
        pCharContext->InsertProps(m_pLastCharacterContext);
    pCharContext->InsertProps(pContext->getProperties());
    pCharContext->Insert(PROP_RUBY_TEXT, uno::Any( aInfo.sRubyText ) );
    pCharContext->Insert(PROP_RUBY_ADJUST, uno::Any(static_cast<sal_Int16>(ConversionHelper::convertRubyAlign(aInfo.nRubyAlign))));
    if ( aInfo.nRubyAlign == NS_ooxml::LN_Value_ST_RubyAlign_rightVertical )
        pCharContext->Insert(PROP_RUBY_POSITION, uno::Any(css::text::RubyPosition::INTER_CHARACTER));
    pCharContext->Insert(PROP_RUBY_STYLE, uno::Any(aInfo.sRubyStyle));
    appendTextPortion(OUString(sPart2), pCharContext);

}

void DomainMapper_Impl::handleAutoNum
    (const FieldContextPtr& pContext,
    uno::Reference< uno::XInterface > const & xFieldInterface,
    uno::Reference< beans::XPropertySet > const& xFieldProperties)
{
    //create a sequence field master "AutoNr"
    uno::Reference< beans::XPropertySet > xMaster =
    FindOrCreateFieldMaster
        ("com.sun.star.text.FieldMaster.SetExpression",
        "AutoNr");

    xMaster->setPropertyValue( getPropertyName(PROP_SUB_TYPE),
        uno::Any(text::SetVariableType::SEQUENCE));

    //apply the numbering type
    xFieldProperties->setPropertyValue(
        getPropertyName(PROP_NUMBERING_TYPE),
        uno::Any( lcl_ParseNumberingType(pContext->GetCommand()) ));
        // attach the master to the field
    uno::Reference< text::XDependentTextField > xDependentField
        ( xFieldInterface, uno::UNO_QUERY_THROW );
    xDependentField->attachTextFieldMaster( xMaster );
}

void DomainMapper_Impl::handleAuthor
    (std::u16string_view,
     uno::Reference< beans::XPropertySet > const& xFieldProperties,
     FieldId  eFieldId )
{
    if (eFieldId == FIELD_USERNAME)
        xFieldProperties->setPropertyValue
            ( getPropertyName(PROP_FULL_NAME), uno::Any( true ));

    // Always set as FIXED b/c MS Word only updates these fields via user intervention (F9)
    // AUTHOR of course never changes and USERNAME is easily mis-used as an original author field.
    // Additionally, this was forced as fixed if any special case-formatting was provided.
    {
        xFieldProperties->setPropertyValue(
                getPropertyName( PROP_IS_FIXED ),
                uno::Any( true ));
        //PROP_CURRENT_PRESENTATION is set later anyway
    }
}

    void DomainMapper_Impl::handleDocProperty
        (const FieldContextPtr& pContext,
        OUString const& rFirstParam,
        uno::Reference< uno::XInterface > & xFieldInterface)
{
    //some docproperties should be imported as document statistic fields, some as DocInfo fields
    //others should be user fields
    if (rFirstParam.isEmpty())
        return;

    constexpr sal_uInt8 SET_ARABIC = 0x01;
    constexpr sal_uInt8 SET_DATE = 0x04;
    struct DocPropertyMap
    {
        const char* pDocPropertyName;
        const char* pServiceName;
        sal_uInt8       nFlags;
    };
    static const DocPropertyMap aDocProperties[] =
    {
        {"CreateTime",       "DocInfo.CreateDateTime",  SET_DATE},
        {"Characters",       "CharacterCount",          SET_ARABIC},
        {"Comments",         "DocInfo.Description",     0},
        {"Keywords",         "DocInfo.KeyWords",        0},
        {"LastPrinted",      "DocInfo.PrintDateTime",   0},
        {"LastSavedBy",      "DocInfo.ChangeAuthor",    0},
        {"LastSavedTime",    "DocInfo.ChangeDateTime",  SET_DATE},
        {"Paragraphs",       "ParagraphCount",          SET_ARABIC},
        {"RevisionNumber",   "DocInfo.Revision",        0},
        {"Subject",          "DocInfo.Subject",         0},
        {"Template",         "TemplateName",            0},
        {"Title",            "DocInfo.Title",           0},
        {"TotalEditingTime", "DocInfo.EditTime",        0},
        {"Words",            "WordCount",               SET_ARABIC}

        //other available DocProperties:
        //Bytes, Category, CharactersWithSpaces, Company
        //HyperlinkBase,
        //Lines, Manager, NameofApplication, ODMADocId, Pages,
        //Security,
    };
    uno::Reference<document::XDocumentProperties> xDocumentProperties = m_xTextDocument->getDocumentProperties();
    uno::Reference<beans::XPropertySet>  xUserDefinedProps(xDocumentProperties->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySetInfo> xPropertySetInfo =  xUserDefinedProps->getPropertySetInfo();
    //search for a field mapping
    OUString sFieldServiceName;
    size_t nMap = 0;
    if (!xPropertySetInfo->hasPropertyByName(rFirstParam))
    {
        for( ; nMap < SAL_N_ELEMENTS(aDocProperties); ++nMap )
        {
            if (rFirstParam.equalsAscii(aDocProperties[nMap].pDocPropertyName))
            {
                sFieldServiceName = OUString::createFromAscii(aDocProperties[nMap].pServiceName);
                break;
            }
        }
    }
    else
        pContext->CacheVariableValue(xUserDefinedProps->getPropertyValue(rFirstParam));

    OUString sServiceName("com.sun.star.text.TextField.");
    bool bIsCustomField = false;
    if(sFieldServiceName.isEmpty())
    {
        //create a custom property field
        sServiceName += "DocInfo.Custom";
        bIsCustomField = true;
    }
    else
    {
        sServiceName += sFieldServiceName;
    }
    if (m_xTextDocument)
        xFieldInterface = m_xTextDocument->createInstance(sServiceName);
    uno::Reference<beans::XPropertySet> xFieldProperties( xFieldInterface, uno::UNO_QUERY_THROW);
    if( bIsCustomField )
    {
        xFieldProperties->setPropertyValue(
            getPropertyName(PROP_NAME), uno::Any(rFirstParam));
        pContext->SetCustomField( xFieldProperties );
    }
    else
    {
        if(0 != (aDocProperties[nMap].nFlags & SET_ARABIC))
            xFieldProperties->setPropertyValue(
                getPropertyName(PROP_NUMBERING_TYPE),
                uno::Any( style::NumberingType::ARABIC ));
        else if(0 != (aDocProperties[nMap].nFlags & SET_DATE))
        {
            xFieldProperties->setPropertyValue(
                getPropertyName(PROP_IS_DATE),
                    uno::Any( true ));
            SetNumberFormat( pContext->GetCommand(), xFieldProperties );
        }
    }
}

static uno::Sequence< beans::PropertyValues > lcl_createTOXLevelHyperlinks( bool bHyperlinks, const OUString& sChapterNoSeparator,
                                   const uno::Sequence< beans::PropertyValues >& aLevel, const std::optional<style::TabStop> numtab)
{
    //create a copy of the level and add new entries

    std::vector<css::beans::PropertyValues> aNewLevel;
    aNewLevel.reserve(aLevel.getLength() + 5); // at most 5 added items

    static constexpr OUString tokType(u"TokenType"_ustr);
    static constexpr OUString tokHStart(u"TokenHyperlinkStart"_ustr);
    static constexpr OUString tokHEnd(u"TokenHyperlinkEnd"_ustr);
    static constexpr OUStringLiteral tokPNum(u"TokenPageNumber");
    static constexpr OUStringLiteral tokENum(u"TokenEntryNumber");

    if (bHyperlinks)
        aNewLevel.push_back({ comphelper::makePropertyValue(tokType, tokHStart) });

    for (const auto& item : aLevel)
    {
        OUString tokenType;
        if (auto it = std::find_if(item.begin(), item.end(),
                                   [](const auto& p) { return p.Name == tokType; });
            it != item.end())
            it->Value >>= tokenType;

        if (bHyperlinks && (tokenType == tokHStart || tokenType == tokHEnd))
            continue; // We add hyperlink ourselves, so just skip existing hyperlink start / end

        if (!sChapterNoSeparator.isEmpty() && tokenType == tokPNum)
        {
            // This is an existing page number token; insert the chapter and separator before it
            aNewLevel.push_back(
                { comphelper::makePropertyValue(tokType, OUString("TokenChapterInfo")),
                  comphelper::makePropertyValue("ChapterFormat", text::ChapterFormat::NUMBER) });
            aNewLevel.push_back({ comphelper::makePropertyValue(tokType, OUString("TokenText")),
                                  comphelper::makePropertyValue("Text", sChapterNoSeparator) });
        }

        aNewLevel.push_back(item);

        if (numtab && tokenType == tokENum)
        {
            // There is a fixed tab stop position needed in the level after the numbering
            aNewLevel.push_back(
                { comphelper::makePropertyValue(tokType, OUString("TokenTabStop")),
                  comphelper::makePropertyValue("TabStopPosition", numtab->Position) });
        }
    }

    if (bHyperlinks)
        aNewLevel.push_back({ comphelper::makePropertyValue(tokType, tokHEnd) });

    return comphelper::containerToSequence(aNewLevel);
}

/// Returns title of the TOC placed in paragraph(s) before TOC field inside STD-frame
OUString DomainMapper_Impl::extractTocTitle()
{
    if (!m_StreamStateStack.top().xSdtEntryStart.is())
        return OUString();

    uno::Reference< text::XTextAppend > xTextAppend = m_aTextAppendStack.top().xTextAppend;
    if(!xTextAppend.is())
        return OUString();

    // try-catch was added in the same way as inside appendTextSectionAfter()
    try
    {
        uno::Reference<text::XParagraphCursor> const xCursor(
            xTextAppend->createTextCursorByRange(m_StreamStateStack.top().xSdtEntryStart), uno::UNO_QUERY_THROW);
        if (!xCursor.is())
            return OUString();

        //the cursor has been moved to the end of the paragraph because of the appendTextPortion() calls
        xCursor->gotoStartOfParagraph( false );
        if (m_aTextAppendStack.top().xInsertPosition.is())
            xCursor->gotoRange( m_aTextAppendStack.top().xInsertPosition, true );
        else
            xCursor->gotoEnd( true );

        // the paragraph after this new section might have been already inserted
        OUString sResult = xCursor->getString();
        if (sResult.endsWith(SAL_NEWLINE_STRING))
            sResult = sResult.copy(0, sResult.getLength() - SAL_N_ELEMENTS(SAL_NEWLINE_STRING) + 1);

        return sResult;
    }
    catch(const uno::Exception&)
    {
    }

    return OUString();
}

css::uno::Reference<css::beans::XPropertySet>
DomainMapper_Impl::StartIndexSectionChecked(const OUString& sServiceName)
{
    if (m_StreamStateStack.top().bParaChanged)
    {
        finishParagraph(GetTopContextOfType(CONTEXT_PARAGRAPH), false); // resets bParaChanged
        PopProperties(CONTEXT_PARAGRAPH);
        PushProperties(CONTEXT_PARAGRAPH);
        SetIsFirstRun(true);
        // The first paragraph of the index that is continuation of just finished one needs to be
        // removed when finished (unless more content will arrive, which will set bParaChanged)
        m_StreamStateStack.top().bRemoveThisParagraph = true;
    }
    const auto& xTextAppend = GetTopTextAppend();
    const auto xTextRange = xTextAppend->getEnd();
    const auto xRet = createSectionForRange(xTextRange, xTextRange, sServiceName, false);
    if (!m_aTextAppendStack.top().xInsertPosition)
    {
        try
        {
            m_bStartedTOC = true;
            uno::Reference<text::XTextCursor> xTOCTextCursor
                = xTextRange->getText()->createTextCursor();
            assert(xTOCTextCursor.is());
            xTOCTextCursor->gotoEnd(false);
            m_aTextAppendStack.push(TextAppendContext(xTextAppend, xTOCTextCursor));
        }
        catch (const uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("writerfilter.dmapper",
                                 "DomainMapper_Impl::StartIndexSectionChecked:");
        }
    }
    return xRet;
}

/**
 This is a heuristic to find Word's w:styleId value from localised style name.
 It's not clear how exactly it works, but apparently Word stores into
 w:styleId some filtered representation of the localised style name.
 Tragically there are references to the localised style name itself in TOC
 fields.
 Hopefully this works and a complete map of >100 built-in style names
 localised to all languages isn't needed.
*/
static auto FilterChars(std::u16string_view const& rStyleName) -> OUString
{
    return msfilter::util::CreateDOCXStyleId(rStyleName);
}

static OUString UnquoteFieldText(std::u16string_view s)
{
    OUStringBuffer result(s.size());
    for (size_t i = 0; i < s.size(); ++i)
    {
        switch (s[i])
        {
        case '"':
            continue;
        case '\\':
            if (i < s.size() - 1)
                ++i;
            [[fallthrough]];
        default:
            result.append(s[i]);
        }
    }
    return result.makeStringAndClear();
}

OUString DomainMapper_Impl::ConvertTOCStyleName(OUString const& rTOCStyleName)
{
    assert(!rTOCStyleName.isEmpty());
    if (auto const pStyle = GetStyleSheetTable()->FindStyleSheetByISTD(rTOCStyleName))
    {   // theoretical case: what OOXML says
        return pStyle->m_sStyleName;
    }
    auto const pStyle = GetStyleSheetTable()->FindStyleSheetByISTD(FilterChars(rTOCStyleName));
    if (pStyle && m_bIsNewDoc)
    {   // practical case: Word wrote i18n name to TOC field, but it doesn't
        // exist in styles.xml; tdf#153083 clone it for best roundtrip
        SAL_INFO("writerfilter.dmapper", "cloning TOC paragraph style (presumed built-in) " << rTOCStyleName << " from " << pStyle->m_sStyleName);
        return GetStyleSheetTable()->CloneTOCStyle(GetFontTable(), pStyle, rTOCStyleName);
    }
    else
    {
        return GetStyleSheetTable()->ConvertStyleName(rTOCStyleName);
    }
}

void DomainMapper_Impl::handleToc
    (const FieldContextPtr& pContext,
    const OUString & sTOCServiceName)
{
    OUString sValue;
    if (IsInHeaderFooter())
        m_bStartTOCHeaderFooter = true;
    bool bTableOfFigures = false;
    bool bHyperlinks = false;
    bool bFromOutline = false;
    bool bFromEntries = false;
    bool bHideTabLeaderPageNumbers = false ;
    bool bIsTabEntry = false ;
    bool bNewLine = false ;
    bool bParagraphOutlineLevel = false;

    sal_Int16 nMaxLevel = 10;
    OUString sTemplate;
    OUString sChapterNoSeparator;
    OUString sFigureSequence;
    OUString aBookmarkName;

//                  \a Builds a table of figures but does not include the captions's label and number
    if( lcl_FindInCommand( pContext->GetCommand(), 'a', sValue ))
    { //make it a table of figures
        bTableOfFigures = true;
    }
//                  \b Uses a bookmark to specify area of document from which to build table of contents
    if( lcl_FindInCommand( pContext->GetCommand(), 'b', sValue ))
    {
        aBookmarkName = sValue.trim().replaceAll("\"","");
    }
    if( lcl_FindInCommand( pContext->GetCommand(), 'c', sValue ))
//                  \c Builds a table of figures of the given label
    {
                        //todo: sValue contains the label's name
        bTableOfFigures = true;
        sFigureSequence = sValue.trim();
        sFigureSequence = sFigureSequence.replaceAll("\"", "").replaceAll("'","");
    }
//                  \d Defines the separator between sequence and page numbers
    if( lcl_FindInCommand( pContext->GetCommand(), 'd', sValue ))
    {
                        //todo: insert the chapter number into each level and insert the separator additionally
        sChapterNoSeparator = UnquoteFieldText(sValue);
    }
//                  \f Builds a table of contents using TC entries instead of outline levels
    if( lcl_FindInCommand( pContext->GetCommand(), 'f', sValue ))
    {
                        //todo: sValue can contain a TOC entry identifier - use unclear
        bFromEntries = true;
    }
//                  \h Hyperlinks the entries and page numbers within the table of contents
    if( lcl_FindInCommand( pContext->GetCommand(), 'h', sValue ))
    {
                        //todo: make all entries to hyperlinks
        bHyperlinks = true;
    }
//                  \l Defines the TC entries field level used to build a table of contents
//                    if( lcl_FindInCommand( pContext->GetCommand(), 'l', sValue ))
//                    {
                            //todo: entries can only be included completely
//                    }
//                  \n Builds a table of contents or a range of entries, such as 1-9 in a table of contents without page numbers
//                    if( lcl_FindInCommand( pContext->GetCommand(), 'n', sValue ))
//                    {
                        //todo: what does the description mean?
//                    }
//                  \o  Builds a table of contents by using outline levels instead of TC entries
    if( lcl_FindInCommand( pContext->GetCommand(), 'o', sValue ))
    {
        bFromOutline = true;
        if (sValue.isEmpty())
            nMaxLevel = WW_OUTLINE_MAX;
        else
        {
            sal_Int32 nIndex = 0;
            o3tl::getToken(sValue, 0, '-', nIndex );
            nMaxLevel = static_cast<sal_Int16>(nIndex != -1 ? o3tl::toInt32(sValue.subView(nIndex)) : 0);
        }
    }
//                  \p Defines the separator between the table entry and its page number
//                  \s  Builds a table of contents by using a sequence type
//                  \t  Builds a table of contents by using style names other than the standard outline styles
    if( lcl_FindInCommand( pContext->GetCommand(), 't', sValue ))
    {
        OUString sToken = sValue.getToken(1, '"');
        sTemplate = sToken.isEmpty() ? sValue : sToken;
    }
//                  \u  Builds a table of contents by using the applied paragraph outline level
    if( lcl_FindInCommand( pContext->GetCommand(), 'u', sValue ))
    {
        bFromOutline = true;
        bParagraphOutlineLevel = true;
                        //todo: what doesn 'the applied paragraph outline level' refer to?
    }
//    \w Preserve tab characters within table entries
    if( lcl_FindInCommand( pContext->GetCommand(), 'w', sValue ))
    {
        bIsTabEntry = true ;
    }
//                  \x Preserve newline characters within table entries
    if( lcl_FindInCommand( pContext->GetCommand(), 'x', sValue ))
    {
        bNewLine = true ;
    }
//                  \z Hides page numbers within the table of contents when shown in Web Layout View
    if( lcl_FindInCommand( pContext->GetCommand(), 'z', sValue ))
    {
        bHideTabLeaderPageNumbers = true ;
    }

                    //if there's no option then it should be created from outline
    if( !bFromOutline && !bFromEntries && sTemplate.isEmpty()  )
        bFromOutline = true;

    const OUString aTocTitle = extractTocTitle();

    uno::Reference<beans::XPropertySet> xTOC;

    if (m_xTextDocument && ! m_aTextAppendStack.empty())
    {
        const auto& xTextAppend = GetTopTextAppend();
        if (aTocTitle.isEmpty() || bTableOfFigures)
        {
            // reset marker of the TOC title
            m_StreamStateStack.top().xSdtEntryStart.clear();

            // Create section before setting m_bStartTOC: finishing paragraph
            // inside StartIndexSectionChecked could do the wrong thing otherwise
            xTOC = StartIndexSectionChecked(bTableOfFigures ? "com.sun.star.text.IllustrationsIndex"
                                                            : sTOCServiceName);

            const auto xTextCursor = xTextAppend->getText()->createTextCursor();
            if (xTextCursor)
                xTextCursor->gotoEnd(false);
            m_xTOCMarkerCursor = xTextCursor;
        }
        else
        {
            // create TOC section
            css::uno::Reference<css::text::XTextRange> xTextRangeEndOfTocHeader = GetTopTextAppend()->getEnd();
            xTOC = createSectionForRange(m_StreamStateStack.top().xSdtEntryStart, xTextRangeEndOfTocHeader, sTOCServiceName, false);

            // init [xTOCMarkerCursor]
            uno::Reference< text::XText > xText = xTextAppend->getText();
            m_xTOCMarkerCursor = xText->createTextCursor();

            // create header of the TOC with the TOC title inside
            createSectionForRange(m_StreamStateStack.top().xSdtEntryStart, xTextRangeEndOfTocHeader, "com.sun.star.text.IndexHeaderSection", true);
        }
    }

    m_bStartTOC = true;
    pContext->SetTOC(xTOC);
    m_StreamStateStack.top().bParaHadField = false;

    if (!xTOC)
        return;

    xTOC->setPropertyValue(getPropertyName( PROP_TITLE ), uno::Any(aTocTitle));

    if (!aBookmarkName.isEmpty())
        xTOC->setPropertyValue(getPropertyName(PROP_TOC_BOOKMARK), uno::Any(aBookmarkName));
    if (!bTableOfFigures)
    {
        xTOC->setPropertyValue( getPropertyName( PROP_LEVEL ), uno::Any( nMaxLevel ) );
        xTOC->setPropertyValue( getPropertyName( PROP_CREATE_FROM_OUTLINE ), uno::Any( bFromOutline ));
        xTOC->setPropertyValue( getPropertyName( PROP_CREATE_FROM_MARKS ), uno::Any( bFromEntries ));
        xTOC->setPropertyValue( getPropertyName( PROP_HIDE_TAB_LEADER_AND_PAGE_NUMBERS ), uno::Any( bHideTabLeaderPageNumbers ));
        xTOC->setPropertyValue( getPropertyName( PROP_TAB_IN_TOC ), uno::Any( bIsTabEntry ));
        xTOC->setPropertyValue( getPropertyName( PROP_TOC_NEW_LINE ), uno::Any( bNewLine ));
        xTOC->setPropertyValue( getPropertyName( PROP_TOC_PARAGRAPH_OUTLINE_LEVEL ), uno::Any( bParagraphOutlineLevel ));
        if( !sTemplate.isEmpty() )
        {
                            //the string contains comma separated the names and related levels
                            //like: "Heading 1,1,Heading 2,2"
            TOCStyleMap aMap;
            sal_Int32 nLevel;
            sal_Int32 nPosition = 0;
            auto const tsep(sTemplate.indexOf(',') != -1 ? ',' : ';');
            while( nPosition >= 0)
            {
                OUString sStyleName = sTemplate.getToken(0, tsep, nPosition);
                                //empty tokens should be skipped
                while( sStyleName.isEmpty() && nPosition > 0 )
                    sStyleName = sTemplate.getToken(0, tsep, nPosition);
                nLevel = o3tl::toInt32(o3tl::getToken(sTemplate, 0, tsep, nPosition ));
                if( !nLevel )
                    nLevel = 1;
                if( !sStyleName.isEmpty() )
                    aMap.emplace(nLevel, sStyleName);
            }
            uno::Reference< container::XIndexReplace> xParaStyles;
            xTOC->getPropertyValue(getPropertyName(PROP_LEVEL_PARAGRAPH_STYLES)) >>= xParaStyles;
            for( nLevel = 1; nLevel < 10; ++nLevel)
            {
                sal_Int32 nLevelCount = aMap.count( nLevel );
                if( nLevelCount  )
                {
                    TOCStyleMap::iterator aTOCStyleIter = aMap.find( nLevel );

                    uno::Sequence< OUString> aStyles( nLevelCount );
                    for ( auto& rStyle : asNonConstRange(aStyles) )
                    {
                        // tdf#153083 must map w:styleId to w:name
                        rStyle = ConvertTOCStyleName(aTOCStyleIter->second);
                        ++aTOCStyleIter;
                    }
                    xParaStyles->replaceByIndex(nLevel - 1, uno::Any(aStyles));
                }
            }
            xTOC->setPropertyValue(getPropertyName(PROP_CREATE_FROM_LEVEL_PARAGRAPH_STYLES), uno::Any( true ));

        }

        uno::Reference<container::XIndexAccess> xChapterNumberingRules;
        if (m_xTextDocument)
            xChapterNumberingRules = m_xTextDocument->getChapterNumberingRules();
        uno::Reference<container::XNameContainer> xStyles;
        if (m_xTextDocument)
        {
            auto xStyleFamilies = m_xTextDocument->getStyleFamilies();
            xStyleFamilies->getByName(getPropertyName(PROP_PARAGRAPH_STYLES)) >>= xStyles;
        }

        uno::Reference< container::XIndexReplace> xLevelFormats;
        xTOC->getPropertyValue(getPropertyName(PROP_LEVEL_FORMAT)) >>= xLevelFormats;
        sal_Int32 nLevelCount = xLevelFormats->getCount();
                        //start with level 1, 0 is the header level
        for( sal_Int32 nLevel = 1; nLevel < nLevelCount; ++nLevel)
        {
            uno::Sequence< beans::PropertyValues > aLevel;
            xLevelFormats->getByIndex( nLevel ) >>= aLevel;

            // Get the tab stops coming from the styles; store to the level definitions
            std::optional<style::TabStop> numTab;
            if (xChapterNumberingRules && xStyles)
            {
                // This relies on the chapter numbering rules already defined
                // (see ListDef::CreateNumberingRules)
                uno::Sequence<beans::PropertyValue> props;
                xChapterNumberingRules->getByIndex(nLevel - 1) >>= props;
                bool bHasNumbering = false;
                bool bUseTabStop = false;
                for (const auto& propval : props)
                {
                    // We rely on PositionAndSpaceMode being always equal to LABEL_ALIGNMENT,
                    // because ListDef::CreateNumberingRules doesn't create legacy lists
                    if (propval.Name == "NumberingType")
                        bHasNumbering = propval.Value != style::NumberingType::NUMBER_NONE;
                    else if (propval.Name == "LabelFollowedBy")
                        bUseTabStop = propval.Value == text::LabelFollow::LISTTAB;
                    // Do not use FirstLineIndent property from the rules, because it is unreliable
                }
                if (bHasNumbering && bUseTabStop)
                {
                    OUString style;
                    xTOC->getPropertyValue("ParaStyleLevel" + OUString::number(nLevel)) >>= style;
                    uno::Reference<beans::XPropertySet> xStyle;
                    if (xStyles->getByName(style) >>= xStyle)
                    {
                        if (uno::Reference<beans::XPropertyState> xPropState{ xStyle,
                                                                              uno::UNO_QUERY })
                        {
                            if (xPropState->getPropertyState("ParaTabStops")
                                == beans::PropertyState_DIRECT_VALUE)
                            {
                                if (uno::Sequence<style::TabStop> tabStops;
                                    xStyle->getPropertyValue("ParaTabStops") >>= tabStops)
                                {
                                    // If the style only has one tab stop, Word uses it for
                                    // page number, and generates the other from defaults
                                    if (tabStops.getLength() > 1)
                                        numTab = tabStops[0];
                                }
                            }
                        }
                    }
                    if (!numTab)
                    {
                        // Generate the default position.
                        // Word uses multiples of 440 twips for default chapter number tab stops
                        numTab.emplace();
                        numTab->Position
                            = o3tl::convert(440 * nLevel, o3tl::Length::twip, o3tl::Length::mm100);
                    }
                }
            }

            uno::Sequence< beans::PropertyValues > aNewLevel = lcl_createTOXLevelHyperlinks(
                                                bHyperlinks, sChapterNoSeparator,
                                                aLevel, numTab);
            xLevelFormats->replaceByIndex( nLevel, uno::Any( aNewLevel ) );
        }
    }
    else // if (bTableOfFigures)
    {
        if (!sFigureSequence.isEmpty())
            xTOC->setPropertyValue(getPropertyName(PROP_LABEL_CATEGORY),
                                   uno::Any(sFigureSequence));

        if (!sTemplate.isEmpty())
        {
            OUString const sConvertedStyleName(ConvertTOCStyleName(sTemplate));
            xTOC->setPropertyValue("CreateFromParagraphStyle", uno::Any(sConvertedStyleName));
        }

        if ( bHyperlinks )
        {
            uno::Reference< container::XIndexReplace> xLevelFormats;
            xTOC->getPropertyValue(getPropertyName(PROP_LEVEL_FORMAT)) >>= xLevelFormats;
            uno::Sequence< beans::PropertyValues > aLevel;
            xLevelFormats->getByIndex( 1 ) >>= aLevel;

            uno::Sequence< beans::PropertyValues > aNewLevel = lcl_createTOXLevelHyperlinks(
                                                bHyperlinks, sChapterNoSeparator,
                                                aLevel, {});
            xLevelFormats->replaceByIndex( 1, uno::Any( aNewLevel ) );
        }
    }
}

uno::Reference<beans::XPropertySet> DomainMapper_Impl::createSectionForRange(
    uno::Reference< css::text::XTextRange > xStart,
    uno::Reference< css::text::XTextRange > xEnd,
    const OUString & sObjectType,
    bool stepLeft)
{
    if (!xStart.is())
        return uno::Reference<beans::XPropertySet>();
    if (!xEnd.is())
        return uno::Reference<beans::XPropertySet>();

    uno::Reference< beans::XPropertySet > xRet;
    if (m_aTextAppendStack.empty())
        return xRet;
    uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
    if(xTextAppend.is())
    {
        try
        {
            uno::Reference< text::XParagraphCursor > xCursor(
                xTextAppend->createTextCursorByRange( xStart ), uno::UNO_QUERY_THROW);
            //the cursor has been moved to the end of the paragraph because of the appendTextPortion() calls
            xCursor->gotoStartOfParagraph( false );
            xCursor->gotoRange( xEnd, true );
            //the paragraph after this new section is already inserted
            if (stepLeft)
                xCursor->goLeft(1, true);
            uno::Reference< text::XTextContent > xSection( m_xTextDocument->createInstance(sObjectType), uno::UNO_QUERY_THROW );
            try
            {
                xSection->attach( uno::Reference< text::XTextRange >( xCursor, uno::UNO_QUERY_THROW) );
            }
            catch(const uno::Exception&)
            {
            }
            xRet.set(xSection, uno::UNO_QUERY );
        }
        catch(const uno::Exception&)
        {
        }
    }

    return xRet;
}

void DomainMapper_Impl::handleBibliography
    (const FieldContextPtr& pContext,
    const OUString & sTOCServiceName)
{
    if (m_aTextAppendStack.empty())
    {
        // tdf#130214: a workaround to avoid crash on import errors
        SAL_WARN("writerfilter.dmapper", "no text append stack");
        return;
    }
    // Create section before setting m_bStartTOC and m_bStartBibliography: finishing paragraph
    // inside StartIndexSectionChecked could do the wrong thing otherwise
    const auto xTOC = StartIndexSectionChecked(sTOCServiceName);
    m_bStartTOC = true;
    m_bStartBibliography = true;

    if (xTOC.is())
        xTOC->setPropertyValue(getPropertyName( PROP_TITLE ), uno::Any(OUString()));

    pContext->SetTOC( xTOC );
    m_StreamStateStack.top().bParaHadField = false;

    uno::Reference< text::XTextContent > xToInsert( xTOC, uno::UNO_QUERY );
    appendTextContent(xToInsert, uno::Sequence< beans::PropertyValue >() );
}

void DomainMapper_Impl::handleIndex
    (const FieldContextPtr& pContext,
    const OUString & sTOCServiceName)
{
    // only UserIndex can handle user index defined by \f
    // e.g. INDEX \f "user-index-id"
    OUString sUserIndex;
    if ( lcl_FindInCommand( pContext->GetCommand(), 'f', sUserIndex ) )
        sUserIndex = lcl_trim(sUserIndex);

    // Create section before setting m_bStartTOC and m_bStartIndex: finishing paragraph
    // inside StartIndexSectionChecked could do the wrong thing otherwise
    const auto xTOC = StartIndexSectionChecked( sUserIndex.isEmpty()
            ? sTOCServiceName
            : "com.sun.star.text.UserIndex");

    m_bStartTOC = true;
    m_bStartIndex = true;
    OUString sValue;
    if (xTOC.is())
    {
        xTOC->setPropertyValue(getPropertyName( PROP_TITLE ), uno::Any(OUString()));

        if( lcl_FindInCommand( pContext->GetCommand(), 'r', sValue ))
        {
            xTOC->setPropertyValue("IsCommaSeparated", uno::Any(true));
        }
        if( lcl_FindInCommand( pContext->GetCommand(), 'h', sValue ))
        {
            xTOC->setPropertyValue("UseAlphabeticalSeparators", uno::Any(true));
        }
        if( !sUserIndex.isEmpty() )
        {
            xTOC->setPropertyValue("UserIndexName", uno::Any(sUserIndex));
        }
    }
    pContext->SetTOC( xTOC );
    m_StreamStateStack.top().bParaHadField = false;

    uno::Reference< text::XTextContent > xToInsert( xTOC, uno::UNO_QUERY );
    appendTextContent(xToInsert, uno::Sequence< beans::PropertyValue >() );

    if( lcl_FindInCommand( pContext->GetCommand(), 'c', sValue ))
    {
        sValue = sValue.replaceAll("\"", "");
        uno::Reference<text::XTextColumns> xTextColumns;
        if (xTOC.is())
        {
            xTOC->getPropertyValue(getPropertyName( PROP_TEXT_COLUMNS )) >>= xTextColumns;
        }
        if (xTextColumns.is())
        {
            xTextColumns->setColumnCount( sValue.toInt32() );
            xTOC->setPropertyValue( getPropertyName( PROP_TEXT_COLUMNS ), uno::Any( xTextColumns ) );
        }
    }
}

static auto InsertFieldmark(std::stack<TextAppendContext> & rTextAppendStack,
        uno::Reference<text::XFormField> const& xFormField,
        uno::Reference<text::XTextRange> const& xStartRange,
        std::optional<FieldId> const oFieldId) -> void
{
    uno::Reference<text::XTextContent> const xTextContent(xFormField, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextAppend> const& xTextAppend(rTextAppendStack.top().xTextAppend);
    uno::Reference<text::XTextCursor> const xCursor =
        xTextAppend->createTextCursorByRange(xStartRange);
    if (rTextAppendStack.top().xInsertPosition.is())
    {
        uno::Reference<text::XTextRangeCompare> const xCompare(
                rTextAppendStack.top().xTextAppend,
                uno::UNO_QUERY);
        if (xCompare->compareRegionStarts(xStartRange, rTextAppendStack.top().xInsertPosition) < 0)
        {
            SAL_WARN("writerfilter.dmapper", "invalid field mark positions");
            assert(false);
        }
        xCursor->gotoRange(rTextAppendStack.top().xInsertPosition, true);
    }
    else
    {
        xCursor->gotoEnd(true);
    }
    xTextAppend->insertTextContent(xCursor, xTextContent, true);
    if (oFieldId
        && (oFieldId == FIELD_FORMCHECKBOX || oFieldId == FIELD_FORMDROPDOWN))
    {
        return; // only a single CH_TXT_ATR_FORMELEMENT!
    }
    // problem: the fieldmark must be inserted in CloseFieldCommand(), because
    //          attach() takes 2 positions, not 3!
    // FAIL: AppendTextNode() ignores the content index!
    // plan B: insert a spurious paragraph break now and join
    //         it in PopFieldContext()!
    xCursor->gotoRange(xTextContent->getAnchor()->getEnd(), false);
    xCursor->goLeft(1, false); // skip CH_TXT_ATR_FIELDEND
    xTextAppend->insertControlCharacter(xCursor, text::ControlCharacter::PARAGRAPH_BREAK, false);
    xCursor->goLeft(1, false); // back to previous paragraph
    rTextAppendStack.push(TextAppendContext(xTextAppend, xCursor));
}

static auto PopFieldmark(std::stack<TextAppendContext> & rTextAppendStack,
        uno::Reference<text::XTextCursor> const& xCursor,
        std::optional<FieldId> const oFieldId) -> void
{
    if (oFieldId
        && (oFieldId == FIELD_FORMCHECKBOX || oFieldId == FIELD_FORMDROPDOWN))
    {
        return; // only a single CH_TXT_ATR_FORMELEMENT!
    }
    xCursor->gotoRange(rTextAppendStack.top().xInsertPosition, false);
    xCursor->goRight(1, true);
    xCursor->setString(OUString()); // undo SplitNode from CloseFieldCommand()
    // note: paragraph properties will be overwritten
    // by finishParagraph() anyway so ignore here
    rTextAppendStack.pop();
}

void DomainMapper_Impl::CloseFieldCommand()
{
    if(m_bDiscardHeaderFooter)
        return;
#ifdef DBG_UTIL
    TagLogger::getInstance().element("closeFieldCommand");
#endif

    FieldContextPtr pContext;
    if(!m_aFieldStack.empty())
        pContext = m_aFieldStack.back();
    OSL_ENSURE( pContext, "no field context available");
    if( !pContext )
        return;

    pContext->m_bSetUserFieldContent = false;
    pContext->m_bSetCitation = false;
    pContext->m_bSetDateValue = false;
    // tdf#124472: If the normal command line is not empty, use it,
    // otherwise, the last active row is evaluated.
    if (!pContext->GetCommandIsEmpty(false))
        pContext->SetCommandType(false);

    const FieldConversionMap_t& aFieldConversionMap = lcl_GetFieldConversion();

    try
    {
        uno::Reference< uno::XInterface > xFieldInterface;

        const auto& [sType, vArguments, vSwitches]{ splitFieldCommand(pContext->GetCommand()) };
        (void)vSwitches;
        OUString const sFirstParam(vArguments.empty() ? OUString() : vArguments.front());

        // apply character properties to the form control
        if (!m_aTextAppendStack.empty() && m_pLastCharacterContext)
        {
            uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
            if (xTextAppend.is())
            {
                uno::Reference< text::XTextCursor > xCrsr = xTextAppend->getText()->createTextCursor();
                if (xCrsr.is())
                {
                    xCrsr->gotoEnd(false);
                    uno::Reference< beans::XPropertySet > xProp( xCrsr, uno::UNO_QUERY );
                    for (auto& rPropValue : m_pLastCharacterContext->GetPropertyValues(false))
                    {
                        try
                        {
                            xProp->setPropertyValue(rPropValue.Name, rPropValue.Value);
                        }
                        catch(uno::Exception&)
                        {
                            TOOLS_WARN_EXCEPTION( "writerfilter.dmapper", "Unknown Field PropVal");
                        }
                    }
                }
            }
        }

        FieldConversionMap_t::const_iterator const aIt = aFieldConversionMap.find(sType);
        if (aIt != aFieldConversionMap.end()
            && (!m_bForceGenericFields
                // these need to convert ffData to properties...
                    || (aIt->second.eFieldId == FIELD_FORMCHECKBOX)
                    || (aIt->second.eFieldId == FIELD_FORMDROPDOWN)
                    || (aIt->second.eFieldId == FIELD_FORMTEXT)))
        {
            pContext->SetFieldId(aIt->second.eFieldId);
            bool bCreateEnhancedField = false;
            uno::Reference< beans::XPropertySet > xFieldProperties;
            bool bCreateField = true;
            switch (aIt->second.eFieldId)
            {
            case FIELD_HYPERLINK:
            case FIELD_DOCPROPERTY:
            case FIELD_TOC:
            case FIELD_INDEX:
            case FIELD_XE:
            case FIELD_BIBLIOGRAPHY:
            case FIELD_CITATION:
            case FIELD_TC:
            case FIELD_EQ:
            case FIELD_INCLUDEPICTURE:
            case FIELD_SYMBOL:
            case FIELD_GOTOBUTTON:
                    bCreateField = false;
                    break;
            case FIELD_FORMCHECKBOX :
            case FIELD_FORMTEXT :
            case FIELD_FORMDROPDOWN :
            {
                // If we use 'enhanced' fields then FIELD_FORMCHECKBOX,
                // FIELD_FORMTEXT & FIELD_FORMDROPDOWN are treated specially
                if ( m_bUsingEnhancedFields  )
                {
                    bCreateField = false;
                    bCreateEnhancedField = true;
                }
                // for non enhanced fields checkboxes are displayed
                // as an awt control not a field
                else if ( aIt->second.eFieldId == FIELD_FORMCHECKBOX )
                    bCreateField = false;
                break;
            }
            default:
            {
                FieldContextPtr pOuter = GetParentFieldContext(m_aFieldStack);
                if (pOuter)
                {
                    if (!IsFieldNestingAllowed(pOuter, m_aFieldStack.back()))
                    {
                        // Parent field can't host this child field: don't create a child field
                        // in this case.
                        bCreateField = false;
                    }
                }
                break;
            }
            }
            if (IsInTOC() && (aIt->second.eFieldId == FIELD_PAGEREF))
            {
                bCreateField = false;
            }

            if( bCreateField || bCreateEnhancedField )
            {
                //add the service prefix
                OUString sServiceName("com.sun.star.text.");
                if ( bCreateEnhancedField )
                {
                    const FieldConversionMap_t& aEnhancedFieldConversionMap = lcl_GetEnhancedFieldConversion();
                    FieldConversionMap_t::const_iterator aEnhancedIt =
                        aEnhancedFieldConversionMap.find(sType);
                    if ( aEnhancedIt != aEnhancedFieldConversionMap.end())
                        sServiceName += OUString::createFromAscii(aEnhancedIt->second.cFieldServiceName );
                }
                else
                {
                    sServiceName += "TextField." + OUString::createFromAscii(aIt->second.cFieldServiceName );
                }

#ifdef DBG_UTIL
                TagLogger::getInstance().startElement("fieldService");
                TagLogger::getInstance().chars(sServiceName);
                TagLogger::getInstance().endElement();
#endif

                if (m_xTextDocument)
                {
                    xFieldInterface = m_xTextDocument->createInstance(sServiceName);
                    xFieldProperties.set( xFieldInterface, uno::UNO_QUERY_THROW);
                }
            }
            switch( aIt->second.eFieldId )
            {
                case FIELD_ADDRESSBLOCK: break;
                case FIELD_ADVANCE     : break;
                case FIELD_ASK         :
                    handleFieldAsk(pContext, xFieldInterface, xFieldProperties);
                break;
                case FIELD_AUTONUM    :
                case FIELD_AUTONUMLGL :
                case FIELD_AUTONUMOUT :
                    handleAutoNum(pContext, xFieldInterface, xFieldProperties);
                break;
                case FIELD_AUTHOR       :
                case FIELD_USERNAME     :
                case FIELD_USERINITIALS :
                    handleAuthor(sFirstParam,
                        xFieldProperties,
                        aIt->second.eFieldId);
                break;
                case FIELD_DATE:
                if (xFieldProperties.is())
                {
                    // Get field fixed property from the context handler
                    if (pContext->IsFieldLocked())
                    {
                        xFieldProperties->setPropertyValue(
                            getPropertyName(PROP_IS_FIXED),
                            uno::Any( true ));
                        pContext->m_bSetDateValue = true;
                    }
                    else
                        xFieldProperties->setPropertyValue(
                            getPropertyName(PROP_IS_FIXED),
                            uno::Any( false ));

                    xFieldProperties->setPropertyValue(
                        getPropertyName(PROP_IS_DATE),
                        uno::Any( true ));
                    SetNumberFormat( pContext->GetCommand(), xFieldProperties );
                }
                break;
                case FIELD_COMMENTS     :
                {
                    // OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" COMMENTS") );
                    // A parameter with COMMENTS shouldn't set fixed
                    // ( or at least the binary filter doesn't )
                    // If we set fixed then we won't export a field cmd.
                    // Additionally the para in COMMENTS is more like an
                    // instruction to set the document property comments
                    // with the param ( e.g. each COMMENT with a param will
                    // overwrite the Comments document property
                    // #TODO implement the above too
                    xFieldProperties->setPropertyValue(
                        getPropertyName( PROP_IS_FIXED ), uno::Any( false ));
                        //PROP_CURRENT_PRESENTATION is set later anyway
                }
                break;
                case FIELD_CREATEDATE  :
                case FIELD_PRINTDATE:
                case FIELD_SAVEDATE:
                {
                    if (pContext->IsFieldLocked())
                    {
                        xFieldProperties->setPropertyValue(
                            getPropertyName(PROP_IS_FIXED), uno::Any( true ));
                    }
                    xFieldProperties->setPropertyValue(
                        getPropertyName( PROP_IS_DATE ), uno::Any( true ));
                    SetNumberFormat( pContext->GetCommand(), xFieldProperties );
                }
                break;
                case FIELD_DOCPROPERTY :
                    handleDocProperty(pContext, sFirstParam,
                            xFieldInterface);
                break;
                case FIELD_DOCVARIABLE  :
                {
                    if (bCreateField)
                    {
                        //create a user field and type
                        uno::Reference<beans::XPropertySet> xMaster = FindOrCreateFieldMaster(
                            "com.sun.star.text.FieldMaster.User", sFirstParam);
                        uno::Reference<text::XDependentTextField> xDependentField(
                            xFieldInterface, uno::UNO_QUERY_THROW);
                        xDependentField->attachTextFieldMaster(xMaster);
                        pContext->m_bSetUserFieldContent = true;
                    }
                }
                break;
                case FIELD_EDITTIME     :
                    //it's a numbering type, no number format! SetNumberFormat( pContext->GetCommand(), xFieldProperties );
                break;
                case FIELD_EQ:
                {
                    OUString aCommand = pContext->GetCommand().trim();

                    msfilter::util::EquationResult aResult(msfilter::util::ParseCombinedChars(aCommand));
                    if (!aResult.sType.isEmpty() && m_xTextDocument)
                    {
                        xFieldInterface = m_xTextDocument->createInstance("com.sun.star.text.TextField." + aResult.sType);
                        xFieldProperties =
                            uno::Reference< beans::XPropertySet >( xFieldInterface,
                                uno::UNO_QUERY_THROW);
                        xFieldProperties->setPropertyValue(getPropertyName(PROP_CONTENT), uno::Any(aResult.sResult));
                    }
                    else
                    {
                        //merge Read_SubF_Ruby into filter/.../util.cxx and reuse that ?
                        sal_Int32 nSpaceIndex = aCommand.indexOf(' ');
                        if(nSpaceIndex > 0)
                            aCommand = o3tl::trim(aCommand.subView(nSpaceIndex));
                        if (aCommand.startsWith("\\s"))
                        {
                            aCommand = aCommand.copy(2);
                            if (aCommand.startsWith("\\do"))
                            {
                                aCommand = aCommand.copy(3);
                                sal_Int32 nStartIndex = aCommand.indexOf('(');
                                sal_Int32 nEndIndex = aCommand.indexOf(')');
                                if (nStartIndex > 0 && nEndIndex > 0)
                                {
                                    // nDown is the requested "lower by" value in points.
                                    sal_Int32 nDown = o3tl::toInt32(aCommand.subView(0, nStartIndex));
                                    OUString aContent = aCommand.copy(nStartIndex + 1, nEndIndex - nStartIndex - 1);
                                    PropertyMapPtr pCharContext = GetTopContext();
                                    // dHeight is the font size of the current style.
                                    double dHeight = 0;
                                    if ((GetPropertyFromParaStyleSheet(PROP_CHAR_HEIGHT) >>= dHeight) && dHeight != 0)
                                        // Character escapement should be given in negative percents for subscripts.
                                        pCharContext->Insert(PROP_CHAR_ESCAPEMENT, uno::Any( sal_Int16(- 100 * nDown / dHeight) ) );
                                    appendTextPortion(aContent, pCharContext);
                                }
                            }
                        }
                        else if (aCommand.startsWith("\\* jc"))
                        {
                            handleRubyEQField(pContext);
                        }
                    }
                }
                break;
                case FIELD_FILLIN       :
                    if (xFieldProperties.is())
                        xFieldProperties->setPropertyValue(
                                getPropertyName(PROP_HINT), uno::Any( pContext->GetCommand().getToken(1, '\"')));
                break;
                case FIELD_FILENAME:
                {
                    sal_Int32 nNumberingTypeIndex = pContext->GetCommand().indexOf("\\p");
                    if (xFieldProperties.is())
                        xFieldProperties->setPropertyValue(
                                getPropertyName(PROP_FILE_FORMAT),
                                uno::Any( nNumberingTypeIndex > 0 ? text::FilenameDisplayFormat::FULL : text::FilenameDisplayFormat::NAME_AND_EXT ));
                }
                break;
                case FIELD_FILESIZE     : break;
                case FIELD_FORMULA :
                    if (bCreateField)
                    {
                        handleFieldFormula(pContext, xFieldProperties);
                    }
                break;
                case FIELD_FORMCHECKBOX :
                case FIELD_FORMDROPDOWN :
                case FIELD_FORMTEXT :
                    {
                        if (bCreateEnhancedField)
                        {
                            FFDataHandler::Pointer_t
                            pFFDataHandler(pContext->getFFDataHandler());
                            FormControlHelper::Pointer_t
                                pFormControlHelper(new FormControlHelper
                                                   (m_bUsingEnhancedFields ? aIt->second.eFieldId : FIELD_FORMCHECKBOX,

                                                    m_xTextDocument, pFFDataHandler));
                            pContext->setFormControlHelper(pFormControlHelper);
                            uno::Reference< text::XFormField > xFormField( xFieldInterface, uno::UNO_QUERY );
                            uno::Reference< container::XNamed > xNamed( xFormField, uno::UNO_QUERY );
                            if ( xNamed.is() )
                            {
                                if ( pFFDataHandler && !pFFDataHandler->getName().isEmpty() )
                                    xNamed->setName(  pFFDataHandler->getName() );
                                pContext->SetFormField( xFormField );
                            }
                            InsertFieldmark(m_aTextAppendStack,
                                xFormField, pContext->GetStartRange(),
                                pContext->GetFieldId());
                        }
                        else
                        {
                            if ( aIt->second.eFieldId == FIELD_FORMDROPDOWN )
                                lcl_handleDropdownField( xFieldProperties, pContext->getFFDataHandler() );
                            else
                                lcl_handleTextField( xFieldProperties, pContext->getFFDataHandler() );
                        }
                    }
                    break;
                case FIELD_GOTOBUTTON   : break;
                case FIELD_HYPERLINK:
                {
                    ::std::vector<OUString> aParts = pContext->GetCommandParts();

                    // Syntax is either:
                    // HYPERLINK "" \l "link"
                    // or
                    // HYPERLINK \l "link"
                    // Make sure "HYPERLINK" doesn't end up as part of link in the second case.
                    if (!aParts.empty() && aParts[0] == "HYPERLINK")
                        aParts.erase(aParts.begin());

                    ::std::vector<OUString>::const_iterator aItEnd = aParts.end();
                    ::std::vector<OUString>::const_iterator aPartIt = aParts.begin();

                    OUString sURL;
                    OUString sTarget;

                    while (aPartIt != aItEnd)
                    {
                        if ( *aPartIt == "\\l" )
                        {
                            ++aPartIt;

                            if (aPartIt == aItEnd)
                                break;

                            sURL += "#" + *aPartIt;
                        }
                        else if (*aPartIt == "\\m" || *aPartIt == "\\n" || *aPartIt == "\\h")
                        {
                        }
                        else if ( *aPartIt == "\\o" || *aPartIt == "\\t" )
                        {
                            ++aPartIt;

                            if (aPartIt == aItEnd)
                                break;

                            sTarget = *aPartIt;
                        }
                        else
                        {
                            sURL = *aPartIt;
                        }

                        ++aPartIt;
                    }

                    if (!sURL.isEmpty())
                    {
                        if (sURL.startsWith("file:///"))
                        {
                            // file:///absolute\\path\\to\\file => invalid file URI (Writer cannot open)
                            // convert all double backslashes to slashes:
                            sURL = sURL.replaceAll("\\\\", "/");

                            // file:///absolute\path\to\file => invalid file URI (Writer cannot open)
                            // convert all backslashes to slashes:
                            sURL = sURL.replace('\\', '/');
                        }
                        // Try to make absolute any relative URLs, except
                        // for relative same-document URLs that only contain
                        // a fragment part:
                        else if (!sURL.startsWith("#")) {
                            try {
                                sURL = rtl::Uri::convertRelToAbs(
                                    m_aBaseUrl, sURL);
                            } catch (rtl::MalformedUriException & e) {
                                SAL_WARN(
                                    "writerfilter.dmapper",
                                    "MalformedUriException "
                                        << e.getMessage());
                            }
                        }
                        pContext->SetHyperlinkURL(sURL);
                    }

                    if (!sTarget.isEmpty())
                        pContext->SetHyperlinkTarget(sTarget);
                }
                break;
                case FIELD_IF:
                {
                    if (vArguments.size() < 3)
                    {
                        SAL_WARN("writerfilter.dmapper", "IF field requires at least 3 parameters!");
                        break;
                    }

                    if (xFieldProperties.is())
                    {
                        // Following code assumes that last argument in field is false value
                        // before it - true value and everything before them is a condition
                        OUString sCondition;
                        size_t i = 0;
                        while (i < vArguments.size() - 2) {
                            if (!sCondition.isEmpty())
                                sCondition += " ";
                            sCondition += vArguments[i++];
                        }

                        xFieldProperties->setPropertyValue(
                            "TrueContent", uno::Any(vArguments[vArguments.size() - 2]));
                        xFieldProperties->setPropertyValue(
                            "FalseContent", uno::Any(vArguments[vArguments.size() - 1]));
                        xFieldProperties->setPropertyValue(
                            "Condition", uno::Any(sCondition));
                    }
                }
                break;
                case FIELD_INFO         : break;
                case FIELD_INCLUDEPICTURE: break;
                case FIELD_KEYWORDS     :
                {
                    if (!sFirstParam.isEmpty())
                    {
                        xFieldProperties->setPropertyValue(
                                getPropertyName( PROP_IS_FIXED ), uno::Any( true ));
                        //PROP_CURRENT_PRESENTATION is set later anyway
                    }
                }
                break;
                case FIELD_LASTSAVEDBY :
                    xFieldProperties->setPropertyValue(
                        getPropertyName(PROP_IS_FIXED), uno::Any(true));
                    break;
                case FIELD_MACROBUTTON:
                {
                    if (xFieldProperties.is())
                    {
                        sal_Int32 nIndex = sizeof(" MACROBUTTON ");
                        OUString sCommand = pContext->GetCommand();

                        //extract macro name
                        if (sCommand.getLength() >= nIndex)
                        {
                            OUString sMacro = sCommand.getToken(0, ' ', nIndex);
                            xFieldProperties->setPropertyValue(
                                    getPropertyName(PROP_MACRO_NAME), uno::Any( sMacro ));
                        }

                        //extract quick help text
                        if (sCommand.getLength() > nIndex + 1)
                        {
                            xFieldProperties->setPropertyValue(
                                getPropertyName(PROP_HINT),
                                uno::Any( sCommand.copy( nIndex )));
                        }
                    }
                }
                break;
                case FIELD_MERGEFIELD  :
                {
                    //todo: create a database field and fieldmaster pointing to a column, only
                    //create a user field and type
                    uno::Reference< beans::XPropertySet > xMaster =
                        FindOrCreateFieldMaster("com.sun.star.text.FieldMaster.Database", sFirstParam);

//                    xFieldProperties->setPropertyValue(
//                             "FieldCode",
//                             uno::makeAny( pContext->GetCommand().copy( nIndex + 1 )));
                    uno::Reference< text::XDependentTextField > xDependentField( xFieldInterface, uno::UNO_QUERY_THROW );
                    xDependentField->attachTextFieldMaster( xMaster );
                }
                break;
                case FIELD_MERGEREC     : break;
                case FIELD_MERGESEQ     : break;
                case FIELD_NEXT         : break;
                case FIELD_NEXTIF       : break;
                case FIELD_PAGE        :
                    if (xFieldProperties.is())
                    {
                        xFieldProperties->setPropertyValue(
                                getPropertyName(PROP_NUMBERING_TYPE),
                                uno::Any( lcl_ParseNumberingType(pContext->GetCommand()) ));
                        xFieldProperties->setPropertyValue(
                                getPropertyName(PROP_SUB_TYPE),
                                uno::Any( text::PageNumberType_CURRENT ));
                    }

                break;
                case FIELD_PAGEREF:
                case FIELD_REF:
                case FIELD_STYLEREF:
                if (xFieldProperties.is() && !IsInTOC())
                {
                    bool bPageRef = aIt->second.eFieldId == FIELD_PAGEREF;
                    bool bStyleRef = aIt->second.eFieldId == FIELD_STYLEREF;

                    // Do we need a GetReference (default) or a GetExpression field?
                    uno::Reference< container::XNameAccess > xFieldMasterAccess = GetTextDocument()->getTextFieldMasters();

                    if (!xFieldMasterAccess->hasByName(
                            "com.sun.star.text.FieldMaster.SetExpression."
                            + sFirstParam))
                    {
                        if (bStyleRef)
                        {
                            xFieldProperties->setPropertyValue(
                                getPropertyName(PROP_REFERENCE_FIELD_SOURCE),
                                uno::Any(sal_Int16(text::ReferenceFieldSource::STYLE)));

                            OUString sStyleSheetName
                                = GetStyleSheetTable()->ConvertStyleName(sFirstParam, true);

                            uno::Any aStyleDisplayName;

                            uno::Reference<container::XNameAccess> xStyleFamilies
                                = GetTextDocument()->getStyleFamilies();
                            uno::Reference<container::XNameAccess> xStyles;
                            xStyleFamilies->getByName(getPropertyName(PROP_PARAGRAPH_STYLES))
                                >>= xStyles;
                            uno::Reference<css::beans::XPropertySet> xStyle;

                            try
                            {
                                xStyles->getByName(sStyleSheetName) >>= xStyle;
                                aStyleDisplayName = xStyle->getPropertyValue("DisplayName");
                            }
                            catch (css::container::NoSuchElementException)
                            {
                                aStyleDisplayName <<= sStyleSheetName;
                            }

                            xFieldProperties->setPropertyValue(
                                getPropertyName(PROP_SOURCE_NAME), aStyleDisplayName);

                            sal_uInt16 nFlags = 0;
                            OUString sValue;
                            if( lcl_FindInCommand( pContext->GetCommand(), 'l', sValue ))
                            {
                                //search-below-first
                                nFlags |= REFFLDFLAG_STYLE_FROM_BOTTOM;
                            }
                            if( lcl_FindInCommand( pContext->GetCommand(), 't', sValue ))
                            {
                                //suppress-nondelimiter
                                nFlags |= REFFLDFLAG_STYLE_HIDE_NON_NUMERICAL;
                            }
                            xFieldProperties->setPropertyValue(
                                    getPropertyName( PROP_REFERENCE_FIELD_FLAGS ), uno::Any(nFlags) );
                        }
                        else
                        {
                            xFieldProperties->setPropertyValue(
                                getPropertyName(PROP_REFERENCE_FIELD_SOURCE),
                                uno::Any( sal_Int16(text::ReferenceFieldSource::BOOKMARK)) );

                            xFieldProperties->setPropertyValue(
                                getPropertyName(PROP_SOURCE_NAME),
                                uno::Any(sFirstParam));
                        }

                        sal_Int16 nFieldPart = (bPageRef ? text::ReferenceFieldPart::PAGE : text::ReferenceFieldPart::TEXT);
                        OUString sValue;
                        if( lcl_FindInCommand( pContext->GetCommand(), 'p', sValue ))
                        {
                            //above-below
                            nFieldPart = text::ReferenceFieldPart::UP_DOWN;
                        }
                        else if( lcl_FindInCommand( pContext->GetCommand(), 'r', sValue ))
                        {
                            //number
                            nFieldPart = text::ReferenceFieldPart::NUMBER;
                        }
                        else if( lcl_FindInCommand( pContext->GetCommand(), 'n', sValue ))
                        {
                            //number-no-context
                            nFieldPart = text::ReferenceFieldPart::NUMBER_NO_CONTEXT;
                        }
                        else if( lcl_FindInCommand( pContext->GetCommand(), 'w', sValue ))
                        {
                            //number-full-context
                            nFieldPart = text::ReferenceFieldPart::NUMBER_FULL_CONTEXT;
                        }
                        xFieldProperties->setPropertyValue(
                                getPropertyName( PROP_REFERENCE_FIELD_PART ), uno::Any( nFieldPart ));
                    }
                    else if( m_xTextDocument )
                    {
                        xFieldInterface = m_xTextDocument->createInstance("com.sun.star.text.TextField.GetExpression");
                        xFieldProperties.set(xFieldInterface, uno::UNO_QUERY);
                        xFieldProperties->setPropertyValue(
                            getPropertyName(PROP_CONTENT),
                            uno::Any(sFirstParam));
                        xFieldProperties->setPropertyValue(getPropertyName(PROP_SUB_TYPE), uno::Any(text::SetVariableType::STRING));
                    }
                }
                break;
                case FIELD_REVNUM       : break;
                case FIELD_SECTION      : break;
                case FIELD_SECTIONPAGES : break;
                case FIELD_SEQ          :
                {
                    // command looks like: " SEQ Table \* ARABIC "
                    OUString sCmd(pContext->GetCommand());
                    // find the sequence name, e.g. "SEQ"
                    std::u16string_view sSeqName = msfilter::util::findQuotedText(sCmd, u"SEQ ", '\\');
                    sSeqName = o3tl::trim(sSeqName);

                    // create a sequence field master using the sequence name
                    uno::Reference< beans::XPropertySet > xMaster = FindOrCreateFieldMaster(
                                "com.sun.star.text.FieldMaster.SetExpression",
                                OUString(sSeqName));

                    xMaster->setPropertyValue(
                        getPropertyName(PROP_SUB_TYPE),
                        uno::Any(text::SetVariableType::SEQUENCE));

                    // apply the numbering type
                    xFieldProperties->setPropertyValue(
                        getPropertyName(PROP_NUMBERING_TYPE),
                        uno::Any( lcl_ParseNumberingType(pContext->GetCommand()) ));

                    // attach the master to the field
                    uno::Reference< text::XDependentTextField > xDependentField( xFieldInterface, uno::UNO_QUERY_THROW );
                    xDependentField->attachTextFieldMaster( xMaster );

                    OUString sFormula = OUString::Concat(sSeqName) + "+1";
                    OUString sValue;
                    if( lcl_FindInCommand( pContext->GetCommand(), 'c', sValue ))
                    {
                        sFormula = sSeqName;
                    }
                    else if( lcl_FindInCommand( pContext->GetCommand(), 'r', sValue ))
                    {
                        sFormula = sValue;
                    }
                    // TODO \s isn't handled, but the spec isn't easy to understand without
                    // an example for this one.
                    xFieldProperties->setPropertyValue(
                            getPropertyName(PROP_CONTENT),
                            uno::Any(sFormula));

                    // Take care of the numeric formatting definition, default is Arabic
                    sal_Int16 nNumberingType = lcl_ParseNumberingType(pContext->GetCommand());
                    if (nNumberingType == style::NumberingType::PAGE_DESCRIPTOR)
                        nNumberingType = style::NumberingType::ARABIC;
                    xFieldProperties->setPropertyValue(
                            getPropertyName(PROP_NUMBERING_TYPE),
                            uno::Any(nNumberingType));
                }
                break;
                case FIELD_SET          :
                    handleFieldSet(pContext, xFieldInterface, xFieldProperties);
                break;
                case FIELD_SKIPIF       : break;
                case FIELD_SUBJECT      :
                {
                    if (!sFirstParam.isEmpty())
                    {
                        xFieldProperties->setPropertyValue(
                                getPropertyName( PROP_IS_FIXED ), uno::Any( true ));
                        //PROP_CURRENT_PRESENTATION is set later anyway
                    }
                }
                break;
                case FIELD_SYMBOL:
                {
                    uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
                    OUString sSymbol( sal_Unicode( sFirstParam.startsWithIgnoreAsciiCase("0x") ?  o3tl::toUInt32(sFirstParam.subView(2),16) : sFirstParam.toUInt32() ) );
                    OUString sFont;
                    bool bHasFont = lcl_FindInCommand( pContext->GetCommand(), 'f', sFont);
                    if ( bHasFont )
                    {
                        sFont = sFont.trim();
                        if (sFont.startsWith("\""))
                            sFont = sFont.copy(1);
                        if (sFont.endsWith("\""))
                            sFont = sFont.copy(0,sFont.getLength()-1);
                    }



                    if (xTextAppend.is())
                    {
                        uno::Reference< text::XText > xText = xTextAppend->getText();
                        uno::Reference< text::XTextCursor > xCrsr = xText->createTextCursor();
                        if (xCrsr.is())
                        {
                            xCrsr->gotoEnd(false);
                            xText->insertString(xCrsr, sSymbol, true);
                            uno::Reference< beans::XPropertySet > xProp( xCrsr, uno::UNO_QUERY );
                            xProp->setPropertyValue(getPropertyName(PROP_CHAR_FONT_CHAR_SET), uno::Any(awt::CharSet::SYMBOL));
                            if(bHasFont)
                            {
                                uno::Any    aVal( sFont );
                                xProp->setPropertyValue(getPropertyName(PROP_CHAR_FONT_NAME), aVal);
                                xProp->setPropertyValue(getPropertyName(PROP_CHAR_FONT_NAME_ASIAN), aVal);
                                xProp->setPropertyValue(getPropertyName(PROP_CHAR_FONT_NAME_COMPLEX), aVal);

                            }
                        }
                    }
                }
                break;
                case FIELD_TEMPLATE: break;
                case FIELD_TIME         :
                {
                    if (pContext->IsFieldLocked())
                    {
                        xFieldProperties->setPropertyValue(
                            getPropertyName(PROP_IS_FIXED),
                            uno::Any( true ));
                        pContext->m_bSetDateValue = true;
                    }
                    SetNumberFormat( pContext->GetCommand(), xFieldProperties );
                }
                break;
                case FIELD_TITLE        :
                {
                    if (!sFirstParam.isEmpty())
                    {
                        xFieldProperties->setPropertyValue(
                                getPropertyName( PROP_IS_FIXED ), uno::Any( true ));
                        //PROP_CURRENT_PRESENTATION is set later anyway
                    }
                }
                break;
                case FIELD_USERADDRESS  : //todo: user address collects street, city ...
                break;
                case FIELD_INDEX:
                    handleIndex(pContext,
                              OUString::createFromAscii(aIt->second.cFieldServiceName));
                    break;
                case FIELD_BIBLIOGRAPHY:
                    handleBibliography(pContext,
                              OUString::createFromAscii(aIt->second.cFieldServiceName));
                    break;
                case FIELD_TOC:
                    handleToc(pContext,
                              OUString::createFromAscii(aIt->second.cFieldServiceName));
                break;
                case FIELD_XE:
                {
                    if( !m_xTextDocument )
                        break;

                    // only UserIndexMark can handle user index types defined by \f
                    // e.g. XE "text" \f "user-index-id"
                    OUString sUserIndex;
                    OUString sFieldServiceName =
                        lcl_FindInCommand( pContext->GetCommand(), 'f', sUserIndex )
                            ? "com.sun.star.text.UserIndexMark"
                            : OUString::createFromAscii(aIt->second.cFieldServiceName);
                    uno::Reference< beans::XPropertySet > xTC(
                            m_xTextDocument->createInstance(sFieldServiceName),
                                    uno::UNO_QUERY_THROW);

                    if (!sFirstParam.isEmpty())
                    {
                        xTC->setPropertyValue(sUserIndex.isEmpty()
                                    ? OUString("PrimaryKey")
                                    : OUString("AlternativeText"),
                                uno::Any(sFirstParam));
                    }

                    sUserIndex = lcl_trim(sUserIndex);
                    if (!sUserIndex.isEmpty())
                    {
                        xTC->setPropertyValue("UserIndexName",
                                uno::Any(sUserIndex));
                    }
                    uno::Reference< text::XTextContent > xToInsert( xTC, uno::UNO_QUERY );
                    uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
                    if (xTextAppend.is())
                    {
                        uno::Reference< text::XText > xText = xTextAppend->getText();
                        uno::Reference< text::XTextCursor > xCrsr = xText->createTextCursor();
                        if (xCrsr.is())
                        {
                            xCrsr->gotoEnd(false);
                            xText->insertTextContent(uno::Reference< text::XTextRange >( xCrsr, uno::UNO_QUERY_THROW ), xToInsert, false);
                        }
                    }
                }
                    break;
                case FIELD_CITATION:
                {
                    if( !m_xTextDocument )
                        break;

                    xFieldInterface = m_xTextDocument->createInstance(
                              OUString::createFromAscii(aIt->second.cFieldServiceName));
                    uno::Reference< beans::XPropertySet > xTC(xFieldInterface,
                              uno::UNO_QUERY_THROW);
                    OUString sCmd(pContext->GetCommand());//sCmd is the entire instrText including the index e.g. CITATION Kra06 \l 1033
                    if( !sCmd.isEmpty()){
                        uno::Sequence<beans::PropertyValue> aValues( comphelper::InitPropertySequence({
                            { "Identifier", uno::Any(sCmd) }
                        }));
                        xTC->setPropertyValue("Fields", uno::Any(aValues));
                    }
                    uno::Reference< text::XTextContent > xToInsert( xTC, uno::UNO_QUERY );

                    uno::Sequence<beans::PropertyValue> aValues
                        = m_aFieldStack.back()->getProperties()->GetPropertyValues();
                    appendTextContent(xToInsert, aValues);
                    pContext->m_bSetCitation = true;
                }
                break;

                case FIELD_TC :
                {
                    if( !m_xTextDocument )
                        break;

                    uno::Reference< beans::XPropertySet > xTC(
                        m_xTextDocument->createInstance(
                            OUString::createFromAscii(aIt->second.cFieldServiceName)),
                            uno::UNO_QUERY_THROW);
                    if (!sFirstParam.isEmpty())
                    {
                        xTC->setPropertyValue(getPropertyName(PROP_ALTERNATIVE_TEXT),
                            uno::Any(sFirstParam));
                    }
                    OUString sValue;
                    // \f TC entry in doc with multiple tables
//                    if( lcl_FindInCommand( pContext->GetCommand(), 'f', sValue ))
//                    {
                        // todo: unsupported
//                    }
                    if( lcl_FindInCommand( pContext->GetCommand(), 'l', sValue ))
                    // \l Outline Level
                    {
                        sal_Int32 nLevel = sValue.toInt32();
                        if( !sValue.isEmpty() && nLevel >= 0 && nLevel <= 10 )
                            xTC->setPropertyValue(getPropertyName(PROP_LEVEL), uno::Any( static_cast<sal_Int16>(nLevel) ));
                    }
//                    if( lcl_FindInCommand( pContext->GetCommand(), 'n', sValue ))
//                    \n Suppress page numbers
//                    {
                        //todo: unsupported feature
//                    }
                    pContext->SetTC( xTC );
                }
                break;
                case  FIELD_NUMCHARS:
                case  FIELD_NUMWORDS:
                case  FIELD_NUMPAGES:
                if (xFieldProperties.is())
                    xFieldProperties->setPropertyValue(
                        getPropertyName(PROP_NUMBERING_TYPE),
                        uno::Any( lcl_ParseNumberingType(pContext->GetCommand()) ));
                break;
            }

            if (!bCreateEnhancedField)
            {
                pContext->SetTextField( uno::Reference<text::XTextField>(xFieldInterface, uno::UNO_QUERY) );
            }
        }
        else
        {
            /* Unsupported fields will be handled here for docx file.
             * To handle unsupported fields used fieldmark API.
             */
            OUString aCode( pContext->GetCommand().trim() );
            // Don't waste resources on wrapping shapes inside a fieldmark.
            if (sType != "SHAPE" && m_xTextDocument && !m_aTextAppendStack.empty())
            {
                xFieldInterface = m_xTextDocument->createInstance("com.sun.star.text.Fieldmark");

                uno::Reference<text::XFormField> const xFormField(xFieldInterface, uno::UNO_QUERY);
                InsertFieldmark(m_aTextAppendStack, xFormField, pContext->GetStartRange(),
                        pContext->GetFieldId());
                xFormField->setFieldType(ODF_UNHANDLED);
                ++m_nStartGenericField;
                pContext->SetFormField( xFormField );
                uno::Reference<container::XNameContainer> const xNameCont(xFormField->getParameters());
                // note: setting the code to empty string is *required* in
                // m_bForceGenericFields mode, or the export will write
                // the ODF_UNHANDLED string!
                assert(!m_bForceGenericFields || aCode.isEmpty());
                xNameCont->insertByName(ODF_CODE_PARAM, uno::Any(aCode));
                ww::eField const id(GetWW8FieldId(sType));
                if (id != ww::eNONE)
                {   // tdf#129247 tdf#134264 set WW8 id for WW8 export
                    xNameCont->insertByName(ODF_ID_PARAM, uno::Any(OUString::number(id)));
                }
            }
            else
                m_StreamStateStack.top().bParaHadField = false;
        }
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION( "writerfilter.dmapper", "Exception in CloseFieldCommand()" );
    }
    pContext->SetCommandCompleted();
}
/*-------------------------------------------------------------------------
//the _current_ fields require a string type result while TOCs accept richt results
  -----------------------------------------------------------------------*/
bool DomainMapper_Impl::IsFieldResultAsString()
{
    bool bRet = false;
    OSL_ENSURE( !m_aFieldStack.empty(), "field stack empty?");
    FieldContextPtr pContext = m_aFieldStack.back();
    OSL_ENSURE( pContext, "no field context available");
    if( pContext )
    {
        bRet = pContext->GetTextField().is()
            || pContext->GetFieldId() == FIELD_FORMDROPDOWN
            || pContext->GetFieldId() == FIELD_FILLIN;
    }

    if (!bRet)
    {
        FieldContextPtr pOuter = GetParentFieldContext(m_aFieldStack);
        if (pOuter)
        {
            if (!IsFieldNestingAllowed(pOuter, m_aFieldStack.back()))
            {
                // If nesting is not allowed, then the result can only be a string.
                bRet = true;
            }
        }
    }
    return bRet;
}

void DomainMapper_Impl::AppendFieldResult(std::u16string_view rString)
{
    assert(!m_aFieldStack.empty());
    FieldContextPtr pContext = m_aFieldStack.back();
    SAL_WARN_IF(!pContext, "writerfilter.dmapper", "no field context");
    if (!pContext)
        return;

    FieldContextPtr pOuter = GetParentFieldContext(m_aFieldStack);
    if (pOuter)
    {
        if (!IsFieldNestingAllowed(pOuter, pContext))
        {
            if (pOuter->IsCommandCompleted())
            {
                // Child can't host the field result, forward to parent's result.
                pOuter->AppendResult(rString);
            }
            return;
        }
    }

    pContext->AppendResult(rString);
}

// Calculates css::DateTime based on ddddd.sssss since 1899-12-30
static util::DateTime lcl_dateTimeFromSerial(const double& dSerial)
{
    DateTime d(Date(30, 12, 1899));
    d.AddTime(dSerial);
    return d.GetUNODateTime();
}

void DomainMapper_Impl::SetFieldResult(OUString const& rResult)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("setFieldResult");
    TagLogger::getInstance().chars(rResult);
#endif

    FieldContextPtr pContext = m_aFieldStack.back();
    OSL_ENSURE( pContext, "no field context available");

    if (m_aFieldStack.size() > 1)
    {
        // This is a nested field. See if the parent supports nesting on the Writer side.
        FieldContextPtr pParentContext = m_aFieldStack[m_aFieldStack.size() - 2];
        if (pParentContext)
        {
            std::vector<OUString> aParentParts = pParentContext->GetCommandParts();
            // Conditional text fields don't support nesting in Writer.
            if (!aParentParts.empty() && aParentParts[0] == "IF")
            {
                return;
            }
        }
    }

    if( !pContext )
        return;

    uno::Reference<text::XTextField> xTextField = pContext->GetTextField();
    try
    {
        OSL_ENSURE( xTextField.is()
        //||m_xTOC.is() ||m_xTC.is()
        //||m_sHyperlinkURL.getLength()
        , "DomainMapper_Impl::SetFieldResult: field not created" );
        if(xTextField.is())
        {
            try
            {
                if (pContext->m_bSetUserFieldContent)
                {
                    // user field content has to be set at the field master
                    uno::Reference< text::XDependentTextField > xDependentField( xTextField, uno::UNO_QUERY_THROW );
                    xDependentField->getTextFieldMaster()->setPropertyValue(
                            getPropertyName(PROP_CONTENT),
                         uno::Any( rResult ));
                }
                else if (pContext->m_bSetCitation)
                {

                    uno::Reference< beans::XPropertySet > xFieldProperties( xTextField, uno::UNO_QUERY_THROW);
                    // In case of SetExpression, the field result contains the content of the variable.
                    uno::Reference<lang::XServiceInfo> xServiceInfo(xTextField, uno::UNO_QUERY);

                    bool bIsSetbiblio = xServiceInfo->supportsService("com.sun.star.text.TextField.Bibliography");
                    if( bIsSetbiblio )
                    {
                        uno::Any aProperty  = xFieldProperties->getPropertyValue("Fields");
                        uno::Sequence<beans::PropertyValue> aValues ;
                        aProperty >>= aValues;
                        beans::PropertyValue propertyVal;
                        sal_Int32 nTitleFoundIndex = -1;
                        for (sal_Int32 i = 0; i < aValues.getLength(); ++i)
                        {
                            propertyVal = aValues[i];
                            if (propertyVal.Name == "Title")
                            {
                                nTitleFoundIndex = i;
                                break;
                            }
                        }
                        if (nTitleFoundIndex != -1)
                        {
                            OUString titleStr;
                            uno::Any aValue(propertyVal.Value);
                            aValue >>= titleStr;
                            titleStr += rResult;
                            propertyVal.Value <<= titleStr;
                            aValues.getArray()[nTitleFoundIndex] = propertyVal;
                        }
                        else
                        {
                            aValues.realloc(aValues.getLength() + 1);
                            propertyVal.Name = "Title";
                            propertyVal.Value <<= rResult;
                            aValues.getArray()[aValues.getLength() - 1] = propertyVal;
                        }
                        xFieldProperties->setPropertyValue("Fields",
                                uno::Any(aValues));
                    }
                }
                else if (pContext->m_bSetDateValue)
                {
                    uno::Reference< util::XNumberFormatsSupplier > xNumberSupplier( static_cast<cppu::OWeakObject*>(m_xTextDocument.get()), uno::UNO_QUERY_THROW );

                    uno::Reference<util::XNumberFormatter> xFormatter(util::NumberFormatter::create(m_xComponentContext), uno::UNO_QUERY_THROW);
                    xFormatter->attachNumberFormatsSupplier( xNumberSupplier );
                    sal_Int32 nKey = 0;

                    uno::Reference< beans::XPropertySet > xFieldProperties( xTextField, uno::UNO_QUERY_THROW);

                    xFieldProperties->getPropertyValue( "NumberFormat" ) >>= nKey;
                    xFieldProperties->setPropertyValue(
                        "DateTimeValue",
                        uno::Any( lcl_dateTimeFromSerial( xFormatter->convertStringToNumber( nKey, rResult ) ) ) );
                }
                else
                {
                    uno::Reference< beans::XPropertySet > xFieldProperties( xTextField, uno::UNO_QUERY_THROW);
                    // In case of SetExpression, and Input fields the field result contains the content of the variable.
                    uno::Reference<lang::XServiceInfo> xServiceInfo(xTextField, uno::UNO_QUERY);
                    // there are fields with a content property, which aren't working correctly with
                    // a generalized try catch of the content, property, so just restrict content
                    // handling to these explicit services.
                    const bool bHasContent = xServiceInfo->supportsService("com.sun.star.text.TextField.SetExpression") ||
                        xServiceInfo->supportsService("com.sun.star.text.TextField.Input");
                    // If we already have content set, then use the current presentation
                    OUString sValue;
                    if (bHasContent)
                    {
                        // this will throw for field types without Content
                        uno::Any aValue(xFieldProperties->getPropertyValue(
                                getPropertyName(PROP_CONTENT)));
                        aValue >>= sValue;
                    }
                    xFieldProperties->setPropertyValue(
                            getPropertyName(bHasContent && sValue.isEmpty()? PROP_CONTENT : PROP_CURRENT_PRESENTATION),
                         uno::Any( rResult ));

                    // LO always automatically updates a DocInfo field from the File-Properties-Custom Prop
                    // while MS Word requires the user to manually refresh the field (with F9).
                    // In other words, Word lets the field to be out of sync with the controlling variable.
                    // Marking as FIXEDFLD solves the automatic replacement problem, but of course prevents
                    // Writer from making any changes, even on an F9 refresh.
                    OUString sVariable = pContext->GetVariableValue();
                    if (rResult.getLength() != sVariable.getLength())
                    {
                        sal_Int32 nLen = sVariable.indexOf('\x0');
                        if (nLen >= 0)
                            sVariable = sVariable.copy(0, nLen);
                    }
                    bool bCustomFixedField = rResult != sVariable &&
                        xServiceInfo->supportsService("com.sun.star.text.TextField.DocInfo.Custom");

                    if (bCustomFixedField || xServiceInfo->supportsService(
                            "com.sun.star.text.TextField.DocInfo.CreateDateTime"))
                    {
                        // Creation time is const, don't try to update it.
                        xFieldProperties->setPropertyValue("IsFixed", uno::Any(true));
                    }
                }
            }
            catch( const beans::UnknownPropertyException& )
            {
                //some fields don't have a CurrentPresentation (DateTime)
            }
        }
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("writerfilter.dmapper", "DomainMapper_Impl::SetFieldResult");
    }
}

void DomainMapper_Impl::SetFieldFFData(const FFDataHandler::Pointer_t& pFFDataHandler)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("setFieldFFData");
#endif

    if (!m_aFieldStack.empty())
    {
        FieldContextPtr pContext = m_aFieldStack.back();
        if (pContext)
        {
            pContext->setFFDataHandler(pFFDataHandler);
        }
    }

#ifdef DBG_UTIL
    TagLogger::getInstance().endElement();
#endif
}

void DomainMapper_Impl::PopFieldContext()
{
    if(m_bDiscardHeaderFooter)
        return;
#ifdef DBG_UTIL
    TagLogger::getInstance().element("popFieldContext");
#endif

    if (m_aFieldStack.empty())
        return;

    FieldContextPtr pContext = m_aFieldStack.back();
    OSL_ENSURE( pContext, "no field context available");
    if( pContext )
    {
        if( !pContext->IsCommandCompleted() )
            CloseFieldCommand();

        if (!pContext->GetResult().isEmpty())
        {
           uno::Reference< beans::XPropertySet > xFieldProperties = pContext->GetCustomField();
           if(xFieldProperties.is())
              SetNumberFormat( pContext->GetResult(), xFieldProperties, true );
           SetFieldResult( pContext->GetResult() );
        }

        //insert the field, TC or TOC
        uno::Reference< text::XTextAppend >  xTextAppend;
        if (!m_aTextAppendStack.empty())
            xTextAppend = m_aTextAppendStack.top().xTextAppend;
        if(xTextAppend.is())
        {
            try
            {
                uno::Reference< text::XTextContent > xToInsert( pContext->GetTOC(), uno::UNO_QUERY );
                if( xToInsert.is() )
                {
                    if (m_bStartedTOC || m_bStartIndex || m_bStartBibliography)
                    {
                        // inside SDT, last empty paragraph is also part of index
                        if (!m_StreamStateStack.top().bParaChanged && !m_StreamStateStack.top().xSdtEntryStart)
                        {
                            // End of index is the first item on a new paragraph - this paragraph
                            // should not be part of index
                            auto xCursor
                                = xTextAppend->createTextCursorByRange(
                                    m_aTextAppendStack.top().xInsertPosition.is()
                                    ? m_aTextAppendStack.top().xInsertPosition
                                    : xTextAppend->getEnd());
                            xCursor->goLeft(1, true);
                            // delete
                            xCursor->setString(OUString());
                            // But a new paragraph should be started after the index instead
                            if (m_bIsNewDoc) // this check - see testTdf129402
                            {   // where finishParagraph inserts between 2 EndNode
                                xTextAppend->finishParagraph(css::beans::PropertyValues());
                            }
                            else
                            {
                                xTextAppend->finishParagraphInsert(css::beans::PropertyValues(),
                                    m_aTextAppendStack.top().xInsertPosition);
                            }
                        }
                        m_bStartedTOC = false;
                        m_aTextAppendStack.pop();
                        m_StreamStateStack.top().bTextInserted = false;
                        m_StreamStateStack.top().bParaChanged = true; // the paragraph must stay anyway
                    }
                    m_bStartTOC = false;
                    m_bStartIndex = false;
                    m_bStartBibliography = false;
                    if (IsInHeaderFooter() && m_bStartTOCHeaderFooter)
                        m_bStartTOCHeaderFooter = false;
                }
                else
                {
                    xToInsert.set(pContext->GetTC(), uno::UNO_QUERY);
                    if (!xToInsert.is() && !IsInTOC() && !m_bStartIndex && !m_bStartBibliography)
                        xToInsert = pContext->GetTextField();
                    if (xToInsert.is() && !IsInTOC() && !m_bStartIndex && !m_bStartBibliography)
                    {
                        PropertyMap aMap;
                        // Character properties of the field show up here the
                        // last (always empty) run. Inherit character
                        // properties from there.
                        // Also merge in the properties from the field context,
                        // e.g. SdtEndBefore.
                        if (m_pLastCharacterContext)
                            aMap.InsertProps(m_pLastCharacterContext);
                        aMap.InsertProps(m_aFieldStack.back()->getProperties());
                        appendTextContent(xToInsert, aMap.GetPropertyValues());
                        CheckRedline( xToInsert->getAnchor( ) );
                    }
                    else
                    {
                        uno::Reference< text::XTextCursor > xCrsr = xTextAppend->createTextCursorByRange(pContext->GetStartRange());
                        FormControlHelper::Pointer_t pFormControlHelper(pContext->getFormControlHelper());
                        if (pFormControlHelper)
                        {
                            // xCrsr may be empty e.g. when pContext->GetStartRange() is outside of
                            // xTextAppend, like when a field started in a parent paragraph is being
                            // closed inside an anchored text box. It could be possible to throw an
                            // exception here, and abort import, but Word tolerates such invalid
                            // input, so it makes sense to do the same (tdf#152200)
                            if (xCrsr.is())
                            {
                                uno::Reference< text::XFormField > xFormField(pContext->GetFormField());
                                if (pFormControlHelper->hasFFDataHandler())
                                {
                                    xToInsert.set(xFormField, uno::UNO_QUERY);
                                    if (xFormField.is() && xToInsert.is())
                                    {
                                        PopFieldmark(m_aTextAppendStack, xCrsr,
                                                     pContext->GetFieldId());
                                        pFormControlHelper->processField(xFormField);
                                    }
                                    else
                                    {
                                        pFormControlHelper->insertControl(xCrsr);
                                    }
                                }
                                else
                                {
                                    PopFieldmark(m_aTextAppendStack, xCrsr,
                                        pContext->GetFieldId());
                                    uno::Reference<lang::XComponent>(xFormField, uno::UNO_QUERY_THROW)->dispose(); // presumably invalid?
                                }
                            }
                        }
                        else if (!pContext->GetHyperlinkURL().isEmpty() && xCrsr.is())
                        {
                            if (m_aTextAppendStack.top().xInsertPosition.is())
                            {
                                xCrsr->gotoRange(m_aTextAppendStack.top().xInsertPosition, true);
                            }
                            else
                            {
                                xCrsr->gotoEnd(true);
                            }

                            // Draw components (like comments) need hyperlinks set differently
                            SvxUnoTextRangeBase* pDrawText = dynamic_cast<SvxUnoTextRangeBase*>(xCrsr.get());
                            if ( pDrawText )
                                pDrawText->attachField( std::make_unique<SvxURLField>(pContext->GetHyperlinkURL(), xCrsr->getString(), SvxURLFormat::AppDefault) );
                            else
                            {
                                uno::Reference< beans::XPropertySet > xCrsrProperties( xCrsr, uno::UNO_QUERY_THROW );
                                xCrsrProperties->setPropertyValue(getPropertyName(PROP_HYPER_LINK_U_R_L), uno::
                                                                  Any(pContext->GetHyperlinkURL()));

                                if (!pContext->GetHyperlinkTarget().isEmpty())
                                    xCrsrProperties->setPropertyValue("HyperLinkTarget", uno::Any(pContext->GetHyperlinkTarget()));

                                if (IsInTOC())
                                {
                                    OUString sDisplayName("Index Link");
                                    xCrsrProperties->setPropertyValue("VisitedCharStyleName",uno::Any(sDisplayName));
                                    xCrsrProperties->setPropertyValue("UnvisitedCharStyleName",uno::Any(sDisplayName));
                                }
                                else if (!pContext->GetHyperlinkStyle().isEmpty())
                                {
                                    uno::Any aAny = xCrsrProperties->getPropertyValue("CharStyleName");
                                    OUString charStyle;
                                    if (css::uno::fromAny(aAny, &charStyle))
                                    {
                                        if (!charStyle.isEmpty() && charStyle.equalsIgnoreAsciiCase("Internet Link"))
                                        {
                                            xCrsrProperties->setPropertyValue("CharStyleName", uno::Any(OUString("Default Style")));
                                        }
                                        else
                                        {
                                            xCrsrProperties->setPropertyValue("VisitedCharStyleName", uno::Any(pContext->GetHyperlinkStyle()));
                                            xCrsrProperties->setPropertyValue("UnvisitedCharStyleName", uno::Any(pContext->GetHyperlinkStyle()));

                                        }
                                    }
                                }
                            }
                        }
                        else if (m_nStartGenericField != 0)
                        {
                            --m_nStartGenericField;
                            PopFieldmark(m_aTextAppendStack, xCrsr, pContext->GetFieldId());
                            if (m_StreamStateStack.top().bTextInserted)
                            {
                                m_StreamStateStack.top().bTextInserted = false;
                            }
                        }
                    }
                }
            }
            catch(const lang::IllegalArgumentException&)
            {
                TOOLS_WARN_EXCEPTION( "writerfilter", "PopFieldContext()" );
            }
            catch(const uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION( "writerfilter", "PopFieldContext()" );
            }
        }

        //TOCs have to include all the imported content
    }

    std::vector<FieldParagraph> aParagraphsToFinish;
    if (pContext)
    {
        aParagraphsToFinish = pContext->GetParagraphsToFinish();
    }

    //remove the field context
    m_aFieldStack.pop_back();

    // Finish the paragraph(s) now that the field is closed.
    for (const auto& rFinish : aParagraphsToFinish)
    {
        finishParagraph(rFinish.m_pPropertyMap, rFinish.m_bRemove);
    }
}


void DomainMapper_Impl::SetBookmarkName( const OUString& rBookmarkName )
{
    BookmarkMap_t::iterator aBookmarkIter = m_aBookmarkMap.find( m_sCurrentBkmkId );
    if( aBookmarkIter != m_aBookmarkMap.end() )
    {
        // fields are internal bookmarks: consume redundant "normal" bookmark
        if ( IsOpenField() )
        {
            FFDataHandler::Pointer_t  pFFDataHandler(GetTopFieldContext()->getFFDataHandler());
            if (pFFDataHandler && pFFDataHandler->getName() == rBookmarkName)
            {
                // HACK: At the END marker, StartOrEndBookmark will START
                // a bookmark which will eventually be abandoned, not created.
                m_aBookmarkMap.erase(aBookmarkIter);
                return;
            }
        }

        if ((m_sCurrentBkmkPrefix == "__RefMoveFrom__"
             || m_sCurrentBkmkPrefix == "__RefMoveTo__")
            && std::find(m_aRedlineMoveIDs.begin(), m_aRedlineMoveIDs.end(), rBookmarkName)
                   == m_aRedlineMoveIDs.end())
        {
            m_aRedlineMoveIDs.push_back(rBookmarkName);
        }

        aBookmarkIter->second.m_sBookmarkName = m_sCurrentBkmkPrefix + rBookmarkName;
        m_sCurrentBkmkPrefix.clear();
    }
    else
    {
        m_sCurrentBkmkName = rBookmarkName;
        m_sCurrentBkmkPrefix.clear();
    }
}

// This method was used as-is for DomainMapper_Impl::startOrEndPermissionRange() implementation.
void DomainMapper_Impl::StartOrEndBookmark( const OUString& rId )
{
    /*
     * Add the dummy paragraph to handle section properties
     * iff the first element in the section is a table. If the dummy para is not added yet, then add it;
     * So bookmark is not attached to the wrong paragraph.
     */
    if (hasTableManager() && getTableManager().isInCell()
        && m_StreamStateStack.top().nTableDepth == 0
        && GetIsFirstParagraphInSection()
        && !GetIsDummyParaAddedForTableInSection() && !GetIsTextFrameInserted())
    {
        AddDummyParaForTableInSection();
    }

    bool bIsAfterDummyPara = GetIsDummyParaAddedForTableInSection() && GetIsFirstParagraphInSection();
    if (m_aTextAppendStack.empty())
        return;
    uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
    BookmarkMap_t::iterator aBookmarkIter = m_aBookmarkMap.find( rId );
    //is the bookmark name already registered?
    try
    {
        if( aBookmarkIter != m_aBookmarkMap.end() )
        {
            if (m_xTextDocument)
            {
                rtl::Reference<SwXBookmark> xBookmark( m_xTextDocument->createBookmark() );
                uno::Reference< text::XTextCursor > xCursor;
                uno::Reference< text::XText > xText = aBookmarkIter->second.m_xTextRange->getText();
                if( aBookmarkIter->second.m_bIsStartOfText && !bIsAfterDummyPara)
                {
                    xCursor = xText->createTextCursorByRange( xText->getStart() );
                }
                else
                {
                    xCursor = xText->createTextCursorByRange( aBookmarkIter->second.m_xTextRange );
                }
                if (!aBookmarkIter->second.m_bIsStartOfText)
                {
                    xCursor->goRight( 1, false );
                }

                xCursor->gotoRange( xTextAppend->getEnd(), true );
                // A Paragraph was recently finished, and a new Paragraph has not been started as yet
                // then  move the bookmark-End to the earlier paragraph
                if (IsOutsideAParagraph())
                {
                    // keep bookmark range, if it doesn't exceed cell boundary
                    uno::Reference< text::XTextRange > xStart = xCursor->getStart();
                    xCursor->goLeft( 1, false );
                    if (m_StreamStateStack.top().nTableDepth == 0
                        || !m_StreamStateStack.top().bFirstParagraphInCell)
                    {
                        xCursor->gotoRange(xStart, true );
                    }
                }
                SAL_WARN_IF(aBookmarkIter->second.m_sBookmarkName.isEmpty(), "writerfilter.dmapper", "anonymous bookmark");
                //todo: make sure the name is not used already!
                xBookmark->setName( aBookmarkIter->second.m_sBookmarkName );
                xTextAppend->insertTextContent( uno::Reference< text::XTextRange >( xCursor, uno::UNO_QUERY_THROW), xBookmark, !xCursor->isCollapsed() );
            }
            m_aBookmarkMap.erase( aBookmarkIter );
            m_sCurrentBkmkId.clear();
        }
        else
        {
            //otherwise insert a text range as marker
            bool bIsStart = true;
            uno::Reference< text::XTextRange > xCurrent;
            if (xTextAppend.is())
            {
                uno::Reference<text::XTextCursor> const xCursor =
                    xTextAppend->createTextCursorByRange(
                        m_aTextAppendStack.top().xInsertPosition.is()
                            ? m_aTextAppendStack.top().xInsertPosition
                            : xTextAppend->getEnd() );

                if (!xCursor)
                    return;

                if (!bIsAfterDummyPara)
                    bIsStart = !xCursor->goLeft(1, false);
                xCurrent = xCursor->getStart();
            }
            m_sCurrentBkmkId = rId;
            m_aBookmarkMap.emplace( rId, BookmarkInsertPosition( bIsStart, m_sCurrentBkmkName, xCurrent ) );
            m_sCurrentBkmkName.clear();
        }
    }
    catch( const uno::Exception& )
    {
        //TODO: What happens to bookmarks where start and end are at different XText objects?
    }
}

void DomainMapper_Impl::SetMoveBookmark( bool bIsFrom )
{
    static constexpr OUStringLiteral MoveFrom_Bookmark_NamePrefix = u"__RefMoveFrom__";
    static constexpr OUStringLiteral MoveTo_Bookmark_NamePrefix = u"__RefMoveTo__";
    if ( bIsFrom )
        m_sCurrentBkmkPrefix = MoveFrom_Bookmark_NamePrefix;
    else
        m_sCurrentBkmkPrefix = MoveTo_Bookmark_NamePrefix;
}

void DomainMapper_Impl::setPermissionRangeEd(const OUString& user)
{
    PermMap_t::iterator aPremIter = m_aPermMap.find(m_sCurrentPermId);
    if (aPremIter != m_aPermMap.end())
        aPremIter->second.m_Ed = user;
    else
        m_sCurrentPermEd = user;
}

void DomainMapper_Impl::setPermissionRangeEdGrp(const OUString& group)
{
    PermMap_t::iterator aPremIter = m_aPermMap.find(m_sCurrentPermId);
    if (aPremIter != m_aPermMap.end())
        aPremIter->second.m_EdGrp = group;
    else
        m_sCurrentPermEdGrp = group;
}

// This method is based on implementation from DomainMapper_Impl::StartOrEndBookmark()
void DomainMapper_Impl::startOrEndPermissionRange(sal_Int32 permissinId)
{
    /*
    * Add the dummy paragraph to handle section properties
    * if the first element in the section is a table. If the dummy para is not added yet, then add it;
    * So permission is not attached to the wrong paragraph.
    */
    if (getTableManager().isInCell()
        && m_StreamStateStack.top().nTableDepth == 0 && GetIsFirstParagraphInSection()
        && !GetIsDummyParaAddedForTableInSection() && !GetIsTextFrameInserted())
    {
        AddDummyParaForTableInSection();
    }

    if (m_aTextAppendStack.empty())
        return;

    const bool bIsAfterDummyPara = GetIsDummyParaAddedForTableInSection() && GetIsFirstParagraphInSection();

    uno::Reference< text::XTextAppend > xTextAppend = m_aTextAppendStack.top().xTextAppend;
    PermMap_t::iterator aPermIter = m_aPermMap.find(permissinId);

    //is the bookmark name already registered?
    try
    {
        if (aPermIter == m_aPermMap.end())
        {
            //otherwise insert a text range as marker
            bool bIsStart = true;
            uno::Reference< text::XTextRange > xCurrent;
            if (xTextAppend.is())
            {
                uno::Reference< text::XTextCursor > xCursor = xTextAppend->createTextCursorByRange(xTextAppend->getEnd());

                if (!bIsAfterDummyPara)
                    bIsStart = !xCursor->goLeft(1, false);
                xCurrent = xCursor->getStart();
            }

            // register the start of the new permission
            m_sCurrentPermId = permissinId;
            m_aPermMap.emplace(permissinId, PermInsertPosition(bIsStart, permissinId, m_sCurrentPermEd, m_sCurrentPermEdGrp, xCurrent));

            // clean up
            m_sCurrentPermEd.clear();
            m_sCurrentPermEdGrp.clear();
        }
        else
        {
            if (m_xTextDocument)
            {
                uno::Reference< text::XTextCursor > xCursor;
                uno::Reference< text::XText > xText = aPermIter->second.m_xTextRange->getText();
                if (aPermIter->second.m_bIsStartOfText && !bIsAfterDummyPara)
                {
                    xCursor = xText->createTextCursorByRange(xText->getStart());
                }
                else
                {
                    xCursor = xText->createTextCursorByRange(aPermIter->second.m_xTextRange);
                }
                if (!aPermIter->second.m_bIsStartOfText)
                {
                    xCursor->goRight(1, false);
                }

                xCursor->gotoRange(xTextAppend->getEnd(), true);
                // A Paragraph was recently finished, and a new Paragraph has not been started as yet
                // then  move the bookmark-End to the earlier paragraph
                if (IsOutsideAParagraph())
                {
                    xCursor->goLeft(1, false);
                }

                // create a new bookmark using specific bookmark name pattern for permissions
                rtl::Reference< SwXBookmark > xPerm(m_xTextDocument->createBookmark());
                xPerm->setName(aPermIter->second.createBookmarkName());

                // add new bookmark
                const bool bAbsorb = !xCursor->isCollapsed();
                uno::Reference< text::XTextRange > xCurrent(xCursor, uno::UNO_QUERY_THROW);
                xTextAppend->insertTextContent(xCurrent, xPerm, bAbsorb);
            }

            // remove processed permission
            m_aPermMap.erase(aPermIter);

            // clean up
            m_sCurrentPermId = 0;
            m_sCurrentPermEd.clear();
            m_sCurrentPermEdGrp.clear();
        }
    }
    catch (const uno::Exception&)
    {
        //TODO: What happens to bookmarks where start and end are at different XText objects?
    }
}

void DomainMapper_Impl::AddAnnotationPosition(
    const bool bStart,
    const sal_Int32 nAnnotationId)
{
    if (m_aTextAppendStack.empty())
        return;

    // Create a cursor, pointing to the current position.
    uno::Reference<text::XTextAppend>  xTextAppend = m_aTextAppendStack.top().xTextAppend;
    uno::Reference<text::XTextRange> xCurrent;
    if (xTextAppend.is())
    {
        uno::Reference<text::XTextCursor> xCursor;
        if (m_bIsNewDoc)
            xCursor = xTextAppend->createTextCursorByRange(xTextAppend->getEnd());
        else
            xCursor = m_aTextAppendStack.top().xCursor;
        if (xCursor.is())
            xCurrent = xCursor->getStart();
    }

    // And save it, to be used by PopAnnotation() later.
    AnnotationPosition& aAnnotationPosition = m_aAnnotationPositions[ nAnnotationId ];
    if (bStart)
    {
        aAnnotationPosition.m_xStart = xCurrent;
    }
    else
    {
        aAnnotationPosition.m_xEnd = xCurrent;
    }
    m_aAnnotationPositions[ nAnnotationId ] = aAnnotationPosition;
}

GraphicImportPtr const & DomainMapper_Impl::GetGraphicImport()
{
    if(!m_pGraphicImport)
    {
        m_pGraphicImport = new GraphicImport(m_xComponentContext, m_xTextDocument, m_rDMapper, m_eGraphicImportType, m_aPositionOffsets, m_aAligns, m_aPositivePercentages);
    }
    return m_pGraphicImport;
}
/*-------------------------------------------------------------------------
    reset graphic import if the last import resulted in a shape, not a graphic
  -----------------------------------------------------------------------*/
void DomainMapper_Impl::ResetGraphicImport()
{
    m_pGraphicImport.clear();
}


void  DomainMapper_Impl::ImportGraphic(const writerfilter::Reference<Properties>::Pointer_t& ref)
{
    GetGraphicImport();
    if (m_eGraphicImportType != IMPORT_AS_DETECTED_INLINE && m_eGraphicImportType != IMPORT_AS_DETECTED_ANCHOR)
    {   // this appears impossible?
        //create the graphic
        ref->resolve( *m_pGraphicImport );
    }

    //insert it into the document at the current cursor position

    uno::Reference<text::XTextContent> xTextContent
        (m_pGraphicImport->GetGraphicObject());

    // In case the SDT starts with the text portion of the graphic, then set the SDT properties here.
    bool bHasGrabBag = false;
    uno::Reference<beans::XPropertySet> xPropertySet(xTextContent, uno::UNO_QUERY);
    if (xPropertySet.is())
    {
        uno::Reference<beans::XPropertySetInfo> xPropertySetInfo = xPropertySet->getPropertySetInfo();
        bHasGrabBag = xPropertySetInfo->hasPropertyByName("FrameInteropGrabBag");
        // In case we're outside a paragraph, then the SDT properties are stored in the paragraph grab-bag, not the frame one.
        if (!m_pSdtHelper->isInteropGrabBagEmpty() && bHasGrabBag && !m_pSdtHelper->isOutsideAParagraph())
        {
            comphelper::SequenceAsHashMap aFrameGrabBag(xPropertySet->getPropertyValue("FrameInteropGrabBag"));
            aFrameGrabBag["SdtPr"] <<= m_pSdtHelper->getInteropGrabBagAndClear();
            xPropertySet->setPropertyValue("FrameInteropGrabBag", uno::Any(aFrameGrabBag.getAsConstPropertyValueList()));
        }
    }

    /* Set "SdtEndBefore" property on Drawing.
     * It is required in a case when Drawing appears immediately after first run i.e.
     * there is no text/space/tab in between two runs.
     * In this case "SdtEndBefore" property needs to be set on Drawing.
     */
    if(IsSdtEndBefore())
    {
        if(xPropertySet.is() && bHasGrabBag)
        {
            uno::Sequence<beans::PropertyValue> aFrameGrabBag( comphelper::InitPropertySequence({
                { "SdtEndBefore", uno::Any(true) }
            }));
            xPropertySet->setPropertyValue("FrameInteropGrabBag",uno::Any(aFrameGrabBag));
        }
    }


    // Update the shape properties if it is embedded object.
    if (m_StreamStateStack.top().xEmbedded.is())
    {
        if (m_pGraphicImport->GetXShapeObject())
                m_pGraphicImport->GetXShapeObject()->setPosition(
                    m_pGraphicImport->GetGraphicObjectPosition());

        uno::Reference<drawing::XShape> xShape = m_pGraphicImport->GetXShapeObject();
        UpdateEmbeddedShapeProps(xShape);
        if (m_eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR)
        {
            uno::Reference<beans::XPropertySet> const xEmbeddedProps(m_StreamStateStack.top().xEmbedded, uno::UNO_QUERY);
            xEmbeddedProps->setPropertyValue("AnchorType", uno::Any(text::TextContentAnchorType_AT_CHARACTER));
            xEmbeddedProps->setPropertyValue("IsFollowingTextFlow", uno::Any(m_pGraphicImport->GetLayoutInCell()));
            uno::Reference<beans::XPropertySet> xShapeProps(xShape, uno::UNO_QUERY);
            xEmbeddedProps->setPropertyValue("HoriOrient", xShapeProps->getPropertyValue("HoriOrient"));
            xEmbeddedProps->setPropertyValue("HoriOrientPosition", xShapeProps->getPropertyValue("HoriOrientPosition"));
            xEmbeddedProps->setPropertyValue("HoriOrientRelation", xShapeProps->getPropertyValue("HoriOrientRelation"));
            xEmbeddedProps->setPropertyValue("VertOrient", xShapeProps->getPropertyValue("VertOrient"));
            xEmbeddedProps->setPropertyValue("VertOrientPosition", xShapeProps->getPropertyValue("VertOrientPosition"));
            xEmbeddedProps->setPropertyValue("VertOrientRelation", xShapeProps->getPropertyValue("VertOrientRelation"));
            //tdf123873 fix missing textwrap import
            xEmbeddedProps->setPropertyValue("TextWrap", xShapeProps->getPropertyValue("TextWrap"));

            // GraphicZOrderHelper::findZOrder() was called already, so can just copy it over.
            xEmbeddedProps->setPropertyValue("ZOrder", xShapeProps->getPropertyValue("ZOrder"));
        }
    }
    //insert it into the document at the current cursor position
    OSL_ENSURE( xTextContent.is(), "DomainMapper_Impl::ImportGraphic");
    if( xTextContent.is())
    {
        bool bAppend = true;
        // workaround for images anchored to characters: add ZWSPs around the anchoring point
        if (m_eGraphicImportType != IMPORT_AS_DETECTED_INLINE && !m_aRedlines.top().empty())
        {
            uno::Reference< text::XTextAppend > xTextAppend = m_aTextAppendStack.top().xTextAppend;
            if(xTextAppend.is())
            {
                try
                {
                    uno::Reference< text::XText > xText = xTextAppend->getText();
                    uno::Reference< text::XTextCursor > xCrsr = xText->createTextCursor();
                    xCrsr->gotoEnd(false);
                    PropertyMapPtr pEmpty(new PropertyMap());
                    appendTextPortion(u"​"_ustr, pEmpty);
                    appendTextContent( xTextContent, uno::Sequence< beans::PropertyValue >() );
                    bAppend = false;
                    xCrsr->gotoEnd(false);
                    appendTextPortion(u"​"_ustr, pEmpty);

                    m_bRedlineImageInPreviousRun = true;
                    m_previousRedline = m_currentRedline;
                }
                catch( const uno::Exception& )
                {
                }
            }
        }

        if ( bAppend )
            appendTextContent( xTextContent, uno::Sequence< beans::PropertyValue >() );

        if (m_eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR && !m_aTextAppendStack.empty())
        {
            // Remember this object is anchored to the current paragraph.
            AnchoredObjectInfo aInfo;
            aInfo.m_xAnchoredObject = xTextContent;
            if (m_pGraphicImport)
            {
                // We still have the graphic import around, remember the original margin, so later
                // SectionPropertyMap::HandleIncreasedAnchoredObjectSpacing() can use it.
                aInfo.m_nLeftMargin = m_pGraphicImport->GetLeftMarginOrig();
            }
            m_aTextAppendStack.top().m_aAnchoredObjects.push_back(aInfo);
        }
        else if (m_eGraphicImportType == IMPORT_AS_DETECTED_INLINE)
        {
            m_StreamStateStack.top().bParaWithInlineObject = true;

            // store inline images with track changes, because the anchor point
            // to set redlining is not available yet
            if (!m_aTextAppendStack.empty() && !m_aRedlines.top().empty() )
            {
                // Remember this object is anchored to the current paragraph.
                AnchoredObjectInfo aInfo;
                aInfo.m_xAnchoredObject = xTextContent;
                aInfo.m_xRedlineForInline = m_aRedlines.top().back();
                m_aTextAppendStack.top().m_aAnchoredObjects.push_back(aInfo);
            }

        }
    }

    // Clear the reference, so in case the embedded object is inside a
    // TextFrame, we won't try to resize it (to match the size of the
    // TextFrame) here.
    m_StreamStateStack.top().xEmbedded.clear();
    m_pGraphicImport.clear();
}


void DomainMapper_Impl::SetLineNumbering( sal_Int32 nLnnMod, sal_uInt32 nLnc, sal_Int32 ndxaLnn )
{
    if (!m_xTextDocument)
        throw uno::RuntimeException();
    if( !m_bLineNumberingSet )
    {
        try
        {
            uno::Reference< beans::XPropertySet > xProperties = m_xTextDocument->getLineNumberingProperties();
            uno::Any aTrue( uno::Any( true ));
            xProperties->setPropertyValue( getPropertyName( PROP_IS_ON                  ), aTrue);
            xProperties->setPropertyValue( getPropertyName( PROP_COUNT_EMPTY_LINES      ), aTrue );
            xProperties->setPropertyValue( getPropertyName( PROP_COUNT_LINES_IN_FRAMES  ), uno::Any( false ) );
            xProperties->setPropertyValue( getPropertyName( PROP_INTERVAL               ), uno::Any( static_cast< sal_Int16 >( nLnnMod )));
            xProperties->setPropertyValue( getPropertyName( PROP_DISTANCE               ), uno::Any( ConversionHelper::convertTwipToMM100(ndxaLnn) ));
            xProperties->setPropertyValue( getPropertyName( PROP_NUMBER_POSITION        ), uno::Any( style::LineNumberPosition::LEFT));
            xProperties->setPropertyValue( getPropertyName( PROP_NUMBERING_TYPE         ), uno::Any( style::NumberingType::ARABIC));
            xProperties->setPropertyValue( getPropertyName( PROP_RESTART_AT_EACH_PAGE   ), uno::Any( nLnc == NS_ooxml::LN_Value_ST_LineNumberRestart_newPage ));
        }
        catch( const uno::Exception& )
        {}
    }
    m_bLineNumberingSet = true;
    uno::Reference< container::XNameAccess > xStyleFamilies = m_xTextDocument->getStyleFamilies();
    uno::Reference<container::XNameContainer> xStyles;
    xStyleFamilies->getByName(getPropertyName( PROP_PARAGRAPH_STYLES )) >>= xStyles;
    lcl_linenumberingHeaderFooter( xStyles, "Header", this );
    lcl_linenumberingHeaderFooter( xStyles, "Footer", this );
}


void DomainMapper_Impl::SetPageMarginTwip( PageMarElement eElement, sal_Int32 nValue )
{
    nValue = ConversionHelper::convertTwipToMM100(nValue);
    switch(eElement)
    {
        case PAGE_MAR_TOP    : m_aPageMargins.top     = nValue; break;
        case PAGE_MAR_RIGHT  : m_aPageMargins.right   = nValue; break;
        case PAGE_MAR_BOTTOM : m_aPageMargins.bottom  = nValue; break;
        case PAGE_MAR_LEFT   : m_aPageMargins.left    = nValue; break;
        case PAGE_MAR_HEADER : m_aPageMargins.header  = nValue; break;
        case PAGE_MAR_FOOTER : m_aPageMargins.footer  = nValue; break;
        case PAGE_MAR_GUTTER:
            m_aPageMargins.gutter = nValue;
            break;
    }
}

void DomainMapper_Impl::SetPaperSource(PaperSourceElement eElement, sal_Int32 nValue)
{
    if(eElement == PAPER_SOURCE_FIRST)
        m_aPaperSource.first = nValue;
    else
        m_aPaperSource.other = nValue;
}


PageMar::PageMar()
    : top(ConversionHelper::convertTwipToMM100( sal_Int32(1440)))
    // This is strange, the RTF spec says it's 1800, but it's clearly 1440 in Word
    // OOXML seems not to specify a default value
    , right(ConversionHelper::convertTwipToMM100( sal_Int32(1440)))
    , bottom(top)
    , left(right)
    , header(ConversionHelper::convertTwipToMM100(sal_Int32(720)))
    , footer(header)
    , gutter(0)
{
}


void DomainMapper_Impl::RegisterFrameConversion(
        uno::Reference< text::XTextRange > const&    xFrameStartRange,
        uno::Reference< text::XTextRange > const&    xFrameEndRange,
        std::vector<beans::PropertyValue>&& rFrameProperties
        )
{
    OSL_ENSURE(
        m_aFrameProperties.empty() && !m_xFrameStartRange.is() && !m_xFrameEndRange.is(),
        "frame properties not removed");
    m_aFrameProperties = std::move(rFrameProperties);
    m_xFrameStartRange = xFrameStartRange;
    m_xFrameEndRange   = xFrameEndRange;
}


void DomainMapper_Impl::ExecuteFrameConversion()
{
    if( m_xFrameStartRange.is() && m_xFrameEndRange.is() && !m_bDiscardHeaderFooter )
    {
        std::vector<sal_Int32> redPos, redLen;
        try
        {
            uno::Reference< text::XTextAppendAndConvert > xTextAppendAndConvert( GetTopTextAppend(), uno::UNO_QUERY_THROW );
            // convert redline ranges to cursor movement and character length
            sal_Int32 redIdx;
            lcl_CopyRedlines(GetTopTextAppend(), m_aStoredRedlines[StoredRedlines::FRAME], redPos, redLen, redIdx);

            const uno::Reference< text::XTextContent >& xTextContent = xTextAppendAndConvert->convertToTextFrame(
                m_xFrameStartRange,
                m_xFrameEndRange,
                comphelper::containerToSequence(m_aFrameProperties) );

            uno::Reference< text::XText > xDest( xTextContent, uno::UNO_QUERY_THROW );
            lcl_PasteRedlines(xDest, m_aStoredRedlines[StoredRedlines::FRAME], redPos, redLen, redIdx);
        }
        catch( const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION( "writerfilter.dmapper", "Exception caught when converting to frame");
        }

        m_bIsActualParagraphFramed = false;

        if (redPos.size() == m_aStoredRedlines[StoredRedlines::FRAME].size()/3)
        {
            for( sal_Int32 i = m_aStoredRedlines[StoredRedlines::FRAME].size() - 1; i >= 0; --i)
            {
                // keep redlines of floating tables to process them in CloseSectionGroup()
                if ( redPos[i/3] != -1 )
                {
                    m_aStoredRedlines[StoredRedlines::FRAME].erase(m_aStoredRedlines[StoredRedlines::FRAME].begin() + i);
                }
            }
        }
        else
            m_aStoredRedlines[StoredRedlines::FRAME].clear();
    }
    m_xFrameStartRange = nullptr;
    m_xFrameEndRange = nullptr;
    m_aFrameProperties.clear();
}

void DomainMapper_Impl::AddNewRedline( sal_uInt32 sprmId )
{
    RedlineParamsPtr pNew( new RedlineParams );
    pNew->m_nToken = XML_mod;
    if ( !m_bIsParaMarkerChange )
    {
        // <w:rPrChange> applies to the whole <w:r>, <w:pPrChange> applies to the whole <w:p>,
        // so keep those two in CONTEXT_CHARACTERS and CONTEXT_PARAGRAPH, which will take
        // care of their scope (i.e. when they should be used and discarded).
        // Let's keep the rest the same way they used to be handled (explicitly dropped
        // from a global stack by endtrackchange), but quite possibly they should not be handled
        // that way either (I don't know).
        if( sprmId == NS_ooxml::LN_EG_RPrContent_rPrChange )
            GetTopContextOfType( CONTEXT_CHARACTER )->Redlines().push_back( pNew );
        else if( sprmId == NS_ooxml::LN_CT_PPr_pPrChange )
            GetTopContextOfType( CONTEXT_PARAGRAPH )->Redlines().push_back( pNew );
        else if( sprmId != NS_ooxml::LN_CT_ParaRPr_rPrChange )
            m_aRedlines.top().push_back( pNew );
    }
    else
    {
        m_pParaMarkerRedline = pNew;
    }
    // Newly read data will go into this redline.
    m_currentRedline = pNew;
}

void DomainMapper_Impl::SetCurrentRedlineIsRead()
{
    m_currentRedline.clear();
}

sal_Int32 DomainMapper_Impl::GetCurrentRedlineToken(  ) const
{
    assert(m_currentRedline);
    return m_currentRedline->m_nToken;
}

void DomainMapper_Impl::SetCurrentRedlineAuthor( const OUString& sAuthor )
{
    if (!m_xAnnotationField.is())
    {
        if (m_currentRedline)
            m_currentRedline->m_sAuthor = sAuthor;
        else
            SAL_INFO("writerfilter.dmapper", "numberingChange not implemented");
    }
    else
        m_xAnnotationField->setPropertyValue("Author", uno::Any(sAuthor));
}

void DomainMapper_Impl::SetCurrentRedlineInitials( const OUString& sInitials )
{
    if (m_xAnnotationField.is())
        m_xAnnotationField->setPropertyValue("Initials", uno::Any(sInitials));
}

void DomainMapper_Impl::SetCurrentRedlineDate( const OUString& sDate )
{
    if (!m_xAnnotationField.is())
    {
        if (m_currentRedline)
            m_currentRedline->m_sDate = sDate;
        else
            SAL_INFO("writerfilter.dmapper", "numberingChange not implemented");
    }
    else
        m_xAnnotationField->setPropertyValue("DateTimeValue", uno::Any(ConversionHelper::ConvertDateStringToDateTime(sDate)));
}

void DomainMapper_Impl::SetCurrentRedlineId( sal_Int32 sId )
{
    if (m_xAnnotationField.is())
    {
        m_nAnnotationId = sId;
    }
    else
    {
        // This should be an assert, but somebody had the smart idea to reuse this function also for comments and whatnot,
        // and in some cases the id is actually not handled, which may be in fact a bug.
        if( !m_currentRedline)
            SAL_INFO("writerfilter.dmapper", "no current redline");
    }
}

void DomainMapper_Impl::SetCurrentRedlineToken( sal_Int32 nToken )
{
    assert(m_currentRedline);
    m_currentRedline->m_nToken = nToken;
}

void DomainMapper_Impl::SetCurrentRedlineRevertProperties( const uno::Sequence<beans::PropertyValue>& aProperties )
{
    assert(m_currentRedline);
    m_currentRedline->m_aRevertProperties = aProperties;
}


// This removes only the last redline stored here, those stored in contexts are automatically removed when
// the context is destroyed.
void DomainMapper_Impl::RemoveTopRedline( )
{
    if (m_aRedlines.top().empty())
    {
        if (GetFootnoteCount() > -1 || GetEndnoteCount() > -1)
            return;
        SAL_WARN("writerfilter.dmapper", "RemoveTopRedline called with empty stack");
        throw uno::Exception("RemoveTopRedline failed", nullptr);
    }
    m_aRedlines.top().pop_back( );
    m_currentRedline.clear();
}

void DomainMapper_Impl::ApplySettingsTable()
{
    if (!(m_pSettingsTable && m_xTextDocument))
        return;

    try
    {
        rtl::Reference< SwXTextDefaults > xTextDefaults(m_xTextDocument->createTextDefaults());
        sal_Int32 nDefTab = m_pSettingsTable->GetDefaultTabStop();
        xTextDefaults->setPropertyValue( getPropertyName( PROP_TAB_STOP_DISTANCE ), uno::Any(nDefTab) );
        if (m_pSettingsTable->GetLinkStyles())
        {
            // If linked styles are enabled, set paragraph defaults from Word's default template
            xTextDefaults->setPropertyValue(getPropertyName(PROP_PARA_BOTTOM_MARGIN), uno::Any(ConversionHelper::convertTwipToMM100(200)));
            style::LineSpacing aSpacing;
            aSpacing.Mode = style::LineSpacingMode::PROP;
            aSpacing.Height = sal_Int16(115);
            xTextDefaults->setPropertyValue(getPropertyName(PROP_PARA_LINE_SPACING), uno::Any(aSpacing));
        }

        if (m_pSettingsTable->GetZoomFactor() || m_pSettingsTable->GetView())
        {
            std::vector<beans::PropertyValue> aViewProps;
            if (m_pSettingsTable->GetZoomFactor())
            {
                aViewProps.emplace_back("ZoomFactor", -1, uno::Any(m_pSettingsTable->GetZoomFactor()), beans::PropertyState_DIRECT_VALUE);
                aViewProps.emplace_back("VisibleBottom", -1, uno::Any(sal_Int32(0)), beans::PropertyState_DIRECT_VALUE);
                aViewProps.emplace_back("ZoomType", -1,
                                        uno::Any(m_pSettingsTable->GetZoomType()),
                                        beans::PropertyState_DIRECT_VALUE);
            }
            rtl::Reference< comphelper::IndexedPropertyValuesContainer > xBox = new comphelper::IndexedPropertyValuesContainer();
            xBox->insertByIndex(sal_Int32(0), uno::Any(comphelper::containerToSequence(aViewProps)));
            m_xTextDocument->setViewData(xBox);
        }

        rtl::Reference<SwXDocumentSettings> xSettings(m_xTextDocument->createDocumentSettings());

        if (m_pSettingsTable->GetDoNotExpandShiftReturn())
            xSettings->setPropertyValue( "DoNotJustifyLinesWithManualBreak", uno::Any(true) );
        // new paragraph justification has been introduced in version 15,
        // breaking text layout interoperability: new line shrinking needs less space
        // i.e. it typesets the same text with less lines and pages.
        if (m_pSettingsTable->GetWordCompatibilityMode() >= 15)
            xSettings->setPropertyValue("JustifyLinesWithShrinking", uno::Any( true ));
        if (m_pSettingsTable->GetUsePrinterMetrics())
            xSettings->setPropertyValue("PrinterIndependentLayout", uno::Any(document::PrinterIndependentLayout::DISABLED));
        if( m_pSettingsTable->GetEmbedTrueTypeFonts())
            xSettings->setPropertyValue( getPropertyName( PROP_EMBED_FONTS ), uno::Any(true) );
        if( m_pSettingsTable->GetEmbedSystemFonts())
            xSettings->setPropertyValue( getPropertyName( PROP_EMBED_SYSTEM_FONTS ), uno::Any(true) );
        xSettings->setPropertyValue("AddParaTableSpacing", uno::Any(m_pSettingsTable->GetDoNotUseHTMLParagraphAutoSpacing()));
        if (m_pSettingsTable->GetNoLeading())
        {
            xSettings->setPropertyValue("AddExternalLeading", uno::Any(!m_pSettingsTable->GetNoLeading()));
        }
        if( m_pSettingsTable->GetProtectForm() )
            xSettings->setPropertyValue("ProtectForm", uno::Any( true ));
        if( m_pSettingsTable->GetReadOnly() )
            xSettings->setPropertyValue("LoadReadonly", uno::Any( true ));
        if (m_pSettingsTable->GetGutterAtTop())
        {
            xSettings->setPropertyValue("GutterAtTop", uno::Any(true));
        }
        uno::Sequence<beans::PropertyValue> aWriteProtection
                = m_pSettingsTable->GetWriteProtectionSettings();
        if (aWriteProtection.hasElements())
                xSettings->setPropertyValue("ModifyPasswordInfo", uno::Any(aWriteProtection));
    }
    catch(const uno::Exception&)
    {
    }
}

SectionPropertyMap * DomainMapper_Impl::GetSectionContext()
{
    SectionPropertyMap* pSectionContext = nullptr;
    //the section context is not available before the first call of startSectionGroup()
    if( !IsAnyTableImport() )
    {
        PropertyMapPtr pContext = GetTopContextOfType(CONTEXT_SECTION);
        pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
    }

    return pSectionContext;
}

void DomainMapper_Impl::deferCharacterProperty(sal_Int32 id, const css::uno::Any& value)
{
    m_StreamStateStack.top().deferredCharacterProperties[ id ] = value;
}

void DomainMapper_Impl::processDeferredCharacterProperties(bool bCharContext)
{
    // Actually process in DomainMapper, so that it's the same source file like normal processing.
    if (!m_StreamStateStack.top().deferredCharacterProperties.empty())
    {
        m_rDMapper.processDeferredCharacterProperties(m_StreamStateStack.top().deferredCharacterProperties, bCharContext);
        m_StreamStateStack.top().deferredCharacterProperties.clear();
    }
}

sal_Int32 DomainMapper_Impl::getNumberingProperty(const sal_Int32 nListId, sal_Int32 nNumberingLevel, const OUString& aProp)
{
    sal_Int32 nRet = 0;
    if ( nListId < 0 )
        return nRet;
    if ( !m_xTextDocument )
        return nRet;

    try
    {
        if (nNumberingLevel < 0) // It seems it's valid to omit numbering level, and in that case it means zero.
            nNumberingLevel = 0;

        auto const pList(GetListTable()->GetList(nListId));
        assert(pList);
        const OUString aListName = pList->GetStyleName();
        const uno::Reference< container::XNameAccess > xStyleFamilies = m_xTextDocument->getStyleFamilies();
        uno::Reference<container::XNameAccess> xNumberingStyles;
        xStyleFamilies->getByName("NumberingStyles") >>= xNumberingStyles;
        const uno::Reference<beans::XPropertySet> xStyle(xNumberingStyles->getByName(aListName), uno::UNO_QUERY);
        const uno::Reference<container::XIndexAccess> xNumberingRules(xStyle->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
        if (xNumberingRules.is())
        {
            uno::Sequence<beans::PropertyValue> aProps;
            xNumberingRules->getByIndex(nNumberingLevel) >>= aProps;
            auto pProp = std::find_if(std::cbegin(aProps), std::cend(aProps),
                [&aProp](const beans::PropertyValue& rProp) { return rProp.Name == aProp; });
            if (pProp != std::cend(aProps))
                pProp->Value >>= nRet;
        }
    }
    catch( const uno::Exception& )
    {
        // This can happen when the doc contains some hand-crafted invalid list level.
    }

    return nRet;
}

sal_Int32 DomainMapper_Impl::getCurrentNumberingProperty(const OUString& aProp)
{
    sal_Int32 nRet = 0;

    std::optional<PropertyMap::Property> pProp = m_pTopContext->getProperty(PROP_NUMBERING_RULES);
    uno::Reference<container::XIndexAccess> xNumberingRules;
    if (pProp)
        xNumberingRules.set(pProp->second, uno::UNO_QUERY);
    pProp = m_pTopContext->getProperty(PROP_NUMBERING_LEVEL);
    // Default numbering level is the first one.
    sal_Int32 nNumberingLevel = 0;
    if (pProp)
        pProp->second >>= nNumberingLevel;
    if (xNumberingRules.is())
    {
        uno::Sequence<beans::PropertyValue> aProps;
        xNumberingRules->getByIndex(nNumberingLevel) >>= aProps;
        auto pPropVal = std::find_if(std::cbegin(aProps), std::cend(aProps),
            [&aProp](const beans::PropertyValue& rProp) { return rProp.Name == aProp; });
        if (pPropVal != std::cend(aProps))
            pPropVal->Value >>= nRet;
    }

    return nRet;
}


void DomainMapper_Impl::enableInteropGrabBag(const OUString& aName)
{
    m_aInteropGrabBagName = aName;
}

void DomainMapper_Impl::disableInteropGrabBag()
{
    m_aInteropGrabBagName.clear();
    m_aInteropGrabBag.clear();
    m_aSubInteropGrabBag.clear();
}

bool DomainMapper_Impl::isInteropGrabBagEnabled() const
{
    return !(m_aInteropGrabBagName.isEmpty());
}

void DomainMapper_Impl::appendGrabBag(std::vector<beans::PropertyValue>& rInteropGrabBag, const OUString& aKey, const OUString& aValue)
{
    if (m_aInteropGrabBagName.isEmpty())
        return;
    beans::PropertyValue aProperty;
    aProperty.Name = aKey;
    aProperty.Value <<= aValue;
    rInteropGrabBag.push_back(aProperty);
}

void DomainMapper_Impl::appendGrabBag(std::vector<beans::PropertyValue>& rInteropGrabBag, const OUString& aKey, std::vector<beans::PropertyValue>& rValue)
{
    if (m_aInteropGrabBagName.isEmpty())
        return;
    beans::PropertyValue aProperty;
    aProperty.Name = aKey;
    aProperty.Value <<= comphelper::containerToSequence(rValue);
    rValue.clear();
    rInteropGrabBag.push_back(aProperty);
}

void DomainMapper_Impl::substream(Id rName,
        ::writerfilter::Reference<Stream>::Pointer_t const& ref)
{
#ifndef NDEBUG
    size_t contextSize(m_aContextStack.size());
    size_t propSize[NUMBER_OF_CONTEXTS];
    for (int i = 0; i < NUMBER_OF_CONTEXTS; ++i) {
        propSize[i] = m_aPropertyStacks[i].size();
    }
#endif

    //finalize any waiting frames before starting alternate streams
    CheckUnregisteredFrameConversion();
    ExecuteFrameConversion();

    appendTableManager();
    // Appending a TableManager resets its TableHandler, so we need to append
    // that as well, or tables won't be imported properly in headers/footers.
    appendTableHandler();
    getTableManager().startLevel();

    // Save "has footnote" state, which is specific to a section in the body
    // text, so state from substreams is not relevant.
    m_StreamStateStack.emplace();

    //import of page header/footer
    //Ensure that only one header/footer per section is pushed

    switch( rName )
    {
    case NS_ooxml::LN_headerl:
            PushPageHeaderFooter(PagePartType::Header, PageType::LEFT);
        break;
    case NS_ooxml::LN_headerr:
            PushPageHeaderFooter(PagePartType::Header, PageType::RIGHT);
        break;
    case NS_ooxml::LN_headerf:
            PushPageHeaderFooter(PagePartType::Header, PageType::FIRST);
        break;
    case NS_ooxml::LN_footerl:
            PushPageHeaderFooter(PagePartType::Footer, PageType::LEFT);
        break;
    case NS_ooxml::LN_footerr:
            PushPageHeaderFooter(PagePartType::Footer, PageType::RIGHT);
        break;
    case NS_ooxml::LN_footerf:
            PushPageHeaderFooter(PagePartType::Footer, PageType::FIRST);
        break;
    case NS_ooxml::LN_footnote:
    case NS_ooxml::LN_endnote:
        PushFootOrEndnote( NS_ooxml::LN_footnote == rName );
    break;
    case NS_ooxml::LN_annotation :
        PushAnnotation();
    break;
    default:
        assert(false); // unexpected?
    }

    assert(m_StreamStateStack.top().eSubstreamType != SubstreamType::Body);

    try
    {
        ref->resolve(m_rDMapper);
    }
    catch (xml::sax::SAXException const&)
    {
        m_bSaxError = true;
        throw;
    }

    switch( rName )
    {
    case NS_ooxml::LN_headerl:
        PopPageHeaderFooter(PagePartType::Header, PageType::LEFT);
    break;
    case NS_ooxml::LN_footerl:
        PopPageHeaderFooter(PagePartType::Footer, PageType::LEFT);
    break;
    case NS_ooxml::LN_headerr:
        PopPageHeaderFooter(PagePartType::Header, PageType::RIGHT);
    break;
    case NS_ooxml::LN_footerr:
        PopPageHeaderFooter(PagePartType::Footer, PageType::RIGHT);
    break;
    case NS_ooxml::LN_headerf:
        PopPageHeaderFooter(PagePartType::Header, PageType::FIRST);
    break;
    case NS_ooxml::LN_footerf:
        PopPageHeaderFooter(PagePartType::Footer, PageType::FIRST);
    break;
    case NS_ooxml::LN_footnote:
    case NS_ooxml::LN_endnote:
        PopFootOrEndnote();
    break;
    case NS_ooxml::LN_annotation :
        PopAnnotation();
    break;
    }

    m_StreamStateStack.pop();
    assert(!m_StreamStateStack.empty());

    getTableManager().endLevel();
    popTableManager();

    switch(rName)
    {
    case NS_ooxml::LN_footnote:
    case NS_ooxml::LN_endnote:
        m_StreamStateStack.top().bHasFtn = true;
        break;
    }

    // check that stacks are the same as before substream
    assert(m_aContextStack.size() == contextSize);
    for (int i = 0; i < NUMBER_OF_CONTEXTS; ++i) {
        assert(m_aPropertyStacks[i].size() == propSize[i]);
    }
}

void DomainMapper_Impl::commentProps(const OUString& sId, const CommentProperties& rProps)
{
    m_aCommentProps[sId] = rProps;
}


bool DomainMapper_Impl::handlePreviousParagraphBorderInBetween() const
{
    if (!m_StreamStateStack.top().xPreviousParagraph.is())
        return false;

    // Connected borders ("ParaIsConnectBorder") are always on by default
    // and never changed by DomainMapper. Except one case when border in
    // between is used. So this is not the best, but easiest way to check
    // is previous paragraph has border in between.
    bool bConnectBorders = true;
    m_StreamStateStack.top().xPreviousParagraph->getPropertyValue(getPropertyName(PROP_PARA_CONNECT_BORDERS)) >>= bConnectBorders;

    if (bConnectBorders)
        return false;

    // Previous paragraph has border in between. Current one also has (since this
    // method is called). So current paragraph will get border above, but
    // also need to ensure, that no unexpected bottom border are remaining in previous
    // paragraph: since ParaIsConnectBorder=false it will be displayed in unexpected way.
    m_StreamStateStack.top().xPreviousParagraph->setPropertyValue(getPropertyName(PROP_BOTTOM_BORDER), uno::Any(table::BorderLine2()));

    return true;
}

OUString DomainMapper_Impl::getFontNameForTheme(const Id id)
{
    auto const& pHandler = getThemeHandler();
    if (pHandler)
        return pHandler->getFontNameForTheme(id);
    return OUString();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
