/*************************************************************************
 *
 *  $RCSfile: recentfilesmenucontroller.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 13:17:01 $
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

#ifndef __FRAMEWORK_UIELEMENT_RECENTFILESMENUCONTROLLER_HXX_
#include <uielement/recentfilesmenucontroller.hxx>
#endif

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include "services.h"
#endif

#ifndef __FRAMEWORK_CLASSES_RESOURCE_HRC_
#include <classes/resource.hrc>
#endif

#ifndef __FRAMEWORK_CLASSES_FWKRESID_HXX_
#include <classes/fwkresid.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_AWT_XDEVICE_HPP_
#include <com/sun/star/awt/XDevice.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_MENUITEMSTYLE_HPP_
#include <com/sun/star/awt/MenuItemStyle.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGWIDTH_HPP_
#include <com/sun/star/util/XStringWidth.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VCL_I18NHELP_HXX
#include <vcl/i18nhelp.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX
#include <svtools/historyoptions.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
//#include <tools/solar.hrc>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::container;

static const char SFX_REFERER_USER[] = "private:user";

namespace framework
{

class RecentFilesStringLength : public ::cppu::WeakImplHelper1< ::com::sun::star::util::XStringWidth >
{
    public:
        RecentFilesStringLength() {}
        virtual ~RecentFilesStringLength() {}

        // XStringWidth
        sal_Int32 SAL_CALL queryStringWidth( const ::rtl::OUString& aString )
            throw (::com::sun::star::uno::RuntimeException)
        {
            return aString.getLength();
        }
};

DEFINE_XSERVICEINFO_MULTISERVICE        (   RecentFilesMenuController                   ,
                                            OWeakObject                                 ,
                                            SERVICENAME_POPUPMENUCONTROLLER             ,
                                            IMPLEMENTATIONNAME_RECENTFILESMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   RecentFilesMenuController, {} )

RecentFilesMenuController::RecentFilesMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    PopupMenuControllerBase( xServiceManager ),
    m_bDisabled( sal_False )
{
}

RecentFilesMenuController::~RecentFilesMenuController()
{
}

// private function
void RecentFilesMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    VCLXPopupMenu*                                     pPopupMenu        = (VCLXPopupMenu *)VCLXMenu::GetImplementation( rPopupMenu );
    PopupMenu*                                         pVCLPopupMenu     = 0;

    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

    if ( pVCLPopupMenu )
    {
        Sequence< Sequence< PropertyValue > > aHistoryList = SvtHistoryOptions().GetList( ePICKLIST );
        Reference< XStringWidth > xStringLength( new RecentFilesStringLength );

        int nPickListMenuItems = ( aHistoryList.getLength() > 99 ) ? 99 : aHistoryList.getLength();

        m_aRecentFilesItems.clear();
        if (( nPickListMenuItems > 0 ) && !m_bDisabled )
        {
            for ( int i = 0; i < nPickListMenuItems; i++ )
            {
                Sequence< PropertyValue >& rPickListEntry = aHistoryList[i];
                RecentFile aRecentFile;

                for ( int j = 0; j < rPickListEntry.getLength(); j++ )
                {
                    Any a = rPickListEntry[j].Value;

                    if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_URL )
                        a >>= aRecentFile.aURL;
                    else if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_FILTER )
                        a >>= aRecentFile.aFilter;
                    else if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_TITLE )
                        a >>= aRecentFile.aTitle;
                    else if ( rPickListEntry[j].Name == HISTORY_PROPERTYNAME_PASSWORD )
                        a >>= aRecentFile.aPassword;
                }

                m_aRecentFilesItems.push_back( aRecentFile );
            }
        }

        if ( m_aRecentFilesItems.size() > 0 )
        {
            URL aTargetURL;

            for ( sal_uInt32 i = 0; i < m_aRecentFilesItems.size(); i++ )
            {
                char menuShortCut[5] = "~n: ";

                ::rtl::OUString aMenuShortCut;
                if ( i <= 9 )
                {
                    if ( i == 9 )
                        aMenuShortCut = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "1~0: " ));
                    else
                    {
                        menuShortCut[1] = (char)( '1' + i );
                        aMenuShortCut = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( menuShortCut ));
                    }
                }
                else
                {
                    aMenuShortCut = rtl::OUString::valueOf((sal_Int32)( i + 1 ));
                    aMenuShortCut += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ": " ));
                }

                // Abbreviate URL
                rtl::OUString   aURLString( m_aRecentFilesItems.at( i ).aURL );
                rtl::OUString   aTipHelpText;
                rtl::OUString   aMenuTitle;
                INetURLObject   aURL( aURLString );

                if ( aURL.GetProtocol() == INET_PROT_FILE )
                {
                    // Do handle file URL differently => convert it to a system
                    // path and abbreviate it with a special function:
                    String aFileSystemPath( aURL.getFSysPath( INetURLObject::FSYS_DETECT ) );

                    ::rtl::OUString aSystemPath( aFileSystemPath );
                    ::rtl::OUString aCompactedSystemPath;

                    aTipHelpText = aSystemPath;
                    oslFileError nError = osl_abbreviateSystemPath( aSystemPath.pData, &aCompactedSystemPath.pData, 46, NULL );
                    if ( !nError )
                        aMenuTitle = String( aCompactedSystemPath );
                    else
                        aMenuTitle = aSystemPath;
                }
                else
                {
                    // Use INetURLObject to abbreviate all other URLs
                    String  aShortURL;
                    aShortURL = aURL.getAbbreviated( xStringLength, 46, INetURLObject::DECODE_UNAMBIGUOUS );
                    aMenuTitle += aShortURL;
                    aTipHelpText = aURLString;
                }

                ::rtl::OUString aTitle( aMenuShortCut + aMenuTitle );

                pVCLPopupMenu->InsertItem( USHORT( i+1 ), aTitle );
                pVCLPopupMenu->SetTipHelpText( USHORT( i+1 ), aTipHelpText );
                pVCLPopupMenu->SetItemCommand( USHORT( i+1 ), aURLString );
            }
        }
        else
        {
            // No recent documents => insert "no document" string
            String aNoDocumentStr = String( FwkResId( STR_NODOCUMENT ));
            pVCLPopupMenu->InsertItem( 1, aNoDocumentStr );
            pVCLPopupMenu->EnableItem( 1, FALSE );
        }
    }
}

// XEventListener
void SAL_CALL RecentFilesMenuController::disposing( const EventObject& Source ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    ResetableGuard aLock( m_aLock );
    m_xFrame.clear();
    m_xDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL RecentFilesMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );
    m_bDisabled = Event.IsEnabled;
}

// XMenuListener
void SAL_CALL RecentFilesMenuController::highlight( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
}

void SAL_CALL RecentFilesMenuController::select( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    static int NUM_OF_PICKLIST_ARGS = 2;

    Reference< css::awt::XPopupMenu > xPopupMenu;
    Reference< XDispatch >            xDispatch;
    Reference< XDispatchProvider >    xDispatchProvider;
    Reference< XMultiServiceFactory > xServiceManager;

    ResetableGuard aLock( m_aLock );
    xPopupMenu          = m_xPopupMenu;
    xDispatchProvider   = Reference< XDispatchProvider >( m_xFrame, UNO_QUERY );
    xServiceManager     = m_xServiceManager;
    aLock.unlock();

    css::util::URL aTargetURL;
    Sequence< PropertyValue > aArgsList;

    if ( xPopupMenu.is() && xDispatchProvider.is() )
    {
        VCLXPopupMenu* pPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( xPopupMenu );
        if ( pPopupMenu )
        {
            Reference< XURLTransformer > xURLTransformer( xServiceManager->createInstance(
                                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                        UNO_QUERY );

            {
                vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
                PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();
                aTargetURL.Complete = pVCLPopupMenu->GetItemCommand( rEvent.MenuId );
            }

            const RecentFile& rRecentFile = m_aRecentFilesItems[ rEvent.MenuId-1 ];

            xURLTransformer->parseStrict( aTargetURL );

            aArgsList.realloc( NUM_OF_PICKLIST_ARGS );
            aArgsList[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Referer" ));
            aArgsList[0].Value = makeAny( ::rtl::OUString::createFromAscii( SFX_REFERER_USER ));

            ::rtl::OUString  aFilter( rRecentFile.aFilter );
            sal_Int32 nPos = aFilter.indexOf( '|' );
            if ( nPos >= 0 )
            {
                ::rtl::OUString aFilterOptions;

                if ( nPos < ( aFilter.getLength() - 1 ) )
                    aFilterOptions = aFilter.copy( nPos+1 );

                aArgsList[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterOptions" ));
                aArgsList[1].Value = makeAny( aFilterOptions );

                aFilter = aFilter.copy( 0, nPos-1 );
                aArgsList.realloc( ++NUM_OF_PICKLIST_ARGS );
            }

            aArgsList[NUM_OF_PICKLIST_ARGS-1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FilterName" ));
            aArgsList[NUM_OF_PICKLIST_ARGS-1].Value = makeAny( aFilter );

            xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString::createFromAscii("_default"), 0 );
        }
    }

    if ( xDispatch.is() )
    {
        // Call dispatch asychronously as we can be destroyed while dispatch is
        // executed. VCL is not able to survive this as it wants to call listeners
        // after select!!!
        LoadRecentFile* pLoadRecentFile = new LoadRecentFile;
        pLoadRecentFile->xDispatch  = xDispatch;
        pLoadRecentFile->aTargetURL = aTargetURL;
        pLoadRecentFile->aArgSeq    = aArgsList;
        Application::PostUserEvent( STATIC_LINK(0, RecentFilesMenuController, ExecuteHdl_Impl), pLoadRecentFile );
    }
}

void SAL_CALL RecentFilesMenuController::activate( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );
    if ( m_xPopupMenu.is() )
        fillPopupMenu( m_xPopupMenu );
}

void SAL_CALL RecentFilesMenuController::deactivate( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
}

// XPopupMenuController
void SAL_CALL RecentFilesMenuController::setPopupMenu( const Reference< css::awt::XPopupMenu >& xPopupMenu ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( m_xFrame.is() && !m_xPopupMenu.is() )
    {
        // Create popup menu on demand
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        m_xPopupMenu = xPopupMenu;
        m_xPopupMenu->addMenuListener( Reference< css::awt::XMenuListener >( (OWeakObject*)this, UNO_QUERY ));

        Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                    UNO_QUERY );
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

        com::sun::star::util::URL aTargetURL;
        aTargetURL.Complete = m_aCommandURL;
        xURLTransformer->parseStrict( aTargetURL );
        m_xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

        ResetableGuard aLock( m_aLock );
        if ( m_xPopupMenu.is() )
            fillPopupMenu( m_xPopupMenu );
    }
}

void SAL_CALL RecentFilesMenuController::updatePopupMenu() throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );
    Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
    Reference< XDispatch > xDispatch( m_xDispatch );
    Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                UNO_QUERY );
    com::sun::star::util::URL aTargetURL;
    aTargetURL.Complete = m_aCommandURL;
    xURLTransformer->parseStrict( aTargetURL );
    aLock.unlock();

    // Add/remove status listener to get a status update once
    if ( xDispatch.is() )
    {
        xDispatch->addStatusListener( xStatusListener, aTargetURL );
        xDispatch->removeStatusListener( xStatusListener, aTargetURL );
    }
}

// XInitialization
void SAL_CALL RecentFilesMenuController::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    const rtl::OUString aFrameName( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    const rtl::OUString aCommandURLName( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));

    ResetableGuard aLock( m_aLock );

    sal_Bool bInitalized( m_bInitialized );
    if ( !bInitalized )
    {
        PropertyValue       aPropValue;
        rtl::OUString       aCommandURL;
        Reference< XFrame > xFrame;

        for ( int i = 0; i < aArguments.getLength(); i++ )
        {
            if ( aArguments[i] >>= aPropValue )
            {
                if ( aPropValue.Name.equalsAscii( "Frame" ))
                    aPropValue.Value >>= xFrame;
                else if ( aPropValue.Name.equalsAscii( "CommandURL" ))
                    aPropValue.Value >>= aCommandURL;
            }
        }

        if ( xFrame.is() && aCommandURL.getLength() )
        {
            ResetableGuard aLock( m_aLock );
            m_xFrame        = xFrame;
            m_aCommandURL   = aCommandURL;
            m_bInitialized = sal_True;
        }
    }
}

IMPL_STATIC_LINK( RecentFilesMenuController, ExecuteHdl_Impl, LoadRecentFile*, pLoadRecentFile )
{
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        pLoadRecentFile->xDispatch->dispatch( pLoadRecentFile->aTargetURL, pLoadRecentFile->aArgSeq );
    }
    catch ( Exception& )
    {
    }

    delete pLoadRecentFile;
    return 0;
}

}
