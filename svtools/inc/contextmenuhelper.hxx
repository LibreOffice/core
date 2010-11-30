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

#ifndef INCLUDED_SVTOOLS_CONTEXTMENUHELPER_HXX
#define INCLUDED_SVTOOLS_CONTEXTMENUHELPER_HXX

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <vcl/menu.hxx>
#include "svtools/svtdllapi.h"

namespace svt
{

/**
    Context menu helper class.

    Fills images and labels for a provided popup menu or
    com.sun.star.awt.XPopupMenu.

    PRECONDITION:
    All commands must be set via SetItemCommand and are part
    of the configuration files
    (see org.openoffice.Office.UI.[Module]Commands.xcu)
*/
struct ExecuteInfo;
class SVT_DLLPUBLIC ContextMenuHelper
{
    public:
        // create context menu helper
        // ARGS: xFrame = frame defines the context of the context menu
        //       bAutoRefresh = specifies that the context will be constant or not
        ContextMenuHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame, bool bAutoRefresh=true );
        ~ContextMenuHelper();

        // methods to complete a popup menu (set images, labels, enable/disable states)
        // ATTENTION: The item ID's must be unique for the whole popup (inclusive the sub menus!)
        void completeAndExecute( const Point& aPos, PopupMenu& aPopupMenu );
        void completeAndExecute( const Point& aPos, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu >& xPopupMenu );

        // methods to create a popup menu referenced by resource URL
        // NOT IMPLEMENTED YET!
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPopupMenu > create( const ::rtl::OUString& aPopupMenuResourceURL );

        // method to create and execute a popup menu referenced by a resource URL
        // NOT IMPLEMENTED YET!
        bool createAndExecute( const Point& aPos, const ::rtl::OUString& aPopupMenuResourceURL );

    private:
        // asynchronous link to prevent destruction while on stack
        DECL_STATIC_LINK( ContextMenuHelper, ExecuteHdl_Impl, ExecuteInfo* );

        // no copy-ctor and operator=
        ContextMenuHelper( const ContextMenuHelper& );
        const ContextMenuHelper& operator=( const ContextMenuHelper& );

        // show context menu and dispatch command automatically
        void            executePopupMenu( const Point& aPos, PopupMenu* pMenu );

        // fill image and label for every menu item on the provided menu
        void            completeMenuProperties( Menu* pMenu );

        // dispatch provided command
        bool            dispatchCommand( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame, const ::rtl::OUString& aCommandURL );


        // methods to retrieve a single command URL dependent value from a
        // ui configuratin manager
        Image           getImageFromCommandURL( const ::rtl::OUString& aCmdURL ) const;
        rtl::OUString   getLabelFromCommandURL( const ::rtl::OUString& aCmdURL ) const;

        // creates an association between current module/controller bound to the
        // provided frame and their ui configuration managers.
        bool            associateUIConfigurationManagers();

        // resets associations to create associations again on-demand.
        // Usefull for implementations which recycle frames. Normal
        // implementations can profit from caching and should set
        // auto refresh on ctor to false (default).
        void            resetAssociations()
        {
            if ( m_bAutoRefresh )
                m_bUICfgMgrAssociated = false;
        }

        ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XFrame >         m_xWeakFrame;
        ::rtl::OUString                                                                 m_aModuleIdentifier;
        ::rtl::OUString                                                                 m_aSelf;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer >     m_xURLTransformer;
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >       m_aDefaultArgs;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XImageManager >         m_xDocImageMgr;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XImageManager >         m_xModuleImageMgr;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xUICommandLabels;
        bool                                                                            m_bAutoRefresh;
        bool                                                                            m_bUICfgMgrAssociated;
};

} // namespace svt

#endif // INCLUDED_SVTOOLS_CONTEXTMENUHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
