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

#include "xmlTable.hxx"

namespace dbaxml
{
    class ODBFilter;
    class OXMLQuery : public OXMLTable
    {
        OUString m_sCommand;
        OUString m_sTable;
        bool        m_bEscapeProcessing;
    protected:
        virtual void setProperties(css::uno::Reference< css::beans::XPropertySet > & _xProp) override;
    public:

        OXMLQuery( ODBFilter& rImport
                    ,const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList
                    ,const css::uno::Reference< css::container::XNameAccess >& _xParentContainer
                    );
        virtual ~OXMLQuery() override;

        virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
                const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}
        virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    };
} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
