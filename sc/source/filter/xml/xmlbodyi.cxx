/*************************************************************************
 *
 *  $RCSfile: xmlbodyi.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: svesik $ $Date: 2004-04-20 13:56:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif

#include "xmlbodyi.hxx"
#include "xmltabi.hxx"
#include "xmlnexpi.hxx"
#include "xmldrani.hxx"
#include "xmlimprt.hxx"
#include "xmldpimp.hxx"
#include "xmlcvali.hxx"
#include "xmlstyli.hxx"

#ifndef SC_XMLLABRI_HXX
#include "xmllabri.hxx"
#endif
#ifndef _SC_XMLCONSOLIDATIONCONTEXT_HXX
#include "XMLConsolidationContext.hxx"
#endif
#ifndef _SC_XMLDDELINKSCONTEXT_HXX
#include "XMLDDELinksContext.hxx"
#endif
#ifndef _SC_XMLCALCULATIONSETTINGSCONTEXT_HXX
#include "XMLCalculationSettingsContext.hxx"
#endif
#ifndef _SC_XMLTRACKEDCHANGESCONTEXT_HXX
#include "XMLTrackedChangesContext.hxx"
#endif
#ifndef SC_XMLEMPTYCONTEXT_HXX
#include "XMLEmptyContext.hxx"
#endif
#ifndef _SCERRORS_HXX
#include "scerrors.hxx"
#endif

#include <xmloff/xmltkmap.hxx>
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLBodyContext::ScXMLBodyContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const ::rtl::OUString& rLName,
                                              const uno::Reference<xml::sax::XAttributeList>& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    pChangeTrackingImportHelper(NULL),
    bProtected(sal_False),
    bHadCalculationSettings(sal_False),
    sPassword()
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName );
        rtl::OUString sValue = xAttrList->getValueByIndex( i );

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
//  case XML_TOK_TEXT_H:
//      bHeading = TRUE;
//  case XML_TOK_TEXT_P:
//      pContext = new SwXMLParaContext( GetSwImport(),nPrefix, rLocalName,
//                                       xAttrList, bHeading );
//      break;
//  case XML_TOK_TEXT_ORDERED_LIST:
//      bOrdered = TRUE;
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

        if (bProtected)
        {
            uno::Sequence<sal_Int8> aPass;
            if (sPassword.getLength())
                SvXMLUnitConverter::decodeBase64(aPass, sPassword);
            pDoc->SetDocProtection(bProtected, aPass);
        }
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
    }
    GetScImport().UnlockSolarMutex();
}

