/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLAutoTextEventImport.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:02:16 $
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

#ifndef _XMLOFF_XMLAUTOTEXTEVENTIMPORT_HXX
#include "XMLAutoTextEventImport.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _XMLOFF_XMLAUTOTEXTCONTAINEREVENTIMPORT_HXX
#include "XMLAutoTextContainerEventImport.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Type;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::document::XEventsSupplier;
using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_AUTO_TEXT_EVENTS;

const sal_Char sAPI_AutoText[] = "com.sun.star.text.AutoTextContainer";


// #110680#
XMLAutoTextEventImport::XMLAutoTextEventImport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory) throw()
:   SvXMLImport(xServiceFactory)
{
}

XMLAutoTextEventImport::~XMLAutoTextEventImport() throw()
{
}

void XMLAutoTextEventImport::initialize(
    const Sequence<Any> & rArguments )
        throw(Exception, RuntimeException)
{
    // The events may come as either an XNameReplace or XEventsSupplier.

    const sal_Int32 nLength = rArguments.getLength();
    for( sal_Int32 i = 0; i < nLength; i++ )
    {
        const Type& rType = rArguments[i].getValueType();
        if ( rType == ::getCppuType( (Reference<XEventsSupplier>*)NULL ) )
        {
            Reference<XEventsSupplier> xSupplier;
            rArguments[i] >>= xSupplier;
            DBG_ASSERT(xSupplier.is(), "need XEventsSupplier or XNameReplace");

            xEvents = xSupplier->getEvents();
        }
        else if (rType == ::getCppuType( (Reference<XNameReplace>*)NULL ) )
        {
            rArguments[i] >>= xEvents;
            DBG_ASSERT(xEvents.is(), "need XEventsSupplier or XNameReplace");
        }
    }

    // call parent
    SvXMLImport::initialize(rArguments);
}



SvXMLImportContext* XMLAutoTextEventImport::CreateContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList > & xAttrList )
{
    if ( xEvents.is() && (XML_NAMESPACE_OOO == nPrefix) &&
         IsXMLToken( rLocalName, XML_AUTO_TEXT_EVENTS) )
    {
        return new XMLAutoTextContainerEventImport(
            *this, nPrefix, rLocalName, xEvents);
    }
    else
    {
        return SvXMLImport::CreateContext(nPrefix, rLocalName, xAttrList);
    }
}


Sequence< OUString > SAL_CALL
    XMLAutoTextEventImport_getSupportedServiceNames()
        throw()
{
    Sequence< OUString > aSeq( 1 );
    aSeq[0] = XMLAutoTextEventImport_getImplementationName();
    return aSeq;
}

OUString SAL_CALL XMLAutoTextEventImport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.Writer.XMLOasisAutotextEventsImporter" ) );
}

Reference< XInterface > SAL_CALL XMLAutoTextEventImport_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new XMLAutoTextEventImport;
    return (cppu::OWeakObject*)new XMLAutoTextEventImport(rSMgr);
}

