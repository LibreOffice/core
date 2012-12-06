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

#include "WordCountDispatch.hxx"
#include "defines.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/Pair.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/i18n/WordType.hpp>

#include <rtl/ustrbuf.hxx>
#include <rtl/instance.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/implbase1.hxx>

#include <vector>
#include <map>
#include <algorithm>
#include <set>

using namespace framework::statusbar_controller_wordcount;

using namespace com::sun::star::awt;
using namespace com::sun::star::i18n;
using namespace com::sun::star::container;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::text;
using namespace com::sun::star::util;
using namespace com::sun::star::view;
using namespace com::sun::star::frame;

using rtl::OUString;
using rtl::OUStringBuffer;

namespace
{
    static OUString st_OwnCommand = OUSTR_COMMAND_STATUSWORDCOUNT;

    class AsyncDispatcher : public cppu::WeakImplHelper1< XCallback >
    {
        public:
            AsyncDispatcher() {}
            ~AsyncDispatcher() {}

            virtual void SAL_CALL notify( const Any &aData )
            throw ( RuntimeException )
            {
                Pair< URL, Reference< XDispatch > > aDispatchInfo;
                if ( ( aData >>= aDispatchInfo ) && aDispatchInfo.Second.is() )
                {
                    try
                    {
                        aDispatchInfo.Second->dispatch( aDispatchInfo.First,
                                                        Sequence<PropertyValue>( 0 ) );
                    }
                    catch ( ... )
                    {}
                }
            }
    };

    struct StaticAsyncDispatcher
            : public rtl::StaticWithInit< rtl::Reference< AsyncDispatcher >, StaticAsyncDispatcher>
    {
        rtl::Reference< AsyncDispatcher >
        operator()()
        {
            OSL_TRACE( "sbctlwc: Initializing static AsyncDispatcher" );
            return rtl::Reference< AsyncDispatcher >( new AsyncDispatcher() );
        }
    };

    template < class REF >
    void lcl_DisposeAndClear( REF &rUnoRef )
    {
        Reference< XComponent > xComp( rUnoRef, UNO_QUERY );
        if ( xComp.is() )
        {
            try
            {
                xComp->dispose();
            }
            catch ( ... )
            {
            }
        }
        rUnoRef.clear();
    }

    struct DefaultFeatureState
            : public rtl::StaticWithInit< FeatureStateEvent, DefaultFeatureState >
    {
        FeatureStateEvent
        operator()()
        {
            FeatureStateEvent aFeatureState;
            aFeatureState.IsEnabled = sal_False;
            aFeatureState.Requery = sal_False;

            URL aURL;
            aURL.Complete = st_OwnCommand;
            aFeatureState.FeatureURL = aURL;

            Any aVal;
            aVal <<= sal_uInt32( 0 );
            Sequence< NamedValue > aArgs( 2 );
            aArgs[0].Name = C2U( "WordCount" );
            aArgs[0].Value = aVal;
            aArgs[1].Name = C2U( "Selection" );
            aArgs[1].Value = aVal;
            aFeatureState.State <<= aArgs;

            return aFeatureState;
        }
    };

    typedef std::set< OUString > SupportedModulesSet;

    struct SupportedModulesInit
    {
            SupportedModulesSet *
            operator()()
            {
                static SupportedModulesSet aSupportedModules;
                lcl_InitSet( aSupportedModules );
                return &aSupportedModules;
            }
        private:
            void lcl_InitSet( SupportedModulesSet &aSet )
            {
                // TODO add all text document services
                aSet.insert( C2U( STR_MODULE_TEXTDOCUMENT ) );
            }
    };

    struct SupportedModules
            : public rtl::StaticAggregate< SupportedModulesSet, SupportedModulesInit > {};
}

WordCountDispatch::WordCountDispatch(
    const Reference< XComponentContext > &rxContext,
    const Reference< XFrame > &rxFrame,
    const OUString &rModuleIdentifier )
    : BaseDispatch( rxContext, rxFrame, rModuleIdentifier )
    , m_aCommand( st_OwnCommand )
    , m_bIsModified( false )
    , m_bInGetStatus( false )
    , m_xBreakIterator()
    , m_xRequestCallback()
{
    OSL_TRACE( "sbctlwc::WordCountDispatch::WordCountDispatch" );
    try
    {
        m_xURLTransformer.set(
            m_xContext->getServiceManager()->createInstanceWithContext(
                OUSTR_SERVICENAME_URLTRANSFORMER, m_xContext ),
            UNO_QUERY_THROW );

        m_xBreakIterator.set(
            m_xContext->getServiceManager()->createInstanceWithContext(
                OUSTR_SERVICENAME_BREAKITERATOR, m_xContext ),
            UNO_QUERY_THROW );

        m_xRequestCallback.set(
            m_xContext->getServiceManager()->createInstanceWithContext(
                OUSTR_SERVICENAME_CALLBACK, m_xContext ),
            UNO_QUERY_THROW );

        Reference< XController > xController( m_xFrame->getController() );
        m_xSelectionSupplier.set( xController, UNO_QUERY_THROW );
        if ( m_xSelectionSupplier.is() )
            m_xSelectionSupplier->addSelectionChangeListener( this );

        m_xModifiable.set( xController->getModel(), UNO_QUERY_THROW );
        Reference< XModifyBroadcaster > xBroadcaster( m_xModifiable, UNO_QUERY_THROW );
        if ( xBroadcaster.is() )
            xBroadcaster->addModifyListener( this );

        static bool bURLParsed = false;
        if ( !bURLParsed )
        {
            FeatureStateEvent &aFeatureState = DefaultFeatureState::get();
            m_xURLTransformer->parseStrict( aFeatureState.FeatureURL );
            bURLParsed = true;
        }
    }
    catch ( ... )
    {
    }
}

WordCountDispatch::~WordCountDispatch( )
{
    OSL_TRACE( "sbctlwc::WordCountDispatch::~WordCountDispatch" );
}

Reference< XDispatch >
WordCountDispatch::Create(
    const Reference< XComponentContext > &rxContext,
    const Reference< XFrame > &rxFrame,
    const OUString &rModuleIdentifier )
{
    OSL_TRACE( "sbctlwc::WordCountDispatch::Create" );
    return Reference< XDispatch > (
               static_cast< cppu::OWeakObject * >(
                   new WordCountDispatch( rxContext, rxFrame, rModuleIdentifier ) ),
               UNO_QUERY );
}

bool WordCountDispatch::SupportsURL(
    const URL &aURL,
    const OUString &rModuleIdentifier )
{
    const SupportedModulesSet &aModules = *SupportedModules::get();
    return aModules.find( rModuleIdentifier ) != aModules.end()
           && aURL.Complete.equals( st_OwnCommand );
}

void SAL_CALL
WordCountDispatch::disposing( )
{
    OSL_TRACE( "sbctlwc::WordCountDispatch::disposing" );
    osl::ClearableMutexGuard aGuard( m_aMutex );
    // When the dispatch pool disposes us, stop listening
    // otherwise the model will keep us alive
    if ( m_xModifiable.is() )
    {
        m_xModifiable->removeModifyListener( this );
        m_xModifiable.clear();
    }

    if ( m_xSelectionSupplier.is() )
    {
        m_xSelectionSupplier->removeSelectionChangeListener( this );
        m_xModifiable.clear();
    }

    m_xBreakIterator.clear();
    m_xFrame.clear();
    lcl_DisposeAndClear( m_xRequestCallback );
    lcl_DisposeAndClear( m_xURLTransformer );

    aGuard.clear();

    BaseDispatch::disposing( );
}


void SAL_CALL WordCountDispatch::modified(
    const EventObject &aEvent )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlwc::WordCountDispatch::modified" );
    // prevent endless loop:
    // getting the document word count triggers a document modified event
    if ( m_bInGetStatus )
        return;

    Reference< XModifiable > xModifiable( aEvent.Source, UNO_QUERY );
    if ( xModifiable.is() )
    {
        bool bModified;
        osl::ClearableMutexGuard aLock( m_aMutex );
        m_bIsModified = bModified = xModifiable->isModified();
        aLock.clear( );

        if ( bModified )
            BaseDispatch::modified( aEvent );
    }
}


void SAL_CALL WordCountDispatch::selectionChanged(
    const EventObject &aEvent )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlwc::WordCountDispatch::selectionChanged" );
    if ( m_bInGetStatus )
        return;

    BaseDispatch::selectionChanged( aEvent );
}


void SAL_CALL WordCountDispatch::disposing(
    const EventObject &aEvent )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlwc::WordCountDispatch::disposing(aEvent)" );
    Reference< XModifiable > xModifiable( aEvent.Source, UNO_QUERY );
    if ( xModifiable.is() && xModifiable == m_xModifiable )
    {
        OSL_TRACE( "disposing XModel" );
        m_xModifiable->removeModifyListener( this );
        m_xModifiable.clear();
        return;
    }

    Reference< XSelectionSupplier > xSelectionSupplier( aEvent.Source, UNO_QUERY );
    if ( xSelectionSupplier.is() && xSelectionSupplier == m_xSelectionSupplier )
    {
        OSL_TRACE( "disposing XController" );
        m_xSelectionSupplier->removeSelectionChangeListener( this );
        m_xSelectionSupplier.clear();
    }
}


void
WordCountDispatch::ExecuteCommand(
    const URL &rURL,
    const Sequence< PropertyValue > &/*lArguments*/ )
{
    OSL_TRACE( "sbctlwc::WordCountDispatch::ExecuteCommand" );
    if ( rURL.Complete.equals( st_OwnCommand ) )
        ShowWordCountDialog();
}

FeatureStateEvent
WordCountDispatch::GetState( const URL &rURL )
{
    OSL_TRACE( "sbctlwc::WordCountDispatch::GetState" );
    FeatureStateEvent aEvent = DefaultFeatureState::get();
    aEvent.Source = Reference<XDispatch > ( this );

    osl::MutexGuard aGuard( m_aMutex );

    // prevent endless loop:
    // getting the document word count triggers a document modified event
    m_bInGetStatus = true;
    try
    {
        if ( rURL.Complete.equals( GetCommand() ) )
        {
            sal_Int32 nWordCount( 0 );
            sal_Int32 nSelWords( 0 );

            Reference< XController> xController = m_xFrame->getController( );
            Reference<XSelectionSupplier> xSelSuppl( xController, UNO_QUERY );
            Any aSelection;
            const bool bHasSelection = xSelSuppl.is() && ( aSelection = xSelSuppl->getSelection() ).hasValue();

            Reference< XModel > xModel( xController->getModel( ) );
            Reference< XPropertySet> xDocPropSet( xModel, UNO_QUERY );
            Reference< XModifiable > xModifiable( xModel, UNO_QUERY );

            // WARNING: this will trigger a document modified event
            xDocPropSet->getPropertyValue( C2U( "WordCount" ) ) >>= nWordCount;

            if ( bHasSelection )
            {
                Reference< XServiceInfo > xServiceInfo( aSelection, UNO_QUERY );
                if ( xServiceInfo.is( ) && xServiceInfo->supportsService( C2U( "com.sun.star.text.TextRanges" ) ) )
                {
                    Reference< XIndexAccess > xIndexAccess( xServiceInfo, UNO_QUERY );
                    for ( sal_Int32 n = 0; n < xIndexAccess->getCount( ); n++ )
                    {
                        try
                        {
                            OUString sText;
                            Reference< XTextRange > xTextRange( xIndexAccess->getByIndex( n ), UNO_QUERY );
                            Reference< XTextRangeCompare > xRangeCompare( xTextRange->getText(), UNO_QUERY );
                            if ( xRangeCompare->compareRegionStarts( xTextRange->getStart(), xTextRange->getEnd() ) == 0
                                    || ( sText = xTextRange->getString() ).getLength() == 0 )
                                continue;
                            // TODO the text range has a Locale in CharLocale
                            // *if* all the text range has the same Locale, of course
                            // Use this Locale with the BreakIterator
                            nSelWords += CountWords( sText );
                        }
                        catch ( ... )
                        {}
                    }
                }
            }

            Sequence< NamedValue > aArgs;
            aEvent.State >>= aArgs;
            aArgs[0].Value <<= sal_uInt32( nWordCount );
            aArgs[1].Value <<= sal_uInt32( nSelWords );
            aEvent.State <<= aArgs;

            aEvent.IsEnabled = sal_True;

            // needed because the controller may die
            // for example, switching to Preview creates a new view/controller
            if ( !m_xSelectionSupplier.is( ) )
            {
                m_xSelectionSupplier.set( xSelSuppl );
                if ( m_xSelectionSupplier.is( ) )
                    m_xSelectionSupplier->addSelectionChangeListener( this );
            }

            // this seems not really needed: we live as the model lives
            if ( !m_xModifiable.is( ) )
            {
                m_xModifiable.set( xModifiable );
                Reference< XModifyBroadcaster > xBroadcaster( m_xModifiable, UNO_QUERY );
                if ( xBroadcaster.is( ) )
                    xBroadcaster->addModifyListener( this );
            }
        }
    }
    catch ( ... )
    {
    }

    m_bInGetStatus = false;

    return aEvent;
}

sal_Int32
WordCountDispatch::CountWords( const OUString &sText )
{
    sal_Int32 nWords( 1 );
    sal_Int32 nStartPos( 1 );
    Locale aLocale;

    Boundary aNext = m_xBreakIterator->nextWord(
                         sText,
                         nStartPos,
                         aLocale,
                         WordType::WORD_COUNT );
    while ( aNext.startPos != aNext.endPos )
    {
        nWords++;
        nStartPos = aNext.startPos;
        aNext = m_xBreakIterator->nextWord( sText,
                                            nStartPos,
                                            aLocale,
                                            WordType::WORD_COUNT );
    }
    return nWords;
}

void
WordCountDispatch::ShowWordCountDialog() const
{
    Reference< XURLTransformer > xURLParser;
    Reference< XFrame > xFrame;
    Reference< XRequestCallback > xRequestCallback;

    osl::ClearableMutexGuard aLock( m_aMutex );
    xURLParser = m_xURLTransformer;
    xFrame = m_xFrame;
    xRequestCallback = m_xRequestCallback;
    aLock.clear( );

    try
    {
        URL aCommand;
        aCommand.Complete = OUSTR_COMMAND_UNO_WORDCOUNTDLG;
        xURLParser->parseStrict( aCommand );

        Reference< XDispatchProvider > xProvider( xFrame, UNO_QUERY );
        Reference< XDispatch > xDispatch = xProvider->queryDispatch( aCommand, C2U( "_self" ), 0 );
        if ( xRequestCallback.is() && xDispatch.is() )
        {
            xRequestCallback->addCallback(
                StaticAsyncDispatcher::get().get(),
                makeAny( Pair< URL, Reference< XDispatch > >( aCommand, xDispatch ) ) );
        }
    }
    catch ( ... )
    {
    }
}
