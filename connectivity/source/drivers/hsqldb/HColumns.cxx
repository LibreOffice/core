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

#include "hsqldb/HColumns.hxx"
#include "TConnection.hxx"


using namespace ::comphelper;
using namespace connectivity::hsqldb;
using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

OHSQLColumns::OHSQLColumns( ::cppu::OWeakObject& _rParent
                                ,bool _bCase
                                ,::osl::Mutex& _rMutex
                                ,const TStringVector &_rVector
                                ,bool _bUseHardRef
            ) : OColumnsHelper(_rParent,_bCase,_rMutex,_rVector,_bUseHardRef)
{
}

Reference< XPropertySet > OHSQLColumns::createDescriptor()
{
    return new OHSQLColumn(true);
}


OHSQLColumn::OHSQLColumn(   bool    _bCase)
    : connectivity::sdbcx::OColumn( _bCase )
{
    construct();
}

void OHSQLColumn::construct()
{
    m_sAutoIncrement = "IDENTITY";
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_AUTOINCREMENTCREATION),PROPERTY_ID_AUTOINCREMENTCREATION,0,&m_sAutoIncrement, cppu::UnoType<decltype(m_sAutoIncrement)>::get());
}

::cppu::IPropertyArrayHelper* OHSQLColumn::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}

::cppu::IPropertyArrayHelper & SAL_CALL OHSQLColumn::getInfoHelper()
{
    return *OHSQLColumn_PROP::getArrayHelper(isNew() ? 1 : 0);
}

Sequence< OUString > SAL_CALL OHSQLColumn::getSupportedServiceNames(  ) throw(RuntimeException, std::exception)
{
    Sequence< OUString > aSupported { "com.sun.star.sdbcx.Column" };

    return aSupported;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
