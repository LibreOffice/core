/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------


#include "xmlbodyi.hxx"
#include "xmltabi.hxx"
#include "xmlnexpi.hxx"
#include "xmldrani.hxx"
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

#include <bf_xmloff/xmlnmspe.hxx>
#include <bf_xmloff/nmspmap.hxx>
#include <bf_xmloff/xmluconv.hxx>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>

#include <sal/types.h>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

//------------------------------------------------------------------

ScXMLBodyContext::ScXMLBodyContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(NULL),
    bProtected(sal_False),
    sPassword()
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        ::rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        ::rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        ::rtl::OUString sValue = xAttrList->getValueByIndex( i );

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

SvXMLImportContext *ScXMLBodyContext::CreateChildContext( USHORT nPrefix,
                                     const ::rtl::OUString& rLocalName,
                                     const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetScImport().GetBodyElemTokenMap();
    sal_Bool bOrdered = sal_False;
    sal_Bool bHeading = sal_False;
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
//	case XML_TOK_TEXT_H:
//		bHeading = TRUE;
//	case XML_TOK_TEXT_P:
//		pContext = new SwXMLParaContext( GetSwImport(),nPrefix, rLocalName,
//										 xAttrList, bHeading );
//		break;
//	case XML_TOK_TEXT_ORDERED_LIST:
//		bOrdered = TRUE;
//	case XML_TOK_TEXT_UNORDERED_LIST:
//		pContext = new SwXMLListBlockContext( GetSwImport(),nPrefix, rLocalName,
//											  xAttrList, bOrdered );
//		break;
    case XML_TOK_BODY_TRACKED_CHANGES :
    {
        pChangeTrackingImportHelper = GetScImport().GetChangeTrackingImportHelper();
        if (pChangeTrackingImportHelper)
            pContext = new ScXMLTrackedChangesContext( GetScImport(), nPrefix, rLocalName, xAttrList, pChangeTrackingImportHelper);
    }
    break;
    case XML_TOK_BODY_CALCULATION_SETTINGS :
        pContext = new ScXMLCalculationSettingsContext( GetScImport(), nPrefix, rLocalName, xAttrList );
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

void ScXMLBodyContext::EndElement()
{
    GetScImport().LockSolarMutex();
    ScMyImpDetectiveOpArray*	pDetOpArray	= GetScImport().GetDetectiveOpArray();
    ScDocument*					pDoc		= GetScImport().GetDocument();
    ScMyImpDetectiveOp			aDetOp;

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

        if (bProtected)
        {
            uno::Sequence<sal_Int8> aPass;
            if (sPassword.getLength())
                SvXMLUnitConverter::decodeBase64(aPass, sPassword);
            pDoc->SetDocProtection(bProtected, aPass);
        }
        uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( GetScImport().GetModel(), uno::UNO_QUERY );
        if ( xSpreadDoc.is() )
        {
            uno::Reference<sheet::XSpreadsheets> xSheets = xSpreadDoc->getSheets();
            uno::Reference <container::XIndexAccess> xIndex( xSheets, uno::UNO_QUERY );
            if ( xIndex.is() )
            {
                uno::Any aSheet = xIndex->getByIndex(0);
                uno::Reference< sheet::XSpreadsheet > xSheet;
                if ( aSheet >>= xSheet )
                {
                    uno::Reference <beans::XPropertySet> xProperties(xSheet, uno::UNO_QUERY);
                    if (xProperties.is())
                    {
                        XMLTableStylesContext *pStyles = (XMLTableStylesContext *)GetScImport().GetAutoStyles();
                        ::rtl::OUString sTableStyleName(GetScImport().GetFirstTableStyle());
                        if (sTableStyleName.getLength())
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
    GetScImport().UnlockSolarMutex();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
