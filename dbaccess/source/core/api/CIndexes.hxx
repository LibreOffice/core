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

#ifndef INCLUDED_DBACCESS_SOURCE_CORE_API_CINDEXES_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_API_CINDEXES_HXX

#include "connectivity/TIndexes.hxx"

namespace dbaccess
{
    class OIndexes : public connectivity::OIndexesHelper
    {
        css::uno::Reference< css::container::XNameAccess > m_xIndexes;
    protected:
        virtual connectivity::sdbcx::ObjectType createObject(const OUString& _rName) override;
        virtual css::uno::Reference< css::beans::XPropertySet > createDescriptor() override;
        virtual connectivity::sdbcx::ObjectType appendObject( const OUString& _rForName, const css::uno::Reference< css::beans::XPropertySet >& descriptor ) override;
        virtual void dropObject(sal_Int32 _nPos, const OUString& _sElementName) override;
    public:
        OIndexes(connectivity::OTableHelper* _pTable,
                 ::osl::Mutex& _rMutex,
                 const ::std::vector< OUString> &_rVector,
                 const css::uno::Reference< css::container::XNameAccess >& _rxIndexes
                 ) : connectivity::OIndexesHelper(_pTable,_rMutex,_rVector)
            ,m_xIndexes(_rxIndexes)
        {}

        virtual void SAL_CALL disposing() override;
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_CORE_API_CINDEXES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
