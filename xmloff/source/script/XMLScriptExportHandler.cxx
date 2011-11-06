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
#include "XMLScriptExportHandler.hxx"

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
using ::com::sun::star::beans::PropertyValue;


XMLScriptExportHandler::XMLScriptExportHandler() :
    sURL(RTL_CONSTASCII_USTRINGPARAM("Script"))
{
}

XMLScriptExportHandler::~XMLScriptExportHandler()
{
}

void XMLScriptExportHandler::Export(
    SvXMLExport& rExport,
    const OUString& rEventQName,
    Sequence<PropertyValue> & rValues,
    sal_Bool bUseWhitespace)
{

    rExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_LANGUAGE,
                         rExport.GetNamespaceMap().GetQNameByKey(
                             XML_NAMESPACE_OOO, GetXMLToken(XML_SCRIPT) ) );
    rExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_EVENT_NAME, rEventQName);

    sal_Int32 nCount = rValues.getLength();
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        if (sURL.equals(rValues[i].Name))
        {
            OUString sTmp;
            rValues[i].Value >>= sTmp;
            rExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sTmp);

            // #i110911# xlink:type="simple" is required
            rExport.AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE);
        }
        // else: disregard
    }

    SvXMLElementExport aEventElemt(rExport, XML_NAMESPACE_SCRIPT,
                                   XML_EVENT_LISTENER,
                                   bUseWhitespace, sal_False);
}
