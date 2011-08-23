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

#ifndef _XMLOFF_XMLSCRIPTCONTEXTFACTORY_HXX
#include "XMLScriptContextFactory.hxx"
#endif

#ifndef _XMLOFF_XMLEVENTSIMPORTCONTEXT_HXX
#include "XMLEventsImportContext.hxx"
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

namespace binfilter {


using namespace ::binfilter::xmloff::token;

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
 const OUString & rLanguage)
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
 
}//end of namespace binfilter
