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



#ifndef _XMLOFF_XMLTEXTMASTERPAGEEXPORT_HXX
#define _XMLOFF_XMLTEXTMASTERPAGEEXPORT_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <rtl/ustring.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/XMLPageExport.hxx>

namespace com { namespace sun { namespace star {
    namespace text { class XText; }
} } }

class XMLOFF_DLLPUBLIC XMLTextMasterPageExport : public XMLPageExport
{
    const ::rtl::OUString sHeaderText;
    const ::rtl::OUString sHeaderOn;
    const ::rtl::OUString sHeaderShareContent;
    const ::rtl::OUString sHeaderTextLeft;

    const ::rtl::OUString sFooterText;
    const ::rtl::OUString sFooterOn;
    const ::rtl::OUString sFooterShareContent;
    const ::rtl::OUString sFooterTextLeft;

protected:

    virtual void exportHeaderFooterContent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XText >& rText,
            sal_Bool bAutoStyles, sal_Bool bExportParagraph = sal_True );

    virtual void exportMasterPageContent(
                const ::com::sun::star::uno::Reference <
                    ::com::sun::star::beans::XPropertySet > & rPropSet,
                 sal_Bool bAutoStyles );

public:
    XMLTextMasterPageExport( SvXMLExport& rExp );
    ~XMLTextMasterPageExport();
};

#endif  //  _XMLOFF_XMLTEXTMASTERPAGEEXPORT_HXX

