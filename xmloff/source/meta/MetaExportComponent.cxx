/*************************************************************************
 *
 *  $RCSfile: MetaExportComponent.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 18:20:31 $
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

#ifndef _XMLOFF_METAEXPORTCOMPONENT_HXX
#include "MetaExportComponent.hxx"
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP
#include <com/sun/star/uno/Exception.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLMETAE_HXX
#include "xmlmetae.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


using namespace ::com::sun::star;
using namespace ::xmloff::token;

XMLMetaExportComponent::XMLMetaExportComponent() :
    SvXMLExport( MAP_INCH, XML_META )
{
}

XMLMetaExportComponent::~XMLMetaExportComponent()
{
}

sal_uInt32 XMLMetaExportComponent::exportDoc( enum XMLTokenEnum eClass )
{
    GetDocHandler()->startDocument();
    {
        GetAttrList().AddAttribute(
            GetNamespaceMap().GetAttrNameByKey( XML_NAMESPACE_DC ),
            GetNamespaceMap().GetNameByKey( XML_NAMESPACE_DC ) );
        GetAttrList().AddAttribute(
            GetNamespaceMap().GetAttrNameByKey( XML_NAMESPACE_META ),
            GetNamespaceMap().GetNameByKey( XML_NAMESPACE_META ) );
        GetAttrList().AddAttribute(
            GetNamespaceMap().GetAttrNameByKey( XML_NAMESPACE_OFFICE ),
            GetNamespaceMap().GetNameByKey( XML_NAMESPACE_OFFICE ) );

        SvXMLElementExport aDocElem( *this, XML_NAMESPACE_OFFICE, XML_DOCUMENT_META,
                        sal_True, sal_True );
        {

            SvXMLElementExport aElem( *this, XML_NAMESPACE_OFFICE, XML_META,
                            sal_True, sal_True );
            SfxXMLMetaExport aMeta( *this, GetModel() );
            aMeta.Export();
        }
    }
    GetDocHandler()->endDocument();
    return 0;
}

// methods without content:
void XMLMetaExportComponent::_ExportAutoStyles() {}
void XMLMetaExportComponent::_ExportMasterStyles() {}
void XMLMetaExportComponent::_ExportContent() {}


uno::Sequence< rtl::OUString > SAL_CALL XMLMetaExportComponent_getSupportedServiceNames()
    throw()
{
    const rtl::OUString aServiceName(
        RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.XMLMetaExporter" ) );
    const uno::Sequence< rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

rtl::OUString SAL_CALL XMLMetaExportComponent_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XMLMetaExportComponent" ) );
}

uno::Reference< uno::XInterface > SAL_CALL XMLMetaExportComponent_createInstance(
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    return (cppu::OWeakObject*)new XMLMetaExportComponent;
}

