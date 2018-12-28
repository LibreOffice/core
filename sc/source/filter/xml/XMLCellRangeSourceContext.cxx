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

#include "XMLCellRangeSourceContext.hxx"

#include <sax/tools/converter.hxx>

#include "xmlimprt.hxx"
#include <xmloff/xmlnmspe.hxx>

using namespace ::com::sun::star;
using namespace xmloff::token;

ScMyImpCellRangeSource::ScMyImpCellRangeSource() :
    nColumns( 0 ),
    nRows( 0 ),
    nRefresh( 0 )
{
}

ScXMLCellRangeSourceContext::ScXMLCellRangeSourceContext(
        ScXMLImport& rImport,
        const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList,
        ScMyImpCellRangeSource* pCellRangeSource ) :
    ScXMLImportContext( rImport )
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( TABLE, XML_NAME ):
                    pCellRangeSource->sSourceStr = aIter.toString();
                break;
                case XML_ELEMENT( TABLE, XML_FILTER_NAME ):
                    pCellRangeSource->sFilterName = aIter.toString();
                break;
                case XML_ELEMENT( TABLE, XML_FILTER_OPTIONS ):
                    pCellRangeSource->sFilterOptions = aIter.toString();
                break;
                case XML_ELEMENT( XLINK, XML_HREF ):
                    pCellRangeSource->sURL = GetScImport().GetAbsoluteReference(aIter.toString());
                break;
                case XML_ELEMENT( TABLE, XML_LAST_COLUMN_SPANNED ):
                {
                    sal_Int32 nValue;
                    if (::sax::Converter::convertNumber( nValue, aIter.toString(), 1 ))
                        pCellRangeSource->nColumns = nValue;
                    else
                        pCellRangeSource->nColumns = 1;
                }
                break;
                case XML_ELEMENT( TABLE, XML_LAST_ROW_SPANNED ):
                {
                    sal_Int32 nValue;
                    if (::sax::Converter::convertNumber( nValue, aIter.toString(), 1 ))
                        pCellRangeSource->nRows = nValue;
                    else
                        pCellRangeSource->nRows = 1;
                }
                break;
                case XML_ELEMENT( TABLE, XML_REFRESH_DELAY ):
                {
                    double fTime;
                    if (::sax::Converter::convertDuration( fTime, aIter.toString() ))
                        pCellRangeSource->nRefresh = std::max( static_cast<sal_Int32>(fTime * 86400.0), sal_Int32(0) );
                }
                break;
            }
        }
    }
}

ScXMLCellRangeSourceContext::~ScXMLCellRangeSourceContext()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
