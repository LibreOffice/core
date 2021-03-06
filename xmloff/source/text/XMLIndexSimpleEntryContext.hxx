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

#pragma once

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>


namespace com::sun::star {
    namespace xml::sax { class XAttributeList; }
}
class XMLIndexTemplateContext;

/**
 * Import index entry templates
 */
class XMLIndexSimpleEntryContext : public SvXMLImportContext
{

    // entry type
    const OUString m_rEntryType;

protected:
    // character style
    OUString m_sCharStyleName;
    bool m_bCharStyleNameOK;

    // surrounding template
    XMLIndexTemplateContext& m_rTemplateContext;

    // number of values for PropertyValues
    sal_Int32 m_nValues;

public:


    XMLIndexSimpleEntryContext(
        SvXMLImport& rImport,
        const OUString& rEntry,
        XMLIndexTemplateContext& rTemplate );

    virtual ~XMLIndexSimpleEntryContext() override;

protected:

    /** process parameters */
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList ) override;

    /** call FillPropertyValues and insert into template */
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    /** fill property values for this template entry */
    virtual void FillPropertyValues(
        css::uno::Sequence<css::beans::PropertyValue> & rValues);

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
