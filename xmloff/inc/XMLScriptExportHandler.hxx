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



#ifndef _XMLOFF_XMLSCRIPTEXPORTHANDLER_HXX
#define _XMLOFF_XMLSCRIPTEXPORTHANDLER_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>
#include <xmloff/xmlevent.hxx>

#include <map>

class SvXMLExport;
namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
} } }

class XMLScriptExportHandler : public XMLEventExportHandler
{
    const ::rtl::OUString sURL;

public:
    XMLScriptExportHandler();
    virtual ~XMLScriptExportHandler();

    virtual void Export(
        SvXMLExport& rExport,
        const ::rtl::OUString& rEventName,
        ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue> & rValues,
        sal_Bool bUseWhitespace);
};

#endif
