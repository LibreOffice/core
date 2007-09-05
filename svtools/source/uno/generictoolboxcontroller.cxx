/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: generictoolboxcontroller.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-05 17:39:34 $
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
#include "precompiled_svtools.hxx"

#ifndef __SVTOOLS_GENERICTOOLBOXCONTROLLER_HXX_
#include <svtools/generictoolboxcontroller.hxx>
#endif

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_STATUS_ITEMSTATUS_HPP_
#include <com/sun/star/frame/status/ItemStatus.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_STATUS_ITEMSTATE_HPP_
#include <com/sun/star/frame/status/ItemState.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::util;

namespace svt
{

struct ExecuteInfo
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >     xDispatch;
    ::com::sun::star::util::URL                                                aTargetURL;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >  aArgs;
};

GenericToolboxController::GenericToolboxController( const Reference< XMultiServiceFactory >& rServiceManager,
                                                    const Reference< XFrame >&               rFrame,
                                                    ToolBox*                                 pToolbox,
                                                    USHORT                                   nID,
                                                    const ::rtl::OUString&                          aCommand ) :
    svt::ToolboxController( rServiceManager, rFrame, aCommand )
    ,   m_pToolbox( pToolbox )
    ,   m_nID( nID )
{
    // Initialization is done through ctor
    m_bInitialized = sal_True;

    // insert main command to our listener map
    if ( m_aCommandURL.getLength() )
        m_aListenerMap.insert( URLToDispatchMap::value_type( aCommand, Reference< XDispatch >() ));
}

GenericToolboxController::~GenericToolboxController()
{
}

void SAL_CALL GenericToolboxController::dispose()
throw ( RuntimeException )
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    svt::ToolboxController::dispose();

    m_pToolbox = 0;
    m_nID = 0;
}

void SAL_CALL GenericToolboxController::execute( sal_Int16 /*KeyModifier*/ )
throw ( RuntimeException )
{
    Reference< XDispatch >       xDispatch;
    Reference< XURLTransformer > xURLTransformer;
    ::rtl::OUString                     aCommandURL;

    {
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_bInitialized &&
             m_xFrame.is() &&
             m_xServiceManager.is() &&
             m_aCommandURL.getLength() )
        {
            xURLTransformer = Reference< XURLTransformer >( m_xServiceManager->createInstance(
                                                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                            UNO_QUERY );

            aCommandURL = m_aCommandURL;
            URLToDispatchMap::iterator pIter = m_aListenerMap.find( m_aCommandURL );
            if ( pIter != m_aListenerMap.end() )
                xDispatch = pIter->second;
        }
    }

    if ( xDispatch.is() && xURLTransformer.is() )
    {
        com::sun::star::util::URL aTargetURL;
        Sequence<PropertyValue>   aArgs;

        aTargetURL.Complete = aCommandURL;
        xURLTransformer->parseStrict( aTargetURL );

        // Execute dispatch asynchronously
        ExecuteInfo* pExecuteInfo = new ExecuteInfo;
        pExecuteInfo->xDispatch     = xDispatch;
        pExecuteInfo->aTargetURL    = aTargetURL;
        pExecuteInfo->aArgs         = aArgs;
        Application::PostUserEvent( STATIC_LINK(0, GenericToolboxController , ExecuteHdl_Impl), pExecuteInfo );
    }
}

void GenericToolboxController::statusChanged( const FeatureStateEvent& Event )
throw ( RuntimeException )
{
    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    if ( m_bDisposed )
        return;

    if ( m_pToolbox )
    {
        m_pToolbox->EnableItem( m_nID, Event.IsEnabled );

        USHORT nItemBits = m_pToolbox->GetItemBits( m_nID );
        nItemBits &= ~TIB_CHECKABLE;
        TriState eTri = STATE_NOCHECK;

        sal_Bool        bValue = sal_Bool();
        rtl::OUString   aStrValue;
        ItemStatus      aItemState;

        if ( Event.State >>= bValue )
        {
            // Boolean, treat it as checked/unchecked
            m_pToolbox->SetItemBits( m_nID, nItemBits );
            m_pToolbox->CheckItem( m_nID, bValue );
            if ( bValue )
                eTri = STATE_CHECK;
            nItemBits |= TIB_CHECKABLE;
        }
        else if ( Event.State >>= aStrValue )
        {
            m_pToolbox->SetItemText( m_nID, aStrValue );
        }
        else if ( Event.State >>= aItemState )
        {
            eTri = STATE_DONTKNOW;
            nItemBits |= TIB_CHECKABLE;
        }

        m_pToolbox->SetItemState( m_nID, eTri );
        m_pToolbox->SetItemBits( m_nID, nItemBits );
    }
}

IMPL_STATIC_LINK_NOINSTANCE( GenericToolboxController, ExecuteHdl_Impl, ExecuteInfo*, pExecuteInfo )
{
   try
   {
       // Asynchronous execution as this can lead to our own destruction!
       // Framework can recycle our current frame and the layout manager disposes all user interface
       // elements if a component gets detached from its frame!
       pExecuteInfo->xDispatch->dispatch( pExecuteInfo->aTargetURL, pExecuteInfo->aArgs );
   }
   catch ( Exception& )
   {
   }
   delete pExecuteInfo;
   return 0;
}

} // namespace
