/*************************************************************************
 *
 *  $RCSfile: vclxwindows.cxx,v $
 *
 *  $Revision: 1.43 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 07:43:47 $
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

#ifndef _TOOLKIT_AWT_VCLXWINDOWS_HXX_
#include <toolkit/awt/vclxwindows.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLEBUTTON_HXX_
#include <toolkit/awt/vclxaccessiblebutton.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLECHECKBOX_HXX_
#include <toolkit/awt/vclxaccessiblecheckbox.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLEDROPDOWCOMBOBOX_HXX_
#include <toolkit/awt/vclxaccessibledropdowncombobox.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLECOMBOBOX_HXX_
#include <toolkit/awt/vclxaccessiblecombobox.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLEDROPDOWLISTBOX_HXX_
#include <toolkit/awt/vclxaccessibledropdownlistbox.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLEEDIT_HXX_
#include <toolkit/awt/vclxaccessibleedit.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLEFIXEDTEXT_HXX_
#include <toolkit/awt/vclxaccessiblefixedtext.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLELISTBOX_HXX_
#include <toolkit/awt/vclxaccessiblelistbox.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLEMENU_HXX_
#include <toolkit/awt/vclxaccessiblemenu.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLERADIOBUTTON_HXX_
#include <toolkit/awt/vclxaccessibleradiobutton.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLESCROLLBAR_HXX_
#include <toolkit/awt/vclxaccessiblescrollbar.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLETEXTCOMPONENT_HXX_
#include <toolkit/awt/vclxaccessibletextcomponent.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXACCESSIBLETOOLBOX_HXX_
#include <toolkit/awt/vclxaccessibletoolbox.hxx>
#endif
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/convert.hxx>
#include <cppuhelper/typeprovider.hxx>

#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/field.hxx>
#include <vcl/longcurr.hxx>
#include <vcl/imgctrl.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/scrbar.hxx>

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//  ----------------------------------------------------
//  class VCLXButton
//  ----------------------------------------------------
VCLXButton::VCLXButton() : maActionListeners( *this )
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXButton::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XImageConsumer*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XButton*, this ) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXButton )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XButton>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXButton::CreateAccessibleContext()
{
    return (::com::sun::star::accessibility::XAccessibleContext*) new VCLXAccessibleButton( this );
}

void VCLXButton::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

void VCLXButton::ImplUpdateImage( sal_Bool bGetNewImage )
{
    PushButton* pButton = (PushButton*) GetWindow();
    if ( pButton )
    {
        sal_Bool bOK = bGetNewImage ? maImageConsumer.GetData( maBitmap ) : sal_True;
        if ( bOK )
            pButton->SetBitmap( maBitmap );
    }
}

void VCLXButton::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l  )throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maActionListeners.addInterface( l );
}

void VCLXButton::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maActionListeners.removeInterface( l );
}

void VCLXButton::setLabel( const ::rtl::OUString& rLabel ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rLabel );
}

void VCLXButton::setActionCommand( const ::rtl::OUString& rCommand ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maActionCommand = rCommand;
}

void VCLXButton::init( sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maImageConsumer.Init( Width, Height );
}

void VCLXButton::setColorModel( sal_Int16 BitCount, const ::com::sun::star::uno::Sequence< sal_Int32 >& RGBAPal, sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask, sal_Int32 AlphaMask ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maImageConsumer.SetColorModel( BitCount, RGBAPal.getLength(), (const unsigned long*) RGBAPal.getConstArray(), RedMask, GreenMask, BlueMask, AlphaMask );
}

void VCLXButton::setPixelsByBytes( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, const ::com::sun::star::uno::Sequence< sal_Int8 >& ProducerData, sal_Int32 Offset, sal_Int32 Scansize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maImageConsumer.SetPixelsByBytes( X, Y, Width, Height, (sal_uInt8*)ProducerData.getConstArray(), Offset, Scansize );
    ImplUpdateImage( sal_True );
}

void VCLXButton::setPixelsByLongs( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, const ::com::sun::star::uno::Sequence< sal_Int32 >& ProducerData, sal_Int32 Offset, sal_Int32 Scansize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maImageConsumer.SetPixelsByLongs( X, Y, Width, Height, (const unsigned long*) ProducerData.getConstArray(), Offset, Scansize );
    ImplUpdateImage( sal_True );
}

void VCLXButton::complete( sal_Int32 Status, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer > & Producer ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maImageConsumer.Completed( Status );

    // Controls sollen angemeldet bleiben...
//  Producer->removeConsumer( this );

    ImplUpdateImage( sal_True );
}


::com::sun::star::awt::Size VCLXButton::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    PushButton* pButton = (PushButton*) GetWindow();
    if ( pButton )
        aSz = pButton->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXButton::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::Size aSz = getMinimumSize();
    aSz.Width += 16;
    aSz.Height += 10;
    return aSz;
}

::com::sun::star::awt::Size VCLXButton::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz = VCLSize(rNewSize);
    PushButton* pButton = (PushButton*) GetWindow();
    if ( pButton )
    {
        Size aMinSz = pButton->CalcMinimumSize();
        // Kein Text, also Image
        if ( !pButton->GetText().Len() )
        {
            if ( aSz.Width() < aMinSz.Width() )
                aSz.Width() = aMinSz.Width();
            if ( aSz.Height() < aMinSz.Height() )
                aSz.Height() = aMinSz.Height();
        }
        else
        {
            if ( ( aSz.Width() > aMinSz.Width() ) && ( aSz.Height() < aMinSz.Height() ) )
                aSz.Height() = aMinSz.Height();
            else
                aSz = aMinSz;
        }
    }
    return AWTSize(aSz);
}

void VCLXButton::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Button* pButton = (Button*)GetWindow();
    if ( pButton )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_DEFAULTBUTTON:
            {
                WinBits nStyle = pButton->GetStyle() | WB_DEFBUTTON;
                sal_Bool b;
                if ( ( Value >>= b ) && !b )
                    nStyle &= ~WB_DEFBUTTON;
                pButton->SetStyle( nStyle );
            }
            break;
            case BASEPROPERTY_STATE:
            {
                if ( GetWindow()->GetType() == WINDOW_PUSHBUTTON )
                {
                    sal_Int16 n;
                    if ( Value >>= n )
                        ((PushButton*)pButton)->SetState( (TriState)n );
                }
            }
            break;
            case BASEPROPERTY_IMAGEALIGN:
            {
                if ( GetWindow()->GetType() == WINDOW_PUSHBUTTON )
                {
                    sal_Int16 n;
                    if ( Value >>= n )
                        ((PushButton*)pButton)->SetImageAlign( (ImageAlign)n );
                }
            }
            break;
            default:
            {
                VCLXWindow::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXButton::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    Button* pButton = (Button*)GetWindow();
    if ( pButton )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_DEFAULTBUTTON:
            {
                aProp <<= (sal_Bool) ( ( pButton->GetStyle() & WB_DEFBUTTON ) ? sal_True : sal_False );
            }
            break;
            case BASEPROPERTY_STATE:
            {
                if ( GetWindow()->GetType() == WINDOW_PUSHBUTTON )
                {
                     aProp <<= (sal_Int16)((PushButton*)pButton)->GetState();
                }
            }
            break;
            case BASEPROPERTY_IMAGEALIGN:
            {
                if ( GetWindow()->GetType() == WINDOW_PUSHBUTTON )
                {
                     aProp <<= (sal_Int16)((PushButton*)pButton)->GetImageAlign();
                }
            }
            break;
            default:
            {
                aProp <<= VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXButton::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_BUTTON_CLICK:
            if ( maActionListeners.getLength() )
            {
                ::com::sun::star::awt::ActionEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                aEvent.ActionCommand = maActionCommand;
                maActionListeners.actionPerformed( aEvent );
            }
            break;

        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

//  ----------------------------------------------------
//  class VCLXImageControl
//  ----------------------------------------------------
VCLXImageControl::VCLXImageControl()
{
}

VCLXImageControl::~VCLXImageControl()
{
}

void VCLXImageControl::ImplUpdateImage( sal_Bool bGetNewImage )
{
    ImageControl* pControl = (ImageControl*) GetWindow();
    if ( pControl )
    {
        sal_Bool bOK = bGetNewImage ? maImageConsumer.GetData( maBitmap ) : sal_True;
        if ( bOK )
            pControl->SetBitmap( maBitmap );
    }
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXImageControl::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XImageConsumer*, this ) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXImageControl )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXImageControl::setPosSize( long X, long Y, long Width, long Height, short Flags ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        Size aOldSize = GetWindow()->GetSizePixel();
        VCLXWindow::setPosSize( X, Y, Width, Height, Flags );
        if ( ( aOldSize.Width() != Width ) || ( aOldSize.Height() != Height ) )
            ImplUpdateImage( sal_False );
    }
}

void VCLXImageControl::init( sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maImageConsumer.Init( Width, Height );
}

void VCLXImageControl::setColorModel( sal_Int16 BitCount, const ::com::sun::star::uno::Sequence< sal_Int32 >& RGBAPal, sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask, sal_Int32 AlphaMask ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maImageConsumer.SetColorModel( BitCount, RGBAPal.getLength(), (const unsigned long*) RGBAPal.getConstArray(), RedMask, GreenMask, BlueMask, AlphaMask );
}

void VCLXImageControl::setPixelsByBytes( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, const ::com::sun::star::uno::Sequence< sal_Int8 >& ProducerData, sal_Int32 Offset, sal_Int32 Scansize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maImageConsumer.SetPixelsByBytes( X, Y, Width, Height, (sal_uInt8*)ProducerData.getConstArray(), Offset, Scansize );
    ImplUpdateImage( sal_True );
}

void VCLXImageControl::setPixelsByLongs( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, const ::com::sun::star::uno::Sequence< sal_Int32 >& ProducerData, sal_Int32 Offset, sal_Int32 Scansize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maImageConsumer.SetPixelsByLongs( X, Y, Width, Height, (const unsigned long*) ProducerData.getConstArray(), Offset, Scansize );
    ImplUpdateImage( sal_True );
}

void VCLXImageControl::complete( sal_Int32 Status, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer > & Producer ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    maImageConsumer.Completed( Status );

    // Controls sollen angemeldet bleiben...
//  Producer->removeConsumer( this );

    ImplUpdateImage( sal_True );
}

::com::sun::star::awt::Size VCLXImageControl::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz = maBitmap.GetSizePixel();
    aSz = ImplCalcWindowSize( aSz );

    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXImageControl::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXImageControl::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::Size aSz = rNewSize;
    ::com::sun::star::awt::Size aMinSz = getMinimumSize();
    if ( aSz.Width < aMinSz.Width )
        aSz.Width = aMinSz.Width;
    if ( aSz.Height < aMinSz.Height )
        aSz.Height = aMinSz.Height;
    return aSz;
}

void VCLXImageControl::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ImageControl* pImageControl = (ImageControl*)GetWindow();
    if ( pImageControl )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_SCALEIMAGE:
            {
                sal_Bool b;
                if ( Value >>= b )
                     pImageControl->SetScaleImage( b );
            }
            break;
            default:
            {
                VCLXWindow::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXImageControl::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    ImageControl* pImageControl = (ImageControl*)GetWindow();
    if ( pImageControl )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_SCALEIMAGE:
            {
                 aProp <<= (sal_Bool)pImageControl->IsScaleImage();
            }
            break;
            default:
            {
                aProp <<= VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

//  ----------------------------------------------------
//  class VCLXCheckBox
//  ----------------------------------------------------
VCLXCheckBox::VCLXCheckBox() : maItemListeners( *this ), maActionListeners( *this )
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXCheckBox::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XButton*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XCheckBox*, this ) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXCheckBox )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XButton>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XCheckBox>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXCheckBox::CreateAccessibleContext()
{
    return (::com::sun::star::accessibility::XAccessibleContext*) new VCLXAccessibleCheckBox( this );
}

void VCLXCheckBox::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

void VCLXCheckBox::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maItemListeners.addInterface( l );
}

void VCLXCheckBox::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maItemListeners.removeInterface( l );
}

void VCLXCheckBox::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l  )throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maActionListeners.addInterface( l );
}

void VCLXCheckBox::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maActionListeners.removeInterface( l );
}

void VCLXCheckBox::setActionCommand( const ::rtl::OUString& rCommand ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maActionCommand = rCommand;
}

void VCLXCheckBox::setLabel( const ::rtl::OUString& rLabel ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rLabel );
}

void VCLXCheckBox::setState( short n ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    CheckBox* pCheckBox = (CheckBox*)GetWindow();
    if ( pCheckBox)
    {
        TriState eState;
        switch ( n )
        {
            case 0:     eState = STATE_NOCHECK;     break;
            case 1:     eState = STATE_CHECK;       break;
            case 2:     eState = STATE_DONTKNOW;    break;
            default:    eState = STATE_NOCHECK;
        }
        pCheckBox->SetState( eState );

        // #105198# call C++ click listeners (needed for accessibility)
        // pCheckBox->GetClickHdl().Call( pCheckBox );

        // #107218# Call same virtual methods and listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pCheckBox->Toggle();
        pCheckBox->Click();
        SetSynthesizingVCLEvent( sal_False );
    }
}

short VCLXCheckBox::getState() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    short nState = -1;
    CheckBox* pCheckBox = (CheckBox*)GetWindow();
    if ( pCheckBox )
    {
        switch ( pCheckBox->GetState() )
        {
            case STATE_NOCHECK:     nState = 0; break;
            case STATE_CHECK:       nState = 1; break;
            case STATE_DONTKNOW:    nState = 2; break;
            default:                DBG_ERROR( "VCLXCheckBox::getState(): unknown TriState!" );
        }
    }

    return nState;
}

void VCLXCheckBox::enableTriState( sal_Bool b ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    CheckBox* pCheckBox = (CheckBox*)GetWindow();
    if ( pCheckBox)
        pCheckBox->EnableTriState( b );
}

::com::sun::star::awt::Size VCLXCheckBox::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    CheckBox* pCheckBox = (CheckBox*) GetWindow();
    if ( pCheckBox )
        aSz = pCheckBox->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXCheckBox::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXCheckBox::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz = VCLSize(rNewSize);
    CheckBox* pCheckBox = (CheckBox*) GetWindow();
    if ( pCheckBox )
    {
        Size aMinSz = pCheckBox->CalcMinimumSize();
        if ( ( aSz.Width() > aMinSz.Width() ) && ( aSz.Height() < aMinSz.Height() ) )
            aSz.Height() = aMinSz.Height();
        else
            aSz = aMinSz;
    }
    return AWTSize(aSz);
}

void VCLXCheckBox::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    CheckBox* pCheckBox = (CheckBox*)GetWindow();
    if ( pCheckBox )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_TRISTATE:
            {
                sal_Bool b;
                if ( Value >>= b )
                     pCheckBox->EnableTriState( b );
            }
            break;
            case BASEPROPERTY_STATE:
            {
                sal_Int16 n;
                if ( Value >>= n )
                    setState( n );
            }
            break;
            default:
            {
                VCLXWindow::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXCheckBox::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    CheckBox* pCheckBox = (CheckBox*)GetWindow();
    if ( pCheckBox )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_TRISTATE:
            {
                 aProp <<= (sal_Bool)pCheckBox->IsTriStateEnabled();
            }
            break;
            case BASEPROPERTY_STATE:
            {
                 aProp <<= (sal_Int16)pCheckBox->GetState();
            }
            break;
            default:
            {
                aProp <<= VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXCheckBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_CHECKBOX_TOGGLE:
        {
            CheckBox* pCheckBox = (CheckBox*)GetWindow();
            if ( pCheckBox )
            {
                if ( maItemListeners.getLength() )
                {
                    ::com::sun::star::awt::ItemEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.Highlighted = sal_False;
                    aEvent.Selected = pCheckBox->GetState();
                    maItemListeners.itemStateChanged( aEvent );
                }
                if ( !IsSynthesizingVCLEvent() && maActionListeners.getLength() )
                {
                    ::com::sun::star::awt::ActionEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.ActionCommand = maActionCommand;
                    maActionListeners.actionPerformed( aEvent );
                }
            }
        }
        break;

        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

//  ----------------------------------------------------
//  class VCLXRadioButton
//  ----------------------------------------------------
VCLXRadioButton::VCLXRadioButton() : maItemListeners( *this ), maActionListeners( *this )
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXRadioButton::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XRadioButton*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XButton*, this ) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXRadioButton )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRadioButton>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XButton>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXRadioButton::CreateAccessibleContext()
{
    return (::com::sun::star::accessibility::XAccessibleContext*) new VCLXAccessibleRadioButton( this );
}

void VCLXRadioButton::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

void VCLXRadioButton::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    RadioButton* pButton = (RadioButton*)GetWindow();
    if ( pButton )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_STATE:
            {
                sal_Int16 n;
                if ( Value >>= n )
                {
                    BOOL b = n ? sal_True : sal_False;
                    if ( pButton->IsRadioCheckEnabled() )
                        pButton->Check( b );
                    else
                        pButton->SetState( b );
                }
            }
            break;
            case BASEPROPERTY_AUTOTOGGLE:
            {
                sal_Bool b;
                if ( Value >>= b )
                    pButton->EnableRadioCheck( b );
            }
            break;
            default:
            {
                VCLXWindow::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXRadioButton::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    RadioButton* pButton = (RadioButton*)GetWindow();
    if ( pButton )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_STATE:
            {
                aProp <<= (sal_Int16) ( pButton->IsChecked() ? 1 : 0 );
            }
            break;
            case BASEPROPERTY_AUTOTOGGLE:
            {
                aProp <<= (sal_Bool) pButton->IsRadioCheckEnabled();
            }
            break;
            default:
            {
                aProp <<= VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXRadioButton::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maItemListeners.addInterface( l );
}

void VCLXRadioButton::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maItemListeners.removeInterface( l );
}

void VCLXRadioButton::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l  )throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maActionListeners.addInterface( l );
}

void VCLXRadioButton::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maActionListeners.removeInterface( l );
}

void VCLXRadioButton::setLabel( const ::rtl::OUString& rLabel ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rLabel );
}

void VCLXRadioButton::setActionCommand( const ::rtl::OUString& rCommand ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maActionCommand = rCommand;
}

void VCLXRadioButton::setState( sal_Bool b ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    RadioButton* pRadioButton = (RadioButton*)GetWindow();
    if ( pRadioButton)
    {
        pRadioButton->Check( b );
        // #102717# item listeners are called, but not C++ click listeners in StarOffice code => call click hdl
        // But this is needed in old code because Accessibility API uses it.
        // pRadioButton->GetClickHdl().Call( pRadioButton );

        // #107218# Call same virtual methods and listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pRadioButton->Click();
        SetSynthesizingVCLEvent( sal_False );
    }
}

sal_Bool VCLXRadioButton::getState() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    RadioButton* pRadioButton = (RadioButton*)GetWindow();
    return pRadioButton ? pRadioButton->IsChecked() : sal_False;
}

::com::sun::star::awt::Size VCLXRadioButton::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    RadioButton* pRadioButton = (RadioButton*) GetWindow();
    if ( pRadioButton )
        aSz = pRadioButton->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXRadioButton::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXRadioButton::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz = VCLSize(rNewSize);
    RadioButton* pRadioButton = (RadioButton*) GetWindow();
    if ( pRadioButton )
    {
        Size aMinSz = pRadioButton->CalcMinimumSize();
        if ( ( aSz.Width() > aMinSz.Width() ) && ( aSz.Height() < aMinSz.Height() ) )
            aSz.Height() = aMinSz.Height();
        else
            aSz = aMinSz;
    }
    return AWTSize(aSz);
}

void VCLXRadioButton::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_BUTTON_CLICK:
            if ( !IsSynthesizingVCLEvent() && maActionListeners.getLength() )
            {
                ::com::sun::star::awt::ActionEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                aEvent.ActionCommand = maActionCommand;
                maActionListeners.actionPerformed( aEvent );
            }
            ImplClickedOrToggled( FALSE );
            break;

        case VCLEVENT_RADIOBUTTON_TOGGLE:
            ImplClickedOrToggled( TRUE );
            break;

        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

void VCLXRadioButton::ImplClickedOrToggled( BOOL bToggled )
{
    // In the formulars, RadioChecked is not enabled, call itemStateChanged only for click
    // In the dialog editor, RadioChecked is enabled, call itemStateChanged only for bToggled
    RadioButton* pRadioButton = (RadioButton*)GetWindow();
    if ( pRadioButton && ( pRadioButton->IsRadioCheckEnabled() == bToggled ) && ( bToggled || pRadioButton->IsStateChanged() ) && maItemListeners.getLength() )
    {
        ::com::sun::star::awt::ItemEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)this;
        aEvent.Highlighted = sal_False;
        aEvent.Selected = pRadioButton->IsChecked();
        maItemListeners.itemStateChanged( aEvent );
    }
}

//  ----------------------------------------------------
//  class VCLXSpinField
//  ----------------------------------------------------
VCLXSpinField::VCLXSpinField() : maSpinListeners( *this )
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXSpinField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XSpinField*, this ) );
    return (aRet.hasValue() ? aRet : VCLXEdit::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXSpinField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinField>* ) NULL ),
    VCLXEdit::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXSpinField::addSpinListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maSpinListeners.addInterface( l );
}

void VCLXSpinField::removeSpinListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maSpinListeners.removeInterface( l );
}

void VCLXSpinField::up() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    SpinField* pSpinField = (SpinField*) GetWindow();
    if ( pSpinField )
        pSpinField->Up();
}

void VCLXSpinField::down() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    SpinField* pSpinField = (SpinField*) GetWindow();
    if ( pSpinField )
        pSpinField->Down();
}

void VCLXSpinField::first() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    SpinField* pSpinField = (SpinField*) GetWindow();
    if ( pSpinField )
        pSpinField->First();
}

void VCLXSpinField::last() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    SpinField* pSpinField = (SpinField*) GetWindow();
    if ( pSpinField )
        pSpinField->Last();
}

void VCLXSpinField::enableRepeat( sal_Bool bRepeat ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nStyle = pWindow->GetStyle();
        if ( bRepeat )
            nStyle |= WB_REPEAT;
        else
            nStyle &= ~WB_REPEAT;
        pWindow->SetStyle( nStyle );
    }
}

void VCLXSpinField::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_SPINFIELD_UP:
        case VCLEVENT_SPINFIELD_DOWN:
        case VCLEVENT_SPINFIELD_FIRST:
        case VCLEVENT_SPINFIELD_LAST:
        {
            if ( maSpinListeners.getLength() )
            {
                ::com::sun::star::awt::SpinEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                switch ( rVclWindowEvent.GetId() )
                {
                    case VCLEVENT_SPINFIELD_UP:     maSpinListeners.up( aEvent );
                                                    break;
                    case VCLEVENT_SPINFIELD_DOWN:   maSpinListeners.down( aEvent );
                                                    break;
                    case VCLEVENT_SPINFIELD_FIRST:  maSpinListeners.first( aEvent );
                                                    break;
                    case VCLEVENT_SPINFIELD_LAST:   maSpinListeners.last( aEvent );
                                                    break;
                }

            }
        }
        break;

        default:
            VCLXEdit::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}


//  ----------------------------------------------------
//  class VCLXListBox
//  ----------------------------------------------------
VCLXListBox::VCLXListBox()
    : maItemListeners( *this ),
      maActionListeners( *this )
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXListBox::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XListBox*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTextLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXListBox )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XListBox>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextLayoutConstrains>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXListBox::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aObj );
    maActionListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

void VCLXListBox::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maItemListeners.addInterface( l );
}

void VCLXListBox::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maItemListeners.removeInterface( l );
}

void VCLXListBox::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maActionListeners.addInterface( l );
}

void VCLXListBox::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maActionListeners.removeInterface( l );
}

void VCLXListBox::addItem( const ::rtl::OUString& aItem, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
        pBox->InsertEntry( aItem, nPos );
}

void VCLXListBox::addItems( const ::com::sun::star::uno::Sequence< ::rtl::OUString>& aItems, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
    {
        sal_uInt16 nP = nPos;
        for ( sal_uInt16 n = 0; n < aItems.getLength(); n++ )
        {
            pBox->InsertEntry( aItems.getConstArray()[n], nP );
            if ( nPos < 0xFFFF )    // Nicht wenn 0xFFFF, weil LIST_APPEND
                nP++;
        }
    }
}

void VCLXListBox::removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
    {
        for ( sal_uInt16 n = nCount; n; )
            pBox->RemoveEntry( nPos + (--n) );
    }
}

sal_Int16 VCLXListBox::getItemCount() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ListBox* pBox = (ListBox*) GetWindow();
    return pBox ? pBox->GetEntryCount() : 0;
}

::rtl::OUString VCLXListBox::getItem( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    String aItem;
    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
        aItem = pBox->GetEntry( nPos );
    return aItem;
}

::com::sun::star::uno::Sequence< ::rtl::OUString> VCLXListBox::getItems() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
    {
        sal_uInt16 nEntries = pBox->GetEntryCount();
        aSeq = ::com::sun::star::uno::Sequence< ::rtl::OUString>( nEntries );
        for ( sal_uInt16 n = nEntries; n; )
        {
            --n;
            aSeq.getArray()[n] = ::rtl::OUString( pBox->GetEntry( n ) );
        }
    }
    return aSeq;
}

sal_Int16 VCLXListBox::getSelectedItemPos() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ListBox* pBox = (ListBox*) GetWindow();
    return pBox ? pBox->GetSelectEntryPos() : 0;
}

::com::sun::star::uno::Sequence<sal_Int16> VCLXListBox::getSelectedItemsPos() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence<sal_Int16> aSeq;
    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
    {
        sal_uInt16 nSelEntries = pBox->GetSelectEntryCount();
        aSeq = ::com::sun::star::uno::Sequence<sal_Int16>( nSelEntries );
        for ( sal_uInt16 n = 0; n < nSelEntries; n++ )
            aSeq.getArray()[n] = pBox->GetSelectEntryPos( n );
    }
    return aSeq;
}

::rtl::OUString VCLXListBox::getSelectedItem() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    String aItem;
    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
        aItem = pBox->GetSelectEntry();
    return aItem;
}

::com::sun::star::uno::Sequence< ::rtl::OUString> VCLXListBox::getSelectedItems() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
    {
        sal_uInt16 nSelEntries = pBox->GetSelectEntryCount();
        aSeq = ::com::sun::star::uno::Sequence< ::rtl::OUString>( nSelEntries );
        for ( sal_uInt16 n = 0; n < nSelEntries; n++ )
            aSeq.getArray()[n] = ::rtl::OUString( pBox->GetSelectEntry( n ) );
    }
    return aSeq;
}

void VCLXListBox::selectItemPos( sal_Int16 nPos, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox && ( pBox->IsEntryPosSelected( nPos ) != bSelect ) )
    {
        pBox->SelectEntryPos( nPos, bSelect );

        // VCL doesn't call select handler after API call.
        // ImplCallItemListeners();

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pBox->Select();
        SetSynthesizingVCLEvent( sal_False );
    }
}

void VCLXListBox::selectItemsPos( const ::com::sun::star::uno::Sequence<sal_Int16>& aPositions, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
    {
        BOOL bChanged = FALSE;
        for ( sal_uInt16 n = (sal_uInt16)aPositions.getLength(); n; )
        {
            USHORT nPos = (USHORT) aPositions.getConstArray()[--n];
            if ( pBox->IsEntryPosSelected( nPos ) != bSelect )
            {
                pBox->SelectEntryPos( nPos, bSelect );
                bChanged = TRUE;
            }
        }

        if ( bChanged )
        {
            // VCL doesn't call select handler after API call.
            // ImplCallItemListeners();

            // #107218# Call same listeners like VCL would do after user interaction
            SetSynthesizingVCLEvent( sal_True );
            pBox->Select();
            SetSynthesizingVCLEvent( sal_False );
        }
    }
}

void VCLXListBox::selectItem( const ::rtl::OUString& rItemText, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
    {
        String aItemText( rItemText );
        selectItemPos( pBox->GetEntryPos( aItemText ), bSelect );
    }
}


void VCLXListBox::setDropDownLineCount( sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
        pBox->SetDropDownLineCount( nLines );
}

sal_Int16 VCLXListBox::getDropDownLineCount() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int16 nLines = 0;
    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
        nLines = pBox->GetDropDownLineCount();
    return nLines;
}

sal_Bool VCLXListBox::isMutipleMode() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Bool bMulti = sal_False;
    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
        bMulti = pBox->IsMultiSelectionEnabled();
    return bMulti;
}

void VCLXListBox::setMultipleMode( sal_Bool bMulti ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
        pBox->EnableMultiSelection( bMulti );
}

void VCLXListBox::makeVisible( sal_Int16 nEntry ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ListBox* pBox = (ListBox*) GetWindow();
    if ( pBox )
        pBox->SetTopEntry( nEntry );
}

void VCLXListBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_LISTBOX_SELECT:
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XListBox > xKeepAlive( this );
                // in the DoubleClickHdl, the object may be destroyed, so we have to ensure
                // that we stay alive as long as we're herein
                // 97515 - 04.03.2002 - fs@openoffice.org

            ListBox* pListBox = (ListBox*)GetWindow();

            if( pListBox )
            {
                sal_Bool bDropDown = ( pListBox->GetStyle() & WB_DROPDOWN ) ? sal_True : sal_False;
                if ( bDropDown && !IsSynthesizingVCLEvent() && maActionListeners.getLength() )
                {
                    // Bei DropDown den ActionListener rufen...
                    ::com::sun::star::awt::ActionEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.ActionCommand = pListBox->GetSelectEntry();
                    maActionListeners.actionPerformed( aEvent );
                }

                if ( maItemListeners.getLength() )
                {
                    ImplCallItemListeners();
                }
            }
        }
        break;

        case VCLEVENT_LISTBOX_DOUBLECLICK:
            if ( GetWindow() && maActionListeners.getLength() )
            {
                ::com::sun::star::awt::ActionEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                aEvent.ActionCommand = ((ListBox*)GetWindow())->GetSelectEntry();
                maActionListeners.actionPerformed( aEvent );
            }
            break;

        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXListBox::CreateAccessibleContext()
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Bool bIsDropDownBox = sal_False;
    ListBox* pBox = static_cast< ListBox* >( GetWindow() );
    if ( pBox )
        bIsDropDownBox = ( ( pBox->GetStyle() & WB_DROPDOWN ) == WB_DROPDOWN );

    if ( bIsDropDownBox )
        return (::com::sun::star::accessibility::XAccessibleContext*) new VCLXAccessibleDropDownListBox( this );
    else
        return (::com::sun::star::accessibility::XAccessibleContext*) new VCLXAccessibleListBox( this );
}

void VCLXListBox::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ListBox* pListBox = (ListBox*)GetWindow();
    if ( pListBox )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_READONLY:
            {
                sal_Bool b;
                if ( Value >>= b )
                     pListBox->SetReadOnly( b);
            }
            break;
            case BASEPROPERTY_MULTISELECTION:
            {
                sal_Bool b;
                if ( Value >>= b )
                     pListBox->EnableMultiSelection( b );
            }
            break;
            case BASEPROPERTY_LINECOUNT:
            {
                sal_Int16 n;
                if ( Value >>= n )
                     pListBox->SetDropDownLineCount( n );
            }
            break;
            case BASEPROPERTY_STRINGITEMLIST:
            {
                ::com::sun::star::uno::Sequence< ::rtl::OUString> aItems;
                if ( Value >>= aItems )
                {
                    pListBox->Clear();
                    addItems( aItems, 0 );
                }
            }
            break;
            case BASEPROPERTY_SELECTEDITEMS:
            {
                ::com::sun::star::uno::Sequence<sal_Int16> aItems;
                if ( Value >>= aItems )
                {
                    for ( sal_uInt16 n = pListBox->GetEntryCount(); n; )
                        pListBox->SelectEntryPos( --n, sal_False );

                    selectItemsPos( aItems, sal_True );

                    if ( !pListBox->GetSelectEntryCount() )
                        pListBox->SetTopEntry( 0 );
                }
            }
            break;
            default:
            {
                VCLXWindow::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXListBox::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    ListBox* pListBox = (ListBox*)GetWindow();
    if ( pListBox )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_READONLY:
            {
                 aProp <<= (sal_Bool) pListBox->IsReadOnly();
            }
            break;
            case BASEPROPERTY_MULTISELECTION:
            {
                 aProp <<= (sal_Bool) pListBox->IsMultiSelectionEnabled();
            }
            break;
            case BASEPROPERTY_LINECOUNT:
            {
                 aProp <<= (sal_Int16) pListBox->GetDropDownLineCount();
            }
            break;
            case BASEPROPERTY_STRINGITEMLIST:
            {
                sal_uInt16 nItems = pListBox->GetEntryCount();
                ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq( nItems );
                ::rtl::OUString* pStrings = aSeq.getArray();
                for ( sal_uInt16 n = 0; n < nItems; n++ )
                    pStrings[n] = pListBox->GetEntry( n );
                aProp <<= aSeq;

            }
            break;
            default:
            {
                aProp <<= VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

::com::sun::star::awt::Size VCLXListBox::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    ListBox* pListBox = (ListBox*) GetWindow();
    if ( pListBox )
        aSz = pListBox->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXListBox::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    ListBox* pListBox = (ListBox*) GetWindow();
    if ( pListBox )
    {
        aSz = pListBox->CalcMinimumSize();
        if ( pListBox->GetStyle() & WB_DROPDOWN )
            aSz.Height() += 4;
    }
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXListBox::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz = VCLSize(rNewSize);
    ListBox* pListBox = (ListBox*) GetWindow();
    if ( pListBox )
        aSz = pListBox->CalcAdjustedSize( aSz );
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXListBox::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    ListBox* pListBox = (ListBox*) GetWindow();
    if ( pListBox )
        aSz = pListBox->CalcSize( nCols, nLines );
    return AWTSize(aSz);
}

void VCLXListBox::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    nCols = nLines = 0;
    ListBox* pListBox = (ListBox*) GetWindow();
    if ( pListBox )
    {
        sal_uInt16 nC, nL;
        pListBox->GetMaxVisColumnsAndLines( nC, nL );
        nCols = nC;
        nLines = nL;
    }
}

void VCLXListBox::ImplCallItemListeners()
{
    ListBox* pListBox = (ListBox*) GetWindow();
    if ( pListBox && maItemListeners.getLength() )
    {
        ::com::sun::star::awt::ItemEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)this;
        aEvent.Highlighted = sal_False;

        // Bei Mehrfachselektion 0xFFFF, sonst die ID
        aEvent.Selected = (pListBox->GetSelectEntryCount() == 1 ) ? pListBox->GetSelectEntryPos() : 0xFFFF;

        maItemListeners.itemStateChanged( aEvent );
    }
}


//  ----------------------------------------------------
//  class VCLXMessageBox
//  ----------------------------------------------------
VCLXMessageBox::VCLXMessageBox()
{
}

VCLXMessageBox::~VCLXMessageBox()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXMessageBox::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XMessageBox*, this ) );
    return (aRet.hasValue() ? aRet : VCLXTopWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXMessageBox )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMessageBox>* ) NULL ),
    VCLXTopWindow::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXMessageBox::setCaptionText( const ::rtl::OUString& rText ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( rText );
}

::rtl::OUString VCLXMessageBox::getCaptionText() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    String aText;
    Window* pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

void VCLXMessageBox::setMessageText( const ::rtl::OUString& rText ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    MessBox* pBox = (MessBox*)GetWindow();
    if ( pBox )
        pBox->SetMessText( rText );
}

::rtl::OUString VCLXMessageBox::getMessageText() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aText;
    MessBox* pBox = (MessBox*)GetWindow();
    if ( pBox )
        aText = pBox->GetMessText();
    return aText;
}

sal_Int16 VCLXMessageBox::execute() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    MessBox* pBox = (MessBox*)GetWindow();
    return pBox ? pBox->Execute() : 0;
}



//  ----------------------------------------------------
//  class VCLXDialog
//  ----------------------------------------------------
VCLXDialog::VCLXDialog()
{
}

VCLXDialog::~VCLXDialog()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXDialog::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XDialog*, this ) );
    return (aRet.hasValue() ? aRet : VCLXTopWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXDialog )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDialog>* ) NULL ),
    VCLXTopWindow::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXDialog::setTitle( const ::rtl::OUString& Title ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( Title );
}

::rtl::OUString VCLXDialog::getTitle() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aTitle;
    Window* pWindow = GetWindow();
    if ( pWindow )
        aTitle = pWindow->GetText();
    return aTitle;
}

sal_Int16 VCLXDialog::execute() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int16 nRet = 0;
    if ( GetWindow() )
    {
        Dialog* pDlg = (Dialog*) GetWindow();
        Window* pParent = pDlg->GetWindow( WINDOW_PARENTOVERLAP );
        Window* pOldParent = NULL;
        if ( pParent && !pParent->IsReallyVisible() )
        {
            pOldParent = pDlg->GetParent();
            pDlg->SetParent( pDlg->GetWindow( WINDOW_FRAME ) );
        }
        nRet = pDlg->Execute();
        if ( pOldParent )
            pDlg->SetParent( pOldParent );
    }
    return nRet;
}

void VCLXDialog::endExecute() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Dialog* pDlg = (Dialog*) GetWindow();
    if ( pDlg )
        pDlg->EndDialog( 0 );
}

::com::sun::star::awt::DeviceInfo VCLXDialog::getInfo() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::DeviceInfo aInfo = VCLXDevice::getInfo();

#if SUPD >= 634
    ::vos::OGuard aGuard( GetMutex() );
    Dialog* pDlg = (Dialog*) GetWindow();
    if ( pDlg )
        pDlg->GetDrawWindowBorder( aInfo.LeftInset, aInfo.TopInset, aInfo.RightInset, aInfo.BottomInset );
#endif

    return aInfo;
}

//  ----------------------------------------------------
//  class VCLXFixedText
//  ----------------------------------------------------
VCLXFixedText::VCLXFixedText()
{
}

VCLXFixedText::~VCLXFixedText()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXFixedText::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XFixedText*, this ) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXFixedText )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFixedText>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXFixedText::CreateAccessibleContext()
{
    return (::com::sun::star::accessibility::XAccessibleContext*) new VCLXAccessibleFixedText( this );
}

void VCLXFixedText::setText( const ::rtl::OUString& Text ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
        pWindow->SetText( Text );
}

::rtl::OUString VCLXFixedText::getText() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aText;
    Window* pWindow = GetWindow();
    if ( pWindow )
           aText = pWindow->GetText();
    return aText;
}

void VCLXFixedText::setAlignment( short nAlign ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nNewBits = 0;
        if ( nAlign == ::com::sun::star::awt::TextAlign::LEFT )
            nNewBits = WB_LEFT;
        else if ( nAlign == ::com::sun::star::awt::TextAlign::CENTER )
            nNewBits = WB_CENTER;
        else
            nNewBits = WB_RIGHT;

        WinBits nStyle = pWindow->GetStyle();
        nStyle &= ~(WB_LEFT|WB_CENTER|WB_RIGHT);
        pWindow->SetStyle( nStyle | nNewBits );
    }
}

short VCLXFixedText::getAlignment() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    short nAlign = 0;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nStyle = pWindow->GetStyle();
        if ( nStyle & WB_LEFT )
            nAlign = ::com::sun::star::awt::TextAlign::LEFT;
        else if ( nStyle & WB_CENTER )
            nAlign = ::com::sun::star::awt::TextAlign::CENTER;
        else
            nAlign = ::com::sun::star::awt::TextAlign::RIGHT;
    }
    return nAlign;
}

::com::sun::star::awt::Size VCLXFixedText::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    FixedText* pFixedText = (FixedText*)GetWindow();
    if ( pFixedText )
        aSz = pFixedText->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXFixedText::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return getMinimumSize();
}

::com::sun::star::awt::Size VCLXFixedText::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::Size aSz = rNewSize;
    ::com::sun::star::awt::Size aMinSz = getMinimumSize();
    if ( aSz.Height != aMinSz.Height )
        aSz.Height = aMinSz.Height;

    return aSz;
}

//  ----------------------------------------------------
//  class VCLXScrollBar
//  ----------------------------------------------------
VCLXScrollBar::VCLXScrollBar() : maAdjustmentListeners( *this )
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXScrollBar::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XScrollBar*, this ) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXScrollBar )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XScrollBar>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXScrollBar::CreateAccessibleContext()
{
    return (::com::sun::star::accessibility::XAccessibleContext*) new VCLXAccessibleScrollBar( this );
}

// ::com::sun::star::lang::XComponent
void VCLXScrollBar::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    maAdjustmentListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

// ::com::sun::star::awt::XScrollbar
void VCLXScrollBar::addAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maAdjustmentListeners.addInterface( l );
}

void VCLXScrollBar::removeAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maAdjustmentListeners.removeInterface( l );
}

void VCLXScrollBar::setValue( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    if ( pScrollBar )
        pScrollBar->DoScroll( n );
}

void VCLXScrollBar::setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    if ( pScrollBar )
    {
        pScrollBar->SetVisibleSize( nVisible );
        pScrollBar->SetRangeMax( nMax );
        pScrollBar->DoScroll( nValue );
    }
}

sal_Int32 VCLXScrollBar::getValue() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    return pScrollBar ? pScrollBar->GetThumbPos() : 0;
}

void VCLXScrollBar::setMaximum( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    if ( pScrollBar )
        pScrollBar->SetRangeMax( n );
}

sal_Int32 VCLXScrollBar::getMaximum() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    return pScrollBar ? pScrollBar->GetRangeMax() : 0;
}

void VCLXScrollBar::setLineIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    if ( pScrollBar )
        pScrollBar->SetLineSize( n );
}

sal_Int32 VCLXScrollBar::getLineIncrement() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    return pScrollBar ? pScrollBar->GetLineSize() : 0;
}

void VCLXScrollBar::setBlockIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    if ( pScrollBar )
        pScrollBar->SetPageSize( n );
}

sal_Int32 VCLXScrollBar::getBlockIncrement() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    return pScrollBar ? pScrollBar->GetPageSize() : 0;
}

void VCLXScrollBar::setVisibleSize( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    if ( pScrollBar )
        pScrollBar->SetVisibleSize( n );
}

sal_Int32 VCLXScrollBar::getVisibleSize() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ScrollBar* pScrollBar = (ScrollBar*) GetWindow();
    return pScrollBar ? pScrollBar->GetVisibleSize() : 0;
}

void VCLXScrollBar::setOrientation( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nStyle = pWindow->GetStyle();
        nStyle &= ~(WB_HORZ|WB_VERT);
        if ( n == ::com::sun::star::awt::ScrollBarOrientation::HORIZONTAL )
            nStyle |= WB_HORZ;
        else
            nStyle |= WB_VERT;

        pWindow->SetStyle( nStyle );
        pWindow->Resize();
    }
}

sal_Int32 VCLXScrollBar::getOrientation() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int32 n = 0;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        WinBits nStyle = pWindow->GetStyle();
        if ( nStyle & WB_HORZ )
            n = ::com::sun::star::awt::ScrollBarOrientation::HORIZONTAL;
        else
            n = ::com::sun::star::awt::ScrollBarOrientation::VERTICAL;
    }
    return n;

}

// ::com::sun::star::awt::VclWindowPeer
void VCLXScrollBar::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ScrollBar* pScrollBar = (ScrollBar*)GetWindow();
    if ( pScrollBar )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_SCROLLVALUE:
            {
                if ( !bVoid )
                {
                    sal_Int32 n;
                    if ( Value >>= n )
                        setValue( n );
                }
            }
            break;
            case BASEPROPERTY_SCROLLVALUE_MAX:
            {
                if ( !bVoid )
                {
                    sal_Int32 n;
                    if ( Value >>= n )
                        setMaximum( n );
                }
            }
            break;
            case BASEPROPERTY_LINEINCREMENT:
            {
                if ( !bVoid )
                {
                    sal_Int32 n;
                    if ( Value >>= n )
                        setLineIncrement( n );
                }
            }
            break;
            case BASEPROPERTY_BLOCKINCREMENT:
            {
                if ( !bVoid )
                {
                    sal_Int32 n;
                    if ( Value >>= n )
                        setBlockIncrement( n );
                }
            }
            break;
            case BASEPROPERTY_VISIBLESIZE:
            {
                if ( !bVoid )
                {
                    sal_Int32 n;
                    if ( Value >>= n )
                        setVisibleSize( n );
                }
            }
            break;
            case BASEPROPERTY_ORIENTATION:
            {
                if ( !bVoid )
                {
                    sal_Int32 n;
                    if ( Value >>= n )
                        setOrientation( n );
                }
            }
            break;
            default:
            {
                VCLXWindow::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXScrollBar::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    ScrollBar* pScrollBar = (ScrollBar*)GetWindow();
    if ( pScrollBar )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );

        switch ( nPropType )
        {
            case BASEPROPERTY_SCROLLVALUE:
            {
                aProp <<= (sal_Int32) getValue();
            }
            break;
            case BASEPROPERTY_SCROLLVALUE_MAX:
            {
                aProp <<= (sal_Int32) getMaximum();
            }
            break;
            case BASEPROPERTY_LINEINCREMENT:
            {
                aProp <<= (sal_Int32) getLineIncrement();
            }
            break;
            case BASEPROPERTY_BLOCKINCREMENT:
            {
                aProp <<= (sal_Int32) getBlockIncrement();
            }
            break;
            case BASEPROPERTY_VISIBLESIZE:
            {
                aProp <<= (sal_Int32) getVisibleSize();
            }
            break;
            case BASEPROPERTY_ORIENTATION:
            {
                aProp <<= (sal_Int32) getOrientation();
            }
            break;
            default:
            {
                aProp <<= VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXScrollBar::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_SCROLLBAR_SCROLL:
            if ( maAdjustmentListeners.getLength() )
            {
                ScrollBar* pScrollBar = (ScrollBar*)GetWindow();

                if( pScrollBar )
                {
                    ::com::sun::star::awt::AdjustmentEvent aEvent;
                    aEvent.Source = (::cppu::OWeakObject*)this;
                    aEvent.Value = pScrollBar->GetThumbPos();

                    // set adjustment type
                    ScrollType aType = pScrollBar->GetType();
                    if ( aType == SCROLL_LINEUP || aType == SCROLL_LINEDOWN )
                    {
                        aEvent.Type = ::com::sun::star::awt::AdjustmentType_ADJUST_LINE;
                    }
                    else if ( aType == SCROLL_PAGEUP || aType == SCROLL_PAGEDOWN )
                    {
                        aEvent.Type = ::com::sun::star::awt::AdjustmentType_ADJUST_PAGE;
                    }
                    else if ( aType == SCROLL_DRAG )
                    {
                        aEvent.Type = ::com::sun::star::awt::AdjustmentType_ADJUST_ABS;
                    }

                    maAdjustmentListeners.adjustmentValueChanged( aEvent );
                }
            }
            break;

        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

//  ----------------------------------------------------
//  class VCLXEdit
//  ----------------------------------------------------

VCLXEdit::VCLXEdit() : maTextListeners( *this )
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXEdit::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTextComponent*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTextEditField*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTextLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : VCLXWindow::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXEdit )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextEditField>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextLayoutConstrains>* ) NULL ),
    VCLXWindow::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXEdit::CreateAccessibleContext()
{
    return (::com::sun::star::accessibility::XAccessibleContext*) new VCLXAccessibleEdit( this );
}

void VCLXEdit::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    maTextListeners.disposeAndClear( aObj );
    VCLXWindow::dispose();
}

void VCLXEdit::addTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    GetTextListeners().addInterface( l );
}

void VCLXEdit::removeTextListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    GetTextListeners().removeInterface( l );
}

void VCLXEdit::setText( const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Edit* pEdit = (Edit*)GetWindow();
    if ( pEdit )
    {
        pEdit->SetText( aText );

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pEdit->SetModifyFlag();
        pEdit->Modify();
        SetSynthesizingVCLEvent( sal_False );
    }
}

void VCLXEdit::insertText( const ::com::sun::star::awt::Selection& rSel, const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Edit* pEdit = (Edit*)GetWindow();
    if ( pEdit )
    {
        pEdit->SetSelection( Selection( rSel.Min, rSel.Max ) );
        pEdit->ReplaceSelected( aText );

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pEdit->SetModifyFlag();
        pEdit->Modify();
        SetSynthesizingVCLEvent( sal_False );
    }
}

::rtl::OUString VCLXEdit::getText() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aText;
    Window* pWindow = GetWindow();
    if ( pWindow )
        aText = pWindow->GetText();
    return aText;
}

::rtl::OUString VCLXEdit::getSelectedText() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aText;
    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit)
        aText = pEdit->GetSelected();
    return aText;

}

void VCLXEdit::setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
        pEdit->SetSelection( Selection( aSelection.Min, aSelection.Max ) );
}

::com::sun::star::awt::Selection VCLXEdit::getSelection() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Selection aSel;
    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
        aSel = pEdit->GetSelection();
    return ::com::sun::star::awt::Selection( aSel.Min(), aSel.Max() );
}

sal_Bool VCLXEdit::isEditable() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Edit* pEdit = (Edit*) GetWindow();
    return ( pEdit && !pEdit->IsReadOnly() && pEdit->IsEnabled() ) ? sal_True : sal_False;
}

void VCLXEdit::setEditable( sal_Bool bEditable ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
        pEdit->SetReadOnly( !bEditable );
}


void VCLXEdit::setMaxTextLen( sal_Int16 nLen ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
        pEdit->SetMaxTextLen( nLen );
}

sal_Int16 VCLXEdit::getMaxTextLen() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Edit* pEdit = (Edit*) GetWindow();
    return pEdit ? pEdit->GetMaxTextLen() : 0;
}

void VCLXEdit::setEchoChar( sal_Unicode cEcho ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
        pEdit->SetEchoChar( cEcho );
}

void VCLXEdit::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Edit* pEdit = (Edit*)GetWindow();
    if ( pEdit )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_READONLY:
            {
                sal_Bool b;
                if ( Value >>= b )
                     pEdit->SetReadOnly( b );
            }
            break;
            case BASEPROPERTY_ECHOCHAR:
            {
                sal_Int16 n;
                if ( Value >>= n )
                     pEdit->SetEchoChar( n );
            }
            break;
            case BASEPROPERTY_MAXTEXTLEN:
            {
                sal_Int16 n;
                if ( Value >>= n )
                     pEdit->SetMaxTextLen( n );
            }
            break;
            default:
            {
                VCLXWindow::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXEdit::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    Edit* pEdit = (Edit*)GetWindow();
    if ( pEdit )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_READONLY:
            {
                 aProp <<= (sal_Bool) pEdit->IsReadOnly();
            }
            break;
            case BASEPROPERTY_ECHOCHAR:
            {
                 aProp <<= (sal_Int16) pEdit->GetEchoChar();
            }
            break;
            case BASEPROPERTY_MAXTEXTLEN:
            {
                 aProp <<= (sal_Int16) pEdit->GetMaxTextLen();
            }
            break;
            default:
            {
                aProp <<= VCLXWindow::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

::com::sun::star::awt::Size VCLXEdit::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
        aSz = pEdit->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXEdit::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
    {
        aSz = pEdit->CalcMinimumSize();
        aSz.Height() += 4;
    }
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXEdit::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::awt::Size aSz = rNewSize;
    ::com::sun::star::awt::Size aMinSz = getMinimumSize();
    if ( aSz.Height != aMinSz.Height )
        aSz.Height = aMinSz.Height;

    return aSz;
}

::com::sun::star::awt::Size VCLXEdit::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
    {
        if ( nCols )
            aSz = pEdit->CalcSize( nCols );
        else
            aSz = pEdit->CalcMinimumSize();
    }
    return AWTSize(aSz);
}

void VCLXEdit::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    nLines = 1;
    nCols = 0;
    Edit* pEdit = (Edit*) GetWindow();
    if ( pEdit )
        nCols = pEdit->GetMaxVisChars();
}

void VCLXEdit::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_EDIT_MODIFY:
            if ( GetTextListeners().getLength() )
            {
                ::com::sun::star::awt::TextEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
                GetTextListeners().textChanged( aEvent );
            }
            break;

        default:
            VCLXWindow::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

//  ----------------------------------------------------
//  class VCLXComboBox
//  ----------------------------------------------------
VCLXComboBox::VCLXComboBox()
    : maItemListeners( *this ),
      maActionListeners( *this )
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXComboBox::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XComboBox*, this ) );
    return (aRet.hasValue() ? aRet : VCLXEdit::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXComboBox )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XComboBox>* ) NULL ),
    VCLXEdit::getTypes()
IMPL_XTYPEPROVIDER_END

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXComboBox::CreateAccessibleContext()
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Bool bIsDropDownBox = sal_False;
    ComboBox* pBox = static_cast< ComboBox* >( GetWindow() );
    if ( pBox )
        bIsDropDownBox = ( ( pBox->GetStyle() & WB_DROPDOWN ) == WB_DROPDOWN );

    if ( bIsDropDownBox )
        return (::com::sun::star::accessibility::XAccessibleContext*) new VCLXAccessibleDropDownComboBox( this );
    else
        return (::com::sun::star::accessibility::XAccessibleContext*) new VCLXAccessibleComboBox( this );
}

void VCLXComboBox::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::lang::EventObject aObj;
    aObj.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aObj );
    maActionListeners.disposeAndClear( aObj );
    VCLXEdit::dispose();
}


void VCLXComboBox::addItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maItemListeners.addInterface( l );
}

void VCLXComboBox::removeItemListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maItemListeners.removeInterface( l );
}

void VCLXComboBox::addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maActionListeners.addInterface( l );
}

void VCLXComboBox::removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );
    maActionListeners.removeInterface( l );
}

void VCLXComboBox::addItem( const ::rtl::OUString& aItem, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ComboBox* pBox = (ComboBox*) GetWindow();
    if ( pBox )
        pBox->InsertEntry( aItem, nPos );
}

void VCLXComboBox::addItems( const ::com::sun::star::uno::Sequence< ::rtl::OUString>& aItems, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ComboBox* pBox = (ComboBox*) GetWindow();
    if ( pBox )
    {
        sal_uInt16 nP = nPos;
        for ( sal_uInt16 n = 0; n < aItems.getLength(); n++ )
        {
            pBox->InsertEntry( aItems.getConstArray()[n], nP );
            if ( nPos < 0xFFFF )    // Nicht wenn 0xFFFF, weil LIST_APPEND
                nP++;
        }
    }
}

void VCLXComboBox::removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ComboBox* pBox = (ComboBox*) GetWindow();
    if ( pBox )
    {
        for ( sal_uInt16 n = nCount; n; )
            pBox->RemoveEntry( nPos + (--n) );
    }
}

sal_Int16 VCLXComboBox::getItemCount() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ComboBox* pBox = (ComboBox*) GetWindow();
    return pBox ? pBox->GetEntryCount() : 0;
}

::rtl::OUString VCLXComboBox::getItem( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aItem;
    ComboBox* pBox = (ComboBox*) GetWindow();
    if ( pBox )
        aItem = pBox->GetEntry( nPos );
    return aItem;
}

::com::sun::star::uno::Sequence< ::rtl::OUString> VCLXComboBox::getItems() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    ComboBox* pBox = (ComboBox*) GetWindow();
    if ( pBox )
    {
        sal_uInt16 nEntries = pBox->GetEntryCount();
        aSeq = ::com::sun::star::uno::Sequence< ::rtl::OUString>( nEntries );
        for ( sal_uInt16 n = nEntries; n; )
        {
            --n;
            aSeq.getArray()[n] = pBox->GetEntry( n );
        }
    }
    return aSeq;
}

void VCLXComboBox::setDropDownLineCount( sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ComboBox* pBox = (ComboBox*) GetWindow();
    if ( pBox )
        pBox->SetDropDownLineCount( nLines );
}

sal_Int16 VCLXComboBox::getDropDownLineCount() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int16 nLines = 0;
    ComboBox* pBox = (ComboBox*) GetWindow();
    if ( pBox )
        nLines = pBox->GetDropDownLineCount();
    return nLines;
}

void VCLXComboBox::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ComboBox* pComboBox = (ComboBox*)GetWindow();
    if ( pComboBox )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_LINECOUNT:
            {
                sal_Int16 n;
                if ( Value >>= n )
                     pComboBox->SetDropDownLineCount( n );
            }
            break;
            case BASEPROPERTY_AUTOCOMPLETE:
            {
                sal_Int16 n;
                if ( Value >>= n )
                     pComboBox->EnableAutocomplete( n != 0 );
            }
            break;
            case BASEPROPERTY_STRINGITEMLIST:
            {
                ::com::sun::star::uno::Sequence< ::rtl::OUString> aItems;
                if ( Value >>= aItems )
                {
                    sal_Bool bUpdate = pComboBox->IsUpdateMode();
                    pComboBox->SetUpdateMode( sal_False );
                    pComboBox->Clear();
                    const ::rtl::OUString* pStrings = aItems.getConstArray();
                    sal_Int32 nItems = aItems.getLength();
                    for ( sal_Int32 n = 0; n < nItems; n++ )
                        pComboBox->InsertEntry( pStrings[n], LISTBOX_APPEND );
                    pComboBox->SetUpdateMode( bUpdate );
                }
            }
            break;
            default:
            {
                VCLXEdit::setProperty( PropertyName, Value );

                // #109385# SetBorderStyle is not virtual
                if ( nPropType == BASEPROPERTY_BORDER )
                {
                    sal_uInt16 nBorder;
                    Value >>= nBorder;
                    if ( nBorder )
                        pComboBox->SetBorderStyle( nBorder );
                }
            }
        }
    }
}

::com::sun::star::uno::Any VCLXComboBox::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    ComboBox* pComboBox = (ComboBox*)GetWindow();
    if ( pComboBox )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_LINECOUNT:
            {
                 aProp <<= (sal_Int16)  pComboBox->GetDropDownLineCount();
            }
            break;
            case BASEPROPERTY_AUTOCOMPLETE:
            {
                 aProp <<= (sal_Bool) pComboBox->IsAutocompleteEnabled();
            }
            break;
            case BASEPROPERTY_STRINGITEMLIST:
            {
                sal_uInt16 nItems = pComboBox->GetEntryCount();
                ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq( nItems );
                ::rtl::OUString* pStrings = aSeq.getArray();
                for ( sal_uInt16 n = 0; n < nItems; n++ )
                    pStrings[n] = pComboBox->GetEntry( n );
                aProp <<= aSeq;

            }
            break;
            default:
            {
                aProp <<= VCLXEdit::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

void VCLXComboBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_COMBOBOX_SELECT:
            if ( maItemListeners.getLength() )
            {
                ComboBox* pComboBox = (ComboBox*)GetWindow();
                if( pComboBox )
                {
                    if ( !pComboBox->IsTravelSelect() )
                    {
                        ::com::sun::star::awt::ItemEvent aEvent;
                        aEvent.Source = (::cppu::OWeakObject*)this;
                        aEvent.Highlighted = sal_False;

                        // Bei Mehrfachselektion 0xFFFF, sonst die ID
                        aEvent.Selected = pComboBox->GetEntryPos( pComboBox->GetText() );

                        maItemListeners.itemStateChanged( aEvent );
                    }
                }
            }
            break;

        case VCLEVENT_COMBOBOX_DOUBLECLICK:
            if ( maActionListeners.getLength() )
            {
                ::com::sun::star::awt::ActionEvent aEvent;
                aEvent.Source = (::cppu::OWeakObject*)this;
//              aEvent.ActionCommand = ...;
                maActionListeners.actionPerformed( aEvent );
            }
            break;

        default:
            VCLXEdit::ProcessWindowEvent( rVclWindowEvent );
            break;
    }
}

::com::sun::star::awt::Size VCLXComboBox::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    ComboBox* pComboBox = (ComboBox*) GetWindow();
    if ( pComboBox )
        aSz = pComboBox->CalcMinimumSize();
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXComboBox::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    ComboBox* pComboBox = (ComboBox*) GetWindow();
    if ( pComboBox )
    {
        aSz = pComboBox->CalcMinimumSize();
        if ( pComboBox->GetStyle() & WB_DROPDOWN )
            aSz.Height() += 4;
    }
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXComboBox::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz = VCLSize(rNewSize);
    ComboBox* pComboBox = (ComboBox*) GetWindow();
    if ( pComboBox )
        aSz = pComboBox->CalcAdjustedSize( aSz );
    return AWTSize(aSz);
}

::com::sun::star::awt::Size VCLXComboBox::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    Size aSz;
    ComboBox* pComboBox = (ComboBox*) GetWindow();
    if ( pComboBox )
        aSz = pComboBox->CalcSize( nCols, nLines );
    return AWTSize(aSz);
}

void VCLXComboBox::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    nCols = nLines = 0;
    ComboBox* pComboBox = (ComboBox*) GetWindow();
    if ( pComboBox )
    {
        sal_uInt16 nC, nL;
        pComboBox->GetMaxVisColumnsAndLines( nC, nL );
        nCols = nC;
        nLines = nL;
    }
}

//  ----------------------------------------------------
//  class VCLXFormattedSpinField
//  ----------------------------------------------------
VCLXFormattedSpinField::VCLXFormattedSpinField()
{
}

VCLXFormattedSpinField::~VCLXFormattedSpinField()
{
}

void VCLXFormattedSpinField::setStrictFormat( sal_Bool bStrict )
{
    ::vos::OGuard aGuard( GetMutex() );

    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
        pFormatter->SetStrictFormat( bStrict );
}

sal_Bool VCLXFormattedSpinField::isStrictFormat()
{
    FormatterBase* pFormatter = GetFormatter();
    return pFormatter ? pFormatter->IsStrictFormat() : sal_False;
}


void VCLXFormattedSpinField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_SPIN:
            {
                sal_Bool b;
                if ( Value >>= b )
                {
                    WinBits nStyle = GetWindow()->GetStyle() | WB_SPIN;
                    if ( !b )
                        nStyle &= ~WB_SPIN;
                    GetWindow()->SetStyle( nStyle );
                }
            }
            break;
            case BASEPROPERTY_STRICTFORMAT:
            {
                sal_Bool b;
                if ( Value >>= b )
                {
                     pFormatter->SetStrictFormat( b );
                }
            }
            break;
            default:
            {
                VCLXSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXFormattedSpinField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_TABSTOP:
            {
                aProp <<= (sal_Bool) ( ( GetWindow()->GetStyle() & WB_SPIN ) ? sal_True : sal_False );
            }
            break;
            case BASEPROPERTY_STRICTFORMAT:
            {
                aProp <<= (sal_Bool) pFormatter->IsStrictFormat();
            }
            break;
            default:
            {
                aProp <<= VCLXSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}


//  ----------------------------------------------------
//  class VCLXDateField
//  ----------------------------------------------------
VCLXDateField::VCLXDateField()
{
}

VCLXDateField::~VCLXDateField()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXDateField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XDateField*, this ) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXDateField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDateField>* ) NULL ),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXDateField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_DATE:
            {
                if ( bVoid )
                {
                    ((DateField*)GetWindow())->EnableEmptyFieldValue( sal_True );
                    ((DateField*)GetWindow())->SetEmptyFieldValue();
                }
                else
                {
                    sal_Int32 n;
                    if ( Value >>= n )
                         setDate( n );
                }
            }
            break;
            case BASEPROPERTY_DATEMIN:
            {
                sal_Int32 n;
                if ( Value >>= n )
                     setMin( n );
            }
            break;
            case BASEPROPERTY_DATEMAX:
            {
                sal_Int32 n;
                if ( Value >>= n )
                     setMax( n );
            }
            break;
            case BASEPROPERTY_EXTDATEFORMAT:
            {
                sal_Int16 n;
                if ( Value >>= n )
                    ((DateField*)GetWindow())->SetExtDateFormat( (ExtDateFieldFormat) n );
            }
            break;
            case BASEPROPERTY_DATESHOWCENTURY:
            {
                sal_Bool b;
                if ( Value >>= b )
                     ((DateField*)GetWindow())->SetShowDateCentury( b );
            }
            break;
            default:
            {
                VCLXFormattedSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXDateField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_DATE:
            {
                aProp <<= (sal_Int32) getDate();
            }
            break;
            case BASEPROPERTY_DATEMIN:
            {
                aProp <<= (sal_Int32) getMin();
            }
            break;
            case BASEPROPERTY_DATEMAX:
            {
                aProp <<= (sal_Int32) getMax();
            }
            break;
            case BASEPROPERTY_DATESHOWCENTURY:
            {
                aProp <<= ((DateField*)GetWindow())->IsShowDateCentury();
            }
            break;
            default:
            {
                aProp <<= VCLXFormattedSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}


void VCLXDateField::setDate( sal_Int32 nDate ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
    {
        pDateField->SetDate( nDate );

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pDateField->SetModifyFlag();
        pDateField->Modify();
        SetSynthesizingVCLEvent( sal_False );
    }
}

sal_Int32 VCLXDateField::getDate() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int32 nDate = 0;
    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        nDate = pDateField->GetDate().GetDate();

    return nDate;
}

void VCLXDateField::setMin( sal_Int32 nDate ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        pDateField->SetMin( nDate );
}

sal_Int32 VCLXDateField::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int32 nDate = 0;
    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        nDate = pDateField->GetMin().GetDate();

    return nDate;
}

void VCLXDateField::setMax( sal_Int32 nDate ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        pDateField->SetMax( nDate );
}

sal_Int32 VCLXDateField::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int32 nDate = 0;
    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        nDate = pDateField->GetMax().GetDate();

    return nDate;
}

void VCLXDateField::setFirst( sal_Int32 nDate ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        pDateField->SetFirst( nDate );
}

sal_Int32 VCLXDateField::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int32 nDate = 0;
    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        nDate = pDateField->GetFirst().GetDate();

    return nDate;
}

void VCLXDateField::setLast( sal_Int32 nDate ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        pDateField->SetLast( nDate );
}

sal_Int32 VCLXDateField::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int32 nDate = 0;
    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        nDate = pDateField->GetLast().GetDate();

    return nDate;
}

void VCLXDateField::setLongFormat( sal_Bool bLong ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
        pDateField->SetLongFormat( bLong );
}

sal_Bool VCLXDateField::isLongFormat() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    DateField* pDateField = (DateField*) GetWindow();
    return pDateField ? pDateField->IsLongFormat() : sal_False;
}

void VCLXDateField::setEmpty() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    DateField* pDateField = (DateField*) GetWindow();
    if ( pDateField )
    {
        pDateField->SetEmptyDate();

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pDateField->SetModifyFlag();
        pDateField->Modify();
        SetSynthesizingVCLEvent( sal_False );
    }
}

sal_Bool VCLXDateField::isEmpty() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    DateField* pDateField = (DateField*) GetWindow();
    return pDateField ? pDateField->IsEmptyDate() : sal_False;
}

void VCLXDateField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXDateField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    return VCLXFormattedSpinField::isStrictFormat();
}


//  ----------------------------------------------------
//  class VCLXTimeField
//  ----------------------------------------------------
VCLXTimeField::VCLXTimeField()
{
}

VCLXTimeField::~VCLXTimeField()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXTimeField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTimeField*, this ) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXTimeField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTimeField>* ) NULL ),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXTimeField::setTime( sal_Int32 nTime ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
    {
        pTimeField->SetTime( nTime );

        // #107218# Call same listeners like VCL would do after user interaction
        SetSynthesizingVCLEvent( sal_True );
        pTimeField->SetModifyFlag();
        pTimeField->Modify();
        SetSynthesizingVCLEvent( sal_False );
    }
}

sal_Int32 VCLXTimeField::getTime() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int32 nTime = 0;
    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        nTime = pTimeField->GetTime().GetTime();

    return nTime;
}

void VCLXTimeField::setMin( sal_Int32 nTime ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        pTimeField->SetMin( nTime );
}

sal_Int32 VCLXTimeField::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int32 nTime = 0;
    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        nTime = pTimeField->GetMin().GetTime();

    return nTime;
}

void VCLXTimeField::setMax( sal_Int32 nTime ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        pTimeField->SetMax( nTime );
}

sal_Int32 VCLXTimeField::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int32 nTime = 0;
    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        nTime = pTimeField->GetMax().GetTime();

    return nTime;
}

void VCLXTimeField::setFirst( sal_Int32 nTime ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        pTimeField->SetFirst( nTime );
}

sal_Int32 VCLXTimeField::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int32 nTime = 0;
    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        nTime = pTimeField->GetFirst().GetTime();

    return nTime;
}

void VCLXTimeField::setLast( sal_Int32 nTime ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        pTimeField->SetLast( nTime );
}

sal_Int32 VCLXTimeField::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    sal_Int32 nTime = 0;
    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        nTime = pTimeField->GetLast().GetTime();

    return nTime;
}

void VCLXTimeField::setEmpty() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TimeField* pTimeField = (TimeField*) GetWindow();
    if ( pTimeField )
        pTimeField->SetEmptyTime();
}

sal_Bool VCLXTimeField::isEmpty() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    TimeField* pTimeField = (TimeField*) GetWindow();
    return pTimeField ? pTimeField->IsEmptyTime() : sal_False;
}

void VCLXTimeField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXTimeField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    return VCLXFormattedSpinField::isStrictFormat();
}


void VCLXTimeField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_TIME:
            {
                if ( bVoid )
                {
                    ((TimeField*)GetWindow())->EnableEmptyFieldValue( sal_True );
                    ((TimeField*)GetWindow())->SetEmptyFieldValue();
                }
                else
                {
                    sal_Int32 n;
                    if ( Value >>= n )
                         setTime( n );
                }
            }
            break;
            case BASEPROPERTY_TIMEMIN:
            {
                sal_Int32 n;
                if ( Value >>= n )
                     setMin( n );
            }
            break;
            case BASEPROPERTY_TIMEMAX:
            {
                sal_Int32 n;
                if ( Value >>= n )
                     setMax( n );
            }
            break;
            case BASEPROPERTY_EXTTIMEFORMAT:
            {
                sal_Int16 n;
                if ( Value >>= n )
                    ((TimeField*)GetWindow())->SetExtFormat( (ExtTimeFieldFormat) n );
            }
            break;
            default:
            {
                VCLXFormattedSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXTimeField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    if ( GetWindow() )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_TIME:
            {
                aProp <<= (sal_Int32) getTime();
            }
            break;
            case BASEPROPERTY_TIMEMIN:
            {
                aProp <<= (sal_Int32) getMin();
            }
            break;
            case BASEPROPERTY_TIMEMAX:
            {
                aProp <<= (sal_Int32) getMax();
            }
            break;
            default:
            {
                aProp <<= VCLXFormattedSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

//  ----------------------------------------------------
//  class VCLXNumericField
//  ----------------------------------------------------
VCLXNumericField::VCLXNumericField()
{
}

VCLXNumericField::~VCLXNumericField()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXNumericField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XNumericField*, this ) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXNumericField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XNumericField>* ) NULL ),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXNumericField::setValue( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    if ( pNumericFormatter )
    {
        // z.B. 105, 2 Digits => 1,05
        // ein float 1,05 muss also eine 105 einstellen...
        pNumericFormatter->SetValue(
            (long)ImplCalcLongValue( Value, pNumericFormatter->GetDecimalDigits() ) );

        // #107218# Call same listeners like VCL would do after user interaction
        Edit* pEdit = (Edit*)GetWindow();
        if ( pEdit )
        {
            SetSynthesizingVCLEvent( sal_True );
            pEdit->SetModifyFlag();
            pEdit->Modify();
            SetSynthesizingVCLEvent( sal_False );
        }
    }
}

double VCLXNumericField::getValue() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    return pNumericFormatter
        ? ImplCalcDoubleValue( (double)pNumericFormatter->GetValue(), pNumericFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setMin( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    if ( pNumericFormatter )
        pNumericFormatter->SetMin(
            (long)ImplCalcLongValue( Value, pNumericFormatter->GetDecimalDigits() ) );
}

double VCLXNumericField::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    return pNumericFormatter
        ? ImplCalcDoubleValue( (double)pNumericFormatter->GetMin(), pNumericFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setMax( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    if ( pNumericFormatter )
        pNumericFormatter->SetMax(
            (long)ImplCalcLongValue( Value, pNumericFormatter->GetDecimalDigits() ) );
}

double VCLXNumericField::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    return pNumericFormatter
        ? ImplCalcDoubleValue( (double)pNumericFormatter->GetMax(), pNumericFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    NumericField* pNumericField = (NumericField*) GetWindow();
    if ( pNumericField )
        pNumericField->SetFirst(
            (long)ImplCalcLongValue( Value, pNumericField->GetDecimalDigits() ) );
}

double VCLXNumericField::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    NumericField* pNumericField = (NumericField*) GetWindow();
    return pNumericField
        ? ImplCalcDoubleValue( (double)pNumericField->GetFirst(), pNumericField->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setLast( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    NumericField* pNumericField = (NumericField*) GetWindow();
    if ( pNumericField )
        pNumericField->SetLast(
            (long)ImplCalcLongValue( Value, pNumericField->GetDecimalDigits() ) );
}

double VCLXNumericField::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    NumericField* pNumericField = (NumericField*) GetWindow();
    return pNumericField
        ? ImplCalcDoubleValue( (double)pNumericField->GetLast(), pNumericField->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXNumericField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    return VCLXFormattedSpinField::isStrictFormat();
}


void VCLXNumericField::setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    NumericField* pNumericField = (NumericField*) GetWindow();
    if ( pNumericField )
        pNumericField->SetSpinSize(
            (long)ImplCalcLongValue( Value, pNumericField->GetDecimalDigits() ) );
}

double VCLXNumericField::getSpinSize() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    NumericField* pNumericField = (NumericField*) GetWindow();
    return pNumericField
        ? ImplCalcDoubleValue( (double)pNumericField->GetSpinSize(), pNumericField->GetDecimalDigits() )
        : 0;
}

void VCLXNumericField::setDecimalDigits( sal_Int16 Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    if ( pNumericFormatter )
    {
        double n = getValue();
        pNumericFormatter->SetDecimalDigits( Value );
        setValue( n );
       }
}

sal_Int16 VCLXNumericField::getDecimalDigits() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    NumericFormatter* pNumericFormatter = (NumericFormatter*) GetFormatter();
    return pNumericFormatter ? pNumericFormatter->GetDecimalDigits() : 0;
}

void VCLXNumericField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VALUE_DOUBLE:
            {
                if ( bVoid )
                {
                    ((NumericField*)GetWindow())->EnableEmptyFieldValue( sal_True );
                    ((NumericField*)GetWindow())->SetEmptyFieldValue();
                }
                else
                {
                    double d;
                    if ( Value >>= d )
                         setValue( d );
                }
            }
            break;
            case BASEPROPERTY_VALUEMIN_DOUBLE:
            {
                double d;
                if ( Value >>= d )
                     setMin( d );
            }
            break;
            case BASEPROPERTY_VALUEMAX_DOUBLE:
            {
                double d;
                if ( Value >>= d )
                     setMax( d );
            }
            break;
            case BASEPROPERTY_VALUESTEP_DOUBLE:
            {
                double d;
                if ( Value >>= d )
                     setSpinSize( d );
            }
            break;
            case BASEPROPERTY_DECIMALACCURACY:
            {
                sal_Int16 n;
                if ( Value >>= n )
                     setDecimalDigits( n );
            }
            break;
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                sal_Bool b;
                if ( Value >>= b )
                     ((NumericField*)GetWindow())->SetUseThousandSep( b );
            }
            break;
            default:
            {
                VCLXFormattedSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXNumericField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VALUE_DOUBLE:
            {
                aProp <<= (double) getValue();
            }
            break;
            case BASEPROPERTY_VALUEMIN_DOUBLE:
            {
                aProp <<= (double) getMin();
            }
            break;
            case BASEPROPERTY_VALUEMAX_DOUBLE:
            {
                aProp <<= (double) getMax();
            }
            break;
            case BASEPROPERTY_VALUESTEP_DOUBLE:
            {
                aProp <<= (double) getSpinSize();
            }
            break;
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                aProp <<= (sal_Bool) ((NumericField*)GetWindow())->IsUseThousandSep();
            }
            break;
            default:
            {
                aProp <<= VCLXFormattedSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}


//  ----------------------------------------------------
//  class VCLXCurrencyField
//  ----------------------------------------------------
VCLXCurrencyField::VCLXCurrencyField()
{
}

VCLXCurrencyField::~VCLXCurrencyField()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXCurrencyField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XCurrencyField*, this ) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXCurrencyField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XCurrencyField>* ) NULL ),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXCurrencyField::setValue( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    if ( pCurrencyFormatter )
    {
        // z.B. 105, 2 Digits => 1,05
        // ein float 1,05 muss also eine 105 einstellen...
        pCurrencyFormatter->SetValue(
            ImplCalcLongValue( Value, pCurrencyFormatter->GetDecimalDigits() ) );

        // #107218# Call same listeners like VCL would do after user interaction
        Edit* pEdit = (Edit*)GetWindow();
        if ( pEdit )
        {
            SetSynthesizingVCLEvent( sal_True );
            pEdit->SetModifyFlag();
            pEdit->Modify();
            SetSynthesizingVCLEvent( sal_False );
        }
    }
}

double VCLXCurrencyField::getValue() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    return pCurrencyFormatter
        ? ImplCalcDoubleValue( (double)pCurrencyFormatter->GetValue(), pCurrencyFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setMin( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    if ( pCurrencyFormatter )
        pCurrencyFormatter->SetMin(
            ImplCalcLongValue( Value, pCurrencyFormatter->GetDecimalDigits() ) );
}

double VCLXCurrencyField::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    return pCurrencyFormatter
        ? ImplCalcDoubleValue( (double)pCurrencyFormatter->GetMin(), pCurrencyFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setMax( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    if ( pCurrencyFormatter )
        pCurrencyFormatter->SetMax(
            ImplCalcLongValue( Value, pCurrencyFormatter->GetDecimalDigits() ) );
}

double VCLXCurrencyField::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    return pCurrencyFormatter
        ? ImplCalcDoubleValue( (double)pCurrencyFormatter->GetMax(), pCurrencyFormatter->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    LongCurrencyField* pCurrencyField = (LongCurrencyField*) GetWindow();
    if ( pCurrencyField )
        pCurrencyField->SetFirst(
            ImplCalcLongValue( Value, pCurrencyField->GetDecimalDigits() ) );
}

double VCLXCurrencyField::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    LongCurrencyField* pCurrencyField = (LongCurrencyField*) GetWindow();
    return pCurrencyField
        ? ImplCalcDoubleValue( (double)pCurrencyField->GetFirst(), pCurrencyField->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setLast( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    LongCurrencyField* pCurrencyField = (LongCurrencyField*) GetWindow();
    if ( pCurrencyField )
        pCurrencyField->SetLast(
            ImplCalcLongValue( Value, pCurrencyField->GetDecimalDigits() ) );
}

double VCLXCurrencyField::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    LongCurrencyField* pCurrencyField = (LongCurrencyField*) GetWindow();
    return pCurrencyField
        ? ImplCalcDoubleValue( (double)pCurrencyField->GetLast(), pCurrencyField->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setSpinSize( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    LongCurrencyField* pCurrencyField = (LongCurrencyField*) GetWindow();
    if ( pCurrencyField )
        pCurrencyField->SetSpinSize(
            ImplCalcLongValue( Value, pCurrencyField->GetDecimalDigits() ) );
}

double VCLXCurrencyField::getSpinSize() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    LongCurrencyField* pCurrencyField = (LongCurrencyField*) GetWindow();
    return pCurrencyField
        ? ImplCalcDoubleValue( (double)pCurrencyField->GetSpinSize(), pCurrencyField->GetDecimalDigits() )
        : 0;
}

void VCLXCurrencyField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXCurrencyField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    return VCLXFormattedSpinField::isStrictFormat();
}


void VCLXCurrencyField::setDecimalDigits( sal_Int16 Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    if ( pCurrencyFormatter )
    {
        double n = getValue();
        pCurrencyFormatter->SetDecimalDigits( Value );
        setValue( n );
       }
}

sal_Int16 VCLXCurrencyField::getDecimalDigits() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    LongCurrencyFormatter* pCurrencyFormatter = (LongCurrencyFormatter*) GetFormatter();
    return pCurrencyFormatter ? pCurrencyFormatter->GetDecimalDigits() : 0;
}

void VCLXCurrencyField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        sal_Bool bVoid = Value.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID;

        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VALUE_DOUBLE:
            {
                if ( bVoid )
                {
                    ((LongCurrencyField*)GetWindow())->EnableEmptyFieldValue( sal_True );
                    ((LongCurrencyField*)GetWindow())->SetEmptyFieldValue();
                }
                else
                {
                    double d;
                    if ( Value >>= d )
                         setValue( d );
                }
            }
            break;
            case BASEPROPERTY_VALUEMIN_DOUBLE:
            {
                double d;
                if ( Value >>= d )
                     setMin( d );
            }
            break;
            case BASEPROPERTY_VALUEMAX_DOUBLE:
            {
                double d;
                if ( Value >>= d )
                     setMax( d );
            }
            break;
            case BASEPROPERTY_VALUESTEP_DOUBLE:
            {
                double d;
                if ( Value >>= d )
                     setSpinSize( d );
            }
            break;
            case BASEPROPERTY_DECIMALACCURACY:
            {
                sal_Int16 n;
                if ( Value >>= n )
                     setDecimalDigits( n );
            }
            break;
            case BASEPROPERTY_CURRENCYSYMBOL:
            {
                ::rtl::OUString aString;
                if ( Value >>= aString )
                     ((LongCurrencyField*)GetWindow())->SetCurrencySymbol( aString );
            }
            break;
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                sal_Bool b;
                if ( Value >>= b )
                     ((LongCurrencyField*)GetWindow())->SetUseThousandSep( b );
            }
            break;
            default:
            {
                VCLXFormattedSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXCurrencyField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    FormatterBase* pFormatter = GetFormatter();
    if ( pFormatter )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_VALUE_DOUBLE:
            {
                aProp <<= (double) getValue();
            }
            break;
            case BASEPROPERTY_VALUEMIN_DOUBLE:
            {
                aProp <<= (double) getMin();
            }
            break;
            case BASEPROPERTY_VALUEMAX_DOUBLE:
            {
                aProp <<= (double) getMax();
            }
            break;
            case BASEPROPERTY_VALUESTEP_DOUBLE:
            {
                aProp <<= (double) getSpinSize();
            }
            break;
            case BASEPROPERTY_CURRENCYSYMBOL:
            {
                aProp <<= ::rtl::OUString( ((LongCurrencyField*)GetWindow())->GetCurrencySymbol() );
            }
            break;
            case BASEPROPERTY_NUMSHOWTHOUSANDSEP:
            {
                aProp <<= (sal_Bool) ((LongCurrencyField*)GetWindow())->IsUseThousandSep();
            }
            break;
            default:
            {
                aProp <<= VCLXFormattedSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

//  ----------------------------------------------------
//  class VCLXPatternField
//  ----------------------------------------------------
VCLXPatternField::VCLXPatternField()
{
}

VCLXPatternField::~VCLXPatternField()
{
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any VCLXPatternField::queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XPatternField*, this ) );
    return (aRet.hasValue() ? aRet : VCLXFormattedSpinField::queryInterface( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( VCLXPatternField )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPatternField>* ) NULL ),
    VCLXFormattedSpinField::getTypes()
IMPL_XTYPEPROVIDER_END

void VCLXPatternField::setMasks( const ::rtl::OUString& EditMask, const ::rtl::OUString& LiteralMask ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    PatternField* pPatternField = (PatternField*) GetWindow();
    if ( pPatternField )
    {
        pPatternField->SetMask( ByteString( UniString( EditMask ), RTL_TEXTENCODING_ASCII_US ), LiteralMask );
    }
}

void VCLXPatternField::getMasks( ::rtl::OUString& EditMask, ::rtl::OUString& LiteralMask ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    PatternField* pPatternField = (PatternField*) GetWindow();
    if ( pPatternField )
    {
        EditMask = String( pPatternField->GetEditMask(), RTL_TEXTENCODING_ASCII_US );
        LiteralMask = pPatternField->GetLiteralMask();
    }
}

void VCLXPatternField::setString( const ::rtl::OUString& Str ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    PatternField* pPatternField = (PatternField*) GetWindow();
    if ( pPatternField )
    {
        pPatternField->SetString( Str );
    }
}

::rtl::OUString VCLXPatternField::getString() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::rtl::OUString aString;
    PatternField* pPatternField = (PatternField*) GetWindow();
    if ( pPatternField )
        aString = pPatternField->GetString();
    return aString;
}

void VCLXPatternField::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    VCLXFormattedSpinField::setStrictFormat( bStrict );
}

sal_Bool VCLXPatternField::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    return VCLXFormattedSpinField::isStrictFormat();
}

void VCLXPatternField::setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    if ( GetWindow() )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_EDITMASK:
            case BASEPROPERTY_LITERALMASK:
            {
                ::rtl::OUString aString;
                if ( Value >>= aString )
                {
                    ::rtl::OUString aEditMask, aLiteralMask;
                    getMasks( aEditMask, aLiteralMask );
                    if ( nPropType == BASEPROPERTY_EDITMASK )
                        aEditMask = aString;
                    else
                        aLiteralMask = aString;
                     setMasks( aEditMask, aLiteralMask );
                }
            }
            break;
            default:
            {
                VCLXFormattedSpinField::setProperty( PropertyName, Value );
            }
        }
    }
}

::com::sun::star::uno::Any VCLXPatternField::getProperty( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aGuard( GetMutex() );

    ::com::sun::star::uno::Any aProp;
    if ( GetWindow() )
    {
        sal_uInt16 nPropType = GetPropertyId( PropertyName );
        switch ( nPropType )
        {
            case BASEPROPERTY_EDITMASK:
            case BASEPROPERTY_LITERALMASK:
            {
                ::rtl::OUString aEditMask, aLiteralMask;
                getMasks( aEditMask, aLiteralMask );
                if ( nPropType == BASEPROPERTY_EDITMASK )
                    aProp <<= aEditMask;
                else
                    aProp <<= aLiteralMask;
            }
            break;
            default:
            {
                aProp <<= VCLXFormattedSpinField::getProperty( PropertyName );
            }
        }
    }
    return aProp;
}

//  ----------------------------------------------------
//  class VCLXToolBox
//  ----------------------------------------------------
VCLXToolBox::VCLXToolBox()
{
}

VCLXToolBox::~VCLXToolBox()
{
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext > VCLXToolBox::CreateAccessibleContext()
{
    return (::com::sun::star::accessibility::XAccessibleContext*) new VCLXAccessibleToolBox( this );
}

