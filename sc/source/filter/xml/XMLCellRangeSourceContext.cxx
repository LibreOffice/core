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




//___________________________________________________________________
#include "XMLCellRangeSourceContext.hxx"

#include <sax/tools/converter.hxx>

#include <xmloff/nmspmap.hxx>
#include "xmlimprt.hxx"

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
        sal_uInt16 nPrfx,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList >& xAttrList,
        ScMyImpCellRangeSource* pCellRangeSource ) :
    SvXMLImportContext( rImport, nPrfx, rLName )
{
    if( !xAttrList.is() ) return;

    sal_Int16               nAttrCount      = xAttrList->getLength();
    const SvXMLTokenMap&    rAttrTokenMap   = GetScImport().GetTableCellRangeSourceAttrTokenMap();

    for( sal_Int16 nIndex = 0; nIndex < nAttrCount; ++nIndex )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( nIndex ));
        const OUString& sValue(xAttrList->getValueByIndex( nIndex ));
        OUString aLocalName;
        sal_uInt16 nPrefix      = GetScImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

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
                if (::sax::Converter::convertNumber( nValue, sValue, 1 ))
                    pCellRangeSource->nColumns = nValue;
                else
                    pCellRangeSource->nColumns = 1;
            }
            break;
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_LAST_ROW:
            {
                sal_Int32 nValue;
                if (::sax::Converter::convertNumber( nValue, sValue, 1 ))
                    pCellRangeSource->nRows = nValue;
                else
                    pCellRangeSource->nRows = 1;
            }
            break;
            case XML_TOK_TABLE_CELL_RANGE_SOURCE_ATTR_REFRESH_DELAY:
            {
                double fTime;
                if (::sax::Converter::convertDuration( fTime, sValue ))
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
        sal_uInt16 nPrefix,
        const OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLCellRangeSourceContext::EndElement()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
