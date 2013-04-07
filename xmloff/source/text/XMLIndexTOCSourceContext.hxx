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

#ifndef _XMLOFF_XMLINDEXTOCSOURCECONTEXT_HXX_
#define _XMLOFF_XMLINDEXTOCSOURCECONTEXT_HXX_

#include "XMLIndexSourceBaseContext.hxx"
#include <com/sun/star/uno/Reference.h>


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }
namespace rtl { class OUString; }


/**
 * Import table of context source element
 */
class XMLIndexTOCSourceContext : public XMLIndexSourceBaseContext
{
    const OUString sCreateFromMarks;
    const OUString sLevel;
    const OUString sCreateFromChapter;
    const OUString sCreateFromOutline;
    const OUString sCreateFromLevelParagraphStyles;

    sal_Int32 nOutlineLevel;
    sal_Bool bUseOutline;
    sal_Bool bUseMarks;
    sal_Bool bUseParagraphStyles;

public:

    TYPEINFO();

    XMLIndexTOCSourceContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet);

    ~XMLIndexTOCSourceContext();

protected:

    virtual void ProcessAttribute(
        enum IndexSourceParamEnum eParam,
        const OUString& rValue);

    virtual void EndElement();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
