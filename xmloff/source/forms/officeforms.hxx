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

#include <com/sun/star/beans/XPropertySet.hpp>

#include "formattributes.hxx"
#include <xmloff/xmlictxt.hxx>
#include <memory>
#include "logging.hxx"

class SvXMLElementExport;
class SvXMLExport;

namespace xmloff
{

    //= OFormsRootImport
    class OFormsRootImport
                :public SvXMLImportContext
                ,public OStackedLogging
    {
    public:

        OFormsRootImport( SvXMLImport& _rImport);
        virtual ~OFormsRootImport() override;

        // SvXMLImportContext overridable
        virtual void SAL_CALL startFastElement(
            sal_Int32 nElement,
            const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList) override;
        virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
        virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    private:
        static void implImportBool(
            const css::uno::Reference< css::xml::sax::XFastAttributeList >& _rxAttributes,
            OfficeFormsAttributes _eAttribute,
            const css::uno::Reference< css::beans::XPropertySet >& _rxProps,
            const css::uno::Reference< css::beans::XPropertySetInfo >& _rxPropInfo,
            const OUString& _rPropName,
            bool _bDefault
            );
    };

    //= OFormsRootExport
    class OFormsRootExport
    {
    private:
        std::unique_ptr<SvXMLElementExport>     m_pImplElement;

    public:
        explicit OFormsRootExport( SvXMLExport& _rExp );
        ~OFormsRootExport();

    private:
        static void addModelAttributes(SvXMLExport& _rExp);

        static void implExportBool(
            SvXMLExport& _rExp,
            OfficeFormsAttributes _eAttribute,
            const css::uno::Reference< css::beans::XPropertySet >& _rxProps,
            const css::uno::Reference< css::beans::XPropertySetInfo >& _rxPropInfo,
            const OUString& _rPropName,
            bool _bDefault
            );

        OFormsRootExport(const OFormsRootExport&) = delete;
        OFormsRootExport& operator=(const OFormsRootExport&) = delete;
    };

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
