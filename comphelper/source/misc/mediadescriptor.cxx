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

#include <comphelper/mediadescriptor.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/stillreadwriteinteraction.hxx>

#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/ucb/UnsupportedDataSinkException.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/ucb/PostCommandArgument2.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <officecfg/Office/Common.hxx>
#include <ucbhelper/interceptedinteraction.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <ucbhelper/activedatasink.hxx>
#include <comphelper/processfactory.hxx>

namespace comphelper{

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
        SAL_WARN("comphelper", "cannot parse <" << uri << ">");
        return uri;
    }
}

}

const OUString& MediaDescriptor::PROP_ABORTED()
{
    static const OUString sProp("Aborted");
    return sProp;
}

const OUString& MediaDescriptor::PROP_ASTEMPLATE()
{
    static const OUString sProp("AsTemplate");
    return sProp;
}

const OUString& MediaDescriptor::PROP_COMPONENTDATA()
{
    static const OUString sProp("ComponentData");
    return sProp;
}

const OUString& MediaDescriptor::PROP_DOCUMENTSERVICE()
{
    static const OUString sProp("DocumentService");
    return sProp;
}

const OUString& MediaDescriptor::PROP_ENCRYPTIONDATA()
{
    static const OUString sProp("EncryptionData");
    return sProp;
}

const OUString& MediaDescriptor::PROP_FILENAME()
{
    static const OUString sProp("FileName");
    return sProp;
}

const OUString& MediaDescriptor::PROP_FILTERNAME()
{
    static const OUString sProp("FilterName");
    return sProp;
}

const OUString& MediaDescriptor::PROP_FILTERPROVIDER()
{
    static const OUString aProp("FilterProvider");
    return aProp;
}

const OUString& MediaDescriptor::PROP_FILTEROPTIONS()
{
    static const OUString sProp("FilterOptions");
    return sProp;
}

const OUString& MediaDescriptor::PROP_FRAME()
{
    static const OUString sProp("Frame");
    return sProp;
}

const OUString& MediaDescriptor::PROP_FRAMENAME()
{
    static const OUString sProp("FrameName");
    return sProp;
}

const OUString& MediaDescriptor::PROP_HIDDEN()
{
    static const OUString sProp("Hidden");
    return sProp;
}

const OUString& MediaDescriptor::PROP_INPUTSTREAM()
{
    static const OUString sProp("InputStream");
    return sProp;
}

const OUString& MediaDescriptor::PROP_INTERACTIONHANDLER()
{
    static const OUString sProp("InteractionHandler");
    return sProp;
}

const OUString& MediaDescriptor::PROP_JUMPMARK()
{
    static const OUString sProp("JumpMark");
    return sProp;
}

const OUString& MediaDescriptor::PROP_MACROEXECUTIONMODE()
{
    static const OUString sProp("MacroExecutionMode");
    return sProp;
}

const OUString& MediaDescriptor::PROP_MEDIATYPE()
{
    static const OUString sProp("MediaType");
    return sProp;
}

const OUString& MediaDescriptor::PROP_MINIMIZED()
{
    static const OUString sProp("Minimized");
    return sProp;
}

const OUString& MediaDescriptor::PROP_NOAUTOSAVE()
{
    static const OUString sProp("NoAutoSave");
    return sProp;
}

const OUString& MediaDescriptor::PROP_OPENNEWVIEW()
{
    static const OUString sProp("OpenNewView");
    return sProp;
}

const OUString& MediaDescriptor::PROP_OUTPUTSTREAM()
{
    static const OUString sProp("OutputStream");
    return sProp;
}

const OUString& MediaDescriptor::PROP_POSTDATA()
{
    static const OUString sProp("PostData");
    return sProp;
}

const OUString& MediaDescriptor::PROP_PREVIEW()
{
    static const OUString sProp("Preview");
    return sProp;
}

const OUString& MediaDescriptor::PROP_READONLY()
{
    static const OUString sProp("ReadOnly");
    return sProp;
}

const OUString& MediaDescriptor::PROP_REFERRER()
{
    static const OUString sProp("Referer");
    return sProp;
}

const OUString& MediaDescriptor::PROP_STATUSINDICATOR()
{
    static const OUString sProp("StatusIndicator");
    return sProp;
}

const OUString& MediaDescriptor::PROP_STREAM()
{
    static const OUString sProp("Stream");
    return sProp;
}

const OUString& MediaDescriptor::PROP_STREAMFOROUTPUT()
{
    static const OUString sProp("StreamForOutput");
    return sProp;
}

const OUString& MediaDescriptor::PROP_TEMPLATENAME()
{
    static const OUString sProp("TemplateName");
    return sProp;
}

const OUString& MediaDescriptor::PROP_TYPENAME()
{
    static const OUString sProp("TypeName");
    return sProp;
}

const OUString& MediaDescriptor::PROP_UCBCONTENT()
{
    static const OUString sProp("UCBContent");
    return sProp;
}

const OUString& MediaDescriptor::PROP_UPDATEDOCMODE()
{
    static const OUString sProp("UpdateDocMode");
    return sProp;
}

const OUString& MediaDescriptor::PROP_URL()
{
    static const OUString sProp("URL");
    return sProp;
}

const OUString& MediaDescriptor::PROP_VERSION()
{
    static const OUString sProp("Version");
    return sProp;
}

const OUString& MediaDescriptor::PROP_DOCUMENTTITLE()
{
    static const OUString sProp("DocumentTitle");
    return sProp;
}

const OUString& MediaDescriptor::PROP_MODEL()
{
    static const OUString sProp("Model");
    return sProp;
}

const OUString& MediaDescriptor::PROP_PASSWORD()
{
    static const OUString sProp("Password");
    return sProp;
}

const OUString& MediaDescriptor::PROP_TITLE()
{
    static const OUString sProp("Title");
    return sProp;
}

const OUString& MediaDescriptor::PROP_SALVAGEDFILE()
{
    static const OUString sProp("SalvagedFile");
    return sProp;
}

const OUString& MediaDescriptor::PROP_VIEWONLY()
{
    static const OUString sProp("ViewOnly");
    return sProp;
}

const OUString& MediaDescriptor::PROP_DOCUMENTBASEURL()
{
    static const OUString sProp("DocumentBaseURL");
    return sProp;
}

const OUString& MediaDescriptor::PROP_DEEPDETECTION()
{
    static const OUString aProp("DeepDetection");
    return aProp;
}

MediaDescriptor::MediaDescriptor()
    : SequenceAsHashMap()
{
}

MediaDescriptor::MediaDescriptor(const css::uno::Sequence< css::beans::PropertyValue >& lSource)
    : SequenceAsHashMap(lSource)
{
}

sal_Bool MediaDescriptor::isStreamReadOnly() const
{
    static OUString CONTENTSCHEME_FILE( "file" );
    static OUString CONTENTPROP_ISREADONLY( "IsReadOnly" );
    static sal_Bool READONLY_FALLBACK = sal_False;

    sal_Bool bReadOnly = READONLY_FALLBACK;

    // check for explicit readonly state
    const_iterator pIt = find(MediaDescriptor::PROP_READONLY());
    if (pIt != end())
    {
        pIt->second >>= bReadOnly;
        return bReadOnly;
    }

    // streams based on post data are readonly by definition
    pIt = find(MediaDescriptor::PROP_POSTDATA());
    if (pIt != end())
        return sal_True;

    // A XStream capsulate XInputStream and XOutputStream ...
    // If it exists - the file must be open in read/write mode!
    pIt = find(MediaDescriptor::PROP_STREAM());
    if (pIt != end())
        return sal_False;

    // Only file system content provider is able to provide XStream
    // so for this content impossibility to create XStream triggers
    // switch to readonly mode.
    try
    {
        css::uno::Reference< css::ucb::XContent > xContent = getUnpackedValueOrDefault(MediaDescriptor::PROP_UCBCONTENT(), css::uno::Reference< css::ucb::XContent >());
        if (xContent.is())
        {
            css::uno::Reference< css::ucb::XContentIdentifier > xId(xContent->getIdentifier(), css::uno::UNO_QUERY);
            OUString aScheme;
            if (xId.is())
                aScheme = xId->getContentProviderScheme();

            if (aScheme.equalsIgnoreAsciiCase(CONTENTSCHEME_FILE))
                bReadOnly = sal_True;
            else
            {
                ::ucbhelper::Content aContent(xContent, css::uno::Reference< css::ucb::XCommandEnvironment >(), getProcessComponentContext());
                aContent.getPropertyValue(CONTENTPROP_ISREADONLY) >>= bReadOnly;
            }
        }
    }
    catch(const css::uno::RuntimeException& )
        { throw; }
    catch(const css::uno::Exception&)
        {}

    return bReadOnly;
}

// ----------------------------------------------------------------------------

css::uno::Any MediaDescriptor::getComponentDataEntry( const OUString& rName ) const
{
    css::uno::Any aEntry;
    SequenceAsHashMap::const_iterator aPropertyIter = find( PROP_COMPONENTDATA() );
    if( aPropertyIter != end() )
        return NamedValueCollection( aPropertyIter->second ).get( rName );
    return css::uno::Any();
}

void MediaDescriptor::setComponentDataEntry( const OUString& rName, const css::uno::Any& rValue )
{
    if( rValue.hasValue() )
    {
        // get or create the 'ComponentData' property entry
        css::uno::Any& rCompDataAny = operator[]( PROP_COMPONENTDATA() );
        // insert the value (retain sequence type, create NamedValue elements by default)
        bool bHasNamedValues = !rCompDataAny.hasValue() || rCompDataAny.has< css::uno::Sequence< css::beans::NamedValue > >();
        bool bHasPropValues = rCompDataAny.has< css::uno::Sequence< css::beans::PropertyValue > >();
        OSL_ENSURE( bHasNamedValues || bHasPropValues, "MediaDescriptor::setComponentDataEntry - incompatible 'ComponentData' property in media descriptor" );
        if( bHasNamedValues || bHasPropValues )
        {
            // insert or overwrite the passed value
            SequenceAsHashMap aCompDataMap( rCompDataAny );
            aCompDataMap[ rName ] = rValue;
            // write back the sequence (restore sequence with correct element type)
            rCompDataAny = aCompDataMap.getAsConstAny( bHasPropValues );
        }
    }
    else
    {
        // if an empty Any is passed, clear the entry
        clearComponentDataEntry( rName );
    }
}

void MediaDescriptor::clearComponentDataEntry( const OUString& rName )
{
    SequenceAsHashMap::iterator aPropertyIter = find( PROP_COMPONENTDATA() );
    if( aPropertyIter != end() )
    {
        css::uno::Any& rCompDataAny = aPropertyIter->second;
        bool bHasNamedValues = rCompDataAny.has< css::uno::Sequence< css::beans::NamedValue > >();
        bool bHasPropValues = rCompDataAny.has< css::uno::Sequence< css::beans::PropertyValue > >();
        OSL_ENSURE( bHasNamedValues || bHasPropValues, "MediaDescriptor::clearComponentDataEntry - incompatible 'ComponentData' property in media descriptor" );
        if( bHasNamedValues || bHasPropValues )
        {
            // remove the value with the passed name
            SequenceAsHashMap aCompDataMap( rCompDataAny );
            aCompDataMap.erase( rName );
            // write back the sequence, or remove it completely if it is empty
            if( aCompDataMap.empty() )
                erase( aPropertyIter );
            else
                rCompDataAny = aCompDataMap.getAsConstAny( bHasPropValues );
        }
    }
}

sal_Bool MediaDescriptor::addInputStream()
{
    return impl_addInputStream( sal_True );
}

/*-----------------------------------------------*/
sal_Bool MediaDescriptor::addInputStreamOwnLock()
{
    return impl_addInputStream(
        officecfg::Office::Common::Misc::UseDocumentSystemFileLocking::get());
}

/*-----------------------------------------------*/
sal_Bool MediaDescriptor::impl_addInputStream( sal_Bool bLockFile )
{
    // check for an already existing stream item first
    const_iterator pIt = find(MediaDescriptor::PROP_INPUTSTREAM());
    if (pIt != end())
        return sal_True;

    try
    {
        // No stream available - create a new one
        // a) data comes as PostData ...
        pIt = find(MediaDescriptor::PROP_POSTDATA());
        if (pIt != end())
        {
            const css::uno::Any& rPostData = pIt->second;
            css::uno::Reference< css::io::XInputStream > xPostData;
            rPostData >>= xPostData;

            return impl_openStreamWithPostData( xPostData );
        }

        // b) ... or we must get it from the given URL
        OUString sURL = getUnpackedValueOrDefault(MediaDescriptor::PROP_URL(), OUString());
        if (sURL.isEmpty())
            throw css::uno::Exception("Found no URL.",
                    css::uno::Reference< css::uno::XInterface >());

        // Parse URL! Only the main part has to be used further. E.g. a jumpmark can make trouble
        // We need to keep the full URL with Mark to store the Object ID
        // in CMIS UCB
        return impl_openStreamWithURL( sURL, bLockFile );
    }
    catch(const css::uno::Exception& ex)
    {
        SAL_WARN(
            "comphelper", "invalid MediaDescriptor detected: " << ex.Message);
        return false;
    }
}

sal_Bool MediaDescriptor::impl_openStreamWithPostData( const css::uno::Reference< css::io::XInputStream >& _rxPostData )
    throw(::com::sun::star::uno::RuntimeException)
{
    if ( !_rxPostData.is() )
        throw css::lang::IllegalArgumentException("Found invalid PostData.",
                css::uno::Reference< css::uno::XInterface >(), 1);

    // PostData can't be used in read/write mode!
    (*this)[MediaDescriptor::PROP_READONLY()] <<= sal_True;

    // prepare the environment
    css::uno::Reference< css::task::XInteractionHandler > xInteraction = getUnpackedValueOrDefault(
        MediaDescriptor::PROP_INTERACTIONHANDLER(),
        css::uno::Reference< css::task::XInteractionHandler >());
    css::uno::Reference< css::ucb::XProgressHandler > xProgress;
    ::ucbhelper::CommandEnvironment* pCommandEnv = new ::ucbhelper::CommandEnvironment(xInteraction, xProgress);
    css::uno::Reference< css::ucb::XCommandEnvironment > xCommandEnv(static_cast< css::ucb::XCommandEnvironment* >(pCommandEnv), css::uno::UNO_QUERY);

    // media type
    OUString sMediaType = getUnpackedValueOrDefault(MediaDescriptor::PROP_MEDIATYPE(), OUString());
    if (sMediaType.isEmpty())
    {
        sMediaType = "application/x-www-form-urlencoded";
        (*this)[MediaDescriptor::PROP_MEDIATYPE()] <<= sMediaType;
    }

    // url
    OUString sURL( getUnpackedValueOrDefault( PROP_URL(), OUString() ) );

    css::uno::Reference< css::io::XInputStream > xResultStream;
    try
    {
        // seek PostData stream to the beginning
        css::uno::Reference< css::io::XSeekable > xSeek( _rxPostData, css::uno::UNO_QUERY );
        if ( xSeek.is() )
            xSeek->seek( 0 );

        // a content for the URL
        ::ucbhelper::Content aContent( sURL, xCommandEnv, getProcessComponentContext() );

        // use post command
        css::ucb::PostCommandArgument2 aPostArgument;
        aPostArgument.Source = _rxPostData;
        css::uno::Reference< css::io::XActiveDataSink > xSink( new ucbhelper::ActiveDataSink );
        aPostArgument.Sink = xSink;
        aPostArgument.MediaType = sMediaType;
        aPostArgument.Referer = getUnpackedValueOrDefault( PROP_REFERRER(), OUString() );

        OUString sCommandName( "post" );
        aContent.executeCommand( sCommandName, css::uno::makeAny( aPostArgument ) );

        // get result
        xResultStream = xSink->getInputStream();
    }
    catch( const css::uno::Exception& )
    {
    }

    // success?
    if ( !xResultStream.is() )
    {
        OSL_FAIL( "no valid reply to the HTTP-Post" );
        return sal_False;
    }

    (*this)[MediaDescriptor::PROP_INPUTSTREAM()] <<= xResultStream;
    return sal_True;
}

/*-----------------------------------------------*/
sal_Bool MediaDescriptor::impl_openStreamWithURL( const OUString& sURL, sal_Bool bLockFile )
    throw(::com::sun::star::uno::RuntimeException)
{
    // prepare the environment
    css::uno::Reference< css::task::XInteractionHandler > xOrgInteraction = getUnpackedValueOrDefault(
        MediaDescriptor::PROP_INTERACTIONHANDLER(),
        css::uno::Reference< css::task::XInteractionHandler >());

    StillReadWriteInteraction* pInteraction = new StillReadWriteInteraction(xOrgInteraction);
    css::uno::Reference< css::task::XInteractionHandler > xInteraction(static_cast< css::task::XInteractionHandler* >(pInteraction), css::uno::UNO_QUERY);

    css::uno::Reference< css::ucb::XProgressHandler > xProgress;
    ::ucbhelper::CommandEnvironment* pCommandEnv = new ::ucbhelper::CommandEnvironment(xInteraction, xProgress);
    css::uno::Reference< css::ucb::XCommandEnvironment > xCommandEnv(static_cast< css::ucb::XCommandEnvironment* >(pCommandEnv), css::uno::UNO_QUERY);

    // try to create the content
    // no content -> no stream => return immediately with FALSE
    ::ucbhelper::Content                      aContent;
    css::uno::Reference< css::ucb::XContent > xContent;
    try
    {
        aContent = ::ucbhelper::Content(sURL, xCommandEnv, getProcessComponentContext());
        xContent = aContent.get();
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::ucb::ContentCreationException& e)
        {
            SAL_WARN(
                "comphelper",
                "caught ContentCreationException \"" << e.Message
                    << "\" while opening <" << sURL << ">");
            return sal_False; // TODO error handling
        }
    catch(const css::uno::Exception& e)
        {
            SAL_WARN(
                "comphelper",
                "caught Exception \"" << e.Message << "\" while opening <"
                    << sURL << ">");
            return sal_False; // TODO error handling
        }

    // try to open the file in read/write mode
    // (if its allowed to do so).
    // But handle errors in a "hidden mode". Because
    // we try it readonly later - if read/write isnt an option.
    css::uno::Reference< css::io::XStream >      xStream     ;
    css::uno::Reference< css::io::XInputStream > xInputStream;

    sal_Bool bReadOnly = sal_False;
    sal_Bool bModeRequestedExplicitly = sal_False;
    const_iterator pIt = find(MediaDescriptor::PROP_READONLY());
    if (pIt != end())
    {
        pIt->second >>= bReadOnly;
        bModeRequestedExplicitly = sal_True;
    }

    if ( !bReadOnly && bLockFile )
    {
        try
        {
            // TODO: use "special" still interaction to supress error messages
            xStream = aContent.openWriteableStream();
            if (xStream.is())
                xInputStream = xStream->getInputStream();
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception& e)
            {
                // ignore exception, if reason was problem reasoned on
                // open it in WRITEABLE mode! Then we try it READONLY
                // later a second time.
                // All other errors must be handled as real error an
                // break this method.
                if (!pInteraction->wasWriteError() || bModeRequestedExplicitly)
                {
                    SAL_WARN(
                        "comphelper",
                        "caught Exception \"" << e.Message
                            << "\" while opening <" << sURL << ">");
                    return sal_False;
                }
                xStream.clear();
                xInputStream.clear();
            }
    }

    // If opening of the stream in read/write mode wasnt allowed
    // or failed by an error - we must try it in readonly mode.
    if (!xInputStream.is())
    {
        OUString aScheme;

        try
        {
            css::uno::Reference< css::ucb::XContentIdentifier > xContId(
                aContent.get().is() ? aContent.get()->getIdentifier() : 0 );

            if ( xContId.is() )
                aScheme = xContId->getContentProviderScheme();

            // Only file system content provider is able to provide XStream
            // so for this content impossibility to create XStream triggers
            // switch to readonly mode in case of opening with locking on
            if( bLockFile && aScheme.equalsIgnoreAsciiCase("file") )
                bReadOnly = sal_True;
            else
            {
                sal_Bool bRequestReadOnly = bReadOnly;
                aContent.getPropertyValue("IsReadOnly") >>= bReadOnly;
                if ( bReadOnly && !bRequestReadOnly && bModeRequestedExplicitly )
                        return sal_False; // the document is explicitly requested with WRITEABLE mode
            }
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { /* no error handling if IsReadOnly property does not exist for UCP */ }

        if ( bReadOnly )
               (*this)[MediaDescriptor::PROP_READONLY()] <<= bReadOnly;

        pInteraction->resetInterceptions();
        pInteraction->resetErrorStates();
        try
        {
            // all the contents except file-URLs should be opened as usual
            if ( bLockFile || !aScheme.equalsIgnoreAsciiCase("file") )
                xInputStream = aContent.openStream();
            else
                xInputStream = aContent.openStreamNoLock();
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception& e)
            {
                SAL_WARN(
                    "comphelper",
                    "caught Exception \"" << e.Message << "\" while opening <"
                        << sURL << ">");
                return sal_False;
            }
    }

    // add streams to the descriptor
    if (xContent.is())
        (*this)[MediaDescriptor::PROP_UCBCONTENT()] <<= xContent;
    if (xStream.is())
        (*this)[MediaDescriptor::PROP_STREAM()] <<= xStream;
    if (xInputStream.is())
        (*this)[MediaDescriptor::PROP_INPUTSTREAM()] <<= xInputStream;

    // At least we need an input stream. The r/w stream is optional ...
    return xInputStream.is();
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
