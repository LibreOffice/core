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

namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::container { class XIndexReplace; }

class SvxXMLListLevelStyleContext_Impl;
typedef std::vector<rtl::Reference<SvxXMLListLevelStyleContext_Impl>> SvxXMLListStyle_Impl;

class XMLOFF_DLLPUBLIC SvxXMLListStyleContext final
    : public SvXMLStyleContext
{
    css::uno::Reference< css::container::XIndexReplace > xNumRules;

    std::unique_ptr<SvxXMLListStyle_Impl> pLevelStyles;

    bool                        bConsecutive : 1;
    bool                        bOutline : 1;

    SAL_DLLPRIVATE virtual void SetAttribute( sal_uInt16 nPrefixKey,
                               const OUString& rLocalName,
                               const OUString& rValue ) override;

public:

    SvxXMLListStyleContext(
            SvXMLImport& rImport,
            bool bOutl = false );

    ~SvxXMLListStyleContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

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
