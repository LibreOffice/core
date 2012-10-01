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
#include <DomainMapper_Impl.hxx>
#include <ConversionHelper.hxx>
#include <ModelEventListener.hxx>
#include <MeasureHandler.hxx>
#include <i18npool/mslangid.hxx>
#include <i18nutil/paper.hxx>
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
#include <comphelper/types.hxx>
#include <comphelper/storagehelper.hxx>

#include <CellColorHandler.hxx>
#include <SectionColumnHandler.hxx>
#include <GraphicHelpers.hxx>

using namespace ::com::sun::star;
using namespace ::rtl;

namespace writerfilter {

using resourcemodel::resolveSprmProps;
using resourcemodel::resolveAttributeProperties;

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
                            SourceDocumentType eDocumentType ) :
LoggedProperties(dmapper_logger, "DomainMapper"),
LoggedTable(dmapper_logger, "DomainMapper"),
LoggedStream(dmapper_logger, "DomainMapper"),
    m_pImpl( new DomainMapper_Impl( *this, xContext, xModel, eDocumentType )),
    mnBackgroundColor(0), mbIsHighlightSet(false)
{
    // #i24363# tab stops relative to indent
    m_pImpl->SetDocumentSettingsProperty(
        PropertyNameSupplier::GetPropertyNameSupplier().GetName( PROP_TABS_RELATIVE_TO_INDENT ),
        uno::makeAny( false ) );

    //import document properties
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory(xContext->getServiceManager(), uno::UNO_QUERY_THROW);
        uno::Reference< embed::XStorage > xDocumentStorage =
            (comphelper::OStorageHelper::GetStorageOfFormatFromInputStream(OFOPXML_STORAGE_FORMAT_STRING, xInputStream, xFactory, bRepairStorage ));

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
        if( nIndexes )
        {
            //index update has to wait until first view is created
            uno::Reference< document::XEventBroadcaster > xBroadcaster(xIndexesSupplier, uno::UNO_QUERY);
            xBroadcaster->addEventListener(uno::Reference< document::XEventListener >(new ModelEventListener));
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

    if( nName >= NS_rtf::LN_WIDENT && nName <= NS_rtf::LN_LCBSTTBFUSSR )
        m_pImpl->GetFIB().SetData( nName, nIntValue );
    else
    {


        switch( nName )
        {
            /* attributes to be ignored */
        case NS_rtf::LN_UNUSED1_3:
        case NS_rtf::LN_UNUSED1_7:
        case NS_rtf::LN_UNUSED8_3:
        case NS_rtf::LN_FWRITERESERVATION:
        case NS_rtf::LN_FLOADOVERRIDE:
        case NS_rtf::LN_FFAREAST:
        case NS_rtf::LN_FCRYPTO:
        case NS_rtf::LN_NFIBBACK:
        case NS_rtf::LN_LKEY:
        case NS_rtf::LN_ENVR:
        case NS_rtf::LN_FMAC:
        case NS_rtf::LN_FWORD97SAVED:
        case NS_rtf::LN_FCMAC:
        case NS_rtf::LN_PNFBPCHPFIRST_W6:
        case NS_rtf::LN_PNCHPFIRST_W6:
        case NS_rtf::LN_CPNBTECHP_W6:
        case NS_rtf::LN_PNFBPPAPFIRST_W6:
        case NS_rtf::LN_PNPAPFIRST_W6:
        case NS_rtf::LN_CPNBTEPAP_W6:
        case NS_rtf::LN_PNFBPLVCFIRST_W6:
        case NS_rtf::LN_PNLVCFIRST_W6:
        case NS_rtf::LN_CPNBTELVC_W6:
        case NS_rtf::LN_CBMAC:
        case NS_rtf::LN_LPRODUCTCREATED:
        case NS_rtf::LN_LPRODUCTREVISED:
        case NS_rtf::LN_CCPMCR:
        case NS_rtf::LN_PNFBPCHPFIRST:
        case NS_rtf::LN_PNFBPPAPFIRST:
        case NS_rtf::LN_PNFBPLVCFIRST:
        case NS_rtf::LN_FCISLANDFIRST:
        case NS_rtf::LN_FCISLANDLIM:
        case NS_rtf::LN_FCSTSHFORIG:
        case NS_rtf::LN_LCBSTSHFORIG:
        case NS_rtf::LN_FCPLCFPAD:
        case NS_rtf::LN_LCBPLCFPAD:
        case NS_rtf::LN_FCSTTBFGLSY:
        case NS_rtf::LN_LCBSTTBFGLSY:
        case NS_rtf::LN_FCPLCFGLSY:
        case NS_rtf::LN_LCBPLCFGLSY:
        case NS_rtf::LN_FCPLCFSEA:
        case NS_rtf::LN_LCBPLCFSEA:
        case NS_rtf::LN_FCPLCFFLDMCR:
        case NS_rtf::LN_LCBPLCFFLDMCR:
        case NS_rtf::LN_FCCMDS:
        case NS_rtf::LN_LCBCMDS:
        case NS_rtf::LN_FCPLCMCR:
        case NS_rtf::LN_LCBPLCMCR:
        case NS_rtf::LN_FCSTTBFMCR:
        case NS_rtf::LN_LCBSTTBFMCR:
        case NS_rtf::LN_FCPRDRVR:
        case NS_rtf::LN_LCBPRDRVR:
        case NS_rtf::LN_FCPRENVPORT:
        case NS_rtf::LN_LCBPRENVPORT:
        case NS_rtf::LN_FCPRENVLAND:
        case NS_rtf::LN_LCBPRENVLAND:
        case NS_rtf::LN_FCWSS:
        case NS_rtf::LN_LCBWSS:
        case NS_rtf::LN_FCPLCFPGDFTN:
        case NS_rtf::LN_LCBPLCFPGDFTN:
        case NS_rtf::LN_FCAUTOSAVESOURCE:
        case NS_rtf::LN_LCBAUTOSAVESOURCE:
        case NS_rtf::LN_FCPLCDOAMOM:
        case NS_rtf::LN_LCBPLCDOAMOM:
        case NS_rtf::LN_FCPLCDOAHDR:
        case NS_rtf::LN_LCBPLCDOAHDR:
        case NS_rtf::LN_FCPMS:
        case NS_rtf::LN_LCBPMS:
        case NS_rtf::LN_FCPLCFPGDEDN:
        case NS_rtf::LN_LCBPLCFPGDEDN:
        case NS_rtf::LN_FCPLCFWKB:
        case NS_rtf::LN_LCBPLCFWKB:
        case NS_rtf::LN_FCPLCFSPL:
        case NS_rtf::LN_LCBPLCFSPL:
        case NS_rtf::LN_FCSTWUSER:
        case NS_rtf::LN_LCBSTWUSER:
        case NS_rtf::LN_FCUNUSED:
        case NS_rtf::LN_LCBUNUSED:
        case NS_rtf::LN_FCSTTBFINTLFLD:
        case NS_rtf::LN_LCBSTTBFINTLFLD:
        case NS_rtf::LN_FCROUTESLIP:
        case NS_rtf::LN_LCBROUTESLIP:
        case NS_rtf::LN_FCSTTBSAVEDBY:
        case NS_rtf::LN_LCBSTTBSAVEDBY:
        case NS_rtf::LN_FCSTTBFNM:
        case NS_rtf::LN_LCBSTTBFNM:
        case NS_rtf::LN_FCDOCUNDO:
        case NS_rtf::LN_LCBDOCUNDO:
        case NS_rtf::LN_FCRGBUSE:
        case NS_rtf::LN_LCBRGBUSE:
        case NS_rtf::LN_FCUSP:
        case NS_rtf::LN_LCBUSP:
        case NS_rtf::LN_FCUSKF:
        case NS_rtf::LN_LCBUSKF:
        case NS_rtf::LN_FCPLCUPCRGBUSE:
        case NS_rtf::LN_LCBPLCUPCRGBUSE:
        case NS_rtf::LN_FCPLCUPCUSP:
        case NS_rtf::LN_LCBPLCUPCUSP:
        case NS_rtf::LN_FCPLGOSL:
        case NS_rtf::LN_LCBPLGOSL:
        case NS_rtf::LN_FCPLCOCX:
        case NS_rtf::LN_LCBPLCOCX:
        case NS_rtf::LN_DWLOWDATETIME:
        case NS_rtf::LN_DWHIGHDATETIME:
        case NS_rtf::LN_FCPLCASUMY:
        case NS_rtf::LN_LCBPLCASUMY:
        case NS_rtf::LN_FCPLCFGRAM:
        case NS_rtf::LN_LCBPLCFGRAM:
        case NS_rtf::LN_FCSTTBFUSSR:
            break;

        case NS_rtf::LN_ISTD: //index of applied style
            {
            //search for the style with the given id and apply it
            //as CharStyleName or ParaStyleName
            //if the style is a user defined style then it must have an ISTD - built-in styles might not have it
            StyleSheetTablePtr pStyleSheets = m_pImpl->GetStyleSheetTable();
            OUString sValue = OUString::valueOf(nIntValue, 16);
            const StyleSheetEntryPtr pEntry = pStyleSheets->FindStyleSheetByISTD(sValue);
            if( pEntry.get( ) )
            {
                bool bParaStyle = (pEntry->nStyleTypeCode == STYLE_TYPE_PARA);
                if(bParaStyle)
                    m_pImpl->SetCurrentParaStyleId(OUString::valueOf(static_cast<sal_Int32>(nIntValue), 16));
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
            break;
        case NS_rtf::LN_NFC:
            break;
        case NS_rtf::LN_FLEGAL:
            break;
        case NS_rtf::LN_FNORESTART:
            break;
        case NS_rtf::LN_FIDENTSAV:
            break;
        case NS_rtf::LN_FCONVERTED:
            break;
        case NS_rtf::LN_FTENTATIVE:
            break;
        case NS_rtf::LN_RGBXCHNUMS:
            break;
        case NS_rtf::LN_IXCHFOLLOW:
            break;
        case NS_rtf::LN_DXASPACE:
            break;
        case NS_rtf::LN_DXAINDENT:
            break;
        case NS_rtf::LN_CBGRPPRLCHPX:
            break;
        case NS_rtf::LN_CBGRPPRLPAPX:
            break;
        case NS_rtf::LN_LSID:
            break;
        case NS_rtf::LN_TPLC:
            break;
        case NS_rtf::LN_RGISTD:
            break;
        case NS_rtf::LN_FSIMPLELIST:
            break;
        case NS_rtf::LN_fAutoNum:
            break;
        case NS_rtf::LN_fHybrid:
            break;
        case NS_rtf::LN_ILVL:
            break;
        case NS_rtf::LN_FSTARTAT:
            break;
        case NS_rtf::LN_FFORMATTING:
            break;
        case NS_rtf::LN_UNSIGNED4_6:
            break;
        case NS_rtf::LN_clfolvl:
            break;
        case NS_rtf::LN_CBFFNM1:
            break;
        case NS_rtf::LN_PRQ:
            break;
        case NS_rtf::LN_FTRUETYPE:
            break;
        case NS_rtf::LN_FF:
            break;
        case NS_rtf::LN_WWEIGHT:
            break;
        case NS_rtf::LN_CHS:

            {
                m_pImpl->GetFIB().SetLNCHS( nIntValue );
            }
            break;
        case NS_rtf::LN_IXCHSZALT:
            break;
        case NS_rtf::LN_PANOSE:
            break;
        case NS_rtf::LN_FS:
            break;
        case NS_rtf::LN_STI:
            break;
        case NS_rtf::LN_FSCRATCH:
            break;
        case NS_rtf::LN_FINVALHEIGHT:
            break;
        case NS_rtf::LN_FHASUPE:
            break;
        case NS_rtf::LN_FMASSCOPY:
            break;
        case NS_rtf::LN_SGC:
            break;
        case NS_rtf::LN_ISTDBASE:
            break;
        case NS_rtf::LN_CUPX:
            break;
        case NS_rtf::LN_ISTDNEXT:
            break;
        case NS_rtf::LN_BCHUPE:
            break;
        case NS_rtf::LN_FAUTOREDEF:
            break;
        case NS_rtf::LN_FHIDDEN:
            break;
        case NS_rtf::LN_CSTD:
            break;
        case NS_rtf::LN_CBSTDBASEINFILE:
            break;
        case NS_rtf::LN_FSTDSTYLENAMESWRITTEN:
            break;
        case NS_rtf::LN_UNUSED4_2:
            break;
        case NS_rtf::LN_STIMAXWHENSAVED:
            break;
        case NS_rtf::LN_ISTDMAXFIXEDWHENSAVED:
            break;
        case NS_rtf::LN_NVERBUILTINNAMESWHENSAVED:
            break;
        case NS_rtf::LN_RGFTCSTANDARDCHPSTSH:
            break;
        case NS_rtf::LN_WIDENT:

        case NS_rtf::LN_NFIB:

        case NS_rtf::LN_NPRODUCT:
        case NS_rtf::LN_LID:
        case NS_rtf::LN_PNNEXT:
        case NS_rtf::LN_FDOT:
        case NS_rtf::LN_FGLSY:
        case NS_rtf::LN_FCOMPLEX:
        case NS_rtf::LN_FHASPIC:
        case NS_rtf::LN_CQUICKSAVES:
        case NS_rtf::LN_FENCRYPTED:
        case NS_rtf::LN_FWHICHTBLSTM:
        case NS_rtf::LN_FREADONLYRECOMMENDED:
        case NS_rtf::LN_FEXTCHAR:
        case NS_rtf::LN_FEMPTYSPECIAL:
        case NS_rtf::LN_FLOADOVERRIDEPAGE:
        case NS_rtf::LN_FFUTURESAVEDUNDO:
        case NS_rtf::LN_FSPARE0:
        case NS_rtf::LN_CHSTABLES:
        case NS_rtf::LN_FCMIN:
        case NS_rtf::LN_CSW:
        case NS_rtf::LN_WMAGICCREATED:
        case NS_rtf::LN_WMAGICREVISED:
        case NS_rtf::LN_WMAGICCREATEDPRIVATE:
        case NS_rtf::LN_WMAGICREVISEDPRIVATE:
        case NS_rtf::LN_LIDFE:
        case NS_rtf::LN_CLW:
        case NS_rtf::LN_CCPTEXT:
        case NS_rtf::LN_CCPFTN:
        case NS_rtf::LN_CCPHDD:
        case NS_rtf::LN_CCPATN:
        case NS_rtf::LN_CCPEDN:
        case NS_rtf::LN_CCPTXBX:
        case NS_rtf::LN_CCPHDRTXBX:
        case NS_rtf::LN_PNCHPFIRST:
        case NS_rtf::LN_CPNBTECHP:
        case NS_rtf::LN_PNPAPFIRST:
        case NS_rtf::LN_CPNBTEPAP:
        case NS_rtf::LN_PNLVCFIRST:
        case NS_rtf::LN_CPNBTELVC:
        case NS_rtf::LN_CFCLCB:
        case NS_rtf::LN_FCSTSHF:
        case NS_rtf::LN_LCBSTSHF:
        case NS_rtf::LN_FCPLCFFNDREF:
        case NS_rtf::LN_LCBPLCFFNDREF:
        case NS_rtf::LN_FCPLCFFNDTXT:
        case NS_rtf::LN_LCBPLCFFNDTXT:
        case NS_rtf::LN_FCPLCFANDREF:
        case NS_rtf::LN_LCBPLCFANDREF:
        case NS_rtf::LN_FCPLCFANDTXT:
        case NS_rtf::LN_LCBPLCFANDTXT:
        case NS_rtf::LN_FCPLCFSED:
        case NS_rtf::LN_LCBPLCFSED:
        case NS_rtf::LN_FCPLCFPHE:
        case NS_rtf::LN_LCBPLCFPHE:
        case NS_rtf::LN_FCPLCFHDD:
        case NS_rtf::LN_LCBPLCFHDD:
        case NS_rtf::LN_FCPLCFBTECHPX:
        case NS_rtf::LN_LCBPLCFBTECHPX:
        case NS_rtf::LN_FCPLCFBTEPAPX:
        case NS_rtf::LN_LCBPLCFBTEPAPX:
        case NS_rtf::LN_FCSTTBFFFN:
        case NS_rtf::LN_LCBSTTBFFFN:
        case NS_rtf::LN_FCPLCFFLDMOM:
        case NS_rtf::LN_LCBPLCFFLDMOM:
        case NS_rtf::LN_FCPLCFFLDHDR:
        case NS_rtf::LN_LCBPLCFFLDHDR:
        case NS_rtf::LN_FCPLCFFLDFTN:
        case NS_rtf::LN_LCBPLCFFLDFTN:
        case NS_rtf::LN_FCPLCFFLDATN:
        case NS_rtf::LN_LCBPLCFFLDATN:
        case NS_rtf::LN_FCSTTBFBKMK:
        case NS_rtf::LN_LCBSTTBFBKMK:
        case NS_rtf::LN_FCPLCFBKF:
        case NS_rtf::LN_LCBPLCFBKF:
        case NS_rtf::LN_FCPLCFBKL:
        case NS_rtf::LN_LCBPLCFBKL:
        case NS_rtf::LN_FCDOP:
        case NS_rtf::LN_LCBDOP:
        case NS_rtf::LN_FCSTTBFASSOC:
        case NS_rtf::LN_LCBSTTBFASSOC:
        case NS_rtf::LN_FCCLX:
        case NS_rtf::LN_LCBCLX:
        case NS_rtf::LN_FCGRPXSTATNOWNERS:
        case NS_rtf::LN_LCBGRPXSTATNOWNERS:
        case NS_rtf::LN_FCSTTBFATNBKMK:
        case NS_rtf::LN_LCBSTTBFATNBKMK:
        case NS_rtf::LN_FCPLCSPAMOM:
        case NS_rtf::LN_LCBPLCSPAMOM:
        case NS_rtf::LN_FCPLCSPAHDR:
        case NS_rtf::LN_LCBPLCSPAHDR:
        case NS_rtf::LN_FCPLCFATNBKF:
        case NS_rtf::LN_LCBPLCFATNBKF:
        case NS_rtf::LN_FCPLCFATNBKL:
        case NS_rtf::LN_LCBPLCFATNBKL:
        case NS_rtf::LN_FCFORMFLDSTTBF:
        case NS_rtf::LN_LCBFORMFLDSTTBF:
        case NS_rtf::LN_FCPLCFENDREF:
        case NS_rtf::LN_LCBPLCFENDREF:
        case NS_rtf::LN_FCPLCFENDTXT:
        case NS_rtf::LN_LCBPLCFENDTXT:
        case NS_rtf::LN_FCPLCFFLDEDN:
        case NS_rtf::LN_LCBPLCFFLDEDN:
        case NS_rtf::LN_FCDGGINFO:
        case NS_rtf::LN_LCBDGGINFO:
        case NS_rtf::LN_FCSTTBFRMARK:
        case NS_rtf::LN_LCBSTTBFRMARK:
        case NS_rtf::LN_FCSTTBFCAPTION:
        case NS_rtf::LN_LCBSTTBFCAPTION:
        case NS_rtf::LN_FCSTTBFAUTOCAPTION:
        case NS_rtf::LN_LCBSTTBFAUTOCAPTION:
        case NS_rtf::LN_LCBPLCFTXBXTXT:
        case NS_rtf::LN_FCPLCFFLDTXBX:
        case NS_rtf::LN_LCBPLCFFLDTXBX:
        case NS_rtf::LN_FCPLCFHDRTXBXTXT:
        case NS_rtf::LN_LCBPLCFHDRTXBXTXT:
        case NS_rtf::LN_FCPLCFFLDHDRTXBX:
        case NS_rtf::LN_LCBPLCFFLDHDRTXBX:
        case NS_rtf::LN_FCSTTBTTMBD:
        case NS_rtf::LN_LCBSTTBTTMBD:
        case NS_rtf::LN_FCPGDMOTHER:
        case NS_rtf::LN_LCBPGDMOTHER:
        case NS_rtf::LN_FCBKDMOTHER:
        case NS_rtf::LN_LCBBKDMOTHER:
        case NS_rtf::LN_FCPGDFTN:
        case NS_rtf::LN_LCBPGDFTN:
        case NS_rtf::LN_FCBKDFTN:
        case NS_rtf::LN_LCBBKDFTN:
        case NS_rtf::LN_FCPGDEDN:
        case NS_rtf::LN_LCBPGDEDN:
        case NS_rtf::LN_FCBKDEDN:
        case NS_rtf::LN_LCBBKDEDN:
        case NS_rtf::LN_FCPLCFLST:
        case NS_rtf::LN_LCBPLCFLST:
        case NS_rtf::LN_FCPLFLFO:
        case NS_rtf::LN_LCBPLFLFO:
        case NS_rtf::LN_FCPLCFTXBXBKD:
        case NS_rtf::LN_LCBPLCFTXBXBKD:
        case NS_rtf::LN_FCPLCFTXBXHDRBKD:
        case NS_rtf::LN_LCBPLCFTXBXHDRBKD:
        case NS_rtf::LN_FCSTTBGLSYSTYLE:
        case NS_rtf::LN_LCBSTTBGLSYSTYLE:
        case NS_rtf::LN_FCPLCFBTELVC:
        case NS_rtf::LN_LCBPLCFBTELVC:
        case NS_rtf::LN_FCPLCFLVC:
        case NS_rtf::LN_LCBPLCFLVC:
        case NS_rtf::LN_FCSTTBLISTNAMES:
        case NS_rtf::LN_LCBSTTBLISTNAMES:
        case NS_rtf::LN_LCBSTTBFUSSR:
            {
                m_pImpl->GetFIB().SetData( nName, nIntValue );
            }
            break;
        case NS_rtf::LN_FN:
        case NS_rtf::LN_FCSEPX:
        case NS_rtf::LN_FNMPR:
        case NS_rtf::LN_FCMPR:

            //section descriptor, unused or internally used
            break;
        case NS_rtf::LN_ICOFORE:
            break;
        case NS_rtf::LN_ICOBACK:
            break;
        case NS_rtf::LN_IPAT:
            break;
        case NS_rtf::LN_SHDFORECOLOR:
            break;
        case NS_rtf::LN_SHDBACKCOLOR:
            break;
        case NS_rtf::LN_SHDPATTERN:
            break;
        case NS_rtf::LN_DPTLINEWIDTH:
            break;
        case NS_rtf::LN_BRCTYPE:
            break;
        case NS_rtf::LN_ICO:
            break;
        case NS_rtf::LN_DPTSPACE:
            break;
        case NS_rtf::LN_FSHADOW:
            break;
        case NS_rtf::LN_FFRAME:
            break;
        case NS_rtf::LN_UNUSED2_15:
            break;
        case NS_rtf::LN_FFIRSTMERGED:
            break;
        case NS_rtf::LN_FMERGED:
            break;
        case NS_rtf::LN_FVERTICAL:
            break;
        case NS_rtf::LN_FBACKWARD:
            break;
        case NS_rtf::LN_FROTATEFONT:
            break;
        case NS_rtf::LN_FVERTMERGE:
            break;
        case NS_rtf::LN_FVERTRESTART:
            break;
        case NS_rtf::LN_VERTALIGN:
            break;
        case NS_rtf::LN_FUNUSED:
            break;
        case NS_rtf::LN_BRCTOP:
        case NS_rtf::LN_BRCLEFT:
        case NS_rtf::LN_BRCBOTTOM:
        case NS_rtf::LN_BRCRIGHT:
        {
            table::BorderLine2 aBorderLine;
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
            (void)eBorderId;
            (void)eBorderDistId;
        }
        break;
        case NS_rtf::LN_ITCFIRST:
            break;
        case NS_rtf::LN_FPUB:
            break;
        case NS_rtf::LN_ITCLIM:
            break;
        case NS_rtf::LN_FCOL:
            break;
        case NS_rtf::LN_LINECOLOR:
            break;
        case NS_rtf::LN_LINEWIDTH:
            break;
        case NS_rtf::LN_LINETYPE:
            break;
        case NS_rtf::LN_MM:
            break;
        case NS_rtf::LN_XEXT:
            break;
        case NS_rtf::LN_YEXT:
            break;
        case NS_rtf::LN_HMF:
            break;
        case NS_rtf::LN_LCB:
            break;
        case NS_rtf::LN_CBHEADER:
            break;
        case NS_rtf::LN_MFP:
            break;
        case NS_rtf::LN_BM_RCWINMF:
            break;
        case NS_rtf::LN_DXAGOAL:
            break;
        case NS_rtf::LN_DYAGOAL:
            break;
        case NS_rtf::LN_MX:
            break;
        case NS_rtf::LN_MY:
            break;
        case NS_rtf::LN_DXACROPLEFT:
            break;
        case NS_rtf::LN_DYACROPTOP:
            break;
        case NS_rtf::LN_DXACROPRIGHT:
            break;
        case NS_rtf::LN_DYACROPBOTTOM:
            break;
        case NS_rtf::LN_BRCL:
            break;
        case NS_rtf::LN_FFRAMEEMPTY:
            break;
        case NS_rtf::LN_FBITMAP:
            break;
        case NS_rtf::LN_FDRAWHATCH:
            break;
        case NS_rtf::LN_FERROR:
            break;
        case NS_rtf::LN_BPP:
            break;
        case NS_rtf::LN_DXAORIGIN:
            break;
        case NS_rtf::LN_DYAORIGIN:
            break;
        case NS_rtf::LN_CPROPS:
            break;
        case NS_rtf::LN_LINEPROPSTOP:
            break;
        case NS_rtf::LN_LINEPROPSLEFT:
            break;
        case NS_rtf::LN_LINEPROPSBOTTOM:
            break;
        case NS_rtf::LN_LINEPROPSRIGHT:
            break;
        case NS_rtf::LN_LINEPROPSHORIZONTAL:
            break;
        case NS_rtf::LN_LINEPROPSVERTICAL:
            break;
        case NS_rtf::LN_headerr:
            break;
        case NS_rtf::LN_footerr:
            break;
        case NS_rtf::LN_endnote:
            break;
        case NS_rtf::LN_BOOKMARKNAME:
            // sStringValue contains the bookmark name
            sLocalBookmarkName = sStringValue;
        break;
        case NS_rtf::LN_IBKL:
            //contains the bookmark identifier - has to be added to the bookmark name imported before
            //if it is already available then the bookmark should be inserted
            m_pImpl->AddBookmark( sLocalBookmarkName, sStringValue );
            sLocalBookmarkName = OUString();
        break;
        case NS_rtf::LN_LISTLEVEL:
            break;
        case NS_rtf::LN_LFOData:
            break;
        case NS_rtf::LN_F:
            break;
        case NS_rtf::LN_ALTFONTNAME:
            break;
        case NS_rtf::LN_XSZFFN:
            break;
        case NS_rtf::LN_XSTZNAME:
            break;
        case NS_rtf::LN_XSTZNAME1:
            break;
        case NS_rtf::LN_UPXSTART:
            break;
        case NS_rtf::LN_UPX:
            break;
        case NS_rtf::LN_sed:
            //section properties
            resolveAttributeProperties(*this, val);
            break;
        case NS_rtf::LN_tbdAdd:
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
            //deleted tab
        case NS_rtf::LN_dxaAdd:
            //set tab
        case NS_rtf::LN_TLC:
            //tab leading characters - for decimal tabs
        case NS_rtf::LN_JC:
            //tab justification
            m_pImpl->ModifyCurrentTabStop(nName, nIntValue);
            break;
        case NS_rtf::LN_UNUSED0_6:
            // really unused
            break;
        case NS_rtf::LN_rgbrc:
            break;
        case NS_rtf::LN_shd:
            break;
        case NS_rtf::LN_cellShd:
            break;
        case NS_rtf::LN_cellTopColor:
        case NS_rtf::LN_cellLeftColor:
        case NS_rtf::LN_cellBottomColor:
        case NS_rtf::LN_cellRightColor:
            OSL_FAIL("handled by DomainMapperTableManager");
        break;

        case NS_rtf::LN_LISTTABLE:
            break;
        case NS_rtf::LN_LFOTABLE:
            break;
        case NS_rtf::LN_FONTTABLE:
            break;
        case NS_rtf::LN_STYLESHEET:
            break;

        case NS_rtf::LN_fcEastAsianLayout:
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
            //footnote reference descriptor, if nIntValue > 0 then automatic, custom otherwise
            //ignored
        break;
        case NS_rtf::LN_FONT: //font of footnote symbol
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->SetFootnoteFontId( nIntValue );
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
        case NS_rtf::LN_CHAR: //footnote symbol character
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->SetFootnoteSymbol( sal_Unicode(nIntValue));
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
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME, true, uno::makeAny( sStringValue ));
        break;
        case NS_ooxml::LN_CT_Underline_val:
            handleUnderlineType(nIntValue, m_pImpl->GetTopContext());
            break;
        case NS_ooxml::LN_CT_Color_val:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COLOR, true, uno::makeAny( nIntValue ) );
            break;
        case NS_ooxml::LN_CT_Underline_color:
            if (m_pImpl->GetTopContext())
            {
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_UNDERLINE_HAS_COLOR, true, uno::makeAny( true ) );
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_UNDERLINE_COLOR, true, uno::makeAny( nIntValue ) );
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
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME, true, uno::makeAny( sStringValue ));
            break;
        case NS_ooxml::LN_CT_Fonts_asciiTheme:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME, true, uno::makeAny( m_pImpl->GetThemeTable()->getFontNameForTheme(nIntValue) ));
            break;
        case NS_ooxml::LN_CT_Fonts_hAnsi:
            break;//unsupported
        case NS_ooxml::LN_CT_Fonts_hAnsiTheme:
            break; //unsupported
        case NS_ooxml::LN_CT_Fonts_eastAsia:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME_ASIAN, true, uno::makeAny( sStringValue ));
            break;
        case NS_ooxml::LN_CT_Fonts_eastAsiaTheme:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME_COMPLEX, true, uno::makeAny( m_pImpl->GetThemeTable()->getFontNameForTheme(nIntValue) ) );
            break;
        case NS_ooxml::LN_CT_Fonts_cs:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME_COMPLEX, true, uno::makeAny( sStringValue ));
            break;
        case NS_ooxml::LN_CT_Fonts_cstheme:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_FONT_NAME_COMPLEX, true, uno::makeAny( m_pImpl->GetThemeTable()->getFontNameForTheme(nIntValue) ));
        break;
        case NS_ooxml::LN_CT_Spacing_before:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_PARA_TOP_MARGIN, true, uno::makeAny( ConversionHelper::convertTwipToMM100( nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Spacing_beforeLines:
            break;
        case NS_ooxml::LN_CT_Spacing_after:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_PARA_BOTTOM_MARGIN, true, uno::makeAny( ConversionHelper::convertTwipToMM100( nIntValue ) ));
            break;
        case NS_ooxml::LN_CT_Spacing_afterLines:
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
                aLineSpacingIter = pTopContext->find(PropertyDefinition( PROP_PARA_LINE_SPACING, true ) );
                bFound = aLineSpacingIter != pTopContext->end();
            }
            if (bFound)
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
            if (pTopContext)
                pTopContext->Insert(PROP_PARA_LINE_SPACING, true, uno::makeAny( aSpacing ));
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
                    m_pImpl->GetTopContext()->Insert(PROP_PARA_FIRST_LINE_INDENT, true, uno::makeAny(nFirstLineIndent));

                m_pImpl->GetTopContext()->Insert(
                    PROP_PARA_LEFT_MARGIN, true, uno::makeAny( ConversionHelper::convertTwipToMM100(nIntValue ) ));
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
                    m_pImpl->GetTopContext()->Insert(PROP_PARA_FIRST_LINE_INDENT, true, uno::makeAny(nFirstLineIndent));
                if (nParaLeftMargin != 0)
                    m_pImpl->GetTopContext()->Insert(PROP_PARA_LEFT_MARGIN, true, uno::makeAny(nParaLeftMargin));

                m_pImpl->GetTopContext()->Insert(
                    PROP_PARA_RIGHT_MARGIN, true, uno::makeAny( ConversionHelper::convertTwipToMM100(nIntValue ) ));
            }
            break;
        case NS_ooxml::LN_CT_Ind_hanging:
            if (m_pImpl->GetTopContext())
            {
                sal_Int32 nValue = ConversionHelper::convertTwipToMM100( nIntValue );
                m_pImpl->GetTopContext()->Insert(
                    PROP_PARA_FIRST_LINE_INDENT, true, uno::makeAny( - nValue ));
            }
            break;
        case NS_ooxml::LN_CT_Ind_firstLine:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(
                    PROP_PARA_FIRST_LINE_INDENT, true, uno::makeAny( ConversionHelper::convertTwipToMM100(nIntValue ) ));
            break;

        case NS_ooxml::LN_CT_EastAsianLayout_id:
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_combine:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_IS_ON, true, uno::makeAny ( nIntValue ? true : false ));
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_combineBrackets:
            if (m_pImpl->GetTopContext())
            {
                OUString sCombinePrefix = getBracketStringFromEnum(nIntValue);
                OUString sCombineSuffix = getBracketStringFromEnum(nIntValue, false);
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_PREFIX, true, uno::makeAny ( sCombinePrefix ));
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_COMBINE_SUFFIX, true, uno::makeAny ( sCombineSuffix ));
            }
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_vert:
            if (m_pImpl->GetTopContext())
            {
                sal_Int16 nRotationAngle = (nIntValue ? 900 : 0);
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_ROTATION, true, uno::makeAny ( nRotationAngle ));
            }
            break;
        case NS_ooxml::LN_CT_EastAsianLayout_vertCompress:
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(PROP_CHAR_ROTATION_IS_FIT_TO_LINE, true, uno::makeAny ( nIntValue ? true : false));
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
            LanguageType eLang = MsLangId::convertIsoStringToLanguage( sStringValue );
            lang::Locale aLocale = MsLangId::convertLanguageToLocale( eLang );
            if (m_pImpl->GetTopContext())
                m_pImpl->GetTopContext()->Insert(NS_ooxml::LN_CT_Language_val== nName ? PROP_CHAR_LOCALE :
                             NS_ooxml::LN_CT_Language_eastAsia == nName ? PROP_CHAR_LOCALE_ASIAN : PROP_CHAR_LOCALE_COMPLEX,
                             true,
                             uno::makeAny( aLocale ) );
        }
        break;
// This is the value when the compat option is not enabled. No idea where it comes from, the spec doesn't mention it.
#define AUTO_PARA_SPACING sal_Int32(49)
        case NS_ooxml::LN_CT_Spacing_beforeAutospacing:
            if (!m_pImpl->GetSettingsTable()->GetDoNotUseHTMLParagraphAutoSpacing())
                m_pImpl->GetTopContext()->Insert( PROP_PARA_TOP_MARGIN, false, uno::makeAny( AUTO_PARA_SPACING ) );
            else
                m_pImpl->GetTopContext()->Insert( PROP_PARA_TOP_MARGIN, false, uno::makeAny( ConversionHelper::convertTwipToMM100(100) ) );
        break;
        case NS_ooxml::LN_CT_Spacing_afterAutospacing:
            if (!m_pImpl->GetSettingsTable()->GetDoNotUseHTMLParagraphAutoSpacing())
                m_pImpl->GetTopContext()->Insert( PROP_PARA_BOTTOM_MARGIN, false, uno::makeAny( AUTO_PARA_SPACING ) );
            else
                m_pImpl->GetTopContext()->Insert( PROP_PARA_BOTTOM_MARGIN, false, uno::makeAny( ConversionHelper::convertTwipToMM100(100) ) );
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
        case NS_rtf::LN_blip: // contains the binary graphic
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
        case NS_sprm::LN_PWr:
        case NS_sprm::LN_PDxaWidth:
        case NS_sprm::LN_PWHeightAbs:
        case NS_sprm::LN_PDxaFromText:
        case NS_sprm::LN_PDyaFromText:
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
                            case  NS_ooxml::LN_Value_wordprocessingml_ST_YAlign_inline  ://todo: what to do with inline - no avail. in WW97 and WW2007
                            //no break;
                            default:nIntValue = text::VertOrientation::NONE;
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
                    case NS_sprm::LN_PWr:
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
                        pParaProperties->Setw(ConversionHelper::convertTwipToMM100(nIntValue));
                    break;
                    case NS_sprm::LN_PWHeightAbs:
                        pParaProperties->Seth(ConversionHelper::convertTwipToMM100(nIntValue));
                    break;
                    case NS_sprm::LN_PDxaFromText:
                        pParaProperties->SethSpace( ConversionHelper::convertTwipToMM100(nIntValue ));
                    break;
                    case NS_sprm::LN_PDyaFromText:
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
        case NS_ooxml::LN_CT_LineNumber_distance:
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
            m_pImpl->AddAnnotationPosition(true);
        break;
        case NS_ooxml::LN_EG_RangeMarkupElements_commentRangeEnd:
            m_pImpl->AddAnnotationPosition(false);
        break;
        case NS_ooxml::LN_CT_Comment_initials:
            m_pImpl->SetCurrentRedlineInitials(sStringValue);
        break;
        case NS_ooxml::LN_token:
            m_pImpl->SetCurrentRedlineToken( nIntValue );
        break;
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
                break;
                case NS_ooxml::LN_CT_LineNumber_start:
                    aSettings.nStartValue = nIntValue; // todo: has to be set at (each) first paragraph
                break;
                case NS_ooxml::LN_CT_LineNumber_distance:
                    aSettings.nDistance = ConversionHelper::convertTwipToMM100( nIntValue );
                break;
                case NS_ooxml::LN_CT_LineNumber_restart:
                    //page:empty, probably 0,section:1,continuous:2;
                    aSettings.bRestartAtEachPage = nIntValue < 1;
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
        case NS_ooxml::LN_CT_Color_themeTint:
        case NS_ooxml::LN_CT_Color_themeShade:
            //unsupported
        break;
        case NS_ooxml::LN_endtrackchange:
            m_pImpl->RemoveCurrentRedline( );
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
        break;
        default:
            {
#if OSL_DEBUG_LEVEL > 0
            OString sMessage("DomainMapper::attribute() - Id: ");
            sMessage += OString::valueOf( sal_Int32( nName ), 10 );
            sMessage += " / 0x";
            sMessage += OString::valueOf( sal_Int32( nName ), 16 );
            sMessage += " value: ";
            sMessage += OString::valueOf( sal_Int32( nIntValue ), 10 );
            sMessage += " / 0x";
            sMessage += OString::valueOf( sal_Int32( nIntValue ), 16 );
            SAL_WARN("writerfilter", sMessage.getStr());
#endif
            }
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

void DomainMapper::sprmWithProps( Sprm& rSprm, PropertyMapPtr rContext, SprmType eSprmType )
{
    OSL_ENSURE(rContext.get(), "PropertyMap has to be valid!");
    if(!rContext.get())
        return ;

    sal_uInt32 nSprmId = rSprm.getId();
    //needed for page properties
    SectionPropertyMap * pSectionContext = m_pImpl->GetSectionContext();

    //TODO: In rtl-paragraphs the meaning of left/right are to be exchanged
    bool bExchangeLeftRight = false;
    Value::Pointer_t pValue = rSprm.getValue();
    sal_Int32 nIntValue = pValue->getInt();
    OUString sStringValue = pValue->getString();
    PropertyNameSupplier& rPropNameSupplier = PropertyNameSupplier::GetPropertyNameSupplier();

    switch(nSprmId)
    {
    case 2:  // sprmPIstd
    case 0x4600:
        break;  // sprmPIstd - style code
    case 3: // "sprmPIstdPermute
    case NS_sprm::LN_PIstdPermute:
        break;  // sprmPIstdPermute
    case NS_sprm::LN_PIncLvl:
        break;  // sprmPIncLvl
    case NS_sprm::LN_PJcExtra: // sprmPJc Asian (undocumented)
    case NS_sprm::LN_PJc: // sprmPJc
        handleParaJustification(nIntValue, rContext, bExchangeLeftRight);
        break;
    case NS_sprm::LN_PFSideBySide:
        break;  // sprmPFSideBySide

    case NS_sprm::LN_PFKeep:   // sprmPFKeep
        rContext->Insert(PROP_PARA_SPLIT, true, uno::makeAny(nIntValue ? false : true));
        break;
    case NS_sprm::LN_PFKeepFollow:   // sprmPFKeepFollow
        rContext->Insert(PROP_PARA_KEEP_TOGETHER, true, uno::makeAny( nIntValue ? true : false) );
        break;
    case NS_sprm::LN_PFPageBreakBefore:
        rContext->Insert(PROP_BREAK_TYPE, true, uno::makeAny( com::sun::star::style::BreakType_PAGE_BEFORE ) );
    break;  // sprmPFPageBreakBefore
    case NS_sprm::LN_PBrcl:
        break;  // sprmPBrcl
    case NS_sprm::LN_PBrcp:
        break;  // sprmPBrcp
    case NS_sprm::LN_PIlvl: // sprmPIlvl
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
        {
            //convert the ListTable entry to a NumberingRules propery and apply it
            ListsManager::Pointer pListTable = m_pImpl->GetListTable();
            ListDef::Pointer pList = pListTable->GetList( nIntValue );
            if( m_pImpl->IsStyleSheetImport() )
            {
                //style sheets cannot have a numbering rule attached
                StyleSheetPropertyMap* pStyleSheetPropertyMap = dynamic_cast< StyleSheetPropertyMap* >( rContext.get() );
                pStyleSheetPropertyMap->SetListId( nIntValue );
            }
            if( pList.get( ) )
            {
                if( !m_pImpl->IsStyleSheetImport() )
                {
                    uno::Any aRules = uno::makeAny( pList->GetNumberingRules( ) );
                    rContext->Insert( PROP_NUMBERING_RULES, true, aRules );
                    // erase numbering from pStyle if already set
                    rContext->erase( PropertyDefinition( PROP_NUMBERING_STYLE_NAME, true ));
                }
            }
            else if ( !m_pImpl->IsStyleSheetImport( ) )
                rContext->Insert( PROP_NUMBERING_STYLE_NAME, true, uno::makeAny( OUString() ) );
        }
        break;
    case NS_sprm::LN_PFNoLineNumb:   // sprmPFNoLineNumb
        rContext->Insert(PROP_PARA_LINE_NUMBER_COUNT, true, uno::makeAny( nIntValue ? false : true) );
        break;
    case NS_sprm::LN_PChgTabsPapx:   // sprmPChgTabsPapx
        {
            // Initialize tab stop vector from style sheet
            uno::Any aValue = m_pImpl->GetPropertyFromStyleSheet(PROP_PARA_TAB_STOPS);
            uno::Sequence< style::TabStop > aStyleTabStops;
            if(aValue >>= aStyleTabStops)
            {
                m_pImpl->InitTabStopFromStyle( aStyleTabStops );
            }

            //create a new tab stop property - this is done with the contained properties
            resolveSprmProps(*this, rSprm);
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
        rContext->Insert(
                         eSprmType == SPRM_DEFAULT ? PROP_PARA_FIRST_LINE_INDENT : PROP_FIRST_LINE_OFFSET,
                         true,
                         uno::makeAny( ConversionHelper::convertTwipToMM100(nIntValue ) ));
        break;
    case 20 : // sprmPDyaLine
    case NS_sprm::LN_PDyaLine:   // sprmPDyaLine
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
        rContext->Insert(PROP_PARA_TOP_MARGIN, true, uno::makeAny( ConversionHelper::convertTwipToMM100( nIntValue ) ));
        break;
    case 22 :
    case NS_sprm::LN_PDyaAfter:   // sprmPDyaAfter
        rContext->Insert(PROP_PARA_BOTTOM_MARGIN, true, uno::makeAny( ConversionHelper::convertTwipToMM100( nIntValue ) ));
        break;

    case  23: //sprmPChgTabs
    case NS_sprm::LN_PChgTabs: // sprmPChgTabs
        OSL_FAIL( "unhandled");
        //tabs of list level?
        break;
    case 24: // "sprmPFInTable"
    case NS_sprm::LN_PFInTable:
        break;  // sprmPFInTable
    case NS_sprm::LN_PTableDepth: //sprmPTableDepth
        //not handled via sprm but via text( 0x07 )
    break;
    case 25: // "sprmPTtp" pap.fTtp
    case NS_sprm::LN_PFTtp:   // sprmPFTtp  was: Read_TabRowEnd
        break;
    case 26:  // "sprmPDxaAbs
    case NS_sprm::LN_PDxaAbs:
        break;  // sprmPDxaAbs
    case 27: //sprmPDyaAbs
    case NS_sprm::LN_PDyaAbs:
        break;  // sprmPDyaAbs
    case NS_sprm::LN_PDxaWidth:
        break;  // sprmPDxaWidth
    case NS_sprm::LN_PPc:
        break;  // sprmPPc
    case NS_sprm::LN_PBrcTop10:
        break;  // sprmPBrcTop10
    case NS_sprm::LN_PBrcLeft10:
        break;  // sprmPBrcLeft10
    case NS_sprm::LN_PBrcBottom10:
        break;  // sprmPBrcBottom10
    case NS_sprm::LN_PBrcRight10:
        break;  // sprmPBrcRight10
    case NS_sprm::LN_PBrcBetween10:
        break;  // sprmPBrcBetween10
    case NS_sprm::LN_PBrcBar10:
        break;  // sprmPBrcBar10
    case NS_sprm::LN_PDxaFromText10:
        break;  // sprmPDxaFromText10
    case NS_sprm::LN_PWr:
        break;  // sprmPWr

    case NS_ooxml::LN_CT_PrBase_pBdr: //paragraph border
        resolveSprmProps(*this, rSprm);
    break;
    case NS_sprm::LN_PBrcTop:   // sprmPBrcTop
    case NS_sprm::LN_PBrcLeft:   // sprmPBrcLeft
    case NS_sprm::LN_PBrcBottom:   // sprmPBrcBottom
    case NS_sprm::LN_PBrcRight:   // sprmPBrcRight
    case NS_sprm::LN_PBrcBetween:   // sprmPBrcBetween
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
                        case NS_sprm::LN_PBrcTop:
                            eBorderId = PROP_TOP_BORDER;
                            eBorderDistId = PROP_TOP_BORDER_DISTANCE;
                        break;
                        case NS_sprm::LN_PBrcLeft:
                            eBorderId = PROP_LEFT_BORDER;
                            eBorderDistId = PROP_LEFT_BORDER_DISTANCE;
                        break;
                        case NS_sprm::LN_PBrcBottom:
                            eBorderId = PROP_BOTTOM_BORDER         ;
                            eBorderDistId = PROP_BOTTOM_BORDER_DISTANCE;
                        break;
                        case NS_sprm::LN_PBrcRight:
                            eBorderId = PROP_RIGHT_BORDER;
                            eBorderDistId = PROP_RIGHT_BORDER_DISTANCE ;
                        break;
                        case NS_sprm::LN_PBrcBetween:
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
                table::BorderLine2 aBorderLine;
                sal_Int32 nLineDistance = ConversionHelper::MakeBorderLine( nIntValue, aBorderLine );
                PropertyIds eBorderId = PROP_LEFT_BORDER;
                PropertyIds eBorderDistId = PROP_LEFT_BORDER_DISTANCE  ;
                switch( nSprmId )
                {
                case NS_sprm::LN_PBrcBetween:   // sprmPBrcBetween
                    OSL_FAIL( "TODO: inner border is not handled");
                    break;
                case NS_sprm::LN_PBrcLeft:   // sprmPBrcLeft
                    eBorderId = PROP_LEFT_BORDER;
                    eBorderDistId = PROP_LEFT_BORDER_DISTANCE  ;
                    break;
                case NS_sprm::LN_PBrcRight:   // sprmPBrcRight
                    eBorderId = PROP_RIGHT_BORDER          ;
                    eBorderDistId = PROP_RIGHT_BORDER_DISTANCE ;
                    break;
                case NS_sprm::LN_PBrcTop:   // sprmPBrcTop
                    eBorderId = PROP_TOP_BORDER            ;
                    eBorderDistId = PROP_TOP_BORDER_DISTANCE;
                    break;
                case NS_sprm::LN_PBrcBottom:   // sprmPBrcBottom
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
    case NS_sprm::LN_PBorderLeft:
    case NS_sprm::LN_PBorderBottom:
    case NS_sprm::LN_PBorderRight:
        OSL_FAIL( "TODO: border color definition");
        break;
    case NS_sprm::LN_PBrcBar:
        break;  // sprmPBrcBar
    case NS_sprm::LN_PFNoAutoHyph:   // sprmPFNoAutoHyph
        rContext->Insert(PROP_PARA_IS_HYPHENATION, true, uno::makeAny( nIntValue ? false : true ));
        break;
    case NS_sprm::LN_PWHeightAbs:
        break;  // sprmPWHeightAbs
    case NS_sprm::LN_PDcs:
        break;  // sprmPDcs

    case NS_sprm::LN_PShd: // sprmPShd
    {
        //contains fore color, back color and shadow percentage, results in a brush
        writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
        if( pProperties.get())
        {
            CellColorHandlerPtr pCellColorHandler( new CellColorHandler );
            pCellColorHandler->setOutputFormat( CellColorHandler::Paragraph );
            pProperties->resolve(*pCellColorHandler);
            rContext->insert( pCellColorHandler->getProperties(), true );
        }
    }
    break;
    case NS_sprm::LN_PDyaFromText:
        break;  // sprmPDyaFromText
    case NS_sprm::LN_PDxaFromText:
        break;  // sprmPDxaFromText
    case NS_sprm::LN_PFLocked:
        break;  // sprmPFLocked
    case NS_sprm::LN_PFWidowControl:
    case NS_ooxml::LN_CT_PPrBase_widowControl:
    {
        uno::Any aVal( uno::makeAny( sal_Int8(nIntValue ? 2 : 0 )));
        rContext->Insert( PROP_PARA_WIDOWS, true, aVal );
        rContext->Insert( PROP_PARA_ORPHANS, true, aVal );
    }
    break;  // sprmPFWidowControl
    case NS_sprm::LN_PRuler:
        break;  // sprmPRuler
    case NS_sprm::LN_PFKinsoku:
        break;  // sprmPFKinsoku
    case NS_sprm::LN_PFWordWrap:
        break;  // sprmPFWordWrap
    case NS_sprm::LN_PFOverflowPunct: ;  // sprmPFOverflowPunct - hanging punctuation
        rContext->Insert(PROP_PARA_IS_HANGING_PUNCTUATION, true, uno::makeAny( nIntValue ? false : true ));
        break;
    case NS_sprm::LN_PFTopLinePunct:
        break;  // sprmPFTopLinePunct
    case NS_sprm::LN_PFAutoSpaceDE:
        break;  // sprmPFAutoSpaceDE
    case NS_sprm::LN_PFAutoSpaceDN:
        break;  // sprmPFAutoSpaceDN
    case NS_sprm::LN_PWAlignFont:
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
            rContext->Insert( PROP_PARA_VERT_ALIGNMENT, true, uno::makeAny( nAlignment) );
        }
        break;  // sprmPWAlignFont
    case NS_sprm::LN_PFrameTextFlow:
        break;  // sprmPFrameTextFlow
    case NS_sprm::LN_PISnapBaseLine:
        break;  // sprmPISnapBaseLine
    case NS_sprm::LN_PAnld:
        break;  // sprmPAnld
    case NS_sprm::LN_PPropRMark:
        break;  // sprmPPropRMark
    case NS_sprm::LN_POutLvl:
        {
            if( m_pImpl->IsStyleSheetImport() )
            {
                sal_Int16 nLvl = static_cast< sal_Int16 >( nIntValue );

                StyleSheetPropertyMap* pStyleSheetPropertyMap = dynamic_cast< StyleSheetPropertyMap* >( rContext.get() );
                pStyleSheetPropertyMap->SetOutlineLevel( nLvl );
            }
        }
        break;  // sprmPOutLvl
    case NS_sprm::LN_PFBiDi:
        {
            if (nIntValue != 0)
            {
                rContext->Insert(PROP_WRITING_MODE, false, uno::makeAny( text::WritingMode2::RL_TB ));
                rContext->Insert(PROP_PARA_ADJUST, false, uno::makeAny( style::ParagraphAdjust_RIGHT ));
            }
        }

        break;  // sprmPFBiDi
    case NS_ooxml::LN_EG_SectPrContents_bidi:
        if (pSectionContext != NULL)
            pSectionContext->Insert(PROP_WRITING_MODE,false, uno::makeAny( text::WritingMode2::RL_TB));
        break;
    case NS_sprm::LN_PFNumRMIns:
        break;  // sprmPFNumRMIns
    case NS_sprm::LN_PCrLf:
        break;  // sprmPCrLf
    case NS_sprm::LN_PNumRM:
        break;  // sprmPNumRM
    case NS_sprm::LN_PHugePapx:
        break;  // sprmPHugePapx
    case NS_sprm::LN_PFUsePgsuSettings:
        break;  // sprmPFUsePgsuSettings
    case NS_sprm::LN_PFAdjustRight:
        break;  // sprmPFAdjustRight
    case NS_sprm::LN_CFRMarkDel:
        break;  // sprmCFRMarkDel
    case NS_sprm::LN_CFRMark:
        break;  // sprmCFRMark
    case NS_sprm::LN_CFFldVanish:
        break;  // sprmCFFldVanish
    case NS_sprm::LN_CFSpec:   // sprmCFSpec
        break;
    case NS_sprm::LN_CPicLocation:   // sprmCPicLocation
            //is being resolved on the tokenizer side
        break;
    case NS_sprm::LN_CIbstRMark:
        break;  // sprmCIbstRMark
    case NS_sprm::LN_CDttmRMark:
        break;  // sprmCDttmRMark
    case NS_sprm::LN_CFData:
        break;  // sprmCFData
    case NS_sprm::LN_CIdslRMark:
        break;  // sprmCIdslRMark
    case NS_sprm::LN_CChs:
        break;  // sprmCChs
    case NS_sprm::LN_CSymbol: // sprmCSymbol
        resolveSprmProps(*this, rSprm); //resolves LN_FONT and LN_CHAR
    break;
    case NS_sprm::LN_CFOle2:
        break;  // sprmCFOle2
    case NS_sprm::LN_CIdCharType:
        break;  // sprmCIdCharType
    case NS_sprm::LN_CHighlight:
        {
            sal_Int32 nColor = 0;
            if(true ==( mbIsHighlightSet = getColorFromIndex(nIntValue, nColor)))
                rContext->Insert(PROP_CHAR_BACK_COLOR, true, uno::makeAny( nColor ));
            else if (mnBackgroundColor)
                rContext->Insert(PROP_CHAR_BACK_COLOR, true, uno::makeAny( mnBackgroundColor ));
        }
        break;  // sprmCHighlight
    case NS_sprm::LN_CObjLocation:
        break;  // sprmCObjLocation
    case NS_sprm::LN_CFFtcAsciSymb:
        break;  // sprmCFFtcAsciSymb
    case NS_sprm::LN_CIstd:
        break;  // sprmCIstd
    case NS_sprm::LN_CIstdPermute:
        break;  // sprmCIstdPermute
    case NS_sprm::LN_CDefault:
        break;  // sprmCDefault
    case NS_sprm::LN_CPlain:
        break;  // sprmCPlain
    case NS_sprm::LN_CKcd:
        rContext->Insert(PROP_CHAR_EMPHASIS, true, uno::makeAny ( getEmphasisValue (nIntValue)));
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

                        rContext->Insert(ePropertyId, true, aBold );
                        if( nSprmId != NS_sprm::LN_CFBoldBi ) // sprmCFBoldBi
                            rContext->Insert(PROP_CHAR_WEIGHT_ASIAN, true, aBold );

                        uno::Reference<beans::XPropertySet> xCharStyle(m_pImpl->GetCurrentNumberingCharStyle());
                        if (xCharStyle.is())
                            xCharStyle->setPropertyValue(rPropNameSupplier.GetName(PROP_CHAR_WEIGHT), aBold);
                    }
                    break;
                    case 61: /*sprmCFItalic*/
                    case NS_sprm::LN_CFItalic: /*sprmCFItalic*/
                    case NS_sprm::LN_CFItalicBi: // sprmCFItalicBi
                    {
                        uno::Any aPosture( uno::makeAny( nIntValue ? awt::FontSlant_ITALIC : awt::FontSlant_NONE ) );
                        rContext->Insert( ePropertyId, true, aPosture );
                        if( nSprmId != NS_sprm::LN_CFItalicBi ) // sprmCFItalicBi
                            rContext->Insert(PROP_CHAR_POSTURE_ASIAN, true, aPosture );
                    }
                    break;
                    case NS_sprm::LN_CFStrike: /*sprmCFStrike*/
                        rContext->Insert(ePropertyId, true,
                                         uno::makeAny( nIntValue ? awt::FontStrikeout::SINGLE : awt::FontStrikeout::NONE ) );
                    break;
                    case NS_sprm::LN_CFDStrike : /*sprmCFDStrike double strike through*/
                        rContext->Insert(ePropertyId, true,
                                         uno::makeAny( nIntValue ? awt::FontStrikeout::DOUBLE : awt::FontStrikeout::NONE ) );
                    break;
                    case NS_sprm::LN_CFOutline: /*sprmCFOutline*/
                    case NS_sprm::LN_CFShadow: /*sprmCFShadow*/
                    case NS_sprm::LN_CFVanish: /*sprmCFVanish*/
                        rContext->Insert(ePropertyId, true, uno::makeAny( nIntValue ? true : false ));
                    break;
                    case NS_sprm::LN_CFSmallCaps: /*sprmCFSmallCaps*/
                        rContext->Insert(ePropertyId, true,
                                         uno::makeAny( nIntValue ? style::CaseMap::SMALLCAPS : style::CaseMap::NONE));
                    break;
                    case NS_sprm::LN_CFCaps: /*sprmCFCaps*/
                        rContext->Insert(ePropertyId, true,
                                         uno::makeAny( nIntValue ? style::CaseMap::UPPERCASE : style::CaseMap::NONE));
                    break;
                    case NS_sprm::LN_CFEmboss: /*sprmCFEmboss*/
                        rContext->Insert(ePropertyId, true,
                                         uno::makeAny( nIntValue ? awt::FontRelief::EMBOSSED : awt::FontRelief::NONE ));
                    break;

                }
            }
        }
        break;
    case NS_sprm::LN_CFtcDefault:
        break;  // sprmCFtcDefault
    case NS_sprm::LN_CKul: // sprmCKul
        {
            // Parameter:  0 = none,    1 = single,  2 = by Word,
            // 3 = double,  4 = dotted,  5 = hidden
            // 6 = thick,   7 = dash,    8 = dot(not used)
            // 9 = dotdash 10 = dotdotdash 11 = wave
            handleUnderlineType(nIntValue, rContext);
        }
        break;
    case NS_sprm::LN_CSizePos:
        break;  // sprmCSizePos
    case NS_sprm::LN_CLid:
        break;  // sprmCLid
    case NS_sprm::LN_CIco:
        {
            sal_Int32 nColor = 0;
            if (getColorFromIndex(nIntValue, nColor))
                rContext->Insert(PROP_CHAR_COLOR, true, uno::makeAny( nColor ) );
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
                rContext->Insert( PROP_CHAR_HEIGHT_COMPLEX, true, aVal );
                // Also set Western, but don't overwrite it.
                rContext->Insert( PROP_CHAR_HEIGHT, true, aVal, false );
            }
            else
            {
                //Asian get the same value as Western
                rContext->Insert( PROP_CHAR_HEIGHT, true, aVal );
                rContext->Insert( PROP_CHAR_HEIGHT_ASIAN, true, aVal );

                uno::Reference<beans::XPropertySet> xCharStyle(m_pImpl->GetCurrentNumberingCharStyle());
                if (xCharStyle.is())
                    xCharStyle->setPropertyValue(rPropNameSupplier.GetName(PROP_CHAR_HEIGHT), aVal);
            }
            // Make sure char sizes defined in the stylesheets don't affect char props from direct formatting.
            if (!m_pImpl->IsStyleSheetImport())
                m_pImpl->deferCharacterProperty( nSprmId, uno::makeAny( nIntValue ));
        }
        break;
    case NS_sprm::LN_CHpsInc:
        break;  // sprmCHpsInc
    case NS_sprm::LN_CHpsPos:
        m_pImpl->deferCharacterProperty( nSprmId, uno::makeAny( nIntValue ));
        break;  // sprmCHpsPos
    case NS_sprm::LN_CHpsPosAdj:
        break;  // sprmCHpsPosAdj
    case NS_sprm::LN_CMajority:
        break;  // sprmCMajority
    case NS_sprm::LN_CIss:   // sprmCIss
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
        break;  // sprmCHpsNew50
    case NS_sprm::LN_CHpsInc1:
        break;  // sprmCHpsInc1
    case 71 : //"sprmCDxaSpace"
    case 96 : //"sprmCDxaSpace"
    case NS_sprm::LN_CDxaSpace:  // sprmCDxaSpace
        //Kerning half point values
        //TODO: there are two kerning values -
        // in ww8par6.cxx NS_sprm::LN_CHpsKern is used as boolean AutoKerning
        rContext->Insert(PROP_CHAR_CHAR_KERNING, true, uno::makeAny( sal_Int16(ConversionHelper::convertTwipToMM100(sal_Int16(nIntValue))) ) );
        break;
    case NS_sprm::LN_CHpsKern:  // sprmCHpsKern    auto kerning is bound to a minimum font size in Word - but not in Writer :-(
        rContext->Insert(PROP_CHAR_AUTO_KERNING, true, uno::makeAny( sal_Bool(nIntValue) ) );
        break;
    case NS_sprm::LN_CMajority50:
        break;  // sprmCMajority50
    case NS_sprm::LN_CHpsMul:
        break;  // sprmCHpsMul
    case NS_sprm::LN_CYsri:
        break;  // sprmCYsri
    case NS_sprm::LN_CRgFtc0:  // sprmCRgFtc0     //ascii font index
    case NS_sprm::LN_CRgFtc1:  // sprmCRgFtc1     //Asian font index
    case NS_sprm::LN_CRgFtc2:  // sprmCRgFtc2     //CTL font index
    case NS_sprm::LN_CFtcBi: // sprmCFtcBi      //font index of a CTL font
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
                (void)eFontFamily;
                (void)eFontStyle;
                const FontEntry::Pointer_t pFontEntry(pFontTable->getFontEntry(sal_uInt32(nIntValue)));
                rContext->Insert(eFontName, true, uno::makeAny( pFontEntry->sFontName  ));
                rContext->Insert(eFontCharSet, true, uno::makeAny( (sal_Int16)pFontEntry->nTextEncoding  ));
                rContext->Insert(eFontPitch, true, uno::makeAny( pFontEntry->nPitchRequest  ));
            }
        }
        break;
    case NS_sprm::LN_CCharScale:  // sprmCCharScale
        rContext->Insert(PROP_CHAR_SCALE_WIDTH, true,
                         uno::makeAny( sal_Int16(nIntValue) ));
        break;
    case NS_sprm::LN_CFImprint: // sprmCFImprint   1 or 0
        // FontRelief: NONE, EMBOSSED, ENGRAVED
        rContext->Insert(PROP_CHAR_RELIEF, true,
                         uno::makeAny( nIntValue ? awt::FontRelief::ENGRAVED : awt::FontRelief::NONE ));
        break;
    case NS_sprm::LN_CFObj:
        break;  // sprmCFObj
    case NS_sprm::LN_CPropRMark:
        break;  // sprmCPropRMark
    case NS_sprm::LN_CSfxText:
        // The file-format has many character animations. We have only
        // one, so we use it always. Suboptimal solution though.
        if (nIntValue)
            rContext->Insert(PROP_CHAR_FLASH, true, uno::makeAny( true ));
        else
            rContext->Insert(PROP_CHAR_FLASH, true, uno::makeAny( false ));
        break;  // sprmCSfxText
    case NS_sprm::LN_CFBiDi:
        break;  // sprmCFBiDi
    case NS_sprm::LN_CFDiacColor:
        break;  // sprmCFDiacColor
    case NS_sprm::LN_CIcoBi:
        break;  // sprmCIcoBi
    case NS_sprm::LN_CDispFldRMark:
        break;  // sprmCDispFldRMark
    case NS_sprm::LN_CIbstRMarkDel:
        break;  // sprmCIbstRMarkDel
    case NS_sprm::LN_CDttmRMarkDel:
        break;  // sprmCDttmRMarkDel
    case NS_sprm::LN_CBrc:
        break;  // sprmCBrc
    case NS_sprm::LN_CShd:
        {
            //contains fore color, back color and shadow percentage, results in a brush
            writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
            if( pProperties.get())
            {
                CellColorHandlerPtr pCellColorHandler( new CellColorHandler );
                pCellColorHandler->setOutputFormat( CellColorHandler::Character );
                pProperties->resolve(*pCellColorHandler);
                rContext->insert( pCellColorHandler->getProperties(), true );
            }
            break;
        }
    case NS_sprm::LN_CIdslRMarkDel:
        break;  // sprmCIdslRMarkDel
    case NS_sprm::LN_CFUsePgsuSettings:
        break;  // sprmCFUsePgsuSettings
    case NS_sprm::LN_CCpg:
        break;  // sprmCCpg
    case NS_sprm::LN_CLidBi:     // sprmCLidBi     language complex
    case NS_sprm::LN_CRgLid0_80: // sprmCRgLid0_80 older language Western
    case NS_sprm::LN_CRgLid0:    // sprmCRgLid0    language Western
    case NS_sprm::LN_CRgLid1:    // sprmCRgLid1    language Asian
    case NS_sprm::LN_CRgLid1_80: // sprmCRgLid1_80 older language Asian
        {
            lang::Locale aLocale;
            MsLangId::convertLanguageToLocale( (LanguageType)nIntValue, aLocale );

            PropertyIds aPropId;
            switch (nSprmId)
            {
                case NS_sprm::LN_CRgLid0:
                case NS_sprm::LN_CRgLid0_80:
                    aPropId = PROP_CHAR_LOCALE;
                    break;
                case NS_sprm::LN_CRgLid1:
                case NS_sprm::LN_CRgLid1_80:
                    aPropId = PROP_CHAR_LOCALE_ASIAN;
                    break;
                default:
                    aPropId = PROP_CHAR_LOCALE_COMPLEX;
                    break;
            }

            rContext->Insert(aPropId, true, uno::makeAny( aLocale ) );
        }
        break;

    case NS_sprm::LN_CIdctHint:   // sprmCIdctHint
        //list table - text offset???
        break;
    case NS_sprm::LN_PicBrcl:
        break;  // sprmPicBrcl
    case NS_sprm::LN_PicScale:
        break;  // sprmPicScale
    case NS_sprm::LN_PicBrcTop:
        break;  // sprmPicBrcTop
    case NS_sprm::LN_PicBrcLeft:
        break;  // sprmPicBrcLeft
    case NS_sprm::LN_PicBrcBottom:
        break;  // sprmPicBrcBoConversionHelper::convertTwipToMM100ttom
    case NS_sprm::LN_PicBrcRight:
        break;  // sprmPicBrcRight
    case NS_sprm::LN_ScnsPgn:
        break;  // sprmScnsPgn
    case NS_sprm::LN_SiHeadingPgn:
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetEvenlySpaced( nIntValue > 0 );

        break;  // sprmSiHeadingPgn
    case NS_sprm::LN_SOlstAnm:
        break;  // sprmSOlstAnm
    case 136:
    case NS_sprm::LN_SDxaColWidth: // sprmSDxaColWidth
        // contains the twip width of the column as 3-byte-code
        // the lowet byte contains the index
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->AppendColumnWidth( ConversionHelper::convertTwipToMM100( (nIntValue & 0xffff00) >> 8 ));
        break;
    case NS_sprm::LN_SDxaColSpacing: // sprmSDxaColSpacing
        // the lowet byte contains the index
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->AppendColumnSpacing( ConversionHelper::convertTwipToMM100( (nIntValue & 0xffff00) >> 8 ));
        break;
    case 138:
    case NS_sprm::LN_SFEvenlySpaced:
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetEvenlySpaced( nIntValue > 0 );
        break;  // sprmSFEvenlySpaced
    case NS_sprm::LN_SFProtected: // sprmSFProtected
        //todo: missing feature - unlocked sections in protected documents
        break;
    case NS_sprm::LN_SDmBinFirst: // sprmSDmBinFirst
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetFirstPaperBin(nIntValue);
        break;
    case NS_sprm::LN_SDmBinOther: // sprmSDmBinOther
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetPaperBin( nIntValue );
        break;
    case NS_sprm::LN_SBkc: // sprmSBkc
        /* break type
          0 - No break
          1 - New Colunn
          2 - New page
          3 - Even page
          4 - odd page
        */
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            // Ignore continous section break at the end of the document
            // It makes the importer loose margin settings with no benefit
            if (m_pImpl->GetParaSectpr() || nIntValue != 0)
                pSectionContext->SetBreakType( nIntValue );
        }
        break;
    case 143:
    case NS_sprm::LN_SFTitlePage: // sprmSFTitlePage
    case NS_ooxml::LN_EG_SectPrContents_titlePg:
    {
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetTitlePage( nIntValue > 0 ? true : false );//section has title page
    }
    break;
    case 144:
    case NS_sprm::LN_SCcolumns: // sprmSCcolumns
        //no of columns - 1
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetColumnCount( (sal_Int16) nIntValue );
    break;
    case 145:
    case NS_sprm::LN_SDxaColumns:           // sprmSDxaColumns
        //column distance - default 708 twip
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetColumnDistance( ConversionHelper::convertTwipToMM100( nIntValue ) );
        break;
    case NS_sprm::LN_SFAutoPgn:
        break;  // sprmSFAutoPgn
    case 147:
    case NS_sprm::LN_SNfcPgn: // sprmSNfcPgn
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
        break;  // sprmSDyaPgn
    case NS_sprm::LN_SDxaPgn:
        break;  // sprmSDxaPgn
    case 150:
    case NS_sprm::LN_SFPgnRestart: // sprmSFPgnRestart
    {
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetPageNoRestart( nIntValue > 0 );
    }
    break;
    case NS_sprm::LN_SFEndnote:
        break;  // sprmSFEndnote
    case 154:
    case NS_sprm::LN_SNLnnMod:// sprmSNLnnMod
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if( pSectionContext )
            pSectionContext->SetLnnMod( nIntValue );
    break;
    case 155:
    case NS_sprm::LN_SDxaLnn: // sprmSDxaLnn
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if( pSectionContext )
            pSectionContext->SetdxaLnn( nIntValue );
    break;
    case 152:
    case NS_sprm::LN_SLnc:// sprmSLnc
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if( pSectionContext )
            pSectionContext->SetLnc( nIntValue );
    break;
    case 160:
    case NS_sprm::LN_SLnnMin: // sprmSLnnMin
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if( pSectionContext )
            pSectionContext->SetLnnMin( nIntValue );
    break;

    case NS_sprm::LN_SGprfIhdt:
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        //flags about header/footer sharing and footnotes?
        /* ww8scan.hxx:
         * WW8_HEADER_EVEN = 0x01, WW8_HEADER_ODD = 0x02, WW8_FOOTER_EVEN = 0x04,
         * WW8_FOOTER_ODD = 0x08, WW8_HEADER_FIRST = 0x10, WW8_FOOTER_FIRST = 0x20
         */

    break;  // sprmSGprfIhdt
    case NS_sprm::LN_SDyaHdrTop: // sprmSDyaHdrTop
        // default 720 twip
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetHeaderTop( ConversionHelper::convertTwipToMM100( nIntValue ));
    break;
    case NS_sprm::LN_SDyaHdrBottom: // sprmSDyaHdrBottom
        // default 720 twip
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetHeaderBottom( ConversionHelper::convertTwipToMM100( nIntValue ) );
    break;
    case 158:
    case NS_sprm::LN_SLBetween: // sprmSLBetween
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetSeparatorLine( nIntValue > 0 );
    break;
    case NS_sprm::LN_SVjc:
        break;  // sprmSVjc
    case 161:
    case NS_sprm::LN_SPgnStart: // sprmSPgnStart
        //page number
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetPageNumber( nIntValue );
    break;
    case 162:
    case NS_sprm::LN_SBOrientation:
        //todo: the old filter assumed that a value of 2 points to double-pages layout
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetLandscape( nIntValue > 0 );
        rContext->Insert( PROP_IS_LANDSCAPE , false, uno::makeAny( nIntValue > 0 ));
    break;  // sprmSBOrientation
    case NS_sprm::LN_SBCustomize:
        break;  // sprmSBCustomize
    case 165:
    case NS_sprm::LN_SYaPage: // sprmSYaPage
    {
        //page height, rounded to default values, default: 0x3dc0 twip
        sal_Int32 nHeight = ConversionHelper::convertTwipToMM100( nIntValue );
        rContext->Insert( PROP_HEIGHT, false, uno::makeAny( PaperInfo::sloppyFitPageDimension( nHeight ) ) );
    }
    break;
    case NS_sprm::LN_SXaPage:   // sprmSXaPage
    {
        //page width, rounded to default values, default 0x2fd0 twip
        sal_Int32 nWidth = ConversionHelper::convertTwipToMM100( nIntValue );
        rContext->Insert( PROP_WIDTH, false, uno::makeAny( PaperInfo::sloppyFitPageDimension( nWidth ) ) );
    }
    break;
    case 166:
    case NS_sprm::LN_SDxaLeft:  // sprmSDxaLeft
    {
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
        // gutter is added to one of the margins of a section depending on RTL, can be placed on top either
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            pSectionContext->SetDzaGutter( ConversionHelper::convertTwipToMM100( nIntValue  ) );
        }
    }
    break;
    case NS_sprm::LN_SDmPaperReq:   // sprmSDmPaperReq
        //paper code - no handled in old filter
        break;
    case NS_sprm::LN_SPropRMark:
        break;  // sprmSPropRMark
    case NS_sprm::LN_SFBiDi:// sprmSFBiDi
    {
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetSFBiDi( nIntValue > 0 );
    }
    break;
    case NS_sprm::LN_SFFacingCol:
        break;  // sprmSFFacingCol
    case NS_sprm::LN_SFRTLGutter: // sprmSFRTLGutter
    {
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
            pSectionContext->SetGutterRTL( nIntValue > 0 );
    }
    break;
    case NS_sprm::LN_SBrcTop:   // sprmSBrcTop
    case NS_sprm::LN_SBrcLeft:   // sprmSBrcLeft
    case NS_sprm::LN_SBrcBottom:  // sprmSBrcBottom
    case NS_sprm::LN_SBrcRight:  // sprmSBrcRight
        {
            table::BorderLine2 aBorderLine;
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
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            pSectionContext->SetDxtCharSpace( nIntValue );
        }
    }
    break;  // sprmSDxtCharSpace
    case NS_sprm::LN_SDyaLinePitch:   // sprmSDyaLinePitch
    {
        //see SwWW8ImplReader::SetDocumentGrid
        OSL_ENSURE(pSectionContext, "SectionContext unavailable!");
        if(pSectionContext)
        {
            pSectionContext->SetGridLinePitch( ConversionHelper::convertTwipToMM100( nIntValue ) );
        }
    }
    break;
    case 0x703a: //undocumented, grid related?
        OSL_FAIL( "TODO: not handled yet"); //nIntValue like 0x008a2373 ?
        break;
    case NS_sprm::LN_SClm:
    {
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

        pTargetContext->Insert(PROP_WRITING_MODE, false, uno::makeAny( nDirection ) );
    }
    break;  // sprmSTextFlow
    case NS_sprm::LN_TJc: // sprmTJc
    case NS_sprm::LN_TDxaLeft:
    case NS_sprm::LN_TDxaGapHalf:
    case NS_sprm::LN_TFCantSplit:
    case NS_sprm::LN_TTableHeader:
    case NS_sprm::LN_TTableBorders: // sprmTTableBorders
    {
        OSL_FAIL( "table propeties should be handled by the table manager");
    }
    break;
    case NS_sprm::LN_TDefTable10:
        break;  // sprmTDefTable10
    case NS_sprm::LN_TDyaRowHeight:
        break;  // sprmTDyaRowHeight
    case NS_sprm::LN_TDefTable:
        break;  // sprmTDefTable
    case NS_sprm::LN_TDefTableShd:
        break;  // sprmTDefTableShd
    case NS_sprm::LN_TTlp:
        break;  // sprmTTlp
    case NS_sprm::LN_TFBiDi:
        break;  // sprmTFBiDi
    case NS_sprm::LN_THTMLProps:
        break;  // sprmTHTMLProps
    case NS_sprm::LN_TSetBrc:
        break;  // sprmTSetBrc
    case NS_sprm::LN_TInsert:
        break;  // sprmTInsert
    case NS_sprm::LN_TDelete:
        break;  // sprmTDelete
    case NS_sprm::LN_TDxaCol:
        break;  // sprmTDxaCol
    case NS_sprm::LN_TMerge:
        break;  // sprmTMerge
    case NS_sprm::LN_TSplit:
        break;  // sprmTSplit
    case NS_sprm::LN_TSetBrc10:
        break;  // sprmTSetBrc10
    case 164: // sprmTSetShd
    case NS_sprm::LN_TSetShd:
        break;  // sprmTSetShd
    case NS_sprm::LN_TSetShdOdd:
        break;  // sprmTSetShdOdd
    case NS_sprm::LN_TTextFlow:
        break;  // sprmTTextFlow
    case NS_sprm::LN_TDiagLine:
        break;  // sprmTDiagLine
    case NS_sprm::LN_TVertMerge:
        break;  // sprmTVertMerge
    case NS_sprm::LN_TVertAlign:
        break;  // sprmTVertAlign
        // the following are not part of the official documentation
    case 0x6870: //TxtForeColor
        {
            //contains a color as 0xTTRRGGBB while SO uses 0xTTRRGGBB
            sal_Int32 nColor = ConversionHelper::ConvertColor(nIntValue);
            rContext->Insert(PROP_CHAR_COLOR, true, uno::makeAny( nColor ) );
        }
        break;
    case 0x6877: //underlining color
        {
            rContext->Insert(PROP_CHAR_UNDERLINE_HAS_COLOR, true, uno::makeAny( true ) );
            rContext->Insert(PROP_CHAR_UNDERLINE_COLOR, true, uno::makeAny( nIntValue ) );
        }
        break;
    case 0x6815:
        break; //undocumented
    case NS_sprm::LN_CIndrsid:
        break; //undocumented
    case 0x6467:
        break; //undocumented
    case 0xF617:
        break; //undocumented
    case 0xd634: // sprmTNewSpacing - table spacing ( see WW8TabBandDesc::ProcessSpacing() )
        break;
    case NS_sprm::LN_TTRLeft:
        break; //undocumented
    case 0x4888:
    case 0x6887:
        //properties of list levels - undocumented
        break;
    case 0xd234:
    case 0xd235:
    case 0xd236:
    case 0xd237:
        break;//undocumented section properties
    case NS_sprm::LN_CEastAsianLayout:
        resolveSprmProps(*this, rSprm);
        break;
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
        rContext->Insert(PROP_PARA_TAB_STOPS, true, uno::makeAny( m_pImpl->GetCurrentTabStopAndClear()));
    }
    break;

    case NS_ooxml::LN_CT_DocDefaults_pPrDefault:
    case NS_ooxml::LN_CT_DocDefaults_rPrDefault:
        GetStyleSheetTable()->sprm( rSprm );
    break;
    case NS_ooxml::LN_CT_PPr_sectPr:
    case NS_ooxml::LN_EG_RPrBase_color:
    case NS_ooxml::LN_EG_RPrBase_rFonts:
    case NS_ooxml::LN_EG_RPrBase_bdr:
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
        if (nSprmId == NS_ooxml::LN_CT_PPr_sectPr)
            m_pImpl->SetParaSectpr(true);
        resolveSprmProps(*this, rSprm);
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
    // Avoid frames if we're inside a structured document tag, would just cause outher tables fail to create.
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
            pSectionContext->Insert( PROP_HEIGHT, false, uno::makeAny( CT_PageSz.h ) );
            pSectionContext->Insert( PROP_IS_LANDSCAPE, false, uno::makeAny( CT_PageSz.orient ));
            pSectionContext->Insert( PROP_WIDTH, false, uno::makeAny( CT_PageSz.w ) );
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

    case NS_ooxml::LN_CT_PPrBase_pStyle:
    {
        m_pImpl->SetCurrentParaStyleId( sStringValue );
        StyleSheetTablePtr pStyleTable = m_pImpl->GetStyleSheetTable();
        const OUString sConvertedStyleName = pStyleTable->ConvertStyleName( sStringValue, true );
        if (m_pImpl->GetTopContext() && m_pImpl->GetTopContextType() != CONTEXT_SECTION)
            m_pImpl->GetTopContext()->Insert( PROP_PARA_STYLE_NAME, true, uno::makeAny( sConvertedStyleName ));
        //apply numbering to paragraph if it was set at the style, but only if the paragraph itself
        //does not specify the numbering
        if( rContext->find( PropertyDefinition( PROP_NUMBERING_RULES, true )) == rContext->end()) // !contains
        {
            const StyleSheetEntryPtr pEntry = pStyleTable->FindStyleSheetByISTD(sStringValue);
            OSL_ENSURE( pEntry.get(), "no style sheet found" );
            const StyleSheetPropertyMap* pStyleSheetProperties = dynamic_cast<const StyleSheetPropertyMap*>(pEntry ? pEntry->pProperties.get() : 0);

            if( pStyleSheetProperties && pStyleSheetProperties->GetListId() >= 0 )
            {
                rContext->Insert( PROP_NUMBERING_STYLE_NAME, true, uno::makeAny(
                            ListDef::GetStyleName( pStyleSheetProperties->GetListId( ) ) ), false);

                // We're inheriting properties from a numbering style. Make sure a possible right margin is inherited from the base style.
                sal_Int32 nParaRightMargin = 0;
                if (!pEntry->sBaseStyleIdentifier.isEmpty())
                {
                    const StyleSheetEntryPtr pParent = pStyleTable->FindStyleSheetByISTD(pEntry->sBaseStyleIdentifier);
                    const StyleSheetPropertyMap* pParentProperties = dynamic_cast<const StyleSheetPropertyMap*>(pParent ? pParent->pProperties.get() : 0);
                    if (pParentProperties->find( PropertyDefinition( PROP_PARA_RIGHT_MARGIN, true )) != pParentProperties->end())
                        nParaRightMargin = pParentProperties->find( PropertyDefinition( PROP_PARA_RIGHT_MARGIN, true ))->second.get<sal_Int32>();
                }
                if (nParaRightMargin != 0)
                {
                    // If we're setting the right margin, we should set the first / left margin as well from the numbering style.
                    sal_Int32 nFirstLineIndent = lcl_getCurrentNumberingProperty(m_pImpl->GetCurrentNumberingRules(), pStyleSheetProperties->GetListLevel(), "FirstLineIndent");
                    sal_Int32 nParaLeftMargin = lcl_getCurrentNumberingProperty(m_pImpl->GetCurrentNumberingRules(), pStyleSheetProperties->GetListLevel(), "IndentAt");
                    if (nFirstLineIndent != 0)
                        rContext->Insert(PROP_PARA_FIRST_LINE_INDENT, true, uno::makeAny(nFirstLineIndent));
                    if (nParaLeftMargin != 0)
                        rContext->Insert(PROP_PARA_LEFT_MARGIN, true, uno::makeAny(nParaLeftMargin));

                    rContext->Insert(PROP_PARA_RIGHT_MARGIN, true, uno::makeAny(nParaRightMargin));
                }
            }

            if( pStyleSheetProperties && pStyleSheetProperties->GetListLevel() >= 0 )
                rContext->Insert( PROP_NUMBERING_LEVEL, true, uno::makeAny(pStyleSheetProperties->GetListLevel()), false);
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
                m_pImpl->GetTopContext()->Insert( PROP_CHAR_STYLE_NAME, true, uno::makeAny( sConvertedName ) );
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
            rContext->Insert( eId, false, uno::makeAny(nMeasureValue), false);
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

        rContext->Insert(PROP_CHAR_ESCAPEMENT,         true, uno::makeAny( nEscapement ) );
        rContext->Insert(PROP_CHAR_ESCAPEMENT_HEIGHT,  true, uno::makeAny( nProp ) );
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
        m_pImpl->StartParaChange( );
    case NS_ooxml::LN_trackchange:
    case NS_ooxml::LN_EG_RPrContent_rPrChange:
    {
        m_pImpl->AddNewRedline( );
        resolveSprmProps(*this, rSprm );
        // now the properties author, date and id should be available
        sal_Int32 nToken = m_pImpl->GetCurrentRedlineToken();
        switch( nToken & 0xffff )
        {
            case ooxml::OOXML_mod :
            case ooxml::OOXML_ins :
            case ooxml::OOXML_del : break;
            default: OSL_FAIL( "redline token other than mod, ins or del" );
        }
        m_pImpl->EndParaChange( );
    }
    break;
    case NS_ooxml::LN_CT_RPrChange_rPr:
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
    case NS_sprm::LN_PContextualSpacing:
        rContext->Insert(PROP_PARA_CONTEXT_MARGIN, true, uno::makeAny( sal_Bool( nIntValue ) ));
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
                PropertyMap::iterator aDefaultFont = pDefaultCharProps->find(PropertyDefinition( PROP_CHAR_HEIGHT, false ));
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
                    aDefaultFont->second >>= fHeight;
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
            rContext->Insert(PROP_CHAR_ESCAPEMENT,         true, uno::makeAny( nEscapement ) );
            rContext->Insert(PROP_CHAR_ESCAPEMENT_HEIGHT,  true, uno::makeAny( nProp ) );
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
    m_pImpl->PushProperties(CONTEXT_SECTION);
}

void DomainMapper::lcl_endSectionGroup()
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

void DomainMapper::lcl_startParagraphGroup()
{
    m_pImpl->getTableManager().startParagraphGroup();
    m_pImpl->PushProperties(CONTEXT_PARAGRAPH);
    static OUString sDefault("Standard");
    if (m_pImpl->GetTopContext())
    {
        if (!m_pImpl->IsInShape())
        {
            m_pImpl->GetTopContext()->Insert( PROP_PARA_STYLE_NAME, true, uno::makeAny( sDefault ) );
            m_pImpl->SetCurrentParaStyleId(sDefault);
        }
        if (m_pImpl->isBreakDeferred(PAGE_BREAK))
               m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, true, uno::makeAny( com::sun::star::style::BreakType_PAGE_BEFORE) );
        else if (m_pImpl->isBreakDeferred(COLUMN_BREAK))
            m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, true, uno::makeAny( com::sun::star::style::BreakType_COLUMN_BEFORE) );
    }
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
    m_pImpl->PushShapeContext( xShape );
    lcl_startParagraphGroup();
}

void DomainMapper::lcl_endShape( )
{
    lcl_endParagraphGroup();
    m_pImpl->PopShapeContext( );
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
    DomainMapperTableManager& rTableManager = m_pImpl->getTableManager();
    if( !rTableManager.getTableStyleName().isEmpty() )
    {
        PropertyMapPtr pTopContext = m_pImpl->GetTopContext();
        rTableManager.CopyTextProperties(pTopContext, m_pImpl->GetStyleSheetTable());
    }
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
            if (pContext == NULL)
                pContext.reset(new PropertyMap());

            m_pImpl->appendTextPortion( sText, pContext );
        }
    }
    catch( const uno::RuntimeException& e )
    {
        SAL_WARN("writerfilter", "failed. Message :" << e.Message);
    }
}

void DomainMapper::lcl_utext(const sal_uInt8 * data_, size_t len)
{
    OUString sText;
    OUStringBuffer aBuffer = OUStringBuffer(len);
    aBuffer.append( (const sal_Unicode *) data_, len);
    sText = aBuffer.makeStringAndClear();

    try
    {
        m_pImpl->getTableManager().utext(data_, len);

        if(len == 1 && (sText[0] == 0x0d || sText[0] == 0x07))
        {
            PropertyMapPtr pContext = m_pImpl->GetTopContextOfType(CONTEXT_PARAGRAPH);
            if (pContext && m_pImpl->GetSettingsTable()->GetSplitPgBreakAndParaMark())
            {
                if (m_pImpl->isBreakDeferred(PAGE_BREAK))
                    pContext->Insert(PROP_BREAK_TYPE, true, uno::makeAny( com::sun::star::style::BreakType_PAGE_BEFORE));
                else if (m_pImpl->isBreakDeferred(COLUMN_BREAK))
                    pContext->Insert(PROP_BREAK_TYPE, true, uno::makeAny( com::sun::star::style::BreakType_COLUMN_BEFORE));
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
                    m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, true, uno::makeAny( com::sun::star::style::BreakType_PAGE_BEFORE) );
                else if (m_pImpl->isBreakDeferred(COLUMN_BREAK))
                    m_pImpl->GetTopContext()->Insert( PROP_BREAK_TYPE, true, uno::makeAny( com::sun::star::style::BreakType_COLUMN_BEFORE) );
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
                if (pContext == NULL)
                    pContext.reset(new PropertyMap());

                m_pImpl->appendTextPortion( sText, pContext );
            }

        }
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
    case NS_rtf::LN_FONTTABLE:

        // create a font table object that listens to the attributes
        // each entry call inserts a new font entry
        ref->resolve( *m_pImpl->GetFontTable() );
        break;
    case NS_rtf::LN_STYLESHEET:
        //same as above to import style sheets
        m_pImpl->SetStyleSheetImport( true );
        ref->resolve( *m_pImpl->GetStyleSheetTable() );
        m_pImpl->GetStyleSheetTable()->ApplyStyleSheets(m_pImpl->GetFontTable());
        m_pImpl->SetStyleSheetImport( false );
        break;
    case NS_ooxml::LN_NUMBERING:
    case NS_rtf::LN_LISTTABLE:
        {

            //the same for list tables
            ref->resolve( *m_pImpl->GetListTable() );
            m_pImpl->GetListTable( )->CreateNumberingRules( );
        }
        break;
    case NS_rtf::LN_LFOTABLE:
        {
            m_pImpl->GetListTable( )->SetLFOImport( true );
            ref->resolve( *m_pImpl->GetListTable() );
            m_pImpl->GetListTable( )->CreateNumberingRules( );
            m_pImpl->GetListTable( )->SetLFOImport( false );
        }
        break;
    case NS_ooxml::LN_THEMETABLE:
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
    case NS_rtf::LN_headerl:

        m_pImpl->PushPageHeader(SectionPropertyMap::PAGE_LEFT);
        break;
    case NS_rtf::LN_headerr:

        m_pImpl->PushPageHeader(SectionPropertyMap::PAGE_RIGHT);
        break;
    case NS_rtf::LN_headerf:

        m_pImpl->PushPageHeader(SectionPropertyMap::PAGE_FIRST);
        break;
    case NS_rtf::LN_footerl:

        m_pImpl->PushPageFooter(SectionPropertyMap::PAGE_LEFT);
        break;
    case NS_rtf::LN_footerr:

        m_pImpl->PushPageFooter(SectionPropertyMap::PAGE_RIGHT);
        break;
    case NS_rtf::LN_footerf:

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

bool DomainMapper::IsInHeaderFooter() const
{
    return m_pImpl->IsInHeaderFooter();
}

} //namespace dmapper
} //namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
