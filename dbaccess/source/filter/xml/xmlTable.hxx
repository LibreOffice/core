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

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

namespace dbaxml
{
    class ODBFilter;
    class OXMLTable : public SvXMLImportContext
    {
    protected:
        css::uno::Reference< css::container::XNameAccess >    m_xParentContainer;
        css::uno::Reference< css::beans::XPropertySet >       m_xTable;
        OUString m_sFilterStatement;
        OUString m_sOrderStatement;
        OUString m_sName;
        OUString m_sSchema;
        OUString m_sCatalog;
        OUString m_sStyleName;
        bool     m_bApplyFilter;
        bool     m_bApplyOrder;

        ODBFilter& GetOwnImport();

        static void fillAttributes(     const css::uno::Reference< css::xml::sax::XFastAttributeList > & xAttrList
                                ,OUString& _rsCommand
                                ,OUString& _rsTableName
                                ,OUString& _rsTableSchema
                                ,OUString& _rsTableCatalog
                            );

        virtual void setProperties(css::uno::Reference< css::beans::XPropertySet > & _xProp);
    public:

        OXMLTable( ODBFilter& rImport
                    ,const css::uno::Reference< css::xml::sax::XFastAttributeList > & _xAttrList
                    ,const css::uno::Reference< css::container::XNameAccess >& _xParentContainer
                    ,const OUString& _sServiceName
                    );
        virtual ~OXMLTable() override;

        virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
            sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
        virtual void SAL_CALL startFastElement( sal_Int32 /*nElement*/,
                const css::uno::Reference< css::xml::sax::XFastAttributeList >& ) override {}
        virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
    };
} // namespace dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
