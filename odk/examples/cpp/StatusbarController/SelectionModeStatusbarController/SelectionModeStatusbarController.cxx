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

#include "SelectionModeStatusbarController.hxx"
#include "macros.hxx"
#include "defines.hxx"

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/awt/Command.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/awt/PopupMenuDirection.hpp>

#include <map>

using namespace framework::statusbar_controller_selectionmode;

using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;

using rtl::OUString;

namespace
{
    static OUString st_SelectionModeCmd = OUSTR_COMMAND_UNO_SELMODE;

    typedef std::pair< OUString, OUString > SelectionInfo;
    typedef std::map< SelectionMode, SelectionInfo > SelectionMap;

    struct StaticSelectionMapInit
    {
            SelectionMap *
            operator()()
            {
                static SelectionMap aMap;
                lcl_InitMap( aMap );
                return &aMap;
            }
        private:
            void lcl_InitMap( SelectionMap &aMap )
            {
                aMap.insert( SelectionMap::value_type( SEL_MODE_STD, SelectionInfo( C2U( "STD" ), C2U( "Standard selection mode" ) ) ) );
                aMap.insert( SelectionMap::value_type( SEL_MODE_EXT, SelectionInfo( C2U( "EXT" ), C2U( "Extended selection mode" ) ) ) );
                aMap.insert( SelectionMap::value_type( SEL_MODE_ADD, SelectionInfo( C2U( "ADD" ), C2U( "Additional selection mode" ) ) ) );
                aMap.insert( SelectionMap::value_type( SEL_MODE_BLK, SelectionInfo( C2U( "BLK" ), C2U( "Block selection mode" ) ) ) );
            }
    };

    struct StaticSelectionModeInfo
            : public rtl::StaticAggregate< SelectionMap, StaticSelectionMapInit > {};

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

SelectionModeStatusbarController::SelectionModeStatusbarController(
    const Reference< XComponentContext> &rxContext )
    : SelectionModeStatusbarController_Base()
    , m_bEnabled( false )
    , m_bSelectionModeEnabled( false )
    , m_eSelectionMode( SEL_MODE_STD )
{
    OSL_TRACE( "sbctlselmode::SelectionModeStatusbarController::SelectionModeStatusbarController" );
    m_xContext = rxContext;
}

SelectionModeStatusbarController::~SelectionModeStatusbarController( )
{
    OSL_TRACE( "sbctlselmode::SelectionModeStatusbarController::~SelectionModeStatusbarController" );
}

void SAL_CALL
SelectionModeStatusbarController::statusChanged(
    const FeatureStateEvent &aFeatureState )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlselmode::SelectionModeStatusbarController::statusChanged" );
    osl::ResettableMutexGuard aGuard( m_aMutex );

    if ( aFeatureState.FeatureURL.Complete.equals( st_SelectionModeCmd ) )
    {
        if ( ( m_bSelectionModeEnabled = aFeatureState.IsEnabled ) )
        {
            sal_Int32 nValue( 0 );
            aFeatureState.State >>= nValue;
            m_eSelectionMode = SelectionMode( nValue );

            const SelectionMap &aSelInfoMap = *StaticSelectionModeInfo::get();
            const SelectionMap::const_iterator it = aSelInfoMap.find( m_eSelectionMode );
            if ( it != aSelInfoMap.end() )
            {
                const SelectionInfo &aInfo = it->second;
                m_xStatusbarItem->setText( aInfo.first );
                m_xStatusbarItem->setQuickHelpText( aInfo.second );
                m_xStatusbarItem->setAccessibleName( aInfo.second );
            }
        }
    }
}

void SAL_CALL
SelectionModeStatusbarController::initialize(
    const Sequence< Any > &aArguments )
throw ( Exception, RuntimeException )
{
    SelectionModeStatusbarController_Base::initialize( aArguments );

    osl::MutexGuard aGuard( m_aMutex );
    m_aListenerMap.insert( URLToDispatchMap::value_type( st_SelectionModeCmd, Reference< XDispatch >() ) );
}

void SAL_CALL
SelectionModeStatusbarController::command(
    const Point &aPos,
    ::sal_Int32 nCommand,
    ::sal_Bool /*bMouseEvent*/,
    const Any &/*aData*/ )
throw ( RuntimeException )
{
    if ( nCommand & Command::CONTEXTMENU )
    {
        ExecutePopupMenu( aPos );
    }
}

void SAL_CALL
SelectionModeStatusbarController::click(  const Point &aPos )
throw ( RuntimeException )
{
    ExecutePopupMenu( aPos );
}

void SelectionModeStatusbarController::ExecutePopupMenu( const Point &aPos )
{
    osl::ClearableMutexGuard aGuard( m_aMutex );
    if ( !m_bSelectionModeEnabled || !m_xFrame.is() )
        return;

    try
    {
        Reference< XPopupMenu > xPopupMenu(
            m_xContext->getServiceManager()->createInstanceWithContext(
                OUSTR_SERVICENAME_POPUPMENU, m_xContext ),
            UNO_QUERY );
        if ( !xPopupMenu.is() )
            return;

        const SelectionMap &aMap = *StaticSelectionModeInfo::get();
        sal_Int16 nPos = 0;
        const sal_Int16 nIdOffset = 100;
        for ( SelectionMap::const_iterator it = aMap.begin(); it != aMap.end(); nPos++, it++ )
        {
            const SelectionMode eMode = it->first;
            const SelectionInfo &aInfo = it->second;
            xPopupMenu->insertItem( nIdOffset + sal_Int16( eMode ), aInfo.second, 0, nPos );
        }

        xPopupMenu->checkItem( nIdOffset + m_eSelectionMode, sal_True );

        Reference< XWindowPeer > xParent( m_xStatusbarWindow, UNO_QUERY );
        const sal_Int16 nSelId = xPopupMenu->execute( xParent, aPos, PopupMenuDirection::EXECUTE_UP );
        if ( nSelId )
        {
            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = C2U( "SelectionMode" );
            aArgs[0].Value <<= sal_Int32( nSelId - nIdOffset );

            Execute( st_SelectionModeCmd, aArgs );
        }
    }
    catch ( ... )
    {}

}

OUString SAL_CALL
SelectionModeStatusbarController::getImplementationName( )
throw ( RuntimeException )
{
    return getImplementationName_static();
}

::sal_Bool SAL_CALL
SelectionModeStatusbarController::supportsService(
    const OUString &ServiceName )
throw ( RuntimeException )
{
    const Sequence< OUString > &aServices = StaticSupportedServices::get();

    for ( const OUString *pService = aServices.getConstArray(),
            *pEnd = pService + aServices.getLength();
            pService != pEnd;
            pService++ )
    {
        if ( pService->equals( ServiceName ) )
            return sal_True;
    }

    return sal_False;
}

Sequence< OUString > SAL_CALL
SelectionModeStatusbarController::getSupportedServiceNames( )
throw ( RuntimeException )
{
    return getSupportedServiceNames_static();
}

Reference< XInterface >
SelectionModeStatusbarController::Create(
    const Reference< XComponentContext > &rxContext )
throw ( Exception )
{
    OSL_TRACE( "sbctlselmode::SelectionModeStatusbarController::Create" );
    return Reference< XInterface >( static_cast< cppu::OWeakObject *>( new SelectionModeStatusbarController( rxContext ) ) );
}

Sequence< OUString >
SelectionModeStatusbarController::getSupportedServiceNames_static( )
{
    return StaticSupportedServices::get();
}

OUString
SelectionModeStatusbarController::getImplementationName_static( )
{
    static OUString st_ImplName( RTL_CONSTASCII_USTRINGPARAM( STATUSBAR_CONTROLLER_IMPL_NAME ) );
    return st_ImplName;
}

