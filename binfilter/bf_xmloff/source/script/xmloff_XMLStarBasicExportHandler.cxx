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

#include "XMLStarBasicExportHandler.hxx"



#include "xmlexp.hxx"


#include "xmlnmspe.hxx"
namespace binfilter {


using namespace ::com::sun::star::uno;
using namespace ::binfilter::xmloff::token;

using ::rtl::OUString;
using ::com::sun::star::beans::PropertyValue;


XMLStarBasicExportHandler::XMLStarBasicExportHandler() :
    sStarBasic(RTL_CONSTASCII_USTRINGPARAM("StarBasic")),
    sStarOffice(RTL_CONSTASCII_USTRINGPARAM("StarOffice")),
    sApplication(RTL_CONSTASCII_USTRINGPARAM("application")),
    sLibrary(RTL_CONSTASCII_USTRINGPARAM("Library")),
    sMacroName(RTL_CONSTASCII_USTRINGPARAM("MacroName"))
{
}

XMLStarBasicExportHandler::~XMLStarBasicExportHandler()
{
}

void XMLStarBasicExportHandler::Export(
    SvXMLExport& rExport,
    const OUString& rEventName,
    Sequence<PropertyValue> & rValues,
    sal_Bool bUseWhitespace)
{
    rExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_LANGUAGE, sStarBasic);
    rExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_EVENT_NAME, rEventName);

    sal_Int32 nCount = rValues.getLength();
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        if (sLibrary.equals(rValues[i].Name))
        {
            OUString sTmp;
            rValues[i].Value >>= sTmp;
            rExport.AddAttribute(
                XML_NAMESPACE_SCRIPT, XML_LOCATION,
                (sTmp.equalsIgnoreAsciiCase(sApplication) || sTmp.equalsIgnoreAsciiCase(sStarOffice) ) ? XML_APPLICATION
                                                        : XML_DOCUMENT );
        }
        else if (sMacroName.equals(rValues[i].Name))
        {
            OUString sTmp;
            rValues[i].Value >>= sTmp;
            rExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_MACRO_NAME, sTmp);
        }
        // else: disregard
    }

    SvXMLElementExport aEventElemt(rExport, XML_NAMESPACE_SCRIPT, XML_EVENT,
                                   bUseWhitespace, sal_False);
}
}//end of namespace binfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
