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

#include <SvXMLAutoCorrectImport.hxx>
#include <vcl/svapp.hxx>
#include <xmloff/xmltoken.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::rtl;

const char aBlockList[] =  "_block-list";

SvXMLAutoCorrectImport::SvXMLAutoCorrectImport(
    const uno::Reference< uno::XComponentContext > xContext,
    SvxAutocorrWordList *pNewAutocorr_List,
    SvxAutoCorrect &rNewAutoCorrect,
    const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rNewStorage)
:   SvXMLImport( xContext ),
    pAutocorr_List (pNewAutocorr_List),
    rAutoCorrect ( rNewAutoCorrect ),
    xStorage ( rNewStorage )
{
    GetNamespaceMap().Add(
            OUString(aBlockList),
            GetXMLToken ( XML_N_BLOCK_LIST),
            XML_NAMESPACE_BLOCKLIST );
}

SvXMLAutoCorrectImport::~SvXMLAutoCorrectImport ( void ) throw ()
{
}

SvXMLImportContext *SvXMLAutoCorrectImport::CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_BLOCKLIST == nPrefix &&
        IsXMLToken ( rLocalName, XML_BLOCK_LIST ) )
        pContext = new SvXMLWordListContext( *this, nPrefix, rLocalName, xAttrList );
    else
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );
    return pContext;
}

SvXMLWordListContext::SvXMLWordListContext(
   SvXMLAutoCorrectImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & /*xAttrList*/ ) :
   SvXMLImportContext ( rImport, nPrefix, rLocalName ),
   rLocalRef(rImport)
{
}

SvXMLImportContext *SvXMLWordListContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_BLOCKLIST &&
        IsXMLToken ( rLocalName, XML_BLOCK ) )
        pContext = new SvXMLWordContext (rLocalRef, nPrefix, rLocalName, xAttrList);
    else
        pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName);
    return pContext;
}
SvXMLWordListContext::~SvXMLWordListContext ( void )
{
}

SvXMLWordContext::SvXMLWordContext(
   SvXMLAutoCorrectImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
   SvXMLImportContext ( rImport, nPrefix, rLocalName ),
   rLocalRef(rImport)
{
    OUString sRight;
    OUString sWrong;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    for (sal_Int16 i=0; i < nAttrCount; i++)
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nAttrPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName);
        const OUString& rAttrValue = xAttrList->getValueByIndex( i );
        if (XML_NAMESPACE_BLOCKLIST == nAttrPrefix)
        {
            if ( IsXMLToken ( aLocalName, XML_ABBREVIATED_NAME ) )
            {
                sWrong = rAttrValue;
            }
            else if ( IsXMLToken ( aLocalName, XML_NAME ) )
            {
                sRight = rAttrValue;
            }
        }
    }
    if (sWrong.isEmpty() || sRight.isEmpty())
        return;

    sal_Bool bOnlyTxt = sRight != sWrong;
    if( !bOnlyTxt )
    {
        const OUString sLongSave( sRight );
        if( !rLocalRef.rAutoCorrect.GetLongText( rLocalRef.xStorage, OUString(), sWrong, sRight ) &&
            !sLongSave.isEmpty() )
        {
            sRight = sLongSave;
            bOnlyTxt = sal_True;
        }
    }
    rLocalRef.pAutocorr_List->LoadEntry( sWrong, sRight, bOnlyTxt );
}

SvXMLWordContext::~SvXMLWordContext ( void )
{
}

SvXMLExceptionListImport::SvXMLExceptionListImport(
    const uno::Reference< uno::XComponentContext > xContext,
    SvStringsISortDtor & rNewList )
:   SvXMLImport( xContext ),
    rList (rNewList)
{
    GetNamespaceMap().Add(
            OUString(aBlockList),
            GetXMLToken ( XML_N_BLOCK_LIST),
            XML_NAMESPACE_BLOCKLIST );
}

SvXMLExceptionListImport::~SvXMLExceptionListImport ( void ) throw ()
{
}

SvXMLImportContext *SvXMLExceptionListImport::CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_BLOCKLIST==nPrefix &&
        IsXMLToken ( rLocalName, XML_BLOCK_LIST ) )
        pContext = new SvXMLExceptionListContext( *this, nPrefix, rLocalName, xAttrList );
    else
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );
    return pContext;
}

SvXMLExceptionListContext::SvXMLExceptionListContext(
   SvXMLExceptionListImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & /* xAttrList */ ) :
   SvXMLImportContext ( rImport, nPrefix, rLocalName ),
   rLocalRef(rImport)
{
}

SvXMLImportContext *SvXMLExceptionListContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if (nPrefix == XML_NAMESPACE_BLOCKLIST &&
        IsXMLToken ( rLocalName, XML_BLOCK ) )
        pContext = new SvXMLExceptionContext (rLocalRef, nPrefix, rLocalName, xAttrList);
    else
        pContext = new SvXMLImportContext( rLocalRef, nPrefix, rLocalName);
    return pContext;
}
SvXMLExceptionListContext::~SvXMLExceptionListContext ( void )
{
}

SvXMLExceptionContext::SvXMLExceptionContext(
   SvXMLExceptionListImport& rImport,
   sal_uInt16 nPrefix,
   const OUString& rLocalName,
   const com::sun::star::uno::Reference<
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
   SvXMLImportContext ( rImport, nPrefix, rLocalName ),
   rLocalRef(rImport)
{
    OUString sWord;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    for (sal_Int16 i=0; i < nAttrCount; i++)
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nAttrPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName);
        const OUString& rAttrValue = xAttrList->getValueByIndex( i );
        if (XML_NAMESPACE_BLOCKLIST == nAttrPrefix)
        {
            if ( IsXMLToken ( aLocalName, XML_ABBREVIATED_NAME ) )
            {
                sWord = rAttrValue;
            }
        }
    }
    if (sWord.isEmpty() )
        return;

    rLocalRef.rList.insert( sWord );
}

SvXMLExceptionContext::~SvXMLExceptionContext ( void )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
