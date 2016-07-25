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

#ifndef INCLUDED_VCL_UNX_GENERIC_DTRANS_X11_SELECTION_HXX
#define INCLUDED_VCL_UNX_GENERIC_DTRANS_X11_SELECTION_HXX

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/awt/XDisplayConnection.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <osl/thread.h>

#include <osl/conditn.hxx>

#include <list>
#include <unordered_map>

#include <X11/Xlib.h>

#define XDND_IMPLEMENTATION_NAME "com.sun.star.datatransfer.dnd.XdndSupport"
#define XDND_DROPTARGET_IMPLEMENTATION_NAME "com.sun.star.datatransfer.dnd.XdndDropTarget"

namespace x11 {

    class PixmapHolder; // in bmp.hxx
    class SelectionManager;

    rtl_TextEncoding getTextPlainEncoding( const OUString& rMimeType );

    class SelectionAdaptor
    {
    public:
        virtual css::uno::Reference< css::datatransfer::XTransferable > getTransferable() = 0;
        virtual void clearTransferable() = 0;
        virtual void fireContentsChanged() = 0;
        virtual css::uno::Reference< css::uno::XInterface > getReference() = 0;
        // returns a reference that will keep the SelectionAdaptor alive until the
        // reference is released

    protected:
        ~SelectionAdaptor() {}
    };

    class DropTarget :
        public ::cppu::WeakComponentImplHelper<
            css::datatransfer::dnd::XDropTarget,
            css::lang::XInitialization,
            css::lang::XServiceInfo
        >
    {
    public:
        ::osl::Mutex                m_aMutex;
        bool                        m_bActive;
        sal_Int8                    m_nDefaultActions;
        ::Window                    m_aTargetWindow;
        SelectionManager*           m_pSelectionManager;
        css::uno::Reference< css::datatransfer::dnd::XDragSource >
                                    m_xSelectionManager;
        ::std::list< css::uno::Reference< css::datatransfer::dnd::XDropTargetListener > >
                            m_aListeners;

        DropTarget();
        virtual ~DropTarget();

        // convenience functions that loop over listeners
        void dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtde ) throw();
        void dragExit( const css::datatransfer::dnd::DropTargetEvent& dte ) throw();
        void dragOver( const css::datatransfer::dnd::DropTargetDragEvent& dtde ) throw();
        void drop( const css::datatransfer::dnd::DropTargetDropEvent& dtde ) throw();

        // XInitialization
        virtual void        SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& args ) throw ( css::uno::Exception, std::exception ) override;

        // XDropTarget
        virtual void        SAL_CALL addDropTargetListener( const css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >& ) throw(std::exception) override;
        virtual void        SAL_CALL removeDropTargetListener( const css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >& ) throw(std::exception) override;
        virtual sal_Bool    SAL_CALL isActive() throw(std::exception) override;
        virtual void        SAL_CALL setActive( sal_Bool active ) throw(std::exception) override;
        virtual sal_Int8    SAL_CALL getDefaultActions() throw(std::exception) override;
        virtual void        SAL_CALL setDefaultActions( sal_Int8 actions ) throw(std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(std::exception) override;
        virtual sal_Bool    SAL_CALL supportsService( const OUString& ServiceName ) throw(std::exception) override;
        virtual css::uno::Sequence< OUString >
                            SAL_CALL getSupportedServiceNames() throw(std::exception) override;
    };

    class SelectionManagerHolder :
        public ::cppu::WeakComponentImplHelper<
            css::datatransfer::dnd::XDragSource,
            css::lang::XInitialization,
            css::lang::XServiceInfo
        >
    {
        ::osl::Mutex m_aMutex;
        css::uno::Reference< css::datatransfer::dnd::XDragSource >
            m_xRealDragSource;
    public:
        SelectionManagerHolder();
        virtual ~SelectionManagerHolder();

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(std::exception) override;
        virtual sal_Bool    SAL_CALL supportsService( const OUString& ServiceName ) throw(std::exception) override;
        virtual css::uno::Sequence< OUString >
                            SAL_CALL getSupportedServiceNames() throw(std::exception) override;

        // XInitialization
        virtual void        SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& arguments ) throw( css::uno::Exception, std::exception ) override;

        // XDragSource
        virtual sal_Bool    SAL_CALL isDragImageSupported() throw(std::exception) override;
        virtual sal_Int32   SAL_CALL getDefaultCursor( sal_Int8 dragAction ) throw(std::exception) override;
        virtual void        SAL_CALL startDrag(
            const css::datatransfer::dnd::DragGestureEvent& trigger,
            sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image,
            const css::uno::Reference< css::datatransfer::XTransferable >& transferable,
            const css::uno::Reference< css::datatransfer::dnd::XDragSourceListener >& listener
            ) throw(std::exception) override;

    };

    class SelectionManager :
        public ::cppu::WeakImplHelper<
            css::datatransfer::dnd::XDragSource,
            css::lang::XInitialization,
            css::awt::XEventHandler,
            css::frame::XTerminateListener
        >,
        public SelectionAdaptor
    {
        static std::unordered_map< OUString, SelectionManager*, OUStringHash >& getInstances();

        // for INCR type selection transfer
        // INCR protocol is used if the data cannot
        // be transported at once but in parts
        // IncrementalTransfer holds the bytes to be transmitted
        // as well a the current position
        // INCR triggers the delivery of the next part by deleting the
        // property used to transfer the data
        struct IncrementalTransfer
        {
            css::uno::Sequence< sal_Int8 >  m_aData;
            int                             m_nBufferPos;
            ::Window                        m_aRequestor;
            Atom                            m_aProperty;
            Atom                            m_aTarget;
            int                             m_nFormat;
            int                             m_nTransferStartTime;
        };
        int m_nIncrementalThreshold;

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
            css::uno::Sequence< sal_Int8 > m_aData;
            css::uno::Sequence< css::datatransfer::DataFlavor >
                                        m_aTypes;
            std::vector< Atom >         m_aNativeTypes;
            // this is used for caching
            // m_aTypes is invalid after 2 seconds
            // m_aNativeTypes contains the corresponding original atom
            Atom                        m_aRequestedType;
            // m_aRequestedType is only valid while WaitingForResponse and WaitingFotData
            int                         m_nLastTimestamp;
            bool                        m_bHaveUTF16;
            Atom                        m_aUTF8Type;
            bool                        m_bHaveCompound;
            bool                        m_bOwner;
            ::Window                    m_aLastOwner;
            PixmapHolder*               m_pPixmap;
            // m_nOrigTimestamp contains the Timestamp at which the seclection
            // was acquired; needed for TimeSTAMP target
            Time                        m_nOrigTimestamp;

            Selection() : m_eState( Inactive ),
                          m_pAdaptor( nullptr ),
                          m_aAtom( None ),
                          m_aRequestedType( None ),
                          m_nLastTimestamp( 0 ),
                          m_bHaveUTF16( false ),
                          m_aUTF8Type( None ),
                          m_bHaveCompound( false ),
                          m_bOwner( false ),
                          m_aLastOwner( None ),
                          m_pPixmap( nullptr ),
                          m_nOrigTimestamp( CurrentTime )
                {}
        };

        // a struct to hold data associated with a XDropTarget
        struct DropTargetEntry
        {
            DropTarget*     m_pTarget;
            ::Window        m_aRootWindow;

            DropTargetEntry() : m_pTarget( nullptr ), m_aRootWindow( None ) {}
            explicit DropTargetEntry( DropTarget* pTarget ) :
                    m_pTarget( pTarget ),
                    m_aRootWindow( None )
                {}
            DropTargetEntry( const DropTargetEntry& rEntry ) :
                    m_pTarget( rEntry.m_pTarget ),
                    m_aRootWindow( rEntry.m_aRootWindow )
                {}
            ~DropTargetEntry() {}

            DropTarget* operator->() const { return m_pTarget; }
            DropTargetEntry& operator=(const DropTargetEntry& rEntry)
                { m_pTarget = rEntry.m_pTarget; m_aRootWindow = rEntry.m_aRootWindow; return *this; }
        };

        // internal data
        Display*                    m_pDisplay;
        oslThread                   m_aThread;
        int                         m_EndThreadPipe[2];
        oslThread                   m_aDragExecuteThread;
        ::osl::Condition            m_aDragRunning;
        ::Window                    m_aWindow;
        css::uno::Reference< css::frame::XDesktop2 > m_xDesktop;
        css::uno::Reference< css::awt::XDisplayConnection >
                                    m_xDisplayConnection;
        sal_Int32                   m_nSelectionTimeout;
        Time                        m_nSelectionTimestamp;

        // members used for Xdnd

        // drop only

        // contains the XdndEnterEvent of a drop action running
        // with one of our targets. The data.l[0] member
        // (containing the drag source ::Window) is set
        // to None while that is not the case
        XClientMessageEvent         m_aDropEnterEvent;
        // set to false on XdndEnter
        // set to true on first XdndPosition or XdndLeave
        bool                        m_bDropEnterSent;
        ::Window                    m_aCurrentDropWindow;
        // Time code of XdndDrop
        Time                        m_nDropTime;
        sal_Int8                    m_nLastDropAction;
        // XTransferable for Xdnd with foreign drag source
        css::uno::Reference< css::datatransfer::XTransferable >
                                    m_xDropTransferable;
        int                         m_nLastX, m_nLastY;
        Time                        m_nDropTimestamp;
        // set to true when calling drop()
        // if another XdndEnter is received this shows that
        // someone forgot to call dropComplete - we should reset
        // and react to the new drop
        bool                        m_bDropWaitingForCompletion;

        // drag only

        // None if no Dnd action is running with us as source
        ::Window                    m_aDropWindow;
        // either m_aDropWindow or its XdndProxy
        ::Window                    m_aDropProxy;
        ::Window                    m_aDragSourceWindow;
        // XTransferable for Xdnd when we are drag source
        css::uno::Reference< css::datatransfer::XTransferable >
                                    m_xDragSourceTransferable;
        css::uno::Reference< css::datatransfer::dnd::XDragSourceListener >
                                    m_xDragSourceListener;
        // root coordinates
        int                         m_nLastDragX, m_nLastDragY;
        css::uno::Sequence< css::datatransfer::DataFlavor >
                                    m_aDragFlavors;
        // the rectangle the pointer must leave until a new XdndPosition should
        // be sent. empty unless the drop target told to fill
        int                         m_nNoPosX, m_nNoPosY, m_nNoPosWidth, m_nNoPosHeight;
        unsigned int                m_nDragButton;
        sal_Int8                    m_nUserDragAction;
        sal_Int8                    m_nTargetAcceptAction;
        sal_Int8                    m_nSourceActions;
        bool                        m_bLastDropAccepted;
        bool                        m_bDropSuccess;
        bool                        m_bDropSent;
        time_t                      m_nDropTimeout;
        bool                        m_bWaitingForPrimaryConversion;
        Time                        m_nDragTimestamp;

        // drag cursors
        Cursor                      m_aMoveCursor;
        Cursor                      m_aCopyCursor;
        Cursor                      m_aLinkCursor;
        Cursor                      m_aNoneCursor;
        Cursor                      m_aCurrentCursor;

        // drag and drop

        int                         m_nCurrentProtocolVersion;
        std::unordered_map< ::Window, DropTargetEntry >
                                    m_aDropTargets;

        // some special atoms that are needed often
        Atom                        m_nCLIPBOARDAtom;
        Atom                        m_nTARGETSAtom;
        Atom                        m_nTIMESTAMPAtom;
        Atom                        m_nTEXTAtom;
        Atom                        m_nINCRAtom;
        Atom                        m_nCOMPOUNDAtom;
        Atom                        m_nMULTIPLEAtom;
        Atom                        m_nUTF16Atom;
        Atom                        m_nImageBmpAtom;
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
        std::unordered_map< Atom, OUString >
                                    m_aAtomToString;
        std::unordered_map< OUString, Atom, OUStringHash >
                                    m_aStringToAtom;

        // the registered selections
        std::unordered_map< Atom, Selection* >
                                    m_aSelections;
        // IncrementalTransfers in progress
        std::unordered_map< ::Window, std::unordered_map< Atom, IncrementalTransfer > >
                                    m_aIncrementals;

        // do not use X11 multithreading capabilities
        // since this leads to deadlocks in different Xlib implentations
        // (XFree as well as Xsun) use an own mutex instead
        ::osl::Mutex                m_aMutex;
        bool                        m_bShutDown;

        SelectionManager();
        virtual ~SelectionManager();

        SelectionAdaptor* getAdaptor( Atom selection );
        PixmapHolder* getPixmapHolder( Atom selection );

        // handle various events
        bool handleSelectionRequest( XSelectionRequestEvent& rRequest );
        bool handleSendPropertyNotify( XPropertyEvent& rNotify );
        bool handleReceivePropertyNotify( XPropertyEvent& rNotify );
        bool handleSelectionNotify( XSelectionEvent& rNotify );
        bool handleDragEvent( XEvent& rMessage );
        bool handleDropEvent( XClientMessageEvent& rMessage );

        // dnd helpers
        void sendDragStatus( Atom nDropAction );
        void sendDropPosition( bool bForce, Time eventTime );
        bool updateDragAction( int modifierState );
        int getXdndVersion( ::Window aXLIB_Window, ::Window& rProxy );
        Cursor createCursor( const unsigned char* pPointerData, const unsigned char* pMaskData, int width, int height, int hotX, int hotY );
        // coordinates on root ::Window
        void updateDragWindow( int nX, int nY, ::Window aRoot );

        bool getPasteData( Atom selection, Atom type, css::uno::Sequence< sal_Int8 >& rData );
        // returns true if conversion was successful
        bool convertData( const css::uno::Reference< css::datatransfer::XTransferable >& xTransferable,
                          Atom nType,
                          Atom nSelection,
                          int & rFormat,
                          css::uno::Sequence< sal_Int8 >& rData );
        bool sendData( SelectionAdaptor* pAdaptor, ::Window requestor, Atom target, Atom property, Atom selection );

        // thread dispatch loop
        public:
        // public for extern "C" stub
        static void run( void* );
        private:
        void dispatchEvent( int millisec );
        // drag thread dispatch
        public:
        // public for extern "C" stub
        static void runDragExecute( void* );
        private:
        void dragDoDispatch();
        bool handleXEvent( XEvent& rEvent );

        // compound text conversion
        OString convertToCompound( const OUString& rText );
        OUString convertFromCompound( const char* pText, int nLen );

        sal_Int8 getUserDragAction() const;
        sal_Int32 getSelectionTimeout();
    public:
        static SelectionManager& get( const OUString& rDisplayName = OUString() );

        Display * getDisplay() { return m_pDisplay; };

        void registerHandler( Atom selection, SelectionAdaptor& rAdaptor );
        void deregisterHandler( Atom selection );
        bool requestOwnership( Atom selection );

        // allow for synchronization over one mutex for XClipboard
        osl::Mutex& getMutex() { return m_aMutex; }

        Atom getAtom( const OUString& rString );
        const OUString& getString( Atom nAtom );

        // type conversion
        // note: convertTypeToNative does NOT clear the list, so you can append
        // multiple types to the same list
        void convertTypeToNative( const OUString& rType, Atom selection, int& rFormat, ::std::list< Atom >& rConversions, bool bPushFront = false );
        OUString convertTypeFromNative( Atom nType, Atom selection, int& rFormat );
        void getNativeTypeList( const css::uno::Sequence< css::datatransfer::DataFlavor >& rTypes, std::list< Atom >& rOutTypeList, Atom targetselection );

        // methods for transferable
        bool getPasteDataTypes( Atom selection, css::uno::Sequence< css::datatransfer::DataFlavor >& rTypes );
        bool getPasteData( Atom selection, const OUString& rType, css::uno::Sequence< sal_Int8 >& rData );

        // for XDropTarget to register/deregister itself
        void registerDropTarget( ::Window aXLIB_Window, DropTarget* pTarget );
        void deregisterDropTarget( ::Window aXLIB_Window );

        // for XDropTarget{Drag|Drop}Context
        void accept( sal_Int8 dragOperation, ::Window aDropXLIB_Window, Time aTimestamp );
        void reject( ::Window aDropXLIB_Window, Time aTimestamp );
        void dropComplete( bool success, ::Window aDropXLIB_Window, Time aTimestamp );

        // for XDragSourceContext
        sal_Int32 getCurrentCursor() { return m_aCurrentCursor;}
        void setCursor( sal_Int32 cursor, ::Window aDropXLIB_Window, Time aTimestamp );
        void transferablesFlavorsChanged();

        void shutdown() throw();

        // XInitialization
        virtual void        SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& arguments ) throw( css::uno::Exception, std::exception ) override;

        // XEventHandler
        virtual sal_Bool    SAL_CALL handleEvent(const css::uno::Any& event)
            throw (css::uno::RuntimeException, std::exception) override;

        // XDragSource
        virtual sal_Bool    SAL_CALL isDragImageSupported() throw(std::exception) override;
        virtual sal_Int32   SAL_CALL getDefaultCursor( sal_Int8 dragAction ) throw(std::exception) override;
        virtual void        SAL_CALL startDrag(
            const css::datatransfer::dnd::DragGestureEvent& trigger,
            sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image,
            const css::uno::Reference< css::datatransfer::XTransferable >& transferable,
            const css::uno::Reference< css::datatransfer::dnd::XDragSourceListener >& listener
            ) throw(std::exception) override;

        // SelectionAdaptor for XdndSelection Drag (we are drag source)
        virtual css::uno::Reference< css::datatransfer::XTransferable > getTransferable() throw() override;
        virtual void clearTransferable() throw() override;
        virtual void fireContentsChanged() throw() override;
        virtual css::uno::Reference< css::uno::XInterface > getReference() throw() override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw( css::uno::RuntimeException, std::exception ) override;

        // XTerminateListener
        virtual void SAL_CALL queryTermination( const css::lang::EventObject& aEvent )
                throw( css::frame::TerminationVetoException, css::uno::RuntimeException, std::exception ) override;
        virtual void SAL_CALL notifyTermination( const css::lang::EventObject& aEvent )
                throw( css::uno::RuntimeException, std::exception ) override;
    };

    css::uno::Sequence< OUString > SAL_CALL Xdnd_getSupportedServiceNames();
    css::uno::Reference< css::uno::XInterface > SAL_CALL Xdnd_createInstance(
        const css::uno::Reference< css::lang::XMultiServiceFactory > & xMultiServiceFactory);

    css::uno::Sequence< OUString > SAL_CALL Xdnd_dropTarget_getSupportedServiceNames();
    css::uno::Reference< css::uno::XInterface > SAL_CALL Xdnd_dropTarget_createInstance(
        const css::uno::Reference< css::lang::XMultiServiceFactory > & xMultiServiceFactory);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
