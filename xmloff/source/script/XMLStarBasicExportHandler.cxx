/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "XMLStarBasicExportHandler.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>


using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

using ::com::sun::star::beans::PropertyValue;


XMLStarBasicExportHandler::XMLStarBasicExportHandler() :
    sStarBasic("StarBasic"),
    sLibrary("Library"),
    sMacroName("MacroName"),
    sStarOffice("StarOffice"),
    sApplication("application")
{
}

XMLStarBasicExportHandler::~XMLStarBasicExportHandler()
{
}

void XMLStarBasicExportHandler::Export(
    SvXMLExport& rExport,
    const OUString& rEventQName,
    Sequence<PropertyValue> & rValues,
    bool bUseWhitespace)
{
    rExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_LANGUAGE,
                         rExport.GetNamespaceMap().GetQNameByKey(
                             XML_NAMESPACE_OOO, sStarBasic ) );
    rExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_EVENT_NAME, rEventQName);

    OUString sLocation, sName;
    sal_Int32 nCount = rValues.getLength();
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        if (sLibrary == rValues[i].Name)
        {
            OUString sTmp;
            rValues[i].Value >>= sTmp;
            sLocation = GetXMLToken(
                (sTmp.equalsIgnoreAsciiCase(sApplication) ||
                 sTmp.equalsIgnoreAsciiCase(sStarOffice) ) ? XML_APPLICATION
                                                           : XML_DOCUMENT );
        }
        else if (sMacroName == rValues[i].Name)
        {
            rValues[i].Value >>= sName;
        }
        // else: disregard
    }

    if( !sLocation.isEmpty() )
    {
        OUStringBuffer sTmp( sLocation.getLength() + sName.getLength() + 1 );
        sTmp = sLocation;
        sTmp.append( ':' );
        sTmp.append( sName );
        rExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_MACRO_NAME,
                            sTmp.makeStringAndClear());
    }
    else
    {
        rExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_MACRO_NAME, sName );
    }

    SvXMLElementExport aEventElemt(rExport, XML_NAMESPACE_SCRIPT,
                                   XML_EVENT_LISTENER,
                                   bUseWhitespace, false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
