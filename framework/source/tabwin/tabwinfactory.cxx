/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabwinfactory.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-10-30 08:11:38 $
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

#ifndef __FRAMEWORK_TABWIN_TABWINFACTORY_HXX_
#include <tabwin/tabwinfactory.hxx>
#endif
#ifndef __FRAMEWORK_TABWIN_TABWINDOW_HXX_
#include <tabwin/tabwindow.hxx>
#endif

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_WINDOWATTRIBUTE_HPP_
#include <com/sun/star/awt/WindowAttribute.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_3                    (    TabWinFactory                                                  ,
                                            OWeakObject                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                       ),
                                            DIRECT_INTERFACE( css::lang::XSingleComponentFactory            )
                                        )

DEFINE_XTYPEPROVIDER_3                  (   TabWinFactory                                   ,
                                            css::lang::XTypeProvider                        ,
                                            css::lang::XServiceInfo                         ,
                                            css::lang::XSingleComponentFactory
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   TabWinFactory                                   ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_TABWINFACTORY                       ,
                                            IMPLEMENTATIONNAME_TABWINFACTORY
                                        )

DEFINE_INIT_SERVICE                     (   TabWinFactory, {} )

TabWinFactory::TabWinFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase( &Application::GetSolarMutex() )
    , m_xServiceManager( xServiceManager )
{
}

TabWinFactory::~TabWinFactory()
{
}

css::uno::Reference< css::uno::XInterface > SAL_CALL TabWinFactory::createInstanceWithContext(
    const css::uno::Reference< css::uno::XComponentContext >& Context )
throw ( css::uno::Exception, css::uno::RuntimeException )
{
    css::uno::Sequence< css::uno::Any > aArgs;

    return createInstanceWithArgumentsAndContext( aArgs, Context );
}

css::uno::Reference< css::uno::XInterface > SAL_CALL TabWinFactory::createInstanceWithArgumentsAndContext(
    const css::uno::Sequence< css::uno::Any >& Arguments, const css::uno::Reference< css::uno::XComponentContext >& )
throw ( css::uno::Exception, css::uno::RuntimeException )
{
    const rtl::OUString aTopWindowArgName( RTL_CONSTASCII_USTRINGPARAM( "TopWindow" ));

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aLock( m_aLock );
    css::uno::Reference< css::awt::XToolkit > xToolkit = m_xToolkit;
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR( m_xServiceManager );
    aLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    css::uno::Reference< css::uno::XInterface > xReturn;
    css::uno::Reference< css::awt::XTopWindow > xTopWindow;
    css::beans::PropertyValue                   aPropValue;

    for ( sal_Int32 i = 0; i < Arguments.getLength(); i++ )
    {
        if ( Arguments[i] >>= aPropValue )
        {
            if ( aPropValue.Name == aTopWindowArgName )
                aPropValue.Value >>= xTopWindow;
        }
    }

    if ( !xToolkit.is() && xSMGR.is() )
    {
        xToolkit = css::uno::Reference< css::awt::XToolkit >( xSMGR->createInstance( SERVICENAME_VCLTOOLKIT ), css::uno::UNO_QUERY );
        if ( xToolkit.is() )
        {
            /* SAFE AREA ----------------------------------------------------------------------------------------------- */
            aLock.lock();
            m_xToolkit = xToolkit;
            aLock.unlock();
            /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        }
    }

    if ( !xTopWindow.is() )
    {
        if ( xToolkit.is() )
        {
            // describe window properties.
            css::awt::WindowDescriptor aDescriptor;
            aDescriptor.Type                =   css::awt::WindowClass_TOP                                           ;
            aDescriptor.ParentIndex         =   -1                                                                  ;
            aDescriptor.Parent              =   css::uno::Reference< css::awt::XWindowPeer >()                      ;
            aDescriptor.Bounds              =   css::awt::Rectangle(0,0,0,0)                                        ;
            aDescriptor.WindowAttributes    =   css::awt::WindowAttribute::BORDER|
                                                css::awt::WindowAttribute::SIZEABLE|
                                                css::awt::WindowAttribute::MOVEABLE|
                                                css::awt::WindowAttribute::CLOSEABLE|
                                                css::awt::WindowAttribute::MINSIZE;

            // create a parent window
            xTopWindow = css::uno::Reference< css::awt::XTopWindow >(
                            xToolkit->createWindow( aDescriptor ), css::uno::UNO_QUERY );
        }
    }

    if ( xTopWindow.is() )
    {
        TabWindow* pTabWindow = new TabWindow( xSMGR );

        css::uno::Sequence< css::uno::Any > aArgs( 1 );

        aPropValue.Name  = aTopWindowArgName;
        aPropValue.Value = css::uno::makeAny( xTopWindow );
        aArgs[0] = css::uno::makeAny( aPropValue );
        pTabWindow->initialize( aArgs );

        xReturn = css::uno::Reference< css::uno::XInterface >(
                    static_cast< OWeakObject* >( pTabWindow ), css::uno::UNO_QUERY );
    }

    return xReturn;
}

}
