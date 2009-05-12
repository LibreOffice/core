/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: service_entry.cxx,v $
 * $Revision: 1.3 $
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
#include "precompiled_dtrans.hxx"

#include "DragSource.hxx"
#include "DropTarget.hxx"
#include "aqua_clipboard.hxx"
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/compbase1.hxx>
#include <osl/diagnose.h>

#include "cppuhelper/implementationentry.hxx"

using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::registry;
using namespace ::cppu;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer::clipboard;


rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;


extern rtl::OUString dragSource_getImplementationName();
extern com::sun::star::uno::Sequence<rtl::OUString> dragSource_getSupportedServiceNames();
extern rtl::OUString dropTarget_getImplementationName();
extern com::sun::star::uno::Sequence<rtl::OUString> dropTarget_getSupportedServiceNames();
extern rtl::OUString clipboard_getImplementationName();
extern com::sun::star::uno::Sequence<rtl::OUString> clipboard_getSupportedServiceNames();


Reference<XInterface> SAL_CALL createDragSource(const Reference<XComponentContext>& rComponentContext);
Reference<XInterface> SAL_CALL createDropTarget(const Reference<XComponentContext>& rComponentContext);
Reference<XInterface> SAL_CALL createClipboard(const Reference<XComponentContext>& rComponentContext);


cppu::ImplementationEntry entries[] = {
    { createDragSource,
      dragSource_getImplementationName,
      dragSource_getSupportedServiceNames,
      cppu::createSingleComponentFactory, 0, 0 },
    { createDropTarget,
      dropTarget_getImplementationName,
      dropTarget_getSupportedServiceNames,
      cppu::createSingleComponentFactory, 0, 0 },
    { createClipboard,
      clipboard_getImplementationName,
      clipboard_getSupportedServiceNames,
      cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};


class AquaClipboardFactory : public WeakComponentImplHelper1<XSingleComponentFactory>
{
public:
    AquaClipboardFactory();
    virtual ~AquaClipboardFactory();

  /*
   *  XSingleComponentFactory
   */
  virtual Reference<XInterface> createInstanceWithContext(const Reference<XComponentContext>& Context)
    throw (Exception);

  virtual Reference<XInterface> createInstanceWithArgumentsAndContext(const Sequence<Any>& rArgs,
                                                                      const Reference<XComponentContext>& Context)
    throw(Exception);

 private:
    static Mutex m_aMutex;
    static Reference<XInterface> mSingleInstance;
};

Reference<XInterface> AquaClipboardFactory::mSingleInstance;
Mutex AquaClipboardFactory::m_aMutex;

// ------------------------------------------------------------------------

AquaClipboardFactory::AquaClipboardFactory() :
  WeakComponentImplHelper1<XSingleComponentFactory>(m_aMutex)
{
}

// ------------------------------------------------------------------------

AquaClipboardFactory::~AquaClipboardFactory()
{
}

// ------------------------------------------------------------------------

Reference<XInterface> AquaClipboardFactory::createInstanceWithContext(const Reference<XComponentContext>& Context) throw(Exception)
{
  return createInstanceWithArgumentsAndContext(Sequence<Any>(), Context);
}

// ------------------------------------------------------------------------

Reference< XInterface > AquaClipboardFactory::createInstanceWithArgumentsAndContext(const Sequence<Any>& arguments,
                                                                                    const Reference<XComponentContext>& Context) throw(Exception)
{
  MutexGuard aGuard(m_aMutex);

  if (!mSingleInstance.is())
    {
      AquaClipboardFactory::mSingleInstance = createClipboard(Context);
    }

  return AquaClipboardFactory::mSingleInstance;
}


Reference<XInterface> SAL_CALL createDragSource( const Reference< XComponentContext >& rComponentContext)
{
    return Reference<XInterface>(static_cast< XInitialization* >(new DragSource(rComponentContext)), UNO_QUERY);
}

Reference<XInterface> SAL_CALL createDropTarget( const Reference< XComponentContext >& rComponentContext)
{
  return Reference<XInterface>(static_cast< XInitialization* >(new DropTarget(rComponentContext)), UNO_QUERY);
}

Reference< XInterface > SAL_CALL createClipboard( const Reference< XComponentContext >& rComponentContext)
{
  return Reference<XInterface>(static_cast< XClipboard* >(new AquaClipboard(rComponentContext)), UNO_QUERY);
}


extern "C" sal_Bool SAL_CALL component_writeInfo(
    void * serviceManager, void * registryKey)
{
    return cppu::component_writeInfoHelper(
        serviceManager, registryKey, entries);
}


extern "C" void * SAL_CALL component_getFactory(
                                                char const * implName, void * serviceManager, void * registryKey)
{
  void* pRet = NULL;
  OUString iName = OUString::createFromAscii(implName);
  Reference<XSingleComponentFactory> xFac;

  if (iName.equalsIgnoreAsciiCase(clipboard_getImplementationName()))
    {
      xFac = new AquaClipboardFactory();
    }
  else
    {
      return component_getFactoryHelper(implName,
                                        serviceManager,
                                        registryKey,
                                        entries);
    }

  if (xFac.is())
    {
      xFac->acquire();
      pRet = xFac.get();
    }

  return pRet;
}


extern "C" void SAL_CALL component_getImplementationEnvironment(
    char const ** envTypeName, uno_Environment **)
{
    *envTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}


extern "C" sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

