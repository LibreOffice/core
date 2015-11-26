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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXSIMPLEENTRYCONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXSIMPLEENTRYCONTEXT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
} } }
class XMLIndexTemplateContext;

/**
 * Import index entry templates
 */
class XMLIndexSimpleEntryContext : public SvXMLImportContext
{

    // entry type
    const OUString& rEntryType;

protected:
    // character style
    OUString sCharStyleName;
    bool bCharStyleNameOK;

    // surrounding template
    XMLIndexTemplateContext& rTemplateContext;

    // number of values for PropertyValues
    sal_Int32 nValues;

public:


    XMLIndexSimpleEntryContext(
        SvXMLImport& rImport,
        const OUString& rEntry,
        XMLIndexTemplateContext& rTemplate,
        sal_uInt16 nPrfx,
        const OUString& rLocalName );

    virtual ~XMLIndexSimpleEntryContext();

protected:

    /** process parameters */
    virtual void StartElement(
        const css::uno::Reference<css::xml::sax::XAttributeList> & xAttrList) override;

    /** call FillPropertyValues and insert into template */
    virtual void EndElement() override;

    /** fill property values for this template entry */
    virtual void FillPropertyValues(
        css::uno::Sequence<css::beans::PropertyValue> & rValues);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
