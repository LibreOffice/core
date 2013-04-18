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

#define BOOKMARK_NEWMENU        OUString( "private:menu_bookmark_new" )
#define BOOKMARK_WIZARDMENU     OUString( "private:menu_bookmark_wizard" )

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
            Attributes( const OUString& aFrame, const OUString& aImageIdStr ) :
                aTargetFrame( aFrame ), aImageId( aImageIdStr ) {}

            OUString aTargetFrame;
            OUString aImageId;
            ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XDispatchProvider > xDispatchProvider;
                        sal_Int16 nStyle;
        };

        MenuConfiguration(
            // use const when giving a uno reference by reference
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );

        virtual ~MenuConfiguration();

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > CreateMenuBarConfigurationFromXML(
            ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rInputStream )
            throw ( ::com::sun::star::lang::WrappedTargetException );

        PopupMenu* CreateBookmarkMenu(
                ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                const OUString& aURL )
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
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext> m_xContext;
};

}

#endif // __FRAMEWORK_XML_MENUCONFIGURATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
