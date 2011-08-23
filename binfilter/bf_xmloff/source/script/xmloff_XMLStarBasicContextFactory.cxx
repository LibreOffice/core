/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "XMLStarBasicContextFactory.hxx"

#include "XMLEventsImportContext.hxx"


#include "xmlimp.hxx"

#include "nmspmap.hxx"

#include "xmlnmspe.hxx"

namespace binfilter {


using namespace ::binfilter::xmloff::token;

using ::rtl::OUString;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;


XMLStarBasicContextFactory::XMLStarBasicContextFactory() :
    sEventType(RTL_CONSTASCII_USTRINGPARAM("EventType")),
    sLibrary(RTL_CONSTASCII_USTRINGPARAM("Library")),
    sMacroName(RTL_CONSTASCII_USTRINGPARAM("MacroName")),
    sStarBasic(RTL_CONSTASCII_USTRINGPARAM("StarBasic"))
{
}

XMLStarBasicContextFactory::~XMLStarBasicContextFactory()
{
}

SvXMLImportContext* XMLStarBasicContextFactory::CreateContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & xAttrList,
    XMLEventsImportContext* rEvents,
    const OUString& rApiEventName,
    const OUString& rLanguage)
{
    OUString sLibraryVal;
    OUString sMacroNameVal;

    sal_Int16 nCount = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nCount; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr), &sLocalName );

        if (XML_NAMESPACE_SCRIPT == nPrefix)
        {
            if (IsXMLToken(sLocalName, XML_LIBRARY))
            {
                sLibraryVal = xAttrList->getValueByIndex(nAttr);
            }
            if (IsXMLToken(sLocalName, XML_LOCATION))
            {
                sLibraryVal = xAttrList->getValueByIndex(nAttr);
                if ( IsXMLToken( sLibraryVal, XML_APPLICATION ) )
                    sLibraryVal = 
                        OUString(RTL_CONSTASCII_USTRINGPARAM("StarOffice"));
            }
            else if (IsXMLToken(sLocalName, XML_MACRO_NAME))
            {
                sMacroNameVal = xAttrList->getValueByIndex(nAttr);
            }
            // else: ingore
        }
        // else: ignore
    }

    Sequence<PropertyValue> aValues(3);

    // EventType
    aValues[0].Name = sEventType;
    aValues[0].Value <<= sStarBasic;

    // library name
    aValues[1].Name = sLibrary;
    aValues[1].Value <<= sLibraryVal;

    // macro name
    aValues[2].Name = sMacroName;
    aValues[2].Value <<= sMacroNameVal;

    // add values for event now
    rEvents->AddEventValues(rApiEventName, aValues);

    // return dummy context
    return new SvXMLImportContext(rImport, nPrefix, rLocalName);
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
