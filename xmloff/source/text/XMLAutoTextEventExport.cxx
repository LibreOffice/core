/*************************************************************************
 *
 *  $RCSfile: XMLAutoTextEventExport.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dvo $ $Date: 2001-02-13 16:55:00 $
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

#ifndef _COM_SUN_STAR_TEXT_XAUTOTEXTCONTAINER_HPP_
#include <com/sun/star/text/XAutoTextContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XAUTOTEXTGROUP_HPP_
#include <com/sun/star/text/XAutoTextGroup.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XAUTOTEXTENTRY_HPP_
#include <com/sun/star/text/XAutoTextEntry.hpp>
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
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::document::XEventsSupplier;
using ::com::sun::star::frame::XModel;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::text::XAutoTextContainer;
using ::com::sun::star::text::XAutoTextEntry;
using ::com::sun::star::text::XAutoTextGroup;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::xml::sax::XDocumentHandler;


const sal_Char sAPI_AutoText[] = "com.sun.star.text.AutoTextContainer";

XMLAutoTextEventExport::XMLAutoTextEventExport() :
    SvXMLExport( MAP_INCH, sXML_auto_text ),
    eventCount( NULL ),
    rGroupNames( * new Sequence<OUString> )
{
}

XMLAutoTextEventExport::XMLAutoTextEventExport(
    const OUString& rFileName,
    const Reference<XDocumentHandler> & rHandler,
    const Reference<XModel> & rModel) :
        SvXMLExport(rFileName, rHandler, rModel, MAP_INCH),
        eventCount( NULL ),
        rGroupNames( * new Sequence<OUString> )
{
}

XMLAutoTextEventExport::~XMLAutoTextEventExport()
{
    delete eventCount;
}

// export the events off all autotexts
sal_uInt32 XMLAutoTextEventExport::exportDoc( const sal_Char *pClass )
{
    // instantiate AutoTextContainer via factory
    Reference<XMultiServiceFactory> xFactory = getProcessServiceFactory();
    if (xFactory.is())
    {
        OUString sService(RTL_CONSTASCII_USTRINGPARAM(sAPI_AutoText));
        Reference<XAutoTextContainer> xAutoTextContainer(
            xFactory->createInstance(sService), uno::UNO_QUERY);

        if (xAutoTextContainer.is())
        {
            // export all, or only the name groups?
            Sequence<OUString> & rNames =
                (rGroupNames.getLength() > 0) ? rGroupNames :
                xAutoTextContainer->getElementNames();

            countEvents(xAutoTextContainer, rNames);
            exportAutoTextContainer(pClass, xAutoTextContainer, rNames);
        }
    }
    return 0;
}

void XMLAutoTextEventExport::exportAutoTextContainer(
    const sal_Char *pClass,
    Reference<XAutoTextContainer> & rAutoTextContainer,
    Sequence<OUString> & rNames)
{
    if (hasDocumentEvents())
    {
        GetDocHandler()->startDocument();

    // do we really want all namespaces ?
//  sal_uInt16 nPos = pNamespaceMap->GetFirstIndex();
//  while( USHRT_MAX != nPos )
//  {
//      pAttrList->AddAttribute( pNamespaceMap->GetAttrNameByIndex( nPos ),
//                               sCDATA,
//                               pNamespaceMap->GetNameByIndex( nPos ) );
//      nPos = pNamespaceMap->GetNextIndex( nPos );
//  }

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

        {
            // container element
            SvXMLElementExport aContainerElement(
                *this, XML_NAMESPACE_TEXT, sXML_auto_text_events,
                sal_True, sal_True);

            // iterate over group list
            sal_Int32 nCount = rNames.getLength();
            for(sal_Int32 i = 0; i < nCount; i++)
            {
                OUString& rName = rNames[i];
                Any aAny = rAutoTextContainer->getByName(rName);
                Reference<XAutoTextGroup> xGroup;
                aAny >>= xGroup;

                exportAutoTextGroup(rName, xGroup);
            }
        }

        // and close document again
        GetDocHandler()->endDocument();
    }
}

void XMLAutoTextEventExport::exportAutoTextGroup(
    OUString& rGroupName,
    Reference<XAutoTextGroup> & rGroup)
{
    DBG_ASSERT(rGroup.is(), "Where is the group?");

    if (hasGroupEvents(rGroupName))
    {
        AddAttribute(XML_NAMESPACE_TEXT, sXML_group_name, rGroupName);
        SvXMLElementExport aGroupElement(
            *this, XML_NAMESPACE_TEXT, sXML_auto_text_group,
            sal_True, sal_True);

        // iterate over all autotexts in this group
        Sequence<OUString> aNames = rGroup->getElementNames();
        sal_Int32 nCount = aNames.getLength();
        for(sal_Int32 i = 0; i < nCount; i++)
        {
            OUString& rTextName = aNames[i];

            Any aAny = rGroup->getByName(rTextName);
            Reference<XAutoTextEntry> xEntry;
            aAny >>= xEntry;

            exportAutoTextEntry(rGroupName, rTextName, xEntry);
        }
    }
}

void XMLAutoTextEventExport::exportAutoTextEntry(
    OUString& rGroupName,
    OUString& rName,
    Reference<XAutoTextEntry> & rEntry)
{
    if (hasEntryEvents(rGroupName, rName))
    {
        Reference<XEventsSupplier> xEventsSupp(rEntry, uno::UNO_QUERY);
        DBG_ASSERT(xEventsSupp.is(),
                   "XEventsSupplier is required by service descr.");

        AddAttribute(XML_NAMESPACE_TEXT, sXML_name, rName);
        SvXMLElementExport aElem(*this, XML_NAMESPACE_TEXT, sXML_auto_text,
                                 sal_True, sal_True);

        GetEventExport().Export(xEventsSupp, sal_True);
    }
}




void XMLAutoTextEventExport::countEvents(
    Reference<XAutoTextContainer> & rAutoTextContainer,
    Sequence<OUString> & rNames)
{
    OUString sEventType(RTL_CONSTASCII_USTRINGPARAM("EventType"));
    OUString sNone(RTL_CONSTASCII_USTRINGPARAM("None"));

    set<OUString> * pEventCount = new set<OUString> ;

    // iterate over all groups
    sal_Int32 nGroupsCount = rNames.getLength();
    for(sal_Int32 i = 0; i < nGroupsCount; i++)
    {
        OUString& rGroupName = rNames[i];
        Any aAny = rAutoTextContainer->getByName(rGroupName);
        Reference<XAutoTextGroup> xGroup;
        aAny >>= xGroup;

        // iterate over all autotexts in this group
        Sequence<OUString> aEntryNames = xGroup->getElementNames();
        sal_Int32 nEntryCount = aEntryNames.getLength();
        for(sal_Int32 i = 0; i < nEntryCount; i++)
        {
            OUString& rEntryName = aEntryNames[i];

            Any aAny = xGroup->getByName(rEntryName);
            Reference<XAutoTextEntry> xEntry;
            aAny >>= xEntry;

            // iterate over all events of the autotext
            Reference<XEventsSupplier> xSupplier(xEntry, uno::UNO_QUERY);
            Reference<XNameReplace> xEvents = xSupplier->getEvents();
            Sequence<OUString> aEventNames = xEvents->getElementNames();
            sal_Int32 nEventsCount = aEventNames.getLength();
            for(sal_Int32 i = 0; i < nEventsCount; i++)
            {
                OUString& rEventName = aEventNames[i];

                Any aAny = xEvents->getByName(rEventName);
                Sequence<PropertyValue> aValues;
                aAny >>= aValues;

                // now, finally, iterate over values and find a non-empty macro
                sal_Int32 nValuesCount = aValues.getLength();
                for(sal_Int32 i = 0; i < nValuesCount; i++)
                {
                    if (aValues[i].Name == sEventType)
                    {
                        OUString sType;
                        aValues[i].Value >>= sType;

                        // non-empty macro?
                        if (sType != sNone)
                        {
                            // Wow! We found a event. Let's celebrate!

                            // insert the group + the event
                            pEventCount->insert(rGroupName);
                            pEventCount->insert(
                                combinedName(rGroupName, rEntryName));
                        }
                    }
                }
            }
        }
    }

    eventCount = pEventCount;
}

sal_Bool XMLAutoTextEventExport::hasDocumentEvents()
{
    return (NULL == eventCount) ? sal_True : (eventCount->size() != 0);
}


sal_Bool XMLAutoTextEventExport::hasGroupEvents(
    const OUString& rGroupName  )
{
    return (NULL == eventCount) ? sal_True :
        (eventCount->find(rGroupName) != eventCount->end());
}

sal_Bool XMLAutoTextEventExport::hasEntryEvents(
    const OUString& rGroupName,
    const OUString& rEntryName )
{
    return (NULL == eventCount) ? sal_True :
        (eventCount->find(combinedName(rGroupName, rEntryName))
                != eventCount->end());
}



OUString XMLAutoTextEventExport::combinedName(
    const OUString& rGroupName,
    const OUString& rEntryName)
{
    OUStringBuffer sBuf;
    sBuf.append(rGroupName);
    sBuf.append(sal_Unicode('#'));
    sBuf.append(rEntryName);
    return sBuf.makeStringAndClear();
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


Sequence< OUString > SAL_CALL XMLAutoTextEventExport_getSupportedServiceNames()
    throw()
{
    const OUString aServiceName(
        RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.office.sax.exporter.AutoTextEventWriter" ) );
    const Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL XMLAutoTextEventExport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "XMLAutoTextEventExport" ) );
}

Reference< XInterface > SAL_CALL XMLAutoTextEventExport_createInstance(
        const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*)new XMLAutoTextEventExport;
}

