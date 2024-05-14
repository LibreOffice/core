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

#include <osl/diagnose.h>
#include <comphelper/propertysequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/CommandEnvironment.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/GlobalTransferCommandArgument2.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/InsertCommandArgument2.hpp>
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/TransferInfo2.hpp>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XInteractionSupplyName.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/simplenameclashresolverequest.hxx>
#include <utility>
#include "ucbcmds.hxx"
#include "ucb.hxx"

using namespace com::sun::star;

namespace
{
// Helper to provide defaults for type and attributes (save some typing)
beans::Property makeProperty(const OUString& n, sal_Int32 h, uno::Type t = {}, sal_Int16 a = {})
{
    return { n, h, t, a };
}

// struct TransferCommandContext.


struct TransferCommandContext
{
    uno::Reference< uno::XComponentContext >     m_xContext;
    uno::Reference< ucb::XCommandProcessor >     xProcessor;
    uno::Reference< ucb::XCommandEnvironment >   xEnv;
    uno::Reference< ucb::XCommandEnvironment >   xOrigEnv;
    ucb::GlobalTransferCommandArgument2          aArg;

    TransferCommandContext(
        uno::Reference< uno::XComponentContext > xContext,
        uno::Reference< ucb::XCommandProcessor > _xProcessor,
        uno::Reference< ucb::XCommandEnvironment > _xEnv,
        uno::Reference< ucb::XCommandEnvironment > _xOrigEnv,
        ucb::GlobalTransferCommandArgument2 _aArg )
    : m_xContext(std::move( xContext )), xProcessor(std::move( _xProcessor )), xEnv(std::move( _xEnv )),
      xOrigEnv(std::move( _xOrigEnv )), aArg(std::move( _aArg )) {}
};




class InteractionHandlerProxy :
    public cppu::WeakImplHelper< task::XInteractionHandler >
{
    uno::Reference< task::XInteractionHandler > m_xOrig;

public:
    explicit InteractionHandlerProxy(
        uno::Reference< task::XInteractionHandler > xOrig )
    : m_xOrig(std::move( xOrig )) {}

    // XInteractionHandler methods.
    virtual void SAL_CALL handle(
            const uno::Reference< task::XInteractionRequest >& Request ) override;
};


// virtual
void SAL_CALL InteractionHandlerProxy::handle(
            const uno::Reference< task::XInteractionRequest >& Request )
{
    if ( !m_xOrig.is() )
        return;

    // Filter unwanted requests by just not handling them.
    uno::Any aRequest = Request->getRequest();

    // "transfer"
    ucb::InteractiveBadTransferURLException aBadTransferURLEx;
    if ( aRequest >>= aBadTransferURLEx )
    {
        return;
    }
    else
    {
        // "transfer"
        ucb::UnsupportedNameClashException aUnsupportedNameClashEx;
        if ( aRequest >>= aUnsupportedNameClashEx )
        {
            if ( aUnsupportedNameClashEx.NameClash
                    != ucb::NameClash::ERROR )
                return;
        }
        else
        {
            // "insert"
            ucb::NameClashException aNameClashEx;
            if ( aRequest >>= aNameClashEx )
            {
                return;
            }
            else
            {
                // "transfer"
                ucb::UnsupportedCommandException aUnsupportedCommandEx;
                if ( aRequest >>= aUnsupportedCommandEx )
                {
                    return;
                }
            }
        }
    }

    // not filtered; let the original handler do the work.
    m_xOrig->handle( Request );
}




class ActiveDataSink : public cppu::WeakImplHelper< io::XActiveDataSink >
{
    uno::Reference< io::XInputStream > m_xStream;

public:
    // XActiveDataSink methods.
    virtual void SAL_CALL setInputStream(
                        const uno::Reference< io::XInputStream >& aStream ) override;
    virtual uno::Reference< io::XInputStream > SAL_CALL getInputStream() override;
};


// virtual
void SAL_CALL ActiveDataSink::setInputStream(
                        const uno::Reference< io::XInputStream >& aStream )
{
    m_xStream = aStream;
}


// virtual
uno::Reference< io::XInputStream > SAL_CALL ActiveDataSink::getInputStream()
{
    return m_xStream;
}




class CommandProcessorInfo :
    public cppu::WeakImplHelper< ucb::XCommandInfo >
{
    uno::Sequence< ucb::CommandInfo > m_xInfo;

public:
    CommandProcessorInfo();

    // XCommandInfo methods
    virtual uno::Sequence< ucb::CommandInfo > SAL_CALL getCommands() override;
    virtual ucb::CommandInfo SAL_CALL
    getCommandInfoByName( const OUString& Name ) override;
    virtual ucb::CommandInfo SAL_CALL
    getCommandInfoByHandle( sal_Int32 Handle ) override;
    virtual sal_Bool SAL_CALL hasCommandByName( const OUString& Name ) override;
    virtual sal_Bool SAL_CALL hasCommandByHandle( sal_Int32 Handle ) override;
};


CommandProcessorInfo::CommandProcessorInfo()
    : m_xInfo{
        ucb::CommandInfo(
            GETCOMMANDINFO_NAME, // Name
            GETCOMMANDINFO_HANDLE, // Handle
            cppu::UnoType<void>::get() ), // ArgType
        ucb::CommandInfo(
            GLOBALTRANSFER_NAME, // Name
            GLOBALTRANSFER_HANDLE, // Handle
            cppu::UnoType<ucb::GlobalTransferCommandArgument>::get() ), // ArgType
        ucb::CommandInfo(
            CHECKIN_NAME, // Name
            CHECKIN_HANDLE, // Handle
            cppu::UnoType<ucb::CheckinArgument>::get() ) } // ArgType
{
}


// virtual
uno::Sequence< ucb::CommandInfo > SAL_CALL
CommandProcessorInfo::getCommands()
{
    return m_xInfo;
}


// virtual
ucb::CommandInfo SAL_CALL
CommandProcessorInfo::getCommandInfoByName( const OUString& Name )
{
    auto pInfo = std::find_if(std::cbegin(m_xInfo), std::cend(m_xInfo),
        [&Name](const ucb::CommandInfo& rInfo) { return rInfo.Name == Name; });
    if (pInfo != std::cend(m_xInfo))
        return *pInfo;

    throw ucb::UnsupportedCommandException();
}


// virtual
ucb::CommandInfo SAL_CALL
CommandProcessorInfo::getCommandInfoByHandle( sal_Int32 Handle )
{
    auto pInfo = std::find_if(std::cbegin(m_xInfo), std::cend(m_xInfo),
        [&Handle](const ucb::CommandInfo& rInfo) { return rInfo.Handle == Handle; });
    if (pInfo != std::cend(m_xInfo))
        return *pInfo;

    throw ucb::UnsupportedCommandException();
}


// virtual
sal_Bool SAL_CALL CommandProcessorInfo::hasCommandByName(
                                                const OUString& Name )
{
    return std::any_of(std::cbegin(m_xInfo), std::cend(m_xInfo),
        [&Name](const ucb::CommandInfo& rInfo) { return rInfo.Name == Name; });
}


// virtual
sal_Bool SAL_CALL CommandProcessorInfo::hasCommandByHandle( sal_Int32 Handle )
{
    return std::any_of(std::cbegin(m_xInfo), std::cend(m_xInfo),
        [&Handle](const ucb::CommandInfo& rInfo) { return rInfo.Handle == Handle; });
}


OUString createDesiredName(
    const OUString & rSourceURL, const OUString & rNewTitle )
{
    OUString aName( rNewTitle );
    if ( aName.isEmpty() )
    {
        // calculate name using source URL

        // @@@ It's not guaranteed that slashes contained in the URL are
        // actually path separators. This depends on the fact whether the
        // URL is hierarchical. Only then the slashes are path separators.
        // Therefore this algorithm is not guaranteed to work! But, ATM
        // I don't know a better solution. It would have been better to
        // have a member for the clashing name in
        // UnsupportedNameClashException...

        sal_Int32 nLastSlash = rSourceURL.lastIndexOf( '/' );
        bool bTrailingSlash = false;
        if ( nLastSlash == rSourceURL.getLength() - 1 )
        {
            nLastSlash = rSourceURL.lastIndexOf( '/', nLastSlash );
            bTrailingSlash = true;
        }

        if ( nLastSlash != -1 )
        {
            if ( bTrailingSlash )
                aName = rSourceURL.copy(
                            nLastSlash + 1,
                            rSourceURL.getLength() - nLastSlash - 2 );
            else
                aName = rSourceURL.copy( nLastSlash + 1 );
        }
        else
        {
            aName = rSourceURL;
        }

        // query, fragment present?
        sal_Int32  nPos = aName.indexOf( '?' );
        if ( nPos == -1 )
          nPos = aName.indexOf( '#' );

        if ( nPos != -1 )
          aName = aName.copy( 0, nPos );
    }
    return aName;
}

OUString createDesiredName(
    const ucb::GlobalTransferCommandArgument & rArg )
{
    return createDesiredName( rArg.SourceURL, rArg.NewTitle );
}

OUString createDesiredName(
    const ucb::TransferInfo & rArg )
{
    return createDesiredName( rArg.SourceURL, rArg.NewTitle );
}


enum NameClashContinuation { NOT_HANDLED, ABORT, OVERWRITE, NEW_NAME, UNKNOWN };

NameClashContinuation interactiveNameClashResolve(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv,
    const OUString & rTargetURL,
    const OUString & rClashingName,
    /* [out] */ uno::Any & rException,
    /* [out] */ OUString & rNewName )
{
    rtl::Reference< ucbhelper::SimpleNameClashResolveRequest > xRequest(
        new ucbhelper::SimpleNameClashResolveRequest(
            rTargetURL,  // target folder URL
            rClashingName
            ) );

    rException = xRequest->getRequest();
    if ( xEnv.is() )
    {
        uno::Reference< task::XInteractionHandler > xIH
            = xEnv->getInteractionHandler();
        if ( xIH.is() )
        {

            xIH->handle( xRequest );

            rtl::Reference< ucbhelper::InteractionContinuation >
                xSelection( xRequest->getSelection() );

            if ( xSelection.is() )
            {
                // Handler handled the request.
                uno::Reference< task::XInteractionAbort > xAbort(
                    xSelection.get(), uno::UNO_QUERY );
                if ( xAbort.is() )
                {
                    // Abort.
                    return ABORT;
                }
                else
                {
                    uno::Reference<
                        ucb::XInteractionReplaceExistingData >
                            xReplace(
                                xSelection.get(), uno::UNO_QUERY );
                    if ( xReplace.is() )
                    {
                        // Try again: Replace existing data.
                        return OVERWRITE;
                    }
                    else
                    {
                        uno::Reference<
                            ucb::XInteractionSupplyName >
                                xSupplyName(
                                    xSelection.get(), uno::UNO_QUERY );
                        if ( xSupplyName.is() )
                        {
                            // Try again: Use new name.
                            rNewName = xRequest->getNewName();
                            return NEW_NAME;
                        }
                        else
                        {
                            OSL_FAIL( "Unknown interaction continuation!" );
                            return UNKNOWN;
                        }
                    }
                }
            }
        }
    }
    return NOT_HANDLED;
}

/// @throws uno::RuntimeException
bool setTitle(
        const uno::Reference< ucb::XCommandProcessor > & xCommandProcessor,
        const uno::Reference< ucb::XCommandEnvironment > & xEnv,
        const OUString & rNewTitle )
{
    try
    {
        uno::Sequence< beans::PropertyValue > aPropValues{ { /* Name   */ u"Title"_ustr,
                                                             /* Handle */ -1,
                                                             /* Value  */ uno::Any(rNewTitle),
                                                             /* State  */ {} } };

        ucb::Command aSetPropsCommand(
            u"setPropertyValues"_ustr,
            -1,
            uno::Any( aPropValues ) );

        uno::Any aResult
            = xCommandProcessor->execute( aSetPropsCommand, 0, xEnv );

        uno::Sequence< uno::Any > aErrors;
        aResult >>= aErrors;

        OSL_ENSURE( aErrors.getLength() == 1,
                    "getPropertyValues return value invalid!" );

        if ( aErrors[ 0 ].hasValue() )
        {
            // error occurred.
            OSL_FAIL( "error setting Title property!" );
            return false;
        }
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( uno::Exception const & )
    {
        return false;
    }

    return true;
}

/// @throws uno::Exception
uno::Reference< ucb::XContent > createNew(
                    const TransferCommandContext & rContext,
                    const uno::Reference< ucb::XContent > & xTarget,
                    bool bSourceIsFolder,
                    bool bSourceIsDocument,
                    bool bSourceIsLink )
{


    // (1) Obtain creatable types from target.


    // First, try it using "CreatabeleContentsInfo" property and
    // "createNewContent" command -> the "new" way.

    uno::Reference< ucb::XCommandProcessor > xCommandProcessorT(
                                                    xTarget, uno::UNO_QUERY );
    if ( !xCommandProcessorT.is() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Folder", uno::Any(rContext.aArg.TargetURL)}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_CREATE,
            aArgs,
            rContext.xOrigEnv,
            u"Target is no XCommandProcessor!"_ustr,
            rContext.xProcessor );
        // Unreachable
    }

    uno::Sequence< beans::Property > aPropsToObtain{ makeProperty(u"CreatableContentsInfo"_ustr, -1) };

    ucb::Command aGetPropsCommand(
            u"getPropertyValues"_ustr,
            -1,
            uno::Any( aPropsToObtain ) );

    uno::Reference< sdbc::XRow > xRow;
    xCommandProcessorT->execute( aGetPropsCommand, 0, rContext.xEnv )  >>= xRow;

    uno::Sequence< ucb::ContentInfo > aTypesInfo;
    bool bGotTypesInfo = false;

    if ( xRow.is() )
    {
        uno::Any  aValue = xRow->getObject(
            1, uno::Reference< container::XNameAccess >() );
        if ( aValue.hasValue() && ( aValue >>= aTypesInfo ) )
        {
            bGotTypesInfo = true;
        }
    }

    uno::Reference< ucb::XContentCreator > xCreator;

    if ( !bGotTypesInfo )
    {
        // Second, try it using XContentCreator interface -> the "old" way (not
        // providing the chance to supply an XCommandEnvironment.

        xCreator.set( xTarget, uno::UNO_QUERY );

        if ( !xCreator.is() )
        {
            uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
            {
                {"Folder", uno::Any(rContext.aArg.TargetURL)}
            }));
            ucbhelper::cancelCommandExecution(
                ucb::IOErrorCode_CANT_CREATE,
                aArgs,
                rContext.xOrigEnv,
                u"Target is no XContentCreator!"_ustr,
                rContext.xProcessor );
            // Unreachable
        }

        aTypesInfo  = xCreator->queryCreatableContentsInfo();
    }

    if ( !aTypesInfo.hasElements() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Folder", uno::Any(rContext.aArg.TargetURL)}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_CREATE,
            aArgs,
            rContext.xOrigEnv,
            u"No types creatable!"_ustr,
            rContext.xProcessor );
        // Unreachable
    }

    // (2) Try to find a matching target type for the source object.

    std::function<bool(const sal_Int32)> lCompare;

    if ( rContext.aArg.Operation == ucb::TransferCommandOperation_LINK )
    {
        // Create link
        lCompare = [](const sal_Int32 nAttribs) { return !!( nAttribs & ucb::ContentInfoAttribute::KIND_LINK ); };
    }
    else if ( ( rContext.aArg.Operation == ucb::TransferCommandOperation_COPY ) ||
              ( rContext.aArg.Operation == ucb::TransferCommandOperation_MOVE ) )
    {
        // Copy / Move
        // Is source a link? Create link in target folder then.
        if ( bSourceIsLink )
        {
            lCompare = [](const sal_Int32 nAttribs) { return !!( nAttribs & ucb::ContentInfoAttribute::KIND_LINK ); };
        }
        else
        {
            // (not a and not b) or (a and b)
            // not( a or b) or (a and b)
            lCompare = [bSourceIsFolder, bSourceIsDocument](const sal_Int32 nAttribs) {
                return ( bSourceIsFolder == !!( nAttribs & ucb::ContentInfoAttribute::KIND_FOLDER ) )
                    && ( bSourceIsDocument == !!( nAttribs & ucb::ContentInfoAttribute::KIND_DOCUMENT ) ) ;
            };
        }
    }
    else
    {
        ucbhelper::cancelCommandExecution(
            uno::Any( lang::IllegalArgumentException(
                                    u"Unknown transfer operation!"_ustr,
                                    rContext.xProcessor,
                                    -1 ) ),
                          rContext.xOrigEnv );
        // Unreachable
    }

    uno::Reference< ucb::XContent > xNew;
    auto pTypeInfo = std::find_if(std::cbegin(aTypesInfo), std::cend(aTypesInfo),
        [&lCompare](const ucb::ContentInfo& rTypeInfo) { return lCompare(rTypeInfo.Attributes); });
    if (pTypeInfo != std::cend(aTypesInfo))
    {
        // (3) Create a new, empty object of matched type.

        if ( !xCreator.is() )
        {
            // First, try it using "CreatabeleContentsInfo" property and
            // "createNewContent" command -> the "new" way.
            ucb::Command aCreateNewCommand(
               u"createNewContent"_ustr,
               -1,
               uno::Any( *pTypeInfo ) );

            xCommandProcessorT->execute( aCreateNewCommand, 0, rContext.xEnv )
                >>= xNew;
        }
        else
        {
            // Second, try it using XContentCreator interface -> the "old"
            // way (not providing the chance to supply an XCommandEnvironment.

            xNew = xCreator->createNewContent( *pTypeInfo );
        }

        if ( !xNew.is() )
        {
            uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
            {
                {"Folder", uno::Any(rContext.aArg.TargetURL)}
            }));
            ucbhelper::cancelCommandExecution(
                ucb::IOErrorCode_CANT_CREATE,
                aArgs,
                rContext.xOrigEnv,
                u"createNewContent failed!"_ustr,
                rContext.xProcessor );
            // Unreachable
        }
    }

    return xNew;
}

/// @throws uno::Exception
void transferProperties(
    const TransferCommandContext & rContext,
    const uno::Reference< ucb::XCommandProcessor > & xCommandProcessorS,
    const uno::Reference< ucb::XCommandProcessor > & xCommandProcessorN )
{
    ucb::Command aGetPropertySetInfoCommand(
                u"getPropertySetInfo"_ustr,
                -1,
                uno::Any() );

    uno::Reference< beans::XPropertySetInfo > xInfo;
    xCommandProcessorS->execute( aGetPropertySetInfoCommand, 0, rContext.xEnv )
        >>= xInfo;

    if ( !xInfo.is() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Uri", uno::Any(rContext.aArg.SourceURL)}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            aArgs,
            rContext.xOrigEnv,
            u"Unable to get propertyset info from source object!"_ustr,
            rContext.xProcessor );
        // Unreachable
    }

    uno::Sequence< beans::Property > aAllProps = xInfo->getProperties();

    ucb::Command aGetPropsCommand1(
                u"getPropertyValues"_ustr,
                -1,
                uno::Any( aAllProps ) );

    uno::Reference< sdbc::XRow > xRow1;
    xCommandProcessorS->execute(
        aGetPropsCommand1, 0, rContext.xEnv ) >>= xRow1;

    if ( !xRow1.is() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Uri", uno::Any(rContext.aArg.SourceURL)}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            aArgs,
            rContext.xOrigEnv,
            u"Unable to get properties from source object!"_ustr,
            rContext.xProcessor );
        // Unreachable
    }

    // Assemble data structure for setPropertyValues command.

    // Note: Make room for additional Title and TargetURL too. -> + 2
    uno::Sequence< beans::PropertyValue > aPropValues(
                                                aAllProps.getLength() + 2 );
    auto pPropValues = aPropValues.getArray();

    bool bHasTitle = rContext.aArg.NewTitle.isEmpty();
    bool bHasTargetURL = ( rContext.aArg.Operation
                                != ucb::TransferCommandOperation_LINK );

    sal_Int32 nWritePos = 0;
    for ( sal_Int32 m = 0; m < aAllProps.getLength(); ++m )
    {
        const beans::Property & rCurrProp = aAllProps[ m ];
        beans::PropertyValue & rCurrValue = pPropValues[ nWritePos ];

        uno::Any aValue;

        if ( rCurrProp.Name == "Title" )
        {
            // Supply new title, if given.
            if ( !bHasTitle )
            {
                bHasTitle = true;
                aValue <<= rContext.aArg.NewTitle;
            }
        }
        else if ( rCurrProp.Name == "TargetURL" )
        {
            // Supply source URL as link target for the new link to create.
            if ( !bHasTargetURL )
            {
                bHasTargetURL = true;
                aValue <<= rContext.aArg.SourceURL;
            }
        }

        if ( !aValue.hasValue() )
        {
            try
            {
                aValue = xRow1->getObject(
                            m + 1, uno::Reference< container::XNameAccess >() );
            }
            catch ( sdbc::SQLException const & )
            {
                // Argh! But try to bring things to an end. Perhaps the
                // mad property is not really important...
            }
        }

        if ( aValue.hasValue() )
        {
            rCurrValue.Name   = rCurrProp.Name;
            rCurrValue.Handle = rCurrProp.Handle;
            rCurrValue.Value  = aValue;
//          rCurrValue.State  =

            nWritePos++;
        }
    }

    // Title needed, but not set yet?
    if ( !bHasTitle && !rContext.aArg.NewTitle.isEmpty() )
    {
        pPropValues[ nWritePos ].Name = "Title";
        pPropValues[ nWritePos ].Handle = -1;
        pPropValues[ nWritePos ].Value <<= rContext.aArg.NewTitle;

        nWritePos++;
    }

    // TargetURL needed, but not set yet?
    if ( !bHasTargetURL && ( rContext.aArg.Operation
                                == ucb::TransferCommandOperation_LINK ) )
    {
        pPropValues[ nWritePos ].Name = "TargetURL";
        pPropValues[ nWritePos ].Handle = -1;
        pPropValues[ nWritePos ].Value <<= rContext.aArg.SourceURL;

        nWritePos++;
    }

    aPropValues.realloc( nWritePos );

    // Set properties at new object.

    ucb::Command aSetPropsCommand(
                u"setPropertyValues"_ustr,
                -1,
                uno::Any( aPropValues ) );

    xCommandProcessorN->execute( aSetPropsCommand, 0, rContext.xEnv );

    // @@@ What to do with source props that are not supported by the
    //     new object? addProperty ???
}

/// @throws uno::Exception
uno::Reference< io::XInputStream > getInputStream(
    const TransferCommandContext & rContext,
    const uno::Reference< ucb::XCommandProcessor > & xCommandProcessorS )
{
    uno::Reference< io::XInputStream > xInputStream;


    // (1) Try to get data as XInputStream via XActiveDataSink.


    try
    {
        uno::Reference< io::XActiveDataSink > xSink = new ActiveDataSink;

        ucb::OpenCommandArgument2 aArg;
        aArg.Mode       = ucb::OpenMode::DOCUMENT;
        aArg.Priority   = 0; // unused
        aArg.Sink       = xSink;
        aArg.Properties = uno::Sequence< beans::Property >( 0 ); // unused

        ucb::Command aOpenCommand(
                                u"open"_ustr,
                                -1,
                                uno::Any( aArg ) );

        xCommandProcessorS->execute( aOpenCommand, 0, rContext.xEnv );
        xInputStream = xSink->getInputStream();
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( uno::Exception const & )
    {
        // will be handled below.
    }

    if ( !xInputStream.is() )
    {


        // (2) Try to get data via XOutputStream.


        try
        {
            uno::Reference< io::XOutputStream > xOutputStream( io::Pipe::create(rContext.m_xContext), uno::UNO_QUERY_THROW );

            ucb::OpenCommandArgument2 aArg;
            aArg.Mode       = ucb::OpenMode::DOCUMENT;
            aArg.Priority   = 0; // unused
            aArg.Sink       = xOutputStream;
            aArg.Properties = uno::Sequence< beans::Property >( 0 );

            ucb::Command aOpenCommand(
                                u"open"_ustr,
                                -1,
                                uno::Any( aArg ) );

            xCommandProcessorS->execute( aOpenCommand, 0, rContext.xEnv );

            xInputStream.set( xOutputStream, uno::UNO_QUERY );
        }
        catch ( uno::RuntimeException const & )
        {
            throw;
        }
        catch ( uno::Exception const & )
        {
            OSL_FAIL( "unable to get input stream from document!" );
        }
    }

    return xInputStream;
}

/// @throws uno::Exception
uno::Reference< sdbc::XResultSet > getResultSet(
    const TransferCommandContext & rContext,
    const uno::Reference< ucb::XCommandProcessor > & xCommandProcessorS )
{
    uno::Reference< sdbc::XResultSet > xResultSet;

    ucb::OpenCommandArgument2 aArg;
    aArg.Mode       = ucb::OpenMode::ALL;
    aArg.Priority   = 0; // unused
    aArg.Sink       = nullptr;
    aArg.Properties = { makeProperty(u"IsFolder"_ustr, -1 /* unknown */),
                        makeProperty(u"IsDocument"_ustr, -1 /* unknown */),
                        makeProperty(u"TargetURL"_ustr, -1 /* unknown */) };

    ucb::Command aOpenCommand( u"open"_ustr,
                                     -1,
                                     uno::Any( aArg ) );
    try
    {
        uno::Reference< ucb::XDynamicResultSet > xSet;
        xCommandProcessorS->execute( aOpenCommand, 0, rContext.xEnv ) >>= xSet;

        if ( xSet.is() )
            xResultSet = xSet->getStaticResultSet();
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( uno::Exception const & )
    {
         OSL_FAIL( "unable to get result set from folder!" );
    }

    return xResultSet;
}

/// @throws uno::Exception
void handleNameClashRename(
        const TransferCommandContext & rContext,
        const uno::Reference< ucb::XContent > & xNew,
        const uno::Reference<
            ucb::XCommandProcessor > & xCommandProcessorN,
        const uno::Reference<
            ucb::XCommandProcessor > & xCommandProcessorS,
        /* [inout] */ uno::Reference< io::XInputStream > & xInputStream )
{
    sal_Int32 nTry = 0;

    // Obtain old title.
    uno::Sequence< beans::Property > aProps{ makeProperty(u"Title"_ustr, -1) };

    ucb::Command aGetPropsCommand(
            u"getPropertyValues"_ustr,
            -1,
            uno::Any( aProps ) );

    uno::Reference< sdbc::XRow > xRow;
    xCommandProcessorN->execute( aGetPropsCommand, 0, rContext.xEnv )  >>= xRow;

    if ( !xRow.is() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Uri", uno::Any(xNew->getIdentifier()->getContentIdentifier())}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            aArgs,
            rContext.xOrigEnv,
            u"Unable to get properties from new object!"_ustr,
            rContext.xProcessor );
        // Unreachable
    }

    OUString aOldTitle = xRow->getString( 1 );
    if ( aOldTitle.isEmpty() )
    {
        ucbhelper::cancelCommandExecution(
            uno::Any( beans::UnknownPropertyException(
                            u"Unable to get property 'Title' from new object!"_ustr,
                            rContext.xProcessor ) ),
            rContext.xOrigEnv );
        // Unreachable
    }

    // Some pseudo-intelligence for not destroying file extensions.
    OUString aOldTitlePre;
    OUString aOldTitlePost;
    sal_Int32 nPos = aOldTitle.lastIndexOf( '.' );
    if ( nPos != -1 )
    {
        aOldTitlePre = aOldTitle.copy( 0, nPos );
        aOldTitlePost = aOldTitle.copy( nPos );
    }
    else
        aOldTitlePre = aOldTitle;

    if ( nPos > 0 )
        aOldTitlePre += "_";

    bool bContinue = true;
    do
    {
        nTry++;

        OUString aNewTitle = aOldTitlePre + OUString::number( nTry ) +
            aOldTitlePost;

        // Set new title
        setTitle( xCommandProcessorN, rContext.xEnv, aNewTitle );

        // Retry inserting the content.
        try
        {
            // Previous try may have read from stream. Seek to begin (if
            // optional interface XSeekable is supported) or get a new stream.
            if ( xInputStream.is() )
            {
                uno::Reference< io::XSeekable > xSeekable(
                    xInputStream, uno::UNO_QUERY );
                if ( xSeekable.is() )
                {
                    try
                    {
                        xSeekable->seek( 0 );
                    }
                    catch ( lang::IllegalArgumentException const & )
                    {
                        xInputStream.clear();
                    }
                    catch ( io::IOException const & )
                    {
                        xInputStream.clear();
                    }
                }
                else
                    xInputStream.clear();

                if ( !xInputStream.is() )
                {
                    xInputStream
                        = getInputStream( rContext, xCommandProcessorS );
                    if ( !xInputStream.is() )
                    {
                        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
                        {
                            {"Uri", uno::Any(xNew->getIdentifier()->getContentIdentifier())}
                        }));
                        ucbhelper::cancelCommandExecution(
                            ucb::IOErrorCode_CANT_READ,
                            aArgs,
                            rContext.xOrigEnv,
                            u"Got no data stream from source!"_ustr,
                            rContext.xProcessor );
                        // Unreachable
                    }
                }
            }

            ucb::InsertCommandArgument2 aArg;
            aArg.Data = xInputStream;
            aArg.ReplaceExisting = false;

            ucb::Command aInsertCommand(
                        u"insert"_ustr,
                        -1,
                        uno::Any( aArg ) );

            xCommandProcessorN->execute( aInsertCommand, 0, rContext.xEnv );

            // Success!
            bContinue = false;
        }
        catch ( uno::RuntimeException const & )
        {
            throw;
        }
        catch ( uno::Exception const & )
        {
        }
    }
    while ( bContinue && ( nTry < 50 ) );

    if ( nTry == 50 )
    {
        ucbhelper::cancelCommandExecution(
            uno::Any(
                ucb::UnsupportedNameClashException(
                    u"Unable to resolve name clash!"_ustr,
                    rContext.xProcessor,
                    ucb::NameClash::RENAME ) ),
            rContext.xOrigEnv );
        // Unreachable
    }
}

/// @throws uno::Exception
void globalTransfer_(
        const TransferCommandContext & rContext,
        const uno::Reference< ucb::XContent > & xSource,
        const uno::Reference< ucb::XContent > & xTarget,
        const uno::Reference< sdbc::XRow > & xSourceProps )
{
    // IsFolder: property is required.
    bool bSourceIsFolder = xSourceProps->getBoolean( 1 );
    if ( !bSourceIsFolder && xSourceProps->wasNull() )
    {
        ucbhelper::cancelCommandExecution(
            uno::Any( beans::UnknownPropertyException(
                            u"Unable to get property 'IsFolder' from source object!"_ustr,
                            rContext.xProcessor ) ),
            rContext.xOrigEnv );
        // Unreachable
    }

    // IsDocument: property is required.
    bool bSourceIsDocument = xSourceProps->getBoolean( 2 );
    if ( !bSourceIsDocument && xSourceProps->wasNull() )
    {
        ucbhelper::cancelCommandExecution(
            uno::Any( beans::UnknownPropertyException(
                            u"Unable to get property 'IsDocument' from source object!"_ustr,
                            rContext.xProcessor ) ),
            rContext.xOrigEnv );
        // Unreachable
    }

    // TargetURL: property is optional.
    bool bSourceIsLink = !xSourceProps->getString( 3 ).isEmpty();


    // (1) Try to find a matching target type for the source object and
    //     create a new, empty object of that type.


    uno::Reference< ucb::XContent > xNew = createNew( rContext,
                                                      xTarget,
                                                      bSourceIsFolder,
                                                      bSourceIsDocument,
                                                      bSourceIsLink );
    if ( !xNew.is() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Folder", uno::Any(rContext.aArg.TargetURL)}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_CREATE,
            aArgs,
            rContext.xOrigEnv,
            u"No matching content type at target!"_ustr,
            rContext.xProcessor );
        // Unreachable
    }


    // (2) Transfer property values from source to new object.


    uno::Reference< ucb::XCommandProcessor > xCommandProcessorN(
                                                    xNew, uno::UNO_QUERY );
    if ( !xCommandProcessorN.is() )
    {
        uno::Any aProps(beans::PropertyValue(
                                  u"Uri"_ustr,
                                  -1,
                                  uno::Any(
                                      xNew->getIdentifier()->
                                                getContentIdentifier()),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_WRITE,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xOrigEnv,
            u"New content is not a XCommandProcessor!"_ustr,
            rContext.xProcessor );
        // Unreachable
    }

    // Obtain all properties from source.

    uno::Reference< ucb::XCommandProcessor > xCommandProcessorS(
                                                    xSource, uno::UNO_QUERY );
    if ( !xCommandProcessorS.is() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Uri", uno::Any(rContext.aArg.SourceURL)}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            aArgs,
            rContext.xOrigEnv,
            u"Source content is not a XCommandProcessor!"_ustr,
            rContext.xProcessor );
        // Unreachable
    }

    transferProperties( rContext, xCommandProcessorS, xCommandProcessorN );


    // (3) Try to obtain a data stream from source.


    uno::Reference< io::XInputStream > xInputStream;

    if ( bSourceIsDocument && ( rContext.aArg.Operation
                                != ucb::TransferCommandOperation_LINK ) )
        xInputStream = getInputStream( rContext, xCommandProcessorS );


    // (4) Try to obtain a resultset (children) from source.


    uno::Reference< sdbc::XResultSet > xResultSet;

    if ( bSourceIsFolder && ( rContext.aArg.Operation
                                != ucb::TransferCommandOperation_LINK ) )
        xResultSet = getResultSet( rContext, xCommandProcessorS );


    // (5) Insert (store) new content.


    ucb::InsertCommandArgument2 aArg;
    aArg.Data = xInputStream;
    aArg.MimeType = rContext.aArg.MimeType;
    aArg.DocumentId = rContext.aArg.DocumentId;

    switch ( rContext.aArg.NameClash )
    {
        case ucb::NameClash::OVERWRITE:
            aArg.ReplaceExisting = true;
            break;

        case ucb::NameClash::ERROR:
        case ucb::NameClash::RENAME:
        case ucb::NameClash::KEEP: // deprecated
        case ucb::NameClash::ASK:
            aArg.ReplaceExisting = false;
            break;

        default:
            aArg.ReplaceExisting = false;
            OSL_FAIL( "Unknown nameclash directive!" );
            break;
    }

    OUString aDesiredName = createDesiredName( rContext.aArg );

    bool bRetry;
    do
    {
        bRetry = false;

        try
        {
            ucb::Command aInsertCommand(
                                    u"insert"_ustr,
                                    -1,
                                    uno::Any( aArg ) );

            xCommandProcessorN->execute( aInsertCommand, 0, rContext.xEnv );
        }
        catch ( ucb::UnsupportedNameClashException const & exc )
        {
            OSL_ENSURE( !aArg.ReplaceExisting,
                        "BUG: UnsupportedNameClashException not allowed here!" );

            if (exc.NameClash != ucb::NameClash::ERROR) {
                OSL_FAIL( "BUG: NameClash::ERROR expected!" );
            }

            // No chance to solve name clashes, because I'm not able to detect
            // whether there is one.
            throw ucb::UnsupportedNameClashException(
                    u"Unable to resolve name clashes, no chance to detect "
                    "that there is one!"_ustr,
                    rContext.xProcessor,
                    rContext.aArg.NameClash );
        }
        catch ( ucb::NameClashException const & )
        {
            // The 'insert' command throws a NameClashException if the parameter
            // ReplaceExisting of the command's argument was set to false and
            // there exists a resource with a clashing name in the target folder
            // of the operation.

            // 'insert' command has no direct support for name clashes other
            // than ERROR ( ReplaceExisting == false ) and OVERWRITE
            // ( ReplaceExisting == true ). So we have to implement the
            // other name clash handling directives on top of the content.

            // @@@ 'insert' command should be extended that it accepts a
            //     name clash handling directive, exactly like 'transfer' command.

            switch ( rContext.aArg.NameClash )
            {
                case ucb::NameClash::OVERWRITE:
                {
                    ucbhelper::cancelCommandExecution(
                        uno::Any(
                            ucb::UnsupportedNameClashException(
                                u"BUG: insert + replace == true MUST NOT "
                                "throw NameClashException."_ustr,
                                rContext.xProcessor,
                                rContext.aArg.NameClash ) ),
                        rContext.xOrigEnv );
                    [[fallthrough]]; // Unreachable
                }

                case ucb::NameClash::ERROR:
                    throw;

                case ucb::NameClash::RENAME:
                {
                    // "invent" a new valid title.
                    handleNameClashRename( rContext,
                                           xNew,
                                           xCommandProcessorN,
                                           xCommandProcessorS,
                                           xInputStream );
                    break;
                }

                case ucb::NameClash::ASK:
                    {
                        uno::Any aExc;
                        OUString aNewTitle;
                        NameClashContinuation eCont
                            = interactiveNameClashResolve(
                                rContext.xOrigEnv, // always use original environment!
                                rContext.aArg.TargetURL, // target folder URL
                                aDesiredName,
                                aExc,
                                aNewTitle );

                        switch ( eCont )
                        {
                            case NOT_HANDLED:
                                // Not handled.
                                cppu::throwException( aExc );
                                [[fallthrough]]; // break;

                            case UNKNOWN:
                                // Handled, but not clear, how...
                                // fall through intended.

                            case ABORT:
                                throw ucb::CommandFailedException(
                                    u"abort requested via interaction "
                                    "handler"_ustr,
                                    uno::Reference< uno::XInterface >(),
                                    aExc );
    //                            break;

                            case OVERWRITE:
                                OSL_ENSURE( !aArg.ReplaceExisting,
                                            "Hu? ReplaceExisting already true?"
                                          );
                                aArg.ReplaceExisting = true;
                                bRetry = true;
                                break;

                            case NEW_NAME:
                            {
                                // set new name -> set "Title" property...
                                if ( setTitle( xCommandProcessorN,
                                               rContext.xEnv,
                                               aNewTitle ) )
                                {
                                    // remember suggested title...
                                    aDesiredName = aNewTitle;

                                    // ... and try again.
                                    bRetry = true;
                                }
                                else
                                {
                                    // error setting title. Abort.
                                    throw ucb::CommandFailedException(
                                        u"error setting Title property!"_ustr,
                                        uno::Reference< uno::XInterface >(),
                                        aExc );
                                }
                                break;
                            }
                        }

                        OSL_ENSURE( bRetry, "bRetry must be true here!!!" );
                    }
                    break;

                case ucb::NameClash::KEEP: // deprecated
                default:
                {
                    ucbhelper::cancelCommandExecution(
                        uno::Any(
                            ucb::UnsupportedNameClashException(
                                u"default action, don't know how to "
                                "handle name clash"_ustr,
                                rContext.xProcessor,
                                rContext.aArg.NameClash ) ),
                        rContext.xOrigEnv );
                    // Unreachable
                }
            }
        }
    }
    while ( bRetry );


    // (6) Process children of source.


    if ( xResultSet.is() )
    {
        try
        {
            // Iterate over children...

            uno::Reference< sdbc::XRow > xChildRow(
                                            xResultSet, uno::UNO_QUERY );

            if ( !xChildRow.is() )
            {
                uno::Any aProps(
                             beans::PropertyValue(
                                 u"Uri"_ustr,
                                 -1,
                                 uno::Any(rContext.aArg.SourceURL),
                                 beans::PropertyState_DIRECT_VALUE));
                ucbhelper::cancelCommandExecution(
                    ucb::IOErrorCode_CANT_READ,
                    uno::Sequence< uno::Any >(&aProps, 1),
                    rContext.xOrigEnv,
                    u"Unable to get properties from children of source!"_ustr,
                    rContext.xProcessor );
                // Unreachable
            }

            uno::Reference< ucb::XContentAccess > xChildAccess(
                                                xResultSet, uno::UNO_QUERY );

            if ( !xChildAccess.is() )
            {
                uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
                {
                    {"Uri", uno::Any(rContext.aArg.SourceURL)}
                }));
                ucbhelper::cancelCommandExecution(
                    ucb::IOErrorCode_CANT_READ,
                    aArgs,
                    rContext.xOrigEnv,
                    u"Unable to get children of source!"_ustr,
                    rContext.xProcessor );
                // Unreachable
            }

            if ( xResultSet->first() )
            {
                ucb::GlobalTransferCommandArgument2 aTransArg(
                        rContext.aArg.Operation,
                        OUString(),              // SourceURL; filled later
                        xNew->getIdentifier()
                            ->getContentIdentifier(), // TargetURL
                        OUString(),              // NewTitle;
                        rContext.aArg.NameClash,
                        rContext.aArg.MimeType,
                        rContext.aArg.DocumentId);

                TransferCommandContext aSubCtx(
                        rContext.m_xContext,
                        rContext.xProcessor,
                        rContext.xEnv,
                        rContext.xOrigEnv,
                        std::move(aTransArg) );
                do
                {
                    uno::Reference< ucb::XContent > xChild
                                        = xChildAccess->queryContent();
                    if ( xChild.is() )
                    {
                        // Recursion!

                        aSubCtx.aArg.SourceURL
                            = xChild->getIdentifier()->getContentIdentifier();

                        globalTransfer_( aSubCtx,
                                         xChild,
                                         xNew,
                                         xChildRow );
                    }
                }
                while ( xResultSet->next() );
            }
        }
        catch ( sdbc::SQLException const & )
        {
        }
    }

    try {
        uno::Reference< ucb::XCommandProcessor > xcp(
            xTarget, uno::UNO_QUERY );

        uno::Any aAny;
        uno::Reference< ucb::XCommandInfo > xci;
        if(xcp.is())
            aAny =
                xcp->execute(
                    ucb::Command(
                        u"getCommandInfo"_ustr,
                        -1,
                        uno::Any()),
                    0,
                    rContext.xEnv );

        static constexpr OUString cmdName(u"flush"_ustr);
        if((aAny >>= xci) && xci->hasCommandByName(cmdName))
            xcp->execute(
                ucb::Command(
                    cmdName,
                    -1,
                    uno::Any()) ,
                0,
                rContext.xEnv );
    }
    catch( uno::Exception const & )
    {
    }
}

} /* namespace */


// UniversalContentBroker implementation ( XCommandProcessor commands ).


uno::Reference< ucb::XCommandInfo >
UniversalContentBroker::getCommandInfo()
{
    return uno::Reference< ucb::XCommandInfo >( new CommandProcessorInfo() );
}


void UniversalContentBroker::globalTransfer(
            const ucb::GlobalTransferCommandArgument2 & rArg,
            const uno::Reference< ucb::XCommandEnvironment > & xEnv )
{
    // Use own command environment with own interaction handler intercepting
    // some interaction requests that shall not be handled by the user-supplied
    // interaction handler.
    uno::Reference< ucb::XCommandEnvironment > xLocalEnv;
    if (xEnv.is())
    {
        xLocalEnv.set( ucb::CommandEnvironment::create(
               m_xContext,
               new InteractionHandlerProxy( xEnv->getInteractionHandler() ),
               xEnv->getProgressHandler() ) );
    }


    // (1) Try to transfer the content using 'transfer' command.


    uno::Reference< ucb::XContent > xTarget;
    uno::Reference< ucb::XContentIdentifier > xId
            = createContentIdentifier( rArg.TargetURL );
    if ( xId.is() )
    {
        try
        {
            xTarget = queryContent( xId );
        }
        catch ( ucb::IllegalIdentifierException const & )
        {
        }
    }

    if ( !xTarget.is() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Uri", uno::Any(rArg.TargetURL)}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            aArgs,
            xEnv,
            u"Can't instantiate target object!"_ustr,
            this );
        // Unreachable
    }

    if ( ( rArg.Operation == ucb::TransferCommandOperation_COPY ) ||
         ( rArg.Operation == ucb::TransferCommandOperation_MOVE ) )
    {
        uno::Reference< ucb::XCommandProcessor > xCommandProcessor(
                                                    xTarget, uno::UNO_QUERY );
        if ( !xCommandProcessor.is() )
        {
            uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
            {
                {"Uri", uno::Any(rArg.TargetURL)}
            }));
            ucbhelper::cancelCommandExecution(
                ucb::IOErrorCode_CANT_READ,
                aArgs,
                xEnv,
                u"Target content is not a XCommandProcessor!"_ustr,
                this );
            // Unreachable
        }

        ucb::TransferInfo2 aTransferArg(
            ( rArg.Operation
                == ucb::TransferCommandOperation_MOVE ), // MoveData
            rArg.SourceURL,
            rArg.NewTitle,
            rArg.NameClash,
            rArg.MimeType );

        bool bRetry;
        do
        {
            bRetry = false;

            try
            {
                ucb::Command aCommand(
                    u"transfer"_ustr, // Name
                    -1,                                           // Handle
                    uno::Any( aTransferArg ) );               // Argument

                xCommandProcessor->execute( aCommand, 0, xLocalEnv );

                // Command succeeded. We're done.
                return;
            }
            catch ( ucb::InteractiveBadTransferURLException const & )
            {
                // Source URL is not supported by target. Try to transfer
                // the content "manually".
            }
            catch ( ucb::UnsupportedCommandException const & )
            {
                // 'transfer' command is not supported by commandprocessor.
                // Try to transfer manually.
            }
            catch ( ucb::UnsupportedNameClashException const & exc )
            {
                OSL_ENSURE( aTransferArg.NameClash == exc.NameClash,
                            "nameclash mismatch!" );
                if ( exc.NameClash == ucb::NameClash::ASK )
                {
                    // Try to detect a name clash by invoking "transfer" with
                    // NameClash::ERROR.
                    try
                    {
                        ucb::TransferInfo2 aTransferArg1(
                            aTransferArg.MoveData,
                            aTransferArg.SourceURL,
                            aTransferArg.NewTitle,
                            ucb::NameClash::ERROR,
                            aTransferArg.MimeType );

                        ucb::Command aCommand1(
                            u"transfer"_ustr,
                            -1,
                            uno::Any( aTransferArg1 ) );

                        xCommandProcessor->execute( aCommand1, 0, xLocalEnv );

                        // Command succeeded. We're done.
                        return;
                    }
                    catch ( ucb::UnsupportedNameClashException const & )
                    {
                        // No chance to solve name clashes, because I'm not
                        // able to detect whether there is one.
                        throw exc; // Not just 'throw;'!
                    }
                    catch ( ucb::NameClashException const & )
                    {
                        // There's a clash. Use interaction handler to solve it.

                        uno::Any aExc;
                        OUString aNewTitle;
                        NameClashContinuation eCont
                            = interactiveNameClashResolve(
                                xEnv, // always use original environment!
                                rArg.TargetURL,  // target folder URL
                                createDesiredName(
                                  aTransferArg ),   // clashing name
                                aExc,
                                aNewTitle );

                        switch ( eCont )
                        {
                            case NOT_HANDLED:
                                // Not handled.
                                cppu::throwException( aExc );
                                [[fallthrough]]; // break;

                            case UNKNOWN:
                                // Handled, but not clear, how...
                                // fall through intended.

                            case ABORT:
                                throw ucb::CommandFailedException(
                                    u"abort requested via interaction "
                                    "handler"_ustr,
                                    uno::Reference< uno::XInterface >(),
                                    aExc );
//                                break;

                            case OVERWRITE:
                                aTransferArg.NameClash
                                    = ucb::NameClash::OVERWRITE;
                                bRetry = true;
                                break;

                            case NEW_NAME:
                                aTransferArg.NewTitle = aNewTitle;
                                bRetry = true;
                                break;
                        }

                        OSL_ENSURE( bRetry, "bRetry must be true here!!!" );
                    }
                }
                else
                {
                    throw;
                }
            }
        }
        while ( bRetry );
    }


    // (2) Try to transfer the content "manually".


    uno::Reference< ucb::XContent > xSource;
    try
    {
        uno::Reference< ucb::XContentIdentifier > xId2
            = createContentIdentifier( rArg.SourceURL );
        if ( xId2.is() )
            xSource = queryContent( xId2 );
    }
    catch ( ucb::IllegalIdentifierException const & )
    {
        // Error handling via "if ( !xSource.is() )" below.
    }

    if ( !xSource.is() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Uri", uno::Any(rArg.SourceURL)}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            aArgs,
            xEnv,
            u"Can't instantiate source object!"_ustr,
            this );
        // Unreachable
    }

    uno::Reference< ucb::XCommandProcessor > xCommandProcessor(
                                                xSource, uno::UNO_QUERY );
    if ( !xCommandProcessor.is() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Uri", uno::Any(rArg.SourceURL)}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            aArgs,
            xEnv,
            u"Source content is not a XCommandProcessor!"_ustr,
            this );
        // Unreachable
    }

    // Obtain interesting property values from source...

    uno::Sequence< beans::Property > aProps{ makeProperty(u"IsFolder"_ustr, -1 /* unknown */),
                                             makeProperty(u"IsDocument"_ustr, -1 /* unknown */),
                                             makeProperty(u"TargetURL"_ustr, -1 /* unknown */),
                                             makeProperty(u"BaseURI"_ustr, -1 /* unknown */) };

    ucb::Command aGetPropsCommand(
                u"getPropertyValues"_ustr,
                -1,
                uno::Any( aProps ) );

    uno::Reference< sdbc::XRow > xRow;
    xCommandProcessor->execute( aGetPropsCommand, 0, xLocalEnv ) >>= xRow;

    if ( !xRow.is() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Uri", uno::Any(rArg.SourceURL)}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            aArgs,
            xEnv,
            u"Unable to get properties from source object!"_ustr,
            this );
        // Unreachable
    }

    TransferCommandContext aTransferCtx(
        m_xContext, this, xLocalEnv, xEnv, rArg );

    if ( rArg.NewTitle.isEmpty() )
    {
        // BaseURI: property is optional.
        OUString aBaseURI( xRow->getString( 4 ) );
        if ( !aBaseURI.isEmpty() )
        {
            aTransferCtx.aArg.NewTitle
                = createDesiredName( aBaseURI, OUString() );
        }
    }

    // Do it!
    globalTransfer_( aTransferCtx, xSource, xTarget, xRow );


    // (3) Delete source, if operation is MOVE.


    if ( rArg.Operation != ucb::TransferCommandOperation_MOVE )
        return;

    try
    {
        ucb::Command aCommand(
            u"delete"_ustr,                   // Name
            -1,                         // Handle
            uno::Any( true ) );     // Argument

        xCommandProcessor->execute( aCommand, 0, xLocalEnv );
    }
    catch ( uno::Exception const & )
    {
        OSL_FAIL( "Cannot delete source object!" );
        throw;
    }
}

uno::Any UniversalContentBroker::checkIn( const ucb::CheckinArgument& rArg,
            const uno::Reference< ucb::XCommandEnvironment >& xEnv )
{
    uno::Any aRet;
    // Use own command environment with own interaction handler intercepting
    // some interaction requests that shall not be handled by the user-supplied
    // interaction handler.
    uno::Reference< ucb::XCommandEnvironment > xLocalEnv;
    if (xEnv.is())
    {
        xLocalEnv.set( ucb::CommandEnvironment::create(
               m_xContext,
               new InteractionHandlerProxy( xEnv->getInteractionHandler() ),
               xEnv->getProgressHandler() ) );
    }

    uno::Reference< ucb::XContent > xTarget;
    uno::Reference< ucb::XContentIdentifier > xId
            = createContentIdentifier( rArg.TargetURL );
    if ( xId.is() )
    {
        try
        {
            xTarget = queryContent( xId );
        }
        catch ( ucb::IllegalIdentifierException const & )
        {
        }
    }

    if ( !xTarget.is() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Uri", uno::Any(rArg.TargetURL)}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            aArgs,
            xEnv,
            u"Can't instantiate target object!"_ustr,
            this );
        // Unreachable
    }

    uno::Reference< ucb::XCommandProcessor > xCommandProcessor(
                                                xTarget, uno::UNO_QUERY );
    if ( !xCommandProcessor.is() )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"Uri", uno::Any(rArg.TargetURL)}
        }));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            aArgs,
            xEnv,
            u"Target content is not a XCommandProcessor!"_ustr,
            this );
        // Unreachable
    }

    try
    {
        ucb::Command aCommand(
            u"checkin"_ustr, -1,
            uno::Any( rArg ) );

        aRet = xCommandProcessor->execute( aCommand, 0, xLocalEnv );
    }
    catch ( ucb::UnsupportedCommandException const & )
    {
        // 'checkin' command is not supported by commandprocessor:
        // ignore.
    }
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
