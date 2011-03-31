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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "vos/mutex.hxx"

#include "osl/mutex.hxx"

#include "vcl/svapp.hxx"

#include "svdata.hxx"
#include "salinst.hxx"

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/datatransfer/XTransferable.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboard.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboardEx.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboardListener.hpp"
#include "com/sun/star/datatransfer/dnd/XDragSource.hpp"
#include "com/sun/star/datatransfer/dnd/XDropTarget.hpp"
#include "com/sun/star/datatransfer/dnd/DNDConstants.hpp"

#include "cppuhelper/compbase1.hxx"
#include "cppuhelper/compbase2.hxx"
#include "cppuhelper/compbase3.hxx"
#include "cppuhelper/implbase1.hxx"

using rtl::OUString;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

// -----------------------------------------------------------------------

namespace vcl
{
// generic implementation to satisfy SalInstance
class GenericClipboard :
        public cppu::WeakComponentImplHelper3 <
        datatransfer::clipboard::XClipboardEx,
        datatransfer::clipboard::XClipboardNotifier,
        XServiceInfo
        >
{
    osl::Mutex                                                              m_aMutex;
    Reference< ::com::sun::star::datatransfer::XTransferable >              m_aContents;
    Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner > m_aOwner;
    std::list< Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener > > m_aListeners;

    void fireChangedContentsEvent();
    void clearContents();

public:

    GenericClipboard() : cppu::WeakComponentImplHelper3<
        datatransfer::clipboard::XClipboardEx,
        datatransfer::clipboard::XClipboardNotifier,
        XServiceInfo
        >( m_aMutex )
    {}
    virtual ~GenericClipboard();

    /*
     * XServiceInfo
     */

    virtual rtl::OUString SAL_CALL getImplementationName() throw( RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw( RuntimeException );
    virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( RuntimeException );

    static rtl::OUString getImplementationName_static();
    static Sequence< rtl::OUString > getSupportedServiceNames_static();

    /*
     * XClipboard
     */

    virtual Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getContents()
        throw(RuntimeException);

    virtual void SAL_CALL setContents(
        const Reference< ::com::sun::star::datatransfer::XTransferable >& xTrans,
        const Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
        throw(RuntimeException);

    virtual ::rtl::OUString SAL_CALL getName()
        throw(RuntimeException);

    /*
     * XClipboardEx
     */

    virtual sal_Int8 SAL_CALL getRenderingCapabilities()
        throw(RuntimeException);

    /*
     * XClipboardNotifier
     */
    virtual void SAL_CALL addClipboardListener(
        const Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
        throw(RuntimeException);

    virtual void SAL_CALL removeClipboardListener(
        const Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
        throw(RuntimeException);
};

GenericClipboard::~GenericClipboard()
{
}

rtl::OUString GenericClipboard::getImplementationName_static()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.datatransfer.VCLGenericClipboard" ) );
}

Sequence< rtl::OUString > GenericClipboard::getSupportedServiceNames_static()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii("com.sun.star.datatransfer.clipboard.SystemClipboard");
    return aRet;
}

rtl::OUString GenericClipboard::getImplementationName() throw( RuntimeException )
{
    return getImplementationName_static();
}

Sequence< rtl::OUString > GenericClipboard::getSupportedServiceNames() throw( RuntimeException )
{
    return getSupportedServiceNames_static();
}

sal_Bool GenericClipboard::supportsService( const ::rtl::OUString& ServiceName ) throw( RuntimeException )
{
    Sequence< OUString > aServices( getSupportedServiceNames() );
    sal_Int32 nServices = aServices.getLength();
    for( sal_Int32 i = 0; i < nServices; i++ )
    {
        if( aServices[i] == ServiceName )
            return sal_True;
    }
    return sal_False;
}

Reference< ::com::sun::star::datatransfer::XTransferable > GenericClipboard::getContents() throw( RuntimeException )
{
    return m_aContents;
}

void GenericClipboard::setContents(
        const Reference< ::com::sun::star::datatransfer::XTransferable >& xTrans,
        const Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
    throw( RuntimeException )
{
    osl::ClearableMutexGuard aGuard( m_aMutex );
    Reference< datatransfer::clipboard::XClipboardOwner > xOldOwner( m_aOwner );
    Reference< datatransfer::XTransferable > xOldContents( m_aContents );
    m_aContents = xTrans;
    m_aOwner = xClipboardOwner;

    std::list< Reference< datatransfer::clipboard::XClipboardListener > > xListeners( m_aListeners );
    datatransfer::clipboard::ClipboardEvent aEv;
    aEv.Contents = m_aContents;

    aGuard.clear();

    if( xOldOwner.is() && xOldOwner != xClipboardOwner )
        xOldOwner->lostOwnership( this, xOldContents );
    for( std::list< Reference< datatransfer::clipboard::XClipboardListener > >::iterator it =
         xListeners.begin(); it != xListeners.end() ; ++it )
    {
        (*it)->changedContents( aEv );
    }
}

rtl::OUString GenericClipboard::getName() throw( RuntimeException )
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CLIPBOARD" ) );
}

sal_Int8 GenericClipboard::getRenderingCapabilities() throw( RuntimeException )
{
    return 0;
}

void GenericClipboard::addClipboardListener( const Reference< datatransfer::clipboard::XClipboardListener >& listener )
    throw( RuntimeException )
{
    osl::ClearableMutexGuard aGuard( m_aMutex );

    m_aListeners.push_back( listener );
}

void GenericClipboard::removeClipboardListener( const Reference< datatransfer::clipboard::XClipboardListener >& listener )
    throw( RuntimeException )
{
    osl::ClearableMutexGuard aGuard( m_aMutex );

    m_aListeners.remove( listener );
}

// ------------------------------------------------------------------------

class ClipboardFactory : public ::cppu::WeakComponentImplHelper1<
    com::sun::star::lang::XSingleServiceFactory
>
{
    osl::Mutex m_aMutex;
public:
    ClipboardFactory();
    virtual ~ClipboardFactory();

    /*
     *  XSingleServiceFactory
     */
    virtual Reference< XInterface > SAL_CALL createInstance() throw();
    virtual Reference< XInterface > SAL_CALL createInstanceWithArguments( const Sequence< Any >& rArgs ) throw();
};

// ------------------------------------------------------------------------

ClipboardFactory::ClipboardFactory() :
        cppu::WeakComponentImplHelper1<
    com::sun::star::lang::XSingleServiceFactory
>( m_aMutex )
{
}

// ------------------------------------------------------------------------

ClipboardFactory::~ClipboardFactory()
{
}

// ------------------------------------------------------------------------

Reference< XInterface > ClipboardFactory::createInstance() throw()
{
    return createInstanceWithArguments( Sequence< Any >() );
}

// ------------------------------------------------------------------------

Reference< XInterface > ClipboardFactory::createInstanceWithArguments( const Sequence< Any >& arguments ) throw()
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    Reference< XInterface > xResult = ImplGetSVData()->mpDefInst->CreateClipboard( arguments );
    return xResult;
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL Clipboard_getSupportedServiceNames()
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii("com.sun.star.datatransfer.clipboard.SystemClipboard");
    return aRet;
}

OUString SAL_CALL Clipboard_getImplementationName()
{
    #if defined UNX
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
    #if ! defined QUARTZ
    "com.sun.star.datatransfer.X11ClipboardSupport"
    #else
    "com.sun.star.datatransfer.clipboard.AquaClipboard"
    #endif
    ) );
    #else
    return GenericClipboard::getImplementationName_static();
    #endif
}

Reference< XSingleServiceFactory > SAL_CALL Clipboard_createFactory( const Reference< XMultiServiceFactory > &  )
{
    return Reference< XSingleServiceFactory >( new ClipboardFactory() );
}

/*
*   generic DragSource dummy
*/
class GenericDragSource : public cppu::WeakComponentImplHelper2<
            datatransfer::dnd::XDragSource,
            XInitialization
            >
{
    osl::Mutex                          m_aMutex;
public:
    GenericDragSource() : cppu::WeakComponentImplHelper2< datatransfer::dnd::XDragSource, XInitialization >( m_aMutex ) {}
    virtual ~GenericDragSource();

    // XDragSource
    virtual sal_Bool    SAL_CALL isDragImageSupported() throw();
    virtual sal_Int32   SAL_CALL getDefaultCursor( sal_Int8 dragAction ) throw();
    virtual void        SAL_CALL startDrag(
                                     const datatransfer::dnd::DragGestureEvent& trigger,
                                     sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image,
                                     const Reference< datatransfer::XTransferable >& transferable,
                                     const Reference< datatransfer::dnd::XDragSourceListener >& listener
                                     ) throw();

    // XInitialization
    virtual void        SAL_CALL initialize( const Sequence< Any >& arguments ) throw( ::com::sun::star::uno::Exception );

    static Sequence< OUString > getSupportedServiceNames_static()
    {
        Sequence< OUString > aRet( 1 );
        aRet[0] = OUString::createFromAscii( "com.sun.star.datatransfer.dnd.GenericDragSource" );
        return aRet;
    }

    static OUString getImplementationName_static()
    {
        return OUString::createFromAscii( "com.sun.star.datatransfer.dnd.VclGenericDragSource" );
    }
};

GenericDragSource::~GenericDragSource()
{
}

sal_Bool GenericDragSource::isDragImageSupported() throw()
{
    return sal_False;
}

sal_Int32 GenericDragSource::getDefaultCursor( sal_Int8 ) throw()
{
    return 0;
}

void GenericDragSource::startDrag( const datatransfer::dnd::DragGestureEvent&,
                                   sal_Int8 /*sourceActions*/, sal_Int32 /*cursor*/, sal_Int32 /*image*/,
                                   const Reference< datatransfer::XTransferable >&,
                                   const Reference< datatransfer::dnd::XDragSourceListener >& listener
                                   ) throw()
{
    datatransfer::dnd::DragSourceDropEvent aEv;
    aEv.DropAction = datatransfer::dnd::DNDConstants::ACTION_COPY;
    aEv.DropSuccess = sal_False;
    listener->dragDropEnd( aEv );
}

void GenericDragSource::initialize( const Sequence< Any >& ) throw( Exception )
{
}


Sequence< OUString > SAL_CALL DragSource_getSupportedServiceNames()
{
    #if defined UNX
    static OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM(
    #if ! defined QUARTZ
    "com.sun.star.datatransfer.dnd.X11DragSource"
    #else
    "com.sun.star.datatransfer.dnd.OleDragSource"
    #endif
                                                              ) );
    static Sequence< OUString > aServiceNames( &aServiceName, 1 );
    return aServiceNames;
    #else
    return GenericDragSource::getSupportedServiceNames_static();
    #endif
}

OUString SAL_CALL DragSource_getImplementationName()
{
    #if defined UNX
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
    #if ! defined QUARTZ
    "com.sun.star.datatransfer.dnd.XdndSupport"
    #else
    "com.sun.star.comp.datatransfer.dnd.OleDragSource_V1"
    #endif
                                                 ) );
    #else
    return GenericDragSource::getImplementationName_static();
    #endif
}

Reference< XInterface > SAL_CALL DragSource_createInstance( const Reference< XMultiServiceFactory >&  )
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    Reference< XInterface > xResult = ImplGetSVData()->mpDefInst->CreateDragSource();
    return xResult;
}

/*
*   generic DragSource dummy
*/

class GenericDropTarget : public cppu::WeakComponentImplHelper2<
                                           datatransfer::dnd::XDropTarget,
                                           XInitialization
                                           >
{
    osl::Mutex m_aMutex;
public:
    GenericDropTarget() : cppu::WeakComponentImplHelper2<
                                           datatransfer::dnd::XDropTarget,
                                           XInitialization
                                           > ( m_aMutex )
    {}
    virtual ~GenericDropTarget();

    // XInitialization
    virtual void        SAL_CALL initialize( const Sequence< Any >& args ) throw ( Exception );

    // XDropTarget
    virtual void        SAL_CALL addDropTargetListener( const Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& ) throw();
    virtual void        SAL_CALL removeDropTargetListener( const Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& ) throw();
    virtual sal_Bool    SAL_CALL isActive() throw();
    virtual void        SAL_CALL setActive( sal_Bool active ) throw();
    virtual sal_Int8    SAL_CALL getDefaultActions() throw();
    virtual void        SAL_CALL setDefaultActions( sal_Int8 actions ) throw();

    static Sequence< OUString > getSupportedServiceNames_static()
    {
        Sequence< OUString > aRet( 1 );
        aRet[0] = OUString::createFromAscii( "com.sun.star.datatransfer.dnd.GenericDropTarget" );
        return aRet;
    }

    static OUString getImplementationName_static()
    {
        return OUString::createFromAscii( "com.sun.star.datatransfer.dnd.VclGenericDropTarget" );
    }
};

GenericDropTarget::~GenericDropTarget()
{
}

void GenericDropTarget::initialize( const Sequence< Any >& ) throw( Exception )
{
}

void GenericDropTarget::addDropTargetListener( const Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& ) throw()
{
}

void GenericDropTarget::removeDropTargetListener( const Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >& ) throw()
{
}

sal_Bool GenericDropTarget::isActive() throw()
{
    return sal_False;
}

void GenericDropTarget::setActive( sal_Bool ) throw()
{
}

sal_Int8 GenericDropTarget::getDefaultActions() throw()
{
    return 0;
}

void GenericDropTarget::setDefaultActions( sal_Int8) throw()
{
}

Sequence< OUString > SAL_CALL DropTarget_getSupportedServiceNames()
{
    #if defined UNX
    static OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM(
    #if ! defined QUARTZ
    "com.sun.star.datatransfer.dnd.X11DropTarget"
    #else
    "com.sun.star.datatransfer.dnd.OleDropTarget"
    #endif
                                                              ) );
    static Sequence< OUString > aServiceNames( &aServiceName, 1 );
    return aServiceNames;
    #else
    return GenericDropTarget::getSupportedServiceNames_static();
    #endif
}

OUString SAL_CALL DropTarget_getImplementationName()
{
    #if defined UNX
    return OUString( RTL_CONSTASCII_USTRINGPARAM(
    #if ! defined QUARTZ
    "com.sun.star.datatransfer.dnd.XdndDropTarget"
    #else
    "com.sun.star.comp.datatransfer.dnd.OleDropTarget_V1"
    #endif
                    ) );
    #else
    return GenericDropTarget::getImplementationName_static();
    #endif
}

Reference< XInterface > SAL_CALL DropTarget_createInstance( const Reference< XMultiServiceFactory >&  )
{
    vos::OGuard aGuard( Application::GetSolarMutex() );
    Reference< XInterface > xResult = ImplGetSVData()->mpDefInst->CreateDropTarget();
    return xResult;
}


} // namespace vcl

/*
*   SalInstance generic
*/
Reference< XInterface > SalInstance::CreateClipboard( const Sequence< Any >& )
{
    return Reference< XInterface >( ( cppu::OWeakObject * )new vcl::GenericClipboard() );
}

Reference< XInterface > SalInstance::CreateDragSource()
{
    return Reference< XInterface >( ( cppu::OWeakObject * )new vcl::GenericDragSource() );
}

Reference< XInterface > SalInstance::CreateDropTarget()
{
    return Reference< XInterface >( ( cppu::OWeakObject * )new vcl::GenericDropTarget() );
}

