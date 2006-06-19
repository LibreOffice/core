/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: moduleacceleratorconfiguration.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:21:59 $
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

#ifndef __FRAMEWORK_ACCELERATORS_MODULEACCELERATORCONFIGURATION_HXX_
#define __FRAMEWORK_ACCELERATORS_MODULEACCELERATORCONFIGURATION_HXX_

//__________________________________________
// own includes

#ifndef __FRAMEWORK_ACCELERATORS_ACCELERATORCONFIGURATION_HXX_
#include <accelerators/acceleratorconfiguration.hxx>
#endif

#ifndef __FRAMEWORK_ACCELERATORS_PRESETHANDLER_HXX_
#include <accelerators/presethandler.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/interface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XSERVICEINFO_HXX_
#include <macros/xserviceinfo.hxx>
#endif

//__________________________________________
// interface includes

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

//__________________________________________
// other includes

//__________________________________________
// definition

namespace framework
{

//__________________________________________
/**
    implements a read/write access to a module
    dependend accelerator configuration.
 */
class ModuleAcceleratorConfiguration : public AcceleratorConfiguration
                                     , public css::lang::XServiceInfo
                                     , public css::lang::XInitialization
{
    //______________________________________
    // member

    private:

        //----------------------------------
        /** identify the application module, where this accelerator
            configuration cache should work on. */
        ::rtl::OUString m_sModule;

    //______________________________________
    // interface

    public:

        //----------------------------------
        /** initialize this instance and fill the internal cache.

            @param  xSMGR
                    reference to an uno service manager, which is used internaly.
         */
        ModuleAcceleratorConfiguration(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR);

        //----------------------------------
        /** TODO */
        virtual ~ModuleAcceleratorConfiguration();

        // XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

    //______________________________________
    // helper

    private:

        //----------------------------------
        /** read all data into the cache. */
        void impl_ts_fillCache();
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_MODULEACCELERATORCONFIGURATION_HXX_
