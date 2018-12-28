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

#include <document.hxx>
#include <docuno.hxx>
#include <sheetdata.hxx>

#include "xmlbodyi.hxx"
#include "xmltabi.hxx"
#include "xmlnexpi.hxx"
#include "xmldrani.hxx"
#include "xmlimprt.hxx"
#include "xmldpimp.hxx"
#include "xmlcvali.hxx"
#include "xmllabri.hxx"
#include "xmlmappingi.hxx"
#include "XMLConsolidationContext.hxx"
#include "XMLDDELinksContext.hxx"
#include "XMLCalculationSettingsContext.hxx"
#include "XMLTrackedChangesContext.hxx"
#include "XMLChangeTrackingImportHelper.hxx"
#include "XMLEmptyContext.hxx"
#include "XMLDetectiveContext.hxx"
#include <scerrors.hxx>
#include <tabprotection.hxx>
#include "datastreamimport.hxx"
#include <sax/fastattribs.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>

#include <comphelper/base64.hxx>
#include <sal/types.h>
#include <sal/log.hxx>

#include <memory>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLBodyContext::ScXMLBodyContext( ScXMLImport& rImport,
                                    const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList ) :
    ScXMLImportContext( rImport ),
    sPassword(),
    meHash1(PASSHASH_SHA1),
    meHash2(PASSHASH_UNSPECIFIED),
    bProtected(false),
    bHadCalculationSettings(false),
    pChangeTrackingImportHelper(nullptr)
{
    ScDocument* pDoc = GetScImport().GetDocument();
    if (pDoc)
    {
        // ODF 1.1 and earlier => GRAM_PODF; ODF 1.2 and later => GRAM_ODFF;
        // no version => earlier than 1.2 => GRAM_PODF.
        formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_ODFF;
        const OUString& aVer( rImport.GetODFVersion());
        sal_Int32 nLen = aVer.getLength();
        SAL_INFO("sc.filter", "ScXMLBodyContext ODFVersion: nLen: " << nLen << " str : " << aVer);
        if (!nLen)
            eGrammar = formula::FormulaGrammar::GRAM_PODF;
        else
        {
            // In case there was a micro version, e.g. "1.2.3", this would
            // still yield major.minor, but pParsedEnd (5th parameter, not
            // passed here) would point before string end upon return.
            double fVer = ::rtl::math::stringToDouble( aVer, '.', 0 );
            if (fVer < 1.2)
                eGrammar = formula::FormulaGrammar::GRAM_PODF;
        }
        pDoc->SetStorageGrammar( eGrammar);
    }

    if ( !rAttrList.is() )
        return;

    for (auto &it : *rAttrList)
    {
        sal_Int32 nToken = it.getToken();
        if( NAMESPACE_TOKEN( XML_NAMESPACE_TABLE ) == ( nToken & NMSP_MASK ) )
        {
            const sal_Int32 nLocalToken = nToken & TOKEN_MASK;
            if( nLocalToken == XML_STRUCTURE_PROTECTED )
                bProtected = IsXMLToken( it, XML_TRUE );
            else if ( nLocalToken == XML_PROTECTION_KEY )
                sPassword = it.toString();
            else if (  nLocalToken == XML_PROTECTION_KEY_DIGEST_ALGORITHM )
                meHash1 = ScPassHashHelper::getHashTypeFromURI( it.toString() );
            else if (  nLocalToken == XML_PROTECTION_KEY_DIGEST_ALGORITHM_2 )
                meHash2 = ScPassHashHelper::getHashTypeFromURI( it.toString() );
        }
        else if ( nToken == XML_ELEMENT( LO_EXT, XML_PROTECTION_KEY_DIGEST_ALGORITHM_2 ) )
        {
            meHash2 = ScPassHashHelper::getHashTypeFromURI( it.toString() );
        }
    }
}

ScXMLBodyContext::~ScXMLBodyContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL
        ScXMLBodyContext::createFastChildContext( sal_Int32 nElement,
        const uno::Reference< xml::sax::XFastAttributeList > & xAttrList )
{
    ScSheetSaveData* pSheetData = ScModelObj::getImplementation(GetScImport().GetModel())->GetSheetSaveData();
    if ( pSheetData && pSheetData->HasStartPos() )
    {
        // stream part to copy ends before the next child element
        sal_Int32 nEndOffset = GetScImport().GetByteOffset();
        pSheetData->EndStreamPos( nEndOffset );
    }

    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch( nElement )
    {
    case XML_ELEMENT( TABLE, XML_TRACKED_CHANGES ):
        pChangeTrackingImportHelper = GetScImport().GetChangeTrackingImportHelper();
        if (pChangeTrackingImportHelper)
            pContext = new ScXMLTrackedChangesContext( GetScImport(), pAttribList, pChangeTrackingImportHelper);
        break;
    case XML_ELEMENT( TABLE, XML_CALCULATION_SETTINGS ):
        pContext = new ScXMLCalculationSettingsContext( GetScImport(), pAttribList );
        bHadCalculationSettings = true;
        break;
    case XML_ELEMENT( TABLE, XML_CONTENT_VALIDATIONS ):
        pContext = new ScXMLContentValidationsContext( GetScImport() );
        break;
    case XML_ELEMENT( TABLE, XML_LABEL_RANGES ):
        pContext = new ScXMLLabelRangesContext( GetScImport() );
        break;
    case XML_ELEMENT( TABLE, XML_TABLE ):
        if (GetScImport().GetTables().GetCurrentSheet() >= MAXTAB)
        {
            GetScImport().SetRangeOverflowType(SCWARN_IMPORT_SHEET_OVERFLOW);
            pContext = new ScXMLEmptyContext(GetScImport() );
        }
        else
        {
            pContext = new ScXMLTableContext( GetScImport(), pAttribList );
        }
        break;
    case XML_ELEMENT( TABLE, XML_NAMED_EXPRESSIONS ):
        pContext = new ScXMLNamedExpressionsContext (
            GetScImport(),
            new ScXMLNamedExpressionsContext::GlobalInserter(GetScImport()) );
        break;
    case XML_ELEMENT( TABLE, XML_DATABASE_RANGES ):
        pContext = new ScXMLDatabaseRangesContext ( GetScImport() );
        break;
    case XML_ELEMENT( CALC_EXT, XML_DATA_MAPPINGS ):
        pContext = new ScXMLMappingsContext(GetScImport());
        break;
    case XML_ELEMENT( TABLE, XML_DATABASE_RANGE ):
        pContext = new ScXMLDatabaseRangeContext ( GetScImport(),
                                                        pAttribList );
        break;
    case XML_ELEMENT( TABLE, XML_DATA_PILOT_TABLES ):
        pContext = new ScXMLDataPilotTablesContext ( GetScImport() );
        break;
    case XML_ELEMENT( TABLE, XML_CONSOLIDATION ):
        pContext = new ScXMLConsolidationContext ( GetScImport(), pAttribList );
        break;
    case XML_ELEMENT( TABLE, XML_DDE_LINKS ):
        pContext = new ScXMLDDELinksContext ( GetScImport() );
        break;
    case XML_ELEMENT( CALC_EXT, XML_DATA_STREAM_SOURCE ):
        pContext = new ScXMLDataStreamContext(GetScImport(), pAttribList);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

void SAL_CALL ScXMLBodyContext::characters(const OUString &)
{
    ScSheetSaveData* pSheetData = ScModelObj::getImplementation(GetScImport().GetModel())->GetSheetSaveData();
    if ( pSheetData && pSheetData->HasStartPos() )
    {
        // stream part to copy ends before any content (whitespace) within the spreadsheet element
        sal_Int32 nEndOffset = GetScImport().GetByteOffset();
        pSheetData->EndStreamPos( nEndOffset );
    }
    // otherwise ignore
}

void SAL_CALL ScXMLBodyContext::endFastElement(sal_Int32 nElement)
{
    ScSheetSaveData* pSheetData = ScModelObj::getImplementation(GetScImport().GetModel())->GetSheetSaveData();
    if ( pSheetData && pSheetData->HasStartPos() )
    {
        // stream part to copy ends before the closing tag of spreadsheet element
        sal_Int32 nEndOffset = GetScImport().GetByteOffset();
        pSheetData->EndStreamPos( nEndOffset );
    }

    if ( pSheetData )
    {
        // store the loaded namespaces (for the office:spreadsheet element),
        // so the prefixes in copied stream fragments remain valid
        const SvXMLNamespaceMap& rNamespaces = GetImport().GetNamespaceMap();
        pSheetData->StoreLoadedNamespaces( rNamespaces );
    }

    if (!bHadCalculationSettings)
    {
        // #111055#; set calculation settings defaults if there is no calculation settings element
        rtl::Reference<ScXMLCalculationSettingsContext> pContext( new ScXMLCalculationSettingsContext(GetScImport(), nullptr) );
        pContext->endFastElement( nElement );
    }

    ScXMLImport::MutexGuard aGuard(GetScImport());

    ScMyImpDetectiveOpArray*    pDetOpArray = GetScImport().GetDetectiveOpArray();
    ScDocument*                 pDoc        = GetScImport().GetDocument();
    ScMyImpDetectiveOp          aDetOp;

    if (pDoc && GetScImport().GetModel().is())
    {
        if (pDetOpArray)
        {
            pDetOpArray->Sort();
            while( pDetOpArray->GetFirstOp( aDetOp ) )
            {
                ScDetOpData aOpData( aDetOp.aPosition, aDetOp.eOpType );
                pDoc->AddDetectiveOperation( aOpData );
            }
        }

        if (pChangeTrackingImportHelper)
            pChangeTrackingImportHelper->CreateChangeTrack(GetScImport().GetDocument());

        // #i37959# handle document protection after the sheet settings
        if (bProtected)
        {
            std::unique_ptr<ScDocProtection> pProtection(new ScDocProtection);
            pProtection->setProtected(true);

            uno::Sequence<sal_Int8> aPass;
            if (!sPassword.isEmpty())
            {
                ::comphelper::Base64::decode(aPass, sPassword);
                pProtection->setPasswordHash(aPass, meHash1, meHash2);
            }

            pDoc->SetDocProtection(pProtection.get());
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
