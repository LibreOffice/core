/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"
#include <SvXMLAutoCorrectExport.hxx>
#define _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>
#include <xmloff/xmltoken.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::rtl;

// #110680#
SvXMLAutoCorrectExport::SvXMLAutoCorrectExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const SvxAutocorrWordList *  pNewAutocorr_List,
    const rtl::OUString &rFileName,
    com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler> &rHandler)
:   SvXMLExport( xServiceFactory, rFileName, rHandler ),
    pAutocorr_List( pNewAutocorr_List )
{
    _GetNamespaceMap().Add( GetXMLToken ( XML_NP_BLOCK_LIST),
                            GetXMLToken ( XML_N_BLOCK_LIST ),
                            XML_NAMESPACE_BLOCKLIST );
}

sal_uInt32 SvXMLAutoCorrectExport::exportDoc(enum XMLTokenEnum /*eClass*/)
{
    GetDocHandler()->startDocument();

    AddAttribute ( XML_NAMESPACE_NONE,
                   _GetNamespaceMap().GetAttrNameByKey ( XML_NAMESPACE_BLOCKLIST ),
                   _GetNamespaceMap().GetNameByKey ( XML_NAMESPACE_BLOCKLIST ) );
    {
        SvXMLElementExport aRoot (*this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK_LIST, sal_True, sal_True);
        sal_uInt16 nBlocks= pAutocorr_List->Count();
        for ( sal_uInt16 i = 0; i < nBlocks; i++)
        {
            SvxAutocorrWord* p = pAutocorr_List->GetObject(i);

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

// #110680#
SvXMLExceptionListExport::SvXMLExceptionListExport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
    const SvStringsISortDtor &rNewList,
    const rtl::OUString &rFileName,
    com::sun::star::uno::Reference< com::sun::star::xml::sax::XDocumentHandler> &rHandler)
:   SvXMLExport( xServiceFactory, rFileName, rHandler ),
    rList( rNewList )
{
    _GetNamespaceMap().Add( GetXMLToken ( XML_NP_BLOCK_LIST ),
                            GetXMLToken ( XML_N_BLOCK_LIST ),
                            XML_NAMESPACE_BLOCKLIST );
}

sal_uInt32 SvXMLExceptionListExport::exportDoc(enum XMLTokenEnum /*eClass*/)
{
    GetDocHandler()->startDocument();

    AddAttribute ( XML_NAMESPACE_NONE,
                   _GetNamespaceMap().GetAttrNameByKey ( XML_NAMESPACE_BLOCKLIST ),
                   _GetNamespaceMap().GetNameByKey ( XML_NAMESPACE_BLOCKLIST ) );
    {
        SvXMLElementExport aRoot (*this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK_LIST, sal_True, sal_True);
        sal_uInt16 nBlocks= rList.Count();
        for ( sal_uInt16 i = 0; i < nBlocks; i++)
        {
            AddAttribute( XML_NAMESPACE_BLOCKLIST,
                          XML_ABBREVIATED_NAME,
                          OUString( *rList[i] ) );
            SvXMLElementExport aBlock( *this, XML_NAMESPACE_BLOCKLIST, XML_BLOCK, sal_True, sal_True);
        }
    }
    GetDocHandler()->endDocument();
    return 0;
}
