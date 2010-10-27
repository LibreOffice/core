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

#ifndef __FRAMEWORK_CLASSES_DROPTARGETLISTENER_HXX_
#define __FRAMEWORK_CLASSES_DROPTARGETLISTENER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/threadhelpbase.hxx>
#include <general.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#include <com/sun/star/frame/XFrame.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <cppuhelper/implbase1.hxx>

#include <sot/exchange.hxx>

namespace framework
{

class DropTargetListener : private ThreadHelpBase
                         , public ::cppu::WeakImplHelper1< ::com::sun::star::datatransfer::dnd::XDropTargetListener >
{
    //___________________________________________
    // member
    private:

        /// uno service manager to create neccessary services
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;
        /// weakreference to target frame (Don't use a hard reference. Owner can't delete us then!)
        css::uno::WeakReference< css::frame::XFrame > m_xTargetFrame;
        /// drag/drop info
        DataFlavorExVector* m_pFormats;

    //___________________________________________
    // c++ interface
    public:

         DropTargetListener( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory ,
                             const css::uno::Reference< css::frame::XFrame >& xFrame                );
        ~DropTargetListener(                                                                        );

    //___________________________________________
    // uno interface
    public:

        // XEventListener
        virtual void SAL_CALL disposing        ( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException);

        // XDropTargetListener
        virtual void SAL_CALL drop             ( const css::datatransfer::dnd::DropTargetDropEvent&      dtde  ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL dragEnter        ( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL dragExit         ( const css::datatransfer::dnd::DropTargetEvent&          dte   ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL dragOver         ( const css::datatransfer::dnd::DropTargetDragEvent&      dtde  ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL dropActionChanged( const css::datatransfer::dnd::DropTargetDragEvent&      dtde  ) throw(css::uno::RuntimeException);

    //___________________________________________
    // internal helper
    private:

        void     implts_BeginDrag            ( const css::uno::Sequence< css::datatransfer::DataFlavor >& rSupportedDataFlavors );
        void     implts_EndDrag              (                                                                                  );
        sal_Bool implts_IsDropFormatSupported( SotFormatStringId nFormat                                                        );
        void     implts_OpenFile             ( const String& rFilePath                                                          );

}; // class DropTargetListener

} // namespace framework

#endif // __FRAMEWORK_CLASSES_DROPTARGETLISTENER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
