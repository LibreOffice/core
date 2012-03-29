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
#ifndef _OODOCUMENTHANDLER_HXX_
#define _OODOCUMENTHANDLER_HXX_

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#if defined _MSC_VER
#pragma warning( push, 1 )
#endif
#include <libwpd/libwpd.h>
#if defined _MSC_VER
#pragma warning( pop )
#endif

using com::sun::star::uno::Reference;
using com::sun::star::xml::sax::XDocumentHandler;

class DocumentHandler
{
public:
        DocumentHandler(Reference < XDocumentHandler > &xHandler);
        void startDocument();
        void endDocument();
        void startElement(const char *psName, const WPXPropertyList &xPropList);
        void endElement(const char *psName);
        void characters(const WPXString &sCharacters);

private:
        Reference < XDocumentHandler > mxHandler;
};

#endif // _OODOCUMENTHANDLER_HXX_
