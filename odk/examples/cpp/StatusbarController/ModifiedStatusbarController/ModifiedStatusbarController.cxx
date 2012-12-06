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

#include "ModifiedStatusbarController.hxx"
#include "defines.hxx"

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <com/sun/star/awt/ImageDrawMode.hpp>

using namespace framework::statusbar_controller_modified;

using namespace com::sun::star::uno;
using namespace com::sun::star::ui;
using namespace com::sun::star::awt;
using namespace com::sun::star::graphic;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;

using rtl::OUString;

namespace
{
    static OUString st_ModifiedStatusCmd = OUSTR_COMMAND_UNO_MODIFIED_STATUS;
    static Size st_GraphicSize;

    struct StaticStatusGraphic
            : public rtl::Static< Reference< XGraphic >, StaticStatusGraphic >
    {
    };

    struct StaticSupportedServices
            : public rtl::StaticWithInit< Sequence< OUString >, StaticSupportedServices >
    {

        Sequence< OUString >
        operator( )( )
        {
            Sequence< OUString > aServices( 2 );
            aServices[0] = C2U( STATUSBAR_CONTROLLER_SERVICE_NAME );
            aServices[1] = C2U( "com.sun.star.frame.StatusbarController" );
            return aServices;
        }
    };
}

ModifiedStatusbarController::ModifiedStatusbarController(
    const Reference< XComponentContext> &rxContext )
    : ModifiedStatusbarController_Base( )
    , m_bEnabled( false )
    , m_bModifiedStatusEnabled( false )
    , m_bIsDocumentModified( false )
{
    OSL_TRACE( "sbctlmodif::ModifiedStatusbarController::ModifiedStatusbarController" );
    m_xContext = rxContext;

    Reference< XGraphic > &xGraphic = StaticStatusGraphic::get( );
    if ( !xGraphic.is( ) )
    {
        try
        {
            Reference< XGraphicProvider > xProvider(
                m_xContext->getServiceManager( )->createInstanceWithContext(
                    OUSTR_SERVICENAME_GRAPHICPROVIDER, m_xContext ),
                UNO_QUERY_THROW );

            Sequence< PropertyValue > aMediaProps( 1 );
            aMediaProps[0].Name = C2U( "URL" );
            aMediaProps[0].Value <<= C2U( "vnd.sun.star.extension://" EXTENSION_IDENTIFIER "/images/statusbaricon.png" );

            xGraphic.set( xProvider->queryGraphic( aMediaProps ) );
            Reference< XPropertySet > xGraphicProps( xGraphic, UNO_QUERY_THROW );
            xGraphicProps->getPropertyValue( C2U( "SizePixel" ) ) >>= st_GraphicSize;

            OSL_ENSURE( st_GraphicSize.Height > 0 && st_GraphicSize.Width > 0, "Empty status bar graphic!" );
        }
        catch ( const Exception &e )
        {
            DISPLAY_EXCEPTION_ENSURE( ModifiedStatusbarController, ModifiedStatusbarController, e );
        }
    }
}

ModifiedStatusbarController::~ModifiedStatusbarController( )
{
    OSL_TRACE( "sbctlmodif::ModifiedStatusbarController::~ModifiedStatusbarController" );
}

void SAL_CALL
ModifiedStatusbarController::statusChanged(
    const FeatureStateEvent &aFeatureState )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlmodif::ModifiedStatusbarController::statusChanged" );
    osl::ResettableMutexGuard aGuard( m_aMutex );

    if ( aFeatureState.FeatureURL.Complete.equals( st_ModifiedStatusCmd ) )
    {
        OUString sTip;
        if ( ( m_bModifiedStatusEnabled = aFeatureState.IsEnabled ) )
        {
            OUString aStrValue;
            if ( aFeatureState.State >>= aStrValue )
            {
                m_bIsDocumentModified = aStrValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "*" ) );
            }

            if ( m_bIsDocumentModified )
                sTip = C2U( "The document is modified. Double-click to save it" );
            else
                sTip = C2U( "The document is not modified." );
        }

        m_xStatusbarItem->setQuickHelpText( sTip );
        m_xStatusbarItem->setAccessibleName( sTip );
        m_xStatusbarItem->repaint( );
    }
}

void SAL_CALL
ModifiedStatusbarController::initialize(
    const Sequence< Any > &aArguments )
throw ( Exception, RuntimeException )
{
    OSL_TRACE( "sbctlmodif::ModifiedStatusbarController::initialize" );
    ModifiedStatusbarController_Base::initialize( aArguments );

    osl::MutexGuard aGuard( m_aMutex );
    m_aListenerMap.insert( URLToDispatchMap::value_type( st_ModifiedStatusCmd, Reference< XDispatch > ( ) ) );
}

void SAL_CALL
ModifiedStatusbarController::paint(
    const Reference< XGraphics > &xGraphics,
    const Rectangle &rOutputRectangle,
    ::sal_Int32 /*nStyle*/ )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlmodif::ModifiedStatusbarController::paint" );
    Reference< XGraphic > &xGraphic = StaticStatusGraphic::get( );
    Reference< XStatusbarItem> xSbItem;

    osl::ClearableMutexGuard aGuard( m_aMutex );
    xSbItem = m_xStatusbarItem;
    bool bEnabled = m_bModifiedStatusEnabled && m_bIsDocumentModified;
    aGuard.clear( );

    if ( !xSbItem.is() || !xGraphics.is() )
        return;

    if ( xGraphic.is() && xGraphic->getType() != GraphicType::EMPTY )
    {
        sal_Int32 nOffset = xSbItem->getOffset( );
        Point aPos;
        aPos.X = ( rOutputRectangle.Width + nOffset ) / 2 - st_GraphicSize.Width / 2;
        aPos.Y = rOutputRectangle.Height / 2 - st_GraphicSize.Height / 2;

        xGraphics->drawImage( rOutputRectangle.X + aPos.X,
                              rOutputRectangle.Y + aPos.Y,
                              st_GraphicSize.Width,
                              st_GraphicSize.Height,
                              bEnabled ? ImageDrawMode::NONE : ImageDrawMode::DISABLE,
                              xGraphic );
    }
    else
    {
        xGraphics->clear( rOutputRectangle );
    }
}

void SAL_CALL
ModifiedStatusbarController::doubleClick(
    const Point &/*aPos*/ )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlmodif::ModifiedStatusbarController::doubleClick" );
    osl::ClearableMutexGuard aGuard( m_aMutex );
    if ( !m_bIsDocumentModified )
        return;
    aGuard.clear( );

    static const Sequence< PropertyValue > aDummyArgs;
    Execute( OUSTR_COMMAND_UNO_SAVE, aDummyArgs );
}

OUString SAL_CALL
ModifiedStatusbarController::getImplementationName( )
throw ( RuntimeException )
{
    return getImplementationName_static( );
}

::sal_Bool SAL_CALL
ModifiedStatusbarController::supportsService(
    const OUString &ServiceName )
throw ( RuntimeException )
{
    const Sequence< OUString > &aServices = StaticSupportedServices::get( );

    for ( const OUString *pService = aServices.getConstArray( ),
            *pEnd = pService + aServices.getLength( );
            pService != pEnd;
            pService++ )
    {
        if ( pService->equals( ServiceName ) )
            return sal_True;
    }

    return sal_False;
}

Sequence< OUString > SAL_CALL
ModifiedStatusbarController::getSupportedServiceNames( )
throw ( RuntimeException )
{
    return getSupportedServiceNames_static( );
}

Reference< XInterface >
ModifiedStatusbarController::Create(
    const Reference< XComponentContext > &rxContext )
throw ( Exception )
{
    OSL_TRACE( "sbctlmodif::ModifiedStatusbarController::Create" );
    return Reference< XInterface > ( static_cast < cppu::OWeakObject * > ( new ModifiedStatusbarController( rxContext ) ) );
}

Sequence< OUString >
ModifiedStatusbarController::getSupportedServiceNames_static( )
{
    return StaticSupportedServices::get( );
}

OUString
ModifiedStatusbarController::getImplementationName_static( )
{
    static OUString st_ImplName( RTL_CONSTASCII_USTRINGPARAM( STATUSBAR_CONTROLLER_IMPL_NAME ) );
    return st_ImplName;
}

