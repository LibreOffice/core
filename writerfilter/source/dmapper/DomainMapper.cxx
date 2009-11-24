/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DomainMapper.cxx,v $
 *
 * $Revision: 1.69 $
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
#include <dmapper/DomainMapper.hxx>
#include <DomainMapper_Impl.hxx>
#include <ConversionHelper.hxx>
#include <ThemeTable.hxx>
#include <ModelEventListener.hxx>
#include <MeasureHandler.hxx>
#include <OLEHandler.hxx>
#include <i18npool/mslangid.hxx>
#include <i18npool/paper.hxx>
#include <ooxml/OOXMLFastTokens.hxx>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XOOXMLDocumentPropertiesImporter.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/XEndnotesSupplier.hpp>
#include <com/sun/star/text/XFootnotesSupplier.hpp>
#include <com/sun/star/text/XLineNumberingProperties.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/text/XTextPortionAppend.hpp>
#include <com/sun/star/text/XParagraphAppend.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/awt/FontRelief.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/document/XEventBroadcaster.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/style/CaseMap.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/LineSpacingMode.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/TextGridMode.hpp>
#include <com/sun/star/text/XDocumentIndexesSupplier.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <comphelper/types.hxx>
#include <comphelper/storagehelper.hxx>

#include <rtl/ustrbuf.hxx>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <tools/color.hxx>
#include <BorderHandler.hxx>
#include <CellColorHandler.hxx>
#include <SectionColumnHandler.hxx>
#include <vector>
#include <iostream>

#ifdef DEBUG_DOMAINMAPPER
#include <resourcemodel/QNameToString.hxx>
#include <resourcemodel/util.hxx>
#include <resourcemodel/TagLogger.hxx>
#endif
#if OSL_DEBUG_LEVEL > 0
#include <resourcemodel/QNameToString.hxx>
#endif


using namespace ::com::sun::star;
using namespace ::rtl;

namespace writerfilter {
namespace dmapper{

#ifdef DEBUG_DOMAINMAPPER
TagLogger::Pointer_t dmapper_logger(TagLogger::getInstance("DOMAINMAPPER"));
#endif

/* ---- Fridrich's mess begins here ---- */
struct _PageSz
{
    sal_Int32 code;
    sal_Int32 h;
    bool      orient;
    sal_Int32 w;
} CT_PageSz;


/* ---- Fridrich's mess (hopefully) ends here ---- */

/*-- 09.06.2006 09:52:11---------------------------------------------------

  -----------------------------------------------------------------------*/
DomainMapper::DomainMapper( const uno::Reference< uno::XComponentContext >& xContext,
                            uno::Reference< io::XInputStream > xInputStream,
                            uno::Reference< lang::XComponent > xModel,
                            SourceDocumentType eDocumentType) :
    m_pImpl( new DomainMapper_Impl( *this, xContext, xModel, eDocumentType )),
    mnBackgroundColor(0), mbIsHighlightSet(false)
{
    // #i24363# tab stops relative to indent
    m_pImpl->SetDocumentSettingsProperty(
        PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_TABS_RELATIVE_TO_INDENT ),
        uno::makeAny( false ) );

    m_pImpl->SetDocumentSettingsProperty(
        PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_ADD_PARA_TABLE_SPACING ),
        uno::makeAny( false ) );

    //import document properties

    try
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory(xContext->getServiceManager(), uno::UNO_QUERY_THROW);
        uno::Reference< embed::XStorage > xDocumentStorage =
            (comphelper::OStorageHelper::GetStorageOfFormatFromInputStream(OFOPXML_STORAGE_FORMAT_STRING, xInputStream));

        uno::Reference< uno::XInterface > xTemp = xContext->getServiceManager()->createInstanceWithContext(
                                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.document.OOXMLDocumentPropertiesImporter")),
                                xContext);

        uno::Reference< document::XOOXMLDocumentPropertiesImporter > xImporter( xTemp, uno::UNO_QUERY_THROW );
        uno::Reference< document::XDocumentPropertiesSupplier > xPropSupplier( xModel, uno::UNO_QUERY_THROW);
        xImporter->importProperties( xDocumentStorage, xPropSupplier->getDocumentProperties() );
    }
    catch( const uno::Exception& rEx )
    {
        (void)rEx;
    }

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("domainmapper");
#endif
}
/*-- 09.06.2006 09:52:12---------------------------------------------------

-----------------------------------------------------------------------*/
DomainMapper::~DomainMapper()
{
    try
    {
        uno::Reference< text::XDocumentIndexesSupplier> xIndexesSupplier( m_pImpl->GetTextDocument(), uno::UNO_QUERY );
        uno::Reference< container::XIndexAccess > xIndexes = xIndexesSupplier->getDocumentIndexes();
        sal_Int32 nIndexes = xIndexes->getCount();
        if( nIndexes )
        {
            //index update has to wait until first view is created
            uno::Reference< document::XEventBroadcaster > xBroadcaster(xIndexesSupplier, uno::UNO_QUERY);
            xBroadcaster->addEventListener(uno::Reference< document::XEventListener >(new ModelEventListener));
        }
    }
    catch( const uno::Exception& rEx )
    {
        (void)rEx;
    }

    delete m_pImpl;

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("domainmapper");
#endif
}
/*-- 09.06.2006 09:52:12---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::attribute(Id nName, Value & val)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("attribute");
    dmapper_logger->attribute("name", (*QNameToString::Instance())(nName));
    dmapper_logger->attribute("value", val.toString());
#endif
    static ::rtl::OUString sLocalBookmarkName;
    sal_Int32 nIntValue = val.getInt();
    rtl::OUString sStringValue = val.getString();
    // printf ( "DomainMapper::attribute(0x%.4x, 0x%.4x) [%s]\n", (unsigned int)nName, (unsigned int)nIntValue, ::rtl::OUStringToOString(sStringValue, RTL_TEXTENCODING_DONTKNOW).getStr());
    if( nName >= NS_rtf::LN_WIDENT && nName <= NS_rtf::LN_LCBSTTBFUSSR )
        m_pImpl->GetFIB().SetData( nName, nIntValue );
    else //if( !m_pImpl->getTableManager().attribute( nName, val) )
    {


        /* WRITERFILTERSTATUS: table: attributedata */
        switch( nName )
        {
            /* attributes to be ignored */
        case NS_rtf::LN_UNUSED1_3:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_UNUSED1_7:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_UNUSED8_3:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FWRITERESERVATION:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FLOADOVERRIDE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FFAREAST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCRYPTO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_NFIBBACK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LKEY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_ENVR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FMAC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FWORD97SAVED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCMAC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNFBPCHPFIRST_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNCHPFIRST_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_CPNBTECHP_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNFBPPAPFIRST_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNPAPFIRST_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_CPNBTEPAP_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNFBPLVCFIRST_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNLVCFIRST_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_CPNBTELVC_W6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_CBMAC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LPRODUCTCREATED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LPRODUCTREVISED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_CCPMCR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNFBPCHPFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNFBPPAPFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_PNFBPLVCFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCISLANDFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCISLANDLIM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTSHFORIG:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBSTSHFORIG:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFPAD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFPAD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTTBFGLSY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBSTTBFGLSY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFGLSY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFGLSY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFSEA:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFSEA:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFFLDMCR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDMCR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCCMDS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBCMDS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCMCR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCMCR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTTBFMCR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBSTTBFMCR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPRDRVR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPRDRVR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPRENVPORT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPRENVPORT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPRENVLAND:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPRENVLAND:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCWSS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBWSS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFPGDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFPGDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCAUTOSAVESOURCE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBAUTOSAVESOURCE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCDOAMOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCDOAMOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCDOAHDR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCDOAHDR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPMS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPMS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFPGDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFPGDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFWKB:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFWKB:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFSPL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFSPL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTWUSER:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBSTWUSER:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCUNUSED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBUNUSED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTTBFINTLFLD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBSTTBFINTLFLD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCROUTESLIP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBROUTESLIP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTTBSAVEDBY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBSTTBSAVEDBY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTTBFNM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBSTTBFNM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCDOCUNDO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBDOCUNDO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCRGBUSE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBRGBUSE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCUSP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBUSP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCUSKF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBUSKF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCUPCRGBUSE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCUPCRGBUSE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCUPCUSP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCUPCUSP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLGOSL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLGOSL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCOCX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCOCX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_DWLOWDATETIME:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_DWHIGHDATETIME:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCASUMY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCASUMY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCPLCFGRAM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_LCBPLCFGRAM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */

        case NS_rtf::LN_FCSTTBFUSSR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            break;

        case NS_rtf::LN_ISTD: //index of applied style
            /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
            {
            //search for the style with the given id and apply it
            //as CharStyleName or ParaStyleName
            //if the style is a user defined style then it must have an ISTD - built-in styles might not have it
            StyleSheetTablePtr pStyleSheets = m_pImpl->GetStyleSheetTable();
            ::rtl::OUString sValue = ::rtl::OUString::valueOf(nIntValue, 16);
            const StyleSheetEntry* pEntry = pStyleSheets->FindStyleSheetByISTD(sValue);
            if(pEntry)
            {
                bool bParaStyle = (pEntry->nStyleTypeCode == STYLE_TYPE_PARA);
                if(bParaStyle)
                    m_pImpl->SetCurrentParaStyleId(::rtl::OUString::valueOf(static_cast<sal_Int32>(nIntValue), 16));
                if (m_pImpl->GetTopContext() && m_pImpl->GetTopContextType() != CONTEXT_SECTION)
                    m_pImpl->GetTopContext()->Insert(
                                                 bParaStyle ?
                                                 PROP_PARA_STYLE_NAME  : PROP_CHAR_STYLE_NAME,
                                                 true,
                                                 uno::makeAny(
                                                 m_pImpl->GetStyleSheetTable()->ConvertStyleName( pEntry->sStyleName ) ) );
            }
        }
        break;
        case NS_rtf::LN_ISTARTAT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_NFC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FLEGAL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FNORESTART:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FIDENTSAV:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FCONVERTED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FTENTATIVE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_RGBXCHNUMS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_IXCHFOLLOW:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DXASPACE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DXAINDENT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CBGRPPRLCHPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CBGRPPRLPAPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LSID:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_TPLC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_RGISTD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FSIMPLELIST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FRESTARTHDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_UNSIGNED26_2:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ILVL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FSTARTAT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FFORMATTING:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_UNSIGNED4_6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_clfolvl:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CBFFNM1:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_PRQ:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FTRUETYPE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_WWEIGHT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CHS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

            {
                m_pImpl->GetFIB().SetLNCHS( nIntValue );
            }
            break;
        case NS_rtf::LN_IXCHSZALT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_PANOSE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_STI:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FSCRATCH:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FINVALHEIGHT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FHASUPE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FMASSCOPY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_SGC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ISTDBASE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CUPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ISTDNEXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BCHUPE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FAUTOREDEF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FHIDDEN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CSTD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CBSTDBASEINFILE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FSTDSTYLENAMESWRITTEN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_UNUSED4_2:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_STIMAXWHENSAVED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ISTDMAXFIXEDWHENSAVED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_NVERBUILTINNAMESWHENSAVED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_RGFTCSTANDARDCHPSTSH:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_WIDENT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_NFIB:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_NPRODUCT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_LID:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_PNNEXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_FDOT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_FGLSY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_FCOMPLEX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_FHASPIC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CQUICKSAVES:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FENCRYPTED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FWHICHTBLSTM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FREADONLYRECOMMENDED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FEXTCHAR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FEMPTYSPECIAL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FLOADOVERRIDEPAGE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FFUTURESAVEDUNDO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FSPARE0:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CHSTABLES:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCMIN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CSW:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_WMAGICCREATED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_WMAGICREVISED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_WMAGICCREATEDPRIVATE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_WMAGICREVISEDPRIVATE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LIDFE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CLW:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CCPTEXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_CCPFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CCPHDD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CCPATN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CCPEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CCPTXBX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CCPHDRTXBX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_PNCHPFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CPNBTECHP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_PNPAPFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CPNBTEPAP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_PNLVCFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CPNBTELVC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_CFCLCB:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTSHF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTSHF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFNDREF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFNDREF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFNDTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFNDTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFANDREF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFANDREF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFANDTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFANDTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFSED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFSED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFPHE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFPHE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFHDD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFHDD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFBTECHPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFBTECHPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_FCPLCFBTEPAPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFBTEPAPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBFFFN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBFFFN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFLDMOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDMOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFLDHDR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDHDR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_FCPLCFFLDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFLDATN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDATN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBFBKMK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBFBKMK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFBKF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFBKF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFBKL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFBKL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCDOP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBDOP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBFASSOC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBFASSOC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCCLX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBCLX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCGRPXSTATNOWNERS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBGRPXSTATNOWNERS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBFATNBKMK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBFATNBKMK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCSPAMOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCSPAMOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCSPAHDR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_LCBPLCSPAHDR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFATNBKF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFATNBKF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFATNBKL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFATNBKL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCFORMFLDSTTBF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBFORMFLDSTTBF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFENDREF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_LCBPLCFENDREF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFENDTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFENDTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFLDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCDGGINFO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBDGGINFO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBFRMARK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_LCBSTTBFRMARK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBFCAPTION:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBFCAPTION:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBFAUTOCAPTION:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBFAUTOCAPTION:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFTXBXTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFLDTXBX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDTXBX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFHDRTXBXTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFHDRTXBXTXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFFLDHDRTXBX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFFLDHDRTXBX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBTTMBD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBTTMBD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPGDMOTHER:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPGDMOTHER:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCBKDMOTHER:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBBKDMOTHER:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPGDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPGDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCBKDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBBKDFTN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPGDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPGDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_FCBKDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBBKDEDN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFLST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_LCBPLCFLST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLFLFO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLFLFO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFTXBXBKD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFTXBXBKD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFTXBXHDRBKD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFTXBXHDRBKD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBGLSYSTYLE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */


        case NS_rtf::LN_LCBSTTBGLSYSTYLE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFBTELVC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFBTELVC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCPLCFLVC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBPLCFLVC:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSTTBLISTNAMES:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBLISTNAMES:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_LCBSTTBFUSSR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            {
                m_pImpl->GetFIB().SetData( nName, nIntValue );
            }
            break;
        case NS_rtf::LN_FN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCSEPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FNMPR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        case NS_rtf::LN_FCMPR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

            //section descriptor, unused or internally used
            break;
        case NS_rtf::LN_ICOFORE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ICOBACK:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_IPAT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_SHDFORECOLOR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_SHDBACKCOLOR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_SHDPATTERN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DPTLINEWIDTH:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BRCTYPE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ICO:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DPTSPACE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FSHADOW:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FFRAME:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_UNUSED2_15:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FFIRSTMERGED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FMERGED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FVERTICAL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FBACKWARD:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FROTATEFONT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FVERTMERGE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FVERTRESTART:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_VERTALIGN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FUNUSED:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BRCTOP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_BRCLEFT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_BRCBOTTOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_rtf::LN_BRCRIGHT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        {
            table::BorderLine aBorderLine;
            sal_Int32 nLineDistance = ConversionHelper::MakeBorderLine( nIntValue, aBorderLine );
            (void)nLineDistance;
            PropertyIds eBorderId = PROP_LEFT_BORDER;
            PropertyIds eBorderDistId = PROP_LEFT_BORDER_DISTANCE  ;
            switch( nName )
            {
                case NS_rtf::LN_BRCTOP:
                    eBorderId = PROP_TOP_BORDER            ;
                    eBorderDistId = PROP_TOP_BORDER_DISTANCE;
                break;
                case NS_rtf::LN_BRCLEFT:
//                    eBorderId = PROP_LEFT_BORDER;
//                    eBorderDistId = PROP_LEFT_BORDER_DISTANCE  ;
                break;
                case NS_rtf::LN_BRCBOTTOM:
                    eBorderId = PROP_BOTTOM_BORDER         ;
                    eBorderDistId = PROP_BOTTOM_BORDER_DISTANCE;
                break;
                case NS_rtf::LN_BRCRIGHT:
                    eBorderId = PROP_RIGHT_BORDER          ;
                    eBorderDistId = PROP_RIGHT_BORDER_DISTANCE ;
                break;
                default:;
            }
            //todo: where to put the border properties
            //rContext->Insert(eBorderId, uno::makeAny( aBorderLine ));
            //rContext->Insert(eBorderDistId, uno::makeAny( nLineDistance ));
        }
        break;
        case NS_rtf::LN_ITCFIRST:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FPUB:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ITCLIM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FCOL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINECOLOR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINEWIDTH:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINETYPE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_MM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_XEXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_YEXT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_HMF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LCB:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CBHEADER:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_MFP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BM_RCWINMF:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DXAGOAL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DYAGOAL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_MX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_MY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DXACROPLEFT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DYACROPTOP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DXACROPRIGHT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DYACROPBOTTOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BRCL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FFRAMEEMPTY:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FBITMAP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FDRAWHATCH:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FERROR:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BPP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DXAORIGIN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_DYAORIGIN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_CPROPS:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINEPROPSTOP:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINEPROPSLEFT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINEPROPSBOTTOM:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINEPROPSRIGHT:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINEPROPSHORIZONTAL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LINEPROPSVERTICAL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_headerr:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_footerr:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_endnote:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_BOOKMARKNAME:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            // sStringValue contains the bookmark name
            sLocalBookmarkName = sStringValue;
        break;
        case NS_rtf::LN_IBKL:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0.5 */
            //contains the bookmark identifier - has to be added to the bookmark name imported before
            //if it is already available then the bookmark should be inserted
            m_pImpl->AddBookmark( sLocalBookmarkName, sStringValue );
            sLocalBookmarkName = ::rtl::OUString();
        break;
        case NS_rtf::LN_LISTLEVEL:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LFOData:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_F:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_ALTFONTNAME:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_XSZFFN:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_XSTZNAME:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_XSTZNAME1:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_UPXSTART:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_UPX:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_sed:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //section properties
            resolveAttributeProperties(val);
            break;
        case NS_rtf::LN_tbdAdd:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //
            {
                writerfilter::Reference<Properties>::Pointer_t pProperties = val.getProperties();
                if( pProperties.get())
                {
                    pProperties->resolve(*this);
                    //increment to the next tab stop
                    m_pImpl->NextTabStop();
                }
            }
            break;
        case NS_rtf::LN_dxaDel:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //deleted tab
        case NS_rtf::LN_dxaAdd:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //set tab
        case NS_rtf::LN_TLC:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //tab leading characters - for decimal tabs
        case NS_rtf::LN_JC:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //tab justification
            m_pImpl->ModifyCurrentTabStop(nName, nIntValue);
            break;
        case NS_rtf::LN_UNUSED0_6:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            // really unused
            break;
        case NS_rtf::LN_rgbrc:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_shd:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_cellShd:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_cellTopColor:
        case NS_rtf::LN_cellLeftColor:
        case NS_rtf::LN_cellBottomColor:
        case NS_rtf::LN_cellRightColor:
            OSL_ASSERT("handled by DomainMapperTableManager");
        break;

        case NS_rtf::LN_LISTTABLE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_LFOTABLE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_FONTTABLE:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_rtf::LN_STYLESHEET:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;

        case NS_rtf::LN_fcEastAsianLayout:
            /* WRITERFILTERSTATUS: done: 50, planned: 0.5, spent: 0 */
        /*  it seems that the value is following:
                ???? XX YYYY ZZ
            where
                XX seems to be the run id
                ZZ is the length of the function that is normally 6
                Lower byte of YYYY determines whether it is
                    vertical text flow (0x01), or
                    two lines in one layout (0x02).
                For 0x01, if the higher byte of YYYY is zero, the text is not scaled to fit the line height,
                    in oposite case, it is to be scaled.
                For 0x02, the higher byte of YYYY is determinig the prefix and suffix of the run:
                    no brackets (0x00) ,
                    () round brackets (0x01),
                    [] square backets (0x02),
                    <> angle brackets (0x03) and
                    {} curly brackets (0x04).
                ???? is different and we do not know its signification
          */

            if ((nIntValue & 0x000000FF) == 6)
            {
                switch ((nIntValue & 0x0000FF00) >> 8)
                {
                case 1: // vertical text
                    if (m_pImpl->GetTopContext())
                    {
                        m_pImpl->GetTopContext()->Insert(PROP_CHAR_ROTATION, true, uno::makeAny ( sal_Int16(900) ));
                        m_pImpl->GetTopContext()->Insert(PROP_CHAR_ROTATION_IS_FIT_TO_LINE, true, uno::makeAny (((nIntValue & 0x00FF0000) >> 16) != 0));
                    }
                    break;
                case 2: // two lines in one
                    if (m_pImpl->GetTopContext())
                    {
                        m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_IS_ON, true, uno::makeAny ( true ));
                        m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_PREFIX, true, uno::makeAny ( getBracketStringFromEnum((nIntValue & 0x00FF0000) >> 16)));
                        m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_SUFFIX, true, uno::makeAny ( getBracketStringFromEnum((nIntValue & 0x00FF0000) >> 16, false)));
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case NS_rtf::LN_FRD :
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            //footnote reference descriptor, if nIntValue > 0 then automatic, custom otherwise
            //ignored
        break;
        case NS_rtf::LN_FONT: //font of footnote symbol
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->SetFootnoteFontId( nIntValue );
        break;
        case NS_ooxml::LN_CT_Sym_char:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        if( m_pImpl->GetTopContext() && m_pImpl->GetTopContext()->GetFootnote().is())
        {
            m_pImpl->GetTopContext()->GetFootnote()->setLabel(::rtl::OUString( sal_Unicode(nIntValue)));
            break;
        }
        else //it's a _real_ symbol
        {
            utext( reinterpret_cast < const sal_uInt8 * >( &nIntValue ), 1 );
        }
        break;
        case NS_rtf::LN_CHAR: //footnote symbol character
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->SetFootnoteSymbol( sal_Unicode(nIntValue));
        break;
        case NS_ooxml::LN_CT_Sym_font:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
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
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME, true, uno::makeAny( sStringValue ));
        break;
        case NS_ooxml::LN_CT_Underline_val:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            handleUnderlineType(nIntValue, m_pImpl->GetTopContext());
            break;
        case NS_ooxml::LN_CT_Color_val:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COLOR, true, uno::makeAny( nIntValue ) );
            break;
        case NS_ooxml::LN_CT_Underline_color:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
            {
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_UNDERLINE_HAS_COLOR, true, uno::makeAny( true ) );
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_UNDERLINE_COLOR, true, uno::makeAny( nIntValue ) );
            }
            break;

        case NS_ooxml::LN_CT_TabStop_val:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_ST_TabJc_clear)
                m_pImpl->m_aCurrentTabStop.bDeleted = true;
            else
            {
                m_pImpl->m_aCurrentTabStop.bDeleted = false;
                m_pImpl->m_aCurrentTabStop.Alignment = getTabAlignFromValue(nIntValue);
            }
            break;
        case NS_ooxml::LN_CT_TabStop_leader:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->m_aCurrentTabStop.FillChar = getFillCharFromValue(nIntValue);
            break;
        case NS_ooxml::LN_CT_TabStop_pos:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->m_aCurrentTabStop.Position = ConversionHelper::convertTwipToMM100(nIntValue);
            break;

        case NS_ooxml::LN_CT_Fonts_ascii:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME, true, uno::makeAny( sStringValue ));
            break;
        case NS_ooxml::LN_CT_Fonts_asciiTheme:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME, true, uno::makeAny( m_pImpl->GetThemeTable()->getFontNameForTheme(nIntValue) ));
            break;
        case NS_ooxml::LN_CT_Fonts_hAnsi:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            break;//unsupported
        case NS_ooxml::LN_CT_Fonts_hAnsiTheme:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            break; //unsupported
        case NS_ooxml::LN_CT_Fonts_eastAsia:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME_ASIAN, true, uno::makeAny( sStringValue ));
            break;
    case NS_ooxml::LN_CT_Fonts_eastAsiaTheme:
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        if (m_pImpl->GetTopContext())
            m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME_COMPLEX, true, uno::makeAny( m_pImpl->GetThemeTable()->getFontNameForTheme(nIntValue) ) );
        break;
        case NS_ooxml::LN_CT_Fonts_cs:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME_COMPLEX, true, uno::makeAny( sStringValue ));
            break;
        case NS_ooxml::LN_CT_Fonts_cstheme:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME_COMPLEX, true, uno::makeAny( m_pImpl->GetThemeTable()->getFontNameForTheme(nIntValue) ));
        break;
        case NS_ooxml::LN_CT_Spacing_before:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_PARA_TOP_MARGIN, true, uno::makeAny( ConversionHelper::convertTwipToMM100( nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Spacing_beforeLines:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_ooxml::LN_CT_Spacing_after:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_PARA_BOTTOM_MARGIN, true, uno::makeAny( ConversionHelper::convertTwipToMM100( nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Spacing_afterLines:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_ooxml::LN_CT_Spacing_line: //91434
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        case NS_ooxml::LN_CT_Spacing_lineRule: //91435
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        {
#define SINGLE_LINE_SPACING 240
            style::LineSpacing aSpacing;
            PropertyMapPtr pTopContext = m_pImpl->GetTopContext();
            PropertyMap::iterator aLineSpacingIter = pTopContext->find(PropertyDefinition( PROP_PARA_LINE_SPACING, true ) );
            if( aLineSpacingIter != pTopContext->end())
            {
                aLineSpacingIter->second >>= aSpacing;
            }
            else
            {
                //default to single line spacing
                aSpacing.Mode = style::LineSpacingMode::FIX;
                aSpacing.Height = sal_Int16(ConversionHelper::convertTwipToMM100( SINGLE_LINE_SPACING ));
            }
            if( nName == NS_ooxml::LN_CT_Spacing_line )
            {
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
                        aSpacing.Mode = style::LineSpacingMode::PROP;
                        //reinterpret the already set value
                        aSpacing.Height = sal_Int16( aSpacing.Height * 100 /  ConversionHelper::convertTwipToMM100( SINGLE_LINE_SPACING ));
                    }
                    else if( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_wordprocessingml_ST_LineSpacingRule_atLeast)
                        aSpacing.Mode = style::LineSpacingMode::MINIMUM;
                    else // NS_ooxml::LN_Value_wordprocessingml_ST_LineSpacingRule_exact
                        aSpacing.Mode = style::LineSpacingMode::FIX;
            }
            pTopContext->Insert(PROP_PARA_LINE_SPACING, true, uno::makeAny( aSpacing ));
        }
        break;
        case NS_ooxml::LN_CT_Ind_left:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(
                    PROP_PARA_LEFT_MARGIN, true, uno::makeAny( ConversionHelper::convertTwipToMM100(nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Ind_right:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(
                    PROP_PARA_RIGHT_MARGIN, true, uno::makeAny( ConversionHelper::convertTwipToMM100(nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Ind_hanging:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(
                    PROP_PARA_FIRST_LINE_INDENT, true, uno::makeAny( - ConversionHelper::convertTwipToMM100(nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Ind_firstLine:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(
                    PROP_PARA_FIRST_LINE_INDENT, true, uno::makeAny( ConversionHelper::convertTwipToMM100(nIntValue ) ));
            break;

        case NS_ooxml::LN_CT_EastAsianLayout_id:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_combine:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_IS_ON, true, uno::makeAny ( nIntValue ? true : false ));
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_combineBrackets:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
            {
                rtl::OUString sCombinePrefix = getBracketStringFromEnum(nIntValue);
                rtl::OUString sCombineSuffix = getBracketStringFromEnum(nIntValue, false);
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_PREFIX, true, uno::makeAny ( sCombinePrefix ));
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_SUFFIX, true, uno::makeAny ( sCombineSuffix ));
            }
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_vert:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
            {
                sal_Int16 nRotationAngle = (nIntValue ? 900 : 0);
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_ROTATION, true, uno::makeAny ( nRotationAngle ));
            }
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_vertCompress:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_ROTATION_IS_FIT_TO_LINE, true, uno::makeAny ( nIntValue ? true : false));
            break;

        case NS_ooxml::LN_CT_PageSz_code:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            CT_PageSz.code = nIntValue;
            break;
        case NS_ooxml::LN_CT_PageSz_h:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            {
                sal_Int32 nHeight = ConversionHelper::convertTwipToMM100(nIntValue);
                CT_PageSz.h = PaperInfo::sloppyFitPageDimension(nHeight);
            }
            break;
        case NS_ooxml::LN_CT_PageSz_orient:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            CT_PageSz.orient = (nIntValue != 0);
            break;
        case NS_ooxml::LN_CT_PageSz_w:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            {
                sal_Int32 nWidth = ConversionHelper::convertTwipToMM100(nIntValue);
                CT_PageSz.w = PaperInfo::sloppyFitPageDimension(nWidth);
            }
            break;

        case NS_ooxml::LN_CT_PageMar_top:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->SetPageMarginTwip( PAGE_MAR_TOP, nIntValue );
        break;
        case NS_ooxml::LN_CT_PageMar_right:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->SetPageMarginTwip( PAGE_MAR_RIGHT, nIntValue );
        break;
        case NS_ooxml::LN_CT_PageMar_bottom:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->SetPageMarginTwip( PAGE_MAR_BOTTOM, nIntValue );
        break;
        case NS_ooxml::LN_CT_PageMar_left:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->SetPageMarginTwip( PAGE_MAR_LEFT, nIntValue );
        break;
        case NS_ooxml::LN_CT_PageMar_header:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->SetPageMarginTwip( PAGE_MAR_HEADER, nIntValue );
        break;
        case NS_ooxml::LN_CT_PageMar_footer:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->SetPageMarginTwip( PAGE_MAR_FOOTER, nIntValue );
        break;
        case NS_ooxml::LN_CT_PageMar_gutter:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->SetPageMarginTwip( PAGE_MAR_GUTTER, nIntValue );
        break;
        case NS_ooxml::LN_CT_Language_val: //90314
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        case NS_ooxml::LN_CT_Language_eastAsia: //90315
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        case NS_ooxml::LN_CT_Language_bidi: //90316
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        {
            LanguageType eLang = MsLangId::convertIsoStringToLanguage( sStringValue );
            lang::Locale aLocale = MsLangId::convertLanguageToLocale( eLang );
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(NS_ooxml::LN_CT_Language_val== nName ? PROP_CHAR_LOCALE :
                             NS_ooxml::LN_CT_Language_eastAsia == nName ? PROP_CHAR_LOCALE_ASIAN : PROP_CHAR_LOCALE_COMPLEX,
                             true,
                             uno::makeAny( aLocale ) );
        }
        break;
#define AUTO_PARA_SPACING sal_Int32(49)
        case NS_ooxml::LN_CT_Spacing_beforeAutospacing:
            /* WRITERFILTERSTATUS: done: 80, planned: 0.5, spent: 0.2 */
            //TODO: autospacing depends on some document property (called fDontUseHTMLAutoSpacing in old ww8 filter) 100 or 280 twip
            //and should be set to 0 on start of page
            m_pImpl->GetTopContext()->Insert( PROP_TOP_MARGIN, false, uno::makeAny( AUTO_PARA_SPACING ) );
        break;
        case NS_ooxml::LN_CT_Spacing_afterAutospacing:
            /* WRITERFILTERSTATUS: done: 80, planned: 0.5, spent: 0.2 */
            //TODO: autospacing depends on some document property (called fDontUseHTMLAutoSpacing in old ww8 filter) 100 or 280 twip
            m_pImpl->GetTopContext()->Insert( PROP_BOTTOM_MARGIN, false, uno::makeAny( AUTO_PARA_SPACING ) );
        break;
        case NS_ooxml::LN_CT_SmartTagRun_uri:
        case NS_ooxml::LN_CT_SmartTagRun_element:
            /* WRITERFILTERSTATUS: done: 0, planned: 1, spent: 0 */
            //TODO: add handling of SmartTags
        break;
        case NS_ooxml::LN_CT_Br_type :
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            //TODO: attributes for break (0x12) are not supported
        break;
        case NS_ooxml::LN_CT_Fonts_hint :
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            /*  assigns script type to ambigous characters, values can be:
                NS_ooxml::LN_Value_ST_Hint_default
                NS_ooxml::LN_Value_ST_Hint_eastAsia
                NS_ooxml::LN_Value_ST_Hint_cs
             */
            //TODO: unsupported?
        break;
        case NS_ooxml::LN_CT_TblCellMar_right: // 92375;
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_ooxml::LN_CT_TblBorders_top: // 92377;
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_ooxml::LN_CT_TblBorders_left: // 92378;
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_ooxml::LN_CT_TblBorders_bottom: // 92379;
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        //todo: handle cell mar
        break;
        case NS_rtf::LN_blip: // contains the binary graphic
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        case NS_ooxml::LN_shape:
            /* WRITERFILTERSTATUS: done: 50, planned: 0.5, spent: 0 */
        {
            //looks a bit like a hack - and it is. The graphic import is split into the inline_inline part and
            //afterwards the adding of the binary data.
            m_pImpl->GetGraphicImport( IMPORT_AS_DETECTED_INLINE )->attribute(nName, val);
            m_pImpl->ImportGraphic( val.getProperties(), IMPORT_AS_DETECTED_INLINE );
            if( m_pImpl->IsInShapeContext() )
            {
                //imported text from temporary shape needs to be copied to the real shape
                uno::Reference< drawing::XShape > xShape;
                val.getAny() >>= xShape;
                m_pImpl->CopyTemporaryShapeText( xShape );
            }
        }
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
        case NS_sprm::LN_PWr:
        case NS_sprm::LN_PDxaWidth:
        case NS_sprm::LN_PWHeightAbs:
        case NS_sprm::LN_PDxaFromText:
        case NS_sprm::LN_PDyaFromText:
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        {
            ParagraphProperties* pParaProperties = dynamic_cast< ParagraphProperties*>(m_pImpl->GetTopContext().get());
            if( pParaProperties )
            {
                switch( nName )
                {
                    case NS_ooxml::LN_CT_FramePr_dropCap:
                        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
                        pParaProperties->SetDropCap( nIntValue );
                    break;
                    case NS_ooxml::LN_CT_FramePr_lines:
                        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
                        pParaProperties->SetLines( nIntValue );
                    break;
                    case NS_ooxml::LN_CT_FramePr_hAnchor:
                        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
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
                        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
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
                        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
                        pParaProperties->Setx( ConversionHelper::convertTwipToMM100(nIntValue ));
                    break;
                    case NS_ooxml::LN_CT_FramePr_xAlign:
                        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
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
                        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
                        pParaProperties->Sety( ConversionHelper::convertTwipToMM100(nIntValue ));
                    break;
                    case NS_ooxml::LN_CT_FramePr_yAlign:
                        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
                        switch( nIntValue )
                        {
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_top     :
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_inside  :nIntValue = text::VertOrientation::TOP; break;
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_center  :nIntValue = text::VertOrientation::CENTER;break;
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_bottom  :
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_outside :nIntValue = text::VertOrientation::BOTTOM;break;
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_inline  ://todo: what to do with inline - no avail. in WW97 and WW2007
                            //no break;
                            default:nIntValue = text::VertOrientation::NONE;
                        }
                        pParaProperties->SetyAlign( nIntValue );
                    break;
                    case NS_ooxml::LN_CT_FramePr_hRule:
                        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
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
                    case NS_sprm::LN_PWr:
                        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
                    {
                        //should be either LN_Value_wordprocessingml_ST_Wrap_notBeside or LN_Value_wordprocessingml_ST_Wrap_around
                        OSL_ENSURE( sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_wordprocessingml_ST_Wrap_around ||
                                    sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_wordprocessingml_ST_Wrap_notBeside,
                            "wrap not around or not_Beside?");
                        pParaProperties->SetWrap(sal::static_int_cast<Id>(nIntValue) == NS_ooxml::LN_Value_wordprocessingml_ST_Wrap_around ?
                                                 text::WrapTextMode_DYNAMIC : text::WrapTextMode_NONE );
                    }
                    break;
                    case NS_sprm::LN_PDxaWidth:
                        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
                        pParaProperties->Setw(ConversionHelper::convertTwipToMM100(nIntValue));
                    break;
                    case NS_sprm::LN_PWHeightAbs:
                        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
                        pParaProperties->Seth(ConversionHelper::convertTwipToMM100(nIntValue));
                    break;
                    case NS_sprm::LN_PDxaFromText:
                        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
                        pParaProperties->SethSpace( ConversionHelper::convertTwipToMM100(nIntValue ));
                    break;
                    case NS_sprm::LN_PDyaFromText:
                        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
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
        case NS_ooxml::LN_CT_LineNumber_start:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_ooxml::LN_CT_LineNumber_distance:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_ooxml::LN_CT_TrackChange_author:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->SetCurrentRedlineAuthor( sStringValue );
        break;
        case NS_ooxml::LN_CT_TrackChange_date:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->SetCurrentRedlineDate( sStringValue );
        break;
        case NS_ooxml::LN_CT_Markup_id:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->SetCurrentRedlineId( sStringValue );
        break;
        case NS_ooxml::LN_token:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->SetCurrentRedlineToken( nIntValue );
        break;
        case NS_ooxml::LN_mark_shape:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            if( nIntValue )
                m_pImpl->PopShapeContext();
            else
                m_pImpl->PushShapeContext();
        break;
        case NS_ooxml::LN_CT_LineNumber_countBy:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        case NS_ooxml::LN_CT_LineNumber_restart:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        {
            //line numbering in Writer is a global document setting
            //in Word is a section setting
            //if line numbering is switched on anywhere in the document it's set at the global settings
            LineNumberSettings aSettings = m_pImpl->GetLineNumberSettings();
            switch( nName )
            {
                case NS_ooxml::LN_CT_LineNumber_countBy:
                    /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
                    aSettings.nInterval = nIntValue;
                break;
                case NS_ooxml::LN_CT_LineNumber_start:
                    /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
                    aSettings.nStartValue = nIntValue; // todo: has to be set at (each) first paragraph
                break;
                case NS_ooxml::LN_CT_LineNumber_distance:
                    /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
                    aSettings.nDistance = ConversionHelper::convertTwipToMM100( nIntValue );
                break;
                case NS_ooxml::LN_CT_LineNumber_restart:
                    /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
                    //page:empty, probably 0,section:1,continuous:2;
                    aSettings.bRestartAtEachPage = nIntValue < 1;
                break;
                default:;
            }
            m_pImpl->SetLineNumberSettings( aSettings );
        }
        break;
        case NS_ooxml::LN_CT_FtnEdnRef_customMarkFollows:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
            m_pImpl->SetCustomFtnMark( true );
        break;
        case NS_ooxml::LN_CT_FtnEdnRef_id:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            // footnote or endnote reference id - not needed
        case NS_ooxml::LN_CT_Color_themeColor:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Color_themeTint:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
        case NS_ooxml::LN_CT_Color_themeShade:
            /* WRITERFILTERSTATUS: done: 0, planned: 0, spent: 0 */
            //unsupported
        break;
        default:
            {
#if OSL_DEBUG_LEVEL > 0
            ::rtl::OString sMessage( "DomainMapper::attribute() - Id: ");
            sMessage += ::rtl::OString::valueOf( sal_Int32( nName ), 10 );
            sMessage += ::rtl::OString(" / 0x");
            sMessage += ::rtl::OString::valueOf( sal_Int32( nName ), 16 );
//            sMessage += ::rtl::OString(" / ");
//            sMessage += ::rtl::OString
//                ((*QNameToString::Instance())(nName).c_str());
            sMessage += ::rtl::OString(" value: ");
            sMessage += ::rtl::OString::valueOf( sal_Int32( nIntValue ), 10 );
            sMessage += ::rtl::OString(" / 0x");
            sMessage += ::rtl::OString::valueOf( sal_Int32( nIntValue ), 16 );
            OSL_ENSURE( false, sMessage.getStr()); //
#endif
            }
        }
    }
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("attribute");
#endif
}
/*-- 09.06.2006 09:52:12---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::sprm(Sprm & rSprm)
{
    if( !m_pImpl->getTableManager().sprm(rSprm))
        DomainMapper::sprm( rSprm, m_pImpl->GetTopContext() );
}
/*-- 20.06.2006 09:58:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper::sprm( Sprm& rSprm, PropertyMapPtr rContext, SprmType eSprmType )
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("sprm");
    dmapper_logger->chars(rSprm.toString());
#endif
    OSL_ENSURE(rContext.get(), "PropertyMap has to be valid!");
    if(!rContext.get())
        return ;

    sal_uInt32 nSprmId = rSprm.getId();
    //needed for page properties
    SectionPropertyMap* pSectionContext = 0;
    //the section context is not availabe before the first call of startSectionGroup()
    if( !m_pImpl->IsAnyTableImport() )
    {
        PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_SECTION);
        OSL_ENSURE(pContext.get(), "Section context is not in the stack!");
        pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
    }

    //TODO: In rtl-paragraphs the meaning of left/right are to be exchanged
    bool bExchangeLeftRight = false;
    // if( nSprmId == NS_sprm::LN_PJcExtra && AlreadyInRTLPara() )
    //      bExchangeLeftRight = true;
    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue->getInt();
    rtl::OUString sStringValue = pValue->getString();
    // printf ( "DomainMapper::sprm(0x%.4x, 0x%.4x) [%s]\n", (unsigned int)nSprmId, (unsigned int)nIntValue, ::rtl::OUStringToOString(sStringValue, RTL_TEXTENCODING_DONTKNOW).getStr());
    /* WRITERFILTERSTATUS: table: sprmdata */

    switch(nSprmId)
    {
    case 2:  // sprmPIstd
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
    case 0x4600:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPIstd - style code
    case 3: // "sprmPIstdPermute
    case NS_sprm::LN_PIstdPermute:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPIstdPermute
    case NS_sprm::LN_PIncLvl:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPIncLvl
    case NS_sprm::LN_PJcExtra: // sprmPJc Asian (undocumented)
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
    case NS_sprm::LN_PJc: // sprmPJc
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        handleParaJustification(nIntValue, rContext, bExchangeLeftRight);
        break;
    case NS_sprm::LN_PFSideBySide:
        /* WRITERFILTERSTATUS: done: 0, planned: 3, spent: 0 */
        /* WRITERFILTERSTATUS: comment: */
        break;  // sprmPFSideBySide

    case NS_sprm::LN_PFKeep:   // sprmPFKeep
        /* WRITERFILTERSTATUS: done: 0, planned: 3, spent: 0 */
        /* WRITERFILTERSTATUS: comment: */
        break;
    case NS_sprm::LN_PFKeepFollow:   // sprmPFKeepFollow
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        /* WRITERFILTERSTATUS: comment:  */
        rContext->Insert(PROP_PARA_KEEP_TOGETHER, true, uno::makeAny( nIntValue ? true : false) );
        break;
    case NS_sprm::LN_PFPageBreakBefore:
        /* WRITERFILTERSTATUS: done: 100, planned: 3, spent: 0 */
        /* WRITERFILTERSTATUS: comment:  */
        rContext->Insert(PROP_BREAK_TYPE, true, uno::makeAny( com::sun::star::style::BreakType_PAGE_BEFORE ) );
    break;  // sprmPFPageBreakBefore
    case NS_sprm::LN_PBrcl:
        break;  // sprmPBrcl
    case NS_sprm::LN_PBrcp:
        break;  // sprmPBrcp
    case NS_sprm::LN_PIlvl: // sprmPIlvl
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        /* WRITERFILTERSTATUS: comment:  */
        //todo: Numbering level will be implemented in the near future (OOo 3.0?)
            if( m_pImpl->IsStyleSheetImport() )
            {
                //style sheets cannot have a numbering rule attached
                StyleSheetPropertyMap* pStyleSheetPropertyMap = dynamic_cast< StyleSheetPropertyMap* >( rContext.get() );
                pStyleSheetPropertyMap->SetListLevel( (sal_Int16)nIntValue );
            }
            else
                rContext->Insert( PROP_NUMBERING_LEVEL, true, uno::makeAny( (sal_Int16)nIntValue ));
        break;
    case NS_sprm::LN_PIlfo: // sprmPIlfo
        /* WRITERFILTERSTATUS: done: 50, planned: 0, spent: 1 */
        /* WRITERFILTERSTATUS: comment:  */
        {
            //convert the ListTable entry to a NumberingRules propery and apply it
            sal_Int32 nListId = m_pImpl->GetLFOTable()->GetListID( nIntValue );
            if(nListId >= 0)
            {
                ListTablePtr pListTable = m_pImpl->GetListTable();
                if( m_pImpl->IsStyleSheetImport() )
                {
                    //style sheets cannot have a numbering rule attached
                    StyleSheetPropertyMap* pStyleSheetPropertyMap = dynamic_cast< StyleSheetPropertyMap* >( rContext.get() );
                    pStyleSheetPropertyMap->SetListId( nListId );
                }
                else
                    rContext->Insert( PROP_NUMBERING_RULES, true,
                                  uno::makeAny(pListTable->GetNumberingRules(nListId)));
                //TODO: Merge overwrittern numbering levels from LFO table
            }
        }
        break;
    case NS_sprm::LN_PFNoLineNumb:   // sprmPFNoLineNumb
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        /* WRITERFILTERSTATUS: comment:  */
        rContext->Insert(PROP_PARA_LINE_NUMBER_COUNT, true, uno::makeAny( nIntValue ? false : true) );
        break;
    case NS_sprm::LN_PChgTabsPapx:   // sprmPChgTabsPapx
        /* WRITERFILTERSTATUS: done: 90, planned: 8, spent: 8 */
        /* WRITERFILTERSTATUS: comment: bar tab stops a unavailable */
        {
            // Initialize tab stop vector from style sheet
            uno::Any aValue = m_pImpl->GetPropertyFromStyleSheet(PROP_PARA_TAB_STOPS);
            uno::Sequence< style::TabStop > aStyleTabStops;
            if(aValue >>= aStyleTabStops)
            {
                m_pImpl->InitTabStopFromStyle( aStyleTabStops );
            }

            //create a new tab stop property - this is done with the contained properties
            resolveSprmProps(rSprm);
            //add this property
            rContext->Insert(PROP_PARA_TAB_STOPS, true, uno::makeAny( m_pImpl->GetCurrentTabStopAndClear()));
        }
        break;
    case 0x845d:    //right margin Asian - undocumented
    case 0x845e:    //left margin Asian - undocumented
    case 16:      // sprmPDxaRight - right margin
    case NS_sprm::LN_PDxaRight:   // sprmPDxaRight - right margin
    case 17:
    case NS_sprm::LN_PDxaLeft:   // sprmPDxaLeft
        /* WRITERFILTERSTATUS: done: 50, planned: 5, spent: 1 */
        if( NS_sprm::LN_PDxaLeft == nSprmId || 0x17 == nSprmId|| (bExchangeLeftRight && nSprmId == 0x845d) || ( !bExchangeLeftRight && nSprmId == 0x845e))
            rContext->Insert(
                             eSprmType == SPRM_DEFAULT ? PROP_PARA_LEFT_MARGIN : PROP_LEFT_MARGIN,
                             true,
                             uno::makeAny( ConversionHelper::convertTwipToMM100( nIntValue ) ));
        else if(eSprmType == SPRM_DEFAULT)
            rContext->Insert(
                             PROP_PARA_RIGHT_MARGIN, true,
                             uno::makeAny( ConversionHelper::convertTwipToMM100(nIntValue ) ));
        //TODO: what happens to the right margins in numberings?
        break;
    case 18: // sprmPNest
    case NS_sprm::LN_PNest: // sprmPNest
        //not handled in the old WW8 filter
        break;
    case NS_sprm::LN_PDxaLeft1:    // sprmPDxaLeft1
    case 19:
    case NS_sprm::LN_PDxaLeft180:   // sprmPDxaLeft180
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        rContext->Insert(
                         eSprmType == SPRM_DEFAULT ? PROP_PARA_FIRST_LINE_INDENT : PROP_FIRST_LINE_OFFSET,
                         true,
                         uno::makeAny( ConversionHelper::convertTwipToMM100(nIntValue ) ));
        break;
    case 20 : // sprmPDyaLine
    case NS_sprm::LN_PDyaLine:   // sprmPDyaLine
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        {
            style::LineSpacing aSpacing;
            sal_Int16 nDistance = sal_Int16(nIntValue & 0xffff);
            if(nIntValue & 0xffff0000)
            {
                // single line in Writer is 100, in Word it is 240
                aSpacing.Mode = style::LineSpacingMode::PROP;
                aSpacing.Height = sal_Int16(sal_Int32(nDistance) * 100 /240);
            }
            else
            {
                if(nDistance < 0)
                {
                    aSpacing.Mode = style::LineSpacingMode::FIX;
                    aSpacing.Height = sal_Int16(ConversionHelper::convertTwipToMM100(-nDistance));
                }
                else if(nDistance >0)
                {
                    aSpacing.Mode = style::LineSpacingMode::MINIMUM;
                    aSpacing.Height = sal_Int16(ConversionHelper::convertTwipToMM100(nDistance));
                }
            }
            rContext->Insert(PROP_PARA_LINE_SPACING, true, uno::makeAny( aSpacing ));
        }
        break;
    case 21 : // legacy version
    case NS_sprm::LN_PDyaBefore:   // sprmPDyaBefore
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        rContext->Insert(PROP_PARA_TOP_MARGIN, true, uno::makeAny( ConversionHelper::convertTwipToMM100( nIntValue ) ));
        break;
    case 22 :
    case NS_sprm::LN_PDyaAfter:   // sprmPDyaAfter
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 1 */
        rContext->Insert(PROP_PARA_BOTTOM_MARGIN, true, uno::makeAny( ConversionHelper::convertTwipToMM100( nIntValue ) ));
        break;

    case  23: //sprmPChgTabs
    case NS_sprm::LN_PChgTabs: // sprmPChgTabs
        /* WRITERFILTERSTATUS: done: 0, planned: 3, spent: 0 */
        OSL_ENSURE( false, "unhandled");
        //tabs of list level?
        break;
    case 24: // "sprmPFInTable"
    case NS_sprm::LN_PFInTable:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFInTable
    case NS_sprm::LN_PTableDepth: //sprmPTableDepth
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        //not handled via sprm but via text( 0x07 )
    break;
    case 25: // "sprmPTtp" pap.fTtp
    case NS_sprm::LN_PFTtp:   // sprmPFTtp  was: Read_TabRowEnd
        break;
    case 26:  // "sprmPDxaAbs
    case NS_sprm::LN_PDxaAbs:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPDxaAbs
    case 27: //sprmPDyaAbs
    case NS_sprm::LN_PDyaAbs:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPDyaAbs
    case NS_sprm::LN_PDxaWidth:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPDxaWidth
    case NS_sprm::LN_PPc:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPPc
    case NS_sprm::LN_PBrcTop10:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPBrcTop10
    case NS_sprm::LN_PBrcLeft10:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPBrcLeft10
    case NS_sprm::LN_PBrcBottom10:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPBrcBottom10
    case NS_sprm::LN_PBrcRight10:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPBrcRight10
    case NS_sprm::LN_PBrcBetween10:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPBrcBetween10
    case NS_sprm::LN_PBrcBar10:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPBrcBar10
    case NS_sprm::LN_PDxaFromText10:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPDxaFromText10
    case NS_sprm::LN_PWr:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPWr

    case NS_ooxml::LN_CT_PrBase_pBdr: //paragraph border
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        resolveSprmProps(rSprm);
    break;
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
    case NS_sprm::LN_PBrcTop:   // sprmPBrcTop
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        /* WRITERFILTERSTATUS: comment: page borders are no handled yet, conversion incomplete */
    case NS_sprm::LN_PBrcLeft:   // sprmPBrcLeft
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        /* WRITERFILTERSTATUS: comment: page borders are no handled yet, conversion incomplete */
    case NS_sprm::LN_PBrcBottom:   // sprmPBrcBottom
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        /* WRITERFILTERSTATUS: comment: page borders are no handled yet, conversion incomplete */
    case NS_sprm::LN_PBrcRight:   // sprmPBrcRight
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        /* WRITERFILTERSTATUS: comment: page borders are no handled yet, conversion incomplete */
    case NS_sprm::LN_PBrcBetween:   // sprmPBrcBetween
        /* WRITERFILTERSTATUS: done: 0, planned: 8, spent: 0 */
        /* WRITERFILTERSTATUS: comment:  */
        {
            //in binary format the borders are directly provided in OOXML they are inside of properties
            if( IsOOXMLImport() )
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
                        case NS_sprm::LN_PBrcTop:
                            /* WRITERFILTERSTATUS: */
                            eBorderId = PROP_TOP_BORDER;
                            eBorderDistId = PROP_TOP_BORDER_DISTANCE;
                        break;
                        case NS_sprm::LN_PBrcLeft:
                            /* WRITERFILTERSTATUS: */
                            eBorderId = PROP_LEFT_BORDER;
                            eBorderDistId = PROP_LEFT_BORDER_DISTANCE;
                        break;
                        case NS_sprm::LN_PBrcBottom:
                            /* WRITERFILTERSTATUS: */
                            eBorderId = PROP_BOTTOM_BORDER         ;
                            eBorderDistId = PROP_BOTTOM_BORDER_DISTANCE;
                        break;
                        case NS_sprm::LN_PBrcRight:
                            /* WRITERFILTERSTATUS: */
                            eBorderId = PROP_RIGHT_BORDER;
                            eBorderDistId = PROP_RIGHT_BORDER_DISTANCE ;
                        break;
                        case NS_sprm::LN_PBrcBetween:
                            /* WRITERFILTERSTATUS: */
                            //not supported
                        break;
                        default:;
                    }
                    if( eBorderId )
                        rContext->Insert( eBorderId, true, uno::makeAny( pBorderHandler->getBorderLine()) , true);
                    if(eBorderDistId)
                        rContext->Insert(eBorderDistId, true, uno::makeAny( pBorderHandler->getLineDistance()), true);
                }
            }
            else
            {
                table::BorderLine aBorderLine;
                sal_Int32 nLineDistance = ConversionHelper::MakeBorderLine( nIntValue, aBorderLine );
                PropertyIds eBorderId = PROP_LEFT_BORDER;
                PropertyIds eBorderDistId = PROP_LEFT_BORDER_DISTANCE  ;
                switch( nSprmId )
                {
                case NS_sprm::LN_PBrcBetween:   // sprmPBrcBetween
                    /* WRITERFILTERSTATUS: */
                    OSL_ENSURE( false, "TODO: inner border is not handled");
                    break;
                case NS_sprm::LN_PBrcLeft:   // sprmPBrcLeft
                    /* WRITERFILTERSTATUS: */
                    eBorderId = PROP_LEFT_BORDER;
                    eBorderDistId = PROP_LEFT_BORDER_DISTANCE  ;
                    break;
                case NS_sprm::LN_PBrcRight:   // sprmPBrcRight
                    /* WRITERFILTERSTATUS: */
                    eBorderId = PROP_RIGHT_BORDER          ;
                    eBorderDistId = PROP_RIGHT_BORDER_DISTANCE ;
                    break;
                case NS_sprm::LN_PBrcTop:   // sprmPBrcTop
                    /* WRITERFILTERSTATUS: */
                    eBorderId = PROP_TOP_BORDER            ;
                    eBorderDistId = PROP_TOP_BORDER_DISTANCE;
                    break;
                case NS_sprm::LN_PBrcBottom:   // sprmPBrcBottom
                    /* WRITERFILTERSTATUS: */
                default:
                    eBorderId = PROP_BOTTOM_BORDER         ;
                    eBorderDistId = PROP_BOTTOM_BORDER_DISTANCE;
                }
                rContext->Insert(eBorderId, true, uno::makeAny( aBorderLine ));
                rContext->Insert(eBorderDistId, true, uno::makeAny( nLineDistance ));
            }
        }
    break;
    case NS_sprm::LN_PBorderTop:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        /* WRITERFILTERSTATUS: comment: probably _real_ border colors, unhandled */
    case NS_sprm::LN_PBorderLeft:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        /* WRITERFILTERSTATUS: comment: probably _real_ border colors, unhandled */
    case NS_sprm::LN_PBorderBottom:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        /* WRITERFILTERSTATUS: comment: probably _real_ border colors, unhandled */
    case NS_sprm::LN_PBorderRight:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        /* WRITERFILTERSTATUS: comment: probably _real_ border colors, unhandled */
        OSL_ENSURE( false, "TODO: border color definition");
        break;
    case NS_sprm::LN_PBrcBar:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPBrcBar
    case NS_sprm::LN_PFNoAutoHyph:   // sprmPFNoAutoHyph
        /* WRITERFILTERSTATUS: done: 100, planned: 1, spent: 0 */
        rContext->Insert(PROP_PARA_IS_HYPHENATION, true, uno::makeAny( nIntValue ? false : true ));
        break;
    case NS_sprm::LN_PWHeightAbs:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPWHeightAbs
    case NS_sprm::LN_PDcs:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPDcs

    case NS_sprm::LN_PShd: // sprmPShd
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 2 */
    {
        //contains fore color, back color and shadow percentage, results in a brush
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
        {
            CellColorHandlerPtr pCellColorHandler( new CellColorHandler );
            pCellColorHandler->setParagraph();
            pProperties->resolve(*pCellColorHandler);
            rContext->insert( pCellColorHandler->getProperties(), true );
        }
    }
    break;
    case NS_sprm::LN_PDyaFromText:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPDyaFromText
    case NS_sprm::LN_PDxaFromText:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPDxaFromText
    case NS_sprm::LN_PFLocked:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFLocked
    case NS_sprm::LN_PFWidowControl:
    case NS_ooxml::LN_CT_PPrBase_widowControl:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
    {
        uno::Any aVal( uno::makeAny( sal_Int8(nIntValue ? 2 : 0 )));
        rContext->Insert( PROP_PARA_WIDOWS, true, aVal );
        rContext->Insert( PROP_PARA_ORPHANS, true, aVal );
    }
    break;  // sprmPFWidowControl
    case NS_sprm::LN_PRuler:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPRuler
    case NS_sprm::LN_PFKinsoku:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFKinsoku
    case NS_sprm::LN_PFWordWrap:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFWordWrap
    case NS_sprm::LN_PFOverflowPunct: ;  // sprmPFOverflowPunct - hanging punctuation
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        rContext->Insert(PROP_PARA_IS_HANGING_PUNCTUATION, true, uno::makeAny( nIntValue ? false : true ));
        break;
    case NS_sprm::LN_PFTopLinePunct:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFTopLinePunct
    case NS_sprm::LN_PFAutoSpaceDE:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFAutoSpaceDE
    case NS_sprm::LN_PFAutoSpaceDN:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFAutoSpaceDN
    case NS_sprm::LN_PWAlignFont:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPWAlignFont
    case NS_sprm::LN_PFrameTextFlow:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFrameTextFlow
    case NS_sprm::LN_PISnapBaseLine:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPISnapBaseLine
    case NS_sprm::LN_PAnld:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPAnld
    case NS_sprm::LN_PPropRMark:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPPropRMark
    case NS_sprm::LN_POutLvl:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPOutLvl
    case NS_sprm::LN_PFBiDi:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFBiDi
    case NS_sprm::LN_PFNumRMIns:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFNumRMIns
    case NS_sprm::LN_PCrLf:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPCrLf
    case NS_sprm::LN_PNumRM:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPNumRM
    case NS_sprm::LN_PHugePapx:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPHugePapx
    case NS_sprm::LN_PFUsePgsuSettings:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFUsePgsuSettings
    case NS_sprm::LN_PFAdjustRight:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPFAdjustRight
    case NS_sprm::LN_CFRMarkDel:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFRMarkDel
    case NS_sprm::LN_CFRMark:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFRMark
    case NS_sprm::LN_CFFldVanish:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFFldVanish
    case NS_sprm::LN_CFSpec:   // sprmCFSpec
        break;
    case NS_sprm::LN_CPicLocation:   // sprmCPicLocation
            //is being resolved on the tokenizer side
        break;
    case NS_sprm::LN_CIbstRMark:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIbstRMark
    case NS_sprm::LN_CDttmRMark:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCDttmRMark
    case NS_sprm::LN_CFData:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFData
    case NS_sprm::LN_CIdslRMark:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIdslRMark
    case NS_sprm::LN_CChs:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCChs
    case NS_sprm::LN_CSymbol: // sprmCSymbol
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        resolveSprmProps(rSprm); //resolves LN_FONT and LN_CHAR
    break;
    case NS_sprm::LN_CFOle2:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFOle2
    case NS_sprm::LN_CIdCharType:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIdCharType
    case NS_sprm::LN_CHighlight:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        {
            sal_Int32 nColor = 0;
            if(true ==( mbIsHighlightSet = getColorFromIndex(nIntValue, nColor)))
                rContext->Insert(PROP_CHAR_BACK_COLOR, true, uno::makeAny( nColor ));
            else if (mnBackgroundColor)
                rContext->Insert(PROP_CHAR_BACK_COLOR, true, uno::makeAny( mnBackgroundColor ));
        }
        break;  // sprmCHighlight
    case NS_sprm::LN_CObjLocation:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCObjLocation
    case NS_sprm::LN_CFFtcAsciSymb:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFFtcAsciSymb
    case NS_sprm::LN_CIstd:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIstd
    case NS_sprm::LN_CIstdPermute:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIstdPermute
    case NS_sprm::LN_CDefault:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCDefault
    case NS_sprm::LN_CPlain:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCPlain
    case NS_sprm::LN_CKcd:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        rContext->Insert(PROP_CHAR_EMPHASIS, true, uno::makeAny ( getEmphasisValue (nIntValue)));
        break;  // sprmCKcd
    case NS_sprm::LN_CFEmboss:// sprmCFEmboss
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case 60:// sprmCFBold
    case NS_sprm::LN_CFBoldBi:// sprmCFBoldBi    (offset 0x27 to normal bold)
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case NS_sprm::LN_CFItalicBi:// sprmCFItalicBi  (offset 0x27 to normal italic)
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case NS_sprm::LN_CFBold: //sprmCFBold
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case 61: /*sprmCFItalic*/
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case NS_sprm::LN_CFItalic: //sprmCFItalic
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case NS_sprm::LN_CFStrike: //sprmCFStrike
        /* WRITERFILTERSTATUS: done: 100, planned: , spent:  0.5*/
    case NS_sprm::LN_CFOutline: //sprmCFOutline
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case NS_sprm::LN_CFShadow: //sprmCFShadow
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case NS_sprm::LN_CFSmallCaps: //sprmCFSmallCaps
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case NS_sprm::LN_CFCaps: //sprmCFCaps
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case NS_sprm::LN_CFVanish: //sprmCFVanish
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
    case NS_sprm::LN_CFDStrike:   // sprmCFDStrike
        /* WRITERFILTERSTATUS: done: 100, planned: , spent: 0.5 */
        {
            PropertyIds ePropertyId = PROP_CHAR_WEIGHT; //initialized to prevent warning!
            switch( nSprmId )
            {
            case 60:// sprmCFBold
            case NS_sprm::LN_CFBoldBi: // sprmCFBoldBi
            case NS_sprm::LN_CFBold: /*sprmCFBold*/
                /* WRITERFILTERSTATUS: */
                ePropertyId = nSprmId != NS_sprm::LN_CFBoldBi ? PROP_CHAR_WEIGHT : PROP_CHAR_WEIGHT_COMPLEX;
                break;
            case 61: /*sprmCFItalic*/
            case NS_sprm::LN_CFItalicBi: // sprmCFItalicBi
            case NS_sprm::LN_CFItalic: /*sprmCFItalic*/
                /* WRITERFILTERSTATUS: */
                ePropertyId = nSprmId == 0x836 ? PROP_CHAR_POSTURE : PROP_CHAR_POSTURE_COMPLEX;
                break;
            case NS_sprm::LN_CFStrike: /*sprmCFStrike*/
            case NS_sprm::LN_CFDStrike : /*sprmCFDStrike double strike through*/
                /* WRITERFILTERSTATUS: */
                ePropertyId = PROP_CHAR_STRIKEOUT;
                break;
            case NS_sprm::LN_CFOutline: /*sprmCFOutline*/
                /* WRITERFILTERSTATUS: */
                ePropertyId = PROP_CHAR_CONTOURED;
                break;
            case NS_sprm::LN_CFShadow: /*sprmCFShadow*/
                /* WRITERFILTERSTATUS: */
                ePropertyId = PROP_CHAR_SHADOWED;
                break;
            case NS_sprm::LN_CFSmallCaps: /*sprmCFSmallCaps*/
            case NS_sprm::LN_CFCaps: /*sprmCFCaps*/
                /* WRITERFILTERSTATUS: */
                ePropertyId = PROP_CHAR_CASE_MAP;
                break;
            case NS_sprm::LN_CFVanish: /*sprmCFVanish*/
                /* WRITERFILTERSTATUS: */
                ePropertyId = PROP_CHAR_HIDDEN;
                break;
            case NS_sprm::LN_CFEmboss: /*sprmCFEmboss*/
                /* WRITERFILTERSTATUS: */
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
                    double fDoubleValue;
                    uno::Any aStyleVal = m_pImpl->GetPropertyFromStyleSheet(ePropertyId);
                    if( !aStyleVal.hasValue() )
                    {
                        nIntValue = 0x83a == nSprmId ?
                            4 : 1;
                    }
                    else if(aStyleVal.getValueTypeClass() == uno::TypeClass_FLOAT )
                    {
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
                        OSL_ENSURE( false, "what type was it");
                    }
                }

                switch( nSprmId )
                {
                    case 60:/*sprmCFBold*/
                    case NS_sprm::LN_CFBold: /*sprmCFBold*/
                    case NS_sprm::LN_CFBoldBi: // sprmCFBoldBi
                        /* WRITERFILTERSTATUS: */
                    {
                        uno::Any aBold( uno::makeAny( nIntValue ? awt::FontWeight::BOLD : awt::FontWeight::NORMAL ) );
                        rContext->Insert(ePropertyId, true, aBold );
                        if( nSprmId != NS_sprm::LN_CFBoldBi ) // sprmCFBoldBi
                            rContext->Insert(PROP_CHAR_WEIGHT_ASIAN, true, aBold );
                    }
                    break;
                    case 61: /*sprmCFItalic*/
                    case NS_sprm::LN_CFItalic: /*sprmCFItalic*/
                    case NS_sprm::LN_CFItalicBi: // sprmCFItalicBi
                        /* WRITERFILTERSTATUS: */
                    {
                        uno::Any aPosture( uno::makeAny( nIntValue ? awt::FontSlant_ITALIC : awt::FontSlant_NONE ) );
                        rContext->Insert( ePropertyId, true, aPosture );
                        if( nSprmId != NS_sprm::LN_CFItalicBi ) // sprmCFItalicBi
                            rContext->Insert(PROP_CHAR_POSTURE_ASIAN, true, aPosture );
                    }
                    break;
                    case NS_sprm::LN_CFStrike: /*sprmCFStrike*/
                        /* WRITERFILTERSTATUS: */
                        rContext->Insert(ePropertyId, true,
                                         uno::makeAny( nIntValue ? awt::FontStrikeout::SINGLE : awt::FontStrikeout::NONE ) );
                    break;
                    case NS_sprm::LN_CFDStrike : /*sprmCFDStrike double strike through*/
                        /* WRITERFILTERSTATUS: */
                        rContext->Insert(ePropertyId, true,
                                         uno::makeAny( awt::FontStrikeout::DOUBLE ) );
                    break;
                    case NS_sprm::LN_CFOutline: /*sprmCFOutline*/
                    case NS_sprm::LN_CFShadow: /*sprmCFShadow*/
                    case NS_sprm::LN_CFVanish: /*sprmCFVanish*/
                        /* WRITERFILTERSTATUS: */
                        rContext->Insert(ePropertyId, true, uno::makeAny( nIntValue ? true : false ));
                    break;
                    case NS_sprm::LN_CFSmallCaps: /*sprmCFSmallCaps*/
                        /* WRITERFILTERSTATUS: */
                        rContext->Insert(ePropertyId, true,
                                         uno::makeAny( nIntValue ? style::CaseMap::SMALLCAPS : style::CaseMap::NONE));
                    break;
                    case NS_sprm::LN_CFCaps: /*sprmCFCaps*/
                        /* WRITERFILTERSTATUS: */
                        rContext->Insert(ePropertyId, true,
                                         uno::makeAny( nIntValue ? style::CaseMap::UPPERCASE : style::CaseMap::NONE));
                    break;
                    case NS_sprm::LN_CFEmboss: /*sprmCFEmboss*/
                        /* WRITERFILTERSTATUS: */
                        rContext->Insert(ePropertyId, true,
                                         uno::makeAny( nIntValue ? awt::FontRelief::EMBOSSED : awt::FontRelief::NONE ));
                    break;

                }
            }
        }
        break;
    case NS_sprm::LN_CFtcDefault:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFtcDefault
    case NS_sprm::LN_CKul: // sprmCKul
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        {
            // Parameter:  0 = none,    1 = single,  2 = by Word,
            // 3 = double,  4 = dotted,  5 = hidden
            // 6 = thick,   7 = dash,    8 = dot(not used)
            // 9 = dotdash 10 = dotdotdash 11 = wave
            handleUnderlineType(nIntValue, rContext);
        }
        break;
    case NS_sprm::LN_CSizePos:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCSizePos
    case NS_sprm::LN_CLid:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCLid
    case NS_sprm::LN_CIco:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        {
            sal_Int32 nColor = 0;
            if (getColorFromIndex(nIntValue, nColor))
                rContext->Insert(PROP_CHAR_COLOR, true, uno::makeAny( nColor ) );
        }
        break;  // sprmCIco
    case NS_sprm::LN_CHpsBi:    // sprmCHpsBi
    case NS_sprm::LN_CHps:    // sprmCHps
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        {
            //multiples of half points (12pt == 24)
            double fVal = double(nIntValue) / 2.;
            uno::Any aVal = uno::makeAny( fVal );
            if( NS_sprm::LN_CHpsBi == nSprmId )
                rContext->Insert( PROP_CHAR_HEIGHT_COMPLEX, true, aVal );
            else
            {
                //Asian get the same value as Western
                rContext->Insert( PROP_CHAR_HEIGHT, true, aVal );
                rContext->Insert( PROP_CHAR_HEIGHT_ASIAN, true, aVal );
            }
        }
        break;
    case NS_sprm::LN_CHpsInc:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCHpsInc
    case NS_sprm::LN_CHpsPos:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        {
        // FIXME: ww8 filter in ww8par6.cxx has a Read_SubSuperProp function
        // that counts the escapement from this value and font size. So it will be
        // on our TODO list
            sal_Int16 nEscapement = 0;
            sal_Int8 nProp  = 100;
            if (nIntValue < 0)
                nEscapement = -58;
            else if (nIntValue > 0)
                nEscapement = 58;
            else /* (nIntValue == 0) */
                nProp = 0;
            rContext->Insert(PROP_CHAR_ESCAPEMENT,         true, uno::makeAny( nEscapement ) );
            rContext->Insert(PROP_CHAR_ESCAPEMENT_HEIGHT,  true, uno::makeAny( nProp ) );
        }
        break;  // sprmCHpsPos
    case NS_sprm::LN_CHpsPosAdj:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCHpsPosAdj
    case NS_sprm::LN_CMajority:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCMajority
    case NS_sprm::LN_CIss:   // sprmCIss
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        {
            //sub/super script 1: super, 2: sub, 0: normal
            sal_Int16 nEscapement = 0;
            sal_Int8 nProp  = 58;
            switch(nIntValue)
            {
            case 1: //super
                nEscapement = 101;
                break;
            case 2: //sub
                nEscapement = -101;
                break;
            case 0: nProp = 0;break; //none
            }
            rContext->Insert(PROP_CHAR_ESCAPEMENT,         true, uno::makeAny( nEscapement ) );
            rContext->Insert(PROP_CHAR_ESCAPEMENT_HEIGHT,  true, uno::makeAny( nProp ) );
        }
        break;
    case NS_sprm::LN_CHpsNew50:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCHpsNew50
    case NS_sprm::LN_CHpsInc1:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCHpsInc1
    case 71 : //"sprmCDxaSpace"
    case 96 : //"sprmCDxaSpace"
    case NS_sprm::LN_CDxaSpace:  // sprmCDxaSpace
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */
        //Kerning half point values
        //TODO: there are two kerning values -
        // in ww8par6.cxx NS_sprm::LN_CHpsKern is used as boolean AutoKerning
        rContext->Insert(PROP_CHAR_CHAR_KERNING, true, uno::makeAny( sal_Int16(ConversionHelper::convertTwipToMM100(sal_Int16(nIntValue))) ) );
        break;
    case NS_sprm::LN_CHpsKern:  // sprmCHpsKern    auto kerning is bound to a minimum font size in Word - but not in Writer :-(
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        rContext->Insert(PROP_CHAR_AUTO_KERNING, true, uno::makeAny( true ) );
        break;
    case NS_sprm::LN_CMajority50:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCMajority50
    case NS_sprm::LN_CHpsMul:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCHpsMul
    case NS_sprm::LN_CYsri:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCYsri
    case NS_sprm::LN_CRgFtc0:  // sprmCRgFtc0     //ascii font index
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
    case NS_sprm::LN_CRgFtc1:  // sprmCRgFtc1     //Asian font index
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
    case NS_sprm::LN_CRgFtc2:  // sprmCRgFtc2     //CTL font index
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
    case NS_sprm::LN_CFtcBi: // sprmCFtcBi      //font index of a CTL font
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
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
                case NS_sprm::LN_CFtcBi:
                    eFontName =     PROP_CHAR_FONT_NAME_COMPLEX;
                    eFontStyle =    PROP_CHAR_FONT_STYLE_COMPLEX;
                    eFontFamily =   PROP_CHAR_FONT_FAMILY_COMPLEX;
                    eFontCharSet =  PROP_CHAR_FONT_CHAR_SET_COMPLEX;
                    eFontPitch =    PROP_CHAR_FONT_PITCH_COMPLEX;
                    break;
                }
                const FontEntry* pFontEntry = pFontTable->getFontEntry(sal_uInt32(nIntValue));
                rContext->Insert(eFontName, true, uno::makeAny( pFontEntry->sFontName  ));
                //                rContext->Insert(eFontStyle, uno::makeAny( pFontEntry->  ));
                //                rContext->Insert(eFontFamily, uno::makeAny( pFontEntry->  ));
                rContext->Insert(eFontCharSet, true, uno::makeAny( (sal_Int16)pFontEntry->nTextEncoding  ));
                rContext->Insert(eFontPitch, true, uno::makeAny( pFontEntry->nPitchRequest  ));
            }
        }
        break;
    case NS_sprm::LN_CCharScale:  // sprmCCharScale
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        rContext->Insert(PROP_CHAR_SCALE_WIDTH, true,
                         uno::makeAny( sal_Int16(nIntValue) ));
        break;
    case NS_sprm::LN_CFImprint: // sprmCFImprint   1 or 0
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        // FontRelief: NONE, EMBOSSED, ENGRAVED
        rContext->Insert(PROP_CHAR_RELIEF, true,
                         uno::makeAny( nIntValue ? awt::FontRelief::ENGRAVED : awt::FontRelief::NONE ));
        break;
    case NS_sprm::LN_CFObj:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFObj
    case NS_sprm::LN_CPropRMark:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCPropRMark
    case NS_sprm::LN_CSfxText:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        // The file-format has many character animations. We have only
        // one, so we use it always. Suboptimal solution though.
        if (nIntValue)
            rContext->Insert(PROP_CHAR_FLASH, true, uno::makeAny( true ));
        else
            rContext->Insert(PROP_CHAR_FLASH, true, uno::makeAny( false ));
        break;  // sprmCSfxText
    case NS_sprm::LN_CFBiDi:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFBiDi
    case NS_sprm::LN_CFDiacColor:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFDiacColor
    case NS_sprm::LN_CIcoBi:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIcoBi
    case NS_sprm::LN_CDispFldRMark:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCDispFldRMark
    case NS_sprm::LN_CIbstRMarkDel:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIbstRMarkDel
    case NS_sprm::LN_CDttmRMarkDel:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCDttmRMarkDel
    case NS_sprm::LN_CBrc:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCBrc
    case NS_sprm::LN_CShd:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCShd
    case NS_sprm::LN_CIdslRMarkDel:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCIdslRMarkDel
    case NS_sprm::LN_CFUsePgsuSettings:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCFUsePgsuSettings
    case NS_sprm::LN_CCpg:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmCCpg
    case NS_sprm::LN_CLidBi:  // sprmCLidBi      language complex
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
    case NS_sprm::LN_CRgLid0_80: //sprmCRgLid0_80
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 1 */
        //undocumented but interpreted as western language
    case NS_sprm::LN_CRgLid0:   // sprmCRgLid0    language Western
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
    case NS_sprm::LN_CRgLid1:   // sprmCRgLid1    language Asian
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        {
            lang::Locale aLocale;
            MsLangId::convertLanguageToLocale( (LanguageType)nIntValue, aLocale );
            rContext->Insert(NS_sprm::LN_CRgLid0 == nSprmId ? PROP_CHAR_LOCALE :
                             NS_sprm::LN_CRgLid1 == nSprmId ? PROP_CHAR_LOCALE_ASIAN : PROP_CHAR_LOCALE_COMPLEX,
                             true,
                             uno::makeAny( aLocale ) );
        }
        break;

    case NS_sprm::LN_CIdctHint:   // sprmCIdctHint
        //list table - text offset???
        break;
    case NS_sprm::LN_PicBrcl:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPicBrcl
    case NS_sprm::LN_PicScale:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPicScale
    case NS_sprm::LN_PicBrcTop:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPicBrcTop
    case NS_sprm::LN_PicBrcLeft:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPicBrcLeft
    case NS_sprm::LN_PicBrcBottom:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPicBrcBoConversionHelper::convertTwipToMM100ttom
    case NS_sprm::LN_PicBrcRight:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmPicBrcRight
    case NS_sprm::LN_ScnsPgn:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmScnsPgn
    case NS_sprm::LN_SiHeadingPgn:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetEvenlySpaced( nIntValue > 0 );

        break;  // sprmSiHeadingPgn
    case NS_sprm::LN_SOlstAnm:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSOlstAnm
    case 136:
    case NS_sprm::LN_SDxaColWidth: // sprmSDxaColWidth
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        // contains the twip width of the column as 3-byte-code
        // the lowet byte contains the index
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->AppendColumnWidth( ConversionHelper::convertTwipToMM100( (nIntValue & 0xffff00) >> 8 ));
        break;
    case NS_sprm::LN_SDxaColSpacing: // sprmSDxaColSpacing
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        // the lowet byte contains the index
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->AppendColumnSpacing( ConversionHelper::convertTwipToMM100( (nIntValue & 0xffff00) >> 8 ));
        break;
    case 138:
    case NS_sprm::LN_SFEvenlySpaced:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetEvenlySpaced( nIntValue > 0 );
        break;  // sprmSFEvenlySpaced
    case NS_sprm::LN_SFProtected: // sprmSFProtected
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        //todo: missing feature - unlocked sections in protected documents
        break;
    case NS_sprm::LN_SDmBinFirst: // sprmSDmBinFirst
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetFirstPaperBin(nIntValue);
        break;
    case NS_sprm::LN_SDmBinOther: // sprmSDmBinOther
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetPaperBin( nIntValue );
        break;
    case NS_sprm::LN_SBkc: // sprmSBkc
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */
        /* break type
          0 - No break
          1 - New Colunn
          2 - New page
          3 - Even page
          4 - odd page
        */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetBreakType( nIntValue );
        break;
    case 143:
    case NS_sprm::LN_SFTitlePage: // sprmSFTitlePage
    case NS_ooxml::LN_EG_SectPrContents_titlePg:
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
    {
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetTitlePage( nIntValue > 0 ? true : false );//section has title page
    }
    break;
    case 144:
    case NS_sprm::LN_SCcolumns: // sprmSCcolumns
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        //no of columns - 1
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetColumnCount( (sal_Int16) nIntValue );
    break;
    case 145:
    case NS_sprm::LN_SDxaColumns:           // sprmSDxaColumns
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        //column distance - default 708 twip
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetColumnDistance( ConversionHelper::convertTwipToMM100( nIntValue ) );
        break;
    case NS_sprm::LN_SFAutoPgn:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSFAutoPgn
    case 147:
    case NS_sprm::LN_SNfcPgn: // sprmSNfcPgn
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        //page numbering 0 - Arab, 1 - ROMAN, 2 - roman, 3 - ABC, 4 abc
        sal_Int16 nNumbering;
        switch( nIntValue )
        {
            case 1:  nNumbering = style::NumberingType::ROMAN_UPPER;
            case 2:  nNumbering = style::NumberingType::ROMAN_LOWER;
            case 3:  nNumbering = style::NumberingType::CHARS_UPPER_LETTER;
            case 4:  nNumbering = style::NumberingType::CHARS_LOWER_LETTER;
            case 0:
            default:
                    nNumbering = style::NumberingType::ARABIC;
        }
        rContext->Insert( PROP_NUMBERING_TYPE, false, uno::makeAny( nNumbering ) );
    break;
    case NS_sprm::LN_SDyaPgn:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSDyaPgn
    case NS_sprm::LN_SDxaPgn:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSDxaPgn
    case 150:
    case NS_sprm::LN_SFPgnRestart: // sprmSFPgnRestart
    {
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetPageNoRestart( nIntValue > 0 );
    }
    break;
    case NS_sprm::LN_SFEndnote:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSFEndnote
    case 154:
    case NS_sprm::LN_SNLnnMod:// sprmSNLnnMod
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if( pSectionContext )
            pSectionContext->SetLnnMod( nIntValue );
    break;
    case 155:
    case NS_sprm::LN_SDxaLnn: // sprmSDxaLnn
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if( pSectionContext )
            pSectionContext->SetdxaLnn( nIntValue );
    break;
    case 152:
    case NS_sprm::LN_SLnc:// sprmSLnc
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if( pSectionContext )
            pSectionContext->SetLnc( nIntValue );
    break;
    case 160:
    case NS_sprm::LN_SLnnMin: // sprmSLnnMin
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if( pSectionContext )
            pSectionContext->SetLnnMin( nIntValue );
    break;

    case NS_sprm::LN_SGprfIhdt:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        //flags about header/footer sharing and footnotes?
        /* ww8scan.hxx:
         * WW8_HEADER_EVEN = 0x01, WW8_HEADER_ODD = 0x02, WW8_FOOTER_EVEN = 0x04,
         * WW8_FOOTER_ODD = 0x08, WW8_HEADER_FIRST = 0x10, WW8_FOOTER_FIRST = 0x20
         */

//        if(pSectionContext)
    break;  // sprmSGprfIhdt
    case NS_sprm::LN_SDyaHdrTop: // sprmSDyaHdrTop
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        // default 720 twip
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetHeaderTop( ConversionHelper::convertTwipToMM100( nIntValue ));
    break;
    case NS_sprm::LN_SDyaHdrBottom: // sprmSDyaHdrBottom
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        // default 720 twip
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetHeaderBottom( ConversionHelper::convertTwipToMM100( nIntValue ) );
    break;
    case 158:
    case NS_sprm::LN_SLBetween: // sprmSLBetween
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetSeparatorLine( nIntValue > 0 );
    break;
    case NS_sprm::LN_SVjc:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        break;  // sprmSVjc
    case 161:
    case NS_sprm::LN_SPgnStart: // sprmSPgnStart
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        //page number
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetPageNumber( nIntValue );
    break;
    case 162:
    case NS_sprm::LN_SBOrientation:
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        //todo: the old filter assumed that a value of 2 points to double-pages layout
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetLandscape( nIntValue > 0 );
        rContext->Insert( PROP_IS_LANDSCAPE , false, uno::makeAny( nIntValue > 0 ));
    break;  // sprmSBOrientation
    case NS_sprm::LN_SBCustomize:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        break;  // sprmSBCustomize
    case 165:
    case NS_sprm::LN_SYaPage: // sprmSYaPage
    {
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        //page height, rounded to default values, default: 0x3dc0 twip
        sal_Int32 nHeight = ConversionHelper::convertTwipToMM100( nIntValue );
        rContext->Insert( PROP_HEIGHT, false, uno::makeAny( PaperInfo::sloppyFitPageDimension( nHeight ) ) );
    }
    break;
    case NS_sprm::LN_SXaPage:   // sprmSXaPage
    {
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        //page width, rounded to default values, default 0x2fd0 twip
        sal_Int32 nWidth = ConversionHelper::convertTwipToMM100( nIntValue );
        rContext->Insert( PROP_WIDTH, false, uno::makeAny( PaperInfo::sloppyFitPageDimension( nWidth ) ) );
    }
    break;
    case 166:
    case NS_sprm::LN_SDxaLeft:  // sprmSDxaLeft
    {
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        //left page margin default 0x708 twip
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        sal_Int32 nConverted = ConversionHelper::convertTwipToMM100( nIntValue );
        if(pSectionContext)
            pSectionContext->SetLeftMargin( nConverted );
        rContext->Insert( PROP_LEFT_MARGIN, false, uno::makeAny( nConverted ));
    }
    break;
    case 167:
    case NS_sprm::LN_SDxaRight: // sprmSDxaRight
    {
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        //right page margin default 0x708 twip
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        sal_Int32 nConverted = ConversionHelper::convertTwipToMM100( nIntValue );
        if(pSectionContext)
            pSectionContext->SetRightMargin( nConverted );
        rContext->Insert( PROP_RIGHT_MARGIN, false, uno::makeAny( nConverted ));
    }
    break;
    case 168:
    case NS_sprm::LN_SDyaTop: // sprmSDyaTop
    {
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        //top page margin default 1440 twip
        //todo: check cast of SVBT16
        sal_Int32 nConverted = ConversionHelper::convertTwipToMM100( static_cast< sal_Int16 >( nIntValue ) );
        rContext->Insert( PROP_TOP_MARGIN, false, uno::makeAny( nConverted ) );
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetTopMargin( nConverted );
    }
    break;
    case 169:
    case NS_sprm::LN_SDyaBottom: // sprmSDyaBottom
    {
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        //bottom page margin default 1440 twip
        //todo: check cast of SVBT16
        sal_Int32 nConverted = ConversionHelper::convertTwipToMM100( static_cast< sal_Int16 >( nIntValue ) );
        rContext->Insert( PROP_BOTTOM_MARGIN, false, uno::makeAny( nConverted) );
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetBottomMargin( nConverted );
    }
    break;
    case 170:
    case NS_sprm::LN_SDzaGutter:   // sprmSDzaGutter
    {
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        // gutter is added to one of the margins of a section depending on RTL, can be placed on top either
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            pSectionContext->SetDzaGutter( ConversionHelper::convertTwipToMM100( nIntValue  ) );
        }
    }
    break;
    case NS_sprm::LN_SDmPaperReq:   // sprmSDmPaperReq
        /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
        //paper code - no handled in old filter
        break;
    case NS_sprm::LN_SPropRMark:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSPropRMark
    case NS_sprm::LN_SFBiDi:// sprmSFBiDi
    {
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetSFBiDi( nIntValue > 0 );
    }
    break;
    case NS_sprm::LN_SFFacingCol:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmSFFacingCol
    case NS_sprm::LN_SFRTLGutter: // sprmSFRTLGutter
    {
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetGutterRTL( nIntValue > 0 );
    }
    break;
    case NS_sprm::LN_SBrcTop:   // sprmSBrcTop
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    case NS_sprm::LN_SBrcLeft:   // sprmSBrcLeft
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    case NS_sprm::LN_SBrcBottom:  // sprmSBrcBottom
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    case NS_sprm::LN_SBrcRight:  // sprmSBrcRight
        /* WRITERFILTERSTATUS: Sectiondone: 100, planned: 0.5, spent: 0 */
        {
            table::BorderLine aBorderLine;
            sal_Int32 nLineDistance = ConversionHelper::MakeBorderLine( nIntValue, aBorderLine );
            OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
            if(pSectionContext)
            {
                static const BorderPosition aPositions[4] =
                    {
                        BORDER_TOP,
                        BORDER_LEFT,
                        BORDER_BOTTOM,
                        BORDER_RIGHT
                    };
                pSectionContext->SetBorder( aPositions[nSprmId - NS_sprm::LN_SBrcTop], nLineDistance, aBorderLine );
            }
        }
        break;

    case NS_sprm::LN_SPgbProp:  // sprmSPgbProp
        {
            OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
            if(pSectionContext)
            {
                pSectionContext->ApplyBorderToPageStyles( m_pImpl->GetPageStyles(), m_pImpl->GetTextFactory(), nIntValue );
            }
        }
        break;
    case NS_sprm::LN_SDxtCharSpace:
    {
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            pSectionContext->SetDxtCharSpace( nIntValue );
        }
    }
    break;  // sprmSDxtCharSpace
    case NS_sprm::LN_SDyaLinePitch:   // sprmSDyaLinePitch
    {
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        //see SwWW8ImplReader::SetDocumentGrid
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            pSectionContext->SetGridLinePitch( ConversionHelper::convertTwipToMM100( nIntValue ) );
        }
    }
    break;
    case 0x703a: //undocumented, grid related?
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        OSL_ENSURE( false, "TODO: not handled yet"); //nIntValue like 0x008a2373 ?
        break;
    case NS_sprm::LN_SClm:
    {
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
        sal_Int16 nGridType = text::TextGridMode::NONE;
        switch( nIntValue )
        {
            case 0:
                nGridType = text::TextGridMode::NONE;
            break;
            case 3:
                //Text snaps to char grid, this doesn't make a lot of sense to
                //me. This is closer than LINES_CHARS
                nGridType = text::TextGridMode::LINES;
            break;
            case 1:
                nGridType = text::TextGridMode::LINES_AND_CHARS;
            break;
            case 2:
                nGridType = text::TextGridMode::LINES;
            break;
            default:;
        }
        rContext->Insert( PROP_GRID_MODE, false, uno::makeAny( nGridType ) );

    //Seems to force this behaviour in word ?
    if(nGridType != text::TextGridMode::NONE)
        m_pImpl->SetDocumentSettingsProperty(
            PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_ADD_EXTERNAL_LEADING ),
            uno::makeAny( true ) );
    }
    break;  // sprmSClm
    case NS_sprm::LN_STextFlow:
    {
        /* WRITERFILTERSTATUS: done: 100, planned: 2, spent: 0 */
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
        rContext->Insert(PROP_WRITING_MODE, false, uno::makeAny( nDirection ) );
    }
    break;  // sprmSTextFlow
    case NS_sprm::LN_TJc: // sprmTJc
    case NS_sprm::LN_TDxaLeft:
    case NS_sprm::LN_TDxaGapHalf:
    case NS_sprm::LN_TFCantSplit:
    case NS_sprm::LN_TTableHeader:
    case NS_sprm::LN_TTableBorders: // sprmTTableBorders
    {
        OSL_ENSURE( false, "table propeties should be handled by the table manager");
    }
    break;
    case NS_sprm::LN_TDefTable10:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDefTable10
    case NS_sprm::LN_TDyaRowHeight:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDyaRowHeight
    case NS_sprm::LN_TDefTable:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDefTable
    case NS_sprm::LN_TDefTableShd:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDefTableShd
    case NS_sprm::LN_TTlp:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTTlp
    case NS_sprm::LN_TFBiDi:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTFBiDi
    case NS_sprm::LN_THTMLProps:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTHTMLProps
    case NS_sprm::LN_TSetBrc:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTSetBrc
    case NS_sprm::LN_TInsert:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTInsert
    case NS_sprm::LN_TDelete:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDelete
    case NS_sprm::LN_TDxaCol:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDxaCol
    case NS_sprm::LN_TMerge:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTMerge
    case NS_sprm::LN_TSplit:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTSplit
    case NS_sprm::LN_TSetBrc10:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTSetBrc10
    case 164: // sprmTSetShd
    case NS_sprm::LN_TSetShd:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTSetShd
    case NS_sprm::LN_TSetShdOdd:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTSetShdOdd
    case NS_sprm::LN_TTextFlow:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTTextFlow
    case NS_sprm::LN_TDiagLine:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTDiagLine
    case NS_sprm::LN_TVertMerge:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTVertMerge
    case NS_sprm::LN_TVertAlign:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;  // sprmTVertAlign
        // the following are not part of the official documentation
    case 0x6870: //TxtForeColor
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        {
            //contains a color as 0xTTRRGGBB while SO uses 0xTTRRGGBB
            sal_Int32 nColor = ConversionHelper::ConvertColor(nIntValue);
            rContext->Insert(PROP_CHAR_COLOR, true, uno::makeAny( nColor ) );
        }
        break;
    case 0x4874:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break; //seems to be a language id for Asian text - undocumented
    case 0x6877: //underlining color
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        {
            sal_Int32 nColor = ConversionHelper::ConvertColor(nIntValue);
            rContext->Insert(PROP_CHAR_UNDERLINE_HAS_COLOR, true, uno::makeAny( true ) );
            rContext->Insert(PROP_CHAR_UNDERLINE_COLOR, true, uno::makeAny( nColor ) );
        }
        break;
    case 0x6815:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break; //undocumented
    case NS_sprm::LN_CIndrsid:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break; //undocumented
    case 0x6467:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break; //undocumented
    case 0xF617:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break; //undocumented
    case 0xd634: // sprmTNewSpacing - table spacing ( see WW8TabBandDesc::ProcessSpacing() )
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break;
    case NS_sprm::LN_TTRLeft:
        /* WRITERFILTERSTATUS: done: 0, planned: 2, spent: 0 */
        break; //undocumented
    case 0x4888:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case 0x6887:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        //properties of list levels - undocumented
        break;
    case 0xd234:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case 0xd235:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case 0xd236:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case 0xd237:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        break;//undocumented section properties
    case NS_sprm::LN_CEastAsianLayout:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        resolveSprmProps(rSprm);
        break;
    case NS_ooxml::LN_CT_Tabs_tab:
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        resolveSprmProps(rSprm);
        m_pImpl->IncorporateTabStop(m_pImpl->m_aCurrentTabStop);
        m_pImpl->m_aCurrentTabStop = DeletableTabStop();
    break;
    case NS_ooxml::LN_CT_PPrBase_tabs:
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
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
        resolveSprmProps(rSprm);
        rContext->Insert(PROP_PARA_TAB_STOPS, true, uno::makeAny( m_pImpl->GetCurrentTabStopAndClear()));
    }
    break;

    case NS_ooxml::LN_CT_PPr_sectPr:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_EG_RPrBase_color:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_EG_RPrBase_rFonts:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_EG_RPrBase_bdr:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_EG_RPrBase_eastAsianLayout:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_EG_RPrBase_u:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_EG_RPrBase_lang:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_CT_PPrBase_spacing:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_CT_PPrBase_ind:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_CT_RPrDefault_rPr:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_CT_PPrDefault_pPr:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_CT_DocDefaults_pPrDefault:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_CT_DocDefaults_rPrDefault:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_CT_Style_pPr:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_CT_Style_rPr:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_CT_PPr_rPr:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_CT_PPrBase_numPr:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        resolveSprmProps(rSprm);
    break;
    case NS_ooxml::LN_EG_SectPrContents_footnotePr:
        /* WRITERFILTERSTATUS: done: 1ß0, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_EG_SectPrContents_endnotePr:
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        m_pImpl->SetInFootnoteProperties( NS_ooxml::LN_EG_SectPrContents_footnotePr == nSprmId );
        resolveSprmProps(rSprm);
    break;
    case NS_ooxml::LN_EG_SectPrContents_lnNumType:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    {
        resolveSprmProps(rSprm);
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
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
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
        resolveSprmProps(rSprm);
    }
    break;
    case NS_ooxml::LN_EG_SectPrContents_pgSz:
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        CT_PageSz.code = 0;
        {
            PaperInfo aLetter(PAPER_LETTER);
            CT_PageSz.w = aLetter.getWidth();
            CT_PageSz.h = aLetter.getHeight();
        }
        CT_PageSz.orient = false;
        resolveSprmProps(rSprm);
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            pSectionContext->Insert( PROP_HEIGHT, false, uno::makeAny( CT_PageSz.h ) );
            pSectionContext->Insert( PROP_IS_LANDSCAPE, false, uno::makeAny( CT_PageSz.orient ));
            pSectionContext->Insert( PROP_WIDTH, false, uno::makeAny( CT_PageSz.w ) );
            pSectionContext->SetLandscape( CT_PageSz.orient );
        }
        break;

    case NS_ooxml::LN_EG_SectPrContents_pgMar:
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        m_pImpl->InitPageMargins();
        resolveSprmProps(rSprm);
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
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
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
                    for (; tmpIter != pSectHdl->GetColumns().end(); tmpIter++)
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
                }
            }
        }
    }
    break;

    case NS_ooxml::LN_CT_PPrBase_pStyle:
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    {
        m_pImpl->SetCurrentParaStyleId( sStringValue );
        StyleSheetTablePtr pStyleTable = m_pImpl->GetStyleSheetTable();
        const ::rtl::OUString sConvertedStyleName = pStyleTable->ConvertStyleName( sStringValue, true );
        if (m_pImpl->GetTopContext() && m_pImpl->GetTopContextType() != CONTEXT_SECTION)
            m_pImpl->GetTopContext()->Insert( PROP_PARA_STYLE_NAME, true, uno::makeAny( sConvertedStyleName ));
        const StyleSheetEntry* pEntry = pStyleTable->FindStyleSheetByISTD(sStringValue);
        //apply numbering to paragraph if it was set at the style
        OSL_ENSURE( pEntry, "no style sheet found" );
        const StyleSheetPropertyMap* pStyleSheetProperties = dynamic_cast<const StyleSheetPropertyMap*>(pEntry ? pEntry->pProperties.get() : 0);
        if( pStyleSheetProperties && pStyleSheetProperties->GetListId() >= 0 )
            rContext->Insert( PROP_NUMBERING_RULES, true, uno::makeAny(m_pImpl->GetListTable()->GetNumberingRules(pStyleSheetProperties->GetListId())), false);
        if( pStyleSheetProperties && pStyleSheetProperties->GetListLevel() >= 0 )
            rContext->Insert( PROP_NUMBERING_LEVEL, true, uno::makeAny(pStyleSheetProperties->GetListLevel()), false);
    }
    break;
    case NS_ooxml::LN_EG_RPrBase_rStyle:
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
        if (m_pImpl->GetTopContext())
            m_pImpl->GetTopContext()->Insert( PROP_CHAR_STYLE_NAME, true, uno::makeAny( m_pImpl->GetStyleSheetTable()->ConvertStyleName( sStringValue, true )));
    break;
    case NS_ooxml::LN_CT_TblPrBase_tblCellMar: //cell margins
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    {
        resolveSprmProps(rSprm);//contains LN_CT_TblCellMar_top, LN_CT_TblCellMar_left, LN_CT_TblCellMar_bottom, LN_CT_TblCellMar_right
    }
    break;
    case NS_ooxml::LN_CT_TblCellMar_top:
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_TblCellMar_left:
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_TblCellMar_bottom:
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
    case NS_ooxml::LN_CT_TblCellMar_right:
            /* WRITERFILTERSTATUS: done: 100, planned: 0, spent: 0 */
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
        {
            MeasureHandlerPtr pMeasureHandler( new MeasureHandler );
            pProperties->resolve(*pMeasureHandler);
            sal_Int32 nMeasureValue = pMeasureHandler->getMeasureValue();
            PropertyIds eId = META_PROP_CELL_MAR_TOP;
            switch(nSprmId)
            {
                case NS_ooxml::LN_CT_TblCellMar_top:
                    /* WRITERFILTERSTATUS: */
                break;
                case NS_ooxml::LN_CT_TblCellMar_left:
                    /* WRITERFILTERSTATUS: */
                    eId = META_PROP_CELL_MAR_LEFT;
                break;
                case NS_ooxml::LN_CT_TblCellMar_bottom:
                    /* WRITERFILTERSTATUS: */
                    eId = META_PROP_CELL_MAR_BOTTOM;
                break;
                case NS_ooxml::LN_CT_TblCellMar_right:
                    /* WRITERFILTERSTATUS: */
                    eId = META_PROP_CELL_MAR_RIGHT;
                break;
                default:;
            }
            rContext->Insert( eId, false, uno::makeAny(nMeasureValue), false);
        }
    }
    break;
    case NS_sprm::LN_CFNoProof: //0x875 no grammar and spell checking, unsupported
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    break;
    case NS_ooxml::LN_anchor_anchor: // at_character drawing
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_inline_inline: // as_character drawing
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
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
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    {
        sal_Int16 nEscapement = 0;
        sal_Int8 nProp  = 58;
        if( sStringValue.equalsAscii( "superscript" ))
                nEscapement = 101;
        else if( sStringValue.equalsAscii( "subscript" ))
                nEscapement = -101;
        else
            nProp = 100;

        rContext->Insert(PROP_CHAR_ESCAPEMENT,         true, uno::makeAny( nEscapement ) );
        rContext->Insert(PROP_CHAR_ESCAPEMENT_HEIGHT,  true, uno::makeAny( nProp ) );
    }
    break;
//    case NS_ooxml::LN_CT_FtnEdn_type
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
//    case NS_ooxml::LN_CT_FtnEdn_id
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
//    case NS_ooxml::LN_EG_FtnEdnNumProps_numRestart
    case NS_ooxml::LN_CT_FtnProps_pos:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    //footnotes in word can be at page end or beneath text - writer supports only the first
    //endnotes in word can be at section end or document end - writer supports only the latter
    // -> so this property can be ignored
    break;
    case NS_ooxml::LN_EG_FtnEdnNumProps_numStart:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_CT_FtnProps_numFmt:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_CT_EdnProps_numFmt:
            /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    {
        try
        {
            uno::Reference< beans::XPropertySet >  xFtnEdnSettings;
            if( m_pImpl->IsInFootnoteProperties() )
            {
                uno::Reference< text::XFootnotesSupplier> xFootnotesSupplier( m_pImpl->GetTextDocument(), uno::UNO_QUERY );
                xFtnEdnSettings = xFootnotesSupplier->getFootnoteSettings();
            }
            else
            {
                uno::Reference< text::XEndnotesSupplier> xEndnotesSupplier( m_pImpl->GetTextDocument(), uno::UNO_QUERY );
                xFtnEdnSettings = xEndnotesSupplier->getEndnoteSettings();
            }
            if( NS_ooxml::LN_EG_FtnEdnNumProps_numStart == nSprmId )
            {
                xFtnEdnSettings->setPropertyValue(
                    PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_START_AT),
                                                                    uno::makeAny( sal_Int16( nIntValue - 1 )));
            }
            else
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
    case NS_ooxml::LN_trackchange:
        /* WRITERFILTERSTATUS: done: 100, planned: 5, spent: 0 */
    case NS_ooxml::LN_EG_RPrContent_rPrChange:
        /* WRITERFILTERSTATUS: done: 100, planned: 5, spent: 0 */
    {
        resolveSprmProps( rSprm );
        // now the properties author, date and id should be available
        ::rtl::OUString sAuthor = m_pImpl->GetCurrentRedlineAuthor();
        ::rtl::OUString sDate = m_pImpl->GetCurrentRedlineDate();
        ::rtl::OUString sId = m_pImpl->GetCurrentRedlineId();
        sal_Int32 nToken = m_pImpl->GetCurrentRedlineToken();
        switch( nToken & 0xffff )
        {
            case ooxml::OOXML_mod :
            case ooxml::OOXML_ins :
            case ooxml::OOXML_del : break;
            default: OSL_ENSURE( false, "redline token other than mod, ins or del" );
        }
    }
    break;
    case NS_ooxml::LN_CT_RPrChange_rPr:
        /* WRITERFILTERSTATUS: done: 100, planned: 5, spent: 0 */
    break;
    /* WRITERFILTERSTATUS: done: 0, planned: 4, spent: 0 */
    case NS_ooxml::LN_object:
    {
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
        {
            OLEHandlerPtr pOLEHandler( new OLEHandler );
            pProperties->resolve(*pOLEHandler);
            ::rtl::OUString sStreamName = pOLEHandler->copyOLEOStream( m_pImpl->GetTextDocument() );
            if(sStreamName.getLength())
            {
                m_pImpl->appendOLE( sStreamName, pOLEHandler );
            }
        }
    }
    break;
//    case NS_ooxml::LN_CT_EdnProps_pos
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
//    case NS_ooxml::LN_CT_EdnProps_numFmt
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
//    case NS_ooxml::LN_CT_FtnDocProps_footnote
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
//    case NS_ooxml::LN_CT_EdnDocProps_endnote
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    //break;
    case NS_ooxml::LN_EG_HdrFtrReferences_headerReference: // header reference - not needed
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    case NS_ooxml::LN_EG_HdrFtrReferences_footerReference: // footer reference - not needed
        /* WRITERFILTERSTATUS: done: 100, planned: 0.5, spent: 0 */
    break;
    case NS_ooxml::LN_EG_RPrBase_snapToGrid: // "Use document grid  settings for inter-paragraph spacing"
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    break;
    case NS_sprm::LN_PContextualSpacing:
        //TODO: determines whether top/bottom paragraph spacing is added if equal styles are following - unsupported
    break;
    case NS_ooxml::LN_EG_SectPrContents_formProt: //section protection, only form editing is enabled - unsupported
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
    break;
    case NS_ooxml::LN_CT_Lvl_pStyle:
            /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        //TODO: numbering style should apply current numbering level - not yet supported
    break;

    default:
        {
#if OSL_DEBUG_LEVEL > 0
            ::rtl::OString sMessage( "DomainMapper::sprm() - Id: ");
            sMessage += ::rtl::OString::valueOf( sal_Int32( nSprmId ), 10 );
            sMessage += ::rtl::OString(" / 0x");
            sMessage += ::rtl::OString::valueOf( sal_Int32( nSprmId ), 16 );
            OSL_ENSURE( false, sMessage.getStr()); //
#endif
        }
    }

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("sprm");
#endif
}
/*-- 09.06.2006 09:52:13---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::entry(int /*pos*/,
                         writerfilter::Reference<Properties>::Pointer_t ref)
{
    ref->resolve(*this);
}
/*-- 09.06.2006 09:52:13---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::data(const sal_uInt8* /*buf*/, size_t /*len*/,
                        writerfilter::Reference<Properties>::Pointer_t /*ref*/)
{
}
/*-- 09.06.2006 09:52:13---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::startSectionGroup()
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("section");
#endif
    m_pImpl->PushProperties(CONTEXT_SECTION);
}
/*-- 09.06.2006 09:52:13---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::endSectionGroup()
{
    PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_SECTION);
    SectionPropertyMap* pSectionContext = dynamic_cast< SectionPropertyMap* >( pContext.get() );
    OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
    if(pSectionContext)
        pSectionContext->CloseSectionGroup( *m_pImpl );
    m_pImpl->PopProperties(CONTEXT_SECTION);

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("section");
#endif
}
/*-- 09.06.2006 09:52:13---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::startParagraphGroup()
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("paragraph");
#endif

    m_pImpl->getTableManager().startParagraphGroup();
    m_pImpl->PushProperties(CONTEXT_PARAGRAPH);
    static ::rtl::OUString sDefault( ::rtl::OUString::createFromAscii("Standard") );
    if (m_pImpl->GetTopContext())
    {
        m_pImpl->GetTopContext()->Insert( PROP_PARA_STYLE_NAME, true, uno::makeAny( sDefault ) );
        if (m_pImpl->isBreakDeferred(PAGE_BREAK))
               m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, true, uno::makeAny( com::sun::star::style::BreakType_PAGE_BEFORE) );
        else if (m_pImpl->isBreakDeferred(COLUMN_BREAK))
            m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, true, uno::makeAny( com::sun::star::style::BreakType_COLUMN_BEFORE) );
    }
    m_pImpl->clearDeferredBreaks();
}
/*-- 09.06.2006 09:52:14---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::endParagraphGroup()
{
    //handle unprocessed deferred breaks
    PropertyMapPtr pParaProperties = m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH);
    if( pParaProperties->hasEmptyPropertyValues() )
    {
        PropertyMap::const_iterator aIter = pParaProperties->find(PropertyDefinition( PROP_BREAK_TYPE , false ) );
        if( aIter != pParaProperties->end() )
        {
            style::BreakType eType;
            aIter->second >>= eType;
            bool bPage = false;
            bool bColumn = false;
            if( eType == style::BreakType_PAGE_BEFORE )
                bPage = true;
            else if( eType == style::BreakType_COLUMN_BEFORE )
                 bColumn = true;

            if( bPage || bColumn )
            {
                try
                {
                        uno::Reference< beans::XPropertySet > xRangeProperties( m_pImpl->GetTopTextAppend()->getEnd(), uno::UNO_QUERY_THROW );
                        xRangeProperties->setPropertyValue(
                            PropertyNameSupplier::GetPropertyNameSupplier().GetName(PROP_BREAK_TYPE),
                                                    uno::makeAny( bPage ? style::BreakType_PAGE_BEFORE : style::BreakType_COLUMN_BEFORE));
                }
                catch( const uno::Exception& )
                {
                }
            }
        }
    }

    m_pImpl->PopProperties(CONTEXT_PARAGRAPH);
    m_pImpl->getTableManager().endParagraphGroup();
    //frame conversion has to be executed after table conversion
    m_pImpl->ExecuteFrameConversion();
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("paragraph");
#endif
}

/*-- 13.06.2007 16:15:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper::PushStyleSheetProperties( PropertyMapPtr pStyleProperties )
{
    m_pImpl->PushStyleProperties( pStyleProperties );
}
/*-- 13.06.2007 16:15:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper::PopStyleSheetProperties()
{
    m_pImpl->PopProperties( CONTEXT_STYLESHEET );
}
/*-- 28.01.2008 14:52:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper::PushListProperties( ::boost::shared_ptr<PropertyMap> pListProperties )
{
    m_pImpl->PushListProperties( pListProperties );
}
/*-- 28.01.2008 14:52:33---------------------------------------------------

  -----------------------------------------------------------------------*/
void DomainMapper::PopListProperties()
{
    m_pImpl->PopProperties( CONTEXT_LIST );
}
/*-- 09.06.2006 09:52:14---------------------------------------------------

-----------------------------------------------------------------------*/

void DomainMapper::startCharacterGroup()
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("charactergroup");
#endif

    m_pImpl->PushProperties(CONTEXT_CHARACTER);
    DomainMapperTableManager& rTableManager = m_pImpl->getTableManager();
    if( rTableManager.getTableStyleName().getLength() )
    {
        PropertyMapPtr pTopContext = m_pImpl->GetTopContext();
        rTableManager.CopyTextProperties(pTopContext, m_pImpl->GetStyleSheetTable());
    }
}
/*-- 09.06.2006 09:52:14---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::endCharacterGroup()
{
    m_pImpl->PopProperties(CONTEXT_CHARACTER);

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("charactergroup");
#endif
}
/*-- 09.06.2006 09:52:14---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::text(const sal_uInt8 * data_, size_t len)
{
    //TODO: Determine the right text encoding (FIB?)
    ::rtl::OUString sText( (const sal_Char*) data_, len, RTL_TEXTENCODING_MS_1252 );
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
                m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, true, uno::makeAny( com::sun::star::style::BreakType_PAGE_BEFORE) );
            else if (m_pImpl->isBreakDeferred(COLUMN_BREAK))
                m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, true, uno::makeAny( com::sun::star::style::BreakType_COLUMN_BEFORE) );
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
            //--> debug
            //sal_uInt32 nSize = pContext->size();
            //<--

            m_pImpl->appendTextPortion( sText, pContext );
#ifdef DEBUG_DOMAINMAPPER
            dmapper_logger->startElement("text");
            dmapper_logger->chars(sText);
            dmapper_logger->endElement("text");
#endif
        }
    }
    catch( const uno::RuntimeException& )
    {
        std::clog << __FILE__ << "(l" << __LINE__ << ")" << std::endl;
    }
}
/*-- 09.06.2006 09:52:15---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::utext(const sal_uInt8 * data_, size_t len)
{
    OUString sText;
    OUStringBuffer aBuffer = OUStringBuffer(len);
    aBuffer.append( (const sal_Unicode *) data_, len);
    sText = aBuffer.makeStringAndClear();
    try
    {
        m_pImpl->getTableManager().utext(data_, len);

        if(len == 1 && ((*data_) == 0x0d || (*data_) == 0x07))
            m_pImpl->finishParagraph(m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH));
        else
        {

            PropertyMapPtr pContext = m_pImpl->GetTopContext();
            if ( pContext && !pContext->GetFootnote().is() )
            {
                if (m_pImpl->isBreakDeferred(PAGE_BREAK))
                    m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, true, uno::makeAny( com::sun::star::style::BreakType_PAGE_BEFORE) );
                else if (m_pImpl->isBreakDeferred(COLUMN_BREAK))
                    m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, true, uno::makeAny( com::sun::star::style::BreakType_COLUMN_BEFORE) );
                m_pImpl->clearDeferredBreaks();
            }

            /* doesn't seem to be working
            if( pContext->GetFootnote().is() )
            {
                //todo: the check for 0x0a is a hack!
                if( *data_ != 0x0a && !pContext->GetFootnoteSymbol() )
                    pContext->GetFootnote()->setLabel( sText );
                //otherwise ignore sText
            }
            else */
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
                m_pImpl->appendTextPortion( sText, pContext );

#ifdef DEBUG_DOMAINMAPPER
            dmapper_logger->startElement("utext");
            dmapper_logger->chars(sText);
            dmapper_logger->endElement("utext");
#endif
        }
    }
    catch( const uno::RuntimeException& )
    {
    }
}
/*-- 09.06.2006 09:52:15---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::props(writerfilter::Reference<Properties>::Pointer_t ref)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("props");
#endif

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

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("props");
#endif
}
/*-- 09.06.2006 09:52:15---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::table(Id name, writerfilter::Reference<Table>::Pointer_t ref)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("table");
    dmapper_logger->attribute("id", (*QNameToString::Instance())(name));
#endif

    // printf ( "DomainMapper::table(0x%.4x)\n", (unsigned int)name);
    m_pImpl->SetAnyTableImport(true);
    /* WRITERFILTERSTATUS: table: attributedata */
    switch(name)
    {
    case NS_rtf::LN_FONTTABLE:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        // create a font table object that listens to the attributes
        // each entry call inserts a new font entry
        ref->resolve( *m_pImpl->GetFontTable() );
        break;
    case NS_rtf::LN_STYLESHEET:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */
        //same as above to import style sheets
        m_pImpl->SetStyleSheetImport( true );
        ref->resolve( *m_pImpl->GetStyleSheetTable() );
        m_pImpl->GetStyleSheetTable()->ApplyStyleSheets(m_pImpl->GetFontTable());
        m_pImpl->SetStyleSheetImport( false );
        break;
    case NS_ooxml::LN_NUMBERING:
    case NS_rtf::LN_LISTTABLE:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        //the same for list tables
        ref->resolve( *m_pImpl->GetListTable() );
        break;
    case NS_rtf::LN_LFOTABLE:
        /* WRITERFILTERSTATUS: done: 0, planned: 0.5, spent: 0 */

        ref->resolve( *m_pImpl->GetLFOTable() );
        break;
    case NS_ooxml::LN_THEMETABLE:
        ref->resolve ( *m_pImpl->GetThemeTable() );
    break;
    case NS_ooxml::LN_settings_settings:
        ref->resolve ( *m_pImpl->GetSettingsTable() );
        m_pImpl->ApplySettingsTable();
    break;
    default:
        OSL_ENSURE( false, "which table is to be filled here?");
    }
    m_pImpl->SetAnyTableImport(false);

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("table");
#endif
}
/*-- 09.06.2006 09:52:16---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::substream(Id rName, ::writerfilter::Reference<Stream>::Pointer_t ref)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("substream");
#endif

    m_pImpl->getTableManager().startLevel();

    //->debug
    //string sName = (*QNameToString::Instance())(rName);
    //--<debug
    //import of page header/footer

    /* WRITERFILTERSTATUS: table: attributedata */
    switch( rName )
    {
    case NS_rtf::LN_headerl:
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */

        m_pImpl->PushPageHeader(SectionPropertyMap::PAGE_LEFT);
        break;
    case NS_rtf::LN_headerr:
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */

        m_pImpl->PushPageHeader(SectionPropertyMap::PAGE_RIGHT);
        break;
    case NS_rtf::LN_headerf:
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */

        m_pImpl->PushPageHeader(SectionPropertyMap::PAGE_FIRST);
        break;
    case NS_rtf::LN_footerl:
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */

        m_pImpl->PushPageFooter(SectionPropertyMap::PAGE_LEFT);
        break;
    case NS_rtf::LN_footerr:
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */

        m_pImpl->PushPageFooter(SectionPropertyMap::PAGE_RIGHT);
        break;
    case NS_rtf::LN_footerf:
        /* WRITERFILTERSTATUS: done: 50, planned: 2, spent: 0 */

        m_pImpl->PushPageFooter(SectionPropertyMap::PAGE_FIRST);
        break;
    case NS_rtf::LN_footnote:
    case NS_rtf::LN_endnote:
        m_pImpl->PushFootOrEndnote( NS_rtf::LN_footnote == rName );
    break;
    case NS_rtf::LN_annotation :
        m_pImpl->PushAnnotation();
    break;
    }
    ref->resolve(*this);
    switch( rName )
    {
    case NS_rtf::LN_headerl:
    case NS_rtf::LN_headerr:
    case NS_rtf::LN_headerf:
    case NS_rtf::LN_footerl:
    case NS_rtf::LN_footerr:
    case NS_rtf::LN_footerf:
        m_pImpl->PopPageHeaderFooter();
    break;
    case NS_rtf::LN_footnote:
    case NS_rtf::LN_endnote:
        m_pImpl->PopFootOrEndnote();
    break;
    case NS_rtf::LN_annotation :
        m_pImpl->PopAnnotation();
    break;
    }

    m_pImpl->getTableManager().endLevel();

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement("substream");
#endif
}
/*-- 09.06.2006 09:52:16---------------------------------------------------

-----------------------------------------------------------------------*/
void DomainMapper::info(const string & /*info_*/)
{
}

void DomainMapper::handleUnderlineType(const sal_Int32 nIntValue, const ::boost::shared_ptr<PropertyMap> pContext)
{
    sal_Int16 eUnderline = awt::FontUnderline::NONE;

    switch(nIntValue)
    {
    case 0: eUnderline = awt::FontUnderline::NONE; break;
    case 2: pContext->Insert(PROP_CHAR_WORD_MODE, true, uno::makeAny( true ) ); // TODO: how to get rid of it?
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
    pContext->Insert(PROP_CHAR_UNDERLINE, true, uno::makeAny( eUnderline ) );
}

void DomainMapper::handleParaJustification(const sal_Int32 nIntValue, const ::boost::shared_ptr<PropertyMap> pContext, const bool bExchangeLeftRight)
{
    sal_Int16 nAdjust = 0;
    sal_Int16 nLastLineAdjust = 0;
    switch(nIntValue)
    {
    case 1:
        nAdjust = style::ParagraphAdjust_CENTER;
        break;
    case 2:
        nAdjust = static_cast< sal_Int16 > (bExchangeLeftRight ? style::ParagraphAdjust_LEFT : style::ParagraphAdjust_RIGHT);
        break;
    case 4:
        nLastLineAdjust = style::ParagraphAdjust_BLOCK;
        //no break;
    case 3:
        nAdjust = style::ParagraphAdjust_BLOCK;
        break;
    case 0:
    default:
        nAdjust = static_cast< sal_Int16 > (bExchangeLeftRight ? style::ParagraphAdjust_RIGHT : style::ParagraphAdjust_LEFT);
        break;
    }
    pContext->Insert( PROP_PARA_ADJUST, true, uno::makeAny( nAdjust ) );
    pContext->Insert( PROP_PARA_LAST_LINE_ADJUST, true, uno::makeAny( nLastLineAdjust ) );
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

rtl::OUString DomainMapper::getBracketStringFromEnum(const sal_Int32 nIntValue, const bool bIsPrefix)
{
    switch(nIntValue)
    {
    case 1:
        if (bIsPrefix)
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "(" ));
        return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( ")" ));

    case 2:
        if (bIsPrefix)
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "[" ));
        return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "]" ));

    case 3:
        if (bIsPrefix)
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "<" ));
        return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( ">" ));

    case 4:
        if (bIsPrefix)
            return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "{" ));
        return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "}" ));

    case 0:
    default:
        return rtl::OUString();
    }
}

void DomainMapper::resolveSprmProps(Sprm & rSprm)
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
    if( pProperties.get())
        pProperties->resolve(*this);
}

void DomainMapper::resolveAttributeProperties(Value & val)
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = val.getProperties();
    if( pProperties.get())
        pProperties->resolve(*this);
}


com::sun::star::style::TabAlign DomainMapper::getTabAlignFromValue(const sal_Int32 nIntValue)
{
    switch (nIntValue)
    {
    case 0:
    case 4: // bar not supported
        return com::sun::star::style::TabAlign_LEFT;
    case 1:
        return com::sun::star::style::TabAlign_CENTER;
    case 2:
        return com::sun::star::style::TabAlign_RIGHT;
    case 3:
        return com::sun::star::style::TabAlign_DECIMAL;
    default:
        return com::sun::star::style::TabAlign_DEFAULT;
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
/*-- 18.07.2007 14:59:00---------------------------------------------------

  -----------------------------------------------------------------------*/
bool DomainMapper::IsOOXMLImport() const
{
    return m_pImpl->IsOOXMLImport();
}
/*-- 18.07.2007 16:03:14---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference < lang::XMultiServiceFactory > DomainMapper::GetTextFactory() const
{
    return m_pImpl->GetTextFactory();
}
/*-- 12.11.2007 10:41:01---------------------------------------------------

  -----------------------------------------------------------------------*/
void  DomainMapper::AddListIDToLFOTable( sal_Int32 nAbstractNumId )
{
    m_pImpl->GetLFOTable()->AddListID( nAbstractNumId );
}
/*-- 31.01.2008 18:19:44---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange > DomainMapper::GetCurrentTextRange()
{
    return m_pImpl->GetTopTextAppend()->getEnd();
}

/*-- 05.02.2008 10:26:26---------------------------------------------------

  -----------------------------------------------------------------------*/
::rtl::OUString DomainMapper::getOrCreateCharStyle( PropertyValueVector_t& rCharProperties )
{
    StyleSheetTablePtr pStyleSheets = m_pImpl->GetStyleSheetTable();
    return pStyleSheets->getOrCreateCharStyle( rCharProperties );
}

} //namespace dmapper
} //namespace writerfilter
