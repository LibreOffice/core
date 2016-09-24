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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLAUTOSTYLE_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLAUTOSTYLE_HXX

#include <xmloff/maptype.hxx>
#include <xmloff/xmlaustp.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/xmlprmap.hxx>

namespace rptxml
{
    class ORptExport;
    class OXMLAutoStylePoolP : public SvXMLAutoStylePoolP
    {
        ORptExport& rORptExport;

        virtual void exportStyleAttributes(
                SvXMLAttributeList& rAttrList,
                sal_Int32 nFamily,
                const ::std::vector< XMLPropertyState >& rProperties,
                const SvXMLExportPropertyMapper& rPropExp,
                const SvXMLUnitConverter& rUnitConverter,
                const SvXMLNamespaceMap& rNamespaceMap
                ) const override;

        OXMLAutoStylePoolP(const OXMLAutoStylePoolP&) = delete;
        void operator =(const OXMLAutoStylePoolP&) = delete;
    public:
        explicit OXMLAutoStylePoolP(ORptExport& rXMLExport);
        virtual ~OXMLAutoStylePoolP() override;
    };

} // rptxml

#endif // INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLAUTOSTYLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
