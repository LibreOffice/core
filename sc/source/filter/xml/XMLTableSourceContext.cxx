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
#include "document.hxx"
#include "xmlsubti.hxx"
#include "tablink.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <sax/tools/converter.hxx>
#include <com/sun/star/sheet/XSheetLinkable.hpp>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLTableSourceContext::ScXMLTableSourceContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList) :
    SvXMLImportContext( rImport, nPrfx, rLName ),
    sLink(),
    sTableName(),
    sFilterName(),
    sFilterOptions(),
    nRefresh(0),
    nMode(sheet::SheetLinkMode_NORMAL)
{
    sal_Int16 nAttrCount(xAttrList.is() ? xAttrList->getLength() : 0);
    for( sal_Int16 i=0; i < nAttrCount; ++i )
    {
        const OUString& sAttrName(xAttrList->getNameByIndex( i ));
        OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const OUString& sValue(xAttrList->getValueByIndex( i ));
        if(nPrefix == XML_NAMESPACE_XLINK)
        {
            if (IsXMLToken(aLocalName, XML_HREF))
                sLink = GetScImport().GetAbsoluteReference(sValue);
        }
        else if (nPrefix == XML_NAMESPACE_TABLE)
        {
            if (IsXMLToken(aLocalName, XML_TABLE_NAME))
                sTableName = sValue;
            else if (IsXMLToken(aLocalName, XML_FILTER_NAME))
                sFilterName = sValue;
            else if (IsXMLToken(aLocalName, XML_FILTER_OPTIONS))
                sFilterOptions = sValue;
            else if (IsXMLToken(aLocalName, XML_MODE))
            {
                if (IsXMLToken(sValue, XML_COPY_RESULTS_ONLY))
                    nMode = sheet::SheetLinkMode_VALUE;
            }
            else if (IsXMLToken(aLocalName, XML_REFRESH_DELAY))
            {
                double fTime;
                if (::sax::Converter::convertDuration( fTime, sValue ))
                    nRefresh = std::max( (sal_Int32)(fTime * 86400.0), (sal_Int32)0 );
            }
        }
    }
}

ScXMLTableSourceContext::~ScXMLTableSourceContext()
{
}

SvXMLImportContext *ScXMLTableSourceContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLTableSourceContext::EndElement()
{
    if (!sLink.isEmpty())
    {
        uno::Reference <sheet::XSheetLinkable> xLinkable (GetScImport().GetTables().GetCurrentXSheet(), uno::UNO_QUERY);
        ScDocument* pDoc(GetScImport().GetDocument());
        if (xLinkable.is() && pDoc)
        {
            ScXMLImport::MutexGuard aGuard(GetScImport());
            if (pDoc->RenameTab( GetScImport().GetTables().GetCurrentSheet(),
                GetScImport().GetTables().GetCurrentSheetName(), false, true))
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
