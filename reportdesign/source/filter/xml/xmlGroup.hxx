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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLGROUP_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLGROUP_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/report/XGroups.hpp>

namespace rptxml
{
    class ORptFilter;
    class OXMLGroup : public SvXMLImportContext
    {
    protected:
        css::uno::Reference< css::report::XGroups >   m_xGroups;
        css::uno::Reference< css::report::XGroup >    m_xGroup;

        ORptFilter& GetOwnImport();

        OXMLGroup(const OXMLGroup&);
        OXMLGroup& operator =(const OXMLGroup&);
    public:

        OXMLGroup( ORptFilter& rImport
                    , sal_uInt16 nPrfx
                    ,const OUString& rLName
                    ,const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList
                    );
        virtual ~OXMLGroup();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const OUString& rLocalName,
                    const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList ) override;

        virtual void EndElement() override;
    };

} // namespace rptxml


#endif // INCLUDED_REPORTDESIGN_SOURCE_FILTER_XML_XMLGROUP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
