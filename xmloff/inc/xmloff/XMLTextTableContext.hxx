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



#ifndef _XMLTEXTTABLECONTEXT_HXX
#define _XMLTEXTTABLECONTEXT_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>

namespace com { namespace sun { namespace star {
    namespace text { class XTextContent; }
} } }

#define XML_TEXT_FRAME_TEXTBOX 1
#define XML_TEXT_FRAME_GRAPHIC 2

class XMLOFF_DLLPUBLIC XMLTextTableContext : public SvXMLImportContext
{

public:

    XMLTextTableContext( SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName );
    virtual ~XMLTextTableContext();

    virtual ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > GetXTextContent() const = 0;
};

#endif
