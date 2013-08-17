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
#ifndef DBA_XMLTABLE_HXX
#define DBA_XMLTABLE_HXX

#include <xmloff/xmlictxt.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

namespace dbaxml
{
    class ODBFilter;
    class OXMLTable : public SvXMLImportContext
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xParentContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xTable;
        OUString m_sFilterStatement;
        OUString m_sOrderStatement;
        OUString m_sName;
        OUString m_sSchema;
        OUString m_sCatalog;
        OUString m_sStyleName;
        OUString m_sServiceName;
        sal_Bool        m_bApplyFilter;
        sal_Bool        m_bApplyOrder;

        ODBFilter& GetOwnImport();

        void fillAttributes(    sal_uInt16 nPrfx
                                ,const OUString& _sLocalName
                                ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                                , OUString& _rsCommand
                                ,OUString& _rsTableName
                                ,OUString& _rsTableSchema
                                ,OUString& _rsTableCatalog
                            );

        virtual void setProperties(::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & _xProp);
    public:

        OXMLTable( ODBFilter& rImport
                    , sal_uInt16 nPrfx
                    ,const OUString& rLName
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xParentContainer
                    ,const OUString& _sServiceName
                    );
        virtual ~OXMLTable();

        virtual SvXMLImportContext *CreateChildContext( sal_uInt16 nPrefix,
                    const OUString& rLocalName,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList );
        virtual void EndElement();
    };
} // namespace dbaxml

#endif // DBA_XMLTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
