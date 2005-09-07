/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: plugin.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:48:13 $
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

#include "plugin.hxx"

#ifndef  _COM_SUN_STAR_PLUGIN_XPLUGINMANAGER_HPP_
#include <com/sun/star/plugin/XPluginManager.hpp>
#endif
#ifndef  _COM_SUN_STAR_PLUGIN_PLUGINMODE_HPP_
#include <com/sun/star/plugin/PluginMode.hpp>
#endif
#ifndef  _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif

#include <tools/debug.hxx>
#include <rtl/ustring.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/miscopt.hxx>
#include <vcl/window.hxx>

using namespace ::com::sun::star;

namespace sfx2
{

class PluginWindow_Impl : public Window
{
public:
    uno::Reference < awt::XWindow > xWindow;
    PluginWindow_Impl( Window* pParent )
     : Window( pParent, WB_CLIPCHILDREN )
     {}

     virtual void Resize();
};

void PluginWindow_Impl::Resize()
{
    Size aSize( GetOutputSizePixel() );
    if ( xWindow.is() )
        xWindow->setPosSize( 0, 0, aSize.Width(), aSize.Height(), WINDOW_POSSIZE_SIZE );
}

#define PROPERTY_UNBOUND 0

SfxItemPropertyMap aPluginPropertyMap_Impl[] =
{
    { "PluginCommands", 14, 1, &::getCppuType((::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >*)0), PROPERTY_UNBOUND, 0},
    { "PluginMimeType", 14, 2, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "PluginURL",       9, 3, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    {0,0,0,0,0}
};

SFX_IMPL_XSERVICEINFO( PluginObject, "com.sun.star.embed.SpecialEmbeddedObject", "com.sun.star.comp.sfx2.PluginObject" )
SFX_IMPL_SINGLEFACTORY( PluginObject );

PluginObject::PluginObject( const uno::Reference < lang::XMultiServiceFactory >& rFact )
    : mxFact( rFact )
    , maPropSet( aPluginPropertyMap_Impl )
{
}

PluginObject::~PluginObject()
{
}

void SAL_CALL PluginObject::initialize( const uno::Sequence< uno::Any >& aArguments ) throw ( uno::Exception, uno::RuntimeException )
{
    if ( aArguments.getLength() )
        aArguments[0] >>= mxObj;
}

sal_Bool SAL_CALL PluginObject::load( const uno::Sequence < com::sun::star::beans::PropertyValue >& lDescriptor,
            const uno::Reference < frame::XFrame >& xFrame ) throw( uno::RuntimeException )
{
    uno::Reference< plugin::XPluginManager > xPMgr( mxFact->createInstance( ::rtl::OUString::createFromAscii("com.sun.star.plugin.PluginManager") ), uno::UNO_QUERY );
    if (!xPMgr.is() )
        return FALSE;

    if ( SvtMiscOptions().IsPluginsEnabled() )
    {
        Window* pParent = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
        PluginWindow_Impl* pWin = new PluginWindow_Impl( pParent );
        pWin->SetSizePixel( pParent->GetOutputSizePixel() );
        pWin->SetBackground();
        pWin->Show();

        ULONG nCount = maCmdList.Count();
        uno::Sequence < ::rtl::OUString > aCmds( nCount ), aArgs( nCount );
        ::rtl::OUString *pCmds = aCmds.getArray(), *pArgs = aArgs.getArray();
        for( ULONG i = 0; i < nCount; i++ )
        {
            SvCommand & rCmd = maCmdList.GetObject( i );
            pCmds[i] = rCmd.GetCommand();
            pArgs[i] = rCmd.GetArgument();
        }

        mxPlugin = xPMgr->createPluginFromURL(
            xPMgr->createPluginContext(), plugin::PluginMode::EMBED, aCmds, aArgs, uno::Reference< awt::XToolkit >(),
            uno::Reference< awt::XWindowPeer >( pWin->GetComponentInterface() ), maURL );

        if ( mxPlugin.is() )
        {
            uno::Reference< awt::XWindow > xWindow( mxPlugin, uno::UNO_QUERY );
            if ( xWindow.is() )
            {
                pWin->xWindow = xWindow;
                pWin->Resize();
                xWindow->setVisible( TRUE );
            }

            try
            {
                uno::Reference< awt::XControl > xControl( mxPlugin, uno::UNO_QUERY );
                if( xControl.is() )
                {
                    uno::Reference< awt::XControlModel > xModel = xControl->getModel();
                    uno::Reference< beans::XPropertySet > xProp( xModel, ::uno::UNO_QUERY );
                    if( xProp.is() )
                    {
                        uno::Any aValue = xProp->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) ) );
                        aValue >>= maURL;
                        aValue = xProp->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TYPE" ) ) );
                        aValue >>= maMimeType;
                    }
                }
            }
            catch( uno::Exception& )
            {
            }
        }

        uno::Reference < awt::XWindow > xWindow( pWin->GetComponentInterface(), uno::UNO_QUERY );

        // we must destroy the plugin before the parent is destroyed
        xWindow->addEventListener( this );
        xFrame->setComponent( xWindow, uno::Reference < frame::XController >() );
        return TRUE;
    }

    return FALSE;
}

void SAL_CALL PluginObject::cancel() throw( com::sun::star::uno::RuntimeException )
{
    uno::Reference< lang::XComponent > xComp( mxPlugin, uno::UNO_QUERY );
    if (xComp.is())
        xComp->dispose();
    mxPlugin = 0;
}

void SAL_CALL PluginObject::close( sal_Bool bDeliverOwnership ) throw( com::sun::star::util::CloseVetoException, com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL PluginObject::addCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& xListener ) throw( com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL PluginObject::removeCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& xListener ) throw( com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL PluginObject::disposing( const com::sun::star::lang::EventObject& aEvent ) throw (com::sun::star::uno::RuntimeException)
{
    cancel();
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL PluginObject::getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException )
{
    return maPropSet.getPropertySetInfo();
}

void SAL_CALL PluginObject::setPropertyValue(const ::rtl::OUString& aPropertyName, const uno::Any& aAny)
    throw ( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if ( aPropertyName.equalsAscii("PluginURL") )
    {
        aAny >>= maURL;
    }
    else if ( aPropertyName.equalsAscii("PluginMimeType") )
    {
        aAny >>= maMimeType;
    }
    else if ( aPropertyName.equalsAscii("PluginCommands") )
    {
        maCmdList.Clear();
        uno::Sequence < beans::PropertyValue > aCommandSequence;
        if( aAny >>= aCommandSequence )
            maCmdList.FillFromSequence( aCommandSequence );
    }
    else
         throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL PluginObject::getPropertyValue(const ::rtl::OUString& aPropertyName)
        throw ( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aAny;
    if ( aPropertyName.equalsAscii("PluginURL") )
    {
        aAny <<= maURL;
    }
    else if ( aPropertyName.equalsAscii("PluginMimeType") )
    {
        aAny <<= maMimeType;
    }
    else if ( aPropertyName.equalsAscii("PluginCommands") )
    {
        uno::Sequence< beans::PropertyValue > aCommandSequence;
        maCmdList.FillSequence( aCommandSequence );
        aAny <<= aCommandSequence;
    }
    else
         throw beans::UnknownPropertyException();
    return aAny;
}

void SAL_CALL PluginObject::addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL PluginObject::removePropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL PluginObject::addVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL PluginObject::removeVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{
}

}
