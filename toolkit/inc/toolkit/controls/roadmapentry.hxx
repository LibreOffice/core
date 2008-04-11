/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: roadmapentry.hxx,v $
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
#ifndef _TOOLKIT_ROADMAPENTRY_HXX_
#define _TOOLKIT_ROADMAPENTRY_HXX_

#include <cppuhelper/implbase1.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/proparrhlp.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>


#define RM_PROPERTY_ID_LABEL        1
#define RM_PROPERTY_ID_ID           2
#define RM_PROPERTY_ID_ENABLED      4
#define RM_PROPERTY_ID_INTERACTIVE  5

typedef ::cppu::WeakImplHelper1 <   ::com::sun::star::lang::XServiceInfo
                                >   ORoadmapEntry_Base;

class ORoadmapEntry :public ORoadmapEntry_Base
            ,public ::comphelper::OMutexAndBroadcastHelper
            ,public ::comphelper::OPropertyContainer
            ,public ::comphelper::OPropertyArrayUsageHelper< ORoadmapEntry >
{

public:
       ORoadmapEntry();

protected:
    DECLARE_XINTERFACE()        // merge XInterface implementations
    DECLARE_XTYPEPROVIDER()     // merge XTypeProvider implementations

    /// @see ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
        SAL_CALL getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);

    // OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;

    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);


    // other stuff
    // ...
    // (e.g. DECLARE_SERVICE_INFO();)

protected:
    // <properties>
    ::rtl::OUString     m_sLabel;
    sal_Int32           m_nID;
    sal_Bool            m_bEnabled;
    sal_Bool            m_bInteractive;

    // </properties>
};

#endif

