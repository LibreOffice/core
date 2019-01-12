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

#ifndef INCLUDED_XMLOFF_XMLNUMI_HXX
#define INCLUDED_XMLOFF_XMLNUMI_HXX

#include <sal/config.h>

#include <memory>
#include <vector>

#include <xmloff/xmlstyle.hxx>

namespace com { namespace sun { namespace star { namespace frame { class XModel; } } } }
namespace com { namespace sun { namespace star { namespace container { class XIndexReplace; } } } }

class SvxXMLListLevelStyleContext_Impl;
typedef std::vector<rtl::Reference<SvxXMLListLevelStyleContext_Impl>> SvxXMLListStyle_Impl;

class XMLOFF_DLLPUBLIC SvxXMLListStyleContext
    : public SvXMLStyleContext
{
    css::uno::Reference< css::container::XIndexReplace > xNumRules;

    std::unique_ptr<SvxXMLListStyle_Impl> pLevelStyles;

    bool                        bConsecutive : 1;
    bool const                  bOutline : 1;

protected:

    SAL_DLLPRIVATE virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const OUString& rLocalName,
                               const OUString& rValue ) override;

public:


    SvxXMLListStyleContext(
            SvXMLImport& rImport,
            sal_uInt16 nPrfx,
            const OUString& rLName,
            const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList,
            bool bOutl = false );

    ~SvxXMLListStyleContext() override;

    virtual SvXMLImportContextRef CreateChildContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList ) override;

    void FillUnoNumRule(
            const css::uno::Reference< css::container::XIndexReplace> & rNumRule) const;

    const css::uno::Reference< css::container::XIndexReplace >& GetNumRules() const
        { return xNumRules; }

    static css::uno::Reference< css::container::XIndexReplace >
    CreateNumRule(
        const css::uno::Reference< css::frame::XModel > & rModel );

    static void SetDefaultStyle(
        const css::uno::Reference< css::container::XIndexReplace > & rNumRule,
        sal_Int16 nLevel,
        bool bOrdered );

    virtual void CreateAndInsertLate( bool bOverwrite ) override;

    void CreateAndInsertAuto() const;
};

#endif // INCLUDED_XMLOFF_XMLNUMI_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
