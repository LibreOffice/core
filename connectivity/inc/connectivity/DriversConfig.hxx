/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef CONNECTIVITY_DRIVERSCONFIG_HXX_INCLUDED
#define CONNECTIVITY_DRIVERSCONFIG_HXX_INCLUDED

#include <comphelper/stl_types.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/uno/Sequence.h>
#include "connectivity/dbtoolsdllapi.hxx"
#include <salhelper/singletonref.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <unotools/confignode.hxx>

namespace connectivity
{
    typedef struct
    {
        ::comphelper::NamedValueCollection aProperties;
        ::comphelper::NamedValueCollection aFeatures;
        ::comphelper::NamedValueCollection aMetaData;
        ::rtl::OUString sDriverFactory;
        ::rtl::OUString sDriverTypeDisplayName;
    } TInstalledDriver;
    DECLARE_STL_USTRINGACCESS_MAP( TInstalledDriver, TInstalledDrivers);

    class DriversConfigImpl
    {
        mutable ::utl::OConfigurationTreeRoot   m_aInstalled;
        mutable TInstalledDrivers       m_aDrivers;
        void Load(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB) const;
    public:
        DriversConfigImpl();

        const TInstalledDrivers& getInstalledDrivers(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB) const { Load(_rxORB); return m_aDrivers; }
    };
    //
    // Allows to access all driver which are located in the configuration
    //
    class OOO_DLLPUBLIC_DBTOOLS DriversConfig
    {
        typedef salhelper::SingletonRef<DriversConfigImpl> OSharedConfigNode;

        const ::comphelper::NamedValueCollection& impl_get(const ::rtl::OUString& _sURL,sal_Int32 _nProps) const;
    public:
        DriversConfig(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
        ~DriversConfig();

        DriversConfig( const DriversConfig& );
        DriversConfig& operator=( const DriversConfig& );

        ::rtl::OUString getDriverFactoryName(const ::rtl::OUString& _sUrl) const;
        ::rtl::OUString getDriverTypeDisplayName(const ::rtl::OUString& _sUrl) const;
        const ::comphelper::NamedValueCollection& getProperties(const ::rtl::OUString& _sURL) const;
        const ::comphelper::NamedValueCollection& getFeatures(const ::rtl::OUString& _sURL) const;
        const ::comphelper::NamedValueCollection& getMetaData(const ::rtl::OUString& _sURL) const;
        ::com::sun::star::uno::Sequence< ::rtl::OUString > getURLs() const;
    private:
        OSharedConfigNode   m_aNode;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xORB;
    };
}
#endif // CONNECTIVITY_DRIVERSCONFIG_HXX_INCLUDED

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
