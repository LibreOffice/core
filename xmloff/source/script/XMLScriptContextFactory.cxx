/*************************************************************************
 *
 *  $RCSfile: XMLScriptContextFactory.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:17:00 $
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

#ifndef _XMLOFF_XMLSCRIPTCONTEXTFACTORY_HXX
#include "XMLScriptContextFactory.hxx"
#endif

#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#include "XMLEventsImportContext.hxx"
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

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif


using namespace ::xmloff::token;

using ::rtl::OUString;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

XMLScriptContextFactory::XMLScriptContextFactory() :
    sEventType(RTL_CONSTASCII_USTRINGPARAM("EventType")),
    sURL(RTL_CONSTASCII_USTRINGPARAM("Script")),
    sScript(RTL_CONSTASCII_USTRINGPARAM("Script"))
{
}

XMLScriptContextFactory::~XMLScriptContextFactory()
{
}

SvXMLImportContext * XMLScriptContextFactory::CreateContext
(SvXMLImport & rImport,
 sal_uInt16 nPrefix,
 const OUString & rLocalName,
 const Reference<XAttributeList> & xAttrList,
 XMLEventsImportContext * rEvents,
 const OUString & rApiEventName,
 const OUString & rApiLanguage)
{
    OUString sURLVal;

    sal_Int16 nCount = xAttrList->getLength();
    for (sal_Int16 nAttr = 0; nAttr < nCount; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().
            GetKeyByAttrName(xAttrList->getNameByIndex(nAttr), &sLocalName);

        if (XML_NAMESPACE_XLINK == nPrefix)
        {
            if (IsXMLToken(sLocalName, XML_HREF))
                sURLVal = xAttrList->getValueByIndex(nAttr);
            // else: ignore
        }
        // else ignore
    }

    Sequence<PropertyValue> aValues(2);

    // EventType
    aValues[0].Name = sEventType;
    aValues[0].Value <<= sScript;

    // URL
    aValues[1].Name = sURL;
    aValues[1].Value <<= sURLVal;

    // add values for event now
    rEvents->AddEventValues(rApiEventName, aValues);

    // return dummy context
    return new SvXMLImportContext(rImport, nPrefix, rLocalName);
}

