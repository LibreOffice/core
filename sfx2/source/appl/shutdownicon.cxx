/*************************************************************************
 *
 *  $RCSfile: shutdownicon.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ssa $ $Date: 2001-07-05 12:12:46 $
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

#include <shutdownicon.hxx>
#include <app.hrc>
#include <app.hxx>
#include <vos/mutex.hxx>
#include <svtools/imagemgr.hxx>



#ifndef _COM_SUN_STAR_FRAME_XTASKSSUPPLIER_HPP_
#include <com/sun/star/frame/XTasksSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAMESSUPPLIER_HPP_
#include <com/sun/star/frame/XFramesSupplier.hpp>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <filedlghelper.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include "dispatch.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::vos;
using namespace ::rtl;
using namespace ::sfx2;

ShutdownIcon* ShutdownIcon::pShutdownIcon = NULL;

ShutdownIcon::ShutdownIcon( Reference< XDesktop >& aDesktop, ResMgr *aResMgr ) :
    m_xDesktop( aDesktop ),
    m_pResMgr( aResMgr ),
    m_bVeto ( false )
{
}

ShutdownIcon::~ShutdownIcon()
{
#ifdef WNT
    deInitSystray();
#endif
    m_xDesktop = Reference < XDesktop > ();
}


void ShutdownIcon::create( Reference< XDesktop >& aDesktop, ResMgr* aResMgr )
{
    if( !pShutdownIcon )
    {
        pShutdownIcon = new ShutdownIcon( aDesktop, aResMgr );
        pShutdownIcon->acquire();
#ifdef WNT
        pShutdownIcon->initSystray();
#endif
    }
}

void ShutdownIcon::destroy()
{
    if( pShutdownIcon )
    {
        pShutdownIcon->release();
        pShutdownIcon = NULL;
    }
}

ShutdownIcon* ShutdownIcon::getInstance()
{
    return pShutdownIcon;
}

// ---------------------------------------------------------------------------

void ShutdownIcon::SetAutostart( bool bActivate )
{
#ifdef WNT
    OUString aShortcutName( RTL_CONSTASCII_USTRINGPARAM( "StarOffice 6.0" ) );
    if( SFX_APP() && SFX_APP()->GetSfxResManager() )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        UniString aRes( ResId( STR_QUICKSTART_LNKNAME, SFX_APP()->GetSfxResManager() ) );
        aShortcutName = OUString( aRes );
    }
    aShortcutName += OUString( RTL_CONSTASCII_USTRINGPARAM( ".lnk" ) );

    SetAutostartW32( aShortcutName, bActivate );
#endif
}

bool ShutdownIcon::GetAutostart( )
{
#ifdef WNT
    OUString aShortcutName( RTL_CONSTASCII_USTRINGPARAM( "StarOffice 6.0" ) );
    if( SFX_APP() && SFX_APP()->GetSfxResManager() )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        UniString aRes( ResId( STR_QUICKSTART_LNKNAME, SFX_APP()->GetSfxResManager() ) );
        aShortcutName = OUString( aRes );
    }
    aShortcutName += OUString( RTL_CONSTASCII_USTRINGPARAM( ".lnk" ) );

    return GetAutostartW32( aShortcutName );
#else
    return false;
#endif
}


// ---------------------------------------------------------------------------

void ShutdownIcon::OpenURL( ::rtl::OUString& aURL )
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        Reference < XComponentLoader > xLoader ( getInstance()->m_xDesktop, UNO_QUERY );
        if( xLoader.is() )
        {
            try
            {
                Sequence< PropertyValue > aArgs(0);
                xLoader->loadComponentFromURL(
                    aURL,
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "_blank" ) ),
                    0,
                    aArgs );
            }
            catch( IOException )
            {
                OSL_ENSURE( 0, "IOException in loadComponentFromURL!");
            }
            catch( IllegalArgumentException )
            {
                OSL_ENSURE( 0, "IllegalArgumentException in loadComponentFromURL!");
            }
        }
    }
}

// ---------------------------------------------------------------------------

void ShutdownIcon::FileOpen()
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );

        FileDialogHelper dlg( WB_OPEN );
        if ( ERRCODE_NONE == dlg.Execute() )
            OpenURL( OUString( dlg.GetPath() ) );
    }
}

// ---------------------------------------------------------------------------

void ShutdownIcon::FromTemplate()
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        Reference < ::com::sun::star::frame::XFramesSupplier > xDesktop ( getInstance()->m_xDesktop, UNO_QUERY);
        Reference < ::com::sun::star::frame::XFrame > xFrame( xDesktop->getActiveFrame() );
        if ( !xFrame.is() )
            xFrame = Reference < ::com::sun::star::frame::XFrame >( xDesktop, UNO_QUERY );

        URL aTargetURL;
        aTargetURL.Complete = OUString( RTL_CONSTASCII_USTRINGPARAM( "slot:5500" ) );
        Reference < XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), UNO_QUERY );
        xTrans->parseStrict( aTargetURL );

        Reference < ::com::sun::star::frame::XDispatchProvider > xProv( xFrame, UNO_QUERY );
        Reference < ::com::sun::star::frame::XDispatch > xDisp;
        if ( xProv.is() )
            if ( aTargetURL.Protocol.compareToAscii("slot:") == COMPARE_EQUAL )
                xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
            else
                xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString::createFromAscii("_blank"), 0 );
        if ( xDisp.is() )
        {
            Sequence<PropertyValue> aArgs(1);
            PropertyValue* pArg = aArgs.getArray();
            pArg[0].Name = rtl::OUString::createFromAscii("Referer");
            pArg[0].Value <<= ::rtl::OUString::createFromAscii("private:user");
            xDisp->dispatch( aTargetURL, aArgs );
        }
    }
}

// ---------------------------------------------------------------------------

OUString ShutdownIcon::GetResString( int id )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if( !m_pResMgr )
        return OUString();

    UniString aRes( ResId(id, m_pResMgr) );
    return OUString( aRes );
}

// ---------------------------------------------------------------------------

OUString ShutdownIcon::GetUrlDescription( const OUString& aUrl )
{
    return OUString( SvFileInformationManager::GetDescription( INetURLObject( aUrl ) ) );
}

// ---------------------------------------------------------------------------

void ShutdownIcon::terminateDesktop()
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
    {
        // always remove ourselves as listener
        getInstance()->m_xDesktop->removeTerminateListener( getInstance() );

        // terminate desktop only if no tasks exist
        Reference < XTasksSupplier > xTasksSupplier( getInstance()->m_xDesktop, UNO_QUERY );
        if( xTasksSupplier.is() )
        {
            Reference < XEnumerationAccess > xEnum = xTasksSupplier->getTasks();
            if( xEnum.is() )
            {
                Reference < XElementAccess > xElement ( xEnum, UNO_QUERY );
                if ( xElement.is() )
                {
                    if( !xElement->hasElements() )
                        getInstance()->m_xDesktop->terminate();
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------

// XInterface
void SAL_CALL ShutdownIcon::acquire()
throw( ::com::sun::star::uno::RuntimeException )
{
    OWeakObject::acquire();
}

void SAL_CALL ShutdownIcon::release()
throw( ::com::sun::star::uno::RuntimeException )
{
    OWeakObject::release();
}

Any SAL_CALL ShutdownIcon::queryInterface( const Type & rType ) throw( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                rType ,
                SAL_STATIC_CAST(
                ::com::sun::star::frame::XTerminateListener* , this ),
                SAL_STATIC_CAST(
                ::com::sun::star::lang::XEventListener*, this ));
    if( a.hasValue() )
    {
        return a;
    }

    return OWeakObject::queryInterface( rType );
}


// XEventListener
void SAL_CALL ShutdownIcon::disposing( const ::com::sun::star::lang::EventObject& Source )
    throw(::com::sun::star::uno::RuntimeException)
{
}

// XTerminateListener
void SAL_CALL ShutdownIcon::queryTermination( const ::com::sun::star::lang::EventObject& aEvent )
throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard  aGuard( m_aMutex );

    if ( m_bVeto )
        throw ::com::sun::star::frame::TerminationVetoException();
}


void SAL_CALL ShutdownIcon::notifyTermination( const ::com::sun::star::lang::EventObject& aEvent )
throw(::com::sun::star::uno::RuntimeException)
{
}

