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



#ifndef _XMLOFF_XMLBASE64IMPORTCONTEXT_HXX
#define _XMLOFF_XMLBASE64IMPORTCONTEXT_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <xmloff/xmlictxt.hxx>

namespace com { namespace sun { namespace star { namespace io {
    class XOutputStream; } } } }

class XMLOFF_DLLPUBLIC XMLBase64ImportContext : public SvXMLImportContext
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::io::XOutputStream > xOut;
    ::rtl::OUString sBase64CharsLeft;

public:
    XMLBase64ImportContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
                            const ::rtl::OUString& rLName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::io::XOutputStream >& rOut );

    virtual ~XMLBase64ImportContext();

    virtual void EndElement();

    virtual void Characters( const ::rtl::OUString& rChars );

};

#endif  //  _XMLOFF_XMLBASE64IMPORTCONTEXT_HXX

