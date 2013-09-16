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

#include <accelerators/acceleratorconfiguration.hxx>

#include <pattern/configuration.hxx>
#include <accelerators/presethandler.hxx>

#include <xml/saxnamespacefilter.hxx>
#include <xml/acceleratorconfigurationreader.hxx>
#include <xml/acceleratorconfigurationwriter.hxx>

#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

#include <acceleratorconst.h>
#include <services.h>

#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <vcl/svapp.hxx>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <comphelper/configurationhelper.hxx>
#include <unotools/configpaths.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <stdio.h>


namespace framework
{
    const char CFG_ENTRY_SECONDARY[] = "SecondaryKeys";
    const char CFG_PROP_COMMAND[] = "Command";

#ifdef fpc
    #error "Who exports this define? I use it as namespace alias ..."
#else
    namespace fpc = ::framework::pattern::configuration;
#endif

    OUString lcl_getKeyString(salhelper::SingletonRef<framework::KeyMapping>& _rKeyMapping, const css::awt::KeyEvent& aKeyEvent)
    {
        const sal_Int32 nBeginIndex = 4; // "KEY_" is the prefix of a identifier...
        OUStringBuffer sKeyBuffer((_rKeyMapping->mapCodeToIdentifier(aKeyEvent.KeyCode)).copy(nBeginIndex));

        if ( (aKeyEvent.Modifiers & css::awt::KeyModifier::SHIFT) == css::awt::KeyModifier::SHIFT )
            sKeyBuffer.appendAscii("_SHIFT");
        if ( (aKeyEvent.Modifiers & css::awt::KeyModifier::MOD1 ) == css::awt::KeyModifier::MOD1  )
            sKeyBuffer.appendAscii("_MOD1");
        if ( (aKeyEvent.Modifiers & css::awt::KeyModifier::MOD2 ) == css::awt::KeyModifier::MOD2  )
            sKeyBuffer.appendAscii("_MOD2");
        if ( (aKeyEvent.Modifiers & css::awt::KeyModifier::MOD3 ) == css::awt::KeyModifier::MOD3  )
            sKeyBuffer.appendAscii("_MOD3");

        return sKeyBuffer.makeStringAndClear();
    }

//-----------------------------------------------
XMLBasedAcceleratorConfiguration::XMLBasedAcceleratorConfiguration(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : ThreadHelpBase  (&Application::GetSolarMutex())
    , m_xContext      (xContext                     )
    , m_aPresetHandler(xContext                     )
    , m_pWriteCache   (0                            )
{
}

//-----------------------------------------------
XMLBasedAcceleratorConfiguration::~XMLBasedAcceleratorConfiguration()
{
    SAL_WARN_IF(m_pWriteCache, "fwk", "XMLBasedAcceleratorConfiguration::~XMLBasedAcceleratorConfiguration(): Changes not flushed. Ignore it ...");
}

//-----------------------------------------------
css::uno::Sequence< css::awt::KeyEvent > SAL_CALL XMLBasedAcceleratorConfiguration::getAllKeyEvents()
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
OUString SAL_CALL XMLBasedAcceleratorConfiguration::getCommandByKeyEvent(const css::awt::KeyEvent& aKeyEvent)
    throw(css::container::NoSuchElementException,
          css::uno::RuntimeException            )
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    AcceleratorCache& rCache = impl_getCFG();
    if (!rCache.hasKey(aKeyEvent))
        throw css::container::NoSuchElementException(
                OUString(),
                static_cast< ::cppu::OWeakObject* >(this));
    return rCache.getCommandByKey(aKeyEvent);

    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void SAL_CALL XMLBasedAcceleratorConfiguration::setKeyEvent(const css::awt::KeyEvent& aKeyEvent,
                                                    const OUString&    sCommand )
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
        OUString("Such key event seams not to be supported by any operating system."),
        static_cast< ::cppu::OWeakObject* >(this),
        0);

    if (sCommand.isEmpty())
        throw css::lang::IllegalArgumentException(
        OUString("Empty command strings are not allowed here."),
        static_cast< ::cppu::OWeakObject* >(this),
        1);

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    AcceleratorCache& rCache = impl_getCFG(sal_True); // sal_True => force getting of a writeable cache!
    rCache.setKeyCommandPair(aKeyEvent, sCommand);

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void SAL_CALL XMLBasedAcceleratorConfiguration::removeKeyEvent(const css::awt::KeyEvent& aKeyEvent)
throw(css::container::NoSuchElementException,
      css::uno::RuntimeException            )
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    AcceleratorCache& rCache = impl_getCFG(sal_True); // true => force using of a writeable cache
    if (!rCache.hasKey(aKeyEvent))
        throw css::container::NoSuchElementException(
        OUString(),
        static_cast< ::cppu::OWeakObject* >(this));
    rCache.removeKey(aKeyEvent);

    // <- SAFE ----------------------------------
}

//-----------------------------------------------
css::uno::Sequence< css::awt::KeyEvent > SAL_CALL XMLBasedAcceleratorConfiguration::getKeyEventsByCommand(const OUString& sCommand)
    throw(css::lang::IllegalArgumentException   ,
          css::container::NoSuchElementException,
          css::uno::RuntimeException            )
{
    if (sCommand.isEmpty())
        throw css::lang::IllegalArgumentException(
                OUString("Empty command strings are not allowed here."),
                static_cast< ::cppu::OWeakObject* >(this),
                1);

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    AcceleratorCache& rCache = impl_getCFG();
    if (!rCache.hasCommand(sCommand))
        throw css::container::NoSuchElementException(
                OUString(),
                static_cast< ::cppu::OWeakObject* >(this));

    AcceleratorCache::TKeyList lKeys  = rCache.getKeysByCommand(sCommand);
    return lKeys.getAsConstList();

    // <- SAFE ----------------------------------
}

//-----------------------------------------------
css::uno::Sequence< css::uno::Any > SAL_CALL XMLBasedAcceleratorConfiguration::getPreferredKeyEventsForCommandList(const css::uno::Sequence< OUString >& lCommandList)
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
        const OUString& rCommand = lCommandList[i];
        if (rCommand.isEmpty())
            throw css::lang::IllegalArgumentException(
                    OUString("Empty command strings are not allowed here."),
                    static_cast< ::cppu::OWeakObject* >(this),
                    (sal_Int16)i);

        if (!rCache.hasCommand(rCommand))
            continue;

        AcceleratorCache::TKeyList lKeys = rCache.getKeysByCommand(rCommand);
        if ( lKeys.empty() )
            continue;

        css::uno::Any& rAny = lPreferredOnes[i];
        rAny <<= *(lKeys.begin());
    }

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    return lPreferredOnes;
}

//-----------------------------------------------
void SAL_CALL XMLBasedAcceleratorConfiguration::removeCommandFromAllKeyEvents(const OUString& sCommand)
    throw(css::lang::IllegalArgumentException   ,
          css::container::NoSuchElementException,
          css::uno::RuntimeException            )
{
    if (sCommand.isEmpty())
        throw css::lang::IllegalArgumentException(
                OUString("Empty command strings are not allowed here."),
                static_cast< ::cppu::OWeakObject* >(this),
                0);

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    AcceleratorCache& rCache = impl_getCFG(sal_True); // sal_True => force getting of a writeable cache!
    if (!rCache.hasCommand(sCommand))
        throw css::container::NoSuchElementException(
                OUString("Command does not exists inside this container."),
                static_cast< ::cppu::OWeakObject* >(this));
    rCache.removeCommand(sCommand);

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void SAL_CALL XMLBasedAcceleratorConfiguration::reload()
    throw(css::uno::Exception       ,
        css::uno::RuntimeException)
{
    css::uno::Reference< css::io::XStream > xStreamNoLang;

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::io::XStream > xStream = m_aPresetHandler.openTarget(PresetHandler::TARGET_CURRENT(), sal_True); // sal_True => open or create!
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
        OUString("Could not open accelerator configuration for reading."),
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
void SAL_CALL XMLBasedAcceleratorConfiguration::store()
    throw(css::uno::Exception       ,
        css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::io::XStream > xStream = m_aPresetHandler.openTarget(PresetHandler::TARGET_CURRENT(), sal_True); // sal_True => open or create!
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::io::XOutputStream > xOut;
    if (xStream.is())
        xOut = xStream->getOutputStream();

    if (!xOut.is())
        throw css::io::IOException(
        OUString("Could not open accelerator configuration for saving."),
        static_cast< ::cppu::OWeakObject* >(this));

    impl_ts_save(xOut);

    xOut.clear();
    xStream.clear();

    m_aPresetHandler.commitUserChanges();
}

//-----------------------------------------------
void SAL_CALL XMLBasedAcceleratorConfiguration::storeToStorage(const css::uno::Reference< css::embed::XStorage >& xStorage)
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
                OUString("Could not open accelerator configuration for saving."),
                static_cast< ::cppu::OWeakObject* >(this));

    impl_ts_save(xOut);

    // TODO inform listener about success, so it can flush the root and sub storage of this stream!
}

//-----------------------------------------------
::sal_Bool SAL_CALL XMLBasedAcceleratorConfiguration::isModified()
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    return (m_pWriteCache != 0);
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
::sal_Bool SAL_CALL XMLBasedAcceleratorConfiguration::isReadOnly()
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::io::XStream > xStream = m_aPresetHandler.openTarget(PresetHandler::TARGET_CURRENT(), sal_True); // sal_True => open or create!
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::io::XOutputStream > xOut;
    if (xStream.is())
        xOut = xStream->getOutputStream();
    return !(xOut.is());
}

//-----------------------------------------------
void SAL_CALL XMLBasedAcceleratorConfiguration::setStorage(const css::uno::Reference< css::embed::XStorage >& /*xStorage*/)
    throw(css::uno::RuntimeException)
{
    SAL_INFO("fwk", "XMLBasedAcceleratorConfiguration::setStorage(): implement this HACK .-)");
}

//-----------------------------------------------
::sal_Bool SAL_CALL XMLBasedAcceleratorConfiguration::hasStorage()
    throw(css::uno::RuntimeException)
{
    SAL_INFO("fwk", "XMLBasedAcceleratorConfiguration::hasStorage(): implement this HACK .-)");
    return sal_False;
}

//-----------------------------------------------
void SAL_CALL XMLBasedAcceleratorConfiguration::addConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& /*xListener*/)
    throw(css::uno::RuntimeException)
{
    SAL_INFO("fwk", "XMLBasedAcceleratorConfiguration::addConfigurationListener(): implement me");
}

//-----------------------------------------------
void SAL_CALL XMLBasedAcceleratorConfiguration::removeConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& /*xListener*/)
    throw(css::uno::RuntimeException)
{
    SAL_INFO("fwk", "XMLBasedAcceleratorConfiguration::removeConfigurationListener(): implement me");
}

//-----------------------------------------------
void SAL_CALL XMLBasedAcceleratorConfiguration::reset()
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
void SAL_CALL XMLBasedAcceleratorConfiguration::addResetListener(const css::uno::Reference< css::form::XResetListener >& /*xListener*/)
    throw(css::uno::RuntimeException)
{
    SAL_INFO("fwk", "XMLBasedAcceleratorConfiguration::addResetListener(): implement me");
}

//-----------------------------------------------
void SAL_CALL XMLBasedAcceleratorConfiguration::removeResetListener(const css::uno::Reference< css::form::XResetListener >& /*xListener*/)
    throw(css::uno::RuntimeException)
{
    SAL_INFO("fwk", "XMLBasedAcceleratorConfiguration::removeResetListener(): implement me");
}

//-----------------------------------------------
// IStorageListener
void XMLBasedAcceleratorConfiguration::changesOccurred(const OUString& /*sPath*/)
{
    reload();
}

//-----------------------------------------------
void XMLBasedAcceleratorConfiguration::impl_ts_load(const css::uno::Reference< css::io::XInputStream >& xStream)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
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
    css::uno::Reference< css::xml::sax::XParser > xParser = css::xml::sax::Parser::create(xContext);
    xParser->setDocumentHandler(xFilter);

    css::xml::sax::InputSource aSource;
    aSource.aInputStream = xStream;

    // TODO think about error handling
    xParser->parseStream(aSource);

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void XMLBasedAcceleratorConfiguration::impl_ts_save(const css::uno::Reference< css::io::XOutputStream >& xStream)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    AcceleratorCache aCache;
    sal_Bool bChanged = (m_pWriteCache != 0);
    if (bChanged)
        aCache.takeOver(*m_pWriteCache);
    else
        aCache.takeOver(m_aReadCache);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::io::XTruncate > xClearable(xStream, css::uno::UNO_QUERY_THROW);
    xClearable->truncate();

    // TODO can be removed if seek(0) is done by truncate() automaticly!
    css::uno::Reference< css::io::XSeekable > xSeek(xStream, css::uno::UNO_QUERY);
    if (xSeek.is())
        xSeek->seek(0);

    // combine writer/cache/stream etcpp.
    css::uno::Reference< css::xml::sax::XWriter > xWriter = css::xml::sax::Writer::create(xContext);
    xWriter->setOutputStream(xStream);

    // write into the stream
    css::uno::Reference< css::xml::sax::XDocumentHandler > xHandler(xWriter, css::uno::UNO_QUERY_THROW);
    AcceleratorConfigurationWriter aWriter(aCache, xHandler);
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
AcceleratorCache& XMLBasedAcceleratorConfiguration::impl_getCFG(sal_Bool bWriteAccessRequested)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    //create copy of our readonly-cache, if write access is forced ... but
    //not still possible!
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
OUString XMLBasedAcceleratorConfiguration::impl_ts_getLocale() const
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::uno::XInterface >     xCFG      = fpc::ConfigurationHelper::openConfig( xContext,
        "/org.openoffice.Setup", "L10N", fpc::ConfigurationHelper::E_READONLY);
    css::uno::Reference< css::beans::XPropertySet > xProp     (xCFG, css::uno::UNO_QUERY_THROW);
    OUString                                 sISOLocale;
    xProp->getPropertyValue("ooLocale") >>= sISOLocale;

    if (sISOLocale.isEmpty())
        return OUString("en-US");
    return sISOLocale;
}

/*******************************************************************************
*
* XCU based accelerator configuration
*
*******************************************************************************/

//-----------------------------------------------
XCUBasedAcceleratorConfiguration::XCUBasedAcceleratorConfiguration(const css::uno::Reference< css::uno::XComponentContext >& xContext)
                                : ThreadHelpBase  (&Application::GetSolarMutex())
                                , m_xContext      (xContext                     )
                                , m_pPrimaryWriteCache(0                        )
                                , m_pSecondaryWriteCache(0                      )
{
    const OUString CFG_ENTRY_ACCELERATORS("org.openoffice.Office.Accelerators");
    m_xCfg = css::uno::Reference< css::container::XNameAccess > (
             ::comphelper::ConfigurationHelper::openConfig( m_xContext, CFG_ENTRY_ACCELERATORS, ::comphelper::ConfigurationHelper::E_ALL_LOCALES ),
             css::uno::UNO_QUERY );
}

//-----------------------------------------------
XCUBasedAcceleratorConfiguration::~XCUBasedAcceleratorConfiguration()
{
}

//-----------------------------------------------
css::uno::Sequence< css::awt::KeyEvent > SAL_CALL XCUBasedAcceleratorConfiguration::getAllKeyEvents()
    throw(css::uno::RuntimeException)
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    AcceleratorCache::TKeyList lKeys  = impl_getCFG(sal_True).getAllKeys(); //get keys from PrimaryKeys set

    AcceleratorCache::TKeyList lSecondaryKeys = impl_getCFG(sal_False).getAllKeys(); //get keys from SecondaryKeys set
    lKeys.reserve(lKeys.size()+lSecondaryKeys.size());
    AcceleratorCache::TKeyList::const_iterator pIt;
    AcceleratorCache::TKeyList::const_iterator pEnd = lSecondaryKeys.end();
    for ( pIt  = lSecondaryKeys.begin(); pIt != pEnd; ++pIt )
        lKeys.push_back(*pIt);

    return lKeys.getAsConstList();

    // <- SAFE ----------------------------------
}

//-----------------------------------------------
OUString SAL_CALL XCUBasedAcceleratorConfiguration::getCommandByKeyEvent(const css::awt::KeyEvent& aKeyEvent)
    throw(css::container::NoSuchElementException,
          css::uno::RuntimeException            )
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    AcceleratorCache& rPrimaryCache   = impl_getCFG(sal_True );
    AcceleratorCache& rSecondaryCache = impl_getCFG(sal_False);

    if (!rPrimaryCache.hasKey(aKeyEvent) && !rSecondaryCache.hasKey(aKeyEvent))
        throw css::container::NoSuchElementException(
        OUString(),
        static_cast< ::cppu::OWeakObject* >(this));

    if (rPrimaryCache.hasKey(aKeyEvent))
        return rPrimaryCache.getCommandByKey(aKeyEvent);
    else
        return rSecondaryCache.getCommandByKey(aKeyEvent);

    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::setKeyEvent(const css::awt::KeyEvent& aKeyEvent,
                                                    const OUString&    sCommand )
                                                    throw(css::lang::IllegalArgumentException,
                                                    css::uno::RuntimeException         )
{
    SAL_INFO( "fwk.accelerators", "XCUBasedAcceleratorConfiguration::setKeyEvent" );

    if (
        (aKeyEvent.KeyCode   == 0) &&
        (aKeyEvent.KeyChar   == 0) &&
        (aKeyEvent.KeyFunc   == 0) &&
        (aKeyEvent.Modifiers == 0)
        )
        throw css::lang::IllegalArgumentException(
                OUString("Such key event seams not to be supported by any operating system."),
                static_cast< ::cppu::OWeakObject* >(this),
                0);

    if (sCommand.isEmpty())
                throw css::lang::IllegalArgumentException(
                OUString("Empty command strings are not allowed here."),
                static_cast< ::cppu::OWeakObject* >(this),
                1);

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    AcceleratorCache& rPrimaryCache   = impl_getCFG(sal_True, sal_True ); // sal_True => force getting of a writeable cache!
    AcceleratorCache& rSecondaryCache = impl_getCFG(sal_False, sal_True); // sal_True => force getting of a writeable cache!

    if ( rPrimaryCache.hasKey(aKeyEvent) )
    {
        OUString sOriginalCommand = rPrimaryCache.getCommandByKey(aKeyEvent);
        if ( sCommand != sOriginalCommand )
        {
            if (rSecondaryCache.hasCommand(sOriginalCommand))
            {
                AcceleratorCache::TKeyList lSecondaryKeys = rSecondaryCache.getKeysByCommand(sOriginalCommand);
                rSecondaryCache.removeKey(lSecondaryKeys[0]);
                rPrimaryCache.setKeyCommandPair(lSecondaryKeys[0], sOriginalCommand);
            }

            if (rPrimaryCache.hasCommand(sCommand))
            {
                AcceleratorCache::TKeyList lPrimaryKeys = rPrimaryCache.getKeysByCommand(sCommand);
                rPrimaryCache.removeKey(lPrimaryKeys[0]);
                rSecondaryCache.setKeyCommandPair(lPrimaryKeys[0], sCommand);
            }

            rPrimaryCache.setKeyCommandPair(aKeyEvent, sCommand);
        }
    }

    else if ( rSecondaryCache.hasKey(aKeyEvent) )
    {
        OUString sOriginalCommand = rSecondaryCache.getCommandByKey(aKeyEvent);
        if (sCommand != sOriginalCommand)
        {
            if (rPrimaryCache.hasCommand(sCommand))
            {
                AcceleratorCache::TKeyList lPrimaryKeys = rPrimaryCache.getKeysByCommand(sCommand);
                rPrimaryCache.removeKey(lPrimaryKeys[0]);
                rSecondaryCache.setKeyCommandPair(lPrimaryKeys[0], sCommand);
            }

            rSecondaryCache.removeKey(aKeyEvent);
            rPrimaryCache.setKeyCommandPair(aKeyEvent, sCommand);
        }
    }

    else
    {
        if (rPrimaryCache.hasCommand(sCommand))
        {
            AcceleratorCache::TKeyList lPrimaryKeys = rPrimaryCache.getKeysByCommand(sCommand);
            rPrimaryCache.removeKey(lPrimaryKeys[0]);
            rSecondaryCache.setKeyCommandPair(lPrimaryKeys[0], sCommand);
        }

        rPrimaryCache.setKeyCommandPair(aKeyEvent, sCommand);
    }

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::removeKeyEvent(const css::awt::KeyEvent& aKeyEvent)
    throw(css::container::NoSuchElementException,
          css::uno::RuntimeException            )
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    AcceleratorCache& rPrimaryCache   = impl_getCFG(sal_True, sal_True );
    AcceleratorCache& rSecondaryCache = impl_getCFG(sal_False, sal_True);

    if (!rPrimaryCache.hasKey(aKeyEvent) && !rSecondaryCache.hasKey(aKeyEvent))
        throw css::container::NoSuchElementException(
        OUString(),
        static_cast< ::cppu::OWeakObject* >(this));

    if (rPrimaryCache.hasKey(aKeyEvent))
    {
        OUString sDelCommand = rPrimaryCache.getCommandByKey(aKeyEvent);
        if (!sDelCommand.isEmpty())
        {
            OUString sOriginalCommand = rPrimaryCache.getCommandByKey(aKeyEvent);
            if (rSecondaryCache.hasCommand(sOriginalCommand))
            {
                AcceleratorCache::TKeyList lSecondaryKeys = rSecondaryCache.getKeysByCommand(sOriginalCommand);
                rSecondaryCache.removeKey(lSecondaryKeys[0]);
                rPrimaryCache.setKeyCommandPair(lSecondaryKeys[0], sOriginalCommand);
            }

            rPrimaryCache.removeKey(aKeyEvent);
        }

    }
    else
    {
        OUString sDelCommand = rSecondaryCache.getCommandByKey(aKeyEvent);
        if (!sDelCommand.isEmpty())
            rSecondaryCache.removeKey(aKeyEvent);
    }

    // <- SAFE ----------------------------------
}

//-----------------------------------------------
css::uno::Sequence< css::awt::KeyEvent > SAL_CALL XCUBasedAcceleratorConfiguration::getKeyEventsByCommand(const OUString& sCommand)
    throw(css::lang::IllegalArgumentException   ,
        css::container::NoSuchElementException,
        css::uno::RuntimeException            )
{
    if (sCommand.isEmpty())
        throw css::lang::IllegalArgumentException(
                OUString("Empty command strings are not allowed here."),
                static_cast< ::cppu::OWeakObject* >(this),
                1);

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    AcceleratorCache& rPrimaryCache   = impl_getCFG(sal_True );
    AcceleratorCache& rSecondaryCache = impl_getCFG(sal_False);

    if (!rPrimaryCache.hasCommand(sCommand) && !rSecondaryCache.hasCommand(sCommand))
        throw css::container::NoSuchElementException(
                OUString(),
                static_cast< ::cppu::OWeakObject* >(this));

    AcceleratorCache::TKeyList lKeys  = rPrimaryCache.getKeysByCommand(sCommand);

    AcceleratorCache::TKeyList lSecondaryKeys = rSecondaryCache.getKeysByCommand(sCommand);
    AcceleratorCache::TKeyList::const_iterator pIt;
    for (pIt = lSecondaryKeys.begin(); pIt != lSecondaryKeys.end(); ++pIt)
        lKeys.push_back(*pIt);

    return lKeys.getAsConstList();

    // <- SAFE ----------------------------------
}

//-----------------------------------------------
AcceleratorCache::TKeyList::const_iterator lcl_getPreferredKey(const AcceleratorCache::TKeyList& lKeys)
{
    AcceleratorCache::TKeyList::const_iterator pIt;
    for (  pIt  = lKeys.begin ();
           pIt != lKeys.end   ();
         ++pIt                  )
    {
        const css::awt::KeyEvent& rAWTKey = *pIt;
        const KeyCode             aVCLKey = ::svt::AcceleratorExecute::st_AWTKey2VCLKey(rAWTKey);
        const OUString            sName   = aVCLKey.GetName();

        if (!sName.isEmpty())
            return pIt;
    }

    return lKeys.end();
}

//-----------------------------------------------
css::uno::Sequence< css::uno::Any > SAL_CALL XCUBasedAcceleratorConfiguration::getPreferredKeyEventsForCommandList(const css::uno::Sequence< OUString >& lCommandList)
    throw(css::lang::IllegalArgumentException   ,
        css::uno::RuntimeException            )
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    sal_Int32                           i              = 0;
    sal_Int32                           c              = lCommandList.getLength();
    css::uno::Sequence< css::uno::Any > lPreferredOnes (c); // dont pack list!
    AcceleratorCache&                   rCache         = impl_getCFG(sal_True);

    for (i=0; i<c; ++i)
    {
        const OUString& rCommand = lCommandList[i];
        if (rCommand.isEmpty())
            throw css::lang::IllegalArgumentException(
                    OUString("Empty command strings are not allowed here."),
                    static_cast< ::cppu::OWeakObject* >(this),
                    (sal_Int16)i);

        if (!rCache.hasCommand(rCommand))
            continue;

        AcceleratorCache::TKeyList lKeys = rCache.getKeysByCommand(rCommand);
        if ( lKeys.empty() )
            continue;

        AcceleratorCache::TKeyList::const_iterator pPreferredKey = lcl_getPreferredKey(lKeys);
        if (pPreferredKey != lKeys.end ())
        {
            css::uno::Any& rAny = lPreferredOnes[i];
            rAny <<= *(pPreferredKey);
        }
    }

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    return lPreferredOnes;
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::removeCommandFromAllKeyEvents(const OUString& sCommand)
    throw(css::lang::IllegalArgumentException   ,
        css::container::NoSuchElementException,
        css::uno::RuntimeException            )
{
    if (sCommand.isEmpty())
        throw css::lang::IllegalArgumentException(
                OUString("Empty command strings are not allowed here."),
                static_cast< ::cppu::OWeakObject* >(this),
                0);

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    AcceleratorCache& rPrimaryCache   = impl_getCFG(sal_True, sal_True );
    AcceleratorCache& rSecondaryCache = impl_getCFG(sal_False, sal_True);

    if (!rPrimaryCache.hasCommand(sCommand) && !rSecondaryCache.hasCommand(sCommand))
        throw css::container::NoSuchElementException(
                OUString("Command does not exists inside this container."),
                static_cast< ::cppu::OWeakObject* >(this));

    if (rPrimaryCache.hasCommand(sCommand))
        rPrimaryCache.removeCommand(sCommand);
    if (rSecondaryCache.hasCommand(sCommand))
        rSecondaryCache.removeCommand(sCommand);

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::reload()
    throw(css::uno::Exception       ,
        css::uno::RuntimeException)
{
    SAL_INFO( "fwk.accelerators", "XCUBasedAcceleratorConfiguration::reload()" );

    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    sal_Bool bPreferred;
    css::uno::Reference< css::container::XNameAccess > xAccess;

    bPreferred = sal_True;
    m_aPrimaryReadCache = AcceleratorCache();
    if (m_pPrimaryWriteCache)
    {
        // be aware of reentrance problems - use temp variable for calling delete ... :-)
        AcceleratorCache* pTemp = m_pPrimaryWriteCache;
        m_pPrimaryWriteCache = 0;
        delete pTemp;
    }
    m_xCfg->getByName(CFG_ENTRY_PRIMARY) >>= xAccess;
    impl_ts_load(bPreferred, xAccess); // load the preferred keys

    bPreferred = sal_False;
    m_aSecondaryReadCache = AcceleratorCache();
    if (m_pSecondaryWriteCache)
    {
        // be aware of reentrance problems - use temp variable for calling delete ... :-)
        AcceleratorCache* pTemp = m_pSecondaryWriteCache;
        m_pSecondaryWriteCache = 0;
        delete pTemp;
    }
    m_xCfg->getByName(CFG_ENTRY_SECONDARY) >>= xAccess;
    impl_ts_load(bPreferred, xAccess); // load the secondary keys

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::store()
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    SAL_INFO( "fwk.accelerators", "XCUBasedAcceleratorConfiguration::store()" );

    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    sal_Bool bPreferred;
    css::uno::Reference< css::container::XNameAccess > xAccess;

    bPreferred = sal_True;
    // on-demand creation of the primary write cache
    impl_getCFG(bPreferred, sal_True);
    m_xCfg->getByName(CFG_ENTRY_PRIMARY) >>= xAccess;
    impl_ts_save(bPreferred, xAccess);

    bPreferred = sal_False;
    // on-demand creation of the secondary write cache
    impl_getCFG(bPreferred, sal_True);
    m_xCfg->getByName(CFG_ENTRY_SECONDARY) >>= xAccess;
    impl_ts_save(bPreferred, xAccess);

    aReadLock.unlock();
    // <- SAFE ----------------------------------
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::storeToStorage(const css::uno::Reference< css::embed::XStorage >& xStorage)
    throw(css::uno::Exception       ,
      css::uno::RuntimeException)
{
    // use m_aCache + old AcceleratorXMLWriter to store data directly on storage given as parameter ...
    if (!xStorage.is())
        return;

    long nOpenModes = css::embed::ElementModes::READWRITE;
    css::uno::Reference< css::embed::XStorage > xAcceleratorTypeStorage = xStorage->openStorageElement(OUString("accelerator"), nOpenModes);
    if (!xAcceleratorTypeStorage.is())
        return;

    css::uno::Reference< css::io::XStream > xStream = xAcceleratorTypeStorage->openStreamElement(OUString("current"), nOpenModes);
    css::uno::Reference< css::io::XOutputStream > xOut;
    if (xStream.is())
        xOut = xStream->getOutputStream();
    if (!xOut.is())
        throw css::io::IOException(
        OUString("Could not open accelerator configuration for saving."),
        static_cast< ::cppu::OWeakObject* >(this));

    // the original m_aCache has been split into primay cache and secondary cache...
    // we should merge them before storing to storage
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    AcceleratorCache aCache;
    if (m_pPrimaryWriteCache != 0)
        aCache.takeOver(*m_pPrimaryWriteCache);
    else
        aCache.takeOver(m_aPrimaryReadCache);

    AcceleratorCache::TKeyList lKeys;
    AcceleratorCache::TKeyList::const_iterator pIt;
    if (m_pSecondaryWriteCache!=0)
    {
        lKeys = m_pSecondaryWriteCache->getAllKeys();
        for ( pIt=lKeys.begin(); pIt!=lKeys.end(); ++pIt )
            aCache.setKeyCommandPair(*pIt, m_pSecondaryWriteCache->getCommandByKey(*pIt));
    }
    else
    {
        lKeys = m_aSecondaryReadCache.getAllKeys();
        for ( pIt=lKeys.begin(); pIt!=lKeys.end(); ++pIt )
            aCache.setKeyCommandPair(*pIt, m_aSecondaryReadCache.getCommandByKey(*pIt));
    }

    aWriteLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::io::XTruncate > xClearable(xOut, css::uno::UNO_QUERY_THROW);
    xClearable->truncate();
    css::uno::Reference< css::io::XSeekable > xSeek(xOut, css::uno::UNO_QUERY);
    if (xSeek.is())
        xSeek->seek(0);

    css::uno::Reference< css::xml::sax::XWriter > xWriter = css::xml::sax::Writer::create(m_xContext);
    xWriter->setOutputStream(xOut);

    // write into the stream
    css::uno::Reference< css::xml::sax::XDocumentHandler > xHandler(xWriter, css::uno::UNO_QUERY_THROW);
    AcceleratorConfigurationWriter aWriter(aCache, xHandler);
    aWriter.flush();
}

//-----------------------------------------------
::sal_Bool SAL_CALL XCUBasedAcceleratorConfiguration::isModified()
    throw(css::uno::RuntimeException)
{
    return sal_False;
}

//-----------------------------------------------
::sal_Bool SAL_CALL XCUBasedAcceleratorConfiguration::isReadOnly()
    throw(css::uno::RuntimeException)
{
    return sal_False;
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::setStorage(const css::uno::Reference< css::embed::XStorage >& /*xStorage*/)
    throw(css::uno::RuntimeException)
{
    SAL_INFO("fwk", "XCUBasedAcceleratorConfiguration::setStorage(): implement this HACK .-)");
}

//-----------------------------------------------
::sal_Bool SAL_CALL XCUBasedAcceleratorConfiguration::hasStorage()
    throw(css::uno::RuntimeException)
{
    SAL_INFO("fwk", "XCUBasedAcceleratorConfiguration::hasStorage(): implement this HACK .-)");
        return sal_False;
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::addConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& /*xListener*/)
    throw(css::uno::RuntimeException)
{
    SAL_INFO("fwk", "XCUBasedAcceleratorConfiguration::addConfigurationListener(): implement me");
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::removeConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& /*xListener*/)
    throw(css::uno::RuntimeException)
{
    SAL_INFO("fwk", "XCUBasedAcceleratorConfiguration::removeConfigurationListener(): implement me");
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::reset()
    throw(css::uno::RuntimeException)
{
    css::uno::Reference< css::container::XNamed > xNamed(m_xCfg, css::uno::UNO_QUERY);
    OUString sConfig = xNamed->getName();
    if ( sConfig == "Global" )
    {
        m_xCfg = css::uno::Reference< css::container::XNameAccess > (
            ::comphelper::ConfigurationHelper::openConfig( m_xContext, CFG_ENTRY_GLOBAL, ::comphelper::ConfigurationHelper::E_ALL_LOCALES ),
            css::uno::UNO_QUERY );
        XCUBasedAcceleratorConfiguration::reload();
    }
    else if ( sConfig == "Modules" )
    {
        m_xCfg = css::uno::Reference< css::container::XNameAccess > (
            ::comphelper::ConfigurationHelper::openConfig( m_xContext, CFG_ENTRY_MODULES, ::comphelper::ConfigurationHelper::E_ALL_LOCALES ),
            css::uno::UNO_QUERY );
        XCUBasedAcceleratorConfiguration::reload();
    }
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::addResetListener(const css::uno::Reference< css::form::XResetListener >& /*xListener*/)
    throw(css::uno::RuntimeException)
{
    SAL_INFO("fwk", "XCUBasedAcceleratorConfiguration::addResetListener(): implement me");
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::removeResetListener(const css::uno::Reference< css::form::XResetListener >& /*xListener*/)
    throw(css::uno::RuntimeException)
{
    SAL_INFO("fwk", "XCUBasedAcceleratorConfiguration::removeResetListener(): implement me");
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::changesOccurred(const css::util::ChangesEvent& aEvent)
    throw(css::uno::RuntimeException)
{
    SAL_INFO( "fwk.accelerators", "XCUBasedAcceleratorConfiguration::changesOccurred()" );

    css::uno::Reference< css::container::XHierarchicalNameAccess > xHAccess;
    aEvent.Base >>= xHAccess;
    if (! xHAccess.is ())
        return;

    css::util::ChangesEvent aReceivedEvents( aEvent );
    const sal_Int32 c = aReceivedEvents.Changes.getLength();
          sal_Int32 i = 0;
    for (i=0; i<c; ++i)
    {
        const css::util::ElementChange& aChange  =   aReceivedEvents.Changes[i];

        // Only path of form "PrimaryKeys/Modules/Module['<module_name>']/Key['<command_url>']/Command[<locale>]" will
        // be interesting for use. Sometimes short path values are given also by the broadcaster ... but they must be ignored :-)
        // So we try to split the path into 3 parts (module isnt important here, because we already know it ... because
        // these instance is bound to a specific module configuration ... or it''s the global configuration where no module is given at all.

        OUString sOrgPath ;
        OUString sPath    ;
        OUString sKey;

        aChange.Accessor >>= sOrgPath;
        sPath              = sOrgPath;
        OUString sPrimarySecondary = ::utl::extractFirstFromConfigurationPath(sPath, &sPath);
        OUString sGlobalModules = ::utl::extractFirstFromConfigurationPath(sPath, &sPath);

        if ( sGlobalModules == CFG_ENTRY_GLOBAL )
        {
            OUString sModule;
            sKey = ::utl::extractFirstFromConfigurationPath(sPath, &sPath);
            if ( !sKey.isEmpty() && !sPath.isEmpty() )
                reloadChanged(sPrimarySecondary, sGlobalModules, sModule, sKey);
        }
        else if ( sGlobalModules == CFG_ENTRY_MODULES )
        {
            OUString sModule = ::utl::extractFirstFromConfigurationPath(sPath, &sPath);
            sKey = ::utl::extractFirstFromConfigurationPath(sPath, &sPath);

            if ( !sKey.isEmpty() && !sPath.isEmpty() )
            {
                reloadChanged(sPrimarySecondary, sGlobalModules, sModule, sKey);
            }
        }
    }
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::disposing(const css::lang::EventObject& /*aSource*/)
    throw(css::uno::RuntimeException)
{
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::dispose()
    throw(css::uno::RuntimeException)
{
    // nop
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& /*xListener*/ )
    throw(css::uno::RuntimeException)
{
    // nop
}

//-----------------------------------------------
void SAL_CALL XCUBasedAcceleratorConfiguration::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& /*xListener*/ )
    throw(css::uno::RuntimeException)
{
    // nop
}

//-----------------------------------------------
void XCUBasedAcceleratorConfiguration::impl_ts_load( sal_Bool bPreferred, const css::uno::Reference< css::container::XNameAccess >& xCfg )
{
    AcceleratorCache aReadCache = AcceleratorCache();
    css::uno::Reference< css::container::XNameAccess > xAccess;
    if ( m_sGlobalOrModules == "Global" )
        xCfg->getByName(CFG_ENTRY_GLOBAL) >>= xAccess;
    else if ( m_sGlobalOrModules == "Modules" )
    {
        css::uno::Reference< css::container::XNameAccess > xModules;
        xCfg->getByName(CFG_ENTRY_MODULES) >>= xModules;
        xModules->getByName(m_sModuleCFG) >>= xAccess;
    }

    const OUString sIsoLang       = impl_ts_getLocale();
    const OUString sDefaultLocale("en-US");

    css::uno::Reference< css::container::XNameAccess > xKey;
    css::uno::Reference< css::container::XNameAccess > xCommand;
    if (xAccess.is())
    {
        css::uno::Sequence< OUString > lKeys = xAccess->getElementNames();
        sal_Int32 nKeys = lKeys.getLength();
        for ( sal_Int32 i=0; i<nKeys; ++i )
        {
            OUString sKey = lKeys[i];
            xAccess->getByName(sKey) >>= xKey;
            xKey->getByName(CFG_PROP_COMMAND) >>= xCommand;

            css::uno::Sequence< OUString > lLocales = xCommand->getElementNames();
            sal_Int32 nLocales = lLocales.getLength();
            ::std::vector< OUString > aLocales;
            for ( sal_Int32 j=0; j<nLocales; ++j )
                aLocales.push_back(lLocales[j]);

            ::std::vector< OUString >::const_iterator pFound;
            for ( pFound = aLocales.begin(); pFound != aLocales.end(); ++pFound )
            {
                if ( *pFound == sIsoLang )
                    break;
            }

            if ( pFound == aLocales.end() )
            {
                for ( pFound = aLocales.begin(); pFound != aLocales.end(); ++pFound )
                {
                    if ( *pFound == sDefaultLocale )
                        break;
                }

                if ( pFound == aLocales.end() )
                    continue;
            }

            OUString sLocale = *pFound;
            OUString sCommand;
            xCommand->getByName(sLocale) >>= sCommand;
            if (sCommand.isEmpty())
                continue;

            css::awt::KeyEvent aKeyEvent;

            sal_Int32 nIndex = 0;
            OUString sKeyCommand = sKey.getToken(0, '_', nIndex);
            OUString sPrefix("KEY_");
            aKeyEvent.KeyCode = m_rKeyMapping->mapIdentifierToCode(sPrefix + sKeyCommand);

            css::uno::Sequence< OUString > sToken(4);
            const sal_Int32 nToken = 4;
            sal_Bool bValid = sal_True;
            sal_Int32 k;
            for (k=0; k<nToken; ++k)
            {
                if (nIndex < 0)
                    break;

                sToken[k] = sKey.getToken(0, '_', nIndex);
                if (sToken[k].isEmpty())
                {
                    bValid = sal_False;
                    break;
                }

                if ( sToken[k] == "SHIFT" )
                    aKeyEvent.Modifiers |= css::awt::KeyModifier::SHIFT;
                else if ( sToken[k] == "MOD1" )
                    aKeyEvent.Modifiers |= css::awt::KeyModifier::MOD1;
                else if ( sToken[k] == "MOD2" )
                    aKeyEvent.Modifiers |= css::awt::KeyModifier::MOD2;
                else if ( sToken[k] == "MOD3" )
                    aKeyEvent.Modifiers |= css::awt::KeyModifier::MOD3;
                else
                {
                    bValid = sal_False;
                    break;
                }
            }

            if ( !aReadCache.hasKey(aKeyEvent) && bValid && k<nToken)
                aReadCache.setKeyCommandPair(aKeyEvent, sCommand);
        }
    }

    if (bPreferred)
        m_aPrimaryReadCache.takeOver(aReadCache);
    else
        m_aSecondaryReadCache.takeOver(aReadCache);
}

//-----------------------------------------------
void XCUBasedAcceleratorConfiguration::impl_ts_save(sal_Bool bPreferred, const css::uno::Reference< css::container::XNameAccess >& /*xCfg*/)
{
    if (bPreferred)
    {
        AcceleratorCache::TKeyList::const_iterator pIt;
        AcceleratorCache::TKeyList lPrimaryReadKeys  = m_aPrimaryReadCache.getAllKeys();
        AcceleratorCache::TKeyList lPrimaryWriteKeys = m_pPrimaryWriteCache->getAllKeys();

        for ( pIt  = lPrimaryReadKeys.begin(); pIt != lPrimaryReadKeys.end(); ++pIt )
        {
            if (!m_pPrimaryWriteCache->hasKey(*pIt))
                removeKeyFromConfiguration(*pIt, sal_True);
        }

        for ( pIt  = lPrimaryWriteKeys.begin(); pIt != lPrimaryWriteKeys.end(); ++pIt )
        {
            OUString sCommand = m_pPrimaryWriteCache->getCommandByKey(*pIt);
            if (!m_aPrimaryReadCache.hasKey(*pIt))
            {
                insertKeyToConfiguration(*pIt, sCommand, sal_True);
            }
            else
            {
                OUString sReadCommand = m_aPrimaryReadCache.getCommandByKey(*pIt);
                if (sReadCommand != sCommand)
                    insertKeyToConfiguration(*pIt, sCommand, sal_True);
            }
        }

        // take over all changes into the original container
        // SAFE -> ----------------------------------
        WriteGuard aWriteLock(m_aLock);

        if (m_pPrimaryWriteCache)
        {
            m_aPrimaryReadCache.takeOver(*m_pPrimaryWriteCache);
            AcceleratorCache* pTemp = m_pPrimaryWriteCache;
            m_pPrimaryWriteCache = 0;
            delete pTemp;
        }

        aWriteLock.unlock();
        // <- SAFE ----------------------------------
    }

    else
    {
        AcceleratorCache::TKeyList::const_iterator pIt;
        AcceleratorCache::TKeyList lSecondaryReadKeys  = m_aSecondaryReadCache.getAllKeys();
        AcceleratorCache::TKeyList lSecondaryWriteKeys = m_pSecondaryWriteCache->getAllKeys();

        for ( pIt  = lSecondaryReadKeys.begin(); pIt != lSecondaryReadKeys.end(); ++pIt)
        {
            if (!m_pSecondaryWriteCache->hasKey(*pIt))
                removeKeyFromConfiguration(*pIt, sal_False);
        }


        for ( pIt  = lSecondaryWriteKeys.begin(); pIt != lSecondaryWriteKeys.end(); ++pIt )
        {
            OUString sCommand = m_pSecondaryWriteCache->getCommandByKey(*pIt);
            if (!m_aSecondaryReadCache.hasKey(*pIt))
            {
                insertKeyToConfiguration(*pIt, sCommand, sal_False);
            }
            else
            {
                OUString sReadCommand = m_aSecondaryReadCache.getCommandByKey(*pIt);
                if (sReadCommand != sCommand)
                    insertKeyToConfiguration(*pIt, sCommand, sal_False);
            }
        }

        // take over all changes into the original container
        // SAFE -> ----------------------------------
        WriteGuard aWriteLock(m_aLock);

        if (m_pSecondaryWriteCache)
        {
            m_aSecondaryReadCache.takeOver(*m_pSecondaryWriteCache);
            AcceleratorCache* pTemp = m_pSecondaryWriteCache;
            m_pSecondaryWriteCache = 0;
            delete pTemp;
        }

        aWriteLock.unlock();
        // <- SAFE ----------------------------------
    }

    ::comphelper::ConfigurationHelper::flush(m_xCfg);
}

//-----------------------------------------------
void XCUBasedAcceleratorConfiguration::insertKeyToConfiguration( const css::awt::KeyEvent& aKeyEvent, const OUString& sCommand, const sal_Bool bPreferred )
{
    css::uno::Reference< css::container::XNameAccess > xAccess;
    css::uno::Reference< css::container::XNameContainer > xContainer;
    css::uno::Reference< css::lang::XSingleServiceFactory > xFac;
    css::uno::Reference< css::uno::XInterface > xInst;

    if ( bPreferred )
        m_xCfg->getByName(CFG_ENTRY_PRIMARY) >>= xAccess;
    else
        m_xCfg->getByName(CFG_ENTRY_SECONDARY) >>= xAccess;

    if ( m_sGlobalOrModules == CFG_ENTRY_GLOBAL )
        xAccess->getByName(CFG_ENTRY_GLOBAL) >>= xContainer;
    else if ( m_sGlobalOrModules == CFG_ENTRY_MODULES )
    {
        css::uno::Reference< css::container::XNameContainer > xModules;
        xAccess->getByName(CFG_ENTRY_MODULES) >>= xModules;
        if ( !xModules->hasByName(m_sModuleCFG) )
        {
            xFac = css::uno::Reference< css::lang::XSingleServiceFactory >(xModules, css::uno::UNO_QUERY);
            xInst = xFac->createInstance();
            xModules->insertByName(m_sModuleCFG, css::uno::makeAny(xInst));
        }
        xModules->getByName(m_sModuleCFG) >>= xContainer;
    }

    const OUString sKey = lcl_getKeyString(m_rKeyMapping,aKeyEvent);
    css::uno::Reference< css::container::XNameAccess > xKey;
    css::uno::Reference< css::container::XNameContainer > xCommand;
    if ( !xContainer->hasByName(sKey) )
    {
        xFac = css::uno::Reference< css::lang::XSingleServiceFactory >(xContainer, css::uno::UNO_QUERY);
        xInst = xFac->createInstance();
        xContainer->insertByName(sKey, css::uno::makeAny(xInst));
    }
    xContainer->getByName(sKey) >>= xKey;

    xKey->getByName(CFG_PROP_COMMAND) >>= xCommand;
    OUString sLocale = impl_ts_getLocale();
    if ( !xCommand->hasByName(sLocale) )
        xCommand->insertByName(sLocale, css::uno::makeAny(sCommand));
    else
        xCommand->replaceByName(sLocale, css::uno::makeAny(sCommand));
}

//-----------------------------------------------
void XCUBasedAcceleratorConfiguration::removeKeyFromConfiguration( const css::awt::KeyEvent& aKeyEvent, const sal_Bool bPreferred )
{
    css::uno::Reference< css::container::XNameAccess > xAccess;
    css::uno::Reference< css::container::XNameContainer > xContainer;

    if ( bPreferred )
        m_xCfg->getByName(CFG_ENTRY_PRIMARY) >>= xAccess;
    else
        m_xCfg->getByName(CFG_ENTRY_SECONDARY) >>= xAccess;

    if ( m_sGlobalOrModules == CFG_ENTRY_GLOBAL )
        xAccess->getByName(CFG_ENTRY_GLOBAL) >>= xContainer;
    else if ( m_sGlobalOrModules == CFG_ENTRY_MODULES )
    {
        css::uno::Reference< css::container::XNameAccess > xModules;
        xAccess->getByName(CFG_ENTRY_MODULES) >>= xModules;
        if ( !xModules->hasByName(m_sModuleCFG) )
            return;
        xModules->getByName(m_sModuleCFG) >>= xContainer;
    }

    const OUString sKey = lcl_getKeyString(m_rKeyMapping,aKeyEvent);
    xContainer->removeByName(sKey);
}

//-----------------------------------------------
void XCUBasedAcceleratorConfiguration::reloadChanged( const OUString& sPrimarySecondary, const OUString& sGlobalModules, const OUString& sModule, const OUString& sKey )
{
    css::uno::Reference< css::container::XNameAccess > xAccess;
    css::uno::Reference< css::container::XNameContainer > xContainer;

    m_xCfg->getByName(sPrimarySecondary) >>= xAccess;
    if ( sGlobalModules == CFG_ENTRY_GLOBAL )
        xAccess->getByName(CFG_ENTRY_GLOBAL) >>= xContainer;
    else
    {
        css::uno::Reference< css::container::XNameAccess > xModules;
        xAccess->getByName(CFG_ENTRY_MODULES) >>= xModules;
        if ( !xModules->hasByName(sModule) )
            return;
        xModules->getByName(sModule) >>= xContainer;
    }

    css::awt::KeyEvent aKeyEvent;
    OUString sKeyIdentifier;

    sal_Int32 nIndex = 0;
    sKeyIdentifier = sKey.getToken(0, '_', nIndex);
    aKeyEvent.KeyCode = m_rKeyMapping->mapIdentifierToCode(OUString("KEY_")+sKeyIdentifier);

    css::uno::Sequence< OUString > sToken(3);
    const sal_Int32 nToken = 3;
    for (sal_Int32 i=0; i<nToken; ++i)
    {
        if ( nIndex < 0 )
            break;

        sToken[i] = sKey.getToken(0, '_', nIndex);
        if ( sToken[i] == "SHIFT" )
            aKeyEvent.Modifiers |= css::awt::KeyModifier::SHIFT;
        else if ( sToken[i] == "MOD1" )
            aKeyEvent.Modifiers |= css::awt::KeyModifier::MOD1;
        else if ( sToken[i] == "MOD2" )
            aKeyEvent.Modifiers |= css::awt::KeyModifier::MOD2;
                else if ( sToken[i] == "MOD3" )
                        aKeyEvent.Modifiers |= css::awt::KeyModifier::MOD3;
    }

    css::uno::Reference< css::container::XNameAccess > xKey;
    css::uno::Reference< css::container::XNameAccess > xCommand;
    OUString sCommand;

    if (xContainer->hasByName(sKey))
    {
        OUString sLocale = impl_ts_getLocale();
        xContainer->getByName(sKey)    >>= xKey;
        xKey->getByName(CFG_PROP_COMMAND)  >>= xCommand;
        xCommand->getByName(sLocale)       >>= sCommand;
    }

    if ( sPrimarySecondary == CFG_ENTRY_PRIMARY )
    {
        if (sCommand.isEmpty())
            m_aPrimaryReadCache.removeKey(aKeyEvent);
        else
            m_aPrimaryReadCache.setKeyCommandPair(aKeyEvent, sCommand);
    }
    else if ( sPrimarySecondary == CFG_ENTRY_SECONDARY )
    {
        if (sCommand.isEmpty())
            m_aSecondaryReadCache.removeKey(aKeyEvent);
        else
            m_aSecondaryReadCache.setKeyCommandPair(aKeyEvent, sCommand);
    }
}

//-----------------------------------------------
AcceleratorCache& XCUBasedAcceleratorConfiguration::impl_getCFG(sal_Bool bPreferred, sal_Bool bWriteAccessRequested)
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    if (bPreferred)
    {
        //create copy of our readonly-cache, if write access is forced ... but
        //not still possible!
        if (
            (bWriteAccessRequested) &&
            (!m_pPrimaryWriteCache       )
            )
        {
            m_pPrimaryWriteCache = new AcceleratorCache(m_aPrimaryReadCache);
        }

        // in case, we have a writeable cache, we use it for reading too!
        // Otherwhise the API user cant find its own changes ...
        if (m_pPrimaryWriteCache)
            return *m_pPrimaryWriteCache;
        else
            return m_aPrimaryReadCache;
    }

    else
    {
        //create copy of our readonly-cache, if write access is forced ... but
        //not still possible!
        if (
            (bWriteAccessRequested) &&
            (!m_pSecondaryWriteCache       )
            )
        {
            m_pSecondaryWriteCache = new AcceleratorCache(m_aSecondaryReadCache);
        }

        // in case, we have a writeable cache, we use it for reading too!
        // Otherwhise the API user cant find its own changes ...
        if (m_pSecondaryWriteCache)
            return *m_pSecondaryWriteCache;
        else
            return m_aSecondaryReadCache;
    }

    // <- SAFE ----------------------------------
}

//-----------------------------------------------
OUString XCUBasedAcceleratorConfiguration::impl_ts_getLocale() const
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aReadLock.unlock();
    // <- SAFE ----------------------------------

    css::uno::Reference< css::uno::XInterface >     xCFG      = fpc::ConfigurationHelper::openConfig( xContext,
        "/org.openoffice.Setup", "L10N", fpc::ConfigurationHelper::E_READONLY);
    css::uno::Reference< css::beans::XPropertySet > xProp     (xCFG, css::uno::UNO_QUERY_THROW);
    OUString                                 sISOLocale;
    xProp->getPropertyValue("ooLocale") >>= sISOLocale;

    if (sISOLocale.isEmpty())
        return OUString("en-US");
    return sISOLocale;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
