/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/config.h>

#include <com/sun/star/plugin/PluginManager.hpp>
#include <com/sun/star/plugin/XPlugin.hpp>
#include <com/sun/star/plugin/XPluginManager.hpp>
#include <com/sun/star/plugin/PluginMode.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XCloseable.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <svl/itemprop.hxx>
#include <svl/ownlist.hxx>
#include <svtools/miscopt.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>

using namespace ::com::sun::star;

namespace {

class PluginWindow_Impl : public vcl::Window
{
public:
    uno::Reference < awt::XWindow > xWindow;
    explicit PluginWindow_Impl(vcl::Window* pParent)
     : Window( pParent, WB_CLIPCHILDREN )
     {}

     virtual void Resize() override;
};

void PluginWindow_Impl::Resize()
{
    Size aSize( GetOutputSizePixel() );
    if ( xWindow.is() )
        xWindow->setPosSize( 0, 0, aSize.Width(), aSize.Height(), css::awt::PosSize::SIZE );
}

#define PROPERTY_UNBOUND 0

#define WID_COMMANDS    1
#define WID_MIMETYPE    2
#define WID_URL         3
const SfxItemPropertyMapEntry* lcl_GetPluginPropertyMap_Impl()
{
    static const SfxItemPropertyMapEntry aPluginPropertyMap_Impl[] =
    {
        { OUString("PluginCommands"), WID_COMMANDS, cppu::UnoType<css::uno::Sequence< css::beans::PropertyValue >>::get(), PROPERTY_UNBOUND, 0},
        { OUString("PluginMimeType"), WID_MIMETYPE, cppu::UnoType<OUString>::get(), PROPERTY_UNBOUND, 0 },
        { OUString("PluginURL"),      WID_URL     , cppu::UnoType<OUString>::get(), PROPERTY_UNBOUND, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aPluginPropertyMap_Impl;
}

class PluginObject : public ::cppu::WeakImplHelper <
        css::util::XCloseable,
        css::lang::XEventListener,
        css::frame::XSynchronousFrameLoader,
        css::beans::XPropertySet,
        css::lang::XServiceInfo >
{
    css::uno::Reference< css::plugin::XPlugin > mxPlugin;
    SfxItemPropertyMap  maPropMap;
    SvCommandList       maCmdList;
    OUString     maURL;
    OUString     maMimeType;

public:
    PluginObject();
    virtual ~PluginObject();

    virtual sal_Bool SAL_CALL load( const css::uno::Sequence < css::beans::PropertyValue >& lDescriptor,
            const css::uno::Reference < css::frame::XFrame >& xFrame ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL cancel() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL close( sal_Bool bDeliverOwnership ) throw( css::util::CloseVetoException, css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addCloseListener( const css::uno::Reference < css::util::XCloseListener >& xListener ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeCloseListener( const css::uno::Reference < css::util::XCloseListener >& xListener ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw (css::uno::RuntimeException, std::exception) override ;
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addPropertyChangeListener(const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener > & aListener) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removePropertyChangeListener(const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener > & aListener) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString& aPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& aPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return OUString("com.sun.star.comp.sfx2.PluginObject");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    {
        css::uno::Sequence< OUString > aSeq(1);
        aSeq[0] = "com.sun.star.frame.SpecialEmbeddedObject";
        return aSeq;
    }
};

PluginObject::PluginObject()
    : maPropMap( lcl_GetPluginPropertyMap_Impl() )
{
}

PluginObject::~PluginObject()
{
}

sal_Bool SAL_CALL PluginObject::load(
    const uno::Sequence < css::beans::PropertyValue >& /*lDescriptor*/,
    const uno::Reference < frame::XFrame >& xFrame )
throw( uno::RuntimeException, std::exception )
{
    uno::Reference< plugin::XPluginManager > xPMgr( plugin::PluginManager::create(comphelper::getProcessComponentContext()) );

    if ( SvtMiscOptions().IsPluginsEnabled() )
    {
        vcl::Window* pParent = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
        VclPtr<PluginWindow_Impl> pWin = VclPtr<PluginWindow_Impl>::Create( pParent );
        pWin->SetSizePixel( pParent->GetOutputSizePixel() );
        pWin->SetBackground();
        pWin->Show();

        size_t nCount = maCmdList.size();
        uno::Sequence < OUString > aCmds( nCount ), aArgs( nCount );
        OUString *pCmds = aCmds.getArray(), *pArgs = aArgs.getArray();
        for( size_t i = 0; i < nCount; i++ )
        {
            pCmds[i] = maCmdList[ i ].GetCommand();
            pArgs[i] = maCmdList[ i ].GetArgument();
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
                xWindow->setVisible( sal_True );
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
                        uno::Any aValue = xProp->getPropertyValue("URL");
                        aValue >>= maURL;
                        aValue = xProp->getPropertyValue("TYPE");
                        aValue >>= maMimeType;
                    }
                }
            }
            catch( const uno::Exception& )
            {
            }
        }

        uno::Reference < awt::XWindow > xWindow( pWin->GetComponentInterface(), uno::UNO_QUERY );

        // we must destroy the plugin before the parent is destroyed
        xWindow->addEventListener( this );
        xFrame->setComponent( xWindow, uno::Reference < frame::XController >() );
        return mxPlugin.is();
    }

    return sal_False;
}

void SAL_CALL PluginObject::cancel() throw( css::uno::RuntimeException, std::exception )
{
    uno::Reference< lang::XComponent > xComp( mxPlugin, uno::UNO_QUERY );
    if (xComp.is())
        xComp->dispose();
    mxPlugin = 0;
}

void SAL_CALL PluginObject::close( sal_Bool /*bDeliverOwnership*/ ) throw( css::util::CloseVetoException, css::uno::RuntimeException, std::exception )
{
}

void SAL_CALL PluginObject::addCloseListener( const css::uno::Reference < css::util::XCloseListener >& ) throw( css::uno::RuntimeException, std::exception )
{
}

void SAL_CALL PluginObject::removeCloseListener( const css::uno::Reference < css::util::XCloseListener >& ) throw( css::uno::RuntimeException, std::exception )
{
}

void SAL_CALL PluginObject::disposing( const css::lang::EventObject& ) throw (css::uno::RuntimeException, std::exception)
{
    cancel();
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL PluginObject::getPropertySetInfo() throw( css::uno::RuntimeException, std::exception )
{
    static uno::Reference< beans::XPropertySetInfo > xInfo = new SfxItemPropertySetInfo( maPropMap );
    return xInfo;
}

void SAL_CALL PluginObject::setPropertyValue(const OUString& aPropertyName, const uno::Any& aAny)
    throw ( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    if ( aPropertyName == "PluginURL" )
    {
        aAny >>= maURL;
    }
    else if ( aPropertyName == "PluginMimeType" )
    {
        aAny >>= maMimeType;
    }
    else if ( aPropertyName == "PluginCommands" )
    {
        maCmdList.clear();
        uno::Sequence < beans::PropertyValue > aCommandSequence;
        if( aAny >>= aCommandSequence )
            maCmdList.FillFromSequence( aCommandSequence );
    }
    else
         throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL PluginObject::getPropertyValue(const OUString& aPropertyName)
        throw ( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    uno::Any aAny;
    if ( aPropertyName == "PluginURL" )
    {
        aAny <<= maURL;
    }
    else if ( aPropertyName == "PluginMimeType" )
    {
        aAny <<= maMimeType;
    }
    else if ( aPropertyName == "PluginCommands" )
    {
        uno::Sequence< beans::PropertyValue > aCommandSequence;
        maCmdList.FillSequence( aCommandSequence );
        aAny <<= aCommandSequence;
    }
    else
         throw beans::UnknownPropertyException();
    return aAny;
}

void SAL_CALL PluginObject::addPropertyChangeListener(const OUString&, const css::uno::Reference< css::beans::XPropertyChangeListener > & ) throw( css::uno::RuntimeException, std::exception )
{
}

void SAL_CALL PluginObject::removePropertyChangeListener(const OUString&, const css::uno::Reference< css::beans::XPropertyChangeListener > & ) throw( css::uno::RuntimeException, std::exception )
{
}

void SAL_CALL PluginObject::addVetoableChangeListener(const OUString&, const css::uno::Reference< css::beans::XVetoableChangeListener > & ) throw( css::uno::RuntimeException, std::exception )
{
}

void SAL_CALL PluginObject::removeVetoableChangeListener(const OUString&, const css::uno::Reference< css::beans::XVetoableChangeListener > & ) throw( css::uno::RuntimeException, std::exception )
{
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_sfx2_PluginObject_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new PluginObject());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
