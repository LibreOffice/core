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
#include "precompiled_xmloff.hxx"
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlnmspe.hxx"

#ifndef _XMLOFF_TRANSFOERMERBASE_HXX
#include "TransformerBase.hxx"
#endif
#include "MutableAttrList.hxx"

#ifndef _XMLOFF_METATCONTEXT_HXX
#include "DocumentTContext.hxx"
#endif

using ::rtl::OUString;

using namespace ::xmloff::token;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::beans;

TYPEINIT1( XMLDocumentTransformerContext, XMLTransformerContext );

XMLDocumentTransformerContext::XMLDocumentTransformerContext( XMLTransformerBase& rImp,
                                                const OUString& rQName ) :
    XMLTransformerContext( rImp, rQName )
{
}

XMLDocumentTransformerContext::~XMLDocumentTransformerContext()
{
}

void XMLDocumentTransformerContext::StartElement( const Reference< XAttributeList >& rAttrList )
{
    Reference< XAttributeList > xAttrList( rAttrList );

    sal_Bool bMimeFound = sal_False;
    OUString aClass;
    OUString aClassQName(
                    GetTransformer().GetNamespaceMap().GetQNameByKey(
                                XML_NAMESPACE_OFFICE, GetXMLToken(XML_CLASS ) ) );

    XMLMutableAttributeList *pMutableAttrList = 0;
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix =
            GetTransformer().GetNamespaceMap().GetKeyByAttrName( rAttrName,
                                                                 &aLocalName );
        if( XML_NAMESPACE_OFFICE == nPrefix &&
            IsXMLToken( aLocalName, XML_MIMETYPE ) )
        {
            const OUString& rValue = xAttrList->getValueByIndex( i );
            static const char * aTmp[] =
            {
                "application/vnd.oasis.openoffice.",
                "application/x-vnd.oasis.openoffice.",
                "application/vnd.oasis.opendocument.",
                "application/x-vnd.oasis.document.",
                NULL
            };
            for (int k=0; aTmp[k]; k++)
            {
                ::rtl::OUString sTmpString = ::rtl::OUString::createFromAscii(aTmp[k]);
                if( rValue.matchAsciiL( aTmp[k], sTmpString.getLength() ) )
                {
                    aClass = rValue.copy( sTmpString.getLength() );
                    break;
                }
            }

            if( !pMutableAttrList )
            {
                pMutableAttrList = new XMLMutableAttributeList( xAttrList );
                xAttrList = pMutableAttrList;
            }
            pMutableAttrList->SetValueByIndex( i, aClass );
            pMutableAttrList->RenameAttributeByIndex(i, aClassQName );
            bMimeFound = sal_True;
            break;
        }
    }

    if( !bMimeFound )
    {
        const Reference< XPropertySet > rPropSet =
            GetTransformer().GetPropertySet();

        if( rPropSet.is() )
        {
            Reference< XPropertySetInfo > xPropSetInfo(
                rPropSet->getPropertySetInfo() );
            OUString aPropName(RTL_CONSTASCII_USTRINGPARAM("Class"));
            if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName( aPropName ) )
            {
                Any aAny = rPropSet->getPropertyValue( aPropName );
                aAny >>= aClass;
            }
        }

        if( aClass.getLength() )
        {
            if( !pMutableAttrList )
            {
                pMutableAttrList = new XMLMutableAttributeList( xAttrList );
                xAttrList = pMutableAttrList;
            }

            pMutableAttrList->AddAttribute( aClassQName, aClass );
        }
    }
    XMLTransformerContext::StartElement( xAttrList );
}
