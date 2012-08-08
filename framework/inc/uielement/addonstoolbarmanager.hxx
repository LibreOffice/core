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

#ifndef __FRAMEWORK_UIELEMENT_ADDONSTOOLBARMANAGER_HXX_
#define __FRAMEWORK_UIELEMENT_ADDONSTOOLBARMANAGER_HXX_

#include <uielement/toolbarmanager.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <vcl/toolbox.hxx>


namespace framework
{

class ToolBar;
class AddonsToolBarManager : public ToolBarManager
{
    public:
        AddonsToolBarManager( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& rServicveManager,
                              const com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame,
                              const rtl::OUString& rResourceName,
                              ToolBar* pToolBar );
        virtual ~AddonsToolBarManager();

        // XComponent
        void SAL_CALL dispose() throw ( ::com::sun::star::uno::RuntimeException );

        virtual void RefreshImages();
        using ToolBarManager::FillToolbar;
        void FillToolbar( const com::sun::star::uno::Sequence< com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > >& rAddonToolbar );

    protected:
        DECL_LINK(Click, void *);
        DECL_LINK(DoubleClick, void *);
        DECL_LINK(Command, void *);
        DECL_LINK(Select, void *);
        DECL_LINK(Activate, void *);
        DECL_LINK(Deactivate, void *);
        DECL_LINK( StateChanged, StateChangedType* );
        DECL_LINK( DataChanged, DataChangedEvent* );

        virtual bool MenuItemAllowed( sal_uInt16 ) const;
};

}

#endif // __FRAMEWORK_UIELEMENT_ADDONSTOOLBARMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
