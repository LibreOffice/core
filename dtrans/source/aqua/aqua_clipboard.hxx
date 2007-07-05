/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: aqua_clipboard.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-05 09:12:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _AQUA_CLIPBOARD_HXX_
#define _AQUA_CLIPBOARD_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase4.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDEX_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDOWNER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDLISTENER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDNOTIFIER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XMIMECONTENTTYPEFACTORY_HPP_
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XFLUSHABLECLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#include "DataFlavorMapping.hxx"

#include <premac.h>
    #include <Carbon/Carbon.h>
    #include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

#include <list>

// the service names
#define AQUA_CLIPBOARD_SERVICE_NAME "com.sun.star.datatransfer.clipboard.SystemClipboard"

// the implementation names
#define AQUA_CLIPBOARD_IMPL_NAME "com.sun.star.datatransfer.clipboard.AquaClipboard"

// the registry key names
#define AQUA_CLIPBOARD_REGKEY_NAME "/com.sun.star.datatransfer.clipboard.AquaClipboard/UNO/SERVICES/com.sun.star.datatransfer.clipboard.SystemClipboard"

namespace aqua {

class AquaClipboard :
    public cppu::WeakComponentImplHelper4< ::com::sun::star::datatransfer::clipboard::XClipboardEx,
                                           ::com::sun::star::datatransfer::clipboard::XClipboardNotifier,
                                           ::com::sun::star::datatransfer::clipboard::XFlushableClipboard,
                                           ::com::sun::star::lang::XServiceInfo >
{
public:
    AquaClipboard(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > ServiceManager);

   ~AquaClipboard();

    //------------------------------------------------
    // XClipboard
    //------------------------------------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > SAL_CALL getContents()
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setContents( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTransferable,
                                       const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual ::rtl::OUString SAL_CALL getName()
        throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // XClipboardEx
    //------------------------------------------------

    virtual sal_Int8 SAL_CALL getRenderingCapabilities()
        throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // XClipboardNotifier
    //------------------------------------------------

    virtual void SAL_CALL addClipboardListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL removeClipboardListener( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener >& listener )
        throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // XFlushableClipboard
    //------------------------------------------------

    virtual void SAL_CALL flushClipboard( ) throw( com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // XServiceInfo
    //------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw(::com::sun::star::uno::RuntimeException);

private:
  /* Notify all registered XClipboardListener that the clipboard content
     has changed.
  */
  void fireClipboardChangedEvent();

  /* Notify the current clipboard owner that he is no longer the clipboard owner.
   */
  void fireLostClipboardOwnershipEvent();

  /* Event handler for application activated events. We need to determine the state of the clipboard
     in this event handler.
  */
  static OSStatus handleAppActivatedEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData);

  /* A clipboard promise keeper handler.
   */
  static OSStatus clipboardPromiseKeeperCallback(PasteboardRef inPasteboard,
                                                 PasteboardItemID itemID,
                                                 CFStringRef inFlavaor,
                                                 void* inContext);
private:
  const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mrServiceMgr;
  ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XMimeContentTypeFactory> mrXMimeCntFactory;
  ::std::list< ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboardListener > > mClipboardListeners;
  ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > mXClipboardContent;
  com::sun::star::uno::Reference<com::sun::star::datatransfer::clipboard::XClipboardOwner> mXClipboardOwner;
  DataFlavorMapperPtr_t mpDataFlavorMapper;
  ::osl::Mutex m_aMutex;
  PasteboardRef mrClipboard;
  EventHandlerRef mrAppActivatedHdl;
};

} // namespace aqua

#endif
