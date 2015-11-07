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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXTITLETEMPLATECONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXTITLETEMPLATECONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>


namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace xml { namespace sax { class XAttributeList; } }
} } }


/**
 * Import index title templates
 */
class XMLIndexTitleTemplateContext : public SvXMLImportContext
{

    const OUString sTitle;
    const OUString sParaStyleHeading;

    // paragraph style
    OUString sStyleName;
    bool bStyleNameOK;

    // content
    OUStringBuffer sContent;

    // TOC property set
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & rTOCPropertySet;

public:


     XMLIndexTitleTemplateContext(
        SvXMLImport& rImport,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet,
        sal_uInt16 nPrfx,
        const OUString& rLocalName);

    virtual ~XMLIndexTitleTemplateContext();

protected:

    /** process parameters */
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList) override;

    /** set values */
    virtual void EndElement() override;

    /** pick up title characters */
    virtual void Characters(const OUString& sString) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
