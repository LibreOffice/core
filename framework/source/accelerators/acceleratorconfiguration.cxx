/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: acceleratorconfiguration.cxx,v $
 * $Revision: 1.8 $
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
#include <accelerators/acceleratorconfiguration.hxx>

//_______________________________________________
// own includes
#include <pattern/configuration.hxx>
#include <accelerators/presethandler.hxx>

#ifndef __FRAMEWORK_XML_SAXNAMESPACEFILTER_HXX_
#include <xml/saxnamespacefilter.hxx>
#endif

#ifndef __FRAMEWORK_XML_ACCELERATORCONFIGURATIONREADER_HXX_
#include <xml/acceleratorconfigurationreader.hxx>
#endif

#ifndef __FRAMEWORK_XML_ACCELERATORCONFIGURATIONWRITER_HXX_
#include <xml/acceleratorconfigurationwriter.hxx>
#endif
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

#ifndef __FRAMEWORK_ACCELERATORCONST_H_
#include <acceleratorconst.h>
#endif
#include <services.h>

//_______________________________________________
// interface includes
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

//_______________________________________________
// other includes
#include <vcl/svapp.hxx>

//_______________________________________________
// const

namespace framework
{

#ifdef fpc
    #error "Who exports this define? I use it as namespace alias ..."
#else
    namespace fpc = ::framework::pattern::configuration;
#endif

//-----------------------------------------------
//  XInterface, XTypeProvider
DEFINE_XINTERFACE_6(AcceleratorConfiguration                               ,
                    OWeakObject                                            ,
                    DIRECT_INTERFACE(css::lang::XTypeProvider             ),
                    DIRECT_INTERFACE(css::ui::XAcceleratorConfiguration  ),
                    DIRECT_INTERFACE(css::form::XReset                    ),
                    DIRECT_INTERFACE(css::ui::XUIConfigurationPersistence),
                    DIRECT_INTERFACE(css::ui::XUIConfigurationStorage    ),
                    DIRECT_INTERFACE(css::ui::XUIConfiguration           ))

DEFINE_XTYPEPROVIDER_6(AcceleratorConfiguration             ,
                       css::lang::XTypeProvider             ,
                       css::ui::XAcceleratorConfiguration  ,
                       css::form::XReset                    ,
                       css::ui::XUIConfigurationPersistence,
                       css::ui::XUIConfigurationStorage    ,
                       css::ui::XUIConfiguration           )

//-----------------------------------------------
AcceleratorConfiguration::AcceleratorConfiguration(const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR)
    : ThreadHelpBase  (&Application::GetSolarMutex())
    , m_xSMGR         (xSMGR                        )
    , m_aPresetHandler(xSMGR                        )
    , m_pWriteCache   (0                            )
{
}

//-----------------------------------------------
AcceleratorConfiguration::~AcceleratorConfiguration()
{
    LOG_ASSERT(!m_pWriteCache, "AcceleratorConfiguration::~AcceleratorConfiguration()\nChanges not flushed. Ignore it ...")
}

//-----------------------------------------------
css::uno::Sequence< css::awt::KeyEvent > SAL_CALL AcceleratorConfiguration::getAllKeyEvents()
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    AcceleratorCache&          rCache = impl_getCFG();
    AcceleratorCache::TKeyList lKeys  = rCache.getAllKeys();
    return lKeys.getAsConstList();

    // <- SAFE ----------------------------------
}

//-----------------------------------------------
::rtl::OUString SAL_CALL AcceleratorConfiguration::getCommandByKeyEvent(const css::awt::KeyEvent& aKeyEvent)
    throw(css::container::NoSuchElementException,
          css::uno::RuntimeException            )
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    AcceleratorCache& rCache = impl_getCFG();
    if (!rCache.hasKey(aKeyEvent))
        throw css::container::NoSuchElementException(
                ::rtl::OUString(),
                static_cast< ::cppu::OWeakObject* >(this));
    return rCache.getCommandByKey(aKeyEvent);

    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void SAL_CALL AcceleratorConfiguration::setKeyEvent(const css::awt::KeyEvent& aKeyEvent,
                                                    const ::rtl::OUString&    sCommand )
    throw(css::lang::IllegalArgumentException,
          css::uno::RuntimeException         )
{
    if (
        (aKeyEvent.KeyCode   == 0) &&
        (aKeyEvent.KeyChar   == 0) &&
        (aKeyEvent.KeyFunc   == 0) &&
        (aKeyEvent.Modifiers == 0)
       )
        throw css::lang::IllegalArgumentException(
                ::rtl::OUString::createFromAscii("Such key event seams not to be supported by any operating system."),
                static_cast< ::cppu::OWeakObject* >(this),
                0);

    if (!sCommand.getLength())
        throw css::lang::IllegalArgumentException(
                ::rtl::OUString::createFromAscii("Empty command strings are not allowed here."),
                static_cast< ::cppu::OWeakObject* >(this),
                1);

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    AcceleratorCache& rCache = impl_getCFG(sal_True); // TRUE => force getting of a writeable cache!
    rCache.setKeyCommandPair(aKeyEvent, sCommand);

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void SAL_CALL AcceleratorConfiguration::removeKeyEvent(const css::awt::KeyEvent& aKeyEvent)
    throw(css::container::NoSuchElementException,
          css::uno::RuntimeException            )
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    AcceleratorCache& rCache = impl_getCFG(sal_True); // true => force using of a writeable cache
    if (!rCache.hasKey(aKeyEvent))
        throw css::container::NoSuchElementException(
                ::rtl::OUString(),
                static_cast< ::cppu::OWeakObject* >(this));
    rCache.removeKey(aKeyEvent);

    // <- SAFE ----------------------------------
}

//-----------------------------------------------
css::uno::Sequence< css::awt::KeyEvent > SAL_CALL AcceleratorConfiguration::getKeyEventsByCommand(const ::rtl::OUString& sCommand)
    throw(css::lang::IllegalArgumentException   ,
          css::container::NoSuchElementException,
          css::uno::RuntimeException            )
{
    if (!sCommand.getLength())
        throw css::lang::IllegalArgumentException(
                ::rtl::OUString::createFromAscii("Empty command strings are not allowed here."),
                static_cast< ::cppu::OWeakObject* >(this),
                1);

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    AcceleratorCache& rCache = impl_getCFG();
    if (!rCache.hasCommand(sCommand))
        throw css::container::NoSuchElementException(
                ::rtl::OUString(),
                static_cast< ::cppu::OWeakObject* >(this));

    AcceleratorCache::TKeyList lKeys  = rCache.getKeysByCommand(sCommand);
    return lKeys.getAsConstList();

    // <- SAFE ----------------------------------
}

//-----------------------------------------------
css::uno::Sequence< css::uno::Any > SAL_CALL AcceleratorConfiguration::getPreferredKeyEventsForCommandList(const css::uno::Sequence< ::rtl::OUString >& lCommandList)
    throw(css::lang::IllegalArgumentException   ,
          css::uno::RuntimeException            )
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    sal_Int32                           i              = 0;
    sal_Int32                           c              = lCommandList.getLength();
    css::uno::Sequence< css::uno::Any > lPreferredOnes (c); // dont pack list!
    AcceleratorCache&                   rCache         = impl_getCFG();

    for (i=0; i<c; ++i)
    {
        const ::rtl::OUString& rCommand = lCommandList[i];
        if (!rCommand.getLength())
            throw css::lang::IllegalArgumentException(
                    ::rtl::OUString::createFromAscii("Empty command strings are not allowed here."),
                    static_cast< ::cppu::OWeakObject* >(this),
                    (sal_Int16)i);

        if (!rCache.hasCommand(rCommand))
            continue;

        AcceleratorCache::TKeyList lKeys = rCache.getKeysByCommand(rCommand);
        if (lKeys.size()<1)
            continue;

        css::uno::Any& rAny = lPreferredOnes[i];
        rAny <<= *(lKeys.begin());
    }

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    return lPreferredOnes;
}

//-----------------------------------------------
void SAL_CALL AcceleratorConfiguration::removeCommandFromAllKeyEvents(const ::rtl::OUString& sCommand)
    throw(css::lang::IllegalArgumentException   ,
          css::container::NoSuchElementException,
          css::uno::RuntimeException            )
{
    if (!sCommand.getLength())
        throw css::lang::IllegalArgumentException(
                ::rtl::OUString::createFromAscii("Empty command strings are not allowed here."),
                static_cast< ::cppu::OWeakObject* >(this),
                0);

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    AcceleratorCache& rCache = impl_getCFG(sal_True); // TRUE => force getting of a writeable cache!
    if (!rCache.hasCommand(sCommand))
        throw css::container::NoSuchElementException(
                ::rtl::OUString::createFromAscii("Command does not exists inside this container."),
                static_cast< ::cppu::OWeakObject* >(this));
    rCache.removeCommand(sCommand);

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void SAL_CALL AcceleratorConfiguration::reload()
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    css::uno::Reference< css::io::XStream > xStreamNoLang;

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::io::XStream > xStream = m_aPresetHandler.openTarget(PresetHandler::TARGET_CURRENT(), sal_True); // TRUE => open or create!
    try
    {
        xStreamNoLang = m_aPresetHandler.openPreset(PresetHandler::PRESET_DEFAULT(), sal_True);
    }
    catch(const css::io::IOException&) {} // does not have to exist
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::io::XInputStream > xIn;
    if (xStream.is())
        xIn = xStream->getInputStream();
    if (!xIn.is())
        throw css::io::IOException(
                ::rtl::OUString::createFromAscii("Could not open accelerator configuration for reading."),
                static_cast< ::cppu::OWeakObject* >(this));

    // impl_ts_load() does not clear the cache
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    m_aReadCache = AcceleratorCache();
    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    impl_ts_load(xIn);

    // Load also the general language independent default accelerators
    // (ignoring the already defined accelerators)
    if (xStreamNoLang.is())
    {
        xIn = xStreamNoLang->getInputStream();
        if (xIn.is())
            impl_ts_load(xIn);
    }
}

//-----------------------------------------------
void SAL_CALL AcceleratorConfiguration::store()
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::io::XStream > xStream = m_aPresetHandler.openTarget(PresetHandler::TARGET_CURRENT(), sal_True); // TRUE => open or create!
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::io::XOutputStream > xOut;
    if (xStream.is())
        xOut = xStream->getOutputStream();

    if (!xOut.is())
        throw css::io::IOException(
                ::rtl::OUString::createFromAscii("Could not open accelerator configuration for saving."),
                static_cast< ::cppu::OWeakObject* >(this));

    impl_ts_save(xOut);

    xOut.clear();
    xStream.clear();

    m_aPresetHandler.commitUserChanges();
}

//-----------------------------------------------
void SAL_CALL AcceleratorConfiguration::storeToStorage(const css::uno::Reference< css::embed::XStorage >& xStorage)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    css::uno::Reference< css::io::XStream > xStream = StorageHolder::openSubStreamWithFallback(
                                                            xStorage,
                                                            PresetHandler::TARGET_CURRENT(),
                                                            css::embed::ElementModes::READWRITE,
                                                            sal_False); // False => no fallback from read/write to readonly!
    css::uno::Reference< css::io::XOutputStream > xOut;
    if (xStream.is())
        xOut = xStream->getOutputStream();

    if (!xOut.is())
        throw css::io::IOException(
                ::rtl::OUString::createFromAscii("Could not open accelerator configuration for saving."),
                static_cast< ::cppu::OWeakObject* >(this));

    impl_ts_save(xOut);

    // TODO inform listener about success, so it can flush the root and sub storage of this stream!
}

//-----------------------------------------------
::sal_Bool SAL_CALL AcceleratorConfiguration::isModified()
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    return (m_pWriteCache != 0);
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
::sal_Bool SAL_CALL AcceleratorConfiguration::isReadOnly()
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::io::XStream > xStream = m_aPresetHandler.openTarget(PresetHandler::TARGET_CURRENT(), sal_True); // TRUE => open or create!
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::io::XOutputStream > xOut;
    if (xStream.is())
        xOut = xStream->getOutputStream();
    return !(xOut.is());
}

//-----------------------------------------------
void SAL_CALL AcceleratorConfiguration::setStorage(const css::uno::Reference< css::embed::XStorage >& /*xStorage*/)
    throw(css::uno::RuntimeException)
{
    LOG_WARNING("AcceleratorConfiguration::setStorage()", "TODO implement this HACK .-)")
}

//-----------------------------------------------
::sal_Bool SAL_CALL AcceleratorConfiguration::hasStorage()
    throw(css::uno::RuntimeException)
{
    LOG_WARNING("AcceleratorConfiguration::hasStorage()", "TODO implement this HACK .-)")
    return sal_False;
}

//-----------------------------------------------
void SAL_CALL AcceleratorConfiguration::addConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& /*xListener*/)
    throw(css::uno::RuntimeException)
{
    LOG_WARNING("AcceleratorConfiguration::addConfigurationListener()", "TODO implement me")
}

//-----------------------------------------------
void SAL_CALL AcceleratorConfiguration::removeConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& /*xListener*/)
    throw(css::uno::RuntimeException)
{
    LOG_WARNING("AcceleratorConfiguration::removeConfigurationListener()", "TODO implement me")
}
//-----------------------------------------------
void SAL_CALL AcceleratorConfiguration::reset()
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);
    m_aPresetHandler.copyPresetToTarget(PresetHandler::PRESET_DEFAULT(), PresetHandler::TARGET_CURRENT());
    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    reload();
}

//-----------------------------------------------
void SAL_CALL AcceleratorConfiguration::addResetListener(const css::uno::Reference< css::form::XResetListener >& /*xListener*/)
    throw(css::uno::RuntimeException)
{
    LOG_WARNING("AcceleratorConfiguration::addResetListener()", "TODO implement me")
}

//-----------------------------------------------
void SAL_CALL AcceleratorConfiguration::removeResetListener(const css::uno::Reference< css::form::XResetListener >& /*xListener*/)
    throw(css::uno::RuntimeException)
{
    LOG_WARNING("AcceleratorConfiguration::removeResetListener()", "TODO implement me")
}

//-----------------------------------------------
// IStorageListener
void AcceleratorConfiguration::changesOccured(const ::rtl::OUString& /*sPath*/)
{
    reload();
}

//-----------------------------------------------
void AcceleratorConfiguration::impl_ts_load(const css::uno::Reference< css::io::XInputStream >& xStream)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    if (m_pWriteCache)
    {
        // be aware of reentrance problems - use temp variable for calling delete ... :-)
        AcceleratorCache* pTemp = m_pWriteCache;
        m_pWriteCache = 0;
        delete pTemp;
    }

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::io::XSeekable > xSeek(xStream, css::uno::UNO_QUERY);
    if (xSeek.is())
        xSeek->seek(0);

    // add accelerators to the cache (the cache is not cleared)
    // SAFE -> ----------------------------------
    aWriteLock.lock();

    // create the parser queue
    // Note: Use special filter object between parser and reader
    // to get filtered xml with right namespaces ...
    // Use further a temp cache for reading!
    AcceleratorConfigurationReader*                        pReader = new AcceleratorConfigurationReader(m_aReadCache);
    css::uno::Reference< css::xml::sax::XDocumentHandler > xReader (static_cast< ::cppu::OWeakObject* >(pReader), css::uno::UNO_QUERY_THROW);
    SaxNamespaceFilter*                                    pFilter = new SaxNamespaceFilter(xReader);
    css::uno::Reference< css::xml::sax::XDocumentHandler > xFilter (static_cast< ::cppu::OWeakObject* >(pFilter), css::uno::UNO_QUERY_THROW);

    // connect parser, filter and stream
    css::uno::Reference< css::xml::sax::XParser > xParser(xSMGR->createInstance(SERVICENAME_SAXPARSER), css::uno::UNO_QUERY_THROW);
    xParser->setDocumentHandler(xFilter);

    css::xml::sax::InputSource aSource;
    aSource.aInputStream = xStream;

    // TODO think about error handling
    xParser->parseStream(aSource);

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void AcceleratorConfiguration::impl_ts_save(const css::uno::Reference< css::io::XOutputStream >& xStream)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    AcceleratorCache aCache;
    sal_Bool bChanged = (m_pWriteCache != 0);
    if (bChanged)
        aCache.takeOver(*m_pWriteCache);
    else
        aCache.takeOver(m_aReadCache);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::io::XTruncate > xClearable(xStream, css::uno::UNO_QUERY_THROW);
    xClearable->truncate();

    // TODO can be removed if seek(0) is done by truncate() automaticly!
    css::uno::Reference< css::io::XSeekable > xSeek(xStream, css::uno::UNO_QUERY);
    if (xSeek.is())
        xSeek->seek(0);

    // combine writer/cache/stream etcpp.
    css::uno::Reference< css::xml::sax::XDocumentHandler > xWriter    (xSMGR->createInstance(SERVICENAME_SAXWRITER), css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::io::XActiveDataSource>       xDataSource(xWriter                                     , css::uno::UNO_QUERY_THROW);
    xDataSource->setOutputStream(xStream);

    // write into the stream
    AcceleratorConfigurationWriter aWriter(aCache, xWriter);
    aWriter.flush();

    // take over all changes into the original container
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // take over all changes into the readonly cache ...
    // and forget the copy-on-write copied cache
    if (bChanged)
    {
        m_aReadCache.takeOver(*m_pWriteCache);
        // live with reentrance .-)
        AcceleratorCache* pTemp = m_pWriteCache;
        m_pWriteCache = 0;
        delete pTemp;
    }

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
AcceleratorCache& AcceleratorConfiguration::impl_getCFG(sal_Bool bWriteAccessRequested)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    // create copy of our readonly-cache, if write access is forced ... but
    // not still possible!
    if (
        (bWriteAccessRequested) &&
        (!m_pWriteCache       )
       )
    {
        m_pWriteCache = new AcceleratorCache(m_aReadCache);
    }

    // in case, we have a writeable cache, we use it for reading too!
    // Otherwhise the API user cant find its own changes ...
    if (m_pWriteCache)
        return *m_pWriteCache;
    else
        return m_aReadCache;
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
::comphelper::Locale AcceleratorConfiguration::impl_ts_getLocale() const
{
    static ::rtl::OUString LOCALE_PACKAGE = ::rtl::OUString::createFromAscii("/org.openoffice.Setup");
    static ::rtl::OUString LOCALE_PATH    = ::rtl::OUString::createFromAscii("L10N"                 );
    static ::rtl::OUString LOCALE_KEY     = ::rtl::OUString::createFromAscii("ooLocale"             );

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::uno::XInterface >     xCFG      = fpc::ConfigurationHelper::openConfig(xSMGR, LOCALE_PACKAGE, LOCALE_PATH, fpc::ConfigurationHelper::E_READONLY);
    css::uno::Reference< css::beans::XPropertySet > xProp     (xCFG, css::uno::UNO_QUERY_THROW);
    ::rtl::OUString                                 sISOLocale;
    xProp->getPropertyValue(LOCALE_KEY) >>= sISOLocale;

    if (!sISOLocale.getLength())
        return ::comphelper::Locale::EN_US();
    return ::comphelper::Locale(sISOLocale);
}

} // namespace framework
