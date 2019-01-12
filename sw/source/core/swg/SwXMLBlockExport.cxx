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

#include <SwXMLBlockExport.hxx>
#include <SwXMLTextBlocks.hxx>
#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

SwXMLBlockListExport::SwXMLBlockListExport(
    const uno::Reference< uno::XComponentContext >& rContext,
    SwXMLTextBlocks & rBlocks,
    const OUString &rFileName,
    uno::Reference< xml::sax::XDocumentHandler> const &rHandler)
:   SvXMLExport( rContext, "", rFileName, util::MeasureUnit::CM, rHandler ),
    rBlockList(rBlocks)
{
    GetNamespaceMap_().Add( GetXMLToken ( XML_NP_BLOCK_LIST ),
                            GetXMLToken ( XML_N_BLOCK_LIST ),
                            XML_NAMESPACE_BLOCKLIST );
}

ErrCode SwXMLBlockListExport::exportDoc(enum XMLTokenEnum )
{
    GetDocHandler()->startDocument();

    addChaffWhenEncryptedStorage();

    AddAttribute ( XML_NAMESPACE_NONE,
                   GetNamespaceMap_().GetAttrNameByKey ( XML_NAMESPACE_BLOCKLIST ),
                   GetNamespaceMap_().GetNameByKey ( XML_NAMESPACE_BLOCKLIST ) );
    AddAttribute( XML_NAMESPACE_BLOCKLIST,
                  XML_LIST_NAME,
                  rBlockList.GetName());
    {
        SvXMLElementExport aRoot (*this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK_LIST, true, true);
        sal_uInt16 nBlocks= rBlockList.GetCount();
        for ( sal_uInt16 i = 0; i < nBlocks; i++)
        {
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_ABBREVIATED_NAME,
                          rBlockList.GetShortName(i));
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_PACKAGE_NAME,
                          rBlockList.GetPackageName(i));
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_NAME,
                          rBlockList.GetLongName(i));
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_UNFORMATTED_TEXT,
                          rBlockList.IsOnlyTextBlock(i) ? XML_TRUE : XML_FALSE );

            SvXMLElementExport aBlock( *this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK, true, true);
        }
    }
    GetDocHandler()->endDocument();
    return ERRCODE_NONE;
}

SwXMLTextBlockExport::SwXMLTextBlockExport(
    const uno::Reference< uno::XComponentContext >& rContext,
    SwXMLTextBlocks & rBlocks,
    const OUString &rFileName,
    uno::Reference< xml::sax::XDocumentHandler> const &rHandler)
:   SvXMLExport( rContext, "", rFileName, util::MeasureUnit::CM, rHandler ),
    rBlockList(rBlocks)
{
    GetNamespaceMap_().Add( GetXMLToken ( XML_NP_BLOCK_LIST ),
                            GetXMLToken ( XML_N_BLOCK_LIST ),
                            XML_NAMESPACE_BLOCKLIST );
    GetNamespaceMap_().Add( GetXMLToken ( XML_NP_OFFICE ),
                            GetXMLToken(XML_N_OFFICE_OOO),
                            XML_NAMESPACE_OFFICE );
    GetNamespaceMap_().Add( GetXMLToken ( XML_NP_TEXT ),
                            GetXMLToken(XML_N_TEXT_OOO),
                            XML_NAMESPACE_TEXT );
}

void SwXMLTextBlockExport::exportDoc(const OUString &rText)
{
    GetDocHandler()->startDocument();

    addChaffWhenEncryptedStorage();

    AddAttribute ( XML_NAMESPACE_NONE,
                   GetNamespaceMap_().GetAttrNameByKey ( XML_NAMESPACE_BLOCKLIST ),
                   GetNamespaceMap_().GetNameByKey ( XML_NAMESPACE_BLOCKLIST ) );
    AddAttribute ( XML_NAMESPACE_NONE,
                   GetNamespaceMap_().GetAttrNameByKey ( XML_NAMESPACE_TEXT ),
                   GetNamespaceMap_().GetNameByKey ( XML_NAMESPACE_TEXT ) );
    AddAttribute ( XML_NAMESPACE_NONE,
                   GetNamespaceMap_().GetAttrNameByKey ( XML_NAMESPACE_OFFICE ),
                   GetNamespaceMap_().GetNameByKey ( XML_NAMESPACE_OFFICE ) );
    AddAttribute( XML_NAMESPACE_BLOCKLIST,
                  XML_LIST_NAME,
                  rBlockList.GetName());
    {
        SvXMLElementExport aDocument (*this, XML_NAMESPACE_OFFICE, XML_DOCUMENT, true, true);
        {
            SvXMLElementExport aBody (*this, XML_NAMESPACE_OFFICE, XML_BODY, true, true);
            {
                sal_Int32 nPos = 0;
                do
                {
                    OUString sTemp ( rText.getToken( 0, '\015', nPos ) );
                    SvXMLElementExport aPara (*this, XML_NAMESPACE_TEXT, XML_P, true, false);
                    GetDocHandler()->characters(sTemp);
                } while (-1 != nPos );
            }

        }
    }
    GetDocHandler()->endDocument();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
