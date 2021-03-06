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

#include <xmloff/xmlstyle.hxx>
#include <xmloff/languagetagodf.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <rtl/ustring.hxx>


#include <vector>

namespace com::sun::star {
    namespace xml::sax { class XAttributeList; }
}


/**
 * Import bibliography configuration.
 *
 * Little cheat: Cover all child elements in CreateChildContext.
 */
class XMLIndexBibliographyConfigurationContext final : public SvXMLStyleContext
{
    OUString sSuffix;
    OUString sPrefix;
    OUString sAlgorithm;
    LanguageTagODF maLanguageTagODF;
    bool bNumberedEntries;
    bool bSortByPosition;

    ::std::vector< css::uno::Sequence< css::beans::PropertyValue> > aSortKeys;

public:


    XMLIndexBibliographyConfigurationContext(SvXMLImport& rImport);

    virtual ~XMLIndexBibliographyConfigurationContext() override;

private:

    virtual void SetAttribute( sal_Int32 nElement,
                               const OUString& rValue ) override;

    virtual void CreateAndInsert( bool bOverwrite ) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
