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
#ifndef _SWDBTOOLSCLIENT_HXX
#define _SWDBTOOLSCLIENT_HXX

#include <connectivity/virtualdbtools.hxx>
#include <osl/mutex.hxx>
#include <osl/module.h>
#include "swdllapi.h"

/*
    Client to use the dbtools library as load-on-call
*/
class SW_DLLPUBLIC SwDbtoolsClient
{
private:
    ::rtl::Reference< ::connectivity::simple::IDataAccessTools >            m_xDataAccessTools;
    ::rtl::Reference< ::connectivity::simple::IDataAccessTypeConversion >   m_xAccessTypeConversion;
    ::rtl::Reference< ::connectivity::simple::IDataAccessToolsFactory >     m_xDataAccessFactory;

    SW_DLLPRIVATE static void registerClient();
    SW_DLLPRIVATE static void revokeClient();
    SW_DLLPRIVATE void getFactory();

    SW_DLLPRIVATE ::rtl::Reference< ::connectivity::simple::IDataAccessTools >    getDataAccessTools();
    SW_DLLPRIVATE ::rtl::Reference< ::connectivity::simple::IDataAccessTypeConversion > getAccessTypeConversion();

public:
    SwDbtoolsClient();
    ~SwDbtoolsClient();

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource > getDataSource(
        const OUString& _rsRegisteredName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxContext
            );

    sal_Int32 getDefaultNumberFormat(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn,
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatTypes >& _rxTypes,
        const ::com::sun::star::lang::Locale& _rLocale
            );

    OUString getFormattedValue(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxColumn,
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter>& _rxFormatter,
        const ::com::sun::star::lang::Locale& _rLocale,
        const ::com::sun::star::util::Date& _rNullDate
            );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
