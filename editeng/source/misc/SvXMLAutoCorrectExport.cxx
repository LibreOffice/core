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

#include <SvXMLAutoCorrectExport.hxx>

#include <com/sun/star/util/MeasureUnit.hpp>
#include <xmloff/xmltoken.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::rtl;

SvXMLAutoCorrectExport::SvXMLAutoCorrectExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
    const SvxAutocorrWordList *  pNewAutocorr_List,
    const OUString &rFileName,
    com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler> &rHandler)
:   SvXMLExport( xContext, rFileName, util::MeasureUnit::CM, rHandler ),
    pAutocorr_List( pNewAutocorr_List )
{
    _GetNamespaceMap().Add( GetXMLToken ( XML_NP_BLOCK_LIST),
                            GetXMLToken ( XML_N_BLOCK_LIST ),
                            XML_NAMESPACE_BLOCKLIST );
}

sal_uInt32 SvXMLAutoCorrectExport::exportDoc(enum XMLTokenEnum /*eClass*/)
{
    GetDocHandler()->startDocument();

    addChaffWhenEncryptedStorage();

    AddAttribute ( XML_NAMESPACE_NONE,
                   _GetNamespaceMap().GetAttrNameByKey ( XML_NAMESPACE_BLOCKLIST ),
                   _GetNamespaceMap().GetNameByKey ( XML_NAMESPACE_BLOCKLIST ) );
    {
        SvXMLElementExport aRoot (*this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK_LIST, sal_True, sal_True);
        SvxAutocorrWordList::Content aContent = pAutocorr_List->getSortedContent();
        for( SvxAutocorrWordList::Content::iterator it = aContent.begin();
             it != aContent.end(); ++it )
        {
            const SvxAutocorrWord* p = *it;

            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_ABBREVIATED_NAME,
                          OUString(p->GetShort()));
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_NAME,
                          OUString(p->IsTextOnly() ? p->GetLong() : p->GetShort()));

            SvXMLElementExport aBlock( *this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK, sal_True, sal_True);
        }
    }
    GetDocHandler()->endDocument();
    return 0;
}

SvXMLExceptionListExport::SvXMLExceptionListExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext,
    const SvStringsISortDtor &rNewList,
    const OUString &rFileName,
    com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler> &rHandler)
:   SvXMLExport( xContext, rFileName, util::MeasureUnit::CM, rHandler ),
    rList( rNewList )
{
    _GetNamespaceMap().Add( GetXMLToken ( XML_NP_BLOCK_LIST ),
                            GetXMLToken ( XML_N_BLOCK_LIST ),
                            XML_NAMESPACE_BLOCKLIST );
}

sal_uInt32 SvXMLExceptionListExport::exportDoc(enum XMLTokenEnum /*eClass*/)
{
    GetDocHandler()->startDocument();

    addChaffWhenEncryptedStorage();

    AddAttribute ( XML_NAMESPACE_NONE,
                   _GetNamespaceMap().GetAttrNameByKey ( XML_NAMESPACE_BLOCKLIST ),
                   _GetNamespaceMap().GetNameByKey ( XML_NAMESPACE_BLOCKLIST ) );
    {
        SvXMLElementExport aRoot (*this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK_LIST, sal_True, sal_True);
        sal_uInt16 nBlocks= rList.size();
        for ( sal_uInt16 i = 0; i < nBlocks; i++)
        {
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_ABBREVIATED_NAME,
                          rList[i] );
            SvXMLElementExport aBlock( *this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK, sal_True, sal_True);
        }
    }
    GetDocHandler()->endDocument();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
