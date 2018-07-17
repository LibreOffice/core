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
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include "DomainMapper_Impl.hxx"
#include "ConversionHelper.hxx"
#include "SdtHelper.hxx"
#include "DomainMapperTableHandler.hxx"
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
#include <com/sun/star/text/XFootnote.hpp>
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
#include <com/sun/star/text/RubyPosition.hpp>
#include <com/sun/star/style/DropCapFormat.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <o3tl/temporary.hxx>
#include <oox/mathml/import.hxx>
#include <rtl/uri.hxx>
#include "GraphicHelpers.hxx"
#include <dmapper/GraphicZOrderHelper.hxx>

#include <oox/token/tokens.hxx>

#include <cmath>
#include <map>
#include <tuple>
#include <unordered_map>

#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <officecfg/Office/Common.hxx>
#include <filter/msfilter/util.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/propertysequence.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/mediadescriptor.hxx>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>


using namespace ::com::sun::star;
using namespace oox;
namespace writerfilter {
namespace dmapper{

//line numbering for header/footer
static void lcl_linenumberingHeaderFooter( const uno::Reference<container::XNameContainer>& xStyles, const OUString& rname, DomainMapper_Impl* dmapper )
{
    const StyleSheetEntryPtr pEntry = dmapper->GetStyleSheetTable()->FindStyleSheetByISTD( rname );
    if (!pEntry)
        return;
    const StyleSheetPropertyMap* pStyleSheetProperties = dynamic_cast<const StyleSheetPropertyMap*>( pEntry->pProperties.get() );
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
    if ( rxFieldProps.is() )
    {
        if ( !pFFDataHandler->getName().isEmpty() )
            rxFieldProps->setPropertyValue( "Name", uno::makeAny( pFFDataHandler->getName() ) );

        const FFDataHandler::DropDownEntries_t& rEntries = pFFDataHandler->getDropDownEntries();
        uno::Sequence< OUString > sItems( rEntries.size() );
        ::std::copy( rEntries.begin(), rEntries.end(), sItems.begin());
        if ( sItems.getLength() )
            rxFieldProps->setPropertyValue( "Items", uno::makeAny( sItems ) );

        sal_Int32 nResult = pFFDataHandler->getDropDownResult().toInt32();
        if ( nResult )
            rxFieldProps->setPropertyValue( "SelectedItem", uno::makeAny( sItems[ nResult ] ) );
        if ( !pFFDataHandler->getHelpText().isEmpty() )
             rxFieldProps->setPropertyValue( "Help", uno::makeAny( pFFDataHandler->getHelpText() ) );
    }
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

struct FieldConversion
{
    const sal_Char*     cFieldServiceName;
    FieldId             eFieldId;
};

typedef std::unordered_map<OUString, FieldConversion> FieldConversionMap_t;

uno::Any FloatingTableInfo::getPropertyValue(const OUString &propertyName)
{
    for( beans::PropertyValue const & propVal : m_aFrameProperties )
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
        m_bStartGenericField(false),
        m_bTextInserted(false),
        m_sCurrentPermId(0),
        m_pLastSectionContext( ),
        m_pLastCharacterContext(),
        m_sCurrentParaStyleName(),
        m_sDefaultParaStyleName(),
        m_bInStyleSheetImport( false ),
        m_bInAnyTableImport( false ),
        m_bInHeaderFooterImport( false ),
        m_bDiscardHeaderFooter( false ),
        m_bInFootOrEndnote(false),
        m_bSeenFootOrEndnoteSeparator(false),
        m_bLineNumberingSet( false ),
        m_bIsInFootnoteProperties( false ),
        m_bIsCustomFtnMark( false ),
        m_bIsParaMarkerChange( false ),
        m_bParaChanged( false ),
        m_bIsFirstParaInSection( true ),
        m_bDummyParaAddedForTableInSection( false ),
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
        m_xAnnotationField(),
        m_nAnnotationId( -1 ),
        m_aAnnotationPositions(),
        m_aSmartTagHandler(m_xComponentContext, m_xTextDocument),
        m_xInsertTextRange(rMediaDesc.getUnpackedValueOrDefault("TextInsertModeRange", uno::Reference<text::XTextRange>())),
        m_bIsNewDoc(!rMediaDesc.getUnpackedValueOrDefault("InsertMode", false)),
        m_bInTableStyleRunProps(false),
        m_nTableDepth(0),
        m_nTableCellDepth(0),
        m_nLastTableCellParagraphDepth(0),
        m_bHasFtn(false),
        m_bHasFtnSep(false),
        m_bIgnoreNextPara(false),
        m_bCheckFirstFootnoteTab(false),
        m_bIgnoreNextTab(false),
        m_bFrameBtLr(false),
        m_bIsSplitPara(false),
        m_vTextFramesForChaining(),
        m_bParaHadField(false),
        m_bParaAutoBefore(false),
        m_bParaAutoAfter(false),
        m_bPrevParaAutoAfter(false),
        m_bParaChangedBottomMargin(false),
        m_bFirstParagraphInCell(true),
        m_bSaveFirstParagraphInCell(false)
{
    m_aBaseUrl = rMediaDesc.getUnpackedValueOrDefault(
        utl::MediaDescriptor::PROP_DOCUMENTBASEURL(), OUString());
    if (m_aBaseUrl.isEmpty()) {
        m_aBaseUrl = rMediaDesc.getUnpackedValueOrDefault(
            utl::MediaDescriptor::PROP_URL(), OUString());
    }

    appendTableManager( );
    GetBodyText();
    uno::Reference< text::XTextAppend > xBodyTextAppend( m_xBodyText, uno::UNO_QUERY );
    m_aTextAppendStack.push(TextAppendContext(xBodyTextAppend,
                m_bIsNewDoc ? uno::Reference<text::XTextCursor>() : m_xBodyText->createTextCursorByRange(m_xInsertTextRange)));

    //todo: does it make sense to set the body text as static text interface?
    uno::Reference< text::XTextAppendAndConvert > xBodyTextAppendAndConvert( m_xBodyText, uno::UNO_QUERY );
    m_pTableHandler = new DomainMapperTableHandler(xBodyTextAppendAndConvert, *this);
    getTableManager( ).setHandler(m_pTableHandler);

    getTableManager( ).startLevel();
    m_bUsingEnhancedFields = !utl::ConfigManager::IsFuzzing() && officecfg::Office::Common::Filter::Microsoft::Import::ImportWWFieldsAsEnhancedFields::get(m_xComponentContext);

    m_pSdtHelper = new SdtHelper(*this);

    m_aRedlines.push(std::vector<RedlineParamsPtr>());
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
    if(!m_xPageStyles.is())
    {
        uno::Reference< style::XStyleFamiliesSupplier > xSupplier( m_xTextDocument, uno::UNO_QUERY );
        if (xSupplier.is())
            xSupplier->getStyleFamilies()->getByName("PageStyles") >>= m_xPageStyles;
    }
    return m_xPageStyles;
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
        uno::Reference< text::XTextCursor > xCrsr = xTextAppend->getText()->createTextCursor();
        uno::Reference< text::XText > xText = xTextAppend->getText();
        if(xCrsr.is() && xText.is())
        {
            xTextAppend->finishParagraph(  uno::Sequence< beans::PropertyValue >() );
            SetIsDummyParaAddedForTableInSection(true);
        }
    }
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
            xCursor.set(m_aTextAppendStack.top().xCursor, uno::UNO_QUERY);
        uno::Reference<container::XEnumerationAccess> xEnumerationAccess(xCursor, uno::UNO_QUERY);
        // Keep the character properties of the last but one paragraph, even if
        // it's empty. This works for headers/footers, and maybe in other cases
        // as well, but surely not in textboxes.
        // fdo#58327: also do this at the end of the document: when pasting,
        // a table before the cursor position would be deleted
        // (but only for paste/insert, not load; otherwise it can happen that
        // flys anchored at the disposed paragraph are deleted (fdo47036.rtf))
        bool const bEndOfDocument(m_aTextAppendStack.size() == 1);
        if ((m_bInHeaderFooterImport || (bEndOfDocument && !m_bIsNewDoc))
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
            if (xCursor->getString() == SAL_NEWLINE_STRING)
            {
                uno::Reference<beans::XPropertySet> xDocProps(GetTextDocument(), uno::UNO_QUERY);
                const OUString aRecordChanges("RecordChanges");
                uno::Any aPreviousValue(xDocProps->getPropertyValue(aRecordChanges));

                // disable redlining for this operation, otherwise we might
                // end up with an unwanted recorded deletion
                xDocProps->setPropertyValue(aRecordChanges, uno::Any(false));

                // delete
                xCursor->setString(OUString());

                // restore again
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

void DomainMapper_Impl::SetIsFirstParagraphInShape(bool bIsFirst)
{
    m_bIsFirstParaInShape = bIsFirst;
}

void DomainMapper_Impl::SetIsDummyParaAddedForTableInSection( bool bIsAdded )
{
    m_bDummyParaAddedForTableInSection = bIsAdded;
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
        m_pLastSectionContext = m_aPropertyStacks[eId].top( );
    }
    else if (eId == CONTEXT_CHARACTER)
    {
        m_pLastCharacterContext = m_aPropertyStacks[eId].top();
        // Sadly an assert about deferredCharacterProperties being empty is not possible
        // here, because appendTextPortion() may not be called for every character section.
        deferredCharacterProperties.clear();
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


uno::Reference< text::XTextAppend > const &  DomainMapper_Impl::GetTopTextAppend()
{
    OSL_ENSURE(!m_aTextAppendStack.empty(), "text append stack is empty" );
    return m_aTextAppendStack.top().xTextAppend;
}

FieldContextPtr const &  DomainMapper_Impl::GetTopFieldContext()
{
    SAL_WARN_IF(m_aFieldStack.empty(), "writerfilter.dmapper", "Field stack is empty");
    return m_aFieldStack.top();
}

void DomainMapper_Impl::InitTabStopFromStyle( const uno::Sequence< style::TabStop >& rInitTabStops )
{
    OSL_ENSURE(m_aCurrentTabStops.empty(), "tab stops already initialized");
    for( sal_Int32 nTab = 0; nTab < rInitTabStops.getLength(); ++nTab)
    {
        m_aCurrentTabStops.emplace_back(rInitTabStops[nTab] );
    }
}

void DomainMapper_Impl::IncorporateTabStop( const DeletableTabStop &  rTabStop )
{
    ::std::vector<DeletableTabStop>::iterator aIt = m_aCurrentTabStops.begin();
    ::std::vector<DeletableTabStop>::iterator aEndIt = m_aCurrentTabStops.end();
    sal_Int32 nConverted = rTabStop.Position;
    bool bFound = false;
    for( ; aIt != aEndIt; ++aIt)
    {
        if( aIt->Position == nConverted )
        {
            bFound = true;
            if( rTabStop.bDeleted )
                m_aCurrentTabStops.erase( aIt );
            else
                *aIt = rTabStop;
            break;
        }
    }
    if( !bFound )
        m_aCurrentTabStops.push_back( rTabStop );
}


uno::Sequence< style::TabStop > DomainMapper_Impl::GetCurrentTabStopAndClear()
{
    std::vector<style::TabStop> aRet;
    for (DeletableTabStop& rStop : m_aCurrentTabStops)
    {
        if (!rStop.bDeleted)
            aRet.push_back(rStop);
    }
    m_aCurrentTabStops.clear();
    return comphelper::containerToSequence(aRet);
}

const OUString DomainMapper_Impl::GetCurrentParaStyleName()
{
    // use saved currParaStyleName as a fallback, in case no particular para style name applied.
    OUString sName = m_sCurrentParaStyleName;
    PropertyMapPtr pParaContext = GetTopContextOfType(CONTEXT_PARAGRAPH);
    if ( pParaContext && pParaContext->isSet(PROP_PARA_STYLE_NAME) )
        pParaContext->getProperty(PROP_PARA_STYLE_NAME)->second >>= sName;

    return sName;
}

const OUString DomainMapper_Impl::GetDefaultParaStyleName()
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
            return OUString( "Standard");
    }
    return m_sDefaultParaStyleName;
}

/*-------------------------------------------------------------------------
    returns the value from the current paragraph style - if available
  -----------------------------------------------------------------------*/
uno::Any DomainMapper_Impl::GetPropertyFromStyleSheet(PropertyIds eId)
{
    StyleSheetEntryPtr pEntry;
    if( m_bInStyleSheetImport )
        pEntry = GetStyleSheetTable()->GetCurrentEntry();
    else
        pEntry = GetStyleSheetTable()->FindStyleSheetByConvertedStyleName(GetCurrentParaStyleName());
    while(pEntry.get( ) )
    {
        if(pEntry->pProperties)
        {
            boost::optional<PropertyMap::Property> aProperty =
                    pEntry->pProperties->getProperty(eId);
            if( aProperty )
            {
                return aProperty->second;
            }
        }
        //search until the property is set or no parent is available
        StyleSheetEntryPtr pNewEntry = GetStyleSheetTable()->FindStyleSheetByISTD(pEntry->sBaseStyleIdentifier);

        SAL_WARN_IF( pEntry == pNewEntry, "writerfilter.dmapper", "circular loop in style hierarchy?");

        if (pEntry == pNewEntry) //fdo#49587
            break;

        pEntry = pNewEntry;
    }
    // not found in style, try the document's DocDefault properties
    if ( m_bInStyleSheetImport )
    {
        const PropertyMapPtr& pDefaultParaProps = GetStyleSheetTable()->GetDefaultParaProps();
        if ( pDefaultParaProps )
        {
            boost::optional<PropertyMap::Property> aProperty = pDefaultParaProps->getProperty(eId);
            if ( aProperty )
                return aProperty->second;
        }
    }
    return uno::Any();
}

uno::Any DomainMapper_Impl::GetAnyProperty(PropertyIds eId, const PropertyMapPtr& rContext)
{
    if ( rContext )
    {
        boost::optional<PropertyMap::Property> aProperty = rContext->getProperty(eId);
        if ( aProperty )
            return aProperty->second;
    }
    return GetPropertyFromStyleSheet(eId);
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

bool DomainMapper_Impl::isSdtEndDeferred()
{
    return m_bSdtEndDeferred;
}

void DomainMapper_Impl::setParaSdtEndDeferred(bool bParaSdtEndDeferred)
{
    m_bParaSdtEndDeferred = bParaSdtEndDeferred;
}

bool DomainMapper_Impl::isParaSdtEndDeferred()
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

        for( sal_uInt32 nProperty = 0; nProperty < SAL_N_ELEMENTS( aBorderProperties ); ++nProperty)
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
        boost::optional<PropertyMap::Property> aParaStyle = pPropertyMap->getProperty(PROP_PARA_STYLE_NAME);
        if( aParaStyle )
        {
            OUString sName;
            aParaStyle->second >>= sName;
            pToBeSavedProperties->SetParaStyleName(sName);
        }
    }
}


//define some default frame width - 0cm ATM: this allow the frame to be wrapped around the text
#define DEFAULT_FRAME_MIN_WIDTH 0
#define DEFAULT_FRAME_MIN_HEIGHT 0
#define DEFAULT_VALUE 0

void DomainMapper_Impl::CheckUnregisteredFrameConversion( )
{
    if (m_aTextAppendStack.empty())
        return;
    TextAppendContext& rAppendContext = m_aTextAppendStack.top();
    // n#779642: ignore fly frame inside table as it could lead to messy situations
    if (!rAppendContext.pLastParagraphProperties.get())
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

        if ( pParaStyle.get( ) )
        {
            const ParagraphProperties* pStyleProperties = dynamic_cast<const ParagraphProperties*>( pParaStyle->pProperties.get() );
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
            aFrameProperties.push_back(comphelper::makePropertyValue(getPropertyName(PROP_VERT_ORIENT_RELATION), sal_Int16(
                rAppendContext.pLastParagraphProperties->GetvAnchor() >= 0 ?
                    rAppendContext.pLastParagraphProperties->GetvAnchor() :
                    pStyleProperties->GetvAnchor() >= 0 ? pStyleProperties->GetvAnchor() : text::RelOrientation::FRAME )));

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
            aFrameProperties );
    }
    catch( const uno::Exception& )
    {
    }
}

/// Check if the style or its parent has a list id, recursively.
static sal_Int32 lcl_getListId(const StyleSheetEntryPtr& rEntry, const StyleSheetTablePtr& rStyleTable)
{
    const StyleSheetPropertyMap* pEntryProperties = dynamic_cast<const StyleSheetPropertyMap*>(rEntry->pProperties.get());
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

    return lcl_getListId(pParent, rStyleTable);
}

void DomainMapper_Impl::finishParagraph( const PropertyMapPtr& pPropertyMap )
{
    if (m_bDiscardHeaderFooter)
        return;
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("finishParagraph");
#endif

    m_nLastTableCellParagraphDepth = m_nTableCellDepth;
    ParagraphPropertyMap* pParaContext = dynamic_cast< ParagraphPropertyMap* >( pPropertyMap.get() );
    if (m_aTextAppendStack.empty())
        return;
    TextAppendContext& rAppendContext = m_aTextAppendStack.top();
    uno::Reference< text::XTextAppend > xTextAppend(rAppendContext.xTextAppend);
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().attribute("isTextAppend", sal_uInt32(xTextAppend.is()));
#endif

    const StyleSheetEntryPtr pEntry = GetStyleSheetTable()->FindStyleSheetByConvertedStyleName( GetCurrentParaStyleName() );
    OSL_ENSURE( pEntry.get(), "no style sheet found" );
    const StyleSheetPropertyMap* pStyleSheetProperties = dynamic_cast<const StyleSheetPropertyMap*>(pEntry ? pEntry->pProperties.get() : nullptr);
    //apply numbering to paragraph if it was set at the style, but only if the paragraph itself
    //does not specify the numbering
    if ( pStyleSheetProperties && pParaContext && !pParaContext->isSet(PROP_NUMBERING_RULES) )
    {

        sal_Int32 nListId = pEntry ? lcl_getListId(pEntry, GetStyleSheetTable()) : -1;
        if ( nListId >= 0 )
        {
            pParaContext->Insert( PROP_NUMBERING_STYLE_NAME, uno::makeAny( ListDef::GetStyleName( nListId ) ), false);

            // Indent properties from the paragraph style have priority
            // over the ones from the numbering styles in Word
            // but in Writer numbering styles have priority,
            // so insert directly into the paragraph properties to compensate.
            boost::optional<PropertyMap::Property> oProperty;
            if ( (oProperty = pStyleSheetProperties->getProperty(PROP_PARA_FIRST_LINE_INDENT)) )
                pParaContext->Insert(PROP_PARA_FIRST_LINE_INDENT, oProperty->second, /*bOverwrite=*/false);
            if ( (oProperty = pStyleSheetProperties->getProperty(PROP_PARA_LEFT_MARGIN)) )
                pParaContext->Insert(PROP_PARA_LEFT_MARGIN, oProperty->second, /*bOverwrite=*/false);

            // We're inheriting properties from a numbering style. Make sure a possible right margin is inherited from the base style.
            sal_Int32 nParaRightMargin = 0;
            if (!pEntry->sBaseStyleIdentifier.isEmpty())
            {
                const StyleSheetEntryPtr pParent = GetStyleSheetTable()->FindStyleSheetByISTD(pEntry->sBaseStyleIdentifier);
                const StyleSheetPropertyMap* pParentProperties = dynamic_cast<const StyleSheetPropertyMap*>(pParent ? pParent->pProperties.get() : nullptr);
                boost::optional<PropertyMap::Property> pPropMargin;
                if (pParentProperties && (pPropMargin = pParentProperties->getProperty(PROP_PARA_RIGHT_MARGIN)) )
                    nParaRightMargin = pPropMargin->second.get<sal_Int32>();
            }
            if (nParaRightMargin != 0)
            {
                // If we're setting the right margin, we should set the first / left margin as well from the numbering style.
                const sal_Int32 nFirstLineIndent = getNumberingProperty(nListId, pStyleSheetProperties->GetListLevel(), "FirstLineIndent");
                const sal_Int32 nParaLeftMargin  = getNumberingProperty(nListId, pStyleSheetProperties->GetListLevel(), "IndentAt");
                if (nFirstLineIndent != 0)
                    pParaContext->Insert(PROP_PARA_FIRST_LINE_INDENT, uno::makeAny(nFirstLineIndent), /*bOverwrite=*/false);
                if (nParaLeftMargin != 0)
                    pParaContext->Insert(PROP_PARA_LEFT_MARGIN, uno::makeAny(nParaLeftMargin), /*bOverwrite=*/false);

                pParaContext->Insert(PROP_PARA_RIGHT_MARGIN, uno::makeAny(nParaRightMargin), /*bOverwrite=*/false);
            }
        }

        if ( pStyleSheetProperties->GetListLevel() >= 0 )
            pParaContext->Insert( PROP_NUMBERING_LEVEL, uno::makeAny(pStyleSheetProperties->GetListLevel()), false);
    }


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
            if( rAppendContext.pLastParagraphProperties.get() )
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
            if (pPropertyMap.get())
                aProperties = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(pPropertyMap->GetPropertyValues());
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
                    if (rAppendContext.pLastParagraphProperties.get())
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
                        OUString const sMarker("X");
                        xCursor = xTextAppend->getText()->createTextCursor();
                        if (xCursor.is())
                            xCursor->gotoEnd(false);
                        PropertyMapPtr pEmpty(new PropertyMap());
                        appendTextPortion(sMarker, pEmpty);
                    }

                    // Check if top / bottom margin has to be updated, now that we know the numbering status of both the previous and
                    // the current text node.
                    auto itNumberingRules = std::find_if(aProperties.begin(), aProperties.end(), [](const beans::PropertyValue& rValue)
                    {
                        return rValue.Name == "NumberingRules";
                    });
                    if (itNumberingRules != aProperties.end())
                    {
                        // This textnode has numbering. Look up the numbering style name of the current and previous paragraph.
                        OUString aCurrentNumberingRuleName;
                        uno::Reference<container::XNamed> xCurrentNumberingRules(itNumberingRules->Value, uno::UNO_QUERY);
                        if (xCurrentNumberingRules.is())
                            aCurrentNumberingRuleName = xCurrentNumberingRules->getName();
                        OUString aPreviousNumberingRuleName;
                        if (m_xPreviousParagraph.is())
                        {
                            uno::Reference<container::XNamed> xPreviousNumberingRules(m_xPreviousParagraph->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
                            if (xPreviousNumberingRules.is())
                                aPreviousNumberingRuleName = xPreviousNumberingRules->getName();
                        }

                        if (!aPreviousNumberingRuleName.isEmpty() && aCurrentNumberingRuleName == aPreviousNumberingRuleName)
                        {
                            // There was a previous textnode and it had the same numbering.
                            if (m_bParaAutoBefore)
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
                            uno::Sequence<beans::PropertyValue> aPrevPropertiesSeq;
                            m_xPreviousParagraph->getPropertyValue("ParaInteropGrabBag") >>= aPrevPropertiesSeq;
                            auto aPrevProperties = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(aPrevPropertiesSeq);
                            auto itPrevParaAutoAfter = std::find_if(aPrevProperties.begin(), aPrevProperties.end(), [](const beans::PropertyValue& rValue)
                            {
                                return rValue.Name == "ParaBottomMarginAfterAutoSpacing";
                            });
                            bool bPrevParaAutoAfter = itPrevParaAutoAfter != aPrevProperties.end();
                            if (bPrevParaAutoAfter)
                            {
                                // Previous after spacing is set to auto, set previous after space to 0.
                                m_xPreviousParagraph->setPropertyValue("ParaBottomMargin", uno::makeAny(static_cast<sal_Int32>(0)));
                            }
                        }
                    }

                    xTextRange = xTextAppend->finishParagraph( comphelper::containerToSequence(aProperties) );
                    m_xPreviousParagraph.set(xTextRange, uno::UNO_QUERY);

                    if (!rAppendContext.m_aAnchoredObjects.empty() && !IsInHeaderFooter())
                    {
                        // Remember what objects are anchored to this paragraph.
                        // That list is only used for Word compat purposes, and
                        // it is only relevant for body text.
                        AnchoredObjectInfo aInfo;
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
                    xCur->goLeft( 1 , true );
                    uno::Reference< text::XTextRange > xParaEnd( xCur, uno::UNO_QUERY );
                    CheckParaMarkerRedline( xParaEnd );
                }

                // set top margin of the previous auto paragraph in cells, keeping zero bottom margin only at the first one
                if (m_nTableDepth > 0 && m_nTableDepth == m_nTableCellDepth && m_xPreviousParagraph.is())
                {
                    bool bParaChangedTopMargin = false;
                    auto itParaTopMargin = std::find_if(aProperties.begin(), aProperties.end(), [](const beans::PropertyValue& rValue)
                    {
                        return rValue.Name == "ParaTopMargin";
                    });
                    if (itParaTopMargin != aProperties.end())
                        bParaChangedTopMargin = true;

                    uno::Sequence<beans::PropertyValue> aPrevPropertiesSeq;
                    m_xPreviousParagraph->getPropertyValue("ParaInteropGrabBag") >>= aPrevPropertiesSeq;
                    auto aPrevProperties = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(aPrevPropertiesSeq);
                    auto itPrevParaAutoBefore = std::find_if(aPrevProperties.begin(), aPrevProperties.end(), [](const beans::PropertyValue& rValue)
                    {
                        return rValue.Name == "ParaTopMarginBeforeAutoSpacing";
                    });
                    bool bPrevParaAutoBefore = itPrevParaAutoBefore != aPrevProperties.end();

                    if ((bPrevParaAutoBefore && !bParaChangedTopMargin) || (bParaChangedTopMargin && m_bParaAutoBefore))
                    {
                        sal_Int32 nSize = m_bFirstParagraphInCell ? 0 : 280;
                        // Previous before spacing is set to auto, set previous before space to 280, except in the first paragraph.
                        m_xPreviousParagraph->setPropertyValue("ParaTopMargin",
                                 uno::makeAny( ConversionHelper::convertTwipToMM100(nSize)));
                    }
                }

                // tdf#118521 set paragraph top or bottom margin based on the paragraph style
                // if we already set the other margin with direct formatting
                if (pStyleSheetProperties && pParaContext && m_xPreviousParagraph.is() &&
                        pParaContext->isSet(PROP_PARA_TOP_MARGIN) != pParaContext->isSet(PROP_PARA_BOTTOM_MARGIN))
                {
                    boost::optional<PropertyMap::Property> oProperty;
                    if (pParaContext->isSet(PROP_PARA_TOP_MARGIN))
                    {
                        if ( (oProperty = pStyleSheetProperties->getProperty(PROP_PARA_BOTTOM_MARGIN)) )
                            m_xPreviousParagraph->setPropertyValue("ParaBottomMargin", oProperty->second);
                    }
                    else
                    {
                        if ( (oProperty = pStyleSheetProperties->getProperty(PROP_PARA_TOP_MARGIN)) )
                            m_xPreviousParagraph->setPropertyValue("ParaTopMargin", oProperty->second);
                    }
                }

                // Left, Right, and Hanging settings are also grouped. Ensure that all or none are set.
                // m_xPreviousParagraph was set earlier, so really it still is the current paragraph...
                if ( pStyleSheetProperties && pParaContext && m_xPreviousParagraph.is() )
                {
                    const bool bLeftSet  = pParaContext->isSet(PROP_PARA_LEFT_MARGIN);
                    const bool bRightSet = pParaContext->isSet(PROP_PARA_RIGHT_MARGIN);
                    const bool bFirstSet = pParaContext->isSet(PROP_PARA_FIRST_LINE_INDENT);
                    if ( !(bLeftSet == bRightSet && bRightSet == bFirstSet) )
                    {
                        boost::optional<PropertyMap::Property> oProperty;
                        if ( !bLeftSet )
                        {
                            if ( (oProperty = pStyleSheetProperties->getProperty(PROP_PARA_LEFT_MARGIN)) )
                                m_xPreviousParagraph->setPropertyValue("ParaLeftMargin", oProperty->second);
                        }
                        if ( !bRightSet )
                        {
                            if ( (oProperty = pStyleSheetProperties->getProperty(PROP_PARA_RIGHT_MARGIN)) )
                                m_xPreviousParagraph->setPropertyValue("ParaRightMargin", oProperty->second);
                        }
                        if ( !bFirstSet )
                        {
                            if ( (oProperty = pStyleSheetProperties->getProperty(PROP_PARA_FIRST_LINE_INDENT)) )
                                m_xPreviousParagraph->setPropertyValue("ParaFirstLineIndent", oProperty->second);
                        }
                    }
                }
            }
            if( !bKeepLastParagraphProperties )
                rAppendContext.pLastParagraphProperties = pToBeSavedProperties;
        }
        catch(const lang::IllegalArgumentException&)
        {
            OSL_FAIL( "IllegalArgumentException in DomainMapper_Impl::finishParagraph" );
        }
        catch(const uno::Exception& e)
        {
            SAL_WARN( "writerfilter.dmapper", "finishParagraph() " << e );
        }

    }

    bool bIgnoreFrameState = IsInHeaderFooter();
    if( (!bIgnoreFrameState && pParaContext && pParaContext->IsFrameMode()) || (bIgnoreFrameState && GetIsPreviousParagraphFramed()) )
        SetIsPreviousParagraphFramed(true);
    else
        SetIsPreviousParagraphFramed(false);

    m_bParaChanged = false;
    if( !IsInHeaderFooter() && !IsInShape() && (!pParaContext || !pParaContext->IsFrameMode()) )
    { // If the paragraph is in a frame, shape or header/footer, it's not a paragraph of the section itself.
        SetIsFirstParagraphInSection(false);
        SetIsLastParagraphInSection(false);
    }

    if (m_bIsFirstParaInShape)
        m_bIsFirstParaInShape = false;

    // keep m_bParaAutoAfter for table paragraphs
    m_bPrevParaAutoAfter = m_bParaAutoAfter || m_bPrevParaAutoAfter;

    // not auto margin in this paragraph
    m_bParaChangedBottomMargin = (pParaContext && pParaContext->isSet(PROP_PARA_BOTTOM_MARGIN) && !m_bParaAutoAfter);

    if (pParaContext)
    {
        // Reset the frame properties for the next paragraph
        pParaContext->ResetFrameProperties();
    }

    SetIsOutsideAParagraph(true);
    m_bParaHadField = false;

    // don't overwrite m_bFirstParagraphInCell in table separator nodes
    if (m_nTableDepth > 0 && m_nTableDepth == m_nTableCellDepth)
        m_bFirstParagraphInCell = false;

    m_bParaAutoBefore = false;
    m_bParaAutoAfter = false;

#ifdef DEBUG_WRITERFILTER
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
    if (xTextAppend.is() && hasTableManager() && !getTableManager().isIgnore())
    {
        try
        {
            // If we are in comments, then disable CharGrabBag, comment text doesn't support that.
            uno::Sequence< beans::PropertyValue > aValues = pPropertyMap->GetPropertyValues(/*bCharGrabBag=*/!m_bIsInComments);
            sal_Int32 len = aValues.getLength();

            if (m_bStartTOC || m_bStartIndex || m_bStartBibliography)
                for( int i =0; i < len; ++i )
                {
                    if (aValues[i].Name == "CharHidden")
                        aValues[i].Value <<= false;
                }

            uno::Reference< text::XTextRange > xTextRange;
            if (m_aTextAppendStack.top().xInsertPosition.is())
            {
                xTextRange = xTextAppend->insertTextPortion(rString, aValues, m_aTextAppendStack.top().xInsertPosition);
                m_aTextAppendStack.top().xCursor->gotoRange(xTextRange->getEnd(), true);
            }
            else
            {
                if (m_bStartTOC || m_bStartIndex || m_bStartBibliography || m_bStartGenericField)
                {
                    if(m_bInHeaderFooterImport && !m_bStartTOCHeaderFooter)
                    {
                        xTextRange = xTextAppend->appendTextPortion(rString, aValues);
                    }
                    else
                    {
                        m_bStartedTOC = true;
                        uno::Reference< text::XTextCursor > xTOCTextCursor;
                        xTOCTextCursor = xTextAppend->getEnd()->getText( )->createTextCursor( );
                        xTOCTextCursor->gotoEnd(false);
                        if (xTOCTextCursor.is())
                        {
                            if (m_bStartIndex || m_bStartBibliography || m_bStartGenericField)
                                xTOCTextCursor->goLeft(1, false);
                            xTextRange = xTextAppend->insertTextPortion(rString, aValues, xTOCTextCursor);
                            SAL_WARN_IF(!xTextRange.is(), "writerfilter.dmapper", "insertTextPortion failed");
                            if (!xTextRange.is())
                                throw uno::Exception("insertTextPortion failed", nullptr);
                            m_bTextInserted = true;
                            xTOCTextCursor->gotoRange(xTextRange->getEnd(), true);
                            mxTOCTextCursor = xTOCTextCursor;
                        }
                        else
                        {
                            xTextRange = xTextAppend->appendTextPortion(rString, aValues);
                            xTOCTextCursor = xTextAppend->createTextCursor();
                            xTOCTextCursor->gotoRange(xTextRange->getEnd(), false);
                        }
                        m_aTextAppendStack.push(TextAppendContext(xTextAppend, xTOCTextCursor));
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

                    xTextRange = xTextAppend->appendTextPortion(rString, aValues);
                }
            }

            CheckRedline( xTextRange );
            m_bParaChanged = true;

            //getTableManager( ).handle(xTextRange);
        }
        catch(const lang::IllegalArgumentException&)
        {
            OSL_FAIL( "IllegalArgumentException in DomainMapper_Impl::appendTextPortion" );
        }
        catch(const uno::Exception&)
        {
            OSL_FAIL( "Exception in DomainMapper_Impl::appendTextPortion" );
        }
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
    if (xTextAppendAndConvert.is() && hasTableManager() && !getTableManager().isIgnore())
    {
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

        uno::Reference< graphic::XGraphic > xGraphic = pOLEHandler->getReplacement();
        xOLEProperties->setPropertyValue(getPropertyName( PROP_GRAPHIC ),
                        uno::makeAny(xGraphic));
        uno::Reference<beans::XPropertySet> xReplacementProperties(pOLEHandler->getShape(), uno::UNO_QUERY);
        if (xReplacementProperties.is())
        {
            OUString pProperties[] = {
                OUString("AnchorType"),
                OUString("Surround"),
                OUString("HoriOrient"),
                OUString("HoriOrientPosition"),
                OUString("VertOrient"),
                OUString("VertOrientPosition")
            };
            for (const OUString & s : pProperties)
                xOLEProperties->setPropertyValue(s, xReplacementProperties->getPropertyValue(s));
        }
        else
            // mimic the treatment of graphics here.. it seems anchoring as character
            // gives a better ( visually ) result
            xOLEProperties->setPropertyValue(getPropertyName( PROP_ANCHOR_TYPE ),  uno::makeAny( text::TextContentAnchorType_AS_CHARACTER ) );
        // remove ( if valid ) associated shape ( used for graphic replacement )
        SAL_WARN_IF(m_aAnchoredStack.empty(), "writerfilter.dmapper", "no anchor stack");
        if (!m_aAnchoredStack.empty())
            m_aAnchoredStack.top( ).bToRemove = true;
        RemoveLastParagraph();
        m_aTextAppendStack.pop();


        appendTextContent( xOLE, uno::Sequence< beans::PropertyValue >() );

        if (!aCLSID.isEmpty())
            pOLEHandler->importStream(m_xComponentContext, GetTextDocument(), xOLE);

    }
    catch( const uno::Exception& )
    {
        OSL_FAIL( "Exception in creation of OLE object" );
    }

}

void DomainMapper_Impl::appendStarMath( const Value& val )
{
    uno::Reference< embed::XEmbeddedObject > formula;
    val.getAny() >>= formula;
    if( formula.is() )
    {
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
            // mimic the treatment of graphics here.. it seems anchoring as character
            // gives a better ( visually ) result
            xStarMathProperties->setPropertyValue(getPropertyName( PROP_ANCHOR_TYPE ),
                uno::makeAny( text::TextContentAnchorType_AS_CHARACTER ) );
            appendTextContent( xStarMath, uno::Sequence< beans::PropertyValue >() );
        }
        catch( const uno::Exception& )
        {
            OSL_FAIL( "Exception in creation of StarMath object" );
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
            //the paragraph after this new section is already inserted
            xCursor->goLeft(1, true);
            uno::Reference< text::XTextContent > xSection( m_xTextFactory->createInstance("com.sun.star.text.TextSection"), uno::UNO_QUERY_THROW );
            xSection->attach( uno::Reference< text::XTextRange >( xCursor, uno::UNO_QUERY_THROW) );
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

void DomainMapper_Impl::PushPageHeaderFooter(bool bHeader, SectionPropertyMap::PageType eType)
{
    m_aHeaderFooterStack.push(HeaderFooterContext(m_bTextInserted));
    m_bTextInserted = false;

    const PropertyIds ePropIsOn = bHeader? PROP_HEADER_IS_ON: PROP_FOOTER_IS_ON;
    const PropertyIds ePropShared = bHeader? PROP_HEADER_IS_SHARED: PROP_FOOTER_IS_SHARED;
    const PropertyIds ePropTextLeft = bHeader? PROP_HEADER_TEXT_LEFT: PROP_FOOTER_TEXT_LEFT;
    const PropertyIds ePropText = bHeader? PROP_HEADER_TEXT: PROP_FOOTER_TEXT;

    m_bInHeaderFooterImport = true;

    //get the section context
    PropertyMapPtr pContext = DomainMapper_Impl::GetTopContextOfType(CONTEXT_SECTION);
    //ask for the header/footer name of the given type
    SectionPropertyMap* pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
    if(pSectionContext)
    {
        // clear the "Link To Previous" flag so that the header/footer
        // content is not copied from the previous section
        pSectionContext->ClearHeaderFooterLinkToPrevious(bHeader, eType);

        uno::Reference< beans::XPropertySet > xPageStyle =
            pSectionContext->GetPageStyle(
                GetPageStyles(),
                m_xTextFactory,
                eType == SectionPropertyMap::PAGE_FIRST );
        if (!xPageStyle.is())
            return;
        try
        {
            bool bLeft = eType == SectionPropertyMap::PAGE_LEFT;
            bool bFirst = eType == SectionPropertyMap::PAGE_FIRST;
            if ((!bLeft && !GetSettingsTable()->GetEvenAndOddHeaders()) || (GetSettingsTable()->GetEvenAndOddHeaders()))
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
                            m_bIsNewDoc? uno::Reference<text::XTextCursor>(): m_xBodyText->createTextCursorByRange(xText->getStart())));
            }
            else
            {
                m_bDiscardHeaderFooter = true;
            }
        }
        catch( const uno::Exception& )
        {
        }
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
    m_bInHeaderFooterImport = false;

    if (!m_aHeaderFooterStack.empty())
    {
        m_bTextInserted = m_aHeaderFooterStack.top().getTextInserted();
        m_aHeaderFooterStack.pop();
    }
}


void DomainMapper_Impl::PushFootOrEndnote( bool bIsFootnote )
{
    m_bInFootOrEndnote = true;
    m_bCheckFirstFootnoteTab = true;
    m_bSaveFirstParagraphInCell = m_bFirstParagraphInCell;
    try
    {
        // Redlines outside the footnote should not affect footnote content
        m_aRedlines.push(std::vector< RedlineParamsPtr >());

        PropertyMapPtr pTopContext = GetTopContext();
        uno::Reference< text::XText > xFootnoteText;
        if (GetTextFactory().is())
            xFootnoteText.set( GetTextFactory()->createInstance(
            bIsFootnote ?
                OUString( "com.sun.star.text.Footnote" ) : OUString( "com.sun.star.text.Endnote" )),
            uno::UNO_QUERY_THROW );
        uno::Reference< text::XFootnote > xFootnote( xFootnoteText, uno::UNO_QUERY_THROW );
        pTopContext->SetFootnote( xFootnote );
        uno::Sequence< beans::PropertyValue > aFontProperties = pTopContext->GetPropertyValues();
        appendTextContent( uno::Reference< text::XTextContent >( xFootnoteText, uno::UNO_QUERY_THROW ), aFontProperties );
        m_aTextAppendStack.push(TextAppendContext(uno::Reference< text::XTextAppend >( xFootnoteText, uno::UNO_QUERY_THROW ),
                    xFootnoteText->createTextCursorByRange(xFootnoteText->getStart())));

        // Redlines for the footnote anchor
        CheckRedline( xFootnote->getAnchor( ) );

    }
    catch( const uno::Exception& e )
    {
        SAL_WARN("writerfilter.dmapper", "exception in PushFootOrEndnote: " << e);
    }
}

void DomainMapper_Impl::CreateRedline(uno::Reference<text::XTextRange> const& xRange,
        const RedlineParamsPtr& pRedline)
{
    if ( pRedline.get( ) )
    {
        try
        {
            OUString sType;
            switch ( pRedline->m_nToken & 0xffff )
            {
            case XML_mod:
                sType = getPropertyName( PROP_FORMAT );
                break;
            case XML_moveTo:
            case XML_ins:
                sType = getPropertyName( PROP_INSERT );
                break;
            case XML_moveFrom:
            case XML_del:
                sType = getPropertyName( PROP_DELETE );
                break;
            case XML_ParagraphFormat:
                sType = getPropertyName( PROP_PARAGRAPH_FORMAT );
                break;
            default:
                throw lang::IllegalArgumentException("illegal redline token type", nullptr, 0);
            }
            uno::Reference < text::XRedline > xRedline( xRange, uno::UNO_QUERY_THROW );
            beans::PropertyValues aRedlineProperties( 3 );
            beans::PropertyValue * pRedlineProperties = aRedlineProperties.getArray(  );
            pRedlineProperties[0].Name = getPropertyName( PROP_REDLINE_AUTHOR );
            pRedlineProperties[0].Value <<= pRedline->m_sAuthor;
            pRedlineProperties[1].Name = getPropertyName( PROP_REDLINE_DATE_TIME );
            pRedlineProperties[1].Value <<= ConversionHelper::ConvertDateStringToDateTime( pRedline->m_sDate );
            pRedlineProperties[2].Name = getPropertyName( PROP_REDLINE_REVERT_PROPERTIES );
            pRedlineProperties[2].Value <<= pRedline->m_aRevertProperties;
            xRedline->makeRedline( sType, aRedlineProperties );
        }
        catch( const uno::Exception & )
        {
            OSL_FAIL( "Exception in makeRedline" );
        }
    }
}

void DomainMapper_Impl::CheckParaMarkerRedline( uno::Reference< text::XTextRange > const& xRange )
{
    if ( m_pParaMarkerRedline.get( ) )
    {
        CreateRedline( xRange, m_pParaMarkerRedline );
        if ( m_pParaMarkerRedline.get( ) )
        {
            m_pParaMarkerRedline.clear();
            m_currentRedline.clear();
        }
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
    if( GetTopContextOfType(CONTEXT_PARAGRAPH) )
    {
        std::vector<RedlineParamsPtr>& avRedLines = GetTopContextOfType(CONTEXT_PARAGRAPH)->Redlines();
        for( std::vector<RedlineParamsPtr>::const_iterator it = avRedLines.begin();
             it != avRedLines.end(); ++it )
            CreateRedline( xRange, *it );
    }
    if( GetTopContextOfType(CONTEXT_CHARACTER) )
    {
        std::vector<RedlineParamsPtr>& avRedLines = GetTopContextOfType(CONTEXT_CHARACTER)->Redlines();
        for( std::vector<RedlineParamsPtr>::const_iterator it = avRedLines.begin();
             it != avRedLines.end(); ++it )
            CreateRedline( xRange, *it );
    }
    std::vector<RedlineParamsPtr>::iterator pIt = m_aRedlines.top().begin( );
    for (; pIt != m_aRedlines.top().end( ); ++pIt )
        CreateRedline( xRange, *pIt );
}

void DomainMapper_Impl::StartParaMarkerChange( )
{
    m_bIsParaMarkerChange = true;
}

void DomainMapper_Impl::EndParaMarkerChange( )
{
    m_bIsParaMarkerChange = false;
    m_currentRedline.clear();
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


void DomainMapper_Impl::PopFootOrEndnote()
{
    if (!IsRTFImport())
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
    m_bSeenFootOrEndnoteSeparator = false;
    m_bInFootOrEndnote = false;
    m_bFirstParagraphInCell = m_bSaveFirstParagraphInCell;
}

void DomainMapper_Impl::SeenFootOrEndnoteSeparator()
{
    if (!m_bSeenFootOrEndnoteSeparator)
    {
        m_bSeenFootOrEndnoteSeparator = true;
        m_bIgnoreNextPara = true;
    }
}

void DomainMapper_Impl::PopAnnotation()
{
    RemoveLastParagraph();

    m_bIsInComments = false;
    m_aTextAppendStack.pop();

    try
    {
        // See if the annotation will be a single position or a range.
        if (m_nAnnotationId == -1 || !m_aAnnotationPositions[m_nAnnotationId].m_xStart.is() || !m_aAnnotationPositions[m_nAnnotationId].m_xEnd.is())
        {
            uno::Sequence< beans::PropertyValue > aEmptyProperties;
            appendTextContent( uno::Reference< text::XTextContent >( m_xAnnotationField, uno::UNO_QUERY_THROW ), aEmptyProperties );
        }
        else
        {
            AnnotationPosition& aAnnotationPosition = m_aAnnotationPositions[m_nAnnotationId];
            // Create a range that points to the annotation start/end.
            uno::Reference<text::XText> const xText = aAnnotationPosition.m_xStart->getText();
            uno::Reference<text::XTextCursor> const xCursor = xText->createTextCursorByRange(aAnnotationPosition.m_xStart);
            xCursor->gotoRange(aAnnotationPosition.m_xEnd, true);
            uno::Reference<text::XTextRange> const xTextRange(xCursor, uno::UNO_QUERY_THROW);

            // Attach the annotation to the range.
            uno::Reference<text::XTextAppend> const xTextAppend = m_aTextAppendStack.top().xTextAppend;
            xTextAppend->insertTextContent(xTextRange, uno::Reference<text::XTextContent>(m_xAnnotationField, uno::UNO_QUERY_THROW), !xCursor->isCollapsed());
        }
        m_aAnnotationPositions.erase( m_nAnnotationId );
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("writerfilter.dmapper",
                "Cannot insert annotation field: " << e);
    }

    m_xAnnotationField.clear();
    m_nAnnotationId = -1;
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
#ifdef DEBUG_WRITERFILTER
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
                bool checkBtLrStatus = false;

                for (int i = 0; i < aGrabBag.getLength(); ++i)
                {
                    if (aGrabBag[i].Name == "mso-layout-flow-alt")
                    {
                        m_bFrameBtLr = aGrabBag[i].Value.get<OUString>() == "bottom-to-top";
                        checkBtLrStatus = true;
                    }
                    if (aGrabBag[i].Name == "VML-Z-ORDER")
                    {
                        GraphicZOrderHelper* pZOrderHelper = m_rDMapper.graphicZOrderHelper();
                        sal_Int32 zOrder(0);
                        aGrabBag[i].Value >>= zOrder;
                        xShapePropertySet->setPropertyValue( "ZOrder", uno::makeAny(pZOrderHelper->findZOrder(zOrder)));
                        pZOrderHelper->addItem(xShapePropertySet, zOrder);
                        xShapePropertySet->setPropertyValue(getPropertyName( PROP_OPAQUE ), uno::makeAny( zOrder >= 0 ) );
                        checkZOrderStatus = true;
                    }
                    if(checkBtLrStatus && checkZOrderStatus)
                        break;

                    if ( aGrabBag[i].Name == "TxbxHasLink" )
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
                boost::optional<PropertyMap::Property> aPropMargin = paragraphContext->getProperty(PROP_PARA_BOTTOM_MARGIN);
                if(aPropMargin)
                    xProps->setPropertyValue( getPropertyName( PROP_BOTTOM_MARGIN ), aPropMargin->second );
            }
            else
            {
                uno::Reference<beans::XPropertySet> xShapePropertySet(xShape, uno::UNO_QUERY);
                uno::Sequence<beans::PropertyValue> aGrabBag;
                xShapePropertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
                for (int i = 0; i < aGrabBag.getLength(); ++i)
                {
                    if (aGrabBag[i].Name == "VML-Z-ORDER")
                    {
                        GraphicZOrderHelper* pZOrderHelper = m_rDMapper.graphicZOrderHelper();
                        sal_Int32 zOrder(0);
                        aGrabBag[i].Value >>= zOrder;
                        xShapePropertySet->setPropertyValue( "ZOrder", uno::makeAny(pZOrderHelper->findZOrder(zOrder)));
                        pZOrderHelper->addItem(xShapePropertySet, zOrder);
                        xShapePropertySet->setPropertyValue(getPropertyName( PROP_OPAQUE ), uno::makeAny( zOrder >= 0 ) );
                        checkZOrderStatus = true;
                    }
                    else if ( aGrabBag[i].Name == "TxbxHasLink" )
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
            if (!m_bInHeaderFooterImport && !checkZOrderStatus)
                xProps->setPropertyValue(
                        getPropertyName( PROP_OPAQUE ),
                        uno::makeAny( true ) );
        }
        m_bParaChanged = true;
        getTableManager().setIsInShape(true);
    }
    catch ( const uno::Exception& e )
    {
        SAL_WARN("writerfilter.dmapper", "Exception when adding shape: " << e);
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
    if ( m_aAnchoredStack.size() > 0 )
    {
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
        if( xShape->getSize().Width <= 2 )
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
    m_bFrameBtLr = false;
}

bool DomainMapper_Impl::IsSdtEndBefore()
{
    bool bIsSdtEndBefore = false;
    PropertyMapPtr pContext = GetTopContextOfType(CONTEXT_CHARACTER);
    if(pContext)
    {
        uno::Sequence< beans::PropertyValue > currentCharProps = pContext->GetPropertyValues();
        for (int i =0; i< currentCharProps.getLength(); i++)
        {
            if (currentCharProps[i].Name == "CharInteropGrabBag")
            {
                uno::Sequence<beans::PropertyValue> aCharGrabBag;
                currentCharProps[i].Value >>= aCharGrabBag;
                for (int j=0; j < aCharGrabBag.getLength();j++)
                {
                    if(aCharGrabBag[j].Name == "SdtEndBefore")
                    {
                        aCharGrabBag[j].Value >>= bIsSdtEndBefore;
                    }
                }
            }
        }
    }
    return bIsSdtEndBefore;
}

bool DomainMapper_Impl::IsDiscardHeaderFooter()
{
    return m_bDiscardHeaderFooter;
}

// called from TableManager::closeCell()
void DomainMapper_Impl::ClearPreviousParagraph()
{
    // in table cells, set bottom auto margin of last paragraph to 0, except in paragraphs with numbering
    if ((m_nTableDepth == (m_nTableCellDepth + 1)) && m_xPreviousParagraph.is() && !m_bParaChangedBottomMargin)
    {
        uno::Sequence<beans::PropertyValue> aPrevPropertiesSeq;
        m_xPreviousParagraph->getPropertyValue("ParaInteropGrabBag") >>= aPrevPropertiesSeq;
        auto aPrevProperties = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(aPrevPropertiesSeq);
        auto itPrevParaAutoAfter = std::find_if(aPrevProperties.begin(), aPrevProperties.end(), [](const beans::PropertyValue& rValue)
        {
            return rValue.Name == "ParaBottomMarginAfterAutoSpacing";
        });
        bool bPrevParaAutoAfter = itPrevParaAutoAfter != aPrevProperties.end();

        bool bPrevNumberingRules = false;
        uno::Reference<container::XNamed> xPreviousNumberingRules(m_xPreviousParagraph->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
        if (xPreviousNumberingRules.is())
             bPrevNumberingRules = !xPreviousNumberingRules->getName().isEmpty();
        if (!bPrevNumberingRules && (bPrevParaAutoAfter || m_bPrevParaAutoAfter))
            m_xPreviousParagraph->setPropertyValue("ParaBottomMargin", uno::makeAny(static_cast<sal_Int32>(0)));
    }

    m_xPreviousParagraph.clear();

    // next table paragraph will be first paragraph in a cell
    m_bFirstParagraphInCell = true;
}

static sal_Int16 lcl_ParseNumberingType( const OUString& rCommand )
{
    sal_Int16 nRet = style::NumberingType::PAGE_DESCRIPTOR;

    //  The command looks like: " PAGE \* Arabic "
    OUString sNumber = msfilter::util::findQuotedText(rCommand, "\\* ", ' ');

    if( !sNumber.isEmpty() )
    {
        //todo: might make sense to hash this list, too
        struct NumberingPairs
        {
            const sal_Char* cWordName;
            sal_Int16       nType;
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
    //  The command looks like: " DATE \@"dd MMMM yyyy"
    //  Remove whitespace permitted by standard between \@ and "
    sal_Int32 delimPos = rCommand.indexOf("\\@");
    sal_Int32 wsChars = rCommand.indexOf('\"') - delimPos - 2;
    OUString command = rCommand.replaceAt(delimPos+2, wsChars, "");

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
                    return OUString("FORMULA");
                }
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

static OUString lcl_ExctractVariableAndHint( const OUString& rCommand, OUString& rHint )
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

    nIndex = 0;
    sShortCommand = sShortCommand.getToken( 0, '\\', nIndex);
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
    OUString sSearch = "\\" + OUStringLiteral1( cSwitch );
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


void DomainMapper_Impl::GetCurrentLocale(lang::Locale& rLocale)
{
    PropertyMapPtr pTopContext = GetTopContext();
    boost::optional<PropertyMap::Property> pLocale = pTopContext->getProperty(PROP_CHAR_LOCALE);
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
    for (int i = 0; i < grabBag.getLength(); ++i)
    {
        if (grabBag[i].Name == name )
            return grabBag[i].Value ;
    }
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
        TextFramesForChaining(): xShape(nullptr), nId(0), nSeq(0), bShapeNameSet(false) {}
    } ;
    typedef std::map <OUString, TextFramesForChaining> ChainMap;

    try
    {
        ChainMap aTextFramesForChainingHelper;
        OUString sChainNextName("ChainNextName");

        //learn about ALL of the textboxes and their chaining values first - because frames are processed in no specific order.
        std::vector<uno::Reference< drawing::XShape > >::iterator iter;
        for( iter = m_vTextFramesForChaining.begin(); iter != m_vTextFramesForChaining.end(); ++iter )
        {
            uno::Reference<text::XTextContent>  xTextContent(*iter, uno::UNO_QUERY_THROW);
            uno::Reference<beans::XPropertySet> xPropertySet(xTextContent, uno::UNO_QUERY);
            uno::Reference<beans::XPropertySetInfo> xPropertySetInfo;
            if( xPropertySet.is() )
                xPropertySetInfo = xPropertySet->getPropertySetInfo();
            uno::Sequence<beans::PropertyValue> aGrabBag;
            uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);

            TextFramesForChaining aChainStruct = TextFramesForChaining();
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
                aChainStruct.xShape = *iter;
                aTextFramesForChainingHelper[sLinkChainName] = aChainStruct;
            }
        }

        //if mso-next-textbox tags are provided, create those vml-style links first. Afterwards we will make dml-style id/seq links.
        for (ChainMap::iterator msoIter= aTextFramesForChainingHelper.begin(); msoIter != aTextFramesForChainingHelper.end(); ++msoIter)
        {
            //if no mso-next-textbox, we are done.
            //if it points to itself, we are done.
            if( !msoIter->second.s_mso_next_textbox.isEmpty()
                && msoIter->second.s_mso_next_textbox != msoIter->first )
            {
                ChainMap::iterator nextFinder=aTextFramesForChainingHelper.find(msoIter->second.s_mso_next_textbox);
                if( nextFinder != aTextFramesForChainingHelper.end() )
                {
                    //if the frames have no name yet, then set them.  LinkDisplayName / ChainName are read-only.
                    if( !msoIter->second.bShapeNameSet )
                    {
                        uno::Reference< container::XNamed > xNamed( msoIter->second.xShape, uno::UNO_QUERY );
                        if ( xNamed.is() )
                        {
                            xNamed->setName( msoIter->first );
                            msoIter->second.bShapeNameSet = true;
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

                    uno::Reference<text::XTextContent>  xTextContent(msoIter->second.xShape, uno::UNO_QUERY_THROW);
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
        for (ChainMap::iterator outer_iter= aTextFramesForChainingHelper.begin(); outer_iter != aTextFramesForChainingHelper.end(); ++outer_iter)
        {
            if( outer_iter->second.s_mso_next_textbox.isEmpty() )  //non-empty ones already handled earlier - so skipping them now.
            {
                for (ChainMap::iterator inner_iter=aTextFramesForChainingHelper.begin(); inner_iter != aTextFramesForChainingHelper.end(); ++inner_iter)
                {
                    if ( inner_iter->second.nId == outer_iter->second.nId )
                    {
                        if (  inner_iter->second.nSeq == ( outer_iter->second.nSeq + nDirection ) )
                        {
                            uno::Reference<text::XTextContent>  xTextContent(outer_iter->second.xShape, uno::UNO_QUERY_THROW);
                            uno::Reference<beans::XPropertySet> xPropertySet(xTextContent, uno::UNO_QUERY);

                            //The reverse chaining happens automatically, so only one direction needs to be set
                            xPropertySet->setPropertyValue(sChainNextName, uno::makeAny(inner_iter->first));
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

uno::Reference<beans::XPropertySet> DomainMapper_Impl::FindOrCreateFieldMaster(const sal_Char* pFieldMasterService, const OUString& rFieldMasterName)
{
    // query master, create if not available
    uno::Reference< text::XTextFieldsSupplier > xFieldsSupplier( GetTextDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xFieldMasterAccess = xFieldsSupplier->getTextFieldMasters();
    uno::Reference< beans::XPropertySet > xMaster;
    OUString sFieldMasterService( OUString::createFromAscii(pFieldMasterService) );
    OUStringBuffer aFieldMasterName;
    aFieldMasterName.appendAscii( pFieldMasterService );
    aFieldMasterName.append('.');
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
        //set the master's name
        xMaster->setPropertyValue(
                    getPropertyName(PROP_NAME),
                    uno::makeAny(rFieldMasterName));
    }
    return xMaster;
}

void DomainMapper_Impl::PushFieldContext()
{
    m_bParaHadField = true;
    if(m_bDiscardHeaderFooter)
        return;
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().element("pushFieldContext");
#endif

    uno::Reference< text::XTextAppend >  xTextAppend;
    if (!m_aTextAppendStack.empty())
        xTextAppend = m_aTextAppendStack.top().xTextAppend;
    uno::Reference< text::XTextRange > xStart;
    if (xTextAppend.is())
    {
        uno::Reference< text::XTextCursor > xCrsr = xTextAppend->createTextCursorByRange( xTextAppend->getEnd() );
        xStart = xCrsr->getStart();
    }
    m_aFieldStack.push( new FieldContext( xStart ) );
}
/*-------------------------------------------------------------------------
//the current field context waits for the completion of the command
  -----------------------------------------------------------------------*/
bool DomainMapper_Impl::IsOpenFieldCommand() const
{
    return !m_aFieldStack.empty() && !m_aFieldStack.top()->IsCommandCompleted();
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
        m_aFieldStack.top()->SetFieldLocked();
}

HeaderFooterContext::HeaderFooterContext(bool bTextInserted)
    : m_bTextInserted(bTextInserted)
{
}

bool HeaderFooterContext::getTextInserted()
{
    return m_bTextInserted;
}

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


void FieldContext::AppendCommand(const OUString& rPart)
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
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("appendFieldCommand");
    TagLogger::getInstance().chars(rPartOfCommand);
    TagLogger::getInstance().endElement();
#endif

    FieldContextPtr pContext = m_aFieldStack.top();
    OSL_ENSURE( pContext.get(), "no field context available");
    if( pContext.get() )
    {
        pContext->AppendCommand( rPartOfCommand );
    }
}


typedef std::multimap < sal_Int32, OUString > TOCStyleMap;

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
//      {"GOTOBUTTON",      {"",                        FIELD_GOTOBUTTON    }},
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

    sVariable = lcl_ExctractVariableAndHint(pContext->GetCommand(), sHint);

    // remove surrounding "" if exists
    if( sHint.getLength() >= 2 && sHint.startsWith("\"") )
    {
        sHint = sHint.trim().copy(1, sHint.getLength() - 2);
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

    sVariable = lcl_ExctractVariableAndHint( pContext->GetCommand(),
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

void DomainMapper_Impl::handleFieldFormula
    (const FieldContextPtr& pContext,
     uno::Reference< beans::XPropertySet > const& xFieldProperties)
{
    OUString command = pContext->GetCommand().trim();
    // command must contains = and at least another char
    if (command.getLength() < 2)
        return;

    // we don't copy the = symbol from the command
    OUString formula = command.copy(1);

    xFieldProperties->setPropertyValue(getPropertyName(PROP_CONTENT), uno::makeAny(formula));
    xFieldProperties->setPropertyValue(getPropertyName(PROP_NUMBER_FORMAT), uno::makeAny(sal_Int32(0)));
    xFieldProperties->setPropertyValue("IsShowFormula", uno::makeAny(false));
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
    if (nIndex == -1 || (nIndex = rCommand.indexOf('(', nIndex)) == -1 || (nEnd = rCommand.lastIndexOf(')'))==-1 || nEnd <= nIndex)
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
    if (m_pLastCharacterContext.get())
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
    (OUString const& rFirstParam,
     uno::Reference< beans::XPropertySet > const& xFieldProperties,
     FieldId  eFieldId )
{
    if ( eFieldId != FIELD_USERINITIALS )
        xFieldProperties->setPropertyValue
            ( getPropertyName(PROP_FULL_NAME), uno::makeAny( true ));

    if (!rFirstParam.isEmpty())
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

    #define SET_ARABIC      0x01
    #define SET_DATE        0x04
    struct DocPropertyMap
    {
        const sal_Char* pDocPropertyName;
        const sal_Char* pServiceName;
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
    sal_uInt16 nMap = 0;
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
    uno::Reference<beans::XPropertySet> xFieldProperties =
        uno::Reference< beans::XPropertySet >( xFieldInterface,
            uno::UNO_QUERY_THROW);
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


#undef SET_ARABIC
#undef SET_DATE
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
        beans::PropertyValues aHyperlink(1);
        aHyperlink[0].Name = getPropertyName( PROP_TOKEN_TYPE );
        aHyperlink[0].Value <<= getPropertyName( PROP_TOKEN_HYPERLINK_START );
        pNewLevel[0] = aHyperlink;
        aHyperlink[0].Value <<= getPropertyName( PROP_TOKEN_HYPERLINK_END );
        pNewLevel[aNewLevel.getLength() -1] = aHyperlink;
    }
    if( bChapterNoSeparator )
    {
        beans::PropertyValues aChapterNo(2);
        aChapterNo[0].Name = getPropertyName( PROP_TOKEN_TYPE );
        aChapterNo[0].Value <<= getPropertyName( PROP_TOKEN_CHAPTER_INFO );
        aChapterNo[1].Name = getPropertyName( PROP_CHAPTER_FORMAT );
        //todo: is ChapterFormat::Number correct?
        aChapterNo[1].Value <<= sal_Int16(text::ChapterFormat::NUMBER);
        pNewLevel[aNewLevel.getLength() - (bHyperlinks ? 4 : 2) ] = aChapterNo;

        beans::PropertyValues aChapterSeparator(2);
        aChapterSeparator[0].Name = getPropertyName( PROP_TOKEN_TYPE );
        aChapterSeparator[0].Value <<= getPropertyName( PROP_TOKEN_TEXT );
        aChapterSeparator[1].Name = getPropertyName( PROP_TEXT );
        aChapterSeparator[1].Value <<= sChapterNoSeparator;
        pNewLevel[aNewLevel.getLength() - (bHyperlinks ? 3 : 1)] = aChapterSeparator;
    }
    //copy the 'old' entries except the last (page no)
    for( sal_Int32 nToken = 0; nToken < aLevel.getLength() - 1; ++nToken)
    {
        pNewLevel[nToken + 1] = aLevel[nToken];
    }
    //copy page no entry (last or last but one depending on bHyperlinks
    sal_Int32 nPageNo = aNewLevel.getLength() - (bHyperlinks ? 2 : 3);
    pNewLevel[nPageNo] = aLevel[aLevel.getLength() - 1];

    return aNewLevel;
}

void DomainMapper_Impl::handleToc
    (const FieldContextPtr& pContext,
    const OUString & sTOCServiceName)
{
    OUString sValue;
    m_bStartTOC = true;
    if(m_bInHeaderFooterImport)
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
        sal_Int32 nPos = 0;
        OUString sToken = sValue.getToken( 1, '"', nPos);
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


    if (m_xTextFactory.is())
        xTOC.set(
                m_xTextFactory->createInstance
                ( bTableOfFigures ?
                  "com.sun.star.text.IllustrationsIndex"
                  : sTOCServiceName),
                uno::UNO_QUERY_THROW);
    if (xTOC.is())
        xTOC->setPropertyValue(getPropertyName( PROP_TITLE ), uno::makeAny(OUString()));
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
                    for ( sal_Int32 nStyle = 0; nStyle < nLevelCount; ++nStyle, ++aTOCStyleIter )
                    {
                        aStyles[nStyle] = aTOCStyleIter->second;
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

    if (m_aTextAppendStack.empty())
        return;

    OUString const sMarker("Y");
    //insert index
    uno::Reference< text::XTextContent > xToInsert( xTOC, uno::UNO_QUERY );
    uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
    if (xTextAppend.is())
    {
        uno::Reference< text::XTextCursor > xCrsr = xTextAppend->getText()->createTextCursor();

        uno::Reference< text::XText > xText = xTextAppend->getText();
        if(xCrsr.is() && xText.is())
        {
            xCrsr->gotoEnd(false);
            xText->insertString(xCrsr, sMarker, false);
            xText->insertTextContent(uno::Reference< text::XTextRange >( xCrsr, uno::UNO_QUERY_THROW ), xToInsert, false);
            xTOCMarkerCursor = xCrsr;
        }
    }
}

void DomainMapper_Impl::handleBibliography
    (const FieldContextPtr& pContext,
    const OUString & sTOCServiceName)
{
    uno::Reference< beans::XPropertySet > xTOC;
    m_bStartTOC = true;
    m_bStartBibliography = true;
    if (m_xTextFactory.is())
        xTOC.set(
                m_xTextFactory->createInstance(
                sTOCServiceName),
                uno::UNO_QUERY_THROW);
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
    uno::Reference< beans::XPropertySet > xTOC;
    m_bStartTOC = true;
    m_bStartIndex = true;
    OUString sValue;
    OUString sIndexEntryType = "I"; // Default value for field flag '\f' is 'I'.


    if (m_xTextFactory.is())
        xTOC.set(
                m_xTextFactory->createInstance(
                sTOCServiceName),
                uno::UNO_QUERY_THROW);
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
        if( lcl_FindInCommand( pContext->GetCommand(), 'f', sValue ))
        {
            if(!sValue.isEmpty())
                sIndexEntryType = sValue ;
            xTOC->setPropertyValue(getPropertyName( PROP_INDEX_ENTRY_TYPE ), uno::makeAny(sIndexEntryType));
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

void DomainMapper_Impl::CloseFieldCommand()
{
    if(m_bDiscardHeaderFooter)
        return;
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().element("closeFieldCommand");
#endif

    FieldContextPtr pContext;
    if(!m_aFieldStack.empty())
        pContext = m_aFieldStack.top();
    OSL_ENSURE( pContext.get(), "no field context available");
    if( pContext.get() )
    {
        m_bSetUserFieldContent = false;
        m_bSetCitation = false;
        m_bSetDateValue = false;
        const FieldConversionMap_t& aFieldConversionMap = lcl_GetFieldConversion();

        try
        {
            uno::Reference< uno::XInterface > xFieldInterface;

            std::tuple<OUString, std::vector<OUString>, std::vector<OUString> > const
                field(splitFieldCommand(pContext->GetCommand()));
            OUString const sFirstParam(std::get<1>(field).empty()
                    ? OUString() : std::get<1>(field).front());

            FieldConversionMap_t::const_iterator const aIt =
                aFieldConversionMap.find(std::get<0>(field));
            if(aIt != aFieldConversionMap.end())
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
                    break;
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
                            aEnhancedFieldConversionMap.find(std::get<0>(field));
                        if ( aEnhancedIt != aEnhancedFieldConversionMap.end())
                            sServiceName += OUString::createFromAscii(aEnhancedIt->second.cFieldServiceName );
                    }
                    else
                    {
                        sServiceName += "TextField.";
                        sServiceName += OUString::createFromAscii(aIt->second.cFieldServiceName );
                    }

#ifdef DEBUG_WRITERFILTER
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
                        //create a user field and type
                        uno::Reference< beans::XPropertySet > xMaster =
                            FindOrCreateFieldMaster("com.sun.star.text.FieldMaster.User", sFirstParam);
                        uno::Reference< text::XDependentTextField > xDependentField( xFieldInterface, uno::UNO_QUERY_THROW );
                        xDependentField->attachTextFieldMaster( xMaster );
                        m_bSetUserFieldContent = true;
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
                                        if ((GetPropertyFromStyleSheet(PROP_CHAR_HEIGHT) >>= dHeight) && dHeight != 0)
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
                    {
                        sal_Int32 nIndex = 0;
                        if (xFieldProperties.is())
                            xFieldProperties->setPropertyValue(
                                    getPropertyName(PROP_HINT), uno::makeAny( pContext->GetCommand().getToken( 1, '\"', nIndex)));
                    }
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
                        handleFieldFormula(pContext, xFieldProperties);
                    break;
                    case FIELD_FORMCHECKBOX :
                    case FIELD_FORMDROPDOWN :
                    case FIELD_FORMTEXT :
                        {
                            uno::Reference< text::XTextField > xTextField( xFieldInterface, uno::UNO_QUERY );
                            if ( !xTextField.is() )
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
                            // Try to make absolute any relative URLs, except
                            // for relative same-document URLs that only contain
                            // a fragment part:
                            if (!sURL.startsWith("#") && !m_aSaveOpt.IsSaveRelFSys()) {
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

                        rtl::OUString sFormula = sSeqName + "+1";
                        rtl::OUString sValue;
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
                            uno::Reference< text::XTextCursor > xCrsr = xTextAppend->getText()->createTextCursor();
                            uno::Reference< text::XText > xText = xTextAppend->getText();
                            if(xCrsr.is() && xText.is())
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

                        uno::Reference< beans::XPropertySet > xTC(
                                m_xTextFactory->createInstance(
                                        OUString::createFromAscii(aIt->second.cFieldServiceName)),
                                        uno::UNO_QUERY_THROW);
                        if (!sFirstParam.isEmpty())
                        {
                            xTC->setPropertyValue("PrimaryKey",
                                    uno::makeAny(sFirstParam));
                        }
                        uno::Reference< text::XTextContent > xToInsert( xTC, uno::UNO_QUERY );
                        uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
                        if (xTextAppend.is())
                        {
                            uno::Reference< text::XTextCursor > xCrsr = xTextAppend->getText()->createTextCursor();

                            uno::Reference< text::XText > xText = xTextAppend->getText();
                            if(xCrsr.is() && xText.is())
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
                        OUString sCmd(pContext->GetCommand());//sCmd is the entire instrText inclusing the index e.g. CITATION Kra06 \l 1033
                        if( !sCmd.isEmpty()){
                            uno::Sequence<beans::PropertyValue> aValues( comphelper::InitPropertySequence({
                                { "Identifier", uno::Any(sCmd) }
                            }));
                            xTC->setPropertyValue("Fields", uno::makeAny(aValues));
                        }
                        uno::Reference< text::XTextContent > xToInsert( xTC, uno::UNO_QUERY );

                        uno::Sequence<beans::PropertyValue> aValues = m_aFieldStack.top()->getProperties()->GetPropertyValues();
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
            }
            else
            {
                /* Unsupported fields will be handled here for docx file.
                 * To handle unsupported fields used fieldmark API.
                 */
                OUString aCode( pContext->GetCommand().trim() );
                // Don't waste resources on wrapping shapes inside a fieldmark.
                if (aCode != "SHAPE" && m_xTextFactory.is() && !m_aTextAppendStack.empty())
                {
                    xFieldInterface = m_xTextFactory->createInstance("com.sun.star.text.Fieldmark");
                    const uno::Reference<text::XTextContent> xTextContent(xFieldInterface, uno::UNO_QUERY_THROW);
                    uno::Reference< text::XTextAppend >  xTextAppend;
                    xTextAppend = m_aTextAppendStack.top().xTextAppend;
                    uno::Reference< text::XTextCursor > xCrsr = xTextAppend->createTextCursorByRange(pContext->GetStartRange());
                    if (xTextContent.is())
                    {
                        xTextAppend->insertTextContent(xCrsr,xTextContent, true);
                    }
                    uno::Reference<uno::XInterface> xContent(xTextContent);
                    uno::Reference< text::XFormField> xFormField(xContent, uno::UNO_QUERY);
                    xFormField->setFieldType(aCode);
                    m_bStartGenericField = true;
                    pContext->SetFormField( xFormField );
                }
                else
                    m_bParaHadField = false;
            }
            //set the text field if there is any
            pContext->SetTextField( uno::Reference< text::XTextField >( xFieldInterface, uno::UNO_QUERY ) );
        }
        catch( const uno::Exception& e )
        {
            SAL_WARN( "writerfilter.dmapper", "Exception in CloseFieldCommand(): " << e );
        }
        pContext->SetCommandCompleted();
    }
}
/*-------------------------------------------------------------------------
//the _current_ fields require a string type result while TOCs accept richt results
  -----------------------------------------------------------------------*/
bool DomainMapper_Impl::IsFieldResultAsString()
{
    bool bRet = false;
    OSL_ENSURE( !m_aFieldStack.empty(), "field stack empty?");
    FieldContextPtr pContext = m_aFieldStack.top();
    OSL_ENSURE( pContext.get(), "no field context available");
    if( pContext.get() )
    {
        bRet = pContext->GetTextField().is();
    }
    return bRet;
}

void DomainMapper_Impl::AppendFieldResult(OUString const& rString)
{
    assert(!m_aFieldStack.empty());
    FieldContextPtr pContext = m_aFieldStack.top();
    SAL_WARN_IF(!pContext.get(), "writerfilter.dmapper", "no field context");
    if (pContext.get())
    {
        pContext->AppendResult(rString);
    }
}

// Calculates css::DateTime based on ddddd.sssss since 1900-1-0
static util::DateTime lcl_dateTimeFromSerial(const double& dSerial)
{
    const sal_uInt32 secondsPerDay = 86400;
    const sal_uInt16 secondsPerHour = 3600;

    DateTime d(Date(30, 12, 1899));
    d.AddDays( static_cast<sal_Int32>(dSerial) );

    double frac = std::modf(dSerial, &o3tl::temporary(double()));
    sal_uInt32 seconds = frac * secondsPerDay;

    util::DateTime date;
    date.Year = d.GetYear();
    date.Month = d.GetMonth();
    date.Day = d.GetDay();
    date.Hours = seconds / secondsPerHour;
    date.Minutes = (seconds % secondsPerHour) / 60;
    date.Seconds = seconds % 60;

    return date;
}

void DomainMapper_Impl::SetFieldResult(OUString const& rResult)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("setFieldResult");
    TagLogger::getInstance().chars(rResult);
#endif

    FieldContextPtr pContext = m_aFieldStack.top();
    OSL_ENSURE( pContext.get(), "no field context available");
    if( pContext.get() )
    {
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
                                titleStr = titleStr + rResult;
                                propertyVal.Value <<= titleStr;
                                aValues[nTitleFoundIndex] = propertyVal;
                            }
                            else
                            {
                                aValues.realloc(aValues.getLength() + 1);
                                propertyVal.Name = "Title";
                                propertyVal.Value <<= rResult;
                                aValues[aValues.getLength() - 1] = propertyVal;
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
                        // In case of SetExpression, the field result contains the content of the variable.
                        uno::Reference<lang::XServiceInfo> xServiceInfo(xTextField, uno::UNO_QUERY);
                        bool bIsSetExpression = xServiceInfo->supportsService("com.sun.star.text.TextField.SetExpression");
                        // If we already have content set, then use the current presentation
                        rtl::OUString sValue;
                        if (bIsSetExpression)
                        {   // this will throw for field types without Content
                            uno::Any aValue(xFieldProperties->getPropertyValue(
                                    getPropertyName(PROP_CONTENT)));
                            aValue >>= sValue;
                        }
                        xFieldProperties->setPropertyValue(
                                getPropertyName(bIsSetExpression && sValue.isEmpty()? PROP_CONTENT : PROP_CURRENT_PRESENTATION),
                             uno::makeAny( rResult ));
                    }
                }
                catch( const beans::UnknownPropertyException& )
                {
                    //some fields don't have a CurrentPresentation (DateTime)
                }
            }
        }
        catch (const uno::Exception& e)
        {
            SAL_WARN("writerfilter.dmapper",
                "DomainMapper_Impl::SetFieldResult: " << e);
        }
    }
}

void DomainMapper_Impl::SetFieldFFData(const FFDataHandler::Pointer_t& pFFDataHandler)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("setFieldFFData");
#endif

    if (m_aFieldStack.size())
    {
        FieldContextPtr pContext = m_aFieldStack.top();
        if (pContext.get())
        {
            pContext->setFFDataHandler(pFFDataHandler);
        }
    }

#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().endElement();
#endif
}

void DomainMapper_Impl::PopFieldContext()
{
    if(m_bDiscardHeaderFooter)
        return;
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().element("popFieldContext");
#endif

    if (m_aFieldStack.empty())
        return;

    FieldContextPtr pContext = m_aFieldStack.top();
    OSL_ENSURE( pContext.get(), "no field context available");
    if( pContext.get() )
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
                    if(xTOCMarkerCursor.is() || m_bStartIndex || m_bStartBibliography)
                    {
                        if (m_bStartIndex || m_bStartBibliography)
                        {
                            if (mxTOCTextCursor.is())
                            {
                                mxTOCTextCursor->goLeft(1,true);
                                mxTOCTextCursor->setString(OUString());
                            }
                            xTextAppend->finishParagraph(  uno::Sequence< beans::PropertyValue >() );
                        }
                        else
                        {
                            xTOCMarkerCursor->goLeft(1,true);
                            xTOCMarkerCursor->setString(OUString());
                            xTOCMarkerCursor->goLeft(1,true);
                            xTOCMarkerCursor->setString(OUString());
                        }
                    }
                    if (m_bStartedTOC || m_bStartIndex || m_bStartBibliography)
                    {
                        m_bStartedTOC = false;
                        m_aTextAppendStack.pop();
                        m_bTextInserted = false;
                    }
                    m_bStartTOC = false;
                    m_bStartIndex = false;
                    m_bStartBibliography = false;
                    if(m_bInHeaderFooterImport && m_bStartTOCHeaderFooter)
                        m_bStartTOCHeaderFooter = false;
                }
                else
                {
                    xToInsert.set(pContext->GetTC(), uno::UNO_QUERY);
                    if( !xToInsert.is() && !m_bStartTOC && !m_bStartIndex && !m_bStartBibliography )
                        xToInsert.set( pContext->GetTextField(), uno::UNO_QUERY);
                    if( xToInsert.is() && !m_bStartTOC && !m_bStartIndex && !m_bStartBibliography)
                    {
                        PropertyMap aMap;
                        // Character properties of the field show up here the
                        // last (always empty) run. Inherit character
                        // properties from there.
                        // Also merge in the properties from the field context,
                        // e.g. SdtEndBefore.
                        if (m_pLastCharacterContext.get())
                            aMap.InsertProps(m_pLastCharacterContext);
                        aMap.InsertProps(m_aFieldStack.top()->getProperties());
                        appendTextContent(xToInsert, aMap.GetPropertyValues());
                    }
                    else
                    {
                        FormControlHelper::Pointer_t pFormControlHelper(pContext->getFormControlHelper());
                        if (pFormControlHelper.get() != nullptr && pFormControlHelper->hasFFDataHandler() )
                        {
                            uno::Reference< text::XFormField > xFormField( pContext->GetFormField() );
                            xToInsert.set(xFormField, uno::UNO_QUERY);
                            if ( xFormField.is() && xToInsert.is() )
                            {
                                xCrsr->gotoEnd( true );
                                xToInsert->attach( uno::Reference< text::XTextRange >( xCrsr, uno::UNO_QUERY_THROW ));
                                pFormControlHelper->processField( xFormField );
                            }
                            else
                            {
                                uno::Reference<text::XTextRange> xTxtRange(xCrsr, uno::UNO_QUERY);
                                pFormControlHelper->insertControl(xTxtRange);
                            }
                        }
                        else if(!pContext->GetHyperlinkURL().isEmpty())
                        {
                            xCrsr->gotoEnd( true );

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
                                if (!pContext->GetHyperlinkStyle().isEmpty())
                                {
                                    xCrsrProperties->setPropertyValue("VisitedCharStyleName", uno::makeAny(pContext->GetHyperlinkStyle()));
                                    xCrsrProperties->setPropertyValue("UnvisitedCharStyleName", uno::makeAny(pContext->GetHyperlinkStyle()));
                                }
                            }
                        }
                        else if(m_bStartGenericField)
                        {
                            m_bStartGenericField = false;
                            if(m_bTextInserted)
                            {
                                m_aTextAppendStack.pop();
                                m_bTextInserted = false;
                            }
                        }
                    }
                }
            }
            catch(const lang::IllegalArgumentException&)
            {
                OSL_FAIL( "IllegalArgumentException in PopFieldContext()" );
            }
            catch(const uno::Exception&)
            {
                OSL_FAIL( "exception in PopFieldContext()" );
            }
        }

        //TOCs have to include all the imported content

    }
    //remove the field context
    m_aFieldStack.pop();
}


void DomainMapper_Impl::SetBookmarkName( const OUString& rBookmarkName )
{
    BookmarkMap_t::iterator aBookmarkIter = m_aBookmarkMap.find( m_sCurrentBkmkId );
    if( aBookmarkIter != m_aBookmarkMap.end() )
        aBookmarkIter->second.m_sBookmarkName = rBookmarkName;
    else
        m_sCurrentBkmkName = rBookmarkName;
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
                    xCursor->goLeft( 1, false );
                }
                uno::Reference< container::XNamed > xBkmNamed( xBookmark, uno::UNO_QUERY_THROW );
                assert(!aBookmarkIter->second.m_sBookmarkName.isEmpty());
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
                uno::Reference< text::XTextCursor > xCursor = xTextAppend->createTextCursorByRange( xTextAppend->getEnd() );

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
                uno::Reference< text::XTextRange > xCurrent = uno::Reference< text::XTextRange >(xCursor, uno::UNO_QUERY_THROW);
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
        if(xPropertySet.is())
        {
            if (bHasGrabBag)
            {
                uno::Sequence<beans::PropertyValue> aFrameGrabBag( comphelper::InitPropertySequence({
                    { "SdtEndBefore", uno::Any(true) }
                }));
                xPropertySet->setPropertyValue("FrameInteropGrabBag",uno::makeAny(aFrameGrabBag));
            }
        }
    }


    // Update the shape properties if it is embedded object.
    if(m_xEmbedded.is()){
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
        }
    }
    //insert it into the document at the current cursor position
    OSL_ENSURE( xTextContent.is(), "DomainMapper_Impl::ImportGraphic");
    if( xTextContent.is())
    {
        appendTextContent( xTextContent, uno::Sequence< beans::PropertyValue >() );

        if (eGraphicImportType == IMPORT_AS_DETECTED_ANCHOR && !m_aTextAppendStack.empty())
            // Remember this object is anchored to the current paragraph.
            m_aTextAppendStack.top().m_aAnchoredObjects.push_back(xTextContent);
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
        case PAGE_MAR_GUTTER : break;
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
{
}


void DomainMapper_Impl::RegisterFrameConversion(
        uno::Reference< text::XTextRange > const&    xFrameStartRange,
        uno::Reference< text::XTextRange > const&    xFrameEndRange,
        const std::vector<beans::PropertyValue>& rFrameProperties
        )
{
    OSL_ENSURE(
        m_aFrameProperties.empty() && !m_xFrameStartRange.is() && !m_xFrameEndRange.is(),
        "frame properties not removed");
    m_aFrameProperties = rFrameProperties;
    m_xFrameStartRange = xFrameStartRange;
    m_xFrameEndRange   = xFrameEndRange;
}


void DomainMapper_Impl::ExecuteFrameConversion()
{
    if( m_xFrameStartRange.is() && m_xFrameEndRange.is() && !m_bDiscardHeaderFooter )
    {
        try
        {
            uno::Reference< text::XTextAppendAndConvert > xTextAppendAndConvert( GetTopTextAppend(), uno::UNO_QUERY_THROW );
            xTextAppendAndConvert->convertToTextFrame(
                m_xFrameStartRange,
                m_xFrameEndRange,
                comphelper::containerToSequence(m_aFrameProperties) );
        }
        catch( const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION( "writerfilter.dmapper", "Exception caught when converting to frame");
        }
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
        else
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

sal_Int32 DomainMapper_Impl::GetCurrentRedlineToken(  )
{
    sal_Int32 nToken = 0;
    assert( m_currentRedline.get());
    nToken = m_currentRedline->m_nToken;
    return nToken;
}

void DomainMapper_Impl::SetCurrentRedlineAuthor( const OUString& sAuthor )
{
    if (!m_xAnnotationField.is())
    {
        if (m_currentRedline.get())
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
        if (m_currentRedline.get())
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
        if( !m_currentRedline.get())
            SAL_INFO("writerfilter.dmapper", "no current redline");
    }
}

void DomainMapper_Impl::SetCurrentRedlineToken( sal_Int32 nToken )
{
    assert( m_currentRedline.get());
    m_currentRedline->m_nToken = nToken;
}

void DomainMapper_Impl::SetCurrentRedlineRevertProperties( const uno::Sequence<beans::PropertyValue>& aProperties )
{
    assert( m_currentRedline.get());
    m_currentRedline->m_aRevertProperties = aProperties;
}


// This removes only the last redline stored here, those stored in contexts are automatically removed when
// the context is destroyed.
void DomainMapper_Impl::RemoveTopRedline( )
{
    assert( m_aRedlines.top().size( ) > 0 );
    m_aRedlines.top().pop_back( );
    m_currentRedline.clear();
}

void DomainMapper_Impl::ApplySettingsTable()
{
    if (m_pSettingsTable && m_xTextFactory.is())
    {
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
                    aViewProps.emplace_back("ZoomType", -1, uno::makeAny(sal_Int16(0)), beans::PropertyState_DIRECT_VALUE);
                }
                uno::Reference<container::XIndexContainer> xBox = document::IndexedPropertyValues::create(m_xComponentContext);
                xBox->insertByIndex(sal_Int32(0), uno::makeAny(comphelper::containerToSequence(aViewProps)));
                uno::Reference<container::XIndexAccess> xIndexAccess(xBox, uno::UNO_QUERY);
                uno::Reference<document::XViewDataSupplier> xViewDataSupplier(m_xTextDocument, uno::UNO_QUERY);
                xViewDataSupplier->setViewData(xIndexAccess);
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
            if( m_pSettingsTable->GetProtectForm() )
                xSettings->setPropertyValue("ProtectForm", uno::makeAny( true ));
        }
        catch(const uno::Exception&)
        {
        }
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
        const StyleSheetPropertyMap* pStyleSheetProperties = dynamic_cast<const StyleSheetPropertyMap*>(pEntry->pProperties.get());
        if (!pStyleSheetProperties)
            return xRet;
        sal_Int32 nListId = pStyleSheetProperties->GetListId();
        if (nListId < 0)
            return xRet;
        if (pListLevel)
            *pListLevel = pStyleSheetProperties->GetListLevel();

        // So we are in a paragraph style and it has numbering. Look up the relevant numbering rules.
        OUString aListName = ListDef::GetStyleName(nListId);
        uno::Reference< style::XStyleFamiliesSupplier > xStylesSupplier(GetTextDocument(), uno::UNO_QUERY_THROW);
        uno::Reference< container::XNameAccess > xStyleFamilies = xStylesSupplier->getStyleFamilies();
        uno::Reference<container::XNameAccess> xNumberingStyles;
        xStyleFamilies->getByName("NumberingStyles") >>= xNumberingStyles;
        uno::Reference<beans::XPropertySet> xStyle(xNumberingStyles->getByName(aListName), uno::UNO_QUERY);
        xRet.set(xStyle->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    }
    catch (const uno::Exception& e)
    {
        SAL_WARN("writerfilter.dmapper",
                "GetCurrentNumberingRules: exception caught: " << e);
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
            boost::optional<PropertyMap::Property> oProp = pContext->getProperty(PROP_NUMBERING_RULES);
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
        for (int i = 0; i < aProps.getLength(); ++i)
        {
            const beans::PropertyValue& rProp = aProps[i];

            if (rProp.Name == "CharStyleName")
            {
                OUString aCharStyle;
                rProp.Value >>= aCharStyle;
                uno::Reference<container::XNameAccess> xCharacterStyles;
                uno::Reference< style::XStyleFamiliesSupplier > xStylesSupplier(GetTextDocument(), uno::UNO_QUERY);
                uno::Reference< container::XNameAccess > xStyleFamilies = xStylesSupplier->getStyleFamilies();
                xStyleFamilies->getByName("CharacterStyles") >>= xCharacterStyles;
                xRet.set(xCharacterStyles->getByName(aCharStyle), uno::UNO_QUERY_THROW);
                break;
            }
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
    // ACtually process in DomainMapper, so that it's the same source file like normal processing.
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

        const OUString aListName = ListDef::GetStyleName(nListId);
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

sal_Int32 DomainMapper_Impl::getCurrentNumberingProperty(const OUString& aProp)
{
    sal_Int32 nRet = 0;

    boost::optional<PropertyMap::Property> pProp = m_pTopContext->getProperty(PROP_NUMBERING_RULES);
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

bool DomainMapper_Impl::isInteropGrabBagEnabled()
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
    ref->resolve(m_rDMapper);

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
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
