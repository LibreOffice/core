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
#include "precompiled_editeng.hxx"
#include <SvXMLAutoCorrectImport.hxx>
#include <vcl/svapp.hxx>

#define _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>
#include <xmloff/xmltoken.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::rtl;


static OUString sBlockList ( RTL_CONSTASCII_USTRINGPARAM ( "_block-list" ) );

SvXMLAutoCorrectImport::SvXMLAutoCorrectImport(
    const uno::Reference< lang::XMultiServiceFactory > xServiceFactory,
    SvxAutocorrWordList *pNewAutocorr_List,
    SvxAutoCorrect &rNewAutoCorrect,
    const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& rNewStorage)
:   SvXMLImport( xServiceFactory ),
    pAutocorr_List (pNewAutocorr_List),
    rAutoCorrect ( rNewAutoCorrect ),
    xStorage ( rNewStorage )
{
    GetNamespaceMap().Add(
            sBlockList,
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
    String sRight, sWrong;
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
    if (!sWrong.Len() || !sRight.Len() )
        return;

    BOOL bOnlyTxt = sRight != sWrong;
    if( !bOnlyTxt )
    {
        String sLongSave( sRight );
        if( !rLocalRef.rAutoCorrect.GetLongText( rLocalRef.xStorage, String(), sWrong, sRight ) &&
            sLongSave.Len() )
        {
            sRight = sLongSave;
            bOnlyTxt = TRUE;
        }
    }
    SvxAutocorrWordPtr pNew = new SvxAutocorrWord( sWrong, sRight, bOnlyTxt );

    if( !rLocalRef.pAutocorr_List->Insert( pNew ) )
        delete pNew;
}

SvXMLWordContext::~SvXMLWordContext ( void )
{
}

SvXMLExceptionListImport::SvXMLExceptionListImport(
    const uno::Reference< lang::XMultiServiceFactory > xServiceFactory,
    SvStringsISortDtor & rNewList )
:   SvXMLImport( xServiceFactory ),
    rList (rNewList)
{
    GetNamespaceMap().Add(
            sBlockList,
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
    String sWord;
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
    if (!sWord.Len() )
        return;

    String * pNew = new String( sWord );

    if( !rLocalRef.rList.Insert( pNew ) )
        delete pNew;
}

SvXMLExceptionContext::~SvXMLExceptionContext ( void )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
