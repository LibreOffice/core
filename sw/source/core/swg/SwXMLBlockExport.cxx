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
#include "precompiled_sw.hxx"
#include <SwXMLBlockExport.hxx>
#include <SwXMLTextBlocks.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;
using ::rtl::OUString;

// #110680#
SwXMLBlockListExport::SwXMLBlockListExport(
    const uno::Reference< lang::XMultiServiceFactory > xServiceFactory,
    SwXMLTextBlocks & rBlocks,
    const rtl::OUString &rFileName,
    uno::Reference< xml::sax::XDocumentHandler> &rHandler)
:   SvXMLExport( xServiceFactory, rFileName, rHandler ),
    rBlockList(rBlocks)
{
    _GetNamespaceMap().Add( GetXMLToken ( XML_NP_BLOCK_LIST ),
                            GetXMLToken ( XML_N_BLOCK_LIST ),
                            XML_NAMESPACE_BLOCKLIST );
}

sal_uInt32 SwXMLBlockListExport::exportDoc(enum XMLTokenEnum )
{
    GetDocHandler()->startDocument();

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
    const uno::Reference< lang::XMultiServiceFactory > xServiceFactory,
    SwXMLTextBlocks & rBlocks,
    const rtl::OUString &rFileName,
    uno::Reference< xml::sax::XDocumentHandler> &rHandler)
:   SvXMLExport( xServiceFactory, rFileName, rHandler ),
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

sal_uInt32 SwXMLTextBlockExport::exportDoc(const String &rText)
{
    GetDocHandler()->startDocument();

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
                xub_StrLen nPos = 0;
                do
                {
                    String sTemp ( rText.GetToken( 0, '\015', nPos ) );
                     SvXMLElementExport aPara (*this, XML_NAMESPACE_TEXT, XML_P, sal_True, sal_False);
                    GetDocHandler()->characters(sTemp);
                } while (STRING_NOTFOUND != nPos );
            }

        }
    }
    GetDocHandler()->endDocument();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
