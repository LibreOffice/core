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


#include <svtools/contextmenuhelper.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/miscopt.hxx>

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/ModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/frame/UICommandDescription.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <osl/conditn.hxx>
#include <cppuhelper/weak.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/image.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxmenu.hxx>

using namespace ::com::sun::star;

namespace svt
{

// internal helper class to retrieve status updates
class StateEventHelper : public ::com::sun::star::frame::XStatusListener,
                         public ::cppu::OWeakObject
{
    public:
        StateEventHelper( const uno::Reference< frame::XDispatchProvider >& xDispatchProvider,
                          const uno::Reference< util::XURLTransformer >& xURLTransformer,
                          const OUString& aCommandURL );
        virtual ~StateEventHelper();

        bool isCommandEnabled();

        // XInterface
        virtual uno::Any SAL_CALL queryInterface( const uno::Type& aType ) throw ( uno::RuntimeException);
        virtual void SAL_CALL acquire() throw ();
        virtual void SAL_CALL release() throw ();

        // XEventListener
        virtual void SAL_CALL disposing(const lang::EventObject& Source) throw( uno::RuntimeException );

        // XStatusListener
        virtual void SAL_CALL statusChanged(const frame::FeatureStateEvent& Event) throw( uno::RuntimeException );

    private:
        StateEventHelper();
        StateEventHelper( const StateEventHelper& );
        StateEventHelper& operator=( const StateEventHelper& );

        bool                                       m_bCurrentCommandEnabled;
        OUString                            m_aCommandURL;
        uno::Reference< frame::XDispatchProvider > m_xDispatchProvider;
        uno::Reference< util::XURLTransformer >    m_xURLTransformer;
        osl::Condition                             m_aCondition;
};

StateEventHelper::StateEventHelper(
    const uno::Reference< frame::XDispatchProvider >& xDispatchProvider,
    const uno::Reference< util::XURLTransformer >& xURLTransformer,
    const OUString& rCommandURL ) :
    m_bCurrentCommandEnabled( true ),
    m_aCommandURL( rCommandURL ),
    m_xDispatchProvider( xDispatchProvider ),
    m_xURLTransformer( xURLTransformer )
{
    m_aCondition.reset();
}

StateEventHelper::~StateEventHelper()
{}

uno::Any SAL_CALL StateEventHelper::queryInterface(
    const uno::Type& aType )
throw ( uno::RuntimeException )
{
    uno::Any a = ::cppu::queryInterface(
                aType,
                (static_cast< XStatusListener* >(this)));

    if( a.hasValue() )
        return a;

    return ::cppu::OWeakObject::queryInterface( aType );
}

void SAL_CALL StateEventHelper::acquire()
throw ()
{
    ::cppu::OWeakObject::acquire();
}

void SAL_CALL StateEventHelper::release()
throw ()
{
    ::cppu::OWeakObject::release();
}

void SAL_CALL StateEventHelper::disposing(
    const lang::EventObject& )
throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    m_xDispatchProvider.clear();
    m_xURLTransformer.clear();
    m_aCondition.set();
}

void SAL_CALL StateEventHelper::statusChanged(
    const frame::FeatureStateEvent& Event )
throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    m_bCurrentCommandEnabled = Event.IsEnabled;
    m_aCondition.set();
}

bool StateEventHelper::isCommandEnabled()
{
    // Be sure that we cannot die during condition wait
    uno::Reference< frame::XStatusListener > xSelf(
        (static_cast< frame::XStatusListener* >(this)));

    uno::Reference< frame::XDispatch > xDispatch;
    util::URL                          aTargetURL;
    {
        SolarMutexGuard aSolarGuard;
        if ( m_xDispatchProvider.is() && m_xURLTransformer.is() )
        {
            OUString aSelf( "_self" );

            aTargetURL.Complete = m_aCommandURL;
            m_xURLTransformer->parseStrict( aTargetURL );

            try
            {
                xDispatch = m_xDispatchProvider->queryDispatch( aTargetURL, aSelf, 0 );
            }
            catch ( uno::RuntimeException& )
            {
                throw;
            }
            catch ( uno::Exception& )
            {
            }
        }
    }

    bool bResult( false );
    if ( xDispatch.is() )
    {
        try
        {
            // add/remove ourself to retrieve status by callback
            xDispatch->addStatusListener( xSelf, aTargetURL );
            xDispatch->removeStatusListener( xSelf, aTargetURL );

            // wait for anwser
            m_aCondition.wait();
        }
        catch ( uno::RuntimeException& )
        {
            throw;
        }
        catch ( uno::Exception& )
        {
        }

        SolarMutexGuard aSolarGuard;
        bResult = m_bCurrentCommandEnabled;
    }

    return bResult;
}

/*************************************************************************/

struct ExecuteInfo
{
    uno::Reference< frame::XDispatch >    xDispatch;
    util::URL                             aTargetURL;
    uno::Sequence< beans::PropertyValue > aArgs;
};

static const PopupMenu* lcl_FindPopupFromItemId( const PopupMenu* pPopupMenu, sal_uInt16 nItemId )
{
    if ( pPopupMenu )
    {
        sal_uInt16 nCount = pPopupMenu->GetItemCount();
        for ( sal_uInt16 i = 0; i < nCount; i++ )
        {
            sal_uInt16 nId = pPopupMenu->GetItemId( i );
            if ( nId == nItemId )
                return pPopupMenu;
            else
            {
                const PopupMenu* pResult( 0 );

                const PopupMenu* pSubPopup = pPopupMenu->GetPopupMenu( i );
                if ( pPopupMenu )
                    pResult = lcl_FindPopupFromItemId( pSubPopup, nItemId );
                if ( pResult != 0 )
                    return pResult;
            }
        }
    }

    return NULL;
}

static OUString lcl_GetItemCommandRecursive( const PopupMenu* pPopupMenu, sal_uInt16 nItemId )
{
    const PopupMenu* pPopup = lcl_FindPopupFromItemId( pPopupMenu, nItemId );
    if ( pPopup )
        return pPopup->GetItemCommand( nItemId );
    else
        return OUString();
}

/*************************************************************************/

ContextMenuHelper::ContextMenuHelper(
    const uno::Reference< frame::XFrame >& xFrame,
    bool bAutoRefresh ) :
    m_xWeakFrame( xFrame ),
    m_aSelf( "_self" ),
    m_bAutoRefresh( bAutoRefresh ),
    m_bUICfgMgrAssociated( false )
{
}

ContextMenuHelper::~ContextMenuHelper()
{
}

void
ContextMenuHelper::completeAndExecute(
    const Point& aPos,
    PopupMenu& rPopupMenu )
{
    SolarMutexGuard aSolarGuard;

    associateUIConfigurationManagers();
    completeMenuProperties( &rPopupMenu );
    executePopupMenu( aPos, &rPopupMenu );
    resetAssociations();
}

void
ContextMenuHelper::completeAndExecute(
    const Point& aPos,
    const uno::Reference< awt::XPopupMenu >& xPopupMenu )
{
    SolarMutexGuard aSolarGuard;

    VCLXMenu* pXMenu = VCLXMenu::GetImplementation( xPopupMenu );
    if ( pXMenu )
    {
        PopupMenu* pPopupMenu = dynamic_cast< PopupMenu* >( pXMenu->GetMenu() );
        // as dynamic_cast can return zero check pointer
        if ( pPopupMenu )
        {
            associateUIConfigurationManagers();
            completeMenuProperties( pPopupMenu );
            executePopupMenu( aPos, pPopupMenu );
            resetAssociations();
        }
    }
}

// private member

void
ContextMenuHelper::executePopupMenu(
    const Point& rPos,
    PopupMenu* pMenu )
{
    if ( pMenu )
    {
        uno::Reference< frame::XFrame > xFrame( m_xWeakFrame );
        if ( xFrame.is() )
        {
            uno::Reference< awt::XWindow > xWindow( xFrame->getContainerWindow() );
            if ( xWindow.is() )
            {
                Window* pParent = VCLUnoHelper::GetWindow( xWindow );
                sal_uInt16 nResult = pMenu->Execute( pParent, rPos );

                if ( nResult > 0 )
                {
                    OUString aCommand = lcl_GetItemCommandRecursive( pMenu, nResult );
                    if ( !aCommand.isEmpty() )
                        dispatchCommand( xFrame, aCommand );
                }
            }
        }
    }
}

bool
ContextMenuHelper::dispatchCommand(
    const uno::Reference< ::frame::XFrame >& rFrame,
    const OUString& aCommandURL )
{
    if ( !m_xURLTransformer.is() )
    {
        m_xURLTransformer = util::URLTransformer::create( ::comphelper::getProcessComponentContext() );
    }

    util::URL aTargetURL;
    aTargetURL.Complete = aCommandURL;
    m_xURLTransformer->parseStrict( aTargetURL );

    uno::Reference< frame::XDispatch > xDispatch;
    uno::Reference< frame::XDispatchProvider > xDispatchProvider(
        rFrame, uno::UNO_QUERY );
    if ( xDispatchProvider.is() )
    {
        try
        {
            xDispatch = xDispatchProvider->queryDispatch( aTargetURL, m_aSelf, 0 );
        }
        catch ( uno::RuntimeException& )
        {
            throw;
        }
        catch ( uno::Exception& )
        {
        }
    }

    if ( xDispatch.is() )
    {
        ExecuteInfo* pExecuteInfo = new ExecuteInfo;
        pExecuteInfo->xDispatch    = xDispatch;
        pExecuteInfo->aTargetURL   = aTargetURL;
        pExecuteInfo->aArgs        = m_aDefaultArgs;

        Application::PostUserEvent( STATIC_LINK(0, ContextMenuHelper , ExecuteHdl_Impl), pExecuteInfo );
        return true;
    }

    return false;
}

// retrieves and stores references to our user-interface
// configuration managers, like image manager, ui command
// description manager.
bool
ContextMenuHelper::associateUIConfigurationManagers()
{
    uno::Reference< frame::XFrame > xFrame( m_xWeakFrame );
    if ( !m_bUICfgMgrAssociated && xFrame.is() )
    {
        // clear current state
        m_xDocImageMgr.clear();
        m_xModuleImageMgr.clear();
        m_xUICommandLabels.clear();

        try
        {
            uno::Reference < frame::XController > xController;
            uno::Reference < frame::XModel > xModel;
            xController = xFrame->getController();
            if ( xController.is() )
                xModel = xController->getModel();

            if ( xModel.is() )
            {
                // retrieve document image manager form model
                uno::Reference< ui::XUIConfigurationManagerSupplier > xSupplier( xModel, uno::UNO_QUERY );
                if ( xSupplier.is() )
                {
                    uno::Reference< ui::XUIConfigurationManager > xDocUICfgMgr(
                        xSupplier->getUIConfigurationManager(), uno::UNO_QUERY );
                    m_xDocImageMgr = uno::Reference< ui::XImageManager >(
                        xDocUICfgMgr->getImageManager(), uno::UNO_QUERY );
                }
            }

            uno::Reference< frame::XModuleManager2 > xModuleManager(
                frame::ModuleManager::create( ::comphelper::getProcessComponentContext() ) );

            uno::Reference< ui::XImageManager > xModuleImageManager;
            OUString                       aModuleId;
            // retrieve module image manager
            aModuleId = xModuleManager->identify( xFrame );

            uno::Reference< ui::XModuleUIConfigurationManagerSupplier > xModuleCfgMgrSupplier(
                ui::ModuleUIConfigurationManagerSupplier::create(
                    ::comphelper::getProcessComponentContext() ) );
            uno::Reference< ui::XUIConfigurationManager > xUICfgMgr(
                xModuleCfgMgrSupplier->getUIConfigurationManager( aModuleId ));
            if ( xUICfgMgr.is() )
            {
                m_xModuleImageMgr = uno::Reference< ui::XImageManager >(
                    xUICfgMgr->getImageManager(), uno::UNO_QUERY );
            }

            uno::Reference< container::XNameAccess > xNameAccess(
                frame::UICommandDescription::create(
                        ::comphelper::getProcessComponentContext()),
                    uno::UNO_QUERY_THROW );
            try
            {
                uno::Any a = xNameAccess->getByName( aModuleId );
                a >>= m_xUICommandLabels;
            }
            catch ( container::NoSuchElementException& )
            {
            }
        }
        catch ( uno::RuntimeException& )
        {
            throw;
        }
        catch ( uno::Exception& )
        {
            m_bUICfgMgrAssociated = true;
            return false;
        }
        m_bUICfgMgrAssociated = true;
    }

    return true;
}

Image
ContextMenuHelper::getImageFromCommandURL( const OUString& aCmdURL ) const
{
    Image     aImage;
    sal_Int16 nImageType( ui::ImageType::COLOR_NORMAL|
                          ui::ImageType::SIZE_DEFAULT );

    uno::Sequence< uno::Reference< graphic::XGraphic > > aGraphicSeq;
    uno::Sequence< OUString > aImageCmdSeq( 1 );
    aImageCmdSeq[0] = aCmdURL;

    if ( m_xDocImageMgr.is() )
    {
        try
        {
            aGraphicSeq = m_xDocImageMgr->getImages( nImageType, aImageCmdSeq );
            uno::Reference< graphic::XGraphic > xGraphic = aGraphicSeq[0];
            aImage = Image( xGraphic );

            if ( !!aImage )
                return aImage;
        }
        catch ( uno::RuntimeException& )
        {
            throw;
        }
        catch ( uno::Exception& )
        {
        }
    }

    if ( m_xModuleImageMgr.is() )
    {
        try
        {
            aGraphicSeq = m_xModuleImageMgr->getImages( nImageType, aImageCmdSeq );
            uno::Reference< ::com::sun::star::graphic::XGraphic > xGraphic = aGraphicSeq[0];
            aImage = Image( xGraphic );

            if ( !!aImage )
                return aImage;
        }
        catch ( uno::RuntimeException& )
        {
            throw;
        }
        catch ( uno::Exception& )
        {
        }
    }

    return aImage;
}

OUString
ContextMenuHelper::getLabelFromCommandURL(
    const OUString& aCmdURL ) const
{
    OUString aLabel;

    if ( m_xUICommandLabels.is() )
    {
        try
        {
            if ( !aCmdURL.isEmpty() )
            {
                OUString aStr;
                uno::Sequence< beans::PropertyValue > aPropSeq;
                uno::Any a( m_xUICommandLabels->getByName( aCmdURL ));
                if ( a >>= aPropSeq )
                {
                    for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                    {
                        if ( aPropSeq[i].Name == "Label" )
                        {
                            aPropSeq[i].Value >>= aStr;
                            break;
                        }
                    }
                }
                aLabel = aStr;
            }
        }
        catch ( uno::RuntimeException& )
        {
        }
        catch ( uno::Exception& )
        {
        }
    }

    return aLabel;
}

void
ContextMenuHelper::completeMenuProperties(
    Menu* pMenu )
{
    // Retrieve some settings necessary to display complete context
    // menu correctly.
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    bool  bShowMenuImages( rSettings.GetUseImagesInMenus() );

    if ( pMenu )
    {
        uno::Reference< frame::XFrame > xFrame( m_xWeakFrame );
        uno::Reference< frame::XDispatchProvider > xDispatchProvider( xFrame, uno::UNO_QUERY );

        if ( !m_xURLTransformer.is() )
        {
            m_xURLTransformer = util::URLTransformer::create( ::comphelper::getProcessComponentContext() );
        }

        for ( sal_uInt16 nPos = 0; nPos < pMenu->GetItemCount(); nPos++ )
        {
            sal_uInt16 nId        = pMenu->GetItemId( nPos );
            PopupMenu* pPopupMenu = pMenu->GetPopupMenu( nId );
            if ( pPopupMenu )
                completeMenuProperties( pPopupMenu );
            if ( pMenu->GetItemType( nPos ) != MENUITEM_SEPARATOR )
            {
                OUString aCmdURL( pMenu->GetItemCommand( nId ));

                if ( bShowMenuImages )
                {
                    Image aImage;
                    if ( !aCmdURL.isEmpty() )
                        aImage = getImageFromCommandURL( aCmdURL );
                    pMenu->SetItemImage( nId, aImage );
                }
                else
                    pMenu->SetItemImage( nId, Image() );

                if (!pMenu->GetItemText(nId).isEmpty())
                {
                    OUString aLabel( getLabelFromCommandURL( aCmdURL ));
                    pMenu->SetItemText( nId, aLabel );
                }

                // Use helper to retrieve state of the command URL
                StateEventHelper* pHelper = new StateEventHelper(
                                                    xDispatchProvider,
                                                    m_xURLTransformer,
                                                    aCmdURL );

                uno::Reference< frame::XStatusListener > xHelper( pHelper );
                pMenu->EnableItem( nId, pHelper->isCommandEnabled() );
            }
        }
    }
}


IMPL_STATIC_LINK_NOINSTANCE( ContextMenuHelper, ExecuteHdl_Impl, ExecuteInfo*, pExecuteInfo )
{
    // Release solar mutex to prevent deadlocks with clipboard thread
    const sal_uInt32 nRef = Application::ReleaseSolarMutex();
    try
    {
        // Asynchronous execution as this can lead to our own destruction while we are
        // on the stack. Stack unwinding would access the destroyed context menu.
        pExecuteInfo->xDispatch->dispatch( pExecuteInfo->aTargetURL, pExecuteInfo->aArgs );
    }
    catch ( uno::Exception& )
    {
    }

    // Acquire solar mutex again
    Application::AcquireSolarMutex( nRef );
    delete pExecuteInfo;
    return 0;
}

} // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
