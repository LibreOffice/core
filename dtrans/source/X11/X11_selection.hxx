/*************************************************************************
 *
 *  $RCSfile: X11_selection.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: pl $ $Date: 2001-02-05 13:08:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DTRANS_X11_SELECTION_HXX_
#define _DTRANS_X11_SELECTION_HXX_

#ifndef _CPPUHELPER_COMPBASE1_HXX_
#include <cppuhelper/compbase1.hxx>
#endif

#ifndef _CPPUHELPER_COMPBASE7_HXX_
#include <cppuhelper/compbase7.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGET_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETDROPCONTEXT_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETFACTORY_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCE_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCECONTEXT_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSourceContext.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XDISPLAYCONNECTION_HPP_
#include <com/sun/star/awt/XDisplayConnection.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _OSL_CONDITION_HXX_
#include <osl/conditn.hxx>
#endif

#ifndef __SGI_STL_HASH_MAP
#include <hash_map>
#endif

#ifndef __SGI_STL_LIST
#include <list>
#endif

#include <X11/Xlib.h>

#define XDND_IMPLEMENTATION_NAME "com.sun.star.datatransfer.dnd.XdndSupport"

using namespace ::com::sun::star::uno;

namespace x11 {

// ------------------------------------------------------------------------
    rtl_TextEncoding getTextPlainEncoding( const ::rtl::OUString& rMimeType );

    class SelectionAdaptor
    {
    public:
        virtual Reference< ::com::sun::star::datatransfer::XTransferable > getTransferable() = 0;
        virtual void clearTransferable() = 0;
    };

    class DropTarget :
        public ::cppu::WeakImplHelper1< ::com::sun::star::datatransfer::dnd::XDropTarget >
    {
    public:
        ::osl::Mutex        m_aMutex;
        bool                m_bActive;
        sal_Int8            m_nDefaultActions;
        ::std::list< Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener > >
                            m_aListeners;

        DropTarget() :
                m_bActive( false ),
                m_nDefaultActions( 0 ) {}
        virtual ~DropTarget();

        // convenience functions that loop over listeners
        void dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde );
        void dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte );
        void dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde );
        void dropActionChanged( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde );
        void drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& dtde );
        // XDropTarget
        virtual void        SAL_CALL addDropTargetListener( const Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& );
        virtual void        SAL_CALL removeDropTargetListener( const Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& );
        virtual sal_Bool    SAL_CALL isActive();
        virtual void        SAL_CALL setActive( sal_Bool active );
        virtual sal_Int8    SAL_CALL getDefaultActions();
        virtual void        SAL_CALL setDefaultActions( sal_Int8 actions );
    };

    class SelectionManager :
        public ::cppu::WeakComponentImplHelper7<
            ::com::sun::star::datatransfer::dnd::XDropTargetDropContext,
            ::com::sun::star::datatransfer::dnd::XDropTargetFactory,
            ::com::sun::star::datatransfer::dnd::XDragSource,
            ::com::sun::star::datatransfer::dnd::XDragSourceContext,
            ::com::sun::star::lang::XInitialization,
            ::com::sun::star::awt::XEventHandler,
            ::com::sun::star::lang::XServiceInfo
        >,
        public SelectionAdaptor
    {
        // for INCR type selection transfer
        // INCR protocol is used if the data cannot
        // be transported at once but in parts
        // IncrementalTransfer holds the bytes to be transmitted
        // as well a the current position
        // INCR triggers the delivery of the next part by deleting the
        // property used to transfer the data
        struct IncrementalTransfer
        {
            Sequence< sal_Int8 >
                                            m_aData;
            int                             m_nBufferPos;
            Window                          m_aRequestor;
            Atom                            m_aProperty;
            Atom                            m_aTarget;
            int                             m_nFormat;
            int                             m_nTransferStartTime;

            IncrementalTransfer( const Sequence< sal_Int8 >& rData, Window aRequestor, Atom aProperty, Atom aTarget, int nFormat );
        };

        // a struct to hold the data associated with a selection
        struct Selection
        {
            enum State
            {
                Inactive, WaitingForResponse, WaitingForData, IncrementalTransfer
            };

            State                       m_eState;
            SelectionAdaptor*           m_pAdaptor;
            Atom                        m_aAtom;
            ::osl::Condition            m_aDataArrived;
            Sequence< sal_Int8 >
                                        m_aData;
            Sequence< ::com::sun::star::datatransfer::DataFlavor >
            m_aTypes;
            // this is used for caching
            // m_aTypes is invalid after 2 seconds
            int                         m_nLastTimestamp;
            bool                        m_bHaveUTF16;

            Selection() : m_eState( Inactive ),
                          m_pAdaptor( NULL ),
                          m_aAtom( None ),
                          m_nLastTimestamp( 0 ),
                          m_bHaveUTF16( false )
                {}
        };

        // a struct to hold data associated with a XDropTarget
        struct DropTargetEntry
        {
            DropTarget*     m_pTarget;
            Reference< ::com::sun::star::datatransfer::dnd::XDropTarget >
                            m_xTarget;
            Window          m_aRootWindow;

            DropTargetEntry() : m_pTarget( NULL ), m_aRootWindow( None ) {}
            DropTargetEntry( DropTarget* pTarget ) :
                    m_pTarget( pTarget ),
                    m_xTarget( pTarget ),
                    m_aRootWindow( None )
                {}
            DropTargetEntry( const DropTargetEntry& rEntry ) :
                    m_pTarget( rEntry.m_pTarget ),
                    m_xTarget( rEntry.m_xTarget ),
                    m_aRootWindow( rEntry.m_aRootWindow )
                {}
            ~DropTargetEntry() {}

            DropTarget* operator->() const { return m_pTarget; }
            DropTargetEntry& operator=(const DropTargetEntry& rEntry)
                { m_pTarget = rEntry.m_pTarget; m_xTarget = rEntry.m_xTarget; m_aRootWindow = rEntry.m_aRootWindow; return *this; }
        };

        // internal data
        Display*                    m_pDisplay;
        oslThread                   m_aThread;
        Window                      m_aWindow;
        Reference< ::com::sun::star::awt::XDisplayConnection >
                                    m_xDisplayConnection;


        // members used for Xdnd

        // drop only

        // contains the XdndEnterEvent of a drop action running
        // with one of our targets. The data.l[0] member
        // (conatining the drag source window) is set
        // to None while that is not the case
        XClientMessageEvent         m_aDropEnterEvent;
        // set to false on XdndEnter
        // set to true on first XdndPosition or XdndLeave
        bool                        m_bDropEnterSent;
        Window                      m_aCurrentDropWindow;
        // time code of XdndDrop
        Time                        m_nDropTime;
        sal_Int8                    m_nLastDropAction;
        // XTransferable for Xdnd with foreign drag source
        Reference< ::com::sun::star::datatransfer::XTransferable >
                                    m_xDropTransferable;
        int                         m_nLastX, m_nLastY;


        // drag only

        // None if no Dnd action is running with us as source
        Window                      m_aDropWindow;
        // either m_aDropWindow or its XdndProxy
        Window                      m_aDropProxy;
        // XTransferable for Xdnd when we are drag source
        Reference< ::com::sun::star::datatransfer::XTransferable >
                                    m_xDragSourceTransferable;
        Reference< ::com::sun::star::datatransfer::dnd::XDragSourceListener >
                                    m_xDragSourceListener;
        // root coordinates
        int                         m_nLastDragX, m_nLastDragY;
        Sequence< ::com::sun::star::datatransfer::DataFlavor >
                                    m_aDragFlavors;
        // the rectangle the pointer must leave until a new XdndPosition should
        // be sent. empty unless the drop target told to fill
        int                         m_nNoPosX, m_nNoPosY, m_nNoPosWidth, m_nNoPosHeight;
        int                         m_nDragButton;
        sal_Int8                    m_nUserDragAction;
        sal_Int8                    m_nSourceActions;
        bool                        m_bDropSuccess;
        bool                        m_bDropSent;
        time_t                      m_nDropTimeout;
        bool                        m_bWaitingForPrimaryConversion;

        // drag cursors
        Cursor                      m_aMoveCursor;
        Cursor                      m_aCopyCursor;
        Cursor                      m_aLinkCursor;
        Cursor                      m_aNoneCursor;
        Cursor                      m_aCurrentCursor;


        // drag and drop

        int                         m_nCurrentProtocolVersion;
        ::std::hash_map< Window, DropTargetEntry >
                                    m_aDropTargets;


        // some special atoms that are needed often
        Atom                        m_nCLIPBOARDAtom;
        Atom                        m_nTARGETSAtom;
        Atom                        m_nTEXTAtom;
        Atom                        m_nINCRAtom;
        Atom                        m_nXdndAware;
        Atom                        m_nXdndEnter;
        Atom                        m_nXdndLeave;
        Atom                        m_nXdndPosition;
        Atom                        m_nXdndStatus;
        Atom                        m_nXdndDrop;
        Atom                        m_nXdndFinished;
        Atom                        m_nXdndSelection;
        Atom                        m_nXdndTypeList;
        Atom                        m_nXdndProxy;
        Atom                        m_nXdndActionCopy;
        Atom                        m_nXdndActionMove;
        Atom                        m_nXdndActionLink;
        Atom                        m_nXdndActionAsk;
        Atom                        m_nXdndActionPrivate;

        // caching for atoms
        ::std::hash_map< Atom, ::rtl::OUString >
                                    m_aAtomToString;
        ::std::hash_map< ::rtl::OUString, Atom, ::rtl::OUStringHash >
                                    m_aStringToAtom;

        // the registered selections
        ::std::hash_map< Atom, Selection* >
                                    m_aSelections;
        // IncrementalTransfers in progress
        ::std::hash_map< Window, ::std::list< IncrementalTransfer > >
                                    m_aIncrementals;

        // do not use X11 multithreading capabilities
        // since this leads to deadlocks in different Xlib implentations
        // (XFree as well as Xsun) use an own mutex instead
        ::osl::Mutex                m_aMutex;
        // currently this service supports only one display
        // this should be changed to a hash_map indexed by the
        // display name in the future
        static SelectionManager*    m_pInstance;

        SelectionManager();
        ~SelectionManager();

        SelectionAdaptor* getAdaptor( Atom selection );

        // handle various events
        void handleSelectionRequest( XSelectionRequestEvent& rRequest );
        void handleSendPropertyNotify( XPropertyEvent& rNotify );
        void handleReceivePropertyNotify( XPropertyEvent& rNotify );
        void handleSelectionNotify( XSelectionEvent& rNotify );
        void handleDragEvent( XEvent& rMessage );
        void handleDropEvent( XClientMessageEvent& rMessage );

        // dnd helpers
        void sendDragStatus( Atom nDropAction );
        void sendDropPosition( bool bForce, Time eventTime );
        bool updateDragAction( int modifierState );
        int getXdndVersion( Window aWindow, Window& rProxy );
        Cursor createCursor( const char* pPointerData, const char* pMaskData, int width, int height, int hotX, int hotY );
        // coordinates on root window
        void updateDragWindow( int nX, int nY, Window aRoot );

        bool getPasteData( Atom selection, Atom type, Sequence< sal_Int8 >& rData );
        // returns true if conversion was successful
        bool convertData( const Reference< ::com::sun::star::datatransfer::XTransferable >& xTransferable,
                          const ::rtl::OUString& rType,
                          Sequence< sal_Int8 >& rData );

        // thread dispatch loop
        static void run( void* );
        void dispatchEvent( int millisec );
        void handleXEvent( XEvent& rEvent );
    public:
        static SelectionManager& get();

        Display * getDisplay() { return m_pDisplay; };
        Window getWindow() { return m_aWindow; };


        void registerHandler( Atom selection, SelectionAdaptor& rAdaptor );
        void deregisterHandler( Atom selection );
        bool requestOwnership( Atom selection );


        Atom getAtom( const ::rtl::OUString& rString );
        const ::rtl::OUString& getString( Atom nAtom );

        // type conversion
        Atom convertTypeToNative( const ::rtl::OUString& rType, Atom selection, int& rFormat );
        ::rtl::OUString convertTypeFromNative( Atom nType, Atom selection );

        Reference< ::com::sun::star::datatransfer::dnd::XDropTarget >
            registerDropTarget( Window aWindow );
        void deregisterDropTarget( const Reference< ::com::sun::star::datatransfer::dnd::XDropTarget >& xTarget );

        // methods for transferable
        bool getPasteDataTypes( Atom selection, Sequence< ::com::sun::star::datatransfer::DataFlavor >& rTypes );
        bool getPasteData( Atom selection, const ::rtl::OUString& rType, Sequence< sal_Int8 >& rData );

        // XDropTargetDragContext
        virtual void        SAL_CALL accept( sal_Int8 dragOperation );
        virtual void        SAL_CALL reject();
        virtual Sequence< ::com::sun::star::datatransfer::DataFlavor >
                            SAL_CALL getCurrentDataFlavors();
        virtual sal_Bool    SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& df );

        // XDropTargetDropContext
        virtual void        SAL_CALL dropComplete( sal_Bool success );

        // XDropTargetFactory
        virtual Reference< ::com::sun::star::datatransfer::dnd::XDropTarget >
                            SAL_CALL createDropTarget( const Sequence< sal_Int8 >& windowId );
        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName();
        virtual sal_Bool    SAL_CALL supportsService( const ::rtl::OUString& ServiceName );
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
                            SAL_CALL getSupportedServiceNames();

        // XInitialization
        virtual void        SAL_CALL initialize( const Sequence< Any >& arguments );

        // XEventHandler
        virtual sal_Bool    SAL_CALL handleEvent( const Any& event );

        // XDragSource
        virtual sal_Bool    SAL_CALL isDragImageSupported();
        virtual sal_Int32   SAL_CALL getDefaultCursor( sal_Int8 dragAction );
        virtual void        SAL_CALL executeDrag(
            const ::com::sun::star::datatransfer::dnd::DragGestureEvent& trigger,
            sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image,
            const Reference< ::com::sun::star::datatransfer::XTransferable >& transferable,
            const Reference< ::com::sun::star::datatransfer::dnd::XDragSourceListener >& listener
            );

        // XDragSourceContext
        virtual sal_Int32   SAL_CALL getCurrentCursor();
        virtual void        SAL_CALL setCursor( sal_Int32 cursor );
        virtual void        SAL_CALL setImage( sal_Int32 image );
        virtual void        SAL_CALL transferablesFlavorsChanged();

        // SelectionAdaptor for XdndSelection Drag (we are drag source)
        virtual Reference< ::com::sun::star::datatransfer::XTransferable > getTransferable();
        virtual void clearTransferable();
    };

// ------------------------------------------------------------------------

    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL Xdnd_getSupportedServiceNames();
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Xdnd_createInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMultiServiceFactory);

// ------------------------------------------------------------------------

}

#endif
