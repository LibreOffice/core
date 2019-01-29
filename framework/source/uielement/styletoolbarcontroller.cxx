/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <uielement/styletoolbarcontroller.hxx>

#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <sal/log.hxx>

#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/status/Template.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

namespace {

OUString MapFamilyToCommand( const OUString& rFamily )
{
    if ( rFamily == "ParagraphStyles" ||
         rFamily == "CellStyles" ||         // In sc
         rFamily == "graphics" )            // In sd
        return OUString( ".uno:ParaStyle" );
    else if ( rFamily == "CharacterStyles" )
        return OUString( ".uno:CharStyle" );
    else if ( rFamily == "PageStyles" )
        return OUString( ".uno:PageStyle" );
    else if ( rFamily == "FrameStyles" )
        return OUString( ".uno:FrameStyle" );
    else if ( rFamily == "NumberingStyles" )
        return OUString( ".uno:ListStyle" );
    else if ( rFamily == "TableStyles" )
        return OUString( ".uno:TableStyle" );

    return OUString();
}

OUString GetDisplayFromInternalName( const css::uno::Reference< css::frame::XFrame >& rFrame,
                                     const OUString& rStyleName,
                                     const OUString& rFamilyName )
{
    try
    {
        css::uno::Reference< css::frame::XController > xController(
            rFrame->getController(), css::uno::UNO_SET_THROW );
        css::uno::Reference< css::style::XStyleFamiliesSupplier > xStylesSupplier(
            xController->getModel(), css::uno::UNO_QUERY_THROW );
        css::uno::Reference< css::container::XNameAccess > xFamilies(
            xStylesSupplier->getStyleFamilies(), css::uno::UNO_SET_THROW );

        css::uno::Reference< css::container::XNameAccess > xStyleSet;
        xFamilies->getByName( rFamilyName ) >>= xStyleSet;
        css::uno::Reference< css::beans::XPropertySet > xStyle;
        xStyleSet->getByName( rStyleName ) >>= xStyle;

        OUString aDisplayName;
        if ( xStyle.is() )
            xStyle->getPropertyValue( "DisplayName" ) >>= aDisplayName;
        return aDisplayName;
    }
    catch ( const css::uno::Exception& )
    {
        // We couldn't get the display name. As a last resort we'll
        // try to use the internal name, as was specified in the URL.
    }

    return rStyleName;
}

}

namespace framework {

StyleDispatcher::StyleDispatcher( const css::uno::Reference< css::frame::XFrame >& rFrame,
                                  const css::uno::Reference< css::util::XURLTransformer >& rUrlTransformer,
                                  const css::util::URL& rURL )
    : m_aCommand( rURL.Complete )
    , m_xUrlTransformer( rUrlTransformer )
    , m_xFrame( rFrame, css::uno::UNO_QUERY )
{
    SAL_WARN_IF( !m_aCommand.startsWith( ".uno:StyleApply?" ), "fwk.uielement", "Wrong dispatcher!" );

    OUString aParams = rURL.Arguments;
    OUString aStyleName, aFamilyName;
    sal_Int32 nIndex = 0;
    do
    {
        OUString aParam = aParams.getToken( 0, '&', nIndex );

        sal_Int32 nParamIndex = 0;
        OUString aParamName = aParam.getToken( 0, '=', nParamIndex );
        if ( nParamIndex < 0 )
            break;

        if ( aParamName == "Style:string" )
        {
            OUString aValue = aParam.getToken( 0, '=', nParamIndex );
            aStyleName = INetURLObject::decode( aValue, INetURLObject::DecodeMechanism::WithCharset );
        }
        else if ( aParamName == "FamilyName:string" )
        {
            aFamilyName = aParam.getToken( 0, '=', nParamIndex );
        }

    } while ( nIndex >= 0 );

    m_aStatusCommand = MapFamilyToCommand( aFamilyName );
    if ( m_aStatusCommand.isEmpty() || aStyleName.isEmpty() )
    {
        // We can't provide status updates for this command, but just executing
        // the command should still work (given that the command is valid).
        SAL_WARN( "fwk.uielement", "Unable to parse as a style command: " << m_aCommand );
        return;
    }

    m_aStyleName = GetDisplayFromInternalName( rFrame, aStyleName, aFamilyName );
    if ( m_xFrame.is() )
    {
        css::util::URL aStatusURL;
        aStatusURL.Complete = m_aStatusCommand;
        m_xUrlTransformer->parseStrict( aStatusURL );
        m_xStatusDispatch = m_xFrame->queryDispatch( aStatusURL, OUString(), 0 );
    }
}

void StyleDispatcher::dispatch( const css::util::URL& rURL,
                                const css::uno::Sequence< css::beans::PropertyValue >& rArguments )
{
    if ( !m_xFrame.is() )
        return;

    css::uno::Reference< css::frame::XDispatch > xDispatch( m_xFrame->queryDispatch( rURL, OUString(), 0 ) );
    if ( xDispatch.is() )
        xDispatch->dispatch( rURL, rArguments );
}

void StyleDispatcher::addStatusListener( const css::uno::Reference< css::frame::XStatusListener >& rListener,
                                         const css::util::URL& /*rURL*/ )
{
    if ( m_xStatusDispatch.is() )
    {
        if ( !m_xOwner.is() )
            m_xOwner.set( rListener );

        css::util::URL aStatusURL;
        aStatusURL.Complete = m_aStatusCommand;
        m_xUrlTransformer->parseStrict( aStatusURL );
        m_xStatusDispatch->addStatusListener( this, aStatusURL );
    }
}

void StyleDispatcher::removeStatusListener( const css::uno::Reference< css::frame::XStatusListener >& /*rListener*/,
                                            const css::util::URL& /*rURL*/ )
{
    if ( m_xStatusDispatch.is() )
    {
        css::util::URL aStatusURL;
        aStatusURL.Complete = m_aStatusCommand;
        m_xUrlTransformer->parseStrict( aStatusURL );
        m_xStatusDispatch->removeStatusListener( this, aStatusURL );
    }
}

void StyleDispatcher::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    css::frame::status::Template aTemplate;
    rEvent.State >>= aTemplate;

    css::frame::FeatureStateEvent aEvent;
    aEvent.FeatureURL.Complete = m_aCommand;
    m_xUrlTransformer->parseStrict( aEvent.FeatureURL );

    aEvent.IsEnabled = rEvent.IsEnabled;
    aEvent.Requery = rEvent.Requery;
    aEvent.State <<= m_aStyleName == aTemplate.StyleName;
    m_xOwner->statusChanged( aEvent );
}

void StyleDispatcher::disposing( const css::lang::EventObject& /*rSource*/ )
{
    m_xStatusDispatch.clear();
}

StyleToolbarController::StyleToolbarController( const css::uno::Reference< css::uno::XComponentContext >& rContext,
                                                const css::uno::Reference< css::frame::XFrame >& rFrame,
                                                const OUString& rCommand )
    : ToolboxController( rContext, rFrame, rCommand )
{
}

void StyleToolbarController::update()
{
    if ( m_bDisposed )
        throw css::lang::DisposedException();

    css::util::URL aURL;
    aURL.Complete = m_aCommandURL;
    m_xUrlTransformer->parseStrict( aURL );

    auto& xDispatcher = m_aListenerMap[m_aCommandURL];
    if ( xDispatcher.is() )
        xDispatcher->removeStatusListener( this, aURL );

    xDispatcher.set( new StyleDispatcher( m_xFrame, m_xUrlTransformer, aURL ) );
    xDispatcher->addStatusListener( this, aURL );
}

void StyleToolbarController::statusChanged( const css::frame::FeatureStateEvent& rEvent )
{
    SolarMutexGuard aGuard;

    if ( m_bDisposed )
        throw css::lang::DisposedException();

    ToolBox* pToolBox = nullptr;
    sal_uInt16 nItemId = 0;
    if ( getToolboxId( nItemId, &pToolBox ) )
    {
        bool bChecked = false;
        rEvent.State >>= bChecked;
        pToolBox->CheckItem( nItemId, bChecked );
        pToolBox->EnableItem( nItemId, rEvent.IsEnabled );
    }
}

void StyleToolbarController::dispose()
{
    ToolboxController::dispose();
    m_aListenerMap.clear(); // Break the cycle with StyleDispatcher.
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
