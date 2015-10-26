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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLREPORTELEMENT_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLREPORTELEMENT_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/report/XReportControlModel.hpp>

namespace rptxml
{
    class ORptFilter;
    class OXMLReportElement : public SvXMLImportContext
    {
        css::uno::Reference< css::report::XReportControlModel >   m_xComponent;
        ORptFilter& GetOwnImport();
        OXMLReportElement(const OXMLReportElement&) = delete;
        void operator =(const OXMLReportElement&) = delete;
    public:

        OXMLReportElement( ORptFilter& rImport, sal_uInt16 nPrfx,
                    const OUString& rLName,
                    const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList
                    ,const css::uno::Reference< css::report::XReportControlModel >& _xComponent);
        virtual ~OXMLReportElement();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const OUString& rLocalName,
                    const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;
    };

} // namespace rptxml


#endif // INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLREPORTELEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
