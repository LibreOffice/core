#ifndef INCLUDED_DMAPPER_DOMAINMAPPER_IMPL_HXX
#include <DomainMapper_Impl.hxx>
#endif
#ifndef INCLUDED_DMAPPER_CONVERSIONHELPER_HXX
#include <ConversionHelper.hxx>
#endif
#ifndef INCLUDED_DOMAIN_MAPPER_TABLE_HANDLER_HXX
#include <DomainMapperTableHandler.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELD_HPP_
#include <com/sun/star/text/XTextField.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELDSSUPPLIER_HPP_
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDEPENDENTTEXTFIELD_HPP_
#include <com/sun/star/text/XDependentTextField.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_PAGENUMBERTYPE_HPP_
#include <com/sun/star/text/PageNumberType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_FILENAMEDISPLAYFORMAT_HPP_
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_USERDATAPART_HPP_
#include <com/sun/star/text/UserDataPart.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_SETVARIABLETYPE_HPP_
#include <com/sun/star/text/SetVariableType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATS_HPP_
#include <com/sun/star/util/XNumberFormats.hpp>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _RTL_STRING_H_
#include <rtl/string.h>
#endif


#include <hash_map>

using namespace ::com::sun::star;
using namespace ::rtl;
using namespace ::writerfilter;
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
        todo: not imported yet
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
};
struct FieldConversion
{
    ::rtl::OUString     sWordCommand;
    const sal_Char*     cFieldServiceName;
    const sal_Char*     cFieldMasterServiceName;
    FieldId             eFieldId;
};

struct FieldConversionHash
{
    unsigned long operator()(const OUString& rKey) const
    {
        return static_cast<unsigned long>(rKey.hashCode());
    }
};

struct FieldConversionEq
{
    bool operator() (const OUString& rA, const OUString& rB) const
    {
        return rA == rB;
    }
};

typedef ::std::hash_map< ::rtl::OUString, FieldConversion, FieldConversionHash, FieldConversionEq>
            FieldConversionMap_t;

/*-- 18.07.2006 08:56:55---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 FIB::GetData( doctok::Id nName )
{
    if( nName >= NS_rtf::LN_WIDENT && nName <= NS_rtf::LN_LCBSTTBFUSSR)
        return aFIBData[nName - NS_rtf::LN_WIDENT];
    OSL_ASSERT("invalid index in FIB");
    return -1;
}
/*-- 18.07.2006 08:56:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void FIB::SetData( doctok::Id nName, sal_Int32 nValue )
{
    OSL_ENSURE( nName >= NS_rtf::LN_WIDENT && nName <= NS_rtf::LN_LCBSTTBFUSSR, "invalid index in FIB");
    if( nName >= NS_rtf::LN_WIDENT && nName <= NS_rtf::LN_LCBSTTBFUSSR)
        aFIBData[nName - NS_rtf::LN_WIDENT] = nValue;
}
/*-- 01.09.2006 10:22:03---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapper_Impl::DomainMapper_Impl(
            DomainMapper& rDMapper,
            uno::Reference< lang::XComponent >  xModel ) :
        m_rDMapper( rDMapper ),
        m_xTextDocument( xModel, uno::UNO_QUERY ),
        m_xTextFactory( xModel, uno::UNO_QUERY ),
        m_bFieldMode( false ),
        m_bSetUserFieldContent( false ),
        m_nCurrentTabStopIndex( 0 ),
        m_nCurrentParaStyleId( -1 ),
        m_bInStyleSheetImport( false )
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
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
void    DomainMapper_Impl::PushProperties(ContextType eId)
{
    PropertyMapPtr pInsert(eId == CONTEXT_SECTION ? new SectionPropertyMap : new PropertyMap);
    m_aPropertyStacks[eId].push( pInsert );
    m_aContextStack.push(eId);

    m_pTopContext = m_aPropertyStacks[eId].top();
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
        OSL_ENSURE(eId == CONTEXT_SECTION, "this should happen at a section context end");
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
void DomainMapper_Impl::ModifyCurrentTabStop( doctok::Id nId, sal_Int32 nValue)
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
                    DeletableTabStop(style::TabStop(lcl_convertToMM100(nValue), style::TabAlign_LEFT, ' ', ' ')));
        break;
        case NS_rtf::LN_dxaDel: //deleted tab
        {
            //mark the tab stop at the given position as deleted
            ::std::vector<DeletableTabStop>::iterator aIt = m_aCurrentTabStops.begin();
            ::std::vector<DeletableTabStop>::iterator aEndIt = m_aCurrentTabStops.end();
            sal_Int32 nConverted = lcl_convertToMM100(nValue);
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
                (m_aCurrentTabStops.begin() + m_nCurrentTabStopIndex)->FillChar = aTabFillCharWord[nValue];
        break;
        case NS_rtf::LN_JC: //tab justification
            //0 - left, 1 - centered, 2 - right, 3 - decimal 4 - bar
            if( nValue >= 0 && nValue < sal::static_int_cast<sal_Int32>(sizeof(aTabAlignFromWord) / sizeof (style::TabAlign)))
                (m_aCurrentTabStops.begin() + m_nCurrentTabStopIndex)->Alignment = aTabAlignFromWord[nValue];
        break;
    }
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
        pEntry = GetStyleSheetTable()->FindParentStyleSheet(-1);
    else
        pEntry =
                GetStyleSheetTable()->FindStyleSheetByISTD(GetCurrentParaStyleId());
    while(pEntry)
    {
        //is there a tab stop set?
        if(pEntry->pProperties)
        {
            PropertyMap::const_iterator aPropertyIter =
                    pEntry->pProperties->find(PropertyNameSupplier::GetPropertyNameSupplier().GetName((eId)));
            if( aPropertyIter != pEntry->pProperties->end())
            {
                return aPropertyIter->second;
            }
        }
        //search until the property is set or no parent is available
        pEntry = GetStyleSheetTable()->FindParentStyleSheet(pEntry->nBaseStyleIdentifier);
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
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::finishParagraph( PropertyMapPtr pPropertyMap )
{
    uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = m_aTextAppendStack.top();
    if(xTextAppendAndConvert.is())
    {
        try
        {
            uno::Reference< text::XTextRange > xTextRange =
                xTextAppendAndConvert->finishParagraph
                (pPropertyMap->GetPropertyValues());

            m_TableManager.handle(xTextRange);
        }
        catch(const lang::IllegalArgumentException& )
        {
        }
        catch(const uno::Exception& )
        {
        }
    }
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::appendTextPortion( const ::rtl::OUString& rString, PropertyMapPtr pPropertyMap )
{
    uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = m_aTextAppendStack.top();
    if(xTextAppendAndConvert.is())
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
/*-- 24.07.2006 09:41:20---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString lcl_FindUnusedPageStyleName(const uno::Sequence< ::rtl::OUString >& rPageStyleNames)
{
    static const sal_Char cDefaultStyle[] = "Converted";
    //find the hightest number x in each style with the name "cDefaultStyle+x" and
    //return an incremented name
    sal_Int32 nMaxIndex = 0;
    const sal_Int32 nDefaultLength = sizeof(cDefaultStyle)/sizeof(sal_Char) - 1;
    const ::rtl::OUString sDefaultStyle( cDefaultStyle, nDefaultLength, RTL_TEXTENCODING_ASCII_US );

    const ::rtl::OUString* pStyleNames = rPageStyleNames.getConstArray();
    for( sal_Int32 nStyle = 0; nStyle < rPageStyleNames.getLength(); ++nStyle)
    {
        if( pStyleNames[nStyle].getLength() > nDefaultLength &&
                !rtl_ustr_compare_WithLength( sDefaultStyle, nDefaultLength, pStyleNames[nStyle], nDefaultLength))
        {
            sal_Int32 nIndex = pStyleNames[nStyle].copy( nDefaultLength ).toInt32();
            if( nIndex > nMaxIndex)
                nMaxIndex = nIndex;
        }
    }
    ::rtl::OUString sRet( sDefaultStyle );
    sRet += rtl::OUString::valueOf( nMaxIndex + 1);
    return sRet;
}

void DomainMapper_Impl::PushPageHeader(SectionPropertyMap::PageType eType)
{
    //get the section context
    PropertyMapPtr pContext = DomainMapper_Impl::GetTopContextOfType(CONTEXT_SECTION);
    //ask for the header name of the given type
    SectionPropertyMap* pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
    if(pSectionContext)
    {
        ::rtl::OUString sPageStyleName = pSectionContext->GetPageStyleName( eType );
        //if there's none create one
        try
        {
            uno::Reference< beans::XPropertySet > xPageStyle;
            if(!sPageStyleName.getLength())
            {
                uno::Sequence< ::rtl::OUString > aPageStyleNames = GetPageStyles()->getElementNames();
                sPageStyleName = lcl_FindUnusedPageStyleName(aPageStyleNames);
                xPageStyle = uno::Reference< beans::XPropertySet > (
                        m_xTextFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.PageStyle") )),
                        uno::UNO_QUERY);
                GetPageStyles()->insertByName( sPageStyleName, uno::makeAny(xPageStyle) );
                pSectionContext->SetPageStyleName( eType, sPageStyleName );
            }
            else
                GetPageStyles()->getByName( sPageStyleName ) >>= xPageStyle;
            //switch on header use
            xPageStyle->setPropertyValue(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HeaderIsOn")),
                    uno::makeAny(sal_True) );
            //set the interface
            uno::Reference< text::XText > xHeaderText;
            xPageStyle->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HeaderText") )) >>= xHeaderText;
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
        ::rtl::OUString sPageStyleName = pSectionContext->GetPageStyleName( eType );
        //if there's none create one
        try
        {
            uno::Reference< beans::XPropertySet > xPageStyle;
            if(!sPageStyleName.getLength())
            {
                uno::Sequence< ::rtl::OUString > aPageStyleNames = GetPageStyles()->getElementNames();
                sPageStyleName = lcl_FindUnusedPageStyleName(aPageStyleNames);
                xPageStyle = uno::Reference< beans::XPropertySet >(
                        m_xTextFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.style.PageStyle") )),
                        uno::UNO_QUERY);
                GetPageStyles()->insertByName( sPageStyleName, uno::makeAny(xPageStyle) );
                pSectionContext->SetPageStyleName( eType, sPageStyleName );
            }
            else
                GetPageStyles()->getByName( sPageStyleName ) >>= xPageStyle;
            //switch on footer use
            xPageStyle->setPropertyValue(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FooterIsOn")),
                    uno::makeAny(sal_True) );
            //set the interface
            uno::Reference< text::XText > xFooterText;
            xPageStyle->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FooterText") )) >>= xFooterText;
            m_aTextAppendStack.push(uno::Reference< text::XTextAppendAndConvert >( xFooterText, uno::UNO_QUERY_THROW));
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
    m_aTextAppendStack.pop();
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 lcl_convertToMM100(sal_Int32 _t)
{
    return TWIP_TO_MM100( _t );
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
sal_Int16 lcl_ParseNumberingType(::rtl::OUString& rCommand)
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
/*-- 01.09.2006 11:48:08---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::CreateField( ::rtl::OUString& rCommand )
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
                    FIELD_DOCPROPERTY != aIt->second.eFieldId)
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

/*                    xFieldProperties->setPropertyValue(
                             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FieldCode")),
                             uno::makeAny( rCommand.copy( nIndex + 1 )));
*/
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
            }
        }
        m_xTextField = uno::Reference< text::XTextField >( xFieldInterface, uno::UNO_QUERY );
    }
    catch( uno::Exception& )
    {
    }
}
/*-- 01.09.2006 11:48:09---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::SetFieldResult( ::rtl::OUString& rResult )
{
    try
    {
        PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        OSL_ENSURE( m_xTextField.is()||m_sHyperlinkURL.getLength(), "DomainMapper_Impl::SetFieldResult: field not created" );
        if(m_xTextField.is())
        {
            try
            {
                if( m_bSetUserFieldContent )
                {
                    // user field content has to be set at the field master
                    uno::Reference< text::XDependentTextField > xDependentField( m_xTextField, uno::UNO_QUERY_THROW );
                    xDependentField->getTextFieldMaster()->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_CONTENT),
                         uno::makeAny( rResult ));
                }
                else
                {
                    uno::Reference< beans::XPropertySet > xFieldProperties( m_xTextField, uno::UNO_QUERY_THROW);
                    xFieldProperties->setPropertyValue(
                            rPropNameSupplier.GetName(PROP_CURRENT_PRESENTATION),
                         uno::makeAny( rResult ));
                }
            }
            catch( const beans::UnknownPropertyException& )
            {
                //some fields don't have a CurrentPresentation (DateTime)
            }
            uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = m_aTextAppendStack.top();
            xTextAppendAndConvert->appendTextContent( uno::Reference<text::XTextContent>(m_xTextField, uno::UNO_QUERY) );
        }
        else if(m_sHyperlinkURL.getLength())
        {
            //insert the string and apply hyperlink attribute to it
            uno::Reference< text::XTextAppendAndConvert >  xTextAppendAndConvert = m_aTextAppendStack.top();
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
    }
    catch( uno::Exception& )
    {

    }
    m_xTextField = 0;
    m_sHyperlinkURL = OUString();
}

/*-- 11.09.2006 13:16:35---------------------------------------------------

  -----------------------------------------------------------------------*/
bool DomainMapper_Impl::IsFieldAvailable() const
{
    return m_xTextField.is() || m_sHyperlinkURL.getLength();
}
/*-- 14.09.2006 12:46:52---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper_Impl::GetCurrentLocale(lang::Locale& rLocale)
{
    ::rtl::OUString sCharLocale = PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_CHAR_LOCALE);
    PropertyMapPtr pTopContext = GetTopContext();
    PropertyMap::iterator aLocaleIter = pTopContext->find(sCharLocale);
    if( aLocaleIter != pTopContext->end())
        aLocaleIter->second >>= rLocale;
    else
    {
        PropertyMapPtr pParaContext = GetTopContextOfType(CONTEXT_PARAGRAPH);
        aLocaleIter = pParaContext->find(sCharLocale);
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

/*-- 18.09.2006 15:47:09---------------------------------------------------
    the string _can_ be quoted or not be available at all
  -----------------------------------------------------------------------*/
//void DomainMapper_Impl::ExtractAndSetDocumentInfo(
//        const ::rtl::OUString& rCommand, sal_Int32 nCommandLength, PropertyIds ePropId )
//{
//    ::rtl::OUString sParam = lcl_ExtractParameter( rCommand, nCommandLength );
//    if( sParam.getLength() )
//    {
//        uno::Reference< document::XDocumentInfoSupplier > xInfoSupplier( m_xTextDocument, uno::UNO_QUERY_THROW );
//        uno::Reference< beans::XPropertySet > xInfoPropSet( xInfoSupplier->getDocumentInfo(), uno::UNO_QUERY_THROW );
//        xInfoPropSet->setPropertyValue(
//                PropertyNameSupplier::GetPropertyNameSupplier().GetName(ePropId), uno::makeAny( sParam ));
//    }

//}


}

