/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: applet.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:36:13 $
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

#include "applet.hxx"
#include "sfxdlg.hxx"
#include "sfxsids.hrc"

#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <sj2/sjapplet.hxx>
#include <vcl/syschild.hxx>
#include <rtl/ustring.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/javaoptions.hxx>
#include <svtools/miscopt.hxx>

using namespace ::com::sun::star;

namespace sfx2
{
class AppletWindow_Impl : public SystemChildWindow
{
public:
    SjApplet2* pApplet;
    AppletWindow_Impl( Window* pParent, SjApplet2* pApp )
     : SystemChildWindow( pParent, WB_CLIPCHILDREN )
     , pApplet(pApp)
     {}

     virtual void Resize();
};

void AppletWindow_Impl::Resize()
{
    Size aSize( GetOutputSizePixel() );
    if ( pApplet )
        pApplet->setSizePixel( aSize );
}

class AppletWrapper_Impl : public SjApplet2
{
    virtual void appletResize( const Size & );
    virtual void showDocument( const INetURLObject &, const XubString & );
    virtual void showStatus( const XubString & );
};

void AppletWrapper_Impl::appletResize( const Size & ) {}
void AppletWrapper_Impl::showDocument( const INetURLObject &, const XubString & ) {}
void AppletWrapper_Impl::showStatus( const XubString & ) {}

#define PROPERTY_UNBOUND 0
#define PROPERTY_MAYBEVOID ::com::sun::star::beans::PropertyAttribute::MAYBEVOID

SfxItemPropertyMap aAppletPropertyMap_Impl[] =
{
    { "AppletCode"    , 10, 1, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "AppletCodeBase", 14, 2, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "AppletCommands", 14, 3, &::getCppuType((::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >*)0), PROPERTY_UNBOUND, 0},
    { "AppletDocBase",  13, 4, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "AppletIsScript", 14, 5, &::getBooleanCppuType(), PROPERTY_UNBOUND, 0 },
    { "AppletName"    , 10, 6, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    {0,0,0,0,0}
};

SFX_IMPL_XSERVICEINFO( AppletObject, "com.sun.star.embed.SpecialEmbeddedObject", "com.sun.star.comp.sfx2.AppletObject" )
SFX_IMPL_SINGLEFACTORY( AppletObject );

AppletObject::AppletObject( const uno::Reference < lang::XMultiServiceFactory >& rFact )
    : mxFact( rFact )
    , maPropSet( aAppletPropertyMap_Impl )
    , mbMayScript( FALSE )
    , mpApplet( NULL )
{
}

AppletObject::~AppletObject()
{
}

void SAL_CALL AppletObject::initialize( const uno::Sequence< uno::Any >& aArguments ) throw ( uno::Exception, uno::RuntimeException )
{
    if ( aArguments.getLength() )
        aArguments[0] >>= mxObj;
}

sal_Bool SAL_CALL AppletObject::load( const uno::Sequence < com::sun::star::beans::PropertyValue >& lDescriptor,
            const uno::Reference < frame::XFrame >& xFrame ) throw( uno::RuntimeException )
{
    if ( SvtJavaOptions().IsExecuteApplets() && SvtMiscOptions().IsPluginsEnabled() )
    {
        mpApplet = new AppletWrapper_Impl;

        Window* pParent = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
        Window* pWin = new AppletWindow_Impl( pParent, mpApplet );
        pWin->SetBackground();
        pWin->Show();

    //      aCmdList.Append( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "width" ) ), String( aPosSize.GetWidth() ) );
    //      aCmdList.Append( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "height" ) ), String( aPosSize.GetHeight() ) );

        if( maName.getLength() )
            maCmdList.Append( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "name" ) ), maName );

        if( maCodeBase.getLength() )
        {
            for ( sal_Int32 nParams=0; nParams<maCmdList.Count(); nParams++ )
            {
                if ( maCmdList[nParams].GetCommand().EqualsAscii("codebase") )
                {
                    maCmdList.Remove(nParams);
                    break;
                }
            }

            maCmdList.Append( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "codebase" ) ), maCodeBase );
        }

        if( maClass.getLength() )
            maCmdList.Append( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "code" ) ), maClass );

        if( mbMayScript )
            maCmdList.Append( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "mayscript" ) ), String() );

        INetURLObject aDocBase( maDocBase );
        mpApplet->Init( pWin, aDocBase, maCmdList );
        uno::Reference < awt::XWindow > xWindow( pWin->GetComponentInterface(), uno::UNO_QUERY );

        // we must destroy the applet before the parent is destroyed
        xWindow->addEventListener( this );

        xFrame->setComponent( xWindow, uno::Reference < frame::XController >() );
        return TRUE;
    }

    return FALSE;
}

void SAL_CALL AppletObject::cancel() throw( com::sun::star::uno::RuntimeException )
{
    if ( mpApplet )
    {
        mpApplet->appletClose(); // reparenting window
        DELETEZ( mpApplet );
    }
}

void SAL_CALL AppletObject::close( sal_Bool bDeliverOwnership ) throw( com::sun::star::util::CloseVetoException, com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL AppletObject::addCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& xListener ) throw( com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL AppletObject::removeCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& xListener ) throw( com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL AppletObject::disposing( const com::sun::star::lang::EventObject& aEvent ) throw (com::sun::star::uno::RuntimeException)
{
    cancel();
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL AppletObject::getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException )
{
    return maPropSet.getPropertySetInfo();
}

void SAL_CALL AppletObject::setPropertyValue(const ::rtl::OUString& aPropertyName, const uno::Any& aAny)
    throw ( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if ( aPropertyName.equalsAscii("AppletCode") )
    {
        aAny >>= maClass;
    }
    else if ( aPropertyName.equalsAscii("AppletCodeBase") )
    {
        //pImpl->aCodeBase = rURL.GetMainURL( INetURLObject::NO_DECODE );
        //if( rURL.GetProtocol() == INET_PROT_FILE
        //    && pImpl->aCodeBase.GetChar( 9 ) == INET_ENC_DELIM_TOKEN )
        //    // Laufwerksbuchstabe auf ':' patchen
        //    pImpl->aCodeBase.SetChar( 9, INET_DELIM_TOKEN );

        aAny >>= maCodeBase;
    }
    else if ( aPropertyName.equalsAscii("AppletCommands") )
    {
        maCmdList.Clear();
        uno::Sequence < beans::PropertyValue > aCommandSequence;
        if( aAny >>= aCommandSequence )
            maCmdList.FillFromSequence( aCommandSequence );
    }
    else if ( aPropertyName.equalsAscii("AppletIsScript") )
    {
        aAny >>= mbMayScript;
    }
    else if ( aPropertyName.equalsAscii("AppletName") )
    {
        aAny >>= maName;
    }
    else if ( aPropertyName.equalsAscii("AppletDocBase") )
    {
        aAny >>= maDocBase;
    }
    else
         throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL AppletObject::getPropertyValue(const ::rtl::OUString& aPropertyName) throw ( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aAny;
    if ( aPropertyName.equalsAscii("AppletCode") )
    {
        aAny <<= maClass;
    }
    else if ( aPropertyName.equalsAscii("AppletCodeBase") )
    {
        aAny <<= maCodeBase;
    }
    else if ( aPropertyName.equalsAscii("AppletCommands") )
    {
        uno::Sequence< beans::PropertyValue > aCommandSequence;
        maCmdList.FillSequence( aCommandSequence );
        aAny <<= aCommandSequence;
    }
    else if ( aPropertyName.equalsAscii("AppletIsScript") )
    {
        aAny <<= mbMayScript;
    }
    else if ( aPropertyName.equalsAscii("AppletName") )
    {
         aAny <<= maName;
    }
    else
         throw beans::UnknownPropertyException();
    return aAny;
}

void SAL_CALL AppletObject::addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL AppletObject::removePropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL AppletObject::addVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL AppletObject::removeVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{
}

::sal_Int16 SAL_CALL AppletObject::execute() throw (::com::sun::star::uno::RuntimeException)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    uno::Reference < beans::XPropertySet > xSet( this );
    VclAbstractDialog* pDlg = pFact->CreateEditObjectDialog( NULL, SID_INSERT_APPLET, mxObj );
    if ( pDlg )
        pDlg->Execute();
    return 0;
}

void SAL_CALL AppletObject::setTitle( const ::rtl::OUString& aTitle ) throw (::com::sun::star::uno::RuntimeException)
{
}

}
