/*************************************************************************
 *
 *  $RCSfile: addonstoolbarmanager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 17:00:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_UIELEMENT_ADDONSTOOLBARMANAGER_HXX_
#include <uielement/addonstoolbarmanager.hxx>
#endif

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

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _DRAFTS_COM_SUN_STAR_UI_ITEMTYPE_HPP_
#include <drafts/com/sun/star/ui/ItemType.hpp>
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

#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XLAYOUTMANAGER_HPP_
#include <drafts/com/sun/star/frame/XLayoutManager.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_UI_XDOCKINGAREA_HPP_
#include <drafts/com/sun/star/ui/DockingArea.hpp>
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
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::drafts::com::sun::star::frame;
using namespace ::drafts::com::sun::star::ui;

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

static BOOL IsCorrectContext( Reference< com::sun::star::frame::XModel >& rModel, const rtl::OUString& aContextList )
{
    if ( rModel.is() )
    {
        Reference< com::sun::star::lang::XServiceInfo > xServiceInfo( rModel, UNO_QUERY );
        if ( xServiceInfo.is() )
        {
            sal_Int32 nIndex = 0;
            do
            {
                rtl::OUString aToken = aContextList.getToken( 0, ',', nIndex );

                if ( xServiceInfo->supportsService( aToken ))
                    return TRUE;
            }
            while ( nIndex >= 0 );
        }
    }

    return ( aContextList.getLength() == 0 );
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
        aImage = GetImageFromURL( rFrame, aImageId, bBigImage, bHiContrast );
        if ( !!aImage )
            return aImage;
    }

    aImage = GetImageFromURL( rFrame, aURL, bBigImage, bHiContrast );
    if ( !aImage )
        aImage = framework::AddonsOptions().GetImageFromURL( aURL, bBigImage, bHiContrast );

    return aImage;
}

void AddonsToolBarManager::RefreshImages()
{
    sal_Int16 nSymbolSet = GetCurrentSymbolSize();
    sal_Bool  bBigImages( nSymbolSet == SFX_SYMBOLS_LARGE );
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
    sal_Int16 nSymbolSet( GetCurrentSymbolSize() );

    RemoveControllers();

    m_pToolBar->Clear();
    m_aControllerVector.clear();

    Reference< XModel > xModel;
    if ( m_xFrame.is() )
    {
        Reference< com::sun::star::frame::XController > xController( m_xFrame->getController(), UNO_QUERY );
        if ( xController.is() )
            xModel = Reference< com::sun::star::frame::XModel >( xController->getModel(), UNO_QUERY );
    }

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

        const Sequence< PropertyValue >& rSeq = rAddonToolbar[n];

        sal_uInt32 nIndex = 0;
        while ( nIndex < (sal_uInt32)rSeq.getLength() )
        {
            aValueName = rSeq[nIndex].Name;
            if ( aValueName.equalsAsciiL( "URL", 3 ))
                rSeq[nIndex].Value >>= aURL;
            else if ( aValueName.equalsAsciiL( "Title", 5 ))
                rSeq[nIndex].Value >>= aTitle;
            else if ( aValueName.equalsAsciiL( "ImageIdentifier", 15 ))
                rSeq[nIndex].Value >>= aImageId;
            else if ( aValueName.equalsAsciiL( "Context", 7 ))
                rSeq[nIndex].Value >>= aContext;
            else if ( aValueName.equalsAsciiL( "Target", 6 ))
                rSeq[nIndex].Value >>= aTarget;
            nIndex++;
        }

        if ( IsCorrectContext( xModel, aContext ))
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
                svt::ToolboxController* pController = new GenericToolbarController( m_xServiceManager, m_xFrame, m_pToolBar, nId, aURL );
                xController = Reference< XStatusListener >( static_cast< ::cppu::OWeakObject *>( pController ), UNO_QUERY );
                m_aControllerVector.push_back( xController );
                Reference< XInitialization > xInit( xController, UNO_QUERY );
                if ( xInit.is() )
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
                    xInit->initialize( aArgs );

                    // Notify controller implementation to its listeners. Controller is now useable from outside.
                    Reference< XUpdatable > xUpdatable( xController, UNO_QUERY );
                    if ( xUpdatable.is() )
                        xUpdatable->update();
                }

                ++nId;
                ++nElements;
            }
        }
    }

    AddFrameActionListener();
}

IMPL_LINK( AddonsToolBarManager, Click, ToolBox*, pToolBar )
{
    if ( m_bDisposed )
        return 1;

    USHORT nId( m_pToolBar->GetCurItemId() );
    if (( nId > 0 ) && ( nId < m_aControllerVector.size() ))
    {
        Reference< XToolbarController > xController( m_aControllerVector[(nId-1)], UNO_QUERY );

        if ( xController.is() )
            xController->click();
    }

    return 1;
}

IMPL_LINK( AddonsToolBarManager, DoubleClick, ToolBox*, pToolBar )
{
    if ( m_bDisposed )
        return 1;

    USHORT nId( m_pToolBar->GetCurItemId() );
    if (( nId > 0 ) && ( nId < m_aControllerVector.size() ))
    {
        Reference< XToolbarController > xController( m_aControllerVector[(nId-1)], UNO_QUERY );

        if ( xController.is() )
            xController->doubleClick();
    }

    return 1;
}

IMPL_LINK( AddonsToolBarManager, Command, CommandEvent*, pCmdEvt )
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        return 1;

    return 0;
}

IMPL_LINK( AddonsToolBarManager, Select, ToolBox*, pToolBar )
{
    if ( m_bDisposed )
        return 1;

    sal_Int16   nKeyModifier( (sal_Int16)m_pToolBar->GetModifier() );
    USHORT      nId( m_pToolBar->GetCurItemId() );
    Reference< XToolbarController > xController( m_aControllerVector[(nId-1)], UNO_QUERY );

    if ( xController.is() )
        xController->execute( nKeyModifier );

    return 1;
}

IMPL_LINK( AddonsToolBarManager, Highlight, ToolBox*, pToolBar )
{
    return 1;
}

IMPL_LINK( AddonsToolBarManager, Activate, ToolBox*, pToolBar )
{
    return 1;
}

IMPL_LINK( AddonsToolBarManager, Deactivate, ToolBox*, pToolBar )
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
