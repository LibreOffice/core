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
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
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
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/text/XRedline.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <editeng/flditem.hxx>
#include <editeng/unotext.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/temporary.hxx>
#include <oox/mathml/import.hxx>
#include <xmloff/odffields.hxx>
#include <rtl/uri.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>

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
#include <unotools/configmgr.hxx>
#include <unotools/mediadescriptor.hxx>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>
#include <com/sun/star/drawing/FillStyle.hpp>

#include <unicode/errorcode.h>
#include <unicode/regex.h>

using namespace ::com::sun::star;
using namespace oox;
namespace writerfilter::dmapper{

//line numbering for header/footer
static void lcl_linenumberingHeaderFooter( const uno::Reference<container::XNameContainer>& xStyles, const OUString& rname, DomainMapper_Impl* dmapper )
{
    const StyleSheetEntryPtr pEntry = dmapper->GetStyleSheetTable()->FindStyleSheetByISTD( rname );
    if (!pEntry)
        return;
    const StyleSheetPropertyMap* pStyleSheetProperties = pEntry->pProperties.get();
    if ( !pStyleSheetProperties )
        return;
    sal_Int32 nListId = pStyleSheetProperties->GetListId();
    if( xStyles.is() )
    {
        if( xStyles->hasByName( rname ) )
        {
            uno::Reference< style::XStyle > xStyle;
            xStyles->getByName( rname ) >>= xStyle;
            if( !xStyle.is() )
                return;
            uno::Reference<beans::XPropertySet> xPropertySet( xStyle, uno::UNO_QUERY );
            xPropertySet->setPropertyValue( getPropertyName( PROP_PARA_LINE_NUMBER_COUNT ), uno::makeAny( nListId >= 0 ) );
        }
    }
}

// Populate Dropdown Field properties from FFData structure
static void lcl_handleDropdownField( const uno::Reference< beans::XPropertySet >& rxFieldProps, const FFDataHandler::Pointer_t& pFFDataHandler )
{
    if ( !rxFieldProps.is() )
        return;

    if ( !pFFDataHandler->getName().isEmpty() )
        rxFieldProps->setPropertyValue( "Name", uno::makeAny( pFFDataHandler->getName() ) );

    const FFDataHandler::DropDownEntries_t& rEntries = pFFDataHandler->getDropDownEntries();
    uno::Sequence< OUString > sItems( rEntries.size() );
    ::std::copy( rEntries.begin(), rEntries.end(), sItems.getArray());
    if ( sItems.hasElements() )
        rxFieldProps->setPropertyValue( "Items", uno::makeAny( sItems ) );

    sal_Int32 nResult = pFFDataHandler->getDropDownResult().toInt32();
    if (nResult > 0 && o3tl::make_unsigned(nResult) < sItems.size())
        rxFieldProps->setPropertyValue( "SelectedItem", uno::makeAny( std::as_const(sItems)[ nResult ] ) );
    if ( !pFFDataHandler->getHelpText().isEmpty() )
         rxFieldProps->setPropertyValue( "Help", uno::makeAny( pFFDataHandler->getHelpText() ) );
}

static void lcl_handleTextField( const uno::Reference< beans::XPropertySet >& rxFieldProps, const FFDataHandler::Pointer_t& pFFDataHandler )
{
    if ( rxFieldProps.is() && pFFDataHandler )
    {
        rxFieldProps->setPropertyValue
            (getPropertyName(PROP_HINT),
            uno::makeAny(pFFDataHandler->getStatusText()));
        rxFieldProps->setPropertyValue
            (getPropertyName(PROP_HELP),
            uno::makeAny(pFFDataHandler->getHelpText()));
        rxFieldProps->setPropertyValue
            (getPropertyName(PROP_CONTENT),
            uno::makeAny(pFFDataHandler->getTextDefault()));
    }
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
    std::optional<FieldId> oOuterFieldId = pOuter->GetFieldId();
    OUString aCommand = pOuter->GetCommand();

    // Ignore leading space before the field name, but don't accept IFF when we check for IF.
    if (!aCommand.isEmpty() && aCommand[0] == ' ')
    {
        aCommand = aCommand.subView(1);
    }

    if (!oOuterFieldId && aCommand.startsWith("IF "))
    {
        // This will be FIELD_IF once the command is closed.
        oOuterFieldId = FIELD_IF;
    }

    if (!oOuterFieldId)
    {
        return true;
    }

    if (!pInner->GetFieldId())
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
                case FIELD_FORMULA:
                case FIELD_IF:
                case FIELD_MERGEFIELD:
                {
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

uno::Any FloatingTableInfo::getPropertyValue(std::u16string_view propertyName)
{
    for( beans::PropertyValue const & propVal : std::as_const(m_aFrameProperties) )
        if( propVal.Name == propertyName )
            return propVal.Value ;
    return uno::Any() ;
}

DomainMapper_Impl::DomainMapper_Impl(
            DomainMapper& rDMapper,
            uno::Reference<uno::XComponentContext> const& xContext,
            uno::Reference<lang::XComponent> const& xModel,
            SourceDocumentType eDocumentType,
            utl::MediaDescriptor const & rMediaDesc) :
        m_eDocumentType( eDocumentType ),
        m_rDMapper( rDMapper ),
        m_xTextDocument( xModel, uno::UNO_QUERY ),
        m_xTextFactory( xModel, uno::UNO_QUERY ),
        m_xComponentContext( xContext ),
        m_bForceGenericFields(!utl::ConfigManager::IsFuzzing() && officecfg::Office::Common::Filter::Microsoft::Import::ForceImportWWFieldsAsGenericFields::get(m_xComponentContext)),
        m_bSetUserFieldContent( false ),
        m_bSetCitation( false ),
        m_bSetDateValue( false ),
        m_bIsFirstSection( true ),
        m_bIsColumnBreakDeferred( false ),
        m_bIsPageBreakDeferred( false ),
        m_bSdtEndDeferred(false),
        m_bParaSdtEndDeferred(false),
        m_bStartTOC(false),
        m_bStartTOCHeaderFooter(false),
        m_bStartedTOC(false),
        m_bStartIndex(false),
        m_bStartBibliography(false),
        m_nStartGenericField(0),
        m_bTextInserted(false),
        m_sCurrentPermId(0),
        m_bFrameDirectionSet(false),
        m_bInDocDefaultsImport(false),
        m_bInStyleSheetImport( false ),
        m_bInNumberingImport(false),
        m_bInAnyTableImport( false ),
        m_eInHeaderFooterImport( HeaderFooterImportState::none ),
        m_bDiscardHeaderFooter( false ),
        m_bInFootOrEndnote(false),
        m_bInFootnote(false),
        m_bHasFootnoteStyle(false),
        m_bCheckFootnoteStyle(false),
        m_eSkipFootnoteState(SkipFootnoteSeparator::OFF),
        m_nFootnotes(-1),
        m_nEndnotes(-1),
        m_bLineNumberingSet( false ),
        m_bIsInFootnoteProperties( false ),
        m_bIsParaMarkerChange( false ),
        m_bRedlineImageInPreviousRun( false ),
        m_bParaChanged( false ),
        m_bIsFirstParaInSection( true ),
        m_bIsFirstParaInSectionAfterRedline( true ),
        m_bDummyParaAddedForTableInSection( false ),
        m_bDummyParaAddedForTableInSectionPage( false ),
        m_bTextFrameInserted(false),
        m_bIsPreviousParagraphFramed( false ),
        m_bIsLastParaInSection( false ),
        m_bIsLastSectionGroup( false ),
        m_bIsInComments( false ),
        m_bParaSectpr( false ),
        m_bUsingEnhancedFields( false ),
        m_bSdt(false),
        m_bIsFirstRun(false),
        m_bIsOutsideAParagraph(true),
        m_nAnnotationId( -1 ),
        m_aSmartTagHandler(m_xComponentContext, m_xTextDocument),
        m_xInsertTextRange(rMediaDesc.getUnpackedValueOrDefault("TextInsertModeRange", uno::Reference<text::XTextRange>())),
        m_xAltChunkStartingRange(rMediaDesc.getUnpackedValueOrDefault("AltChunkStartingRange", uno::Reference<text::XTextRange>())),
        m_bIsInTextBox(false),
        m_bIsNewDoc(!rMediaDesc.getUnpackedValueOrDefault("InsertMode", false)),
        m_bIsAltChunk(rMediaDesc.getUnpackedValueOrDefault("AltChunkMode", false)),
        m_bIsReadGlossaries(rMediaDesc.getUnpackedValueOrDefault("ReadGlossaries", false)),
        m_nTableDepth(0),
        m_nTableCellDepth(0),
        m_nLastTableCellParagraphDepth(0),
        m_bHasFtn(false),
        m_bHasFtnSep(false),
        m_bCheckFirstFootnoteTab(false),
        m_bIgnoreNextTab(false),
        m_bIsSplitPara(false),
        m_bIsActualParagraphFramed( false ),
        m_bParaHadField(false),
        m_bSaveParaHadField(false),
        m_bParaAutoBefore(false),
        m_bFirstParagraphInCell(true),
        m_bSaveFirstParagraphInCell(false),
        m_bParaWithInlineObject(false),
        m_bSaxError(false)
{
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
    m_bUsingEnhancedFields = !utl::ConfigManager::IsFuzzing() && officecfg::Office::Common::Filter::Microsoft::Import::ImportWWFieldsAsEnhancedFields::get(m_xComponentContext);

    m_pSdtHelper = new SdtHelper(*this);

    m_aRedlines.push(std::vector<RedlineParamsPtr>());

    if (m_bIsAltChunk)
    {
        m_bIsFirstSection = false;
    }
}


DomainMapper_Impl::~DomainMapper_Impl()
{
    ChainTextFrames();
    // Don't remove last paragraph when pasting, sw expects that empty paragraph.
    if (m_bIsNewDoc)
        RemoveLastParagraph();
    if (hasTableManager())
    {
        getTableManager().endLevel();
        popTableManager();
    }
}

uno::Reference< container::XNameContainer > const &  DomainMapper_Impl::GetPageStyles()
{
    if(!m_xPageStyles1.is())
    {
        uno::Reference< style::XStyleFamiliesSupplier > xSupplier( m_xTextDocument, uno::UNO_QUERY );
        if (xSupplier.is())
            xSupplier->getStyleFamilies()->getByName("PageStyles") >>= m_xPageStyles1;
    }
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
                sal_Int32 nIndex = rStyleName.copy( strlen( DEFAULT_STYLE ) ).toInt32();
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
    if(!m_xCharacterStyles.is())
    {
        uno::Reference< style::XStyleFamiliesSupplier > xSupplier( m_xTextDocument, uno::UNO_QUERY );
        if (xSupplier.is())
            xSupplier->getStyleFamilies()->getByName("CharacterStyles") >>= m_xCharacterStyles;
    }
    return m_xCharacterStyles;
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


uno::Reference< beans::XPropertySet > const & DomainMapper_Impl::GetDocumentSettings()
{
    if( !m_xDocumentSettings.is() && m_xTextFactory.is())
    {
        m_xDocumentSettings.set( m_xTextFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY );
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
        xParagraph->dispose();
    }
}
void DomainMapper_Impl::AddDummyParaForTableInSection()
{
    // Shapes can't have sections.
    if (IsInShape())
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

 static OUString lcl_FindLastBookmark(const uno::Reference<text::XTextCursor>& xCursor)
 {
     OUString sName;
     if (!xCursor.is())
         return sName;

     // Select 1 previous element
     xCursor->goLeft(1, true);
     uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCursor, uno::UNO_QUERY);
     if (!xParaEnumAccess.is())
     {
         xCursor->goRight(1, true);
         return sName;
     }

     // Iterate through selection paragraphs
     uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
     if (!xParaEnum->hasMoreElements())
     {
         xCursor->goRight(1, true);
         return sName;
     }

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

     xCursor->goRight(1, true);
     return sName;
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
    try
    {
        uno::Reference< text::XTextCursor > xCursor;
        if (m_bIsNewDoc)
        {
            xCursor = xTextAppend->createTextCursor();
            xCursor->gotoEnd(false);
        }
        else
            xCursor = m_aTextAppendStack.top().xCursor;
        uno::Reference<container::XEnumerationAccess> xEnumerationAccess(xCursor, uno::UNO_QUERY);
        // Keep the character properties of the last but one paragraph, even if
        // it's empty. This works for headers/footers, and maybe in other cases
        // as well, but surely not in textboxes.
        // fdo#58327: also do this at the end of the document: when pasting,
        // a table before the cursor position would be deleted
        // (but only for paste/insert, not load; otherwise it can happen that
        // flys anchored at the disposed paragraph are deleted (fdo47036.rtf))
        bool const bEndOfDocument(m_aTextAppendStack.size() == 1);

        // Try to find and remember last bookmark in document: it potentially
        // can be deleted by xCursor->setString() but not by xParagraph->dispose()
        OUString sLastBookmarkName;
        if (bEndOfDocument)
            sLastBookmarkName = lcl_FindLastBookmark(xCursor);

        if ((IsInHeaderFooter() || (bEndOfDocument && !m_bIsNewDoc))
            && xEnumerationAccess.is())
        {
            uno::Reference<container::XEnumeration> xEnumeration = xEnumerationAccess->createEnumeration();
            uno::Reference<lang::XComponent> xParagraph(xEnumeration->nextElement(), uno::UNO_QUERY);
            xParagraph->dispose();
        }
        else if (xCursor.is())
        {
            xCursor->goLeft( 1, true );
            // If this is a text on a shape, possibly the text has the trailing
            // newline removed already.
            if (xCursor->getString() == SAL_NEWLINE_STRING ||
                    // tdf#105444 comments need an exception, if SAL_NEWLINE_STRING defined as "\r\n"
                    (sizeof(SAL_NEWLINE_STRING)-1 == 2 && xCursor->getString() == "\n"))
            {
                uno::Reference<beans::XPropertySet> xDocProps(GetTextDocument(), uno::UNO_QUERY);
                static const OUStringLiteral aRecordChanges(u"RecordChanges");
                uno::Any aPreviousValue(xDocProps->getPropertyValue(aRecordChanges));

                // disable redlining for this operation, otherwise we might
                // end up with an unwanted recorded deletion
                xDocProps->setPropertyValue(aRecordChanges, uno::Any(false));

                // delete
                xCursor->setString(OUString());

                // call to xCursor->setString possibly did remove final bookmark
                // from previous paragraph. We need to restore it, if there was any.
                if (sLastBookmarkName.getLength())
                {
                    OUString sBookmarkNameAfterRemoval = lcl_FindLastBookmark(xCursor);
                    if (sBookmarkNameAfterRemoval.isEmpty())
                    {
                        // Yes, it was removed. Restore
                        uno::Reference<text::XTextContent> xBookmark(
                            m_xTextFactory->createInstance("com.sun.star.text.Bookmark"),
                            uno::UNO_QUERY_THROW);

                        uno::Reference<container::XNamed> xBkmNamed(xBookmark,
                                                                    uno::UNO_QUERY_THROW);
                        xBkmNamed->setName(sLastBookmarkName);
                        xTextAppend->insertTextContent(
                            uno::Reference<text::XTextRange>(xCursor, uno::UNO_QUERY_THROW),
                            xBookmark, !xCursor->isCollapsed());
                    }
                }
                // restore redline options again
                xDocProps->setPropertyValue(aRecordChanges, aPreviousValue);
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
    m_bIsLastParaInSection = bIsLast;
}


void DomainMapper_Impl::SetIsFirstParagraphInSection( bool bIsFirst )
{
    m_bIsFirstParaInSection = bIsFirst;
}

void DomainMapper_Impl::SetIsFirstParagraphInSectionAfterRedline( bool bIsFirstAfterRedline )
{
    m_bIsFirstParaInSectionAfterRedline = bIsFirstAfterRedline;
}

bool DomainMapper_Impl::GetIsFirstParagraphInSection( bool bAfterRedline ) const
{
    // Anchored objects may include multiple paragraphs,
    // and none of them should be considered the first para in section.
    return ( bAfterRedline ? m_bIsFirstParaInSectionAfterRedline : m_bIsFirstParaInSection )
                && !IsInShape()
                && !m_bIsInComments
                && !IsInFootOrEndnote();
}

void DomainMapper_Impl::SetIsFirstParagraphInShape(bool bIsFirst)
{
    m_bIsFirstParaInShape = bIsFirst;
}

void DomainMapper_Impl::SetIsDummyParaAddedForTableInSection( bool bIsAdded )
{
    m_bDummyParaAddedForTableInSection = bIsAdded;
    m_bDummyParaAddedForTableInSectionPage = bIsAdded;
}

void DomainMapper_Impl::SetIsDummyParaAddedForTableInSectionPage( bool bIsAdded )
{
    m_bDummyParaAddedForTableInSectionPage = bIsAdded;
}


void DomainMapper_Impl::SetIsTextFrameInserted( bool bIsInserted )
{
    m_bTextFrameInserted  = bIsInserted;
}


void DomainMapper_Impl::SetParaSectpr(bool bParaSectpr)
{
    m_bParaSectpr = bParaSectpr;
}


void DomainMapper_Impl::SetSdt(bool bSdt)
{
    m_bSdt = bSdt;

    if (m_bSdt && !m_aTextAppendStack.empty())
    {
        m_xSdtEntryStart = GetTopTextAppend()->getEnd();
    }
    else
    {
        m_xSdtEntryStart.clear();
    }
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
            m_pLastSectionContext = m_aPropertyStacks[eId].top();
        }
    }
    else if (eId == CONTEXT_CHARACTER)
    {
        m_pLastCharacterContext = m_aPropertyStacks[eId].top();
        // Sadly an assert about deferredCharacterProperties being empty is not possible
        // here, because appendTextPortion() may not be called for every character section.
        deferredCharacterProperties.clear();
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
        sName = m_sCurrentParaStyleName;

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
        if ( pEntry && !pEntry->sConvertedStyleName.isEmpty() )
        {
            if ( !m_bInStyleSheetImport )
                m_sDefaultParaStyleName = pEntry->sConvertedStyleName;
            return pEntry->sConvertedStyleName;
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
        if(pEntry->pProperties)
        {
            std::optional<PropertyMap::Property> aProperty =
                    pEntry->pProperties->getProperty(eId);
            if( aProperty )
            {
                if (pIsDocDefault)
                    *pIsDocDefault = pEntry->pProperties->isDocDefault(eId);

                return aProperty->second;
            }
        }
        //search until the property is set or no parent is available
        StyleSheetEntryPtr pNewEntry;
        if ( !pEntry->sBaseStyleIdentifier.isEmpty() )
            pNewEntry = GetStyleSheetTable()->FindStyleSheetByISTD(pEntry->sBaseStyleIdentifier);

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
            new ListsManager( m_rDMapper, m_xTextFactory );
    return m_pListTable;
}


void DomainMapper_Impl::deferBreak( BreakType deferredBreakType)
{
    switch (deferredBreakType)
    {
    case COLUMN_BREAK:
            m_bIsColumnBreakDeferred = true;
        break;
    case PAGE_BREAK:
            // See SwWW8ImplReader::HandlePageBreakChar(), page break should be
            // ignored inside tables.
            if (m_nTableDepth > 0)
                return;

            m_bIsPageBreakDeferred = true;
        break;
    default:
        return;
    }
}

bool DomainMapper_Impl::isBreakDeferred( BreakType deferredBreakType )
{
    switch (deferredBreakType)
    {
    case COLUMN_BREAK:
        return m_bIsColumnBreakDeferred;
    case PAGE_BREAK:
        return m_bIsPageBreakDeferred;
    default:
        return false;
    }
}

void DomainMapper_Impl::clearDeferredBreak(BreakType deferredBreakType)
{
    switch (deferredBreakType)
    {
    case COLUMN_BREAK:
        m_bIsColumnBreakDeferred = false;
        break;
    case PAGE_BREAK:
        m_bIsPageBreakDeferred = false;
        break;
    default:
        break;
    }
}

void DomainMapper_Impl::clearDeferredBreaks()
{
    m_bIsColumnBreakDeferred = false;
    m_bIsPageBreakDeferred = false;
}

void DomainMapper_Impl::setSdtEndDeferred(bool bSdtEndDeferred)
{
    m_bSdtEndDeferred = bSdtEndDeferred;
}

bool DomainMapper_Impl::isSdtEndDeferred() const
{
    return m_bSdtEndDeferred;
}

void DomainMapper_Impl::setParaSdtEndDeferred(bool bParaSdtEndDeferred)
{
    m_bParaSdtEndDeferred = bParaSdtEndDeferred;
}

bool DomainMapper_Impl::isParaSdtEndDeferred() const
{
    return m_bParaSdtEndDeferred;
}

static void lcl_MoveBorderPropertiesToFrame(std::vector<beans::PropertyValue>& rFrameProperties,
    uno::Reference<text::XTextRange> const& xStartTextRange,
    uno::Reference<text::XTextRange> const& xEndTextRange )
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

        for( size_t nProperty = 0; nProperty < SAL_N_ELEMENTS( aBorderProperties ); ++nProperty)
        {
            OUString sPropertyName = getPropertyName(aBorderProperties[nProperty]);
            beans::PropertyValue aValue;
            aValue.Name = sPropertyName;
            aValue.Value = xTextRangeProperties->getPropertyValue(sPropertyName);
            rFrameProperties.push_back(aValue);
            if( nProperty < 4 )
                xTextRangeProperties->setPropertyValue( sPropertyName, uno::makeAny(table::BorderLine2()));
        }
    }
    catch( const uno::Exception& )
    {
    }
}


static void lcl_AddRangeAndStyle(
    ParagraphPropertiesPtr const & pToBeSavedProperties,
    uno::Reference< text::XTextAppend > const& xTextAppend,
    const PropertyMapPtr& pPropertyMap,
    TextAppendContext const & rAppendContext)
{
    uno::Reference<text::XParagraphCursor> xParaCursor(
        xTextAppend->createTextCursorByRange( rAppendContext.xInsertPosition.is() ? rAppendContext.xInsertPosition : xTextAppend->getEnd()), uno::UNO_QUERY_THROW );
    pToBeSavedProperties->SetEndingRange(xParaCursor->getStart());
    xParaCursor->gotoStartOfParagraph( false );

    pToBeSavedProperties->SetStartingRange(xParaCursor->getStart());
    if(pPropertyMap)
    {
        std::optional<PropertyMap::Property> aParaStyle = pPropertyMap->getProperty(PROP_PARA_STYLE_NAME);
        if( aParaStyle )
        {
            OUString sName;
            aParaStyle->second >>= sName;
            pToBeSavedProperties->SetParaStyleName(sName);
        }
    }
}


//define some default frame width - 0cm ATM: this allow the frame to be wrapped around the text
constexpr sal_Int32 DEFAULT_FRAME_MIN_WIDTH = 0;
constexpr sal_Int32 DEFAULT_FRAME_MIN_HEIGHT = 0;
constexpr sal_Int32 DEFAULT_VALUE = 0;

void DomainMapper_Impl::CheckUnregisteredFrameConversion( )
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
    try
    {
        StyleSheetEntryPtr pParaStyle =
            GetStyleSheetTable()->FindStyleSheetByConvertedStyleName(rAppendContext.pLastParagraphProperties->GetParaStyleName());

        std::vector<beans::PropertyValue> aFrameProperties;

        if ( pParaStyle )
        {
            const StyleSheetPropertyMap* pStyleProperties = pParaStyle->pProperties.get();
            if (!pStyleProperties)
                return;
            sal_Int32 nWidth =
                rAppendContext.pLastParagraphProperties->Getw() > 0 ?
                    rAppendContext.pLastParagraphProperties->Getw() :
                    pStyleProperties->Getw();
            bool bAutoWidth = nWidth < 1;
            if( bAutoWidth )
                nWidth = DEFAULT_FRAME_MIN_WIDTH;
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_WIDTH), nWidth));

            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_HEIGHT),
                rAppendContext.pLastParagraphProperties->Geth() > 0 ?
                    rAppendContext.pLastParagraphProperties->Geth() :
                    pStyleProperties->Geth() > 0 ? pStyleProperties->Geth() : DEFAULT_FRAME_MIN_HEIGHT));

            sal_Int16 nhRule = sal_Int16(
                rAppendContext.pLastParagraphProperties->GethRule() >= 0 ?
                    rAppendContext.pLastParagraphProperties->GethRule() :
                    pStyleProperties->GethRule());
            if ( nhRule < 0 )
            {
                if ( rAppendContext.pLastParagraphProperties->Geth() >= 0 ||
                    pStyleProperties->GethRule() >= 0 )
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
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_SIZE_TYPE), nhRule));

            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_WIDTH_TYPE), bAutoWidth ?  text::SizeType::MIN : text::SizeType::FIX));

            if (const std::optional<sal_Int16> nDirection = PopFrameDirection())
            {
                aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_FRM_DIRECTION), *nDirection));
            }

            sal_Int16 nHoriOrient = sal_Int16(
                rAppendContext.pLastParagraphProperties->GetxAlign() >= 0 ?
                    rAppendContext.pLastParagraphProperties->GetxAlign() :
                    pStyleProperties->GetxAlign() >= 0 ? pStyleProperties->GetxAlign() : text::HoriOrientation::NONE );
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_HORI_ORIENT), nHoriOrient));

            //set a non negative default value
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_HORI_ORIENT_POSITION),
                rAppendContext.pLastParagraphProperties->IsxValid() ?
                    rAppendContext.pLastParagraphProperties->Getx() :
                    pStyleProperties->IsxValid() ? pStyleProperties->Getx() : DEFAULT_VALUE));

            //Default the anchor in case FramePr_hAnchor is missing ECMA 17.3.1.11
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_HORI_ORIENT_RELATION), sal_Int16(
                rAppendContext.pLastParagraphProperties->GethAnchor() >= 0 ?
                    rAppendContext.pLastParagraphProperties->GethAnchor() :
                pStyleProperties->GethAnchor() >=0 ? pStyleProperties->GethAnchor() : text::RelOrientation::FRAME )));

            sal_Int16 nVertOrient = sal_Int16(
                rAppendContext.pLastParagraphProperties->GetyAlign() >= 0 ?
                    rAppendContext.pLastParagraphProperties->GetyAlign() :
                    pStyleProperties->GetyAlign() >= 0 ? pStyleProperties->GetyAlign() : text::VertOrientation::NONE );
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_VERT_ORIENT), nVertOrient));

            //set a non negative default value
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_VERT_ORIENT_POSITION),
                rAppendContext.pLastParagraphProperties->IsyValid() ?
                    rAppendContext.pLastParagraphProperties->Gety() :
                    pStyleProperties->IsyValid() ? pStyleProperties->Gety() : DEFAULT_VALUE));

            //Default the anchor in case FramePr_vAnchor is missing ECMA 17.3.1.11
            if (rAppendContext.pLastParagraphProperties->GetWrap() == text::WrapTextMode::WrapTextMode_MAKE_FIXED_SIZE &&
                pStyleProperties->GetWrap() == text::WrapTextMode::WrapTextMode_MAKE_FIXED_SIZE)
            {
                aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_VERT_ORIENT_RELATION), sal_Int16(
                    rAppendContext.pLastParagraphProperties->GetvAnchor() >= 0 ?
                    rAppendContext.pLastParagraphProperties->GetvAnchor() :
                    pStyleProperties->GetvAnchor() >= 0 ? pStyleProperties->GetvAnchor() : text::RelOrientation::FRAME)));
            }
            else
            {
                aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_VERT_ORIENT_RELATION), sal_Int16(
                    rAppendContext.pLastParagraphProperties->GetvAnchor() >= 0 ?
                    rAppendContext.pLastParagraphProperties->GetvAnchor() :
                    pStyleProperties->GetvAnchor() >= 0 ? pStyleProperties->GetvAnchor() : text::RelOrientation::PAGE_PRINT_AREA)));
            }

            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_SURROUND),
                rAppendContext.pLastParagraphProperties->GetWrap() != text::WrapTextMode::WrapTextMode_MAKE_FIXED_SIZE
                ? rAppendContext.pLastParagraphProperties->GetWrap()
                : pStyleProperties->GetWrap() != text::WrapTextMode::WrapTextMode_MAKE_FIXED_SIZE
                  ? pStyleProperties->GetWrap()
                  : text::WrapTextMode_NONE ));

            /** FDO#73546 : distL & distR should be unsigned integers <Ecma 20.4.3.6>
                Swapped the array elements 11,12 & 13,14 since 11 & 12 are
                LEFT & RIGHT margins and 13,14 are TOP and BOTTOM margins respectively.
            */
            sal_Int32 nRightDist;
            sal_Int32 nLeftDist = nRightDist =
                rAppendContext.pLastParagraphProperties->GethSpace() >= 0 ?
                rAppendContext.pLastParagraphProperties->GethSpace() :
                pStyleProperties->GethSpace() >= 0 ? pStyleProperties->GethSpace() : 0;

            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_LEFT_MARGIN), nHoriOrient == text::HoriOrientation::LEFT ? 0 : nLeftDist));
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_RIGHT_MARGIN), nHoriOrient == text::HoriOrientation::RIGHT ? 0 : nRightDist));

            sal_Int32 nBottomDist;
            sal_Int32 nTopDist = nBottomDist =
                rAppendContext.pLastParagraphProperties->GetvSpace() >= 0 ?
                rAppendContext.pLastParagraphProperties->GetvSpace() :
                pStyleProperties->GetvSpace() >= 0 ? pStyleProperties->GetvSpace() : 0;

            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_TOP_MARGIN), nVertOrient == text::VertOrientation::TOP ? 0 : nTopDist));
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_BOTTOM_MARGIN), nVertOrient == text::VertOrientation::BOTTOM ? 0 : nBottomDist));
            // If there is no fill, the Word default is 100% transparency.
            // Otherwise CellColorHandler has priority, and this setting
            // will be ignored.
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_BACK_COLOR_TRANSPARENCY), sal_Int32(100)));

            uno::Sequence<beans::PropertyValue> aGrabBag( comphelper::InitPropertySequence({
                    { "ParaFrameProperties", uno::Any(rAppendContext.pLastParagraphProperties->IsFrameMode()) }
            }));
            aFrameProperties.push_back(comphelper::makePropertyValue("FrameInteropGrabBag", aGrabBag));

            lcl_MoveBorderPropertiesToFrame(aFrameProperties,
                rAppendContext.pLastParagraphProperties->GetStartingRange(),
                rAppendContext.pLastParagraphProperties->GetEndingRange());
        }
        else
        {
            sal_Int32 nWidth = rAppendContext.pLastParagraphProperties->Getw();
            bool bAutoWidth = nWidth < 1;
            if( bAutoWidth )
                nWidth = DEFAULT_FRAME_MIN_WIDTH;
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_WIDTH), nWidth));

            sal_Int16 nhRule = sal_Int16(rAppendContext.pLastParagraphProperties->GethRule());
            if ( nhRule < 0 )
            {
                if ( rAppendContext.pLastParagraphProperties->Geth() >= 0 )
                {
                    // [MS-OE376] Word uses a default value of atLeast for
                    // this attribute when the value of the h attribute is not 0.
                    nhRule = text::SizeType::MIN;
                }
                else
                {
                    nhRule = text::SizeType::VARIABLE;
                }
            }
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_SIZE_TYPE), nhRule));

            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_WIDTH_TYPE), bAutoWidth ?  text::SizeType::MIN : text::SizeType::FIX));

            sal_Int16 nHoriOrient = sal_Int16(
                rAppendContext.pLastParagraphProperties->GetxAlign() >= 0 ?
                    rAppendContext.pLastParagraphProperties->GetxAlign() :
                    text::HoriOrientation::NONE );
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_HORI_ORIENT), nHoriOrient));

            sal_Int16 nVertOrient = sal_Int16(
                rAppendContext.pLastParagraphProperties->GetyAlign() >= 0 ?
                    rAppendContext.pLastParagraphProperties->GetyAlign() :
                    text::VertOrientation::NONE );
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_VERT_ORIENT), nVertOrient));

            sal_Int32 nVertDist = rAppendContext.pLastParagraphProperties->GethSpace();
            if( nVertDist < 0 )
                nVertDist = 0;
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_LEFT_MARGIN), nVertOrient == text::VertOrientation::TOP ? 0 : nVertDist));
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_RIGHT_MARGIN), nVertOrient == text::VertOrientation::BOTTOM ? 0 : nVertDist));

            sal_Int32 nHoriDist = rAppendContext.pLastParagraphProperties->GetvSpace();
            if( nHoriDist < 0 )
                nHoriDist = 0;
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_TOP_MARGIN), nHoriOrient == text::HoriOrientation::LEFT ? 0 : nHoriDist));
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_BOTTOM_MARGIN), nHoriOrient == text::HoriOrientation::RIGHT ? 0 : nHoriDist));

            if( rAppendContext.pLastParagraphProperties->Geth() > 0 )
                aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_HEIGHT), rAppendContext.pLastParagraphProperties->Geth()));

            if( rAppendContext.pLastParagraphProperties->IsxValid() )
                aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_HORI_ORIENT_POSITION), rAppendContext.pLastParagraphProperties->Getx()));

            if( rAppendContext.pLastParagraphProperties->GethAnchor() >= 0 )
                aFrameProperties.push_back(comphelper::makePropertyValue("HoriOrientRelation", sal_Int16(rAppendContext.pLastParagraphProperties->GethAnchor())));

            if( rAppendContext.pLastParagraphProperties->IsyValid() )
                aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_VERT_ORIENT_POSITION), rAppendContext.pLastParagraphProperties->Gety()));

            if( rAppendContext.pLastParagraphProperties->GetvAnchor() >= 0 )
                aFrameProperties.push_back(comphelper::makePropertyValue("VertOrientRelation", sal_Int16(rAppendContext.pLastParagraphProperties->GetvAnchor())));

            if( rAppendContext.pLastParagraphProperties->GetWrap() >= text::WrapTextMode_NONE )
                aFrameProperties.push_back(comphelper::makePropertyValue("Surround", rAppendContext.pLastParagraphProperties->GetWrap()));

            lcl_MoveBorderPropertiesToFrame(aFrameProperties,
                rAppendContext.pLastParagraphProperties->GetStartingRange(),
                rAppendContext.pLastParagraphProperties->GetEndingRange());
        }

        //frame conversion has to be executed after table conversion
        RegisterFrameConversion(
            rAppendContext.pLastParagraphProperties->GetStartingRange(),
            rAppendContext.pLastParagraphProperties->GetEndingRange(),
            std::move(aFrameProperties) );
    }
    catch( const uno::Exception& )
    {
    }
}

/// Check if the style or its parent has a list id, recursively.
static sal_Int32 lcl_getListId(const StyleSheetEntryPtr& rEntry, const StyleSheetTablePtr& rStyleTable, bool & rNumberingFromBaseStyle)
{
    const StyleSheetPropertyMap* pEntryProperties = rEntry->pProperties.get();
    if (!pEntryProperties)
        return -1;

    sal_Int32 nListId = pEntryProperties->GetListId();
    // The style itself has a list id.
    if (nListId >= 0)
        return nListId;

    // The style has no parent.
    if (rEntry->sBaseStyleIdentifier.isEmpty())
        return -1;

    const StyleSheetEntryPtr pParent = rStyleTable->FindStyleSheetByISTD(rEntry->sBaseStyleIdentifier);
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

    const StyleSheetPropertyMap* pEntryProperties = pEntry->pProperties.get();
    if (!pEntryProperties)
        return -1;

    nListLevel = pEntryProperties->GetListLevel();
    // The style itself has a list level.
    if (nListLevel >= 0)
        return nListLevel;

    // The style has no parent.
    if (pEntry->sBaseStyleIdentifier.isEmpty())
        return -1;

    const StyleSheetEntryPtr pParent = GetStyleSheetTable()->FindStyleSheetByISTD(pEntry->sBaseStyleIdentifier);
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
        pMyStyle->pProperties->SetListLevel(WW_OUTLINE_MAX);
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

    m_nLastTableCellParagraphDepth = m_nTableCellDepth;
    ParagraphPropertyMap* pParaContext = dynamic_cast< ParagraphPropertyMap* >( pPropertyMap.get() );
    if (m_aTextAppendStack.empty())
        return;
    TextAppendContext& rAppendContext = m_aTextAppendStack.top();
    uno::Reference< text::XTextAppend > xTextAppend(rAppendContext.xTextAppend);
#ifdef DBG_UTIL
    TagLogger::getInstance().attribute("isTextAppend", sal_uInt32(xTextAppend.is()));
#endif

    const StyleSheetEntryPtr pEntry = GetStyleSheetTable()->FindStyleSheetByConvertedStyleName( GetCurrentParaStyleName() );
    OSL_ENSURE( pEntry, "no style sheet found" );
    const StyleSheetPropertyMap* pStyleSheetProperties = pEntry ? pEntry->pProperties.get() : nullptr;
    sal_Int32 nListId = pParaContext ? pParaContext->GetListId() : -1;
    bool isNumberingViaStyle(false);
    bool isNumberingViaRule = nListId > -1;
    if ( !bRemove && pStyleSheetProperties && pParaContext )
    {
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
            pParaContext->Insert( PROP_NUMBERING_LEVEL, uno::makeAny(nListLevel), false );

        auto const pList(GetListTable()->GetList(nListId));
        if (pList && !pParaContext->isSet(PROP_NUMBERING_STYLE_NAME))
        {
            // ListLevel 9 means Body Level/no numbering.
            if (bNoNumbering || nListLevel == 9)
            {
                pParaContext->Insert(PROP_NUMBERING_STYLE_NAME, uno::makeAny(OUString()), true);
                pParaContext->Erase(PROP_NUMBERING_LEVEL);
            }
            else if ( !isNumberingViaRule )
            {
                isNumberingViaStyle = true;
                // Since LO7.0/tdf#131321 fixed the loss of numbering in styles, this OUGHT to be obsolete,
                // but now other new/critical LO7.0 code expects it, and perhaps some corner cases still need it as well.
                pParaContext->Insert(PROP_NUMBERING_STYLE_NAME, uno::makeAny(pList->GetStyleName()), true);
            }
            else
            {
                // we have direct numbering, as well as paragraph-style numbering.
                // Apply the style if it uses the same list as the direct numbering,
                // otherwise the directly-applied-to-paragraph status will be lost,
                // and the priority of the numbering-style-indents will be lowered. tdf#133000
                bool bDummy;
                if (nListId == lcl_getListId(pEntry, GetStyleSheetTable(), bDummy))
                    pParaContext->Insert( PROP_NUMBERING_STYLE_NAME, uno::makeAny(pList->GetStyleName()), true );
            }
        }

        if ( isNumberingViaStyle )
        {
            // When numbering is defined by the paragraph style, then the para-style indents have priority.
            // But since import has just copied para-style's PROP_NUMBERING_STYLE_NAME directly onto the paragraph,
            // the numbering indents now have the priority.
            // So now import must also copy the para-style indents directly onto the paragraph to compensate.
            std::optional<PropertyMap::Property> oProperty;
            const StyleSheetEntryPtr pParent = (!pEntry->sBaseStyleIdentifier.isEmpty()) ? GetStyleSheetTable()->FindStyleSheetByISTD(pEntry->sBaseStyleIdentifier) : nullptr;
            const StyleSheetPropertyMap* pParentProperties = pParent ? pParent->pProperties.get() : nullptr;
            if (!pEntry->sBaseStyleIdentifier.isEmpty())
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
                    pParaContext->Insert(PROP_PARA_FIRST_LINE_INDENT, uno::makeAny(nFirstLineIndent), /*bOverwrite=*/false);
                if (nParaLeftMargin != 0)
                    pParaContext->Insert(PROP_PARA_LEFT_MARGIN, uno::makeAny(nParaLeftMargin), /*bOverwrite=*/false);

                pParaContext->Insert(PROP_PARA_RIGHT_MARGIN, uno::makeAny(nParaRightMargin), /*bOverwrite=*/false);
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
                    pParaContext->Insert(PROP_PARA_FIRST_LINE_INDENT, uno::makeAny(nFirstLineIndent), /*bOverwrite=*/false);
                if (nParaLeftMargin != 0)
                    pParaContext->Insert(PROP_PARA_LEFT_MARGIN, uno::makeAny(nParaLeftMargin), /*bOverwrite=*/false);
            }
        }

        if (nListId == 0 && !pList)
        {
            // Seems situation with listid=0 and missing list definition is used by DOCX
            // to remove numbering defined previously. But some default numbering attributes
            // are still applied. This is first line indent, probably something more?
            if (!pParaContext->isSet(PROP_PARA_FIRST_LINE_INDENT))
                pParaContext->Insert(PROP_PARA_FIRST_LINE_INDENT, uno::makeAny(sal_Int16(0)), false);
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
                 (m_bFirstParagraphInCell && m_nTableDepth > 0 && m_nTableDepth == m_nTableCellDepth) )
            {
                // export requires grabbag to match top_margin, so keep them in sync
                if (nBeforeAutospacing && bIsAutoSet)
                    pParaContext->Insert( PROP_PARA_TOP_MARGIN_BEFORE_AUTO_SPACING, uno::makeAny( sal_Int32(0) ),true, PARA_GRAB_BAG );
                nBeforeAutospacing = 0;
            }
        }
        pParaContext->Insert(PROP_PARA_TOP_MARGIN, uno::makeAny(nBeforeAutospacing));
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
        pParaContext->Insert(PROP_PARA_BOTTOM_MARGIN, uno::makeAny(nAfterAutospacing));
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

            bool bIsDropCap =
                pParaContext->IsFrameMode() &&
                sal::static_int_cast<Id>(pParaContext->GetDropCap()) != NS_ooxml::LN_Value_doc_ST_DropCap_none;

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
                pToBeSavedProperties = new ParagraphProperties(*pParaContext);
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
                    if( pParaContext->IsFrameMode() )
                        pToBeSavedProperties = new ParagraphProperties(*pParaContext);
                }
                else if(*rAppendContext.pLastParagraphProperties == *pParaContext )
                {
                    //handles (7)
                    rAppendContext.pLastParagraphProperties->SetEndingRange(rAppendContext.xInsertPosition.is() ? rAppendContext.xInsertPosition : xTextAppend->getEnd());
                    bKeepLastParagraphProperties = true;
                }
                else
                {
                    //handles (8)(9) and completes (6)
                    CheckUnregisteredFrameConversion( );

                    // If different frame properties are set on this paragraph, keep them.
                    if ( !bIsDropCap && pParaContext->IsFrameMode() )
                    {
                        pToBeSavedProperties = new ParagraphProperties(*pParaContext);
                        lcl_AddRangeAndStyle(pToBeSavedProperties, xTextAppend, pPropertyMap, rAppendContext);
                    }
                }
            }
            else
            {
                // (1) doesn't need handling

                if( !bIsDropCap && pParaContext->IsFrameMode() )
                {
                    pToBeSavedProperties = new ParagraphProperties(*pParaContext);
                    lcl_AddRangeAndStyle(pToBeSavedProperties, xTextAppend, pPropertyMap, rAppendContext);
                }
            }
            std::vector<beans::PropertyValue> aProperties;
            if (pPropertyMap)
            {
                aProperties = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(pPropertyMap->GetPropertyValues());
            }
            // TODO: this *should* work for RTF but there are test failures, maybe rtftok doesn't distinguish between formatting for the paragraph marker and for the paragraph as a whole; needs investigation
            if (pPropertyMap && IsOOXMLImport())
            {
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
                        0, uno::makeAny(comphelper::containerToSequence(charProperties)), beans::PropertyState_DIRECT_VALUE));
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
                    if (m_bParaHadField && !m_bIsInComments && !xTOCMarkerCursor.is())
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
                    if (m_xPreviousParagraph.is() && (isNumberingViaRule || isNumberingViaStyle))
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
                            if (m_xPreviousParagraph.is())
                            {
                                uno::Reference<container::XNamed> xPreviousNumberingRules(m_xPreviousParagraph->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
                                if (xPreviousNumberingRules.is())
                                    aPreviousNumberingName = xPreviousNumberingRules->getName();
                            }
                        }
                        else if ( m_xPreviousParagraph->getPropertySetInfo()->hasPropertyByName("NumberingStyleName") &&
                                // don't update before tables
                                (m_nTableDepth == 0 || !m_bFirstParagraphInCell))
                        {
                            aCurrentNumberingName = GetListStyleName(nListId);
                            m_xPreviousParagraph->getPropertyValue("NumberingStyleName") >>= aPreviousNumberingName;
                        }

                        if (!aPreviousNumberingName.isEmpty() && aCurrentNumberingName == aPreviousNumberingName)
                        {
                            uno::Sequence<beans::PropertyValue> aPrevPropertiesSeq;
                            m_xPreviousParagraph->getPropertyValue("ParaInteropGrabBag") >>= aPrevPropertiesSeq;
                            auto aPrevProperties = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(aPrevPropertiesSeq);
                            bool bParaAutoBefore = m_bParaAutoBefore || std::any_of(aPrevProperties.begin(), aPrevProperties.end(), [](const beans::PropertyValue& rValue)
                            {
                                    return rValue.Name == "ParaTopMarginBeforeAutoSpacing";
                            });
                            // if style based spacing was set to auto in the previous paragraph, style of the actual paragraph must be the same
                            if (bParaAutoBefore && !m_bParaAutoBefore && m_xPreviousParagraph->getPropertySetInfo()->hasPropertyByName("ParaStyleName"))
                            {
                               auto itParaStyle = std::find_if(aProperties.begin(), aProperties.end(), [](const beans::PropertyValue& rValue)
                               {
                                   return rValue.Name == "ParaStyleName";
                               });
                               bParaAutoBefore = itParaStyle != aProperties.end() &&
                                   m_xPreviousParagraph->getPropertyValue("ParaStyleName") == itParaStyle->Value;
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

                            bool bPrevParaAutoAfter = std::any_of(aPrevProperties.begin(), aPrevProperties.end(), [](const beans::PropertyValue& rValue)
                            {
                                return rValue.Name == "ParaBottomMarginAfterAutoSpacing";
                            });
                            if (bPrevParaAutoAfter)
                            {
                                // Previous after spacing is set to auto, set previous after space to 0.
                                m_xPreviousParagraph->setPropertyValue("ParaBottomMargin", uno::makeAny(static_cast<sal_Int32>(0)));
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
                    m_xPreviousParagraph.set(xTextRange, uno::UNO_QUERY);

                    if (m_xPreviousParagraph.is() && // null for SvxUnoTextBase
                        (isNumberingViaStyle || isNumberingViaRule))
                    {
                        assert(pParaContext);
                        if (ListDef::Pointer const& pList = m_pListTable->GetList(nListId))
                        {   // styles could refer to non-existing lists...
                            AbstractListDef::Pointer const& pAbsList =
                                    pList->GetAbstractDefinition();
                            if (pAbsList &&
                                // SvxUnoTextRange doesn't have ListId
                                m_xPreviousParagraph->getPropertySetInfo()->hasPropertyByName("ListId"))
                            {
                                OUString paraId;
                                m_xPreviousParagraph->getPropertyValue("ListId") >>= paraId;
                                if (!paraId.isEmpty()) // must be on some list?
                                {
                                    OUString const listId = pAbsList->MapListId(paraId);
                                    if (listId != paraId)
                                    {
                                        m_xPreviousParagraph->setPropertyValue("ListId", uno::makeAny(listId));
                                    }
                                }
                            }
                            if (pList->GetCurrentLevel())
                            {
                                sal_Int16 nOverrideLevel = pList->GetCurrentLevel()->GetStartOverride();
                                if (nOverrideLevel != -1 && m_aListOverrideApplied.find(nListId) == m_aListOverrideApplied.end())
                                {
                                    // Apply override: we have override instruction for this level
                                    // And this was not done for this list before: we can do this only once on first occurrence
                                    // of list with override
                                    // TODO: Not tested variant with different levels override in different lists.
                                    // Probably m_aListOverrideApplied as a set of overridden listids is not sufficient
                                    // and we need to register level overrides separately.
                                    m_xPreviousParagraph->setPropertyValue("ParaIsNumberingRestart", uno::makeAny(true));
                                    m_xPreviousParagraph->setPropertyValue("NumberingStartValue", uno::makeAny(nOverrideLevel));
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

                    // We're no longer right after a table conversion.
                    m_bConvertedTable = false;

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
                    if ( m_nTableDepth > 0 && nMode > 0 && nMode <= 14 )
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
                        xCur->goRight(2, false);
                    }

                    xCur->goLeft( 1 , true );
                    // Extend the redline ranges for empty paragraphs
                    if ( !m_bParaChanged && m_previousRedline )
                        CreateRedline( xCur, m_previousRedline );
                    CheckParaMarkerRedline( xCur );
                }

                css::uno::Reference<css::beans::XPropertySet> xParaProps(xTextRange, uno::UNO_QUERY);

                // table style precedence and not hidden shapes anchored to hidden empty table paragraphs
                if (xParaProps && (m_nTableDepth > 0 || !m_aAnchoredObjectAnchors.empty()) )
                {
                    // table style has got bigger precedence than docDefault style
                    // collect these pending paragraph properties to process in endTable()
                    uno::Reference<text::XTextCursor> xCur = xTextRange->getText( )->createTextCursor( );
                    xCur->gotoEnd(false);
                    xCur->goLeft(1, false);
                    uno::Reference<text::XTextCursor> xCur2 =  xTextRange->getText()->createTextCursorByRange(xCur);
                    uno::Reference<text::XParagraphCursor> xParaCursor(xCur2, uno::UNO_QUERY_THROW);
                    xParaCursor->gotoStartOfParagraph(false);
                    if (m_nTableDepth > 0)
                    {
                        TableParagraph aPending{xParaCursor, xCur, pParaContext, xParaProps, std::set<OUString>()};
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
                                    xProp->setPropertyValue(getPropertyName(PROP_CHAR_HIDDEN), uno::makeAny(false));
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
                                    xParaProps->setPropertyValue("ParaLeftMargin", uno::makeAny(nParaLeftMargin));
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
                                    xParaProps->setPropertyValue("ParaFirstLineIndent", uno::makeAny(nFirstLineIndent));
                            }
                        }
                    }
                }

                // fix table paragraph properties
                if ( xTextRange.is() && xParaProps && m_nTableDepth > 0 )
                {
                    // tdf#128959 table paragraphs haven't got window and orphan controls
                    uno::Any aAny = uno::makeAny(static_cast<sal_Int8>(0));
                    xParaProps->setPropertyValue("ParaOrphans", aAny);
                    xParaProps->setPropertyValue("ParaWidows", aAny);
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
    if( (!bIgnoreFrameState && pParaContext && pParaContext->IsFrameMode()) || (bIgnoreFrameState && GetIsPreviousParagraphFramed()) )
        SetIsPreviousParagraphFramed(true);
    else
        SetIsPreviousParagraphFramed(false);

    m_bRemoveThisParagraph = false;
    if( !IsInHeaderFooter() && !IsInShape() && (!pParaContext || !pParaContext->IsFrameMode()) )
    { // If the paragraph is in a frame, shape or header/footer, it's not a paragraph of the section itself.
        SetIsFirstParagraphInSection(false);
        // don't count an empty deleted paragraph as first paragraph in section to avoid of
        // the deletion of the next empty paragraph later, resulting loss of the associated page break
        if (!m_previousRedline || m_bParaChanged)
        {
            SetIsFirstParagraphInSectionAfterRedline(false);
            SetIsLastParagraphInSection(false);
        }
    }
    m_previousRedline.clear();
    m_bParaChanged = false;

    if (m_bIsInComments && pParaContext)
    {
        if (const OUString sParaId = pParaContext->GetParaId(); !sParaId.isEmpty())
        {
            if (const auto& item = m_aCommentProps.find(sParaId); item != m_aCommentProps.end())
            {
                m_bAnnotationResolved = item->second.bDone;
            }
        }
    }

    if (m_bIsFirstParaInShape)
        m_bIsFirstParaInShape = false;

    if (pParaContext)
    {
        // Reset the frame properties for the next paragraph
        pParaContext->ResetFrameProperties();
    }

    SetIsOutsideAParagraph(true);
    m_bParaHadField = false;

    // don't overwrite m_bFirstParagraphInCell in table separator nodes
    // and in text boxes anchored to the first paragraph of table cells
    if (m_nTableDepth > 0 && m_nTableDepth == m_nTableCellDepth && !IsInShape())
        m_bFirstParagraphInCell = false;

    m_bParaAutoBefore = false;
    m_bParaWithInlineObject = false;

#ifdef DBG_UTIL
    TagLogger::getInstance().endElement();
#endif

}

void DomainMapper_Impl::appendTextPortion( const OUString& rString, const PropertyMapPtr& pPropertyMap )
{
    if (m_bDiscardHeaderFooter)
        return;

    if (m_aTextAppendStack.empty())
        return;
    // Before placing call to processDeferredCharacterProperties(), TopContextType should be CONTEXT_CHARACTER
    // processDeferredCharacterProperties() invokes only if character inserted
    if( pPropertyMap == m_pTopContext && !deferredCharacterProperties.empty() && (GetTopContextType() == CONTEXT_CHARACTER) )
        processDeferredCharacterProperties();
    uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
    if (!xTextAppend.is() || !hasTableManager() || getTableManager().isIgnore())
        return;

    try
    {
        // If we are in comments, then disable CharGrabBag, comment text doesn't support that.
        uno::Sequence< beans::PropertyValue > aValues = pPropertyMap->GetPropertyValues(/*bCharGrabBag=*/!m_bIsInComments);

        if (m_bStartTOC || m_bStartIndex || m_bStartBibliography)
            for( auto& rValue : asNonConstRange(aValues) )
            {
                if (rValue.Name == "CharHidden")
                    rValue.Value <<= false;
            }

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

        uno::Reference< text::XTextRange > xTextRange;
        if (m_aTextAppendStack.top().xInsertPosition.is())
        {
            xTextRange = xTextAppend->insertTextPortion(rString, aValues, m_aTextAppendStack.top().xInsertPosition);
            m_aTextAppendStack.top().xCursor->gotoRange(xTextRange->getEnd(), true);
        }
        else
        {
            if (m_bStartTOC || m_bStartIndex || m_bStartBibliography || m_nStartGenericField != 0)
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
                    xTextRange = xTextAppend->insertTextPortion(rString, aValues, xTOCTextCursor);
                    SAL_WARN_IF(!xTextRange.is(), "writerfilter.dmapper", "insertTextPortion failed");
                    if (!xTextRange.is())
                        throw uno::Exception("insertTextPortion failed", nullptr);
                    m_bTextInserted = true;
                    xTOCTextCursor->gotoRange(xTextRange->getEnd(), true);
                    if (m_nStartGenericField == 0)
                    {
                        m_aTextAppendStack.push(TextAppendContext(xTextAppend, xTOCTextCursor));
                    }
                }
            }
            else
            {
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

        // reset moveFrom data of non-terminating runs of the paragraph
        if ( m_pParaMarkerRedlineMoveFrom )
        {
            m_pParaMarkerRedlineMoveFrom.clear();
        }
        CheckRedline( xTextRange );
        m_bParaChanged = true;

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
        uno::Reference< text::XTextContent > xOLE( m_xTextFactory->createInstance("com.sun.star.text.TextEmbeddedObject"), uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xOLEProperties(xOLE, uno::UNO_QUERY_THROW);

        OUString aCLSID = pOLEHandler->getCLSID(m_xComponentContext);
        if (aCLSID.isEmpty())
            xOLEProperties->setPropertyValue(getPropertyName( PROP_STREAM_NAME ),
                            uno::makeAny( rStreamName ));
        else
            xOLEProperties->setPropertyValue("CLSID", uno::makeAny(aCLSID));

        OUString aDrawAspect = pOLEHandler->GetDrawAspect();
        if(!aDrawAspect.isEmpty())
            xOLEProperties->setPropertyValue("DrawAspect", uno::makeAny(aDrawAspect));

        awt::Size aSize = pOLEHandler->getSize();
        if( !aSize.Width )
            aSize.Width = 1000;
        if( !aSize.Height )
            aSize.Height = 1000;
        xOLEProperties->setPropertyValue(getPropertyName( PROP_WIDTH ),
                        uno::makeAny(aSize.Width));
        xOLEProperties->setPropertyValue(getPropertyName( PROP_HEIGHT ),
                        uno::makeAny(aSize.Height));

        OUString aVisAreaWidth = pOLEHandler->GetVisAreaWidth();
        if(!aVisAreaWidth.isEmpty())
            xOLEProperties->setPropertyValue("VisibleAreaWidth", uno::makeAny(aVisAreaWidth));

        OUString aVisAreaHeight = pOLEHandler->GetVisAreaHeight();
        if(!aVisAreaHeight.isEmpty())
            xOLEProperties->setPropertyValue("VisibleAreaHeight", uno::makeAny(aVisAreaHeight));

        uno::Reference< graphic::XGraphic > xGraphic = pOLEHandler->getReplacement();
        xOLEProperties->setPropertyValue(getPropertyName( PROP_GRAPHIC ),
                        uno::makeAny(xGraphic));
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
            xOLEProperties->setPropertyValue(getPropertyName( PROP_ANCHOR_TYPE ),  uno::makeAny( text::TextContentAnchorType_AS_CHARACTER ) );
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
        uno::Reference< text::XTextContent > xStarMath( m_xTextFactory->createInstance("com.sun.star.text.TextEmbeddedObject"), uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xStarMathProperties(xStarMath, uno::UNO_QUERY_THROW);

        xStarMathProperties->setPropertyValue(getPropertyName( PROP_EMBEDDED_OBJECT ),
            val.getAny());
        // tdf#66405: set zero margins for embedded object
        xStarMathProperties->setPropertyValue(getPropertyName( PROP_LEFT_MARGIN ),
            uno::makeAny(sal_Int32(0)));
        xStarMathProperties->setPropertyValue(getPropertyName( PROP_RIGHT_MARGIN ),
            uno::makeAny(sal_Int32(0)));
        xStarMathProperties->setPropertyValue(getPropertyName( PROP_TOP_MARGIN ),
            uno::makeAny(sal_Int32(0)));
        xStarMathProperties->setPropertyValue(getPropertyName( PROP_BOTTOM_MARGIN ),
            uno::makeAny(sal_Int32(0)));

        uno::Reference< uno::XInterface > xInterface( formula->getComponent(), uno::UNO_QUERY );
        // set zero margins for object's component
        uno::Reference< beans::XPropertySet > xComponentProperties( xInterface, uno::UNO_QUERY_THROW );
        xComponentProperties->setPropertyValue(getPropertyName( PROP_LEFT_MARGIN ),
            uno::makeAny(sal_Int32(0)));
        xComponentProperties->setPropertyValue(getPropertyName( PROP_RIGHT_MARGIN ),
            uno::makeAny(sal_Int32(0)));
        xComponentProperties->setPropertyValue(getPropertyName( PROP_TOP_MARGIN ),
            uno::makeAny(sal_Int32(0)));
        xComponentProperties->setPropertyValue(getPropertyName( PROP_BOTTOM_MARGIN ),
            uno::makeAny(sal_Int32(0)));
        Size size( 1000, 1000 );
        if( oox::FormulaImportBase* formulaimport = dynamic_cast< oox::FormulaImportBase* >( xInterface.get()))
            size = formulaimport->getFormulaSize();
        xStarMathProperties->setPropertyValue(getPropertyName( PROP_WIDTH ),
            uno::makeAny( sal_Int32(size.Width())));
        xStarMathProperties->setPropertyValue(getPropertyName( PROP_HEIGHT ),
            uno::makeAny( sal_Int32(size.Height())));
        xStarMathProperties->setPropertyValue(getPropertyName(PROP_ANCHOR_TYPE),
                uno::makeAny(text::TextContentAnchorType_AS_CHARACTER));
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
    pLastPara->Insert(PROP_PARA_ADJUST, uno::makeAny(nAlign), true);
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
            //the paragraph after this new section is already inserted
            xCursor->goLeft(1, true);
            css::uno::Reference<css::text::XTextRange> xTextRange(xCursor, css::uno::UNO_QUERY_THROW);

            if (css::uno::Reference<css::text::XDocumentIndexesSupplier> xIndexSupplier{
                    GetTextDocument(), css::uno::UNO_QUERY })
            {
                css::uno::Reference<css::text::XTextRangeCompare> xCompare(
                    xTextAppend, css::uno::UNO_QUERY);
                const auto xIndexAccess = xIndexSupplier->getDocumentIndexes();
                for (sal_Int32 i = xIndexAccess->getCount(); i > 0; --i)
                {
                    if (css::uno::Reference<css::text::XDocumentIndex> xIndex{
                            xIndexAccess->getByIndex(i - 1), css::uno::UNO_QUERY })
                    {
                        const auto xIndexTextRange = xIndex->getAnchor();
                        if (xCompare->compareRegionStarts(xTextRange, xIndexTextRange) == 0
                            && xCompare->compareRegionEnds(xTextRange, xIndexTextRange) == 0)
                        {
                            // The boundaries coincide with an index: trying to attach a section
                            // to the range will insert the section inside the index. goRight will
                            // extend the range outside of the index, so that created section will
                            // be around it. Alternatively we could return index section itself
                            // instead : xRet.set(xIndex, uno::UNO_QUERY) - to set its properties,
                            // like columns/fill.
                            xCursor->goRight(1, true);
                            break;
                        }
                    }
                }
            }

            uno::Reference< text::XTextContent > xSection( m_xTextFactory->createInstance("com.sun.star.text.TextSection"), uno::UNO_QUERY_THROW );
            xSection->attach( xTextRange );
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

    uno::Any aEmptyBorder = uno::makeAny(table::BorderLine2());
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

void DomainMapper_Impl::ConvertHeaderFooterToTextFrame(bool bDynamicHeightTop, bool bDynamicHeightBottom)
{
    while (!m_aHeaderFooterTextAppendStack.empty())
    {
        auto aFooterHeader = m_aHeaderFooterTextAppendStack.top();
        if ((aFooterHeader.second && !bDynamicHeightTop) || (!aFooterHeader.second && !bDynamicHeightBottom))
        {
            uno::Reference< text::XTextAppend > xTextAppend = aFooterHeader.first.xTextAppend;
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
            if (!aFooterHeader.second)
                aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_VERT_ORIENT), text::VertOrientation::BOTTOM));

            uno::Reference<text::XTextAppendAndConvert> xBodyText(
                xRangeStart->getText(), uno::UNO_QUERY);
            xBodyText->convertToTextFrame(xTextAppend, xRangeEnd,
                comphelper::containerToSequence(aFrameProperties));
        }
        m_aHeaderFooterTextAppendStack.pop();
    }
}

void DomainMapper_Impl::PushPageHeaderFooter(bool bHeader, SectionPropertyMap::PageType eType)
{
    m_bSaveParaHadField = m_bParaHadField;
    m_aHeaderFooterStack.push(HeaderFooterContext(m_bTextInserted, m_nTableDepth));
    m_bTextInserted = false;
    m_nTableDepth = 0;

    const PropertyIds ePropIsOn = bHeader? PROP_HEADER_IS_ON: PROP_FOOTER_IS_ON;
    const PropertyIds ePropShared = bHeader? PROP_HEADER_IS_SHARED: PROP_FOOTER_IS_SHARED;
    const PropertyIds ePropTextLeft = bHeader? PROP_HEADER_TEXT_LEFT: PROP_FOOTER_TEXT_LEFT;
    const PropertyIds ePropText = bHeader? PROP_HEADER_TEXT: PROP_FOOTER_TEXT;

    m_bDiscardHeaderFooter = true;
    m_eInHeaderFooterImport
        = bHeader ? HeaderFooterImportState::header : HeaderFooterImportState::footer;

    //get the section context
    PropertyMapPtr pContext = DomainMapper_Impl::GetTopContextOfType(CONTEXT_SECTION);
    //ask for the header/footer name of the given type
    SectionPropertyMap* pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
    if(!pSectionContext)
        return;

    // clear the "Link To Previous" flag so that the header/footer
    // content is not copied from the previous section
    pSectionContext->ClearHeaderFooterLinkToPrevious(bHeader, eType);

    if (!m_bIsNewDoc)
    {
        return; // TODO sw cannot Undo insert header/footer without crashing
    }

    uno::Reference< beans::XPropertySet > xPageStyle =
        pSectionContext->GetPageStyle(
            *this,
            eType == SectionPropertyMap::PAGE_FIRST );
    if (!xPageStyle.is())
        return;
    try
    {
        bool bLeft = eType == SectionPropertyMap::PAGE_LEFT;
        bool bFirst = eType == SectionPropertyMap::PAGE_FIRST;
        if (!bLeft || GetSettingsTable()->GetEvenAndOddHeaders())
        {
            //switch on header/footer use
            xPageStyle->setPropertyValue(
                    getPropertyName(ePropIsOn),
                    uno::makeAny(true));

            // If the 'Different Even & Odd Pages' flag is turned on - do not ignore it
            // Even if the 'Even' header/footer is blank - the flag should be imported (so it would look in LO like in Word)
            if (!bFirst && GetSettingsTable()->GetEvenAndOddHeaders())
                xPageStyle->setPropertyValue(getPropertyName(ePropShared), uno::makeAny(false));

            //set the interface
            uno::Reference< text::XText > xText;
            xPageStyle->getPropertyValue(getPropertyName(bLeft? ePropTextLeft: ePropText)) >>= xText;

            m_aTextAppendStack.push(TextAppendContext(uno::Reference< text::XTextAppend >(xText, uno::UNO_QUERY_THROW),
                m_bIsNewDoc
                    ? uno::Reference<text::XTextCursor>()
                    : xText->createTextCursorByRange(xText->getStart())));
            m_aHeaderFooterTextAppendStack.push(std::make_pair(TextAppendContext(uno::Reference< text::XTextAppend >(xText, uno::UNO_QUERY_THROW),
                m_bIsNewDoc
                    ? uno::Reference<text::XTextCursor>()
                    : xText->createTextCursorByRange(xText->getStart())),
                bHeader));
        }
        // If we have *hidden* header footer
        else
        {
            bool bIsShared = false;
            // Turn on the headers
            xPageStyle->setPropertyValue(getPropertyName(ePropIsOn), uno::makeAny(true));
            // Store the state of the previous state of shared prop
            xPageStyle->getPropertyValue(getPropertyName(ePropShared)) >>= bIsShared;
            // Turn on the shared prop in order to save the headers/footers in time
            xPageStyle->setPropertyValue(getPropertyName(ePropShared), uno::makeAny(false));
            // Add the content of the headers footers to the doc
            uno::Reference<text::XText> xText;
            xPageStyle->getPropertyValue(getPropertyName(bLeft ? ePropTextLeft : ePropText))
                >>= xText;

            m_aTextAppendStack.push(
                TextAppendContext(uno::Reference<text::XTextAppend>(xText, uno::UNO_QUERY_THROW),
                                  m_bIsNewDoc ? uno::Reference<text::XTextCursor>()
                                              : xText->createTextCursorByRange(xText->getStart())));
            // Restore the original state of the shared prop after we stored the necessary values.
            xPageStyle->setPropertyValue(getPropertyName(ePropShared), uno::makeAny(bIsShared));
        }
        m_bDiscardHeaderFooter = false; // set only on success!
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("writerfilter.dmapper");
    }
}

void DomainMapper_Impl::PushPageHeader(SectionPropertyMap::PageType eType)
{
    PushPageHeaderFooter(/* bHeader = */ true, eType);
}

void DomainMapper_Impl::PushPageFooter(SectionPropertyMap::PageType eType)
{
    PushPageHeaderFooter(/* bHeader = */ false, eType);
}

void DomainMapper_Impl::PopPageHeaderFooter()
{
    //header and footer always have an empty paragraph at the end
    //this has to be removed
    RemoveLastParagraph( );

    if (!m_aTextAppendStack.empty())
    {
        if (!m_bDiscardHeaderFooter)
        {
            m_aTextAppendStack.pop();
        }
        m_bDiscardHeaderFooter = false;
    }
    m_eInHeaderFooterImport = HeaderFooterImportState::none;

    if (!m_aHeaderFooterStack.empty())
    {
        m_bTextInserted = m_aHeaderFooterStack.top().getTextInserted();
        m_nTableDepth = m_aHeaderFooterStack.top().getTableDepth();
        m_aHeaderFooterStack.pop();
    }

    m_bParaHadField = m_bSaveParaHadField;
}

void DomainMapper_Impl::PushFootOrEndnote( bool bIsFootnote )
{
    SAL_WARN_IF(m_bInFootOrEndnote, "writerfilter.dmapper", "PushFootOrEndnote() is called from another foot or endnote");
    m_bInFootOrEndnote = true;
    m_bInFootnote = bIsFootnote;
    m_bCheckFirstFootnoteTab = true;
    m_bSaveFirstParagraphInCell = m_bFirstParagraphInCell;
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
        if (GetTextFactory().is())
            xFootnoteText.set( GetTextFactory()->createInstance(
            bIsFootnote ?
                OUString( "com.sun.star.text.Footnote" ) : OUString( "com.sun.star.text.Endnote" )),
            uno::UNO_QUERY_THROW );
        uno::Reference< text::XFootnote > xFootnote( xFootnoteText, uno::UNO_QUERY_THROW );
        pTopContext->SetFootnote(xFootnote, sFootnoteCharStyleName);
        uno::Sequence< beans::PropertyValue > aFontProperties = pTopContext->GetPropertyValues();
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
            [[fallthrough]];
        case XML_ins:
            sType = getPropertyName( PROP_INSERT );
            break;
        case XML_moveFrom:
            bRedlineMoved = true;
            m_pParaMarkerRedlineMoveFrom = pRedline.get();
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
        pRedlineProperties[1].Value <<= ConversionHelper::ConvertDateStringToDateTime( pRedline->m_sDate );
        pRedlineProperties[2].Name = getPropertyName( PROP_REDLINE_REVERT_PROPERTIES );
        pRedlineProperties[2].Value <<= pRedline->m_aRevertProperties;
        pRedlineProperties[3].Name = "RedlineMoved";
        pRedlineProperties[3].Value <<= bRedlineMoved;

        if (!m_bIsActualParagraphFramed)
        {
            uno::Reference < text::XRedline > xRedline( xRange, uno::UNO_QUERY_THROW );
            xRedline->makeRedline( sType, aRedlineProperties );
        }
        // store frame and (possible floating) table redline data for restoring them after frame conversion
        enum StoredRedlines eType;
        if (m_bIsActualParagraphFramed || m_nTableDepth > 0)
            eType = StoredRedlines::FRAME;
        else if (IsInFootOrEndnote())
            eType = IsInFootnote() ? StoredRedlines::FOOTNOTE : StoredRedlines::ENDNOTE;
        else
            eType = StoredRedlines::NONE;

        if (eType != StoredRedlines::NONE)
        {
            m_aStoredRedlines[eType].push_back( uno::makeAny(xRange) );
            m_aStoredRedlines[eType].push_back( uno::makeAny(sType) );
            m_aStoredRedlines[eType].push_back( uno::makeAny(aRedlineProperties) );
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
    else if ( m_pParaMarkerRedlineMoveFrom )
    {
        // terminating moveFrom redline removes also the paragraph mark
        CreateRedline( xRange, m_pParaMarkerRedlineMoveFrom );
    }
    if ( m_pParaMarkerRedlineMoveFrom )
    {
        m_pParaMarkerRedlineMoveFrom.clear();
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
    if( (!bUsedRange || !m_bParaChanged) && GetTopContextOfType(CONTEXT_PARAGRAPH) )
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

void DomainMapper_Impl::StartCustomFootnote(const PropertyMapPtr pContext)
{
    if (pContext == m_pFootnoteContext)
        return;

    assert(pContext->GetFootnote().is());
    m_bHasFootnoteStyle = true;
    m_bCheckFootnoteStyle = !pContext->GetFootnoteStyle().isEmpty();
    m_pFootnoteContext = pContext;
}

void DomainMapper_Impl::EndCustomFootnote()
{
    m_bHasFootnoteStyle = false;
    m_bCheckFootnoteStyle = false;
}

void DomainMapper_Impl::PushAnnotation()
{
    try
    {
        m_bIsInComments = true;
        if (!GetTextFactory().is())
            return;
        m_xAnnotationField.set( GetTextFactory()->createInstance( "com.sun.star.text.TextField.Annotation" ),
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

void DomainMapper_Impl::PopFootOrEndnote()
{
    // content of the footnotes were inserted after the first footnote in temporary footnotes,
    // restore the content of the actual footnote by copying its content from the first
    // (remaining) temporary footnote and remove the temporary footnote.
    // FIXME: add footnote IDs to handle possible differences in footnote serialization
    uno::Reference< text::XFootnotesSupplier> xFootnotesSupplier( GetTextDocument(), uno::UNO_QUERY );
    uno::Reference< text::XEndnotesSupplier> xEndnotesSupplier( GetTextDocument(), uno::UNO_QUERY );
    bool bCopied = false;
    if ( IsInFootOrEndnote() && ( ( IsInFootnote() && GetFootnoteCount() > -1 && xFootnotesSupplier.is() ) ||
         ( !IsInFootnote() && GetEndnoteCount() > -1 && xEndnotesSupplier.is() ) ) )
    {
        uno::Reference< text::XFootnote > xFootnoteFirst, xFootnoteLast;
        auto xFootnotes = xFootnotesSupplier->getFootnotes();
        auto xEndnotes = xEndnotesSupplier->getEndnotes();
        if (IsInFootnote())
            xFootnotes->getByIndex(xFootnotes->getCount()-1) >>= xFootnoteLast;
        else
            xEndnotes->getByIndex(xEndnotes->getCount()-1) >>= xFootnoteLast;
        if ( ( ( IsInFootnote() && xFootnotes->getCount() > 1 ) ||
             ( !IsInFootnote() && xEndnotes->getCount() > 1 ) ) &&
                        xFootnoteLast->getLabel().isEmpty() )
        {
            // copy content of the first remaining temporary footnote
            if ( IsInFootnote() )
                xFootnotes->getByIndex(1) >>= xFootnoteFirst;
            else
                xEndnotes->getByIndex(1) >>= xFootnoteFirst;
            if (!m_bSaxError && xFootnoteFirst != xFootnoteLast)
            {
                uno::Reference< text::XText > xSrc( xFootnoteFirst, uno::UNO_QUERY_THROW );
                uno::Reference< text::XText > xDest( xFootnoteLast, uno::UNO_QUERY_THROW );
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

                // remove temporary footnote
                xFootnoteFirst->getAnchor()->setString("");
                bCopied = true;
            }
        }
    }

    if (!IsRTFImport() && !bCopied)
        RemoveLastParagraph();

    // In case the foot or endnote did not contain a tab.
    m_bIgnoreNextTab = false;

    if (!m_aTextAppendStack.empty())
        m_aTextAppendStack.pop();

    if (m_aRedlines.size() == 1)
    {
        SAL_WARN("writerfilter.dmapper", "PopFootOrEndnote() is called without PushFootOrEndnote()?");
        return;
    }
    m_aRedlines.pop();
    m_eSkipFootnoteState = SkipFootnoteSeparator::OFF;
    m_bInFootOrEndnote = false;
    m_pFootnoteContext = nullptr;
    m_bFirstParagraphInCell = m_bSaveFirstParagraphInCell;
}

void DomainMapper_Impl::PopAnnotation()
{
    RemoveLastParagraph();

    m_bIsInComments = false;
    m_aTextAppendStack.pop();

    try
    {
        if (m_bAnnotationResolved)
            m_xAnnotationField->setPropertyValue("Resolved", css::uno::Any(true));

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
                    uno::Reference<text::XTextRange> xFrame(xShapes->getByIndex(i), uno::UNO_QUERY_THROW);
                    uno::Reference<beans::XPropertySet> xSyncedPropertySet(xFrame, uno::UNO_QUERY_THROW);
                    comphelper::SequenceAsHashMap aGrabBag( xSyncedPropertySet->getPropertyValue("CharInteropGrabBag") );

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
                        const uno::Reference<beans::XPropertyState> xSyncedPropertyState(xSyncedPropertySet, uno::UNO_QUERY_THROW);
                        for ( const auto& eId : eIds )
                        {
                            try
                            {
                                if ( bOnlyApplyCharHeight && eId != PROP_CHAR_HEIGHT )
                                    continue;

                                const OUString sPropName = getPropertyName(eId);
                                if ( beans::PropertyState_DEFAULT_VALUE == xSyncedPropertyState->getPropertyState(sPropName) )
                                {
                                    const uno::Any aProp = GetPropertyFromStyleSheet(eId, pEntry, /*bDocDefaults=*/true, /*bPara=*/true);
                                    if ( aProp.hasValue() )
                                        xSyncedPropertySet->setPropertyValue( sPropName, aProp );
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

            m_xEmbedded.set(m_xTextFactory->createInstance("com.sun.star.text.TextEmbeddedObject"), uno::UNO_QUERY_THROW);
            uno::Reference<beans::XPropertySet> xEmbeddedProperties(m_xEmbedded, uno::UNO_QUERY_THROW);
            xEmbeddedProperties->setPropertyValue(getPropertyName(PROP_EMBEDDED_OBJECT), xShapePropertySet->getPropertyValue(getPropertyName(PROP_EMBEDDED_OBJECT)));
            xEmbeddedProperties->setPropertyValue(getPropertyName(PROP_ANCHOR_TYPE), uno::makeAny(text::TextContentAnchorType_AS_CHARACTER));
            // So that the original bitmap-only shape will be replaced by the embedded object.
            m_aAnchoredStack.top().bToRemove = true;
            m_aTextAppendStack.pop();
            appendTextContent(m_xEmbedded, uno::Sequence<beans::PropertyValue>());
        }
        else
        {
            uno::Reference< text::XTextRange > xShapeText( xShape, uno::UNO_QUERY_THROW);
            // Add the shape to the text append stack
            m_aTextAppendStack.push( TextAppendContext(uno::Reference< text::XTextAppend >( xShape, uno::UNO_QUERY_THROW ),
                        m_bIsNewDoc ? uno::Reference<text::XTextCursor>() : m_xBodyText->createTextCursorByRange(xShapeText->getStart() )));

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

                for (const auto& rProp : std::as_const(aGrabBag))
                {
                    if (rProp.Name == "VML-Z-ORDER")
                    {
                        GraphicZOrderHelper* pZOrderHelper = m_rDMapper.graphicZOrderHelper();
                        sal_Int32 zOrder(0);
                        rProp.Value >>= zOrder;
                        xShapePropertySet->setPropertyValue( "ZOrder", uno::makeAny(pZOrderHelper->findZOrder(zOrder)));
                        pZOrderHelper->addItem(xShapePropertySet, zOrder);
                        xShapePropertySet->setPropertyValue(getPropertyName( PROP_OPAQUE ), uno::makeAny( zOrder >= 0 ) );
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
                xPropertySet->setPropertyValue("FrameInteropGrabBag",uno::makeAny(aGrabBag));
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
                for (const auto& rProp : std::as_const(aGrabBag))
                {
                    if (rProp.Name == "VML-Z-ORDER")
                    {
                        GraphicZOrderHelper* pZOrderHelper = m_rDMapper.graphicZOrderHelper();
                        sal_Int32 zOrder(0);
                        rProp.Value >>= zOrder;
                        xShapePropertySet->setPropertyValue( "ZOrder", uno::makeAny(pZOrderHelper->findZOrder(zOrder)));
                        pZOrderHelper->addItem(xShapePropertySet, zOrder);
                        xShapePropertySet->setPropertyValue(getPropertyName( PROP_OPAQUE ), uno::makeAny( zOrder >= 0 ) );
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
                            xShapePropertySet->setPropertyValue("InteropGrabBag",uno::makeAny(aShapeGrabBag));
                        }
                    }
                }
            }
            if (!IsInHeaderFooter() && !checkZOrderStatus)
                xProps->setPropertyValue(
                        getPropertyName( PROP_OPAQUE ),
                        uno::makeAny( true ) );
        }
        m_bParaChanged = true;
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

    uno::Reference<beans::XPropertySet> xEmbeddedProperties(m_xEmbedded, uno::UNO_QUERY_THROW);
    awt::Size aSize = xShape->getSize( );
    xEmbeddedProperties->setPropertyValue(getPropertyName(PROP_WIDTH), uno::makeAny(sal_Int32(aSize.Width)));
    xEmbeddedProperties->setPropertyValue(getPropertyName(PROP_HEIGHT), uno::makeAny(sal_Int32(aSize.Height)));
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
    if ( m_aAnchoredStack.top().bToRemove || m_bDiscardHeaderFooter )
    {
        try
        {
            uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(m_xTextDocument, uno::UNO_QUERY_THROW);
            uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
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
                for (const auto& rProp : std::as_const(aCharGrabBag))
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
    if ((m_nTableDepth == (m_nTableCellDepth + 1))
        && m_xPreviousParagraph.is()
        && hasTableManager() && getTableManager().isCellLastParaAfterAutospacing())
    {
        uno::Reference<container::XNamed> xPreviousNumberingRules(m_xPreviousParagraph->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
        if ( !xPreviousNumberingRules.is() || xPreviousNumberingRules->getName().isEmpty() )
            m_xPreviousParagraph->setPropertyValue("ParaBottomMargin", uno::makeAny(static_cast<sal_Int32>(0)));
    }

    m_xPreviousParagraph.clear();

    // next table paragraph will be first paragraph in a cell
    m_bFirstParagraphInCell = true;
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
        xImporter->setTargetDocument(m_xTextDocument);

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

static sal_Int16 lcl_ParseNumberingType( const OUString& rCommand )
{
    sal_Int16 nRet = style::NumberingType::PAGE_DESCRIPTOR;

    //  The command looks like: " PAGE \* Arabic "
    // tdf#132185: but may as well be "PAGE \* Arabic"
    OUString sNumber;
    constexpr OUStringLiteral rSeparator(u"\\* ");
    if (sal_Int32 nStartIndex = rCommand.indexOf(rSeparator); nStartIndex >= 0)
    {
        nStartIndex += rSeparator.getLength();
        sNumber = rCommand.getToken(0, ' ', nStartIndex);
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
    CHARS_CYRILLIC_LOWER_LETTER_N_SR*/

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
    //  Remove whitespace permitted by standard between \@ and "
    OUString command;
    sal_Int32 delimPos = rCommand.indexOf("\\@");
    if (delimPos != -1)
    {
        sal_Int32 wsChars = rCommand.indexOf('\"') - delimPos - 2;
        command = rCommand.replaceAt(delimPos+2, wsChars, u"");
    }
    else
        command = rCommand;

    return msfilter::util::findQuotedText(command, "\\@\"", '\"');
}
/*-------------------------------------------------------------------------
extract a parameter (with or without quotes) between the command and the following backslash
  -----------------------------------------------------------------------*/
static OUString lcl_ExtractToken(OUString const& rCommand,
        sal_Int32 & rIndex, bool & rHaveToken, bool & rIsSwitch)
{
    rHaveToken = false;
    rIsSwitch = false;

    OUStringBuffer token;
    bool bQuoted(false);
    for (; rIndex < rCommand.getLength(); ++rIndex)
    {
        sal_Unicode const currentChar(rCommand[rIndex]);
        switch (currentChar)
        {
            case '\\':
            {
                if (rIndex == rCommand.getLength() - 1)
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
                        return rCommand.copy(rIndex - 2, 2).toAsciiUpperCase();
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
    assert(rIndex == rCommand.getLength());
    if (bQuoted)
    {
        // MS Word allows this, so just emit a debug message
        SAL_INFO("writerfilter.dmapper",
                    "field argument with unterminated quote");
    }
    rHaveToken = !token.isEmpty();
    return token.makeStringAndClear();
}

std::tuple<OUString, std::vector<OUString>, std::vector<OUString> > splitFieldCommand(const OUString& rCommand)
{
    OUString sType;
    std::vector<OUString> arguments;
    std::vector<OUString> switches;
    sal_Int32 nStartIndex(0);
    // tdf#54584: Field may be prepended by a backslash
    // This is not an escapement, but already escaped literal "\"
    // MS Word allows this, so just skip it
    if ((rCommand.getLength() >= nStartIndex + 2) &&
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
        assert(nStartIndex <= rCommand.getLength());
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
    } while (nStartIndex < rCommand.getLength());

    return std::make_tuple(sType, arguments, switches);
}

static OUString lcl_ExtractVariableAndHint( const OUString& rCommand, OUString& rHint )
{
    // the first word after "ASK " is the variable
    // the text after the variable and before a '\' is the hint
    // if no hint is set the variable is used as hint
    // the quotes of the hint have to be removed
    sal_Int32 nIndex = rCommand.indexOf( ' ', 2); //find last space after 'ASK'
    if (nIndex == -1)
        return OUString();
    while(rCommand[nIndex] == ' ')
        ++nIndex;
    OUString sShortCommand( rCommand.copy( nIndex ) ); //cut off the " ASK "

    sShortCommand = sShortCommand.getToken(0, '\\');
    nIndex = 0;
    OUString sRet = sShortCommand.getToken( 0, ' ', nIndex);
    if( nIndex > 0)
        rHint = sShortCommand.copy( nIndex );
    if( rHint.isEmpty() )
        rHint = sRet;
    return sRet;
}


static bool lcl_FindInCommand(
    const OUString& rCommand,
    sal_Unicode cSwitch,
    OUString& rValue )
{
    bool bRet = false;
    OUString sSearch = "\\" + OUStringChar( cSwitch );
    sal_Int32 nIndex = rCommand.indexOf( sSearch  );
    if( nIndex >= 0 )
    {
        bRet = true;
        //find next '\' or end of string
        sal_Int32 nEndIndex = rCommand.indexOf( '\\', nIndex + 1);
        if( nEndIndex < 0 )
            nEndIndex = rCommand.getLength() ;
        if( nEndIndex - nIndex > 3 )
            rValue = rCommand.copy( nIndex + 3, nEndIndex - nIndex - 3);
    }
    return bRet;
}

static OUString lcl_trim(const OUString& sValue)
{
    // it seems, all kind of quotation marks are allowed around index type identifiers
    // TODO apply this on bookmarks, too, if needed
    return sValue.trim().replaceAll("\"","").replaceAll(u"“", "").replaceAll(u"”", "");
}

void DomainMapper_Impl::GetCurrentLocale(lang::Locale& rLocale)
{
    PropertyMapPtr pTopContext = GetTopContext();
    std::optional<PropertyMap::Property> pLocale = pTopContext->getProperty(PROP_CHAR_LOCALE);
    if( pLocale )
        pLocale->second >>= rLocale;
    else
    {
        PropertyMapPtr pParaContext = GetTopContextOfType(CONTEXT_PARAGRAPH);
        pLocale = pParaContext->getProperty(PROP_CHAR_LOCALE);
        if( pLocale )
        {
            pLocale->second >>= rLocale;
        }
    }
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

    //determine current locale - todo: is it necessary to initialize this locale?
    lang::Locale aCurrentLocale = aUSLocale;
    GetCurrentLocale( aCurrentLocale );
    OUString sFormat = ConversionHelper::ConvertMSFormatStringToSO( sFormatString, aCurrentLocale, bHijri);
    //get the number formatter and convert the string to a format value
    try
    {
        sal_Int32 nKey = 0;
        uno::Reference< util::XNumberFormatsSupplier > xNumberSupplier( m_xTextDocument, uno::UNO_QUERY_THROW );
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
            uno::makeAny( nKey ));
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
        bool bShapeNameSet;
        TextFramesForChaining(): nId(0), nSeq(0), bShapeNameSet(false) {}
    } ;
    typedef std::map <OUString, TextFramesForChaining> ChainMap;

    try
    {
        ChainMap aTextFramesForChainingHelper;
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
            if( sShapeName.isEmpty() )
                aChainStruct.bShapeNameSet = false;
            else
            {
                aChainStruct.bShapeNameSet = true;
                sLinkChainName = sShapeName;
            }

            if( !sLinkChainName.isEmpty() )
            {
                aChainStruct.xShape = rTextFrame;
                aTextFramesForChainingHelper[sLinkChainName] = aChainStruct;
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
                    if( !msoItem.second.bShapeNameSet )
                    {
                        uno::Reference< container::XNamed > xNamed( msoItem.second.xShape, uno::UNO_QUERY );
                        if ( xNamed.is() )
                        {
                            xNamed->setName( msoItem.first );
                            msoItem.second.bShapeNameSet = true;
                        }
                    }
                    if( !nextFinder->second.bShapeNameSet )
                    {
                        uno::Reference< container::XNamed > xNamed( nextFinder->second.xShape, uno::UNO_QUERY );
                        if ( xNamed.is() )
                        {
                            xNamed->setName( nextFinder->first );
                            nextFinder->second.bShapeNameSet = true;
                        }
                    }

                    uno::Reference<text::XTextContent>  xTextContent(msoItem.second.xShape, uno::UNO_QUERY_THROW);
                    uno::Reference<beans::XPropertySet> xPropertySet(xTextContent, uno::UNO_QUERY);

                    //The reverse chaining happens automatically, so only one direction needs to be set
                    xPropertySet->setPropertyValue(sChainNextName, uno::makeAny(nextFinder->first));

                    //the last item in an mso-next-textbox chain is indistinguishable from id/seq items.  Now that it is handled, remove it.
                    if( nextFinder->second.s_mso_next_textbox.isEmpty() )
                        aTextFramesForChainingHelper.erase(nextFinder->first);
                }
            }
        }

        //TODO: Perhaps allow reverse sequences when mso-layout-flow-alt = "bottom-to-top"
        const sal_Int32 nDirection = 1;

        //Finally - go through and attach the chains based on matching ID and incremented sequence number (dml-style).
        for (const auto& rOuter : aTextFramesForChainingHelper)
        {
            if( rOuter.second.s_mso_next_textbox.isEmpty() )  //non-empty ones already handled earlier - so skipping them now.
            {
                for (const auto& rInner : aTextFramesForChainingHelper)
                {
                    if ( rInner.second.nId == rOuter.second.nId )
                    {
                        if ( rInner.second.nSeq == ( rOuter.second.nSeq + nDirection ) )
                        {
                            uno::Reference<text::XTextContent>  xTextContent(rOuter.second.xShape, uno::UNO_QUERY_THROW);
                            uno::Reference<beans::XPropertySet> xPropertySet(xTextContent, uno::UNO_QUERY);

                            //The reverse chaining happens automatically, so only one direction needs to be set
                            xPropertySet->setPropertyValue(sChainNextName, uno::makeAny(rInner.first));
                            break ; //there cannot be more than one next frame
                        }
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
    if (m_bIsInTextBox)
        return;

    try
    {
        uno::Reference<text::XTextFrame> xTBoxFrame(
            m_xTextFactory->createInstance("com.sun.star.text.TextFrame"), uno::UNO_QUERY_THROW);
        uno::Reference<container::XNamed>(xTBoxFrame, uno::UNO_QUERY_THROW)
            ->setName("textbox" + OUString::number(m_xPendigTextBoxFrames.size() + 1));
        uno::Reference<text::XTextAppendAndConvert>(m_aTextAppendStack.top().xTextAppend,
            uno::UNO_QUERY_THROW)
            ->appendTextContent(xTBoxFrame, beans::PropertyValues());
        m_xPendigTextBoxFrames.push(xTBoxFrame);

        m_aTextAppendStack.push(TextAppendContext(uno::Reference<text::XTextAppend>(xTBoxFrame, uno::UNO_QUERY_THROW), {}));
        m_bIsInTextBox = true;

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
    if (!m_bIsInTextBox || m_xPendigTextBoxFrames.empty())
        return;

    if (uno::Reference<text::XTextFrame>(m_aTextAppendStack.top().xTextAppend, uno::UNO_QUERY).is())
    {
        if (hasTableManager())
        {
            getTableManager().endLevel();
            popTableManager();
        }
        m_aTextAppendStack.pop();
        m_bIsInTextBox = false;
    }
}

void DomainMapper_Impl::AttachTextBoxContentToShape(css::uno::Reference<css::drawing::XShape> xShape)
{
    if (m_xPendigTextBoxFrames.empty() || !xShape)
        return;

    uno::Reference< drawing::XShapes >xGroup(xShape, uno::UNO_QUERY);
    uno::Reference< beans::XPropertySet >xProps(xShape, uno::UNO_QUERY);

    if (xGroup)
        for (sal_Int32 i = 0; i < xGroup->getCount(); ++i)
            AttachTextBoxContentToShape(uno::Reference<drawing::XShape>(xGroup->getByIndex(i),uno::UNO_QUERY_THROW));

    if (xProps->getPropertyValue("TextBox").get<bool>())
    {
        xProps->setPropertyValue("TextBoxContent", uno::Any(m_xPendigTextBoxFrames.front()));
        m_xPendigTextBoxFrames.pop();
    }
}

uno::Reference<beans::XPropertySet> DomainMapper_Impl::FindOrCreateFieldMaster(const char* pFieldMasterService, const OUString& rFieldMasterName)
{
    // query master, create if not available
    uno::Reference< text::XTextFieldsSupplier > xFieldsSupplier( GetTextDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xFieldMasterAccess = xFieldsSupplier->getTextFieldMasters();
    uno::Reference< beans::XPropertySet > xMaster;
    OUString sFieldMasterService( OUString::createFromAscii(pFieldMasterService) );
    OUStringBuffer aFieldMasterName;
    OUString sDatabaseDataSourceName = GetSettingsTable()->GetCurrentDatabaseDataSource();
    bool bIsMergeField = sFieldMasterService.endsWith("Database");
    aFieldMasterName.appendAscii( pFieldMasterService );
    aFieldMasterName.append('.');
    if ( bIsMergeField && !sDatabaseDataSourceName.isEmpty() )
    {
        aFieldMasterName.append(sDatabaseDataSourceName);
        aFieldMasterName.append('.');
    }
    aFieldMasterName.append(rFieldMasterName);
    OUString sFieldMasterName = aFieldMasterName.makeStringAndClear();
    if(xFieldMasterAccess->hasByName(sFieldMasterName))
    {
        //get the master
        xMaster.set(xFieldMasterAccess->getByName(sFieldMasterName), uno::UNO_QUERY_THROW);
    }
    else if( m_xTextFactory.is() )
    {
        //create the master
        xMaster.set( m_xTextFactory->createInstance(sFieldMasterService), uno::UNO_QUERY_THROW);
        if ( !bIsMergeField || sDatabaseDataSourceName.isEmpty() )
        {
            //set the master's name
            xMaster->setPropertyValue(
                    getPropertyName(PROP_NAME),
                    uno::makeAny(rFieldMasterName));
        } else {
           // set database data, based on the "databasename.tablename" of sDatabaseDataSourceName
           xMaster->setPropertyValue(
                    getPropertyName(PROP_DATABASE_NAME),
                    uno::makeAny(sDatabaseDataSourceName.copy(0, sDatabaseDataSourceName.indexOf('.'))));
           xMaster->setPropertyValue(
                    getPropertyName(PROP_COMMAND_TYPE),
                    uno::makeAny(sal_Int32(0)));
           xMaster->setPropertyValue(
                    getPropertyName(PROP_DATATABLE_NAME),
                    uno::makeAny(sDatabaseDataSourceName.copy(sDatabaseDataSourceName.indexOf('.') + 1)));
           xMaster->setPropertyValue(
                    getPropertyName(PROP_DATACOLUMN_NAME),
                    uno::makeAny(rFieldMasterName));
        }
    }
    return xMaster;
}

void DomainMapper_Impl::PushFieldContext()
{
    m_bParaHadField = true;
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

HeaderFooterContext::HeaderFooterContext(bool bTextInserted, sal_Int32 nTableDepth)
    : m_bTextInserted(bTextInserted)
    , m_nTableDepth(nTableDepth)
{
}

bool HeaderFooterContext::getTextInserted() const
{
    return m_bTextInserted;
}

sal_Int32 HeaderFooterContext::getTableDepth() const { return m_nTableDepth; }

FieldContext::FieldContext(uno::Reference< text::XTextRange > const& xStart)
    : m_bFieldCommandCompleted(false)
    , m_xStartRange( xStart )
    , m_bFieldLocked( false )
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

void FieldContext::AppendCommand(std::u16string_view rPart)
{
    m_sCommand += rPart;
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
        {"REF",             {"GetReference",            FIELD_REF           }},
        {"REVNUM",          {"DocInfo.Revision",        FIELD_REVNUM        }},
        {"SAVEDATE",        {"DocInfo.Change",          FIELD_SAVEDATE      }},
//      {"SECTION",         {"",                        FIELD_SECTION       }},
//      {"SECTIONPAGES",    {"",                        FIELD_SECTIONPAGES  }},
        {"SEQ",             {"SetExpression",           FIELD_SEQ           }},
        {"SET",             {"SetExpression",           FIELD_SET           }},
//      {"SKIPIF",          {"",                        FIELD_SKIPIF        }},
//      {"STYLEREF",        {"",                        FIELD_STYLEREF      }},
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
        OUString sTmp = sHint.trim();
        if (sTmp.startsWith("\"") && sTmp.endsWith("\""))
        {
            sHint = sTmp.copy(1, sTmp.getLength() - 2);
        }
    }

    // determine field master name
    uno::Reference< beans::XPropertySet > xMaster =
        FindOrCreateFieldMaster
        ("com.sun.star.text.FieldMaster.SetExpression", sVariable );

    // a set field is a string
    xMaster->setPropertyValue(getPropertyName(PROP_SUB_TYPE), uno::makeAny(text::SetVariableType::STRING));

    // attach the master to the field
    uno::Reference< text::XDependentTextField > xDependentField
        ( xFieldInterface, uno::UNO_QUERY_THROW );
    xDependentField->attachTextFieldMaster( xMaster );

    xFieldProperties->setPropertyValue(getPropertyName(PROP_HINT), uno::makeAny( sHint ));
    xFieldProperties->setPropertyValue(getPropertyName(PROP_CONTENT), uno::makeAny( sHint ));
    xFieldProperties->setPropertyValue(getPropertyName(PROP_SUB_TYPE), uno::makeAny(text::SetVariableType::STRING));

    // Mimic MS Word behavior (hide the SET)
    xFieldProperties->setPropertyValue(getPropertyName(PROP_IS_VISIBLE), uno::makeAny(false));
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
        xMaster->setPropertyValue(getPropertyName(PROP_SUB_TYPE), uno::makeAny(text::SetVariableType::STRING));

        // attach the master to the field
        uno::Reference< text::XDependentTextField > xDependentField
            ( xFieldInterface, uno::UNO_QUERY_THROW );
        xDependentField->attachTextFieldMaster( xMaster );

        // set input flag at the field
        xFieldProperties->setPropertyValue(
            getPropertyName(PROP_IS_INPUT), uno::makeAny( true ));
        // set the prompt
        xFieldProperties->setPropertyValue(
            getPropertyName(PROP_HINT),
            uno::makeAny( sHint ));
        xFieldProperties->setPropertyValue(getPropertyName(PROP_SUB_TYPE), uno::makeAny(text::SetVariableType::STRING));
        // The ASK has no field value to display
        xFieldProperties->setPropertyValue(getPropertyName(PROP_IS_VISIBLE), uno::makeAny(false));
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

    xFieldProperties->setPropertyValue(getPropertyName(PROP_CONTENT), uno::makeAny(formula));
    xFieldProperties->setPropertyValue(getPropertyName(PROP_NUMBER_FORMAT), uno::makeAny(sal_Int32(0)));
    xFieldProperties->setPropertyValue("IsShowFormula", uno::makeAny(false));

    // grab-bag the original and converted formula
    if (hasTableManager())
    {
        TablePropertyMapPtr pPropMap(new TablePropertyMap());
        pPropMap->Insert(PROP_CELL_FORMULA, uno::makeAny(command.copy(1)), true, CELL_GRAB_BAG);
        pPropMap->Insert(PROP_CELL_FORMULA_CONVERTED, uno::makeAny(formula), true, CELL_GRAB_BAG);
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
        sal_uInt32  nJc = rCommand.getToken(0, ' ',nIndex).toInt32();
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
        aInfo.nHps = rCommand.getToken(0, ' ',nIndex).toInt32();
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

    OUString sRubyParts = rCommand.copy(nIndex+1,nEnd-nIndex-1);
    nIndex = 0;
    OUString sPart1 = sRubyParts.getToken(0, ',', nIndex);
    OUString sPart2 = sRubyParts.getToken(0, ',', nIndex);
    if ((nIndex = sPart1.indexOf('(')) != -1 && (nEnd = sPart1.lastIndexOf(')'))!=-1  && nEnd > nIndex)
    {
        aInfo.sRubyText = sPart1.copy(nIndex+1,nEnd-nIndex-1);
    }

    PropertyMapPtr pRubyContext(new PropertyMap());
    pRubyContext->InsertProps(GetTopContext());
    if (aInfo.nHps > 0)
    {
        double fVal = double(aInfo.nHps) / 2.;
        uno::Any aVal = uno::makeAny( fVal );

        pRubyContext->Insert(PROP_CHAR_HEIGHT, aVal);
        pRubyContext->Insert(PROP_CHAR_HEIGHT_ASIAN, aVal);
    }
    PropertyValueVector_t aProps = comphelper::sequenceToContainer< PropertyValueVector_t >(pRubyContext->GetPropertyValues());
    aInfo.sRubyStyle = m_rDMapper.getOrCreateCharStyle(aProps, /*bAlwaysCreate=*/false);
    PropertyMapPtr pCharContext(new PropertyMap());
    if (m_pLastCharacterContext)
        pCharContext->InsertProps(m_pLastCharacterContext);
    pCharContext->InsertProps(pContext->getProperties());
    pCharContext->Insert(PROP_RUBY_TEXT, uno::makeAny( aInfo.sRubyText ) );
    pCharContext->Insert(PROP_RUBY_ADJUST, uno::makeAny(static_cast<sal_Int16>(ConversionHelper::convertRubyAlign(aInfo.nRubyAlign))));
    if ( aInfo.nRubyAlign == NS_ooxml::LN_Value_ST_RubyAlign_rightVertical )
        pCharContext->Insert(PROP_RUBY_POSITION, uno::makeAny(css::text::RubyPosition::INTER_CHARACTER));
    pCharContext->Insert(PROP_RUBY_STYLE, uno::makeAny(aInfo.sRubyStyle));
    appendTextPortion(sPart2, pCharContext);

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
        uno::makeAny(text::SetVariableType::SEQUENCE));

    //apply the numbering type
    xFieldProperties->setPropertyValue(
        getPropertyName(PROP_NUMBERING_TYPE),
        uno::makeAny( lcl_ParseNumberingType(pContext->GetCommand()) ));
        // attach the master to the field
    uno::Reference< text::XDependentTextField > xDependentField
        ( xFieldInterface, uno::UNO_QUERY_THROW );
    xDependentField->attachTextFieldMaster( xMaster );
}

void DomainMapper_Impl::handleAuthor
    (std::u16string_view rFirstParam,
     uno::Reference< beans::XPropertySet > const& xFieldProperties,
     FieldId  eFieldId )
{
    if ( eFieldId != FIELD_USERINITIALS )
        xFieldProperties->setPropertyValue
            ( getPropertyName(PROP_FULL_NAME), uno::makeAny( true ));

    if (!rFirstParam.empty())
    {
        xFieldProperties->setPropertyValue(
                getPropertyName( PROP_IS_FIXED ),
                uno::makeAny( true ));
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
    uno::Reference<document::XDocumentPropertiesSupplier> xDocumentPropertiesSupplier(m_xTextDocument, uno::UNO_QUERY);
    uno::Reference<document::XDocumentProperties> xDocumentProperties = xDocumentPropertiesSupplier->getDocumentProperties();
    uno::Reference<beans::XPropertySet>  xUserDefinedProps(xDocumentProperties->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySetInfo> xPropertySetInfo =  xUserDefinedProps->getPropertySetInfo();
    //search for a field mapping
    OUString sFieldServiceName;
    size_t nMap = 0;
    for( ; nMap < SAL_N_ELEMENTS(aDocProperties); ++nMap )
    {
        if ((rFirstParam.equalsAscii(aDocProperties[nMap].pDocPropertyName)) && (!xPropertySetInfo->hasPropertyByName(rFirstParam)))
        {
            sFieldServiceName =
            OUString::createFromAscii
            (aDocProperties[nMap].pServiceName);
            break;
        }
    }
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
    if (m_xTextFactory.is())
        xFieldInterface = m_xTextFactory->createInstance(sServiceName);
    uno::Reference<beans::XPropertySet> xFieldProperties( xFieldInterface, uno::UNO_QUERY_THROW);
    if( bIsCustomField )
    {
        xFieldProperties->setPropertyValue(
            getPropertyName(PROP_NAME), uno::makeAny(rFirstParam));
        pContext->SetCustomField( xFieldProperties );
    }
    else
    {
        if(0 != (aDocProperties[nMap].nFlags & SET_ARABIC))
            xFieldProperties->setPropertyValue(
                getPropertyName(PROP_NUMBERING_TYPE),
                uno::makeAny( style::NumberingType::ARABIC ));
        else if(0 != (aDocProperties[nMap].nFlags & SET_DATE))
        {
            xFieldProperties->setPropertyValue(
                getPropertyName(PROP_IS_DATE),
                    uno::makeAny( true ));
            SetNumberFormat( pContext->GetCommand(), xFieldProperties );
        }
    }
}

static uno::Sequence< beans::PropertyValues > lcl_createTOXLevelHyperlinks( bool bHyperlinks, const OUString& sChapterNoSeparator,
                                   const uno::Sequence< beans::PropertyValues >& aLevel )
{
    //create a copy of the level and add two new entries - hyperlink start and end
    bool bChapterNoSeparator  = !sChapterNoSeparator.isEmpty();
    sal_Int32 nAdd = (bHyperlinks && bChapterNoSeparator) ? 4 : 2;
    uno::Sequence< beans::PropertyValues > aNewLevel( aLevel.getLength() + nAdd);
    beans::PropertyValues* pNewLevel = aNewLevel.getArray();
    if( bHyperlinks )
    {
        beans::PropertyValues aHyperlink{ comphelper::makePropertyValue(
            getPropertyName( PROP_TOKEN_TYPE ), getPropertyName( PROP_TOKEN_HYPERLINK_START )) };
        pNewLevel[0] = aHyperlink;
        aHyperlink = { comphelper::makePropertyValue(
            getPropertyName(PROP_TOKEN_TYPE), getPropertyName( PROP_TOKEN_HYPERLINK_END )) };
        pNewLevel[aNewLevel.getLength() -1] = aHyperlink;
    }
    if( bChapterNoSeparator )
    {
        beans::PropertyValues aChapterNo{
            comphelper::makePropertyValue(getPropertyName( PROP_TOKEN_TYPE ),
                                          getPropertyName( PROP_TOKEN_CHAPTER_INFO )),
            comphelper::makePropertyValue(getPropertyName( PROP_CHAPTER_FORMAT ),
                                          //todo: is ChapterFormat::Number correct?
                                          sal_Int16(text::ChapterFormat::NUMBER))
        };
        pNewLevel[aNewLevel.getLength() - (bHyperlinks ? 4 : 2) ] = aChapterNo;

        beans::PropertyValues aChapterSeparator{
            comphelper::makePropertyValue(getPropertyName( PROP_TOKEN_TYPE ),
                                          getPropertyName( PROP_TOKEN_TEXT )),
            comphelper::makePropertyValue(getPropertyName( PROP_TEXT ), sChapterNoSeparator)
        };
        pNewLevel[aNewLevel.getLength() - (bHyperlinks ? 3 : 1)] = aChapterSeparator;
    }
    //copy the 'old' entries except the last (page no)
    std::copy(aLevel.begin(), std::prev(aLevel.end()), std::next(pNewLevel));
    //copy page no entry (last or last but one depending on bHyperlinks
    sal_Int32 nPageNo = aNewLevel.getLength() - (bHyperlinks ? 2 : 3);
    pNewLevel[nPageNo] = aLevel[aLevel.getLength() - 1];

    return aNewLevel;
}

/// Returns title of the TOC placed in paragraph(s) before TOC field inside STD-frame
OUString DomainMapper_Impl::extractTocTitle()
{
    if (!m_xSdtEntryStart.is())
        return OUString();

    uno::Reference< text::XTextAppend > xTextAppend = m_aTextAppendStack.top().xTextAppend;
    if(!xTextAppend.is())
        return OUString();

    // try-catch was added in the same way as inside appendTextSectionAfter()
    try
    {
        uno::Reference<text::XParagraphCursor> xCursor(xTextAppend->createTextCursorByRange(m_xSdtEntryStart), uno::UNO_QUERY_THROW);
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
    if (m_bParaChanged)
    {
        finishParagraph(GetTopContextOfType(CONTEXT_PARAGRAPH), false); // resets m_bParaChanged
        PopProperties(CONTEXT_PARAGRAPH);
        PushProperties(CONTEXT_PARAGRAPH);
        SetIsFirstRun(true);
        // The first paragraph of the index that is continuation of just finished one needs to be
        // removed when finished (unless more content will arrive, which will set m_bParaChanged)
        m_bRemoveThisParagraph = true;
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
    uno::Reference< beans::XPropertySet > xTOC;
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
        sChapterNoSeparator = sValue;
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
            sValue.getToken( 0, '-', nIndex );
            nMaxLevel = static_cast<sal_Int16>(nIndex != -1 ? sValue.copy(nIndex).toInt32() : 0);
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

    if (m_xTextFactory.is() && ! m_aTextAppendStack.empty())
    {
        const auto& xTextAppend = GetTopTextAppend();
        if (aTocTitle.isEmpty() || bTableOfFigures)
        {
            // reset marker of the TOC title
            m_xSdtEntryStart.clear();

            // Create section before setting m_bStartTOC: finishing paragraph
            // inside StartIndexSectionChecked could do the wrong thing otherwise
            xTOC = StartIndexSectionChecked(bTableOfFigures ? "com.sun.star.text.IllustrationsIndex"
                                                            : sTOCServiceName);

            const auto xTextCursor = xTextAppend->getText()->createTextCursor();
            if (xTextCursor)
                xTextCursor->gotoEnd(false);
            xTOCMarkerCursor = xTextCursor;
        }
        else
        {
            // create TOC section
            css::uno::Reference<css::text::XTextRange> xTextRangeEndOfTocHeader = GetTopTextAppend()->getEnd();
            xTOC = createSectionForRange(m_xSdtEntryStart, xTextRangeEndOfTocHeader, sTOCServiceName, false);

            // init [xTOCMarkerCursor]
            uno::Reference< text::XText > xText = xTextAppend->getText();
            uno::Reference< text::XTextCursor > xCrsr = xText->createTextCursor();
            xTOCMarkerCursor = xCrsr;

            // create header of the TOC with the TOC title inside
            createSectionForRange(m_xSdtEntryStart, xTextRangeEndOfTocHeader, "com.sun.star.text.IndexHeaderSection", true);
        }
    }

    m_bStartTOC = true;

    if (xTOC.is())
        xTOC->setPropertyValue(getPropertyName( PROP_TITLE ), uno::makeAny(aTocTitle));

    if (!aBookmarkName.isEmpty())
        xTOC->setPropertyValue(getPropertyName(PROP_TOC_BOOKMARK), uno::makeAny(aBookmarkName));
    if( !bTableOfFigures && xTOC.is() )
    {
        xTOC->setPropertyValue( getPropertyName( PROP_LEVEL ), uno::makeAny( nMaxLevel ) );
        xTOC->setPropertyValue( getPropertyName( PROP_CREATE_FROM_OUTLINE ), uno::makeAny( bFromOutline ));
        xTOC->setPropertyValue( getPropertyName( PROP_CREATE_FROM_MARKS ), uno::makeAny( bFromEntries ));
        xTOC->setPropertyValue( getPropertyName( PROP_HIDE_TAB_LEADER_AND_PAGE_NUMBERS ), uno::makeAny( bHideTabLeaderPageNumbers ));
        xTOC->setPropertyValue( getPropertyName( PROP_TAB_IN_TOC ), uno::makeAny( bIsTabEntry ));
        xTOC->setPropertyValue( getPropertyName( PROP_TOC_NEW_LINE ), uno::makeAny( bNewLine ));
        xTOC->setPropertyValue( getPropertyName( PROP_TOC_PARAGRAPH_OUTLINE_LEVEL ), uno::makeAny( bParagraphOutlineLevel ));
        if( !sTemplate.isEmpty() )
        {
                            //the string contains comma separated the names and related levels
                            //like: "Heading 1,1,Heading 2,2"
            TOCStyleMap aMap;
            sal_Int32 nLevel;
            sal_Int32 nPosition = 0;
            while( nPosition >= 0)
            {
                OUString sStyleName = sTemplate.getToken( 0, ',', nPosition );
                                //empty tokens should be skipped
                while( sStyleName.isEmpty() && nPosition > 0 )
                    sStyleName = sTemplate.getToken( 0, ',', nPosition );
                nLevel = sTemplate.getToken( 0, ',', nPosition ).toInt32();
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
                        rStyle = (aTOCStyleIter++)->second;
                    }
                    xParaStyles->replaceByIndex(nLevel - 1, uno::makeAny(aStyles));
                }
            }
            xTOC->setPropertyValue(getPropertyName(PROP_CREATE_FROM_LEVEL_PARAGRAPH_STYLES), uno::makeAny( true ));

        }
        if(bHyperlinks  || !sChapterNoSeparator.isEmpty())
        {
            uno::Reference< container::XIndexReplace> xLevelFormats;
            xTOC->getPropertyValue(getPropertyName(PROP_LEVEL_FORMAT)) >>= xLevelFormats;
            sal_Int32 nLevelCount = xLevelFormats->getCount();
                            //start with level 1, 0 is the header level
            for( sal_Int32 nLevel = 1; nLevel < nLevelCount; ++nLevel)
            {
                uno::Sequence< beans::PropertyValues > aLevel;
                xLevelFormats->getByIndex( nLevel ) >>= aLevel;

                uno::Sequence< beans::PropertyValues > aNewLevel = lcl_createTOXLevelHyperlinks(
                                                    bHyperlinks, sChapterNoSeparator,
                                                    aLevel );
                xLevelFormats->replaceByIndex( nLevel, uno::makeAny( aNewLevel ) );
            }
        }
    }
    else if (bTableOfFigures && xTOC.is())
    {
        if (!sFigureSequence.isEmpty())
            xTOC->setPropertyValue(getPropertyName(PROP_LABEL_CATEGORY),
                                   uno::makeAny(sFigureSequence));

        if ( bHyperlinks )
        {
            uno::Reference< container::XIndexReplace> xLevelFormats;
            xTOC->getPropertyValue(getPropertyName(PROP_LEVEL_FORMAT)) >>= xLevelFormats;
            uno::Sequence< beans::PropertyValues > aLevel;
            xLevelFormats->getByIndex( 1 ) >>= aLevel;

            uno::Sequence< beans::PropertyValues > aNewLevel = lcl_createTOXLevelHyperlinks(
                                                bHyperlinks, sChapterNoSeparator,
                                                aLevel );
            xLevelFormats->replaceByIndex( 1, uno::makeAny( aNewLevel ) );
        }
    }
    pContext->SetTOC( xTOC );
    m_bParaHadField = false;
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
            uno::Reference< text::XTextContent > xSection( m_xTextFactory->createInstance(sObjectType), uno::UNO_QUERY_THROW );
            xSection->attach( uno::Reference< text::XTextRange >( xCursor, uno::UNO_QUERY_THROW) );
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
        xTOC->setPropertyValue(getPropertyName( PROP_TITLE ), uno::makeAny(OUString()));

    pContext->SetTOC( xTOC );
    m_bParaHadField = false;

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
        xTOC->setPropertyValue(getPropertyName( PROP_TITLE ), uno::makeAny(OUString()));

        if( lcl_FindInCommand( pContext->GetCommand(), 'r', sValue ))
        {
            xTOC->setPropertyValue("IsCommaSeparated", uno::makeAny(true));
        }
        if( lcl_FindInCommand( pContext->GetCommand(), 'h', sValue ))
        {
            xTOC->setPropertyValue("UseAlphabeticalSeparators", uno::makeAny(true));
        }
        if( !sUserIndex.isEmpty() )
        {
            xTOC->setPropertyValue("UserIndexName", uno::makeAny(sUserIndex));
        }
    }
    pContext->SetTOC( xTOC );
    m_bParaHadField = false;

    uno::Reference< text::XTextContent > xToInsert( xTOC, uno::UNO_QUERY );
    appendTextContent(xToInsert, uno::Sequence< beans::PropertyValue >() );

    if( lcl_FindInCommand( pContext->GetCommand(), 'c', sValue ))
    {
        sValue = sValue.replaceAll("\"", "");
        uno::Reference<text::XTextColumns> xTextColumns;
        xTOC->getPropertyValue(getPropertyName( PROP_TEXT_COLUMNS )) >>= xTextColumns;
        if (xTextColumns.is())
        {
            xTextColumns->setColumnCount( sValue.toInt32() );
            xTOC->setPropertyValue( getPropertyName( PROP_TEXT_COLUMNS ), uno::makeAny( xTextColumns ) );
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

    m_bSetUserFieldContent = false;
    m_bSetCitation = false;
    m_bSetDateValue = false;
    const FieldConversionMap_t& aFieldConversionMap = lcl_GetFieldConversion();

    try
    {
        uno::Reference< uno::XInterface > xFieldInterface;

        const auto& [sType, vArguments, vSwitches]{ splitFieldCommand(pContext->GetCommand()) };
        (void)vSwitches;
        OUString const sFirstParam(vArguments.empty() ? OUString() : vArguments.front());

        // apply font size to the form control
        if (!m_aTextAppendStack.empty() &&  m_pLastCharacterContext && ( m_pLastCharacterContext->isSet(PROP_CHAR_HEIGHT) || m_pLastCharacterContext->isSet(PROP_CHAR_FONT_NAME )))
        {
            uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
            if (xTextAppend.is())
            {
                uno::Reference< text::XTextCursor > xCrsr = xTextAppend->getText()->createTextCursor();
                if (xCrsr.is())
                {
                    xCrsr->gotoEnd(false);
                    uno::Reference< beans::XPropertySet > xProp( xCrsr, uno::UNO_QUERY );
                    if (m_pLastCharacterContext->isSet(PROP_CHAR_HEIGHT))
                    {
                        xProp->setPropertyValue(getPropertyName(PROP_CHAR_HEIGHT), m_pLastCharacterContext->getProperty(PROP_CHAR_HEIGHT)->second);
                        if (m_pLastCharacterContext->isSet(PROP_CHAR_HEIGHT_COMPLEX))
                            xProp->setPropertyValue(getPropertyName(PROP_CHAR_HEIGHT_COMPLEX), m_pLastCharacterContext->getProperty(PROP_CHAR_HEIGHT_COMPLEX)->second);
                    }
                    if (m_pLastCharacterContext->isSet(PROP_CHAR_FONT_NAME))
                        xProp->setPropertyValue(getPropertyName(PROP_CHAR_FONT_NAME), m_pLastCharacterContext->getProperty(PROP_CHAR_FONT_NAME)->second);
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
            if (m_bStartTOC && (aIt->second.eFieldId == FIELD_PAGEREF) )
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

                if (m_xTextFactory.is())
                {
                    xFieldInterface = m_xTextFactory->createInstance(sServiceName);
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
                            uno::makeAny( true ));
                        m_bSetDateValue = true;
                    }
                    else
                        xFieldProperties->setPropertyValue(
                            getPropertyName(PROP_IS_FIXED),
                            uno::makeAny( false ));

                    xFieldProperties->setPropertyValue(
                        getPropertyName(PROP_IS_DATE),
                        uno::makeAny( true ));
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
                        getPropertyName( PROP_IS_FIXED ), uno::makeAny( false ));
                        //PROP_CURRENT_PRESENTATION is set later anyway
                }
                break;
                case FIELD_CREATEDATE  :
                {
                    xFieldProperties->setPropertyValue(
                        getPropertyName( PROP_IS_DATE ), uno::makeAny( true ));
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
                        m_bSetUserFieldContent = true;
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
                    if (!aResult.sType.isEmpty() && m_xTextFactory.is())
                    {
                        xFieldInterface = m_xTextFactory->createInstance("com.sun.star.text.TextField." + aResult.sType);
                        xFieldProperties =
                            uno::Reference< beans::XPropertySet >( xFieldInterface,
                                uno::UNO_QUERY_THROW);
                        xFieldProperties->setPropertyValue(getPropertyName(PROP_CONTENT), uno::makeAny(aResult.sResult));
                    }
                    else
                    {
                        //merge Read_SubF_Ruby into filter/.../util.cxx and reuse that ?
                        sal_Int32 nSpaceIndex = aCommand.indexOf(' ');
                        if(nSpaceIndex > 0)
                            aCommand = aCommand.copy(nSpaceIndex).trim();
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
                                    sal_Int32 nDown = aCommand.copy(0, nStartIndex).toInt32();
                                    OUString aContent = aCommand.copy(nStartIndex + 1, nEndIndex - nStartIndex - 1);
                                    PropertyMapPtr pCharContext = GetTopContext();
                                    // dHeight is the font size of the current style.
                                    double dHeight = 0;
                                    if ((GetPropertyFromParaStyleSheet(PROP_CHAR_HEIGHT) >>= dHeight) && dHeight != 0)
                                        // Character escapement should be given in negative percents for subscripts.
                                        pCharContext->Insert(PROP_CHAR_ESCAPEMENT, uno::makeAny( sal_Int16(- 100 * nDown / dHeight) ) );
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
                                getPropertyName(PROP_HINT), uno::makeAny( pContext->GetCommand().getToken(1, '\"')));
                break;
                case FIELD_FILENAME:
                {
                    sal_Int32 nNumberingTypeIndex = pContext->GetCommand().indexOf("\\p");
                    if (xFieldProperties.is())
                        xFieldProperties->setPropertyValue(
                                getPropertyName(PROP_FILE_FORMAT),
                                uno::makeAny( nNumberingTypeIndex > 0 ? text::FilenameDisplayFormat::FULL : text::FilenameDisplayFormat::NAME_AND_EXT ));
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
                case FIELD_IF           : break;
                case FIELD_INFO         : break;
                case FIELD_INCLUDEPICTURE: break;
                case FIELD_KEYWORDS     :
                {
                    if (!sFirstParam.isEmpty())
                    {
                        xFieldProperties->setPropertyValue(
                                getPropertyName( PROP_IS_FIXED ), uno::makeAny( true ));
                        //PROP_CURRENT_PRESENTATION is set later anyway
                    }
                }
                break;
                case FIELD_LASTSAVEDBY  : break;
                case FIELD_MACROBUTTON:
                {
                    //extract macro name
                    sal_Int32 nIndex = sizeof(" MACROBUTTON ");
                    OUString sMacro = pContext->GetCommand().getToken( 0, ' ', nIndex);
                    if (xFieldProperties.is())
                        xFieldProperties->setPropertyValue(
                                getPropertyName(PROP_MACRO_NAME), uno::makeAny( sMacro ));

                    //extract quick help text
                    if(xFieldProperties.is() && pContext->GetCommand().getLength() > nIndex + 1)
                    {
                        xFieldProperties->setPropertyValue(
                            getPropertyName(PROP_HINT),
                            uno::makeAny( pContext->GetCommand().copy( nIndex )));
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
                                uno::makeAny( lcl_ParseNumberingType(pContext->GetCommand()) ));
                        xFieldProperties->setPropertyValue(
                                getPropertyName(PROP_SUB_TYPE),
                                uno::makeAny( text::PageNumberType_CURRENT ));
                    }

                break;
                case FIELD_PAGEREF:
                case FIELD_REF:
                if (xFieldProperties.is() && !m_bStartTOC)
                {
                    bool bPageRef = aIt->second.eFieldId == FIELD_PAGEREF;

                    // Do we need a GetReference (default) or a GetExpression field?
                    uno::Reference< text::XTextFieldsSupplier > xFieldsSupplier( GetTextDocument(), uno::UNO_QUERY );
                    uno::Reference< container::XNameAccess > xFieldMasterAccess = xFieldsSupplier->getTextFieldMasters();

                    if (!xFieldMasterAccess->hasByName(
                            "com.sun.star.text.FieldMaster.SetExpression."
                            + sFirstParam))
                    {
                        xFieldProperties->setPropertyValue(
                            getPropertyName(PROP_REFERENCE_FIELD_SOURCE),
                            uno::makeAny( sal_Int16(text::ReferenceFieldSource::BOOKMARK)) );
                        xFieldProperties->setPropertyValue(
                            getPropertyName(PROP_SOURCE_NAME),
                            uno::makeAny(sFirstParam) );
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
                                getPropertyName( PROP_REFERENCE_FIELD_PART ), uno::makeAny( nFieldPart ));
                    }
                    else if( m_xTextFactory.is() )
                    {
                        xFieldInterface = m_xTextFactory->createInstance("com.sun.star.text.TextField.GetExpression");
                        xFieldProperties.set(xFieldInterface, uno::UNO_QUERY);
                        xFieldProperties->setPropertyValue(
                            getPropertyName(PROP_CONTENT),
                            uno::makeAny(sFirstParam));
                        xFieldProperties->setPropertyValue(getPropertyName(PROP_SUB_TYPE), uno::makeAny(text::SetVariableType::STRING));
                    }
                }
                break;
                case FIELD_REVNUM       : break;
                case FIELD_SAVEDATE     :
                    SetNumberFormat( pContext->GetCommand(), xFieldProperties );
                break;
                case FIELD_SECTION      : break;
                case FIELD_SECTIONPAGES : break;
                case FIELD_SEQ          :
                {
                    // command looks like: " SEQ Table \* ARABIC "
                    OUString sCmd(pContext->GetCommand());
                    // find the sequence name, e.g. "SEQ"
                    OUString sSeqName = msfilter::util::findQuotedText(sCmd, "SEQ ", '\\');
                    sSeqName = sSeqName.trim();

                    // create a sequence field master using the sequence name
                    uno::Reference< beans::XPropertySet > xMaster = FindOrCreateFieldMaster(
                                "com.sun.star.text.FieldMaster.SetExpression",
                                sSeqName);

                    xMaster->setPropertyValue(
                        getPropertyName(PROP_SUB_TYPE),
                        uno::makeAny(text::SetVariableType::SEQUENCE));

                    // apply the numbering type
                    xFieldProperties->setPropertyValue(
                        getPropertyName(PROP_NUMBERING_TYPE),
                        uno::makeAny( lcl_ParseNumberingType(pContext->GetCommand()) ));

                    // attach the master to the field
                    uno::Reference< text::XDependentTextField > xDependentField( xFieldInterface, uno::UNO_QUERY_THROW );
                    xDependentField->attachTextFieldMaster( xMaster );

                    OUString sFormula = sSeqName + "+1";
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
                            uno::makeAny(sFormula));

                    // Take care of the numeric formatting definition, default is Arabic
                    sal_Int16 nNumberingType = lcl_ParseNumberingType(pContext->GetCommand());
                    if (nNumberingType == style::NumberingType::PAGE_DESCRIPTOR)
                        nNumberingType = style::NumberingType::ARABIC;
                    xFieldProperties->setPropertyValue(
                            getPropertyName(PROP_NUMBERING_TYPE),
                            uno::makeAny(nNumberingType));
                }
                break;
                case FIELD_SET          :
                    handleFieldSet(pContext, xFieldInterface, xFieldProperties);
                break;
                case FIELD_SKIPIF       : break;
                case FIELD_STYLEREF     : break;
                case FIELD_SUBJECT      :
                {
                    if (!sFirstParam.isEmpty())
                    {
                        xFieldProperties->setPropertyValue(
                                getPropertyName( PROP_IS_FIXED ), uno::makeAny( true ));
                        //PROP_CURRENT_PRESENTATION is set later anyway
                    }
                }
                break;
                case FIELD_SYMBOL:
                {
                    uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
                    OUString sSymbol( sal_Unicode( sFirstParam.startsWithIgnoreAsciiCase("0x") ?  sFirstParam.copy(2).toUInt32(16) : sFirstParam.toUInt32() ) );
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
                            xProp->setPropertyValue(getPropertyName(PROP_CHAR_FONT_CHAR_SET), uno::makeAny(awt::CharSet::SYMBOL));
                            if(bHasFont)
                            {
                                uno::Any    aVal = uno::makeAny( sFont );
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
                            uno::makeAny( true ));
                        m_bSetDateValue = true;
                    }
                    SetNumberFormat( pContext->GetCommand(), xFieldProperties );
                }
                break;
                case FIELD_TITLE        :
                {
                    if (!sFirstParam.isEmpty())
                    {
                        xFieldProperties->setPropertyValue(
                                getPropertyName( PROP_IS_FIXED ), uno::makeAny( true ));
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
                    if( !m_xTextFactory.is() )
                        break;

                    // only UserIndexMark can handle user index types defined by \f
                    // e.g. XE "text" \f "user-index-id"
                    OUString sUserIndex;
                    OUString sFieldServiceName =
                        lcl_FindInCommand( pContext->GetCommand(), 'f', sUserIndex )
                            ? "com.sun.star.text.UserIndexMark"
                            : OUString::createFromAscii(aIt->second.cFieldServiceName);
                    uno::Reference< beans::XPropertySet > xTC(
                            m_xTextFactory->createInstance(sFieldServiceName),
                                    uno::UNO_QUERY_THROW);

                    if (!sFirstParam.isEmpty())
                    {
                        xTC->setPropertyValue(sUserIndex.isEmpty()
                                    ? OUString("PrimaryKey")
                                    : OUString("AlternativeText"),
                                uno::makeAny(sFirstParam));
                    }

                    sUserIndex = lcl_trim(sUserIndex);
                    if (!sUserIndex.isEmpty())
                    {
                        xTC->setPropertyValue("UserIndexName",
                                uno::makeAny(sUserIndex));
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
                    if( !m_xTextFactory.is() )
                        break;

                    xFieldInterface = m_xTextFactory->createInstance(
                              OUString::createFromAscii(aIt->second.cFieldServiceName));
                    uno::Reference< beans::XPropertySet > xTC(xFieldInterface,
                              uno::UNO_QUERY_THROW);
                    OUString sCmd(pContext->GetCommand());//sCmd is the entire instrText including the index e.g. CITATION Kra06 \l 1033
                    if( !sCmd.isEmpty()){
                        uno::Sequence<beans::PropertyValue> aValues( comphelper::InitPropertySequence({
                            { "Identifier", uno::Any(sCmd) }
                        }));
                        xTC->setPropertyValue("Fields", uno::makeAny(aValues));
                    }
                    uno::Reference< text::XTextContent > xToInsert( xTC, uno::UNO_QUERY );

                    uno::Sequence<beans::PropertyValue> aValues
                        = m_aFieldStack.back()->getProperties()->GetPropertyValues();
                    appendTextContent(xToInsert, aValues);
                    m_bSetCitation = true;
                }
                break;

                case FIELD_TC :
                {
                    if( !m_xTextFactory.is() )
                        break;

                    uno::Reference< beans::XPropertySet > xTC(
                        m_xTextFactory->createInstance(
                            OUString::createFromAscii(aIt->second.cFieldServiceName)),
                            uno::UNO_QUERY_THROW);
                    if (!sFirstParam.isEmpty())
                    {
                        xTC->setPropertyValue(getPropertyName(PROP_ALTERNATIVE_TEXT),
                            uno::makeAny(sFirstParam));
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
                            xTC->setPropertyValue(getPropertyName(PROP_LEVEL), uno::makeAny( static_cast<sal_Int16>(nLevel) ));
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
                        uno::makeAny( lcl_ParseNumberingType(pContext->GetCommand()) ));
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
            if (sType != "SHAPE" && m_xTextFactory.is() && !m_aTextAppendStack.empty())
            {
                xFieldInterface = m_xTextFactory->createInstance("com.sun.star.text.Fieldmark");

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
                xNameCont->insertByName(ODF_CODE_PARAM, uno::makeAny(aCode));
                ww::eField const id(GetWW8FieldId(sType));
                if (id != ww::eNONE)
                {   // tdf#129247 tdf#134264 set WW8 id for WW8 export
                    xNameCont->insertByName(ODF_ID_PARAM, uno::makeAny(OUString::number(id)));
                }
            }
            else
                m_bParaHadField = false;
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
                if( m_bSetUserFieldContent )
                {
                    // user field content has to be set at the field master
                    uno::Reference< text::XDependentTextField > xDependentField( xTextField, uno::UNO_QUERY_THROW );
                    xDependentField->getTextFieldMaster()->setPropertyValue(
                            getPropertyName(PROP_CONTENT),
                         uno::makeAny( rResult ));
                }
                else if ( m_bSetCitation )
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
                                uno::makeAny(aValues));
                    }
                }
                else if ( m_bSetDateValue )
                {
                    uno::Reference< util::XNumberFormatsSupplier > xNumberSupplier( m_xTextDocument, uno::UNO_QUERY_THROW );

                    uno::Reference<util::XNumberFormatter> xFormatter(util::NumberFormatter::create(m_xComponentContext), uno::UNO_QUERY_THROW);
                    xFormatter->attachNumberFormatsSupplier( xNumberSupplier );
                    sal_Int32 nKey = 0;

                    uno::Reference< beans::XPropertySet > xFieldProperties( xTextField, uno::UNO_QUERY_THROW);

                    xFieldProperties->getPropertyValue( "NumberFormat" ) >>= nKey;
                    xFieldProperties->setPropertyValue(
                        "DateTimeValue",
                        uno::makeAny( lcl_dateTimeFromSerial( xFormatter->convertStringToNumber( nKey, rResult ) ) ) );
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
                         uno::makeAny( rResult ));

                    if (xServiceInfo->supportsService(
                            "com.sun.star.text.TextField.DocInfo.CreateDateTime"))
                    {
                        // Creation time is const, don't try to update it.
                        xFieldProperties->setPropertyValue("IsFixed", uno::makeAny(true));
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
                uno::Reference< text::XTextCursor > xCrsr = xTextAppend->createTextCursorByRange(pContext->GetStartRange());
                uno::Reference< text::XTextContent > xToInsert( pContext->GetTOC(), uno::UNO_QUERY );
                if( xToInsert.is() )
                {
                    if (m_bStartedTOC || m_bStartIndex || m_bStartBibliography)
                    {
                        // inside SDT, last empty paragraph is also part of index
                        if (!m_bParaChanged && !m_xSdtEntryStart)
                        {
                            // End of index is the first item on a new paragraph - this paragraph
                            // should not be part of index
                            auto xCursor
                                = xTextAppend->createTextCursorByRange(xTextAppend->getEnd());
                            xCursor->gotoEnd(false);
                            xCursor->goLeft(1, true);
                            // delete
                            xCursor->setString(OUString());
                            // But a new paragraph should be started after the index instead
                            xTextAppend->finishParagraph(css::beans::PropertyValues());
                        }
                        m_bStartedTOC = false;
                        m_aTextAppendStack.pop();
                        m_bTextInserted = false;
                        m_bParaChanged = true; // the paragraph must stay anyway
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
                    if( !xToInsert.is() && !m_bStartTOC && !m_bStartIndex && !m_bStartBibliography )
                        xToInsert = pContext->GetTextField();
                    if( xToInsert.is() && !m_bStartTOC && !m_bStartIndex && !m_bStartBibliography)
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
                        FormControlHelper::Pointer_t pFormControlHelper(pContext->getFormControlHelper());
                        if (pFormControlHelper)
                        {
                            uno::Reference< text::XFormField > xFormField( pContext->GetFormField() );
                            assert(xCrsr.is());
                            if (pFormControlHelper->hasFFDataHandler())
                            {
                                xToInsert.set(xFormField, uno::UNO_QUERY);
                                if (xFormField.is() && xToInsert.is())
                                {
                                    PopFieldmark(m_aTextAppendStack, xCrsr,
                                        pContext->GetFieldId());
                                    pFormControlHelper->processField( xFormField );
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
                        else if (!pContext->GetHyperlinkURL().isEmpty() && xCrsr.is())
                        {
                            xCrsr->gotoEnd( true );

                            // Draw components (like comments) need hyperlinks set differently
                            SvxUnoTextRangeBase* pDrawText = dynamic_cast<SvxUnoTextRangeBase*>(xCrsr.get());
                            if ( pDrawText )
                                pDrawText->attachField( std::make_unique<SvxURLField>(pContext->GetHyperlinkURL(), xCrsr->getString(), SvxURLFormat::AppDefault) );
                            else
                            {
                                uno::Reference< beans::XPropertySet > xCrsrProperties( xCrsr, uno::UNO_QUERY_THROW );
                                xCrsrProperties->setPropertyValue(getPropertyName(PROP_HYPER_LINK_U_R_L), uno::
                                                                  makeAny(pContext->GetHyperlinkURL()));

                                if (!pContext->GetHyperlinkTarget().isEmpty())
                                    xCrsrProperties->setPropertyValue("HyperLinkTarget", uno::makeAny(pContext->GetHyperlinkTarget()));

                                if (m_bStartTOC) {
                                    OUString sDisplayName("Index Link");
                                    xCrsrProperties->setPropertyValue("VisitedCharStyleName",uno::makeAny(sDisplayName));
                                    xCrsrProperties->setPropertyValue("UnvisitedCharStyleName",uno::makeAny(sDisplayName));
                                }
                                else
                                {
                                    uno::Any aAny = xCrsrProperties->getPropertyValue("CharStyleName");
                                    OUString charStyle;
                                    if (css::uno::fromAny(aAny, &charStyle))
                                    {
                                        if (charStyle.isEmpty())
                                        {
                                            xCrsrProperties->setPropertyValue("VisitedCharStyleName", uno::makeAny(OUString("Default Style")));
                                            xCrsrProperties->setPropertyValue("UnvisitedCharStyleName", uno::makeAny(OUString("Default Style")));
                                        }
                                        else if (charStyle.equalsIgnoreAsciiCase("Internet Link"))
                                        {
                                            xCrsrProperties->setPropertyValue("CharStyleName", uno::makeAny(OUString("Default Style")));
                                        }
                                        else
                                        {
                                            xCrsrProperties->setPropertyValue("VisitedCharStyleName", aAny);
                                            xCrsrProperties->setPropertyValue("UnvisitedCharStyleName", aAny);
                                        }
                                    }
                                }
                            }
                        }
                        else if (m_nStartGenericField != 0)
                        {
                            --m_nStartGenericField;
                            PopFieldmark(m_aTextAppendStack, xCrsr, pContext->GetFieldId());
                            if(m_bTextInserted)
                            {
                                m_bTextInserted = false;
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
    if(hasTableManager() && getTableManager().isInCell() && m_nTableDepth == 0 && GetIsFirstParagraphInSection()
                    && !GetIsDummyParaAddedForTableInSection() &&!GetIsTextFrameInserted())
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
            if (m_xTextFactory.is())
            {
                uno::Reference< text::XTextContent > xBookmark( m_xTextFactory->createInstance( "com.sun.star.text.Bookmark" ), uno::UNO_QUERY_THROW );
                uno::Reference< text::XTextCursor > xCursor;
                uno::Reference< text::XText > xText = aBookmarkIter->second.m_xTextRange->getText();
                if( aBookmarkIter->second.m_bIsStartOfText && !bIsAfterDummyPara)
                {
                    xCursor = xText->createTextCursorByRange( xText->getStart() );
                }
                else
                {
                    xCursor = xText->createTextCursorByRange( aBookmarkIter->second.m_xTextRange );
                    xCursor->goRight( 1, false );
                }

                xCursor->gotoRange( xTextAppend->getEnd(), true );
                // A Paragraph was recently finished, and a new Paragraph has not been started as yet
                // then  move the bookmark-End to the earlier paragraph
                if (IsOutsideAParagraph())
                {
                    // keep bookmark range
                    uno::Reference< text::XTextRange > xStart = xCursor->getStart();
                    xCursor->goLeft( 1, false );
                    xCursor->gotoRange(xStart, true );
                }
                uno::Reference< container::XNamed > xBkmNamed( xBookmark, uno::UNO_QUERY_THROW );
                SAL_WARN_IF(aBookmarkIter->second.m_sBookmarkName.isEmpty(), "writerfilter.dmapper", "anonymous bookmark");
                //todo: make sure the name is not used already!
                xBkmNamed->setName( aBookmarkIter->second.m_sBookmarkName );
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
    if (getTableManager().isInCell() && m_nTableDepth == 0 && GetIsFirstParagraphInSection()
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
            if (m_xTextFactory.is())
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
                uno::Reference< text::XTextContent > xPerm(m_xTextFactory->createInstance("com.sun.star.text.Bookmark"), uno::UNO_QUERY_THROW);
                uno::Reference< container::XNamed > xPermNamed(xPerm, uno::UNO_QUERY_THROW);
                xPermNamed->setName(aPermIter->second.createBookmarkName());

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

GraphicImportPtr const & DomainMapper_Impl::GetGraphicImport(GraphicImportType eGraphicImportType)
{
    if(!m_pGraphicImport)
        m_pGraphicImport = new GraphicImport( m_xComponentContext, m_xTextFactory, m_rDMapper, eGraphicImportType, m_aPositionOffsets, m_aAligns, m_aPositivePercentages );
    return m_pGraphicImport;
}
/*-------------------------------------------------------------------------
    reset graphic import if the last import resulted in a shape, not a graphic
  -----------------------------------------------------------------------*/
void DomainMapper_Impl::ResetGraphicImport()
{
    m_pGraphicImport.clear();
}


void  DomainMapper_Impl::ImportGraphic(const writerfilter::Reference< Properties >::Pointer_t& ref, GraphicImportType eGraphicImportType)
{
    GetGraphicImport(eGraphicImportType);
    if( eGraphicImportType != IMPORT_AS_DETECTED_INLINE && eGraphicImportType != IMPORT_AS_DETECTED_ANCHOR )
    {
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
            xPropertySet->setPropertyValue("FrameInteropGrabBag", uno::makeAny(aFrameGrabBag.getAsConstPropertyValueList()));
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
            xPropertySet->setPropertyValue("FrameInteropGrabBag",uno::makeAny(aFrameGrabBag));
        }
    }


    // Update the shape properties if it is embedded object.
    if(m_xEmbedded.is()){
        if (m_pGraphicImport->GetXShapeObject())
                m_pGraphicImport->GetXShapeObject()->setPosition(
                    m_pGraphicImport->GetGraphicObjectPosition());

        uno::Reference<drawing::XShape> xShape = m_pGraphicImport->GetXShapeObject();
        UpdateEmbeddedShapeProps(xShape);
        if (eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR)
        {
            uno::Reference<beans::XPropertySet> xEmbeddedProps(m_xEmbedded, uno::UNO_QUERY);
            xEmbeddedProps->setPropertyValue("AnchorType", uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));
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
        if ( eGraphicImportType != IMPORT_AS_DETECTED_INLINE && !m_aRedlines.top().empty() )
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
                    appendTextPortion(u"​", pEmpty);
                    appendTextContent( xTextContent, uno::Sequence< beans::PropertyValue >() );
                    bAppend = false;
                    xCrsr->gotoEnd(false);
                    appendTextPortion(u"​", pEmpty);

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

        if (eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR && !m_aTextAppendStack.empty())
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
        else if (eGraphicImportType == IMPORT_AS_DETECTED_INLINE)
        {
            m_bParaWithInlineObject = true;

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
    m_xEmbedded.clear();
    m_pGraphicImport.clear();
}


void DomainMapper_Impl::SetLineNumbering( sal_Int32 nLnnMod, sal_uInt32 nLnc, sal_Int32 ndxaLnn )
{
    if( !m_bLineNumberingSet )
    {
        try
        {
            uno::Reference< text::XLineNumberingProperties > xLineProperties( m_xTextDocument, uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xProperties = xLineProperties->getLineNumberingProperties();
            uno::Any aTrue( uno::makeAny( true ));
            xProperties->setPropertyValue( getPropertyName( PROP_IS_ON                  ), aTrue);
            xProperties->setPropertyValue( getPropertyName( PROP_COUNT_EMPTY_LINES      ), aTrue );
            xProperties->setPropertyValue( getPropertyName( PROP_COUNT_LINES_IN_FRAMES  ), uno::makeAny( false ) );
            xProperties->setPropertyValue( getPropertyName( PROP_INTERVAL               ), uno::makeAny( static_cast< sal_Int16 >( nLnnMod )));
            xProperties->setPropertyValue( getPropertyName( PROP_DISTANCE               ), uno::makeAny( ConversionHelper::convertTwipToMM100(ndxaLnn) ));
            xProperties->setPropertyValue( getPropertyName( PROP_NUMBER_POSITION        ), uno::makeAny( style::LineNumberPosition::LEFT));
            xProperties->setPropertyValue( getPropertyName( PROP_NUMBERING_TYPE         ), uno::makeAny( style::NumberingType::ARABIC));
            xProperties->setPropertyValue( getPropertyName( PROP_RESTART_AT_EACH_PAGE   ), uno::makeAny( nLnc == NS_ooxml::LN_Value_ST_LineNumberRestart_newPage ));
        }
        catch( const uno::Exception& )
        {}
    }
    m_bLineNumberingSet = true;
    uno::Reference< style::XStyleFamiliesSupplier > xStylesSupplier( GetTextDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xStyleFamilies = xStylesSupplier->getStyleFamilies();
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
        m_xAnnotationField->setPropertyValue("Author", uno::makeAny(sAuthor));
}

void DomainMapper_Impl::SetCurrentRedlineInitials( const OUString& sInitials )
{
    if (m_xAnnotationField.is())
        m_xAnnotationField->setPropertyValue("Initials", uno::makeAny(sInitials));
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
        m_xAnnotationField->setPropertyValue("DateTimeValue", uno::makeAny(ConversionHelper::ConvertDateStringToDateTime(sDate)));
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
    if (!(m_pSettingsTable && m_xTextFactory.is()))
        return;

    try
    {
        uno::Reference< beans::XPropertySet > xTextDefaults(m_xTextFactory->createInstance("com.sun.star.text.Defaults"), uno::UNO_QUERY_THROW );
        sal_Int32 nDefTab = m_pSettingsTable->GetDefaultTabStop();
        xTextDefaults->setPropertyValue( getPropertyName( PROP_TAB_STOP_DISTANCE ), uno::makeAny(nDefTab) );
        if (m_pSettingsTable->GetLinkStyles())
        {
            // If linked styles are enabled, set paragraph defaults from Word's default template
            xTextDefaults->setPropertyValue(getPropertyName(PROP_PARA_BOTTOM_MARGIN), uno::makeAny(ConversionHelper::convertTwipToMM100(200)));
            style::LineSpacing aSpacing;
            aSpacing.Mode = style::LineSpacingMode::PROP;
            aSpacing.Height = sal_Int16(115);
            xTextDefaults->setPropertyValue(getPropertyName(PROP_PARA_LINE_SPACING), uno::makeAny(aSpacing));
        }

        if (m_pSettingsTable->GetZoomFactor() || m_pSettingsTable->GetView())
        {
            std::vector<beans::PropertyValue> aViewProps;
            if (m_pSettingsTable->GetZoomFactor())
            {
                aViewProps.emplace_back("ZoomFactor", -1, uno::makeAny(m_pSettingsTable->GetZoomFactor()), beans::PropertyState_DIRECT_VALUE);
                aViewProps.emplace_back("VisibleBottom", -1, uno::makeAny(sal_Int32(0)), beans::PropertyState_DIRECT_VALUE);
                aViewProps.emplace_back("ZoomType", -1,
                                        uno::makeAny(m_pSettingsTable->GetZoomType()),
                                        beans::PropertyState_DIRECT_VALUE);
            }
            uno::Reference<container::XIndexContainer> xBox = document::IndexedPropertyValues::create(m_xComponentContext);
            xBox->insertByIndex(sal_Int32(0), uno::makeAny(comphelper::containerToSequence(aViewProps)));
            uno::Reference<document::XViewDataSupplier> xViewDataSupplier(m_xTextDocument, uno::UNO_QUERY);
            xViewDataSupplier->setViewData(xBox);
        }

        uno::Reference< beans::XPropertySet > xSettings(m_xTextFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY);

        if (m_pSettingsTable->GetDoNotExpandShiftReturn())
            xSettings->setPropertyValue( "DoNotJustifyLinesWithManualBreak", uno::makeAny(true) );
        if (m_pSettingsTable->GetUsePrinterMetrics())
            xSettings->setPropertyValue("PrinterIndependentLayout", uno::makeAny(document::PrinterIndependentLayout::DISABLED));
        if( m_pSettingsTable->GetEmbedTrueTypeFonts())
            xSettings->setPropertyValue( getPropertyName( PROP_EMBED_FONTS ), uno::makeAny(true) );
        if( m_pSettingsTable->GetEmbedSystemFonts())
            xSettings->setPropertyValue( getPropertyName( PROP_EMBED_SYSTEM_FONTS ), uno::makeAny(true) );
        xSettings->setPropertyValue("AddParaTableSpacing", uno::makeAny(m_pSettingsTable->GetDoNotUseHTMLParagraphAutoSpacing()));
        if (m_pSettingsTable->GetNoLeading())
        {
            xSettings->setPropertyValue("AddExternalLeading", uno::makeAny(!m_pSettingsTable->GetNoLeading()));
        }
        if( m_pSettingsTable->GetProtectForm() )
            xSettings->setPropertyValue("ProtectForm", uno::makeAny( true ));
        if( m_pSettingsTable->GetReadOnly() )
            xSettings->setPropertyValue("LoadReadonly", uno::makeAny( true ));
        if (m_pSettingsTable->GetGutterAtTop())
        {
            xSettings->setPropertyValue("GutterAtTop", uno::makeAny(true));
        }
        uno::Sequence<beans::PropertyValue> aWriteProtection
                = m_pSettingsTable->GetWriteProtectionSettings();
        if (aWriteProtection.hasElements())
                xSettings->setPropertyValue("ModifyPasswordInfo", uno::makeAny(aWriteProtection));
    }
    catch(const uno::Exception&)
    {
    }
}

uno::Reference<container::XIndexAccess> DomainMapper_Impl::GetCurrentNumberingRules(sal_Int32* pListLevel)
{
    uno::Reference<container::XIndexAccess> xRet;
    try
    {
        OUString aStyle = GetCurrentParaStyleName();
        if (aStyle.isEmpty())
            return xRet;
        const StyleSheetEntryPtr pEntry = GetStyleSheetTable()->FindStyleSheetByConvertedStyleName(aStyle);
        if (!pEntry)
            return xRet;
        const StyleSheetPropertyMap* pStyleSheetProperties = pEntry->pProperties.get();
        if (!pStyleSheetProperties)
            return xRet;
        sal_Int32 nListId = pStyleSheetProperties->GetListId();
        if (nListId < 0)
            return xRet;
        if (pListLevel)
            *pListLevel = pStyleSheetProperties->GetListLevel();

        // So we are in a paragraph style and it has numbering. Look up the relevant numbering rules.
        auto const pList(GetListTable()->GetList(nListId));
        OUString aListName;
        if (pList)
        {
            aListName = pList->GetStyleName();
        }
        uno::Reference< style::XStyleFamiliesSupplier > xStylesSupplier(GetTextDocument(), uno::UNO_QUERY_THROW);
        uno::Reference< container::XNameAccess > xStyleFamilies = xStylesSupplier->getStyleFamilies();
        uno::Reference<container::XNameAccess> xNumberingStyles;
        xStyleFamilies->getByName("NumberingStyles") >>= xNumberingStyles;
        uno::Reference<beans::XPropertySet> xStyle(xNumberingStyles->getByName(aListName), uno::UNO_QUERY);
        xRet.set(xStyle->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("writerfilter.dmapper", "GetCurrentNumberingRules: exception caught");
    }
    return xRet;
}

uno::Reference<beans::XPropertySet> DomainMapper_Impl::GetCurrentNumberingCharStyle()
{
    uno::Reference<beans::XPropertySet> xRet;
    try
    {
        sal_Int32 nListLevel = -1;
        uno::Reference<container::XIndexAccess> xLevels;
        if ( GetTopContextType() == CONTEXT_PARAGRAPH )
            xLevels = GetCurrentNumberingRules(&nListLevel);
        if (!xLevels.is())
        {
            if (IsOOXMLImport())
                return xRet;

            PropertyMapPtr pContext = m_pTopContext;
            if (IsRTFImport() && !IsOpenField())
            {
                // Looking up the paragraph context explicitly (and not just taking
                // the top context) is necessary for RTF, where formatting of a run
                // and of the paragraph mark is not separated.
                // We know that the formatting inside a field won't affect the
                // paragraph marker formatting, though.
                pContext = GetTopContextOfType(CONTEXT_PARAGRAPH);
                if (!pContext)
                    return xRet;
            }

            // In case numbering rules is not found via a style, try the direct formatting instead.
            std::optional<PropertyMap::Property> oProp = pContext->getProperty(PROP_NUMBERING_RULES);
            if (oProp)
            {
                xLevels.set(oProp->second, uno::UNO_QUERY);
                // Found the rules, then also try to look up our numbering level.
                oProp = pContext->getProperty(PROP_NUMBERING_LEVEL);
                if (oProp)
                    oProp->second >>= nListLevel;
                else
                    nListLevel = 0;
            }

            if (!xLevels.is())
                return xRet;
        }
        uno::Sequence<beans::PropertyValue> aProps;
        xLevels->getByIndex(nListLevel) >>= aProps;
        auto pProp = std::find_if(std::cbegin(aProps), std::cend(aProps),
            [](const beans::PropertyValue& rProp) { return rProp.Name == "CharStyleName"; });
        if (pProp != std::cend(aProps))
        {
            OUString aCharStyle;
            pProp->Value >>= aCharStyle;
            uno::Reference<container::XNameAccess> xCharacterStyles;
            uno::Reference< style::XStyleFamiliesSupplier > xStylesSupplier(GetTextDocument(), uno::UNO_QUERY);
            uno::Reference< container::XNameAccess > xStyleFamilies = xStylesSupplier->getStyleFamilies();
            xStyleFamilies->getByName("CharacterStyles") >>= xCharacterStyles;
            xRet.set(xCharacterStyles->getByName(aCharStyle), uno::UNO_QUERY_THROW);
        }
    }
    catch( const uno::Exception& )
    {
    }
    return xRet;
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
    deferredCharacterProperties[ id ] = value;
}

void DomainMapper_Impl::processDeferredCharacterProperties()
{
    // Actually process in DomainMapper, so that it's the same source file like normal processing.
    if( !deferredCharacterProperties.empty())
    {
        m_rDMapper.processDeferredCharacterProperties( deferredCharacterProperties );
        deferredCharacterProperties.clear();
    }
}

sal_Int32 DomainMapper_Impl::getNumberingProperty(const sal_Int32 nListId, sal_Int32 nNumberingLevel, const OUString& aProp)
{
    sal_Int32 nRet = 0;
    if ( nListId < 0 )
        return nRet;

    try
    {
        if (nNumberingLevel < 0) // It seems it's valid to omit numbering level, and in that case it means zero.
            nNumberingLevel = 0;

        auto const pList(GetListTable()->GetList(nListId));
        assert(pList);
        const OUString aListName = pList->GetStyleName();
        const uno::Reference< style::XStyleFamiliesSupplier > xStylesSupplier(GetTextDocument(), uno::UNO_QUERY_THROW);
        const uno::Reference< container::XNameAccess > xStyleFamilies = xStylesSupplier->getStyleFamilies();
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

    // Save "has footnote" state, which is specific to a section in the body
    // text, so state from substreams is not relevant.
    bool bHasFtn = m_bHasFtn;

    //finalize any waiting frames before starting alternate streams
    CheckUnregisteredFrameConversion();
    ExecuteFrameConversion();

    appendTableManager();
    // Appending a TableManager resets its TableHandler, so we need to append
    // that as well, or tables won't be imported properly in headers/footers.
    appendTableHandler();
    getTableManager().startLevel();

    //import of page header/footer
    //Ensure that only one header/footer per section is pushed

    switch( rName )
    {
    case NS_ooxml::LN_headerl:
            PushPageHeader(SectionPropertyMap::PAGE_LEFT);
        break;
    case NS_ooxml::LN_headerr:
            PushPageHeader(SectionPropertyMap::PAGE_RIGHT);
        break;
    case NS_ooxml::LN_headerf:
            PushPageHeader(SectionPropertyMap::PAGE_FIRST);
        break;
    case NS_ooxml::LN_footerl:
            PushPageFooter(SectionPropertyMap::PAGE_LEFT);
        break;
    case NS_ooxml::LN_footerr:
            PushPageFooter(SectionPropertyMap::PAGE_RIGHT);
        break;
    case NS_ooxml::LN_footerf:
            PushPageFooter(SectionPropertyMap::PAGE_FIRST);
        break;
    case NS_ooxml::LN_footnote:
    case NS_ooxml::LN_endnote:
        PushFootOrEndnote( NS_ooxml::LN_footnote == rName );
    break;
    case NS_ooxml::LN_annotation :
        PushAnnotation();
    break;
    }

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
    case NS_ooxml::LN_headerr:
    case NS_ooxml::LN_headerf:
    case NS_ooxml::LN_footerl:
    case NS_ooxml::LN_footerr:
    case NS_ooxml::LN_footerf:
        PopPageHeaderFooter();
    break;
    case NS_ooxml::LN_footnote:
    case NS_ooxml::LN_endnote:
        PopFootOrEndnote();
    break;
    case NS_ooxml::LN_annotation :
        PopAnnotation();
    break;
    }

    getTableManager().endLevel();
    popTableManager();
    m_bHasFtn = bHasFtn;

    switch(rName)
    {
    case NS_ooxml::LN_footnote:
    case NS_ooxml::LN_endnote:
        m_pTableHandler->setHadFootOrEndnote(true);
        m_bHasFtn = true;
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
