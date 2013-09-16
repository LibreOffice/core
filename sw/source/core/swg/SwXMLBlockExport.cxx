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
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

// #110680#
SwXMLBlockListExport::SwXMLBlockListExport(
    const uno::Reference< uno::XComponentContext > xContext,
    SwXMLTextBlocks & rBlocks,
    const OUString &rFileName,
    uno::Reference< xml::sax::XDocumentHandler> &rHandler)
:   SvXMLExport( xContext, rFileName, util::MeasureUnit::CM, rHandler ),
    rBlockList(rBlocks)
{
    _GetNamespaceMap().Add( GetXMLToken ( XML_NP_BLOCK_LIST ),
                            GetXMLToken ( XML_N_BLOCK_LIST ),
                            XML_NAMESPACE_BLOCKLIST );
}

sal_uInt32 SwXMLBlockListExport::exportDoc(enum XMLTokenEnum )
{
    GetDocHandler()->startDocument();

    addChaffWhenEncryptedStorage();

    AddAttribute ( XML_NAMESPACE_NONE,
                   _GetNamespaceMap().GetAttrNameByKey ( XML_NAMESPACE_BLOCKLIST ),
                   _GetNamespaceMap().GetNameByKey ( XML_NAMESPACE_BLOCKLIST ) );
    AddAttribute( XML_NAMESPACE_BLOCKLIST,
                  XML_LIST_NAME,
                  OUString (rBlockList.GetName()));
    {
        SvXMLElementExport pRoot (*this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK_LIST, sal_True, sal_True);
        sal_uInt16 nBlocks= rBlockList.GetCount();
        for ( sal_uInt16 i = 0; i < nBlocks; i++)
        {
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_ABBREVIATED_NAME,
                          OUString(rBlockList.GetShortName(i)));
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_PACKAGE_NAME,
                          OUString(rBlockList.GetPackageName(i)));
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_NAME,
                          OUString(rBlockList.GetLongName(i)));
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_UNFORMATTED_TEXT,
                          rBlockList.IsOnlyTextBlock(i) ? XML_TRUE : XML_FALSE );

            SvXMLElementExport aBlock( *this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK, sal_True, sal_True);
        }
    }
    GetDocHandler()->endDocument();
    return 0;
}

// #110680#
SwXMLTextBlockExport::SwXMLTextBlockExport(
    const uno::Reference< uno::XComponentContext > xContext,
    SwXMLTextBlocks & rBlocks,
    const OUString &rFileName,
    uno::Reference< xml::sax::XDocumentHandler> &rHandler)
:   SvXMLExport( xContext, rFileName, util::MeasureUnit::CM, rHandler ),
    rBlockList(rBlocks)
{
    _GetNamespaceMap().Add( GetXMLToken ( XML_NP_BLOCK_LIST ),
                            GetXMLToken ( XML_N_BLOCK_LIST ),
                            XML_NAMESPACE_BLOCKLIST );
    _GetNamespaceMap().Add( GetXMLToken ( XML_NP_OFFICE ),
                            GetXMLToken(XML_N_OFFICE_OOO),
                            XML_NAMESPACE_OFFICE );
    _GetNamespaceMap().Add( GetXMLToken ( XML_NP_TEXT ),
                            GetXMLToken(XML_N_TEXT_OOO),
                            XML_NAMESPACE_TEXT );
}

sal_uInt32 SwXMLTextBlockExport::exportDoc(const OUString &rText)
{
    GetDocHandler()->startDocument();

    addChaffWhenEncryptedStorage();

    AddAttribute ( XML_NAMESPACE_NONE,
                   _GetNamespaceMap().GetAttrNameByKey ( XML_NAMESPACE_BLOCKLIST ),
                   _GetNamespaceMap().GetNameByKey ( XML_NAMESPACE_BLOCKLIST ) );
    AddAttribute ( XML_NAMESPACE_NONE,
                   _GetNamespaceMap().GetAttrNameByKey ( XML_NAMESPACE_TEXT ),
                   _GetNamespaceMap().GetNameByKey ( XML_NAMESPACE_TEXT ) );
    AddAttribute ( XML_NAMESPACE_NONE,
                   _GetNamespaceMap().GetAttrNameByKey ( XML_NAMESPACE_OFFICE ),
                   _GetNamespaceMap().GetNameByKey ( XML_NAMESPACE_OFFICE ) );
    AddAttribute( XML_NAMESPACE_BLOCKLIST,
                  XML_LIST_NAME,
                  OUString (rBlockList.GetName()));
    {
        SvXMLElementExport aDocument (*this, XML_NAMESPACE_OFFICE, XML_DOCUMENT, sal_True, sal_True);
        {
            SvXMLElementExport aBody (*this, XML_NAMESPACE_OFFICE, XML_BODY, sal_True, sal_True);
            {
                sal_Int32 nPos = 0;
                do
                {
                    OUString sTemp ( rText.getToken( 0, '\015', nPos ) );
                    SvXMLElementExport aPara (*this, XML_NAMESPACE_TEXT, XML_P, sal_True, sal_False);
                    GetDocHandler()->characters(sTemp);
                } while (-1 != nPos );
            }

        }
    }
    GetDocHandler()->endDocument();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
