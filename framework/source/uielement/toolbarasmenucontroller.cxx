/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/implbase.hxx>
#include <svtools/popupmenucontrollerbase.hxx>
#include <vcl/menu.hxx>

#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

namespace framework {

class CommandStatusHelper : public cppu::WeakImplHelper< css::frame::XStatusListener >
{
public:
    CommandStatusHelper( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                         const css::uno::Reference< css::frame::XFrame >& rxFrame,
                         const OUString& rCommandURL );
    virtual ~CommandStatusHelper();

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;

    void updateCommand();
    bool isCommandEnabled() { return m_bEnabled; }
    bool isCommandChecked() { return m_bChecked; }

private:
    bool m_bEnabled;
    bool m_bChecked;
    OUString m_aCommandURL;
    css::uno::Reference< css::frame::XFrame > m_xFrame;
    css::uno::Reference< css::util::XURLTransformer > m_xURLTransformer;
};

CommandStatusHelper::CommandStatusHelper( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                          const css::uno::Reference< css::frame::XFrame >& rxFrame,
                                          const OUString& rCommandURL ) :
    m_bEnabled( true ),
    m_bChecked( false ),
    m_aCommandURL( rCommandURL ),
    m_xFrame( rxFrame ),
    m_xURLTransformer( css::util::URLTransformer::create( rxContext ) )
{
}

CommandStatusHelper::~CommandStatusHelper()
{
}

void CommandStatusHelper::updateCommand()
{
    css::uno::Reference< css::frame::XDispatchProvider > xDispatchProvider( m_xFrame, css::uno::UNO_QUERY );
    if ( xDispatchProvider.is() )
    {
        css::util::URL aTargetURL;
        aTargetURL.Complete = m_aCommandURL;
        m_xURLTransformer->parseStrict( aTargetURL );

        css::uno::Reference< css::frame::XDispatch > xDispatch( xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 ) );
        if ( xDispatch.is() )
        {
            xDispatch->addStatusListener( this, aTargetURL );
            xDispatch->removeStatusListener( this, aTargetURL );
        }
    }
}

void CommandStatusHelper::statusChanged( const css::frame::FeatureStateEvent& rEvent )
    throw ( css::uno::RuntimeException, std::exception )
{
    OUString aStringStatus;
    if ( rEvent.State >>= aStringStatus )
    {
        const OUString aEnumPart( rEvent.FeatureURL.Complete.getToken( 2, '.' ) );
        if ( !aEnumPart.isEmpty() && rEvent.FeatureURL.Protocol == ".uno:" )
            m_bChecked = aStringStatus == aEnumPart;
    }
    rEvent.State >>= m_bChecked;
    m_bEnabled = rEvent.IsEnabled;
}

void CommandStatusHelper::disposing( const css::lang::EventObject& /*rEvent*/ )
    throw ( css::uno::RuntimeException, std::exception )
{
    // We aren't holding reference to the dispatcher, so nothing to do here.
}


class ToolbarAsMenuController : public cppu::ImplInheritanceHelper< svt::PopupMenuControllerBase, css::ui::XUIConfigurationListener >
{
public:
    ToolbarAsMenuController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                             const css::uno::Sequence< css::uno::Any >& rxArgs );
    virtual ~ToolbarAsMenuController();

    // XPopupMenuController
    virtual void SAL_CALL updatePopupMenu() throw ( css::uno::RuntimeException, std::exception ) override;

    // XStatusListener
    virtual void SAL_CALL statusChanged( const css::frame::FeatureStateEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XUIConfigurationListener
    virtual void SAL_CALL elementInserted( const css::ui::ConfigurationEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL elementRemoved( const css::ui::ConfigurationEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL elementReplaced( const css::ui::ConfigurationEvent& rEvent ) throw ( css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw ( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw ( css::uno::RuntimeException, std::exception ) override;

private:
    OUString m_aToolbarURL;
    css::uno::Reference< css::container::XIndexAccess > m_xToolbarContainer;
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::ui::XUIConfigurationManager > m_xConfigManager, m_xModuleConfigManager;
    void fillPopupMenu();
    virtual void SAL_CALL disposing() override;
    virtual void impl_setPopupMenu() override;
};

ToolbarAsMenuController::ToolbarAsMenuController( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                                  const css::uno::Sequence< css::uno::Any >& rxArgs ) :
    ImplInheritanceHelper( rxContext ),
    m_xContext( rxContext )
{
    css::beans::PropertyValue aPropValue;
    for ( const auto& arg: rxArgs )
    {
        if ( ( arg >>= aPropValue ) && aPropValue.Name == "Value" )
        {
            OUString aToolbarName;
            aPropValue.Value >>= aToolbarName;
            m_aToolbarURL = "private:resource/toolbar/" + aToolbarName;
            break;
        }
    }
}

ToolbarAsMenuController::~ToolbarAsMenuController()
{
}

void ToolbarAsMenuController::impl_setPopupMenu()
{
    if ( m_aToolbarURL.isEmpty() )
        return;

    if ( !m_xConfigManager.is() )
    {
        try
        {
            css::uno::Reference< css::frame::XController > xController( m_xFrame->getController() );
            css::uno::Reference< css::frame::XModel > xModel( xController->getModel() );
            css::uno::Reference< css::ui::XUIConfigurationManagerSupplier > xSupplier( xModel, css::uno::UNO_QUERY_THROW );
            m_xConfigManager.set( xSupplier->getUIConfigurationManager() );
            css::uno::Reference< css::ui::XUIConfiguration > xConfig( m_xConfigManager, css::uno::UNO_QUERY_THROW );
            xConfig->addConfigurationListener( this );
        }
        catch( const css::uno::RuntimeException& )
        {}
    }

    if ( !m_xModuleConfigManager.is() )
    {
        try
        {
            css::uno::Reference< css::ui::XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier(
                css::ui::theModuleUIConfigurationManagerSupplier::get( m_xContext ) );
            m_xModuleConfigManager.set( xModuleCfgMgrSupplier->getUIConfigurationManager( m_aModuleName ) );
            css::uno::Reference< css::ui::XUIConfiguration > xConfig( m_xModuleConfigManager, css::uno::UNO_QUERY_THROW );
            xConfig->addConfigurationListener( this );
        }
        catch ( const css::container::NoSuchElementException& )
        {
            SAL_WARN( "fwk.uielement", "Invalid module identifier: " << m_aModuleName );
        }
        catch( const css::uno::RuntimeException& )
        {}
    }

    try
    {
        if ( m_xConfigManager.is() && m_xConfigManager->hasSettings( m_aToolbarURL ) )
            m_xToolbarContainer.set( m_xConfigManager->getSettings( m_aToolbarURL, sal_False ) );
        else if ( m_xModuleConfigManager.is() && m_xModuleConfigManager->hasSettings( m_aToolbarURL ) )
            m_xToolbarContainer.set( m_xModuleConfigManager->getSettings( m_aToolbarURL, sal_False ) );
    }
    catch ( const css::container::NoSuchElementException& )
    {
        SAL_WARN( "fwk.uielement", "Can not find settings for " << m_aToolbarURL );
    }
    catch ( const css::lang::IllegalArgumentException& )
    {
        SAL_WARN( "fwk.uielement", "The given URL is not valid: " << m_aToolbarURL );
    }

    fillPopupMenu();
}

void ToolbarAsMenuController::fillPopupMenu()
{
    resetPopupMenu( m_xPopupMenu );
    if ( !m_xToolbarContainer.is() )
        return;

    VCLXMenu* pAwtMenu = VCLXMenu::GetImplementation( m_xPopupMenu );
    Menu* pVCLMenu = pAwtMenu->GetMenu();

    css::uno::Sequence< css::beans::PropertyValue > aPropSequence;
    for ( sal_Int32 i = 0; i < m_xToolbarContainer->getCount(); ++i )
    {
        try
        {
            if ( m_xToolbarContainer->getByIndex( i ) >>= aPropSequence )
            {
                OUString aCommandURL;
                sal_uInt16 nType = css::ui::ItemType::DEFAULT;
                bool bVisible = true;

                for ( const auto& aProp: aPropSequence )
                {
                    if ( aProp.Name == "CommandURL" )
                        aProp.Value >>= aCommandURL;
                    else if ( aProp.Name == "Type" )
                        aProp.Value >>= nType;
                    else if ( aProp.Name == "IsVisible" )
                        aProp.Value >>= bVisible;
                }

                switch ( nType )
                {
                case css::ui::ItemType::DEFAULT:
                    if ( bVisible )
                        pVCLMenu->InsertItem( aCommandURL, m_xFrame );
                    break;
                case css::ui::ItemType::SEPARATOR_LINE:
                case css::ui::ItemType::SEPARATOR_LINEBREAK:
                    if ( bVisible && pVCLMenu->GetItemId( pVCLMenu->GetItemCount() - 1 ) != 0 )
                        pVCLMenu->InsertSeparator();
                    break;
                default: ;
                }
            }
        }
        catch ( const css::uno::Exception& )
        {
            break;
        }
    }
}

void ToolbarAsMenuController::updatePopupMenu()
    throw ( css::uno::RuntimeException, std::exception )
{
    VCLXMenu* pAwtMenu = VCLXMenu::GetImplementation( m_xPopupMenu );
    Menu* pVCLMenu = pAwtMenu->GetMenu();

    for ( sal_uInt16 i = 0; i < pVCLMenu->GetItemCount(); ++i )
    {
        const sal_uInt16 nItemId( pVCLMenu->GetItemId( i ) );
        const OUString aCommandURL( pVCLMenu->GetItemCommand( nItemId ) );
        if ( !aCommandURL.isEmpty() )
        {
            rtl::Reference< CommandStatusHelper > xHelper( new CommandStatusHelper( m_xContext, m_xFrame, aCommandURL ) );
            xHelper->updateCommand();
            pVCLMenu->EnableItem( nItemId, xHelper->isCommandEnabled() );
            pVCLMenu->CheckItem( nItemId, xHelper->isCommandChecked() );
        }
    }
}

void ToolbarAsMenuController::disposing()
{
    svt::PopupMenuControllerBase::disposing();

    css::uno::Reference< css::ui::XUIConfiguration > xConfig( m_xConfigManager, css::uno::UNO_QUERY );
    if ( xConfig.is() )
        xConfig->removeConfigurationListener( this );

    css::uno::Reference< css::ui::XUIConfiguration > xModuleConfig( m_xModuleConfigManager, css::uno::UNO_QUERY );
    if ( xModuleConfig.is() )
        xModuleConfig->removeConfigurationListener( this );

    m_xConfigManager.clear();
    m_xModuleConfigManager.clear();
}

void ToolbarAsMenuController::statusChanged( const css::frame::FeatureStateEvent& /*rEvent*/ )
    throw ( css::uno::RuntimeException, std::exception )
{
}

void ToolbarAsMenuController::disposing( const css::lang::EventObject& rEvent )
    throw ( css::uno::RuntimeException, std::exception )
{
    if ( rEvent.Source == m_xConfigManager )
        m_xConfigManager.clear();
    else if ( rEvent.Source == m_xModuleConfigManager )
        m_xModuleConfigManager.clear();
    else
        svt::PopupMenuControllerBase::disposing( rEvent );
}

void ToolbarAsMenuController::elementInserted( const css::ui::ConfigurationEvent& rEvent )
    throw ( css::uno::RuntimeException, std::exception )
{
    if ( rEvent.ResourceURL == m_aToolbarURL )
        impl_setPopupMenu();
}

void ToolbarAsMenuController::elementRemoved( const css::ui::ConfigurationEvent& rEvent )
    throw ( css::uno::RuntimeException, std::exception )
{
    if ( rEvent.ResourceURL == m_aToolbarURL )
        impl_setPopupMenu();
}

void ToolbarAsMenuController::elementReplaced( const css::ui::ConfigurationEvent& rEvent )
    throw ( css::uno::RuntimeException, std::exception )
{
    if ( rEvent.ResourceURL == m_aToolbarURL )
        impl_setPopupMenu();
}

OUString ToolbarAsMenuController::getImplementationName()
    throw ( css::uno::RuntimeException, std::exception )
{
    return OUString( "com.sun.star.comp.framework.ToolbarAsMenuController" );
}

css::uno::Sequence< OUString > ToolbarAsMenuController::getSupportedServiceNames()
    throw ( css::uno::RuntimeException, std::exception )
{
    css::uno::Sequence< OUString > aRet { "com.sun.star.frame.PopupMenuController" };
    return aRet;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_ToolbarAsMenuController_get_implementation(
    css::uno::XComponentContext* xContext,
    css::uno::Sequence< css::uno::Any > const & args )
{
    return cppu::acquire( new framework::ToolbarAsMenuController( xContext, args ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
