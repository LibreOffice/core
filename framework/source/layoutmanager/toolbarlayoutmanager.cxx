/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <toolbarlayoutmanager.hxx>
#include <helpers.hxx>
#include <services.h>
#include <classes/resource.hrc>
#include <classes/fwkresid.hxx>
#include <uiconfiguration/windowstateconfiguration.hxx>

#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/ui/XUIElementSettings.hpp>
#include <com/sun/star/ui/XUIFunctionListener.hpp>

#include <unotools/cmdoptions.hxx>
#include <toolkit/unohlp.hxx>
#include <toolkit/helper/convert.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/dockingarea.hxx>
#include <boost/bind.hpp>

using namespace ::com::sun::star;

namespace framework
{

ToolbarLayoutManager::ToolbarLayoutManager(
    const uno::Reference< lang::XMultiServiceFactory >& xSMGR,
    const uno::Reference< ui::XUIElementFactory >& xUIElementFactory,
    ILayoutNotifications* pParentLayouter )
    : ThreadHelpBase( &Application::GetSolarMutex() ),
    m_xSMGR( xSMGR ),
    m_xUIElementFactoryManager( xUIElementFactory ),
    m_pParentLayouter( pParentLayouter ),
    m_eDockOperation( DOCKOP_ON_COLROW ),
    m_ePreviewDetection( PREVIEWFRAME_UNKNOWN ),
    m_bComponentAttached( false ),
    m_bLayoutDirty( false ),
    m_bStoreWindowState( false ),
    m_bGlobalSettings( false ),
    m_bDockingInProgress( false ),
    m_bVisible( true ),
    m_bLayoutInProgress( false ),
    m_bToolbarCreation( false ),
    m_aFullAddonTbxPrefix( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/addon_" )),
    m_aCustomTbxPrefix( RTL_CONSTASCII_USTRINGPARAM( "custom_" )),
    m_aCustomizeCmd( RTL_CONSTASCII_USTRINGPARAM( "ConfigureDialog" )),
    m_aToolbarTypeString( RTL_CONSTASCII_USTRINGPARAM( UIRESOURCETYPE_TOOLBAR ))
{
    // initialize rectangles to zero values
    setZeroRectangle( m_aDockingAreaOffsets );
    setZeroRectangle( m_aDockingArea );

    // create toolkit object
    m_xToolkit = uno::Reference< awt::XToolkit >( m_xSMGR->createInstance( SERVICENAME_VCLTOOLKIT ), uno::UNO_QUERY );
}

ToolbarLayoutManager::~ToolbarLayoutManager()
{
}

//---------------------------------------------------------------------------------------------------------
//  XInterface
//---------------------------------------------------------------------------------------------------------
void SAL_CALL ToolbarLayoutManager::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL ToolbarLayoutManager::release() throw()
{
    OWeakObject::release();
}

uno::Any SAL_CALL ToolbarLayoutManager::queryInterface( const uno::Type & rType ) throw( uno::RuntimeException )
{
    uno::Any a = ::cppu::queryInterface( rType,
        (static_cast< awt::XDockableWindowListener* >(this)),
        (static_cast< ui::XUIConfigurationListener* >(this)),
        (static_cast< awt::XWindowListener* >(this)));

    if ( a.hasValue() )
        return a;

    return OWeakObject::queryInterface( rType );
}

void SAL_CALL ToolbarLayoutManager::disposing( const lang::EventObject& aEvent ) throw( uno::RuntimeException )
{
    if ( aEvent.Source == m_xFrame )
    {
        // Reset all internal references
        reset();
        implts_destroyDockingAreaWindows();
    }
}

awt::Rectangle ToolbarLayoutManager::getDockingArea()
{
    WriteGuard aWriteLock( m_aLock );
    Rectangle aNewDockingArea( m_aDockingArea );
    aWriteLock.unlock();

    if ( isLayoutDirty() )
        aNewDockingArea = implts_calcDockingArea();

    aWriteLock.lock();
    m_aDockingArea = aNewDockingArea;
    aWriteLock.unlock();

    return putRectangleValueToAWT(aNewDockingArea);
}

void ToolbarLayoutManager::setDockingArea( const awt::Rectangle& rDockingArea )
{
    WriteGuard aWriteLock( m_aLock );
    m_aDockingArea = putAWTToRectangle( rDockingArea );
    m_bLayoutDirty = true;
    aWriteLock.unlock();
}

void ToolbarLayoutManager::implts_setDockingAreaWindowSizes( const awt::Rectangle& rBorderSpace )
{
    ReadGuard aReadLock( m_aLock );
    Rectangle aDockOffsets = m_aDockingAreaOffsets;
    uno::Reference< awt::XWindow2 > xContainerWindow( m_xContainerWindow );
    uno::Reference< awt::XWindow > xTopDockAreaWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_TOP] );
    uno::Reference< awt::XWindow > xBottomDockAreaWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_BOTTOM] );
    uno::Reference< awt::XWindow > xLeftDockAreaWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_LEFT] );
    uno::Reference< awt::XWindow > xRightDockAreaWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_RIGHT] );
    aReadLock.unlock();

    uno::Reference< awt::XDevice > xDevice( xContainerWindow, uno::UNO_QUERY );

    // Convert relativ size to output size.
    awt::Rectangle  aRectangle           = xContainerWindow->getPosSize();
    awt::DeviceInfo aInfo                = xDevice->getInfo();
    awt::Size       aContainerClientSize = awt::Size( aRectangle.Width - aInfo.LeftInset - aInfo.RightInset  ,
                                                      aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );
    long aStatusBarHeight                = aDockOffsets.GetHeight();

    sal_Int32 nLeftRightDockingAreaHeight( aContainerClientSize.Height );
    if ( rBorderSpace.Y >= 0 )
    {
        // Top docking area window
        xTopDockAreaWindow->setPosSize( 0, 0, aContainerClientSize.Width, rBorderSpace.Y, awt::PosSize::POSSIZE );
        xTopDockAreaWindow->setVisible( sal_True );
        nLeftRightDockingAreaHeight -= rBorderSpace.Y;
    }

    if ( rBorderSpace.Height >= 0 )
    {
        // Bottom docking area window
        sal_Int32 nBottomPos = std::max( sal_Int32( aContainerClientSize.Height - rBorderSpace.Height - aStatusBarHeight + 1 ), sal_Int32( 0 ));
        sal_Int32 nHeight = ( nBottomPos == 0 ) ? 0 : rBorderSpace.Height;

        xBottomDockAreaWindow->setPosSize( 0, nBottomPos, aContainerClientSize.Width, nHeight, awt::PosSize::POSSIZE );
        xBottomDockAreaWindow->setVisible( sal_True );
        nLeftRightDockingAreaHeight -= nHeight - 1;
    }

    nLeftRightDockingAreaHeight -= aStatusBarHeight;
    if ( rBorderSpace.X >= 0 || nLeftRightDockingAreaHeight > 0 )
    {
        // Left docking area window
        // We also have to change our right docking area window if the top or bottom area has changed. They have a higher priority!
        sal_Int32 nHeight = std::max( sal_Int32( 0 ), sal_Int32( nLeftRightDockingAreaHeight ));

        xLeftDockAreaWindow->setPosSize( 0, rBorderSpace.Y, rBorderSpace.X, nHeight, awt::PosSize::POSSIZE );
        xLeftDockAreaWindow->setVisible( sal_True );
    }
    if ( rBorderSpace.Width >= 0 || nLeftRightDockingAreaHeight > 0 )
    {
        // Right docking area window
        // We also have to change our right docking area window if the top or bottom area has changed. They have a higher priority!
        sal_Int32 nLeftPos = std::max( sal_Int32( 0 ), sal_Int32( aContainerClientSize.Width - rBorderSpace.Width ));
        sal_Int32 nHeight  = std::max( sal_Int32( 0 ), sal_Int32( nLeftRightDockingAreaHeight ));
        sal_Int32 nWidth   = ( nLeftPos == 0 ) ? 0 : rBorderSpace.Width;

        xRightDockAreaWindow->setPosSize( nLeftPos, rBorderSpace.Y, nWidth, nHeight, awt::PosSize::POSSIZE );
        xRightDockAreaWindow->setVisible( sal_True );
    }
}

bool ToolbarLayoutManager::isLayoutDirty()
{
    return m_bLayoutDirty;
}

void ToolbarLayoutManager::doLayout(const ::Size& aContainerSize)
{
    WriteGuard aWriteLock( m_aLock );
    bool bLayoutInProgress( m_bLayoutInProgress );
    m_bLayoutInProgress = true;
    awt::Rectangle aDockingArea = putRectangleValueToAWT( m_aDockingArea );
    aWriteLock.unlock();

    if ( bLayoutInProgress )
        return;

    // Retrieve row/column dependent data from all docked user-interface elements
    for ( sal_Int32 i = 0; i < DOCKINGAREAS_COUNT; i++ )
    {
        bool bReverse( isReverseOrderDockingArea( i ));
        std::vector< SingleRowColumnWindowData > aRowColumnsWindowData;

        implts_getDockingAreaElementInfos( (ui::DockingArea)i, aRowColumnsWindowData );

        sal_Int32 nOffset( 0 );
        const sal_uInt32 nCount = aRowColumnsWindowData.size();
        for ( sal_uInt32 j = 0; j < nCount; ++j )
        {
            sal_uInt32 nIndex = bReverse ? nCount-j-1 : j;
            implts_calcWindowPosSizeOnSingleRowColumn( i, nOffset, aRowColumnsWindowData[nIndex], aContainerSize );
            nOffset += aRowColumnsWindowData[j].nStaticSize;
        }
    }

    implts_setDockingAreaWindowSizes( aDockingArea );

    aWriteLock.lock();
    m_bLayoutDirty      = false;
    m_bLayoutInProgress = false;
    aWriteLock.unlock();
}

bool ToolbarLayoutManager::implts_isParentWindowVisible() const
{
    ReadGuard aReadLock( m_aLock );
    bool bVisible( false );
    if ( m_xContainerWindow.is() )
        bVisible = m_xContainerWindow->isVisible();

    return bVisible;
}

Rectangle ToolbarLayoutManager::implts_calcDockingArea()
{
    ReadGuard aReadLock( m_aLock );
    UIElementVector aWindowVector( m_aUIElements );
    aReadLock.unlock();

    Rectangle                aBorderSpace;
    sal_Int32                nCurrRowColumn( 0 );
    sal_Int32                nCurrPos( 0 );
    sal_Int32                nCurrDockingArea( ui::DockingArea_DOCKINGAREA_TOP );
    std::vector< sal_Int32 > aRowColumnSizes[DOCKINGAREAS_COUNT];
    UIElementVector::const_iterator pConstIter;

    // initialize rectangle with zero values!
    aBorderSpace.setWidth(0);
    aBorderSpace.setHeight(0);

    aRowColumnSizes[nCurrDockingArea].clear();
    aRowColumnSizes[nCurrDockingArea].push_back( 0 );

    for ( pConstIter = aWindowVector.begin(); pConstIter != aWindowVector.end(); ++pConstIter )
    {
        uno::Reference< ui::XUIElement > xUIElement( pConstIter->m_xUIElement, uno::UNO_QUERY );
        if ( xUIElement.is() )
        {
            uno::Reference< awt::XWindow > xWindow( xUIElement->getRealInterface(), uno::UNO_QUERY );
            uno::Reference< awt::XDockableWindow > xDockWindow( xWindow, uno::UNO_QUERY );
            if ( xWindow.is() && xDockWindow.is() )
            {
                SolarMutexGuard aGuard;

                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow && !xDockWindow->isFloating() && pConstIter->m_bVisible )
                {
                    awt::Rectangle aPosSize = xWindow->getPosSize();
                    if ( pConstIter->m_aDockedData.m_nDockedArea != nCurrDockingArea )
                    {
                        nCurrDockingArea = pConstIter->m_aDockedData.m_nDockedArea;
                        nCurrRowColumn   = 0;
                        nCurrPos         = 0;
                        aRowColumnSizes[nCurrDockingArea].clear();
                        aRowColumnSizes[nCurrDockingArea].push_back( 0 );
                    }

                    if ( pConstIter->m_aDockedData.m_nDockedArea == nCurrDockingArea )
                    {
                        if ( isHorizontalDockingArea( pConstIter->m_aDockedData.m_nDockedArea ))
                        {
                            if ( pConstIter->m_aDockedData.m_aPos.Y > nCurrPos )
                            {
                                ++nCurrRowColumn;
                                nCurrPos = pConstIter->m_aDockedData.m_aPos.Y;
                                aRowColumnSizes[nCurrDockingArea].push_back( 0 );
                            }

                            if ( aPosSize.Height > aRowColumnSizes[nCurrDockingArea][nCurrRowColumn] )
                                aRowColumnSizes[nCurrDockingArea][nCurrRowColumn] = aPosSize.Height;
                        }
                        else
                        {
                            if ( pConstIter->m_aDockedData.m_aPos.X > nCurrPos )
                            {
                                ++nCurrRowColumn;
                                nCurrPos = pConstIter->m_aDockedData.m_aPos.X;
                                aRowColumnSizes[nCurrDockingArea].push_back( 0 );
                            }

                            if ( aPosSize.Width > aRowColumnSizes[nCurrDockingArea][nCurrRowColumn] )
                                aRowColumnSizes[nCurrDockingArea][nCurrRowColumn] = aPosSize.Width;
                        }
                    }
                }
            }
        }
    }

    // Sum up max heights from every row/column
    if ( !aWindowVector.empty() )
    {
        for ( sal_Int32 i = 0; i <= ui::DockingArea_DOCKINGAREA_RIGHT; i++ )
        {
            sal_Int32 nSize( 0 );
            const sal_uInt32 nCount = aRowColumnSizes[i].size();
            for ( sal_uInt32 j = 0; j < nCount; j++ )
                nSize += aRowColumnSizes[i][j];

            if ( i == ui::DockingArea_DOCKINGAREA_TOP )
                aBorderSpace.Top() = nSize;
            else if ( i == ui::DockingArea_DOCKINGAREA_BOTTOM )
                aBorderSpace.Bottom() = nSize;
            else if ( i == ui::DockingArea_DOCKINGAREA_LEFT )
                aBorderSpace.Left() = nSize;
            else
                aBorderSpace.Right() = nSize;
        }
    }

    return aBorderSpace;
}

void ToolbarLayoutManager::reset()
{
    WriteGuard aWriteLock( m_aLock );
    uno::Reference< ui::XUIConfigurationManager > xModuleCfgMgr( m_xModuleCfgMgr );
    uno::Reference< ui::XUIConfigurationManager > xDocCfgMgr( m_xDocCfgMgr );
    m_xModuleCfgMgr.clear();
    m_xDocCfgMgr.clear();
    m_ePreviewDetection = PREVIEWFRAME_UNKNOWN;
    m_bComponentAttached = false;
    aWriteLock.unlock();

    destroyToolbars();
    resetDockingArea();
}

void ToolbarLayoutManager::attach(
    const uno::Reference< frame::XFrame >& xFrame,
    const uno::Reference< ui::XUIConfigurationManager >& xModuleCfgMgr,
    const uno::Reference< ui::XUIConfigurationManager >& xDocCfgMgr,
    const uno::Reference< container::XNameAccess >& xPersistentWindowState )
{
    // reset toolbar manager if we lose our current frame
    if ( m_xFrame.is() && m_xFrame != xFrame )
        reset();

    WriteGuard aWriteLock( m_aLock );
    m_xFrame                 = xFrame;
    m_xModuleCfgMgr          = xModuleCfgMgr;
    m_xDocCfgMgr             = xDocCfgMgr;
    m_xPersistentWindowState = xPersistentWindowState;
    m_bComponentAttached     = true;
}

bool ToolbarLayoutManager::isPreviewFrame()
{
    ReadGuard aReadLock( m_aLock );
    if (m_ePreviewDetection == PREVIEWFRAME_UNKNOWN)
    {
        uno::Reference< frame::XFrame > xFrame( m_xFrame );

        uno::Reference< frame::XModel > xModel( impl_getModelFromFrame( xFrame ));

        WriteGuard aWriteLock( m_aLock );
        m_ePreviewDetection = (implts_isPreviewModel( xModel ) ? PREVIEWFRAME_YES : PREVIEWFRAME_NO);
    }
    return m_ePreviewDetection == PREVIEWFRAME_YES;
}

void ToolbarLayoutManager::createStaticToolbars()
{
    resetDockingArea();
    implts_createCustomToolBars();
    implts_createAddonsToolBars();
    implts_createNonContextSensitiveToolBars();
    implts_sortUIElements();
}

bool ToolbarLayoutManager::requestToolbar( const ::rtl::OUString& rResourceURL )
{
    if (isPreviewFrame())
        return false; // no toolbars for preview frame!

    bool bNotify( false );
    bool bMustCallCreate( false );
    uno::Reference< ui::XUIElement > xUIElement;

    UIElement aRequestedToolbar = impl_findToolbar( rResourceURL );
    if ( aRequestedToolbar.m_aName != rResourceURL  )
    {
        bMustCallCreate = true;
        aRequestedToolbar.m_aName      = rResourceURL;
        aRequestedToolbar.m_aType      = m_aToolbarTypeString;
        aRequestedToolbar.m_xUIElement = xUIElement;
        implts_readWindowStateData( rResourceURL, aRequestedToolbar );
    }

    xUIElement = aRequestedToolbar.m_xUIElement;
    if ( !xUIElement.is() )
        bMustCallCreate = true;

    bool bCreateOrShowToolbar( aRequestedToolbar.m_bVisible & !aRequestedToolbar.m_bMasterHide );
    uno::Reference< awt::XWindow2 > xContainerWindow( m_xContainerWindow, uno::UNO_QUERY );
    if ( xContainerWindow.is() && aRequestedToolbar.m_bFloating )
        bCreateOrShowToolbar &= bool( xContainerWindow->isActive());

    if ( bCreateOrShowToolbar )
        bNotify = ( bMustCallCreate ) ? createToolbar( rResourceURL ) : showToolbar( rResourceURL );

    return bNotify;
}

bool ToolbarLayoutManager::createToolbar( const ::rtl::OUString& rResourceURL )
{
    bool bNotify( false );
    uno::Reference< ui::XUIElement > xUITempElement;

    implts_createToolBar( rResourceURL, bNotify, xUITempElement );
    return bNotify;
}

bool ToolbarLayoutManager::destroyToolbar( const ::rtl::OUString& rResourceURL )
{
    const rtl::OUString aAddonTbResourceName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/addon_" ));

    UIElementVector::iterator pIter;
    uno::Reference< lang::XComponent > xComponent;

    bool bNotify( false );
    bool bMustBeSorted( false );
    bool bMustLayouted( false );
    bool bMustBeDestroyed( rResourceURL.indexOf( aAddonTbResourceName ) != 0 );

    WriteGuard aWriteLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_aName == rResourceURL )
        {
            xComponent.set( pIter->m_xUIElement, uno::UNO_QUERY );
            if ( bMustBeDestroyed )
                pIter->m_xUIElement.clear();
            else
                pIter->m_bVisible = false;
            break;
        }
    }
    aWriteLock.unlock();

    uno::Reference< ui::XUIElement > xUIElement( xComponent, uno::UNO_QUERY );
    if ( xUIElement.is() )
    {
        uno::Reference< awt::XWindow > xWindow( xUIElement->getRealInterface(), uno::UNO_QUERY );
        uno::Reference< awt::XDockableWindow > xDockWindow( xWindow, uno::UNO_QUERY );

        if ( bMustBeDestroyed )
        {
            try
            {
                if ( xWindow.is() )
                    xWindow->removeWindowListener( uno::Reference< awt::XWindowListener >(
                        static_cast< OWeakObject * >( this ), uno::UNO_QUERY ));
            }
            catch (const uno::Exception&)
            {
            }

            try
            {
                if ( xDockWindow.is() )
                    xDockWindow->removeDockableWindowListener( uno::Reference< awt::XDockableWindowListener >(
                        static_cast< OWeakObject * >( this ), uno::UNO_QUERY ));
            }
            catch (const uno::Exception&)
            {
            }
        }
        else
        {
            if ( xWindow.is() )
                xWindow->setVisible( sal_False );
            bNotify = true;
        }

        if ( !xDockWindow->isFloating() )
            bMustLayouted = true;
        bMustBeSorted = true;
    }

    if ( bMustBeDestroyed )
    {
        if ( xComponent.is() )
            xComponent->dispose();
        bNotify = true;
    }

    if ( bMustLayouted )
         implts_setLayoutDirty();

    if ( bMustBeSorted )
        implts_sortUIElements();

    return bNotify;
}

void ToolbarLayoutManager::destroyToolbars()
{
    UIElementVector aUIElementVector;
    implts_getUIElementVectorCopy( aUIElementVector );

    WriteGuard aWriteLock( m_aLock );
    m_aUIElements.clear();
    m_bLayoutDirty = true;
    aWriteLock.unlock();

    UIElementVector::iterator pIter;
    for ( pIter = aUIElementVector.begin(); pIter != aUIElementVector.end(); ++pIter )
    {
        uno::Reference< lang::XComponent > xComponent( pIter->m_xUIElement, uno::UNO_QUERY );
        if ( xComponent.is() )
            xComponent->dispose();
    }
}

bool ToolbarLayoutManager::showToolbar( const ::rtl::OUString& rResourceURL )
{
    UIElement aUIElement = implts_findToolbar( rResourceURL );

    SolarMutexGuard aGuard;
    Window* pWindow = getWindowFromXUIElement( aUIElement.m_xUIElement );
    if ( pWindow )
    {
        if ( !aUIElement.m_bFloating )
            implts_setLayoutDirty();
        else
            pWindow->Show( sal_True, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );

        aUIElement.m_bVisible = true;
        implts_writeWindowStateData( aUIElement );
        implts_setToolbar( aUIElement );
        implts_sortUIElements();
        return true;
    }

    return false;
}

bool ToolbarLayoutManager::hideToolbar( const ::rtl::OUString& rResourceURL )
{
    UIElement aUIElement = implts_findToolbar( rResourceURL );

    SolarMutexGuard aGuard;
    Window* pWindow = getWindowFromXUIElement( aUIElement.m_xUIElement );
    if ( pWindow )
    {
        pWindow->Show( sal_False );
        if ( !aUIElement.m_bFloating )
            implts_setLayoutDirty();

        aUIElement.m_bVisible = false;
        implts_writeWindowStateData( aUIElement );
        implts_setToolbar( aUIElement );
        return true;
    }

    return false;
}

void ToolbarLayoutManager::refreshToolbarsVisibility( bool bAutomaticToolbars )
{
    UIElementVector aUIElementVector;

    ReadGuard aReadLock( m_aLock );
    bool bVisible( m_bVisible );
    aReadLock.unlock();

    if ( !bVisible || !bAutomaticToolbars )
        return;

    implts_getUIElementVectorCopy( aUIElementVector );

    UIElement aUIElement;
    SolarMutexGuard aGuard;
    UIElementVector::iterator pIter;
    for ( pIter = aUIElementVector.begin(); pIter != aUIElementVector.end(); ++pIter )
    {
        if ( implts_readWindowStateData( pIter->m_aName, aUIElement ) &&
             ( pIter->m_bVisible != aUIElement.m_bVisible ) && !pIter->m_bMasterHide )
        {
            WriteGuard aWriteLock( m_aLock );
            UIElement& rUIElement = impl_findToolbar( pIter->m_aName );
            if ( rUIElement.m_aName == pIter->m_aName )
            {
                rUIElement.m_bVisible = aUIElement.m_bVisible;
                implts_setLayoutDirty();
            }
        }
    }
}

void ToolbarLayoutManager::setFloatingToolbarsVisibility( bool bVisible )
{
    UIElementVector aUIElementVector;
    implts_getUIElementVectorCopy( aUIElementVector );

    SolarMutexGuard aGuard;
    UIElementVector::iterator pIter;
    for ( pIter = aUIElementVector.begin(); pIter != aUIElementVector.end(); ++pIter )
    {
        Window* pWindow = getWindowFromXUIElement( pIter->m_xUIElement );
        if ( pWindow && pIter->m_bFloating )
        {
            if ( bVisible )
            {
                if ( pIter->m_bVisible && !pIter->m_bMasterHide )
                    pWindow->Show( sal_True, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
            }
            else
                pWindow->Show( sal_False );
        }
    }
}

void ToolbarLayoutManager::setVisible( bool bVisible )
{
    UIElementVector aUIElementVector;
    implts_getUIElementVectorCopy( aUIElementVector );

    SolarMutexGuard aGuard;
    UIElementVector::iterator pIter;
    for ( pIter = aUIElementVector.begin(); pIter != aUIElementVector.end(); ++pIter )
    {
        pIter->m_bMasterHide = !bVisible;
        Window* pWindow = getWindowFromXUIElement( pIter->m_xUIElement );
        if ( pWindow )
        {
            bool bSetVisible( pIter->m_bVisible & bVisible );
            if ( !bSetVisible )
                pWindow->Hide();
            else
            {
                if ( pIter->m_bFloating )
                    pWindow->Show(true, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
                else
                    implts_setLayoutDirty();
            }
        }
    }

    if ( !bVisible )
        resetDockingArea();
}

bool ToolbarLayoutManager::dockToolbar( const ::rtl::OUString& rResourceURL, ui::DockingArea eDockingArea, const awt::Point& aPos )
{
    UIElement aUIElement = implts_findToolbar( rResourceURL );

    if ( aUIElement.m_xUIElement.is() )
    {
        try
        {
            uno::Reference< awt::XWindow > xWindow( aUIElement.m_xUIElement->getRealInterface(), uno::UNO_QUERY );
            uno::Reference< awt::XDockableWindow > xDockWindow( xWindow, uno::UNO_QUERY );
            if ( xDockWindow.is() )
            {
                if ( eDockingArea != ui::DockingArea_DOCKINGAREA_DEFAULT )
                    aUIElement.m_aDockedData.m_nDockedArea = sal_Int16( eDockingArea );

                if ( !isDefaultPos( aPos ))
                    aUIElement.m_aDockedData.m_aPos = aPos;

                if ( !xDockWindow->isFloating() )
                {
                    Window*  pWindow( 0 );
                    ToolBox* pToolBox( 0 );

                    {
                        SolarMutexGuard aGuard;
                        pWindow = VCLUnoHelper::GetWindow( xWindow );
                        if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
                        {
                            pToolBox = (ToolBox *)pWindow;

                            // We have to set the alignment of the toolbox. It's possible that the toolbox is moved from a
                            // horizontal to a vertical docking area!
                            pToolBox->SetAlign( ImplConvertAlignment( aUIElement.m_aDockedData.m_nDockedArea ));
                        }
                    }

                    if ( hasDefaultPosValue( aUIElement.m_aDockedData.m_aPos ))
                    {
                        // Docking on its default position without a preset position -
                        // we have to find a good place for it.
                        ::Size aSize;

                        SolarMutexGuard aGuard;
                        {
                            if ( pToolBox )
                                aSize = pToolBox->CalcWindowSizePixel( 1, ImplConvertAlignment( aUIElement.m_aDockedData.m_nDockedArea ) );
                            else
                                aSize = pWindow->GetSizePixel();
                        }

                        ::Point aPixelPos;
                        awt::Point aDockPos;
                        implts_findNextDockingPos((ui::DockingArea)aUIElement.m_aDockedData.m_nDockedArea, aSize, aDockPos, aPixelPos );
                        aUIElement.m_aDockedData.m_aPos = aDockPos;
                    }
                }

                implts_setToolbar( aUIElement );

                if ( xDockWindow->isFloating() )
                {
                    // ATTENTION: This will call toggleFloatingMode() via notifications which
                    // sets the floating member of the UIElement correctly!
                    xDockWindow->setFloatingMode( sal_False );
                }
                else
                {
                    implts_writeWindowStateData( aUIElement );
                    implts_sortUIElements();

                    if ( aUIElement.m_bVisible )
                        implts_setLayoutDirty();
                }
                return true;
            }
        }
        catch (const lang::DisposedException&)
        {
        }
    }

    return false;
}

bool ToolbarLayoutManager::dockAllToolbars()
{
    std::vector< ::rtl::OUString > aToolBarNameVector;

    ::rtl::OUString aElementType;
    ::rtl::OUString aElementName;

    ReadGuard aReadLock( m_aLock );
    UIElementVector::iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_aType == "toolbar" && pIter->m_xUIElement.is() && pIter->m_bFloating && pIter->m_bVisible )
            aToolBarNameVector.push_back( pIter->m_aName );
    }
    aReadLock.unlock();

    bool bResult(true);
    const sal_uInt32 nCount = aToolBarNameVector.size();
    for ( sal_uInt32 i = 0; i < nCount; ++i )
    {
        awt::Point aPoint;
        aPoint.X = aPoint.Y = SAL_MAX_INT32;
        bResult &= dockToolbar( aToolBarNameVector[i], ui::DockingArea_DOCKINGAREA_DEFAULT, aPoint );
    }

    return bResult;
}

long ToolbarLayoutManager::childWindowEvent( VclSimpleEvent* pEvent )
{
    // To enable toolbar controllers to change their image when a sub-toolbar function
    // is activated, we need this mechanism. We have NO connection between these toolbars
    // anymore!
    if ( pEvent && pEvent->ISA( VclWindowEvent ))
    {
        if ( pEvent->GetId() == VCLEVENT_TOOLBOX_SELECT )
        {
            ::rtl::OUString aToolbarName;
            ::rtl::OUString aCommand;
            ToolBox*        pToolBox = getToolboxPtr( ((VclWindowEvent*)pEvent)->GetWindow() );

            if ( pToolBox )
            {
                aToolbarName = retrieveToolbarNameFromHelpURL( pToolBox );
                sal_uInt16 nId = pToolBox->GetCurItemId();
                if ( nId > 0 )
                    aCommand = pToolBox->GetItemCommand( nId );
            }

            if ( !aToolbarName.isEmpty() && !aCommand.isEmpty() )
            {
                ReadGuard aReadLock( m_aLock );
                ::std::vector< uno::Reference< ui::XUIFunctionListener > > aListenerArray;
                UIElementVector::iterator pIter;

                for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
                {
                    if ( pIter->m_xUIElement.is() )
                    {
                        uno::Reference< ui::XUIFunctionListener > xListener( pIter->m_xUIElement, uno::UNO_QUERY );
                        if ( xListener.is() )
                            aListenerArray.push_back( xListener );
                    }
                }
                aReadLock.unlock();

                const sal_uInt32 nCount = aListenerArray.size();
                for ( sal_uInt32 i = 0; i < nCount; ++i )
                {
                    try
                    {
                        aListenerArray[i]->functionExecute( aToolbarName, aCommand );
                    }
                    catch (const uno::RuntimeException&)
                    {
                        throw;
                    }
                    catch (const uno::Exception&)
                    {
                    }
                }
            }
        }
        else if ( pEvent->GetId() == VCLEVENT_TOOLBOX_FORMATCHANGED )
        {
            if ( !implts_isToolbarCreationActive() )
            {
                ToolBox* pToolBox = getToolboxPtr( ((VclWindowEvent*)pEvent)->GetWindow() );
                if ( pToolBox )
                {
                    ::rtl::OUString aToolbarName = retrieveToolbarNameFromHelpURL( pToolBox );
                    if ( !aToolbarName.isEmpty() )
                    {
                        ::rtl::OUStringBuffer aBuf(100);
                        aBuf.appendAscii( "private:resource/toolbar/" );
                        aBuf.append( aToolbarName );

                        UIElement aToolbar = implts_findToolbar( aBuf.makeStringAndClear() );
                        if ( aToolbar.m_xUIElement.is() && !aToolbar.m_bFloating )
                        {
                            implts_setLayoutDirty();
                            m_pParentLayouter->requestLayout( ILayoutNotifications::HINT_TOOLBARSPACE_HAS_CHANGED );
                        }
                    }
                }
            }
        }
    }

    return 1;
}

void ToolbarLayoutManager::resetDockingArea()
{
    ReadGuard aReadLock( m_aLock );
    uno::Reference< awt::XWindow > xTopDockingWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_TOP] );
    uno::Reference< awt::XWindow > xLeftDockingWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_LEFT] );
    uno::Reference< awt::XWindow > xRightDockingWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_RIGHT] );
    uno::Reference< awt::XWindow > xBottomDockingWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_BOTTOM] );
    aReadLock.unlock();

    if ( xTopDockingWindow.is() )
        xTopDockingWindow->setPosSize( 0, 0, 0, 0, awt::PosSize::POSSIZE );
    if ( xLeftDockingWindow.is() )
        xLeftDockingWindow->setPosSize( 0, 0, 0, 0, awt::PosSize::POSSIZE );
    if ( xRightDockingWindow.is() )
        xRightDockingWindow->setPosSize( 0, 0, 0, 0, awt::PosSize::POSSIZE );
    if ( xBottomDockingWindow.is() )
        xBottomDockingWindow->setPosSize( 0, 0, 0, 0, awt::PosSize::POSSIZE );
}

void ToolbarLayoutManager::setParentWindow(
    const uno::Reference< awt::XWindowPeer >& xParentWindow )
{
    static const char DOCKINGAREASTRING[] = "dockingarea";

    uno::Reference< awt::XWindow > xTopDockWindow = uno::Reference< awt::XWindow >( createToolkitWindow( m_xSMGR, xParentWindow, DOCKINGAREASTRING ), uno::UNO_QUERY );
    uno::Reference< awt::XWindow > xLeftDockWindow = uno::Reference< awt::XWindow >( createToolkitWindow( m_xSMGR, xParentWindow, DOCKINGAREASTRING ), uno::UNO_QUERY );
    uno::Reference< awt::XWindow > xRightDockWindow = uno::Reference< awt::XWindow >( createToolkitWindow( m_xSMGR, xParentWindow, DOCKINGAREASTRING ), uno::UNO_QUERY );
    uno::Reference< awt::XWindow > xBottomDockWindow = uno::Reference< awt::XWindow >( createToolkitWindow( m_xSMGR, xParentWindow, DOCKINGAREASTRING ), uno::UNO_QUERY );

    WriteGuard aWriteLock( m_aLock );
    m_xContainerWindow = uno::Reference< awt::XWindow2 >( xParentWindow, uno::UNO_QUERY );
    m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_TOP]    = xTopDockWindow;
    m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_LEFT]   = xLeftDockWindow;
    m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_RIGHT]  = xRightDockWindow;
    m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_BOTTOM] = xBottomDockWindow;
    aWriteLock.unlock();

    if ( xParentWindow.is() )
    {
        SolarMutexGuard aGuard;
        ::DockingAreaWindow* pWindow = dynamic_cast< ::DockingAreaWindow* >(VCLUnoHelper::GetWindow( xTopDockWindow ) );
        if( pWindow ) pWindow->SetAlign( WINDOWALIGN_TOP );
        pWindow = dynamic_cast< ::DockingAreaWindow* >(VCLUnoHelper::GetWindow( xBottomDockWindow ) );
            if( pWindow ) pWindow->SetAlign( WINDOWALIGN_BOTTOM );
        pWindow = dynamic_cast< ::DockingAreaWindow* >(VCLUnoHelper::GetWindow( xLeftDockWindow ) );
            if( pWindow ) pWindow->SetAlign( WINDOWALIGN_LEFT );
        pWindow = dynamic_cast< ::DockingAreaWindow* >(VCLUnoHelper::GetWindow( xRightDockWindow ) );
            if( pWindow ) pWindow->SetAlign( WINDOWALIGN_RIGHT );
        implts_reparentToolbars();
    }
    else
    {
        destroyToolbars();
        resetDockingArea();
    }
}

void ToolbarLayoutManager::setDockingAreaOffsets( const ::Rectangle aOffsets )
{
    WriteGuard aWriteLock( m_aLock );
    m_aDockingAreaOffsets = aOffsets;
    m_bLayoutDirty        = true;
}

rtl::OUString ToolbarLayoutManager::implts_generateGenericAddonToolbarTitle( sal_Int32 nNumber ) const
{
    String aAddonGenericTitle;

    aAddonGenericTitle = String( FwkResId( STR_TOOLBAR_TITLE_ADDON ));
    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();

    String aNumStr = rI18nHelper.GetNum( nNumber, 0, sal_False, sal_False );
    aAddonGenericTitle.SearchAndReplaceAscii( "%num%", aNumStr );

    return rtl::OUString( aAddonGenericTitle );
}

void ToolbarLayoutManager::implts_createAddonsToolBars()
{
    WriteGuard aWriteLock( m_aLock );
    if ( m_pAddonOptions.get() == 0 )
        m_pAddonOptions.reset( new AddonsOptions );

    uno::Reference< ui::XUIElementFactory > xUIElementFactory( m_xUIElementFactoryManager );
    uno::Reference< frame::XFrame > xFrame( m_xFrame );
    aWriteLock.unlock();

    if (isPreviewFrame())
        return; // no addon toolbars for preview frame!

    UIElementVector aUIElementVector;
    uno::Sequence< uno::Sequence< beans::PropertyValue > > aAddonToolBarData;
    uno::Reference< ui::XUIElement >                       xUIElement;

    sal_uInt32 nCount = m_pAddonOptions->GetAddonsToolBarCount();
    ::rtl::OUString aAddonsToolBarStaticName( m_aFullAddonTbxPrefix );
    ::rtl::OUString aElementType( RTL_CONSTASCII_USTRINGPARAM( "toolbar" ));

    uno::Sequence< beans::PropertyValue > aPropSeq( 2 );
    aPropSeq[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    aPropSeq[0].Value <<= xFrame;
    aPropSeq[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationData" ));
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        ::rtl::OUString aAddonToolBarName( aAddonsToolBarStaticName + m_pAddonOptions->GetAddonsToolbarResourceName(i) );
        aAddonToolBarData = m_pAddonOptions->GetAddonsToolBarPart( i );
        aPropSeq[1].Value <<= aAddonToolBarData;

        UIElement aElement = implts_findToolbar( aAddonToolBarName );

        // #i79828
        // It's now possible that we are called more than once. Be sure to not create
        // add-on toolbars more than once!
        if ( aElement.m_xUIElement.is() )
            continue;

        try
        {
            xUIElement = xUIElementFactory->createUIElement( aAddonToolBarName, aPropSeq );
            if ( xUIElement.is() )
            {
                uno::Reference< awt::XDockableWindow > xDockWindow( xUIElement->getRealInterface(), uno::UNO_QUERY );
                if ( xDockWindow.is() )
                {
                    try
                    {
                        xDockWindow->addDockableWindowListener( uno::Reference< awt::XDockableWindowListener >( static_cast< OWeakObject * >( this ), uno::UNO_QUERY ));
                        xDockWindow->enableDocking( sal_True );
                        uno::Reference< awt::XWindow > xWindow( xDockWindow, uno::UNO_QUERY );
                        if ( xWindow.is() )
                            xWindow->addWindowListener( uno::Reference< awt::XWindowListener >( static_cast< OWeakObject * >( this ), uno::UNO_QUERY ));
                    }
                    catch (const uno::Exception&)
                    {
                    }
                }

                ::rtl::OUString aGenericAddonTitle = implts_generateGenericAddonToolbarTitle( i+1 );

                if ( !aElement.m_aName.isEmpty() )
                {
                    // Reuse a local entry so we are able to use the latest
                    // UI changes for this document.
                    implts_setElementData( aElement, xDockWindow );
                    aElement.m_xUIElement = xUIElement;
                    if ( aElement.m_aUIName.isEmpty() )
                    {
                        aElement.m_aUIName = aGenericAddonTitle;
                        implts_writeWindowStateData( aElement );
                    }
                }
                else
                {
                    // Create new UI element and try to read its state data
                    UIElement aNewToolbar( aAddonToolBarName, aElementType, xUIElement );
                    aNewToolbar.m_bFloating = true;
                    implts_readWindowStateData( aAddonToolBarName, aNewToolbar );
                    implts_setElementData( aNewToolbar, xDockWindow );
                    if ( aNewToolbar.m_aUIName.isEmpty() )
                    {
                        aNewToolbar.m_aUIName = aGenericAddonTitle;
                        implts_writeWindowStateData( aNewToolbar );
                    }
                    implts_insertToolbar( aNewToolbar );
                }

                uno::Reference< awt::XWindow > xWindow( xDockWindow, uno::UNO_QUERY );
                if ( xWindow.is() )
                {
                    // Set generic title for add-on toolbar
                    SolarMutexGuard aGuard;
                    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                    if ( pWindow->GetText().Len() == 0 )
                        pWindow->SetText( aGenericAddonTitle );
                    if ( pWindow->GetType() == WINDOW_TOOLBOX )
                    {
                        ToolBox* pToolbar = (ToolBox *)pWindow;
                        pToolbar->SetMenuType();
                    }
                }
            }
        }
        catch (const container::NoSuchElementException&)
        {
        }
        catch (const lang::IllegalArgumentException&)
        {
        }
    }
}

void ToolbarLayoutManager::implts_createCustomToolBars()
{
    ReadGuard aReadLock( m_aLock );
    if ( !m_bComponentAttached )
        return;

    uno::Reference< ui::XUIElementFactory > xUIElementFactory( m_xUIElementFactoryManager );
    uno::Reference< frame::XFrame > xFrame( m_xFrame );
    uno::Reference< ui::XUIConfigurationManager > xModuleCfgMgr( m_xModuleCfgMgr, uno::UNO_QUERY );
    uno::Reference< ui::XUIConfigurationManager > xDocCfgMgr( m_xDocCfgMgr, uno::UNO_QUERY );
    aReadLock.unlock();

    if ( xFrame.is() )
    {
        if (isPreviewFrame())
            return; // no custom toolbars for preview frame!

        uno::Sequence< uno::Sequence< beans::PropertyValue > > aTbxSeq;
        if ( xDocCfgMgr.is() )
        {
            aTbxSeq = xDocCfgMgr->getUIElementsInfo( ui::UIElementType::TOOLBAR );
            implts_createCustomToolBars( aTbxSeq ); // first create all document based toolbars
        }
        if ( xModuleCfgMgr.is() )
        {
            aTbxSeq = xModuleCfgMgr->getUIElementsInfo( ui::UIElementType::TOOLBAR );
            implts_createCustomToolBars( aTbxSeq ); // second create module based toolbars
        }
    }
}

void ToolbarLayoutManager::implts_createNonContextSensitiveToolBars()
{
    ReadGuard aReadLock( m_aLock );

    if ( !m_xPersistentWindowState.is() || !m_xFrame.is() || !m_bComponentAttached )
        return;

    uno::Reference< ui::XUIElementFactory >  xUIElementFactory( m_xUIElementFactoryManager );
    uno::Reference< container::XNameAccess > xPersistentWindowState( m_xPersistentWindowState );
    aReadLock.unlock();

    if (isPreviewFrame())
        return;

    std::vector< rtl::OUString > aMakeVisibleToolbars;

    try
    {
        uno::Sequence< ::rtl::OUString > aToolbarNames = xPersistentWindowState->getElementNames();

        if ( aToolbarNames.getLength() > 0 )
        {
            ::rtl::OUString aElementType;
            ::rtl::OUString aElementName;
            ::rtl::OUString aName;

            uno::Reference< ui::XUIElement > xUIElement;
            aMakeVisibleToolbars.reserve(aToolbarNames.getLength());

            /* SAFE AREA ----------------------------------------------------------------------------------------------- */
            WriteGuard aWriteLock( m_aLock );

            const rtl::OUString* pTbNames = aToolbarNames.getConstArray();
            for ( sal_Int32 i = 0; i < aToolbarNames.getLength(); i++ )
            {
                aName = pTbNames[i];
                parseResourceURL( aName, aElementType, aElementName );

                // Check that we only create:
                // - Toolbars (the statusbar is also member of the persistent window state)
                // - Not custom toolbars, there are created with their own method (implts_createCustomToolbars)
                if ( aElementType.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("toolbar")) &&
                     aElementName.indexOf( m_aCustomTbxPrefix ) == -1 )
                {
                    UIElement aNewToolbar = implts_findToolbar( aName );
                    bool bFound = ( aNewToolbar.m_aName == aName );
                    if ( !bFound )
                        implts_readWindowStateData( aName, aNewToolbar );

                    if ( aNewToolbar.m_bVisible && !aNewToolbar.m_bContextSensitive )
                    {
                        if ( !bFound )
                            implts_insertToolbar( aNewToolbar );
                        aMakeVisibleToolbars.push_back( aName );
                    }
                }
            }
            /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        }
    }
    catch (const uno::RuntimeException&)
    {
        throw;
    }
    catch (const uno::Exception&)
    {
    }

    if ( !aMakeVisibleToolbars.empty() )
        ::std::for_each( aMakeVisibleToolbars.begin(), aMakeVisibleToolbars.end(),
                ::boost::bind( &ToolbarLayoutManager::requestToolbar, this, _1));
}

void ToolbarLayoutManager::implts_createCustomToolBars( const uno::Sequence< uno::Sequence< beans::PropertyValue > >& aTbxSeqSeq )
{
    const uno::Sequence< beans::PropertyValue >* pTbxSeq = aTbxSeqSeq.getConstArray();
    for ( sal_Int32 i = 0; i < aTbxSeqSeq.getLength(); i++ )
    {
        const uno::Sequence< beans::PropertyValue >& rTbxSeq = pTbxSeq[i];
        ::rtl::OUString aTbxResName;
        ::rtl::OUString aTbxTitle;
        for ( sal_Int32 j = 0; j < rTbxSeq.getLength(); j++ )
        {
            if ( rTbxSeq[j].Name == "ResourceURL" )
                rTbxSeq[j].Value >>= aTbxResName;
            else if ( rTbxSeq[j].Name == "UIName" )
                rTbxSeq[j].Value >>= aTbxTitle;
        }

        // Only create custom toolbars. Their name have to start with "custom_"!
        if ( !aTbxResName.isEmpty() && ( aTbxResName.indexOf( m_aCustomTbxPrefix ) != -1 ) )
            implts_createCustomToolBar( aTbxResName, aTbxTitle );
    }
}

void ToolbarLayoutManager::implts_createCustomToolBar( const rtl::OUString& aTbxResName, const rtl::OUString& aTitle )
{
    if ( !aTbxResName.isEmpty() )
    {
        bool bNotify( false );
        uno::Reference< ui::XUIElement > xUIElement;
        implts_createToolBar( aTbxResName, bNotify, xUIElement );

        if ( !aTitle.isEmpty() && xUIElement.is() )
        {
            SolarMutexGuard aGuard;

            Window* pWindow = getWindowFromXUIElement( xUIElement );
            if ( pWindow  )
                pWindow->SetText( aTitle );
        }
    }
}

void ToolbarLayoutManager::implts_reparentToolbars()
{
    WriteGuard aWriteLock( m_aLock );
    UIElementVector aUIElementVector = m_aUIElements;
    Window* pContainerWindow  = VCLUnoHelper::GetWindow( m_xContainerWindow );
    Window* pTopDockWindow    = VCLUnoHelper::GetWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_TOP] );
    Window* pBottomDockWindow = VCLUnoHelper::GetWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_BOTTOM] );
    Window* pLeftDockWindow   = VCLUnoHelper::GetWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_LEFT] );
    Window* pRightDockWindow  = VCLUnoHelper::GetWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_RIGHT] );
    aWriteLock.unlock();

    SolarMutexGuard aGuard;
    if ( pContainerWindow )
    {
        UIElementVector::iterator pIter;
        for ( pIter = aUIElementVector.begin(); pIter != aUIElementVector.end(); ++pIter )
        {
            uno::Reference< ui::XUIElement > xUIElement( pIter->m_xUIElement );
            if ( xUIElement.is() )
            {
                uno::Reference< awt::XWindow > xWindow;
                try
                {
                    // We have to retreive the window reference with try/catch as it is
                    // possible that all elements have been disposed!
                    xWindow = uno::Reference< awt::XWindow >( xUIElement->getRealInterface(), uno::UNO_QUERY );
                }
                catch (const uno::RuntimeException&)
                {
                    throw;
                }
                catch (const uno::Exception&)
                {
                }

                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow )
                {
                    // Reparent our child windows acording to their current state.
                    if ( pIter->m_bFloating )
                        pWindow->SetParent( pContainerWindow );
                    else
                    {
                        if ( pIter->m_aDockedData.m_nDockedArea == ui::DockingArea_DOCKINGAREA_TOP )
                            pWindow->SetParent( pTopDockWindow );
                        else if ( pIter->m_aDockedData.m_nDockedArea == ui::DockingArea_DOCKINGAREA_BOTTOM )
                            pWindow->SetParent( pBottomDockWindow );
                        else if ( pIter->m_aDockedData.m_nDockedArea == ui::DockingArea_DOCKINGAREA_LEFT )
                            pWindow->SetParent( pLeftDockWindow );
                        else
                            pWindow->SetParent( pRightDockWindow );
                    }
                }
            }
        }
    }
}

void ToolbarLayoutManager::implts_setToolbarCreation( bool bStart )
{
    WriteGuard aWriteLock( m_aLock );
    m_bToolbarCreation = bStart;
}

bool ToolbarLayoutManager::implts_isToolbarCreationActive()
{
    ReadGuard aReadLock( m_aLock );
    return m_bToolbarCreation;
}

void ToolbarLayoutManager::implts_createToolBar( const ::rtl::OUString& aName, bool& bNotify, uno::Reference< ui::XUIElement >& rUIElement )
{
    ReadGuard aReadLock( m_aLock );
    uno::Reference< frame::XFrame > xFrame( m_xFrame );
    uno::Reference< awt::XWindow2 > xContainerWindow( m_xContainerWindow );
    aReadLock.unlock();

    bNotify = false;

    if ( !xFrame.is() || !xContainerWindow.is() )
        return;

    UIElement aToolbarElement = implts_findToolbar( aName );
    if ( !aToolbarElement.m_xUIElement.is()  )
    {
        uno::Reference< ui::XUIElement > xUIElement = implts_createElement( aName );

        bool bVisible( false );
        bool bFloating( false );
        if ( xUIElement.is() )
        {
            rUIElement = xUIElement;

            uno::Reference< awt::XWindow > xWindow( xUIElement->getRealInterface(), uno::UNO_QUERY );
            uno::Reference< awt::XDockableWindow > xDockWindow( xWindow, uno::UNO_QUERY );
            if ( xDockWindow.is() && xWindow.is() )
            {
                try
                {
                    xDockWindow->addDockableWindowListener( uno::Reference< awt::XDockableWindowListener >(
                        static_cast< OWeakObject * >( this ), uno::UNO_QUERY ));
                    xWindow->addWindowListener( uno::Reference< awt::XWindowListener >(
                        static_cast< OWeakObject * >( this ), uno::UNO_QUERY ));
                    xDockWindow->enableDocking( sal_True );
                }
                catch (const uno::Exception&)
                {
                }
            }

            /* SAFE AREA ----------------------------------------------------------------------------------------------- */
            WriteGuard aWriteLock( m_aLock );

            UIElement& rElement = impl_findToolbar( aName );
            if ( !rElement.m_aName.isEmpty() )
            {
                // Reuse a local entry so we are able to use the latest
                // UI changes for this document.
                implts_setElementData( rElement, xDockWindow );
                rElement.m_xUIElement = xUIElement;
                bVisible = rElement.m_bVisible;
                bFloating = rElement.m_bFloating;
            }
            else
            {
                // Create new UI element and try to read its state data
                UIElement aNewToolbar( aName, m_aToolbarTypeString, xUIElement );
                implts_readWindowStateData( aName, aNewToolbar );
                implts_setElementData( aNewToolbar, xDockWindow );
                implts_insertToolbar( aNewToolbar );
                bVisible = aNewToolbar.m_bVisible;
                bFloating = rElement.m_bFloating;
            }
            aWriteLock.unlock();
            /* SAFE AREA ----------------------------------------------------------------------------------------------- */

            // set toolbar menu style according to customize command state
            SvtCommandOptions aCmdOptions;

            SolarMutexGuard aGuard;
            Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
            if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
            {
                ToolBox* pToolbar = (ToolBox *)pWindow;
                sal_uInt16 nMenuType = pToolbar->GetMenuType();
                if ( aCmdOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED, m_aCustomizeCmd ))
                    pToolbar->SetMenuType( nMenuType & ~TOOLBOX_MENUTYPE_CUSTOMIZE );
                else
                    pToolbar->SetMenuType( nMenuType | TOOLBOX_MENUTYPE_CUSTOMIZE );
            }
            bNotify = true;

            implts_sortUIElements();

            if ( bVisible && !bFloating )
                implts_setLayoutDirty();
        }
    }
}

uno::Reference< ui::XUIElement > ToolbarLayoutManager::implts_createElement( const ::rtl::OUString& aName )
{
    uno::Reference< ui::XUIElement > xUIElement;

    ReadGuard aReadLock( m_aLock );
    uno::Sequence< beans::PropertyValue > aPropSeq( 2 );
    aPropSeq[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    aPropSeq[0].Value <<= m_xFrame;
    aPropSeq[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Persistent" ));
    aPropSeq[1].Value <<= true;
    uno::Reference< ui::XUIElementFactory > xUIElementFactory( m_xUIElementFactoryManager );
    aReadLock.unlock();

    implts_setToolbarCreation( true );
    try
    {
        if ( xUIElementFactory.is() )
            xUIElement = xUIElementFactory->createUIElement( aName, aPropSeq );
    }
    catch (const container::NoSuchElementException&)
    {
    }
    catch (const lang::IllegalArgumentException&)
    {
    }
    implts_setToolbarCreation( false );

    return xUIElement;
}

void ToolbarLayoutManager::implts_setElementData( UIElement& rElement, const uno::Reference< awt::XDockableWindow >& rDockWindow )
{
    ReadGuard aReadLock( m_aLock );
    bool bShowElement( rElement.m_bVisible && !rElement.m_bMasterHide && implts_isParentWindowVisible() );
    aReadLock.unlock();

    uno::Reference< awt::XDockableWindow > xDockWindow( rDockWindow );
    uno::Reference< awt::XWindow2 >        xWindow( xDockWindow, uno::UNO_QUERY );

    Window*  pWindow( 0 );
    ToolBox* pToolBox( 0 );

    if ( xDockWindow.is() && xWindow.is() )
    {
        {
            SolarMutexGuard aGuard;
            pWindow = VCLUnoHelper::GetWindow( xWindow );
            if ( pWindow )
            {
                String aText = pWindow->GetText();
                if ( aText.Len() == 0 )
                    pWindow->SetText( rElement.m_aUIName );
                if ( rElement.m_bNoClose )
                    pWindow->SetStyle( pWindow->GetStyle() & ~WB_CLOSEABLE );
                if ( pWindow->GetType() == WINDOW_TOOLBOX )
                    pToolBox = (ToolBox *)pWindow;
            }
            if ( pToolBox )
            {
                if (( rElement.m_nStyle < 0 ) || ( rElement.m_nStyle > BUTTON_SYMBOLTEXT ))
                    rElement.m_nStyle = BUTTON_SYMBOL;
                pToolBox->SetButtonType( (ButtonType)rElement.m_nStyle );
                if ( rElement.m_bNoClose )
                    pToolBox->SetFloatStyle( pToolBox->GetFloatStyle() & ~WB_CLOSEABLE );
            }
        }

        if ( rElement.m_bFloating )
        {
            if ( pWindow )
            {
                SolarMutexGuard aGuard;
                String aText = pWindow->GetText();
                if ( aText.Len() == 0 )
                    pWindow->SetText( rElement.m_aUIName );
            }

            awt::Point aPos(rElement.m_aFloatingData.m_aPos);
            bool bWriteData( false );
            bool bUndefPos = hasDefaultPosValue( rElement.m_aFloatingData.m_aPos );
            bool bSetSize = ( rElement.m_aFloatingData.m_aSize.Width != 0 &&
                              rElement.m_aFloatingData.m_aSize.Height != 0 );
            xDockWindow->setFloatingMode( sal_True );
            if ( bUndefPos )
            {
                aPos = implts_findNextCascadeFloatingPos();
                rElement.m_aFloatingData.m_aPos = aPos; // set new cascaded position
                bWriteData = true;
            }

            if( bSetSize )
                xWindow->setOutputSize(rElement.m_aFloatingData.m_aSize);
            else
            {
                if( pToolBox )
                {
                    // set an optimal initial floating size
                    SolarMutexGuard aGuard;
                    ::Size aSize( pToolBox->CalcFloatingWindowSizePixel() );
                    pToolBox->SetOutputSizePixel( aSize );
                }
            }

            // #i60882# IMPORTANT: Set position after size as it is
            // possible that we position some part of the toolbar
            // outside of the desktop. A default constructed toolbar
            // always has one line. Now VCL automatically
            // position the toolbar back into the desktop. Therefore
            // we resize the toolbar with the new (wrong) position.
            // To fix this problem we have to set the size BEFORE the
            // position.
            xWindow->setPosSize( aPos.X, aPos.Y, 0, 0, awt::PosSize::POS );

            if ( bWriteData )
                implts_writeWindowStateData( rElement );
            if ( bShowElement && pWindow )
            {
                SolarMutexGuard aGuard;
                pWindow->Show( sal_True, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
            }
        }
        else
        {
            bool    bSetSize( false );
            awt::Point aDockPos;
            ::Point aPixelPos;
            ::Size  aSize;

            if ( pToolBox )
            {
                SolarMutexGuard aGuard;
                pToolBox->SetAlign( ImplConvertAlignment(rElement.m_aDockedData.m_nDockedArea )  );
                pToolBox->SetLineCount( 1 );
                xDockWindow->setFloatingMode( sal_False );
                if ( rElement.m_aDockedData.m_bLocked )
                    xDockWindow->lock();
                aSize = pToolBox->CalcWindowSizePixel();
                bSetSize = true;

                if ( isDefaultPos( rElement.m_aDockedData.m_aPos ))
                {
                    implts_findNextDockingPos( (ui::DockingArea)rElement.m_aDockedData.m_nDockedArea, aSize, aDockPos, aPixelPos );
                    rElement.m_aDockedData.m_aPos = aDockPos;
                }
            }

            xWindow->setPosSize( aPixelPos.X(), aPixelPos.Y(), 0, 0, awt::PosSize::POS );
            if( bSetSize )
                xWindow->setOutputSize( AWTSize( aSize) );

            if ( pWindow )
            {
                SolarMutexGuard aGuard;
                if ( !bShowElement )
                    pWindow->Hide();
            }
        }
    }
}

void ToolbarLayoutManager::implts_destroyDockingAreaWindows()
{
    WriteGuard aWriteLock( m_aLock );
    uno::Reference< awt::XWindow > xTopDockingWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_TOP] );
    uno::Reference< awt::XWindow > xLeftDockingWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_LEFT] );
    uno::Reference< awt::XWindow > xRightDockingWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_RIGHT] );
    uno::Reference< awt::XWindow > xBottomDockingWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_BOTTOM] );
    m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_TOP].clear();
    m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_LEFT].clear();
    m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_RIGHT].clear();
    m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_BOTTOM].clear();
    aWriteLock.unlock();

    // destroy windows
    xTopDockingWindow->dispose();
    xLeftDockingWindow->dispose();
    xRightDockingWindow->dispose();
    xBottomDockingWindow->dispose();
}

//---------------------------------------------------------------------------------------------------------
// persistence methods
//---------------------------------------------------------------------------------------------------------

sal_Bool ToolbarLayoutManager::implts_readWindowStateData( const rtl::OUString& aName, UIElement& rElementData )
{
    WriteGuard aWriteLock( m_aLock );
    uno::Reference< container::XNameAccess > xPersistentWindowState( m_xPersistentWindowState );
    bool bGetSettingsState( false );
    aWriteLock.unlock();

    if ( xPersistentWindowState.is() )
    {
        aWriteLock.lock();
        bool bGlobalSettings( m_bGlobalSettings );
        GlobalSettings* pGlobalSettings( 0 );
        if ( m_pGlobalSettings.get() == 0 )
        {
            m_pGlobalSettings.reset( new GlobalSettings( m_xSMGR ) );
            bGetSettingsState = true;
        }
        pGlobalSettings = m_pGlobalSettings.get();
        aWriteLock.unlock();

        try
        {
            uno::Sequence< beans::PropertyValue > aWindowState;
            if ( xPersistentWindowState->getByName( aName ) >>= aWindowState )
            {
                sal_Bool bValue( sal_False );
                for ( sal_Int32 n = 0; n < aWindowState.getLength(); n++ )
                {
                    if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_DOCKED ))
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bFloating = !bValue;
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_VISIBLE ))
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bVisible = bValue;
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_DOCKINGAREA ))
                    {
                        ui::DockingArea eDockingArea;
                        if ( aWindowState[n].Value >>= eDockingArea )
                            rElementData.m_aDockedData.m_nDockedArea = sal_Int16( eDockingArea );
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_DOCKPOS ))
                    {
                        awt::Point aPoint;
                        if ( aWindowState[n].Value >>= aPoint )
                            rElementData.m_aDockedData.m_aPos = aPoint;
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_POS ))
                    {
                        awt::Point aPoint;
                        if ( aWindowState[n].Value >>= aPoint )
                            rElementData.m_aFloatingData.m_aPos = aPoint;
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_SIZE ))
                    {
                        awt::Size aSize;
                        if ( aWindowState[n].Value >>= aSize )
                            rElementData.m_aFloatingData.m_aSize = aSize;
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_UINAME ))
                        aWindowState[n].Value >>= rElementData.m_aUIName;
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_STYLE ))
                    {
                        sal_Int32 nStyle = 0;
                        if ( aWindowState[n].Value >>= nStyle )
                            rElementData.m_nStyle = sal_Int16( nStyle );
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_LOCKED ))
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_aDockedData.m_bLocked = bValue;
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_CONTEXT ))
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bContextSensitive = bValue;
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_NOCLOSE ))
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bNoClose = bValue;
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_CONTEXTACTIVE ))
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bContextActive = bValue;
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_SOFTCLOSE ))
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bSoftClose = bValue;
                    }
                }
            }

            // oversteer values with global settings
            if ( pGlobalSettings && ( bGetSettingsState || bGlobalSettings ))
            {
                if ( pGlobalSettings->HasStatesInfo( GlobalSettings::UIELEMENT_TYPE_TOOLBAR ))
                {
                    WriteGuard aWriteLock2( m_aLock );
                    m_bGlobalSettings = true;
                    aWriteLock2.unlock();

                    uno::Any aValue;
                    sal_Bool bValue = sal_Bool();
                    if ( pGlobalSettings->GetStateInfo( GlobalSettings::UIELEMENT_TYPE_TOOLBAR,
                                                        GlobalSettings::STATEINFO_LOCKED,
                                                        aValue ))
                        aValue >>= rElementData.m_aDockedData.m_bLocked;
                    if ( pGlobalSettings->GetStateInfo( GlobalSettings::UIELEMENT_TYPE_TOOLBAR,
                                                        GlobalSettings::STATEINFO_DOCKED,
                                                        aValue ))
                    {
                        if ( aValue >>= bValue )
                            rElementData.m_bFloating = !bValue;
                    }
                }
            }

            return sal_True;
        }
        catch (const container::NoSuchElementException&)
        {
        }
    }

    return sal_False;
}

void ToolbarLayoutManager::implts_writeWindowStateData( const UIElement& rElementData )
{
    WriteGuard aWriteLock( m_aLock );
    uno::Reference< container::XNameAccess > xPersistentWindowState( m_xPersistentWindowState );
    m_bStoreWindowState = true; // set flag to determine that we triggered the notification
    aWriteLock.unlock();

    bool bPersistent( sal_False );
    uno::Reference< beans::XPropertySet > xPropSet( rElementData.m_xUIElement, uno::UNO_QUERY );
    if ( xPropSet.is() )
    {
        try
        {
            // Check persistent flag of the user interface element
            xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Persistent" ))) >>= bPersistent;
        }
        catch (const beans::UnknownPropertyException&)
        {
            bPersistent = true; // Non-configurable elements should at least store their dimension/position
        }
        catch (const lang::WrappedTargetException&)
        {
        }
    }

    if ( bPersistent && xPersistentWindowState.is() )
    {
        try
        {
            uno::Sequence< beans::PropertyValue > aWindowState( 8 );

            aWindowState[0].Name  = ::rtl::OUString::createFromAscii( WINDOWSTATE_PROPERTY_DOCKED );
            aWindowState[0].Value = ::uno::makeAny( sal_Bool( !rElementData.m_bFloating ));
            aWindowState[1].Name  = ::rtl::OUString::createFromAscii( WINDOWSTATE_PROPERTY_VISIBLE );
            aWindowState[1].Value = uno::makeAny( sal_Bool( rElementData.m_bVisible ));
            aWindowState[2].Name  = ::rtl::OUString::createFromAscii( WINDOWSTATE_PROPERTY_DOCKINGAREA );
            aWindowState[2].Value = uno::makeAny( static_cast< ui::DockingArea >( rElementData.m_aDockedData.m_nDockedArea ) );

            awt::Point aPos = rElementData.m_aDockedData.m_aPos;
            aWindowState[3].Name  = ::rtl::OUString::createFromAscii( WINDOWSTATE_PROPERTY_DOCKPOS );
            aWindowState[3].Value <<= aPos;

            aPos = rElementData.m_aFloatingData.m_aPos;
            aWindowState[4].Name  = ::rtl::OUString::createFromAscii( WINDOWSTATE_PROPERTY_POS );
            aWindowState[4].Value <<= aPos;

            aWindowState[5].Name  = ::rtl::OUString::createFromAscii( WINDOWSTATE_PROPERTY_SIZE );
            aWindowState[5].Value <<= rElementData.m_aFloatingData.m_aSize;
            aWindowState[6].Name  = ::rtl::OUString::createFromAscii( WINDOWSTATE_PROPERTY_UINAME );
            aWindowState[6].Value = uno::makeAny( rElementData.m_aUIName );
            aWindowState[7].Name  = ::rtl::OUString::createFromAscii( WINDOWSTATE_PROPERTY_LOCKED );
            aWindowState[7].Value = uno::makeAny( rElementData.m_aDockedData.m_bLocked );

            ::rtl::OUString aName = rElementData.m_aName;
            if ( xPersistentWindowState->hasByName( aName ))
            {
                uno::Reference< container::XNameReplace > xReplace( xPersistentWindowState, uno::UNO_QUERY );
                xReplace->replaceByName( aName, uno::makeAny( aWindowState ));
            }
            else
            {
                uno::Reference< container::XNameContainer > xInsert( xPersistentWindowState, uno::UNO_QUERY );
                xInsert->insertByName( aName, uno::makeAny( aWindowState ));
            }
        }
        catch (const uno::Exception&)
        {
        }
    }

    // Reset flag
    aWriteLock.lock();
    m_bStoreWindowState = false;
    aWriteLock.unlock();
}

/******************************************************************************
                        LOOKUP PART FOR TOOLBARS
******************************************************************************/

UIElement& ToolbarLayoutManager::impl_findToolbar( const rtl::OUString& aName )
{
    static UIElement aEmptyElement;
    UIElementVector::iterator pIter;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_aName == aName )
            return *pIter;
    }

    return aEmptyElement;
}

UIElement ToolbarLayoutManager::implts_findToolbar( const rtl::OUString& aName )
{
    ReadGuard aReadLock( m_aLock );
    UIElement aElement = impl_findToolbar( aName );
    aReadLock.unlock();

    return aElement;
}

UIElement ToolbarLayoutManager::implts_findToolbar( const uno::Reference< uno::XInterface >& xToolbar )
{
    UIElement                       aToolbar;
    UIElementVector::const_iterator pIter;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_xUIElement.is() )
        {
            uno::Reference< uno::XInterface > xIfac( pIter->m_xUIElement->getRealInterface(), uno::UNO_QUERY );
            if ( xIfac == xToolbar )
            {
                aToolbar = *pIter;
                break;
            }
        }
    }

    return aToolbar;
}

uno::Reference< awt::XWindow > ToolbarLayoutManager::implts_getXWindow( const ::rtl::OUString& aName )
{
    UIElementVector::iterator pIter;
    uno::Reference< awt::XWindow > xWindow;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_aName == aName && pIter->m_xUIElement.is() )
        {
             xWindow = uno::Reference< awt::XWindow >( pIter->m_xUIElement->getRealInterface(), uno::UNO_QUERY );
             break;
        }
    }

    return xWindow;
}

Window* ToolbarLayoutManager::implts_getWindow( const ::rtl::OUString& aName )
{
    uno::Reference< awt::XWindow > xWindow = implts_getXWindow( aName );
    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );

    return pWindow;
}

bool ToolbarLayoutManager::implts_insertToolbar( const UIElement& rUIElement )
{
    UIElement aTempData;
    bool      bFound( false );
    bool      bResult( false );

    aTempData = implts_findToolbar( rUIElement.m_aName );
    if ( aTempData.m_aName == rUIElement.m_aName )
        bFound = true;

    if ( !bFound )
    {
        WriteGuard aWriteLock( m_aLock );
        m_aUIElements.push_back( rUIElement );
        bResult = true;
    }

    return bResult;
}

void ToolbarLayoutManager::implts_setToolbar( const UIElement& rUIElement )
{
    WriteGuard aWriteLock( m_aLock );
    UIElement& rData = impl_findToolbar( rUIElement.m_aName );
    if ( rData.m_aName == rUIElement.m_aName )
        rData = rUIElement;
    else
        m_aUIElements.push_back( rUIElement );
}

/******************************************************************************
                        LAYOUT CODE PART FOR TOOLBARS
******************************************************************************/

awt::Point ToolbarLayoutManager::implts_findNextCascadeFloatingPos()
{
    const sal_Int32 nHotZoneX       = 50;
    const sal_Int32 nHotZoneY       = 50;
    const sal_Int32 nCascadeIndentX = 15;
    const sal_Int32 nCascadeIndentY = 15;

    ReadGuard aReadLock( m_aLock );
    uno::Reference< awt::XWindow2 > xContainerWindow( m_xContainerWindow );
    uno::Reference< awt::XWindow > xTopDockingWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_TOP] );
    uno::Reference< awt::XWindow > xLeftDockingWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_LEFT] );
    aReadLock.unlock();

    awt::Point aStartPos( nCascadeIndentX, nCascadeIndentY );
    awt::Point aCurrPos( aStartPos );
    awt::Rectangle aRect;

    Window* pContainerWindow( 0 );
    if ( xContainerWindow.is() )
    {
        SolarMutexGuard aGuard;
        pContainerWindow = VCLUnoHelper::GetWindow( xContainerWindow );
        if ( pContainerWindow )
            aStartPos = AWTPoint(pContainerWindow->OutputToScreenPixel(VCLPoint(aStartPos)));
    }

    // Determine size of top and left docking area
    awt::Rectangle aTopRect( xTopDockingWindow->getPosSize() );
    awt::Rectangle aLeftRect( xLeftDockingWindow->getPosSize() );

    aStartPos.X += aLeftRect.Width + nCascadeIndentX;
    aStartPos.Y += aTopRect.Height + nCascadeIndentY;
    aCurrPos = aStartPos;

    // Try to find a cascaded position for the new floating window
    UIElementVector::const_iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_xUIElement.is() )
        {
            uno::Reference< awt::XDockableWindow > xDockWindow( pIter->m_xUIElement->getRealInterface(), uno::UNO_QUERY );
            uno::Reference< awt::XWindow > xWindow( xDockWindow, uno::UNO_QUERY );
            if ( xDockWindow.is() && xDockWindow->isFloating() )
            {
                SolarMutexGuard aGuard;
                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow && pWindow->IsVisible() )
                {
                    awt::Rectangle aFloatRect = xWindow->getPosSize();
                    if ((( aFloatRect.X - nHotZoneX ) <= aCurrPos.X ) &&
                        ( aFloatRect.X >= aCurrPos.X ) &&
                        (( aFloatRect.Y - nHotZoneY ) <= aCurrPos.Y ) &&
                        ( aFloatRect.Y >= aCurrPos.Y ))
                    {
                        aCurrPos.X = aFloatRect.X + nCascadeIndentX;
                        aCurrPos.Y = aFloatRect.Y + nCascadeIndentY;
                    }
                }
            }
        }
    }

    return aCurrPos;
}

void ToolbarLayoutManager::implts_sortUIElements()
{
    WriteGuard aWriteLock( m_aLock );
    UIElementVector::iterator pIterStart = m_aUIElements.begin();
    UIElementVector::iterator pIterEnd   = m_aUIElements.end();

    std::stable_sort( pIterStart, pIterEnd ); // first created element should first

    // We have to reset our temporary flags.
    UIElementVector::iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
        pIter->m_bUserActive = sal_False;
    aWriteLock.unlock();
}

void ToolbarLayoutManager::implts_getUIElementVectorCopy( UIElementVector& rCopy )
{
    ReadGuard aReadLock( m_aLock );
    rCopy = m_aUIElements;
}

::Size ToolbarLayoutManager::implts_getTopBottomDockingAreaSizes()
{
    ::Size                         aSize;
    uno::Reference< awt::XWindow > xTopDockingAreaWindow;
    uno::Reference< awt::XWindow > xBottomDockingAreaWindow;

    ReadGuard aReadLock( m_aLock );
    xTopDockingAreaWindow    = m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_TOP];
    xBottomDockingAreaWindow = m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_BOTTOM];
    aReadLock.unlock();

    if ( xTopDockingAreaWindow.is() )
        aSize.Width() = xTopDockingAreaWindow->getPosSize().Height;
    if ( xBottomDockingAreaWindow.is() )
        aSize.Height() = xBottomDockingAreaWindow->getPosSize().Height;

    return aSize;
}

void ToolbarLayoutManager::implts_getDockingAreaElementInfos( ui::DockingArea eDockingArea, std::vector< SingleRowColumnWindowData >& rRowColumnsWindowData )
{
    std::vector< UIElement > aWindowVector;

    if (( eDockingArea < ui::DockingArea_DOCKINGAREA_TOP ) || ( eDockingArea > ui::DockingArea_DOCKINGAREA_RIGHT ))
        eDockingArea = ui::DockingArea_DOCKINGAREA_TOP;

    uno::Reference< awt::XWindow > xDockAreaWindow;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    aWindowVector.reserve(m_aUIElements.size());
    xDockAreaWindow = m_xDockAreaWindows[eDockingArea];
    UIElementVector::iterator   pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_aDockedData.m_nDockedArea == eDockingArea && pIter->m_bVisible && !pIter->m_bFloating )
        {
            uno::Reference< ui::XUIElement > xUIElement( pIter->m_xUIElement );
            if ( xUIElement.is() )
            {
                uno::Reference< awt::XWindow > xWindow( xUIElement->getRealInterface(), uno::UNO_QUERY );
                uno::Reference< awt::XDockableWindow > xDockWindow( xWindow, uno::UNO_QUERY );
                if ( xDockWindow.is() )
                {
                    // docked windows
                    aWindowVector.push_back( *pIter );
                }
            }
        }
    }
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    rRowColumnsWindowData.clear();

    // Collect data from windows that are on the same row/column
    sal_Int32 j;
    sal_Int32 nIndex( 0 );
    sal_Int32 nLastPos( 0 );
    sal_Int32 nCurrPos( -1 );
    sal_Int32 nLastRowColPixelPos( 0 );
    awt::Rectangle aDockAreaRect;

    if ( xDockAreaWindow.is() )
        aDockAreaRect = xDockAreaWindow->getPosSize();

    if ( eDockingArea == ui::DockingArea_DOCKINGAREA_TOP )
        nLastRowColPixelPos = 0;
    else if ( eDockingArea == ui::DockingArea_DOCKINGAREA_BOTTOM )
        nLastRowColPixelPos = aDockAreaRect.Height;
    else if ( eDockingArea == ui::DockingArea_DOCKINGAREA_LEFT )
        nLastRowColPixelPos = 0;
    else
        nLastRowColPixelPos = aDockAreaRect.Width;

    const sal_uInt32 nCount = aWindowVector.size();
    for ( j = 0; j < sal_Int32( nCount); j++ )
    {
        const UIElement& rElement = aWindowVector[j];
        uno::Reference< awt::XWindow > xWindow;
        uno::Reference< ui::XUIElement > xUIElement( rElement.m_xUIElement );
        awt::Rectangle aPosSize;

        if ( !lcl_checkUIElement(xUIElement,aPosSize,xWindow) )
            continue;
        if ( isHorizontalDockingArea( eDockingArea ))
        {
            if ( nCurrPos == -1 )
            {
                nCurrPos = rElement.m_aDockedData.m_aPos.Y;
                nLastPos = 0;

                SingleRowColumnWindowData aRowColumnWindowData;
                aRowColumnWindowData.nRowColumn = nCurrPos;
                rRowColumnsWindowData.push_back( aRowColumnWindowData );
            }

            sal_Int32 nSpace( 0 );
            if ( rElement.m_aDockedData.m_aPos.Y != nCurrPos )
            {
                if ( eDockingArea == ui::DockingArea_DOCKINGAREA_TOP )
                    nLastRowColPixelPos += rRowColumnsWindowData[nIndex].nStaticSize;
                else
                    nLastRowColPixelPos -= rRowColumnsWindowData[nIndex].nStaticSize;
                ++nIndex;
                nLastPos = 0;
                nCurrPos = rElement.m_aDockedData.m_aPos.Y;
                SingleRowColumnWindowData aRowColumnWindowData;
                aRowColumnWindowData.nRowColumn = nCurrPos;
                rRowColumnsWindowData.push_back( aRowColumnWindowData );
            }

            // Calc space before an element and store it
            nSpace = ( rElement.m_aDockedData.m_aPos.X - nLastPos );
            if ( rElement.m_aDockedData.m_aPos.X >= nLastPos )
            {
                rRowColumnsWindowData[nIndex].nSpace += nSpace;
                nLastPos = rElement.m_aDockedData.m_aPos.X + aPosSize.Width;
            }
            else
            {
                nSpace = 0;
                nLastPos += aPosSize.Width;
            }
            rRowColumnsWindowData[nIndex].aRowColumnSpace.push_back( nSpace );

            rRowColumnsWindowData[nIndex].aRowColumnWindows.push_back( xWindow );
            rRowColumnsWindowData[nIndex].aUIElementNames.push_back( rElement.m_aName );
            rRowColumnsWindowData[nIndex].aRowColumnWindowSizes.push_back(
                awt::Rectangle( rElement.m_aDockedData.m_aPos.X,
                                rElement.m_aDockedData.m_aPos.Y,
                                aPosSize.Width,
                                aPosSize.Height ));
            if ( rRowColumnsWindowData[nIndex].nStaticSize < aPosSize.Height )
                rRowColumnsWindowData[nIndex].nStaticSize = aPosSize.Height;
            if ( eDockingArea == ui::DockingArea_DOCKINGAREA_TOP )
                rRowColumnsWindowData[nIndex].aRowColumnRect = awt::Rectangle( 0, nLastRowColPixelPos,
                                                                               aDockAreaRect.Width, aPosSize.Height );
            else
                rRowColumnsWindowData[nIndex].aRowColumnRect = awt::Rectangle( 0, ( nLastRowColPixelPos - aPosSize.Height ),
                                                                               aDockAreaRect.Width, aPosSize.Height );
            rRowColumnsWindowData[nIndex].nVarSize += aPosSize.Width + nSpace;
        }
        else
        {
            if ( nCurrPos == -1 )
            {
                nCurrPos = rElement.m_aDockedData.m_aPos.X;
                nLastPos = 0;

                SingleRowColumnWindowData aRowColumnWindowData;
                aRowColumnWindowData.nRowColumn = nCurrPos;
                rRowColumnsWindowData.push_back( aRowColumnWindowData );
            }

            sal_Int32 nSpace( 0 );
            if ( rElement.m_aDockedData.m_aPos.X != nCurrPos )
            {
                if ( eDockingArea == ui::DockingArea_DOCKINGAREA_LEFT )
                    nLastRowColPixelPos += rRowColumnsWindowData[nIndex].nStaticSize;
                else
                    nLastRowColPixelPos -= rRowColumnsWindowData[nIndex].nStaticSize;
                ++nIndex;
                nLastPos = 0;
                nCurrPos = rElement.m_aDockedData.m_aPos.X;
                SingleRowColumnWindowData aRowColumnWindowData;
                aRowColumnWindowData.nRowColumn = nCurrPos;
                rRowColumnsWindowData.push_back( aRowColumnWindowData );
            }

            // Calc space before an element and store it
            nSpace = ( rElement.m_aDockedData.m_aPos.Y - nLastPos );
            if ( rElement.m_aDockedData.m_aPos.Y > nLastPos )
            {
                rRowColumnsWindowData[nIndex].nSpace += nSpace;
                nLastPos = rElement.m_aDockedData.m_aPos.Y + aPosSize.Height;
            }
            else
            {
                nSpace = 0;
                nLastPos += aPosSize.Height;
            }
            rRowColumnsWindowData[nIndex].aRowColumnSpace.push_back( nSpace );

            rRowColumnsWindowData[nIndex].aRowColumnWindows.push_back( xWindow );
            rRowColumnsWindowData[nIndex].aUIElementNames.push_back( rElement.m_aName );
            rRowColumnsWindowData[nIndex].aRowColumnWindowSizes.push_back(
                awt::Rectangle( rElement.m_aDockedData.m_aPos.X,
                                rElement.m_aDockedData.m_aPos.Y,
                                aPosSize.Width,
                                aPosSize.Height ));
            if ( rRowColumnsWindowData[nIndex].nStaticSize < aPosSize.Width )
                rRowColumnsWindowData[nIndex].nStaticSize = aPosSize.Width;
            if ( eDockingArea == ui::DockingArea_DOCKINGAREA_LEFT )
                rRowColumnsWindowData[nIndex].aRowColumnRect = awt::Rectangle( nLastRowColPixelPos, 0,
                                                                               aPosSize.Width, aDockAreaRect.Height );
            else
                rRowColumnsWindowData[nIndex].aRowColumnRect = awt::Rectangle( ( nLastRowColPixelPos - aPosSize.Width ), 0,
                                                                                 aPosSize.Width, aDockAreaRect.Height );
            rRowColumnsWindowData[nIndex].nVarSize += aPosSize.Height + nSpace;
        }
    }
}

void ToolbarLayoutManager::implts_getDockingAreaElementInfoOnSingleRowCol( ui::DockingArea eDockingArea, sal_Int32 nRowCol, SingleRowColumnWindowData& rRowColumnWindowData )
{
    std::vector< UIElement > aWindowVector;

    if (( eDockingArea < ui::DockingArea_DOCKINGAREA_TOP ) || ( eDockingArea > ui::DockingArea_DOCKINGAREA_RIGHT ))
        eDockingArea = ui::DockingArea_DOCKINGAREA_TOP;

    bool bHorzDockArea = isHorizontalDockingArea( eDockingArea );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    UIElementVector::iterator   pIter;
    UIElementVector::iterator   pEnd = m_aUIElements.end();
    for ( pIter = m_aUIElements.begin(); pIter != pEnd; ++pIter )
    {
        if ( pIter->m_aDockedData.m_nDockedArea == eDockingArea )
        {
            bool bSameRowCol = bHorzDockArea ? ( pIter->m_aDockedData.m_aPos.Y == nRowCol ) : ( pIter->m_aDockedData.m_aPos.X == nRowCol );
            uno::Reference< ui::XUIElement > xUIElement( pIter->m_xUIElement );

            if ( bSameRowCol && xUIElement.is() )
            {
                uno::Reference< awt::XWindow > xWindow( xUIElement->getRealInterface(), uno::UNO_QUERY );
                if ( xWindow.is() )
                {
                    SolarMutexGuard aGuard;
                    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                    uno::Reference< awt::XDockableWindow > xDockWindow( xWindow, uno::UNO_QUERY );
                    if ( pWindow && pIter->m_bVisible && xDockWindow.is() && !pIter->m_bFloating )
                        aWindowVector.push_back( *pIter ); // docked windows
                }
            }
        }
    }
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    // Initialize structure
    rRowColumnWindowData.aUIElementNames.clear();
    rRowColumnWindowData.aRowColumnWindows.clear();
    rRowColumnWindowData.aRowColumnWindowSizes.clear();
    rRowColumnWindowData.aRowColumnSpace.clear();
    rRowColumnWindowData.nVarSize = 0;
    rRowColumnWindowData.nStaticSize = 0;
    rRowColumnWindowData.nSpace = 0;
    rRowColumnWindowData.nRowColumn = nRowCol;

    // Collect data from windows that are on the same row/column
    sal_Int32 j;
    sal_Int32 nLastPos( 0 );

    const sal_uInt32 nCount = aWindowVector.size();
    for ( j = 0; j < sal_Int32( nCount); j++ )
    {
        const UIElement& rElement = aWindowVector[j];
        uno::Reference< awt::XWindow > xWindow;
        uno::Reference< ui::XUIElement > xUIElement( rElement.m_xUIElement );
        awt::Rectangle aPosSize;
        if ( !lcl_checkUIElement(xUIElement,aPosSize,xWindow) )
            continue;

        sal_Int32 nSpace;
        if ( isHorizontalDockingArea( eDockingArea ))
        {
            nSpace = ( rElement.m_aDockedData.m_aPos.X - nLastPos );

            // Calc space before an element and store it
            if ( rElement.m_aDockedData.m_aPos.X > nLastPos )
                rRowColumnWindowData.nSpace += nSpace;
            else
                nSpace = 0;

            nLastPos = rElement.m_aDockedData.m_aPos.X + aPosSize.Width;


            rRowColumnWindowData.aRowColumnWindowSizes.push_back(
                awt::Rectangle( rElement.m_aDockedData.m_aPos.X, rElement.m_aDockedData.m_aPos.Y,
                                aPosSize.Width, aPosSize.Height ));
            if ( rRowColumnWindowData.nStaticSize < aPosSize.Height )
                rRowColumnWindowData.nStaticSize = aPosSize.Height;
            rRowColumnWindowData.nVarSize += aPosSize.Width;
        }
        else
        {
            // Calc space before an element and store it
            nSpace = ( rElement.m_aDockedData.m_aPos.Y - nLastPos );
            if ( rElement.m_aDockedData.m_aPos.Y > nLastPos )
                rRowColumnWindowData.nSpace += nSpace;
            else
                nSpace = 0;

            nLastPos = rElement.m_aDockedData.m_aPos.Y + aPosSize.Height;

            rRowColumnWindowData.aRowColumnWindowSizes.push_back(
                awt::Rectangle( rElement.m_aDockedData.m_aPos.X, rElement.m_aDockedData.m_aPos.Y,
                                aPosSize.Width, aPosSize.Height ));
            if ( rRowColumnWindowData.nStaticSize < aPosSize.Width )
                rRowColumnWindowData.nStaticSize = aPosSize.Width;
            rRowColumnWindowData.nVarSize += aPosSize.Height;
        }

        rRowColumnWindowData.aUIElementNames.push_back( rElement.m_aName );
        rRowColumnWindowData.aRowColumnWindows.push_back( xWindow );
        rRowColumnWindowData.aRowColumnSpace.push_back( nSpace );
        rRowColumnWindowData.nVarSize += nSpace;
    }
}

::Rectangle ToolbarLayoutManager::implts_getWindowRectFromRowColumn(
    ui::DockingArea DockingArea,
    const SingleRowColumnWindowData& rRowColumnWindowData,
    const ::Point& rMousePos,
    const rtl::OUString& rExcludeElementName )
{
    ::Rectangle aWinRect;

    if (( DockingArea < ui::DockingArea_DOCKINGAREA_TOP ) || ( DockingArea > ui::DockingArea_DOCKINGAREA_RIGHT ))
        DockingArea = ui::DockingArea_DOCKINGAREA_TOP;

    if ( rRowColumnWindowData.aRowColumnWindows.empty() )
        return aWinRect;
    else
    {
        ReadGuard aReadLock( m_aLock );
        Window* pContainerWindow( VCLUnoHelper::GetWindow( m_xContainerWindow ));
        Window* pDockingAreaWindow( VCLUnoHelper::GetWindow( m_xDockAreaWindows[DockingArea] ));
        aReadLock.unlock();

        // Calc correct position of the column/row rectangle to be able to compare it with mouse pos/tracking rect
        SolarMutexGuard aGuard;

        // Retrieve output size from container Window
        if ( pDockingAreaWindow && pContainerWindow )
        {
            const sal_uInt32 nCount = rRowColumnWindowData.aRowColumnWindows.size();
            for ( sal_uInt32 i = 0; i < nCount; i++ )
            {
                awt::Rectangle aWindowRect = rRowColumnWindowData.aRowColumnWindows[i]->getPosSize();
                ::Rectangle aRect( aWindowRect.X, aWindowRect.Y, aWindowRect.X+aWindowRect.Width, aWindowRect.Y+aWindowRect.Height );
                aRect.SetPos( pContainerWindow->ScreenToOutputPixel( pDockingAreaWindow->OutputToScreenPixel( aRect.TopLeft() )));
                if ( aRect.IsInside( rMousePos ))
                {
                    // Check if we have found the excluded element. If yes, we have to provide an empty rectangle.
                    // We prevent that a toolbar cannot be moved when the mouse pointer is inside its own rectangle!
                    if ( rExcludeElementName != rRowColumnWindowData.aUIElementNames[i] )
                        return aRect;
                    else
                        break;
                }
            }
        }
    }

    return aWinRect;
}

::Rectangle ToolbarLayoutManager::implts_determineFrontDockingRect(
    ui::DockingArea        eDockingArea,
    sal_Int32              nRowCol,
    const ::Rectangle&     rDockedElementRect,
    const ::rtl::OUString& rMovedElementName,
    const ::Rectangle&     rMovedElementRect )
{
    SingleRowColumnWindowData aRowColumnWindowData;

    sal_Bool bHorzDockArea( isHorizontalDockingArea( eDockingArea ));
    implts_getDockingAreaElementInfoOnSingleRowCol( eDockingArea, nRowCol, aRowColumnWindowData );
    if ( aRowColumnWindowData.aRowColumnWindows.empty() )
        return rMovedElementRect;
    else
    {
        sal_Int32 nSpace( 0 );
        ::Rectangle aFrontDockingRect( rMovedElementRect );
        const sal_uInt32 nCount = aRowColumnWindowData.aRowColumnWindows.size();
        for ( sal_uInt32 i = 0; i < nCount; i++ )
        {
            if ( bHorzDockArea )
            {
                if ( aRowColumnWindowData.aRowColumnWindowSizes[i].X >= rDockedElementRect.Left() )
                {
                    nSpace += aRowColumnWindowData.aRowColumnSpace[i];
                    break;
                }
                else if ( aRowColumnWindowData.aUIElementNames[i] == rMovedElementName )
                    nSpace += aRowColumnWindowData.aRowColumnWindowSizes[i].Width +
                              aRowColumnWindowData.aRowColumnSpace[i];
                else
                    nSpace = 0;
            }
            else
            {
                if ( aRowColumnWindowData.aRowColumnWindowSizes[i].Y >= rDockedElementRect.Top() )
                {
                    nSpace += aRowColumnWindowData.aRowColumnSpace[i];
                    break;
                }
                else if ( aRowColumnWindowData.aUIElementNames[i] == rMovedElementName )
                    nSpace += aRowColumnWindowData.aRowColumnWindowSizes[i].Height +
                              aRowColumnWindowData.aRowColumnSpace[i];
                else
                    nSpace = 0;
            }
        }

        if ( nSpace > 0 )
        {
            sal_Int32 nMove = std::min( nSpace, static_cast<sal_Int32>(aFrontDockingRect.getWidth()) );
            if ( bHorzDockArea )
                aFrontDockingRect.Move( -nMove, 0 );
            else
                aFrontDockingRect.Move( 0, -nMove );
        }

        return aFrontDockingRect;
    }
}

void ToolbarLayoutManager::implts_findNextDockingPos( ui::DockingArea DockingArea, const ::Size& aUIElementSize, awt::Point& rVirtualPos, ::Point& rPixelPos )
{
    ReadGuard aReadLock( m_aLock );
    uno::Reference< awt::XWindow > xDockingWindow( m_xDockAreaWindows[DockingArea] );
    ::Size                         aDockingWinSize;
    Window*                        pDockingWindow( 0 );
    aReadLock.unlock();

    if (( DockingArea < ui::DockingArea_DOCKINGAREA_TOP ) || ( DockingArea > ui::DockingArea_DOCKINGAREA_RIGHT ))
        DockingArea = ui::DockingArea_DOCKINGAREA_TOP;

    {
        // Retrieve output size from container Window
        SolarMutexGuard aGuard;
        pDockingWindow  = VCLUnoHelper::GetWindow( xDockingWindow );
        if ( pDockingWindow )
            aDockingWinSize = pDockingWindow->GetOutputSizePixel();
    }

    sal_Int32 nFreeRowColPixelPos( 0 );
    sal_Int32 nMaxSpace( 0 );
    sal_Int32 nNeededSpace( 0 );
    sal_Int32 nTopDockingAreaSize( 0 );

    if ( isHorizontalDockingArea( DockingArea ))
    {
        nMaxSpace    = aDockingWinSize.Width();
        nNeededSpace = aUIElementSize.Width();
    }
    else
    {
        nMaxSpace           = aDockingWinSize.Height();
        nNeededSpace        = aUIElementSize.Height();
        nTopDockingAreaSize = implts_getTopBottomDockingAreaSizes().Width();
    }

    std::vector< SingleRowColumnWindowData > aRowColumnsWindowData;

    implts_getDockingAreaElementInfos( DockingArea, aRowColumnsWindowData );
    sal_Int32 nPixelPos( 0 );
    const sal_uInt32 nCount = aRowColumnsWindowData.size();
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        SingleRowColumnWindowData& rRowColumnWindowData = aRowColumnsWindowData[i];

        if (( DockingArea == ui::DockingArea_DOCKINGAREA_BOTTOM ) ||
            ( DockingArea == ui::DockingArea_DOCKINGAREA_RIGHT  ))
            nPixelPos += rRowColumnWindowData.nStaticSize;

        if ((( nMaxSpace - rRowColumnWindowData.nVarSize ) >= nNeededSpace ) ||
            ( rRowColumnWindowData.nSpace >= nNeededSpace ))
        {
            // Check current row where we can find the needed space
            sal_Int32 nCurrPos( 0 );
            const sal_uInt32 nWindowSizesCount = rRowColumnWindowData.aRowColumnWindowSizes.size();
            for ( sal_uInt32 j = 0; j < nWindowSizesCount; j++ )
            {
                awt::Rectangle rRect  = rRowColumnWindowData.aRowColumnWindowSizes[j];
                sal_Int32&     rSpace = rRowColumnWindowData.aRowColumnSpace[j];
                if ( isHorizontalDockingArea( DockingArea ))
                {
                    if ( rSpace >= nNeededSpace )
                    {
                        rVirtualPos = awt::Point( nCurrPos, rRowColumnWindowData.nRowColumn );
                        if ( DockingArea == ui::DockingArea_DOCKINGAREA_TOP )
                            rPixelPos   = ::Point( nCurrPos, nPixelPos );
                        else
                            rPixelPos   = ::Point( nCurrPos, aDockingWinSize.Height() - nPixelPos );
                        return;
                    }
                    nCurrPos = rRect.X + rRect.Width;
                }
                else
                {
                    if ( rSpace >= nNeededSpace )
                    {
                        rVirtualPos = awt::Point( rRowColumnWindowData.nRowColumn, nCurrPos );
                        if ( DockingArea == ui::DockingArea_DOCKINGAREA_LEFT )
                            rPixelPos   = ::Point( nPixelPos, nTopDockingAreaSize + nCurrPos );
                        else
                            rPixelPos   = ::Point( aDockingWinSize.Width() - nPixelPos , nTopDockingAreaSize + nCurrPos );
                        return;
                    }
                    nCurrPos = rRect.Y + rRect.Height;
                }
            }

            if (( nCurrPos + nNeededSpace ) <= nMaxSpace )
            {
                if ( isHorizontalDockingArea( DockingArea ))
                {
                    rVirtualPos = awt::Point( nCurrPos, rRowColumnWindowData.nRowColumn );
                    if ( DockingArea == ui::DockingArea_DOCKINGAREA_TOP )
                        rPixelPos   = ::Point( nCurrPos, nPixelPos );
                    else
                        rPixelPos   = ::Point( nCurrPos, aDockingWinSize.Height() - nPixelPos );
                    return;
                }
                else
                {
                    rVirtualPos = awt::Point( rRowColumnWindowData.nRowColumn, nCurrPos );
                    if ( DockingArea == ui::DockingArea_DOCKINGAREA_LEFT )
                        rPixelPos   = ::Point( nPixelPos, nTopDockingAreaSize + nCurrPos );
                    else
                        rPixelPos   = ::Point( aDockingWinSize.Width() - nPixelPos , nTopDockingAreaSize + nCurrPos );
                    return;
                }
            }
        }

        if (( DockingArea == ui::DockingArea_DOCKINGAREA_TOP ) || ( DockingArea == ui::DockingArea_DOCKINGAREA_LEFT  ))
            nPixelPos += rRowColumnWindowData.nStaticSize;
    }

    sal_Int32 nNextFreeRowCol( 0 );
    sal_Int32 nRowColumnsCount = aRowColumnsWindowData.size();
    if ( nRowColumnsCount > 0 )
        nNextFreeRowCol = aRowColumnsWindowData[nRowColumnsCount-1].nRowColumn+1;
    else
        nNextFreeRowCol = 0;

    if ( nNextFreeRowCol == 0 )
    {
        if ( DockingArea == ui::DockingArea_DOCKINGAREA_BOTTOM )
            nFreeRowColPixelPos = aDockingWinSize.Height() - aUIElementSize.Height();
        else if ( DockingArea == ui::DockingArea_DOCKINGAREA_RIGHT  )
            nFreeRowColPixelPos = aDockingWinSize.Width() - aUIElementSize.Width();
    }

    if ( isHorizontalDockingArea( DockingArea ))
    {
        rVirtualPos = awt::Point( 0, nNextFreeRowCol );
        if ( DockingArea == ui::DockingArea_DOCKINGAREA_TOP )
            rPixelPos = ::Point( 0, nFreeRowColPixelPos );
        else
            rPixelPos = ::Point( 0, aDockingWinSize.Height() - nFreeRowColPixelPos );
    }
    else
    {
        rVirtualPos = awt::Point( nNextFreeRowCol, 0 );
        rPixelPos   = ::Point( aDockingWinSize.Width() - nFreeRowColPixelPos, 0 );
    }
}

void ToolbarLayoutManager::implts_calcWindowPosSizeOnSingleRowColumn(
    sal_Int32 nDockingArea,
    sal_Int32 nOffset,
    SingleRowColumnWindowData& rRowColumnWindowData,
    const ::Size& rContainerSize )
{
    sal_Int32 nDiff(0);
    sal_Int32 nRCSpace( rRowColumnWindowData.nSpace );
    sal_Int32 nTopDockingAreaSize(0);
    sal_Int32 nBottomDockingAreaSize(0);
    sal_Int32 nContainerClientSize(0);

    if ( rRowColumnWindowData.aRowColumnWindows.empty() )
        return;

    if ( isHorizontalDockingArea( nDockingArea ))
    {
        nContainerClientSize = rContainerSize.Width();
        nDiff = nContainerClientSize - rRowColumnWindowData.nVarSize;
    }
    else
    {
        nTopDockingAreaSize    = implts_getTopBottomDockingAreaSizes().Width();
        nBottomDockingAreaSize = implts_getTopBottomDockingAreaSizes().Height();
        nContainerClientSize   = ( rContainerSize.Height() - nTopDockingAreaSize - nBottomDockingAreaSize );
        nDiff = nContainerClientSize - rRowColumnWindowData.nVarSize;
    }

    const sal_uInt32 nCount = rRowColumnWindowData.aRowColumnWindowSizes.size();
    if (( nDiff < 0 ) && ( nRCSpace > 0 ))
    {
        // First we try to reduce the size of blank space before/behind docked windows
        sal_Int32 i = nCount - 1;
        while ( i >= 0 )
        {
            sal_Int32 nSpace = rRowColumnWindowData.aRowColumnSpace[i];
            if ( nSpace >= -nDiff )
            {
                if ( isHorizontalDockingArea( nDockingArea ))
                {
                    // Try to move this and all user elements behind with the calculated difference
                    for ( sal_uInt32 j = i; j < nCount ; j++ )
                        rRowColumnWindowData.aRowColumnWindowSizes[j].X += nDiff;
                }
                else
                {
                    // Try to move this and all user elements behind with the calculated difference
                    for ( sal_uInt32 j = i; j < nCount ; j++ )
                        rRowColumnWindowData.aRowColumnWindowSizes[j].Y += nDiff;
                }
                nDiff = 0;

                break;
            }
            else if ( nSpace > 0 )
            {
                if ( isHorizontalDockingArea( nDockingArea ))
                {
                    // Try to move this and all user elements behind with the calculated difference
                    for ( sal_uInt32 j = i; j < nCount; j++ )
                        rRowColumnWindowData.aRowColumnWindowSizes[j].X -= nSpace;
                }
                else
                {
                    // Try to move this and all user elements behind with the calculated difference
                    for ( sal_uInt32 j = i; j < nCount; j++ )
                        rRowColumnWindowData.aRowColumnWindowSizes[j].Y -= nSpace;
                }
                nDiff += nSpace;
            }
            --i;
        }
    }

    // Check if we have to reduce further
    if ( nDiff < 0 )
    {
        // Now we have to reduce the size of certain docked windows
        sal_Int32 i = sal_Int32( nCount - 1 );
        while ( i >= 0 )
        {
            awt::Rectangle& rWinRect = rRowColumnWindowData.aRowColumnWindowSizes[i];
            ::Size          aMinSize;

            SolarMutexGuard aGuard;
            {
                uno::Reference< awt::XWindow > xWindow = rRowColumnWindowData.aRowColumnWindows[i];
                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
                    aMinSize = ((ToolBox *)pWindow)->CalcMinimumWindowSizePixel();
            }

            if (( aMinSize.Width() > 0 ) && ( aMinSize.Height() > 0 ))
            {
                if ( isHorizontalDockingArea( nDockingArea ))
                {
                    sal_Int32 nMaxReducation = ( rWinRect.Width - aMinSize.Width() );
                    if ( nMaxReducation >= -nDiff )
                    {
                        rWinRect.Width = rWinRect.Width + nDiff;
                        nDiff = 0;
                    }
                    else
                    {
                        rWinRect.Width = aMinSize.Width();
                        nDiff += nMaxReducation;
                    }

                    // Try to move this and all user elements behind with the calculated difference
                    for ( sal_uInt32 j = i; j < nCount; j++ )
                        rRowColumnWindowData.aRowColumnWindowSizes[j].X += nDiff;
                }
                else
                {
                    sal_Int32 nMaxReducation = ( rWinRect.Height - aMinSize.Height() );
                    if ( nMaxReducation >= -nDiff )
                    {
                        rWinRect.Height = rWinRect.Height + nDiff;
                        nDiff = 0;
                    }
                    else
                    {
                        rWinRect.Height = aMinSize.Height();
                        nDiff += nMaxReducation;
                    }

                    // Try to move this and all user elements behind with the calculated difference
                    for ( sal_uInt32 j = i; j < nCount; j++ )
                        rRowColumnWindowData.aRowColumnWindowSizes[j].Y += nDiff;
                }
            }

            if ( nDiff >= 0 )
                break;

            --i;
        }
    }

    ReadGuard aReadLock( m_aLock );
    Window* pDockAreaWindow = VCLUnoHelper::GetWindow( m_xDockAreaWindows[nDockingArea] );
    aReadLock.unlock();

    sal_Int32 nCurrPos( 0 );

    SolarMutexGuard aGuard;
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        uno::Reference< awt::XWindow > xWindow = rRowColumnWindowData.aRowColumnWindows[i];
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        Window* pOldParentWindow = pWindow->GetParent();

        if ( pDockAreaWindow != pOldParentWindow )
            pWindow->SetParent( pDockAreaWindow );

        awt::Rectangle aWinRect = rRowColumnWindowData.aRowColumnWindowSizes[i];
        if ( isHorizontalDockingArea( nDockingArea ))
        {
            if ( aWinRect.X < nCurrPos )
                aWinRect.X = nCurrPos;
            pWindow->SetPosSizePixel( ::Point( aWinRect.X, nOffset ), ::Size( aWinRect.Width, rRowColumnWindowData.nStaticSize ));
            pWindow->Show( sal_True, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
            nCurrPos += ( aWinRect.X - nCurrPos ) + aWinRect.Width;
        }
        else
        {
            if ( aWinRect.Y < nCurrPos )
                aWinRect.Y = nCurrPos;
            pWindow->SetPosSizePixel( ::Point( nOffset, aWinRect.Y ), ::Size( rRowColumnWindowData.nStaticSize, aWinRect.Height ));
            pWindow->Show( sal_True, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
            nCurrPos += ( aWinRect.Y - nCurrPos ) + aWinRect.Height;
        }
    }
}

void ToolbarLayoutManager::implts_setLayoutDirty()
{
    WriteGuard aWriteLock( m_aLock );
    m_bLayoutDirty = true;
}

void ToolbarLayoutManager::implts_setLayoutInProgress( bool bInProgress )
{
    WriteGuard aWriteLock( m_aLock );
    m_bLayoutInProgress = bInProgress;
}

::Rectangle ToolbarLayoutManager::implts_calcHotZoneRect( const ::Rectangle& rRect, sal_Int32 nHotZoneOffset )
{
    ::Rectangle aRect( rRect );

    aRect.Left() -= nHotZoneOffset;
    aRect.Top() -= nHotZoneOffset;
    aRect.Right() += nHotZoneOffset;
    aRect.Bottom() += nHotZoneOffset;

    return aRect;
}

void ToolbarLayoutManager::implts_calcDockingPosSize(
    UIElement&          rUIElement,
    DockingOperation&   rDockingOperation,
    ::Rectangle&        rTrackingRect,
    const Point&        rMousePos )
{
    ReadGuard aReadLock( m_aLock );
    uno::Reference< awt::XWindow2 > xContainerWindow( m_xContainerWindow );
    ::Size                          aContainerWinSize;
    Window*                         pContainerWindow( 0 );
    ::Rectangle                     aDockingAreaOffsets( m_aDockingAreaOffsets );
    aReadLock.unlock();

    if ( !rUIElement.m_xUIElement.is() )
    {
        rTrackingRect = ::Rectangle();
        return;
    }

    {
        // Retrieve output size from container Window
        SolarMutexGuard aGuard;
        pContainerWindow  = VCLUnoHelper::GetWindow( xContainerWindow );
        aContainerWinSize = pContainerWindow->GetOutputSizePixel();
    }

    Window*                        pDockWindow( 0 );
    Window*                        pDockingAreaWindow( 0 );
    ToolBox*                       pToolBox( 0 );
    uno::Reference< awt::XWindow > xWindow( rUIElement.m_xUIElement->getRealInterface(), uno::UNO_QUERY );
    uno::Reference< awt::XWindow > xDockingAreaWindow;
    ::Rectangle                    aTrackingRect( rTrackingRect );
    ui::DockingArea                eDockedArea( (ui::DockingArea)rUIElement.m_aDockedData.m_nDockedArea );
    sal_Int32                      nTopDockingAreaSize( implts_getTopBottomDockingAreaSizes().Width() );
    sal_Int32                      nBottomDockingAreaSize( implts_getTopBottomDockingAreaSizes().Height() );
    bool                           bHorizontalDockArea(( eDockedArea == ui::DockingArea_DOCKINGAREA_TOP ) ||
                                                       ( eDockedArea == ui::DockingArea_DOCKINGAREA_BOTTOM ));
    sal_Int32                      nMaxLeftRightDockAreaSize = aContainerWinSize.Height() -
                                                               nTopDockingAreaSize -
                                                               nBottomDockingAreaSize -
                                                               aDockingAreaOffsets.Top() -
                                                               aDockingAreaOffsets.Bottom();
    ::Rectangle                    aDockingAreaRect;

    aReadLock.lock();
    xDockingAreaWindow = m_xDockAreaWindows[eDockedArea];
    aReadLock.unlock();

    {
        SolarMutexGuard aGuard;
        pDockingAreaWindow = VCLUnoHelper::GetWindow( xDockingAreaWindow );
        pDockWindow        = VCLUnoHelper::GetWindow( xWindow );
        if ( pDockWindow && pDockWindow->GetType() == WINDOW_TOOLBOX )
            pToolBox = (ToolBox *)pDockWindow;

        aDockingAreaRect = ::Rectangle( pDockingAreaWindow->GetPosPixel(), pDockingAreaWindow->GetSizePixel() );
        if ( pToolBox )
        {
            // docked toolbars always have one line
            ::Size aSize = pToolBox->CalcWindowSizePixel( 1, ImplConvertAlignment( sal_Int16( eDockedArea )) );
            aTrackingRect.SetSize( ::Size( aSize.Width(), aSize.Height() ));
        }
    }

    // default docking operation, dock on the given row/column
    bool                                     bOpOutsideOfDockingArea( !aDockingAreaRect.IsInside( rMousePos ));

    std::vector< SingleRowColumnWindowData > aRowColumnsWindowData;

    rDockingOperation = DOCKOP_ON_COLROW;
    implts_getDockingAreaElementInfos( eDockedArea, aRowColumnsWindowData );

    // determine current first row/column and last row/column
    sal_Int32 nMaxRowCol( -1 );
    sal_Int32 nMinRowCol( SAL_MAX_INT32 );
    const sal_uInt32 nCount = aRowColumnsWindowData.size();
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        if ( aRowColumnsWindowData[i].nRowColumn > nMaxRowCol )
            nMaxRowCol = aRowColumnsWindowData[i].nRowColumn;
        if ( aRowColumnsWindowData[i].nRowColumn < nMinRowCol )
            nMinRowCol = aRowColumnsWindowData[i].nRowColumn;
    }

    if ( !bOpOutsideOfDockingArea )
    {
        // docking inside our docking area
        sal_Int32   nIndex( -1 );
        sal_Int32   nRowCol( -1 );
        ::Rectangle aWindowRect;
        ::Rectangle aRowColumnRect;

        const sal_uInt32 nWindowDataCount = aRowColumnsWindowData.size();
        for ( sal_uInt32 i = 0; i < nWindowDataCount; i++ )
        {
            ::Rectangle aRect( aRowColumnsWindowData[i].aRowColumnRect.X,
                               aRowColumnsWindowData[i].aRowColumnRect.Y,
                               aRowColumnsWindowData[i].aRowColumnRect.X + aRowColumnsWindowData[i].aRowColumnRect.Width,
                               aRowColumnsWindowData[i].aRowColumnRect.Y + aRowColumnsWindowData[i].aRowColumnRect.Height );

            {
                // Calc correct position of the column/row rectangle to be able to compare it with mouse pos/tracking rect
                SolarMutexGuard aGuard;
                aRect.SetPos( pContainerWindow->ScreenToOutputPixel( pDockingAreaWindow->OutputToScreenPixel( aRect.TopLeft() )));
            }

            bool bIsInsideRowCol( aRect.IsInside( rMousePos ) );
            if ( bIsInsideRowCol )
            {
                nIndex            = i;
                nRowCol           = aRowColumnsWindowData[i].nRowColumn;
                rDockingOperation = implts_determineDockingOperation( eDockedArea, aRect, rMousePos );
                aWindowRect       = implts_getWindowRectFromRowColumn( eDockedArea, aRowColumnsWindowData[i], rMousePos, rUIElement.m_aName );
                aRowColumnRect    = aRect;
                break;
            }
        }

        OSL_ENSURE( ( nIndex >= 0 ) && ( nRowCol >= 0 ), "Impossible case - no row/column found but mouse pointer is inside our docking area" );
        if (( nIndex >= 0 ) && ( nRowCol >= 0 ))
        {
            if ( rDockingOperation == DOCKOP_ON_COLROW )
            {
                if ( !aWindowRect.IsEmpty())
                {
                    // Tracking rect is on a row/column and mouse is over a docked toolbar.
                    // Determine if the tracking rect must be located before/after the docked toolbar.

                    ::Rectangle aUIElementRect( aWindowRect );
                    sal_Int32   nMiddle( bHorizontalDockArea ? ( aWindowRect.Left() + aWindowRect.getWidth() / 2 ) :
                                                               ( aWindowRect.Top() + aWindowRect.getHeight() / 2 ));
                    sal_Bool    bInsertBefore( bHorizontalDockArea ? ( rMousePos.X() < nMiddle ) : ( rMousePos.Y() < nMiddle ));
                    if ( bInsertBefore )
                    {
                        if ( bHorizontalDockArea )
                        {
                            sal_Int32 nSize = ::std::max( sal_Int32( 0 ), std::min( sal_Int32( aContainerWinSize.Width() -  aWindowRect.Left() ),
                                                                                    sal_Int32( aTrackingRect.getWidth() )));
                            if ( nSize == 0 )
                                nSize = aWindowRect.getWidth();

                            aUIElementRect.SetSize( ::Size( nSize, aWindowRect.getHeight() ));
                            aWindowRect = implts_determineFrontDockingRect( eDockedArea, nRowCol, aWindowRect,rUIElement.m_aName, aUIElementRect );

                            // Set virtual position
                            rUIElement.m_aDockedData.m_aPos.X = aWindowRect.Left();
                            rUIElement.m_aDockedData.m_aPos.Y = nRowCol;
                        }
                        else
                        {
                            sal_Int32 nSize = ::std::max( sal_Int32( 0 ), std::min( sal_Int32(
                                                    nTopDockingAreaSize + nMaxLeftRightDockAreaSize - aWindowRect.Top() ),
                                                    sal_Int32( aTrackingRect.getHeight() )));
                            if ( nSize == 0 )
                                nSize = aWindowRect.getHeight();

                            aUIElementRect.SetSize( ::Size( aWindowRect.getWidth(), nSize ));
                            aWindowRect = implts_determineFrontDockingRect( eDockedArea, nRowCol, aWindowRect, rUIElement.m_aName, aUIElementRect );

                            // Set virtual position
                            sal_Int32 nPosY = pDockingAreaWindow->ScreenToOutputPixel(
                                                pContainerWindow->OutputToScreenPixel( aWindowRect.TopLeft() )).Y();
                            rUIElement.m_aDockedData.m_aPos.X = nRowCol;
                            rUIElement.m_aDockedData.m_aPos.Y = nPosY;
                        }

                        rTrackingRect = aWindowRect;
                        return;
                    }
                    else
                    {
                        if ( bHorizontalDockArea )
                        {
                            sal_Int32 nSize = ::std::max( sal_Int32( 0 ), std::min( sal_Int32(( aContainerWinSize.Width() ) - aWindowRect.Right() ),
                                                                                    sal_Int32( aTrackingRect.getWidth() )));
                            if ( nSize == 0 )
                            {
                                aUIElementRect.SetPos( ::Point( aContainerWinSize.Width() - aTrackingRect.getWidth(), aWindowRect.Top() ));
                                aUIElementRect.SetSize( ::Size( aTrackingRect.getWidth(), aWindowRect.getHeight() ));
                                rUIElement.m_aDockedData.m_aPos.X = aUIElementRect.Left();

                            }
                            else
                            {
                                aUIElementRect.SetPos( ::Point( aWindowRect.Right(), aWindowRect.Top() ));
                                aUIElementRect.SetSize( ::Size( nSize, aWindowRect.getHeight() ));
                                rUIElement.m_aDockedData.m_aPos.X = aWindowRect.Right();
                            }

                            // Set virtual position
                            rUIElement.m_aDockedData.m_aPos.Y = nRowCol;
                        }
                        else
                        {
                            sal_Int32 nSize = ::std::max( sal_Int32( 0 ), std::min( sal_Int32( nTopDockingAreaSize + nMaxLeftRightDockAreaSize - aWindowRect.Bottom() ),
                                                                                    sal_Int32( aTrackingRect.getHeight() )));
                            aUIElementRect.SetPos( ::Point( aWindowRect.Left(), aWindowRect.Bottom() ));
                            aUIElementRect.SetSize( ::Size( aWindowRect.getWidth(), nSize ));

                            // Set virtual position
                            sal_Int32 nPosY( 0 );
                            {
                                SolarMutexGuard aGuard;
                                nPosY = pDockingAreaWindow->ScreenToOutputPixel(
                                                    pContainerWindow->OutputToScreenPixel( aWindowRect.BottomRight() )).Y();
                            }
                            rUIElement.m_aDockedData.m_aPos.X = nRowCol;
                            rUIElement.m_aDockedData.m_aPos.Y = nPosY;
                        }

                        rTrackingRect = aUIElementRect;
                        return;
                    }
                }
                else
                {
                    implts_setTrackingRect( eDockedArea, rMousePos, aTrackingRect );
                    rTrackingRect = implts_calcTrackingAndElementRect(
                                        eDockedArea, nRowCol, rUIElement,
                                        aTrackingRect, aRowColumnRect, aContainerWinSize );
                    return;
                }
            }
            else
            {
                if ((( nRowCol == nMinRowCol ) && ( rDockingOperation == DOCKOP_BEFORE_COLROW )) ||
                    (( nRowCol == nMaxRowCol ) && ( rDockingOperation == DOCKOP_AFTER_COLROW  )))
                    bOpOutsideOfDockingArea = true;
                else
                {
                    // handle docking before/after a row
                    implts_setTrackingRect( eDockedArea, rMousePos, aTrackingRect );
                    rTrackingRect = implts_calcTrackingAndElementRect(
                                        eDockedArea, nRowCol, rUIElement,
                                        aTrackingRect, aRowColumnRect, aContainerWinSize );

                    sal_Int32 nOffsetX( 0 );
                    sal_Int32 nOffsetY( 0 );
                    if ( bHorizontalDockArea )
                        nOffsetY = sal_Int32( floor( aRowColumnRect.getHeight() / 2 + 0.5 ));
                    else
                        nOffsetX = sal_Int32( floor( aRowColumnRect.getWidth() / 2 + 0.5 ));

                    if ( rDockingOperation == DOCKOP_BEFORE_COLROW )
                    {
                        if (( eDockedArea == ui::DockingArea_DOCKINGAREA_TOP ) || ( eDockedArea == ui::DockingArea_DOCKINGAREA_LEFT ))
                        {
                            // Docking before/after means move track rectangle half column/row.
                            // As left and top are ordered 0...n instead of right and bottom
                            // which uses n...0, we have to use negative values for top/left.
                            nOffsetX *= -1;
                            nOffsetY *= -1;
                        }
                    }
                    else
                    {
                        if (( eDockedArea == ui::DockingArea_DOCKINGAREA_BOTTOM ) || ( eDockedArea == ui::DockingArea_DOCKINGAREA_RIGHT ))
                        {
                            // Docking before/after means move track rectangle half column/row.
                            // As left and top are ordered 0...n instead of right and bottom
                            // which uses n...0, we have to use negative values for top/left.
                            nOffsetX *= -1;
                            nOffsetY *= -1;
                        }
                        nRowCol++;
                    }

                    if ( bHorizontalDockArea )
                        rUIElement.m_aDockedData.m_aPos.Y = nRowCol;
                    else
                        rUIElement.m_aDockedData.m_aPos.X = nRowCol;

                    rTrackingRect.Move( nOffsetX, nOffsetY );
                    rTrackingRect.SetSize( aTrackingRect.GetSize() );
                }
            }
        }
    }

    // Docking outside of our docking window area =>
    // Users want to dock before/after first/last docked element or to an empty docking area
    if ( bOpOutsideOfDockingArea )
    {
        // set correct size for docking
        implts_setTrackingRect( eDockedArea, rMousePos, aTrackingRect );
        rTrackingRect = aTrackingRect;

        if ( bHorizontalDockArea )
        {
            sal_Int32 nPosX( std::max( sal_Int32( rTrackingRect.Left()), sal_Int32( 0 )));
            if (( nPosX + rTrackingRect.getWidth()) > aContainerWinSize.Width() )
                nPosX = std::min( nPosX,
                                std::max( sal_Int32( aContainerWinSize.Width() - rTrackingRect.getWidth() ),
                                          sal_Int32( 0 )));

            sal_Int32 nSize = std::min( aContainerWinSize.Width(), rTrackingRect.getWidth() );
            sal_Int32 nDockHeight = std::max( static_cast<sal_Int32>(aDockingAreaRect.getHeight()), sal_Int32( 0 ));
            if ( nDockHeight == 0 )
            {
                sal_Int32 nPosY( std::max( aDockingAreaRect.Top(), aDockingAreaRect.Bottom() ));
                if ( eDockedArea == ui::DockingArea_DOCKINGAREA_BOTTOM )
                    nPosY -= rTrackingRect.getHeight();
                rTrackingRect.SetPos( Point( nPosX, nPosY ));
                rUIElement.m_aDockedData.m_aPos.Y = 0;
            }
            else if ( rMousePos.Y() < ( aDockingAreaRect.Top() + ( nDockHeight / 2 )))
            {
                rTrackingRect.SetPos( Point( nPosX, aDockingAreaRect.Top() - rTrackingRect.getHeight() ));
                if ( eDockedArea == ui::DockingArea_DOCKINGAREA_TOP )
                    rUIElement.m_aDockedData.m_aPos.Y = 0;
                else
                    rUIElement.m_aDockedData.m_aPos.Y = ( nMaxRowCol >= 0 ) ? nMaxRowCol+1 : 0;
                rDockingOperation = DOCKOP_BEFORE_COLROW;
            }
            else
            {
                rTrackingRect.SetPos( Point( nPosX, aDockingAreaRect.Bottom() ));
                if ( eDockedArea == ui::DockingArea_DOCKINGAREA_TOP )
                    rUIElement.m_aDockedData.m_aPos.Y = ( nMaxRowCol >= 0 ) ? nMaxRowCol+1 : 0;
                else
                    rUIElement.m_aDockedData.m_aPos.Y = 0;
                rDockingOperation = DOCKOP_AFTER_COLROW;
            }
            rTrackingRect.setWidth( nSize );

            {
                SolarMutexGuard aGuard;
                nPosX = pDockingAreaWindow->ScreenToOutputPixel(
                                    pContainerWindow->OutputToScreenPixel( rTrackingRect.TopLeft() )).X();
            }
            rUIElement.m_aDockedData.m_aPos.X = nPosX;
        }
        else
        {
            sal_Int32 nMaxDockingAreaHeight = std::max( sal_Int32( 0 ), sal_Int32( nMaxLeftRightDockAreaSize ));
            sal_Int32 nPosY( std::max( sal_Int32( aTrackingRect.Top()), sal_Int32( nTopDockingAreaSize )));
            if (( nPosY + aTrackingRect.getHeight()) > ( nTopDockingAreaSize + nMaxDockingAreaHeight ))
                nPosY = std::min( nPosY,
                                std::max( sal_Int32( nTopDockingAreaSize + ( nMaxDockingAreaHeight - aTrackingRect.getHeight() )),
                                        sal_Int32( nTopDockingAreaSize )));

            sal_Int32 nSize = std::min( nMaxDockingAreaHeight, static_cast<sal_Int32>(aTrackingRect.getHeight()) );
            sal_Int32 nDockWidth = std::max( static_cast<sal_Int32>(aDockingAreaRect.getWidth()), sal_Int32( 0 ));
            if ( nDockWidth == 0 )
            {
                sal_Int32 nPosX( std::max( aDockingAreaRect.Left(), aDockingAreaRect.Right() ));
                if ( eDockedArea == ui::DockingArea_DOCKINGAREA_RIGHT )
                    nPosX -= rTrackingRect.getWidth();
                rTrackingRect.SetPos( Point( nPosX, nPosY ));
                rUIElement.m_aDockedData.m_aPos.X = 0;
            }
            else if ( rMousePos.X() < ( aDockingAreaRect.Left() + ( nDockWidth / 2 )))
            {
                rTrackingRect.SetPos( Point( aDockingAreaRect.Left() - rTrackingRect.getWidth(), nPosY ));
                if ( eDockedArea == ui::DockingArea_DOCKINGAREA_LEFT )
                    rUIElement.m_aDockedData.m_aPos.X = 0;
                else
                    rUIElement.m_aDockedData.m_aPos.X = ( nMaxRowCol >= 0 ) ? nMaxRowCol+1 : 0;
                rDockingOperation = DOCKOP_BEFORE_COLROW;
            }
            else
            {
                rTrackingRect.SetPos( Point( aDockingAreaRect.Right(), nPosY ));
                if ( eDockedArea == ui::DockingArea_DOCKINGAREA_LEFT )
                    rUIElement.m_aDockedData.m_aPos.X = ( nMaxRowCol >= 0 ) ? nMaxRowCol+1 : 0;
                else
                    rUIElement.m_aDockedData.m_aPos.X = 0;
                rDockingOperation = DOCKOP_AFTER_COLROW;
            }
            rTrackingRect.setHeight( nSize );

            {
                SolarMutexGuard aGuard;
                nPosY = pDockingAreaWindow->ScreenToOutputPixel(
                                    pContainerWindow->OutputToScreenPixel( rTrackingRect.TopLeft() )).Y();
            }
            rUIElement.m_aDockedData.m_aPos.Y = nPosY;
        }
    }
}

framework::ToolbarLayoutManager::DockingOperation ToolbarLayoutManager::implts_determineDockingOperation(
    ui::DockingArea    DockingArea,
    const ::Rectangle& rRowColRect,
    const Point&       rMousePos )
{
    const sal_Int32 nHorzVerticalRegionSize        = 6;
    const sal_Int32 nHorzVerticalMoveRegion        = 4;

    if ( rRowColRect.IsInside( rMousePos ))
    {
        if ( isHorizontalDockingArea( DockingArea ))
        {
            sal_Int32 nRegion = rRowColRect.getHeight() / nHorzVerticalRegionSize;
            sal_Int32 nPosY   = rRowColRect.Top() + nRegion;

            if ( rMousePos.Y() < nPosY )
                return ( DockingArea == ui::DockingArea_DOCKINGAREA_TOP ) ? DOCKOP_BEFORE_COLROW : DOCKOP_AFTER_COLROW;
            else if ( rMousePos.Y() < ( nPosY + nRegion*nHorzVerticalMoveRegion ))
                return DOCKOP_ON_COLROW;
            else
                return ( DockingArea == ui::DockingArea_DOCKINGAREA_TOP ) ? DOCKOP_AFTER_COLROW : DOCKOP_BEFORE_COLROW;
        }
        else
        {
            sal_Int32 nRegion = rRowColRect.getWidth() / nHorzVerticalRegionSize;
            sal_Int32 nPosX   = rRowColRect.Left() + nRegion;

            if ( rMousePos.X() < nPosX )
                return ( DockingArea == ui::DockingArea_DOCKINGAREA_LEFT ) ? DOCKOP_BEFORE_COLROW : DOCKOP_AFTER_COLROW;
            else if ( rMousePos.X() < ( nPosX + nRegion*nHorzVerticalMoveRegion ))
                return DOCKOP_ON_COLROW;
            else
                return ( DockingArea == ui::DockingArea_DOCKINGAREA_LEFT ) ? DOCKOP_AFTER_COLROW : DOCKOP_BEFORE_COLROW;
        }
    }
    else
        return DOCKOP_ON_COLROW;
}

::Rectangle ToolbarLayoutManager::implts_calcTrackingAndElementRect(
    ui::DockingArea eDockingArea,
    sal_Int32 nRowCol,
    UIElement& rUIElement,
    const ::Rectangle& rTrackingRect,
    const ::Rectangle& rRowColumnRect,
    const ::Size& rContainerWinSize )
{
    ReadGuard aReadGuard( m_aLock );
    ::Rectangle aDockingAreaOffsets( m_aDockingAreaOffsets );
    aReadGuard.unlock();

    bool      bHorizontalDockArea( isHorizontalDockingArea( eDockingArea ));

    sal_Int32 nTopDockingAreaSize( implts_getTopBottomDockingAreaSizes().Width() );
    sal_Int32 nBottomDockingAreaSize( implts_getTopBottomDockingAreaSizes().Height() );

    sal_Int32 nMaxLeftRightDockAreaSize = rContainerWinSize.Height() -
                                          nTopDockingAreaSize -
                                          nBottomDockingAreaSize -
                                          aDockingAreaOffsets.Top() -
                                          aDockingAreaOffsets.Bottom();

    ::Rectangle aTrackingRect( rTrackingRect );
    if ( bHorizontalDockArea )
    {
        sal_Int32 nPosX( std::max( sal_Int32( rTrackingRect.Left()), sal_Int32( 0 )));
        if (( nPosX + rTrackingRect.getWidth()) > rContainerWinSize.Width() )
            nPosX = std::min( nPosX,
                                std::max( sal_Int32( rContainerWinSize.Width() - rTrackingRect.getWidth() ),
                                          sal_Int32( 0 )));

        sal_Int32 nSize = std::min( rContainerWinSize.Width(), rTrackingRect.getWidth() );

        aTrackingRect.SetPos( ::Point( nPosX, rRowColumnRect.Top() ));
        aTrackingRect.setWidth( nSize );
        aTrackingRect.setHeight( rRowColumnRect.getHeight() );

        // Set virtual position
        rUIElement.m_aDockedData.m_aPos.X = nPosX;
        rUIElement.m_aDockedData.m_aPos.Y = nRowCol;
    }
    else
    {
        sal_Int32 nMaxDockingAreaHeight = std::max( sal_Int32( 0 ),
                                                    sal_Int32( nMaxLeftRightDockAreaSize ));

        sal_Int32 nPosY( std::max( sal_Int32( aTrackingRect.Top()), sal_Int32( nTopDockingAreaSize )));
        if (( nPosY + aTrackingRect.getHeight()) > ( nTopDockingAreaSize + nMaxDockingAreaHeight ))
            nPosY = std::min( nPosY,
                                std::max( sal_Int32( nTopDockingAreaSize + ( nMaxDockingAreaHeight - aTrackingRect.getHeight() )),
                                        sal_Int32( nTopDockingAreaSize )));

        sal_Int32 nSize = std::min( nMaxDockingAreaHeight, static_cast<sal_Int32>(aTrackingRect.getHeight()) );

        aTrackingRect.SetPos( ::Point( rRowColumnRect.Left(), nPosY ));
        aTrackingRect.setWidth( rRowColumnRect.getWidth() );
        aTrackingRect.setHeight( nSize );

        aReadGuard.lock();
        uno::Reference< awt::XWindow  > xDockingAreaWindow( m_xDockAreaWindows[eDockingArea] );
        uno::Reference< awt::XWindow2 > xContainerWindow( m_xContainerWindow );
        aReadGuard.unlock();

        sal_Int32 nDockPosY( 0 );
        Window* pDockingAreaWindow( 0 );
        Window* pContainerWindow( 0 );
        {
            SolarMutexGuard aGuard;
            pDockingAreaWindow = VCLUnoHelper::GetWindow( xDockingAreaWindow );
            pContainerWindow = VCLUnoHelper::GetWindow( xContainerWindow );
            nDockPosY = pDockingAreaWindow->ScreenToOutputPixel( pContainerWindow->OutputToScreenPixel( ::Point( 0, nPosY ))).Y();
        }

        // Set virtual position
        rUIElement.m_aDockedData.m_aPos.X = nRowCol;
        rUIElement.m_aDockedData.m_aPos.Y = nDockPosY;
    }

    return aTrackingRect;
}

void ToolbarLayoutManager::implts_setTrackingRect( ui::DockingArea eDockingArea, const ::Point& rMousePos, ::Rectangle& rTrackingRect )
{
    ::Point aPoint( rTrackingRect.TopLeft());
    if ( isHorizontalDockingArea( eDockingArea ))
        aPoint.X() = rMousePos.X();
    else
        aPoint.Y() = rMousePos.Y();
    rTrackingRect.SetPos( aPoint );
}

void ToolbarLayoutManager::implts_renumberRowColumnData(
    ui::DockingArea eDockingArea,
    DockingOperation /*eDockingOperation*/,
    const UIElement& rUIElement )
{
    ReadGuard aReadLock( m_aLock );
    uno::Reference< container::XNameAccess > xPersistentWindowState( m_xPersistentWindowState );
    aReadLock.unlock();

    bool bHorzDockingArea( isHorizontalDockingArea( eDockingArea ));
    sal_Int32 nRowCol( bHorzDockingArea ? rUIElement.m_aDockedData.m_aPos.Y : rUIElement.m_aDockedData.m_aPos.X );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    UIElementVector::iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if (( pIter->m_aDockedData.m_nDockedArea == sal_Int16( eDockingArea )) && ( pIter->m_aName != rUIElement.m_aName ))
        {
            // Don't change toolbars without a valid docking position!
            if ( isDefaultPos( pIter->m_aDockedData.m_aPos ))
                continue;

            sal_Int32 nWindowRowCol = ( bHorzDockingArea ) ? pIter->m_aDockedData.m_aPos.Y : pIter->m_aDockedData.m_aPos.X;
            if ( nWindowRowCol >= nRowCol )
            {
                if ( bHorzDockingArea )
                    pIter->m_aDockedData.m_aPos.Y += 1;
                else
                    pIter->m_aDockedData.m_aPos.X += 1;
            }
        }
    }
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    // We have to change the persistent window state part
    if ( xPersistentWindowState.is() )
    {
        try
        {
            uno::Sequence< ::rtl::OUString > aWindowElements = xPersistentWindowState->getElementNames();
            for ( sal_Int32 i = 0; i < aWindowElements.getLength(); i++ )
            {
                if ( rUIElement.m_aName != aWindowElements[i] )
                {
                    try
                    {
                        uno::Sequence< beans::PropertyValue > aPropValueSeq;
                        awt::Point                            aDockedPos;
                        ui::DockingArea                       nDockedArea( ui::DockingArea_DOCKINGAREA_DEFAULT );

                        xPersistentWindowState->getByName( aWindowElements[i] ) >>= aPropValueSeq;
                        for ( sal_Int32 j = 0; j < aPropValueSeq.getLength(); j++ )
                        {
                            if ( aPropValueSeq[j].Name.equalsAscii( WINDOWSTATE_PROPERTY_DOCKINGAREA ))
                                aPropValueSeq[j].Value >>= nDockedArea;
                            else if ( aPropValueSeq[j].Name.equalsAscii( WINDOWSTATE_PROPERTY_DOCKPOS ))
                                aPropValueSeq[j].Value >>= aDockedPos;
                        }

                        // Don't change toolbars without a valid docking position!
                        if ( isDefaultPos( aDockedPos ))
                            continue;

                        sal_Int32 nWindowRowCol = ( bHorzDockingArea ) ? aDockedPos.Y : aDockedPos.X;
                        if (( nDockedArea == eDockingArea ) && ( nWindowRowCol >= nRowCol ))
                        {
                            if ( bHorzDockingArea )
                                aDockedPos.Y += 1;
                            else
                                aDockedPos.X += 1;

                            uno::Reference< container::XNameReplace > xReplace( xPersistentWindowState, uno::UNO_QUERY );
                            xReplace->replaceByName( aWindowElements[i], makeAny( aPropValueSeq ));
                        }
                    }
                    catch (const uno::Exception&)
                    {
                    }
                }
            }
        }
        catch (const uno::Exception&)
        {
        }
    }
}

//---------------------------------------------------------------------------------------------------------
//  XWindowListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL ToolbarLayoutManager::windowResized( const awt::WindowEvent& aEvent )
throw( uno::RuntimeException )
{
    WriteGuard aWriteLock( m_aLock );
    bool bLocked( m_bDockingInProgress );
    bool bLayoutInProgress( m_bLayoutInProgress );
    aWriteLock.unlock();

    // Do not do anything if we are in the middle of a docking process. This would interfere all other
    // operations. We will store the new position and size in the docking handlers.
    // Do not do anything if we are in the middle of our layouting process. We will adapt the position
    // and size of the user interface elements.
    if ( !bLocked && !bLayoutInProgress )
    {
        bool                           bNotify( false );
        uno::Reference< awt::XWindow > xWindow( aEvent.Source, uno::UNO_QUERY );

        UIElement aUIElement = implts_findToolbar( aEvent.Source );
        if ( aUIElement.m_xUIElement.is() )
        {
            if ( aUIElement.m_bFloating )
            {
                uno::Reference< awt::XWindow2 > xWindow2( xWindow, uno::UNO_QUERY );

                if( xWindow2.is() )
                {
                    awt::Rectangle aPos     = xWindow2->getPosSize();
                    awt::Size      aSize    = xWindow2->getOutputSize();   // always use output size for consistency
                    bool           bVisible = xWindow2->isVisible();

                    // update element data
                    aUIElement.m_aFloatingData.m_aPos = awt::Point(aPos.X, aPos.Y);
                    aUIElement.m_aFloatingData.m_aSize = aSize;
                    aUIElement.m_bVisible              = bVisible;
                }

                implts_writeWindowStateData( aUIElement );
            }
            else
            {
                implts_setLayoutDirty();
                bNotify = true;
            }
        }

        if ( bNotify )
            m_pParentLayouter->requestLayout( ILayoutNotifications::HINT_TOOLBARSPACE_HAS_CHANGED );
    }
}

void SAL_CALL ToolbarLayoutManager::windowMoved( const awt::WindowEvent& /*aEvent*/ )
throw( uno::RuntimeException )
{
}

void SAL_CALL ToolbarLayoutManager::windowShown( const lang::EventObject& /*aEvent*/ )
throw( uno::RuntimeException )
{
}

void SAL_CALL ToolbarLayoutManager::windowHidden( const lang::EventObject& /*aEvent*/ )
throw( uno::RuntimeException )
{
}

//---------------------------------------------------------------------------------------------------------
//  XDockableWindowListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL ToolbarLayoutManager::startDocking( const awt::DockingEvent& e )
throw (uno::RuntimeException)
{
    bool bWinFound( false );

    ReadGuard aReadGuard( m_aLock );
    uno::Reference< awt::XWindow2 > xContainerWindow( m_xContainerWindow );
    uno::Reference< awt::XWindow2 > xWindow( e.Source, uno::UNO_QUERY );
    aReadGuard.unlock();

    Window* pContainerWindow( 0 );
    Window* pWindow( 0 );
    ::Point aMousePos;
    {
        SolarMutexGuard aGuard;
        pContainerWindow = VCLUnoHelper::GetWindow( xContainerWindow );
        aMousePos = pContainerWindow->ScreenToOutputPixel( ::Point( e.MousePos.X, e.MousePos.Y ));
    }

    UIElement aUIElement = implts_findToolbar( e.Source );

    if ( aUIElement.m_xUIElement.is() && xWindow.is() )
    {
        awt::Rectangle aRect;

        bWinFound = true;
        uno::Reference< awt::XDockableWindow > xDockWindow( xWindow, uno::UNO_QUERY );
        if ( xDockWindow->isFloating() )
        {
            awt::Rectangle aPos  = xWindow->getPosSize();
            awt::Size      aSize = xWindow->getOutputSize();

            aUIElement.m_aFloatingData.m_aPos = awt::Point(aPos.X, aPos.Y);
            aUIElement.m_aFloatingData.m_aSize = aSize;

            SolarMutexGuard aGuard;

            pWindow = VCLUnoHelper::GetWindow( xWindow );
            if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
            {
                ToolBox* pToolBox = (ToolBox *)pWindow;
                aUIElement.m_aFloatingData.m_nLines        = pToolBox->GetFloatingLines();
                aUIElement.m_aFloatingData.m_bIsHorizontal = isToolboxHorizontalAligned( pToolBox );
            }
        }
    }

    WriteGuard aWriteLock( m_aLock );
    m_bDockingInProgress = bWinFound;
    m_aDockUIElement = aUIElement;
    m_aDockUIElement.m_bUserActive = true;
    m_aStartDockMousePos = aMousePos;
    aWriteLock.unlock();
}

awt::DockingData SAL_CALL ToolbarLayoutManager::docking( const awt::DockingEvent& e )
throw (uno::RuntimeException)
{
    const sal_Int32 MAGNETIC_DISTANCE_UNDOCK = 25;
    const sal_Int32 MAGNETIC_DISTANCE_DOCK   = 20;

    ReadGuard aReadLock( m_aLock );
    awt::DockingData                       aDockingData;
    uno::Reference< awt::XDockableWindow > xDockWindow( e.Source, uno::UNO_QUERY );
    uno::Reference< awt::XWindow >         xWindow( e.Source, uno::UNO_QUERY );
    uno::Reference< awt::XWindow >         xTopDockingWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_TOP] );
    uno::Reference< awt::XWindow >         xLeftDockingWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_LEFT] );
    uno::Reference< awt::XWindow >         xRightDockingWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_RIGHT] );
    uno::Reference< awt::XWindow >         xBottomDockingWindow( m_xDockAreaWindows[ui::DockingArea_DOCKINGAREA_BOTTOM] );
    uno::Reference< awt::XWindow2 >        xContainerWindow( m_xContainerWindow );
    UIElement                              aUIDockingElement( m_aDockUIElement );


    DockingOperation                       eDockingOperation( DOCKOP_ON_COLROW );
    bool                                   bDockingInProgress( m_bDockingInProgress );
    aReadLock.unlock();

    if ( bDockingInProgress )
        aDockingData.TrackingRectangle = e.TrackingRectangle;

    if ( bDockingInProgress && xDockWindow.is() && xWindow.is() )
    {
        try
        {
            SolarMutexGuard aGuard;

            sal_Int16 eDockingArea( -1 ); // none
            sal_Int32 nMagneticZone( aUIDockingElement.m_bFloating ? MAGNETIC_DISTANCE_DOCK : MAGNETIC_DISTANCE_UNDOCK );
            awt::Rectangle aNewTrackingRect;
            ::Rectangle aTrackingRect( e.TrackingRectangle.X, e.TrackingRectangle.Y,
                                       ( e.TrackingRectangle.X + e.TrackingRectangle.Width ),
                                       ( e.TrackingRectangle.Y + e.TrackingRectangle.Height ));

            awt::Rectangle aTmpRect = xTopDockingWindow->getPosSize();
            ::Rectangle aTopDockRect( aTmpRect.X, aTmpRect.Y, aTmpRect.Width, aTmpRect.Height );
            ::Rectangle aHotZoneTopDockRect( implts_calcHotZoneRect( aTopDockRect, nMagneticZone ));

            aTmpRect = xBottomDockingWindow->getPosSize();
            ::Rectangle aBottomDockRect( aTmpRect.X, aTmpRect.Y, ( aTmpRect.X + aTmpRect.Width), ( aTmpRect.Y + aTmpRect.Height ));
            ::Rectangle aHotZoneBottomDockRect( implts_calcHotZoneRect( aBottomDockRect, nMagneticZone ));

            aTmpRect = xLeftDockingWindow->getPosSize();
            ::Rectangle aLeftDockRect( aTmpRect.X, aTmpRect.Y, ( aTmpRect.X + aTmpRect.Width ), ( aTmpRect.Y + aTmpRect.Height ));
            ::Rectangle aHotZoneLeftDockRect( implts_calcHotZoneRect( aLeftDockRect, nMagneticZone ));

            aTmpRect = xRightDockingWindow->getPosSize();
            ::Rectangle aRightDockRect( aTmpRect.X, aTmpRect.Y, ( aTmpRect.X + aTmpRect.Width ), ( aTmpRect.Y + aTmpRect.Height ));
            ::Rectangle aHotZoneRightDockRect( implts_calcHotZoneRect( aRightDockRect, nMagneticZone ));

            Window* pContainerWindow( VCLUnoHelper::GetWindow( xContainerWindow ) );
            ::Point aMousePos( pContainerWindow->ScreenToOutputPixel( ::Point( e.MousePos.X, e.MousePos.Y )));

            if ( aHotZoneTopDockRect.IsInside( aMousePos ))
                eDockingArea = ui::DockingArea_DOCKINGAREA_TOP;
            else if ( aHotZoneBottomDockRect.IsInside( aMousePos ))
                eDockingArea = ui::DockingArea_DOCKINGAREA_BOTTOM;
            else if ( aHotZoneLeftDockRect.IsInside( aMousePos ))
                eDockingArea = ui::DockingArea_DOCKINGAREA_LEFT;
            else if ( aHotZoneRightDockRect.IsInside( aMousePos ))
                eDockingArea = ui::DockingArea_DOCKINGAREA_RIGHT;

            // Higher priority for movements inside the real docking area
            if ( aTopDockRect.IsInside( aMousePos ))
                eDockingArea = ui::DockingArea_DOCKINGAREA_TOP;
            else if ( aBottomDockRect.IsInside( aMousePos ))
                eDockingArea = ui::DockingArea_DOCKINGAREA_BOTTOM;
            else if ( aLeftDockRect.IsInside( aMousePos ))
                eDockingArea = ui::DockingArea_DOCKINGAREA_LEFT;
            else if ( aRightDockRect.IsInside( aMousePos ))
                eDockingArea = ui::DockingArea_DOCKINGAREA_RIGHT;

            // Determine if we have a toolbar and set alignment according to the docking area!
            Window*  pWindow = VCLUnoHelper::GetWindow( xWindow );
            ToolBox* pToolBox = 0;
            if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
                pToolBox = (ToolBox *)pWindow;

            if ( eDockingArea != -1 )
            {
                if ( eDockingArea == ui::DockingArea_DOCKINGAREA_TOP )
                {
                    aUIDockingElement.m_aDockedData.m_nDockedArea = ui::DockingArea_DOCKINGAREA_TOP;
                    aUIDockingElement.m_bFloating = false;
                }
                else if ( eDockingArea == ui::DockingArea_DOCKINGAREA_BOTTOM )
                {
                    aUIDockingElement.m_aDockedData.m_nDockedArea = ui::DockingArea_DOCKINGAREA_BOTTOM;
                    aUIDockingElement.m_bFloating = false;
                }
                else if ( eDockingArea == ui::DockingArea_DOCKINGAREA_LEFT )
                {
                    aUIDockingElement.m_aDockedData.m_nDockedArea = ui::DockingArea_DOCKINGAREA_LEFT;
                    aUIDockingElement.m_bFloating = false;
                }
                else if ( eDockingArea == ui::DockingArea_DOCKINGAREA_RIGHT )
                {
                    aUIDockingElement.m_aDockedData.m_nDockedArea = ui::DockingArea_DOCKINGAREA_RIGHT;
                    aUIDockingElement.m_bFloating = false;
                }

                ::Point aOutputPos = pContainerWindow->ScreenToOutputPixel( aTrackingRect.TopLeft() );
                aTrackingRect.SetPos( aOutputPos );

                ::Rectangle aNewDockingRect( aTrackingRect );


                implts_calcDockingPosSize( aUIDockingElement, eDockingOperation, aNewDockingRect, aMousePos );

                ::Point aScreenPos = pContainerWindow->OutputToScreenPixel( aNewDockingRect.TopLeft() );
                aNewTrackingRect = awt::Rectangle( aScreenPos.X(), aScreenPos.Y(),
                                                   aNewDockingRect.getWidth(), aNewDockingRect.getHeight() );
                aDockingData.TrackingRectangle = aNewTrackingRect;
            }
            else if ( pToolBox && bDockingInProgress )
            {
                bool bIsHorizontal = isToolboxHorizontalAligned( pToolBox );
                awt::Size aFloatSize = aUIDockingElement.m_aFloatingData.m_aSize;
                if ( aFloatSize.Width > 0 && aFloatSize.Height > 0 )
                {
                    aUIDockingElement.m_aFloatingData.m_aPos = AWTPoint(pContainerWindow->ScreenToOutputPixel(VCLPoint(e.MousePos)));
                    aDockingData.TrackingRectangle.Height = aFloatSize.Height;
                    aDockingData.TrackingRectangle.Width  = aFloatSize.Width;
                }
                else
                {
                    aFloatSize = AWTSize(pToolBox->CalcWindowSizePixel());
                    if ( !bIsHorizontal )
                    {
                        // Floating toolbars are always horizontal aligned! We have to swap
                        // width/height if we have a vertical aligned toolbar.
                        sal_Int32 nTemp = aFloatSize.Height;
                        aFloatSize.Height = aFloatSize.Width;
                        aFloatSize.Width  = nTemp;
                    }

                    aDockingData.TrackingRectangle.Height = aFloatSize.Height;
                    aDockingData.TrackingRectangle.Width  = aFloatSize.Width;

                    // For the first time we don't have any data about the floating size of a toolbar.
                    // We calculate it and store it for later use.
                    aUIDockingElement.m_aFloatingData.m_aPos = AWTPoint(pContainerWindow->ScreenToOutputPixel(VCLPoint(e.MousePos)));
                    aUIDockingElement.m_aFloatingData.m_aSize = aFloatSize;
                    aUIDockingElement.m_aFloatingData.m_nLines        = pToolBox->GetFloatingLines();
                    aUIDockingElement.m_aFloatingData.m_bIsHorizontal = isToolboxHorizontalAligned( pToolBox );
                }
                aDockingData.TrackingRectangle.X = e.MousePos.X;
                aDockingData.TrackingRectangle.Y = e.MousePos.Y;
            }

            aDockingData.bFloating = ( eDockingArea == -1 );

            // Write current data to the member docking progress data
            WriteGuard aWriteLock( m_aLock );
            m_aDockUIElement.m_bFloating = aDockingData.bFloating;
            if ( !aDockingData.bFloating )
            {
                m_aDockUIElement.m_aDockedData = aUIDockingElement.m_aDockedData;


                m_eDockOperation               = eDockingOperation;
            }
            else
                m_aDockUIElement.m_aFloatingData = aUIDockingElement.m_aFloatingData;
            aWriteLock.unlock();
        }
        catch (const uno::Exception&)
        {
        }
    }

    return aDockingData;
}

void SAL_CALL ToolbarLayoutManager::endDocking( const awt::EndDockingEvent& e )
throw (uno::RuntimeException)
{
    bool bDockingInProgress( false );
    bool bStartDockFloated( false );
    bool bFloating( false );
    UIElement aUIDockingElement;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    bDockingInProgress = m_bDockingInProgress;
    aUIDockingElement  = m_aDockUIElement;
    bFloating          = aUIDockingElement.m_bFloating;

    UIElement& rUIElement = impl_findToolbar( aUIDockingElement.m_aName );
    if ( rUIElement.m_aName == aUIDockingElement.m_aName )
    {
        if ( aUIDockingElement.m_bFloating )
        {
            // Write last position into position data
            uno::Reference< awt::XWindow > xWindow( aUIDockingElement.m_xUIElement->getRealInterface(), uno::UNO_QUERY );
            rUIElement.m_aFloatingData = aUIDockingElement.m_aFloatingData;
            awt::Rectangle aTmpRect = xWindow->getPosSize();
            rUIElement.m_aFloatingData.m_aPos = awt::Point(aTmpRect.X, aTmpRect.Y);
            // make changes also for our local data as we use it to make data persistent
            aUIDockingElement.m_aFloatingData = rUIElement.m_aFloatingData;
        }
        else
        {
            rUIElement.m_aDockedData = aUIDockingElement.m_aDockedData;
            rUIElement.m_aFloatingData.m_aSize = aUIDockingElement.m_aFloatingData.m_aSize;

            if ( m_eDockOperation != DOCKOP_ON_COLROW )
            {
                // we have to renumber our row/column data to insert a new row/column
                implts_renumberRowColumnData((ui::DockingArea)aUIDockingElement.m_aDockedData.m_nDockedArea, m_eDockOperation, aUIDockingElement );
            }
        }

        bStartDockFloated        = rUIElement.m_bFloating;
        rUIElement.m_bFloating   = m_aDockUIElement.m_bFloating;
        rUIElement.m_bUserActive = true;
    }

    // reset member for next docking operation
    m_aDockUIElement.m_xUIElement.clear();
    m_eDockOperation = DOCKOP_ON_COLROW;
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    implts_writeWindowStateData( aUIDockingElement );

    if ( bDockingInProgress )
    {
        SolarMutexGuard aGuard;
        Window*  pWindow = VCLUnoHelper::GetWindow( uno::Reference< awt::XWindow >( e.Source, uno::UNO_QUERY ));
        ToolBox* pToolBox = 0;
        if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
            pToolBox = (ToolBox *)pWindow;

        if ( pToolBox )
        {
            if( e.bFloating )
            {
                if ( aUIDockingElement.m_aFloatingData.m_bIsHorizontal )
                    pToolBox->SetAlign( WINDOWALIGN_TOP );
                else
                    pToolBox->SetAlign( WINDOWALIGN_LEFT );
            }
            else
            {
                ::Size aSize;

                pToolBox->SetAlign( ImplConvertAlignment( aUIDockingElement.m_aDockedData.m_nDockedArea) );

                // Docked toolbars have always one line
                aSize = pToolBox->CalcWindowSizePixel( 1 );

                // Lock layouting updates as our listener would be called due to SetSizePixel
                pToolBox->SetOutputSizePixel( aSize );
            }
        }
    }

    implts_sortUIElements();

    aWriteLock.lock();
    m_bDockingInProgress = sal_False;
    m_bLayoutDirty       = !bStartDockFloated || !bFloating;
    bool bNotify         = m_bLayoutDirty;
    aWriteLock.unlock();

    if ( bNotify )
        m_pParentLayouter->requestLayout( ILayoutNotifications::HINT_TOOLBARSPACE_HAS_CHANGED );
}

sal_Bool SAL_CALL ToolbarLayoutManager::prepareToggleFloatingMode( const lang::EventObject& e )
throw (uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );
    bool bDockingInProgress = m_bDockingInProgress;
    aReadLock.unlock();

    UIElement aUIDockingElement = implts_findToolbar( e.Source );
    bool bWinFound( !aUIDockingElement.m_aName.isEmpty() );
    uno::Reference< awt::XWindow > xWindow( e.Source, uno::UNO_QUERY );

    if ( bWinFound && xWindow.is() )
    {
        if ( !bDockingInProgress )
        {
            awt::Rectangle aRect;
            uno::Reference< awt::XDockableWindow > xDockWindow( xWindow, uno::UNO_QUERY );
            if ( xDockWindow->isFloating() )
            {
                {
                    SolarMutexGuard aGuard;
                    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                    if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
                    {
                        ToolBox* pToolBox = static_cast< ToolBox *>( pWindow );
                        aUIDockingElement.m_aFloatingData.m_aPos = AWTPoint(pToolBox->GetPosPixel());
                        aUIDockingElement.m_aFloatingData.m_aSize = AWTSize(pToolBox->GetOutputSizePixel());
                        aUIDockingElement.m_aFloatingData.m_nLines        = pToolBox->GetFloatingLines();
                        aUIDockingElement.m_aFloatingData.m_bIsHorizontal = isToolboxHorizontalAligned( pToolBox );
                    }
                }

                UIElement aUIElement = implts_findToolbar( aUIDockingElement.m_aName );
                if ( aUIElement.m_aName == aUIDockingElement.m_aName )
                    implts_setToolbar( aUIDockingElement );
            }
        }
    }

    return sal_True;
}

void SAL_CALL ToolbarLayoutManager::toggleFloatingMode( const lang::EventObject& e )
throw (uno::RuntimeException)
{
    UIElement aUIDockingElement;

    ReadGuard aReadLock( m_aLock );
    bool bDockingInProgress( m_bDockingInProgress );
    if ( bDockingInProgress )
        aUIDockingElement = m_aDockUIElement;
    aReadLock.unlock();

    Window*  pWindow( 0 );
    ToolBox* pToolBox( 0 );
    uno::Reference< awt::XWindow2 > xWindow;

    {
        SolarMutexGuard aGuard;
        xWindow = uno::Reference< awt::XWindow2 >( e.Source, uno::UNO_QUERY );
        pWindow = VCLUnoHelper::GetWindow( xWindow );

        if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
            pToolBox = (ToolBox *)pWindow;
    }

    if ( !bDockingInProgress )
    {
        aUIDockingElement = implts_findToolbar( e.Source );
        bool bWinFound = ( !aUIDockingElement.m_aName.isEmpty() );

        if ( bWinFound && xWindow.is() )
        {
            aUIDockingElement.m_bFloating   = !aUIDockingElement.m_bFloating;
            aUIDockingElement.m_bUserActive = true;

            implts_setLayoutInProgress( true );
            if ( aUIDockingElement.m_bFloating )
            {
                SolarMutexGuard aGuard;
                if ( pToolBox )
                {
                    pToolBox->SetLineCount( aUIDockingElement.m_aFloatingData.m_nLines );
                    if ( aUIDockingElement.m_aFloatingData.m_bIsHorizontal )
                        pToolBox->SetAlign( WINDOWALIGN_TOP );
                    else
                        pToolBox->SetAlign( WINDOWALIGN_LEFT );
                }

                bool bUndefPos = hasDefaultPosValue( aUIDockingElement.m_aFloatingData.m_aPos );
                bool bSetSize = !hasEmptySize( aUIDockingElement.m_aFloatingData.m_aSize );

                if ( bUndefPos )
                    aUIDockingElement.m_aFloatingData.m_aPos = implts_findNextCascadeFloatingPos();

                if ( !bSetSize )
                {
                    if ( pToolBox )
                        aUIDockingElement.m_aFloatingData.m_aSize = AWTSize(pToolBox->CalcFloatingWindowSizePixel());
                    else
                        aUIDockingElement.m_aFloatingData.m_aSize = AWTSize(pWindow->GetOutputSizePixel());
                }

                xWindow->setPosSize( aUIDockingElement.m_aFloatingData.m_aPos.X,
                                     aUIDockingElement.m_aFloatingData.m_aPos.Y,
                                     0, 0, awt::PosSize::POS );
                xWindow->setOutputSize(aUIDockingElement.m_aFloatingData.m_aSize);
            }
            else
            {
                if ( isDefaultPos( aUIDockingElement.m_aDockedData.m_aPos ))
                {
                    // Docking on its default position without a preset position -
                    // we have to find a good place for it.
                    ::Point aPixelPos;
                    awt::Point aDockPos;
                    ::Size  aSize;

                    {
                        SolarMutexGuard aGuard;
                        if ( pToolBox )
                            aSize = pToolBox->CalcWindowSizePixel( 1, ImplConvertAlignment( aUIDockingElement.m_aDockedData.m_nDockedArea ) );
                        else
                            aSize = pWindow->GetSizePixel();
                    }

                    implts_findNextDockingPos((ui::DockingArea)aUIDockingElement.m_aDockedData.m_nDockedArea, aSize, aDockPos, aPixelPos );
                    aUIDockingElement.m_aDockedData.m_aPos = aDockPos;
                }

                SolarMutexGuard aGuard;
                if ( pToolBox )
                {
                    pToolBox->SetAlign( ImplConvertAlignment( aUIDockingElement.m_aDockedData.m_nDockedArea) );
                    ::Size aSize = pToolBox->CalcWindowSizePixel( 1 );
                    awt::Rectangle aRect = xWindow->getPosSize();
                    xWindow->setPosSize( aRect.X, aRect.Y, 0, 0, awt::PosSize::POS );
                    xWindow->setOutputSize( AWTSize( aSize ) );
                }
            }

            implts_setLayoutInProgress( false );
            implts_setToolbar( aUIDockingElement );
            implts_writeWindowStateData( aUIDockingElement );
            implts_sortUIElements();
            implts_setLayoutDirty();

            aReadLock.lock();
            ILayoutNotifications* pParentLayouter( m_pParentLayouter );
            aReadLock.unlock();

            if ( pParentLayouter )
                pParentLayouter->requestLayout( ILayoutNotifications::HINT_TOOLBARSPACE_HAS_CHANGED );
        }
    }
    else
    {
        SolarMutexGuard aGuard;
        if ( pToolBox )
        {
            if ( aUIDockingElement.m_bFloating )
            {
                if ( aUIDockingElement.m_aFloatingData.m_bIsHorizontal )
                    pToolBox->SetAlign( WINDOWALIGN_TOP );
                else
                    pToolBox->SetAlign( WINDOWALIGN_LEFT );
            }
            else
                pToolBox->SetAlign( ImplConvertAlignment( aUIDockingElement.m_aDockedData.m_nDockedArea) );
        }
    }
}

void SAL_CALL ToolbarLayoutManager::closed( const lang::EventObject& e )
throw (uno::RuntimeException)
{
    rtl::OUString aName;
    UIElement     aUIElement;
    UIElementVector::iterator pIter;

    WriteGuard aWriteLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        uno::Reference< ui::XUIElement > xUIElement( pIter->m_xUIElement );
        if ( xUIElement.is() )
        {
            uno::Reference< uno::XInterface > xIfac( xUIElement->getRealInterface(), uno::UNO_QUERY );
            if ( xIfac == e.Source )
            {
                aName = pIter->m_aName;

                // user closes a toolbar =>
                // context sensitive toolbar: only destroy toolbar and store state.
                // non context sensitive toolbar: make it invisible, store state and destroy it.
                if ( !pIter->m_bContextSensitive )
                    pIter->m_bVisible = sal_False;

                aUIElement = *pIter;
                break;
            }
        }
    }
    aWriteLock.unlock();

    // destroy element
    if ( !aName.isEmpty() )
    {
        implts_writeWindowStateData( aUIElement );
        destroyToolbar( aName );

        ReadGuard aReadLock( m_aLock );
        bool bLayoutDirty = m_bLayoutDirty;
        ILayoutNotifications* pParentLayouter( m_pParentLayouter );
        aWriteLock.unlock();

        if ( bLayoutDirty && pParentLayouter )
            pParentLayouter->requestLayout( ILayoutNotifications::HINT_TOOLBARSPACE_HAS_CHANGED );
    }
}

void SAL_CALL ToolbarLayoutManager::endPopupMode( const awt::EndPopupModeEvent& /*e*/ )
throw (uno::RuntimeException)
{
}

//---------------------------------------------------------------------------------------------------------
//  XUIConfigurationListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL ToolbarLayoutManager::elementInserted( const ui::ConfigurationEvent& rEvent )
throw (uno::RuntimeException)
{
    UIElement aUIElement = implts_findToolbar( rEvent.ResourceURL );

    uno::Reference< ui::XUIElementSettings > xElementSettings( aUIElement.m_xUIElement, uno::UNO_QUERY );
    if ( xElementSettings.is() )
    {
        ::rtl::OUString aConfigSourcePropName( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
        uno::Reference< beans::XPropertySet > xPropSet( xElementSettings, uno::UNO_QUERY );
        if ( xPropSet.is() )
        {
            if ( rEvent.Source == uno::Reference< uno::XInterface >( m_xDocCfgMgr, uno::UNO_QUERY ))
                xPropSet->setPropertyValue( aConfigSourcePropName, makeAny( m_xDocCfgMgr ));
        }
        xElementSettings->updateSettings();
    }
    else
    {
        ::rtl::OUString aElementType;
        ::rtl::OUString aElementName;
        parseResourceURL( rEvent.ResourceURL, aElementType, aElementName );
        if ( aElementName.indexOf( m_aCustomTbxPrefix ) != -1 )
        {
            // custom toolbar must be directly created, shown and layouted!
            createToolbar( rEvent.ResourceURL );
            uno::Reference< ui::XUIElement > xUIElement = getToolbar( rEvent.ResourceURL );
            if ( xUIElement.is() )
            {
                ::rtl::OUString                               aUIName;
                uno::Reference< ui::XUIConfigurationManager > xCfgMgr;
                uno::Reference< beans::XPropertySet >         xPropSet;

                try
                {
                    xCfgMgr  = uno::Reference< ui::XUIConfigurationManager >( rEvent.Source, uno::UNO_QUERY );
                    xPropSet = uno::Reference< beans::XPropertySet >( xCfgMgr->getSettings( rEvent.ResourceURL, sal_False ), uno::UNO_QUERY );

                    if ( xPropSet.is() )
                        xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UIName" ))) >>= aUIName;
                }
                catch (const container::NoSuchElementException&)
                {
                }
                catch (const beans::UnknownPropertyException&)
                {
                }
                catch (const lang::WrappedTargetException&)
                {
                }

                {
                    SolarMutexGuard aGuard;
                    Window* pWindow = getWindowFromXUIElement( xUIElement );
                    if ( pWindow  )
                        pWindow->SetText( aUIName );
                }

                showToolbar( rEvent.ResourceURL );
            }
        }
    }
}

void SAL_CALL ToolbarLayoutManager::elementRemoved( const ui::ConfigurationEvent& rEvent )
throw (uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );
    uno::Reference< awt::XWindow > xContainerWindow( m_xContainerWindow, uno::UNO_QUERY );
    uno::Reference< ui::XUIConfigurationManager > xModuleCfgMgr( m_xModuleCfgMgr );
    uno::Reference< ui::XUIConfigurationManager > xDocCfgMgr( m_xDocCfgMgr );
    aReadLock.unlock();

    UIElement aUIElement = implts_findToolbar( rEvent.ResourceURL );
    uno::Reference< ui::XUIElementSettings > xElementSettings( aUIElement.m_xUIElement, uno::UNO_QUERY );
    if ( xElementSettings.is() )
    {
        bool                                  bNoSettings( false );
        ::rtl::OUString                       aConfigSourcePropName( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
        uno::Reference< uno::XInterface >     xElementCfgMgr;
        uno::Reference< beans::XPropertySet > xPropSet( xElementSettings, uno::UNO_QUERY );

        if ( xPropSet.is() )
            xPropSet->getPropertyValue( aConfigSourcePropName ) >>= xElementCfgMgr;

        if ( !xElementCfgMgr.is() )
            return;

        // Check if the same UI configuration manager has changed => check further
        if ( rEvent.Source == xElementCfgMgr )
        {
            // Same UI configuration manager where our element has its settings
            if ( rEvent.Source == uno::Reference< uno::XInterface >( xDocCfgMgr, uno::UNO_QUERY ))
            {
                // document settings removed
                if ( xModuleCfgMgr->hasSettings( rEvent.ResourceURL ))
                {
                    xPropSet->setPropertyValue( aConfigSourcePropName, makeAny( xModuleCfgMgr ));
                    xElementSettings->updateSettings();
                    return;
                }
            }

            bNoSettings = true;
        }

        // No settings anymore, element must be destroyed
        if ( xContainerWindow.is() && bNoSettings )
            destroyToolbar( rEvent.ResourceURL );
    }
}

void SAL_CALL ToolbarLayoutManager::elementReplaced( const ui::ConfigurationEvent& rEvent )
throw (uno::RuntimeException)
{
    UIElement aUIElement = implts_findToolbar( rEvent.ResourceURL );

    uno::Reference< ui::XUIElementSettings > xElementSettings( aUIElement.m_xUIElement, uno::UNO_QUERY );
    if ( xElementSettings.is() )
    {
        ::rtl::OUString                       aConfigSourcePropName( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
        uno::Reference< uno::XInterface >     xElementCfgMgr;
        uno::Reference< beans::XPropertySet > xPropSet( xElementSettings, uno::UNO_QUERY );

        if ( xPropSet.is() )
            xPropSet->getPropertyValue( aConfigSourcePropName ) >>= xElementCfgMgr;

        if ( !xElementCfgMgr.is() )
            return;

        // Check if the same UI configuration manager has changed => update settings
        if ( rEvent.Source == xElementCfgMgr )
        {
            xElementSettings->updateSettings();

            WriteGuard aWriteLock( m_aLock );
            bool bNotify = !aUIElement.m_bFloating;
            m_bLayoutDirty = bNotify;
            ILayoutNotifications* pParentLayouter( m_pParentLayouter );
            aWriteLock.unlock();

            if ( bNotify && pParentLayouter )
                pParentLayouter->requestLayout( ILayoutNotifications::HINT_TOOLBARSPACE_HAS_CHANGED );
        }
    }
}

uno::Reference< ui::XUIElement > ToolbarLayoutManager::getToolbar( const ::rtl::OUString& aName )
{
    return implts_findToolbar( aName ).m_xUIElement;
}

uno::Sequence< uno::Reference< ui::XUIElement > > ToolbarLayoutManager::getToolbars()
{
    uno::Sequence< uno::Reference< ui::XUIElement > > aSeq;

    ReadGuard aReadLock( m_aLock );
    if ( m_aUIElements.size() > 0 )
    {
        sal_uInt32 nCount(0);
        UIElementVector::iterator pIter;
        for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
        {
            if ( pIter->m_xUIElement.is() )
            {
                ++nCount;
                aSeq.realloc( nCount );
                aSeq[nCount-1] = pIter->m_xUIElement;
            }
        }
    }

    return aSeq;
}

bool ToolbarLayoutManager::floatToolbar( const ::rtl::OUString& rResourceURL )
{
    UIElement aUIElement = implts_findToolbar( rResourceURL );
    if ( aUIElement.m_xUIElement.is() )
    {
        try
        {
            uno::Reference< awt::XDockableWindow > xDockWindow( aUIElement.m_xUIElement->getRealInterface(), uno::UNO_QUERY );
            if ( xDockWindow.is() && !xDockWindow->isFloating() )
            {
                aUIElement.m_bFloating = true;
                implts_writeWindowStateData( aUIElement );
                xDockWindow->setFloatingMode( true );

                implts_setLayoutDirty();
                implts_setToolbar( aUIElement );
                return true;
            }
        }
        catch (const lang::DisposedException&)
        {
        }
    }

    return false;
}

bool ToolbarLayoutManager::lockToolbar( const ::rtl::OUString& rResourceURL )
{
    UIElement aUIElement = implts_findToolbar( rResourceURL );
    if ( aUIElement.m_xUIElement.is() )
    {
        try
        {
            uno::Reference< awt::XDockableWindow > xDockWindow( aUIElement.m_xUIElement->getRealInterface(), uno::UNO_QUERY );
            if ( xDockWindow.is() && !xDockWindow->isFloating() && !xDockWindow->isLocked() )
            {
                aUIElement.m_aDockedData.m_bLocked = true;
                implts_writeWindowStateData( aUIElement );
                xDockWindow->lock();

                implts_setLayoutDirty();
                implts_setToolbar( aUIElement );
                return true;
            }
        }
        catch (const lang::DisposedException&)
        {
        }
    }

    return false;
}

bool ToolbarLayoutManager::unlockToolbar( const ::rtl::OUString& rResourceURL )
{
    UIElement aUIElement = implts_findToolbar( rResourceURL );
    if ( aUIElement.m_xUIElement.is() )
    {
        try
        {
            uno::Reference< awt::XDockableWindow > xDockWindow( aUIElement.m_xUIElement->getRealInterface(), uno::UNO_QUERY );
            if ( xDockWindow.is() && !xDockWindow->isFloating() && xDockWindow->isLocked() )
            {
                aUIElement.m_aDockedData.m_bLocked = false;
                implts_writeWindowStateData( aUIElement );
                xDockWindow->unlock();

                implts_setLayoutDirty();
                implts_setToolbar( aUIElement );
                return true;
            }
        }
        catch (const lang::DisposedException&)
        {
        }
    }

    return false;
}

bool ToolbarLayoutManager::isToolbarVisible( const ::rtl::OUString& rResourceURL )
{
    uno::Reference< awt::XWindow2 > xWindow2( implts_getXWindow( rResourceURL ), uno::UNO_QUERY );
    return ( xWindow2.is() && xWindow2->isVisible() );
}

bool ToolbarLayoutManager::isToolbarFloating( const ::rtl::OUString& rResourceURL )
{
    uno::Reference< awt::XDockableWindow > xDockWindow( implts_getXWindow( rResourceURL ), uno::UNO_QUERY );
    return ( xDockWindow.is() && xDockWindow->isFloating() );
}

bool ToolbarLayoutManager::isToolbarDocked( const ::rtl::OUString& rResourceURL )
{
    return !isToolbarFloating( rResourceURL );
}

bool ToolbarLayoutManager::isToolbarLocked( const ::rtl::OUString& rResourceURL )
{
    uno::Reference< awt::XDockableWindow > xDockWindow( implts_getXWindow( rResourceURL ), uno::UNO_QUERY );
    return ( xDockWindow.is() && xDockWindow->isLocked() );
}

awt::Size ToolbarLayoutManager::getToolbarSize( const ::rtl::OUString& rResourceURL )
{
    Window* pWindow = implts_getWindow( rResourceURL );

    SolarMutexGuard aGuard;
    if ( pWindow )
    {
        ::Size    aSize = pWindow->GetSizePixel();
        awt::Size aWinSize;
        aWinSize.Width  = aSize.Width();
        aWinSize.Height = aSize.Height();
        return aWinSize;
    }

    return awt::Size();
}

awt::Point ToolbarLayoutManager::getToolbarPos( const ::rtl::OUString& rResourceURL )
{
    awt::Point aPos;
    UIElement  aUIElement = implts_findToolbar( rResourceURL );

    uno::Reference< awt::XWindow > xWindow( implts_getXWindow( rResourceURL ));
    if ( xWindow.is() )
    {
        if ( aUIElement.m_bFloating )
        {
            awt::Rectangle aRect = xWindow->getPosSize();
            aPos.X = aRect.X;
            aPos.Y = aRect.Y;
        }
        else
            aPos = aUIElement.m_aDockedData.m_aPos;
    }

    return aPos;
}

void ToolbarLayoutManager::setToolbarSize( const ::rtl::OUString& rResourceURL, const awt::Size& aSize )
{
  uno::Reference< awt::XWindow2 > xWindow( implts_getXWindow( rResourceURL ), uno::UNO_QUERY );
    uno::Reference< awt::XDockableWindow > xDockWindow( xWindow, uno::UNO_QUERY );
    UIElement aUIElement = implts_findToolbar( rResourceURL );

    if ( xWindow.is() && xDockWindow.is() && xDockWindow->isFloating() )
    {
        xWindow->setOutputSize( aSize );
        aUIElement.m_aFloatingData.m_aSize = aSize;
        implts_setToolbar( aUIElement );
        implts_writeWindowStateData( aUIElement );
        implts_sortUIElements();
    }
}

void ToolbarLayoutManager::setToolbarPos( const ::rtl::OUString& rResourceURL, const awt::Point& aPos )
{
    uno::Reference< awt::XWindow > xWindow( implts_getXWindow( rResourceURL ));
    uno::Reference< awt::XDockableWindow > xDockWindow( xWindow, uno::UNO_QUERY );
    UIElement aUIElement = implts_findToolbar( rResourceURL );

    if ( xWindow.is() && xDockWindow.is() && xDockWindow->isFloating() )
    {
        xWindow->setPosSize( aPos.X, aPos.Y, 0, 0, awt::PosSize::POS );
        aUIElement.m_aFloatingData.m_aPos = aPos;
        implts_setToolbar( aUIElement );
        implts_writeWindowStateData( aUIElement );
        implts_sortUIElements();
    }
}

void ToolbarLayoutManager::setToolbarPosSize( const ::rtl::OUString& rResourceURL, const awt::Point& aPos, const awt::Size& aSize )
{
    setToolbarPos( rResourceURL, aPos );
    setToolbarSize( rResourceURL, aSize );
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
