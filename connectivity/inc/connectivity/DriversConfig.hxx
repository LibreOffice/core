/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: makefile,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: st $ $Date: 2000/11/22 02:32:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
        ::utl::OConfigurationTreeRoot m_aInstalled;
    public:
        DriversConfigImpl();
        void Load(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,TInstalledDrivers& _rDrivers);
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

        ::rtl::OUString getDriverFactoryName(const ::rtl::OUString& _sUrl) const;
        ::rtl::OUString getDriverTypeDisplayName(const ::rtl::OUString& _sUrl) const;
        const ::comphelper::NamedValueCollection& getProperties(const ::rtl::OUString& _sURL) const;
        const ::comphelper::NamedValueCollection& getFeatures(const ::rtl::OUString& _sURL) const;
        const ::comphelper::NamedValueCollection& getMetaData(const ::rtl::OUString& _sURL) const;
        ::com::sun::star::uno::Sequence< ::rtl::OUString > getURLs() const;
    private:
        TInstalledDrivers   m_aDrivers;
        OSharedConfigNode   m_aNode;
    };
}
#endif // CONNECTIVITY_DRIVERSCONFIG_HXX_INCLUDED
