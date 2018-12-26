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

#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/Frame.hpp>
#include <com/sun/star/frame/XFrame2.hpp>
#include <com/sun/star/frame/XSynchronousFrameLoader.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ref.hxx>
#include <svtools/miscopt.hxx>
#include <svl/ownlist.hxx>
#include <svl/itemprop.hxx>
#include <sfx2/frmdescr.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/sfxsids.hrc>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <tools/debug.hxx>

using namespace ::com::sun::star;

namespace {

class IFrameObject : public ::cppu::WeakImplHelper <
        css::util::XCloseable,
        css::lang::XEventListener,
        css::frame::XSynchronousFrameLoader,
        css::ui::dialogs::XExecutableDialog,
        css::lang::XServiceInfo,
        css::beans::XPropertySet >
{
    css::uno::Reference < css::uno::XComponentContext > mxContext;
    css::uno::Reference < css::frame::XFrame2 > mxFrame;
    css::uno::Reference < css::embed::XEmbeddedObject > mxObj;
    SfxItemPropertyMap const  maPropMap;
    SfxFrameDescriptor  maFrmDescr;

public:
    /// @throws css::uno::Exception
    /// @throws css::uno::RuntimeException
    IFrameObject(const css::uno::Reference < css::uno::XComponentContext>& rxContext, const css::uno::Sequence< css::uno::Any >& aArguments);

    virtual OUString SAL_CALL getImplementationName() override
    {
        return OUString("com.sun.star.comp.sfx2.IFrameObject");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        css::uno::Sequence< OUString > aSeq { "com.sun.star.frame.SpecialEmbeddedObject" };
        return aSeq;
    }

    virtual sal_Bool SAL_CALL load( const css::uno::Sequence < css::beans::PropertyValue >& lDescriptor,
            const css::uno::Reference < css::frame::XFrame >& xFrame ) override;
    virtual void SAL_CALL cancel() override;
    virtual void SAL_CALL close( sal_Bool bDeliverOwnership ) override;
    virtual void SAL_CALL addCloseListener( const css::uno::Reference < css::util::XCloseListener >& xListener ) override;
    virtual void SAL_CALL removeCloseListener( const css::uno::Reference < css::util::XCloseListener >& xListener ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) override ;
    virtual void SAL_CALL setTitle( const OUString& aTitle ) override;
    virtual ::sal_Int16 SAL_CALL execute(  ) override;
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL addPropertyChangeListener(const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener > & aListener) override;
    virtual void SAL_CALL removePropertyChangeListener(const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener > & aListener) override;
    virtual void SAL_CALL addVetoableChangeListener(const OUString& aPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(const OUString& aPropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener > & aListener) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
};

class IFrameWindow_Impl : public vcl::Window
{
public:
    IFrameWindow_Impl( vcl::Window *pParent, bool bHasBorder );
};

IFrameWindow_Impl::IFrameWindow_Impl( vcl::Window *pParent, bool bHasBorder )
    : Window( pParent, WB_CLIPCHILDREN | WB_NODIALOGCONTROL )
{
    if ( !bHasBorder )
        SetBorderStyle( WindowBorderStyle::NOBORDER );
    else
        SetBorderStyle( WindowBorderStyle::NORMAL );
}

#define PROPERTY_UNBOUND 0

#define WID_FRAME_URL                   1
#define WID_FRAME_NAME                  2
#define WID_FRAME_IS_AUTO_SCROLL        3
#define WID_FRAME_IS_SCROLLING_MODE     4
#define WID_FRAME_IS_BORDER             5
#define WID_FRAME_IS_AUTO_BORDER        6
#define WID_FRAME_MARGIN_WIDTH          7
#define WID_FRAME_MARGIN_HEIGHT         8

const SfxItemPropertyMapEntry* lcl_GetIFramePropertyMap_Impl()
{
    static const SfxItemPropertyMapEntry aIFramePropertyMap_Impl[] =
    {
        { OUString("FrameIsAutoBorder"),    WID_FRAME_IS_AUTO_BORDER,   cppu::UnoType<bool>::get(), PROPERTY_UNBOUND, 0 },
        { OUString("FrameIsAutoScroll"),    WID_FRAME_IS_AUTO_SCROLL,   cppu::UnoType<bool>::get(), PROPERTY_UNBOUND, 0 },
        { OUString("FrameIsBorder"),        WID_FRAME_IS_BORDER,        cppu::UnoType<bool>::get(), PROPERTY_UNBOUND, 0 },
        { OUString("FrameIsScrollingMode"), WID_FRAME_IS_SCROLLING_MODE,cppu::UnoType<bool>::get(), PROPERTY_UNBOUND, 0 },
        { OUString("FrameMarginHeight"),    WID_FRAME_MARGIN_HEIGHT,    cppu::UnoType<sal_Int32>::get(), PROPERTY_UNBOUND, 0 },
        { OUString("FrameMarginWidth"),     WID_FRAME_MARGIN_WIDTH,     cppu::UnoType<sal_Int32>::get(), PROPERTY_UNBOUND, 0 },
        { OUString("FrameName"),            WID_FRAME_NAME,             cppu::UnoType<OUString>::get(), PROPERTY_UNBOUND, 0 },
        { OUString("FrameURL"),             WID_FRAME_URL,              cppu::UnoType<OUString>::get(), PROPERTY_UNBOUND, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aIFramePropertyMap_Impl;
}

IFrameObject::IFrameObject(const uno::Reference < uno::XComponentContext >& rxContext, const css::uno::Sequence< css::uno::Any >& aArguments)
    : mxContext( rxContext )
    , maPropMap( lcl_GetIFramePropertyMap_Impl() )
{
    if ( aArguments.getLength() )
        aArguments[0] >>= mxObj;
}

sal_Bool SAL_CALL IFrameObject::load(
    const uno::Sequence < css::beans::PropertyValue >& /*lDescriptor*/,
    const uno::Reference < frame::XFrame >& xFrame )
{
    if ( SvtMiscOptions().IsPluginsEnabled() )
    {
        DBG_ASSERT( !mxFrame.is(), "Frame already existing!" );
        VclPtr<vcl::Window> pParent = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
        VclPtr<IFrameWindow_Impl> pWin = VclPtr<IFrameWindow_Impl>::Create( pParent, maFrmDescr.IsFrameBorderOn() );
        pWin->SetSizePixel( pParent->GetOutputSizePixel() );
        pWin->SetBackground();
        pWin->Show();

        uno::Reference < awt::XWindow > xWindow( pWin->GetComponentInterface(), uno::UNO_QUERY );
        xFrame->setComponent( xWindow, uno::Reference < frame::XController >() );

        // we must destroy the IFrame before the parent is destroyed
        xWindow->addEventListener( this );

        mxFrame = frame::Frame::create( mxContext );
        uno::Reference < awt::XWindow > xWin( pWin->GetComponentInterface(), uno::UNO_QUERY );
        mxFrame->initialize( xWin );
        mxFrame->setName( maFrmDescr.GetName() );

        uno::Reference < frame::XFramesSupplier > xFramesSupplier( xFrame, uno::UNO_QUERY );
        if ( xFramesSupplier.is() )
            mxFrame->setCreator( xFramesSupplier );

        util::URL aTargetURL;
        aTargetURL.Complete = maFrmDescr.GetURL().GetMainURL( INetURLObject::DecodeMechanism::NONE );
        uno::Reference < util::XURLTransformer > xTrans( util::URLTransformer::create( mxContext ) );
        xTrans->parseStrict( aTargetURL );

        uno::Sequence < beans::PropertyValue > aProps(2);
        aProps[0].Name = "PluginMode";
        aProps[0].Value <<= sal_Int16(2);
        aProps[1].Name = "ReadOnly";
        aProps[1].Value <<= true;
        uno::Reference < frame::XDispatch > xDisp = mxFrame->queryDispatch( aTargetURL, "_self", 0 );
        if ( xDisp.is() )
            xDisp->dispatch( aTargetURL, aProps );

        return true;
    }

    return false;
}

void SAL_CALL IFrameObject::cancel()
{
    try
    {
        uno::Reference < util::XCloseable > xClose( mxFrame, uno::UNO_QUERY );
        if ( xClose.is() )
            xClose->close( true );
        mxFrame = nullptr;
    }
    catch (const uno::Exception&)
    {
    }
}

void SAL_CALL IFrameObject::close( sal_Bool /*bDeliverOwnership*/ )
{
}

void SAL_CALL IFrameObject::addCloseListener( const css::uno::Reference < css::util::XCloseListener >& )
{
}

void SAL_CALL IFrameObject::removeCloseListener( const css::uno::Reference < css::util::XCloseListener >& )
{
}

void SAL_CALL IFrameObject::disposing( const css::lang::EventObject& )
{
    cancel();
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL IFrameObject::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xInfo = new SfxItemPropertySetInfo( maPropMap );
    return xInfo;
}

void SAL_CALL IFrameObject::setPropertyValue(const OUString& aPropertyName, const uno::Any& aAny)
{
    const SfxItemPropertySimpleEntry*  pEntry = maPropMap.getByName( aPropertyName );
    if( !pEntry )
         throw beans::UnknownPropertyException();
    switch( pEntry->nWID )
    {
    case WID_FRAME_URL:
    {
        OUString aURL;
        aAny >>= aURL;
        maFrmDescr.SetURL( aURL );
    }
    break;
    case WID_FRAME_NAME:
    {
        OUString aName;
        if ( aAny >>= aName )
            maFrmDescr.SetName( aName );
    }
    break;
    case WID_FRAME_IS_AUTO_SCROLL:
    {
        bool bIsAutoScroll;
        if ( (aAny >>= bIsAutoScroll) && bIsAutoScroll )
            maFrmDescr.SetScrollingMode( ScrollingMode::Auto );
    }
    break;
    case WID_FRAME_IS_SCROLLING_MODE:
    {
        bool bIsScroll;
        if ( aAny >>= bIsScroll )
            maFrmDescr.SetScrollingMode( bIsScroll ? ScrollingMode::Yes : ScrollingMode::No );
    }
    break;
    case WID_FRAME_IS_BORDER:
    {
        bool bIsBorder;
        if ( aAny >>= bIsBorder )
            maFrmDescr.SetFrameBorder( bIsBorder );
    }
    break;
    case WID_FRAME_IS_AUTO_BORDER:
    {
        bool bIsAutoBorder;
        if ( aAny >>= bIsAutoBorder )
        {
            bool bBorder = maFrmDescr.IsFrameBorderOn();
            maFrmDescr.ResetBorder();
            if ( bIsAutoBorder )
                maFrmDescr.SetFrameBorder( bBorder );
        }
    }
    break;
    case WID_FRAME_MARGIN_WIDTH:
    {
        sal_Int32 nMargin = 0;
        Size aSize = maFrmDescr.GetMargin();
        if ( aAny >>= nMargin )
        {
            aSize.setWidth( nMargin );
            maFrmDescr.SetMargin( aSize );
        }
    }
    break;
    case WID_FRAME_MARGIN_HEIGHT:
    {
        sal_Int32 nMargin = 0;
        Size aSize = maFrmDescr.GetMargin();
        if ( aAny >>= nMargin )
        {
            aSize.setHeight( nMargin );
            maFrmDescr.SetMargin( aSize );
        }
    }
    break;
    default: ;
    }
}

uno::Any SAL_CALL IFrameObject::getPropertyValue(const OUString& aPropertyName)
{
    const SfxItemPropertySimpleEntry*  pEntry = maPropMap.getByName( aPropertyName );
    if( !pEntry )
         throw beans::UnknownPropertyException();
    uno::Any aAny;
    switch( pEntry->nWID )
    {
    case WID_FRAME_URL:
    {
        aAny <<= maFrmDescr.GetURL().GetMainURL( INetURLObject::DecodeMechanism::NONE );
    }
    break;
    case WID_FRAME_NAME:
    {
        aAny <<= maFrmDescr.GetName();
    }
    break;
    case WID_FRAME_IS_AUTO_SCROLL:
    {
        bool bIsAutoScroll = ( maFrmDescr.GetScrollingMode() == ScrollingMode::Auto );
        aAny <<= bIsAutoScroll;
    }
    break;
    case WID_FRAME_IS_SCROLLING_MODE:
    {
        bool bIsScroll = ( maFrmDescr.GetScrollingMode() == ScrollingMode::Yes );
        aAny <<= bIsScroll;
    }
    break;
    case WID_FRAME_IS_BORDER:
    {
        bool bIsBorder = maFrmDescr.IsFrameBorderOn();
        aAny <<= bIsBorder;
    }
    break;
    case WID_FRAME_IS_AUTO_BORDER:
    {
        bool bIsAutoBorder = !maFrmDescr.IsFrameBorderSet();
        aAny <<= bIsAutoBorder;
    }
    break;
    case WID_FRAME_MARGIN_WIDTH:
    {
        aAny <<= static_cast<sal_Int32>(maFrmDescr.GetMargin().Width());
    }
    break;
    case WID_FRAME_MARGIN_HEIGHT:
    {
        aAny <<= static_cast<sal_Int32>(maFrmDescr.GetMargin().Height());
    }
    break;
    default: ;
    }
    return aAny;
}

void SAL_CALL IFrameObject::addPropertyChangeListener(const OUString&, const css::uno::Reference< css::beans::XPropertyChangeListener > & )
{
}

void SAL_CALL IFrameObject::removePropertyChangeListener(const OUString&, const css::uno::Reference< css::beans::XPropertyChangeListener > & )
{
}

void SAL_CALL IFrameObject::addVetoableChangeListener(const OUString&, const css::uno::Reference< css::beans::XVetoableChangeListener > & )
{
}

void SAL_CALL IFrameObject::removeVetoableChangeListener(const OUString&, const css::uno::Reference< css::beans::XVetoableChangeListener > & )
{
}

::sal_Int16 SAL_CALL IFrameObject::execute()
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    //we really should set a parent here
    ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateEditObjectDialog(nullptr, ".uno:InsertObjectFloatingFrame", mxObj));
    pDlg->Execute();
    return 0;
}

void SAL_CALL IFrameObject::setTitle( const OUString& )
{
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_sfx2_IFrameObject_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &arguments)
{
    return cppu::acquire(new IFrameObject(context, arguments));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
