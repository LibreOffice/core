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

#include <rtl/ustrbuf.hxx>


namespace comphelper{

namespace css = ::com::sun::star;


const ::rtl::OUString& MediaDescriptor::PROP_ABORTED()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Aborted"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_ASTEMPLATE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("AsTemplate"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_COMPONENTDATA()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("ComponentData"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_DOCUMENTSERVICE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("DocumentService"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_ENCRYPTIONDATA()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("EncryptionData"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_FILENAME()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("FileName"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_FILTERNAME()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("FilterName"));
    return sProp;
}

const OUString& MediaDescriptor::PROP_FILTERPROVIDER()
{
    static const OUString aProp("FilterProvider");
    return aProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_FILTEROPTIONS()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("FilterOptions"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_FRAME()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Frame"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_FRAMENAME()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("FrameName"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_HIDDEN()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Hidden"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_INPUTSTREAM()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("InputStream"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_INTERACTIONHANDLER()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("InteractionHandler"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_JUMPMARK()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("JumpMark"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_MACROEXECUTIONMODE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("MacroExecutionMode"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_MEDIATYPE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("MediaType"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_MINIMIZED()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Minimized"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_NOAUTOSAVE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("NoAutoSave"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_OPENNEWVIEW()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("OpenNewView"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_OUTPUTSTREAM()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("OutputStream"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_POSTDATA()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("PostData"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_PREVIEW()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Preview"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_READONLY()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_REFERRER()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Referer"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_STATUSINDICATOR()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("StatusIndicator"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_STREAM()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Stream"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_STREAMFOROUTPUT()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("StreamForOutput"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_TEMPLATENAME()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("TemplateName"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_TYPENAME()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("TypeName"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_UCBCONTENT()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("UCBContent"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_UPDATEDOCMODE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("UpdateDocMode"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_URL()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("URL"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_VERSION()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Version"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_DOCUMENTTITLE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("DocumentTitle"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_MODEL()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Model"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_PASSWORD()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Password"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_TITLE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("Title"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_SALVAGEDFILE()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("SalvagedFile"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_VIEWONLY()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("ViewOnly"));
    return sProp;
}

const ::rtl::OUString& MediaDescriptor::PROP_DOCUMENTBASEURL()
{
    static const ::rtl::OUString sProp(RTL_CONSTASCII_USTRINGPARAM("DocumentBaseURL"));
    return sProp;
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
    static ::rtl::OUString CONTENTSCHEME_FILE(     RTL_CONSTASCII_USTRINGPARAM( "file" ));
    static ::rtl::OUString CONTENTPROP_ISREADONLY( RTL_CONSTASCII_USTRINGPARAM( "IsReadOnly" ));
    static sal_Bool        READONLY_FALLBACK      = sal_False;

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
            ::rtl::OUString aScheme;
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

css::uno::Any MediaDescriptor::getComponentDataEntry( const ::rtl::OUString& rName ) const
{
    css::uno::Any aEntry;
    SequenceAsHashMap::const_iterator aPropertyIter = find( PROP_COMPONENTDATA() );
    if( aPropertyIter != end() )
        return NamedValueCollection( aPropertyIter->second ).get( rName );
    return css::uno::Any();
}

void MediaDescriptor::setComponentDataEntry( const ::rtl::OUString& rName, const css::uno::Any& rValue )
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

void MediaDescriptor::clearComponentDataEntry( const ::rtl::OUString& rName )
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
        ::rtl::OUString sURL = getUnpackedValueOrDefault(MediaDescriptor::PROP_URL(), ::rtl::OUString());
        if (sURL.isEmpty())
            throw css::uno::Exception(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Found no URL." )),
                    css::uno::Reference< css::uno::XInterface >());

        // Parse URL! Only the main part has to be used further. E.g. a jumpmark can make trouble
        ::rtl::OUString sNormalizedURL = impl_normalizeURL( sURL );
        return impl_openStreamWithURL( sNormalizedURL, bLockFile );
    }
#if OSL_DEBUG_LEVEL > 0
    catch(const css::uno::Exception& ex)
    {
        ::rtl::OUStringBuffer sMsg(256);
        sMsg.appendAscii("Invalid MediaDescriptor detected:\n");
        sMsg.append     (ex.Message                           );
        OSL_FAIL(::rtl::OUStringToOString(sMsg.makeStringAndClear(), RTL_TEXTENCODING_UTF8).getStr());
    }
#else
    catch(const css::uno::Exception&)
        {}
#endif

    return sal_False;
}

sal_Bool MediaDescriptor::impl_openStreamWithPostData( const css::uno::Reference< css::io::XInputStream >& _rxPostData )
    throw(::com::sun::star::uno::RuntimeException)
{
    if ( !_rxPostData.is() )
        throw css::lang::IllegalArgumentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Found invalid PostData." )),
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
    ::rtl::OUString sMediaType = getUnpackedValueOrDefault(MediaDescriptor::PROP_MEDIATYPE(), ::rtl::OUString());
    if (sMediaType.isEmpty())
    {
        sMediaType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "application/x-www-form-urlencoded" ));
        (*this)[MediaDescriptor::PROP_MEDIATYPE()] <<= sMediaType;
    }

    // url
    ::rtl::OUString sURL( getUnpackedValueOrDefault( PROP_URL(), ::rtl::OUString() ) );

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
        aPostArgument.Referer = getUnpackedValueOrDefault( PROP_REFERRER(), ::rtl::OUString() );

        ::rtl::OUString sCommandName( RTL_CONSTASCII_USTRINGPARAM( "post" ) );
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
sal_Bool MediaDescriptor::impl_openStreamWithURL( const ::rtl::OUString& sURL, sal_Bool bLockFile )
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
    // no content -> no stream => return immediatly with FALSE
    ::ucbhelper::Content                      aContent;
    css::uno::Reference< css::ucb::XContent > xContent;
    try
    {
        aContent = ::ucbhelper::Content(sURL, xCommandEnv, getProcessComponentContext());
        xContent = aContent.get();
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::ucb::ContentCreationException&)
        { return sal_False; } // TODO error handling
    catch(const css::uno::Exception&)
        { return sal_False; } // TODO error handling

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
        catch(const css::uno::Exception&)
            {
                // ignore exception, if reason was problem reasoned on
                // open it in WRITEABLE mode! Then we try it READONLY
                // later a second time.
                // All other errors must be handled as real error an
                // break this method.
                if (!pInteraction->wasWriteError() || bModeRequestedExplicitly)
                    return sal_False;
                xStream.clear();
                xInputStream.clear();
            }
    }

    // If opening of the stream in read/write mode wasnt allowed
    // or failed by an error - we must try it in readonly mode.
    if (!xInputStream.is())
    {
        rtl::OUString aScheme;

        try
        {
            css::uno::Reference< css::ucb::XContentIdentifier > xContId(
                aContent.get().is() ? aContent.get()->getIdentifier() : 0 );

            if ( xContId.is() )
                aScheme = xContId->getContentProviderScheme();

            // Only file system content provider is able to provide XStream
            // so for this content impossibility to create XStream triggers
            // switch to readonly mode in case of opening with locking on
            if( bLockFile && aScheme.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("file")) )
                bReadOnly = sal_True;
            else
            {
                sal_Bool bRequestReadOnly = bReadOnly;
                aContent.getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsReadOnly" ) ) ) >>= bReadOnly;
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
            if ( bLockFile || !aScheme.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("file")) )
                xInputStream = aContent.openStream();
            else
                xInputStream = aContent.openStreamNoLock();
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { return sal_False; }
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

::rtl::OUString MediaDescriptor::impl_normalizeURL(const ::rtl::OUString& sURL)
{
    /* Remove Jumpmarks (fragments) of an URL only here.
       They are not part of any URL and as a result may be
       no ucb content can be created then.
       On the other side arguments must exists ... because
       they are part of an URL.

       Do not use the URLTransformer service here. Because
       it parses the URL in another way. It's main part isnt enough
       and it's complete part contains the jumpmark (fragment) parameter ...
    */

    try
    {
        css::uno::Reference< css::uno::XComponentContext >    xContext    = ::comphelper::getProcessComponentContext();
        css::uno::Reference< css::uri::XUriReferenceFactory > xUriFactory = css::uri::UriReferenceFactory::create(xContext);;
        css::uno::Reference< css::uri::XUriReference >        xUriRef     = xUriFactory->parse(sURL);
        if (xUriRef.is())
        {
            xUriRef->clearFragment();
            return xUriRef->getUriReference();
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}

    // If an error ocurred ... return the original URL.
    // It's a try .-)
    return sURL;
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
