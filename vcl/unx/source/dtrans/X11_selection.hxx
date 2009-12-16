/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: X11_selection.hxx,v $
 * $Revision: 1.37 $
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

#ifndef _DTRANS_X11_SELECTION_HXX_
#define _DTRANS_X11_SELECTION_HXX_

#include <cppuhelper/compbase3.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/awt/XDisplayConnection.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/XInvocation.hpp>
#include <osl/thread.h>

#ifndef _OSL_CONDITION_HXX_
#include <osl/conditn.hxx>
#endif

#include <hash_map>
#include <list>

#include <X11/Xlib.h>

#define XDND_IMPLEMENTATION_NAME "com.sun.star.datatransfer.dnd.XdndSupport"
#define XDND_DROPTARGET_IMPLEMENTATION_NAME "com.sun.star.datatransfer.dnd.XdndDropTarget"

using namespace ::com::sun::star::uno;

namespace x11 {

    class PixmapHolder; // in bmp.hxx

// ------------------------------------------------------------------------
    rtl_TextEncoding getTextPlainEncoding( const ::rtl::OUString& rMimeType );

    class SelectionAdaptor
    {
    public:
        virtual Reference< ::com::sun::star::datatransfer::XTransferable > getTransferable() = 0;
        virtual void clearTransferable() = 0;
        virtual void fireContentsChanged() = 0;
        virtual Reference< XInterface > getReference() = 0;
        // returns a reference that will keep the SelectionAdaptor alive until the
        // refernce is released
    };

    class DropTarget :
        public ::cppu::WeakComponentImplHelper3<
            ::com::sun::star::datatransfer::dnd::XDropTarget,
            ::com::sun::star::lang::XInitialization,
            ::com::sun::star::lang::XServiceInfo
        >
    {
    public:
        ::osl::Mutex                m_aMutex;
        bool                        m_bActive;
        sal_Int8                    m_nDefaultActions;
        Window                      m_aTargetWindow;
        class SelectionManager*     m_pSelectionManager;
        Reference< ::com::sun::star::datatransfer::dnd::XDragSource >
                                    m_xSelectionManager;
        ::std::list< Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener > >
                            m_aListeners;

        DropTarget();
        virtual ~DropTarget();

        // convenience functions that loop over listeners
        void dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& dtde ) throw();
        void dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw();
        void dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw();
        void drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& dtde ) throw();

        // XInitialization
        virtual void        SAL_CALL initialize( const Sequence< Any >& args ) throw ( ::com::sun::star::uno::Exception );

        // XDropTarget
        virtual void        SAL_CALL addDropTargetListener( const Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& ) throw();
        virtual void        SAL_CALL removeDropTargetListener( const Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& ) throw();
        virtual sal_Bool    SAL_CALL isActive() throw();
        virtual void        SAL_CALL setActive( sal_Bool active ) throw();
        virtual sal_Int8    SAL_CALL getDefaultActions() throw();
        virtual void        SAL_CALL setDefaultActions( sal_Int8 actions ) throw();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw();
        virtual sal_Bool    SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw();
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
                            SAL_CALL getSupportedServiceNames() throw();
    };

    class SelectionManagerHolder :
        public ::cppu::WeakComponentImplHelper3<
            ::com::sun::star::datatransfer::dnd::XDragSource,
            ::com::sun::star::lang::XInitialization,
            ::com::sun::star::lang::XServiceInfo
        >
    {
        ::osl::Mutex m_aMutex;
        Reference< ::com::sun::star::datatransfer::dnd::XDragSource >
            m_xRealDragSource;
    public:
        SelectionManagerHolder();
        virtual ~SelectionManagerHolder();

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw();
        virtual sal_Bool    SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw();
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >
                            SAL_CALL getSupportedServiceNames() throw();

        // XInitialization
        virtual void        SAL_CALL initialize( const Sequence< Any >& arguments ) throw( ::com::sun::star::uno::Exception );

        // XDragSource
        virtual sal_Bool    SAL_CALL isDragImageSupported() throw();
        virtual sal_Int32   SAL_CALL getDefaultCursor( sal_Int8 dragAction ) throw();
        virtual void        SAL_CALL startDrag(
            const ::com::sun::star::datatransfer::dnd::DragGestureEvent& trigger,
            sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image,
            const Reference< ::com::sun::star::datatransfer::XTransferable >& transferable,
            const Reference< ::com::sun::star::datatransfer::dnd::XDragSourceListener >& listener
            ) throw();

    };


    class SelectionManager :
        public ::cppu::WeakImplHelper3<
            ::com::sun::star::datatransfer::dnd::XDragSource,
            ::com::sun::star::lang::XInitialization,
            ::com::sun::star::awt::XEventHandler
        >,
        public SelectionAdaptor
    {
        static ::std::hash_map< ::rtl::OUString, SelectionManager*, ::rtl::OUStringHash >& getInstances();

        // for INCR type selection transfer
        // INCR protocol is used if the data cannot
        // be transported at once but in parts
        // IncrementalTransfer holds the bytes to be transmitted
        // as well a the current position
        // INCR triggers the delivery of the next part by deleting the
        // property used to transfer the data
        struct IncrementalTransfer
        {
            Sequence< sal_Int8 >            m_aData;
            int                             m_nBufferPos;
            Window                          m_aRequestor;
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
            Sequence< sal_Int8 >        m_aData;
            Sequence< ::com::sun::star::datatransfer::DataFlavor >
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
            Window                      m_aLastOwner;
            PixmapHolder*               m_pPixmap;
            // m_nOrigTimestamp contains the timestamp at which the seclection
            // was acquired; needed for TIMESTAMP target
            Time                        m_nOrigTimestamp;

            Selection() : m_eState( Inactive ),
                          m_pAdaptor( NULL ),
                          m_aAtom( None ),
                          m_aRequestedType( None ),
                          m_nLastTimestamp( 0 ),
                          m_bHaveUTF16( false ),
                          m_aUTF8Type( None ),
                          m_bHaveCompound( false ),
                          m_bOwner( false ),
                          m_aLastOwner( None ),
                          m_pPixmap( NULL ),
                          m_nOrigTimestamp( CurrentTime )
                {}
        };

        // a struct to hold data associated with a XDropTarget
        struct DropTargetEntry
        {
            DropTarget*     m_pTarget;
            Window          m_aRootWindow;

            DropTargetEntry() : m_pTarget( NULL ), m_aRootWindow( None ) {}
            DropTargetEntry( DropTarget* pTarget ) :
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
        oslThread                   m_aDragExecuteThread;
        ::osl::Condition            m_aDragRunning;
        Window                      m_aWindow;
        Reference< ::com::sun::star::awt::XDisplayConnection >
                                    m_xDisplayConnection;
        Reference< com::sun::star::script::XInvocation >
                                    m_xBitmapConverter;
        sal_Int32                   m_nSelectionTimeout;
        Time                        m_nSelectionTimestamp;


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
        Time                        m_nDropTimestamp;
        // set to true when calling drop()
        // if another XdndEnter is received this shows that
        // someone forgot to call dropComplete - we should reset
        // and react to the new drop
        bool                        m_bDropWaitingForCompletion;

        // drag only

        // None if no Dnd action is running with us as source
        Window                      m_aDropWindow;
        // either m_aDropWindow or its XdndProxy
        Window                      m_aDropProxy;
        Window                      m_aDragSourceWindow;
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
        ::std::hash_map< Window, DropTargetEntry >
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
        ::std::hash_map< Atom, ::rtl::OUString >
                                    m_aAtomToString;
        ::std::hash_map< ::rtl::OUString, Atom, ::rtl::OUStringHash >
                                    m_aStringToAtom;

        // the registered selections
        ::std::hash_map< Atom, Selection* >
                                    m_aSelections;
        // IncrementalTransfers in progress
        std::hash_map< Window, std::hash_map< Atom, IncrementalTransfer > >
                                    m_aIncrementals;

        // do not use X11 multithreading capabilities
        // since this leads to deadlocks in different Xlib implentations
        // (XFree as well as Xsun) use an own mutex instead
        ::osl::Mutex                m_aMutex;

        SelectionManager();
        ~SelectionManager();

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
        int getXdndVersion( Window aWindow, Window& rProxy );
        Cursor createCursor( const char* pPointerData, const char* pMaskData, int width, int height, int hotX, int hotY );
        // coordinates on root window
        void updateDragWindow( int nX, int nY, Window aRoot );

        bool getPasteData( Atom selection, Atom type, Sequence< sal_Int8 >& rData );
        // returns true if conversion was successful
        bool convertData( const Reference< ::com::sun::star::datatransfer::XTransferable >& xTransferable,
                          Atom nType,
                          Atom nSelection,
                          int & rFormat,
                          Sequence< sal_Int8 >& rData );
        bool sendData( SelectionAdaptor* pAdaptor, Window requestor, Atom target, Atom property, Atom selection );

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
        ::rtl::OString convertToCompound( const ::rtl::OUString& rText );
        ::rtl::OUString convertFromCompound( const char* pText, int nLen = -1 );

        sal_Int8 getUserDragAction() const;
        sal_Int32 getSelectionTimeout();
    public:
        static SelectionManager& get( const ::rtl::OUString& rDisplayName = ::rtl::OUString() );

        Display * getDisplay() { return m_pDisplay; };
        Window getWindow() { return m_aWindow; };


        void registerHandler( Atom selection, SelectionAdaptor& rAdaptor );
        void deregisterHandler( Atom selection );
        bool requestOwnership( Atom selection );

        // allow for synchronization over one mutex for XClipboard
        osl::Mutex& getMutex() { return m_aMutex; }


        Atom getAtom( const ::rtl::OUString& rString );
        const ::rtl::OUString& getString( Atom nAtom );

        // type conversion
        // note: convertTypeToNative does NOT clear the list, so you can append
        // multiple types to the same list
        void convertTypeToNative( const ::rtl::OUString& rType, Atom selection, int& rFormat, ::std::list< Atom >& rConversions, bool bPushFront = false );
        ::rtl::OUString convertTypeFromNative( Atom nType, Atom selection, int& rFormat );
        void getNativeTypeList( const Sequence< com::sun::star::datatransfer::DataFlavor >& rTypes, std::list< Atom >& rOutTypeList, Atom targetselection );

        // methods for transferable
        bool getPasteDataTypes( Atom selection, Sequence< ::com::sun::star::datatransfer::DataFlavor >& rTypes );
        bool getPasteData( Atom selection, const ::rtl::OUString& rType, Sequence< sal_Int8 >& rData );

        // for XDropTarget to register/deregister itself
        void registerDropTarget( Window aWindow, DropTarget* pTarget );
        void deregisterDropTarget( Window aWindow );

        // for XDropTarget{Drag|Drop}Context
        void accept( sal_Int8 dragOperation, Window aDropWindow, Time aTimestamp );
        void reject( Window aDropWindow, Time aTimestamp );
        void dropComplete( sal_Bool success, Window aDropWindow, Time aTimestamp );

        // for XDragSourceContext
        sal_Int32 getCurrentCursor();
        void setCursor( sal_Int32 cursor, Window aDropWindow, Time aTimestamp );
        void setImage( sal_Int32 image, Window aDropWindow, Time aTimestamp );
        void transferablesFlavorsChanged();

        // XInitialization
        virtual void        SAL_CALL initialize( const Sequence< Any >& arguments ) throw( ::com::sun::star::uno::Exception );

        // XEventHandler
        virtual sal_Bool    SAL_CALL handleEvent( const Any& event ) throw();

        // XDragSource
        virtual sal_Bool    SAL_CALL isDragImageSupported() throw();
        virtual sal_Int32   SAL_CALL getDefaultCursor( sal_Int8 dragAction ) throw();
        virtual void        SAL_CALL startDrag(
            const ::com::sun::star::datatransfer::dnd::DragGestureEvent& trigger,
            sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image,
            const Reference< ::com::sun::star::datatransfer::XTransferable >& transferable,
            const Reference< ::com::sun::star::datatransfer::dnd::XDragSourceListener >& listener
            ) throw();

        // SelectionAdaptor for XdndSelection Drag (we are drag source)
        virtual Reference< ::com::sun::star::datatransfer::XTransferable > getTransferable() throw();
        virtual void clearTransferable() throw();
        virtual void fireContentsChanged() throw();
        virtual Reference< XInterface > getReference() throw();
    };

// ------------------------------------------------------------------------

    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL Xdnd_getSupportedServiceNames();
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Xdnd_createInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMultiServiceFactory);

    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL Xdnd_dropTarget_getSupportedServiceNames();
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL Xdnd_dropTarget_createInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & xMultiServiceFactory);

// ------------------------------------------------------------------------

}

#endif
