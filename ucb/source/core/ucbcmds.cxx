/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <osl/diagnose.h>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/io/Pipe.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/ucb/CommandEnvironment.hpp>
#include <com/sun/star/ucb/CommandFailedException.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/GlobalTransferCommandArgument2.hpp>
#include <com/sun/star/ucb/InsertCommandArgument2.hpp>
#include <com/sun/star/ucb/InteractiveBadTransferURLException.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/TransferInfo2.hpp>
#include <com/sun/star/ucb/UnsupportedNameClashException.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XContentCreator.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <ucbhelper/cancelcommandexecution.hxx>
#include <ucbhelper/simplenameclashresolverequest.hxx>
#include "ucbcmds.hxx"
#include "ucb.hxx"

using namespace com::sun::star;

namespace
{


//

//


struct TransferCommandContext
{
    uno::Reference< uno::XComponentContext >     m_xContext;
    uno::Reference< ucb::XCommandProcessor >     xProcessor;
    uno::Reference< ucb::XCommandEnvironment >   xEnv;
    uno::Reference< ucb::XCommandEnvironment >   xOrigEnv;
    ucb::GlobalTransferCommandArgument2          aArg;

    TransferCommandContext(
        const uno::Reference< uno::XComponentContext > & xContext,
        const uno::Reference< ucb::XCommandProcessor > & rxProcessor,
        const uno::Reference< ucb::XCommandEnvironment > & rxEnv,
        const uno::Reference< ucb::XCommandEnvironment > & rxOrigEnv,
        const ucb::GlobalTransferCommandArgument2 & rArg )
    : m_xContext( xContext ), xProcessor( rxProcessor ), xEnv( rxEnv ),
      xOrigEnv( rxOrigEnv ), aArg( rArg ) {}
};


//

//


class InteractionHandlerProxy :
    public cppu::WeakImplHelper1< task::XInteractionHandler >
{
    uno::Reference< task::XInteractionHandler > m_xOrig;

public:
    InteractionHandlerProxy(
        const uno::Reference< task::XInteractionHandler > & xOrig )
    : m_xOrig( xOrig ) {}

    
    virtual void SAL_CALL handle(
            const uno::Reference< task::XInteractionRequest >& Request )
        throw ( uno::RuntimeException );
};



void SAL_CALL InteractionHandlerProxy::handle(
            const uno::Reference< task::XInteractionRequest >& Request )
    throw ( uno::RuntimeException )
{
    if ( !m_xOrig.is() )
        return;

    
    uno::Any aRequest = Request->getRequest();

    
    ucb::InteractiveBadTransferURLException aBadTransferURLEx;
    if ( aRequest >>= aBadTransferURLEx )
    {
        return;
    }
    else
    {
        
        ucb::UnsupportedNameClashException aUnsupportedNameClashEx;
        if ( aRequest >>= aUnsupportedNameClashEx )
        {
            if ( aUnsupportedNameClashEx.NameClash
                    != ucb::NameClash::ERROR )
                return;
        }
        else
        {
            
            ucb::NameClashException aNameClashEx;
            if ( aRequest >>= aNameClashEx )
            {
                return;
            }
            else
            {
                
                ucb::UnsupportedCommandException aUnsupportedCommandEx;
                if ( aRequest >>= aUnsupportedCommandEx )
                {
                    return;
                }
            }
        }
    }

    
    m_xOrig->handle( Request );
}


//

//


class ActiveDataSink : public cppu::WeakImplHelper1< io::XActiveDataSink >
{
    uno::Reference< io::XInputStream > m_xStream;

public:
    
    virtual void SAL_CALL setInputStream(
                        const uno::Reference< io::XInputStream >& aStream )
        throw( uno::RuntimeException );
    virtual uno::Reference< io::XInputStream > SAL_CALL getInputStream()
        throw( uno::RuntimeException );
};



void SAL_CALL ActiveDataSink::setInputStream(
                        const uno::Reference< io::XInputStream >& aStream )
    throw( uno::RuntimeException )
{
    m_xStream = aStream;
}



uno::Reference< io::XInputStream > SAL_CALL ActiveDataSink::getInputStream()
    throw( uno::RuntimeException )
{
    return m_xStream;
}


//

//


class CommandProcessorInfo :
    public cppu::WeakImplHelper1< ucb::XCommandInfo >
{
    uno::Sequence< ucb::CommandInfo > * m_pInfo;

public:
    CommandProcessorInfo();
    virtual ~CommandProcessorInfo();

    
    virtual uno::Sequence< ucb::CommandInfo > SAL_CALL getCommands()
        throw( uno::RuntimeException );
    virtual ucb::CommandInfo SAL_CALL
    getCommandInfoByName( const OUString& Name )
        throw( ucb::UnsupportedCommandException, uno::RuntimeException );
    virtual ucb::CommandInfo SAL_CALL
    getCommandInfoByHandle( sal_Int32 Handle )
        throw( ucb::UnsupportedCommandException, uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasCommandByName( const OUString& Name )
        throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL hasCommandByHandle( sal_Int32 Handle )
        throw( uno::RuntimeException );
};


CommandProcessorInfo::CommandProcessorInfo()
{
    m_pInfo = new uno::Sequence< ucb::CommandInfo >( 2 );

    (*m_pInfo)[ 0 ]
        = ucb::CommandInfo(
            OUString( GETCOMMANDINFO_NAME ), 
            GETCOMMANDINFO_HANDLE, 
            getCppuVoidType() ); 
    (*m_pInfo)[ 1 ]
        = ucb::CommandInfo(
            OUString( GLOBALTRANSFER_NAME ), 
            GLOBALTRANSFER_HANDLE, 
            getCppuType(
                static_cast<
                    ucb::GlobalTransferCommandArgument * >( 0 ) ) ); 
    (*m_pInfo)[ 2 ]
        = ucb::CommandInfo(
            OUString( CHECKIN_NAME ), 
            CHECKIN_HANDLE, 
            getCppuType(
                static_cast<
                    ucb::GlobalTransferCommandArgument * >( 0 ) ) ); 
}



CommandProcessorInfo::~CommandProcessorInfo()
{
    delete m_pInfo;
}



uno::Sequence< ucb::CommandInfo > SAL_CALL
CommandProcessorInfo::getCommands()
    throw( uno::RuntimeException )
{
    return uno::Sequence< ucb::CommandInfo >( *m_pInfo );
}



ucb::CommandInfo SAL_CALL
CommandProcessorInfo::getCommandInfoByName( const OUString& Name )
    throw( ucb::UnsupportedCommandException, uno::RuntimeException )
{
    for ( sal_Int32 n = 0; n < m_pInfo->getLength(); ++n )
    {
        if ( (*m_pInfo)[ n ].Name == Name )
            return ucb::CommandInfo( (*m_pInfo)[ n ] );
    }

    throw ucb::UnsupportedCommandException();
}



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



sal_Bool SAL_CALL CommandProcessorInfo::hasCommandByName(
                                                const OUString& Name )
    throw( uno::RuntimeException )
{
    for ( sal_Int32 n = 0; n < m_pInfo->getLength(); ++n )
    {
        if ( (*m_pInfo)[ n ].Name == Name )
            return sal_True;
    }

    return sal_False;
}



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





OUString createDesiredName(
    const OUString & rSourceURL, const OUString & rNewTitle )
{
    OUString aName( rNewTitle );
    if ( aName.isEmpty() )
    {
        

        
        
        
        
        
        
        

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

        
    sal_Int32  nPos = aName.indexOf( '?' );
    if ( nPos == -1 )
      nPos = aName.indexOf( '#' );

    if ( nPos != -1 )
      aName = aName.copy( 0, nPos );
    }
    return OUString( aName );
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
            rTargetURL,  
            rClashingName,   
            OUString(), 
            true /* bSupportsOverwriteData */ ) );

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
                
                uno::Reference< task::XInteractionAbort > xAbort(
                    xSelection.get(), uno::UNO_QUERY );
                if ( xAbort.is() )
                {
                    
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


bool setTitle(
        const uno::Reference< ucb::XCommandProcessor > & xCommandProcessor,
        const uno::Reference< ucb::XCommandEnvironment > & xEnv,
        const OUString & rNewTitle )
    throw( uno::RuntimeException )
{
    try
    {
        uno::Sequence< beans::PropertyValue > aPropValues( 1 );
        aPropValues[ 0 ].Name = "Title";
        aPropValues[ 0 ].Handle = -1;
        aPropValues[ 0 ].Value  = uno::makeAny( rNewTitle );

        ucb::Command aSetPropsCommand(
            OUString(  "setPropertyValues"  ),
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


uno::Reference< ucb::XContent > createNew(
                    const TransferCommandContext & rContext,
                    const uno::Reference< ucb::XContent > & xTarget,
                    sal_Bool bSourceIsFolder,
                    sal_Bool bSourceIsDocument,
                    sal_Bool bSourceIsLink )
    throw( uno::Exception )
{
    
    //
    
    //
    

    
    

    uno::Reference< ucb::XCommandProcessor > xCommandProcessorT(
                                                    xTarget, uno::UNO_QUERY );
    if ( !xCommandProcessorT.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  OUString( "Folder"),
                                  -1,
                                  uno::makeAny(rContext.aArg.TargetURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_CREATE,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xOrigEnv,
            OUString("Target is no XCommandProcessor!"),
            rContext.xProcessor );
        
    }

    uno::Sequence< beans::Property > aPropsToObtain( 1 );
    aPropsToObtain[ 0 ].Name = "CreatableContentsInfo";
    aPropsToObtain[ 0 ].Handle = -1;

    ucb::Command aGetPropsCommand(
            OUString("getPropertyValues"),
            -1,
            uno::makeAny( aPropsToObtain ) );

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
        
        

        xCreator.set( xTarget, uno::UNO_QUERY );

        if ( !xCreator.is() )
        {
            uno::Any aProps
                = uno::makeAny(beans::PropertyValue(
                                  OUString( "Folder"),
                                  -1,
                                  uno::makeAny(rContext.aArg.TargetURL),
                                  beans::PropertyState_DIRECT_VALUE));
            ucbhelper::cancelCommandExecution(
                ucb::IOErrorCode_CANT_CREATE,
                uno::Sequence< uno::Any >(&aProps, 1),
                rContext.xOrigEnv,
                OUString("Target is no XContentCreator!"),
                rContext.xProcessor );
            
        }

        aTypesInfo  = xCreator->queryCreatableContentsInfo();
    }

    sal_Int32 nCount = aTypesInfo.getLength();
    if ( !nCount )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                              OUString("Folder"),
                              -1,
                              uno::makeAny(rContext.aArg.TargetURL),
                              beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_CREATE,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xOrigEnv,
            OUString("No types creatable!"),
            rContext.xProcessor );
        
    }

    
    //
    
    //
    

    uno::Reference< ucb::XContent > xNew;
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        sal_Int32 nAttribs = aTypesInfo[ n ].Attributes;
        sal_Bool  bMatch   = sal_False;

        if ( rContext.aArg.Operation == ucb::TransferCommandOperation_LINK )
        {
            

            if ( nAttribs & ucb::ContentInfoAttribute::KIND_LINK )
            {
                
                bMatch = sal_True;
            }
        }
        else if ( ( rContext.aArg.Operation
                        == ucb::TransferCommandOperation_COPY ) ||
                  ( rContext.aArg.Operation
                        == ucb::TransferCommandOperation_MOVE ) )
        {
            

            
            if ( bSourceIsLink )
            {
                if ( nAttribs & ucb::ContentInfoAttribute::KIND_LINK )
                {
                    
                    bMatch = sal_True;
                }
            }
            else
            {
                
                
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
                    
                    bMatch = sal_True;
                }
            }
        }
        else
        {
            ucbhelper::cancelCommandExecution(
                uno::makeAny( lang::IllegalArgumentException(
                                        OUString( "Unknown transfer operation!" ),
                                        rContext.xProcessor,
                                        -1 ) ),
                              rContext.xOrigEnv );
            
        }

        if ( bMatch )
        {
            
            //
            
            //
            

            if ( !xCreator.is() )
            {
                
                
                ucb::Command aCreateNewCommand(
                   OUString("createNewContent"),
                   -1,
                   uno::makeAny( aTypesInfo[ n ] ) );

                xCommandProcessorT->execute( aCreateNewCommand, 0, rContext.xEnv )
                    >>= xNew;
            }
            else
            {
                
                

                xNew = xCreator->createNewContent( aTypesInfo[ n ] );
            }

            if ( !xNew.is() )
            {
                uno::Any aProps
                    = uno::makeAny(
                             beans::PropertyValue(
                                 OUString( "Folder"),
                                 -1,
                                 uno::makeAny(rContext.aArg.TargetURL),
                                 beans::PropertyState_DIRECT_VALUE));
                ucbhelper::cancelCommandExecution(
                    ucb::IOErrorCode_CANT_CREATE,
                    uno::Sequence< uno::Any >(&aProps, 1),
                    rContext.xOrigEnv,
                    OUString( "createNewContent failed!" ),
                    rContext.xProcessor );
                
            }
            break; 
        }
    } 

    return xNew;
}


void transferProperties(
    const TransferCommandContext & rContext,
    const uno::Reference< ucb::XCommandProcessor > & xCommandProcessorS,
    const uno::Reference< ucb::XCommandProcessor > & xCommandProcessorN )
        throw( uno::Exception )
{
    ucb::Command aGetPropertySetInfoCommand(
                OUString("getPropertySetInfo"),
                -1,
                uno::Any() );

    uno::Reference< beans::XPropertySetInfo > xInfo;
    xCommandProcessorS->execute( aGetPropertySetInfoCommand, 0, rContext.xEnv )
        >>= xInfo;

    if ( !xInfo.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  OUString( "Uri"),
                                  -1,
                                  uno::makeAny(rContext.aArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xOrigEnv,
            OUString( "Unable to get propertyset info from source object!" ),
            rContext.xProcessor );
        
    }

    uno::Sequence< beans::Property > aAllProps = xInfo->getProperties();

    ucb::Command aGetPropsCommand1(
                OUString("getPropertyValues"),
                -1,
                uno::makeAny( aAllProps ) );

    uno::Reference< sdbc::XRow > xRow1;
    xCommandProcessorS->execute(
        aGetPropsCommand1, 0, rContext.xEnv ) >>= xRow1;

    if ( !xRow1.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  OUString( "Uri"),
                                  -1,
                                  uno::makeAny(rContext.aArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xOrigEnv,
            OUString( "Unable to get properties from source object!" ),
            rContext.xProcessor );
        
    }

    

    
    uno::Sequence< beans::PropertyValue > aPropValues(
                                                aAllProps.getLength() + 2 );

    sal_Bool bHasTitle = rContext.aArg.NewTitle.isEmpty();
    sal_Bool bHasTargetURL = ( rContext.aArg.Operation
                                != ucb::TransferCommandOperation_LINK );

    sal_Int32 nWritePos = 0;
    for ( sal_Int32 m = 0; m < aAllProps.getLength(); ++m )
    {
        const beans::Property & rCurrProp = aAllProps[ m ];
        beans::PropertyValue & rCurrValue = aPropValues[ nWritePos ];

        uno::Any aValue;

        if ( rCurrProp.Name.equalsAscii( "Title" ) )
        {
            
            if ( !bHasTitle )
            {
                bHasTitle = sal_True;
                aValue <<= rContext.aArg.NewTitle;
            }
        }
        else if ( rCurrProp.Name.equalsAscii( "TargetURL" ) )
        {
            
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
                
                
            }
        }

        if ( aValue.hasValue() )
        {
            rCurrValue.Name   = rCurrProp.Name;
            rCurrValue.Handle = rCurrProp.Handle;
            rCurrValue.Value  = aValue;


            nWritePos++;
        }
    }

    
    if ( !bHasTitle && !rContext.aArg.NewTitle.isEmpty() )
    {
        aPropValues[ nWritePos ].Name = "Title";
        aPropValues[ nWritePos ].Handle = -1;
        aPropValues[ nWritePos ].Value <<= rContext.aArg.NewTitle;

        nWritePos++;
    }

    
    if ( !bHasTargetURL && ( rContext.aArg.Operation
                                == ucb::TransferCommandOperation_LINK ) )
    {
        aPropValues[ nWritePos ].Name = "TargetURL";
        aPropValues[ nWritePos ].Handle = -1;
        aPropValues[ nWritePos ].Value <<= rContext.aArg.SourceURL;

        nWritePos++;
    }

    aPropValues.realloc( nWritePos );

    

    ucb::Command aSetPropsCommand(
                OUString("setPropertyValues"),
                -1,
                uno::makeAny( aPropValues ) );

    xCommandProcessorN->execute( aSetPropsCommand, 0, rContext.xEnv );

    
    
}


uno::Reference< io::XInputStream > getInputStream(
    const TransferCommandContext & rContext,
    const uno::Reference< ucb::XCommandProcessor > & xCommandProcessorS )
        throw( uno::Exception )
{
    uno::Reference< io::XInputStream > xInputStream;

    
    //
    
    //
    

    try
    {
        uno::Reference< io::XActiveDataSink > xSink = new ActiveDataSink;

        ucb::OpenCommandArgument2 aArg;
        aArg.Mode       = ucb::OpenMode::DOCUMENT;
        aArg.Priority   = 0; 
        aArg.Sink       = xSink;
        aArg.Properties = uno::Sequence< beans::Property >( 0 ); 

        ucb::Command aOpenCommand(
                                OUString("open"),
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
        
    }

    if ( !xInputStream.is() )
    {
        
        //
        
        //
        

        try
        {
            uno::Reference< io::XOutputStream > xOutputStream( io::Pipe::create(rContext.m_xContext), uno::UNO_QUERY_THROW );

            ucb::OpenCommandArgument2 aArg;
            aArg.Mode       = ucb::OpenMode::DOCUMENT;
            aArg.Priority   = 0; 
            aArg.Sink       = xOutputStream;
            aArg.Properties = uno::Sequence< beans::Property >( 0 );

            ucb::Command aOpenCommand(
                                OUString("open"),
                                -1,
                                uno::makeAny( aArg ) );

            xCommandProcessorS->execute( aOpenCommand, 0, rContext.xEnv );

            xInputStream = uno::Reference< io::XInputStream >(
                                    xOutputStream, uno::UNO_QUERY );
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


uno::Reference< sdbc::XResultSet > getResultSet(
    const TransferCommandContext & rContext,
    const uno::Reference< ucb::XCommandProcessor > & xCommandProcessorS )
        throw( uno::Exception )
{
    uno::Reference< sdbc::XResultSet > xResultSet;

    uno::Sequence< beans::Property > aProps( 3 );

    aProps[ 0 ].Name   = "IsFolder";
    aProps[ 0 ].Handle = -1; /* unknown */
    aProps[ 1 ].Name   = "IsDocument";
    aProps[ 1 ].Handle = -1; /* unknown */
    aProps[ 2 ].Name   = "TargetURL";
    aProps[ 2 ].Handle = -1; /* unknown */

    ucb::OpenCommandArgument2 aArg;
    aArg.Mode       = ucb::OpenMode::ALL;
    aArg.Priority   = 0; 
    aArg.Sink       = 0;
    aArg.Properties = aProps;

    ucb::Command aOpenCommand( OUString("open"),
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
         OSL_FAIL( "unable to get result set from folder!" );
    }

    return xResultSet;
}


void handleNameClashRename(
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

    
    uno::Sequence< beans::Property > aProps( 1 );
    aProps[ 0 ].Name   = "Title";
    aProps[ 0 ].Handle = -1;

    ucb::Command aGetPropsCommand(
            OUString("getPropertyValues"),
            -1,
            uno::makeAny( aProps ) );

    uno::Reference< sdbc::XRow > xRow;
    xCommandProcessorN->execute( aGetPropsCommand, 0, rContext.xEnv )  >>= xRow;

    if ( !xRow.is() )
    {
        uno::Any aProps2
            = uno::makeAny(
                     beans::PropertyValue(
                         OUString(  "Uri"  ),
                         -1,
                         uno::makeAny(
                             xNew->getIdentifier()->getContentIdentifier() ),
                         beans::PropertyState_DIRECT_VALUE ) );
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >( &aProps2, 1 ),
            rContext.xOrigEnv,
            OUString( "Unable to get properties from new object!" ),
            rContext.xProcessor );
        
    }

    OUString aOldTitle = xRow->getString( 1 );
    if ( aOldTitle.isEmpty() )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( beans::UnknownPropertyException(
                            OUString( "Unable to get property 'Title' from new object!" ),
                            rContext.xProcessor ) ),
            rContext.xOrigEnv );
        
    }

    
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

    sal_Bool bContinue = sal_True;
    do
    {
        nTry++;

        OUString aNewTitle = aOldTitlePre;
        aNewTitle += OUString::number( nTry );
        aNewTitle += aOldTitlePost;

        
        setTitle( xCommandProcessorN, rContext.xEnv, aNewTitle );

        
        try
        {
            
            
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
                                    OUString( "Uri"  ),
                                    -1,
                                    uno::makeAny(
                                        xNew->getIdentifier()->
                                            getContentIdentifier() ),
                                    beans::PropertyState_DIRECT_VALUE ) );
                        ucbhelper::cancelCommandExecution(
                            ucb::IOErrorCode_CANT_READ,
                            uno::Sequence< uno::Any >( &aProps2, 1 ),
                            rContext.xOrigEnv,
                            OUString( "Got no data stream from source!" ),
                            rContext.xProcessor );
                        
                    }
                }
            }

            ucb::InsertCommandArgument2 aArg;
            aArg.Data = xInputStream;
            aArg.ReplaceExisting = sal_False;

            ucb::Command aInsertCommand(
                        OUString("insert"),
                        -1,
                        uno::makeAny( aArg ) );

            xCommandProcessorN->execute( aInsertCommand, 0, rContext.xEnv );

            
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
                    OUString( "Unable to resolve name clash!" ),
                    rContext.xProcessor,
                    ucb::NameClash::RENAME ) ),
            rContext.xOrigEnv );
        
    }
}


void globalTransfer_(
        const TransferCommandContext & rContext,
        const uno::Reference< ucb::XContent > & xSource,
        const uno::Reference< ucb::XContent > & xTarget,
        const uno::Reference< sdbc::XRow > & xSourceProps )
    throw( uno::Exception )
{
    
    sal_Bool bSourceIsFolder = xSourceProps->getBoolean( 1 );
    if ( !bSourceIsFolder && xSourceProps->wasNull() )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( beans::UnknownPropertyException(
                            OUString( "Unable to get property 'IsFolder' "
                                "from source object!" ),
                            rContext.xProcessor ) ),
            rContext.xOrigEnv );
        
    }

    
    sal_Bool bSourceIsDocument = xSourceProps->getBoolean( 2 );
    if ( !bSourceIsDocument && xSourceProps->wasNull() )
    {
        ucbhelper::cancelCommandExecution(
            uno::makeAny( beans::UnknownPropertyException(
                            OUString( "Unable to get property 'IsDocument' "
                                "from source object!" ),
                            rContext.xProcessor ) ),
            rContext.xOrigEnv );
        
    }

    
    sal_Bool bSourceIsLink = !xSourceProps->getString( 3 ).isEmpty();

    
    //
    
    
    //
    

    uno::Reference< ucb::XContent > xNew = createNew( rContext,
                                                      xTarget,
                                                      bSourceIsFolder,
                                                      bSourceIsDocument,
                                                      bSourceIsLink );
    if ( !xNew.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  OUString( "Folder"),
                                  -1,
                                  uno::makeAny(rContext.aArg.TargetURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_CREATE,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xOrigEnv,
            OUString( "No matching content type at target!" ),
            rContext.xProcessor );
        
    }

    
    //
    
    //
    

    uno::Reference< ucb::XCommandProcessor > xCommandProcessorN(
                                                    xNew, uno::UNO_QUERY );
    if ( !xCommandProcessorN.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  OUString( "Uri"),
                                  -1,
                                  uno::makeAny(
                                      xNew->getIdentifier()->
                                                getContentIdentifier()),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_WRITE,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xOrigEnv,
            OUString( "New content is not a XCommandProcessor!" ),
            rContext.xProcessor );
        
    }

    

    uno::Reference< ucb::XCommandProcessor > xCommandProcessorS(
                                                    xSource, uno::UNO_QUERY );
    if ( !xCommandProcessorS.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  OUString( "Uri"),
                                  -1,
                                  uno::makeAny(rContext.aArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            rContext.xOrigEnv,
            OUString( "Source content is not a XCommandProcessor!" ),
            rContext.xProcessor );
        
    }

    transferProperties( rContext, xCommandProcessorS, xCommandProcessorN );

    
    //
    
    //
    

    uno::Reference< io::XInputStream > xInputStream;

    if ( bSourceIsDocument && ( rContext.aArg.Operation
                                != ucb::TransferCommandOperation_LINK ) )
        xInputStream = getInputStream( rContext, xCommandProcessorS );

    
    //
    
    //
    

    uno::Reference< sdbc::XResultSet > xResultSet;

    if ( bSourceIsFolder && ( rContext.aArg.Operation
                                != ucb::TransferCommandOperation_LINK ) )
        xResultSet = getResultSet( rContext, xCommandProcessorS );

    
    //
    
    //
    

    ucb::InsertCommandArgument2 aArg;
    aArg.Data = xInputStream;
    aArg.MimeType = rContext.aArg.MimeType;
    aArg.DocumentId = rContext.aArg.DocumentId;

    switch ( rContext.aArg.NameClash )
    {
        case ucb::NameClash::OVERWRITE:
            aArg.ReplaceExisting = sal_True;
            break;

        case ucb::NameClash::ERROR:
        case ucb::NameClash::RENAME:
        case ucb::NameClash::KEEP: 
        case ucb::NameClash::ASK:
            aArg.ReplaceExisting = sal_False;
            break;

        default:
            aArg.ReplaceExisting = sal_False;
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
                                    OUString("insert"),
                                    -1,
                                    uno::makeAny( aArg ) );

            xCommandProcessorN->execute( aInsertCommand, 0, rContext.xEnv );
        }
        catch ( ucb::UnsupportedNameClashException const & exc )
        {
            OSL_ENSURE( !aArg.ReplaceExisting,
                        "BUG: UnsupportedNameClashException not allowed here!" );

            if (exc.NameClash != ucb::NameClash::ERROR) {
                OSL_FAIL( "BUG: NameClash::ERROR expected!" );
            }

            
            
            throw ucb::UnsupportedNameClashException(
                    OUString(
                        "Unable to resolve name clashes, no chance to detect "
                        "that there is one!" ),
                    rContext.xProcessor,
                    rContext.aArg.NameClash );
        }
        catch ( ucb::NameClashException const & )
        {
            
            
            
            

            
            
            
            

            
            

            switch ( rContext.aArg.NameClash )
            {
                case ucb::NameClash::OVERWRITE:
                {
                    ucbhelper::cancelCommandExecution(
                        uno::makeAny(
                            ucb::UnsupportedNameClashException(
                                OUString(
                                    "BUG: insert + replace == true MUST NOT "
                                    "throw NameClashException." ),
                                rContext.xProcessor,
                                rContext.aArg.NameClash ) ),
                        rContext.xOrigEnv );
                    
                }

                case ucb::NameClash::ERROR:
                    throw;

                case ucb::NameClash::RENAME:
                {
                    
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
                                rContext.xOrigEnv, 
                                rContext.aArg.TargetURL, 
                                aDesiredName,
                                aExc,
                                aNewTitle );

                        switch ( eCont )
                        {
                            case NOT_HANDLED:
                                
                                cppu::throwException( aExc );
    

                            case UNKNOWN:
                                
                                

                            case ABORT:
                                throw ucb::CommandFailedException(
                                    OUString(
                                            "abort requested via interaction "
                                            "handler"  ),
                                    uno::Reference< uno::XInterface >(),
                                    aExc );
    

                            case OVERWRITE:
                                OSL_ENSURE( aArg.ReplaceExisting == sal_False,
                                            "Hu? ReplaceExisting already true?"
                                          );
                                aArg.ReplaceExisting = sal_True;
                                bRetry = true;
                                break;

                            case NEW_NAME:
                            {
                                
                                if ( setTitle( xCommandProcessorN,
                                               rContext.xEnv,
                                               aNewTitle ) )
                                {
                                    
                                    aDesiredName = aNewTitle;

                                    
                                    bRetry = true;
                                }
                                else
                                {
                                    
                                    throw ucb::CommandFailedException(
                                        OUString( "error setting Title property!" ),
                                        uno::Reference< uno::XInterface >(),
                                        aExc );
                                }
                                break;
                            }
                        }

                        OSL_ENSURE( bRetry, "bRetry must be true here!!!" );
                    }
                    break;

                case ucb::NameClash::KEEP: 
                default:
                {
                    ucbhelper::cancelCommandExecution(
                        uno::makeAny(
                            ucb::UnsupportedNameClashException(
                                OUString(
                                        "default action, don't know how to "
                                        "handle name clash"  ),
                                rContext.xProcessor,
                                rContext.aArg.NameClash ) ),
                        rContext.xOrigEnv );
                    
                }
            }
        }
    }
    while ( bRetry );

    
    //
    
    //
    

    if ( xResultSet.is() )
    {
        try
        {
            

            uno::Reference< sdbc::XRow > xChildRow(
                                            xResultSet, uno::UNO_QUERY );

            if ( !xChildRow.is() )
            {
                uno::Any aProps
                    = uno::makeAny(
                             beans::PropertyValue(
                                 OUString( "Uri"),
                                 -1,
                                 uno::makeAny(rContext.aArg.SourceURL),
                                 beans::PropertyState_DIRECT_VALUE));
                ucbhelper::cancelCommandExecution(
                    ucb::IOErrorCode_CANT_READ,
                    uno::Sequence< uno::Any >(&aProps, 1),
                    rContext.xOrigEnv,
                    OUString( "Unable to get properties from children of source!" ),
                    rContext.xProcessor );
                
            }

            uno::Reference< ucb::XContentAccess > xChildAccess(
                                                xResultSet, uno::UNO_QUERY );

            if ( !xChildAccess.is() )
            {
                uno::Any aProps
                    = uno::makeAny(
                             beans::PropertyValue(
                                 OUString( "Uri"),
                                 -1,
                                 uno::makeAny(rContext.aArg.SourceURL),
                                 beans::PropertyState_DIRECT_VALUE));
                ucbhelper::cancelCommandExecution(
                    ucb::IOErrorCode_CANT_READ,
                    uno::Sequence< uno::Any >(&aProps, 1),
                    rContext.xOrigEnv,
                    OUString( "Unable to get children of source!" ),
                    rContext.xProcessor );
                
            }

            if ( xResultSet->first() )
            {
                ucb::GlobalTransferCommandArgument2 aTransArg(
                        rContext.aArg.Operation,
                        OUString(),              
                        xNew->getIdentifier()
                            ->getContentIdentifier(), 
                        OUString(),              
                        rContext.aArg.NameClash,
                        rContext.aArg.MimeType,
                        rContext.aArg.DocumentId);

                TransferCommandContext aSubCtx(
                        rContext.m_xContext,
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
                        OUString("getCommandInfo"),
                        -1,
                        uno::Any()),
                    0,
                    rContext.xEnv );

        const OUString cmdName("flush");
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

} /* namescpace */


//

//


uno::Reference< ucb::XCommandInfo >
UniversalContentBroker::getCommandInfo()
{
    return uno::Reference< ucb::XCommandInfo >( new CommandProcessorInfo() );
}


void UniversalContentBroker::globalTransfer(
            const ucb::GlobalTransferCommandArgument2 & rArg,
            const uno::Reference< ucb::XCommandEnvironment > & xEnv )
    throw( uno::Exception )
{
    
    
    
    uno::Reference< ucb::XCommandEnvironment > xLocalEnv;
    if (xEnv.is())
    {
        xLocalEnv.set( ucb::CommandEnvironment::create(
               m_xContext,
               new InteractionHandlerProxy( xEnv->getInteractionHandler() ),
               xEnv->getProgressHandler() ) );
    }

    
    //
    
    //
    

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
                                  OUString( "Uri"),
                                  -1,
                                  uno::makeAny(rArg.TargetURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            OUString( "Can't instanciate target object!" ),
            this );
        
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
                             OUString( "Uri"),
                             -1,
                             uno::makeAny(rArg.TargetURL),
                             beans::PropertyState_DIRECT_VALUE));
            ucbhelper::cancelCommandExecution(
                ucb::IOErrorCode_CANT_READ,
                uno::Sequence< uno::Any >(&aProps, 1),
                xEnv,
                OUString( "Target content is not a XCommandProcessor!" ),
                this );
            
        }

        ucb::TransferInfo2 aTransferArg(
            ( rArg.Operation
                == ucb::TransferCommandOperation_MOVE ), 
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
                    OUString( "transfer" ), 
                    -1,                                           
                    uno::makeAny( aTransferArg ) );               

                xCommandProcessor->execute( aCommand, 0, xLocalEnv );

                
                return;
            }
            catch ( ucb::InteractiveBadTransferURLException const & )
            {
                
                
            }
            catch ( ucb::UnsupportedCommandException const & )
            {
                
                
            }
            catch ( ucb::UnsupportedNameClashException const & exc )
            {
                OSL_ENSURE( aTransferArg.NameClash == exc.NameClash,
                            "nameclash mismatch!" );
                if ( exc.NameClash == ucb::NameClash::ASK )
                {
                    
                    
                    try
                    {
                        ucb::TransferInfo2 aTransferArg1(
                            aTransferArg.MoveData,
                            aTransferArg.SourceURL,
                            aTransferArg.NewTitle,
                            ucb::NameClash::ERROR,
                            aTransferArg.MimeType );

                        ucb::Command aCommand1(
                            OUString("transfer"),
                            -1,
                            uno::makeAny( aTransferArg1 ) );

                        xCommandProcessor->execute( aCommand1, 0, xLocalEnv );

                        
                        return;
                    }
                    catch ( ucb::UnsupportedNameClashException const & )
                    {
                        
                        
                        throw exc; 
                    }
                    catch ( ucb::NameClashException const & )
                    {
                        

                        uno::Any aExc;
                        OUString aNewTitle;
                        NameClashContinuation eCont
                            = interactiveNameClashResolve(
                                xEnv, 
                                rArg.TargetURL,  
                                createDesiredName(
                                  aTransferArg ),   
                                aExc,
                                aNewTitle );

                        switch ( eCont )
                        {
                            case NOT_HANDLED:
                                
                                cppu::throwException( aExc );


                            case UNKNOWN:
                                
                                

                            case ABORT:
                                throw ucb::CommandFailedException(
                                    OUString(
                                            "abort requested via interaction "
                                            "handler"  ),
                                    uno::Reference< uno::XInterface >(),
                                    aExc );


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

    
    //
    
    //
    

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
        
    }

    if ( !xSource.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  OUString( "Uri"),
                                  -1,
                                  uno::makeAny(rArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            OUString( "Can't instanciate source object!" ),
            this );
        
    }

    uno::Reference< ucb::XCommandProcessor > xCommandProcessor(
                                                xSource, uno::UNO_QUERY );
    if ( !xCommandProcessor.is() )
    {
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  OUString( "Uri"),
                                  -1,
                                  uno::makeAny(rArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps, 1),
            xEnv,
            OUString( "Source content is not a XCommandProcessor!" ),
            this );
        
    }

    

    uno::Sequence< beans::Property > aProps( 4 );

    aProps[ 0 ].Name   = "IsFolder";
    aProps[ 0 ].Handle = -1; /* unknown */
    aProps[ 1 ].Name   = "IsDocument";
    aProps[ 1 ].Handle = -1; /* unknown */
    aProps[ 2 ].Name   = "TargetURL";
    aProps[ 2 ].Handle = -1; /* unknown */
    aProps[ 3 ].Name   = "BaseURI";
    aProps[ 3 ].Handle = -1; /* unknown */

    ucb::Command aGetPropsCommand(
                OUString("getPropertyValues"),
                -1,
                uno::makeAny( aProps ) );

    uno::Reference< sdbc::XRow > xRow;
    xCommandProcessor->execute( aGetPropsCommand, 0, xLocalEnv ) >>= xRow;

    if ( !xRow.is() )
    {
        uno::Any aProps2
            = uno::makeAny(beans::PropertyValue(
                               OUString( "Uri"),
                                  -1,
                                  uno::makeAny(rArg.SourceURL),
                                  beans::PropertyState_DIRECT_VALUE));
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >(&aProps2, 1),
            xEnv,
            OUString( "Unable to get properties from source object!" ),
            this );
        
    }

    TransferCommandContext aTransferCtx(
        m_xContext, this, xLocalEnv, xEnv, rArg );

    if ( rArg.NewTitle.isEmpty() )
    {
        
        OUString aBaseURI( xRow->getString( 4 ) );
        if ( !aBaseURI.isEmpty() )
        {
            aTransferCtx.aArg.NewTitle
                = createDesiredName( aBaseURI, OUString() );
        }
    }

    
    globalTransfer_( aTransferCtx, xSource, xTarget, xRow );

    
    //
    
    //
    

    if ( rArg.Operation == ucb::TransferCommandOperation_MOVE )
    {
        try
        {
            ucb::Command aCommand(
                OUString("delete"), 
                -1,                                         
                uno::makeAny( sal_Bool( sal_True ) ) );     

            xCommandProcessor->execute( aCommand, 0, xLocalEnv );
        }
        catch ( uno::Exception const & )
        {
            OSL_FAIL( "Cannot delete source object!" );
            throw;
        }
    }
}

uno::Any UniversalContentBroker::checkIn( const ucb::CheckinArgument& rArg,
            const uno::Reference< ucb::XCommandEnvironment >& xEnv ) throw ( uno::Exception )
{
    uno::Any aRet;
    
    
    
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
        uno::Any aProps
            = uno::makeAny(beans::PropertyValue(
                                  OUString( "Uri" ), -1,
                                  uno::makeAny( rArg.TargetURL ),
                                  beans::PropertyState_DIRECT_VALUE ) );
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >( &aProps, 1 ),
            xEnv,
            OUString( "Can't instanciate target object!" ),
            this );
        
    }

    uno::Reference< ucb::XCommandProcessor > xCommandProcessor(
                                                xTarget, uno::UNO_QUERY );
    if ( !xCommandProcessor.is() )
    {
        uno::Any aProps
            = uno::makeAny(
                     beans::PropertyValue(
                         OUString( "Uri" ), -1,
                         uno::makeAny( rArg.TargetURL ),
                         beans::PropertyState_DIRECT_VALUE ) );
        ucbhelper::cancelCommandExecution(
            ucb::IOErrorCode_CANT_READ,
            uno::Sequence< uno::Any >( &aProps, 1 ),
            xEnv,
            OUString( "Target content is not a XCommandProcessor!" ),
            this );
        
    }

    try
    {
        ucb::Command aCommand(
            OUString( "checkin" ), -1,
            uno::makeAny( rArg ) );

        aRet = xCommandProcessor->execute( aCommand, 0, xLocalEnv );
    }
    catch ( ucb::UnsupportedCommandException const & )
    {
        
        
    }
    return aRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
