/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: oemjob.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _SOCOMP_OEMJOB_HXX_
#define _SOCOMP_OEMJOB_HXX_

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <osl/mutex.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::task;

namespace desktop{

class OEMPreloadJob : public ::cppu::WeakImplHelper3< XJob, XComponent, XServiceInfo >
{

private:
    ::osl::Mutex                        m_aMutex;
    ::cppu::OInterfaceContainerHelper   m_aListeners;
    Reference< XMultiServiceFactory >   m_xServiceManager;

    sal_Bool checkOEMPreloadFlag();
    void disableOEMPreloadFlag();

public:
    OEMPreloadJob( const Reference < XMultiServiceFactory >& xFactory );
    virtual ~OEMPreloadJob();

    static ::rtl::OUString                      GetImplementationName();
    static Sequence< rtl::OUString >            GetSupportedServiceNames();


    // XComponent
    virtual void SAL_CALL dispose() throw ( RuntimeException );
    virtual void SAL_CALL addEventListener( const Reference< XEventListener > & aListener) throw ( RuntimeException );
    virtual void SAL_CALL removeEventListener(const Reference< XEventListener > & aListener) throw ( RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL    getImplementationName() throw ( RuntimeException );
    virtual sal_Bool SAL_CALL           supportsService( const ::rtl::OUString& rServiceName ) throw ( RuntimeException );
    virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw ( RuntimeException );

    //XJob
    virtual Any SAL_CALL execute(const Sequence<NamedValue>& args)throw ( RuntimeException );


    static const char* interfaces[];
    static const char* implementationName;
    static const char* serviceName;
    static Reference<XInterface> SAL_CALL CreateInstance(
        const Reference< XMultiServiceFactory >&);


};
}

#endif // _SOCOMP_OEMJOB_HXX_
