/*************************************************************************
 *
 *  $RCSfile: XMLAutoTextEventExport.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dvo $ $Date: 2001-03-09 14:53:43 $
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

#ifndef _XMLOFF_XMLAUTOTEXTEVENTEXPORT_HXX
#include "XMLAutoTextEventExport.hxx"
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

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
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

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLEVENTEXPORT_HXX
#include "XMLEventExport.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


using namespace ::com::sun::star;

using ::comphelper::getProcessServiceFactory;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::std::set;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::document::XEventsSupplier;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::xml::sax::XDocumentHandler;


const sal_Char sAPI_AutoText[] = "com.sun.star.text.AutoTextContainer";


XMLAutoTextEventExport::XMLAutoTextEventExport() :
        SvXMLExport( MAP_INCH, sXML_auto_text ),
        sEventType(RTL_CONSTASCII_USTRINGPARAM("EventType")),
        sNone(RTL_CONSTASCII_USTRINGPARAM("None"))
{
}

XMLAutoTextEventExport::XMLAutoTextEventExport(
    const OUString& rFileName,
    const Reference<XDocumentHandler> & rHandler,
    const Reference<XModel> & rModel,
    const Reference<XNameAccess> & rEvents) :
        SvXMLExport(rFileName, rHandler, rModel, MAP_INCH),
        xEvents(rEvents),
        sEventType(RTL_CONSTASCII_USTRINGPARAM("EventType")),
        sNone(RTL_CONSTASCII_USTRINGPARAM("None"))
{
}

XMLAutoTextEventExport::~XMLAutoTextEventExport()
{
}


void XMLAutoTextEventExport::initialize(
    const Sequence<Any> & rArguments )
        throw(uno::Exception, uno::RuntimeException)
{
    if (rArguments.getLength() > 1)
    {
        Reference<XEventsSupplier> xSupplier;
        rArguments[1] >>= xSupplier;
        if (xSupplier.is())
        {
            Reference<XNameAccess> xAccess(xSupplier->getEvents(), UNO_QUERY);
            xEvents = xAccess;
        }
        else
        {
            Reference<XNameReplace> xReplace;
            rArguments[1] >>= xReplace;
            if (xReplace.is())
            {
                Reference<XNameAccess> xAccess(xReplace, UNO_QUERY);
                xEvents = xAccess;
            }
            else
            {
                rArguments[1] >>= xEvents;
            }
        }
    }

    // call super class (for XHandler)
    SvXMLExport::initialize(rArguments);
}


sal_uInt32 XMLAutoTextEventExport::exportDoc(
    const sal_Char* pClass)
{
    if (hasEvents())
    {
        GetDocHandler()->startDocument();

        addNamespaces();

        {
            // container element
            SvXMLElementExport aContainerElement(
                *this, XML_NAMESPACE_OFFICE, sXML_auto_text_events,
                sal_True, sal_True);

            exportEvents();
        }

        // and close document again
        GetDocHandler()->endDocument();
    }

    return 0;
}

sal_Bool XMLAutoTextEventExport::hasEvents()
{
    // TODO: provide full implementation that check for presence of events
    return xEvents.is();
}

void XMLAutoTextEventExport::addNamespaces()
{
    // namespaces for office:, text: and script:
    GetAttrList().AddAttribute(
        GetNamespaceMap().GetAttrNameByIndex( XML_NAMESPACE_TEXT ),
        sCDATA, GetNamespaceMap().GetNameByIndex( XML_NAMESPACE_TEXT ) );
    GetAttrList().AddAttribute(
        GetNamespaceMap().GetAttrNameByIndex( XML_NAMESPACE_SCRIPT ),
        sCDATA, GetNamespaceMap().GetNameByIndex( XML_NAMESPACE_SCRIPT ) );
    GetAttrList().AddAttribute(
        GetNamespaceMap().GetAttrNameByIndex( XML_NAMESPACE_OFFICE ),
        sCDATA, GetNamespaceMap().GetNameByIndex( XML_NAMESPACE_OFFICE ) );
}

void XMLAutoTextEventExport::exportEvents()
{
    DBG_ASSERT(hasEvents(), "no events to export!");

    GetEventExport().Export(xEvents, sal_True);
}



// methods without content:

void XMLAutoTextEventExport::_ExportMeta() {}
void XMLAutoTextEventExport::_ExportScripts() {}
void XMLAutoTextEventExport::_ExportFontDecls() {}
void XMLAutoTextEventExport::_ExportStyles( sal_Bool bUsed ) {}
void XMLAutoTextEventExport::_ExportAutoStyles() {}
void XMLAutoTextEventExport::_ExportMasterStyles() {}
void XMLAutoTextEventExport::_ExportChangeTracking() {}
void XMLAutoTextEventExport::_ExportContent() {}



// methods to support the component registration

Sequence< OUString > SAL_CALL XMLAutoTextEventExport_getSupportedServiceNames()
    throw()
{
    Sequence< OUString > aSeq( 1 );
    aSeq[0] = XMLAutoTextEventExport_getImplementationName();
    return aSeq;
}

OUString SAL_CALL XMLAutoTextEventExport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.Writer.XMLAutotextEventsExporter" ) );
}

Reference< XInterface > SAL_CALL XMLAutoTextEventExport_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new XMLAutoTextEventExport;
}

