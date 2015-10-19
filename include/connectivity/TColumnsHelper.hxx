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

#ifndef INCLUDED_CONNECTIVITY_TCOLUMNSHELPER_HXX
#define INCLUDED_CONNECTIVITY_TCOLUMNSHELPER_HXX

#include <connectivity/sdbcx/VCollection.hxx>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <connectivity/sdbcx/IRefreshable.hxx>
#include <connectivity/dbtoolsdllapi.hxx>

namespace connectivity
{
    class OTableHelper;
    class OColumnsHelperImpl;
    /** contains generell column handling to create default columns and default sql statements.
    */
    class OOO_DLLPUBLIC_DBTOOLS OColumnsHelper : public sdbcx::OCollection
    {
        OColumnsHelperImpl* m_pImpl;
    protected:
        OTableHelper*   m_pTable;

        virtual sdbcx::ObjectType createObject(const OUString& _rName) override;
        virtual void impl_refresh() throw(css::uno::RuntimeException) override;
        virtual css::uno::Reference< css::beans::XPropertySet > createDescriptor() override;
        virtual sdbcx::ObjectType appendObject( const OUString& _rForName, const css::uno::Reference< css::beans::XPropertySet >& descriptor ) override;
        virtual void dropObject(sal_Int32 _nPos, const OUString& _sElementName) override;
    public:
        OColumnsHelper( ::cppu::OWeakObject& _rParent
                        ,bool _bCase
                        ,::osl::Mutex& _rMutex
                        ,const TStringVector &_rVector
                        ,bool _bUseHardRef = true
                    );
        virtual ~OColumnsHelper();

        /** set the parent of the columns. Can also be <NULL/>.
            @param  _pTable
                The parent.
        */
        inline void setParent(OTableHelper* _pTable) { m_pTable = _pTable;}
    };
}
#endif // INCLUDED_CONNECTIVITY_TCOLUMNSHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
