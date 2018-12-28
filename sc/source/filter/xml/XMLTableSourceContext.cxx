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

#include "XMLTableSourceContext.hxx"
#include "xmlimprt.hxx"
#include <document.hxx>
#include "xmlsubti.hxx"
#include <tablink.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <sax/tools/converter.hxx>
#include <com/sun/star/sheet/XSheetLinkable.hpp>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLTableSourceContext::ScXMLTableSourceContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList ) :
    ScXMLImportContext( rImport ),
    sLink(),
    sTableName(),
    sFilterName(),
    sFilterOptions(),
    nRefresh(0),
    nMode(sheet::SheetLinkMode_NORMAL)
{
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
            case XML_ELEMENT( XLINK, XML_HREF ):
                sLink = GetScImport().GetAbsoluteReference(aIter.toString());
                break;
            case XML_ELEMENT( TABLE, XML_TABLE_NAME ):
                sTableName = aIter.toString();
                break;
            case XML_ELEMENT( TABLE, XML_FILTER_NAME):
                sFilterName = aIter.toString();
                break;
            case XML_ELEMENT( TABLE, XML_FILTER_OPTIONS ):
                sFilterOptions = aIter.toString();
                break;
            case XML_ELEMENT( TABLE, XML_MODE ):
                if (IsXMLToken(aIter, XML_COPY_RESULTS_ONLY))
                    nMode = sheet::SheetLinkMode_VALUE;
                break;
            case XML_ELEMENT( TABLE, XML_REFRESH_DELAY ):
                double fTime;
                if (::sax::Converter::convertDuration( fTime, aIter.toString() ))
                    nRefresh = std::max( static_cast<sal_Int32>(fTime * 86400.0), sal_Int32(0) );
                break;
            }
        }
    }
}

ScXMLTableSourceContext::~ScXMLTableSourceContext()
{
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLTableSourceContext::createFastChildContext(
    sal_Int32 /*nElement*/, const uno::Reference< xml::sax::XFastAttributeList >& /*xAttrList*/ )
{
    return new SvXMLImportContext( GetImport() );
}

void SAL_CALL ScXMLTableSourceContext::endFastElement( sal_Int32 /*nElement*/ )
{
    if (!sLink.isEmpty())
    {
        uno::Reference <sheet::XSheetLinkable> xLinkable (GetScImport().GetTables().GetCurrentXSheet(), uno::UNO_QUERY);
        ScDocument* pDoc(GetScImport().GetDocument());
        if (xLinkable.is() && pDoc)
        {
            ScXMLImport::MutexGuard aGuard(GetScImport());
            if (pDoc->RenameTab( GetScImport().GetTables().GetCurrentSheet(),
                GetScImport().GetTables().GetCurrentSheetName(), true/*bExternalDocument*/))
            {
                sLink = ScGlobal::GetAbsDocName( sLink, pDoc->GetDocumentShell() );
                if (sFilterName.isEmpty())
                    ScDocumentLoader::GetFilterName( sLink, sFilterName, sFilterOptions, false, false );

                ScLinkMode nLinkMode = ScLinkMode::NONE;
                if ( nMode == sheet::SheetLinkMode_NORMAL )
                    nLinkMode = ScLinkMode::NORMAL;
                else if ( nMode == sheet::SheetLinkMode_VALUE )
                    nLinkMode = ScLinkMode::VALUE;

                pDoc->SetLink( GetScImport().GetTables().GetCurrentSheet(),
                    nLinkMode, sLink, sFilterName, sFilterOptions,
                    sTableName, nRefresh );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
