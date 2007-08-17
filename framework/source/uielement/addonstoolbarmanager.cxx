/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: addonstoolbarmanager.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: ihi $ $Date: 2007-08-17 13:33:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#ifndef __FRAMEWORK_UIELEMENT_ADDONSTOOLBARMANAGER_HXX_
#include <uielement/addonstoolbarmanager.hxx>
#endif
#include <uielement/toolbarmerger.hxx>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________


#ifndef __FRAMEWORK_UIELEMENT_TOOLBAR_HXX
#include <uielement/toolbar.hxx>
#endif
#ifndef __FRAMEWORK_UIELEMENT_GENERICTOOLBARCONTROLLER_HXX
#include <uielement/generictoolbarcontroller.hxx>
#endif
#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif
#ifndef __FRAMEWORK_SERVICES_H_
#include "services.h"
#endif
#ifndef __FRAMEWORK_HELPER_IMAGEPRODUCER_HXX_
#include <helper/imageproducer.hxx>
#endif
#ifndef __FRAMEWORK_CLASSES_SFXHELPERFUNCTIONS_HXX_
#include <classes/sfxhelperfunctions.hxx>
#endif
#ifndef __FRAMEWORK_CLASSES_FWKRESID_HXX_
#include <classes/fwkresid.hxx>
#endif
#ifndef __FRAMEWORK_CLASES_RESOURCE_HRC_
#include <classes/resource.hrc>
#endif
#ifndef __FRAMEWORK_CLASSES_ADDONSOPTIONS_HXX_
#include <classes/addonsoptions.hxx>
#endif
#ifndef __FRAMEWORK_UIELEMENT_COMBOBOXTOOLBARCONTROLLER_HXX
#include <uielement/comboboxtoolbarcontroller.hxx>
#endif
#ifndef __FRAMEWORK_UIELEMENT_IMAGEBUTTONTOOLBARCONTROLLER_HXX
#include <uielement/imagebuttontoolbarcontroller.hxx>
#endif
#ifndef __FRAMEWORK_UIELEMENT_TOGGLEBUTTONTOOLBARCONTROLLER_HXX
#include <uielement/togglebuttontoolbarcontroller.hxx>
#endif
#ifndef __FRAMEWORK_UIELEMENT_BUTTONTOOLBARCONTROLLER_HXX_
#include <uielement/buttontoolbarcontroller.hxx>
#endif
#ifndef __FRAMEWORK_UIELEMENT_SPINFIELDTOOLBARCONTROLLER_HXX_
#include <uielement/spinfieldtoolbarcontroller.hxx>
#endif
#ifndef __FRAMEWORK_UIELEMENT_EDITTOOLBARCONTROLLER_HXX_
#include <uielement/edittoolbarcontroller.hxx>
#endif
#ifndef __FRAMEWORK_UIELEMENT_DROPDOWNBOXTOOLBARCONTROLLER_HXX_
#include <uielement/dropdownboxtoolbarcontroller.hxx>
#endif
#include <uielement/toolbarmerger.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UI_ITEMTYPE_HPP_
#include <com/sun/star/ui/ItemType.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTOOLBARCONTROLLER_HPP_
#include <com/sun/star/frame/XToolbarController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XLAYOUTMANAGER_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <com/sun/star/frame/XLayoutManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XDOCKINGAREA_HPP_
#include <com/sun/star/ui/DockingArea.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTICOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _SVTOOLS_IMGDEF_HXX
#include <svtools/imgdef.hxx>
#endif
#ifndef _SVTOOLS_TOOLBOXCONTROLLER_HXX
#include <svtools/toolboxcontroller.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#include <svtools/miscopt.hxx>
#include <vcl/svapp.hxx>
#include <vcl/menu.hxx>
#include <vcl/syswin.hxx>
#include <vcl/taskpanelist.hxx>

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::ui;

namespace framework
{

static const char   TOOLBOXITEM_SEPARATOR_STR[] = "private:separator";
static const USHORT TOOLBOXITEM_SEPARATOR_STR_LEN = sizeof( TOOLBOXITEM_SEPARATOR_STR )-1;

AddonsToolBarManager::AddonsToolBarManager( const Reference< XMultiServiceFactory >& rServiceManager,
                                const Reference< XFrame >& rFrame,
                                const rtl::OUString& rResourceName,
                                ToolBar* pToolBar ) :
    ToolBarManager( rServiceManager, rFrame, rResourceName, pToolBar )
{
    // Configuration data is retrieved from non-writable configuration layer. Therefor we
    // must disable some menu entries.
    m_bCanBeCustomized = sal_False;

    m_pToolBar->SetMenuType( TOOLBOX_MENUTYPE_CLIPPEDITEMS );
    m_pToolBar->SetSelectHdl( LINK( this, AddonsToolBarManager, Select) );
    m_pToolBar->SetActivateHdl( LINK( this, AddonsToolBarManager, Activate) );
    m_pToolBar->SetDeactivateHdl( LINK( this, AddonsToolBarManager, Deactivate) );
    m_pToolBar->SetClickHdl( LINK( this, AddonsToolBarManager, Click ) );
    m_pToolBar->SetDoubleClickHdl( LINK( this, AddonsToolBarManager, DoubleClick ) );
    m_pToolBar->SetCommandHdl( LINK( this, AddonsToolBarManager, Command ) );
    m_pToolBar->SetStateChangedHdl( LINK( this, AddonsToolBarManager, StateChanged ) );
    m_pToolBar->SetDataChangedHdl( LINK( this, AddonsToolBarManager, DataChanged ) );
}

AddonsToolBarManager::~AddonsToolBarManager()
{
}

static sal_Bool IsCorrectContext( const ::rtl::OUString& rModuleIdentifier, const ::rtl::OUString& aContextList )
{
    if ( aContextList.getLength() == 0 )
        return sal_True;

    if ( rModuleIdentifier.getLength() > 0 )
    {
        sal_Int32 nIndex = aContextList.indexOf( rModuleIdentifier );
        return ( nIndex >= 0 );
    }

    return sal_False;
}

static Image RetrieveImage( Reference< com::sun::star::frame::XFrame >& rFrame,
                            const rtl::OUString& aImageId,
                            const rtl::OUString& aURL,
                            BOOL bBigImage,
                            BOOL bHiContrast )
{
    Image aImage;

    if ( aImageId.getLength() > 0 )
    {
        aImage = framework::AddonsOptions().GetImageFromURL( aImageId, bBigImage, bHiContrast );
        if ( !!aImage )
            return aImage;
        else
            aImage = GetImageFromURL( rFrame, aImageId, bBigImage, bHiContrast );
        if ( !!aImage )
            return aImage;
    }

    aImage = framework::AddonsOptions().GetImageFromURL( aURL, bBigImage, bHiContrast );
    if ( !aImage )
        aImage = GetImageFromURL( rFrame, aImageId, bBigImage, bHiContrast );

    return aImage;
}

// XComponent
void SAL_CALL AddonsToolBarManager::dispose() throw( RuntimeException )
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    {
        // Remove addon specific data from toolbar items.
        ResetableGuard aGuard( m_aLock );
        for ( sal_uInt16 n = 0; n < m_pToolBar->GetItemCount(); n++ )
        {
            USHORT nId( m_pToolBar->GetItemId( n ) );

            if ( nId > 0 )
            {
                AddonsParams* pRuntimeItemData = (AddonsParams*)m_pToolBar->GetItemData( nId );
                if ( pRuntimeItemData )
                    delete pRuntimeItemData;
                m_pToolBar->SetItemData( nId, NULL );
            }
        }
    }

    // Base class will destroy our m_pToolBar member
    ToolBarManager::dispose();
}

bool AddonsToolBarManager::MenuItemAllowed( sal_uInt16 nId ) const
{
    if (( nId == MENUITEM_TOOLBAR_VISIBLEBUTTON ) ||
        ( nId == MENUITEM_TOOLBAR_CUSTOMIZETOOLBAR ))
        return false;
    else
        return true;
}

void AddonsToolBarManager::RefreshImages()
{
    sal_Bool  bBigImages( SvtMiscOptions().AreCurrentSymbolsLarge() );
    for ( USHORT nPos = 0; nPos < m_pToolBar->GetItemCount(); nPos++ )
    {
        USHORT nId( m_pToolBar->GetItemId( nPos ) );

        if ( nId > 0 )
        {
            OUString aCommandURL = m_pToolBar->GetItemCommand( nId );
            OUString aImageId;
            AddonsParams* pRuntimeItemData = (AddonsParams*)m_pToolBar->GetItemData( nId );
            if ( pRuntimeItemData )
                aImageId  = pRuntimeItemData->aImageId;

            m_pToolBar->SetItemImage( nId, RetrieveImage( m_xFrame,
                                                          aImageId,
                                                          aCommandURL,
                                                          bBigImages,
                                                          m_bIsHiContrast ));
        }
    }
}

void AddonsToolBarManager::FillToolbar( const Sequence< Sequence< PropertyValue > >& rAddonToolbar )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return;

    USHORT    nId( 1 );

    RemoveControllers();

    m_pToolBar->Clear();
    m_aControllerMap.clear();

    ::rtl::OUString aModuleIdentifier;
    try
    {
        Reference< XModuleManager > xModuleManager(
            m_xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY_THROW );
        aModuleIdentifier = xModuleManager->identify( m_xFrame );
    }
    catch ( Exception& )
    {
    }

    Reference< XMultiComponentFactory > xToolbarControllerFactory( m_xToolbarControllerRegistration, UNO_QUERY );
    Reference< XComponentContext > xComponentContext;
    Reference< XPropertySet > xProps( m_xServiceManager, UNO_QUERY );

    if ( xProps.is() )
        xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" ))) >>= xComponentContext;

    sal_uInt32  nElements( 0 );
    sal_Bool    bAppendSeparator( sal_False );
    Reference< XWindow > xToolbarWindow = VCLUnoHelper::GetInterface( m_pToolBar );
    for ( sal_uInt32 n = 0; n < (sal_uInt32)rAddonToolbar.getLength(); n++ )
    {
        rtl::OUString   aValueName;

        rtl::OUString   aURL;
        rtl::OUString   aTitle;
        rtl::OUString   aImageId;
        rtl::OUString   aContext;
        rtl::OUString   aTarget;
        rtl::OUString   aControlType;
        sal_uInt16      nWidth( 0 );

        const Sequence< PropertyValue >& rSeq = rAddonToolbar[n];

        ToolBarMerger::ConvertSequenceToValues( rSeq, aURL, aTitle, aImageId, aTarget, aContext, aControlType, nWidth );

        if ( IsCorrectContext( aModuleIdentifier, aContext ))
        {
            if ( aURL.equalsAsciiL( TOOLBOXITEM_SEPARATOR_STR, TOOLBOXITEM_SEPARATOR_STR_LEN ))
            {
                USHORT nCount = m_pToolBar->GetItemCount();
                if ( nCount > 0 && ( m_pToolBar->GetItemType( nCount-1 ) != TOOLBOXITEM_SEPARATOR ) && nElements > 0 )
                {
                    nElements = 0;
                    m_pToolBar->InsertSeparator();
                }
            }
            else
            {
                USHORT nCount = m_pToolBar->GetItemCount();
                if ( bAppendSeparator && nCount > 0 && ( m_pToolBar->GetItemType( nCount-1 ) != TOOLBOXITEM_SEPARATOR ))
                {
                    // We have to append a separator first if the last item is not a separator
                    m_pToolBar->InsertSeparator();
                }
                bAppendSeparator = FALSE;

                m_pToolBar->InsertItem( nId, aTitle );

                Image aImage = RetrieveImage( m_xFrame, aImageId, aURL, !m_bSmallSymbols, m_bIsHiContrast );
                if ( !!aImage )
                    m_pToolBar->SetItemImage( nId, aImage );

                // Create TbRuntimeItemData to hold additional information we will need in the future
                AddonsParams* pRuntimeItemData = new AddonsParams;
                pRuntimeItemData->aImageId  = aImageId;
                pRuntimeItemData->aTarget   = aTarget;
                m_pToolBar->SetItemData( nId, pRuntimeItemData );
                m_pToolBar->SetItemCommand( nId, aURL );

                Reference< XStatusListener > xController;

                sal_Bool bMustBeInit( sal_True );

                // Support external toolbar controller for add-ons!
                if ( m_xToolbarControllerRegistration.is() &&
                     m_xToolbarControllerRegistration->hasController( aURL, m_aModuleIdentifier ))
                {
                    if ( xToolbarControllerFactory.is() )
                    {
                        Sequence< Any > aArgs(5);
                        PropertyValue   aPropValue;

                        aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ModuleName" ));
                        aPropValue.Value    = makeAny( m_aModuleIdentifier );
                        aArgs[0] = makeAny( aPropValue );
                        aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
                        aPropValue.Value    = makeAny( m_xFrame );
                        aArgs[1] = makeAny( aPropValue );
                        aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ServiceManager" ));
                        aPropValue.Value    = makeAny( m_xServiceManager );
                        aArgs[2] = makeAny( aPropValue );
                        aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ParentWindow" ));
                        aPropValue.Value    = makeAny( xToolbarWindow );
                        aArgs[3] = makeAny( aPropValue );
                        aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ItemId" ));
                        aPropValue.Value    = makeAny( sal_Int32( nId ));
                        aArgs[4] = makeAny( aPropValue );

                        try
                        {
                            xController = Reference< XStatusListener >( xToolbarControllerFactory->createInstanceWithArgumentsAndContext(
                                                                            aURL, aArgs, xComponentContext ),
                                                                        UNO_QUERY );
                        }
                        catch ( uno::Exception& )
                        {
                        }
                        bMustBeInit = sal_False; // factory called init already!
                    }
                }
                else
                {
                    ::cppu::OWeakObject* pController = 0;

                    pController = ToolBarMerger::CreateController( m_xServiceManager, m_xFrame, m_pToolBar, aURL, nId, nWidth, aControlType );
                    xController = Reference< XStatusListener >( pController, UNO_QUERY );
                }

                // insert controller to the map
                m_aControllerMap[nId] = xController;

                Reference< XInitialization > xInit( xController, UNO_QUERY );
                if ( xInit.is() && bMustBeInit )
                {
                    PropertyValue aPropValue;
                    Sequence< Any > aArgs( 3 );
                    aPropValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
                    aPropValue.Value = makeAny( m_xFrame );
                    aArgs[0] = makeAny( aPropValue );
                    aPropValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));
                    aPropValue.Value = makeAny( aURL );
                    aArgs[1] = makeAny( aPropValue );
                    aPropValue.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ServiceManager" ));
                    aPropValue.Value = makeAny( m_xServiceManager );
                    aArgs[2] = makeAny( aPropValue );
                    try
                    {
                        xInit->initialize( aArgs );
                    }
                    catch ( uno::Exception& )
                    {
                    }
                }

                // Request a item window from the toolbar controller and set it at the VCL toolbar
                Reference< XToolbarController > xTbxController( xController, UNO_QUERY );
                if ( xTbxController.is() && xToolbarWindow.is() )
                {
                    Reference< XWindow > xWindow = xTbxController->createItemWindow( xToolbarWindow );
                    if ( xWindow.is() )
                    {
                        Window* pItemWin = VCLUnoHelper::GetWindow( xWindow );
                        if ( pItemWin )
                        {
                            WindowType nType = pItemWin->GetType();
                            if ( nType == WINDOW_LISTBOX || nType == WINDOW_MULTILISTBOX || nType == WINDOW_COMBOBOX )
                                pItemWin->SetAccessibleName( m_pToolBar->GetItemText( nId ) );
                            m_pToolBar->SetItemWindow( nId, pItemWin );
                        }
                    }
                }

                // Notify controller implementation to its listeners. Controller is now useable from outside.
                Reference< XUpdatable > xUpdatable( xController, UNO_QUERY );
                if ( xUpdatable.is() )
                {
                    try
                    {
                        xUpdatable->update();
                    }
                    catch ( uno::Exception& )
                    {
                    }
                }

                ++nId;
                ++nElements;
            }
        }
    }

    AddFrameActionListener();
}

IMPL_LINK( AddonsToolBarManager, Click, ToolBox*, EMPTYARG )
{
    if ( m_bDisposed )
        return 1;

    USHORT nId( m_pToolBar->GetCurItemId() );
    ToolBarControllerMap::const_iterator pIter = m_aControllerMap.find( nId );
    if ( pIter != m_aControllerMap.end() )
    {
        Reference< XToolbarController > xController( pIter->second, UNO_QUERY );

        if ( xController.is() )
            xController->click();
    }

    return 1;
}

IMPL_LINK( AddonsToolBarManager, DoubleClick, ToolBox*, EMPTYARG )
{
    if ( m_bDisposed )
        return 1;

    USHORT nId( m_pToolBar->GetCurItemId() );
    ToolBarControllerMap::const_iterator pIter = m_aControllerMap.find( nId );
    if ( pIter != m_aControllerMap.end() )
    {
        Reference< XToolbarController > xController( pIter->second, UNO_QUERY );

        if ( xController.is() )
            xController->doubleClick();
    }

    return 1;
}

IMPL_LINK( AddonsToolBarManager, Command, CommandEvent*, EMPTYARG )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    return 0;
}

IMPL_LINK( AddonsToolBarManager, Select, ToolBox*, EMPTYARG )
{
    if ( m_bDisposed )
        return 1;

    sal_Int16   nKeyModifier( (sal_Int16)m_pToolBar->GetModifier() );
    USHORT      nId( m_pToolBar->GetCurItemId() );
    ToolBarControllerMap::const_iterator pIter = m_aControllerMap.find( nId );
    if ( pIter != m_aControllerMap.end() )
    {
        Reference< XToolbarController > xController( pIter->second, UNO_QUERY );

        if ( xController.is() )
            xController->execute( nKeyModifier );
    }

    return 1;
}

IMPL_LINK( AddonsToolBarManager, Highlight, ToolBox*, EMPTYARG )
{
    return 1;
}

IMPL_LINK( AddonsToolBarManager, Activate, ToolBox*, EMPTYARG )
{
    return 1;
}

IMPL_LINK( AddonsToolBarManager, Deactivate, ToolBox*, EMPTYARG )
{
    return 1;
}

IMPL_LINK( AddonsToolBarManager, StateChanged, StateChangedType*, pStateChangedType )
{
    if ( *pStateChangedType == STATE_CHANGE_CONTROLBACKGROUND )
    {
        // Check if we need to get new images for normal/high contrast mode
        CheckAndUpdateImages();
    }
    return 1;
}

IMPL_LINK( AddonsToolBarManager, DataChanged, DataChangedEvent*, pDataChangedEvent  )
{
    if ((( pDataChangedEvent->GetType() == DATACHANGED_SETTINGS )   ||
        (  pDataChangedEvent->GetType() == DATACHANGED_DISPLAY  ))  &&
        ( pDataChangedEvent->GetFlags() & SETTINGS_STYLE        ))
    {
        // Check if we need to get new images for normal/high contrast mode
        CheckAndUpdateImages();
    }

    for ( USHORT nPos = 0; nPos < m_pToolBar->GetItemCount(); ++nPos )
    {
        const USHORT nId = m_pToolBar->GetItemId(nPos);
        Window* pWindow = m_pToolBar->GetItemWindow( nId );
        if ( pWindow )
        {
            const DataChangedEvent& rDCEvt( *pDataChangedEvent );
            pWindow->DataChanged( rDCEvt );
        }
    }

    return 1;
}

}
