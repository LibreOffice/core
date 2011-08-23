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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

//___________________________________________________________________

#ifndef _SC_XMLCELLRANGESOURCECONTEXT_HXX
#include "XMLCellRangeSourceContext.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <bf_xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <bf_xmloff/xmluconv.hxx>
#endif

#ifndef SC_XMLIMPRT_HXX
#include "xmlimprt.hxx"
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;


//___________________________________________________________________

ScMyImpCellRangeSource::ScMyImpCellRangeSource() :
    nColumns( 0 ),
    nRows( 0 ),
    nRefresh( 0 )
{
}


//___________________________________________________________________

ScXMLCellRangeSourceContext::ScXMLCellRangeSourceContext(
        ScXMLImport& rImport,
        USHORT nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList,
        ScMyImpCellRangeSource* pCellRangeSource ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    if( !xAttrList.is() ) return;

    sal_Int16				nAttrCount		= xAttrList->getLength();
    const SvXMLTokenMap&	rAttrTokenMap	= GetScImport().GetTableCellRangeSourceAttrTokenMap();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; nIndex++ )
    {
        OUString sAttrName	= xAttrList->getNameByIndex( nIndex );
        OUString sValue		= xAttrList->getValueByIndex( nIndex );
        OUString aLocalName;
        USHORT nPrefix		= GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ) )
        {
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_NAME:
                pCellRangeSource->sSourceStr = sValue;
            break;
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_FILTER_NAME:
                pCellRangeSource->sFilterName = sValue;
            break;
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_FILTER_OPTIONS:
                pCellRangeSource->sFilterOptions = sValue;
            break;
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_HREF:
                pCellRangeSource->sURL = GetScImport().GetAbsoluteReference(sValue);
            break;
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_LAST_COLUMN:
            {
                sal_Int32 nValue;
                if( SvXMLUnitConverter::convertNumber( nValue, sValue, 1 ) )
                    pCellRangeSource->nColumns = nValue;
                else
                    pCellRangeSource->nColumns = 1;
            }
            break;
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_LAST_ROW:
            {
                sal_Int32 nValue;
                if( SvXMLUnitConverter::convertNumber( nValue, sValue, 1 ) )
                    pCellRangeSource->nRows = nValue;
                else
                    pCellRangeSource->nRows = 1;
            }
            break;
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_REFRESH_DELAY:
            {
                double fTime;
                if( SvXMLUnitConverter::convertTime( fTime, sValue ) )
                    pCellRangeSource->nRefresh = Max( (sal_Int32)(fTime * 86400.0), (sal_Int32)0 );
            }
            break;
        }
    }
}

ScXMLCellRangeSourceContext::~ScXMLCellRangeSourceContext()
{
}

SvXMLImportContext *ScXMLCellRangeSourceContext::CreateChildContext(
        USHORT nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLCellRangeSourceContext::EndElement()
{
}

}
