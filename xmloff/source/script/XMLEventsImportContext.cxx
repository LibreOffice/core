/*************************************************************************
 *
 *  $RCSfile: XMLEventsImportContext.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2000-12-19 18:56:45 $
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

#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#include "XMLEventsImportContext.hxx"
#endif

#ifndef _XMLOFF_XMLEVENTIMPORTHELPER_HXX
#include "XMLEventImportHelper.hxx"
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSUPPLIER_HPP
#include <com/sun/star/document/XEventSupplier.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif



using namespace ::com::sun::star::uno;

using ::rtl::OUString;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::container::XNameReplace;
using ::com::sun::star::document::XEventSupplier;


TYPEINIT1(XMLEventsImportContext,  SvXMLImportContext);

XMLEventsImportContext::XMLEventsImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const Reference<XEventSupplier> & xEventSupplier) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        xEvents(xEventSupplier->getEvents())
{
}


XMLEventsImportContext::XMLEventsImportContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const Reference<XNameReplace> & xNameReplace) :
        SvXMLImportContext(rImport, nPrfx, rLocalName),
        xEvents(xNameReplace)
{
}


void XMLEventsImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    // nothing to be done
}

void XMLEventsImportContext::EndElement()
{
    // nothing to be done
}

SvXMLImportContext* XMLEventsImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList )
{
    // a) search for script:language and script:event-name attribute
    // b) delegate to factory. The factory will:
    //    1) translate XML event name into API event name
    //    2) get proper event context factory from import
    //    3) instantiate context

    // a) search for script:language and script:event-name attribute
    OUString sLanguage;
    OUString sEventName;
    sal_Int16 nCount = xAttrList->getLength();
    for (sal_Int16 nAttr = 0; nAttr < nCount; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), &sLocalName );

        if (XML_NAMESPACE_SCRIPT == nPrefix)
        {
            if (sLocalName.equalsAsciiL(sXML_event_name,
                                        sizeof(sXML_event_name)-1))
            {
                sEventName = xAttrList->getValueByIndex(nAttr);
            }
            else if (sLocalName.equalsAsciiL(sXML_language,
                                             sizeof(sXML_language)-1))
            {
                sLanguage = xAttrList->getValueByIndex(nAttr);
            }
            // else: ignore -> let child context handle this
        }
        // else: ignore -> let child context handle this
    }

    // b) delegate to factory
    return GetImport().GetEventImport().CreateContext(
        GetImport(), nPrefix, rLocalName, xAttrList,
        this, sEventName, sLanguage);
}

void XMLEventsImportContext::AddEventValues(
    const OUString& rEventName,
    const Sequence<PropertyValue> & rValues )
{
    // set event (if name is known)
    if (xEvents->hasByName(rEventName))
    {
        Any aAny;
        aAny <<= rValues;
        xEvents->replaceByName(rEventName, aAny);
    }
}
