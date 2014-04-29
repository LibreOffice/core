/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETELISTENER_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#endif
#include <com/sun/star/frame/XFrame.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <cppuhelper/implbase1.hxx>

#ifndef _SOT_EXCHANGE_HXX_
#include <sot/exchange.hxx>
#endif

namespace framework
{

class DropTargetListener : private ThreadHelpBase
                         , public ::cppu::WeakImplHelper1< ::com::sun::star::datatransfer::dnd::XDropTargetListener >
{
    //___________________________________________
    // member
    private:

        /// uno service manager to create necessary services
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
