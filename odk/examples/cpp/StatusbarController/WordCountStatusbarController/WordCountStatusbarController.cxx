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

#include "WordCountStatusbarController.hxx"
#include "macros.hxx"
#include "defines.hxx"

#include <rtl/ustrbuf.hxx>
#include <com/sun/star/beans/NamedValue.hpp>

using namespace framework::statusbar_controller_wordcount;

using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;

using rtl::OUString;

namespace
{
    static OUString st_WordCountDlgCmd = OUSTR_COMMAND_UNO_WORDCOUNTDLG;

    struct StaticSupportedServices
            : public rtl::StaticWithInit< Sequence< OUString >, StaticSupportedServices >
    {
        Sequence< OUString >
        operator()()
        {
            Sequence< OUString > aServices( 2 );
            aServices[0] = C2U( STATUSBAR_CONTROLLER_SERVICE_NAME );
            aServices[1] = C2U( "com.sun.star.frame.StatusbarController" );
            return aServices;
        }
    };
}

WordCountStatusbarController::WordCountStatusbarController(
    const Reference< XComponentContext> &rxContext )
    : WordCountStatusbarController_Base()
    , m_bEnabled( false )
    , m_bWordCountDlgEnabled( false )
{
    OSL_TRACE( "sbctlwc::WordCountStatusbarController::WordCountStatusbarController" );
    m_xContext = rxContext;
}

WordCountStatusbarController::~WordCountStatusbarController( )
{
    OSL_TRACE( "sbctlwc::WordCountStatusbarController::~WordCountStatusbarController" );
}

void SAL_CALL
WordCountStatusbarController::statusChanged(
    const FeatureStateEvent &aFeatureState )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlwc::WordCountStatusbarController::statusChanged" );
    osl::ResettableMutexGuard aGuard( m_aMutex );

    if ( aFeatureState.FeatureURL.Complete.equals( m_sCommandURL ) )
    {
        OUString aStateValue;
        m_bEnabled = aFeatureState.IsEnabled;
        if ( m_bEnabled )
        {
            Sequence< NamedValue > aArgs;
            sal_uInt32 nWordCount( 0 );
            sal_uInt32 nSelection( 0 );
            if ( aFeatureState.State >>= aArgs )
            {
                for ( const NamedValue *pNamedValue = aArgs.getConstArray(),
                        *pEnd = pNamedValue + aArgs.getLength();
                        pNamedValue != pEnd;
                        pNamedValue++ )
                {
                    if ( pNamedValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "WordCount" ) ) )
                        pNamedValue->Value >>= nWordCount;
                    else if ( pNamedValue->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Selection" ) ) )
                        pNamedValue->Value >>= nSelection;
                }
            }

            rtl::OUStringBuffer aBuff;
            // TODO: localize me!
            aBuff.appendAscii( RTL_CONSTASCII_STRINGPARAM( "Words: " ) );
            if ( nSelection )
            {
                aBuff.append( sal_Int32( nSelection ) );
                aBuff.append( sal_Unicode( '/' ) );
            }
            aBuff.append( sal_Int32( nWordCount ) );
            aStateValue = aBuff.makeStringAndClear();
        }

        m_xStatusbarItem->setText( aStateValue );
    }
    else if ( aFeatureState.FeatureURL.Complete.equals( st_WordCountDlgCmd ) )
    {
        rtl::OUStringBuffer aBuff;
        // TODO: localize me!
        aBuff.appendAscii( RTL_CONSTASCII_STRINGPARAM( "Word count" ) );
        m_bWordCountDlgEnabled = aFeatureState.IsEnabled;
        if ( m_bWordCountDlgEnabled )
            aBuff.appendAscii( RTL_CONSTASCII_STRINGPARAM( ". Double-click to open the Word Count dialog" ) );

        OUString aTip = aBuff.makeStringAndClear();
        m_xStatusbarItem->setQuickHelpText( aTip );
        m_xStatusbarItem->setAccessibleName( aTip );
    }
}
void SAL_CALL
WordCountStatusbarController::initialize(
    const Sequence< Any > &aArguments )
throw ( Exception, RuntimeException )
{
    WordCountStatusbarController_Base::initialize( aArguments );

    osl::MutexGuard aGuard( m_aMutex );
    m_aListenerMap.insert( URLToDispatchMap::value_type( st_WordCountDlgCmd, Reference< XDispatch >() ) );
}

void SAL_CALL
WordCountStatusbarController::doubleClick()
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlwc::WordCountStatusbarController::doubleClick" );

    osl::ClearableMutexGuard aGuard( m_aMutex );
    if ( !m_bWordCountDlgEnabled )
        return;
    aGuard.clear();

    static const Sequence< PropertyValue > aDummy;
    Execute( st_WordCountDlgCmd, aDummy );
}

OUString SAL_CALL
WordCountStatusbarController::getImplementationName( )
throw ( RuntimeException )
{
    return getImplementationName_static();
}

::sal_Bool SAL_CALL
WordCountStatusbarController::supportsService(
    const OUString &ServiceName )
throw ( RuntimeException )
{
    const Sequence< OUString > &aServices = StaticSupportedServices::get();

    for ( const OUString *pService = aServices.getConstArray(),
            *pEnd = pService + aServices.getLength();
            pService != pEnd; pService++ )
    {
        if ( pService->equals( ServiceName ) )
            return sal_True;
    }

    return sal_False;
}

Sequence< OUString > SAL_CALL
WordCountStatusbarController::getSupportedServiceNames( )
throw ( RuntimeException )
{
    return getSupportedServiceNames_static();
}

Reference< XInterface >
WordCountStatusbarController::Create(
    const Reference< XComponentContext > &rxContext )
throw ( Exception )
{
    OSL_TRACE( "sbctlwc::WordCountStatusbarController::Create" );
    return Reference< XInterface >( static_cast< cppu::OWeakObject *>( new WordCountStatusbarController( rxContext ) ) );
}

Sequence< OUString >
WordCountStatusbarController::getSupportedServiceNames_static( )
{
    return StaticSupportedServices::get();
}

OUString
WordCountStatusbarController::getImplementationName_static( )
{
    static OUString st_ImplName( RTL_CONSTASCII_USTRINGPARAM( STATUSBAR_CONTROLLER_IMPL_NAME ) );
    return st_ImplName;
}

