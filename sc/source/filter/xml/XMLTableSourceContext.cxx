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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------
#include "XMLTableSourceContext.hxx"
#include "xmlimprt.hxx"
#include "document.hxx"
#include "xmlsubti.hxx"
#include "tablink.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include <com/sun/star/sheet/XSheetLinkable.hpp>

using namespace com::sun::star;
using namespace xmloff::token;

//------------------------------------------------------------------

ScXMLTableSourceContext::ScXMLTableSourceContext( ScXMLImport& rImport,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& rLName,
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
        const rtl::OUString& sAttrName(xAttrList->getNameByIndex( i ));
        rtl::OUString aLocalName;
        sal_uInt16 nPrefix(GetScImport().GetNamespaceMap().GetKeyByAttrName(
                                            sAttrName, &aLocalName ));
        const rtl::OUString& sValue(xAttrList->getValueByIndex( i ));
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
                if( SvXMLUnitConverter::convertTime( fTime, sValue ) )
                    nRefresh = Max( (sal_Int32)(fTime * 86400.0), (sal_Int32)0 );
            }
        }
    }
}

ScXMLTableSourceContext::~ScXMLTableSourceContext()
{
}

SvXMLImportContext *ScXMLTableSourceContext::CreateChildContext( sal_uInt16 nPrefix,
                                            const ::rtl::OUString& rLName,
                                            const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::xml::sax::XAttributeList>& /* xAttrList */ )
{
    return new SvXMLImportContext( GetImport(), nPrefix, rLName );
}

void ScXMLTableSourceContext::EndElement()
{
    if (sLink.getLength())
    {
        uno::Reference <sheet::XSheetLinkable> xLinkable (GetScImport().GetTables().GetCurrentXSheet(), uno::UNO_QUERY);
        ScDocument* pDoc(GetScImport().GetDocument());
        if (xLinkable.is() && pDoc)
        {
            ScXMLImport::MutexGuard aGuard(GetScImport());
            if (pDoc->RenameTab( static_cast<SCTAB>(GetScImport().GetTables().GetCurrentSheet()),
                GetScImport().GetTables().GetCurrentSheetName(), false, sal_True))
            {
                 String aFileString(sLink);
                String aFilterString(sFilterName);
                String aOptString(sFilterOptions);
                String aSheetString(sTableName);

                aFileString = ScGlobal::GetAbsDocName( aFileString, pDoc->GetDocumentShell() );
                if ( !aFilterString.Len() )
                    ScDocumentLoader::GetFilterName( aFileString, aFilterString, aOptString, false, false );

                sal_uInt8 nLinkMode = SC_LINK_NONE;
                if ( nMode == sheet::SheetLinkMode_NORMAL )
                    nLinkMode = SC_LINK_NORMAL;
                else if ( nMode == sheet::SheetLinkMode_VALUE )
                    nLinkMode = SC_LINK_VALUE;

                pDoc->SetLink( static_cast<SCTAB>(GetScImport().GetTables().GetCurrentSheet()),
                    nLinkMode, aFileString, aFilterString, aOptString,
                    aSheetString, nRefresh );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
