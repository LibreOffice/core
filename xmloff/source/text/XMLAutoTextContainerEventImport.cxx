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
#include "XMLAutoTextContainerEventImport.hxx"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <xmloff/xmlimp.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/XMLEventsImportContext.hxx>


using namespace ::com::sun::star;

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::container::XNameReplace;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_EVENT_LISTENERS;


XMLAutoTextContainerEventImport::XMLAutoTextContainerEventImport(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const Reference<XNameReplace> & rEvnts ) :
        SvXMLImportContext(rImport, nPrfx, rLName),
        rEvents(rEvnts)
{
}

XMLAutoTextContainerEventImport::~XMLAutoTextContainerEventImport()
{
}

SvXMLImportContext* XMLAutoTextContainerEventImport::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & )
{
    if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
         IsXMLToken( rLocalName, XML_EVENT_LISTENERS)   )
    {
        return new XMLEventsImportContext(GetImport(), nPrefix, rLocalName,
                                          rEvents);
    }
    else
        return new SvXMLImportContext(GetImport(), nPrefix, rLocalName);
}
