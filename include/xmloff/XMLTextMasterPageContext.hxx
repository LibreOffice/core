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
#ifndef _XMLOFF_TEXTMASTERPAGECONTEXT_HXX_
#define _XMLOFF_TEXTMASTERPAGECONTEXT_HXX_

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include "sal/types.h"

#include <xmloff/xmlstyle.hxx>

namespace com { namespace sun { namespace star {
    namespace style { class XStyle; }
} } }

class XMLOFF_DLLPUBLIC XMLTextMasterPageContext : public SvXMLStyleContext
{
    const OUString sIsPhysical;
    const OUString sPageStyleLayout;
    const OUString sFollowStyle;
    OUString       sFollow;
    OUString       sPageMasterName;

    ::com::sun::star::uno::Reference < ::com::sun::star::style::XStyle > xStyle;

    sal_Bool bInsertHeader;
    sal_Bool bInsertFooter;
    sal_Bool bInsertHeaderLeft;
    sal_Bool bInsertFooterLeft;
    sal_Bool bInsertHeaderFirst;
    sal_Bool bInsertFooterFirst;
    sal_Bool bHeaderInserted;
    sal_Bool bFooterInserted;
    sal_Bool bHeaderLeftInserted;
    sal_Bool bFooterLeftInserted;
    sal_Bool bHeaderFirstInserted;
    sal_Bool bFooterFirstInserted;

    SAL_DLLPRIVATE ::com::sun::star::uno::Reference <
        ::com::sun::star::style::XStyle > Create();
protected:
    ::com::sun::star::uno::Reference <
        ::com::sun::star::style::XStyle > GetStyle() { return xStyle; }
public:

    TYPEINFO();

    XMLTextMasterPageContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
            const OUString& rLName,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            sal_Bool bOverwrite );
    virtual ~XMLTextMasterPageContext();

    virtual SvXMLImportContext *CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

    virtual SvXMLImportContext *CreateHeaderFooterContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const sal_Bool bFooter,
            const sal_Bool bLeft,
            const sal_Bool bFirst );

    virtual void Finish( sal_Bool bOverwrite );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
