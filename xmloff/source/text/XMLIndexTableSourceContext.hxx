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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXTABLESOURCECONTEXT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLINDEXTABLESOURCECONTEXT_HXX

#include "XMLIndexSourceBaseContext.hxx"
#include <com/sun/star/uno/Reference.h>


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }


/**
 * Import table index source element
 */
class XMLIndexTableSourceContext : public XMLIndexSourceBaseContext
{
    const OUString sCreateFromLabels;
    const OUString sLabelCategory;
    const OUString sLabelDisplayType;

    OUString sSequence;
    sal_Int16 nDisplayFormat;

    bool bSequenceOK;
    bool bDisplayFormatOK;
    bool bUseCaption;

public:


    XMLIndexTableSourceContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const OUString& rLocalName,
        css::uno::Reference<css::beans::XPropertySet> & rPropSet);

    virtual ~XMLIndexTableSourceContext();

protected:

    virtual void ProcessAttribute(
        enum IndexSourceParamEnum eParam,
        const OUString& rValue) override;

    virtual void EndElement() override;

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const css::uno::Reference<css::xml::sax::XAttributeList> & xAttrList ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
