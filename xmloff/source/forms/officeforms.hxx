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

#ifndef INCLUDED_XMLOFF_SOURCE_FORMS_OFFICEFORMS_HXX
#define INCLUDED_XMLOFF_SOURCE_FORMS_OFFICEFORMS_HXX

#include "formattributes.hxx"
#include <xmloff/xmlictxt.hxx>
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

        OFormsRootImport( SvXMLImport& _rImport, sal_uInt16 _nPrfx, const OUString& _rLocalName);
        virtual ~OFormsRootImport();

        // SvXMLImportContext overridable
        virtual SvXMLImportContext * CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList ) override;
        virtual void StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttrList ) override;
        virtual void EndElement() override;

    protected:
        void implImportBool(
            const css::uno::Reference< css::xml::sax::XAttributeList >& _rxAttributes,
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
        SvXMLElementExport*     m_pImplElement;

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
    };

}   // namespace xmloff

#endif // INCLUDED_XMLOFF_SOURCE_FORMS_OFFICEFORMS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
