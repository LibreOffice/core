/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include "applet.hxx"
#include <sfx2/sfxdlg.hxx>
#include <sfx2/sfxsids.hrc>

#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/factory.hxx"
#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <sj2/sjapplet.hxx>
#include <vcl/syschild.hxx>
#include <rtl/ustring.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/javaoptions.hxx>
#include <svtools/miscopt.hxx>
#include <comphelper/TypeGeneration.hxx>

using namespace ::com::sun::star;
using namespace ::comphelper;

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

#define WID_APPLET_CODE                 1
#define WID_APPLET_CODEBASE             2
#define WID_APPLET_COMMANDS             3
#define WID_APPLET_DOCBASE              4
#define WID_APPLET_ISSCRIPT             5
#define WID_APPLET_NAME                 6
const SfxItemPropertyMapEntry* lcl_GetAppletPropertyMap_Impl()
{
    static SfxItemPropertyMapEntry aAppletPropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("AppletCode")    , WID_APPLET_CODE     , CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_UNBOUND, 0 },
        { MAP_CHAR_LEN("AppletCodeBase"), WID_APPLET_CODEBASE , CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_UNBOUND, 0 },
        { MAP_CHAR_LEN("AppletCommands"), WID_APPLET_COMMANDS , CPPU_E2T(CPPUTYPE_PROPERTYVALUE), PROPERTY_UNBOUND, 0 },
        { MAP_CHAR_LEN("AppletDocBase"),  WID_APPLET_DOCBASE  , CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_UNBOUND, 0 },
        { MAP_CHAR_LEN("AppletIsScript"), WID_APPLET_ISSCRIPT , CPPU_E2T(CPPUTYPE_BOOLEAN), PROPERTY_UNBOUND, 0 },
        { MAP_CHAR_LEN("AppletName")    , WID_APPLET_NAME     , CPPU_E2T(CPPUTYPE_OUSTRING), PROPERTY_UNBOUND, 0 },
        {0,0,0,0,0,0}
    };
return aAppletPropertyMap_Impl;
}

::rtl::OUString AppletObject::getImplementationName()
    throw( ::com::sun::star::uno::RuntimeException )
{
    return impl_getStaticImplementationName();
}

::sal_Bool AppletObject::supportsService( const ::rtl::OUString& sServiceName )
    throw( ::com::sun::star::uno::RuntimeException )
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > seqServiceNames =
          getSupportedServiceNames();
    const ::rtl::OUString* pArray = seqServiceNames.getConstArray();
    for ( ::sal_Int32 nCounter=0; nCounter<seqServiceNames.getLength();
          nCounter++ )
    {
        if ( pArray[nCounter] == sServiceName )
        {
            return sal_True ;
        }
    }
    return sal_False ;
}

::com::sun::star::uno::Sequence< ::rtl::OUString >
AppletObject::getSupportedServiceNames()
    throw( ::com::sun::star::uno::RuntimeException )
{
    return impl_getStaticSupportedServiceNames();
}

::com::sun::star::uno::Sequence< ::rtl::OUString >
AppletObject::impl_getStaticSupportedServiceNames()
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.embed.SpecialEmbeddedObject" ));
    return seqServiceNames ;
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
AppletObject::impl_createInstance(
    const ::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XComponentContext >& xContext )
    throw( ::com::sun::star::uno::Exception )
{
    return static_cast< ::cppu::OWeakObject * >( new AppletObject( xContext ) );
}

::rtl::OUString AppletObject::impl_getStaticImplementationName()
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.sfx2.AppletObject" ));
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
AppletObject::impl_createFactory()
{
    return uno::Reference< uno::XInterface >(
        cppu::createSingleComponentFactory(
            impl_createInstance, impl_getStaticImplementationName(),
            impl_getStaticSupportedServiceNames() ),
        uno::UNO_QUERY_THROW );
}

AppletObject::AppletObject(
    const uno::Reference < uno::XComponentContext >& rContext )
    : mxContext( rContext )
    , maPropMap( lcl_GetAppletPropertyMap_Impl() )
    , mpApplet( NULL )
    , mbMayScript( FALSE )
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

sal_Bool SAL_CALL AppletObject::load(
    const uno::Sequence < com::sun::star::beans::PropertyValue >& /*lDescriptor*/,
    const uno::Reference < frame::XFrame >& xFrame )
throw( uno::RuntimeException )
{
    if ( SvtJavaOptions().IsExecuteApplets() && SvtMiscOptions().IsPluginsEnabled() )
    {
        mpApplet = new AppletWrapper_Impl;

        Window* pParent = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
        Window* pWin = new AppletWindow_Impl( pParent, mpApplet );
        pWin->SetBackground();
        pWin->Show();

        if( maName.getLength() )
            maCmdList.Append( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "name" ) ), maName );

        if( maCodeBase.getLength() )
        {
            for ( sal_uInt32 nParams=0; nParams<maCmdList.Count(); nParams++ )
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
        mpApplet->Init( mxContext, pWin, aDocBase, maCmdList );
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

void SAL_CALL AppletObject::close( sal_Bool /*bDeliverOwnership*/ ) throw( com::sun::star::util::CloseVetoException, com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL AppletObject::addCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& ) throw( com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL AppletObject::removeCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& ) throw( com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL AppletObject::disposing( const com::sun::star::lang::EventObject& ) throw (com::sun::star::uno::RuntimeException)
{
    cancel();
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL AppletObject::getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo > xInfo = new SfxItemPropertySetInfo( &maPropMap );
    return xInfo;
}

void SAL_CALL AppletObject::setPropertyValue(const ::rtl::OUString& aPropertyName, const uno::Any& aAny)
    throw ( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{

    const SfxItemPropertySimpleEntry*  pEntry = maPropMap.getByName( aPropertyName );
    if( !pEntry )
         throw beans::UnknownPropertyException();
    switch( pEntry->nWID )
    {
        case WID_APPLET_CODE      :
            aAny >>= maClass;
        break;
        case WID_APPLET_CODEBASE  :
            aAny >>= maCodeBase;
        break;
        case WID_APPLET_COMMANDS  :
        {
            maCmdList.Clear();
            uno::Sequence < beans::PropertyValue > aCommandSequence;
            if( aAny >>= aCommandSequence )
                maCmdList.FillFromSequence( aCommandSequence );
        }
        break;
        case WID_APPLET_DOCBASE   :
            aAny >>= maDocBase;
        break;
        case WID_APPLET_ISSCRIPT  :
            aAny >>= mbMayScript;
        break;
        case WID_APPLET_NAME      :
            aAny >>= maName;
        break;
        default:;

    }
}

uno::Any SAL_CALL AppletObject::getPropertyValue(const ::rtl::OUString& aPropertyName) throw ( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aAny;
    const SfxItemPropertySimpleEntry*  pEntry = maPropMap.getByName( aPropertyName );
    if( !pEntry )
         throw beans::UnknownPropertyException();
    switch( pEntry->nWID )
    {
        case WID_APPLET_CODE      :
        aAny <<= maClass;
        break;
        case WID_APPLET_CODEBASE  :
        aAny <<= maCodeBase;
        break;
        case WID_APPLET_COMMANDS  :
        {
            uno::Sequence< beans::PropertyValue > aCommandSequence;
            maCmdList.FillSequence( aCommandSequence );
            aAny <<= aCommandSequence;
        }
        break;
        case WID_APPLET_DOCBASE   :
        break;
        case WID_APPLET_ISSCRIPT  :
            aAny <<= mbMayScript;
        break;
        case WID_APPLET_NAME      :
            aAny <<= maName;
        break;
        default:;

    }
    return aAny;
}

void SAL_CALL AppletObject::addPropertyChangeListener(const ::rtl::OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & ) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL AppletObject::removePropertyChangeListener(const ::rtl::OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & ) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL AppletObject::addVetoableChangeListener(const ::rtl::OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & ) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL AppletObject::removeVetoableChangeListener(const ::rtl::OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & ) throw( ::com::sun::star::uno::RuntimeException )
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

void SAL_CALL AppletObject::setTitle( const ::rtl::OUString& ) throw (::com::sun::star::uno::RuntimeException)
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
