/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


// INCLUDE ---------------------------------------------------------------
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
#include "scerrors.hxx"
#include "tabprotection.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <sal/types.h>
#include <tools/debug.hxx>

#include <memory>

using rtl::OUString;

using namespace com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLBodyContext::ScXMLBodyContext( ScXMLImport& rImport,
                                              sal_uInt16 nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sPassword(),
    bProtected(sal_False),
    bHadCalculationSettings(sal_False),
    pChangeTrackingImportHelper(NULL)
{
    ScDocument* pDoc = GetScImport().GetDocument();
    if (pDoc)
    {
        // ODF 1.1 and earlier => GRAM_PODF; ODF 1.2 and later => GRAM_ODFF;
        // no version => earlier than 1.2 => GRAM_PODF.
        formula::FormulaGrammar::Grammar eGrammar = formula::FormulaGrammar::GRAM_ODFF;
        OUString aVer( rImport.GetODFVersion());
        sal_Int32 nLen = aVer.getLength();
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "\n ScXMLBodyContext ODFVersion: nLen: %d, str: %s\n",
                (int)nLen, OUStringToOString( aVer, RTL_TEXTENCODING_UTF8).getStr());
#endif
        if (!nLen)
            eGrammar = formula::FormulaGrammar::GRAM_PODF;
        else
        {
            // In case there was a micro version, e.g. "1.2.3", this would
            // still yield major.minor, but pParsedEnd (5th parameter, not
            // passed here) would point before string end upon return.
            double fVer = ::rtl::math::stringToDouble( aVer, '.', 0, NULL, NULL);
            if (fVer < 1.2)
                eGrammar = formula::FormulaGrammar::GRAM_PODF;
        }
        pDoc->SetStorageGrammar( eGrammar);
    }

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));

        if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_STRUCTURE_PROTECTED))
                bProtected = IsXMLToken(sValue, XML_TRUE);
            else if (IsXMLToken(aLocalName, XML_PROTECTION_KEY))
                sPassword = sValue;
        }
    }
}

ScXMLBodyContext::~ScXMLBodyContext()
{
}

SvXMLImportContext *ScXMLBodyContext::CreateChildContext( sal_uInt16 nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    ScSheetSaveData* pSheetData = ScModelObj::getImplementation(GetScImport().GetModel())->GetSheetSaveData();
    if ( pSheetData && pSheetData->HasStartPos() )
    {
        // stream part to copy ends before the next child element
        sal_Int32 nEndOffset = GetScImport().GetByteOffset();
        pSheetData->EndStreamPos( nEndOffset );
    }

    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetBodyElemTokenMap();
//    sal_Bool bOrdered = sal_False;
//    sal_Bool bHeading = sal_False;
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
//  case XML_TOK_TEXT_H:
//      bHeading = sal_True;
//  case XML_TOK_TEXT_P:
//      pContext = new SwXMLParaContext( GetSwImport(),nPrefix, rLocalName,
//                                       xAttrList, bHeading );
//      break;
//  case XML_TOK_TEXT_ORDERED_LIST:
//      bOrdered = sal_True;
//  case XML_TOK_TEXT_UNORDERED_LIST:
//      pContext = new SwXMLListBlockContext( GetSwImport(),nPrefix, rLocalName,
//                                            xAttrList, bOrdered );
//      break;
    case XML_TOK_BODY_TRACKED_CHANGES :
    {
        pChangeTrackingImportHelper = GetScImport().GetChangeTrackingImportHelper();
        if (pChangeTrackingImportHelper)
            pContext = new ScXMLTrackedChangesContext( GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    break;
    case XML_TOK_BODY_CALCULATION_SETTINGS :
        pContext = new ScXMLCalculationSettingsContext( GetScImport(), nPrefix, rLocalName, xAttrList );
        bHadCalculationSettings = sal_True;
        break;
    case XML_TOK_BODY_CONTENT_VALIDATIONS :
        pContext = new ScXMLContentValidationsContext( GetScImport(), nPrefix, rLocalName, xAttrList );
        break;
    case XML_TOK_BODY_LABEL_RANGES:
        pContext = new ScXMLLabelRangesContext( GetScImport(), nPrefix, rLocalName, xAttrList );
        break;
    case XML_TOK_BODY_TABLE:
        {
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
        }
        break;
    case XML_TOK_BODY_NAMED_EXPRESSIONS:
        pContext = new ScXMLNamedExpressionsContext ( GetScImport(), nPrefix, rLocalName,
                                                        xAttrList );
        static_cast<ScXMLNamedExpressionsContext*>(pContext)->SetScope( MAXTABCOUNT );//workbookname
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
        SvXMLImportContext *pContext = new ScXMLCalculationSettingsContext( GetScImport(), XML_NAMESPACE_TABLE, GetXMLToken(XML_CALCULATION_SETTINGS), NULL );
        pContext->EndElement();
    }
    GetScImport().LockSolarMutex();
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

#if 0
        // #i57869# table styles are applied before the contents now

        std::vector<rtl::OUString> aTableStyleNames(GetScImport().GetTableStyle());
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( GetScImport().GetModel(), uno::UNO_QUERY );
        if ( xSpreadDoc.is() && !aTableStyleNames.empty())
        {
            uno::Reference <container::XIndexAccess> xIndex( xSpreadDoc->getSheets(), uno::UNO_QUERY );
            if ( xIndex.is() )
            {
                sal_Int32 nTableCount = xIndex->getCount();
                sal_Int32 nSize(aTableStyleNames.size());
                DBG_ASSERT(nTableCount == nSize, "every table should have a style name");
                for(sal_uInt32 i = 0; i < nTableCount; i++)
                {
                    if (i < nSize)
                    {
                        uno::Reference <beans::XPropertySet> xProperties(xIndex->getByIndex(i), uno::UNO_QUERY);
                        if (xProperties.is())
                        {
                            rtl::OUString sTableStyleName(aTableStyleNames[i]);
                            XMLTableStylesContext *pStyles = (XMLTableStylesContext *)GetScImport().GetAutoStyles();
                            if ( pStyles && sTableStyleName.getLength() )
                            {
                                XMLTableStyleContext* pStyle = (XMLTableStyleContext *)pStyles->FindStyleChildContext(
                                    XML_STYLE_FAMILY_TABLE_TABLE, sTableStyleName, sal_True);
                                if (pStyle)
                                    pStyle->FillPropertySet(xProperties);
                            }
                        }
                    }
                }
            }
        }
#endif

        // #i37959# handle document protection after the sheet settings
        if (bProtected)
        {
            ::std::auto_ptr<ScDocProtection> pProtection(new ScDocProtection);
            pProtection->setProtected(true);

            uno::Sequence<sal_Int8> aPass;
            if (sPassword.getLength())
            {
                SvXMLUnitConverter::decodeBase64(aPass, sPassword);
                pProtection->setPasswordHash(aPass, PASSHASH_OOO);
            }

            pDoc->SetDocProtection(pProtection.get());
        }
    }
    GetScImport().UnlockSolarMutex();
}

