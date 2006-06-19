/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: documentacceleratorconfiguration.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:21:18 $
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

#ifndef __FRAMEWORK_ACCELERATORS_DOCUMENTACCELERATORCONFIGURATION_HXX_
#define __FRAMEWORK_ACCELERATORS_DOCUMENTACCELERATORCONFIGURATION_HXX_

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

#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONSTORAGE_HPP_
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#endif

//__________________________________________
// other includes

//__________________________________________
// definition

namespace framework
{

//__________________________________________
/**
    implements a read/write access to a document
    based accelerator configuration.
 */
class DocumentAcceleratorConfiguration : public AcceleratorConfiguration
                                       , public css::lang::XServiceInfo
                                       , public css::lang::XInitialization
//                                       , public css::ui::XUIConfigurationStorage
{
    //______________________________________
    // member

    private:

        //----------------------------------
        /** points to the root storage of the outside document,
            where we can read/save our configuration data. */
        css::uno::Reference< css::embed::XStorage > m_xDocumentRoot;

    //______________________________________
    // interface

    public:

        //----------------------------------
        /** initialize this instance and fill the internal cache.

            @param  xSMGR
                    reference to an uno service manager, which is used internaly.
         */
        DocumentAcceleratorConfiguration(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR);
        virtual ~DocumentAcceleratorConfiguration();

        // XInterface, XTypeProvider, XServiceInfo
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        // XUIConfigurationStorage
        virtual void SAL_CALL setStorage(const css::uno::Reference< css::embed::XStorage >& xStorage)
            throw(css::uno::RuntimeException);

        virtual sal_Bool SAL_CALL hasStorage()
            throw(css::uno::RuntimeException);

    //______________________________________
    // helper

    private:

        //----------------------------------
        /** read all data into the cache. */
        void impl_ts_fillCache();

        //----------------------------------
        /** forget all currently cached data AND(!)
            forget all currently used storages. */
        void impl_ts_clearCache();
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_DOCUMENTACCELERATORCONFIGURATION_HXX_
