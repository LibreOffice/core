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

#include "XMLIndexSimpleEntryContext.hxx"
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <rtl/ustrbuf.hxx>

namespace com::sun::star {
    namespace xml::sax { class XAttributeList; }
}

/**
 * Import index entry templates
 */
class XMLIndexSpanEntryContext : public XMLIndexSimpleEntryContext
{
    OUStringBuffer sContent;

public:


    XMLIndexSpanEntryContext(
        SvXMLImport& rImport,
        XMLIndexTemplateContext& rTemplate );

    virtual ~XMLIndexSpanEntryContext() override;

protected:

    /// Collect element contents
    virtual void SAL_CALL characters(const OUString& sString) override;

    /// add Text PropertyValue
    virtual void FillPropertyValues(
        css::uno::Sequence<css::beans::PropertyValue> & rValues) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
