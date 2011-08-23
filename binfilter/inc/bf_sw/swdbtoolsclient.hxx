/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SWDBTOOLSCLIENT_HXX
#define _SWDBTOOLSCLIENT_HXX

#ifndef CONNECTIVITY_VIRTUAL_DBTOOLS_HXX
#include <connectivity/virtualdbtools.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
namespace binfilter {

/* -----------------------------30.08.2001 11:01------------------------------
    Client to use the dbtools library as load-on-call
 ---------------------------------------------------------------------------*/
class SwDbtoolsClient
{
private:
    ::rtl::Reference< ::connectivity::simple::IDataAccessTools >			m_xDataAccessTools;
    ::rtl::Reference< ::connectivity::simple::IDataAccessTypeConversion >	m_xAccessTypeConversion;
    ::rtl::Reference< ::connectivity::simple::IDataAccessToolsFactory >		m_xDataAccessFactory;

    

public:
    SwDbtoolsClient(){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 SwDbtoolsClient();
    ~SwDbtoolsClient(){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 ~SwDbtoolsClient();

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource > getDataSource(
        const ::rtl::OUString& _rsRegisteredName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory
            ){DBG_BF_ASSERT(0, "STRIP"); return NULL;} //STRIP001 );


};

} //namespace binfilter
#endif

