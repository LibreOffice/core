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


#ifndef _XMLOFF_TEXTMASTERPAGECONTEXT_HXX_
#define _XMLOFF_TEXTMASTERPAGECONTEXT_HXX_

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"

#ifndef _XMLOFF_XMLSTYLE_HXX_
#include <xmloff/xmlstyle.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace style { class XStyle; }
} } }

class XMLOFF_DLLPUBLIC XMLTextMasterPageContext : public SvXMLStyleContext
{
    const ::rtl::OUString sIsPhysical;
    const ::rtl::OUString sPageStyleLayout;
    const ::rtl::OUString sFollowStyle;
    ::rtl::OUString       sFollow;
    ::rtl::OUString       sPageMasterName;

    ::com::sun::star::uno::Reference < ::com::sun::star::style::XStyle > xStyle;

    sal_Bool bInsertHeader;
    sal_Bool bInsertFooter;
    sal_Bool bInsertHeaderLeft;
    sal_Bool bInsertFooterLeft;
    sal_Bool bHeaderInserted;
    sal_Bool bFooterInserted;
    sal_Bool bHeaderLeftInserted;
    sal_Bool bFooterLeftInserted;

    SAL_DLLPRIVATE ::com::sun::star::uno::Reference <
        ::com::sun::star::style::XStyle > Create();
protected:
    ::com::sun::star::uno::Reference <
        ::com::sun::star::style::XStyle > GetStyle() { return xStyle; }
public:

    XMLTextMasterPageContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            sal_Bool bOverwrite );
    virtual ~XMLTextMasterPageContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual SvXMLImportContext *CreateHeaderFooterContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const sal_Bool bFooter,
            const sal_Bool bLeft );

    virtual void Finish( sal_Bool bOverwrite );
};

#endif
