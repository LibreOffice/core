/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: documentacceleratorconfiguration.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_framework.hxx"
#include <accelerators/documentacceleratorconfiguration.hxx>

//_______________________________________________
// own includes
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

#ifndef __FRAMEWORK_ACCELERATORCONST_H_
#include <acceleratorconst.h>
#endif
#include <services.h>

//_______________________________________________
// interface includes

//_______________________________________________
// other includes

#ifndef _COMPHELPER_SEQUENCEASHASHMAP_HXX
#include <comphelper/sequenceashashmap.hxx>
#endif

//_______________________________________________
// const

namespace framework
{

//-----------------------------------------------
// XInterface, XTypeProvider, XServiceInfo
DEFINE_XINTERFACE_2(DocumentAcceleratorConfiguration                   ,
                    AcceleratorConfiguration                           ,
                    DIRECT_INTERFACE(css::lang::XServiceInfo)          ,
                    DIRECT_INTERFACE(css::lang::XInitialization))
//                    DIRECT_INTERFACE(css::ui::XUIConfigurationStorage))

DEFINE_XTYPEPROVIDER_2_WITH_BASECLASS(DocumentAcceleratorConfiguration ,
                                      AcceleratorConfiguration         ,
                                      css::lang::XServiceInfo          ,
                                      css::lang::XInitialization)
//                                      css::ui::XUIConfigurationStorage)

DEFINE_XSERVICEINFO_MULTISERVICE(DocumentAcceleratorConfiguration                   ,
                                 ::cppu::OWeakObject                                ,
                                 SERVICENAME_DOCUMENTACCELERATORCONFIGURATION       ,
                                 IMPLEMENTATIONNAME_DOCUMENTACCELERATORCONFIGURATION)

DEFINE_INIT_SERVICE(DocumentAcceleratorConfiguration,
                    {
                        /*Attention
                        I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                        to create a new instance of this class by our own supported service factory.
                        see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further informations!
                        */
                    }
                   )

//-----------------------------------------------
DocumentAcceleratorConfiguration::DocumentAcceleratorConfiguration(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR)
    : AcceleratorConfiguration(xSMGR)
{
}

//-----------------------------------------------
DocumentAcceleratorConfiguration::~DocumentAcceleratorConfiguration()
{
    m_aPresetHandler.removeStorageListener(this);
}

//-----------------------------------------------
void SAL_CALL DocumentAcceleratorConfiguration::initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    ::comphelper::SequenceAsHashMap lArgs(lArguments);
    m_xDocumentRoot = lArgs.getUnpackedValueOrDefault(
                        ::rtl::OUString::createFromAscii("DocumentRoot"),
                        css::uno::Reference< css::embed::XStorage >());

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    impl_ts_fillCache();
}

//-----------------------------------------------
void SAL_CALL DocumentAcceleratorConfiguration::setStorage(const css::uno::Reference< css::embed::XStorage >& xStorage)
    throw(css::uno::RuntimeException)
{
    // Attention! xStorage must be accepted too, if it's NULL !

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    sal_Bool bForgetOldStorages = m_xDocumentRoot.is();
    m_xDocumentRoot = xStorage;
    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    if (bForgetOldStorages)
        impl_ts_clearCache();

    if (xStorage.is())
        impl_ts_fillCache();
}

//-----------------------------------------------
sal_Bool SAL_CALL DocumentAcceleratorConfiguration::hasStorage()
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    return m_xDocumentRoot.is();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void DocumentAcceleratorConfiguration::impl_ts_fillCache()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::embed::XStorage > xDocumentRoot = m_xDocumentRoot;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    // Sometimes we must live without a document root.
    // E.g. if the document is readonly ...
    if (!xDocumentRoot.is())
        return;

    // get current office locale ... but dont cache it.
    // Otherwise we must be listener on the configuration layer
    // which seems to superflous for this small implementation .-)
    ::comphelper::Locale aLocale = impl_ts_getLocale();

    // May be the current document does not contain any
    // accelerator config? Handle it gracefully :-)
    try
    {
        // Note: The used preset class is threadsafe by itself ... and live if we live!
        // We do not need any mutex here.

        // open the folder, where the configuration exists
        m_aPresetHandler.connectToResource(
            PresetHandler::E_DOCUMENT,
            PresetHandler::RESOURCETYPE_ACCELERATOR(),
            ::rtl::OUString(),
            xDocumentRoot,
            aLocale);

        AcceleratorConfiguration::reload();
        m_aPresetHandler.addStorageListener(this);
    }
    /*

        Sometimes the configuration seams to be corrupted ..
        So it would be nice if we dont crash the office then .-)
        #121559#

    catch(const css::uno::RuntimeException& exRun)
        { throw exRun; }
    */
    catch(const css::uno::Exception&)
        {}
}

//-----------------------------------------------
void DocumentAcceleratorConfiguration::impl_ts_clearCache()
{
    m_aPresetHandler.forgetCachedStorages();
}

} // namespace framework
