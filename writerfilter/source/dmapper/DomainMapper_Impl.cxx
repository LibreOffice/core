/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DomainMapper_Impl.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:50:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include <DomainMapper_Impl.hxx>
#include <ConversionHelper.hxx>
#include <DomainMapperTableHandler.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/LineNumberPosition.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/ChapterFormat.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/text/UserDataPart.hpp>
#include <com/sun/star/text/SetVariableType.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XLineNumberingProperties.hpp>
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/style/DropCapFormat.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/XNumberFormats.hpp>
#include <rtl/ustrbuf.hxx>
#include <rtl/string.h>
#include <resourcemodel/QNameToString.hxx>


#include <map>

using namespace ::com::sun::star;
using namespace ::rtl;
namespace writerfilter {
namespace dmapper{
/*-- 08.09.2006 09:39:50---------------------------------------------------

  -----------------------------------------------------------------------*/
//defaultswitch { Upper FirstCap Lower }
//Numberingswitch    { Arabic, ALPHABETIC, ...} see lcl_ParseNumberingType
enum FieldId
{
    /* ADDRESSBLOCK \d \* MERGEFORMAT -> Addressblock completely unsupported*/
    FIELD_ADDRESSBLOCK
    /* ADVANCE \d downvalue \l leftvalue \r rightvalue \u upvalue \x xvalue \y yvalue -> unsupported*/
    ,FIELD_ADVANCE
    /* ASK bookmarkname "hint" \d defaultanswer \o \* MERGEFORMAT ->
     the hint is not always quoted, inputfield with default answer, prompt before merge (\o)
     */
    ,FIELD_ASK
    /* AUTONUM \* Numberingswitch ->
     mapped to sequence field "AutoNr"
     */
    ,FIELD_AUTONUM
    /* AUTONUMLGL \* Numberingswitch ->
     mapped to sequence field "AutoNr"
     */
    ,FIELD_AUTONUMLGL
    /* AUTONUMOUT \* Numberingswitch ->
        mapped to sequence field "AutoNr"
     */
    ,FIELD_AUTONUMOUT
    /* AUTHOR NewAuthor \* defaultswitch \* MERGEFORMAT ->
        mapped to sequence field "AutoNr"
     */
    ,FIELD_AUTHOR
    /* COMMENTS "comment" \* MERGEFORMAT ->
        Docinfo-Comments
     */
    ,FIELD_COMMENTS
    /* CREATEDATE \h \* MERGEFORMAT ->
     docinfo-created-date
     */
    ,FIELD_CREATEDATE
    /* DATE \@ "number format" \s \* MERGEFORMAT ->
        ww8filterimprovement: multiple languages now supported
     */
    ,FIELD_DATE
    /* DOCPROPERTY propertyname \* MERGEFORMAT ->
        ww8filterimprovement: some fields imported as functionally equivalent fields if possible,
        the others imported as UserField
     */
    ,FIELD_DOCPROPERTY
    /* DOCVARIABLE Name \* MERGEFORMAT ->
     ww8filterimprovement: now imported as user fields
     */
    ,FIELD_DOCVARIABLE
    /* EDITTIME \# "displayformat" \* Numberingswitch \* MERGEFORMAT ->
        DocInfo-Modified-Date
        ww8filterimprovement: multiple languages now supported
     */
    ,FIELD_EDITTIME
    /* FILLIN "text to fill in" \d defaultanswer \o \* MERGEFORMAT ->
        Function-InputField
     */
    ,FIELD_FILLIN
    /* FILENAME \p \* * MERGEFORMAT ->
        file name (\p with path)
     */
    ,FIELD_FILENAME
    /* FILESIZE \* NumberingType \* MERGEFORMAT ->
     not imported in old ww8 filter, see lcl_ParseNumberingType
     todo find alternative field
     */
    ,FIELD_FILESIZE
    /* =formula \# "number format"
     todo find alternative field
     */
    ,FIELD_FORMULA
    /* GOTOBUTTON text \* MERGEFORMAT ->
        not imported in old ww8 filter
        todo find alternative field
     */
    ,FIELD_GOTOBUTTON
    /* HYPERLINK "link" \* MERGEFORMAT ->
        not imported in old ww8 filter
        ww8filterimprovement: now imported as hyperlink
     */
    ,FIELD_HYPERLINK
    /* IF condition "then text" "else text" ->
        not imported in old ww8 filter
        ww8filterimprovement: now imported
        todo: condition, if text, else text still missing
     */
    ,FIELD_IF
    /* INFO NameOfInfo \* MERGEFORMAT -> old
        todo: filter imports wrong?
     */
    ,FIELD_INFO
    /* INCLUDEPICTURE path \* MERGEFORMAT->
     old filter imports an embedded picture
     todo: not yet supported
     */
    ,FIELD_INCLUDEPICTURE
    /* KEYWORDS keyword \* defaultswitch \* Numberingswitch \* MERGEFORMAT ->
     DocInfo Keywords
     */
    ,FIELD_KEYWORDS
    /* LASTSAVEDBY \* MERGEFORMAT ->
     DocInfo-Modified-Author
     */
    ,FIELD_LASTSAVEDBY
    /* MACROBUTTON MacroName quick help text ->
     Macro field
     */
    ,FIELD_MACROBUTTON
    /* MERGEFIELD ColumName \b prefix \f suffix \* MERGEFORMAT ->
        ww8filterimprovement: column-only API now upporterd
     */
    ,FIELD_MERGEFIELD
    /* MERGEREC \* MERGEFORMAT ->
     RecordNumber field, maybe without db name
     todo: currently unchecked
     */
    ,FIELD_MERGEREC
    /* MERGESEQ \* MERGEFORMAT ->
     not imported in old ww8 filter
     ww8filterimprovement: now imported
     todo: currently unchecked
     */
    ,FIELD_MERGESEQ
    /* NEXT text ->
     Next record
     todo: currently unchecked
     */
    ,FIELD_NEXT
    /* NEXTIF condition
     todo: condition not imported
     */
    ,FIELD_NEXTIF
    /* PAGE \* Numberingswitch \* MERGEFORMAT ->
     see lcl_ParseNumberingType
     */
    ,FIELD_PAGE
    /* REF targetbkm \f \* MERGEFORMAT ->
        imports a ShowVariable (bookmarkname)?
        \h hyerlink to paragraph
        \p relative to para above/below
        \f refenence number
        \d separator number separator
        \n paragraph number
        \r paragraph number in relative context
        \t suppres non delimiters
        \w paragraph number in full context
        \* Upper/Lower...
     */
    ,FIELD_REF          //
    /* REVNUM \* Numberingswitch \* MERGEFORMAT ->
     DocInfo-revision number
     */
    ,FIELD_REVNUM
    /* SAVEDATE \@ "NumberFormat"\* MERGEFORMAT ->
     DocInfo-modified-date
     */
    ,FIELD_SAVEDATE
    /* SECTION \* NumberFormat \* MERGEFORMAT ->
        not imported in old ww8 filter see lcl_ParseNumberingType
        todo: find alternative
     */
    ,FIELD_SECTION
    /* SECTIONPAGES \* NumberFormat \* MERGEFORMAT ->
     not imported in old ww8 filter see lcl_ParseNumberingType
        todo: find alternative
     */
    ,FIELD_SECTIONPAGES
    /* SEQ sequencename \h \c \n \r \s \* MERGEFORMAT ->
     number range name:sequencename value:sequencename+1
     todo: only partially implemented, switches unsupported
     */
    ,FIELD_SEQ
    /* SET bookmarkname newtext \* MERGEFORMAT ->
     SetVariable bookmarkname = newtext
     todo: not implemented yet
     */
    ,FIELD_SET
    /* SKIPIF condition \* MERGEFORMAT ->
     ??
     todo: not implemented yet
     */
    ,FIELD_SKIPIF
    /* STYLEREF stylename \* MERGEFORMAT ->
     not imported in old ww8 filter
     todo: add an equivalent field type
     */
    ,FIELD_STYLEREF
    /* SUBJECT subject \* Defaultswitch \* MERGEFORMAT ->
     DocInfo - subject
     */
    ,FIELD_SUBJECT
    /* SYMBOL symbolnumber \* MERGEFORMAT ->
     inserts a special char (symbolnumber)
     todo: find alternative
     */
    ,FIELD_SYMBOL
    /* TEMPLATE \* Defaultswitch \* MERGEFORMAT
     TemplateName field
     */
    ,FIELD_TEMPLATE
    /* TIME \@ "number format" \* MERGEFORMAT
     ww8filterimprovement: multiple languages now supported
     */
    ,FIELD_TIME
    /* TITLE \* Defaultswitch \* MERGEFORMAT ->
     DocInfo-title
     */
    ,FIELD_TITLE
    /* USERINITIALS newinitials \* MERGEFORMAT ->
     ExtendedUser field (SHORTCUT)
     */
    ,FIELD_USERINITIALS
    /* USERADDRESS \* MERGEFORMAT ->
     not imported in old ww8 filter
        todo: find alternative
     */
    ,FIELD_USERADDRESS
    /* USERNAME newusername \* MERGEFORMAT ->
     not imported in old ww8 filter
     todo: import as extended user field(s)
     */
    ,FIELD_USERNAME
    /*
    TOC options:
    \a Builds a table of figures but does not include the captions's label and number
    \b Uses a bookmark to specify area of document from which to build table of contents
    \c Builds a table of figures of the given label
    \d Defines the separator between sequence and page numbers
    \f Builds a table of contents using TC entries instead of outline levels
    \h Hyperlinks the entries and page numbers within the table of contents
    \l Defines the TC entries field level used to build a table of contents
    \n Builds a table of contents or a range of entries, sucah as “1-9”, in a table of contents without page numbers
    \o  Builds a table of contents by using outline levels instead of TC entries
    \p Defines the separator between the table entry and its page number
    \s  Builds a table of contents by using a sequence type
    \t  Builds a table of contents by using style names other than the standard outline styles
    \u  Builds a table of contents by using the applied paragraph outline level
    \w Preserve tab characters within table entries
    \x Preserve newline characters within table entries
    \z Hides page numbers within the table of contens when shown in Web Layout View
     */
    ,FIELD_TOC
    /*
     TOC entry: “text”
        \f TC entry in doc with multiple tables
        \l Outline Level
        \n Suppress page numbers
     example: TOC "EntryText \f \l 2 \n
     */
    ,FIELD_TC
    /* document statistic - number of characters
     */
    ,FIELD_NUMCHARS
    /* document statistic - number of words
     */
    ,FIELD_NUMWORDS
    /* document statistic - number of pages
     */
    ,FIELD_NUMPAGES
};
struct FieldConversion
{
    ::rtl::OUString     sWordCommand;
    const sal_Char*     cFieldServiceName;
    const sal_Char*     cFieldMasterServiceName;
    FieldId             eFieldId;
};

typedef ::std::map< ::rtl::OUString, FieldConversion>
            FieldConversionMap_t;

/*-- 18.07.2006 08:56:55---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 FIB::GetData( Id nName )
{
    if( nName >= NS_rtf::LN_WIDENT && nName <= NS_rtf::LN_LCBSTTBFUSSR)
        return aFIBData[nName - NS_rtf::LN_WIDENT];
    OSL_ENSURE( false, "invalid index in FIB");
    return -1;
}
/*-- 18.07.2006 08:56:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void FIB::SetData( Id nName, sal_Int32 nValue )
{
    OSL_ENSURE( nName >= NS_rtf::LN_WIDENT && nName <= NS_rtf::LN_LCBSTTBFUSSR, "invalid index in FIB");
    if( nName >= NS_rtf::LN_WIDENT && nName <= NS_rtf::LN_LCBSTTBFUSSR)
        aFIBData[nName - NS_rtf::LN_WIDENT] = nValue;
}
/*-- 01.09.2006 10:22:03---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapper_Impl::DomainMapper_Impl(
            DomainMapper& rDMapper,
            uno::Reference < uno::XComponentContext >  xContext,
            uno::Reference< lang::XComponent >  xModel,
            SourceDocumentType eDocumentType) :
        m_eDocumentType( eDocumentType ),
        m_rDMapper( rDMapper ),
        m_xTextDocument( xModel, uno::UNO_QUERY ),
        m_xTextFactory( xModel, uno::UNO_QUERY ),
        m_xComponentContext( xContext ),
        m_bFieldMode( false ),
        m_bSetUserFieldContent( false ),
        m_bIsFirstSection( true ),
        m_bIsColumnBreakDeferred( false ),
        m_bIsPageBreakDeferred( false ),
        m_TableManager( eDocumentType == DOCUMENT_OOXML ),
        m_nCurrentTabStopIndex( 0 ),
        m_sCurrentParaStyleId(),
        m_bInStyleSheetImport( false ),
        m_bInAnyTableImport( false ),
        m_bLineNumberingSet( false )
{
    GetBodyText();
    uno::Reference< text::XTextAppendAndConvert > xBodyTextAppendAndConvert = uno::Reference< text::XTextAppendAndConvert >( m_xBodyText, uno::UNO_QUERY );
    m_aTextAppendStack.push(xBodyTextAppendAndConvert);

    TableDataHandler_t::Pointer_t pTableHandler
        (new DomainMapperTableHandler(xBodyTextAppendAndConvert));
    m_TableManager.setHandler(pTableHandler);

    m_TableManager.startLevel();
}
/*-- 01.09.2006 10:22:28---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapper_Impl::~DomainMapper_Impl()
{
    m_TableManager.endLevel();
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< container::XNameContainer >    DomainMapper_Impl::GetPageStyles()
{
    if(!m_xPageStyles.is())
    {
        uno::Reference< style::XStyleFamiliesSupplier > xSupplier( m_xTextDocument, uno::UNO_QUERY );
        xSupplier->getStyleFamilies()->getByName(::rtl::OUString::createFromAscii("PageStyles")) >>= m_xPageStyles;
    }
    return m_xPageStyles;
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XText > DomainMapper_Impl::GetBodyText()
{
    if(!m_xBodyText.is() && m_xTextDocument.is())
    {
        m_xBodyText = m_xTextDocument->getText();
    }
    return m_xBodyText;
}
/*-- 21.12.2006 12:09:30---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySet > DomainMapper_Impl::GetDocumentSettings()
{
    if( !m_xDocumentSettings.is() )
    {
        m_xDocumentSettings = uno::Reference< beans::XPropertySet >(
            m_xTextFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.Settings"))), uno::UNO_QUERY );
    }
    return m_xDocumentSettings;
}
/*-- 21.12.2006 12:16:23---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::SetDocumentSettingsProperty( const ::rtl::OUString& rPropName, const uno::Any& rValue )
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
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
void    DomainMapper_Impl::PushProperties(ContextType eId)
{
    SectionPropertyMap* pSectionContext = 0;
    PropertyMapPtr pInsert(eId == CONTEXT_SECTION ?
        (pSectionContext = new SectionPropertyMap( m_bIsFirstSection )) :
        eId == CONTEXT_PARAGRAPH ? new ParagraphPropertyMap :  new PropertyMap);
    if(eId == CONTEXT_SECTION)
    {
        if( m_bIsFirstSection )
            m_bIsFirstSection = false;
        else
        {
            // beginning with the second section group a section has to be inserted
            // into the document
            SectionPropertyMap* pSectionContext_ = dynamic_cast< SectionPropertyMap* >( pInsert.get() );
            uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = m_aTextAppendStack.top().xTextAppendAndConvert;
            if(xTextAppendAndConvert.is())
                pSectionContext_->SetStart( xTextAppendAndConvert->getEnd() );
        }
    }
    m_aPropertyStacks[eId].push( pInsert );
    m_aContextStack.push(eId);

    m_pTopContext = m_aPropertyStacks[eId].top();
}
/*-- 13.06.2007 16:18:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::PushStyleProperties( PropertyMapPtr pStyleProperties )
{
    m_aPropertyStacks[CONTEXT_STYLESHEET].push( pStyleProperties );
    m_aContextStack.push(CONTEXT_STYLESHEET);

    m_pTopContext = m_aPropertyStacks[CONTEXT_STYLESHEET].top();
}
/*-- 28.01.2008 14:47:46---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::PushListProperties(PropertyMapPtr pListProperties)
{
    m_aPropertyStacks[CONTEXT_LIST].push( pListProperties );
    m_aContextStack.push(CONTEXT_LIST);
    m_pTopContext = m_aPropertyStacks[CONTEXT_LIST].top();
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
void    DomainMapper_Impl::PopProperties(ContextType eId)
{
    OSL_ENSURE(!m_aPropertyStacks[eId].empty(), "section stack already empty");
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
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyMapPtr DomainMapper_Impl::GetTopContextOfType(ContextType eId)
{
    PropertyMapPtr pRet;
    OSL_ENSURE( !m_aPropertyStacks[eId].empty(),
            "no context of this type available");
    if(!m_aPropertyStacks[eId].empty())
        pRet = m_aPropertyStacks[eId].top();
    return pRet;
}

/*-- 24.05.2007 15:54:51---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextAppendAndConvert >  DomainMapper_Impl::GetTopTextAppendAndConvert()
{
    OSL_ENSURE(!m_aTextAppendStack.empty(), "text append stack is empty" );
    return m_aTextAppendStack.top().xTextAppendAndConvert;
}

/*-- 17.07.2006 08:47:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::InitTabStopFromStyle( const uno::Sequence< style::TabStop >& rInitTabStops )
{
    OSL_ENSURE(!m_aCurrentTabStops.size(), "tab stops already initialized");
    for( sal_Int32 nTab = 0; nTab < rInitTabStops.getLength(); ++nTab)
    {
        m_aCurrentTabStops.push_back( DeletableTabStop(rInitTabStops[nTab]) );
    }
}

/*-- 29.06.2006 13:35:33---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 29.06.2006 13:35:33---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< style::TabStop > DomainMapper_Impl::GetCurrentTabStopAndClear()
{
    uno::Sequence< style::TabStop > aRet( m_aCurrentTabStops.size() );
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

/*-- 17.07.2006 09:08:26---------------------------------------------------
    returns a the value from the current paragraph style - if available
    TODO: What about parent styles?
  -----------------------------------------------------------------------*/
uno::Any DomainMapper_Impl::GetPropertyFromStyleSheet(PropertyIds eId)
{
    const StyleSheetEntry* pEntry = 0;
    if( m_bInStyleSheetImport )
        pEntry = GetStyleSheetTable()->FindParentStyleSheet(::rtl::OUString());
    else
        pEntry =
                GetStyleSheetTable()->FindStyleSheetByISTD(GetCurrentParaStyleId());
    while(pEntry)
    {
        //is there a tab stop set?
        if(pEntry->pProperties)
        {
            PropertyMap::const_iterator aPropertyIter =
                    pEntry->pProperties->find(PropertyDefinition(eId, false ));
            if( aPropertyIter != pEntry->pProperties->end())
            {
                return aPropertyIter->second;
            }
        }
        //search until the property is set or no parent is available
        pEntry = GetStyleSheetTable()->FindParentStyleSheet(pEntry->sBaseStyleIdentifier);
    }
    return uno::Any();
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
ListTablePtr DomainMapper_Impl::GetListTable()
{
    if(!m_pListTable)
        m_pListTable.reset(
            new ListTable( m_rDMapper, m_xTextFactory ));
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

void DomainMapper_Impl::clearDeferredBreaks()
{
    m_bIsColumnBreakDeferred = false;
    m_bIsPageBreakDeferred = false;
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
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
            ::rtl::OUString sPropertyName = rPropNameSupplier.GetName(aBorderProperties[nProperty]);
            pFrameProperties[nStart].Name = sPropertyName;
            pFrameProperties[nStart].Value = xTextRangeProperties->getPropertyValue(sPropertyName);
            if( nProperty < 4 )
                xTextRangeProperties->setPropertyValue( sPropertyName, uno::makeAny(table::BorderLine()));
            ++nStart;
        }
        rFrameProperties.realloc(nStart);
    }
    catch( const uno::Exception& rEx )
   {
        (void)rEx;
   }
}
/*-- 04.01.2008 10:59:19---------------------------------------------------

  -----------------------------------------------------------------------*/
void lcl_AddRangeAndStyle(
    ParagraphPropertiesPtr& pToBeSavedProperties,
    uno::Reference< text::XTextAppendAndConvert > xTextAppendAndConvert,
    PropertyMapPtr pPropertyMap)
{
    uno::Reference<text::XParagraphCursor> xParaCursor(
        xTextAppendAndConvert->createTextCursorByRange(xTextAppendAndConvert->getEnd()), uno::UNO_QUERY_THROW);
    pToBeSavedProperties->SetEndingRange(xParaCursor->getStart());
    xParaCursor->gotoStartOfParagraph( false );

    pToBeSavedProperties->SetStartingRange(xParaCursor->getStart());
    if(pPropertyMap)
    {
        PropertyMap::iterator aParaStyleIter = pPropertyMap->find(PropertyDefinition( PROP_PARA_STYLE_NAME, false ) );
        if( aParaStyleIter != pPropertyMap->end())
        {
            ::rtl::OUString sName;
            aParaStyleIter->second >>= sName;
            pToBeSavedProperties->SetParaStyleName(sName);
        }
    }
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
//define some default frame width - 10cm ATM
#define DEFAULT_FRAME_MIN_WIDTH 10000

void DomainMapper_Impl::finishParagraph( PropertyMapPtr pPropertyMap )
{
    ParagraphPropertyMap* pParaContext = dynamic_cast< ParagraphPropertyMap* >( pPropertyMap.get() );
    TextAppendContext& rAppendContext = m_aTextAppendStack.top();
    uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = rAppendContext.xTextAppendAndConvert;
    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
    if(xTextAppendAndConvert.is() && ! m_TableManager.isIgnore())
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

            bool bIsDropCap = pParaContext->IsFrameMode() && sal::static_int_cast<Id>(pParaContext->GetDropCap()) != NS_ooxml::LN_Value_wordprocessingml_ST_DropCap_none;
            style::DropCapFormat aDrop;
            ParagraphPropertiesPtr pToBeSavedProperties;
            bool bKeepLastParagraphProperties = false;
            if( bIsDropCap )
            {
                uno::Reference<text::XParagraphCursor> xParaCursor(
                    xTextAppendAndConvert->createTextCursorByRange(xTextAppendAndConvert->getEnd()), uno::UNO_QUERY_THROW);
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
                    rAppendContext.pLastParagraphProperties->SetEndingRange(xTextAppendAndConvert->getEnd());
                    bKeepLastParagraphProperties = true;
                }
                else
                {
                    //handles (8)(9) and completes (6)
                    try
                       {
                            //
                           const StyleSheetEntry* pParaStyle =
                                m_pStyleSheetTable->FindStyleSheetByConvertedStyleName(rAppendContext.pLastParagraphProperties->GetParaStyleName());

                            uno::Sequence< beans::PropertyValue > aFrameProperties(pParaStyle ? 15: 0);
                            if(pParaStyle)
                            {
                                const ParagraphProperties* pStyleProperties = dynamic_cast<const ParagraphProperties*>( pParaStyle->pProperties.get() );
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

                                lcl_MoveBorderPropertiesToFrame(aFrameProperties,
                                    rAppendContext.pLastParagraphProperties->GetStartingRange(),
                                    rAppendContext.pLastParagraphProperties->GetEndingRange());
                            }
                            xTextAppendAndConvert->convertToTextFrame(
                            rAppendContext.pLastParagraphProperties->GetStartingRange(),
                            rAppendContext.pLastParagraphProperties->GetEndingRange(),
                            aFrameProperties );
                            // next frame follows directly
                            if( pParaContext->IsFrameMode() )
                            {
                                pToBeSavedProperties.reset( new ParagraphProperties(*pParaContext) );
                                lcl_AddRangeAndStyle(pToBeSavedProperties, xTextAppendAndConvert, pPropertyMap);
                            }
                       }
                       catch( const uno::Exception& rEx )
                       {
                            (void)rEx;
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
                    lcl_AddRangeAndStyle(pToBeSavedProperties, xTextAppendAndConvert, pPropertyMap);
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
                uno::Reference< text::XTextRange > xTextRange =
                    xTextAppendAndConvert->finishParagraph( aProperties );
                m_TableManager.handle(xTextRange);
            }
            else
            {

            }
            if( !bKeepLastParagraphProperties )
                rAppendContext.pLastParagraphProperties = pToBeSavedProperties;

        }
        catch(const lang::IllegalArgumentException& rIllegal)
        {
            (void)rIllegal;
            OSL_ENSURE( false, "IllegalArgumentException in DomainMapper_Impl::finishParagraph" );
        }
        catch(const uno::Exception& rEx)
        {
            (void)rEx;
            OSL_ENSURE( false, "ArgumentException in DomainMapper_Impl::finishParagraph" );
        }
    }
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::appendTextPortion( const ::rtl::OUString& rString, PropertyMapPtr pPropertyMap )
{
    uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = m_aTextAppendStack.top().xTextAppendAndConvert;
    if(xTextAppendAndConvert.is() && ! m_TableManager.isIgnore())
    {
        try
        {
            uno::Reference< text::XTextRange > xTextRange =
                xTextAppendAndConvert->appendTextPortion
                (rString, pPropertyMap->GetPropertyValues());

            //m_TableManager.handle(xTextRange);
        }
        catch(const lang::IllegalArgumentException& )
        {
        }
        catch(const uno::Exception& )
        {
        }
    }
}
/*-- 02.11.2006 12:08:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::appendTextContent(
    const uno::Reference< text::XTextContent > xContent,
    const uno::Sequence< beans::PropertyValue > xPropertyValues
    )
{
    uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = m_aTextAppendStack.top().xTextAppendAndConvert;
    if(xTextAppendAndConvert.is() && ! m_TableManager.isIgnore())
    {
        try
        {
            xTextAppendAndConvert->appendTextContent( xContent, xPropertyValues );
        }
        catch(const lang::IllegalArgumentException& )
        {
        }
        catch(const uno::Exception& )
        {
        }
    }
}
/*-- 14.12.2006 12:26:00---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySet > DomainMapper_Impl::appendTextSectionAfter(
                                    uno::Reference< text::XTextRange >& xBefore )
{
    uno::Reference< beans::XPropertySet > xRet;
    uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = m_aTextAppendStack.top().xTextAppendAndConvert;
    if(xTextAppendAndConvert.is())
    {
        try
        {
            uno::Reference< text::XParagraphCursor > xCursor(
                xTextAppendAndConvert->createTextCursorByRange( xBefore ), uno::UNO_QUERY_THROW);
            //the cursor has been moved to the end of the paragraph because of the appendTextPortion() calls
            xCursor->gotoStartOfParagraph( false );
            xCursor->gotoEnd( true );
            //the paragraph after this new section is already inserted
            xCursor->goLeft(1, true);
            static const rtl::OUString sSectionService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextSection"));
            uno::Reference< text::XTextContent > xSection( m_xTextFactory->createInstance(sSectionService), uno::UNO_QUERY_THROW );
            xSection->attach( uno::Reference< text::XTextRange >( xCursor, uno::UNO_QUERY_THROW) );
            xRet = uno::Reference< beans::XPropertySet > (xSection, uno::UNO_QUERY );
        }
        catch(const uno::Exception& )
        {
        }

    }

    return xRet;
}


/*-- 14.12.2006 11:03:32---------------------------------------------------

  -----------------------------------------------------------------------*/
/*void DomainMapper_Impl::appendTextSection()
{
    uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = m_aTextAppendStack.top().xTextAppendAndConvert;
    if(xTextAppendAndConvert.is())
    {
        try
        {
            uno::Reference< text::XTextRange >  xRange = xTextAppendAndConvert->getEnd();
            uno::Reference< beans::XPropertySet > xRangePropSet( xRange, uno::UNO_QUERY_THROW );
            static const rtl::OUString sSectionProperty(RTL_CONSTASCII_USTRINGPARAM("TextSection"));
            uno::Reference< text::XTextContent > xPrevSection;
            xRangePropSet->getPropertyValue(sSectionProperty) >>= xPrevSection;
            //close previous section by appending a new paragraph behind the current section
            if(xPrevSection.is())
            {
                uno::Reference< text::XRelativeTextContentInsert > xRelativeInsert( xTextAppendAndConvert, uno::UNO_QUERY_THROW );
                static const rtl::OUString sParagraphService(
                    RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Paragraph"));
                uno::Reference< text::XTextContent > xParagraph( m_xTextFactory->createInstance(sParagraphService), uno::UNO_QUERY_THROW );
                xRelativeInsert->insertTextContentAfter( xParagraph, xPrevSection );
            }
            static const rtl::OUString sSectionService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextSection"));
            uno::Reference< text::XTextContent > xSection( m_xTextFactory->createInstance(sSectionService), uno::UNO_QUERY_THROW );
            xSection->attach( xRange );
            //remove the appended paragraph behind the new section
            uno::Reference< text::XRelativeTextContentRemove > xRemove( xTextAppendAndConvert, uno::UNO_QUERY_THROW );
            xRemove->removeTextContentAfter( xSection );
        }
        catch(const uno::Exception& )
        {
        }
    }
} */
/*-- 02.11.2006 12:08:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::PushPageHeader(SectionPropertyMap::PageType eType)
{
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
        try
        {
            PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
            //switch on header use
            xPageStyle->setPropertyValue(
                    rPropNameSupplier.GetName(PROP_HEADER_IS_ON),
                    uno::makeAny(sal_True) );
            // if a left header is available then header are not shared
            bool bLeft = eType == SectionPropertyMap::PAGE_LEFT;
            if( bLeft )
                xPageStyle->setPropertyValue(rPropNameSupplier.GetName(PROP_HEADER_IS_SHARED), uno::makeAny( false ));

            //set the interface
            uno::Reference< text::XText > xHeaderText;
            xPageStyle->getPropertyValue(rPropNameSupplier.GetName( bLeft ? PROP_HEADER_TEXT_LEFT : PROP_HEADER_TEXT) ) >>= xHeaderText;
            m_aTextAppendStack.push( uno::Reference< text::XTextAppendAndConvert >( xHeaderText, uno::UNO_QUERY_THROW));
        }
        catch( uno::Exception& )
        {
        }
    }
}
/*-- 24.07.2006 09:41:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::PushPageFooter(SectionPropertyMap::PageType eType)
{
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
        try
        {
            PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
            //switch on footer use
            xPageStyle->setPropertyValue(
                    rPropNameSupplier.GetName(PROP_FOOTER_IS_ON),
                    uno::makeAny(sal_True) );
            // if a left header is available then footer is not shared
            bool bLeft = eType == SectionPropertyMap::PAGE_LEFT;
            if( bLeft )
                xPageStyle->setPropertyValue(rPropNameSupplier.GetName(PROP_FOOTER_IS_SHARED), uno::makeAny( false ));
            //set the interface
            uno::Reference< text::XText > xFooterText;
            xPageStyle->getPropertyValue(rPropNameSupplier.GetName( bLeft ? PROP_FOOTER_TEXT_LEFT : PROP_FOOTER_TEXT) ) >>= xFooterText;
            m_aTextAppendStack.push(uno::Reference< text::XTextAppendAndConvert >( xFooterText, uno::UNO_QUERY_THROW ));
        }
        catch( uno::Exception& )
        {
        }
    }
}
/*-- 24.07.2006 09:41:20---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::PopPageHeaderFooter()
{
    //header and footer always have an empty paragraph at the end
    //this has to be removed
    uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = m_aTextAppendStack.top().xTextAppendAndConvert;
    try
    {
        uno::Reference< text::XTextCursor > xCursor = xTextAppendAndConvert->createTextCursor();
        xCursor->gotoEnd(false);
        xCursor->goLeft( 1, true );
        xCursor->setString(::rtl::OUString());
    }
    catch( const uno::Exception& rEx)
    {
        (void)rEx;
    }
    m_aTextAppendStack.pop();
}
/*-- 24.05.2007 14:22:28---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::PushFootOrEndnote( bool bIsFootnote )
{
    try
    {
        PropertyMapPtr pTopContext = GetTopContext();
        uno::Reference< text::XText > xFootnoteText( GetTextFactory()->createInstance(
            bIsFootnote ?
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Footnote") ) : ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Endnote") )),
            uno::UNO_QUERY_THROW );
        uno::Reference< text::XFootnote > xFootnote( xFootnoteText, uno::UNO_QUERY_THROW );
        pTopContext->SetFootnote( xFootnote );
        if( pTopContext->GetFootnoteSymbol() != 0)
        {
            xFootnote->setLabel( ::rtl::OUString( pTopContext->GetFootnoteSymbol() ) );
        }
        FontTablePtr pFontTable = GetFontTable();
        uno::Sequence< beans::PropertyValue > aFontProperties;
        if( pFontTable && pTopContext->GetFootnoteFontId() >= 0 && pFontTable->size() > (size_t)pTopContext->GetFootnoteFontId() )
        {
            const FontEntry* pFontEntry = pFontTable->getFontEntry(sal_uInt32(pTopContext->GetFootnoteFontId()));
            PropertyMapPtr aFontProps( new PropertyMap );
            aFontProps->Insert(PROP_CHAR_FONT_NAME, true, uno::makeAny( pFontEntry->sFontName  ));
            aFontProps->Insert(PROP_CHAR_FONT_CHAR_SET, true, uno::makeAny( (sal_Int16)pFontEntry->nTextEncoding  ));
            aFontProps->Insert(PROP_CHAR_FONT_PITCH, true, uno::makeAny( pFontEntry->nPitchRequest  ));
            aFontProperties = aFontProps->GetPropertyValues();
        }
        else if(pTopContext->GetFootnoteFontName().getLength())
        {
            PropertyMapPtr aFontProps( new PropertyMap );
            aFontProps->Insert(PROP_CHAR_FONT_NAME, true, uno::makeAny( pTopContext->GetFootnoteFontName()  ));
            aFontProperties = aFontProps->GetPropertyValues();
        }

        appendTextContent( uno::Reference< text::XTextContent >( xFootnoteText, uno::UNO_QUERY_THROW ), aFontProperties );
        m_aTextAppendStack.push(uno::Reference< text::XTextAppendAndConvert >( xFootnoteText, uno::UNO_QUERY_THROW ));
    }
    catch( uno::Exception& )
    {
    }
}
/*-- 24.05.2007 14:22:29---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::PopFootOrEndnote()
{
    m_aTextAppendStack.pop();
}
/*-- 12.09.2006 08:07:55---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString lcl_FindQuotedText( const ::rtl::OUString& rCommand,
                const sal_Char* cStartQuote, const sal_Unicode uEndQuote )
{
    ::rtl::OUString sRet;
    ::rtl::OUString sStartQuote( ::rtl::OUString::createFromAscii(cStartQuote) );
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
/*-- 08.09.2006 14:05:17---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int16 lcl_ParseNumberingType( const ::rtl::OUString& rCommand )
{
    sal_Int16 nRet = style::NumberingType::PAGE_DESCRIPTOR;

    //  The command looks like: " PAGE \* Arabic "
    ::rtl::OUString sNumber = lcl_FindQuotedText(rCommand, "\\* ", ' ');

    if( sNumber.getLength() )
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
    CHARS_CYRILLIC_LOWER_LETTER_N_RU*/

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
/*-- 08.09.2006 13:52:09---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString lcl_ParseFormat( const ::rtl::OUString& rCommand )
{
    //  The command looks like: " DATE \@ "dd MMMM yyyy"
    return lcl_FindQuotedText(rCommand, "\\@ \"", '\"');
}
/*-- 19.09.2006 10:01:20---------------------------------------------------
extract a parameter (with or without quotes) between the command and the following backslash
  -----------------------------------------------------------------------*/
::rtl::OUString lcl_ExtractParameter(const ::rtl::OUString& rCommand, sal_Int32 nCommandLength )
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
        nEndIndex = rCommand.indexOf( ::rtl::OUString::createFromAscii(" \\"), nStartIndex);
    }
    ::rtl::OUString sRet;
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

/*-- 15.09.2006 10:57:57---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString lcl_ExctractAskVariableAndHint( const ::rtl::OUString& rCommand, ::rtl::OUString& rHint )
{
    // the first word after "ASK " is the variable
    // the text after the variable and before a '\' is the hint
    // if no hint is set the variable is used as hint
    // the quotes of the hint have to be removed
    sal_Int32 nIndex = rCommand.indexOf( ' ', 2);//find last space after 'ASK'
    while(rCommand.getStr()[nIndex] == ' ')
        ++nIndex;
    ::rtl::OUString sShortCommand( rCommand.copy( nIndex ) ); //cut off the " ASK "

    nIndex = 0;
    sShortCommand = sShortCommand.getToken( 0, '\\', nIndex);
    nIndex = 0;
    ::rtl::OUString sRet = sShortCommand.getToken( 0, ' ', nIndex);
    if( nIndex > 0)
        rHint = sShortCommand.copy( nIndex );
    if( !rHint.getLength() )
        rHint = sRet;
    return sRet;
}
/*-- 24.01.2007 16:04:33---------------------------------------------------

  -----------------------------------------------------------------------*/
bool lcl_FindInCommand(
    const ::rtl::OUString& rCommand,
    sal_Unicode cSwitch,
    ::rtl::OUString& rValue )
{
    bool bRet = false;
    ::rtl::OUString sSearch('\\');
    sSearch += ::rtl::OUString( cSwitch );
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
/*-- 01.09.2006 11:48:08---------------------------------------------------

  -----------------------------------------------------------------------*/
/*void DomainMapper_Impl::CreateField( ::rtl::OUString& rCommand )
{
    static FieldConversionMap_t aFieldConversionMap;
    static bool bFilled = false;
    m_bSetUserFieldContent = false;
    if(!bFilled)
    {
        static const FieldConversion aFields[] =
        {
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ADDRESSBLOCK")),  "",                         "", FIELD_ADDRESSBLOCK  },
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ADVANCE")),       "",                         "", FIELD_ADVANCE       },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ASK")),           "SetExpression",             "SetExpression", FIELD_ASK      },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AUTONUM")),       "SetExpression",            "SetExpression", FIELD_AUTONUM   },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AUTONUMLGL")),     "SetExpression",            "SetExpression", FIELD_AUTONUMLGL },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AUTONUMOUT")),     "SetExpression",            "SetExpression", FIELD_AUTONUMOUT },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AUTHOR")),        "Author",                   "", FIELD_AUTHOR       },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DATE")),          "DateTime",                 "", FIELD_DATE         },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("COMMENTS")),      "DocInfo.Description",      "", FIELD_COMMENTS     },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CREATEDATE")),    "DocInfo.CreateDateTime",   "", FIELD_CREATEDATE   },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DOCPROPERTY")),   "",                         "", FIELD_DOCPROPERTY },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DOCVARIABLE")),   "User",                     "", FIELD_DOCVARIABLE  },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EDITTIME")),      "DocInfo.EditTime",         "", FIELD_EDITTIME     },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FILLIN")),        "Input",                    "", FIELD_FILLIN       },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FILENAME")),      "FileName",                 "", FIELD_FILENAME     },
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FILESIZE")),      "",                         "", FIELD_FILESIZE     },
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FORMULA")),     "",                           "", FIELD_FORMULA
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GOTOBUTTON")),    "",                         "", FIELD_GOTOBUTTON   },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HYPERLINK")),     "",                         "", FIELD_HYPERLINK    },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IF")),            "ConditionalText",          "", FIELD_IF           },
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INFO")),      "","", FIELD_INFO         },
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INCLUDEPICTURE")), "",                        "", FIELD_INCLUDEPICTURE},
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("KEYWORDS")),      "DocInfo.KeyWords",         "", FIELD_KEYWORDS     },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LASTSAVEDBY")),   "DocInfo.ChangeAuthor",                         "", FIELD_LASTSAVEDBY  },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MACROBUTTON")),   "Macro",                         "", FIELD_MACROBUTTON  },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MERGEFIELD")),    "Database",                 "Database", FIELD_MERGEFIELD},
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MERGEREC")),      "DatabaseNumberOfSet",      "", FIELD_MERGEREC     },
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MERGESEQ")),      "",                         "", FIELD_MERGESEQ     },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NEXT")),          "DatabaseNextSet",          "", FIELD_NEXT         },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NEXTIF")),        "DatabaseNextSet",          "", FIELD_NEXTIF       },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PAGE")),          "PageNumber",               "", FIELD_PAGE         },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("REF")),           "",                         "", FIELD_REF          },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("REVNUM")),        "DocInfo.Revision",         "", FIELD_REVNUM       },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SAVEDATE")),      "DocInfo.Change",           "", FIELD_SAVEDATE     },
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SECTION")),       "",                         "", FIELD_SECTION      },
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SECTIONPAGES")),  "",                         "", FIELD_SECTIONPAGES },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SEQ")),           "SetExpression",            "SetExpression", FIELD_SEQ          },
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SET")),           "","", FIELD_SET          },
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SKIPIF")),"",                                 "", FIELD_SKIPIF       },
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("STYLEREF")),"",                               "", FIELD_STYLEREF     },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SUBJECT")),       "DocInfo.Subject",          "", FIELD_SUBJECT      },
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SYMBOL")),"",                                 "", FIELD_SYMBOL       },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TEMPLATE")),      "TemplateName",             "", FIELD_TEMPLATE},
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TIME")),          "DateTime",                 "", FIELD_TIME         },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TITLE")),         "DocInfo.Title",            "", FIELD_TITLE        },
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("USERINITIALS")),  "ExtendedUser",              "", FIELD_USERINITIALS},
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("USERADDRESS")),   "",                         "", FIELD_USERADDRESS  },
//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("USERNAME")),      "ExtendedUser",             "", FIELD_USERNAME     }
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TOC")), "com.sun.star.text.ContentIndex", "", FIELD_TOC},
            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TC")), "com.sun.star.text.ContentIndexMark", "", FIELD_TC},

//            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")), "", "", FIELD_},

        };
        size_t nConversions = sizeof(aFields)/sizeof(FieldConversion);
        for( size_t nConversion = 0; nConversion < nConversions; ++nConversion)
        {
            aFieldConversionMap.insert( FieldConversionMap_t::value_type(
                    aFields[nConversion].sWordCommand,
                    aFields[nConversion] ));
        }

        bFilled = true;
    }
    try
    {
        uno::Reference< uno::XInterface > xFieldInterface;
        //at first determine the field type - skip first space
        ::rtl::OUString sCommand( rCommand.copy(rCommand.getLength() ? 1 : 0) );
        sal_Int32 nSpaceIndex = sCommand.indexOf( ' ' );
        if( 0 <= nSpaceIndex )
            sCommand = sCommand.copy( 0, nSpaceIndex );

        FieldConversionMap_t::iterator aIt = aFieldConversionMap.find(sCommand);
        if(aIt != aFieldConversionMap.end())
        {
            uno::Reference< beans::XPropertySet > xFieldProperties;
            if( FIELD_HYPERLINK != aIt->second.eFieldId &&
                    FIELD_DOCPROPERTY != aIt->second.eFieldId &&
                    FIELD_TOC != aIt->second.eFieldId &&
                    FIELD_TC != aIt->second.eFieldId)
            {
                //add the service prefix
                OUString sServiceName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextField."));
                sServiceName += ::rtl::OUString::createFromAscii(aIt->second.cFieldServiceName );
                xFieldInterface = m_xTextFactory->createInstance(sServiceName);
                xFieldProperties = uno::Reference< beans::XPropertySet >( xFieldInterface, uno::UNO_QUERY_THROW);
            }
            PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
            switch( aIt->second.eFieldId )
            {
                case FIELD_ADDRESSBLOCK: break;
                case FIELD_ADVANCE     : break;
                case FIELD_ASK         :
                {
                    //doesn the command contain a variable name?
                    ::rtl::OUString sVariable, sHint;

                    sVariable = lcl_ExctractAskVariableAndHint( rCommand, sHint );
                    if(sVariable.getLength())
                    {
                        // determine field master name
                        uno::Reference< beans::XPropertySet > xMaster = FindOrCreateFieldMaster(
                                "com.sun.star.text.FieldMaster.SetExpression", sVariable );

                        // attach the master to the field
                        uno::Reference< text::XDependentTextField > xDependentField( xFieldInterface, uno::UNO_QUERY_THROW );
                        xDependentField->attachTextFieldMaster( xMaster );

                        // set input flag at the field
                        xFieldProperties->setPropertyValue(
                                rPropNameSupplier.GetName(PROP_IS_INPUT), uno::makeAny( true ));
                        // set the prompt
                        xFieldProperties->setPropertyValue(
                                rPropNameSupplier.GetName(PROP_HINT),
                                uno::makeAny( sHint ));
                    }
                    else
                    {
                        //don't insert the field
                        //todo: maybe import a 'normal' input field here?
                        xFieldInterface = 0;
                    }
                }
                break;
                case FIELD_AUTONUM    :
                case FIELD_AUTONUMLGL :
                case FIELD_AUTONUMOUT :
                {
                    //create a sequence field master "AutoNr"
                    uno::Reference< beans::XPropertySet > xMaster = FindOrCreateFieldMaster(
                                "com.sun.star.text.FieldMaster.SetExpression",
                                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AutoNr") ));

                    xMaster->setPropertyValue( rPropNameSupplier.GetName(PROP_SUB_TYPE),
                                uno::makeAny(text::SetVariableType::SEQUENCE));

                    //apply the numbering type
                    xFieldProperties->setPropertyValue(
                        rPropNameSupplier.GetName(PROP_NUMBERING_TYPE),
                        uno::makeAny( lcl_ParseNumberingType(rCommand) ));
                        // attach the master to the field
                    uno::Reference< text::XDependentTextField > xDependentField( xFieldInterface, uno::UNO_QUERY_THROW );
                    xDependentField->attachTextFieldMaster( xMaster );
                }
                break;
                case FIELD_AUTHOR       :
                {
                    xFieldProperties->setPropertyValue( rPropNameSupplier.GetName(PROP_FULL_NAME), uno::makeAny( true ));
                    ::rtl::OUString sParam = lcl_ExtractParameter(rCommand, sizeof(" AUTHOR") );
                    if(sParam.getLength())
                    {
                        xFieldProperties->setPropertyValue(
                                rPropNameSupplier.GetName( PROP_IS_FIXED ), uno::makeAny( true ));
                        //PROP_CURRENT_PRESENTATION is set later anyway
                    }
                }
                break;
                case FIELD_DATE:
                {
                    //not fixed,
                    xFieldProperties->setPropertyValue(
                        rPropNameSupplier.GetName(PROP_IS_FIXED),
                        uno::makeAny( false ));
                    SetNumberFormat( rCommand, xFieldProperties );
                }
                break;
                case FIELD_COMMENTS     :
                {
                    ::rtl::OUString sParam = lcl_ExtractParameter(rCommand, sizeof(" COMMENTS") );
                    if(sParam.getLength())
                    {
                        xFieldProperties->setPropertyValue(
                                rPropNameSupplier.GetName( PROP_IS_FIXED ), uno::makeAny( true ));
                        //PROP_CURRENT_PRESENTATION is set later anyway
                    }
                }
                break;
                case FIELD_CREATEDATE  :
                {
                    SetNumberFormat( rCommand, xFieldProperties );
                }
                break;
                case FIELD_DOCPROPERTY :
                {
                    //some docproperties should be imported as document statistic fields, some as DocInfo fields
                    //others should be user fields
                    ::rtl::OUString sParam = lcl_ExtractParameter(rCommand, sizeof(" DOCPROPERTY") );
                    if(sParam.getLength())
                    {
                        #define SET_ARABIC      0x01
                        #define SET_FULL_NAME   0x02
                        struct DocPropertyMap
                        {
                            const sal_Char* pDocPropertyName;
                            const sal_Char* pServiceName;
                            sal_uInt8       nFlags;
                        };
                        static const DocPropertyMap aDocProperties[] =
                        {
                            {"Author",           "Author",                  SET_FULL_NAME},
                            {"CreateTime",       "DocInfo.CreateDateTime",  0},
                            {"Characters",       "CharacterCount",          SET_ARABIC},
                            {"Comments",         "DocInfo.Description",     0},
                            {"Keywords",         "DocInfo.KeyWords",        0},
                            {"LastPrinted",      "DocInfo.PrintDateTime",   0},
                            {"LastSavedBy",      "DocInfo.ChangeAuthor",    0},
                            {"LastSavedTime",    "DocInfo.ChangeDateTime",  0},
                            {"Paragraphs",       "ParagraphCount",          SET_ARABIC},
                            {"RevisionNumber",   "DocInfo.Revision",        0},
                            {"Subject",          "DocInfo.Subject",         0},
                            {"Template",         "TemplateName",            0},
                            {"Title",            "DocInfo.Title",           0},
                            {"TotalEditingTime", "DocInfo.EditTime",        9},
                            {"Words",            "WordCount",               SET_ARABIC}

                            //other available DocProperties:
                            //Bytes, Category, CharactersWithSpaces, Company
                            //HyperlinkBase,
                            //Lines, Manager, NameofApplication, ODMADocId, Pages,
                            //Security,
                        };
                        //search for a field mapping
                        ::rtl::OUString sFieldServiceName;
                        sal_uInt16 nMap = 0;
                        for( ; nMap < sizeof(aDocProperties) / sizeof(DocPropertyMap); ++nMap )
                        {
                            if(sParam.equalsAscii(aDocProperties[nMap].pDocPropertyName))
                            {
                                sFieldServiceName = ::rtl::OUString::createFromAscii(aDocProperties[nMap].pServiceName);
                                break;
                            }
                        }
                        ::rtl::OUString sServiceName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextField."));
                        if(sFieldServiceName.getLength())
                        {
                            sServiceName += sFieldServiceName;
                            xFieldInterface = m_xTextFactory->createInstance(sServiceName);
                            xFieldProperties = uno::Reference< beans::XPropertySet >( xFieldInterface, uno::UNO_QUERY_THROW);
                            if(0 != (aDocProperties[nMap].nFlags & SET_ARABIC))
                                xFieldProperties->setPropertyValue(
                                    rPropNameSupplier.GetName(PROP_NUMBERING_TYPE),
                                    uno::makeAny( style::NumberingType::ARABIC ));
                            else if(0 != (aDocProperties[nMap].nFlags & SET_FULL_NAME))
                                xFieldProperties->setPropertyValue(
                                    rPropNameSupplier.GetName(PROP_FULL_NAME), uno::makeAny( true ));

                        }
                        else
                        {
                            //create a user field and type
                            uno::Reference< beans::XPropertySet > xMaster =
                                FindOrCreateFieldMaster( "com.sun.star.text.FieldMaster.User", sParam );
                            sServiceName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("User"));
                            xFieldInterface = m_xTextFactory->createInstance(sServiceName);
                            xFieldProperties = uno::Reference< beans::XPropertySet >( xFieldInterface, uno::UNO_QUERY_THROW);
                            uno::Reference< text::XDependentTextField > xDependentField( xFieldInterface, uno::UNO_QUERY_THROW );
                            xDependentField->attachTextFieldMaster( xMaster );
                            m_bSetUserFieldContent = true;
                        }
                    }
                }
                #undef SET_ARABIC
                #undef SET_FULL_NAME
                break;
                case FIELD_DOCVARIABLE  :
                {
                    ::rtl::OUString sParam = lcl_ExtractParameter(rCommand, sizeof(" DOCVARIABLE") );
                    //create a user field and type
                    uno::Reference< beans::XPropertySet > xMaster =
                        FindOrCreateFieldMaster( "com.sun.star.text.FieldMaster.User", sParam );
                    uno::Reference< text::XDependentTextField > xDependentField( xFieldInterface, uno::UNO_QUERY_THROW );
                    xDependentField->attachTextFieldMaster( xMaster );
                    m_bSetUserFieldContent = true;
                }
                break;
                case FIELD_EDITTIME     :
                    //it's a numbering type, no number format! SetNumberFormat( rCommand, xFieldProperties );
                break;
                case FIELD_FILLIN       :
                {
                    sal_Int32 nIndex = 0;
                    xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_HINT), uno::makeAny( rCommand.getToken( 1, '\"', nIndex)));
                }
                break;
                case FIELD_FILENAME:
                {
                    sal_Int32 nNumberingTypeIndex = rCommand.indexOf( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\\p")));
                    xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_FILE_FORMAT),
                            uno::makeAny( nNumberingTypeIndex > 0 ? text::FilenameDisplayFormat::FULL : text::FilenameDisplayFormat::NAME ));
                }
                break;
                case FIELD_FILESIZE     : break;
                case FIELD_FORMULA : break;
                case FIELD_GOTOBUTTON   : break;
                case FIELD_HYPERLINK:
                {
                    sal_Int32 nStartQuote = rCommand.indexOf( '\"' );
                    sal_Int32 nEndQuote = nStartQuote < rCommand.getLength() + 1 ? rCommand.indexOf( '\"', nStartQuote + 1) : -1;
                    if( nEndQuote > 0)
                        m_sHyperlinkURL = rCommand.copy(nStartQuote + 1, nEndQuote - nStartQuote - 1);
                }
                break;
                case FIELD_IF           : break;
                case FIELD_INFO         : break;
                case FIELD_INCLUDEPICTURE: break;
                case FIELD_KEYWORDS     :
                {
                    ::rtl::OUString sParam = lcl_ExtractParameter(rCommand, sizeof(" KEYWORDS") );
                    if(sParam.getLength())
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
                    ::rtl::OUString sMacro = rCommand.getToken( 0, ' ', nIndex);
                    xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_MACRO_NAME), uno::makeAny( sMacro ));

                    //extract quick help text
                    if( rCommand.getLength() > nIndex + 1)
                    {
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_HINT),
                            uno::makeAny( rCommand.copy( nIndex )));
                    }
                }
                break;
                case FIELD_MERGEFIELD  :
                {
                    //todo: create a database field and fieldmaster pointing to a column, only
                    ::rtl::OUString sParam = lcl_ExtractParameter(rCommand, sizeof(" MERGEFIELD") );
                    //create a user field and type
                    uno::Reference< beans::XPropertySet > xMaster =
                        FindOrCreateFieldMaster( "com.sun.star.text.FieldMaster.Database", sParam );

//                    xFieldProperties->setPropertyValue(
//                             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FieldCode")),
//                             uno::makeAny( rCommand.copy( nIndex + 1 )));
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
                    xFieldProperties->setPropertyValue(
                        rPropNameSupplier.GetName(PROP_NUMBERING_TYPE),
                        uno::makeAny( lcl_ParseNumberingType(rCommand) ));
                    xFieldProperties->setPropertyValue(
                        rPropNameSupplier.GetName(PROP_SUB_TYPE),
                        uno::makeAny( text::PageNumberType_CURRENT ));

                break;
                case FIELD_REF          : break;
                case FIELD_REVNUM       : break;
                case FIELD_SAVEDATE     :
                    SetNumberFormat( rCommand, xFieldProperties );
                break;
                case FIELD_SECTION      : break;
                case FIELD_SECTIONPAGES : break;
                case FIELD_SEQ          : break;
                case FIELD_SET          : break;
                case FIELD_SKIPIF       : break;
                case FIELD_STYLEREF     : break;
                case FIELD_SUBJECT      :
                {
                    ::rtl::OUString sParam = lcl_ExtractParameter(rCommand, sizeof(" SUBJECT") );
                    if(sParam.getLength())
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
                    SetNumberFormat( rCommand, xFieldProperties );
                break;
                case FIELD_TITLE        :
                {
                    ::rtl::OUString sParam = lcl_ExtractParameter(rCommand, sizeof(" TITLE") );
                    if(sParam.getLength())
                    {
                        xFieldProperties->setPropertyValue(
                                rPropNameSupplier.GetName( PROP_IS_FIXED ), uno::makeAny( true ));
                        //PROP_CURRENT_PRESENTATION is set later anyway
                    }
                }
                break;
                case FIELD_USERINITIALS:
                {
                    xFieldProperties->setPropertyValue(
                        rPropNameSupplier.GetName(PROP_USER_DATA_TYPE), uno::makeAny( text::UserDataPart::SHORTCUT ));
                    //todo: if initials are provided - set them as fixed content
                    ::rtl::OUString sParam = lcl_ExtractParameter(rCommand, sizeof(" USERINITIALS") );
                    if(sParam.getLength())
                    {
                        xFieldProperties->setPropertyValue(
                                rPropNameSupplier.GetName( PROP_IS_FIXED ), uno::makeAny( true ));
                        //PROP_CURRENT_PRESENTATION is set later anyway
                    }
                }
                break;
                case FIELD_USERADDRESS  : //todo: user address collects street, city ...
                break;
                case FIELD_USERNAME     : //todo: user name is firstname + lastname
                break;
                case FIELD_TOC:
                {
                    ::rtl::OUString sValue;
                    bool bTableOfFigures = false;
                    bool bHyperlinks = false;
                    bool bFromOutline = false;
                    bool bFromEntries = false;
                    ::rtl::OUString sTemplate;
                    ::rtl::OUString sChapterNoSeparator;
//                  \a Builds a table of figures but does not include the captions's label and number
                    if( lcl_FindInCommand( rCommand, 'a', sValue ))
                    { //make it a table of figures
                        bTableOfFigures = true;
                    }
//                  \b Uses a bookmark to specify area of document from which to build table of contents
//                    if( lcl_FindInCommand( rCommand, 'b', sValue ))
//                    { //todo: sValue contains the bookmark name - unsupported feature
//                    }
                    if( lcl_FindInCommand( rCommand, 'c', sValue ))
//                  \c Builds a table of figures of the given label
                    {
                        //todo: sValue contains the label's name
                        bTableOfFigures = true;
                    }
//                  \d Defines the separator between sequence and page numbers
                    if( lcl_FindInCommand( rCommand, 'd', sValue ))
                    {
                        //todo: insert the chapter number into each level and insert the separator additionally
                        sChapterNoSeparator = sValue;
                    }
//                  \f Builds a table of contents using TC entries instead of outline levels
                    if( lcl_FindInCommand( rCommand, 'f', sValue ))
                    {
                        //todo: sValue can contain a TOC entry identifier - use unclear
                        bFromEntries = true;
                    }
//                  \h Hyperlinks the entries and page numbers within the table of contents
                    if( lcl_FindInCommand( rCommand, 'h', sValue ))
                    {
                        //todo: make all entries to hyperlinks
                        bHyperlinks = true;
                    }
//                  \l Defines the TC entries field level used to build a table of contents
//                    if( lcl_FindInCommand( rCommand, 'l', sValue ))
//                    {
                            //todo: entries can only be included completely
//                    }
//                  \n Builds a table of contents or a range of entries, sucah as “1-9”, in a table of contents without page numbers
//                    if( lcl_FindInCommand( rCommand, 'n', sValue ))
//                    {
                        //todo: what does the description mean?
//                    }
//                  \o  Builds a table of contents by using outline levels instead of TC entries
                    if( lcl_FindInCommand( rCommand, 'o', sValue ))
                    {
                        bFromOutline = true;
                    }
//                  \p Defines the separator between the table entry and its page number
                    if( lcl_FindInCommand( rCommand, 'p', sValue ))
                    {  }
//                  \s  Builds a table of contents by using a sequence type
                    if( lcl_FindInCommand( rCommand, 's', sValue ))
                    {  }
//                  \t  Builds a table of contents by using style names other than the standard outline styles
                    if( lcl_FindInCommand( rCommand, 't', sValue ))
                    {
                        sTemplate = sValue;
                    }
//                  \u  Builds a table of contents by using the applied paragraph outline level
                    if( lcl_FindInCommand( rCommand, 'u', sValue ))
                    {
                        bFromOutline = true;
                        //todo: what doesn 'the applied paragraph outline level' refer to?
                    }
//                  \w Preserve tab characters within table entries
//                    if( lcl_FindInCommand( rCommand, 'w', sValue ))
//                    {
                        //todo: not supported
//                    }
//                  \x Preserve newline characters within table entries
//                    if( lcl_FindInCommand( rCommand, 'x', sValue ))
//                    {
                        //todo: unsupported
//                    }
//                  \z Hides page numbers within the table of contens when shown in Web Layout View
//                    if( lcl_FindInCommand( rCommand, 'z', sValue ))
//                    { //todo: unsupported feature  }

                    m_xTOC = uno::Reference< beans::XPropertySet >(
                            m_xTextFactory->createInstance(
                                bTableOfFigures ?
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.IllustrationsIndex")) :
                                ::rtl::OUString::createFromAscii(aIt->second.cFieldServiceName)),
                                uno::UNO_QUERY_THROW);
                    if( !bTableOfFigures )
                    {
                        m_xTOC->setPropertyValue( rPropNameSupplier.GetName( PROP_CREATE_FROM_OUTLINE ), uno::makeAny( bFromOutline ));
                        m_xTOC->setPropertyValue( rPropNameSupplier.GetName( PROP_CREATE_FROM_MARKS ), uno::makeAny( bFromEntries ));
                        if( sTemplate.getLength() )
                        {
                            uno::Reference< container::XIndexReplace> xParaStyles;
                            m_xTOC->getPropertyValue(rPropNameSupplier.GetName(PROP_LEVEL_PARAGRAPH_STYLES)) >>= xParaStyles;
                            uno::Sequence< rtl::OUString> aStyles(1);
                            aStyles[0] = sTemplate;
                            xParaStyles->replaceByIndex(0, uno::makeAny(aStyles));
                        }
                        if(bHyperlinks  || sChapterNoSeparator.getLength())
                        {
                            uno::Reference< container::XIndexReplace> xLevelFormats;
                            m_xTOC->getPropertyValue(rPropNameSupplier.GetName(PROP_LEVEL_FORMAT)) >>= xLevelFormats;
                            sal_Int32 nLevelCount = xLevelFormats->getCount();
                            //start with level 1, 0 is the header level
                            for( sal_Int32 nLevel = 1; nLevel < nLevelCount; ++nLevel)
                            {
                                uno::Sequence< beans::PropertyValues > aLevel;
                                xLevelFormats->getByIndex( nLevel ) >>= aLevel;
                                //create a copy of the level and add two new entries - hyperlink start and end
                                bool bChapterNoSeparator  = sChapterNoSeparator.getLength() > 0;
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

                                xLevelFormats->replaceByIndex( nLevel, uno::makeAny( aNewLevel ) );
                            }
                        }
                    }
                }
                break;
                case FIELD_TC :
                {
                    m_xTC = uno::Reference< beans::XPropertySet >(
                        m_xTextFactory->createInstance(
                            ::rtl::OUString::createFromAscii(aIt->second.cFieldServiceName)),
                            uno::UNO_QUERY_THROW);
                    ::rtl::OUString sTCText = lcl_ExtractParameter(rCommand, sizeof(" TC") );
                    if( sTCText.getLength())
                        m_xTC->setPropertyValue(rPropNameSupplier.GetName(PROP_ALTERNATIVE_TEXT),
                            uno::makeAny(sTCText));
                    ::rtl::OUString sValue;
                    // \f TC entry in doc with multiple tables
//                    if( lcl_FindInCommand( rCommand, 'f', sValue ))
//                    {
                        // todo: unsupported
//                    }
                    if( lcl_FindInCommand( rCommand, 'l', sValue ))
                    // \l Outline Level
                    {
                        sal_Int32 nLevel = sValue.toInt32();
                        if( sValue.getLength() && nLevel >= 0 && nLevel <= 10 )
                            m_xTC->setPropertyValue(rPropNameSupplier.GetName(PROP_LEVEL), uno::makeAny( nLevel ));
                    }
//                    if( lcl_FindInCommand( rCommand, 'n', sValue ))
//                    \n Suppress page numbers
//                    {
                        //todo: unsupported feature
//                    }
                }
                break;
            }
        }
        m_xTextField = uno::Reference< text::XTextField >( xFieldInterface, uno::UNO_QUERY );
    }
    catch( uno::Exception& )
    {
    }
}
*/

/*-- 11.09.2006 13:16:35---------------------------------------------------

  -----------------------------------------------------------------------*/
/*bool DomainMapper_Impl::IsFieldAvailable() const
{
    return m_xTextField.is() || m_xTOC.is() || m_xTC.is() || m_sHyperlinkURL.getLength();
}
*/
/*-- 14.09.2006 12:46:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::GetCurrentLocale(lang::Locale& rLocale)
{
    PropertyMapPtr pTopContext = GetTopContext();
    PropertyDefinition aCharLocale( PROP_CHAR_LOCALE, true );
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

/*-- 14.09.2006 12:52:58---------------------------------------------------
    extract the number format from the command and apply the resulting number
    format to the XPropertySet
  -----------------------------------------------------------------------*/
void DomainMapper_Impl::SetNumberFormat( const ::rtl::OUString& rCommand,
                            uno::Reference< beans::XPropertySet >& xPropertySet )
{
    OUString sFormatString = lcl_ParseFormat( rCommand );
    // find \h - hijri/luna calendar todo: what about saka/era calendar?
    bool bHijri = 0 < rCommand.indexOf( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\\h ")));
    lang::Locale aUSLocale;
    aUSLocale.Language = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("en"));
    aUSLocale.Country = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("US"));

    //determine current locale - todo: is it necessary to initialize this locale?
    lang::Locale aCurrentLocale = aUSLocale;
    GetCurrentLocale( aCurrentLocale );
    ::rtl::OUString sFormat = ConversionHelper::ConvertMSFormatStringToSO( sFormatString, aCurrentLocale, bHijri);

    //get the number formatter and convert the string to a format value
    try
    {
        uno::Reference< util::XNumberFormatsSupplier > xNumberSupplier( m_xTextDocument, uno::UNO_QUERY_THROW );
        long nKey = xNumberSupplier->getNumberFormats()->addNewConverted( sFormat, aUSLocale, aCurrentLocale );
        xPropertySet->setPropertyValue(
            PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_NUMBER_FORMAT),
            uno::makeAny( nKey ));
    }
    catch(const uno::Exception&)
    {
    }
}

/*-- 15.09.2006 15:10:20---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySet > DomainMapper_Impl::FindOrCreateFieldMaster(
        const sal_Char* pFieldMasterService, const ::rtl::OUString& rFieldMasterName )
            throw(::com::sun::star::uno::Exception)
{
    // query master, create if not available
    uno::Reference< text::XTextFieldsSupplier > xFieldsSupplier( GetTextDocument(), uno::UNO_QUERY );
    uno::Reference< container::XNameAccess > xFieldMasterAccess = xFieldsSupplier->getTextFieldMasters();
    uno::Reference< beans::XPropertySet > xMaster;
    ::rtl::OUString sFieldMasterService( ::rtl::OUString::createFromAscii(pFieldMasterService) );
    ::rtl::OUStringBuffer aFieldMasterName;
    aFieldMasterName.appendAscii( pFieldMasterService );
    aFieldMasterName.append(sal_Unicode('.'));
    aFieldMasterName.append(rFieldMasterName);
    ::rtl::OUString sFieldMasterName = aFieldMasterName.makeStringAndClear();
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
//        sal_Int32 nIndex = rtl_str_indexOfStr( pFieldMasterService, "Database" );
//        if( nIndex < 0 )
            xMaster->setPropertyValue(
                    PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_NAME),
                    uno::makeAny(rFieldMasterName));
//        else
//        {
//            xMaster->setPropertyValue(
//                    PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_DATA_COLUMN_NAME),
//                    uno::makeAny(rFieldMasterName));
//        }
    }
    return xMaster;
}

/*-- 29.01.2007 11:33:10---------------------------------------------------
//field context starts with a 0x13
  -----------------------------------------------------------------------*/
void DomainMapper_Impl::PushFieldContext()
{
    uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = m_aTextAppendStack.top().xTextAppendAndConvert;
    //insert a dummy char to make sure the start range doesn't move together with the to-be-appended text
    xTextAppendAndConvert->appendTextPortion(::rtl::OUString( '-' ), uno::Sequence< beans::PropertyValue >() );
    uno::Reference< text::XTextCursor > xCrsr = xTextAppendAndConvert->createTextCursorByRange( xTextAppendAndConvert->getEnd() );
    xCrsr->goLeft( 1, false );
    m_aFieldStack.push( FieldContextPtr( new FieldContext( xCrsr->getStart() ) ) );
}
/*-- 29.01.2007 11:33:13---------------------------------------------------
//the current field context waits for the completion of the command
  -----------------------------------------------------------------------*/
bool DomainMapper_Impl::IsOpenFieldCommand() const
{
    return !m_aFieldStack.empty() && !m_aFieldStack.top()->IsCommandCompleted();
}
/*-- 29.01.2007 11:33:13---------------------------------------------------
//the current field context waits for the completion of the command
  -----------------------------------------------------------------------*/
bool DomainMapper_Impl::IsOpenField() const
{
    return !m_aFieldStack.empty();
}
/*-- 29.01.2007 11:49:13---------------------------------------------------

  -----------------------------------------------------------------------*/
FieldContext::FieldContext(uno::Reference< text::XTextRange > xStart) :
    m_bFieldCommandCompleted( false )
    ,m_xStartRange( xStart )
{
}
/*-- 29.01.2007 11:48:44---------------------------------------------------

  -----------------------------------------------------------------------*/
FieldContext::~FieldContext()
{
}
/*-- 29.01.2007 11:48:45---------------------------------------------------

  -----------------------------------------------------------------------*/
void FieldContext::AppendCommand(const ::rtl::OUString& rPart)
{
    m_sCommand += rPart;
}
/*-- 29.01.2007 11:33:15---------------------------------------------------
//collect the pieces of the command
  -----------------------------------------------------------------------*/
void DomainMapper_Impl::AppendFieldCommand(::rtl::OUString& rPartOfCommand)
{
    FieldContextPtr pContext = m_aFieldStack.top();
    OSL_ENSURE( pContext.get(), "no field context available");
    if( pContext.get() )
    {
        pContext->AppendCommand( rPartOfCommand );
    }
}
/*-- 13.12.2007 11:45:43---------------------------------------------------

  -----------------------------------------------------------------------*/
typedef std::multimap < sal_Int32, ::rtl::OUString > TOCStyleMap;
/*-- 29.01.2007 11:33:16---------------------------------------------------
//the field command has to be closed (0x14 appeared)
  -----------------------------------------------------------------------*/
void DomainMapper_Impl::CloseFieldCommand()
{
    FieldContextPtr pContext = m_aFieldStack.top();
    OSL_ENSURE( pContext.get(), "no field context available");
    if( pContext.get() )
    {
        static FieldConversionMap_t aFieldConversionMap;
        static bool bFilled = false;
        m_bSetUserFieldContent = false;
        if(!bFilled)
        {
            static const FieldConversion aFields[] =
            {
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ADDRESSBLOCK")),  "",                         "", FIELD_ADDRESSBLOCK  },
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ADVANCE")),       "",                         "", FIELD_ADVANCE       },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ASK")),           "SetExpression",             "SetExpression", FIELD_ASK      },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AUTONUM")),       "SetExpression",            "SetExpression", FIELD_AUTONUM   },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AUTONUMLGL")),     "SetExpression",            "SetExpression", FIELD_AUTONUMLGL },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AUTONUMOUT")),     "SetExpression",            "SetExpression", FIELD_AUTONUMOUT },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AUTHOR")),        "Author",                   "", FIELD_AUTHOR       },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DATE")),          "DateTime",                 "", FIELD_DATE         },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("COMMENTS")),      "DocInfo.Description",      "", FIELD_COMMENTS     },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CREATEDATE")),    "DocInfo.CreateDateTime",   "", FIELD_CREATEDATE   },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DOCPROPERTY")),   "",                         "", FIELD_DOCPROPERTY },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DOCVARIABLE")),   "User",                     "", FIELD_DOCVARIABLE  },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("EDITTIME")),      "DocInfo.EditTime",         "", FIELD_EDITTIME     },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FILLIN")),        "Input",                    "", FIELD_FILLIN       },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FILENAME")),      "FileName",                 "", FIELD_FILENAME     },
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FILESIZE")),      "",                         "", FIELD_FILESIZE     },
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FORMULA")),     "",                           "", FIELD_FORMULA
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("GOTOBUTTON")),    "",                         "", FIELD_GOTOBUTTON   },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HYPERLINK")),     "",                         "", FIELD_HYPERLINK    },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IF")),            "ConditionalText",          "", FIELD_IF           },
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INFO")),      "","", FIELD_INFO         },
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INCLUDEPICTURE")), "",                        "", FIELD_INCLUDEPICTURE},
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("KEYWORDS")),      "DocInfo.KeyWords",         "", FIELD_KEYWORDS     },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LASTSAVEDBY")),   "DocInfo.ChangeAuthor",                         "", FIELD_LASTSAVEDBY  },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MACROBUTTON")),   "Macro",                         "", FIELD_MACROBUTTON  },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MERGEFIELD")),    "Database",                 "Database", FIELD_MERGEFIELD},
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MERGEREC")),      "DatabaseNumberOfSet",      "", FIELD_MERGEREC     },
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MERGESEQ")),      "",                         "", FIELD_MERGESEQ     },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NEXT")),          "DatabaseNextSet",          "", FIELD_NEXT         },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NEXTIF")),        "DatabaseNextSet",          "", FIELD_NEXTIF       },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PAGE")),          "PageNumber",               "", FIELD_PAGE         },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("REF")),           "GetReference",             "", FIELD_REF          },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("REVNUM")),        "DocInfo.Revision",         "", FIELD_REVNUM       },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SAVEDATE")),      "DocInfo.Change",           "", FIELD_SAVEDATE     },
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SECTION")),       "",                         "", FIELD_SECTION      },
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SECTIONPAGES")),  "",                         "", FIELD_SECTIONPAGES },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SEQ")),           "SetExpression",            "SetExpression", FIELD_SEQ          },
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SET")),           "","", FIELD_SET          },
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SKIPIF")),"",                                 "", FIELD_SKIPIF       },
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("STYLEREF")),"",                               "", FIELD_STYLEREF     },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SUBJECT")),       "DocInfo.Subject",          "", FIELD_SUBJECT      },
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SYMBOL")),"",                                 "", FIELD_SYMBOL       },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TEMPLATE")),      "TemplateName",             "", FIELD_TEMPLATE},
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TIME")),          "DateTime",                 "", FIELD_TIME         },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TITLE")),         "DocInfo.Title",            "", FIELD_TITLE        },
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("USERINITIALS")),  "ExtendedUser",              "", FIELD_USERINITIALS},
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("USERADDRESS")),   "",                         "", FIELD_USERADDRESS  },
    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("USERNAME")),      "ExtendedUser",             "", FIELD_USERNAME     }
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TOC")), "com.sun.star.text.ContentIndex", "", FIELD_TOC},
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TC")), "com.sun.star.text.ContentIndexMark", "", FIELD_TC},
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NUMCHARS")), "CharacterCount", "", FIELD_NUMCHARS},
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NUMWORDS")), "WordCount", "", FIELD_NUMWORDS},
                {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NUMPAGES")), "PageCount", "", FIELD_NUMPAGES},

    //            {::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("")), "", "", FIELD_},

            };
            size_t nConversions = sizeof(aFields)/sizeof(FieldConversion);
            for( size_t nConversion = 0; nConversion < nConversions; ++nConversion)
            {
                aFieldConversionMap.insert( FieldConversionMap_t::value_type(
                        aFields[nConversion].sWordCommand,
                        aFields[nConversion] ));
            }

            bFilled = true;
        }
        try
        {
            uno::Reference< uno::XInterface > xFieldInterface;
            //at first determine the field type - erase leading and trailing whitespaces
            ::rtl::OUString sCommand( pContext->GetCommand().trim() );
            sal_Int32 nSpaceIndex = sCommand.indexOf( ' ' );
            if( 0 <= nSpaceIndex )
                sCommand = sCommand.copy( 0, nSpaceIndex );

            FieldConversionMap_t::iterator aIt = aFieldConversionMap.find(sCommand);
            if(aIt != aFieldConversionMap.end())
            {
                uno::Reference< beans::XPropertySet > xFieldProperties;
                if( FIELD_HYPERLINK != aIt->second.eFieldId &&
                        FIELD_DOCPROPERTY != aIt->second.eFieldId &&
                        FIELD_TOC != aIt->second.eFieldId &&
                        FIELD_TC != aIt->second.eFieldId)
                {
                    //add the service prefix
                    OUString sServiceName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextField."));
                    sServiceName += ::rtl::OUString::createFromAscii(aIt->second.cFieldServiceName );
                    xFieldInterface = m_xTextFactory->createInstance(sServiceName);
                    xFieldProperties = uno::Reference< beans::XPropertySet >( xFieldInterface, uno::UNO_QUERY_THROW);
                }
                PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
                switch( aIt->second.eFieldId )
                {
                    case FIELD_ADDRESSBLOCK: break;
                    case FIELD_ADVANCE     : break;
                    case FIELD_ASK         :
                    {
                        //doesn the command contain a variable name?
                        ::rtl::OUString sVariable, sHint;

                        sVariable = lcl_ExctractAskVariableAndHint( pContext->GetCommand(), sHint );
                        if(sVariable.getLength())
                        {
                            // determine field master name
                            uno::Reference< beans::XPropertySet > xMaster = FindOrCreateFieldMaster(
                                    "com.sun.star.text.FieldMaster.SetExpression", sVariable );

                            // attach the master to the field
                            uno::Reference< text::XDependentTextField > xDependentField( xFieldInterface, uno::UNO_QUERY_THROW );
                            xDependentField->attachTextFieldMaster( xMaster );

                            // set input flag at the field
                            xFieldProperties->setPropertyValue(
                                    rPropNameSupplier.GetName(PROP_IS_INPUT), uno::makeAny( true ));
                            // set the prompt
                            xFieldProperties->setPropertyValue(
                                    rPropNameSupplier.GetName(PROP_HINT),
                                    uno::makeAny( sHint ));
                        }
                        else
                        {
                            //don't insert the field
                            //todo: maybe import a 'normal' input field here?
                            xFieldInterface = 0;
                        }
                    }
                    break;
                    case FIELD_AUTONUM    :
                    case FIELD_AUTONUMLGL :
                    case FIELD_AUTONUMOUT :
                    {
                        //create a sequence field master "AutoNr"
                        uno::Reference< beans::XPropertySet > xMaster = FindOrCreateFieldMaster(
                                    "com.sun.star.text.FieldMaster.SetExpression",
                                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AutoNr") ));

                        xMaster->setPropertyValue( rPropNameSupplier.GetName(PROP_SUB_TYPE),
                                    uno::makeAny(text::SetVariableType::SEQUENCE));

                        //apply the numbering type
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_NUMBERING_TYPE),
                            uno::makeAny( lcl_ParseNumberingType(pContext->GetCommand()) ));
                            // attach the master to the field
                        uno::Reference< text::XDependentTextField > xDependentField( xFieldInterface, uno::UNO_QUERY_THROW );
                        xDependentField->attachTextFieldMaster( xMaster );
                    }
                    break;
                    case FIELD_AUTHOR       :
                    {
                        xFieldProperties->setPropertyValue( rPropNameSupplier.GetName(PROP_FULL_NAME), uno::makeAny( true ));
                        ::rtl::OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" AUTHOR") );
                        if(sParam.getLength())
                        {
                            xFieldProperties->setPropertyValue(
                                    rPropNameSupplier.GetName( PROP_IS_FIXED ), uno::makeAny( true ));
                            //PROP_CURRENT_PRESENTATION is set later anyway
                        }
                    }
                    break;
                    case FIELD_DATE:
                    {
                        //not fixed,
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_IS_FIXED),
                            uno::makeAny( false ));
                        SetNumberFormat( pContext->GetCommand(), xFieldProperties );
                    }
                    break;
                    case FIELD_COMMENTS     :
                    {
                        ::rtl::OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" COMMENTS") );
                        if(sParam.getLength())
                        {
                            xFieldProperties->setPropertyValue(
                                    rPropNameSupplier.GetName( PROP_IS_FIXED ), uno::makeAny( true ));
                            //PROP_CURRENT_PRESENTATION is set later anyway
                        }
                    }
                    break;
                    case FIELD_CREATEDATE  :
                    {
                        SetNumberFormat( pContext->GetCommand(), xFieldProperties );
                    }
                    break;
                    case FIELD_DOCPROPERTY :
                    {
                        //some docproperties should be imported as document statistic fields, some as DocInfo fields
                        //others should be user fields
                        ::rtl::OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" DOCPROPERTY") );
                        if(sParam.getLength())
                        {
                            #define SET_ARABIC      0x01
                            #define SET_FULL_NAME   0x02
                            struct DocPropertyMap
                            {
                                const sal_Char* pDocPropertyName;
                                const sal_Char* pServiceName;
                                sal_uInt8       nFlags;
                            };
                            static const DocPropertyMap aDocProperties[] =
                            {
                                {"Author",           "Author",                  SET_FULL_NAME},
                                {"CreateTime",       "DocInfo.CreateDateTime",  0},
                                {"Characters",       "CharacterCount",          SET_ARABIC},
                                {"Comments",         "DocInfo.Description",     0},
                                {"Keywords",         "DocInfo.KeyWords",        0},
                                {"LastPrinted",      "DocInfo.PrintDateTime",   0},
                                {"LastSavedBy",      "DocInfo.ChangeAuthor",    0},
                                {"LastSavedTime",    "DocInfo.ChangeDateTime",  0},
                                {"Paragraphs",       "ParagraphCount",          SET_ARABIC},
                                {"RevisionNumber",   "DocInfo.Revision",        0},
                                {"Subject",          "DocInfo.Subject",         0},
                                {"Template",         "TemplateName",            0},
                                {"Title",            "DocInfo.Title",           0},
                                {"TotalEditingTime", "DocInfo.EditTime",        9},
                                {"Words",            "WordCount",               SET_ARABIC}

                                //other available DocProperties:
                                //Bytes, Category, CharactersWithSpaces, Company
                                //HyperlinkBase,
                                //Lines, Manager, NameofApplication, ODMADocId, Pages,
                                //Security,
                            };
                            //search for a field mapping
                            ::rtl::OUString sFieldServiceName;
                            sal_uInt16 nMap = 0;
                            for( ; nMap < sizeof(aDocProperties) / sizeof(DocPropertyMap); ++nMap )
                            {
                                if(sParam.equalsAscii(aDocProperties[nMap].pDocPropertyName))
                                {
                                    sFieldServiceName = ::rtl::OUString::createFromAscii(aDocProperties[nMap].pServiceName);
                                    break;
                                }
                            }
                            ::rtl::OUString sServiceName(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextField."));
                            if(sFieldServiceName.getLength())
                            {
                                sServiceName += sFieldServiceName;
                                xFieldInterface = m_xTextFactory->createInstance(sServiceName);
                                xFieldProperties = uno::Reference< beans::XPropertySet >( xFieldInterface, uno::UNO_QUERY_THROW);
                                if(0 != (aDocProperties[nMap].nFlags & SET_ARABIC))
                                    xFieldProperties->setPropertyValue(
                                        rPropNameSupplier.GetName(PROP_NUMBERING_TYPE),
                                        uno::makeAny( style::NumberingType::ARABIC ));
                                else if(0 != (aDocProperties[nMap].nFlags & SET_FULL_NAME))
                                    xFieldProperties->setPropertyValue(
                                        rPropNameSupplier.GetName(PROP_FULL_NAME), uno::makeAny( true ));

                            }
                            else
                            {
                                //create a user field and type
                                uno::Reference< beans::XPropertySet > xMaster =
                                    FindOrCreateFieldMaster( "com.sun.star.text.FieldMaster.User", sParam );
                                sServiceName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("User"));
                                xFieldInterface = m_xTextFactory->createInstance(sServiceName);
                                xFieldProperties = uno::Reference< beans::XPropertySet >( xFieldInterface, uno::UNO_QUERY_THROW);
                                uno::Reference< text::XDependentTextField > xDependentField( xFieldInterface, uno::UNO_QUERY_THROW );
                                xDependentField->attachTextFieldMaster( xMaster );
                                m_bSetUserFieldContent = true;
                            }
                        }
                    }
                    #undef SET_ARABIC
                    #undef SET_FULL_NAME
                    break;
                    case FIELD_DOCVARIABLE  :
                    {
                        ::rtl::OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" DOCVARIABLE") );
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
                    case FIELD_FILLIN       :
                    {
                        sal_Int32 nIndex = 0;
                        xFieldProperties->setPropertyValue(
                                rPropNameSupplier.GetName(PROP_HINT), uno::makeAny( pContext->GetCommand().getToken( 1, '\"', nIndex)));
                    }
                    break;
                    case FIELD_FILENAME:
                    {
                        sal_Int32 nNumberingTypeIndex = pContext->GetCommand().indexOf( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\\p")));
                        xFieldProperties->setPropertyValue(
                                rPropNameSupplier.GetName(PROP_FILE_FORMAT),
                                uno::makeAny( nNumberingTypeIndex > 0 ? text::FilenameDisplayFormat::FULL : text::FilenameDisplayFormat::NAME ));
                    }
                    break;
                    case FIELD_FILESIZE     : break;
                    case FIELD_FORMULA : break;
                    case FIELD_GOTOBUTTON   : break;
                    case FIELD_HYPERLINK:
                    {
                        sal_Int32 nStartQuote = pContext->GetCommand().indexOf( '\"' );
                        sal_Int32 nEndQuote = nStartQuote < pContext->GetCommand().getLength() + 1 ? pContext->GetCommand().indexOf( '\"', nStartQuote + 1) : -1;
                        if( nEndQuote > 0)
                            pContext->SetHyperlinkURL( pContext->GetCommand().copy(nStartQuote + 1, nEndQuote - nStartQuote - 1) );
                    }
                    break;
                    case FIELD_IF           : break;
                    case FIELD_INFO         : break;
                    case FIELD_INCLUDEPICTURE: break;
                    case FIELD_KEYWORDS     :
                    {
                        ::rtl::OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" KEYWORDS") );
                        if(sParam.getLength())
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
                        ::rtl::OUString sMacro = pContext->GetCommand().getToken( 0, ' ', nIndex);
                        xFieldProperties->setPropertyValue(
                                rPropNameSupplier.GetName(PROP_MACRO_NAME), uno::makeAny( sMacro ));

                        //extract quick help text
                        if( pContext->GetCommand().getLength() > nIndex + 1)
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
                        ::rtl::OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" MERGEFIELD") );
                        //create a user field and type
                        uno::Reference< beans::XPropertySet > xMaster =
                            FindOrCreateFieldMaster( "com.sun.star.text.FieldMaster.Database", sParam );

    //                    xFieldProperties->setPropertyValue(
    //                             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FieldCode")),
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
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_NUMBERING_TYPE),
                            uno::makeAny( lcl_ParseNumberingType(pContext->GetCommand()) ));
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_SUB_TYPE),
                            uno::makeAny( text::PageNumberType_CURRENT ));

                    break;
                    case FIELD_REF:
                    {
                        ::rtl::OUString sBookmark = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" REF") );
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_REFERENCE_FIELD_SOURCE),
                            uno::makeAny( sal_Int16(text::ReferenceFieldSource::BOOKMARK)) );
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_SOURCE_NAME),
                            uno::makeAny( sBookmark) );
                        sal_Int16 nFieldPart = text::ReferenceFieldPart::TEXT;
                        ::rtl::OUString sValue;
                        if( lcl_FindInCommand( pContext->GetCommand(), 'p', sValue ))
                        {
                            //above-below
                            nFieldPart = text::ReferenceFieldPart::UP_DOWN;
                        }
                        xFieldProperties->setPropertyValue(
                                rPropNameSupplier.GetName( PROP_REFERENCE_FIELD_PART ), uno::makeAny( nFieldPart ));
                    }
                    break;
                    case FIELD_REVNUM       : break;
                    case FIELD_SAVEDATE     :
                        SetNumberFormat( pContext->GetCommand(), xFieldProperties );
                    break;
                    case FIELD_SECTION      : break;
                    case FIELD_SECTIONPAGES : break;
                    case FIELD_SEQ          : break;
                    case FIELD_SET          : break;
                    case FIELD_SKIPIF       : break;
                    case FIELD_STYLEREF     : break;
                    case FIELD_SUBJECT      :
                    {
                        ::rtl::OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" SUBJECT") );
                        if(sParam.getLength())
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
                        ::rtl::OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" TITLE") );
                        if(sParam.getLength())
                        {
                            xFieldProperties->setPropertyValue(
                                    rPropNameSupplier.GetName( PROP_IS_FIXED ), uno::makeAny( true ));
                            //PROP_CURRENT_PRESENTATION is set later anyway
                        }
                    }
                    break;
                    case FIELD_USERINITIALS:
                    {
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_USER_DATA_TYPE), uno::makeAny( text::UserDataPart::SHORTCUT ));
                        //todo: if initials are provided - set them as fixed content
                        ::rtl::OUString sParam = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" USERINITIALS") );
                        if(sParam.getLength())
                        {
                            xFieldProperties->setPropertyValue(
                                    rPropNameSupplier.GetName( PROP_IS_FIXED ), uno::makeAny( true ));
                            //PROP_CURRENT_PRESENTATION is set later anyway
                        }
                    }
                    break;
                    case FIELD_USERADDRESS  : //todo: user address collects street, city ...
                    break;
                    case FIELD_USERNAME     : //todo: user name is firstname + lastname
                    break;
                    case FIELD_TOC:
                    {
                        ::rtl::OUString sValue;
                        bool bTableOfFigures = false;
                        bool bHyperlinks = false;
                        bool bFromOutline = false;
                        bool bFromEntries = false;
                        ::rtl::OUString sTemplate;
                        ::rtl::OUString sChapterNoSeparator;
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
    //                  \n Builds a table of contents or a range of entries, sucah as “1-9”, in a table of contents without page numbers
    //                    if( lcl_FindInCommand( pContext->GetCommand(), 'n', sValue ))
    //                    {
                            //todo: what does the description mean?
    //                    }
    //                  \o  Builds a table of contents by using outline levels instead of TC entries
                        if( lcl_FindInCommand( pContext->GetCommand(), 'o', sValue ))
                        {
                            bFromOutline = true;
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
                            ::rtl::OUString sToken = sValue.getToken( 1, '"', nPos);
                            sTemplate = sToken.getLength() ? sToken : sValue;
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

                        uno::Reference< beans::XPropertySet > xTOC(
                                m_xTextFactory->createInstance(
                                    bTableOfFigures ?
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.IllustrationsIndex")) :
                                    ::rtl::OUString::createFromAscii(aIt->second.cFieldServiceName)),
                                    uno::UNO_QUERY_THROW);
                        xTOC->setPropertyValue(rPropNameSupplier.GetName( PROP_TITLE ), uno::makeAny(::rtl::OUString()));
                        if( !bTableOfFigures )
                        {
                            xTOC->setPropertyValue( rPropNameSupplier.GetName( PROP_CREATE_FROM_OUTLINE ), uno::makeAny( bFromOutline ));
                            xTOC->setPropertyValue( rPropNameSupplier.GetName( PROP_CREATE_FROM_MARKS ), uno::makeAny( bFromEntries ));
                            if( sTemplate.getLength() )
                            {
                                //the string contains comma separated the names and related levels
                                //like: "Heading 1,1,Heading 2,2"
                                TOCStyleMap aMap;
                                sal_Int32 nLevel;
                                sal_Int32 nPosition = 0;
                                while( nPosition >= 0)
                                {
                                    ::rtl::OUString sStyleName = sTemplate.getToken( 0, ',', nPosition );
                                    //empty tokens should be skipped
                                    while( !sStyleName.getLength() && nPosition > 0 )
                                        sStyleName = sTemplate.getToken( 0, ',', nPosition );
                                    nLevel = sTemplate.getToken( 0, ',', nPosition ).toInt32();
                                    if( !nLevel )
                                        nLevel = 1;
                                    if( sStyleName.getLength() )
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

                                        uno::Sequence< rtl::OUString> aStyles( nLevelCount );
                                        for ( sal_Int32 nStyle = 0; nStyle < nLevelCount; ++nStyle, ++aTOCStyleIter )
                                        {
                                            aStyles[nStyle] = aTOCStyleIter->second;
                                        }
                                        xParaStyles->replaceByIndex(nLevel - 1, uno::makeAny(aStyles));
                                    }
                                }
                                xTOC->setPropertyValue(rPropNameSupplier.GetName(PROP_CREATE_FROM_LEVEL_PARAGRAPH_STYLES), uno::makeAny( true ));

                            }
                            if(bHyperlinks  || sChapterNoSeparator.getLength())
                            {
                                uno::Reference< container::XIndexReplace> xLevelFormats;
                                xTOC->getPropertyValue(rPropNameSupplier.GetName(PROP_LEVEL_FORMAT)) >>= xLevelFormats;
                                sal_Int32 nLevelCount = xLevelFormats->getCount();
                                //start with level 1, 0 is the header level
                                for( sal_Int32 nLevel = 1; nLevel < nLevelCount; ++nLevel)
                                {
                                    uno::Sequence< beans::PropertyValues > aLevel;
                                    xLevelFormats->getByIndex( nLevel ) >>= aLevel;
                                    //create a copy of the level and add two new entries - hyperlink start and end
                                    bool bChapterNoSeparator  = sChapterNoSeparator.getLength() > 0;
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

                                    xLevelFormats->replaceByIndex( nLevel, uno::makeAny( aNewLevel ) );
                                }
                            }
                        }
                        pContext->SetTOC( xTOC );
                    }
                    break;
                    case FIELD_TC :
                    {
                        uno::Reference< beans::XPropertySet > xTC(
                            m_xTextFactory->createInstance(
                                ::rtl::OUString::createFromAscii(aIt->second.cFieldServiceName)),
                                uno::UNO_QUERY_THROW);
                        ::rtl::OUString sTCText = lcl_ExtractParameter(pContext->GetCommand(), sizeof(" TC") );
                        if( sTCText.getLength())
                            xTC->setPropertyValue(rPropNameSupplier.GetName(PROP_ALTERNATIVE_TEXT),
                                uno::makeAny(sTCText));
                        ::rtl::OUString sValue;
                        // \f TC entry in doc with multiple tables
    //                    if( lcl_FindInCommand( pContext->GetCommand(), 'f', sValue ))
    //                    {
                            // todo: unsupported
    //                    }
                        if( lcl_FindInCommand( pContext->GetCommand(), 'l', sValue ))
                        // \l Outline Level
                        {
                            sal_Int32 nLevel = sValue.toInt32();
                            if( sValue.getLength() && nLevel >= 0 && nLevel <= 10 )
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
                        xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_NUMBERING_TYPE),
                            uno::makeAny( lcl_ParseNumberingType(pContext->GetCommand()) ));
                        break;

                }
            }
            //set the text field if there is any
            pContext->SetTextField( uno::Reference< text::XTextField >( xFieldInterface, uno::UNO_QUERY ) );
        }
        catch( uno::Exception& )
        {
        }
        pContext->SetCommandCompleted();
    }
}
/*-- 29.01.2007 11:33:16---------------------------------------------------
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
/*-- 01.09.2006 11:48:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::SetFieldResult( ::rtl::OUString& rResult )
{
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
                        xFieldProperties->setPropertyValue(
                                rPropNameSupplier.GetName(PROP_CURRENT_PRESENTATION),
                             uno::makeAny( rResult ));
                    }
                }
                catch( const beans::UnknownPropertyException& )
                {
                    //some fields don't have a CurrentPresentation (DateTime)
                }
            }
/*            else if(m_sHyperlinkURL.getLength())
            {
                //insert the string and apply hyperlink attribute to it
                uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = m_aTextAppendStack.top().xTextAppendAndConvert;
                if(xTextAppendAndConvert.is())
                {
                    try
                    {
                        uno::Sequence< beans::PropertyValue> aLink(1);
                        aLink[0].Name = rPropNameSupplier.GetName(PROP_HYPER_LINK_U_R_L);
                        aLink[0].Value = uno::makeAny(m_sHyperlinkURL);
                        xTextAppendAndConvert->appendTextPortion(rResult, aLink);
                    }
                    catch(const lang::IllegalArgumentException& )
                    {
                    }
                    catch(const uno::Exception& )
                    {
                    }
                }
            }
            else if( m_xTOC.is() || m_xTC.is() )
            {
                uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = m_aTextAppendStack.top().xTextAppendAndConvert;
                if(xTextAppendAndConvert.is())
                {
                    try
                    {
                        xTextAppendAndConvert->appendTextContent(
                            uno::Reference< text::XTextContent >(m_xTOC.is() ? m_xTOC : m_xTC, uno::UNO_QUERY_THROW));
                    }
                    catch(const lang::IllegalArgumentException& )
                    {
                    }
                    catch(const uno::Exception& )
                    {
                    }
                    m_xTOC = 0;
                    m_xTC = 0;
                }
            }*/
        }
        catch( uno::Exception& )
        {

        }
    }
//    m_xTextField = 0;
//    m_sHyperlinkURL = OUString();
}

/*-- 29.01.2007 11:33:17---------------------------------------------------
//the end of field is reached (0x15 appeared) - the command might still be open
  -----------------------------------------------------------------------*/
void DomainMapper_Impl::PopFieldContext()
{
    FieldContextPtr pContext = m_aFieldStack.top();
    OSL_ENSURE( pContext.get(), "no field context available");
    if( pContext.get() )
    {
        if( !pContext->IsCommandCompleted() )
            CloseFieldCommand();

        //insert the field, TC or TOC
        uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = m_aTextAppendStack.top().xTextAppendAndConvert;
        if(xTextAppendAndConvert.is())
        {
            try
            {
                uno::Reference< text::XTextCursor > xCrsr = xTextAppendAndConvert->createTextCursorByRange(pContext->GetStartRange());
                //remove the dummy character
                xCrsr->goRight( 1, true );
                xCrsr->setString( ::rtl::OUString() );
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
                        xTextAppendAndConvert->appendTextContent( xToInsert, uno::Sequence< beans::PropertyValue >() );
                    else if(pContext->GetHyperlinkURL().getLength())
                    {
                        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
                        xCrsr->gotoEnd( true );

                        uno::Reference< beans::XPropertySet > xCrsrProperties( xCrsr, uno::UNO_QUERY_THROW );
                        xCrsrProperties->setPropertyValue(
                                rPropNameSupplier.GetName(PROP_HYPER_LINK_U_R_L), uno::
                                makeAny(pContext->GetHyperlinkURL()));
                    }
                }
            }
            catch(const lang::IllegalArgumentException& )
            {
            }
            catch(const uno::Exception& )
            {
            }
        }
        //
        //TOCs have to include all the imported content
        //...
    }
    //remove the field context
    m_aFieldStack.pop();
}
/*-- 11.06.2007 16:19:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::AddBookmark( const ::rtl::OUString& rBookmarkName, const ::rtl::OUString& rId )
{
    uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = m_aTextAppendStack.top().xTextAppendAndConvert;
    BookmarkMap_t::iterator aBookmarkIter = m_aBookmarkMap.find( rId );
    //is the bookmark name already registered?
    try
    {
        if( aBookmarkIter != m_aBookmarkMap.end() )
        {
            static const rtl::OUString sBookmarkService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.Bookmark"));
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

            xCursor->gotoRange( xTextAppendAndConvert->getEnd(), true );
            uno::Reference< container::XNamed > xBkmNamed( xBookmark, uno::UNO_QUERY_THROW );
            //todo: make sure the name is not used already!
            xBkmNamed->setName( aBookmarkIter->second.m_sBookmarkName );
            xTextAppendAndConvert->insertTextContent( uno::Reference< text::XTextRange >( xCursor, uno::UNO_QUERY_THROW), xBookmark, !xCursor->isCollapsed() );
            m_aBookmarkMap.erase( aBookmarkIter );
        }
        else
        {
            //otherwise insert a text range as marker
            uno::Reference< text::XTextCursor > xCursor = xTextAppendAndConvert->createTextCursorByRange( xTextAppendAndConvert->getEnd() );
            bool bIsStart = !xCursor->goLeft(1, false);
            uno::Reference< text::XTextRange > xCurrent = xCursor->getStart();
            m_aBookmarkMap.insert(BookmarkMap_t::value_type( rId, BookmarkInsertPosition( bIsStart, rBookmarkName, xCurrent ) ));
        }
    }
    catch( const uno::Exception& )
    {
        //TODO: What happens to bookmarks where start and end are at different XText objects?
    }
}
/*-- 01.11.2006 14:57:44---------------------------------------------------

  -----------------------------------------------------------------------*/
GraphicImportPtr DomainMapper_Impl::GetGraphicImport(GraphicImportType eGraphicImportType)
{
    if(!m_pGraphicImport)
        m_pGraphicImport.reset( new GraphicImport( m_xComponentContext, m_xTextFactory, m_rDMapper, eGraphicImportType ) );
    return m_pGraphicImport;
}
/*-- 09.08.2007 10:19:45---------------------------------------------------
    reset graphic import if the last import resulted in a shape, not a graphic
  -----------------------------------------------------------------------*/
void DomainMapper_Impl::ResetGraphicImport()
{
    m_pGraphicImport.reset();
}
/*-- 01.11.2006 09:25:40---------------------------------------------------

  -----------------------------------------------------------------------*/
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

#ifdef DEBUG_DOMAINMAPPER
    uno::Reference<drawing::XShape> xShape
        (xTextContent, uno::UNO_QUERY_THROW);
    {
        uno::Reference<beans::XPropertySet> xPropSet
            (xShape, uno::UNO_QUERY_THROW);
        logger("DOMAINMAPPER", "<shapeprops-before>");
        try
        {
            logger("DOMAINMAPPER", propertysetToString(xPropSet));
        }
        catch (...)
        {
            logger("DOMAINMAPPER", "<exception/>");
        }
        logger("DOMAINMAPPER", "</shapeprops-before>");
    }
#endif
    //insert it into the document at the current cursor position
    OSL_ENSURE( xTextContent.is(), "DomainMapper_Impl::ImportGraphic");
    if( xTextContent.is())
        appendTextContent( xTextContent, uno::Sequence< beans::PropertyValue >() );

#ifdef DEBUG_DOMAINMAPPER
    {
        uno::Reference<beans::XPropertySet> xPropSet
            (xShape, uno::UNO_QUERY_THROW);
        logger("DOMAINMAPPER", "<shapeprops-after>");
        try
        {
            logger("DOMAINMAPPER", propertysetToString(xPropSet));
        }
        catch (...)
        {
            logger("DOMAINMAPPER", "<exception/>");
        }
        awt::Size aSize(xShape->getSize());
        awt::Point aPoint(xShape->getPosition());
        char buffer[256];
        snprintf(buffer, sizeof(buffer),
                 "<shape x=\"%ld\" y=\"%ld\" width=\"%ld\" height=\"%ld\">",
                 aPoint.X, aPoint.Y, aSize.Width, aSize.Height);
        logger("DOMAINMAPPER", buffer);
        logger("DOMAINMAPPER", "</shape>");
        logger("DOMAINMAPPER", "</shapeprops-after>");
    }
#endif

    m_pGraphicImport.reset();
}

/*-- 28.12.2006 14:00:47---------------------------------------------------

  -----------------------------------------------------------------------*/
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



/*
        { SW_PROP_NAME(UNO_NAME_CHAR_STYLE_NAME
        { SW_PROP_NAME(UNO_NAME_COUNT_EMPTY_LINES
        { SW_PROP_NAME(UNO_NAME_COUNT_LINES_IN_FRAMES
        { SW_PROP_NAME(UNO_NAME_DISTANCE
        { SW_PROP_NAME(UNO_NAME_IS_ON
        { SW_PROP_NAME(UNO_NAME_INTERVAL
        { SW_PROP_NAME(UNO_NAME_SEPARATOR_TEXT
        { SW_PROP_NAME(UNO_NAME_NUMBER_POSITION
        { SW_PROP_NAME(UNO_NAME_NUMBERING_TYPE
        { SW_PROP_NAME(UNO_NAME_RESTART_AT_EACH_PAGE
        { SW_PROP_NAME(UNO_NAME_SEPARATOR_INTERVAL
*/
    }
    m_bLineNumberingSet = true;
}
/*-- 31.08.2007 13:50:49---------------------------------------------------

  -----------------------------------------------------------------------*/
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

/*-- 31.08.2007 13:47:50---------------------------------------------------

  -----------------------------------------------------------------------*/
_PageMar::_PageMar()
{
    header = footer = top = bottom = ConversionHelper::convertTwipToMM100( sal_Int32(1440));
    right = left = ConversionHelper::convertTwipToMM100( sal_Int32(1800));
    gutter = 0;
}

}}
