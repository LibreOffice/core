/*************************************************************************
 *
 *  $RCSfile: iframe.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-12-13 12:53:05 $
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

#include "iframe.hxx"
#include "sfxdlg.hxx"
#include "sfxsids.hrc"

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

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
    //SetActivateMode( ACTIVATE_MODE_GRABFOCUS );
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

SfxItemPropertyMap aIFramePropertyMap_Impl[] =
{
    { "FrameIsAutoBorder",    17, 1, &::getBooleanCppuType(), PROPERTY_UNBOUND, 0 },
    { "FrameIsAutoScroll",    17, 2, &::getBooleanCppuType(), PROPERTY_UNBOUND, 0 },
    { "FrameIsBorder",        13, 3, &::getBooleanCppuType(), PROPERTY_UNBOUND, 0 },
    { "FrameIsScrollingMode", 20, 4, &::getBooleanCppuType(), PROPERTY_UNBOUND, 0 },
    { "FrameMarginHeight",    17, 5, &::getCppuType( (sal_Int32*)0 ), PROPERTY_UNBOUND, 0 },
    { "FrameMarginWidth",     16, 6, &::getCppuType( (sal_Int32*)0 ), PROPERTY_UNBOUND, 0 },
    { "FrameName",             9, 7, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    { "FrameURL",              8, 8, &::getCppuType((const ::rtl::OUString*)0), PROPERTY_UNBOUND, 0 },
    {0,0,0,0,0}
};

SFX_IMPL_XSERVICEINFO( IFrameObject, "com.sun.star.embed.SpecialEmbeddedObject", "com.sun.star.comp.sfx2.IFrameObject" )
SFX_IMPL_SINGLEFACTORY( IFrameObject );

IFrameObject::IFrameObject( const uno::Reference < lang::XMultiServiceFactory >& rFact )
    : mxFact( rFact )
    , maPropSet( aIFramePropertyMap_Impl )
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

sal_Bool SAL_CALL IFrameObject::load( const uno::Sequence < com::sun::star::beans::PropertyValue >& lDescriptor,
            const uno::Reference < frame::XFrame >& xFrame ) throw( uno::RuntimeException )
{
    if ( SvtMiscOptions().IsPluginsEnabled() )
    {
        uno::Reference < util::XCloseable > xClose( xFrame, uno::UNO_QUERY );
        if ( xClose.is() )
            xClose->addCloseListener( static_cast < util::XCloseListener* >(this) );
        else
            xFrame->addEventListener( static_cast < lang::XEventListener* >(this) );

        DBG_ASSERT( !mxFrame.is(), "Frame already existing!" );
        Window* pParent = VCLUnoHelper::GetWindow( xFrame->getContainerWindow() );
        IFrameWindow_Impl* pWin = new IFrameWindow_Impl( pParent, maFrmDescr.IsFrameBorderOn() );
        pWin->SetSizePixel( pParent->GetOutputSizePixel() );
        pWin->SetBackground();
        pWin->Show();

        uno::Reference < awt::XWindow > xWindow( pWin->GetComponentInterface(), uno::UNO_QUERY );
        xFrame->setComponent( xWindow, uno::Reference < frame::XController >() );

        mxFrame = uno::Reference< frame::XFrame >( mxFact->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.frame.Frame" ) ),
                    uno::UNO_QUERY );
        uno::Reference < awt::XWindow > xWin( pWin->GetComponentInterface(), uno::UNO_QUERY );
        mxFrame->initialize( xWin );
        mxFrame->setName( maFrmDescr.GetName() );

        uno::Reference< frame::XDispatchProvider > xProv( mxFrame, uno::UNO_QUERY );

        util::URL aTargetURL;
        aTargetURL.Complete = ::rtl::OUString( maFrmDescr.GetURL().GetMainURL( INetURLObject::NO_DECODE ) );
        uno::Reference < util::XURLTransformer > xTrans( mxFact->createInstance( rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer" )), uno::UNO_QUERY );
        xTrans->parseStrict( aTargetURL );

        uno::Sequence < beans::PropertyValue > aProps(2);
        aProps[0].Name = ::rtl::OUString::createFromAscii("PluginMode");
        aProps[0].Value <<= (sal_Int16) 2;
        aProps[1].Name = ::rtl::OUString::createFromAscii("ReadOnly");
        aProps[1].Value <<= (sal_Bool) sal_True;
        uno::Reference < frame::XDispatch > xDisp = xProv->queryDispatch( aTargetURL, ::rtl::OUString::createFromAscii("_self"), 0 );
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

void SAL_CALL IFrameObject::close( sal_Bool bDeliverOwnership ) throw( com::sun::star::util::CloseVetoException, com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL IFrameObject::addCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& xListener ) throw( com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL IFrameObject::removeCloseListener( const com::sun::star::uno::Reference < com::sun::star::util::XCloseListener >& xListener ) throw( com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL IFrameObject::queryClosing( const com::sun::star::lang::EventObject& aEvent, sal_Bool bDeliverOwnership ) throw (com::sun::star::uno::RuntimeException)
{
    cancel();
}

void SAL_CALL IFrameObject::notifyClosing( const com::sun::star::lang::EventObject& aEvent ) throw (com::sun::star::uno::RuntimeException)
{
    cancel();
}

void SAL_CALL IFrameObject::disposing( const com::sun::star::lang::EventObject& aEvent ) throw (com::sun::star::uno::RuntimeException)
{
    cancel();
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL IFrameObject::getPropertySetInfo() throw( ::com::sun::star::uno::RuntimeException )
{
    return maPropSet.getPropertySetInfo();
}

void SAL_CALL IFrameObject::setPropertyValue(const ::rtl::OUString& aPropertyName, const uno::Any& aAny)
    throw ( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    if ( aPropertyName.equalsAscii("FrameURL") )
    {
        ::rtl::OUString aURL;
        aAny >>= aURL;
        maFrmDescr.SetURL( String(aURL) );
    }
    else if ( aPropertyName.equalsAscii("FrameName") )
    {
        ::rtl::OUString aName;
        if ( aAny >>= aName )
            maFrmDescr.SetName( aName );
    }
    else if ( aPropertyName.equalsAscii("FrameIsAutoScroll") )
    {
        sal_Bool bIsAutoScroll;
        if ( (aAny >>= bIsAutoScroll) && bIsAutoScroll )
            maFrmDescr.SetScrollingMode( ScrollingAuto );
    }
    else if ( aPropertyName.equalsAscii("FrameIsScrollingMode") )
    {
        sal_Bool bIsScroll;
        if ( aAny >>= bIsScroll )
            maFrmDescr.SetScrollingMode( bIsScroll ? ScrollingYes : ScrollingNo );
    }
    else if ( aPropertyName.equalsAscii("FrameIsBorder") )
    {
        sal_Bool bIsBorder;
        if ( aAny >>= bIsBorder )
            maFrmDescr.SetFrameBorder( bIsBorder );
    }
    else if ( aPropertyName.equalsAscii("FrameIsAutoBorder") )
    {
        sal_Bool bIsAutoBorder;
        if ( (aAny >>= bIsAutoBorder) )
        {
            BOOL bBorder = maFrmDescr.IsFrameBorderOn();
            maFrmDescr.ResetBorder();
            if ( bIsAutoBorder )
                maFrmDescr.SetFrameBorder( bBorder );
        }
    }
    else if ( aPropertyName.equalsAscii("FrameMarginWidth") )
    {
        sal_Int32 nMargin;
        Size aSize = maFrmDescr.GetMargin();
        if ( aAny >>= nMargin )
        {
            aSize.Width() = nMargin;
            maFrmDescr.SetMargin( aSize );
        }
    }
    else if ( aPropertyName.equalsAscii("FrameMarginHeight") )
    {
        sal_Int32 nMargin;
        Size aSize = maFrmDescr.GetMargin();
        if ( aAny >>= nMargin )
        {
            aSize.Height() = nMargin;
            maFrmDescr.SetMargin( aSize );
        }
    }
    else
         throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL IFrameObject::getPropertyValue(const ::rtl::OUString& aPropertyName)
        throw ( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aAny;
    if ( aPropertyName.equalsAscii("FrameURL") )
    {
        aAny <<= ::rtl::OUString( maFrmDescr.GetURL().GetMainURL( INetURLObject::NO_DECODE ) );
    }
    else if ( aPropertyName.equalsAscii("FrameName") )
    {
        aAny <<= ::rtl::OUString( maFrmDescr.GetName() );
    }
    else if ( aPropertyName.equalsAscii("FrameIsAutoScroll") )
    {
        sal_Bool bIsAutoScroll = ( maFrmDescr.GetScrollingMode() == ScrollingAuto );
        aAny <<= bIsAutoScroll;
    }
    else if ( aPropertyName.equalsAscii("FrameIsScrollingMode") )
    {
        sal_Bool bIsScroll = ( maFrmDescr.GetScrollingMode() == ScrollingYes );
        aAny <<= bIsScroll;
    }
    else if ( aPropertyName.equalsAscii("FrameIsBorder") )
    {
        sal_Bool bIsBorder = maFrmDescr.IsFrameBorderOn();
        aAny <<= bIsBorder;
    }
    else if ( aPropertyName.equalsAscii("FrameIsAutoBorder") )
    {
        sal_Bool bIsAutoBorder = !maFrmDescr.IsFrameBorderSet();
        aAny <<= bIsAutoBorder;
    }
    else if ( aPropertyName.equalsAscii("FrameMarginWidth") )
    {
        aAny <<= (sal_Int32 ) maFrmDescr.GetMargin().Width();
    }
    else if ( aPropertyName.equalsAscii("FrameMarginHeight") )
    {
        aAny <<= (sal_Int32 ) maFrmDescr.GetMargin().Height();
    }
    else
         throw beans::UnknownPropertyException();
    return aAny;
}

void SAL_CALL IFrameObject::addPropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL IFrameObject::removePropertyChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL IFrameObject::addVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
{
}

void SAL_CALL IFrameObject::removeVetoableChangeListener(const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener > & aListener) throw( ::com::sun::star::uno::RuntimeException )
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

void SAL_CALL IFrameObject::setTitle( const ::rtl::OUString& aTitle ) throw (::com::sun::star::uno::RuntimeException)
{
}

}
