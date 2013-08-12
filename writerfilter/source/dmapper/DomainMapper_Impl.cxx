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

#include <DomainMapper_Impl.hxx>
#include <ConversionHelper.hxx>
#include <SdtHelper.hxx>
#include <DomainMapperTableHandler.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/document/PrinterIndependentLayout.hpp>
#include <com/sun/star/document/IndexedPropertyValues.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
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
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/text/XRedline.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/style/DropCapFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <oox/mathml/import.hxx>

#ifdef DEBUG_DOMAINMAPPER
#include <resourcemodel/QNameToString.hxx>
#include <resourcemodel/util.hxx>
#include <dmapperLoggers.hxx>
#endif
#include <ooxml/OOXMLFastTokens.hxx>

#include <map>

#include <comphelper/stlunosequence.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <officecfg/Office/Common.hxx>

using namespace ::com::sun::star;
using namespace ::rtl;
namespace writerfilter {
namespace dmapper{

// Populate Dropdown Field properties from FFData structure
void lcl_handleDropdownField( const uno::Reference< beans::XPropertySet >& rxFieldProps, FFDataHandler::Pointer_t pFFDataHandler )
{
    if ( rxFieldProps.is() )
    {
        if ( !pFFDataHandler->getName().isEmpty() )
            rxFieldProps->setPropertyValue( "Name", uno::makeAny( pFFDataHandler->getName() ) );

        const FFDataHandler::DropDownEntries_t& rEntries = pFFDataHandler->getDropDownEntries();
        uno::Sequence< OUString > sItems( rEntries.size() );
        ::std::copy( rEntries.begin(), rEntries.end(), ::comphelper::stl_begin(sItems));
        if ( sItems.getLength() )
            rxFieldProps->setPropertyValue( "Items", uno::makeAny( sItems ) );

        sal_Int32 nResult = pFFDataHandler->getDropDownResult().toInt32();
        if ( nResult )
            rxFieldProps->setPropertyValue( "SelectedItem", uno::makeAny( sItems[ nResult ] ) );
        if ( !pFFDataHandler->getHelpText().isEmpty() )
             rxFieldProps->setPropertyValue( "Help", uno::makeAny( pFFDataHandler->getHelpText() ) );
    }
}

void lcl_handleTextField( const uno::Reference< beans::XPropertySet >& rxFieldProps, FFDataHandler::Pointer_t pFFDataHandler, PropertyNameSupplier& rPropNameSupplier )
{
    if ( rxFieldProps.is() && pFFDataHandler )
    {
        rxFieldProps->setPropertyValue
            (rPropNameSupplier.GetName(PROP_HINT),
            uno::makeAny(pFFDataHandler->getStatusText()));
        rxFieldProps->setPropertyValue
            (rPropNameSupplier.GetName(PROP_HELP),
            uno::makeAny(pFFDataHandler->getHelpText()));
        rxFieldProps->setPropertyValue
            (rPropNameSupplier.GetName(PROP_CONTENT),
            uno::makeAny(pFFDataHandler->getTextDefault()));
    }
}

struct FieldConversion
{
    OUString     sWordCommand;
    const sal_Char*     cFieldServiceName;
    const sal_Char*     cFieldMasterServiceName;
    FieldId             eFieldId;
};

typedef ::std::map< OUString, FieldConversion>
            FieldConversionMap_t;


void FIB::SetData( Id nName, sal_Int32 nValue )
{
    OSL_ENSURE( nName >= NS_rtf::LN_WIDENT && nName <= NS_rtf::LN_LCBSTTBFUSSR, "invalid index in FIB");
    if( nName >= NS_rtf::LN_WIDENT && nName <= NS_rtf::LN_LCBSTTBFUSSR)
        aFIBData[nName - NS_rtf::LN_WIDENT] = nValue;
}


DomainMapper_Impl::DomainMapper_Impl(
            DomainMapper& rDMapper,
            uno::Reference < uno::XComponentContext >  xContext,
            uno::Reference< lang::XComponent >  xModel,
            SourceDocumentType eDocumentType,
            uno::Reference< text::XTextRange > xInsertTextRange,
            bool bIsNewDoc) :
        m_eDocumentType( eDocumentType ),
        m_rDMapper( rDMapper ),
        m_xTextDocument( xModel, uno::UNO_QUERY ),
        m_xTextFactory( xModel, uno::UNO_QUERY ),
        m_xComponentContext( xContext ),
        m_bSetUserFieldContent( false ),
        m_bIsFirstSection( true ),
        m_bIsColumnBreakDeferred( false ),
        m_bIsPageBreakDeferred( false ),
        m_pLastSectionContext( ),
        m_pLastCharacterContext(),
        m_nCurrentTabStopIndex( 0 ),
        m_sCurrentParaStyleId(),
        m_bInStyleSheetImport( false ),
        m_bInAnyTableImport( false ),
        m_bInHeaderFooterImport( false ),
        m_bDiscardHeaderFooter( false ),
        m_bLineNumberingSet( false ),
        m_bIsInFootnoteProperties( true ),
        m_bIsCustomFtnMark( false ),
        m_bIsParaChange( false ),
        m_bParaChanged( false ),
        m_bIsFirstParaInSection( true ),
        m_bIsLastParaInSection( false ),
        m_bIsInComments( false ),
        m_bParaSectpr( false ),
        m_bUsingEnhancedFields( false ),
        m_bSdt(false),
        m_xInsertTextRange(xInsertTextRange),
        m_bIsNewDoc(bIsNewDoc),
        m_bInTableStyleRunProps(false),
        m_pSdtHelper(0)

{
    appendTableManager( );
    GetBodyText();
    uno::Reference< text::XTextAppend > xBodyTextAppend = uno::Reference< text::XTextAppend >( m_xBodyText, uno::UNO_QUERY );
    m_aTextAppendStack.push(TextAppendContext(xBodyTextAppend,
                m_bIsNewDoc ? uno::Reference<text::XTextCursor>() : m_xBodyText->createTextCursorByRange(m_xInsertTextRange)));

    //todo: does it make sense to set the body text as static text interface?
    uno::Reference< text::XTextAppendAndConvert > xBodyTextAppendAndConvert( m_xBodyText, uno::UNO_QUERY );
    m_pTableHandler.reset
        (new DomainMapperTableHandler(xBodyTextAppendAndConvert, *this));
    getTableManager( ).setHandler(m_pTableHandler);

    getTableManager( ).startLevel();
    m_bUsingEnhancedFields = officecfg::Office::Common::Filter::Microsoft::Import::ImportWWFieldsAsEnhancedFields::get(m_xComponentContext);

    m_pSdtHelper = new SdtHelper(*this);

    m_aRedlines.push(std::vector<RedlineParamsPtr>());
}


DomainMapper_Impl::~DomainMapper_Impl()
{
    RemoveLastParagraph( );
    getTableManager( ).endLevel();
    popTableManager( );
    delete m_pSdtHelper;
}


uno::Reference< container::XNameContainer >    DomainMapper_Impl::GetPageStyles()
{
    if(!m_xPageStyles.is())
    {
        uno::Reference< style::XStyleFamiliesSupplier > xSupplier( m_xTextDocument, uno::UNO_QUERY );
        if (xSupplier.is())
            xSupplier->getStyleFamilies()->getByName("PageStyles") >>= m_xPageStyles;
    }
    return m_xPageStyles;
}


uno::Reference< text::XText > DomainMapper_Impl::GetBodyText()
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


uno::Reference< beans::XPropertySet > DomainMapper_Impl::GetDocumentSettings()
{
    if( !m_xDocumentSettings.is() && m_xTextFactory.is())
    {
        m_xDocumentSettings = uno::Reference< beans::XPropertySet >(
            m_xTextFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY );
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

void DomainMapper_Impl::RemoveLastParagraph( )
{
    if (m_aTextAppendStack.empty())
        return;
    uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
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
        else
        {
            xCursor->goLeft( 1, true );
            // If this is a text on a shape, possibly the text has the trailing
            // newline removed already.
            if (xCursor->getString() == "\n")
                xCursor->setString(OUString());
        }
    }
    catch( const uno::Exception& )
    {
    }
}

void DomainMapper_Impl::SetIsLastParagraphInSection( bool bIsLast )
{
    m_bIsLastParaInSection = bIsLast;
}

bool DomainMapper_Impl::GetIsLastParagraphInSection()
{
    return m_bIsLastParaInSection;
}

void DomainMapper_Impl::SetIsFirstParagraphInSection( bool bIsFirst )
{
    m_bIsFirstParaInSection = bIsFirst;
}

bool DomainMapper_Impl::GetIsFirstParagraphInSection()
{
    return m_bIsFirstParaInSection;
}

void DomainMapper_Impl::SetParaSectpr(bool bParaSectpr)
{
    m_bParaSectpr = bParaSectpr;
}

bool DomainMapper_Impl::GetParaSectpr()
{
    return m_bParaSectpr;
}

void DomainMapper_Impl::SetSdt(bool bSdt)
{
    m_bSdt = bSdt;
}

bool DomainMapper_Impl::GetSdt()
{
    return m_bSdt;
}

bool DomainMapper_Impl::GetParaChanged()
{
    return m_bParaChanged;
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
            if (xTextAppend.is())
                pSectionContext_->SetStart( xTextAppend->getEnd() );
        }
    }
    m_aPropertyStacks[eId].push( pInsert );
    m_aContextStack.push(eId);

    m_pTopContext = m_aPropertyStacks[eId].top();
}


void DomainMapper_Impl::PushStyleProperties( PropertyMapPtr pStyleProperties )
{
    m_aPropertyStacks[CONTEXT_STYLESHEET].push( pStyleProperties );
    m_aContextStack.push(CONTEXT_STYLESHEET);

    m_pTopContext = m_aPropertyStacks[CONTEXT_STYLESHEET].top();
}


void DomainMapper_Impl::PushListProperties(PropertyMapPtr pListProperties)
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
        // here, becase appendTextPortion() may not be called for every character section.
        deferredCharacterProperties.clear();
    }

    m_aPropertyStacks[eId].pop();
    m_aContextStack.pop();
    if(!m_aContextStack.empty() && !m_aPropertyStacks[m_aContextStack.top()].empty())

            m_pTopContext = m_aPropertyStacks[m_aContextStack.top()].top();
    else
    {
        // OSL_ENSURE(eId == CONTEXT_SECTION, "this should happen at a section context end");
        m_pTopContext.reset();
    }
}


PropertyMapPtr DomainMapper_Impl::GetTopContextOfType(ContextType eId)
{
    PropertyMapPtr pRet;
    SAL_WARN_IF( m_aPropertyStacks[eId].empty(), "writerfilter",
        "no context of type " << static_cast<int>(eId) << " available");
    if(!m_aPropertyStacks[eId].empty())
        pRet = m_aPropertyStacks[eId].top();
    return pRet;
}



uno::Reference< text::XTextAppend >  DomainMapper_Impl::GetTopTextAppend()
{
    OSL_ENSURE(!m_aTextAppendStack.empty(), "text append stack is empty" );
    return m_aTextAppendStack.top().xTextAppend;
}



void DomainMapper_Impl::InitTabStopFromStyle( const uno::Sequence< style::TabStop >& rInitTabStops )
{
    OSL_ENSURE(!m_aCurrentTabStops.size(), "tab stops already initialized");
    for( sal_Int32 nTab = 0; nTab < rInitTabStops.getLength(); ++nTab)
    {
        m_aCurrentTabStops.push_back( DeletableTabStop(rInitTabStops[nTab]) );
    }
}



void DomainMapper_Impl::ModifyCurrentTabStop( Id nId, sal_Int32 nValue)
{
    OSL_ENSURE(nId == NS_rtf::LN_dxaAdd || m_nCurrentTabStopIndex < m_aCurrentTabStops.size(),
        "tab stop creation error");

    if( nId != NS_rtf::LN_dxaAdd && m_nCurrentTabStopIndex >= m_aCurrentTabStops.size())
        return;
    static const style::TabAlign aTabAlignFromWord[] =
    {
        style::TabAlign_LEFT,
        style::TabAlign_CENTER,
        style::TabAlign_RIGHT,
        style::TabAlign_DECIMAL,
        style::TabAlign_LEFT
    };
    static const sal_Unicode aTabFillCharWord[] =
    {
        ' ',
        '.',
        '-',
        '_',
        '_',
        0xb7
    };

    switch(nId)
    {
        case NS_rtf::LN_dxaAdd: //set tab
            m_aCurrentTabStops.push_back(
                    DeletableTabStop(style::TabStop(ConversionHelper::convertTwipToMM100(nValue), style::TabAlign_LEFT, ' ', ' ')));
        break;
        case NS_rtf::LN_dxaDel: //deleted tab
        {
            //mark the tab stop at the given position as deleted
            ::std::vector<DeletableTabStop>::iterator aIt = m_aCurrentTabStops.begin();
            ::std::vector<DeletableTabStop>::iterator aEndIt = m_aCurrentTabStops.end();
            sal_Int32 nConverted = ConversionHelper::convertTwipToMM100(nValue);
            for( ; aIt != aEndIt; ++aIt)
            {
                if( aIt->Position == nConverted )
                {
                    aIt->bDeleted = true;
                    break;
                }
            }
        }
        break;
        case NS_rtf::LN_TLC: //tab leading characters - for decimal tabs
            // 0 - no leader, 1- dotted, 2 - hyphenated, 3 - single line, 4 - heavy line, 5 - middle dot
            if( nValue >= 0 &&  nValue < sal::static_int_cast<sal_Int32>(sizeof(aTabFillCharWord) / sizeof (sal_Unicode)))
                m_aCurrentTabStops[m_nCurrentTabStopIndex].FillChar = aTabFillCharWord[nValue];
        break;
        case NS_rtf::LN_JC: //tab justification
            //0 - left, 1 - centered, 2 - right, 3 - decimal 4 - bar
            if( nValue >= 0 && nValue < sal::static_int_cast<sal_Int32>(sizeof(aTabAlignFromWord) / sizeof (style::TabAlign)))
                m_aCurrentTabStops[m_nCurrentTabStopIndex].Alignment = aTabAlignFromWord[nValue];
        break;
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
    uno::Sequence< style::TabStop > aRet( sal_Int32( m_aCurrentTabStops.size() ) );
    style::TabStop* pArray = aRet.getArray();
    ::std::vector<DeletableTabStop>::const_iterator aIt = m_aCurrentTabStops.begin();
    ::std::vector<DeletableTabStop>::const_iterator aEndIt = m_aCurrentTabStops.end();
    sal_Int32 nDeleted = 0;
    for(sal_Int32 nIndex = 0; aIt != aEndIt; ++aIt)
    {
        if(!aIt->bDeleted)
            pArray[nIndex++] = *aIt;
        else
            ++nDeleted;
    }
    m_aCurrentTabStops.clear();
    m_nCurrentTabStopIndex = 0;
    if(nDeleted)
    {
        aRet.realloc( aRet.getLength() - nDeleted);
    }
    return aRet;
}

/*-------------------------------------------------------------------------
    returns a the value from the current paragraph style - if available
    TODO: What about parent styles?
  -----------------------------------------------------------------------*/
uno::Any DomainMapper_Impl::GetPropertyFromStyleSheet(PropertyIds eId)
{
    StyleSheetEntryPtr pEntry;
    if( m_bInStyleSheetImport )
        pEntry = GetStyleSheetTable()->FindParentStyleSheet(OUString());
    else
        pEntry =
                GetStyleSheetTable()->FindStyleSheetByISTD(GetCurrentParaStyleId());
    while(pEntry.get( ) )
    {
        //is there a tab stop set?
        if(pEntry->pProperties)
        {
            PropertyMap::const_iterator aPropertyIter =
                    pEntry->pProperties->find(PropertyDefinition(eId));
            if( aPropertyIter != pEntry->pProperties->end())
            {
                return aPropertyIter->second;
            }
        }
        //search until the property is set or no parent is available
        StyleSheetEntryPtr pNewEntry = GetStyleSheetTable()->FindParentStyleSheet(pEntry->sBaseStyleIdentifier);

        SAL_WARN_IF( pEntry == pNewEntry, "writerfilter", "circular loop in style hierarchy?");

        if (pEntry == pNewEntry) //fdo#49587
            break;

        pEntry = pNewEntry;
    }
    return uno::Any();
}


ListsManager::Pointer DomainMapper_Impl::GetListTable()
{
    if(!m_pListTable)
        m_pListTable.reset(
            new ListsManager( m_rDMapper, m_xTextFactory ));
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


void lcl_MoveBorderPropertiesToFrame(uno::Sequence<beans::PropertyValue>& rFrameProperties,
    uno::Reference<text::XTextRange> xStartTextRange,
    uno::Reference<text::XTextRange> xEndTextRange )
{
    try
    {
        uno::Reference<text::XTextCursor> xRangeCursor = xStartTextRange->getText()->createTextCursorByRange( xStartTextRange );
        xRangeCursor->gotoRange( xEndTextRange, true );

        uno::Reference<beans::XPropertySet> xTextRangeProperties(xRangeCursor, uno::UNO_QUERY);
        if(!xTextRangeProperties.is())
            return ;

        PropertyIds aBorderProperties[] =
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

        sal_uInt32 nStart = rFrameProperties.getLength();
        sal_uInt32 nBorderPropertyCount = sizeof( aBorderProperties ) / sizeof(PropertyIds);
        rFrameProperties.realloc(nStart + nBorderPropertyCount);

        beans::PropertyValue* pFrameProperties = rFrameProperties.getArray();
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        for( sal_uInt32 nProperty = 0; nProperty < nBorderPropertyCount; ++nProperty)
        {
            OUString sPropertyName = rPropNameSupplier.GetName(aBorderProperties[nProperty]);
            pFrameProperties[nStart].Name = sPropertyName;
            pFrameProperties[nStart].Value = xTextRangeProperties->getPropertyValue(sPropertyName);
            if( nProperty < 4 )
                xTextRangeProperties->setPropertyValue( sPropertyName, uno::makeAny(table::BorderLine2()));
            ++nStart;
        }
        rFrameProperties.realloc(nStart);
    }
    catch( const uno::Exception& )
    {
    }
}


void lcl_AddRangeAndStyle(
    ParagraphPropertiesPtr& pToBeSavedProperties,
    uno::Reference< text::XTextAppend > xTextAppend,
    PropertyMapPtr pPropertyMap,
    TextAppendContext& rAppendContext)
{
    uno::Reference<text::XParagraphCursor> xParaCursor(
        xTextAppend->createTextCursorByRange( rAppendContext.xInsertPosition.is() ? rAppendContext.xInsertPosition : xTextAppend->getEnd()), uno::UNO_QUERY_THROW );
    pToBeSavedProperties->SetEndingRange(xParaCursor->getStart());
    xParaCursor->gotoStartOfParagraph( false );

    pToBeSavedProperties->SetStartingRange(xParaCursor->getStart());
    if(pPropertyMap)
    {
        PropertyMap::iterator aParaStyleIter = pPropertyMap->find(PropertyDefinition( PROP_PARA_STYLE_NAME ) );
        if( aParaStyleIter != pPropertyMap->end())
        {
            OUString sName;
            aParaStyleIter->second >>= sName;
            pToBeSavedProperties->SetParaStyleName(sName);
        }
    }
}


//define some default frame width - 0cm ATM: this allow the frame to be wrapped around the text
#define DEFAULT_FRAME_MIN_WIDTH 0

void DomainMapper_Impl::CheckUnregisteredFrameConversion( )
{
    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
    if (m_aTextAppendStack.empty())
        return;
    TextAppendContext& rAppendContext = m_aTextAppendStack.top();
    // n#779642: ignore fly frame inside table as it could lead to messy situations
    if( rAppendContext.pLastParagraphProperties.get() && rAppendContext.pLastParagraphProperties->IsFrameMode()
        && !getTableManager().isInTable() )
    {
        try
        {
            StyleSheetEntryPtr pParaStyle =
                GetStyleSheetTable()->FindStyleSheetByConvertedStyleName(rAppendContext.pLastParagraphProperties->GetParaStyleName());

            uno::Sequence< beans::PropertyValue > aFrameProperties(pParaStyle ? 16: 9);

            if ( pParaStyle.get( ) )
            {
                beans::PropertyValue* pFrameProperties = aFrameProperties.getArray();
                pFrameProperties[0].Name = rPropNameSupplier.GetName(PROP_WIDTH);
                pFrameProperties[1].Name = rPropNameSupplier.GetName(PROP_HEIGHT);
                pFrameProperties[2].Name = rPropNameSupplier.GetName(PROP_SIZE_TYPE);
                pFrameProperties[3].Name = rPropNameSupplier.GetName(PROP_WIDTH_TYPE);
                pFrameProperties[4].Name = rPropNameSupplier.GetName(PROP_HORI_ORIENT);
                pFrameProperties[5].Name = rPropNameSupplier.GetName(PROP_HORI_ORIENT_POSITION);
                pFrameProperties[6].Name = rPropNameSupplier.GetName(PROP_HORI_ORIENT_RELATION);
                pFrameProperties[7].Name = rPropNameSupplier.GetName(PROP_VERT_ORIENT);
                pFrameProperties[8].Name = rPropNameSupplier.GetName(PROP_VERT_ORIENT_POSITION);
                pFrameProperties[9].Name = rPropNameSupplier.GetName(PROP_VERT_ORIENT_RELATION);
                pFrameProperties[10].Name = rPropNameSupplier.GetName(PROP_SURROUND);
                pFrameProperties[11].Name = rPropNameSupplier.GetName(PROP_LEFT_MARGIN);
                pFrameProperties[12].Name = rPropNameSupplier.GetName(PROP_RIGHT_MARGIN);
                pFrameProperties[13].Name = rPropNameSupplier.GetName(PROP_TOP_MARGIN);
                pFrameProperties[14].Name = rPropNameSupplier.GetName(PROP_BOTTOM_MARGIN);
                pFrameProperties[15].Name = rPropNameSupplier.GetName(PROP_BACK_COLOR_TRANSPARENCY);

                const ParagraphProperties* pStyleProperties = dynamic_cast<const ParagraphProperties*>( pParaStyle->pProperties.get() );
                sal_Int32 nWidth =
                    rAppendContext.pLastParagraphProperties->Getw() > 0 ?
                        rAppendContext.pLastParagraphProperties->Getw() :
                        pStyleProperties->Getw();
                bool bAutoWidth = nWidth < 1;
                if( bAutoWidth )
                    nWidth = DEFAULT_FRAME_MIN_WIDTH;
                pFrameProperties[0].Value <<= nWidth;

                pFrameProperties[1].Value <<=
                    rAppendContext.pLastParagraphProperties->Geth() > 0 ?
                        rAppendContext.pLastParagraphProperties->Geth() :
                        pStyleProperties->Geth();

                pFrameProperties[2].Value <<= sal_Int16(
                    rAppendContext.pLastParagraphProperties->GethRule() >= 0 ?
                        rAppendContext.pLastParagraphProperties->GethRule() :
                pStyleProperties->GethRule() >=0 ? pStyleProperties->GethRule() : text::SizeType::VARIABLE);

                pFrameProperties[3].Value <<= bAutoWidth ?  text::SizeType::MIN : text::SizeType::FIX;

                sal_Int16 nHoriOrient = sal_Int16(
                    rAppendContext.pLastParagraphProperties->GetxAlign() >= 0 ?
                        rAppendContext.pLastParagraphProperties->GetxAlign() :
                        pStyleProperties->GetxAlign() >= 0 ? pStyleProperties->GetxAlign() : text::HoriOrientation::NONE );
                pFrameProperties[4].Value <<= nHoriOrient;

                pFrameProperties[5].Value <<=
                    rAppendContext.pLastParagraphProperties->IsxValid() ?
                        rAppendContext.pLastParagraphProperties->Getx() : pStyleProperties->Getx();
                pFrameProperties[6].Value <<= sal_Int16(
                    rAppendContext.pLastParagraphProperties->GethAnchor() >= 0 ?
                        rAppendContext.pLastParagraphProperties->GethAnchor() :
                    pStyleProperties->GethAnchor() );

                sal_Int16 nVertOrient = sal_Int16(
                    rAppendContext.pLastParagraphProperties->GetyAlign() >= 0 ?
                        rAppendContext.pLastParagraphProperties->GetyAlign() :
                        pStyleProperties->GetyAlign() >= 0 ? pStyleProperties->GetyAlign() : text::VertOrientation::NONE );
                pFrameProperties[7].Value <<= nVertOrient;

                pFrameProperties[8].Value <<=
                    rAppendContext.pLastParagraphProperties->IsyValid() ?
                        rAppendContext.pLastParagraphProperties->Gety() : pStyleProperties->Gety();
                pFrameProperties[9].Value <<= sal_Int16(
                    rAppendContext.pLastParagraphProperties->GetvAnchor() >= 0 ?
                        rAppendContext.pLastParagraphProperties->GetvAnchor() :
                        pStyleProperties->GetvAnchor() );

                pFrameProperties[10].Value <<= text::WrapTextMode(
                    rAppendContext.pLastParagraphProperties->GetWrap() >= 0 ?
                    rAppendContext.pLastParagraphProperties->GetWrap() :
                    pStyleProperties->GetWrap());

                sal_Int32 nBottomDist;
                sal_Int32 nTopDist = nBottomDist =
                    rAppendContext.pLastParagraphProperties->GethSpace() >= 0 ?
                    rAppendContext.pLastParagraphProperties->GethSpace() :
                    pStyleProperties->GethSpace();

                pFrameProperties[11].Value <<= nVertOrient == text::VertOrientation::TOP ? 0 : nTopDist;
                pFrameProperties[12].Value <<= nVertOrient == text::VertOrientation::BOTTOM ? 0 : nBottomDist;

                sal_Int32 nRightDist;
                sal_Int32 nLeftDist = nRightDist =
                    rAppendContext.pLastParagraphProperties->GetvSpace() >= 0 ?
                    rAppendContext.pLastParagraphProperties->GetvSpace() :
                pStyleProperties->GetvSpace() >= 0 ? pStyleProperties->GetvSpace() : 0;
                pFrameProperties[13].Value <<= nHoriOrient == text::HoriOrientation::LEFT ? 0 : nLeftDist;
                pFrameProperties[14].Value <<= nHoriOrient == text::HoriOrientation::RIGHT ? 0 : nRightDist;
                // If there is no fill, the Word default is 100% transparency.
                // Otherwise CellColorHandler has priority, and this setting
                // will be ignored.
                pFrameProperties[15].Value <<= sal_Int32(100);

                lcl_MoveBorderPropertiesToFrame(aFrameProperties,
                    rAppendContext.pLastParagraphProperties->GetStartingRange(),
                    rAppendContext.pLastParagraphProperties->GetEndingRange());
            }
            else
            {
                beans::PropertyValue* pFrameProperties = aFrameProperties.getArray();
                pFrameProperties[0].Name = rPropNameSupplier.GetName(PROP_WIDTH);
                pFrameProperties[1].Name = rPropNameSupplier.GetName(PROP_SIZE_TYPE);
                pFrameProperties[2].Name = rPropNameSupplier.GetName(PROP_WIDTH_TYPE);
                pFrameProperties[3].Name = rPropNameSupplier.GetName(PROP_HORI_ORIENT);
                pFrameProperties[4].Name = rPropNameSupplier.GetName(PROP_VERT_ORIENT);
                pFrameProperties[5].Name = rPropNameSupplier.GetName(PROP_LEFT_MARGIN);
                pFrameProperties[6].Name = rPropNameSupplier.GetName(PROP_RIGHT_MARGIN);
                pFrameProperties[7].Name = rPropNameSupplier.GetName(PROP_TOP_MARGIN);
                pFrameProperties[8].Name = rPropNameSupplier.GetName(PROP_BOTTOM_MARGIN);

                sal_Int32 nWidth = rAppendContext.pLastParagraphProperties->Getw();
                bool bAutoWidth = nWidth < 1;
                if( bAutoWidth )
                    nWidth = DEFAULT_FRAME_MIN_WIDTH;
                pFrameProperties[0].Value <<= nWidth;

                pFrameProperties[1].Value <<= sal_Int16(
                    rAppendContext.pLastParagraphProperties->GethRule() >= 0 ?
                        rAppendContext.pLastParagraphProperties->GethRule() :
                        text::SizeType::VARIABLE);

                pFrameProperties[2].Value <<= bAutoWidth ?  text::SizeType::MIN : text::SizeType::FIX;

                sal_Int16 nHoriOrient = sal_Int16(
                    rAppendContext.pLastParagraphProperties->GetxAlign() >= 0 ?
                        rAppendContext.pLastParagraphProperties->GetxAlign() :
                        text::HoriOrientation::NONE );
                pFrameProperties[3].Value <<= nHoriOrient;

                sal_Int16 nVertOrient = sal_Int16(
                    rAppendContext.pLastParagraphProperties->GetyAlign() >= 0 ?
                        rAppendContext.pLastParagraphProperties->GetyAlign() :
                        text::VertOrientation::NONE );
                pFrameProperties[4].Value <<= nVertOrient;

                sal_Int32 nVertDist = rAppendContext.pLastParagraphProperties->GethSpace();
                if( nVertDist < 0 )
                    nVertDist = 0;
                pFrameProperties[5].Value <<= nVertOrient == text::VertOrientation::TOP ? 0 : nVertDist;
                pFrameProperties[6].Value <<= nVertOrient == text::VertOrientation::BOTTOM ? 0 : nVertDist;

                sal_Int32 nHoriDist = rAppendContext.pLastParagraphProperties->GetvSpace();
                if( nHoriDist < 0 )
                    nHoriDist = 0;
                pFrameProperties[7].Value <<= nHoriOrient == text::HoriOrientation::LEFT ? 0 : nHoriDist;
                pFrameProperties[8].Value <<= nHoriOrient == text::HoriOrientation::RIGHT ? 0 : nHoriDist;

                if( rAppendContext.pLastParagraphProperties->Geth() > 0 )
                {
                    sal_Int32 nOldSize = aFrameProperties.getLength();
                    aFrameProperties.realloc( nOldSize + 1 );
                    pFrameProperties = aFrameProperties.getArray();
                    pFrameProperties[nOldSize].Name = rPropNameSupplier.GetName(PROP_HEIGHT);
                    pFrameProperties[nOldSize].Value <<= rAppendContext.pLastParagraphProperties->Geth();
                }

                if( rAppendContext.pLastParagraphProperties->IsxValid() )
                {
                    sal_Int32 nOldSize = aFrameProperties.getLength();
                    aFrameProperties.realloc( nOldSize + 1 );
                    pFrameProperties = aFrameProperties.getArray();
                    pFrameProperties[nOldSize].Name = rPropNameSupplier.GetName(PROP_HORI_ORIENT_POSITION);
                    pFrameProperties[nOldSize].Value <<= rAppendContext.pLastParagraphProperties->Getx();
                }

                if( rAppendContext.pLastParagraphProperties->GethAnchor() >= 0 )
                {
                    sal_Int32 nOldSize = aFrameProperties.getLength();
                    aFrameProperties.realloc( nOldSize + 1 );
                    pFrameProperties = aFrameProperties.getArray();
                    pFrameProperties[nOldSize].Name = rPropNameSupplier.GetName(PROP_HORI_ORIENT_RELATION);
                    pFrameProperties[nOldSize].Value <<= sal_Int16(
                        rAppendContext.pLastParagraphProperties->GethAnchor() );
                }

                if( rAppendContext.pLastParagraphProperties->IsyValid() )
                {
                    sal_Int32 nOldSize = aFrameProperties.getLength();
                    aFrameProperties.realloc( nOldSize + 1 );
                    pFrameProperties = aFrameProperties.getArray();
                    pFrameProperties[nOldSize].Name = rPropNameSupplier.GetName(PROP_VERT_ORIENT_POSITION);
                    pFrameProperties[nOldSize].Value <<= rAppendContext.pLastParagraphProperties->Gety();
                }

                if( rAppendContext.pLastParagraphProperties->GetvAnchor() >= 0 )
                {
                    sal_Int32 nOldSize = aFrameProperties.getLength();
                    aFrameProperties.realloc( nOldSize + 1 );
                    pFrameProperties = aFrameProperties.getArray();
                    pFrameProperties[nOldSize].Name = rPropNameSupplier.GetName(PROP_VERT_ORIENT_RELATION);
                    pFrameProperties[nOldSize].Value <<= sal_Int16(
                        rAppendContext.pLastParagraphProperties->GetvAnchor() );
                }

                if( rAppendContext.pLastParagraphProperties->GetWrap() >= 0 )
                {
                    sal_Int32 nOldSize = aFrameProperties.getLength();
                    aFrameProperties.realloc( nOldSize + 1 );
                    pFrameProperties = aFrameProperties.getArray();
                    pFrameProperties[nOldSize].Name = rPropNameSupplier.GetName(PROP_SURROUND);
                    pFrameProperties[nOldSize].Value <<= text::WrapTextMode(
                        rAppendContext.pLastParagraphProperties->GetWrap() );
                }

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
}

void DomainMapper_Impl::finishParagraph( PropertyMapPtr pPropertyMap )
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("finishParagraph");
#endif

    ParagraphPropertyMap* pParaContext = dynamic_cast< ParagraphPropertyMap* >( pPropertyMap.get() );
    if (!m_aTextAppendStack.size())
        return;
    TextAppendContext& rAppendContext = m_aTextAppendStack.top();
    uno::Reference< text::XTextAppend >  xTextAppend;
    if (!m_aTextAppendStack.empty())
        xTextAppend = rAppendContext.xTextAppend;
    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->attribute("isTextAppend", xTextAppend.is());
#endif

    if(xTextAppend.is() && ! getTableManager( ).isIgnore() && pParaContext != NULL)
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
                sal::static_int_cast<Id>(pParaContext->GetDropCap()) != NS_ooxml::LN_Value_wordprocessingml_ST_DropCap_none;

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
                xParaProperties->setPropertyToDefault(rPropNameSupplier.GetName(PROP_CHAR_ESCAPEMENT));
                xParaProperties->setPropertyToDefault(rPropNameSupplier.GetName(PROP_CHAR_HEIGHT));
                //handles (2) and part of (6)
                pToBeSavedProperties.reset( new ParagraphProperties(*pParaContext) );
                sal_Int32 nCount = xParaCursor->getString().getLength();
                pToBeSavedProperties->SetDropCapLength(nCount > 0 && nCount < 255 ? (sal_Int8)nCount : 1);
            }
            if( rAppendContext.pLastParagraphProperties.get() )
            {
                if( sal::static_int_cast<Id>(rAppendContext.pLastParagraphProperties->GetDropCap()) != NS_ooxml::LN_Value_wordprocessingml_ST_DropCap_none)
                {
                    //handles (4) and part of (5)
                    //create a DropCap property, add it to the property sequence of finishParagraph
                    sal_Int32 nLines = rAppendContext.pLastParagraphProperties->GetLines();
                    aDrop.Lines = nLines > 0 && nLines < 254 ? (sal_Int8)++nLines : 2;
                    aDrop.Count = rAppendContext.pLastParagraphProperties->GetDropCapLength();
                    aDrop.Distance  = 0; //TODO: find distance value
                    //completes (5)
                    if( pParaContext->IsFrameMode() )
                        pToBeSavedProperties.reset( new ParagraphProperties(*pParaContext) );
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
                        pToBeSavedProperties.reset( new ParagraphProperties(*pParaContext) );
                        lcl_AddRangeAndStyle(pToBeSavedProperties, xTextAppend, pPropertyMap, rAppendContext);
                    }
                }

            }
            else //
            {
                // (1) doesn't need handling
                //
                if( !bIsDropCap && pParaContext->IsFrameMode() )
                {
                    pToBeSavedProperties.reset( new ParagraphProperties(*pParaContext) );
                    lcl_AddRangeAndStyle(pToBeSavedProperties, xTextAppend, pPropertyMap, rAppendContext);
                }
            }
            uno::Sequence< beans::PropertyValue > aProperties;
            if( pPropertyMap.get() )
            {
                aProperties = pPropertyMap->GetPropertyValues();
            }
            if( !bIsDropCap )
            {
                if( aDrop.Lines > 1 )
                {
                    sal_uInt32 nLength = aProperties.getLength();
                    aProperties.realloc(  nLength + 1 );
                    aProperties[nLength].Value <<= aDrop;
                    aProperties[nLength].Name = rPropNameSupplier.GetName(PROP_DROP_CAP_FORMAT);
                }
                uno::Reference< text::XTextRange > xTextRange;
                if (rAppendContext.xInsertPosition.is())
                {
                    xTextRange = xTextAppend->finishParagraphInsert( aProperties, rAppendContext.xInsertPosition );
                    rAppendContext.xCursor->gotoNextParagraph(false);
                    if (rAppendContext.pLastParagraphProperties.get())
                        rAppendContext.pLastParagraphProperties->SetEndingRange(xTextRange->getEnd());
                }
                else
                    xTextRange = xTextAppend->finishParagraph( aProperties );
                getTableManager( ).handle(xTextRange);

                // Get the end of paragraph character inserted
                uno::Reference< text::XTextCursor > xCur = xTextRange->getText( )->createTextCursor( );
                if (rAppendContext.xInsertPosition.is())
                    xCur->gotoRange( rAppendContext.xInsertPosition, false );
                else
                    xCur->gotoEnd( false );
                xCur->goLeft( 1 , true );
                uno::Reference< text::XTextRange > xParaEnd( xCur, uno::UNO_QUERY );
                CheckParaRedline( xParaEnd );

                m_bIsFirstParaInSection = false;
                m_bIsLastParaInSection = false;
                m_bParaChanged = false;

                // Reset the frame properties for the next paragraph
                pParaContext->ResetFrameProperties();
            }
            if( !bKeepLastParagraphProperties )
                rAppendContext.pLastParagraphProperties = pToBeSavedProperties;
        }
        catch(const lang::IllegalArgumentException&)
        {
            OSL_FAIL( "IllegalArgumentException in DomainMapper_Impl::finishParagraph" );
        }
        catch(const uno::Exception&)
        {
        }
    }

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement();
#endif
}


util::DateTime lcl_DateStringToDateTime( const OUString& rDateTime )
{
    util::DateTime aDateTime;
    //xsd::DateTime in the format [-]CCYY-MM-DDThh:mm:ss[Z|(+|-)hh:mm] example: 2008-01-21T10:42:00Z
    //OUString getToken( sal_Int32 token, sal_Unicode cTok, sal_Int32& index ) const SAL_THROW(())
    sal_Int32 nIndex = 0;
    OUString sDate = rDateTime.getToken( 0, 'T', nIndex );
    // HACK: this is broken according to the spec, but MSOffice always treats the time as local,
    // and writes it as Z (=UTC+0)
    OUString sTime = rDateTime.getToken( 0, 'Z', nIndex );
    nIndex = 0;
    aDateTime.Year = sal_uInt16( sDate.getToken( 0, '-', nIndex ).toInt32() );
    aDateTime.Month = sal_uInt16( sDate.getToken( 0, '-', nIndex ).toInt32() );
    aDateTime.Day = sal_uInt16( sDate.copy( nIndex ).toInt32() );

    nIndex = 0;
    aDateTime.Hours = sal_uInt16( sTime.getToken( 0, ':', nIndex ).toInt32() );
    aDateTime.Minutes = sal_uInt16( sTime.getToken( 0, ':', nIndex ).toInt32() );
    aDateTime.Seconds = sal_uInt16( sTime.copy( nIndex ).toInt32() );

    return aDateTime;
}

void DomainMapper_Impl::appendTextPortion( const OUString& rString, PropertyMapPtr pPropertyMap )
{
    if (m_bDiscardHeaderFooter)
        return;

    if (m_aTextAppendStack.empty())
        return;

    if( pPropertyMap == m_pTopContext && !deferredCharacterProperties.empty())
        processDeferredCharacterProperties();
    uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
    if(xTextAppend.is() && ! getTableManager( ).isIgnore())
    {
        try
        {
            uno::Reference< text::XTextRange > xTextRange;
            if (m_aTextAppendStack.top().xInsertPosition.is())
            {
                xTextRange = xTextAppend->insertTextPortion(rString, pPropertyMap->GetPropertyValues(), m_aTextAppendStack.top().xInsertPosition);
                m_aTextAppendStack.top().xCursor->gotoRange(xTextRange->getEnd(), false);
            }
            else
                xTextRange = xTextAppend->appendTextPortion(rString, pPropertyMap->GetPropertyValues());
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
    const uno::Reference< text::XTextContent > xContent,
    const uno::Sequence< beans::PropertyValue > xPropertyValues
    )
{
    uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert( m_aTextAppendStack.top().xTextAppend, uno::UNO_QUERY );
    OSL_ENSURE( xTextAppendAndConvert.is(), "trying to append a text content without XTextAppendAndConvert" );
    if(xTextAppendAndConvert.is() && ! getTableManager( ).isIgnore())
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



void DomainMapper_Impl::appendOLE( const OUString& rStreamName, OLEHandlerPtr pOLEHandler )
{
    static const OUString sEmbeddedService("com.sun.star.text.TextEmbeddedObject");
    try
    {
        uno::Reference< text::XTextContent > xOLE( m_xTextFactory->createInstance(sEmbeddedService), uno::UNO_QUERY_THROW );
        uno::Reference< beans::XPropertySet > xOLEProperties(xOLE, uno::UNO_QUERY_THROW);

        xOLEProperties->setPropertyValue(PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_STREAM_NAME ),
                        uno::makeAny( rStreamName ));
        awt::Size aSize = pOLEHandler->getSize();
        if( !aSize.Width )
            aSize.Width = 1000;
        if( !aSize.Height )
            aSize.Height = 1000;
        xOLEProperties->setPropertyValue(PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_WIDTH ),
                        uno::makeAny(aSize.Width));
        xOLEProperties->setPropertyValue(PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_HEIGHT ),
                        uno::makeAny(aSize.Height));

        uno::Reference< graphic::XGraphic > xGraphic = pOLEHandler->getReplacement();
        xOLEProperties->setPropertyValue(PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_GRAPHIC ),
                        uno::makeAny(xGraphic));
        // mimic the treatment of graphics here.. it seems anchoring as character
        // gives a better ( visually ) result
        xOLEProperties->setPropertyValue(PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_ANCHOR_TYPE ),  uno::makeAny( text::TextContentAnchorType_AS_CHARACTER ) );
        // remove ( if valid ) associated shape ( used for graphic replacement )
        m_aAnchoredStack.top( ).bToRemove = true;
        RemoveLastParagraph();
        m_aTextAppendStack.pop();

        //
        appendTextContent( xOLE, uno::Sequence< beans::PropertyValue >() );

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
        static const OUString sEmbeddedService("com.sun.star.text.TextEmbeddedObject");
        try
        {
            uno::Reference< text::XTextContent > xStarMath( m_xTextFactory->createInstance(sEmbeddedService), uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xStarMathProperties(xStarMath, uno::UNO_QUERY_THROW);

            xStarMathProperties->setPropertyValue(PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_EMBEDDED_OBJECT ),
                val.getAny());

            uno::Reference< uno::XInterface > xInterface( formula->getComponent(), uno::UNO_QUERY );
            Size size( 1000, 1000 );
            if( oox::FormulaImportBase* formulaimport = dynamic_cast< oox::FormulaImportBase* >( xInterface.get()))
                size = formulaimport->getFormulaSize();
            xStarMathProperties->setPropertyValue(PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_WIDTH ),
                uno::makeAny( sal_Int32(size.Width())));
            xStarMathProperties->setPropertyValue(PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_HEIGHT ),
                uno::makeAny( sal_Int32(size.Height())));
            // mimic the treatment of graphics here.. it seems anchoring as character
            // gives a better ( visually ) result
            xStarMathProperties->setPropertyValue(PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_ANCHOR_TYPE ),
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
                                    uno::Reference< text::XTextRange >& xBefore )
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
            static const OUString sSectionService("com.sun.star.text.TextSection");
            uno::Reference< text::XTextContent > xSection( m_xTextFactory->createInstance(sSectionService), uno::UNO_QUERY_THROW );
            xSection->attach( uno::Reference< text::XTextRange >( xCursor, uno::UNO_QUERY_THROW) );
            xRet = uno::Reference< beans::XPropertySet > (xSection, uno::UNO_QUERY );
        }
        catch(const uno::Exception&)
        {
        }

    }

    return xRet;
}


void DomainMapper_Impl::PushPageHeader(SectionPropertyMap::PageType eType)
{
    m_bInHeaderFooterImport = true;

    //get the section context
    PropertyMapPtr pContext = DomainMapper_Impl::GetTopContextOfType(CONTEXT_SECTION);
    //ask for the header name of the given type
    SectionPropertyMap* pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
    if(pSectionContext)
    {
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
            if ((!bLeft && !m_pSettingsTable->GetEvenAndOddHeaders()) || (m_pSettingsTable->GetEvenAndOddHeaders()))
            {
                PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

                //switch on header use
                xPageStyle->setPropertyValue(
                        rPropNameSupplier.GetName(PROP_HEADER_IS_ON),
                        uno::makeAny(sal_True) );

                // If the 'Different Even & Odd Pages' flag is turned on - do not ignore it
                // Even if the 'Even' header is blank - the flag should be imported (so it would look in LO like in Word)
                if( m_pSettingsTable->GetEvenAndOddHeaders() )
                    xPageStyle->setPropertyValue(rPropNameSupplier.GetName(PROP_HEADER_IS_SHARED), uno::makeAny( false ));

                //set the interface
                uno::Reference< text::XText > xHeaderText;
                xPageStyle->getPropertyValue(rPropNameSupplier.GetName( bLeft ? PROP_HEADER_TEXT_LEFT : PROP_HEADER_TEXT) ) >>= xHeaderText;
                m_aTextAppendStack.push( TextAppendContext(uno::Reference< text::XTextAppend >( xHeaderText, uno::UNO_QUERY_THROW),
                            m_bIsNewDoc ? uno::Reference<text::XTextCursor>() : m_xBodyText->createTextCursorByRange(xHeaderText->getStart())));
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


void DomainMapper_Impl::PushPageFooter(SectionPropertyMap::PageType eType)
{
    m_bInHeaderFooterImport = true;

    //get the section context
    PropertyMapPtr pContext = DomainMapper_Impl::GetTopContextOfType(CONTEXT_SECTION);
    //ask for the footer name of the given type
    SectionPropertyMap* pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
    if(pSectionContext)
    {
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
            if ((!bLeft && !m_pSettingsTable->GetEvenAndOddHeaders()) || (m_pSettingsTable->GetEvenAndOddHeaders()))
            {
                PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

                //switch on footer use
                xPageStyle->setPropertyValue(
                        rPropNameSupplier.GetName(PROP_FOOTER_IS_ON),
                        uno::makeAny(sal_True) );

                // If the 'Different Even & Odd Pages' flag is turned on - do not ignore it
                // Even if the 'Even' footer is blank - the flag should be imported (so it would look in LO like in Word)
                if( m_pSettingsTable->GetEvenAndOddHeaders() )
                    xPageStyle->setPropertyValue(rPropNameSupplier.GetName(PROP_FOOTER_IS_SHARED), uno::makeAny( false ));

                //set the interface
                uno::Reference< text::XText > xFooterText;
                xPageStyle->getPropertyValue(rPropNameSupplier.GetName( bLeft ? PROP_FOOTER_TEXT_LEFT : PROP_FOOTER_TEXT) ) >>= xFooterText;
                m_aTextAppendStack.push(TextAppendContext(uno::Reference< text::XTextAppend >( xFooterText, uno::UNO_QUERY_THROW ),
                            m_bIsNewDoc ? uno::Reference<text::XTextCursor>() : m_xBodyText->createTextCursorByRange(xFooterText->getStart())));
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
}


void DomainMapper_Impl::PushFootOrEndnote( bool bIsFootnote )
{
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
        if( pTopContext->GetFootnoteSymbol() != 0)
        {
            xFootnote->setLabel( OUString( pTopContext->GetFootnoteSymbol() ) );
        }
        FontTablePtr pFontTable = GetFontTable();
        uno::Sequence< beans::PropertyValue > aFontProperties;
        if( pFontTable && pTopContext->GetFootnoteFontId() >= 0 && pFontTable->size() > (size_t)pTopContext->GetFootnoteFontId() )
        {
            const FontEntry::Pointer_t pFontEntry(pFontTable->getFontEntry(sal_uInt32(pTopContext->GetFootnoteFontId())));
            PropertyMapPtr aFontProps( new PropertyMap );
            aFontProps->Insert(PROP_CHAR_FONT_NAME, uno::makeAny( pFontEntry->sFontName  ));
            aFontProps->Insert(PROP_CHAR_FONT_CHAR_SET, uno::makeAny( (sal_Int16)pFontEntry->nTextEncoding  ));
            aFontProps->Insert(PROP_CHAR_FONT_PITCH, uno::makeAny( pFontEntry->nPitchRequest  ));
            aFontProperties = aFontProps->GetPropertyValues();
        }
        else if(!pTopContext->GetFootnoteFontName().isEmpty())
        {
            PropertyMapPtr aFontProps( new PropertyMap );
            aFontProps->Insert(PROP_CHAR_FONT_NAME, uno::makeAny( pTopContext->GetFootnoteFontName()  ));
            aFontProperties = aFontProps->GetPropertyValues();
        }
        appendTextContent( uno::Reference< text::XTextContent >( xFootnoteText, uno::UNO_QUERY_THROW ), aFontProperties );
        m_aTextAppendStack.push(TextAppendContext(uno::Reference< text::XTextAppend >( xFootnoteText, uno::UNO_QUERY_THROW ),
                    m_bIsNewDoc ? uno::Reference<text::XTextCursor>() : xFootnoteText->createTextCursorByRange(xFootnoteText->getStart())));

        // Redlines for the footnote anchor
        CheckRedline( xFootnote->getAnchor( ) );
    }
    catch( const uno::Exception& e )
    {
        SAL_WARN("writerfilter", "exception in PushFootOrEndnote: " << e.Message);
    }
}

void DomainMapper_Impl::CreateRedline( uno::Reference< text::XTextRange > xRange, RedlineParamsPtr& pRedline )
{
    if ( pRedline.get( ) )
    {
        try
        {
            OUString sType;
            PropertyNameSupplier & rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier(  );
            switch ( pRedline->m_nToken & 0xffff )
            {
            case ooxml::OOXML_mod:
                sType = rPropNameSupplier.GetName( PROP_FORMAT );
                break;
            case ooxml::OOXML_ins:
                sType = rPropNameSupplier.GetName( PROP_INSERT );
                break;
            case ooxml::OOXML_del:
                sType = rPropNameSupplier.GetName( PROP_DELETE );
                break;
            }
            uno::Reference < text::XRedline > xRedline( xRange, uno::UNO_QUERY_THROW );
            beans::PropertyValues aRedlineProperties( 2 );
            beans::PropertyValue * pRedlineProperties = aRedlineProperties.getArray(  );
            pRedlineProperties[0].Name = rPropNameSupplier.GetName( PROP_REDLINE_AUTHOR );
            pRedlineProperties[0].Value <<= pRedline->m_sAuthor;
            pRedlineProperties[1].Name = rPropNameSupplier.GetName( PROP_REDLINE_DATE_TIME );
            pRedlineProperties[1].Value <<= lcl_DateStringToDateTime( pRedline->m_sDate );

            xRedline->makeRedline( sType, aRedlineProperties );
        }
        catch( const uno::Exception & )
        {
            OSL_FAIL( "Exception in makeRedline" );
        }
    }
}

void DomainMapper_Impl::CheckParaRedline( uno::Reference< text::XTextRange > xRange )
{
    if ( m_pParaRedline.get( ) )
    {
        CreateRedline( xRange, m_pParaRedline );
        ResetParaRedline( );
    }
}

void DomainMapper_Impl::CheckRedline( uno::Reference< text::XTextRange > xRange )
{
    vector<RedlineParamsPtr>::iterator pIt = m_aRedlines.top().begin( );
    vector< RedlineParamsPtr > aCleaned;
    for (; pIt != m_aRedlines.top().end( ); ++pIt )
    {
        CreateRedline( xRange, *pIt );

        // Adding the non-mod redlines to the temporary vector
        if ( pIt->get( ) && ( ( *pIt )->m_nToken & 0xffff ) != ooxml::OOXML_mod )
        {
            aCleaned.push_back( *pIt );
        }
    }

    m_aRedlines.top().swap( aCleaned );
}

void DomainMapper_Impl::StartParaChange( )
{
    m_bIsParaChange = true;
}

void DomainMapper_Impl::EndParaChange( )
{
    m_bIsParaChange = false;
}



void DomainMapper_Impl::PushAnnotation()
{
    try
    {
        PropertyMapPtr pTopContext = GetTopContext();
        m_bIsInComments = true;
        if (!GetTextFactory().is())
            return;
        m_xAnnotationField = uno::Reference< beans::XPropertySet >( GetTextFactory()->createInstance(
                "com.sun.star.text.TextField.Annotation" ),
            uno::UNO_QUERY_THROW );
        uno::Reference< text::XText > xAnnotationText;
        m_xAnnotationField->getPropertyValue("TextRange") >>= xAnnotationText;
        m_aTextAppendStack.push(TextAppendContext(uno::Reference< text::XTextAppend >( xAnnotationText, uno::UNO_QUERY_THROW ),
                    m_bIsNewDoc ? uno::Reference<text::XTextCursor>() : m_xBodyText->createTextCursorByRange(xAnnotationText->getStart())));
    }
    catch( const uno::Exception& rException)
    {
        SAL_WARN("writerfilter", "exception in PushAnnotation: " << rException.Message);
    }
}


void DomainMapper_Impl::PopFootOrEndnote()
{
    RemoveLastParagraph();
    if (!m_aTextAppendStack.empty())
        m_aTextAppendStack.pop();

    if (m_aRedlines.size() == 1)
    {
        SAL_WARN("writerfilter", "PopFootOrEndnote() is called without PushFootOrEndnote()?");
        return;
    }
    m_aRedlines.pop();
}


void DomainMapper_Impl::PopAnnotation()
{
    RemoveLastParagraph();

    m_bIsInComments = false;
    m_aTextAppendStack.pop();

    try
    {
        // See if the annotation will be a single position or a range.
        if (!m_aAnnotationPosition.m_xStart.is() ||
            !m_aAnnotationPosition.m_xEnd.is())
        {
            uno::Sequence< beans::PropertyValue > aEmptyProperties;
            appendTextContent(uno::Reference<text::XTextContent>(
                m_xAnnotationField, uno::UNO_QUERY_THROW), aEmptyProperties);
        }
        else
        {
            // Create a range that points to the annotation start/end.
            uno::Reference<text::XText> const xText =
                m_aAnnotationPosition.m_xStart->getText();
            uno::Reference<text::XTextCursor> const xCursor =
                xText->createTextCursorByRange(m_aAnnotationPosition.m_xStart);
            xCursor->gotoRange(m_aAnnotationPosition.m_xEnd, true);
            uno::Reference<text::XTextRange> const xTextRange(
                    xCursor, uno::UNO_QUERY_THROW);

            // Attach the annotation to the range.
            uno::Reference<text::XTextAppend> const xTextAppend =
                m_aTextAppendStack.top().xTextAppend;
            xTextAppend->insertTextContent(xTextRange,
                    uno::Reference<text::XTextContent>(m_xAnnotationField,
                        uno::UNO_QUERY_THROW),
                    !xCursor->isCollapsed());
        }
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("writerfilter",
                "Cannot insert annotation field: exception: " << e.Message);
    }

    m_aAnnotationPosition.m_xStart.clear();
    m_aAnnotationPosition.m_xEnd.clear();
    m_xAnnotationField.clear();

}

void DomainMapper_Impl::PushPendingShape( const uno::Reference< drawing::XShape > xShape )
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

void DomainMapper_Impl::PushShapeContext( const uno::Reference< drawing::XShape > xShape )
{
    if (m_aTextAppendStack.empty())
        return;
    uno::Reference<text::XTextAppend> xTextAppend = m_aTextAppendStack.top().xTextAppend;

    appendTableManager( );
    appendTableHandler( );
    getTableManager().startLevel();
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
            m_aAnchoredStack.push(xTxtContent);
        }
        else
        {
            uno::Reference< text::XTextRange > xShapeText( xShape, uno::UNO_QUERY_THROW);
            // Add the shape to the text append stack
            m_aTextAppendStack.push( TextAppendContext(uno::Reference< text::XTextAppend >( xShape, uno::UNO_QUERY_THROW ),
                        m_bIsNewDoc ? uno::Reference<text::XTextCursor>() : m_xBodyText->createTextCursorByRange(xShapeText->getStart() )));

            // Add the shape to the anchored objects stack
            uno::Reference< text::XTextContent > xTxtContent( xShape, uno::UNO_QUERY_THROW );
            m_aAnchoredStack.push( xTxtContent );

            PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

            uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY_THROW );
#ifdef DEBUG_DOMAINMAPPER
            dmapper_logger->unoPropertySet(xProps);
#endif
            bool bIsGraphic = xSInfo->supportsService( "com.sun.star.drawing.GraphicObjectShape" );

            // If there are position properties, the shape should not be inserted "as character".
            sal_Int32 nHoriPosition = 0, nVertPosition = 0;
            xProps->getPropertyValue(rPropNameSupplier.GetName(PROP_HORI_ORIENT_POSITION)) >>= nHoriPosition;
            xProps->getPropertyValue(rPropNameSupplier.GetName(PROP_VERT_ORIENT_POSITION)) >>= nVertPosition;
            if (nHoriPosition != 0 || nVertPosition != 0)
                bIsGraphic = false;
            text::TextContentAnchorType nAnchorType(text::TextContentAnchorType_AT_PARAGRAPH);
            xProps->getPropertyValue(rPropNameSupplier.GetName( PROP_ANCHOR_TYPE )) >>= nAnchorType;
            if (nAnchorType == text::TextContentAnchorType_AT_PAGE)
                bIsGraphic = false;

            if (!m_bInHeaderFooterImport)
                xProps->setPropertyValue(
                        rPropNameSupplier.GetName( PROP_OPAQUE ),
                        uno::makeAny( true ) );
            if (xSInfo->supportsService("com.sun.star.text.TextFrame"))
            {
                uno::Reference<text::XTextContent> xTextContent(xShape, uno::UNO_QUERY_THROW);
                uno::Reference<text::XTextRange> xTextRange(xTextAppend->createTextCursorByRange(xTextAppend->getEnd()), uno::UNO_QUERY_THROW);
                xTextAppend->insertTextContent(xTextRange, xTextContent, sal_False);
            }
            else if (nAnchorType != text::TextContentAnchorType_AS_CHARACTER)
            {
                xProps->setPropertyValue( rPropNameSupplier.GetName( PROP_ANCHOR_TYPE ), bIsGraphic  ?  uno::makeAny( text::TextContentAnchorType_AS_CHARACTER ) : uno::makeAny( text::TextContentAnchorType_AT_PARAGRAPH ) );
            }
        }
    }
    catch ( const uno::Exception& e )
    {
        SAL_WARN("writerfilter", "Exception when adding shape: " << e.Message);
    }
}



void DomainMapper_Impl::PopShapeContext()
{
    getTableManager().endLevel();
    popTableManager();
    if ( m_aAnchoredStack.size() > 0 )
    {
        // For OLE object replacement shape, the text append context was already removed
        // or the OLE object couldn't be inserted.
        if ( !m_aAnchoredStack.top().bToRemove )
        {
            RemoveLastParagraph();
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

        // Remove the shape if required (most likely replacement shape for OLE object)
        if ( m_aAnchoredStack.top().bToRemove )
        {
            try
            {
                uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(m_xTextDocument, uno::UNO_QUERY_THROW);
                uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
                if ( xDrawPage.is() )
                {
                    uno::Reference<drawing::XShape> xShape( xObj, uno::UNO_QUERY_THROW );
                    xDrawPage->remove( xShape );
                }
            }
            catch( const uno::Exception& )
            {
            }
        }
        m_aAnchoredStack.pop();
    }
}


OUString lcl_FindQuotedText( const OUString& rCommand,
                const sal_Char* cStartQuote, const sal_Unicode uEndQuote )
{
    OUString sRet;
    OUString sStartQuote( OUString::createFromAscii(cStartQuote) );
    sal_Int32 nStartIndex = rCommand.indexOf( sStartQuote );
    if( nStartIndex >= 0 )
    {
        sal_Int32 nStartLength = sStartQuote.getLength();
        sal_Int32 nEndIndex = rCommand.indexOf( uEndQuote, nStartIndex + nStartLength);
        if( nEndIndex > nStartIndex )
        {
            sRet = rCommand.copy( nStartIndex + nStartLength, nEndIndex - nStartIndex - nStartLength);
        }
    }
    return sRet;

}


sal_Int16 lcl_ParseNumberingType( const OUString& rCommand )
{
    sal_Int16 nRet = style::NumberingType::PAGE_DESCRIPTOR;

    //  The command looks like: " PAGE \* Arabic "
    OUString sNumber = lcl_FindQuotedText(rCommand, "\\* ", ' ');

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
        for( sal_uInt32 nNum = 0; nNum < sizeof(aNumberingPairs)/sizeof( NumberingPairs ); ++nNum)
        {
            if( /*sCommand*/sNumber.equalsAscii(aNumberingPairs[nNum].cWordName ))
            {
                nRet = aNumberingPairs[nNum].nType;
                break;
            }
        }

    }
    return nRet;
}


OUString lcl_ParseFormat( const OUString& rCommand )
{
    //  The command looks like: " DATE \@ "dd MMMM yyyy"
    return lcl_FindQuotedText(rCommand, "\\@ \"", '\"');
}
/*-------------------------------------------------------------------------
extract a parameter (with or without quotes) between the command and the following backslash
  -----------------------------------------------------------------------*/
OUString lcl_ExtractParameter(const OUString& rCommand, sal_Int32 nCommandLength )
{
    sal_Int32 nStartIndex = nCommandLength;
    sal_Int32 nEndIndex = 0;
    sal_Int32 nQuoteIndex = rCommand.indexOf( '\"', nStartIndex);
    if( nQuoteIndex >= 0)
    {
        nStartIndex = nQuoteIndex + 1;
        nEndIndex = rCommand.indexOf( '\"', nStartIndex + 1) - 1;
    }
    else
    {
        nEndIndex = rCommand.indexOf(" \\", nStartIndex);
    }
    OUString sRet;
    if( nEndIndex > nStartIndex + 1 )
    {
        //remove spaces at start and end of the result
        if(nQuoteIndex <= 0)
        {
            const sal_Unicode* pCommandStr = rCommand.getStr();
            while( nStartIndex < nEndIndex && pCommandStr[nStartIndex] == ' ')
                    ++nStartIndex;
            while( nEndIndex > nStartIndex && pCommandStr[nEndIndex] == ' ')
                    --nEndIndex;
        }
        sRet = rCommand.copy( nStartIndex, nEndIndex - nStartIndex + 1);
    }
    return sRet;
}



OUString lcl_ExctractAskVariableAndHint( const OUString& rCommand, OUString& rHint )
{
    // the first word after "ASK " is the variable
    // the text after the variable and before a '\' is the hint
    // if no hint is set the variable is used as hint
    // the quotes of the hint have to be removed
    sal_Int32 nIndex = rCommand.indexOf( ' ', 2);//find last space after 'ASK'
    while(rCommand.getStr()[nIndex] == ' ')
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


bool lcl_FindInCommand(
    const OUString& rCommand,
    sal_Unicode cSwitch,
    OUString& rValue )
{
    bool bRet = false;
    OUString sSearch('\\');
    sSearch += OUString( cSwitch );
    sal_Int32 nIndex = rCommand.indexOf( sSearch  );
    if( nIndex >= 0 )
    {
        bRet = true;
        //find next '\' or end of string
        sal_Int32 nEndIndex = rCommand.indexOf( '\\', nIndex + 1);
        if( nEndIndex < 0 )
            nEndIndex = rCommand.getLength() - 1;
        if( nEndIndex - nIndex > 3 )
            rValue = rCommand.copy( nIndex + 3, nEndIndex - nIndex - 3);
    }
    return bRet;
}


void DomainMapper_Impl::GetCurrentLocale(lang::Locale& rLocale)
{
    PropertyMapPtr pTopContext = GetTopContext();
    PropertyDefinition aCharLocale( PROP_CHAR_LOCALE );
    PropertyMap::iterator aLocaleIter = pTopContext->find( aCharLocale );
    if( aLocaleIter != pTopContext->end())
        aLocaleIter->second >>= rLocale;
    else
    {
        PropertyMapPtr pParaContext = GetTopContextOfType(CONTEXT_PARAGRAPH);
        aLocaleIter = pParaContext->find(aCharLocale);
        if( aLocaleIter != pParaContext->end())
        {
            aLocaleIter->second >>= rLocale;
        }
    }
}

/*-------------------------------------------------------------------------
    extract the number format from the command and apply the resulting number
    format to the XPropertySet
  -----------------------------------------------------------------------*/
void DomainMapper_Impl::SetNumberFormat( const OUString& rCommand,
                            uno::Reference< beans::XPropertySet >& xPropertySet )
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
        uno::Reference< util::XNumberFormatsSupplier > xNumberSupplier( m_xTextDocument, uno::UNO_QUERY_THROW );
        sal_Int32 nKey = xNumberSupplier->getNumberFormats()->addNewConverted( sFormat, aUSLocale, aCurrentLocale );
        xPropertySet->setPropertyValue(
            PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_NUMBER_FORMAT),
            uno::makeAny( nKey ));
        xPropertySet->getPropertyValue(
            PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_NUMBER_FORMAT ) ) >>= nKey;
    }
    catch(const uno::Exception&)
    {
    }
}



uno::Reference< beans::XPropertySet > DomainMapper_Impl::FindOrCreateFieldMaster(
        const sal_Char* pFieldMasterService, const OUString& rFieldMasterName )
            throw(::com::sun::star::uno::Exception)
{
    // query master, create if not available
    uno::Reference< text::XTextFieldsSupplier > xFieldsSupplier( GetTextDocument(), uno::UNO_QUERY_THROW );
    uno::Reference< container::XNameAccess > xFieldMasterAccess = xFieldsSupplier->getTextFieldMasters();
    uno::Reference< beans::XPropertySet > xMaster;
    OUString sFieldMasterService( OUString::createFromAscii(pFieldMasterService) );
    OUStringBuffer aFieldMasterName;
    aFieldMasterName.appendAscii( pFieldMasterService );
    aFieldMasterName.append(sal_Unicode('.'));
    aFieldMasterName.append(rFieldMasterName);
    OUString sFieldMasterName = aFieldMasterName.makeStringAndClear();
    if(xFieldMasterAccess->hasByName(sFieldMasterName))
    {
        //get the master
        xMaster = uno::Reference< beans::XPropertySet >(xFieldMasterAccess->getByName(sFieldMasterName),
                                                                            uno::UNO_QUERY_THROW);
    }
    else
    {
        //create the master
        xMaster = uno::Reference< beans::XPropertySet >(
                m_xTextFactory->createInstance(sFieldMasterService), uno::UNO_QUERY_THROW);
        //set the master's name
            xMaster->setPropertyValue(
                    PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_NAME),
                    uno::makeAny(rFieldMasterName));
    }
    return xMaster;
}

/*-------------------------------------------------------------------------
//field context starts with a 0x13
  -----------------------------------------------------------------------*/
void DomainMapper_Impl::PushFieldContext()
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->element("pushFieldContext");
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
    m_aFieldStack.push( FieldContextPtr( new FieldContext( xStart ) ) );
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


FieldContext::FieldContext(uno::Reference< text::XTextRange > xStart) :
    m_bFieldCommandCompleted( false )
    ,m_xStartRange( xStart )
{
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

        if (sToken.getStr()[0] == '"')
        {
            bInStringNext = true;
            sToken = sToken.copy(1);
        }
        if (sToken.getStr()[sToken.getLength() - 1] == '"')
        {
            bInStringNext = false;
            sToken = sToken.copy(0, sToken.getLength() - 1);
        }

        if (bInString)
        {
            if (bInStringNext)
            {
                sPart += OUString(' ');
                sPart += sToken;
            }
            else
            {
                sPart += sToken;
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
void DomainMapper_Impl::AppendFieldCommand(OUString& rPartOfCommand)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("appendFieldCommand");
    dmapper_logger->chars(rPartOfCommand);
    dmapper_logger->endElement();
#endif

    FieldContextPtr pContext = m_aFieldStack.top();
    OSL_ENSURE( pContext.get(), "no field context available");
    if( pContext.get() )
    {
        pContext->AppendCommand( rPartOfCommand );
    }
}


typedef std::multimap < sal_Int32, OUString > TOCStyleMap;

const FieldConversionMap_t & lcl_GetFieldConversion()
{
static FieldConversionMap_t aFieldConversionMap;
static FieldConversionMap_t aEnhancedFieldConversionMap;

static bool bFilled = false;

if(!bFilled)
{
    static const FieldConversion aFields[] =
    {
//            {OUString("ADDRESSBLOCK"),  "",                         "", FIELD_ADDRESSBLOCK  },
//            {OUString("ADVANCE"),       "",                         "", FIELD_ADVANCE       },
        {OUString("ASK"),           "SetExpression",             "SetExpression", FIELD_ASK      },
            {OUString("AUTONUM"),       "SetExpression",            "SetExpression", FIELD_AUTONUM   },
            {OUString("AUTONUMLGL"),     "SetExpression",            "SetExpression", FIELD_AUTONUMLGL },
            {OUString("AUTONUMOUT"),     "SetExpression",            "SetExpression", FIELD_AUTONUMOUT },
            {OUString("AUTHOR"),        "DocInfo.CreateAuthor",                   "", FIELD_AUTHOR       },
            {OUString("DATE"),          "DateTime",                 "", FIELD_DATE         },
            {OUString("COMMENTS"),      "DocInfo.Description",      "", FIELD_COMMENTS     },
            {OUString("CREATEDATE"),    "DocInfo.CreateDateTime",   "", FIELD_CREATEDATE   },
            {OUString("DOCPROPERTY"),   "",                         "", FIELD_DOCPROPERTY },
            {OUString("DOCVARIABLE"),   "User",                     "", FIELD_DOCVARIABLE  },
            {OUString("EDITTIME"),      "DocInfo.EditTime",         "", FIELD_EDITTIME     },
            {OUString("EQ"),            "",                         "", FIELD_EQ     },
            {OUString("FILLIN"),        "Input",                    "", FIELD_FILLIN       },
            {OUString("FILENAME"),      "FileName",                 "", FIELD_FILENAME     },
//            {OUString("FILESIZE"),      "",                         "", FIELD_FILESIZE     },
//            {OUString("FORMULA"),     "",                           "", FIELD_FORMULA },
            {OUString("FORMCHECKBOX"),     "",                           "", FIELD_FORMCHECKBOX},
            {OUString("FORMDROPDOWN"),     "DropDown",                           "", FIELD_FORMDROPDOWN},
            {OUString("FORMTEXT"),     "Input", "", FIELD_FORMTEXT},
//            {OUString("GOTOBUTTON"),    "",                         "", FIELD_GOTOBUTTON   },
            {OUString("HYPERLINK"),     "",                         "", FIELD_HYPERLINK    },
            {OUString("IF"),            "ConditionalText",          "", FIELD_IF           },
//            {OUString("INFO"),      "","", FIELD_INFO         },
//            {OUString("INCLUDEPICTURE"), "",                        "", FIELD_INCLUDEPICTURE},
            {OUString("KEYWORDS"),      "DocInfo.KeyWords",         "", FIELD_KEYWORDS     },
            {OUString("LASTSAVEDBY"),   "DocInfo.ChangeAuthor",                         "", FIELD_LASTSAVEDBY  },
            {OUString("MACROBUTTON"),   "Macro",                         "", FIELD_MACROBUTTON  },
            {OUString("MERGEFIELD"),    "Database",                 "Database", FIELD_MERGEFIELD},
            {OUString("MERGEREC"),      "DatabaseNumberOfSet",      "", FIELD_MERGEREC     },
//            {OUString("MERGESEQ"),      "",                         "", FIELD_MERGESEQ     },
            {OUString("NEXT"),          "DatabaseNextSet",          "", FIELD_NEXT         },
            {OUString("NEXTIF"),        "DatabaseNextSet",          "", FIELD_NEXTIF       },
            {OUString("PAGE"),          "PageNumber",               "", FIELD_PAGE         },
            {OUString("PAGEREF"),       "GetReference",             "", FIELD_PAGEREF      },
            {OUString("REF"),           "GetReference",             "", FIELD_REF          },
            {OUString("REVNUM"),        "DocInfo.Revision",         "", FIELD_REVNUM       },
            {OUString("SAVEDATE"),      "DocInfo.Change",           "", FIELD_SAVEDATE     },
//            {OUString("SECTION"),       "",                         "", FIELD_SECTION      },
//            {OUString("SECTIONPAGES"),  "",                         "", FIELD_SECTIONPAGES },
            {OUString("SEQ"),           "SetExpression",            "SetExpression", FIELD_SEQ          },
//            {OUString("SET"),           "","", FIELD_SET          },
//            {OUString("SKIPIF"),"",                                 "", FIELD_SKIPIF       },
//            {OUString("STYLEREF"),"",                               "", FIELD_STYLEREF     },
            {OUString("SUBJECT"),       "DocInfo.Subject",          "", FIELD_SUBJECT      },
//            {OUString("SYMBOL"),"",                                 "", FIELD_SYMBOL       },
            {OUString("TEMPLATE"),      "TemplateName",             "", FIELD_TEMPLATE},
            {OUString("TIME"),          "DateTime",                 "", FIELD_TIME         },
            {OUString("TITLE"),         "DocInfo.Title",            "", FIELD_TITLE        },
            {OUString("USERINITIALS"),  "Author",                   "", FIELD_USERINITIALS       },
//            {OUString("USERADDRESS"),   "",                         "", FIELD_USERADDRESS  },
            {OUString("USERNAME"), "Author",                   "", FIELD_USERNAME       },


            {OUString("TOC"), "com.sun.star.text.ContentIndex", "", FIELD_TOC},
            {OUString("TC"), "com.sun.star.text.ContentIndexMark", "", FIELD_TC},
            {OUString("NUMCHARS"), "CharacterCount", "", FIELD_NUMCHARS},
            {OUString("NUMWORDS"), "WordCount", "", FIELD_NUMWORDS},
            {OUString("NUMPAGES"), "PageCount", "", FIELD_NUMPAGES},

//            {OUString(""), "", "", FIELD_},

        };
        size_t nConversions = SAL_N_ELEMENTS(aFields);
        for( size_t nConversion = 0; nConversion < nConversions; ++nConversion)
        {
            aFieldConversionMap.insert( FieldConversionMap_t::value_type(
                aFields[nConversion].sWordCommand,
                aFields[nConversion] ));
        }

        bFilled = true;
    }

    return aFieldConversionMap;
}

const FieldConversionMap_t & lcl_GetEnhancedFieldConversion()
{
    static FieldConversionMap_t aEnhancedFieldConversionMap;

    static bool bFilled = false;

    if(!bFilled)
    {
        static const FieldConversion aEnhancedFields[] =
        {
            {OUString("FORMCHECKBOX"),     "FormFieldmark",                           "", FIELD_FORMCHECKBOX},
            {OUString("FORMDROPDOWN"),     "FormFieldmark",                           "", FIELD_FORMDROPDOWN},
            {OUString("FORMTEXT"),     "Fieldmark", "", FIELD_FORMTEXT},
        };

        size_t nConversions = SAL_N_ELEMENTS(aEnhancedFields);
        for( size_t nConversion = 0; nConversion < nConversions; ++nConversion)
        {
            aEnhancedFieldConversionMap.insert( FieldConversionMap_t::value_type(
                aEnhancedFields[nConversion].sWordCommand,
                aEnhancedFields[nConversion] ));
        }
    }
    return aEnhancedFieldConversionMap;
}

void DomainMapper_Impl::handleFieldAsk
    (FieldContextPtr pContext,
     PropertyNameSupplier& rPropNameSupplier,
     uno::Reference< uno::XInterface > & xFieldInterface,
     uno::Reference< beans::XPropertySet > xFieldProperties)
{
    //doesn the command contain a variable name?
    OUString sVariable, sHint;

    sVariable = lcl_ExctractAskVariableAndHint( pContext->GetCommand(),
        sHint );
    if(!sVariable.isEmpty())
    {
        // determine field master name
        uno::Reference< beans::XPropertySet > xMaster =
            FindOrCreateFieldMaster
            ("com.sun.star.text.FieldMaster.SetExpression", sVariable );
        // An ASK field is always a string of characters
        xMaster->setPropertyValue(rPropNameSupplier.GetName(PROP_SUB_TYPE), uno::makeAny(text::SetVariableType::STRING));

        // attach the master to the field
        uno::Reference< text::XDependentTextField > xDependentField
            ( xFieldInterface, uno::UNO_QUERY_THROW );
        xDependentField->attachTextFieldMaster( xMaster );

        // set input flag at the field
        xFieldProperties->setPropertyValue(
            rPropNameSupplier.GetName(PROP_IS_INPUT), uno::makeAny( true ));
        // set the prompt
        xFieldProperties->setPropertyValue(
            rPropNameSupplier.GetName(PROP_HINT),
            uno::makeAny( sHint ));
        xFieldProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_SUB_TYPE), uno::makeAny(text::SetVariableType::STRING));
        // The ASK has no field value to display
        xFieldProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_IS_VISIBLE), uno::makeAny(sal_False));
    }
    else
    {
        //don't insert the field
        //todo: maybe import a 'normal' input field here?
        xFieldInterface = 0;
    }
}

void DomainMapper_Impl::handleAutoNum
    (FieldContextPtr pContext,
    PropertyNameSupplier& rPropNameSupplier,
    uno::Reference< uno::XInterface > & xFieldInterface,
    uno::Reference< beans::XPropertySet > xFieldProperties)
{
    //create a sequence field master "AutoNr"
    uno::Reference< beans::XPropertySet > xMaster =
    FindOrCreateFieldMaster
        ("com.sun.star.text.FieldMaster.SetExpression",
        "AutoNr");

    xMaster->setPropertyValue( rPropNameSupplier.GetName(PROP_SUB_TYPE),
        uno::makeAny(text::SetVariableType::SEQUENCE));

    //apply the numbering type
    xFieldProperties->setPropertyValue(
        rPropNameSupplier.GetName(PROP_NUMBERING_TYPE),
        uno::makeAny( lcl_ParseNumberingType(pContext->GetCommand()) ));
        // attach the master to the field
    uno::Reference< text::XDependentTextField > xDependentField
        ( xFieldInterface, uno::UNO_QUERY_THROW );
    xDependentField->attachTextFieldMaster( xMaster );
}

void DomainMapper_Impl::handleAuthor
    (FieldContextPtr pContext,
    PropertyNameSupplier& rPropNameSupplier,
     uno::Reference< uno::XInterface > & /*xFieldInterface*/,
     uno::Reference< beans::XPropertySet > xFieldProperties,
     FieldId  eFieldId )
{
    if ( eFieldId != FIELD_USERINITIALS )
        xFieldProperties->setPropertyValue
            ( rPropNameSupplier.GetName(PROP_FULL_NAME), uno::makeAny( true ));

    sal_Int32 nLen = sizeof( " AUTHOR" );
    if ( eFieldId != FIELD_AUTHOR )
    {
        if (  eFieldId == FIELD_USERINITIALS )
            nLen = sizeof( " USERINITIALS" );
        else if (  eFieldId == FIELD_USERNAME )
            nLen = sizeof( " USERNAME" );
    }

    OUString sParam =
        lcl_ExtractParameter(pContext->GetCommand(), nLen );

    if(!sParam.isEmpty())
    {
        xFieldProperties->setPropertyValue(
                rPropNameSupplier.GetName( PROP_IS_FIXED ),
                uno::makeAny( true ));
        //PROP_CURRENT_PRESENTATION is set later anyway
    }
}

    void DomainMapper_Impl::handleDocProperty
        (FieldContextPtr pContext,
        PropertyNameSupplier& rPropNameSupplier,
        uno::Reference< uno::XInterface > & xFieldInterface,
        uno::Reference< beans::XPropertySet > xFieldProperties)
{
    //some docproperties should be imported as document statistic fields, some as DocInfo fields
    //others should be user fields
    OUString sParam =
        lcl_ExtractParameter(pContext->GetCommand(), sizeof(" DOCPROPERTY") );

    if(!sParam.isEmpty())
    {
        #define SET_ARABIC      0x01
        #define SET_FULL_NAME   0x02
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
        //search for a field mapping
        OUString sFieldServiceName;
        sal_uInt16 nMap = 0;
        for( ; nMap < sizeof(aDocProperties) / sizeof(DocPropertyMap);
            ++nMap )
        {
            if(sParam.equalsAscii(aDocProperties[nMap].pDocPropertyName))
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
        xFieldProperties =
            uno::Reference< beans::XPropertySet >( xFieldInterface,
                uno::UNO_QUERY_THROW);
        if( bIsCustomField )
            xFieldProperties->setPropertyValue(
                rPropNameSupplier.GetName(PROP_NAME), uno::makeAny( sParam ));
        else
        {
            if(0 != (aDocProperties[nMap].nFlags & SET_ARABIC))
                xFieldProperties->setPropertyValue(
                    rPropNameSupplier.GetName(PROP_NUMBERING_TYPE),
                    uno::makeAny( style::NumberingType::ARABIC ));
            else if(0 != (aDocProperties[nMap].nFlags & SET_FULL_NAME))
                xFieldProperties->setPropertyValue(
                    rPropNameSupplier.GetName(PROP_FULL_NAME),
                        uno::makeAny( true ));
            else if(0 != (aDocProperties[nMap].nFlags & SET_DATE))
            {
                xFieldProperties->setPropertyValue(
                    rPropNameSupplier.GetName(PROP_IS_DATE),
                        uno::makeAny( true ));
                SetNumberFormat( pContext->GetCommand(), xFieldProperties );
            }
        }
    }

#undef SET_ARABIC
#undef SET_FULL_NAME
#undef SET_DATE
}

uno::Sequence< beans::PropertyValues > lcl_createTOXLevelHyperlinks( bool bHyperlinks, OUString sChapterNoSeparator,
                                   uno::Sequence< beans::PropertyValues >aLevel,
                                   PropertyNameSupplier& rPropNameSupplier )
{
    //create a copy of the level and add two new entries - hyperlink start and end
    bool bChapterNoSeparator  = !sChapterNoSeparator.isEmpty();
    sal_Int32 nAdd = (bHyperlinks && bChapterNoSeparator) ? 4 : 2;
    uno::Sequence< beans::PropertyValues > aNewLevel( aLevel.getLength() + nAdd);
    beans::PropertyValues* pNewLevel = aNewLevel.getArray();
    if( bHyperlinks )
    {
        beans::PropertyValues aHyperlink(1);
        aHyperlink[0].Name = rPropNameSupplier.GetName( PROP_TOKEN_TYPE );
        aHyperlink[0].Value <<= rPropNameSupplier.GetName( PROP_TOKEN_HYPERLINK_START );
        pNewLevel[0] = aHyperlink;
        aHyperlink[0].Value <<= rPropNameSupplier.GetName( PROP_TOKEN_HYPERLINK_END );
        pNewLevel[aNewLevel.getLength() -1] = aHyperlink;
    }
    if( bChapterNoSeparator )
    {
        beans::PropertyValues aChapterNo(2);
        aChapterNo[0].Name = rPropNameSupplier.GetName( PROP_TOKEN_TYPE );
        aChapterNo[0].Value <<= rPropNameSupplier.GetName( PROP_TOKEN_CHAPTER_INFO );
        aChapterNo[1].Name = rPropNameSupplier.GetName( PROP_CHAPTER_FORMAT );
        //todo: is ChapterFormat::Number correct?
        aChapterNo[1].Value <<= (sal_Int16)text::ChapterFormat::NUMBER;
        pNewLevel[aNewLevel.getLength() - (bHyperlinks ? 4 : 2) ] = aChapterNo;

        beans::PropertyValues aChapterSeparator(2);
        aChapterSeparator[0].Name = rPropNameSupplier.GetName( PROP_TOKEN_TYPE );
        aChapterSeparator[0].Value <<= rPropNameSupplier.GetName( PROP_TOKEN_TEXT );
        aChapterSeparator[1].Name = rPropNameSupplier.GetName( PROP_TEXT );
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
    (FieldContextPtr pContext,
    PropertyNameSupplier& rPropNameSupplier,
     uno::Reference< uno::XInterface > & /*xFieldInterface*/,
     uno::Reference< beans::XPropertySet > /*xFieldProperties*/,
    const OUString & sTOCServiceName)
{
    OUString sValue;
    bool bTableOfFigures = false;
    bool bHyperlinks = false;
    bool bFromOutline = false;
    bool bFromEntries = false;
    sal_Int16 nMaxLevel = 10;
    OUString sTemplate;
    OUString sChapterNoSeparator;
    OUString sFigureSequence;

//                  \a Builds a table of figures but does not include the captions's label and number
    if( lcl_FindInCommand( pContext->GetCommand(), 'a', sValue ))
    { //make it a table of figures
        bTableOfFigures = true;
    }
//                  \b Uses a bookmark to specify area of document from which to build table of contents
//                    if( lcl_FindInCommand( pContext->GetCommand(), 'b', sValue ))
//                    { //todo: sValue contains the bookmark name - unsupported feature
//                    }
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
    if( lcl_FindInCommand( pContext->GetCommand(), 'p', sValue ))
        {  }
//                  \s  Builds a table of contents by using a sequence type
    if( lcl_FindInCommand( pContext->GetCommand(), 's', sValue ))
        {  }
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
                        //todo: what doesn 'the applied paragraph outline level' refer to?
    }
//                  \w Preserve tab characters within table entries
//                    if( lcl_FindInCommand( pContext->GetCommand(), 'w', sValue ))
//                    {
                        //todo: not supported
//                    }
//                  \x Preserve newline characters within table entries
//                    if( lcl_FindInCommand( pContext->GetCommand(), 'x', sValue ))
//                    {
                        //todo: unsupported
//                    }
//                  \z Hides page numbers within the table of contens when shown in Web Layout View
//                    if( lcl_FindInCommand( pContext->GetCommand(), 'z', sValue ))
//                    { //todo: unsupported feature  }

                    //if there's no option then it should be created from outline
    if( !bFromOutline && !bFromEntries && sTemplate.isEmpty()  )
        bFromOutline = true;

    uno::Reference< beans::XPropertySet > xTOC;
    if (m_xTextFactory.is())
        xTOC.set(
                m_xTextFactory->createInstance
                ( bTableOfFigures ?
                  "com.sun.star.text.IllustrationsIndex"
                  : sTOCServiceName),
                uno::UNO_QUERY_THROW);
    if (xTOC.is())
        xTOC->setPropertyValue(rPropNameSupplier.GetName( PROP_TITLE ), uno::makeAny(OUString()));
    if( !bTableOfFigures && xTOC.is() )
    {
        xTOC->setPropertyValue( rPropNameSupplier.GetName( PROP_LEVEL ), uno::makeAny( nMaxLevel ) );
        xTOC->setPropertyValue( rPropNameSupplier.GetName( PROP_CREATE_FROM_OUTLINE ), uno::makeAny( bFromOutline ));
        xTOC->setPropertyValue( rPropNameSupplier.GetName( PROP_CREATE_FROM_MARKS ), uno::makeAny( bFromEntries ));
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
                    aMap.insert( TOCStyleMap::value_type(nLevel, sStyleName) );
            }
            uno::Reference< container::XIndexReplace> xParaStyles;
            xTOC->getPropertyValue(rPropNameSupplier.GetName(PROP_LEVEL_PARAGRAPH_STYLES)) >>= xParaStyles;
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
            xTOC->setPropertyValue(rPropNameSupplier.GetName(PROP_CREATE_FROM_LEVEL_PARAGRAPH_STYLES), uno::makeAny( true ));

        }
        if(bHyperlinks  || !sChapterNoSeparator.isEmpty())
        {
            uno::Reference< container::XIndexReplace> xLevelFormats;
            xTOC->getPropertyValue(rPropNameSupplier.GetName(PROP_LEVEL_FORMAT)) >>= xLevelFormats;
            sal_Int32 nLevelCount = xLevelFormats->getCount();
                            //start with level 1, 0 is the header level
            for( sal_Int32 nLevel = 1; nLevel < nLevelCount; ++nLevel)
            {
                uno::Sequence< beans::PropertyValues > aLevel;
                xLevelFormats->getByIndex( nLevel ) >>= aLevel;

                uno::Sequence< beans::PropertyValues > aNewLevel = lcl_createTOXLevelHyperlinks(
                                                    bHyperlinks, sChapterNoSeparator,
                                                    aLevel, rPropNameSupplier );
                xLevelFormats->replaceByIndex( nLevel, uno::makeAny( aNewLevel ) );
            }
        }
    }
    else if (bTableOfFigures && xTOC.is())
    {
        if (!sFigureSequence.isEmpty())
            xTOC->setPropertyValue(rPropNameSupplier.GetName(PROP_LABEL_CATEGORY),
                                   uno::makeAny(sFigureSequence));

        if ( bHyperlinks )
        {
            uno::Reference< container::XIndexReplace> xLevelFormats;
            xTOC->getPropertyValue(rPropNameSupplier.GetName(PROP_LEVEL_FORMAT)) >>= xLevelFormats;
            uno::Sequence< beans::PropertyValues > aLevel;
            xLevelFormats->getByIndex( 1 ) >>= aLevel;

            uno::Sequence< beans::PropertyValues > aNewLevel = lcl_createTOXLevelHyperlinks(
                                                bHyperlinks, sChapterNoSeparator,
                                                aLevel, rPropNameSupplier );
            xLevelFormats->replaceByIndex( 1, uno::makeAny( aNewLevel ) );
        }
    }
    pContext->SetTOC( xTOC );
}


/*-------------------------------------------------------------------------
//the field command has to be closed (0x14 appeared)
  -----------------------------------------------------------------------*/
void DomainMapper_Impl::CloseFieldCommand()
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->element("closeFieldCommand");
#endif

    FieldContextPtr pContext = m_aFieldStack.top();
    OSL_ENSURE( pContext.get(), "no field context available");
    if( pContext.get() )
    {
        m_bSetUserFieldContent = false;
        FieldConversionMap_t aFieldConversionMap = lcl_GetFieldConversion();
        bool bCreateEnhancedField = false;

        try
        {
            uno::Reference< uno::XInterface > xFieldInterface;
            //at first determine the field type - erase leading and trailing whitespaces
            OUString sCommand( pContext->GetCommand().trim() );
            sal_Int32 nSpaceIndex = sCommand.indexOf( ' ' );
            if( 0 <= nSpaceIndex )
                sCommand = sCommand.copy( 0, nSpaceIndex );

            FieldConversionMap_t::iterator aIt = aFieldConversionMap.find(sCommand);
            if(aIt != aFieldConversionMap.end())
            {
                uno::Reference< beans::XPropertySet > xFieldProperties;
                bool bCreateField = true;
                switch (aIt->second.eFieldId)
                {
                case FIELD_HYPERLINK:
                case FIELD_DOCPROPERTY:
                case FIELD_TOC:
                case FIELD_TC:
                case FIELD_EQ:
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

                if( bCreateField || bCreateEnhancedField )
                {
                    //add the service prefix
                    OUString sServiceName("com.sun.star.text.");
                    if ( bCreateEnhancedField )
                    {
                        FieldConversionMap_t aEnhancedFieldConversionMap = lcl_GetEnhancedFieldConversion();
                        FieldConversionMap_t::iterator aEnhancedIt = aEnhancedFieldConversionMap.find(sCommand);
                        if ( aEnhancedIt != aEnhancedFieldConversionMap.end())
                            sServiceName += OUString::createFromAscii(aEnhancedIt->second.cFieldServiceName );
                    }
                    else
                    {
                        sServiceName += "TextField.";
                        sServiceName += OUString::createFromAscii(aIt->second.cFieldServiceName );
                    }

#ifdef DEBUG_DOMAINMAPPER
                    dmapper_logger->startElement("fieldService");
                    dmapper_logger->chars(sServiceName);
                    dmapper_logger->endElement();
#endif

                    if (m_xTextFactory.is())
                    {
                        xFieldInterface = m_xTextFactory->createInstance(sServiceName);
                        xFieldProperties = uno::Reference< beans::XPropertySet >( xFieldInterface, uno::UNO_QUERY_THROW);
                    }
                }
                PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
                switch( aIt->second.eFieldId )
                {
                    case FIELD_ADDRESSBLOCK: break;
                    case FIELD_ADVANCE     : break;
                    case FIELD_ASK         :
                        handleFieldAsk(pContext, rPropNameSupplier, xFieldInterface, xFieldProperties);
                    break;
                    case FIELD_AUTONUM    :
                    case FIELD_AUTONUMLGL :
                    case FIELD_AUTONUMOUT :
                        handleAutoNum(pContext, rPropNameSupplier, xFieldInterface, xFieldProperties);
                    break;
                    case FIELD_AUTHOR       :
                    case FIELD_USERNAME     :
                    case FIELD_USERINITIALS :
                        handleAuthor(pContext, rPropNameSupplier, xFieldInterface, xFieldProperties, aIt->second.eFieldId  );
                    break;
                    case FIELD_DATE:
                    if (xFieldProperties.is())
                    {
                        //not fixed,
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_IS_FIXED),
                            uno::makeAny( false ));
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_IS_DATE),
                            uno::makeAny( true ));
                        SetNumberFormat( pContext->GetCommand(), xFieldProperties );
                    }
                    break;
                    case FIELD_COMMENTS     :
                    {
                        // OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" COMMENTS") );
                        // A parameter with COMMENTS shouldn't set fixed
                        // ( or at least the binary filter doesn't )
                        // If we set fixed then we wont export a field cmd.
                        // Additionally the para in COMMENTS is more like an
                        // instruction to set the document property comments
                        // with the param ( e.g. each COMMENT with a param will
                        // overwrite the Comments document property
                        // #TODO implement the above too
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName( PROP_IS_FIXED ), uno::makeAny( false ));
                            //PROP_CURRENT_PRESENTATION is set later anyway
                    }
                    break;
                    case FIELD_CREATEDATE  :
                    {
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName( PROP_IS_DATE ), uno::makeAny( true ));
                        SetNumberFormat( pContext->GetCommand(), xFieldProperties );
                    }
                    break;
                    case FIELD_DOCPROPERTY :
                        handleDocProperty(pContext, rPropNameSupplier, xFieldInterface, xFieldProperties);
                    break;
                    case FIELD_DOCVARIABLE  :
                    {
                        OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" DOCVARIABLE") );
                        //create a user field and type
                        uno::Reference< beans::XPropertySet > xMaster =
                            FindOrCreateFieldMaster( "com.sun.star.text.FieldMaster.User", sParam );
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
                        nSpaceIndex = aCommand.indexOf(' ');
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
                                    if (GetPropertyFromStyleSheet(PROP_CHAR_HEIGHT) >>= dHeight)
                                        // Character escapement should be given in negative percents for subscripts.
                                        pCharContext->Insert(PROP_CHAR_ESCAPEMENT, uno::makeAny( sal_Int16(- 100 * nDown / dHeight) ) );
                                    appendTextPortion(aContent, pCharContext);
                                }
                            }
                        }
                    }
                    break;
                    case FIELD_FILLIN       :
                    {
                        sal_Int32 nIndex = 0;
                        if (xFieldProperties.is())
                            xFieldProperties->setPropertyValue(
                                    rPropNameSupplier.GetName(PROP_HINT), uno::makeAny( pContext->GetCommand().getToken( 1, '\"', nIndex)));
                    }
                    break;
                    case FIELD_FILENAME:
                    {
                        sal_Int32 nNumberingTypeIndex = pContext->GetCommand().indexOf("\\p");
                        if (xFieldProperties.is())
                            xFieldProperties->setPropertyValue(
                                    rPropNameSupplier.GetName(PROP_FILE_FORMAT),
                                    uno::makeAny( nNumberingTypeIndex > 0 ? text::FilenameDisplayFormat::FULL : text::FilenameDisplayFormat::NAME ));
                    }
                    break;
                    case FIELD_FILESIZE     : break;
                    case FIELD_FORMULA : break;
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
                                    lcl_handleTextField( xFieldProperties, pContext->getFFDataHandler(), rPropNameSupplier );
                            }
                        }
                        break;
                    case FIELD_GOTOBUTTON   : break;
                    case FIELD_HYPERLINK:
                    {
                        ::std::vector<OUString> aParts = pContext->GetCommandParts();
                        ::std::vector<OUString>::const_iterator aItEnd = aParts.end();
                        ::std::vector<OUString>::const_iterator aPartIt = aParts.begin();

                        OUString sURL;

                        while (aPartIt != aItEnd)
                        {
                            if ( *aPartIt == "\\l" )
                            {
                                ++aPartIt;

                                if (aPartIt == aItEnd)
                                    break;

                                sURL += OUString('#');
                                sURL += *aPartIt;
                            }
                            else if ( *aPartIt == "\\m" || *aPartIt == "\\n" )
                            {
                            }
                            else if ( *aPartIt == "\\o" || *aPartIt == "\\t" )
                            {
                                ++aPartIt;

                                if (aPartIt == aItEnd)
                                    break;
                            }
                            else
                            {
                                sURL = *aPartIt;
                            }

                            ++aPartIt;
                        }

                        if (!sURL.isEmpty())
                        {
                            pContext->SetHyperlinkURL(sURL);
                        }
                    }
                    break;
                    case FIELD_IF           : break;
                    case FIELD_INFO         : break;
                    case FIELD_INCLUDEPICTURE: break;
                    case FIELD_KEYWORDS     :
                    {
                        OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" KEYWORDS") );
                        if(!sParam.isEmpty())
                        {
                            xFieldProperties->setPropertyValue(
                                    rPropNameSupplier.GetName( PROP_IS_FIXED ), uno::makeAny( true ));
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
                                    rPropNameSupplier.GetName(PROP_MACRO_NAME), uno::makeAny( sMacro ));

                        //extract quick help text
                        if(xFieldProperties.is() && pContext->GetCommand().getLength() > nIndex + 1)
                        {
                            xFieldProperties->setPropertyValue(
                                rPropNameSupplier.GetName(PROP_HINT),
                                uno::makeAny( pContext->GetCommand().copy( nIndex )));
                        }
                    }
                    break;
                    case FIELD_MERGEFIELD  :
                    {
                        //todo: create a database field and fieldmaster pointing to a column, only
                        OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" MERGEFIELD") );
                        //create a user field and type
                        uno::Reference< beans::XPropertySet > xMaster =
                            FindOrCreateFieldMaster( "com.sun.star.text.FieldMaster.Database", sParam );

    //                    xFieldProperties->setPropertyValue(
    //                             "FieldCode",
    //                             uno::makeAny( pContext->GetCommand().copy( nIndex + 1 )));
                        uno::Reference< text::XDependentTextField > xDependentField( xFieldInterface, uno::UNO_QUERY_THROW );
                        xDependentField->attachTextFieldMaster( xMaster );
                        m_bSetUserFieldContent = true;
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
                                    rPropNameSupplier.GetName(PROP_NUMBERING_TYPE),
                                    uno::makeAny( lcl_ParseNumberingType(pContext->GetCommand()) ));
                            xFieldProperties->setPropertyValue(
                                    rPropNameSupplier.GetName(PROP_SUB_TYPE),
                                    uno::makeAny( text::PageNumberType_CURRENT ));
                        }

                    break;
                    case FIELD_PAGEREF:
                    case FIELD_REF:
                    if (xFieldProperties.is())
                    {
                        bool bPageRef = aIt->second.eFieldId == FIELD_PAGEREF;
                        OUString sBookmark = lcl_ExtractParameter(pContext->GetCommand(),
                                (bPageRef ? sizeof(" PAGEREF") : sizeof(" REF")));

                        // Do we need a GetReference (default) or a GetExpression field?
                        uno::Reference< text::XTextFieldsSupplier > xFieldsSupplier( GetTextDocument(), uno::UNO_QUERY );
                        uno::Reference< container::XNameAccess > xFieldMasterAccess = xFieldsSupplier->getTextFieldMasters();

                        if (!xFieldMasterAccess->hasByName("com.sun.star.text.FieldMaster.SetExpression." + sBookmark))
                        {
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_REFERENCE_FIELD_SOURCE),
                            uno::makeAny( sal_Int16(text::ReferenceFieldSource::BOOKMARK)) );
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_SOURCE_NAME),
                            uno::makeAny( sBookmark) );
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
                                rPropNameSupplier.GetName( PROP_REFERENCE_FIELD_PART ), uno::makeAny( nFieldPart ));
                        }
                        else
                        {
                            xFieldInterface = m_xTextFactory->createInstance("com.sun.star.text.TextField.GetExpression");
                            xFieldProperties.set(xFieldInterface, uno::UNO_QUERY);
                            xFieldProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_CONTENT), uno::makeAny(sBookmark));
                            xFieldProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_SUB_TYPE), uno::makeAny(text::SetVariableType::STRING));
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
                        OUString sSeqName = lcl_FindQuotedText(sCmd, "SEQ ", '\\');
                        sSeqName = sSeqName.trim();

                        // create a sequence field master using the sequence name
                        uno::Reference< beans::XPropertySet > xMaster = FindOrCreateFieldMaster(
                                    "com.sun.star.text.FieldMaster.SetExpression",
                                    sSeqName);

                        xMaster->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_SUB_TYPE),
                            uno::makeAny(text::SetVariableType::SEQUENCE));

                        // apply the numbering type
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_NUMBERING_TYPE),
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
                                rPropNameSupplier.GetName(PROP_CONTENT),
                                uno::makeAny(sFormula));

                        // Take care of the numeric formatting definition, default is Arabic
                        sal_Int16 nNumberingType = lcl_ParseNumberingType(pContext->GetCommand());
                        if (nNumberingType == style::NumberingType::PAGE_DESCRIPTOR)
                            nNumberingType = style::NumberingType::ARABIC;
                        xFieldProperties->setPropertyValue(
                                rPropNameSupplier.GetName(PROP_NUMBERING_TYPE),
                                uno::makeAny(nNumberingType));
                    }
                    break;
                    case FIELD_SET          : break;
                    case FIELD_SKIPIF       : break;
                    case FIELD_STYLEREF     : break;
                    case FIELD_SUBJECT      :
                    {
                        OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" SUBJECT") );
                        if(!sParam.isEmpty())
                        {
                            xFieldProperties->setPropertyValue(
                                    rPropNameSupplier.GetName( PROP_IS_FIXED ), uno::makeAny( true ));
                            //PROP_CURRENT_PRESENTATION is set later anyway
                        }
                    }
                    break;
                    case FIELD_SYMBOL       : break;
                    case FIELD_TEMPLATE: break;
                    case FIELD_TIME         :
                        SetNumberFormat( pContext->GetCommand(), xFieldProperties );
                    break;
                    case FIELD_TITLE        :
                    {
                        OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" TITLE") );
                        if(!sParam.isEmpty())
                        {
                            xFieldProperties->setPropertyValue(
                                    rPropNameSupplier.GetName( PROP_IS_FIXED ), uno::makeAny( true ));
                            //PROP_CURRENT_PRESENTATION is set later anyway
                        }
                    }
                    break;
                    case FIELD_USERADDRESS  : //todo: user address collects street, city ...
                    break;
                    case FIELD_TOC:
                        handleToc(pContext, rPropNameSupplier, xFieldInterface, xFieldProperties,
                                  OUString::createFromAscii(aIt->second.cFieldServiceName));
                    break;
                    case FIELD_TC :
                    {
                        uno::Reference< beans::XPropertySet > xTC(
                            m_xTextFactory->createInstance(
                                OUString::createFromAscii(aIt->second.cFieldServiceName)),
                                uno::UNO_QUERY_THROW);
                        OUString sTCText = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" TC") );
                        if( !sTCText.isEmpty())
                            xTC->setPropertyValue(rPropNameSupplier.GetName(PROP_ALTERNATIVE_TEXT),
                                uno::makeAny(sTCText));
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
                                xTC->setPropertyValue(rPropNameSupplier.GetName(PROP_LEVEL), uno::makeAny( (sal_Int16)nLevel ));
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
                            rPropNameSupplier.GetName(PROP_NUMBERING_TYPE),
                            uno::makeAny( lcl_ParseNumberingType(pContext->GetCommand()) ));
                    break;
                }
            }
            //set the text field if there is any
            pContext->SetTextField( uno::Reference< text::XTextField >( xFieldInterface, uno::UNO_QUERY ) );
        }
        catch( const uno::Exception& e )
        {
            SAL_WARN( "writerfilter", "Exception in CloseFieldCommand(): " << e.Message );
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


void DomainMapper_Impl::SetFieldResult( OUString& rResult )
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("setFieldResult");
    dmapper_logger->chars(rResult);
#endif

    FieldContextPtr pContext = m_aFieldStack.top();
    OSL_ENSURE( pContext.get(), "no field context available");
    if( pContext.get() )
    {
        uno::Reference<text::XTextField> xTextField = pContext->GetTextField();
        try
        {
            PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
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
                                rPropNameSupplier.GetName(PROP_CONTENT),
                             uno::makeAny( rResult ));
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
                                    rPropNameSupplier.GetName(PROP_CONTENT)));
                            aValue >>= sValue;
                        }
                        xFieldProperties->setPropertyValue(
                                rPropNameSupplier.GetName(bIsSetExpression && sValue.isEmpty()? PROP_CONTENT : PROP_CURRENT_PRESENTATION),
                             uno::makeAny( rResult ));
                    }
                }
                catch( const beans::UnknownPropertyException& )
                {
                    //some fields don't have a CurrentPresentation (DateTime)
                }
            }
        }
        catch( const uno::Exception& )
        {

        }
    }
}

void DomainMapper_Impl::SetFieldFFData(FFDataHandler::Pointer_t pFFDataHandler)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("setFieldFFData");
#endif

    if (m_aFieldStack.size())
    {
        FieldContextPtr pContext = m_aFieldStack.top();
        if (pContext.get())
        {
            pContext->setFFDataHandler(pFFDataHandler);
        }
    }

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement();
#endif
}

/*-------------------------------------------------------------------------
//the end of field is reached (0x15 appeared) - the command might still be open
  -----------------------------------------------------------------------*/
void DomainMapper_Impl::PopFieldContext()
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->element("popFieldContext");
#endif

    if (m_aFieldStack.empty())
        return;

    FieldContextPtr pContext = m_aFieldStack.top();
    OSL_ENSURE( pContext.get(), "no field context available");
    if( pContext.get() )
    {
        if( !pContext->IsCommandCompleted() )
            CloseFieldCommand();

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
                    xCrsr->gotoEnd( true );
                    xToInsert->attach( uno::Reference< text::XTextRange >( xCrsr, uno::UNO_QUERY_THROW ));
                }
                else
                {
                    xToInsert = uno::Reference< text::XTextContent >(pContext->GetTC(), uno::UNO_QUERY);
                    if( !xToInsert.is() )
                        xToInsert = uno::Reference< text::XTextContent >(pContext->GetTextField(), uno::UNO_QUERY);
                    if( xToInsert.is() )
                    {
                        uno::Sequence<beans::PropertyValue> aValues;
                        // Character properties of the field show up here the
                        // last (always empty) run. Inherit character
                        // properties from there.
                        if (m_pLastCharacterContext.get())
                            aValues = m_pLastCharacterContext->GetPropertyValues();
                        appendTextContent(xToInsert, aValues);
                    }
                    else
                    {
                        FormControlHelper::Pointer_t pFormControlHelper(pContext->getFormControlHelper());
                        if (pFormControlHelper.get() != NULL && pFormControlHelper->hasFFDataHandler() )
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
                            PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
                            xCrsr->gotoEnd( true );

                            uno::Reference< beans::XPropertySet > xCrsrProperties( xCrsr, uno::UNO_QUERY_THROW );
                            xCrsrProperties->setPropertyValue(rPropNameSupplier.GetName(PROP_HYPER_LINK_U_R_L), uno::
                                                              makeAny(pContext->GetHyperlinkURL()));
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
        //
        //TOCs have to include all the imported content
        //...
    }
    //remove the field context
    m_aFieldStack.pop();
}


void DomainMapper_Impl::AddBookmark( const OUString& rBookmarkName, const OUString& rId )
{
    if (m_aTextAppendStack.empty())
        return;
    uno::Reference< text::XTextAppend >  xTextAppend = m_aTextAppendStack.top().xTextAppend;
    BookmarkMap_t::iterator aBookmarkIter = m_aBookmarkMap.find( rId );
    //is the bookmark name already registered?
    try
    {
        if( aBookmarkIter != m_aBookmarkMap.end() )
        {
            static const OUString sBookmarkService("com.sun.star.text.Bookmark");
            if (m_xTextFactory.is())
            {
                uno::Reference< text::XTextContent > xBookmark( m_xTextFactory->createInstance( sBookmarkService ), uno::UNO_QUERY_THROW );
                uno::Reference< text::XTextCursor > xCursor;
                uno::Reference< text::XText > xText = aBookmarkIter->second.m_xTextRange->getText();
                if( aBookmarkIter->second.m_bIsStartOfText )
                    xCursor = xText->createTextCursorByRange( xText->getStart() );
                else
                {
                    xCursor = xText->createTextCursorByRange( aBookmarkIter->second.m_xTextRange );
                    xCursor->goRight( 1, false );
                }

                xCursor->gotoRange( xTextAppend->getEnd(), true );
                uno::Reference< container::XNamed > xBkmNamed( xBookmark, uno::UNO_QUERY_THROW );
                //todo: make sure the name is not used already!
                if ( !aBookmarkIter->second.m_sBookmarkName.isEmpty() )
                    xBkmNamed->setName( aBookmarkIter->second.m_sBookmarkName );
                else
                    xBkmNamed->setName( rBookmarkName );
                xTextAppend->insertTextContent( uno::Reference< text::XTextRange >( xCursor, uno::UNO_QUERY_THROW), xBookmark, !xCursor->isCollapsed() );
            }
            m_aBookmarkMap.erase( aBookmarkIter );
        }
        else
        {
            //otherwise insert a text range as marker
            bool bIsStart = true;
            uno::Reference< text::XTextRange > xCurrent;
            if (xTextAppend.is())
            {
                uno::Reference< text::XTextCursor > xCursor = xTextAppend->createTextCursorByRange( xTextAppend->getEnd() );
                bIsStart = !xCursor->goLeft(1, false);
                xCurrent = xCursor->getStart();
            }
            m_aBookmarkMap.insert(BookmarkMap_t::value_type( rId, BookmarkInsertPosition( bIsStart, rBookmarkName, xCurrent ) ));
        }
    }
    catch( const uno::Exception& )
    {
        //TODO: What happens to bookmarks where start and end are at different XText objects?
    }
}

void DomainMapper_Impl::AddAnnotationPosition(const bool bStart)
{
    if (m_aTextAppendStack.empty())
        return;

    // Create a cursor, pointing to the current position.
    uno::Reference<text::XTextAppend>  xTextAppend = m_aTextAppendStack.top().xTextAppend;
    uno::Reference<text::XTextRange> xCurrent;
    if (xTextAppend.is())
    {
        uno::Reference<text::XTextCursor> xCursor = xTextAppend->createTextCursorByRange(xTextAppend->getEnd());
        xCurrent = xCursor->getStart();
    }

    // And save it, to be used by PopAnnotation() later.
    if (bStart)
        m_aAnnotationPosition.m_xStart = xCurrent;
    else
        m_aAnnotationPosition.m_xEnd = xCurrent;
}

GraphicImportPtr DomainMapper_Impl::GetGraphicImport(GraphicImportType eGraphicImportType)
{
    if(!m_pGraphicImport)
        m_pGraphicImport.reset( new GraphicImport( m_xComponentContext, m_xTextFactory, m_rDMapper, eGraphicImportType ) );
    return m_pGraphicImport;
}
/*-------------------------------------------------------------------------
    reset graphic import if the last import resulted in a shape, not a graphic
  -----------------------------------------------------------------------*/
void DomainMapper_Impl::ResetGraphicImport()
{
    m_pGraphicImport.reset();
}


void  DomainMapper_Impl::ImportGraphic(writerfilter::Reference< Properties >::Pointer_t ref, GraphicImportType eGraphicImportType)
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

    //insert it into the document at the current cursor position
    OSL_ENSURE( xTextContent.is(), "DomainMapper_Impl::ImportGraphic");
    if( xTextContent.is())
        appendTextContent( xTextContent, uno::Sequence< beans::PropertyValue >() );

    m_pGraphicImport.reset();
}



void DomainMapper_Impl::SetLineNumbering( sal_Int32 nLnnMod, sal_Int32 nLnc, sal_Int32 ndxaLnn )
{
    if( !m_bLineNumberingSet )
    {
        const PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

        try
        {
            uno::Reference< text::XLineNumberingProperties > xLineProperties( m_xTextDocument, uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xProperties = xLineProperties->getLineNumberingProperties();
            uno::Any aTrue( uno::makeAny( true ));
            xProperties->setPropertyValue( rPropNameSupplier.GetName( PROP_IS_ON                  ), aTrue);
            xProperties->setPropertyValue( rPropNameSupplier.GetName( PROP_COUNT_EMPTY_LINES      ), aTrue );
            xProperties->setPropertyValue( rPropNameSupplier.GetName( PROP_COUNT_LINES_IN_FRAMES  ), uno::makeAny( false ) );
            xProperties->setPropertyValue( rPropNameSupplier.GetName( PROP_INTERVAL               ), uno::makeAny( static_cast< sal_Int16 >( nLnnMod )));
            xProperties->setPropertyValue( rPropNameSupplier.GetName( PROP_DISTANCE               ), uno::makeAny( ConversionHelper::convertTwipToMM100(ndxaLnn) ));
            xProperties->setPropertyValue( rPropNameSupplier.GetName( PROP_NUMBER_POSITION        ), uno::makeAny( style::LineNumberPosition::LEFT));
            xProperties->setPropertyValue( rPropNameSupplier.GetName( PROP_NUMBERING_TYPE         ), uno::makeAny( style::NumberingType::ARABIC));
            xProperties->setPropertyValue( rPropNameSupplier.GetName( PROP_RESTART_AT_EACH_PAGE   ), uno::makeAny( nLnc == 0 ));
        }
        catch( const uno::Exception& )
        {}
    }
    m_bLineNumberingSet = true;
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
        case PAGE_MAR_GUTTER : m_aPageMargins.gutter  = nValue; break;
    }
}



_PageMar::_PageMar()
{
    header = footer = ConversionHelper::convertTwipToMM100(sal_Int32(720));
    top = bottom = ConversionHelper::convertTwipToMM100( sal_Int32(1440));
    // This is strange, the RTF spec says it's 1800, but it's clearly 1440 in Word
    // OOXML seems not to specify a default value
    right = left = ConversionHelper::convertTwipToMM100( sal_Int32(1440));
    gutter = 0;
}



void DomainMapper_Impl::RegisterFrameConversion(
        uno::Reference< text::XTextRange >      xFrameStartRange,
        uno::Reference< text::XTextRange >      xFrameEndRange,
        uno::Sequence< beans::PropertyValue >   aFrameProperties
        )
{
    OSL_ENSURE(
        !m_aFrameProperties.getLength() && !m_xFrameStartRange.is() && !m_xFrameEndRange.is(),
        "frame properties not removed");
    m_aFrameProperties = aFrameProperties;
    m_xFrameStartRange = xFrameStartRange;
    m_xFrameEndRange   = xFrameEndRange;
}


bool DomainMapper_Impl::ExecuteFrameConversion()
{
    bool bRet = false;
    if( m_xFrameStartRange.is() && m_xFrameEndRange.is() )
    {
        bRet = true;
        try
        {
            uno::Reference< text::XTextAppendAndConvert > xTextAppendAndConvert( GetTopTextAppend(), uno::UNO_QUERY_THROW );
            xTextAppendAndConvert->convertToTextFrame(
                m_xFrameStartRange,
                m_xFrameEndRange,
                m_aFrameProperties );
        }
        catch( const uno::Exception& rEx)
        {
            SAL_WARN( "writerfilter", "Exception caught when converting to frame: " + rEx.Message );
            bRet = false;
        }
        m_xFrameStartRange = 0;
        m_xFrameEndRange = 0;
        m_aFrameProperties.realloc( 0 );
    }
    return bRet;
}

void DomainMapper_Impl::AddNewRedline(  )
{
    RedlineParamsPtr pNew( new RedlineParams );
    pNew->m_nToken = ooxml::OOXML_mod;
    if ( !m_bIsParaChange )
    {
        m_aRedlines.top().push_back( pNew );
    }
    else
    {
        m_pParaRedline.swap( pNew );
    }
}

RedlineParamsPtr DomainMapper_Impl::GetTopRedline(  )
{
    RedlineParamsPtr pResult;
    if ( !m_bIsParaChange && m_aRedlines.top().size(  ) > 0 )
        pResult = m_aRedlines.top().back(  );
    else if ( m_bIsParaChange )
        pResult = m_pParaRedline;
    return pResult;
}

sal_Int32 DomainMapper_Impl::GetCurrentRedlineToken(  )
{
    sal_Int32 nToken = 0;
    RedlineParamsPtr pCurrent( GetTopRedline(  ) );
    if ( pCurrent.get(  ) )
        nToken = pCurrent->m_nToken;
    return nToken;
}

void DomainMapper_Impl::SetCurrentRedlineAuthor( OUString sAuthor )
{
    if (!m_xAnnotationField.is())
    {
        RedlineParamsPtr pCurrent( GetTopRedline(  ) );
        if ( pCurrent.get(  ) )
            pCurrent->m_sAuthor = sAuthor;
    }
    else
        m_xAnnotationField->setPropertyValue("Author", uno::makeAny(sAuthor));
}

void DomainMapper_Impl::SetCurrentRedlineInitials( OUString sInitials )
{
    if (m_xAnnotationField.is())
        m_xAnnotationField->setPropertyValue("Initials", uno::makeAny(sInitials));
}

void DomainMapper_Impl::SetCurrentRedlineDate( OUString sDate )
{
    if (!m_xAnnotationField.is())
    {
        RedlineParamsPtr pCurrent( GetTopRedline(  ) );
        if ( pCurrent.get(  ) )
            pCurrent->m_sDate = sDate;
    }
    else
        m_xAnnotationField->setPropertyValue("DateTimeValue", uno::makeAny(lcl_DateStringToDateTime(sDate)));
}

void DomainMapper_Impl::SetCurrentRedlineId( sal_Int32 sId )
{
    RedlineParamsPtr pCurrent( GetTopRedline(  ) );
    if ( pCurrent.get(  ) )
        pCurrent->m_nId = sId;
}

void DomainMapper_Impl::SetCurrentRedlineToken( sal_Int32 nToken )
{
    RedlineParamsPtr pCurrent( GetTopRedline(  ) );
    if ( pCurrent.get(  ) )
        pCurrent->m_nToken = nToken;
}



void DomainMapper_Impl::RemoveCurrentRedline( )
{
    if ( m_aRedlines.top().size( ) > 0 )
    {
        m_aRedlines.top().pop_back( );
    }
}

void DomainMapper_Impl::ResetParaRedline( )
{
    if ( m_pParaRedline.get( ) )
    {
        RedlineParamsPtr pEmpty;
        m_pParaRedline.swap( pEmpty );
    }
}



void DomainMapper_Impl::ApplySettingsTable()
{
    if( m_pSettingsTable && m_xTextFactory.is() )
    {
        try
        {
            uno::Reference< beans::XPropertySet > xTextDefaults(m_xTextFactory->createInstance("com.sun.star.text.Defaults"), uno::UNO_QUERY_THROW );
            sal_Int32 nDefTab = m_pSettingsTable->GetDefaultTabStop();
            xTextDefaults->setPropertyValue( PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_TAB_STOP_DISTANCE ), uno::makeAny(nDefTab) );
            if (m_pSettingsTable->GetLinkStyles())
            {
                PropertyNameSupplier& rSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
                // If linked styles are enabled, set paragraph defaults from Word's default template
                xTextDefaults->setPropertyValue(rSupplier.GetName(PROP_PARA_BOTTOM_MARGIN), uno::makeAny(ConversionHelper::convertTwipToMM100(200)));
                style::LineSpacing aSpacing;
                aSpacing.Mode = style::LineSpacingMode::PROP;
                aSpacing.Height = sal_Int16(115);
                xTextDefaults->setPropertyValue(rSupplier.GetName(PROP_PARA_LINE_SPACING), uno::makeAny(aSpacing));
            }

            if (m_pSettingsTable->GetZoomFactor())
            {
                uno::Sequence<beans::PropertyValue> aViewProps(3);
                aViewProps[0].Name = "ZoomFactor";
                aViewProps[0].Value <<= m_pSettingsTable->GetZoomFactor();
                aViewProps[1].Name = "VisibleBottom";
                aViewProps[1].Value <<= sal_Int32(0);
                aViewProps[2].Name = "ZoomType";
                aViewProps[2].Value <<= sal_Int16(0);

                uno::Reference<container::XIndexContainer> xBox = document::IndexedPropertyValues::create(m_xComponentContext);
                xBox->insertByIndex(sal_Int32(0), uno::makeAny(aViewProps));
                uno::Reference<container::XIndexAccess> xIndexAccess(xBox, uno::UNO_QUERY);
                uno::Reference<document::XViewDataSupplier> xViewDataSupplier(m_xTextDocument, uno::UNO_QUERY);
                xViewDataSupplier->setViewData(xIndexAccess);
            }

            uno::Reference< beans::XPropertySet > xSettings(m_xTextFactory->createInstance("com.sun.star.document.Settings"), uno::UNO_QUERY);
            if (m_pSettingsTable->GetUsePrinterMetrics())
                xSettings->setPropertyValue("PrinterIndependentLayout", uno::makeAny(document::PrinterIndependentLayout::DISABLED));
            if( m_pSettingsTable->GetEmbedTrueTypeFonts())
                xSettings->setPropertyValue( PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_EMBED_FONTS ), uno::makeAny(true) );
            if( m_pSettingsTable->GetEmbedSystemFonts())
                xSettings->setPropertyValue( PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_EMBED_SYSTEM_FONTS ), uno::makeAny(true) );
            xSettings->setPropertyValue("AddParaTableSpacing", uno::makeAny(m_pSettingsTable->GetDoNotUseHTMLParagraphAutoSpacing()));
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
        OUString aStyle = GetCurrentParaStyleId();
        if (aStyle.isEmpty() || GetTopContextType() != CONTEXT_PARAGRAPH)
            return xRet;
        const StyleSheetEntryPtr pEntry = GetStyleSheetTable()->FindStyleSheetByISTD(aStyle);
        if (!pEntry)
            return xRet;
        const StyleSheetPropertyMap* pStyleSheetProperties = dynamic_cast<const StyleSheetPropertyMap*>(pEntry ? pEntry->pProperties.get() : 0);
        sal_Int32 nListId = pStyleSheetProperties->GetListId();
        if (nListId < 0)
            return xRet;
        if (pListLevel)
            *pListLevel = pStyleSheetProperties->GetListLevel();

        // So we are in a paragraph style and it has numbering. Look up the relevant numbering rules.
        OUString aListName = ListDef::GetStyleName(nListId);
        uno::Reference< style::XStyleFamiliesSupplier > xStylesSupplier(GetTextDocument(), uno::UNO_QUERY);
        uno::Reference< container::XNameAccess > xStyleFamilies = xStylesSupplier->getStyleFamilies();
        uno::Reference<container::XNameAccess> xNumberingStyles;
        xStyleFamilies->getByName("NumberingStyles") >>= xNumberingStyles;
        uno::Reference<beans::XPropertySet> xStyle(xNumberingStyles->getByName(aListName), uno::UNO_QUERY);
        xRet.set(xStyle->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    }
    catch( const uno::Exception& )
    {
    }
    return xRet;
}

uno::Reference<beans::XPropertySet> DomainMapper_Impl::GetCurrentNumberingCharStyle()
{
    uno::Reference<beans::XPropertySet> xRet;
    try
    {
        sal_Int32 nListLevel = -1;
        uno::Reference<container::XIndexAccess> xLevels = GetCurrentNumberingRules(&nListLevel);
        if (!xLevels.is())
            return xRet;
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
    SectionPropertyMap* pSectionContext = 0;
    //the section context is not availabe before the first call of startSectionGroup()
    if( !IsAnyTableImport() )
    {
        PropertyMapPtr pContext = GetTopContextOfType(CONTEXT_SECTION);
        OSL_ENSURE(pContext.get(), "Section context is not in the stack!");
        pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
    }

    return pSectionContext;
}

void DomainMapper_Impl::deferCharacterProperty( sal_Int32 id, com::sun::star::uno::Any value )
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

sal_Int32 DomainMapper_Impl::getCurrentNumberingProperty(OUString aProp)
{
    sal_Int32 nRet = 0;

    PropertyMap::iterator it = m_pTopContext->find(PropertyDefinition( PROP_NUMBERING_RULES ) );
    uno::Reference<container::XIndexAccess> xNumberingRules;
    if (it != m_pTopContext->end())
        xNumberingRules.set(it->second, uno::UNO_QUERY);
    it = m_pTopContext->find(PropertyDefinition( PROP_NUMBERING_LEVEL ) );
    sal_Int32 nNumberingLevel = -1;
    if (it != m_pTopContext->end())
        it->second >>= nNumberingLevel;
    if (xNumberingRules.is() && nNumberingLevel != -1)
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

bool DomainMapper_Impl::IsNewDoc()
{
    return m_bIsNewDoc;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
