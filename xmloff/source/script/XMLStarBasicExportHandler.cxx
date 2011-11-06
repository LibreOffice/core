/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "XMLStarBasicExportHandler.hxx"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#include <tools/debug.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"


using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::beans::PropertyValue;


XMLStarBasicExportHandler::XMLStarBasicExportHandler() :
    sStarBasic(RTL_CONSTASCII_USTRINGPARAM("StarBasic")),
    sLibrary(RTL_CONSTASCII_USTRINGPARAM("Library")),
    sMacroName(RTL_CONSTASCII_USTRINGPARAM("MacroName")),
    sStarOffice(RTL_CONSTASCII_USTRINGPARAM("StarOffice")),
    sApplication(RTL_CONSTASCII_USTRINGPARAM("application"))
{
}

XMLStarBasicExportHandler::~XMLStarBasicExportHandler()
{
}

void XMLStarBasicExportHandler::Export(
    SvXMLExport& rExport,
    const OUString& rEventQName,
    Sequence<PropertyValue> & rValues,
    sal_Bool bUseWhitespace)
{
    rExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_LANGUAGE,
                         rExport.GetNamespaceMap().GetQNameByKey(
                             XML_NAMESPACE_OOO, sStarBasic ) );
    rExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_EVENT_NAME, rEventQName);

    OUString sLocation, sName;
    sal_Int32 nCount = rValues.getLength();
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        if (sLibrary.equals(rValues[i].Name))
        {
            OUString sTmp;
            rValues[i].Value >>= sTmp;
            sLocation = GetXMLToken(
                (sTmp.equalsIgnoreAsciiCase(sApplication) ||
                 sTmp.equalsIgnoreAsciiCase(sStarOffice) ) ? XML_APPLICATION
                                                           : XML_DOCUMENT );
        }
        else if (sMacroName.equals(rValues[i].Name))
        {
            rValues[i].Value >>= sName;
        }
        // else: disregard
    }

    if( sLocation.getLength() )
    {
        OUStringBuffer sTmp( sLocation.getLength() + sName.getLength() + 1 );
        sTmp = sLocation;
        sTmp.append( sal_Unicode( ':' ) );
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
                                   bUseWhitespace, sal_False);
}
