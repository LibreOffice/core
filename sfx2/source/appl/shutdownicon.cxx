/*************************************************************************
 *
 *  $RCSfile: shutdownicon.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hro $ $Date: 2001-08-20 15:50:40 $
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
#include <cmdlineargs.hxx>


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
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <filedlghelper.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include "dispatch.hxx"
#include <comphelper/extract.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::vos;
using namespace ::rtl;
using namespace ::sfx2;

SFX_IMPL_XSERVICEINFO( ShutdownIcon, "com.sun.star.office.Quickstart", "com.sun.star.comp.desktop.QuickstartWrapper" )  \
SFX_IMPL_ONEINSTANCEFACTORY( ShutdownIcon );

ShutdownIcon* ShutdownIcon::pShutdownIcon = 0;

ShutdownIcon::ShutdownIcon( Reference< XMultiServiceFactory > aSMgr ) :
    ShutdownIconServiceBase( m_aMutex ),
    m_xServiceManager( aSMgr ),
    m_pResMgr( 0 ),
    m_bVeto ( false )
{
}

ShutdownIcon::~ShutdownIcon()
{
#ifdef WNT
    deInitSystray();
#endif
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

        // use ctor for filling up filters automatically! #89169#
        FileDialogHelper dlg( WB_OPEN | SFXWB_MULTISELECTION, *(SfxObjectFactory*) NULL );
        if ( ERRCODE_NONE == dlg.Execute() )
        {
            Reference< XFilePicker >    xPicker = dlg.GetFilePicker();

            try
            {
                if ( xPicker.is() )
                {
                    Sequence< OUString >    sFiles = xPicker->getFiles();
                    int                     nFiles = sFiles.getLength();

                    if ( 1 == nFiles )
                        OpenURL( sFiles[0] );
                    else
                    {
                        OUString    aBaseDirURL = sFiles[0];
                        if ( aBaseDirURL.getLength() > 0 && aBaseDirURL[aBaseDirURL.getLength()-1] != '/' )
                            aBaseDirURL += OUString::createFromAscii("/");

                        int iFiles;
                        for ( iFiles = 1; iFiles < nFiles; iFiles++ )
                        {
                            OUString    aURL = aBaseDirURL;
                            aURL += sFiles[iFiles];
                            OpenURL( aURL );
                        }
                    }
                }

//              OpenURL( OUString( dlg.GetPath() ) );
            }
            catch ( ... )
            {
            }
        }
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

void ShutdownIcon::addTerminateListener()
{
    if ( getInstance() && getInstance()->m_xDesktop.is() )
        getInstance()->m_xDesktop->addTerminateListener( getInstance() );
}

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

ShutdownIcon* ShutdownIcon::getInstance()
{
    OSL_ASSERT( pShutdownIcon );
    return pShutdownIcon;
}

void SAL_CALL ShutdownIcon::disposing()
{
    m_xServiceManager = Reference< XMultiServiceFactory >();
    m_xDesktop = Reference< XDesktop >();
}

// ---------------------------------------------------------------------------

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


void SAL_CALL ShutdownIcon::initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& aArguments )
    throw( ::com::sun::star::uno::Exception )
{
    ::osl::ClearableMutexGuard  aGuard( m_aMutex );

    if ( !ShutdownIcon::pShutdownIcon && aArguments.getLength() > 0 )
    {
        try
        {
            sal_Bool bQuickstart = sal_False;
            bQuickstart = ::cppu::any2bool( aArguments[0] );
            if( Application::IsRemoteServer() || ( !bQuickstart && !GetAutostart() ) )
                return;

            m_pResMgr = SFX_APP()->GetSfxResManager();

            m_xDesktop = Reference < XDesktop >( m_xServiceManager->createInstance(
                                                        DEFINE_CONST_UNICODE( "com.sun.star.frame.Desktop" )),
                                                    UNO_QUERY );

            if ( !m_xDesktop.is() )
                return;

            ShutdownIcon::pShutdownIcon = this;
#ifdef WNT
            initSystray();
#endif
        }
        catch(const ::com::sun::star::lang::IllegalArgumentException&)
        {
        }
    }
}
