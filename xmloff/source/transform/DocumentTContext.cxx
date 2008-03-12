/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentTContext.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:13:45 $
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
#include "precompiled_xmloff.hxx"

#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_SAXEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXException.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_TRANSFOERMERBASE_HXX
#include "TransformerBase.hxx"
#endif
#ifndef _XMLOFF_MUTABLEATTRLIST_HXX
#include "MutableAttrList.hxx"
#endif

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
