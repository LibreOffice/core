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

#ifndef __FRAMEWORK_XML_MENUCONFIGURATION_HXX_
#define __FRAMEWORK_XML_MENUCONFIGURATION_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <framework/fwedllapi.h>

#include <cppuhelper/weak.hxx>
#include <vcl/menu.hxx>
#include <vcl/toolbox.hxx>

#define BOOKMARK_NEWMENU        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:menu_bookmark_new" ))
#define BOOKMARK_WIZARDMENU     ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:menu_bookmark_wizard" ))

// Prepare for inclusion by framework and sfx
// Please consider that there is a corresponding define also in sfxsids.hrc!! (SID_SFX_START)/(SID_ADDONS)
#define FWK_SID_SFX_START 5000
#define FWK_SID_ADDONS (FWK_SID_SFX_START+1678)
#define FWK_SID_ADDONHELP (FWK_SID_SFX_START+1684)

const sal_uInt16 START_ITEMID_PICKLIST      = 4500;
const sal_uInt16 END_ITEMID_PICKLIST        = 4599;
const sal_uInt16 MAX_ITEMCOUNT_PICKLIST     =   99; // difference between START_... & END_... for picklist / must be changed too, if these values are changed!
const sal_uInt16 START_ITEMID_WINDOWLIST    = 4600;
const sal_uInt16 END_ITEMID_WINDOWLIST      = 4699;
const sal_uInt16 ITEMID_ADDONLIST           = FWK_SID_ADDONS;
const sal_uInt16 ITEMID_ADDONHELP           = FWK_SID_ADDONHELP;

namespace framework
{

class FWE_DLLPUBLIC MenuConfiguration
{
    public:
        struct Attributes
        {
            Attributes() {}
            Attributes( const ::rtl::OUString& aFrame, const ::rtl::OUString& aImageIdStr ) :
                aTargetFrame( aFrame ), aImageId( aImageIdStr ) {}

            ::rtl::OUString aTargetFrame;
            ::rtl::OUString aImageId;
            ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XDispatchProvider > xDispatchProvider;
                        sal_Int16 nStyle;
        };

        MenuConfiguration(
            // use const when giving a uno reference by reference
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rServiceManager );

        virtual ~MenuConfiguration();

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > CreateMenuBarConfigurationFromXML(
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rInputStream )
            throw ( ::com::sun::star::lang::WrappedTargetException );

        PopupMenu* CreateBookmarkMenu(
                ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                const ::rtl::OUString& aURL )
            throw ( ::com::sun::star::lang::WrappedTargetException );

        ToolBox* CreateToolBoxFromConfiguration(
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rInputStream )
            throw ( ::com::sun::star::lang::WrappedTargetException );

        void StoreMenuBarConfigurationToXML( ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& rMenuBarConfiguration,
                      ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rOutputStream )
            throw ( ::com::sun::star::lang::WrappedTargetException );

        void StoreToolBox( ToolBox* pToolBox,
                      ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rOutputStream )
            throw ( ::com::sun::star::lang::WrappedTargetException );

    private:
        // do not hold the uno reference by reference
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& m_rxServiceManager;
};

}

#endif // __FRAMEWORK_XML_MENUCONFIGURATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
