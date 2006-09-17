/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SvXMLAutoCorrectImport.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:47:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#ifndef _SV_XMLAUTOCORRECTIMPORT_HXX
#include <SvXMLAutoCorrectImport.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#define _SVSTDARR_STRINGSISORTDTOR
#define _SVSTDARR_STRINGSDTOR
#include <svtools/svstdarr.hxx>
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;
using namespace ::rtl;

static OUString sBlockList ( RTL_CONSTASCII_USTRINGPARAM ( "_block-list" ) );

// #110680#
SvXMLAutoCorrectImport::SvXMLAutoCorrectImport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
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
        const Reference< xml::sax::XAttributeList > & xAttrList )
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
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
   rLocalRef(rImport),
   SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
}

SvXMLImportContext *SvXMLWordListContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList > & xAttrList )
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
   rLocalRef(rImport),
   SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
    String sRight, sWrong;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    for (sal_Int16 i=0; i < nAttrCount; i++)
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName);
        const OUString& rAttrValue = xAttrList->getValueByIndex( i );
        if (XML_NAMESPACE_BLOCKLIST == nPrefix)
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

//  const International& rInter = Application::GetAppInternational();
//  BOOL bOnlyTxt = COMPARE_EQUAL != rInter.Compare( sRight, sWrong, INTN_COMPARE_IGNORECASE );
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

// #110680#
SvXMLExceptionListImport::SvXMLExceptionListImport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
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
        const Reference< xml::sax::XAttributeList > & xAttrList )
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
   com::sun::star::xml::sax::XAttributeList > & xAttrList ) :
   rLocalRef(rImport),
   SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
}

SvXMLImportContext *SvXMLExceptionListContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference< xml::sax::XAttributeList > & xAttrList )
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
   rLocalRef(rImport),
   SvXMLImportContext ( rImport, nPrefix, rLocalName )
{
    String sWord;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;

    for (sal_Int16 i=0; i < nAttrCount; i++)
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName);
        const OUString& rAttrValue = xAttrList->getValueByIndex( i );
        if (XML_NAMESPACE_BLOCKLIST == nPrefix)
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
