/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ucbcmds.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-04 14:31:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CPPUHELPER_EXC_HLP_HXX_
#include <cppuhelper/exc_hlp.hxx>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef __COM_SUN_STAR_LANG_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDFAILEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandFailedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTINFOATTRIBUTE_HPP_
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_GLOBALTRANSFERCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/GlobalTransferCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INSERTCOMMANDARGUMENT_HPP_
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_INTERACTIVEBADTRANSFRERURLEXCEPTION_HPP_
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASHEXCEPTION_HPP_
#include <com/sun/star/ucb/NameClashException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_UNSUPPORTEDNAMECLASHEXCEPTION_HPP_
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDINFO_HPP_
#include <com/sun/star/ucb/XCommandInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTCREATOR_HPP_
#include <com/sun/star/ucb/XContentCreator.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XDYNAMICRESULTSET_HPP_
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _UCBHELPER_CANCELCOMMANDEXECUTION_HXX_
#include <ucbhelper/cancelcommandexecution.hxx>
#endif
#ifndef _UCBHELPER_SIMPLENAMECLASHRESOLVEREQUEST_HXX
#include <ucbhelper/simplenameclashresolverequest.hxx>
#endif

#ifndef _UCBCMDS_HXX
#include "ucbcmds.hxx"
#endif
#ifndef _UCB_HXX
#include "ucb.hxx"
#endif

using namespace com::sun::star;

namespace ucb_commands
{

//=========================================================================
//
// struct TransferCommandContext.
//
//=========================================================================

struct TransferCommandContext
{
    uno::Reference< lang::XMultiServiceFactory > xSMgr;
    uno::Reference< ucb::XCommandProcessor >     xProcessor;
    uno::Reference< ucb::XCommandEnvironment >   xEnv;
    uno::Reference< ucb::XCommandEnvironment >   xOrigEnv;
    ucb::GlobalTransferCommandArgument           aArg;

    TransferCommandContext(
        const uno::Reference< lang::XMultiServiceFactory > & rxSMgr,
        const uno::Reference< ucb::XCommandProcessor > & rxProcessor,
        const uno::Reference< ucb::XCommandEnvironment > & rxEnv,
        const uno::Reference< ucb::XCommandEnvironment > & rxOrigEnv,
        const ucb::GlobalTransferCommandArgument & rArg )
    : xSMgr( rxSMgr ), xProcessor( rxProcessor ), xEnv( rxEnv ),
      xOrigEnv( rxOrigEnv ), aArg( rArg ) {}
};

//=========================================================================
//
// class InteractionHandlerProxy.
//
//=========================================================================

class InteractionHandlerProxy :
    public cppu::WeakImplHelper1< task::XInteractionHandler >
{
    uno::Reference< task::XInteractionHandler > m_xOrig;

public:
    InteractionHandlerProxy(
        const uno::Reference< task::XInteractionHandler > & xOrig )
    : m_xOrig( xOrig ) {}

    // XInteractionHandler methods.
    virtual void SAL_CALL handle(
            const uno::Reference< task::XInteractionRequest >& Request )
        throw ( uno::RuntimeException );
};

//=========================================================================
// virtual
void SAL_CALL InteractionHandlerProxy::handle(
            const uno::Reference< task::XInteractionRequest >& Request )
    throw ( uno::RuntimeException )
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

//=========================================================================
//
// class CommandEnvironment.
//
//=========================================================================

class CommandEnvironment :
    public cppu::WeakImplHelper1< ucb::XCommandEnvironment >
{
    uno::Reference< task::XInteractionHandler > m_xIH;
    uno::Reference< ucb::XProgressHandler >     m_xPH;

public:
    CommandEnvironment(
        const uno::Reference< task::XInteractionHandler > & xIH,
        const uno::Reference< ucb::XProgressHandler > & xPH )
    : m_xIH( xIH ), m_xPH( xPH ) {}

    // XCommandEnvironment methods.
    virtual uno::Reference< task::XInteractionHandler > SAL_CALL
    getInteractionHandler()
        throw ( uno::RuntimeException );
    virtual uno::Reference< ucb::XProgressHandler > SAL_CALL
    getProgressHandler()
        throw ( uno::RuntimeException );
};

//=========================================================================
// virtual
uno::Reference< task::XInteractionHandler > SAL_CALL
CommandEnvironment::getInteractionHandler()
    throw ( uno::RuntimeException )
{
    return m_xIH;
}

//=========================================================================
// virtual
uno::Reference< ucb::XProgressHandler > SAL_CALL
CommandEnvironment::getProgressHandler()
    throw ( uno::RuntimeException )
{
    return m_xPH;
}

//=========================================================================
//
// class ActiveDataSink.
//
//=========================================================================

class ActiveDataSink : public cppu::WeakImplHelper1< io::XActiveDataSink >
{
    uno::Reference< io::XInputStream > m_xStream;

public:
    // XActiveDataSink methods.
    virtual void SAL_CALL setInputStream(
                        const uno::Reference< io::XInputStream >& aStream )
        throw( uno::RuntimeException );
    virtual uno::Reference< io::XInputStream > SAL_CALL getInputStream()
        throw( uno::RuntimeException );
};

//=========================================================================
// virtual
void SAL_CALL ActiveDataSink::setInputStream(
                        const uno::Reference< io::XInputStream >& aStream )
    throw( uno::RuntimeException )
{
    m_xStream = aStream;
}

//=========================================================================
// virtual
uno::Reference< io::XInputStream > SAL_CALL ActiveDataSink::getInputStream()
    throw( uno::RuntimeException )
{
    return m_xStream;
}

//=========================================================================
//
// class CommandProcessorInfo.
//
//=========================================================================

class CommandProcessorInfo :
    public cppu::WeakImplHelper1< ucb::XCommandInfo >
{
    uno::Sequence< ucb::CommandInfo > * m_pInfo;

public:
    CommandProcessorInfo();
    virtual ~CommandProcessorInfo();

    // XCommandInfo methods
    virtual uno::Sequence< ucb::CommandInfo > SAL_CALL getCommands()
        throw( uno::RuntimeException );
    virtual ucb::CommandInfo SAL_CALL
    getCommandInfoByName( const rtl::OUString& Name )
        throw( ucb::UnsupportedCommandException, uno::RuntimeException );
    virtual ucb::CommandInfo SAL_CALL
    getCommandInfoByHandle( sal_Int32 Handle )
        throw( ucb::UnsupportedCommandException, uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasCommandByName( const rtl::OUString& Name )
        throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasCommandByHandle( sal_Int32 Handle )
        throw( uno::RuntimeException );
};

//=========================================================================
CommandProcessorInfo::CommandProcessorInfo()
{
    m_pInfo = new uno::Sequence< ucb::CommandInfo >( 2 );

    (*m_pInfo)[ 0 ]
        = ucb::CommandInfo(
            rtl::OUString::createFromAscii( GETCOMMANDINFO_NAME ), // Name
            GETCOMMANDINFO_HANDLE, // Handle
            getCppuVoidType() ); // ArgType
    (*m_pInfo)[ 1 ]
        = ucb::CommandInfo(
            rtl::OUString::createFromAscii( GLOBALTRANSFER_NAME ), // Name
            GLOBALTRANSFER_HANDLE, // Handle
            getCppuType(
                static_cast<
                    ucb::GlobalTransferCommandArgument * >( 0 ) ) ); // ArgType
}

//=========================================================================
// virtual
CommandProcessorInfo::~CommandProcessorInfo()
{
    delete m_pInfo;
}

//=========================================================================
// virtual
uno::Sequence< ucb::CommandInfo > SAL_CALL
CommandProcessorInfo::getCommands()
    throw( uno::RuntimeException )
{
    return uno::Sequence< ucb::CommandInfo >( *m_pInfo );
}

//=========================================================================
// virtual
ucb::CommandInfo SAL_CALL
CommandProcessorInfo::getCommandInfoByName( const rtl::OUString& Name )
    throw( ucb::UnsupportedCommandException, uno::RuntimeException )
{
    for ( sal_Int32 n = 0; n < m_pInfo->getLength(); ++n )
    {
        if ( (*m_pInfo)[ n ].Name == Name )
            return ucb::CommandInfo( (*m_pInfo)[ n ] );
    }

    throw ucb::UnsupportedCommandException();
}

//=========================================================================
// virtual
ucb::CommandInfo SAL_CALL
CommandProcessorInfo::getCommandInfoByHandle( sal_Int32 Handle )
    throw( ucb::UnsupportedCommandException, uno::RuntimeException )
{
    for ( sal_Int32 n = 0; n < m_pInfo->getLength(); ++n )
    {
        if ( (*m_pInfo)[ n ].Handle == Handle )
            return ucb::CommandInfo( (*m_pInfo)[ n ] );
    }

    throw ucb::UnsupportedCommandException();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL CommandProcessorInfo::hasCommandByName(
                                                const rtl::OUString& Name )
    throw( uno::RuntimeException )
{
    for ( sal_Int32 n = 0; n < m_pInfo->getLength(); ++n )
    {
        if ( (*m_pInfo)[ n ].Name == Name )
            return sal_True;
    }

    return sal_False;
}

//=========================================================================
// virtual
sal_Bool SAL_CALL CommandProcessorInfo::hasCommandByHandle( sal_Int32 Handle )
    throw( uno::RuntimeException )
{
    for ( sal_Int32 n = 0; n < m_pInfo->getLength(); ++n )
    {
        if ( (*m_pInfo)[ n ].Handle == Handle )
            return sal_True;
    }

    return sal_False;
}

//=========================================================================
//=========================================================================
//=========================================================================

static rtl::OUString createDesiredName(
    const rtl::OUString & rSourceURL, const rtl::OUString & rNewTitle )
{
    rtl::OUString aName( rNewTitle );
    if ( aName.getLength() == 0 )
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
    }
    return rtl::OUString( aName );
}

static rtl::OUString createDesiredName(
    const ucb::GlobalTransferCommandArgument & rArg )
{
    return createDesiredName( rArg.SourceURL, rArg.NewTitle );
}

static rtl::OUString createDesiredName(
    const ucb::TransferInfo & rArg )
{
    return createDesiredName( rArg.SourceURL, rArg.NewTitle );
}

//=========================================================================
enum NameClashContinuation { NOT_HANDLED, ABORT, OVERWRITE, NEW_NAME, UNKNOWN };

static NameClashContinuation interactiveNameClashResolve(
    const uno::Reference< ucb::XCommandEnvironment > & xEnv,
    const rtl::OUString & rTargetURL,
    const rtl::OUString & rClashingName,
    /* [out] */ uno::Any & rException,
    /* [out] */ rtl::OUString & rNewName )
{
    rtl::Reference< ucbhelper::SimpleNameClashResolveRequest > xRequest(
        new ucbhelper::SimpleNameClashResolveRequest(
            rTargetURL,  // target folder URL
            rClashingName,   // clashing name
            rtl::OUString(), // no proposal for new name
            sal_True /* bSupportsOverwriteData */ ) );

    rException = xRequest->getRequest();
    if ( xEnv.is() )
    {
        uno::Reference< task::XInteractionHandler > xIH
            = xEnv->getInteractionHandler();
        if ( xIH.is() )
        {

            xIH->handle( xRequest.get() );

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
                            OSL_ENSURE( sal_False,
                                        "Unknown interaction continuation!" );
                            return UNKNOWN;
                        }
                    }
                }
            }
        }
    }
    return NOT_HANDLED;
}

//=========================================================================
static bool setTitle(
        const uno::Reference< ucb::XCommandProcessor > & xCommandProcessor,
        const uno::Reference< ucb::XCommandEnvironment > & xEnv,
        const rtl::OUString & rNewTitle )
    throw( uno::RuntimeException )
{
    try
    {
        uno::Sequence< beans::PropertyValue > aPropValues( 1 );
        aPropValues[ 0 ].Name
            = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) );
        aPropValues[ 0 ].Handle = -1;
        aPropValues[ 0 ].Value  = uno::makeAny( rNewTitle );

        ucb::Command aSetPropsCommand(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "setPropertyValues" ) ),
            -1,
            uno::makeAny( aPropValues ) );

        uno::Any aResult
            = xCommandProcessor->execute( aSetPropsCommand, 0, xEnv );

        uno::Sequence< uno::Any > aErrors;
        aResult >>= aErrors;

        OSL_ENSURE( aErrors.getLength() == 1,
                    "getPropertyValues return value invalid!" );

        if ( aErrors[ 0 ].hasValue() )
        {
            // error occured.
            OSL_ENSURE( sal_False, "error setting Title property!" );
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

//=========================================================================
static uno::Reference< ucb::XContent > createNew(
                    const TransferCommandContext & rContext,
                    const uno::Reference< ucb::XContent > & xTarget,
                    sal_Bool bSourceIsFolder,
                    sal_Bool bSourceIsDocument,
                    sal_Bool bSourceIsLink )
    throw( uno::Exception )
{
    //////////////////////////////////////////////////////////////////////
    //
    // (1) Obtain creatable types from target.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< ucb::XContentCreator > xCreator(
                                                    xTarget, uno::UNO_QUERY );

    if ( !xCreator.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Folder")),
                                  -1,
                                  uno::makeAny(rContext.aArg.TargetURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_CREATE,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xOrigEnv,
            rtl::OUString::createFromAscii( "Target is no XContentCreator!" ),
            rContext.xProcessor );
        // Unreachable
    }

    uno::Sequence< ucb::ContentInfo > aTypesInfo
                            = xCreator->queryCreatableContentsInfo();

    sal_Int32 nCount = aTypesInfo.getLength();
    if ( !nCount )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Folder")),
                                  -1,
                                  uno::makeAny(rContext.aArg.TargetURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_CREATE,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xOrigEnv,
            rtl::OUString::createFromAscii( "No types creatable!" ),
            rContext.xProcessor );
        // Unreachable
    }

    //////////////////////////////////////////////////////////////////////
    //
    // (2) Try to find a matching target type for the source object.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< ucb::XContent > xNew;
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        sal_Int32 nAttribs = aTypesInfo[ n ].Attributes;
        sal_Bool  bMatch   = sal_False;

        if ( rContext.aArg.Operation == ucb::TransferCommandOperation_LINK )
        {
            // Create link

            if ( nAttribs & ucb::ContentInfoAttribute::KIND_LINK )
            {
                // Match!
                bMatch = sal_True;
            }
        }
        else if ( ( rContext.aArg.Operation
                        == ucb::TransferCommandOperation_COPY ) ||
                  ( rContext.aArg.Operation
                        == ucb::TransferCommandOperation_MOVE ) )
        {
            // Copy / Move

            // Is source a link? Create link in target folder then.
            if ( bSourceIsLink )
            {
                if ( nAttribs & ucb::ContentInfoAttribute::KIND_LINK )
                {
                    // Match!
                    bMatch = sal_True;
                }
            }
            else
            {
                // (not a and not b) or (a and b)
                // not( a or b) or (a and b)
                //
                if ( ( !!bSourceIsFolder ==
                        !!( nAttribs
                            & ucb::ContentInfoAttribute::KIND_FOLDER ) )
                     &&
                      ( !!bSourceIsDocument ==
                        !!( nAttribs
                            & ucb::ContentInfoAttribute::KIND_DOCUMENT ) )
                   )
                {
                    // Match!
                    bMatch = sal_True;
                }
            }
        }
        else
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                        rtl::OUString::createFromAscii(
                                            "Unknown transfer operation!" ),
                                        rContext.xProcessor,
                                        -1 ) ),
                              rContext.xOrigEnv );
            // Unreachable
        }

        if ( bMatch )
        {
            //////////////////////////////////////////////////////////////
            //
            // (3) Create a new, empty object of matched type.
            //
            //////////////////////////////////////////////////////////////

            xNew = xCreator->createNewContent( aTypesInfo[ n ] );

            if ( !xNew.is() )
            {
                uno::Any aProps
                    = uno::makeAny(
                             beans::PropertyValue(
                                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "Folder")),
                                 -1,
                                 uno::makeAny(rContext.aArg.TargetURL),
                                 beans::PropertyState_DIRECT_VALUE));
                ucbhelper::cancelCommandExecution(
                    ucb::IOErrorCode_CANT_CREATE,
                    uno::Sequence< uno::Any >(&aProps, 1),
                    rContext.xOrigEnv,
                    rtl::OUString::createFromAscii(
                        "createNewContent failed!" ),
                    rContext.xProcessor );
                // Unreachable
            }
            break; // escape from 'for' loop
        }
    } // for

    return xNew;
}

//=========================================================================
static void transferProperties(
    const TransferCommandContext & rContext,
    const uno::Reference< ucb::XCommandProcessor > & xCommandProcessorS,
    const uno::Reference< ucb::XCommandProcessor > & xCommandProcessorN )
        throw( uno::Exception )
{
    ucb::Command aGetPropertySetInfoCommand(
                rtl::OUString::createFromAscii( "getPropertySetInfo" ),
                -1,
                uno::Any() );

    uno::Reference< beans::XPropertySetInfo > xInfo;
    xCommandProcessorS->execute( aGetPropertySetInfoCommand, 0, rContext.xEnv )
        >>= xInfo;

    if ( !xInfo.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(rContext.aArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xOrigEnv,
            rtl::OUString::createFromAscii(
                "Unable to get propertyset info from source object!" ),
            rContext.xProcessor );
        // Unreachable
    }

    uno::Sequence< beans::Property > aAllProps = xInfo->getProperties();

    ucb::Command aGetPropsCommand1(
                rtl::OUString::createFromAscii( "getPropertyValues" ),
                -1,
                uno::makeAny( aAllProps ) );

    uno::Reference< sdbc::XRow > xRow1;
    xCommandProcessorS->execute(
        aGetPropsCommand1, 0, rContext.xEnv ) >>= xRow1;

    if ( !xRow1.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(rContext.aArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xOrigEnv,
            rtl::OUString::createFromAscii(
                "Unable to get properties from source object!" ),
            rContext.xProcessor );
        // Unreachable
    }

    // Assemble data structure for setPropertyValues command.

    // Note: Make room for additional Title and TargetURL too. -> + 2
    uno::Sequence< beans::PropertyValue > aPropValues(
                                                aAllProps.getLength() + 2 );

    sal_Bool bHasTitle = ( rContext.aArg.NewTitle.getLength() == 0 );
    sal_Bool bHasTargetURL = ( rContext.aArg.Operation
                                != ucb::TransferCommandOperation_LINK );

    sal_Int32 nWritePos = 0;
    for ( sal_Int32 m = 0; m < aAllProps.getLength(); ++m )
    {
        const beans::Property & rCurrProp = aAllProps[ m ];
        beans::PropertyValue & rCurrValue = aPropValues[ nWritePos ];

        uno::Any aValue;

        if ( rCurrProp.Name.compareToAscii( "Title" ) == 0 )
        {
            // Supply new title, if given.
            if ( !bHasTitle )
            {
                bHasTitle = sal_True;
                aValue <<= rContext.aArg.NewTitle;
            }
        }
        else if ( rCurrProp.Name.compareToAscii( "TargetURL" ) == 0 )
        {
            // Supply source URL as link target for the new link to create.
            if ( !bHasTargetURL )
            {
                bHasTargetURL = sal_True;
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
    if ( !bHasTitle && ( rContext.aArg.NewTitle.getLength() > 0 ) )
    {
        aPropValues[ nWritePos ].Name
            = rtl::OUString::createFromAscii( "Title" );
        aPropValues[ nWritePos ].Handle = -1;
        aPropValues[ nWritePos ].Value <<= rContext.aArg.NewTitle;

        nWritePos++;
    }

    // TargetURL needed, but not set yet?
    if ( !bHasTargetURL && ( rContext.aArg.Operation
                                == ucb::TransferCommandOperation_LINK ) )
    {
        aPropValues[ nWritePos ].Name
            = rtl::OUString::createFromAscii( "TargetURL" );
        aPropValues[ nWritePos ].Handle = -1;
        aPropValues[ nWritePos ].Value <<= rContext.aArg.SourceURL;

        nWritePos++;
    }

    aPropValues.realloc( nWritePos );

    // Set properties at new object.

    ucb::Command aSetPropsCommand(
                rtl::OUString::createFromAscii( "setPropertyValues" ),
                -1,
                uno::makeAny( aPropValues ) );

    xCommandProcessorN->execute( aSetPropsCommand, 0, rContext.xEnv );

    // @@@ What to do with source props that are not supported by the
    //     new object? addProperty ???
}

//=========================================================================
static uno::Reference< io::XInputStream > getInputStream(
    const TransferCommandContext & rContext,
    const uno::Reference< ucb::XCommandProcessor > & xCommandProcessorS )
        throw( uno::Exception )
{
    uno::Reference< io::XInputStream > xInputStream;

    //////////////////////////////////////////////////////////////////////
    //
    // (1) Try to get data as XInputStream via XActiveDataSink.
    //
    //////////////////////////////////////////////////////////////////////

    try
    {
        uno::Reference< io::XActiveDataSink > xSink = new ActiveDataSink;

        ucb::OpenCommandArgument2 aArg;
        aArg.Mode       = ucb::OpenMode::DOCUMENT;
        aArg.Priority   = 0; // unused
        aArg.Sink       = xSink;
        aArg.Properties = uno::Sequence< beans::Property >( 0 ); // unused

        ucb::Command aOpenCommand(
                                rtl::OUString::createFromAscii( "open" ),
                                -1,
                                uno::makeAny( aArg ) );

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
        //////////////////////////////////////////////////////////////////
        //
        // (2) Try to get data via XOutputStream.
        //
        //////////////////////////////////////////////////////////////////

        try
        {
            uno::Reference< io::XOutputStream > xOutputStream(
                rContext.xSMgr->createInstance(
                    rtl::OUString::createFromAscii( "com.sun.star.io.Pipe" ) ),
                uno::UNO_QUERY );

            if ( xOutputStream.is() )
            {
                ucb::OpenCommandArgument2 aArg;
                aArg.Mode       = ucb::OpenMode::DOCUMENT;
                aArg.Priority   = 0; // unused
                aArg.Sink       = xOutputStream;
                aArg.Properties = uno::Sequence< beans::Property >( 0 );

                ucb::Command aOpenCommand(
                                    rtl::OUString::createFromAscii( "open" ),
                                    -1,
                                    uno::makeAny( aArg ) );

                xCommandProcessorS->execute( aOpenCommand, 0, rContext.xEnv );

                xInputStream = uno::Reference< io::XInputStream >(
                                        xOutputStream, uno::UNO_QUERY );
            }
        }
        catch ( uno::RuntimeException const & )
        {
            throw;
        }
        catch ( uno::Exception const & )
        {
            OSL_ENSURE( sal_False, "unable to get input stream from document!" );
        }
    }

    return xInputStream;
}

//=========================================================================
static uno::Reference< sdbc::XResultSet > getResultSet(
    const TransferCommandContext & rContext,
    const uno::Reference< ucb::XCommandProcessor > & xCommandProcessorS )
        throw( uno::Exception )
{
    uno::Reference< sdbc::XResultSet > xResultSet;

    uno::Sequence< beans::Property > aProps( 3 );

    aProps[ 0 ].Name   = rtl::OUString::createFromAscii( "IsFolder" );
    aProps[ 0 ].Handle = -1; /* unknown */
    aProps[ 1 ].Name   = rtl::OUString::createFromAscii( "IsDocument" );
    aProps[ 1 ].Handle = -1; /* unknown */
    aProps[ 2 ].Name   = rtl::OUString::createFromAscii( "TargetURL" );
    aProps[ 2 ].Handle = -1; /* unknown */

    ucb::OpenCommandArgument2 aArg;
    aArg.Mode       = ucb::OpenMode::ALL;
    aArg.Priority   = 0; // unused
    aArg.Sink       = 0;
    aArg.Properties = aProps;

    ucb::Command aOpenCommand( rtl::OUString::createFromAscii( "open" ),
                                     -1,
                                     uno::makeAny( aArg ) );
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
         OSL_ENSURE( sal_False, "unable to get result set from folder!" );
    }

    return xResultSet;
}

//=========================================================================
static void handleNameClashRename(
        const TransferCommandContext & rContext,
        const uno::Reference< ucb::XContent > & xNew,
        const uno::Reference<
            ucb::XCommandProcessor > & xCommandProcessorN,
        const uno::Reference<
            ucb::XCommandProcessor > & xCommandProcessorS,
        /* [inout] */ uno::Reference< io::XInputStream > & xInputStream )
    throw( uno::Exception )
{
    sal_Int32 nTry = 0;

    // Obtain old title.
    uno::Sequence< beans::Property > aProps( 1 );
    aProps[ 0 ].Name   = rtl::OUString::createFromAscii( "Title" );
    aProps[ 0 ].Handle = -1;

    ucb::Command aGetPropsCommand(
            rtl::OUString::createFromAscii( "getPropertyValues" ),
            -1,
            uno::makeAny( aProps ) );

    uno::Reference< sdbc::XRow > xRow;
    xCommandProcessorN->execute( aGetPropsCommand, 0, rContext.xEnv )  >>= xRow;

    if ( !xRow.is() )
    {
        uno::Any aProps2
            = uno::makeAny(
                     beans::PropertyValue(
                         rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Uri" ) ),
                         -1,
                         uno::makeAny(
                             xNew->getIdentifier()->getContentIdentifier() ),
                         beans::PropertyState_DIRECT_VALUE ) );
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >( &aProps2, 1 ),
            rContext.xOrigEnv,
            rtl::OUString::createFromAscii(
                "Unable to get properties from new object!" ),
            rContext.xProcessor );
        // Unreachable
    }

    rtl::OUString aOldTitle = xRow->getString( 1 );
    if ( !aOldTitle.getLength() )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( beans::UnknownPropertyException(
                            rtl::OUString::createFromAscii(
                                "Unable to get property 'Title' "
                                "from new object!" ),
                            rContext.xProcessor ) ),
            rContext.xOrigEnv );
        // Unreachable
    }

    // Some pseudo-intelligence for not destroying file extensions.
    rtl::OUString aOldTitlePre;
    rtl::OUString aOldTitlePost;
    sal_Int32 nPos = aOldTitle.lastIndexOf( '.' );
    if ( nPos != -1 )
    {
        aOldTitlePre = aOldTitle.copy( 0, nPos );
        aOldTitlePost = aOldTitle.copy( nPos );
    }
    else
        aOldTitlePre = aOldTitle;

    if ( nPos > 0 )
        aOldTitlePre += rtl::OUString::createFromAscii( "_" );

    sal_Bool bContinue = sal_True;
    do
    {
        nTry++;

        rtl::OUString aNewTitle = aOldTitlePre;
        aNewTitle += rtl::OUString::valueOf( nTry );
        aNewTitle += aOldTitlePost;

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
                        uno::Any aProps2
                            = uno::makeAny(
                                beans::PropertyValue(
                                    rtl::OUString(
                                        RTL_CONSTASCII_USTRINGPARAM( "Uri" ) ),
                                    -1,
                                    uno::makeAny(
                                        xNew->getIdentifier()->
                                            getContentIdentifier() ),
                                    beans::PropertyState_DIRECT_VALUE ) );
                        ucbhelper::cancelCommandExecution(
                            ucb::IOErrorCode_CANT_READ,
                            uno::Sequence< uno::Any >( &aProps2, 1 ),
                            rContext.xOrigEnv,
                            rtl::OUString::createFromAscii(
                                "Got no data stream from source!" ),
                            rContext.xProcessor );
                        // Unreachable
                    }
                }
            }

            ucb::InsertCommandArgument aArg;
            aArg.Data = xInputStream;
            aArg.ReplaceExisting = sal_False;

            ucb::Command aInsertCommand(
                        rtl::OUString::createFromAscii( "insert" ),
                        -1,
                        uno::makeAny( aArg ) );

            xCommandProcessorN->execute( aInsertCommand, 0, rContext.xEnv );

            // Success!
            bContinue = sal_False;
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
            uno::makeAny(
                ucb::UnsupportedNameClashException(
                    rtl::OUString::createFromAscii(
                        "Unable to resolve name clash!" ),
                    rContext.xProcessor,
                    ucb::NameClash::RENAME ) ),
            rContext.xOrigEnv );
        // Unreachable
    }
}

//=========================================================================
static void globalTransfer(
        const TransferCommandContext & rContext,
        const uno::Reference< ucb::XContent > & xSource,
        const uno::Reference< ucb::XContent > & xTarget,
        const uno::Reference< sdbc::XRow > & xSourceProps )
    throw( uno::Exception )
{
    // IsFolder: property is required.
    sal_Bool bSourceIsFolder = xSourceProps->getBoolean( 1 );
    if ( !bSourceIsFolder && xSourceProps->wasNull() )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( beans::UnknownPropertyException(
                            rtl::OUString::createFromAscii(
                                "Unable to get property 'IsFolder' "
                                "from source object!" ),
                            rContext.xProcessor ) ),
            rContext.xOrigEnv );
        // Unreachable
    }

    // IsDocument: property is required.
    sal_Bool bSourceIsDocument = xSourceProps->getBoolean( 2 );
    if ( !bSourceIsDocument && xSourceProps->wasNull() )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( beans::UnknownPropertyException(
                            rtl::OUString::createFromAscii(
                                "Unable to get property 'IsDocument' "
                                "from source object!" ),
                            rContext.xProcessor ) ),
            rContext.xOrigEnv );
        // Unreachable
    }

    // TargetURL: property is optional.
    sal_Bool bSourceIsLink = ( xSourceProps->getString( 3 ).getLength() > 0 );

    //////////////////////////////////////////////////////////////////////
    //
    // (1) Try to find a matching target type for the source object and
    //     create a new, empty object of that type.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< ucb::XContent > xNew = createNew( rContext,
                                                      xTarget,
                                                      bSourceIsFolder,
                                                      bSourceIsDocument,
                                                      bSourceIsLink );
    if ( !xNew.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                      "Folder")),
                                  -1,
                                  uno::makeAny(rContext.aArg.TargetURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_CREATE,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xOrigEnv,
            rtl::OUString::createFromAscii(
                "No matching content type at target!" ),
            rContext.xProcessor );
        // Unreachable
    }

    //////////////////////////////////////////////////////////////////////
    //
    // (2) Transfer property values from source to new object.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< ucb::XCommandProcessor > xCommandProcessorN(
                                                    xNew, uno::UNO_QUERY );
    if ( !xCommandProcessorN.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(
                                      xNew->getIdentifier()->
                                                getContentIdentifier()),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_WRITE,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xOrigEnv,
            rtl::OUString::createFromAscii(
                "New content is not a XCommandProcessor!" ),
            rContext.xProcessor );
        // Unreachable
    }

    // Obtain all properties from source.

    uno::Reference< ucb::XCommandProcessor > xCommandProcessorS(
                                                    xSource, uno::UNO_QUERY );
    if ( !xCommandProcessorS.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(rContext.aArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xOrigEnv,
            rtl::OUString::createFromAscii(
                "Source content is not a XCommandProcessor!" ),
            rContext.xProcessor );
        // Unreachable
    }

    transferProperties( rContext, xCommandProcessorS, xCommandProcessorN );

    //////////////////////////////////////////////////////////////////////
    //
    // (3) Try to obtain a data stream from source.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< io::XInputStream > xInputStream;

    if ( bSourceIsDocument && ( rContext.aArg.Operation
                                != ucb::TransferCommandOperation_LINK ) )
        xInputStream = getInputStream( rContext, xCommandProcessorS );

    //////////////////////////////////////////////////////////////////////
    //
    // (4) Try to obtain a resultset (children) from source.
    //
    //////////////////////////////////////////////////////////////////////

    uno::Reference< sdbc::XResultSet > xResultSet;

    if ( bSourceIsFolder && ( rContext.aArg.Operation
                                != ucb::TransferCommandOperation_LINK ) )
        xResultSet = getResultSet( rContext, xCommandProcessorS );

    //////////////////////////////////////////////////////////////////////
    //
    // (5) Insert (store) new content.
    //
    //////////////////////////////////////////////////////////////////////

    ucb::InsertCommandArgument aArg;
    aArg.Data = xInputStream;

    switch ( rContext.aArg.NameClash )
    {
        case ucb::NameClash::OVERWRITE:
            aArg.ReplaceExisting = sal_True;
            break;

        case ucb::NameClash::ERROR:
        case ucb::NameClash::RENAME:
        case ucb::NameClash::KEEP: // deprecated
        case ucb::NameClash::ASK:
            aArg.ReplaceExisting = sal_False;
            break;

        default:
            aArg.ReplaceExisting = sal_False;
            OSL_ENSURE( sal_False, "Unknown nameclash directive!" );
            break;
    }

    rtl::OUString aDesiredName = createDesiredName( rContext.aArg );

    bool bRetry;
    do
    {
        bRetry = false;

        try
        {
            ucb::Command aInsertCommand(
                                    rtl::OUString::createFromAscii( "insert" ),
                                    -1,
                                    uno::makeAny( aArg ) );

            xCommandProcessorN->execute( aInsertCommand, 0, rContext.xEnv );
        }
        catch ( ucb::UnsupportedNameClashException const & exc )
        {
            OSL_ENSURE( !aArg.ReplaceExisting,
                        "BUG: UnsupportedNameClashException not allowed here!" );

            if (exc.NameClash != ucb::NameClash::ERROR) {
                OSL_ENSURE( false, "BUG: NameClash::ERROR expected!" );
            }

            // No chance to solve name clashes, because I'm not able to detect
            // whether there is one.
            throw ucb::UnsupportedNameClashException(
                    rtl::OUString::createFromAscii(
                        "Unable to resolve name clashes, no chance to detect "
                        "that there is one!" ),
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
                        uno::makeAny(
                            ucb::UnsupportedNameClashException(
                                rtl::OUString::createFromAscii(
                                    "BUG: insert + replace == true MUST NOT "
                                    "throw NameClashException." ),
                                rContext.xProcessor,
                                rContext.aArg.NameClash ) ),
                        rContext.xOrigEnv );
                    // Unreachable
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
                        rtl::OUString aNewTitle;
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
    //                            break;

                            case UNKNOWN:
                                // Handled, but not clear, how...
                                // fall-thru intended.

                            case ABORT:
                                throw ucb::CommandFailedException(
                                    rtl::OUString(
                                        RTL_CONSTASCII_USTRINGPARAM(
                                            "abort requested via interaction "
                                            "handler" ) ),
                                    uno::Reference< uno::XInterface >(),
                                    aExc );
    //                            break;

                            case OVERWRITE:
                                OSL_ENSURE( aArg.ReplaceExisting == sal_False,
                                            "Hu? ReplaceExisting already true?"
                                          );
                                aArg.ReplaceExisting = sal_True;
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
                                        rtl::OUString(
                                            RTL_CONSTASCII_USTRINGPARAM(
                                                "error setting Title property!"
                                            ) ),
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
                        uno::makeAny(
                            ucb::UnsupportedNameClashException(
                                rtl::OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "default action, don't know how to "
                                        "handle name clash" ) ),
                                rContext.xProcessor,
                                rContext.aArg.NameClash ) ),
                        rContext.xOrigEnv );
                    // Unreachable
                }
            }
        }
    }
    while ( bRetry );

    //////////////////////////////////////////////////////////////////////
    //
    // (6) Process children of source.
    //
    //////////////////////////////////////////////////////////////////////

    if ( xResultSet.is() )
    {
        try
        {
            // Iterate over children...

            uno::Reference< sdbc::XRow > xChildRow(
                                            xResultSet, uno::UNO_QUERY );

            if ( !xChildRow.is() )
            {
                uno::Any aProps
                    = uno::makeAny(
                             beans::PropertyValue(
                                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "Uri")),
                                 -1,
                                 uno::makeAny(rContext.aArg.SourceURL),
                                 beans::PropertyState_DIRECT_VALUE));
                ucbhelper::cancelCommandExecution(
                    ucb::IOErrorCode_CANT_READ,
                    uno::Sequence< uno::Any >(&aProps, 1),
                    rContext.xOrigEnv,
                    rtl::OUString::createFromAscii(
                        "Unable to get properties from children of source!" ),
                    rContext.xProcessor );
                // Unreachable
            }

            uno::Reference< ucb::XContentAccess > xChildAccess(
                                                xResultSet, uno::UNO_QUERY );

            if ( !xChildAccess.is() )
            {
                uno::Any aProps
                    = uno::makeAny(
                             beans::PropertyValue(
                                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                     "Uri")),
                                 -1,
                                 uno::makeAny(rContext.aArg.SourceURL),
                                 beans::PropertyState_DIRECT_VALUE));
                ucbhelper::cancelCommandExecution(
                    ucb::IOErrorCode_CANT_READ,
                    uno::Sequence< uno::Any >(&aProps, 1),
                    rContext.xOrigEnv,
                    rtl::OUString::createFromAscii(
                        "Unable to get children of source!" ),
                    rContext.xProcessor );
                // Unreachable
            }

            if ( xResultSet->first() )
            {
                ucb::GlobalTransferCommandArgument aTransArg(
                        rContext.aArg.Operation,      // Operation
                        rtl::OUString(),              // SourceURL; filled later
                        xNew->getIdentifier()
                            ->getContentIdentifier(), // TargetURL
                        rtl::OUString(),              // NewTitle;
                        rContext.aArg.NameClash );    // NameClash

                ucb_commands::TransferCommandContext aSubCtx(
                        rContext.xSMgr,
                        rContext.xProcessor,
                        rContext.xEnv,
                        rContext.xOrigEnv,
                        aTransArg );
                do
                {
                    uno::Reference< ucb::XContent > xChild
                                        = xChildAccess->queryContent();
                    if ( xChild.is() )
                    {
                        // Recursion!

                        aSubCtx.aArg.SourceURL
                            = xChild->getIdentifier()->getContentIdentifier();

                        ucb_commands::globalTransfer( aSubCtx,
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
                        rtl::OUString::createFromAscii("getCommandInfo"),
                        -1,
                        uno::Any()),
                    0,
                    rContext.xEnv );

        const rtl::OUString cmdName =
            rtl::OUString::createFromAscii("flush");
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

} /* namescpace ucb_commands */

//=========================================================================
//
// UniversalContentBroker implementation ( XCommandProcessor commands ).
//
//=========================================================================

uno::Reference< ucb::XCommandInfo >
UniversalContentBroker::getCommandInfo()
{
    return uno::Reference< ucb::XCommandInfo >(
                            new ucb_commands::CommandProcessorInfo() );
}

//=========================================================================
void UniversalContentBroker::globalTransfer(
            const ucb::GlobalTransferCommandArgument & rArg,
            const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    // Use own command environment with own interaction handler intercepting
    // some interaction requests that shell not be handled by the user-supplied
    // interaction handler.
    uno::Reference< ucb::XCommandEnvironment > xLocalEnv;
    if (xEnv.is())
        xLocalEnv.set( new ucb_commands::CommandEnvironment(
                           new ucb_commands::InteractionHandlerProxy(
                               xEnv->getInteractionHandler() ),
                           xEnv->getProgressHandler() ) );

    //////////////////////////////////////////////////////////////////////
    //
    // (1) Try to transfer the content using 'transfer' command.
    //
    //////////////////////////////////////////////////////////////////////

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
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                      "Uri")),
                                  -1,
                                  uno::makeAny(rArg.TargetURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            rtl::OUString::createFromAscii(
                "Can't instanciate target object!" ),
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
            uno::Any aProps
                = uno::makeAny(
                         beans::PropertyValue(
                             rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                               "Uri")),
                             -1,
                             uno::makeAny(rArg.TargetURL),
                             beans::PropertyState_DIRECT_VALUE));
            ucbhelper::cancelCommandExecution(
                ucb::IOErrorCode_CANT_READ,
                uno::Sequence< uno::Any >(&aProps, 1),
                xEnv,
                rtl::OUString::createFromAscii(
                    "Target content is not a XCommandProcessor!" ),
                this );
            // Unreachable
        }

        ucb::TransferInfo aTransferArg(
            ( rArg.Operation
                == ucb::TransferCommandOperation_MOVE ), // MoveData
            rArg.SourceURL,   // SourceURL
            rArg.NewTitle,    // NewTitle
            rArg.NameClash ); // NameClash

        bool bRetry;
        do
        {
            bRetry = false;

            try
            {
                ucb::Command aCommand(
                    rtl::OUString::createFromAscii( "transfer" ), // Name
                    -1,                                           // Handle
                    uno::makeAny( aTransferArg ) );               // Argument

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
                        ucb::TransferInfo aTransferArg1(
                            aTransferArg.MoveData,
                            aTransferArg.SourceURL,
                            aTransferArg.NewTitle,
                            ucb::NameClash::ERROR );

                        ucb::Command aCommand1(
                            rtl::OUString::createFromAscii( "transfer" ),
                            -1,
                            uno::makeAny( aTransferArg1 ) );

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
                        rtl::OUString aNewTitle;
                        ucb_commands::NameClashContinuation eCont
                            = ucb_commands::interactiveNameClashResolve(
                                xEnv, // always use original environment!
                                rArg.TargetURL,  // target folder URL
                                ucb_commands::createDesiredName(
                                    aTransferArg ),   // clashing name
                                aExc,
                                aNewTitle );

                        switch ( eCont )
                        {
                            case ucb_commands::NOT_HANDLED:
                                // Not handled.
                                cppu::throwException( aExc );
//                                break;

                            case ucb_commands::UNKNOWN:
                                // Handled, but not clear, how...
                                // fall-thru intended.

                            case ucb_commands::ABORT:
                                throw ucb::CommandFailedException(
                                    rtl::OUString(
                                        RTL_CONSTASCII_USTRINGPARAM(
                                            "abort requested via interaction "
                                            "handler" ) ),
                                    uno::Reference< uno::XInterface >(),
                                    aExc );
//                                break;

                            case ucb_commands::OVERWRITE:
                                aTransferArg.NameClash
                                    = ucb::NameClash::OVERWRITE;
                                bRetry = true;
                                break;

                            case ucb_commands::NEW_NAME:
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

    //////////////////////////////////////////////////////////////////////
    //
    // (2) Try to transfer the content "manually".
    //
    //////////////////////////////////////////////////////////////////////

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
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(rArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            rtl::OUString::createFromAscii(
                "Can't instanciate source object!" ),
            this );
        // Unreachable
    }

    uno::Reference< ucb::XCommandProcessor > xCommandProcessor(
                                                xSource, uno::UNO_QUERY );
    if ( !xCommandProcessor.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(rArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            rtl::OUString::createFromAscii(
                "Source content is not a XCommandProcessor!" ),
            this );
        // Unreachable
    }

    // Obtain interesting property values from source...

    uno::Sequence< beans::Property > aProps( 4 );

    aProps[ 0 ].Name   = rtl::OUString::createFromAscii( "IsFolder" );
    aProps[ 0 ].Handle = -1; /* unknown */
    aProps[ 1 ].Name   = rtl::OUString::createFromAscii( "IsDocument" );
    aProps[ 1 ].Handle = -1; /* unknown */
    aProps[ 2 ].Name   = rtl::OUString::createFromAscii( "TargetURL" );
    aProps[ 2 ].Handle = -1; /* unknown */
    aProps[ 3 ].Name   = rtl::OUString::createFromAscii( "BaseURI" );
    aProps[ 3 ].Handle = -1; /* unknown */

    ucb::Command aGetPropsCommand(
                rtl::OUString::createFromAscii( "getPropertyValues" ),
                -1,
                uno::makeAny( aProps ) );

    uno::Reference< sdbc::XRow > xRow;
    xCommandProcessor->execute( aGetPropsCommand, 0, xLocalEnv ) >>= xRow;

    if ( !xRow.is() )
    {
        uno::Any aProps2
            = uno::makeAny(beans::PropertyValue(
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                    "Uri")),
                                  -1,
                                  uno::makeAny(rArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps2, 1),
            xEnv,
            rtl::OUString::createFromAscii(
                "Unable to get properties from source object!" ),
            this );
        // Unreachable
    }

    ucb_commands::TransferCommandContext aTransferCtx(
        m_xSMgr, this, xLocalEnv, xEnv, rArg );

    if ( rArg.NewTitle.getLength() == 0 )
    {
        // BaseURI: property is optional.
        rtl::OUString aBaseURI( xRow->getString( 4 ) );
        if ( aBaseURI.getLength() )
        {
            aTransferCtx.aArg.NewTitle
                = ucb_commands::createDesiredName( aBaseURI, rtl::OUString() );
        }
    }

    // Do it!
    ucb_commands::globalTransfer( aTransferCtx, xSource, xTarget, xRow );

    //////////////////////////////////////////////////////////////////////
    //
    // (3) Delete source, if operation is MOVE.
    //
    //////////////////////////////////////////////////////////////////////

    if ( rArg.Operation == ucb::TransferCommandOperation_MOVE )
    {
        try
        {
            ucb::Command aCommand(
                rtl::OUString::createFromAscii( "delete" ), // Name
                -1,                                         // Handle
                uno::makeAny( sal_Bool( sal_True ) ) );     // Argument

            xCommandProcessor->execute( aCommand, 0, xLocalEnv );
        }
        catch ( uno::Exception const & )
        {
            OSL_ENSURE( sal_False, "Cannot delete source object!" );
            throw;
        }
    }
}
