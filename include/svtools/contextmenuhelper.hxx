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

#ifndef INCLUDED_SVTOOLS_CONTEXTMENUHELPER_HXX
#define INCLUDED_SVTOOLS_CONTEXTMENUHELPER_HXX

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakref.hxx>
#include <vcl/menu.hxx>
#include <svtools/svtdllapi.h>

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
        ContextMenuHelper( const css::uno::Reference< css::frame::XFrame >& xFrame, bool bAutoRefresh=true );
        ~ContextMenuHelper();

        // methods to complete a popup menu (set images, labels, enable/disable states)
        // ATTENTION: The item ID's must be unique for the whole popup (inclusive the sub menus!)
        void completeAndExecute( const Point& aPos, PopupMenu& aPopupMenu );
        void completeAndExecute( const Point& aPos, const css::uno::Reference< css::awt::XPopupMenu >& xPopupMenu );

    private:
        // asynchronous link to prevent destruction while on stack
        DECL_STATIC_LINK_TYPED( ContextMenuHelper, ExecuteHdl_Impl, void*, void );

        ContextMenuHelper( const ContextMenuHelper& ) = delete;
        const ContextMenuHelper& operator=( const ContextMenuHelper& ) = delete;

        // show context menu and dispatch command automatically
        void            executePopupMenu( const Point& aPos, PopupMenu* pMenu );

        // fill image and label for every menu item on the provided menu
        void            completeMenuProperties( Menu* pMenu );

        // dispatch provided command
        bool            dispatchCommand( const css::uno::Reference< css::frame::XFrame >& xFrame, const OUString& aCommandURL );


        // methods to retrieve a single command URL dependent value from a
        // ui configuration manager
        Image           getImageFromCommandURL( const OUString& aCmdURL ) const;
        OUString        getLabelFromCommandURL( const OUString& aCmdURL ) const;

        // creates an association between current module/controller bound to the
        // provided frame and their ui configuration managers.
        bool            associateUIConfigurationManagers();

        // resets associations to create associations again on-demand.
        // Useful for implementations which recycle frames. Normal
        // implementations can profit from caching and should set
        // auto refresh on ctor to false (default).
        void            resetAssociations()
        {
            if ( m_bAutoRefresh )
                m_bUICfgMgrAssociated = false;
        }

        css::uno::WeakReference< css::frame::XFrame >         m_xWeakFrame;
        OUString                                              m_aSelf;
        css::uno::Reference< css::util::XURLTransformer >     m_xURLTransformer;
        css::uno::Sequence< css::beans::PropertyValue >       m_aDefaultArgs;
        css::uno::Reference< css::ui::XImageManager >         m_xDocImageMgr;
        css::uno::Reference< css::ui::XImageManager >         m_xModuleImageMgr;
        css::uno::Reference< css::container::XNameAccess >    m_xUICommandLabels;
        bool                                                  m_bAutoRefresh;
        bool                                                  m_bUICfgMgrAssociated;
};

} // namespace svt

#endif // INCLUDED_SVTOOLS_CONTEXTMENUHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
