/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SWDBTOOLSCLIENT_HXX
#define _SWDBTOOLSCLIENT_HXX

#include <connectivity/virtualdbtools.hxx>
#include <osl/mutex.hxx>
#include <osl/module.h>
#include "swdllapi.h"

/* -----------------------------30.08.2001 11:01------------------------------
    Client to use the dbtools library as load-on-call
 ---------------------------------------------------------------------------*/
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
        const ::rtl::OUString& _rsRegisteredName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory
            );

    sal_Int32 getDefaultNumberFormat(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxColumn,
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatTypes >& _rxTypes,
        const ::com::sun::star::lang::Locale& _rLocale
            );

    ::rtl::OUString getFormattedValue(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _rxColumn,
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter>& _rxFormatter,
        const ::com::sun::star::lang::Locale& _rLocale,
        const ::com::sun::star::util::Date& _rNullDate
            );
};

#endif

