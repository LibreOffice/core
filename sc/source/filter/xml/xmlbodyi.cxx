/*************************************************************************
 *
 *  $RCSfile: xmlbodyi.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: sab $ $Date: 2000-11-21 16:28:20 $
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

#ifndef SC_XMLLABRI_HXX
#include "xmllabri.hxx"
#endif
#ifndef _SC_XMLCONSOLIDATIONCONTEXT_HXX
#include "XMLConsolidationContext.hxx"
#endif
#ifndef _SC_XMLDDELINKSCONTEXT_HXX
#include "XMLDDELinksContext.hxx"
#endif

#include <xmloff/xmltkmap.hxx>

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

using namespace com::sun::star;

//------------------------------------------------------------------

ScXMLBodyContext::ScXMLBodyContext( ScXMLImport& rImport,
                                              USHORT nPrfx,
                                                   const NAMESPACE_RTL(OUString)& rLName ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
}

ScXMLBodyContext::~ScXMLBodyContext()
{
}

SvXMLImportContext *ScXMLBodyContext::CreateChildContext( USHORT nPrefix,
                                     const NAMESPACE_RTL(OUString)& rLocalName,
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
    case XML_TOK_BODY_CONTENT_VALIDATIONS :
        pContext = new ScXMLContentValidationsContext( GetScImport(), nPrefix, rLocalName, xAttrList );
        break;
    case XML_TOK_BODY_LABEL_RANGES:
        pContext = new ScXMLLabelRangesContext( GetScImport(), nPrefix, rLocalName, xAttrList );
        break;
    case XML_TOK_BODY_TABLE:
//      if( !GetScImport().GetPaM().GetNode()->FindTableNode() )
            pContext = new ScXMLTableContext( GetScImport(),nPrefix, rLocalName,
                                              xAttrList );
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
    ScMyImpDetectiveOpArray&    rDetOpArray = GetScImport().GetDetectiveOpArray();
    ScDocument*                 pDoc        = GetScImport().GetDocument();
    ScMyImpDetectiveOp          rDetOp;

    rDetOpArray.Sort();
    while( rDetOpArray.GetFirstOp( rDetOp ) )
    {
        ScDetOpData aOpData( rDetOp.aPosition, rDetOp.eOpType );
        pDoc->AddDetectiveOperation( aOpData );
    }
}

