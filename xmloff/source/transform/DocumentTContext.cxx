/*************************************************************************
 *
 *  $RCSfile: DocumentTContext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:47:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
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

using namespace ::rtl;
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
            OUString aTmp( RTL_CONSTASCII_USTRINGPARAM("application/vnd.oasis.openoffice.") );
            if( 0 == rValue.compareTo( aTmp, aTmp.getLength() ) )
            {
                aClass = rValue.copy( aTmp.getLength() );
            }
            else
            {
                aTmp = OUString( RTL_CONSTASCII_USTRINGPARAM("application/x-vnd.oasis.openoffice.") );
                if( 0 == rValue.compareTo( aTmp, aTmp.getLength() ) )
                    aClass = rValue.copy( aTmp.getLength() );
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
