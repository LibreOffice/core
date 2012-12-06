/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include "DispatchPool.hxx"
#include "WordCountDispatch.hxx"
#include "defines.hxx"

#include <com/sun/star/util/XCloseable.hpp>

#include <rtl/instance.hxx>
#include <rtl/ref.hxx>

#include <map>
#include <set>
#include <algorithm>

using namespace framework::statusbar_controller_wordcount;

using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::frame;
using namespace com::sun::star::util;

using rtl::OUString;

namespace
{
    struct CommandDispatchInfo
    {
        Reference< XComponent > Component;
        Reference< XDispatch > Dispatch;

        explicit CommandDispatchInfo(
            const Reference< XComponent > &xComponent,
            const Reference< XDispatch > &xDispatch )
            : Component( xComponent )
            , Dispatch( xDispatch ) {}
    };

    typedef std::map< OUString, CommandDispatchInfo > CommandDispatchMap;
    typedef std::map< Reference< XFrame >, CommandDispatchMap > FrameDispatchMap;

    struct StaticFrameDispatchMap
            : public rtl::Static< FrameDispatchMap, StaticFrameDispatchMap > {};

    struct TheDispatchPool
            : public rtl::StaticWithInit< rtl::Reference< DispatchPool >, TheDispatchPool >
    {
        rtl::Reference< DispatchPool >
        operator()()
        {
            OSL_TRACE( "sbctlwc: Initializing static DispatchPool" );
            return rtl::Reference< DispatchPool >( DispatchPool::Create() );
        }
    };

    typedef Reference< XDispatch > ( *CreateFnPtr )(
        const Reference<XComponentContext> &,
        const Reference<XFrame> &,
        const OUString & );
    typedef bool ( *SupportsURLFnPtr )( const URL &, const OUString & );

    struct DispatchQueryInfo
    {
        SupportsURLFnPtr Supports;
        CreateFnPtr      Create;
        DispatchQueryInfo( SupportsURLFnPtr supports, CreateFnPtr create )
            : Supports( supports )
            , Create( create ) {}

        bool operator<( const DispatchQueryInfo &other ) const
        {
            return Supports < other.Supports;
        }
    };

    typedef std::set< DispatchQueryInfo > DispatchInfoSet;
    struct StaticDispatchInfoInit
    {
            DispatchInfoSet *operator()()
            {
                static DispatchInfoSet aSet;
                lcl_InitSet( aSet );
                return &aSet;
            }
        private:
            void lcl_InitSet( DispatchInfoSet &aSet )
            {
                aSet.insert( DispatchInfoSet::value_type(
                                 &WordCountDispatch::SupportsURL,
                                 &WordCountDispatch::Create ) );
            }
    };

    struct DispatchQuery :
        public rtl::StaticAggregate< DispatchInfoSet, StaticDispatchInfoInit > {};

    struct lcl_QueryDispatch
            : public std::unary_function< const DispatchQueryInfo &, bool>
    {
        private:
            URL aURL;
            OUString sMoudleIdetnifier;

        public:
            lcl_QueryDispatch( const URL &_aURL, const OUString &_sModuleId )
                : aURL( _aURL )
                , sMoudleIdetnifier( _sModuleId ) {}

            bool operator()( const DispatchQueryInfo &aDispatch )
            {
                return ( *aDispatch.Supports )( aURL, sMoudleIdetnifier );
            }
    };
}

DispatchPool *
DispatchPool::Create()
{
    return new DispatchPool();
}

DispatchPool::DispatchPool()
    :  DipatchPool_Base()
{
    OSL_TRACE( "sbctlwc::DispatchPool::DispatchPool" );
}

DispatchPool::~DispatchPool( )
{
    OSL_TRACE( "sbctlwc::DispatchPool::~DispatchPool" );
}

void SAL_CALL
DispatchPool::disposing(
    const EventObject &aEventObject )
throw ( RuntimeException )
{
    Reference< XFrame > xFrame( aEventObject.Source, UNO_QUERY );
    if ( xFrame.is( ) )
    {
        FrameDispatchMap &aMap = StaticFrameDispatchMap::get( );
        FrameDispatchMap::iterator it = aMap.find( xFrame );
        if ( it != aMap.end( ) )
        {
            OSL_TRACE( "sbctlwc::DispatchPool::disposing : disposing dispatches for this XFrame" );
            CommandDispatchMap &aCmds = it->second;
            for ( CommandDispatchMap::iterator aCmdIt = aCmds.begin( );
                    aCmdIt != aCmds.end( );
                    aCmdIt++ )
            {
                CommandDispatchInfo &aInfo = aCmdIt->second;
                if ( aInfo.Dispatch.is( ) )
                {
                    Reference< XComponent > xComp( aInfo.Dispatch, UNO_QUERY );
                    if ( xComp.is( ) )
                        xComp->dispose( );
                    aInfo.Dispatch.clear();
                    aInfo.Component.clear();
                }

            }
            aCmds.clear();
            aMap.erase( it );
        }

        return;
    }

    Reference< XComponent > xComponent( aEventObject.Source, UNO_QUERY );
    if ( xComponent.is() )
    {
        FrameDispatchMap &aFrames = StaticFrameDispatchMap::get( );
        for ( FrameDispatchMap::iterator aFrameIt = aFrames.begin();
                aFrameIt != aFrames.end();
                aFrameIt++ )
        {
            CommandDispatchMap &aCommands = aFrameIt->second;
            for ( CommandDispatchMap::iterator aCommandIt = aCommands.begin();
                    aCommandIt != aCommands.end();
                    aCommandIt++ )
            {
                CommandDispatchInfo &aInfo = aCommandIt->second;
                if ( xComponent == aInfo.Component )
                {
                    Reference< XComponent> xComp( aInfo.Dispatch, UNO_QUERY );
                    if ( xComp.is( ) )
                        xComp->dispose( );
                    aInfo.Component.clear();
                    aInfo.Dispatch.clear();
                    aCommands.erase( aCommandIt );
                }
            }
        }
    }
}

Reference< XDispatch >
DispatchPool::GetDispatch(
    const Reference<XComponentContext> &xContext,
    const Reference< XFrame > &xFrame,
    const URL &rCommandURL,
    const rtl::OUString &rModuleIdentifier )
{

    OSL_ENSURE( xFrame.is(), "DispatchPool::GetDispatch - no XFrame!" );
    Reference< XDispatch > xRet;

    const DispatchInfoSet &aDispatchInfo = *DispatchQuery::get();
    const DispatchInfoSet::const_iterator pDispatchInfo = std::find_if(
                aDispatchInfo.begin(),
                aDispatchInfo.end(),
                lcl_QueryDispatch( rCommandURL, rModuleIdentifier ) );
    if ( pDispatchInfo == aDispatchInfo.end() )
        return xRet;

    Reference< XComponent > xComponent;
    bool bListen = false;
    const OUString sCommandURLPath = rCommandURL.Path;
    const rtl::Reference<DispatchPool> &aPool = TheDispatchPool::get();
    FrameDispatchMap &aFrames = StaticFrameDispatchMap::get();

    Reference< XController > xController( xFrame->getController() );
    if ( xController.is() )
    {
        Reference< XModel > xModel( xController->getModel(), UNO_QUERY );
        if ( xModel.is() )
            xComponent.set( xModel, UNO_QUERY );
        else
            xComponent.set( xController, UNO_QUERY );
    }

    FrameDispatchMap::iterator aFramesIt = aFrames.find( xFrame );
    if ( aFramesIt == aFrames.end() )
    {
        CommandDispatchMap aMap;
        xRet.set(  ( * pDispatchInfo->Create )( xContext, xFrame, rModuleIdentifier ) );
        aMap.insert( CommandDispatchMap::value_type( sCommandURLPath, CommandDispatchInfo( xComponent, xRet ) ) );
        aFrames[xFrame] = aMap;

        xFrame->addEventListener( aPool.get() );
        bListen = true;
    }
    else
    {
        // found XFrame
        CommandDispatchMap &aMap = aFramesIt->second;
        // find Command
        const CommandDispatchMap::iterator &aDispatchesIt = aMap.find( sCommandURLPath );
        if ( aDispatchesIt == aMap.end() )
        {
            // Command not found, create dispatch
            xRet.set(  ( * pDispatchInfo->Create )( xContext, xFrame, rModuleIdentifier ) );
            aMap.insert( CommandDispatchMap::value_type( sCommandURLPath, CommandDispatchInfo( xComponent, xRet ) ) );
            bListen = true;
        }
        else
        {
            // command found
            CommandDispatchInfo &aInfo = aDispatchesIt->second;
            OSL_ENSURE( aInfo.Component == xComponent, "Requesting a dispatch for a different component!" );
            xRet.set( aInfo.Dispatch );
        }
    }

    if ( bListen )
    {
        xComponent->addEventListener( aPool.get() );
    }

    return xRet;
}
