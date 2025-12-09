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

#include <comphelper/docpasswordhelper.hxx>
#include <sal/log.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/securityoptions.hxx>
#include <unotools/ucbhelper.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/stillreadwriteinteraction.hxx>

#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/ucb/PostCommandArgument2.hpp>
#include <officecfg/Office/Common.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <ucbhelper/activedatasink.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/urlobj.hxx>
#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>

namespace utl::MediaDescriptor {

namespace {

OUString removeFragment(OUString const & uri) {
    css::uno::Reference< css::uri::XUriReference > ref(
        css::uri::UriReferenceFactory::create(
            comphelper::getProcessComponentContext())->
        parse(uri));
    if (ref.is()) {
        ref->clearFragment();
        return ref->getUriReference();
    } else {
        SAL_WARN("unotools.misc", "cannot parse <" << uri << ">");
        return uri;
    }
}

/** @short  tries to open a stream by using the given URL.

    @descr  First it tries to open the content in r/w mode (if it's
            allowed to do so). Only in case it's not allowed or it failed
            the stream will be tried to open in readonly mode.

            The MediaDescriptor itself is changed inside this method.
            Means: the stream is added internal and not returned by a value.

    @param  sURL
            the URL for open.

    @param  bLockFile
            specifies whether the file should be locked

    @return TRUE if the stream could be added successfully.
            Note: If FALSE is returned, the error was already handled inside!

    @throw  [css::uno::RuntimeException]
            if the MediaDescriptor seems to be invalid!
 */
bool impl_openStreamWithURL(comphelper::SequenceAsHashMap& rMediaDescriptor, const OUString& sURL,
                            bool bLockFile)
{
    if (sURL.matchIgnoreAsciiCase(".component:") || sURL.matchIgnoreAsciiCase("private:factory/"))
        return false; // No UCB content for .component URLs and factory URLs

    if (INetURLObject(sURL).IsExoticProtocol())
        return false;

    OUString referer(rMediaDescriptor.getUnpackedValueOrDefault(PROP_REFERRER, OUString()));
    if (SvtSecurityOptions::isUntrustedReferer(referer))
    {
        return false;
    }

    // prepare the environment
    auto xOrgInteraction = rMediaDescriptor.getUnpackedValueOrDefault(
        PROP_INTERACTIONHANDLER, css::uno::Reference<css::task::XInteractionHandler>());

    auto xAuthenticationInteraction = rMediaDescriptor.getUnpackedValueOrDefault(
        PROP_AUTHENTICATIONHANDLER, css::uno::Reference<css::task::XInteractionHandler>());

    rtl::Reference<comphelper::StillReadWriteInteraction> xInteraction
        = new comphelper::StillReadWriteInteraction(xOrgInteraction, xAuthenticationInteraction);

    css::uno::Reference<css::ucb::XProgressHandler> xProgress;
    rtl::Reference<ucbhelper::CommandEnvironment> xCommandEnv
        = new ucbhelper::CommandEnvironment(xInteraction, xProgress);

    // try to create the content
    // no content -> no stream => return immediately with FALSE
    ucbhelper::Content aContent;
    css::uno::Reference<css::ucb::XContent> xContent;
    try
    {
        aContent = ucbhelper::Content(sURL, xCommandEnv, comphelper::getProcessComponentContext());
        xContent = aContent.get();
    }
    catch (const css::uno::RuntimeException&)
    {
        throw;
    }
    catch (const css::ucb::ContentCreationException&)
    {
        TOOLS_WARN_EXCEPTION("unotools.misc", "url: '" << sURL << "'");
        return false; // TODO error handling
    }
    catch (const css::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("unotools.misc", "url: '" << sURL << "'");
        return false; // TODO error handling
    }

    // try to open the file in read/write mode
    // (if it's allowed to do so).
    // But handle errors in a "hidden mode". Because
    // we try it readonly later - if read/write is not an option.
    css::uno::Reference<css::io::XStream> xStream;
    css::uno::Reference<css::io::XInputStream> xInputStream;

    bool bReadOnly = false;
    bool bModeRequestedExplicitly = false;
    auto pIt = rMediaDescriptor.find(PROP_READONLY);
    if (pIt != rMediaDescriptor.end())
    {
        pIt->second >>= bReadOnly;
        bModeRequestedExplicitly = true;
    }

    if (!bReadOnly && bLockFile)
    {
        try
        {
            // TODO: use "special" still interaction to suppress error messages
            xStream = aContent.openWriteableStream();
            if (xStream.is())
                xInputStream = xStream->getInputStream();
        }
        catch (const css::uno::RuntimeException&)
        {
            throw;
        }
        catch (const css::uno::Exception&)
        {
            css::uno::Any ex(cppu::getCaughtException());
            // ignore exception, if reason was problem reasoned on
            // open it in WRITABLE mode! Then we try it READONLY
            // later a second time.
            // All other errors must be handled as real error an
            // break this method.
            if (!xInteraction->wasWriteError() || bModeRequestedExplicitly)
            {
                SAL_WARN("unotools.misc", "url: '" << sURL << "' " << exceptionToString(ex));
                // If the protocol is webdav, then we need to treat the stream as readonly, even if the
                // operation was requested as read/write explicitly (the WebDAV UCB implementation is monodirectional
                // read or write not both at the same time).
                if (!INetURLObject(sURL).isAnyKnownWebDAVScheme())
                    return false;
            }
            xStream.clear();
            xInputStream.clear();
        }
    }

    // If opening of the stream in read/write mode was not allowed
    // or failed by an error - we must try it in readonly mode.
    if (!xInputStream.is())
    {
        OUString aScheme;

        try
        {
            css::uno::Reference<css::ucb::XContentIdentifier> xContId(
                aContent.get().is() ? aContent.get()->getIdentifier() : nullptr);

            if (xContId.is())
                aScheme = xContId->getContentProviderScheme();

            // Only file system content provider is able to provide XStream
            // so for this content impossibility to create XStream triggers
            // switch to readonly mode in case of opening with locking on
            if (bLockFile && aScheme.equalsIgnoreAsciiCase("file"))
                bReadOnly = true;
            else
            {
                bool bRequestReadOnly = bReadOnly;
                aContent.getPropertyValue(u"IsReadOnly"_ustr) >>= bReadOnly;
                if (bReadOnly && !bRequestReadOnly && bModeRequestedExplicitly)
                    return false; // the document is explicitly requested with WRITABLE mode
            }
        }
        catch (const css::uno::RuntimeException&)
        {
            throw;
        }
        catch (const css::uno::Exception&)
        { /* no error handling if IsReadOnly property does not exist for UCP */
        }

        if (bReadOnly)
            rMediaDescriptor[PROP_READONLY] <<= bReadOnly;

        xInteraction->resetInterceptions();
        xInteraction->resetErrorStates();
        try
        {
            // all the contents except file-URLs should be opened as usual
            if (bLockFile || !aScheme.equalsIgnoreAsciiCase("file"))
                xInputStream = aContent.openStream();
            else
                xInputStream = aContent.openStreamNoLock();
        }
        catch (const css::uno::RuntimeException&)
        {
            throw;
        }
        catch (const css::uno::Exception&)
        {
            TOOLS_INFO_EXCEPTION("unotools.misc", "url: '" << sURL << "'");
            return false;
        }
    }

    // add streams to the descriptor
    if (xContent.is())
        rMediaDescriptor[PROP_UCBCONTENT] <<= xContent;
    if (xStream.is())
        rMediaDescriptor[PROP_STREAM] <<= xStream;
    if (xInputStream.is())
        rMediaDescriptor[PROP_INPUTSTREAM] <<= xInputStream;

    // At least we need an input stream. The r/w stream is optional ...
    return xInputStream.is();
}

/** @short  tries to open a stream by using the given PostData stream.

    @descr  The stream is used directly ...

            The MediaDescriptor itself is changed inside this method.
            Means: the stream is added internal and not returned by a value.

    @param  _rxPostData
            the PostData stream.

    @return TRUE if the stream could be added successfully.
            Note: If FALSE is returned, the error was already handled inside!

    @throw  [css::uno::RuntimeException]
            if the MediaDescriptor seems to be invalid!

    @throw  [css::lang::IllegalArgumentException]
            if the given PostData stream is <NULL/>.
 */
bool impl_openStreamWithPostData(comphelper::SequenceAsHashMap& rMediaDescriptor,
                                 const css::uno::Reference<css::io::XInputStream>& _rxPostData)
{
    if (!_rxPostData.is())
        throw css::lang::IllegalArgumentException(u"Found invalid PostData."_ustr,
                                                  css::uno::Reference<css::uno::XInterface>(), 1);

    // PostData can't be used in read/write mode!
    rMediaDescriptor[PROP_READONLY] <<= true;

    // prepare the environment
    auto xInteraction = rMediaDescriptor.getUnpackedValueOrDefault(
        PROP_INTERACTIONHANDLER, css::uno::Reference<css::task::XInteractionHandler>());
    css::uno::Reference<css::ucb::XProgressHandler> xProgress;
    rtl::Reference<::ucbhelper::CommandEnvironment> xCommandEnv
        = new ::ucbhelper::CommandEnvironment(xInteraction, xProgress);

    // media type
    OUString sMediaType = rMediaDescriptor.getUnpackedValueOrDefault(PROP_MEDIATYPE, OUString());
    if (sMediaType.isEmpty())
    {
        sMediaType = "application/x-www-form-urlencoded";
        rMediaDescriptor[PROP_MEDIATYPE] <<= sMediaType;
    }

    // url
    OUString sURL(rMediaDescriptor.getUnpackedValueOrDefault(PROP_URL, OUString()));

    css::uno::Reference<css::io::XInputStream> xResultStream;
    try
    {
        // seek PostData stream to the beginning
        css::uno::Reference<css::io::XSeekable> xSeek(_rxPostData, css::uno::UNO_QUERY);
        if (xSeek.is())
            xSeek->seek(0);

        // a content for the URL
        ::ucbhelper::Content aContent(sURL, xCommandEnv, comphelper::getProcessComponentContext());

        // use post command
        css::ucb::PostCommandArgument2 aPostArgument;
        aPostArgument.Source = _rxPostData;
        css::uno::Reference<css::io::XActiveDataSink> xSink(new ucbhelper::ActiveDataSink);
        aPostArgument.Sink = xSink;
        aPostArgument.MediaType = sMediaType;
        aPostArgument.Referer
            = rMediaDescriptor.getUnpackedValueOrDefault(PROP_REFERRER, OUString());

        aContent.executeCommand(u"post"_ustr, css::uno::Any(aPostArgument));

        // get result
        xResultStream = xSink->getInputStream();
    }
    catch (const css::uno::Exception&)
    {
    }

    // success?
    if (!xResultStream.is())
    {
        OSL_FAIL("no valid reply to the HTTP-Post");
        return false;
    }

    rMediaDescriptor[PROP_INPUTSTREAM] <<= xResultStream;
    return true;
}

/** @short  it checks if the descriptor already has a valid
            InputStream item and creates a new one, if not.

    @descr  This function uses the current items of this MediaDescriptor,
            to open the stream (as e.g. URL, ReadOnly, PostData etcpp.).
            It creates a seekable stream and put it into the descriptor.

            A might existing InteractionHandler will be used automatically,
            to solve problems!

    @param  bLockFile
            specifies whether the file should be locked

    @return TRUE, if the stream was already part of the descriptor or could
            be created as new item. FALSE otherwise.
 */
bool impl_addInputStream(comphelper::SequenceAsHashMap& rMediaDescriptor, bool bLockFile)
{
    // check for an already existing stream item first
    auto pIt = rMediaDescriptor.find(PROP_INPUTSTREAM);
    if (pIt != rMediaDescriptor.end())
        return true;

    try
    {
        // No stream available - create a new one
        // a) data comes as PostData ...
        pIt = rMediaDescriptor.find(PROP_POSTDATA);
        if (pIt != rMediaDescriptor.end())
        {
            const css::uno::Any& rPostData = pIt->second;
            css::uno::Reference<css::io::XInputStream> xPostData;
            rPostData >>= xPostData;

            return impl_openStreamWithPostData(rMediaDescriptor, xPostData);
        }

        // b) ... or we must get it from the given URL
        OUString sURL = rMediaDescriptor.getUnpackedValueOrDefault(PROP_URL, OUString());
        if (sURL.isEmpty())
            throw css::uno::Exception(u"Found no URL."_ustr,
                                      css::uno::Reference<css::uno::XInterface>());

        return impl_openStreamWithURL(rMediaDescriptor, removeFragment(sURL), bLockFile);
    }
    catch (const css::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("unotools.misc", "invalid MediaDescriptor detected");
        return false;
    }
}

}

bool isStreamReadOnly(const comphelper::SequenceAsHashMap& rMediaDescriptor)
{
    bool bReadOnly = false;

    // check for explicit readonly state
    auto pIt = rMediaDescriptor.find(PROP_READONLY);
    if (pIt != rMediaDescriptor.end())
    {
        pIt->second >>= bReadOnly;
        return bReadOnly;
    }

    // streams based on post data are readonly by definition
    pIt = rMediaDescriptor.find(PROP_POSTDATA);
    if (pIt != rMediaDescriptor.end())
        return true;

    // A XStream capsulate XInputStream and XOutputStream ...
    // If it exists - the file must be open in read/write mode!
    pIt = rMediaDescriptor.find(PROP_STREAM);
    if (pIt != rMediaDescriptor.end())
        return false;

    // Only file system content provider is able to provide XStream
    // so for this content impossibility to create XStream triggers
    // switch to readonly mode.
    try
    {
        auto xContent = rMediaDescriptor.getUnpackedValueOrDefault(PROP_UCBCONTENT, css::uno::Reference< css::ucb::XContent >());
        if (xContent.is())
        {
            css::uno::Reference< css::ucb::XContentIdentifier > xId = xContent->getIdentifier();
            OUString aScheme;
            if (xId.is())
                aScheme = xId->getContentProviderScheme();

            if (aScheme.equalsIgnoreAsciiCase("file"))
                bReadOnly = true;
            else
            {
                ::ucbhelper::Content aContent(xContent,
                                              utl::UCBContentHelper::getDefaultCommandEnvironment(),
                                              comphelper::getProcessComponentContext());
                aContent.getPropertyValue(u"IsReadOnly"_ustr) >>= bReadOnly;
            }
        }
    }
    catch(const css::uno::RuntimeException& )
        { throw; }
    catch(const css::uno::Exception&)
        {}

    return bReadOnly;
}

css::uno::Any getComponentDataEntry(const comphelper::SequenceAsHashMap& rMediaDescriptor,
                                                     const OUString& rName)
{
    auto aPropertyIter = rMediaDescriptor.find(PROP_COMPONENTDATA);
    if( aPropertyIter != rMediaDescriptor.end() )
        return comphelper::NamedValueCollection( aPropertyIter->second ).get( rName );
    return css::uno::Any();
}

void setComponentDataEntry(comphelper::SequenceAsHashMap& rMediaDescriptor,
                                            const OUString& rName, const css::uno::Any& rValue)
{
    if( rValue.hasValue() )
    {
        // get or create the 'ComponentData' property entry
        css::uno::Any& rCompDataAny = rMediaDescriptor[PROP_COMPONENTDATA];
        // insert the value (retain sequence type, create NamedValue elements by default)
        bool bHasNamedValues = !rCompDataAny.hasValue() || rCompDataAny.has< css::uno::Sequence< css::beans::NamedValue > >();
        bool bHasPropValues = rCompDataAny.has< css::uno::Sequence< css::beans::PropertyValue > >();
        OSL_ENSURE( bHasNamedValues || bHasPropValues, "MediaDescriptor::setComponentDataEntry - incompatible 'ComponentData' property in media descriptor" );
        if( bHasNamedValues || bHasPropValues )
        {
            // insert or overwrite the passed value
            comphelper::SequenceAsHashMap aCompDataMap( rCompDataAny );
            aCompDataMap[ rName ] = rValue;
            // write back the sequence (restore sequence with correct element type)
            rCompDataAny = aCompDataMap.getAsConstAny( bHasPropValues );
        }
    }
    else
    {
        // if an empty Any is passed, clear the entry
        clearComponentDataEntry(rMediaDescriptor, rName);
    }
}

void clearComponentDataEntry(comphelper::SequenceAsHashMap& rMediaDescriptor,
                                              const OUString& rName)
{
    auto aPropertyIter = rMediaDescriptor.find(PROP_COMPONENTDATA);
    if( aPropertyIter == rMediaDescriptor.end() )
        return;

    css::uno::Any& rCompDataAny = aPropertyIter->second;
    bool bHasNamedValues = rCompDataAny.has< css::uno::Sequence< css::beans::NamedValue > >();
    bool bHasPropValues = rCompDataAny.has< css::uno::Sequence< css::beans::PropertyValue > >();
    OSL_ENSURE( bHasNamedValues || bHasPropValues, "MediaDescriptor::clearComponentDataEntry - incompatible 'ComponentData' property in media descriptor" );
    if( bHasNamedValues || bHasPropValues )
    {
        // remove the value with the passed name
        comphelper::SequenceAsHashMap aCompDataMap( rCompDataAny );
        aCompDataMap.erase( rName );
        // write back the sequence, or remove it completely if it is empty
        if( aCompDataMap.empty() )
            rMediaDescriptor.erase(aPropertyIter);
        else
            rCompDataAny = aCompDataMap.getAsConstAny( bHasPropValues );
    }
}

css::uno::Sequence< css::beans::NamedValue > requestAndVerifyDocPassword(
        comphelper::SequenceAsHashMap& rMediaDescriptor,
        comphelper::IDocPasswordVerifier& rVerifier,
        comphelper::DocPasswordRequestType eRequestType,
        const ::std::vector< OUString >* pDefaultPasswords )
{
    auto aMediaEncData = rMediaDescriptor.getUnpackedValueOrDefault(
        PROP_ENCRYPTIONDATA, css::uno::Sequence< css::beans::NamedValue >() );
    OUString aMediaPassword = rMediaDescriptor.getUnpackedValueOrDefault(
        PROP_PASSWORD, OUString() );
    auto xInteractHandler = rMediaDescriptor.getUnpackedValueOrDefault(
        PROP_INTERACTIONHANDLER, css::uno::Reference< css::task::XInteractionHandler >() );
    OUString aDocumentName = rMediaDescriptor.getUnpackedValueOrDefault(
        PROP_URL, OUString() );

    bool bIsDefaultPassword = false;
    css::uno::Sequence< css::beans::NamedValue > aEncryptionData = comphelper::DocPasswordHelper::requestAndVerifyDocPassword(
        rVerifier, aMediaEncData, aMediaPassword, xInteractHandler, aDocumentName, eRequestType, pDefaultPasswords, &bIsDefaultPassword );

    rMediaDescriptor.erase(PROP_PASSWORD);
    rMediaDescriptor.erase(PROP_ENCRYPTIONDATA);

    // insert encryption info into media descriptor
    // TODO
    if( aEncryptionData.hasElements() )
        rMediaDescriptor[PROP_ENCRYPTIONDATA] <<= aEncryptionData;

    return aEncryptionData;
}

bool addInputStream(comphelper::SequenceAsHashMap& rMediaDescriptor)
{
    return impl_addInputStream(rMediaDescriptor, true);
}

/*-----------------------------------------------*/
bool addInputStreamOwnLock(comphelper::SequenceAsHashMap& rMediaDescriptor)
{
    const bool bLock = !comphelper::IsFuzzing()
        && officecfg::Office::Common::Misc::UseDocumentSystemFileLocking::get();
    return impl_addInputStream(rMediaDescriptor, bLock);
}

} // namespace utl::MediaDescriptor

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
