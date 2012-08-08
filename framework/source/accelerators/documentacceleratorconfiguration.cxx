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

#include <accelerators/documentacceleratorconfiguration.hxx>

#include <xml/acceleratorconfigurationreader.hxx>

#include <xml/acceleratorconfigurationwriter.hxx>

#include <xml/saxnamespacefilter.hxx>

#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

#include <acceleratorconst.h>
#include <services.h>

#include <com/sun/star/io/XActiveDataSource.hpp>

#include <com/sun/star/io/XSeekable.hpp>

#include <com/sun/star/io/XTruncate.hpp>

#include <com/sun/star/embed/ElementModes.hpp>

#include <com/sun/star/xml/sax/InputSource.hpp>

#include <com/sun/star/xml/sax/XParser.hpp>

#include <comphelper/sequenceashashmap.hxx>

//_______________________________________________
// const

namespace framework
{

//-----------------------------------------------
// XInterface, XTypeProvider, XServiceInfo
DEFINE_XINTERFACE_2(DocumentAcceleratorConfiguration                   ,
                    XMLBasedAcceleratorConfiguration                           ,
                    DIRECT_INTERFACE(css::lang::XServiceInfo)          ,
                    DIRECT_INTERFACE(css::lang::XInitialization))
//                    DIRECT_INTERFACE(css::ui::XUIConfigurationStorage))

DEFINE_XTYPEPROVIDER_2_WITH_BASECLASS(DocumentAcceleratorConfiguration ,
                                      XMLBasedAcceleratorConfiguration         ,
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
    : XMLBasedAcceleratorConfiguration(xSMGR)
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
                        ::rtl::OUString("DocumentRoot"),
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

        DocumentAcceleratorConfiguration::reload();
        m_aPresetHandler.addStorageListener(this);
    }
    catch(const css::uno::Exception&)
    {}
}

//-----------------------------------------------
void DocumentAcceleratorConfiguration::impl_ts_clearCache()
{
    m_aPresetHandler.forgetCachedStorages();
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
