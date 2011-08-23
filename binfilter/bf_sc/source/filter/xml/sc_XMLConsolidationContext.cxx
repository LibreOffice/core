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

//___________________________________________________________________

#include "XMLConsolidationContext.hxx"

#include "document.hxx"
#include "rangeutl.hxx"
#include "xmlimprt.hxx"
#include "XMLConverter.hxx"

#include <bf_xmloff/nmspmap.hxx>
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace xmloff::token;


//___________________________________________________________________

ScXMLConsolidationContext::ScXMLConsolidationContext(
        ScXMLImport& rImport,
        USHORT nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    eFunction( SUBTOTAL_FUNC_NONE ),
    bLinkToSource( sal_False ),
    bTargetAddr(sal_False)
{
    rImport.LockSolarMutex();
    if( !xAttrList.is() ) return;

    sal_Int16				nAttrCount		= xAttrList->getLength();
    const SvXMLTokenMap&	rAttrTokenMap	= GetScImport().GetConsolidationAttrTokenMap();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString sAttrName	= xAttrList->getNameByIndex( nIndex );
        OUString sValue		= xAttrList->getValueByIndex( nIndex );
        OUString aLocalName;
        USHORT nPrefix		= GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_CONSOLIDATION_ATTR_FUNCTION:
                eFunction = ScXMLConverter::GetSubTotalFuncFromString( sValue );
            break;
            case XML_TOK_CONSOLIDATION_ATTR_SOURCE_RANGES:
                sSourceList = sValue;
            break;
            case XML_TOK_CONSOLIDATION_ATTR_TARGET_ADDRESS:
                {
                    sal_Int32 nOffset(0);
                    bTargetAddr = ScXMLConverter::GetAddressFromString(
                        aTargetAddr, sValue, GetScImport().GetDocument(), nOffset );
                }
                break;
            case XML_TOK_CONSOLIDATION_ATTR_USE_LABEL:
                sUseLabel = sValue;
            break;
            case XML_TOK_CONSOLIDATION_ATTR_LINK_TO_SOURCE:
                bLinkToSource = IsXMLToken(sValue, XML_TRUE);
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
    if (bTargetAddr)
    {
        ScConsolidateParam aConsParam;
        aConsParam.nCol = aTargetAddr.Col();
        aConsParam.nRow = aTargetAddr.Row();
        aConsParam.nTab = aTargetAddr.Tab();
        aConsParam.eFunction = eFunction;

        sal_Bool bError = sal_False;
        USHORT nCount = (USHORT) Min( ScXMLConverter::GetTokenCount( sSourceList ), (sal_Int32)0xFFFF );
        ScArea** ppAreas = nCount ? new ScArea*[ nCount ] : NULL;
        if( ppAreas )
        {
            sal_Int32 nOffset = 0;
            USHORT nIndex;
            for( nIndex = 0; nIndex < nCount; nIndex++ )
            {
                ppAreas[ nIndex ] = new ScArea;
                if ( !ScXMLConverter::GetAreaFromString(
                    *ppAreas[ nIndex ], sSourceList, GetScImport().GetDocument(), nOffset ) )
                {
                    bError = sal_True;		//! handle error
                }
            }

            aConsParam.SetAreas( ppAreas, nCount );

            // array is copied in SetAreas
            for( nIndex = 0; nIndex < nCount; nIndex++ )
                delete ppAreas[nIndex];
            delete[] ppAreas;
        }

        aConsParam.bByCol = aConsParam.bByRow = FALSE;
        if( IsXMLToken(sUseLabel, XML_COLUMN ) )
            aConsParam.bByCol = TRUE;
        else if( IsXMLToken( sUseLabel, XML_ROW ) )
            aConsParam.bByRow = TRUE;
        else if( IsXMLToken( sUseLabel, XML_BOTH ) )
            aConsParam.bByCol = aConsParam.bByRow = TRUE;

        aConsParam.bReferenceData = bLinkToSource;

        ScDocument* pDoc = GetScImport().GetDocument();
        if( pDoc )
            pDoc->SetConsolidateDlgData( &aConsParam );
    }
    GetScImport().UnlockSolarMutex();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
