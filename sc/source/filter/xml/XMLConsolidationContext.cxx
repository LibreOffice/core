/*************************************************************************
 *
 *  $RCSfile: XMLConsolidationContext.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dr $ $Date: 2000-11-02 16:51:44 $
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

//___________________________________________________________________

#ifndef _SC_XMLCONSOLIDATIONCONTEXT_HXX
#include "XMLConsolidationContext.hxx"
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_RANGEUTL_HXX
#include "rangeutl.hxx"
#endif
#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif
#ifndef _SC_XMLCONVERTER_HXX
#include "XMLConverter.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include <xmloff/xmlkywd.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;


//___________________________________________________________________

ScXMLConsolidationContext::ScXMLConsolidationContext(
        ScXMLImport& rImport,
        USHORT nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    eFunction( SUBTOTAL_FUNC_NONE ),
    bLinkToSource( sal_False )
{
    if( !xAttrList.is() ) return;

    sal_Int16               nAttrCount      = xAttrList->getLength();
    const SvXMLTokenMap&    rAttrTokenMap   = GetScImport().GetConsolidationAttrTokenMap();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString sAttrName  = xAttrList->getNameByIndex( nIndex );
        OUString sValue     = xAttrList->getValueByIndex( nIndex );
        OUString aLocalName;
        USHORT nPrefix      = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_CONSOLIDATION_ATTR_FUNCTION:
                eFunction = ScXMLConverter::GetSubTotalFuncFromString( sValue );
            break;
            case XML_TOK_CONSOLIDATION_ATTR_SOURCE_RANGES:
                sSourceList = sValue;
            break;
            case XML_TOK_CONSOLIDATION_ATTR_TARGET_ADDRESS:
                ScXMLConverter::GetAddressFromString(
                    aTargetAddr, sValue, GetScImport().GetDocument() );
            break;
            case XML_TOK_CONSOLIDATION_ATTR_USE_LABEL:
                sUseLabel = sValue;
            break;
            case XML_TOK_CONSOLIDATION_ATTR_LINK_TO_SOURCE:
                bLinkToSource = (sValue.compareToAscii( sXML_true ) == 0);
            break;
        }
    }
}

ScXMLConsolidationContext::~ScXMLConsolidationContext()
{
}

SvXMLImportContext *ScXMLConsolidationContext::CreateChildContext(
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLConsolidationContext::EndElement()
{
    ScConsolidateParam aConsParam;
    aConsParam.nCol = aTargetAddr.Col();
    aConsParam.nRow = aTargetAddr.Row();
    aConsParam.nTab = aTargetAddr.Tab();
    aConsParam.eFunction = eFunction;

    USHORT nCount = (USHORT) Min( ScXMLConverter::GetTokenCount( sSourceList ), (sal_Int32)0xFFFF );
    ScArea** ppAreas = nCount ? new ScArea*[ nCount ] : NULL;
    if( ppAreas )
    {
        sal_Int32 nOffset = 0;
        for( USHORT nIndex = 0; nIndex < nCount; nIndex++ )
        {
            ppAreas[ nIndex ] = new ScArea;
            nOffset = ScXMLConverter::GetAreaFromString(
                *ppAreas[ nIndex ], sSourceList, GetScImport().GetDocument(), nOffset );
        }
        aConsParam.SetAreas( ppAreas, nCount );
    }

    aConsParam.bByCol = aConsParam.bByRow = FALSE;
    if( sUseLabel.compareToAscii( sXML_column ) == 0 )
        aConsParam.bByCol = TRUE;
    else if( sUseLabel.compareToAscii( sXML_row ) == 0 )
        aConsParam.bByRow = TRUE;
    else if( sUseLabel.compareToAscii( sXML_both ) == 0 )
        aConsParam.bByCol = aConsParam.bByRow = TRUE;

    aConsParam.bReferenceData = bLinkToSource;

    ScDocument* pDoc = GetScImport().GetDocument();
    if( pDoc )
        pDoc->SetConsolidateDlgData( &aConsParam );
}

