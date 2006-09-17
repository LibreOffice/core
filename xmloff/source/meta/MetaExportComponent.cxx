/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MetaExportComponent.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:38:57 $
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

// #110680#
//#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
//#include <comphelper/processfactory.hxx>
//#endif

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

// #110680#
XMLMetaExportComponent::XMLMetaExportComponent(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
        sal_uInt16 nFlags )
:   SvXMLExport( xServiceFactory, MAP_INCH, XML_META, nFlags )
{
}

XMLMetaExportComponent::~XMLMetaExportComponent()
{
}

sal_uInt32 XMLMetaExportComponent::exportDoc( enum XMLTokenEnum )
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
        RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.XMLOasisMetaExporter" ) );
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
    // #110680#
    // return (cppu::OWeakObject*)new XMLMetaExportComponent;
    return (cppu::OWeakObject*)new XMLMetaExportComponent(rSMgr, EXPORT_ALL|EXPORT_OASIS);
}

uno::Sequence< rtl::OUString > SAL_CALL XMLMetaExportOOO_getSupportedServiceNames()
    throw()
{
    const rtl::OUString aServiceName(
        RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.XMLMetaExporter" ) );
    const uno::Sequence< rtl::OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

rtl::OUString SAL_CALL XMLMetaExportOOO_getImplementationName() throw()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XMLMetaExportOOo" ) );
}

uno::Reference< uno::XInterface > SAL_CALL XMLMetaExportOOO_createInstance(
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr)
    throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new XMLMetaExportComponent;
    return (cppu::OWeakObject*)new XMLMetaExportComponent(rSMgr, EXPORT_ALL);
}

