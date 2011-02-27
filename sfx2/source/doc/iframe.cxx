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

#include "iframe.hxx"
#include <sfx2/sfxdlg.hxx>
#include <sfx2/sfxsids.hrc>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <tools/urlobj.hxx>
#include <tools/debug.hxx>
#include <rtl/ustring.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <svtools/miscopt.hxx>
#include <vcl/window.hxx>

using namespace ::com::sun::star;

namespace sfx2
{

class IFrameWindow_Impl : public Window
{
    uno::Reference < frame::XFrame > mxFrame;
    sal_Bool                bActive;
    sal_Bool                bBorder;

public:
    IFrameWindow_Impl( Window *pParent,
                       sal_Bool bHasBorder,
                       WinBits nWinBits = 0 );

public:
    void            SetBorder( sal_Bool bNewBorder = sal_True );
    sal_Bool        HasBorder() const { return bBorder; }
};

IFrameWindow_Impl::IFrameWindow_Impl( Window *pParent, sal_Bool bHasBorder, WinBits nWinBits )
    : Window( pParent, nWinBits | WB_CLIPCHILDREN | WB_NODIALOGCONTROL | WB_DOCKBORDER )
    , bActive(sal_False)
    , bBorder(bHasBorder)
{
    if ( !bHasBorder )
        SetBorderStyle( WINDOW_BORDER_NOBORDER );
    else
        SetBorderStyle( WINDOW_BORDER_NORMAL );
}

void IFrameWindow_Impl::SetBorder( sal_Bool bNewBorder )
{
    if ( bBorder != bNewBorder )
    {
        Size aSize = GetSizePixel();
        bBorder = bNewBorder;
        if ( bBorder )
            SetBorderStyle( WINDOW_BORDER_NORMAL );
        else
            SetBorderStyle( WINDOW_BORDER_NOBORDER );
        if ( GetSizePixel() != aSize )
            SetSizePixel( aSize );
    }
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
    static SfxItemPropertyMapEntry aIFramePropertyMap_Impl[] =
    {
        { MAP_CHAR_LEN("FrameIsAutoBorder"),    WID_FRAME_IS_AUTO_BORDER,   &::getBooleanCppuType(), PROPERTY_UNBOUND, 0 },
        { MAP_CHAR_LEN("FrameIsAutoScroll"),    WID_FRAME_IS_AUTO_SCROLL,   &::getBooleanCppuType(), PROPERTY_UNBOUND, 0 },
        { MAP_CHAR_LEN("FrameIsBorder"),        WID_FRAME_IS_BORDER,        &::getBooleanCppuType(), PROPERTY_UNBOUND, 0 },
        { MAP_CHAR_LEN("FrameIsScrollingMode"), WID_FRAME_IS_SCROLLING_MODE, &::getBooleanCppuType(), PROPERTY_UNBOUND, 0 },
        { MAP_CHAR_LEN("FrameMarginHeight"),    WID_FRAME_MARGIN_HEIGHT,    &::getCppuType( (sal_Int32*)0 ), PROPERTY_UNBOUND, 0 },
        { MAP_CHAR_LEN("FrameMarginWidth"),     WID_FRAME_MARGIN_WIDTH,     &::getCppuType( (sal_Int32*)0 ), PROPERTY_UNBOUND, 0 },
        { MAP_CHAR_LEN("FrameName"),            WID_FRAME_NAME,             &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
        { MAP_CHAR_LEN("FrameURL"),             WID_FRAME_URL,              &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
        {0,0,0,0,0,0}
    };
    return aIFramePropertyMap_Impl;
}

SFX_IMPL_XSERVICEINFO( IFrameObject, "com.sun.star.embed.SpecialEmbeddedObject", "com.sun.star.comp.sfx2.IFrameObject" )
SFX_IMPL_SINGLEFACTORY( IFrameObject );

IFrameObject::IFrameObject( const uno::Reference < lang::XMultiServiceFactory >& rFact )
    : mxFact( rFact )
    , maPropMap( lcl_GetIFramePropertyMap_Impl() )
{
}

IFrameObject::~IFrameObject()
{
}


void SAL_CALL IFrameObject::initialize( const uno::Sequence< uno::Any >& aArguments ) throw ( uno::Exception, uno::RuntimeException )
{
    if ( aArguments.getLength() )
        aArguments[0] >>= mxObj;
}

sal_Bool SAL_CALL IFrameObject::load(
    const uno::Sequence < com::sun::star::beans::PropertyValue >& /*lDescriptor*/,
    const uno::Reference < frame::XFrame >& xFrame )
throw( uno::RuntimeException )
{
    if ( SvtMiscOptions().IsPluginsEnabled() )
    {
        DBG_ASSERT( !mxFrame.is(), "Frame already existing!" );
        Window* pParent = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
        IFrameWindow_Impl* pWin = new IFrameWindow_Impl( pParent, maFrmDescr.IsFrameBorderOn() );
        pWin->SetSizePixel( pParent->GetOutputSizePixel() );
        pWin->SetBackground();
        pWin->Show();

        uno::Reference < awt::XWindow > xWindow( pWin->GetComponentInterface(), uno::UNO_QUERY );
        xFrame->setComponent( xWindow, uno::Reference < frame::XController >() );

        // we must destroy the IFrame before the parent is destroyed
        xWindow->addEventListener( this );

        mxFrame = uno::Reference< frame::XFrame >( mxFact->createInstance( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Frame")) ),
                    uno::UNO_QUERY );
        uno::Reference < awt::XWindow > xWin( pWin->GetComponentInterface(), uno::UNO_QUERY );
        mxFrame->initialize( xWin );
        mxFrame->setName( maFrmDescr.GetName() );

        uno::Reference < frame::XFramesSupplier > xFramesSupplier( xFrame, uno::UNO_QUERY );
        if ( xFramesSupplier.is() )
            mxFrame->setCreator( xFramesSupplier );

        uno::Reference< frame::XDispatchProvider > xProv( mxFrame, uno::UNO_QUERY );

        util::URL aTargetURL;
        aTargetURL.Complete = ::rtl::OUString( maFrmDescr.GetURL().GetMainURL( INetURLObject::NO_DECODE ) );
        uno::Reference < util::XURLTransformer > xTrans( mxFact->createInstance( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.URLTransformer"))), uno::UNO_QUERY );
        xTrans->parseStrict( aTargetURL );

        uno::Sequence < beans::PropertyValue > aProps(2);
        aProps[0].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PluginMode"));
        aProps[0].Value <<= (sal_Int16) 2;
        aProps[1].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
        aProps[1].Value <<= (sal_Bool) sal_True;
        uno::Reference < frame::XDispatch > xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_self")), 0 );
        if ( xDisp.is() )
            xDisp->dispatch( aTargetURL, aProps );

        return TRUE;
    }

    return FALSE;
}

void SAL_CALL IFrameObject::cancel() throw( com::sun::star::uno::RuntimeException )
{
    try
    {
        uno::Reference < util::XCloseable > xClose( mxFrame, uno::UNO_QUERY );
        if ( xClose.is() )
            xClose->close( sal_True );
        mxFrame = 0;
    }
    catch ( uno::Exception& )
    {}
}

void SAL_CALL IFrameObject::close( sal_Bool /*bDeliverOwnership*/ ) throw( com::sun::star::util::CloseVetoException, com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL IFrameObject::addCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& ) throw( com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL IFrameObject::removeCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& ) throw( com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL IFrameObject::disposing( const com::sun::star::lang::EventObject& ) throw (com::sun::star::uno::RuntimeException)
{
    cancel();
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL IFrameObject::getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo > xInfo = new SfxItemPropertySetInfo( &maPropMap );
    return xInfo;
}

void SAL_CALL IFrameObject::setPropertyValue(const ::rtl::OUString& aPropertyName, const uno::Any& aAny)
    throw ( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    const SfxItemPropertySimpleEntry*  pEntry = maPropMap.getByName( aPropertyName );
    if( !pEntry )
         throw beans::UnknownPropertyException();
    switch( pEntry->nWID )
    {
    case WID_FRAME_URL:
    {
        ::rtl::OUString aURL;
        aAny >>= aURL;
        maFrmDescr.SetURL( String(aURL) );
    }
    break;
    case WID_FRAME_NAME:
    {
        ::rtl::OUString aName;
        if ( aAny >>= aName )
            maFrmDescr.SetName( aName );
    }
    break;
    case WID_FRAME_IS_AUTO_SCROLL:
    {
        sal_Bool bIsAutoScroll = sal_Bool();
        if ( (aAny >>= bIsAutoScroll) && bIsAutoScroll )
            maFrmDescr.SetScrollingMode( ScrollingAuto );
    }
    break;
    case WID_FRAME_IS_SCROLLING_MODE:
    {
        sal_Bool bIsScroll = sal_Bool();
        if ( aAny >>= bIsScroll )
            maFrmDescr.SetScrollingMode( bIsScroll ? ScrollingYes : ScrollingNo );
    }
    break;
    case WID_FRAME_IS_BORDER:
    {
        sal_Bool bIsBorder = sal_Bool();
        if ( aAny >>= bIsBorder )
            maFrmDescr.SetFrameBorder( bIsBorder );
    }
    break;
    case WID_FRAME_IS_AUTO_BORDER:
    {
        sal_Bool bIsAutoBorder = sal_Bool();
        if ( (aAny >>= bIsAutoBorder) )
        {
            BOOL bBorder = maFrmDescr.IsFrameBorderOn();
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
            aSize.Width() = nMargin;
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
            aSize.Height() = nMargin;
            maFrmDescr.SetMargin( aSize );
        }
    }
    break;
    default: ;
    }
}

uno::Any SAL_CALL IFrameObject::getPropertyValue(const ::rtl::OUString& aPropertyName)
        throw ( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    const SfxItemPropertySimpleEntry*  pEntry = maPropMap.getByName( aPropertyName );
    if( !pEntry )
         throw beans::UnknownPropertyException();
    uno::Any aAny;
    switch( pEntry->nWID )
    {
    case WID_FRAME_URL:
    {
        aAny <<= ::rtl::OUString( maFrmDescr.GetURL().GetMainURL( INetURLObject::NO_DECODE ) );
    }
    break;
    case WID_FRAME_NAME:
    {
        aAny <<= ::rtl::OUString( maFrmDescr.GetName() );
    }
    break;
    case WID_FRAME_IS_AUTO_SCROLL:
    {
        sal_Bool bIsAutoScroll = ( maFrmDescr.GetScrollingMode() == ScrollingAuto );
        aAny <<= bIsAutoScroll;
    }
    break;
    case WID_FRAME_IS_SCROLLING_MODE:
    {
        sal_Bool bIsScroll = ( maFrmDescr.GetScrollingMode() == ScrollingYes );
        aAny <<= bIsScroll;
    }
    break;
    case WID_FRAME_IS_BORDER:
    {
        sal_Bool bIsBorder = maFrmDescr.IsFrameBorderOn();
        aAny <<= bIsBorder;
    }
    break;
    case WID_FRAME_IS_AUTO_BORDER:
    {
        sal_Bool bIsAutoBorder = !maFrmDescr.IsFrameBorderSet();
        aAny <<= bIsAutoBorder;
    }
    break;
    case WID_FRAME_MARGIN_WIDTH:
    {
        aAny <<= (sal_Int32 ) maFrmDescr.GetMargin().Width();
    }
    break;
    case WID_FRAME_MARGIN_HEIGHT:
    {
        aAny <<= (sal_Int32 ) maFrmDescr.GetMargin().Height();
    }
    default: ;
    }
    return aAny;
}

void SAL_CALL IFrameObject::addPropertyChangeListener(const ::rtl::OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & ) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL IFrameObject::removePropertyChangeListener(const ::rtl::OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & ) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL IFrameObject::addVetoableChangeListener(const ::rtl::OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & ) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL IFrameObject::removeVetoableChangeListener(const ::rtl::OUString&, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & ) throw( ::com::sun::star::uno::RuntimeException )
{
}

::sal_Int16 SAL_CALL IFrameObject::execute() throw (::com::sun::star::uno::RuntimeException)
{
    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
    VclAbstractDialog* pDlg = pFact->CreateEditObjectDialog( NULL, SID_INSERT_FLOATINGFRAME, mxObj );
    if ( pDlg )
        pDlg->Execute();
    return 0;
}

void SAL_CALL IFrameObject::setTitle( const ::rtl::OUString& ) throw (::com::sun::star::uno::RuntimeException)
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
