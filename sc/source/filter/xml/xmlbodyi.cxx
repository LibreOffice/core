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

#include <cstdio>

#include "document.hxx"
#include "docuno.hxx"
#include "sheetdata.hxx"

#include "xmlbodyi.hxx"
#include "xmltabi.hxx"
#include "xmlnexpi.hxx"
#include "xmldrani.hxx"
#include "xmlimprt.hxx"
#include "xmldpimp.hxx"
#include "xmlcvali.hxx"
#include "xmlstyli.hxx"
#include "xmllabri.hxx"
#include "XMLConsolidationContext.hxx"
#include "XMLDDELinksContext.hxx"
#include "XMLCalculationSettingsContext.hxx"
#include "XMLTrackedChangesContext.hxx"
#include "XMLEmptyContext.hxx"
#include "XMLDetectiveContext.hxx"
#include "scerrors.hxx"
#include "tabprotection.hxx"
#include "datastreamimport.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>

#include <sax/tools/converter.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <sal/types.h>

#include <memory>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLBodyContext::ScXMLBodyContext( ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
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
        OUString aVer( rImport.GetODFVersion());
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

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_STRUCTURE_PROTECTED))
                bProtected = IsXMLToken(sValue, XML_TRUE);
            else if (IsXMLToken(aLocalName, XML_PROTECTION_KEY))
                sPassword = sValue;
            else if (IsXMLToken(aLocalName, XML_PROTECTION_KEY_DIGEST_ALGORITHM))
                meHash1 = ScPassHashHelper::getHashTypeFromURI(sValue);
            else if (IsXMLToken(aLocalName, XML_PROTECTION_KEY_DIGEST_ALGORITHM_2))
                meHash2 = ScPassHashHelper::getHashTypeFromURI(sValue);
        }
        else if(nPrefix == XML_NAMESPACE_LO_EXT)
        {
            if (IsXMLToken(aLocalName, XML_PROTECTION_KEY_DIGEST_ALGORITHM_2))
                meHash2 = ScPassHashHelper::getHashTypeFromURI(sValue);
        }
    }
}

ScXMLBodyContext::~ScXMLBodyContext()
{
}

SvXMLImportContext *ScXMLBodyContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const OUString& rLocalName,
                                     const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList )
{
    ScSheetSaveData* pSheetData = ScModelObj::getImplementation(GetScImport().GetModel())->GetSheetSaveData();
    if ( pSheetData && pSheetData->HasStartPos() )
    {
        // stream part to copy ends before the next child element
        sal_Int32 nEndOffset = GetScImport().GetByteOffset();
        pSheetData->EndStreamPos( nEndOffset );
    }

    SvXMLImportContext *pContext = nullptr;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetBodyElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_BODY_TRACKED_CHANGES :
        pChangeTrackingImportHelper = GetScImport().GetChangeTrackingImportHelper();
        if (pChangeTrackingImportHelper)
            pContext = new ScXMLTrackedChangesContext( GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
        break;
    case XML_TOK_BODY_CALCULATION_SETTINGS :
        pContext = new ScXMLCalculationSettingsContext( GetScImport(), nPrefix, rLocalName, xAttrList );
        bHadCalculationSettings = true;
        break;
    case XML_TOK_BODY_CONTENT_VALIDATIONS :
        pContext = new ScXMLContentValidationsContext( GetScImport(), nPrefix, rLocalName, xAttrList );
        break;
    case XML_TOK_BODY_LABEL_RANGES:
        pContext = new ScXMLLabelRangesContext( GetScImport(), nPrefix, rLocalName, xAttrList );
        break;
    case XML_TOK_BODY_TABLE:
        if (GetScImport().GetTables().GetCurrentSheet() >= MAXTAB)
        {
            GetScImport().SetRangeOverflowType(SCWARN_IMPORT_SHEET_OVERFLOW);
            pContext = new ScXMLEmptyContext(GetScImport(), nPrefix, rLocalName);
        }
        else
        {
            pContext = new ScXMLTableContext( GetScImport(),nPrefix, rLocalName,
                                              xAttrList );
        }
        break;
    case XML_TOK_BODY_NAMED_EXPRESSIONS:
        pContext = new ScXMLNamedExpressionsContext (
            GetScImport(), nPrefix, rLocalName, xAttrList,
            new ScXMLNamedExpressionsContext::GlobalInserter(GetScImport()) );
        break;
    case XML_TOK_BODY_DATABASE_RANGES:
        pContext = new ScXMLDatabaseRangesContext ( GetScImport(), nPrefix, rLocalName,
                                                        xAttrList );
        break;
    case XML_TOK_BODY_DATABASE_RANGE:
        pContext = new ScXMLDatabaseRangeContext ( GetScImport(), nPrefix, rLocalName,
                                                        xAttrList );
        break;
    case XML_TOK_BODY_DATA_PILOT_TABLES:
        pContext = new ScXMLDataPilotTablesContext ( GetScImport(), nPrefix, rLocalName,
                                                        xAttrList );
        break;
    case XML_TOK_BODY_CONSOLIDATION:
        pContext = new ScXMLConsolidationContext ( GetScImport(), nPrefix, rLocalName,
                                                        xAttrList );
        break;
    case XML_TOK_BODY_DDE_LINKS:
        pContext = new ScXMLDDELinksContext ( GetScImport(), nPrefix, rLocalName,
                                                        xAttrList );
        break;
    case XML_TOK_BODY_DATA_STREAM_SOURCE:
        pContext = new ScXMLDataStreamContext(GetScImport(), nPrefix, rLocalName, xAttrList);
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

void ScXMLBodyContext::Characters( const OUString& )
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

void ScXMLBodyContext::EndElement()
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
        ScXMLCalculationSettingsContext aContext( GetScImport(), XML_NAMESPACE_TABLE, GetXMLToken(XML_CALCULATION_SETTINGS), nullptr );
        aContext.EndElement();
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
                ::sax::Converter::decodeBase64(aPass, sPassword);
                pProtection->setPasswordHash(aPass, meHash1, meHash2);
            }

            pDoc->SetDocProtection(pProtection.get());
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
