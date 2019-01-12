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

#include "SvXMLAutoCorrectExport.hxx"

#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <xmloff/xmltoken.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

SvXMLAutoCorrectExport::SvXMLAutoCorrectExport(
    const css::uno::Reference< css::uno::XComponentContext > & xContext,
    const SvxAutocorrWordList *  pNewAutocorr_List,
    const OUString &rFileName,
    css::uno::Reference< css::xml::sax::XDocumentHandler> const &rHandler)
:   SvXMLExport( xContext, "", rFileName, util::MeasureUnit::CM, rHandler ),
    pAutocorr_List( pNewAutocorr_List )
{
    GetNamespaceMap_().Add( GetXMLToken ( XML_NP_BLOCK_LIST),
                            GetXMLToken ( XML_N_BLOCK_LIST ),
                            XML_NAMESPACE_BLOCKLIST );
}

ErrCode SvXMLAutoCorrectExport::exportDoc(enum XMLTokenEnum /*eClass*/)
{
    GetDocHandler()->startDocument();

    addChaffWhenEncryptedStorage();

    AddAttribute ( XML_NAMESPACE_NONE,
                   GetNamespaceMap_().GetAttrNameByKey ( XML_NAMESPACE_BLOCKLIST ),
                   GetNamespaceMap_().GetNameByKey ( XML_NAMESPACE_BLOCKLIST ) );
    {
        SvXMLElementExport aRoot (*this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK_LIST, true, true);
        SvxAutocorrWordList::Content aContent = pAutocorr_List->getSortedContent();
        for (auto const& content : aContent)
        {
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_ABBREVIATED_NAME,
                          content->GetShort());
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_NAME,
                          content->IsTextOnly() ? content->GetLong() : content->GetShort());

            SvXMLElementExport aBlock( *this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK, true, true);
        }
    }
    GetDocHandler()->endDocument();
    return ERRCODE_NONE;
}

SvXMLExceptionListExport::SvXMLExceptionListExport(
    const css::uno::Reference< css::uno::XComponentContext > & xContext,
    const SvStringsISortDtor &rNewList,
    const OUString &rFileName,
    css::uno::Reference< css::xml::sax::XDocumentHandler> const &rHandler)
:   SvXMLExport( xContext, "", rFileName, util::MeasureUnit::CM, rHandler ),
    rList( rNewList )
{
    GetNamespaceMap_().Add( GetXMLToken ( XML_NP_BLOCK_LIST ),
                            GetXMLToken ( XML_N_BLOCK_LIST ),
                            XML_NAMESPACE_BLOCKLIST );
}

ErrCode SvXMLExceptionListExport::exportDoc(enum XMLTokenEnum /*eClass*/)
{
    GetDocHandler()->startDocument();

    addChaffWhenEncryptedStorage();

    AddAttribute ( XML_NAMESPACE_NONE,
                   GetNamespaceMap_().GetAttrNameByKey ( XML_NAMESPACE_BLOCKLIST ),
                   GetNamespaceMap_().GetNameByKey ( XML_NAMESPACE_BLOCKLIST ) );
    {
        SvXMLElementExport aRoot (*this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK_LIST, true, true);
        sal_uInt16 nBlocks= rList.size();
        for ( sal_uInt16 i = 0; i < nBlocks; i++)
        {
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_ABBREVIATED_NAME,
                          rList[i] );
            SvXMLElementExport aBlock( *this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK, true, true);
        }
    }
    GetDocHandler()->endDocument();
    return ERRCODE_NONE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
