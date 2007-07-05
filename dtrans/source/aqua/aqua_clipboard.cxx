/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: aqua_clipboard.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-05 09:11:49 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dtrans.hxx"

#ifndef _AQUA_CLIPBOARD_HXX_
#include "aqua_clipboard.hxx"
#endif

#include "DataFlavorMapping.hxx"
#include "OSXTransferable.hxx"

#include <boost/assert.hpp>

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace rtl;
using namespace aqua;
using namespace std;

const EventTypeSpec appActivatedEvent = { kEventClassApplication, kEventAppActivated };


AquaClipboard::AquaClipboard(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > ServiceManager) :
  WeakComponentImplHelper4< XClipboardEx, XClipboardNotifier, XFlushableClipboard, XServiceInfo >(m_aMutex),
  mrServiceMgr(ServiceManager)
{
  if (PasteboardCreate(kPasteboardClipboard, &mrClipboard) != noErr)
    {
      throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot create Mac OS X system cliboard")),
                             static_cast<XClipboardEx*>(this));
    }

  /* We need to register ourself for application activated events.
     This is the suggested way is to check for clipboard modifications
     during this event.
   */
  if (InstallApplicationEventHandler(NewEventHandlerUPP(handleAppActivatedEvent),
                                     1,
                                     &appActivatedEvent,
                                     this,
                                     &mrAppActivatedHdl) != noErr)
    {
      CFRelease(mrClipboard);
      throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot register for application activated events")),
                             static_cast<XClipboardEx*>(this));
    }

  mrXMimeCntFactory = Reference<XMimeContentTypeFactory>(mrServiceMgr->createInstance(
      OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.MimeContentTypeFactory"))), UNO_QUERY);

  if (!mrXMimeCntFactory.is())
    throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot create com.sun.star.datatransfer.MimeContentTypeFactory")),
                           static_cast<XClipboardEx*>(this));

  mpDataFlavorMapper = DataFlavorMapperPtr_t(new DataFlavorMapper(mrServiceMgr));
}

AquaClipboard::~AquaClipboard()
{
  RemoveEventHandler(mrAppActivatedHdl);
  CFRelease(mrClipboard);
}

Reference< XTransferable > SAL_CALL AquaClipboard::getContents() throw( RuntimeException )
{
  MutexGuard aGuard(m_aMutex);

  // Shortcut: If we are clipboard owner already we don't need
  // to drag the data through the system clipboard
  if (mXClipboardContent.is())
    {
      return mXClipboardContent;
    }
  else
    {
      return Reference<XTransferable>(new OSXTransferable(mrServiceMgr, mrXMimeCntFactory, mpDataFlavorMapper));
    }
}

void SAL_CALL AquaClipboard::setContents(const Reference<XTransferable>& xTransferable,
                                         const Reference<XClipboardOwner>& xClipboardOwner)
  throw( RuntimeException )
{
    MutexGuard aGuard( m_aMutex );

    // if we are already the owner of the clipboard
    // then fire lost ownership event
    if (mXClipboardContent.is())
      {
        fireLostClipboardOwnershipEvent();
      }

    if (PasteboardClear(mrClipboard) != noErr)
      {
        throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot set clipboard content")),
                               static_cast<XClipboardEx*>(this));
      }

    mXClipboardContent = xTransferable;
    mXClipboardOwner = xClipboardOwner;

    // An empty transferable means clear the clipboard, so we can
    // return here
    if (!mXClipboardContent.is())
      {
        return;
      }

    Sequence<DataFlavor> flavorList = mXClipboardContent->getTransferDataFlavors();
    sal_Int32 nClipboardFlavors = flavorList.getLength();

    if (nClipboardFlavors > 0 &&
        PasteboardSetPromiseKeeper(mrClipboard, AquaClipboard::clipboardPromiseKeeperCallback, this) != noErr)
      {
        throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot set clipboard prmise keeper callback")),
                               static_cast<XClipboardEx*>(this));
      }

    // We always only have one clipboard item to add but with multiple flavors
    for (sal_Int32 i = 0; i < nClipboardFlavors; i++)
      {
        CFStringRef systemFlavor = mpDataFlavorMapper->openOfficeToSystemFlavor(flavorList[i]);

        if (systemFlavor != NULL)
          {
            OSStatus ret = PasteboardPutItemFlavor(mrClipboard,
                                                   (PasteboardItemID)1,
                                                   systemFlavor,
                                                   kPasteboardPromisedData,
                                                   kPasteboardFlavorNoFlags);
            if (ret != noErr)
              {
                throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot set clipboard contents")),
                                       static_cast<XClipboardEx*>(this));
              }
          }
      }
}

OUString SAL_CALL AquaClipboard::getName() throw( RuntimeException )
{
  return OUString();
}

sal_Int8 SAL_CALL AquaClipboard::getRenderingCapabilities() throw( RuntimeException )
{
    return 0;
}

void SAL_CALL AquaClipboard::addClipboardListener( const Reference< XClipboardListener >& listener ) throw( RuntimeException )
{
  MutexGuard aGuard(m_aMutex);

  if (!listener.is())
     throw IllegalArgumentException(OUString(RTL_CONSTASCII_USTRINGPARAM("empty reference")),
                                   static_cast<XClipboardEx*>(this), 1);

  mClipboardListeners.push_back(listener);
}

void SAL_CALL AquaClipboard::removeClipboardListener( const Reference< XClipboardListener >& listener ) throw( RuntimeException )
{
  MutexGuard aGuard(m_aMutex);

  if (!listener.is())
     throw IllegalArgumentException(OUString(RTL_CONSTASCII_USTRINGPARAM("empty reference")),
                                   static_cast<XClipboardEx*>(this), 1);

  mClipboardListeners.remove(listener);
}

OSStatus AquaClipboard::handleAppActivatedEvent(EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData)
{
  AquaClipboard* pAquaClipboard = reinterpret_cast<AquaClipboard*>(inUserData);

  BOOST_ASSERT(pAquaClipboard && "Invalid argument");

  PasteboardSyncFlags syncFlags = PasteboardSynchronize(pAquaClipboard->mrClipboard);

  if (syncFlags & kPasteboardModified)
    {
      pAquaClipboard->fireClipboardChangedEvent();
    }

  if (pAquaClipboard->mXClipboardContent.is() && !(syncFlags & kPasteboardClientIsOwner))
    {
      pAquaClipboard->fireLostClipboardOwnershipEvent();
    }

  // we are just silent listener to this event
  // let others really handle it
  return eventNotHandledErr;
}

void AquaClipboard::fireClipboardChangedEvent()
{
    ClearableMutexGuard aGuard(m_aMutex);
    // copy the clipboard listener list
    list<Reference< XClipboardListener > > listeners(mClipboardListeners);
    aGuard.clear();

    Reference<XTransferable> content = getContents();

    ClipboardEvent aEvent(static_cast<OWeakObject*>(this), content);

    while(listeners.begin() != listeners.end())
    {
        if(listeners.front().is())
            listeners.front()->changedContents(aEvent);
        listeners.pop_front();
    }
}

void AquaClipboard::fireLostClipboardOwnershipEvent()
{
  try
    {
      // Fire event only if a lost owner ship handler has been registered
      if (mXClipboardOwner.is())
        {
          mXClipboardOwner->lostOwnership(static_cast<XClipboardEx*>(this), mXClipboardContent);
        }
    }
  catch(RuntimeException&)
    {
      OSL_ENSURE( sal_False, "AquaClipboard: fireLostClipboardOwnershipEvent, RuntimeException caught" );
    }

  // Reset the references to clipboard owner and content
  mXClipboardOwner = Reference<XClipboardOwner>();
  mXClipboardContent = Reference<XTransferable>();
}

OSStatus AquaClipboard::clipboardPromiseKeeperCallback(PasteboardRef inPasteboard,
                                                       PasteboardItemID itemID,
                                                       CFStringRef inFlavor,
                                                       void* inContext)
{
  AquaClipboard* pAquaClipboard = reinterpret_cast<AquaClipboard*>(inContext);

  DataProviderPtr_t pdp = pAquaClipboard->mpDataFlavorMapper->getDataProvider(inFlavor, pAquaClipboard->mXClipboardContent);
  OSStatus rc = badPasteboardFlavorErr;
  CFDataRef sysData = NULL;

  if ((pdp.get()) && ((sysData = pdp->getSystemData()) != NULL))
    {
      rc = PasteboardPutItemFlavor(pAquaClipboard->mrClipboard, (PasteboardItemID)1, inFlavor, sysData, 0);
      CFRelease(sysData);
    }

  return rc;
}


//------------------------------------------------
// XFlushableClipboard
//------------------------------------------------

void SAL_CALL AquaClipboard::flushClipboard()
  throw(RuntimeException)
{
  PasteboardResolvePromises(mrClipboard);
}


OUString SAL_CALL AquaClipboard::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii( AQUA_CLIPBOARD_IMPL_NAME );
}

sal_Bool SAL_CALL AquaClipboard::supportsService( const OUString& ServiceName ) throw( RuntimeException )
{
    return sal_False;
}

Sequence< OUString > SAL_CALL AquaClipboard::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii( AQUA_CLIPBOARD_SERVICE_NAME );
    return aRet;
}

