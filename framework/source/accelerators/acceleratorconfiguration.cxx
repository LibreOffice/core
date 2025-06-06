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

#include <sal/config.h>

#include <utility>

#include <accelerators/acceleratorconfiguration.hxx>
#include <accelerators/keymapping.hxx>
#include <accelerators/presethandler.hxx>

#include <xml/saxnamespacefilter.hxx>
#include <xml/acceleratorconfigurationreader.hxx>
#include <xml/acceleratorconfigurationwriter.hxx>

#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>

#include <vcl/svapp.hxx>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/sequence.hxx>
#include <officecfg/Setup.hxx>
#include <unotools/configpaths.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <sal/log.hxx>
#include <rtl/ustrbuf.hxx>
#include <o3tl/string_view.hxx>

constexpr OUString PRESET_DEFAULT = u"default"_ustr;
constexpr OUString TARGET_CURRENT = u"current"_ustr;

namespace framework
{
    constexpr OUString CFG_ENTRY_SECONDARY = u"SecondaryKeys"_ustr;
    constexpr OUString CFG_PROP_COMMAND = u"Command"_ustr;

    static OUString lcl_getKeyString(const css::awt::KeyEvent& aKeyEvent)
    {
        const sal_Int32 nBeginIndex = 4; // "KEY_" is the prefix of an identifier...
        OUString sKey(KeyMapping::get().mapCodeToIdentifier(aKeyEvent.KeyCode));
        if (sKey.getLength() < nBeginIndex) // dead key
            return OUString();
        OUStringBuffer sKeyBuffer(sKey.subView(nBeginIndex));

        if ( (aKeyEvent.Modifiers & css::awt::KeyModifier::SHIFT) == css::awt::KeyModifier::SHIFT )
            sKeyBuffer.append("_SHIFT");
        if ( (aKeyEvent.Modifiers & css::awt::KeyModifier::MOD1 ) == css::awt::KeyModifier::MOD1  )
            sKeyBuffer.append("_MOD1");
        if ( (aKeyEvent.Modifiers & css::awt::KeyModifier::MOD2 ) == css::awt::KeyModifier::MOD2  )
            sKeyBuffer.append("_MOD2");
        if ( (aKeyEvent.Modifiers & css::awt::KeyModifier::MOD3 ) == css::awt::KeyModifier::MOD3  )
            sKeyBuffer.append("_MOD3");

        return sKeyBuffer.makeStringAndClear();
    }

XMLBasedAcceleratorConfiguration::XMLBasedAcceleratorConfiguration(const css::uno::Reference< css::uno::XComponentContext >& xContext)
    : m_xContext      (xContext                     )
    , m_aPresetHandler(xContext                     )
{
}

XMLBasedAcceleratorConfiguration::~XMLBasedAcceleratorConfiguration()
{
    SAL_WARN_IF(m_pWriteCache, "fwk.accelerators", "XMLBasedAcceleratorConfiguration::~XMLBasedAcceleratorConfiguration(): Changes not flushed. Ignore it ...");
}

css::uno::Sequence< css::awt::KeyEvent > SAL_CALL XMLBasedAcceleratorConfiguration::getAllKeyEvents()
{
    SolarMutexGuard g;
    AcceleratorCache&          rCache = impl_getCFG();
    AcceleratorCache::TKeyList lKeys  = rCache.getAllKeys();
    return comphelper::containerToSequence(lKeys);
}

OUString SAL_CALL XMLBasedAcceleratorConfiguration::getCommandByKeyEvent(const css::awt::KeyEvent& aKeyEvent)
{
    SolarMutexGuard g;
    AcceleratorCache& rCache = impl_getCFG();
    if (!rCache.hasKey(aKeyEvent))
        throw css::container::NoSuchElementException(
                OUString(),
                static_cast< ::cppu::OWeakObject* >(this));
    return rCache.getCommandByKey(aKeyEvent);
}

void SAL_CALL XMLBasedAcceleratorConfiguration::setKeyEvent(const css::awt::KeyEvent& aKeyEvent,
                                                    const OUString&    sCommand )
{
    if (
        (aKeyEvent.KeyCode   == 0) &&
        (aKeyEvent.KeyChar   == 0) &&
        (aKeyEvent.KeyFunc   == 0) &&
        (aKeyEvent.Modifiers == 0)
        )
        throw css::lang::IllegalArgumentException(
            u"Such key event seems not to be supported by any operating system."_ustr,
            static_cast< ::cppu::OWeakObject* >(this),
            0);

    if (sCommand.isEmpty())
        throw css::lang::IllegalArgumentException(
            u"Empty command strings are not allowed here."_ustr,
            static_cast< ::cppu::OWeakObject* >(this),
            1);

    SolarMutexGuard g;
    AcceleratorCache& rCache = impl_getCFG(true); // sal_True => force getting of a writeable cache!
    rCache.setKeyCommandPair(aKeyEvent, sCommand);
}

void SAL_CALL XMLBasedAcceleratorConfiguration::removeKeyEvent(const css::awt::KeyEvent& aKeyEvent)
{
    SolarMutexGuard g;
    AcceleratorCache& rCache = impl_getCFG(true); // true => force using of a writeable cache
    if (!rCache.hasKey(aKeyEvent))
        throw css::container::NoSuchElementException(
        OUString(),
        static_cast< ::cppu::OWeakObject* >(this));
    rCache.removeKey(aKeyEvent);
}

css::uno::Sequence< css::awt::KeyEvent > SAL_CALL XMLBasedAcceleratorConfiguration::getKeyEventsByCommand(const OUString& sCommand)
{
    if (sCommand.isEmpty())
        throw css::lang::IllegalArgumentException(
                u"Empty command strings are not allowed here."_ustr,
                static_cast< ::cppu::OWeakObject* >(this),
                1);

    SolarMutexGuard g;
    AcceleratorCache& rCache = impl_getCFG();
    if (!rCache.hasCommand(sCommand))
        throw css::container::NoSuchElementException(
                OUString(),
                static_cast< ::cppu::OWeakObject* >(this));

    AcceleratorCache::TKeyList lKeys  = rCache.getKeysByCommand(sCommand);
    return comphelper::containerToSequence(lKeys);
}

css::uno::Sequence< css::uno::Any > SAL_CALL XMLBasedAcceleratorConfiguration::getPreferredKeyEventsForCommandList(const css::uno::Sequence< OUString >& lCommandList)
{
    SolarMutexGuard g;

    sal_Int32                           i              = 0;
    sal_Int32                           c              = lCommandList.getLength();
    css::uno::Sequence< css::uno::Any > lPreferredOnes (c); // don't pack list!
    AcceleratorCache&                   rCache         = impl_getCFG();

    auto lPreferredOnesRange = asNonConstRange(lPreferredOnes);
    for (i=0; i<c; ++i)
    {
        const OUString& rCommand = lCommandList[i];
        if (rCommand.isEmpty())
            throw css::lang::IllegalArgumentException(
                    u"Empty command strings are not allowed here."_ustr,
                    static_cast< ::cppu::OWeakObject* >(this),
                    static_cast<sal_Int16>(i));

        if (!rCache.hasCommand(rCommand))
            continue;

        AcceleratorCache::TKeyList lKeys = rCache.getKeysByCommand(rCommand);
        if ( lKeys.empty() )
            continue;

        css::uno::Any& rAny = lPreferredOnesRange[i];
        rAny <<= *(lKeys.begin());
    }

    return lPreferredOnes;
}

void SAL_CALL XMLBasedAcceleratorConfiguration::removeCommandFromAllKeyEvents(const OUString& sCommand)
{
    if (sCommand.isEmpty())
        throw css::lang::IllegalArgumentException(
                u"Empty command strings are not allowed here."_ustr,
                static_cast< ::cppu::OWeakObject* >(this),
                0);

    SolarMutexGuard g;
    AcceleratorCache& rCache = impl_getCFG(true); // sal_True => force getting of a writeable cache!
    if (!rCache.hasCommand(sCommand))
        throw css::container::NoSuchElementException(
                u"Command does not exists inside this container."_ustr,
                static_cast< ::cppu::OWeakObject* >(this));
    rCache.removeCommand(sCommand);
}

void SAL_CALL XMLBasedAcceleratorConfiguration::reload()
{
    css::uno::Reference< css::io::XStream > xStream;
    css::uno::Reference< css::io::XStream > xStreamNoLang;
    {
        SolarMutexGuard g;
        xStream = m_aPresetHandler.openTarget(TARGET_CURRENT,
                css::embed::ElementModes::READ);
        try
        {
            xStreamNoLang = m_aPresetHandler.openPreset(PRESET_DEFAULT);
        }
        catch(const css::io::IOException&) {} // does not have to exist
    }

    css::uno::Reference< css::io::XInputStream > xIn;
    if (xStream.is())
        xIn = xStream->getInputStream();
    if (!xIn.is())
        throw css::io::IOException(
            u"Could not open accelerator configuration for reading."_ustr,
            static_cast< ::cppu::OWeakObject* >(this));

    // impl_ts_load() does not clear the cache
    {
        SolarMutexGuard g;
        m_aReadCache = AcceleratorCache();
    }

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

void SAL_CALL XMLBasedAcceleratorConfiguration::store()
{
    css::uno::Reference< css::io::XStream > xStream;
    {
        SolarMutexGuard g;
        xStream = m_aPresetHandler.openTarget(TARGET_CURRENT,
               css::embed::ElementModes::READWRITE); // open or create!
    }

    css::uno::Reference< css::io::XOutputStream > xOut;
    if (xStream.is())
        xOut = xStream->getOutputStream();

    if (!xOut.is())
        throw css::io::IOException(
            u"Could not open accelerator configuration for saving."_ustr,
            static_cast< ::cppu::OWeakObject* >(this));

    impl_ts_save(xOut);

    xOut.clear();
    xStream.clear();

    m_aPresetHandler.commitUserChanges();
}

void SAL_CALL XMLBasedAcceleratorConfiguration::storeToStorage(const css::uno::Reference< css::embed::XStorage >& xStorage)
{
    // no fallback from read/write to readonly!
    css::uno::Reference< css::io::XStream > xStream = xStorage->openStreamElement(TARGET_CURRENT, css::embed::ElementModes::READWRITE);

    css::uno::Reference< css::io::XOutputStream > xOut;
    if (xStream.is())
        xOut = xStream->getOutputStream();

    if (!xOut.is())
        throw css::io::IOException(
                u"Could not open accelerator configuration for saving."_ustr,
                static_cast< ::cppu::OWeakObject* >(this));

    impl_ts_save(xOut);

    // TODO inform listener about success, so it can flush the root and sub storage of this stream!
}

sal_Bool SAL_CALL XMLBasedAcceleratorConfiguration::isModified()
{
    SolarMutexGuard g;
    return (m_pWriteCache != nullptr);
}

sal_Bool SAL_CALL XMLBasedAcceleratorConfiguration::isReadOnly()
{
    return m_aPresetHandler.isReadOnly();
}

void SAL_CALL XMLBasedAcceleratorConfiguration::setStorage(const css::uno::Reference< css::embed::XStorage >& /*xStorage*/)
{
    SAL_INFO("fwk.accelerators", "XMLBasedAcceleratorConfiguration::setStorage(): implement this HACK .-)");
}

sal_Bool SAL_CALL XMLBasedAcceleratorConfiguration::hasStorage()
{
    SAL_INFO("fwk.accelerators", "XMLBasedAcceleratorConfiguration::hasStorage(): implement this HACK .-)");
    return false;
}

void SAL_CALL XMLBasedAcceleratorConfiguration::addConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& /*xListener*/)
{
    SAL_INFO("fwk.accelerators", "XMLBasedAcceleratorConfiguration::addConfigurationListener(): implement me");
}

void SAL_CALL XMLBasedAcceleratorConfiguration::removeConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& /*xListener*/)
{
    SAL_INFO("fwk.accelerators", "XMLBasedAcceleratorConfiguration::removeConfigurationListener(): implement me");
}

void SAL_CALL XMLBasedAcceleratorConfiguration::reset()
{
    {
        SolarMutexGuard g;
        m_aPresetHandler.copyPresetToTarget(PRESET_DEFAULT, TARGET_CURRENT);
    }

    reload();
}

void SAL_CALL XMLBasedAcceleratorConfiguration::addResetListener(const css::uno::Reference< css::form::XResetListener >& /*xListener*/)
{
    SAL_INFO("fwk.accelerators", "XMLBasedAcceleratorConfiguration::addResetListener(): implement me");
}

void SAL_CALL XMLBasedAcceleratorConfiguration::removeResetListener(const css::uno::Reference< css::form::XResetListener >& /*xListener*/)
{
    SAL_INFO("fwk.accelerators", "XMLBasedAcceleratorConfiguration::removeResetListener(): implement me");
}

// IStorageListener
void XMLBasedAcceleratorConfiguration::changesOccurred()
{
    reload();
}

void XMLBasedAcceleratorConfiguration::impl_ts_load(const css::uno::Reference< css::io::XInputStream >& xStream)
{
    css::uno::Reference< css::uno::XComponentContext > xContext;
    {
        SolarMutexGuard g;
        xContext = m_xContext;
        m_pWriteCache.reset();
    }

    css::uno::Reference< css::io::XSeekable > xSeek(xStream, css::uno::UNO_QUERY);
    if (xSeek.is())
        xSeek->seek(0);

    SolarMutexGuard g;

    // create the parser queue
    // Note: Use special filter object between parser and reader
    // to get filtered xml with right namespaces ...
    // Use further a temp cache for reading!
    rtl::Reference<AcceleratorConfigurationReader> pReader = new AcceleratorConfigurationReader(m_aReadCache);
    rtl::Reference<SaxNamespaceFilter> pFilter = new SaxNamespaceFilter(pReader);

    // connect parser, filter and stream
    css::uno::Reference< css::xml::sax::XParser > xParser = css::xml::sax::Parser::create(xContext);
    xParser->setDocumentHandler(pFilter);

    css::xml::sax::InputSource aSource;
    aSource.aInputStream = xStream;

    // TODO think about error handling
    xParser->parseStream(aSource);
}

void XMLBasedAcceleratorConfiguration::impl_ts_save(const css::uno::Reference< css::io::XOutputStream >& xStream)
{
    bool bChanged;
    AcceleratorCache aCache;
    css::uno::Reference< css::uno::XComponentContext > xContext;
    {
        SolarMutexGuard g;
        bChanged = (m_pWriteCache != nullptr);
        if (bChanged)
            aCache = *m_pWriteCache;
        else
            aCache = m_aReadCache;
        xContext = m_xContext;
    }

    css::uno::Reference< css::io::XTruncate > xClearable(xStream, css::uno::UNO_QUERY_THROW);
    xClearable->truncate();

    // TODO can be removed if seek(0) is done by truncate() automatically!
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

    SolarMutexGuard g;
    // take over all changes into the readonly cache ...
    // and forget the copy-on-write copied cache
    if (bChanged)
    {
        m_aReadCache = *m_pWriteCache;
        m_pWriteCache.reset();
    }
}

AcceleratorCache& XMLBasedAcceleratorConfiguration::impl_getCFG(bool bWriteAccessRequested)
{
    SolarMutexGuard g;

    //create copy of our readonly-cache, if write access is forced ... but
    //not still possible!
    if ( bWriteAccessRequested && !m_pWriteCache )
    {
        m_pWriteCache.reset(new AcceleratorCache(m_aReadCache));
    }

    // in case, we have a writeable cache, we use it for reading too!
    // Otherwise the API user can't find its own changes...
    if (m_pWriteCache)
        return *m_pWriteCache;
    else
        return m_aReadCache;
}

// static
OUString XMLBasedAcceleratorConfiguration::impl_ts_getLocale()
{
    OUString sISOLocale = officecfg::Setup::L10N::ooLocale::get();

    if (sISOLocale.isEmpty())
        return u"en-US"_ustr;
    return sISOLocale;
}

/*******************************************************************************
*
* XCU based accelerator configuration
*
*******************************************************************************/

XCUBasedAcceleratorConfiguration::XCUBasedAcceleratorConfiguration(css::uno::Reference< css::uno::XComponentContext > xContext)
                                : m_xContext      (std::move(xContext                     ))
{
    m_xCfg.set(
             ::comphelper::ConfigurationHelper::openConfig( m_xContext, u"org.openoffice.Office.Accelerators"_ustr, ::comphelper::EConfigurationModes::AllLocales ),
             css::uno::UNO_QUERY );
}

XCUBasedAcceleratorConfiguration::~XCUBasedAcceleratorConfiguration()
{
}

css::uno::Sequence< css::awt::KeyEvent > SAL_CALL XCUBasedAcceleratorConfiguration::getAllKeyEvents()
{
    SolarMutexGuard g;

    AcceleratorCache::TKeyList lKeys  = impl_getCFG(true).getAllKeys(); //get keys from PrimaryKeys set

    AcceleratorCache::TKeyList lSecondaryKeys = impl_getCFG(false).getAllKeys(); //get keys from SecondaryKeys set
    lKeys.reserve(lKeys.size()+lSecondaryKeys.size());
    for (auto const& secondaryKey : lSecondaryKeys)
        lKeys.push_back(secondaryKey);

    return comphelper::containerToSequence(lKeys);
}

OUString SAL_CALL XCUBasedAcceleratorConfiguration::getCommandByKeyEvent(const css::awt::KeyEvent& aKeyEvent)
{
    SolarMutexGuard g;

    AcceleratorCache& rPrimaryCache   = impl_getCFG(true );
    AcceleratorCache& rSecondaryCache = impl_getCFG(false);

    if (!rPrimaryCache.hasKey(aKeyEvent) && !rSecondaryCache.hasKey(aKeyEvent))
        throw css::container::NoSuchElementException(
        OUString(),
        static_cast< ::cppu::OWeakObject* >(this));

    if (rPrimaryCache.hasKey(aKeyEvent))
        return rPrimaryCache.getCommandByKey(aKeyEvent);
    else
        return rSecondaryCache.getCommandByKey(aKeyEvent);
}

void SAL_CALL XCUBasedAcceleratorConfiguration::setKeyEvent(const css::awt::KeyEvent& aKeyEvent,
                                                    const OUString&    sCommand )
{
    SAL_INFO( "fwk.accelerators", "XCUBasedAcceleratorConfiguration::setKeyEvent" );

    if (
        (aKeyEvent.KeyCode   == 0) &&
        (aKeyEvent.KeyChar   == 0) &&
        (aKeyEvent.KeyFunc   == 0) &&
        (aKeyEvent.Modifiers == 0)
        )
        throw css::lang::IllegalArgumentException(
                u"Such key event seems not to be supported by any operating system."_ustr,
                static_cast< ::cppu::OWeakObject* >(this),
                0);

    if (sCommand.isEmpty())
                throw css::lang::IllegalArgumentException(
                    u"Empty command strings are not allowed here."_ustr,
                    static_cast< ::cppu::OWeakObject* >(this),
                    1);

    SolarMutexGuard g;

    AcceleratorCache& rPrimaryCache   = impl_getCFG(true, true ); // sal_True => force getting of a writeable cache!
    AcceleratorCache& rSecondaryCache = impl_getCFG(false, true); // sal_True => force getting of a writeable cache!

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
}

void SAL_CALL XCUBasedAcceleratorConfiguration::removeKeyEvent(const css::awt::KeyEvent& aKeyEvent)
{
    SolarMutexGuard g;

    AcceleratorCache& rPrimaryCache   = impl_getCFG(true, true );
    AcceleratorCache& rSecondaryCache = impl_getCFG(false, true);

    if (!rPrimaryCache.hasKey(aKeyEvent) && !rSecondaryCache.hasKey(aKeyEvent))
        throw css::container::NoSuchElementException(
        OUString(),
        static_cast< ::cppu::OWeakObject* >(this));

    if (rPrimaryCache.hasKey(aKeyEvent))
    {
        OUString sOriginalCommand = rPrimaryCache.getCommandByKey(aKeyEvent);
        if (!sOriginalCommand.isEmpty())
        {
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
}

css::uno::Sequence< css::awt::KeyEvent > SAL_CALL XCUBasedAcceleratorConfiguration::getKeyEventsByCommand(const OUString& sCommand)
{
    if (sCommand.isEmpty())
        throw css::lang::IllegalArgumentException(
                u"Empty command strings are not allowed here."_ustr,
                static_cast< ::cppu::OWeakObject* >(this),
                1);

    SolarMutexGuard g;

    AcceleratorCache& rPrimaryCache   = impl_getCFG(true );
    AcceleratorCache& rSecondaryCache = impl_getCFG(false);

    if (!rPrimaryCache.hasCommand(sCommand) && !rSecondaryCache.hasCommand(sCommand))
        throw css::container::NoSuchElementException(
                OUString(),
                static_cast< ::cppu::OWeakObject* >(this));

    AcceleratorCache::TKeyList lKeys  = rPrimaryCache.getKeysByCommand(sCommand);

    AcceleratorCache::TKeyList lSecondaryKeys = rSecondaryCache.getKeysByCommand(sCommand);
    for (auto const& secondaryKey : lSecondaryKeys)
        lKeys.push_back(secondaryKey);

    return comphelper::containerToSequence(lKeys);
}

static AcceleratorCache::TKeyList::const_iterator lcl_getPreferredKey(const AcceleratorCache::TKeyList& lKeys)
{
    return std::find_if(lKeys.begin(), lKeys.end(), [](const css::awt::KeyEvent& rAWTKey) {
        return !::svt::AcceleratorExecute::st_AWTKey2VCLKey(rAWTKey).GetName().isEmpty(); });
}

css::uno::Sequence< css::uno::Any > SAL_CALL XCUBasedAcceleratorConfiguration::getPreferredKeyEventsForCommandList(const css::uno::Sequence< OUString >& lCommandList)
{
    SolarMutexGuard g;

    sal_Int32                           i              = 0;
    sal_Int32                           c              = lCommandList.getLength();
    css::uno::Sequence< css::uno::Any > lPreferredOnes (c); // don't pack list!
    AcceleratorCache&                   rCache         = impl_getCFG(true);

    auto lPreferredOnesRange = asNonConstRange(lPreferredOnes);
    for (i=0; i<c; ++i)
    {
        const OUString& rCommand = lCommandList[i];
        if (rCommand.isEmpty())
            throw css::lang::IllegalArgumentException(
                    u"Empty command strings are not allowed here."_ustr,
                    static_cast< ::cppu::OWeakObject* >(this),
                    static_cast<sal_Int16>(i));

        if (!rCache.hasCommand(rCommand))
            continue;

        AcceleratorCache::TKeyList lKeys = rCache.getKeysByCommand(rCommand);
        if ( lKeys.empty() )
            continue;

        AcceleratorCache::TKeyList::const_iterator pPreferredKey = lcl_getPreferredKey(lKeys);
        if (pPreferredKey != lKeys.end ())
        {
            css::uno::Any& rAny = lPreferredOnesRange[i];
            rAny <<= *pPreferredKey;
        }
    }

    return lPreferredOnes;
}

void SAL_CALL XCUBasedAcceleratorConfiguration::removeCommandFromAllKeyEvents(const OUString& sCommand)
{
    if (sCommand.isEmpty())
        throw css::lang::IllegalArgumentException(
                u"Empty command strings are not allowed here."_ustr,
                static_cast< ::cppu::OWeakObject* >(this),
                0);

    SolarMutexGuard g;

    AcceleratorCache& rPrimaryCache   = impl_getCFG(true, true );
    AcceleratorCache& rSecondaryCache = impl_getCFG(false, true);

    if (!rPrimaryCache.hasCommand(sCommand) && !rSecondaryCache.hasCommand(sCommand))
        throw css::container::NoSuchElementException(
                u"Command does not exists inside this container."_ustr,
                static_cast< ::cppu::OWeakObject* >(this));

    if (rPrimaryCache.hasCommand(sCommand))
        rPrimaryCache.removeCommand(sCommand);
    if (rSecondaryCache.hasCommand(sCommand))
        rSecondaryCache.removeCommand(sCommand);
}

void SAL_CALL XCUBasedAcceleratorConfiguration::reload()
{
    SAL_INFO( "fwk.accelerators", "XCUBasedAcceleratorConfiguration::reload()" );

    SolarMutexGuard g;

    bool bPreferred;
    css::uno::Reference< css::container::XNameAccess > xAccess;

    bPreferred = true;
    m_aPrimaryReadCache = AcceleratorCache();
    m_pPrimaryWriteCache.reset();
    m_xCfg->getByName(CFG_ENTRY_PRIMARY) >>= xAccess;
    impl_ts_load(bPreferred, xAccess); // load the preferred keys

    bPreferred = false;
    m_aSecondaryReadCache = AcceleratorCache();
    m_pSecondaryWriteCache.reset();
    m_xCfg->getByName(CFG_ENTRY_SECONDARY) >>= xAccess;
    impl_ts_load(bPreferred, xAccess); // load the secondary keys
}

void SAL_CALL XCUBasedAcceleratorConfiguration::store()
{
    SAL_INFO( "fwk.accelerators", "XCUBasedAcceleratorConfiguration::store()" );

    SolarMutexGuard g;

    bool bPreferred;

    bPreferred = true;
    // on-demand creation of the primary write cache
    impl_getCFG(bPreferred, true);
    impl_ts_save(bPreferred);

    bPreferred = false;
    // on-demand creation of the secondary write cache
    impl_getCFG(bPreferred, true);
    impl_ts_save(bPreferred);
}

void SAL_CALL XCUBasedAcceleratorConfiguration::storeToStorage(const css::uno::Reference< css::embed::XStorage >& xStorage)
{
    // use m_aCache + old AcceleratorXMLWriter to store data directly on storage given as parameter ...
    if (!xStorage.is())
        return;

    tools::Long nOpenModes = css::embed::ElementModes::READWRITE;
    css::uno::Reference< css::embed::XStorage > xAcceleratorTypeStorage = xStorage->openStorageElement(u"accelerator"_ustr, nOpenModes);
    if (!xAcceleratorTypeStorage.is())
        return;

    css::uno::Reference< css::io::XStream > xStream = xAcceleratorTypeStorage->openStreamElement(u"current"_ustr, nOpenModes);
    css::uno::Reference< css::io::XOutputStream > xOut;
    if (xStream.is())
        xOut = xStream->getOutputStream();
    if (!xOut.is())
        throw css::io::IOException(
            u"Could not open accelerator configuration for saving."_ustr,
            static_cast< ::cppu::OWeakObject* >(this));

    // the original m_aCache has been split into primary cache and secondary cache...
    // we should merge them before storing to storage
    AcceleratorCache aCache;
    {
        SolarMutexGuard g;

        if (m_pPrimaryWriteCache != nullptr)
            aCache = *m_pPrimaryWriteCache;
        else
            aCache = m_aPrimaryReadCache;

        AcceleratorCache::TKeyList lKeys;
        if (m_pSecondaryWriteCache!=nullptr)
        {
            lKeys = m_pSecondaryWriteCache->getAllKeys();
            for (auto const& lKey : lKeys)
                aCache.setKeyCommandPair(lKey, m_pSecondaryWriteCache->getCommandByKey(lKey));
        }
        else
        {
            lKeys = m_aSecondaryReadCache.getAllKeys();
            for (auto const& lKey : lKeys)
                aCache.setKeyCommandPair(lKey, m_aSecondaryReadCache.getCommandByKey(lKey));
        }
    }

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

sal_Bool SAL_CALL XCUBasedAcceleratorConfiguration::isModified()
{
    return false;
}

sal_Bool SAL_CALL XCUBasedAcceleratorConfiguration::isReadOnly()
{
    return false;
}

void SAL_CALL XCUBasedAcceleratorConfiguration::setStorage(const css::uno::Reference< css::embed::XStorage >& /*xStorage*/)
{
    SAL_INFO("fwk.accelerators", "XCUBasedAcceleratorConfiguration::setStorage(): implement this HACK .-)");
}

sal_Bool SAL_CALL XCUBasedAcceleratorConfiguration::hasStorage()
{
    SAL_INFO("fwk.accelerators", "XCUBasedAcceleratorConfiguration::hasStorage(): implement this HACK .-)");
    return false;
}

void SAL_CALL XCUBasedAcceleratorConfiguration::addConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& /*xListener*/)
{
    SAL_INFO("fwk.accelerators", "XCUBasedAcceleratorConfiguration::addConfigurationListener(): implement me");
}

void SAL_CALL XCUBasedAcceleratorConfiguration::removeConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& /*xListener*/)
{
    SAL_INFO("fwk.accelerators", "XCUBasedAcceleratorConfiguration::removeConfigurationListener(): implement me");
}

void SAL_CALL XCUBasedAcceleratorConfiguration::reset()
{
    css::uno::Reference< css::container::XNamed > xNamed(m_xCfg, css::uno::UNO_QUERY);
    OUString sConfig = xNamed->getName();
    if ( sConfig == "Global" )
    {
        m_xCfg.set(
            ::comphelper::ConfigurationHelper::openConfig( m_xContext, CFG_ENTRY_GLOBAL, ::comphelper::EConfigurationModes::AllLocales ),
            css::uno::UNO_QUERY );
        XCUBasedAcceleratorConfiguration::reload();
    }
    else if ( sConfig == "Modules" )
    {
        m_xCfg.set(
            ::comphelper::ConfigurationHelper::openConfig( m_xContext, CFG_ENTRY_MODULES, ::comphelper::EConfigurationModes::AllLocales ),
            css::uno::UNO_QUERY );
        XCUBasedAcceleratorConfiguration::reload();
    }
}

void SAL_CALL XCUBasedAcceleratorConfiguration::addResetListener(const css::uno::Reference< css::form::XResetListener >& /*xListener*/)
{
    SAL_INFO("fwk.accelerators", "XCUBasedAcceleratorConfiguration::addResetListener(): implement me");
}

void SAL_CALL XCUBasedAcceleratorConfiguration::removeResetListener(const css::uno::Reference< css::form::XResetListener >& /*xListener*/)
{
    SAL_INFO("fwk.accelerators", "XCUBasedAcceleratorConfiguration::removeResetListener(): implement me");
}

void SAL_CALL XCUBasedAcceleratorConfiguration::changesOccurred(const css::util::ChangesEvent& aReceivedEvents)
{
    SAL_INFO( "fwk.accelerators", "XCUBasedAcceleratorConfiguration::changesOccurred()" );

    css::uno::Reference< css::container::XHierarchicalNameAccess > xHAccess;
    aReceivedEvents.Base >>= xHAccess;
    if (! xHAccess.is ())
        return;

    const sal_Int32 c = aReceivedEvents.Changes.getLength();
    for (sal_Int32 i=0; i<c; ++i)
    {
        const css::util::ElementChange& aChange  =   aReceivedEvents.Changes[i];

        // Only path of form "PrimaryKeys/Modules/Module['<module_name>']/Key['<command_url>']/Command[<locale>]" will
        // be interesting for use. Sometimes short path values are given also by the broadcaster ... but they must be ignored :-)
        // So we try to split the path into 3 parts (module isn't important here, because we already know it ... because
        // these instance is bound to a specific module configuration ... or it''s the global configuration where no module is given at all.

        OUString sOrgPath;
        OUString sPath;
        OUString sKey;

        aChange.Accessor >>= sOrgPath;
        sPath              = sOrgPath;
        OUString sPrimarySecondary = ::utl::extractFirstFromConfigurationPath(sPath, &sPath);
        OUString sGlobalModules = ::utl::extractFirstFromConfigurationPath(sPath, &sPath);

        if ( sGlobalModules == CFG_ENTRY_GLOBAL )
        {
            sKey = ::utl::extractFirstFromConfigurationPath(sPath, &sPath);
            if ( !sKey.isEmpty() && !sPath.isEmpty() )
                reloadChanged(sPrimarySecondary, sGlobalModules, OUString(), sKey);
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

void SAL_CALL XCUBasedAcceleratorConfiguration::disposing(const css::lang::EventObject& /*aSource*/)
{
}

void XCUBasedAcceleratorConfiguration::impl_ts_load( bool bPreferred, const css::uno::Reference< css::container::XNameAccess >& xCfg )
{
    AcceleratorCache aReadCache;
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
    static constexpr OUStringLiteral sDefaultLocale(u"en-US");

    css::uno::Reference< css::container::XNameAccess > xKey;
    css::uno::Reference< css::container::XNameAccess > xCommand;
    if (xAccess.is())
    {
        css::uno::Sequence< OUString > lKeys = xAccess->getElementNames();
        sal_Int32 nKeys = lKeys.getLength();
        for ( sal_Int32 i=0; i<nKeys; ++i )
        {
            const OUString& sKey = lKeys[i];
            xAccess->getByName(sKey) >>= xKey;
            xKey->getByName(CFG_PROP_COMMAND) >>= xCommand;

            const css::uno::Sequence< OUString > lLocales = xCommand->getElementNames();
            ::std::vector< OUString > aLocales { lLocales.begin(), lLocales.end() };

            OUString sLocale;
            for (auto const& locale : aLocales)
            {
                if ( locale == sIsoLang )
                {
                    sLocale = locale;
                    break;
                }
            }

            if (sLocale.isEmpty())
            {
                for (auto const& locale : aLocales)
                {
                    if ( locale == sDefaultLocale )
                    {
                        sLocale = locale;
                        break;
                    }
                }

                if (sLocale.isEmpty())
                    continue;
            }

            OUString sCommand;
            xCommand->getByName(sLocale) >>= sCommand;
            if (sCommand.isEmpty())
                continue;

            css::awt::KeyEvent aKeyEvent;

            sal_Int32 nIndex = 0;
            std::u16string_view sKeyCommand = o3tl::getToken(sKey, 0, '_', nIndex);
            aKeyEvent.KeyCode = KeyMapping::get().mapIdentifierToCode(OUString::Concat("KEY_") + sKeyCommand);

            const sal_Int32 nToken = 4;
            bool bValid = true;
            sal_Int32 k;
            for (k = 0; k < nToken; ++k)
            {
                if (nIndex < 0)
                    break;

                std::u16string_view sToken = o3tl::getToken(sKey, 0, '_', nIndex);
                if (sToken.empty())
                {
                    bValid = false;
                    break;
                }

                if ( sToken == u"SHIFT" )
                    aKeyEvent.Modifiers |= css::awt::KeyModifier::SHIFT;
                else if ( sToken == u"MOD1" )
                    aKeyEvent.Modifiers |= css::awt::KeyModifier::MOD1;
                else if ( sToken == u"MOD2" )
                    aKeyEvent.Modifiers |= css::awt::KeyModifier::MOD2;
                else if ( sToken == u"MOD3" )
                    aKeyEvent.Modifiers |= css::awt::KeyModifier::MOD3;
                else
                {
                    bValid = false;
                    break;
                }
            }

            if ( !aReadCache.hasKey(aKeyEvent) && bValid && k<nToken)
                aReadCache.setKeyCommandPair(aKeyEvent, sCommand);
        }
    }

    if (bPreferred)
        m_aPrimaryReadCache = std::move(aReadCache);
    else
        m_aSecondaryReadCache = std::move(aReadCache);
}

void XCUBasedAcceleratorConfiguration::impl_ts_save(bool bPreferred)
{
    if (bPreferred)
    {
        AcceleratorCache::TKeyList lPrimaryReadKeys  = m_aPrimaryReadCache.getAllKeys();
        AcceleratorCache::TKeyList lPrimaryWriteKeys = m_pPrimaryWriteCache->getAllKeys();

        for (auto const& primaryReadKey : lPrimaryReadKeys)
        {
            if (!m_pPrimaryWriteCache->hasKey(primaryReadKey))
                removeKeyFromConfiguration(primaryReadKey, true);
        }

        for (auto const& primaryWriteKey : lPrimaryWriteKeys)
        {
            OUString sCommand = m_pPrimaryWriteCache->getCommandByKey(primaryWriteKey);
            if (!m_aPrimaryReadCache.hasKey(primaryWriteKey))
            {
                insertKeyToConfiguration(primaryWriteKey, sCommand, true);
            }
            else
            {
                OUString sReadCommand = m_aPrimaryReadCache.getCommandByKey(primaryWriteKey);
                if (sReadCommand != sCommand)
                    insertKeyToConfiguration(primaryWriteKey, sCommand, true);
            }
        }

        // take over all changes into the original container
        SolarMutexGuard g;
        // coverity[check_after_deref] - confusing but correct
        if (m_pPrimaryWriteCache)
        {
            m_aPrimaryReadCache = *m_pPrimaryWriteCache;
            m_pPrimaryWriteCache.reset();
        }
    }

    else
    {
        AcceleratorCache::TKeyList lSecondaryReadKeys  = m_aSecondaryReadCache.getAllKeys();
        AcceleratorCache::TKeyList lSecondaryWriteKeys = m_pSecondaryWriteCache->getAllKeys();

        for (auto const& secondaryReadKey : lSecondaryReadKeys)
        {
            if (!m_pSecondaryWriteCache->hasKey(secondaryReadKey))
                removeKeyFromConfiguration(secondaryReadKey, false);
        }

        for (auto const& secondaryWriteKey : lSecondaryWriteKeys)
        {
            OUString sCommand = m_pSecondaryWriteCache->getCommandByKey(secondaryWriteKey);
            if (!m_aSecondaryReadCache.hasKey(secondaryWriteKey))
            {
                insertKeyToConfiguration(secondaryWriteKey, sCommand, false);
            }
            else
            {
                OUString sReadCommand = m_aSecondaryReadCache.getCommandByKey(secondaryWriteKey);
                if (sReadCommand != sCommand)
                    insertKeyToConfiguration(secondaryWriteKey, sCommand, false);
            }
        }

        // take over all changes into the original container
        SolarMutexGuard g;
        // coverity[check_after_deref] - confusing but correct
        if (m_pSecondaryWriteCache)
        {
            m_aSecondaryReadCache = *m_pSecondaryWriteCache;
            m_pSecondaryWriteCache.reset();
        }
    }

    ::comphelper::ConfigurationHelper::flush(m_xCfg);
}

void XCUBasedAcceleratorConfiguration::insertKeyToConfiguration( const css::awt::KeyEvent& aKeyEvent, const OUString& sCommand, const bool bPreferred )
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
            xFac.set(xModules, css::uno::UNO_QUERY);
            xInst = xFac->createInstance();
            xModules->insertByName(m_sModuleCFG, css::uno::Any(xInst));
        }
        xModules->getByName(m_sModuleCFG) >>= xContainer;
    }

    const OUString sKey = lcl_getKeyString(aKeyEvent);
    css::uno::Reference< css::container::XNameAccess > xKey;
    css::uno::Reference< css::container::XNameContainer > xCommand;
    if ( !xContainer->hasByName(sKey) )
    {
        xFac.set(xContainer, css::uno::UNO_QUERY);
        xInst = xFac->createInstance();
        xContainer->insertByName(sKey, css::uno::Any(xInst));
    }
    xContainer->getByName(sKey) >>= xKey;

    xKey->getByName(CFG_PROP_COMMAND) >>= xCommand;
    OUString sLocale = impl_ts_getLocale();
    if ( !xCommand->hasByName(sLocale) )
        xCommand->insertByName(sLocale, css::uno::Any(sCommand));
    else
        xCommand->replaceByName(sLocale, css::uno::Any(sCommand));
}

void XCUBasedAcceleratorConfiguration::removeKeyFromConfiguration( const css::awt::KeyEvent& aKeyEvent, const bool bPreferred )
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

    const OUString sKey = lcl_getKeyString(aKeyEvent);
    xContainer->removeByName(sKey);
}

void XCUBasedAcceleratorConfiguration::reloadChanged( const OUString& sPrimarySecondary, std::u16string_view sGlobalModules, const OUString& sModule, const OUString& sKey )
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

    sal_Int32 nIndex = 0;
    std::u16string_view sKeyIdentifier = o3tl::getToken(sKey, 0, '_', nIndex);
    aKeyEvent.KeyCode = KeyMapping::get().mapIdentifierToCode(OUString::Concat("KEY_") + sKeyIdentifier);

    const int nToken = 4;
    for (sal_Int32 i = 0; i < nToken; ++i)
    {
        if ( nIndex < 0 )
            break;

        std::u16string_view sToken = o3tl::getToken(sKey, 0, '_', nIndex);
        if ( sToken == u"SHIFT" )
            aKeyEvent.Modifiers |= css::awt::KeyModifier::SHIFT;
        else if ( sToken == u"MOD1" )
            aKeyEvent.Modifiers |= css::awt::KeyModifier::MOD1;
        else if ( sToken == u"MOD2" )
            aKeyEvent.Modifiers |= css::awt::KeyModifier::MOD2;
        else if ( sToken == u"MOD3" )
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

AcceleratorCache& XCUBasedAcceleratorConfiguration::impl_getCFG(bool bPreferred, bool bWriteAccessRequested)
{
    SolarMutexGuard g;

    if (bPreferred)
    {
        //create copy of our readonly-cache, if write access is forced ... but
        //not still possible!
        if ( bWriteAccessRequested && !m_pPrimaryWriteCache )
        {
            m_pPrimaryWriteCache.reset(new AcceleratorCache(m_aPrimaryReadCache));
        }

        // in case, we have a writeable cache, we use it for reading too!
        // Otherwise the API user can't find its own changes...
        if (m_pPrimaryWriteCache)
            return *m_pPrimaryWriteCache;
        else
            return m_aPrimaryReadCache;
    }

    else
    {
        //create copy of our readonly-cache, if write access is forced ... but
        //not still possible!
        if ( bWriteAccessRequested && !m_pSecondaryWriteCache )
        {
            m_pSecondaryWriteCache.reset(new AcceleratorCache(m_aSecondaryReadCache));
        }

        // in case, we have a writeable cache, we use it for reading too!
        // Otherwise the API user can't find its own changes...
        if (m_pSecondaryWriteCache)
            return *m_pSecondaryWriteCache;
        else
            return m_aSecondaryReadCache;
    }
}

// static
OUString XCUBasedAcceleratorConfiguration::impl_ts_getLocale()
{
    OUString sISOLocale = officecfg::Setup::L10N::ooLocale::get();

    if (sISOLocale.isEmpty())
        return u"en-US"_ustr;
    return sISOLocale;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
