/*************************************************************************
 *
 *  $RCSfile: unocontrols.cxx,v $
 *
 *  $Revision: 1.58 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:03:22 $
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

#ifndef _COM_SUN_STAR_AWT_XTEXTAREA_HPP_
#include <com/sun/star/awt/XTextArea.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLWINDOWPEER_HPP_
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif


#ifndef TOOLKIT_FORMATTED_CONTROL_HXX
#include <toolkit/controls/formattedcontrol.hxx>
#endif
#ifndef _TOOLKIT_HELPER_UNOCONTROLS_HXX_
#include <toolkit/controls/unocontrols.hxx>
#endif
#ifndef _TOOLKIT_HELPERS_GEOMETRYCONTROLMODEL_HXX_
#include <toolkit/controls/geometrycontrolmodel.hxx>
#endif
#ifndef _TOOLKIT_CONTROLS_STDTABCONTROLLER_HXX_
#include <toolkit/controls/stdtabcontroller.hxx>
#endif
#ifndef _TOOLKIT_HELPER_PROPERTY_HXX_
#include <toolkit/helper/property.hxx>
#endif
#ifndef _TOOLKIT_HELPER_UNOPROPERTYARRAYHELPER_HXX_
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#endif
#ifndef _TOOLKIT_HELPER_UNOMEMORYSTREAM_HXX_
#include <toolkit/helper/unomemorystream.hxx>
#endif
#ifndef _TOOLKIT_HELPER_SERVICENAMES_HXX_
#include <toolkit/helper/servicenames.hxx>
#endif
#ifndef _TOOLKIT_HELPER_MACROS_HXX_
#include <toolkit/helper/macros.hxx>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include <tools/date.hxx>
#include <tools/time.hxx>

using namespace ::com::sun::star;
using namespace ::toolkit;


//  ----------------------------------------------------
//  class UnoControlEditModel
//  ----------------------------------------------------
UnoControlEditModel::UnoControlEditModel()
{
    ImplRegisterProperty( BASEPROPERTY_ALIGN );
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ECHOCHAR );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HARDLINEBREAKS );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_HSCROLL );
    ImplRegisterProperty( BASEPROPERTY_MAXTEXTLEN );
    ImplRegisterProperty( BASEPROPERTY_MULTILINE );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
    ImplRegisterProperty( BASEPROPERTY_VSCROLL );
}

::rtl::OUString UnoControlEditModel::getServiceName( ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlEditModel );
}

uno::Any UnoControlEditModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlEdit );
        return aAny;
    }

    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlEditModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlEditModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


//  ----------------------------------------------------
//  class UnoEditControl
//  ----------------------------------------------------
UnoEditControl::UnoEditControl()
    : maTextListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;

    mnMaxTextLen = 0;
    mbSetMaxTextLenInPeer = FALSE;

    mbSetTextInPeer = FALSE;

}

::rtl::OUString UnoEditControl::GetComponentServiceName()
{
    ::rtl::OUString aName( ::rtl::OUString::createFromAscii( "Edit" ) );
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_MULTILINE ) );
    sal_Bool b;
    if ( ( aVal >>= b ) && b )
        aName= ::rtl::OUString::createFromAscii( "MultiLineEdit" );
    return aName;
}

void UnoEditControl::ImplSetPeerProperty( const ::rtl::OUString& rPropName, const uno::Any& rVal )
{
    sal_Bool bDone = sal_False;
    if ( getPeer().is() && ( GetPropertyId( rPropName ) == BASEPROPERTY_TEXT ) )
    {
        // #96986# use setText(), or text listener will not be called.
        uno::Reference < awt::XTextComponent > xTextComponent( getPeer(), uno::UNO_QUERY );
        if ( xTextComponent.is() )
        {
            ::rtl::OUString aText;
            rVal >>= aText;
            xTextComponent->setText( aText );
            bDone = sal_True;
        }
    }

    if ( !bDone )
        UnoControlBase::ImplSetPeerProperty( rPropName, rVal );
}

// uno::XInterface
uno::Any UnoEditControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XTextComponent*, this ),
                                        SAL_STATIC_CAST( awt::XTextListener*, this ),
                                        SAL_STATIC_CAST( lang::XEventListener*, SAL_STATIC_CAST( awt::XTextListener*, this ) ),
                                        SAL_STATIC_CAST( awt::XLayoutConstrains*, this ),
                                        SAL_STATIC_CAST( awt::XTextLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoEditControl )
getCppuType( ( uno::Reference< awt::XTextComponent>* ) NULL ),
getCppuType( ( uno::Reference< awt::XTextListener>* ) NULL ),
getCppuType( ( uno::Reference< awt::XLayoutConstrains>* ) NULL ),
getCppuType( ( uno::Reference< awt::XTextLayoutConstrains>* ) NULL ),
UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END


void UnoEditControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maTextListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoEditControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference< awt::XTextComponent > xText( getPeer(), uno::UNO_QUERY );
    xText->addTextListener( this );

    if ( mbSetMaxTextLenInPeer )
        xText->setMaxTextLen( mnMaxTextLen );
    if ( mbSetTextInPeer )
        xText->setText( maText );
}

void UnoEditControl::textChanged(const awt::TextEvent& e) throw(uno::RuntimeException)
{
    uno::Reference< awt::XTextComponent > xText( getPeer(), uno::UNO_QUERY );

    if ( ImplHasProperty( BASEPROPERTY_TEXT ) )
    {
        uno::Any aAny;
        aAny <<= xText->getText();
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TEXT ), aAny, sal_False );
    }
    else
    {
        maText = xText->getText();
    }

    if ( maTextListeners.getLength() )
        maTextListeners.textChanged( e );
}

void UnoEditControl::addTextListener(const uno::Reference< awt::XTextListener > & l) throw(uno::RuntimeException)
{
    maTextListeners.addInterface( l );
}

void UnoEditControl::removeTextListener(const uno::Reference< awt::XTextListener > & l) throw(uno::RuntimeException)
{
    maTextListeners.removeInterface( l );
}

void UnoEditControl::setText( const ::rtl::OUString& aText ) throw(uno::RuntimeException)
{
    if ( ImplHasProperty( BASEPROPERTY_TEXT ) )
    {
        uno::Any aAny;
        aAny <<= aText;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TEXT ), aAny, sal_True );
    }
    else
    {
        maText = aText;
        mbSetTextInPeer = TRUE;
        if ( getPeer().is() )
        {
            uno::Reference < awt::XTextComponent > xText( getPeer(), uno::UNO_QUERY );
            xText->setText( maText );
        }
    }

    // Setting the property to the VCLXWindow doesn't call textChanged
    if ( maTextListeners.getLength() )
    {
        ::com::sun::star::awt::TextEvent aEvent;
        aEvent.Source = (::cppu::OWeakObject*)this;

        maTextListeners.textChanged( aEvent );
    }
}

void UnoEditControl::insertText( const awt::Selection& rSel, const ::rtl::OUString& rNewText ) throw(uno::RuntimeException)
{
    ::rtl::OUString aOldText = getText();
    ::rtl::OUString  aNewText = aOldText.replaceAt( rSel.Min, rSel.Max - rSel.Min, rNewText );
    setText( aNewText );
}

::rtl::OUString UnoEditControl::getText() throw(uno::RuntimeException)
{
    ::rtl::OUString aText = maText;

    if ( ImplHasProperty( BASEPROPERTY_TEXT ) )
        aText = ImplGetPropertyValue_UString( BASEPROPERTY_TEXT );

    return aText;
}

::rtl::OUString UnoEditControl::getSelectedText( void ) throw(uno::RuntimeException)
{
    ::rtl::OUString aSelected;
    if ( getPeer().is() )
    {
        uno::Reference< awt::XTextComponent > xText( getPeer(), uno::UNO_QUERY );
        aSelected = xText->getSelectedText();
    }
    return aSelected;
}

void UnoEditControl::setSelection( const awt::Selection& aSelection ) throw(uno::RuntimeException)
{
    if ( getPeer().is() )
    {
        uno::Reference< awt::XTextComponent > xText( getPeer(), uno::UNO_QUERY );
        xText->setSelection( aSelection );
    }
}

awt::Selection UnoEditControl::getSelection( void ) throw(uno::RuntimeException)
{
    awt::Selection aSel;
    if ( getPeer().is() )
    {
        uno::Reference< awt::XTextComponent > xText( getPeer(), uno::UNO_QUERY );
        aSel = xText->getSelection();
    }
    return aSel;
}

sal_Bool UnoEditControl::isEditable( void ) throw(uno::RuntimeException)
{
    return !ImplGetPropertyValue_BOOL( BASEPROPERTY_READONLY );
}

void UnoEditControl::setEditable( sal_Bool bEditable ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= (sal_Bool)!bEditable;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_READONLY ), aAny, sal_True );
}

sal_Int16 UnoEditControl::getMaxTextLen() throw(uno::RuntimeException)
{
    sal_Int16 nMaxLen = mnMaxTextLen;

    if ( ImplHasProperty( BASEPROPERTY_MAXTEXTLEN) )
        nMaxLen = ImplGetPropertyValue_INT16( BASEPROPERTY_MAXTEXTLEN );

    return nMaxLen;
}

void UnoEditControl::setMaxTextLen( sal_Int16 nLen ) throw(uno::RuntimeException)
{
    if ( ImplHasProperty( BASEPROPERTY_MAXTEXTLEN) )
    {
        uno::Any aAny;
        aAny <<= (sal_Int16)nLen;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_MAXTEXTLEN ), aAny, sal_True );
    }
    else
    {
        mnMaxTextLen = nLen;
        mbSetMaxTextLenInPeer = TRUE;
        if ( getPeer().is() )
        {
            uno::Reference < awt::XTextComponent > xText( getPeer(), uno::UNO_QUERY );
            xText->setMaxTextLen( mnMaxTextLen );
        }
    }
}

awt::Size UnoEditControl::getMinimumSize(  ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

awt::Size UnoEditControl::getPreferredSize(  ) throw(uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

awt::Size UnoEditControl::calcAdjustedSize( const awt::Size& rNewSize ) throw(uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

awt::Size UnoEditControl::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize( nCols, nLines );
}

void UnoEditControl::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(uno::RuntimeException)
{
    Impl_getColumnsAndLines( nCols, nLines );
}


//  ----------------------------------------------------
//  class UnoControlFileControlModel
//  ----------------------------------------------------
UnoControlFileControlModel::UnoControlFileControlModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
}

::rtl::OUString UnoControlFileControlModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlFileControlModel );
}

uno::Any UnoControlFileControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlFileControl );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlFileControlModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlFileControlModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//  ----------------------------------------------------
//  class UnoFileControl
//  ----------------------------------------------------
UnoFileControl::UnoFileControl()
{
}

::rtl::OUString UnoFileControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "filecontrol" );
}

//  ----------------------------------------------------
//  class UnoControlButtonModel
//  ----------------------------------------------------
UnoControlButtonModel::UnoControlButtonModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTBUTTON );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_IMAGEALIGN );
    ImplRegisterProperty( BASEPROPERTY_IMAGEURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_PUSHBUTTONTYPE );
    ImplRegisterProperty( BASEPROPERTY_STATE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
}

uno::Any UnoControlButtonModel::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType, SAL_STATIC_CAST( awt::XImageProducer*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlModel::queryAggregation( rType ));
}

::rtl::OUString UnoControlButtonModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlButtonModel );
}

uno::Any UnoControlButtonModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlButton );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}


::cppu::IPropertyArrayHelper& UnoControlButtonModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlButtonModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

void UnoControlButtonModel::addConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException)
{
    maListeners.push_back( xConsumer );
}

void UnoControlButtonModel::removeConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException)
{
    maListeners.remove( xConsumer );
}

void UnoControlButtonModel::startProduction(  ) throw (::com::sun::star::uno::RuntimeException)
{
    uno::Sequence<uno::Any> aArgs(1);
    aArgs.getArray()[0] = getPropertyValue( GetPropertyName( BASEPROPERTY_IMAGEURL ) );
    uno::Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    uno::Reference< awt::XImageProducer > xImageProducer( xMSF->createInstanceWithArguments( ::rtl::OUString::createFromAscii( "com.sun.star.awt.ImageProducer" ), aArgs ), uno::UNO_QUERY );
    if ( xImageProducer.is() )
    {
        std::list< uno::Reference< awt::XImageConsumer > >::iterator aIter( maListeners.begin() );
        while ( aIter != maListeners.end() )
        {
            xImageProducer->addConsumer( *aIter );
            aIter++;
        }
        xImageProducer->startProduction();
    }
}

//  ----------------------------------------------------
//  class UnoButtonControl
//  ----------------------------------------------------
UnoButtonControl::UnoButtonControl()
    : maActionListeners( *this )
{
    maComponentInfos.nWidth = 50;
    maComponentInfos.nHeight = 14;
}

::rtl::OUString UnoButtonControl::GetComponentServiceName()
{
    ::rtl::OUString aName( ::rtl::OUString::createFromAscii( "pushbutton" ) );
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_PUSHBUTTONTYPE ) );
    sal_Int16 n;
    if ( ( aVal >>= n ) && n )
    {
        // Use PushButtonType later when available...
        switch ( n )
        {
            case 1 /*PushButtonType::OK*/:      aName= ::rtl::OUString::createFromAscii( "okbutton" );
                                                break;
            case 2 /*PushButtonType::CANCEL*/:  aName= ::rtl::OUString::createFromAscii( "cancelbutton" );
                                                break;
            case 3 /*PushButtonType::HELP*/:    aName= ::rtl::OUString::createFromAscii( "helpbutton" );
                                                break;
            default:
            {
                DBG_ERROR( "Unknown Button Type!" );
            }
        }
    }
    return aName;
}

// uno::XInterface
uno::Any UnoButtonControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XButton*, this ),
                                        SAL_STATIC_CAST( awt::XLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoButtonControl )
getCppuType( ( uno::Reference< awt::XButton>* ) NULL ),
getCppuType( ( uno::Reference< awt::XLayoutConstrains>* ) NULL ),
UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoButtonControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoButtonControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XButton > xButton( getPeer(), uno::UNO_QUERY );
    xButton->setActionCommand( maActionCommand );
    if ( maActionListeners.getLength() )
        xButton->addActionListener( &maActionListeners );
}

void UnoButtonControl::ImplSetPeerProperty( const ::rtl::OUString& rPropName, const uno::Any& rVal )
{
    sal_uInt16 nType = GetPropertyId( rPropName );
    if ( getPeer().is() && ( nType == BASEPROPERTY_IMAGEURL ) )
    {
        uno::Reference < awt::XImageProducer > xImgProd( getModel(), uno::UNO_QUERY );
        uno::Reference < awt::XImageConsumer > xImgCons( getPeer(), uno::UNO_QUERY );

        if ( xImgProd.is() && xImgCons.is() )
        {
            xImgProd->addConsumer( xImgCons );
            xImgProd->startProduction();
            xImgProd->removeConsumer( xImgCons );
        }
    }
    else
        UnoControl::ImplSetPeerProperty( rPropName, rVal );
}


void UnoButtonControl::addActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    maActionListeners.addInterface( l );
    if( getPeer().is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XButton >  xButton( getPeer(), uno::UNO_QUERY );
        xButton->addActionListener( &maActionListeners );
    }
}

void UnoButtonControl::removeActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    if( getPeer().is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XButton >  xButton( getPeer(), uno::UNO_QUERY );
        xButton->removeActionListener( &maActionListeners );
    }
    maActionListeners.removeInterface( l );
}

void UnoButtonControl::setLabel( const ::rtl::OUString&  rLabel ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= rLabel;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

void UnoButtonControl::setActionCommand( const ::rtl::OUString& rCommand ) throw(uno::RuntimeException)
{
    maActionCommand = rCommand;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XButton >  xButton( getPeer(), uno::UNO_QUERY );
        xButton->setActionCommand( rCommand );
    }
}

awt::Size UnoButtonControl::getMinimumSize(  ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

awt::Size UnoButtonControl::getPreferredSize(  ) throw(uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

awt::Size UnoButtonControl::calcAdjustedSize( const awt::Size& rNewSize ) throw(uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

//  ----------------------------------------------------
//  class UnoControlImageControlModel
//  ----------------------------------------------------
UnoControlImageControlModel::UnoControlImageControlModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_IMAGEURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_SCALEIMAGE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
}

uno::Any UnoControlImageControlModel::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType, SAL_STATIC_CAST( awt::XImageProducer*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlModel::queryAggregation( rType ));
}

::rtl::OUString UnoControlImageControlModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlImageControlModel );
}

uno::Any UnoControlImageControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlImageControl );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}


::cppu::IPropertyArrayHelper& UnoControlImageControlModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlImageControlModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

// ::com::sun::star::awt::XImageProducer
void UnoControlImageControlModel::addConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException)
{
    maListeners.push_back( xConsumer );
}

void UnoControlImageControlModel::removeConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException)
{
    maListeners.remove( xConsumer );
}

void UnoControlImageControlModel::startProduction(  ) throw (::com::sun::star::uno::RuntimeException)
{
    uno::Sequence<uno::Any> aArgs(1);
    aArgs.getArray()[0] = getPropertyValue( GetPropertyName( BASEPROPERTY_IMAGEURL ) );
    uno::Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    uno::Reference< awt::XImageProducer > xImageProducer( xMSF->createInstanceWithArguments( ::rtl::OUString::createFromAscii( "com.sun.star.awt.ImageProducer" ), aArgs ), uno::UNO_QUERY );
    if ( xImageProducer.is() )
    {
        std::list< uno::Reference< awt::XImageConsumer > >::iterator aIter( maListeners.begin() );
        while ( aIter != maListeners.end() )
        {
            xImageProducer->addConsumer( *aIter );
            aIter++;
        }
        xImageProducer->startProduction();
    }
}


//  ----------------------------------------------------
//  class UnoImageControlControl
//  ----------------------------------------------------
UnoImageControlControl::UnoImageControlControl()
    : maActionListeners( *this )
{
    // Woher die Defaults nehmen?
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 100;
}

::rtl::OUString UnoImageControlControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "fixedimage" );
}

// uno::XInterface
uno::Any UnoImageControlControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoImageControlControl )
    getCppuType( ( uno::Reference< awt::XLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoImageControlControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

sal_Bool UnoImageControlControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}

awt::Size UnoImageControlControl::getMinimumSize(  ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

awt::Size UnoImageControlControl::getPreferredSize(  ) throw(uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

awt::Size UnoImageControlControl::calcAdjustedSize( const awt::Size& rNewSize ) throw(uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

void UnoImageControlControl::ImplSetPeerProperty( const ::rtl::OUString& rPropName, const uno::Any& rVal )
{
    sal_uInt16 nType = GetPropertyId( rPropName );
    if ( getPeer().is() && ( nType == BASEPROPERTY_IMAGEURL ) )
    {
        uno::Reference < awt::XImageProducer > xImgProd( getModel(), uno::UNO_QUERY );
        uno::Reference < awt::XImageConsumer > xImgCons( getPeer(), uno::UNO_QUERY );

        if ( xImgProd.is() && xImgCons.is() )
        {
            xImgProd->addConsumer( xImgCons );
            xImgProd->startProduction();
            xImgProd->removeConsumer( xImgCons );
        }
    }
    else
        UnoControl::ImplSetPeerProperty( rPropName, rVal );
}

//  ----------------------------------------------------
//  class UnoControlRadioButtonModel
//  ----------------------------------------------------
UnoControlRadioButtonModel::UnoControlRadioButtonModel()
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_STATE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
}

::rtl::OUString UnoControlRadioButtonModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlRadioButtonModel );
}

uno::Any UnoControlRadioButtonModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlRadioButton );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlRadioButtonModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlRadioButtonModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoRadioButtonControl
//  ----------------------------------------------------
UnoRadioButtonControl::UnoRadioButtonControl()
    : maItemListeners( *this ), maActionListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

::rtl::OUString UnoRadioButtonControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "radiobutton" );
}

// uno::XInterface
uno::Any UnoRadioButtonControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XButton*, this ),
                                        SAL_STATIC_CAST( awt::XRadioButton*, this ),
                                        SAL_STATIC_CAST( awt::XItemListener*, this ),
                                        SAL_STATIC_CAST( lang::XEventListener*, SAL_STATIC_CAST( awt::XItemListener*, this ) ),
                                        SAL_STATIC_CAST( awt::XLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoRadioButtonControl )
    getCppuType( ( uno::Reference< awt::XButton>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XRadioButton>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XItemListener>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoRadioButtonControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}


sal_Bool UnoRadioButtonControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}

void UnoRadioButtonControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XRadioButton >  xRadioButton( getPeer(), uno::UNO_QUERY );
    xRadioButton->addItemListener( this );

    uno::Reference < awt::XButton > xButton( getPeer(), uno::UNO_QUERY );
    xButton->setActionCommand( maActionCommand );
    if ( maActionListeners.getLength() )
        xButton->addActionListener( &maActionListeners );

    // as default, set the "AutoToggle" to true
    // (it is set to false in VCLXToolkit::ImplCreateWindow because of #87254#, but we want to
    // have it enabled by default because of 85071)
    uno::Reference< awt::XVclWindowPeer >  xVclWindowPeer( getPeer(), uno::UNO_QUERY );
    if ( xVclWindowPeer.is() )
        xVclWindowPeer->setProperty( GetPropertyName( BASEPROPERTY_AUTOTOGGLE ), ::cppu::bool2any( sal_True ) );
}

void UnoRadioButtonControl::addItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.addInterface( l );
}

void UnoRadioButtonControl::removeItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.removeInterface( l );
}

void UnoRadioButtonControl::addActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    maActionListeners.addInterface( l );
    if( getPeer().is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XButton >  xButton( getPeer(), uno::UNO_QUERY );
        xButton->addActionListener( &maActionListeners );
    }
}

void UnoRadioButtonControl::removeActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    if( getPeer().is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XButton >  xButton( getPeer(), uno::UNO_QUERY );
        xButton->removeActionListener( &maActionListeners );
    }
    maActionListeners.removeInterface( l );
}

void UnoRadioButtonControl::setLabel( const ::rtl::OUString&  rLabel ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= rLabel;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

void UnoRadioButtonControl::setActionCommand( const ::rtl::OUString& rCommand ) throw(uno::RuntimeException)
{
    maActionCommand = rCommand;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XButton >  xButton( getPeer(), uno::UNO_QUERY );
        xButton->setActionCommand( rCommand );
    }
}

void UnoRadioButtonControl::setState( sal_Bool bOn ) throw(uno::RuntimeException)
{
    sal_Int16 nState = bOn ? 1 : 0;
    uno::Any aAny;
    aAny <<= nState;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ), aAny, sal_True );
}

sal_Bool UnoRadioButtonControl::getState() throw(uno::RuntimeException)
{
    sal_Int16 nState = 0;
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ) );
    aVal >>= nState;
    return nState ? sal_True : sal_False;
}

void UnoRadioButtonControl::itemStateChanged( const awt::ItemEvent& rEvent ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= (sal_Int16)rEvent.Selected;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ), aAny, sal_False );

    if ( maItemListeners.getLength() )
        maItemListeners.itemStateChanged( rEvent );
}

awt::Size UnoRadioButtonControl::getMinimumSize(  ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

awt::Size UnoRadioButtonControl::getPreferredSize(  ) throw(uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

awt::Size UnoRadioButtonControl::calcAdjustedSize( const awt::Size& rNewSize ) throw(uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

//  ----------------------------------------------------
//  class UnoControlCheckBoxModel
//  ----------------------------------------------------
UnoControlCheckBoxModel::UnoControlCheckBoxModel()
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_STATE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TRISTATE );
}

::rtl::OUString UnoControlCheckBoxModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlCheckBoxModel );
}

uno::Any UnoControlCheckBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlCheckBox );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}


::cppu::IPropertyArrayHelper& UnoControlCheckBoxModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlCheckBoxModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoCheckBoxControl
//  ----------------------------------------------------
UnoCheckBoxControl::UnoCheckBoxControl()
    : maItemListeners( *this ), maActionListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

::rtl::OUString UnoCheckBoxControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "checkbox" );
}

// uno::XInterface
uno::Any UnoCheckBoxControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XButton*, this ),
                                        SAL_STATIC_CAST( awt::XCheckBox*, this ),
                                        SAL_STATIC_CAST( awt::XItemListener*, this ),
                                        SAL_STATIC_CAST( lang::XEventListener*, SAL_STATIC_CAST( awt::XItemListener*, this ) ),
                                        SAL_STATIC_CAST( awt::XLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoCheckBoxControl )
    getCppuType( ( uno::Reference< awt::XButton>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XCheckBox>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XItemListener>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoCheckBoxControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

sal_Bool UnoCheckBoxControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}

void UnoCheckBoxControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XCheckBox >  xCheckBox( getPeer(), uno::UNO_QUERY );
    xCheckBox->addItemListener( this );

    uno::Reference < awt::XButton > xButton( getPeer(), uno::UNO_QUERY );
    xButton->setActionCommand( maActionCommand );
    if ( maActionListeners.getLength() )
        xButton->addActionListener( &maActionListeners );
}

void UnoCheckBoxControl::addItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.addInterface( l );
}

void UnoCheckBoxControl::removeItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.removeInterface( l );
}

void UnoCheckBoxControl::addActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    maActionListeners.addInterface( l );
    if( getPeer().is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XButton >  xButton( getPeer(), uno::UNO_QUERY );
        xButton->addActionListener( &maActionListeners );
    }
}

void UnoCheckBoxControl::removeActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    if( getPeer().is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XButton >  xButton( getPeer(), uno::UNO_QUERY );
        xButton->removeActionListener( &maActionListeners );
    }
    maActionListeners.removeInterface( l );
}

void UnoCheckBoxControl::setActionCommand( const ::rtl::OUString& rCommand ) throw(uno::RuntimeException)
{
    maActionCommand = rCommand;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XButton > xButton( getPeer(), uno::UNO_QUERY );
        xButton->setActionCommand( rCommand );
    }
}


void UnoCheckBoxControl::setLabel( const ::rtl::OUString&  rLabel ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= rLabel;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

void UnoCheckBoxControl::setState( short n ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= (sal_Int16)n;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ), aAny, sal_True );
}

short UnoCheckBoxControl::getState() throw(uno::RuntimeException)
{
    short nState = 0;
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ) );
    aVal >>= nState;
    return nState;
}

void UnoCheckBoxControl::enableTriState( sal_Bool b ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= b;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TRISTATE ), aAny, sal_True );
}

void UnoCheckBoxControl::itemStateChanged( const awt::ItemEvent& rEvent ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= (sal_Int16)rEvent.Selected;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ), aAny, sal_False );

    if ( maItemListeners.getLength() )
        maItemListeners.itemStateChanged( rEvent );
}

awt::Size UnoCheckBoxControl::getMinimumSize(  ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

awt::Size UnoCheckBoxControl::getPreferredSize(  ) throw(uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

awt::Size UnoCheckBoxControl::calcAdjustedSize( const awt::Size& rNewSize ) throw(uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

//  ----------------------------------------------------
//  class UnoControlFixedTextModel
//  ----------------------------------------------------
UnoControlFixedTextModel::UnoControlFixedTextModel()
{
    ImplRegisterProperty( BASEPROPERTY_ALIGN );
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_MULTILINE );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
}

::rtl::OUString UnoControlFixedTextModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlFixedTextModel );
}

uno::Any UnoControlFixedTextModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlFixedText );
        return aAny;
    }
    else if ( nPropId == BASEPROPERTY_BORDER )
    {
        uno::Any aAny;
        aAny <<= (sal_Int16)0;
        return aAny;
    }

    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlFixedTextModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlFixedTextModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


//  ----------------------------------------------------
//  class UnoFixedTextControl
//  ----------------------------------------------------
UnoFixedTextControl::UnoFixedTextControl()
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

::rtl::OUString UnoFixedTextControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "fixedtext" );
}

// uno::XInterface
uno::Any UnoFixedTextControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XFixedText*, this ),
                                        SAL_STATIC_CAST( awt::XLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoFixedTextControl )
    getCppuType( ( uno::Reference< awt::XFixedText>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

sal_Bool UnoFixedTextControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}

void UnoFixedTextControl::setText( const ::rtl::OUString& Text ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Text;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

::rtl::OUString UnoFixedTextControl::getText() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_UString( BASEPROPERTY_LABEL );
}

void UnoFixedTextControl::setAlignment( short nAlign ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= (sal_Int16)nAlign;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_ALIGN ), aAny, sal_True );
}

short UnoFixedTextControl::getAlignment() throw(uno::RuntimeException)
{
    short nAlign = 0;
    if ( mxModel.is() )
    {
        uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_ALIGN ) );
        aVal >>= nAlign;
    }
    return nAlign;
}

awt::Size UnoFixedTextControl::getMinimumSize(  ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

awt::Size UnoFixedTextControl::getPreferredSize(  ) throw(uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

awt::Size UnoFixedTextControl::calcAdjustedSize( const awt::Size& rNewSize ) throw(uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

//  ----------------------------------------------------
//  class UnoControlGroupBoxModel
//  ----------------------------------------------------
UnoControlGroupBoxModel::UnoControlGroupBoxModel()
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
}

::rtl::OUString UnoControlGroupBoxModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlGroupBoxModel );
}

uno::Any UnoControlGroupBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlGroupBox );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlGroupBoxModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlGroupBoxModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//  ----------------------------------------------------
//  class UnoGroupBoxControl
//  ----------------------------------------------------
UnoGroupBoxControl::UnoGroupBoxControl()
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 100;
}

::rtl::OUString UnoGroupBoxControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "groupbox" );
}

sal_Bool UnoGroupBoxControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}

//  ----------------------------------------------------
//  class UnoControlListBoxModel
//  ----------------------------------------------------
UnoControlListBoxModel::UnoControlListBoxModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_DROPDOWN );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LINECOUNT );
    ImplRegisterProperty( BASEPROPERTY_MULTISELECTION );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_SELECTEDITEMS );
    ImplRegisterProperty( BASEPROPERTY_STRINGITEMLIST );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
}

::rtl::OUString UnoControlListBoxModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlListBoxModel );
}

uno::Any UnoControlListBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlListBox );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlListBoxModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlListBoxModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

void UnoControlListBoxModel::ImplPropertyChanged( sal_uInt16 nPropId )
{
    if ( nPropId == BASEPROPERTY_STRINGITEMLIST )
    {
        uno::Sequence<sal_Int16> aSeq;
        uno::Any aAny;
        aAny <<= aSeq;
        setPropertyValue( GetPropertyName( BASEPROPERTY_SELECTEDITEMS ), aAny );
    }

    UnoControlModel::ImplPropertyChanged( nPropId );
}

void UnoControlListBoxModel::ImplNormalizePropertySequence( const sal_Int32 _nCount, sal_Int32* _pHandles,
    uno::Any* _pValues, sal_Int32* _pValidHandles ) const SAL_THROW(())
{
    // dependencies we know:
    // BASEPROPERTY_STRINGITEMLIST->BASEPROPERTY_SELECTEDITEMS
    // a more generic approach when it is needed would be nice ....

    for ( sal_Int32 i=0; i < _nCount; ++_pHandles, ++_pValues, ++i )
    {
        if ( BASEPROPERTY_SELECTEDITEMS  == *_pHandles )
        {
            // look if the property SelectedItems depends on is _behind_ SelectedItems
            sal_Int32* pLaterHandles = _pHandles + 1;
            uno::Any* pLaterValues = _pValues + 1;
            for ( sal_Int32 j = i + 1; j < _nCount; ++j, ++pLaterHandles, ++pLaterValues )
            {
                if ( BASEPROPERTY_STRINGITEMLIST == *pLaterHandles )
                {
                    // indeed it is -> exchange the both places in the sequences
                    sal_Int32 nHandle( *_pHandles );
                    *_pHandles = *pLaterHandles;
                    *pLaterHandles = nHandle;

                    uno::Any aValue( *_pValues );
                    *_pValues = *pLaterValues;
                    *pLaterValues = aValue;

                    break;
                    // this will leave the inner loop, and continue with the outer loop.
                    // Note that this means we will encounter the SelectedItems handle, again, once we reached
                    // (in the outer loop) the place where we just put it.
                }
            }
        }
    }

    UnoControlModel::ImplNormalizePropertySequence( _nCount, _pHandles, _pValues, _pValidHandles );
}

//  ----------------------------------------------------
//  class UnoListBoxControl
//  ----------------------------------------------------
UnoListBoxControl::UnoListBoxControl()
    :   maActionListeners( *this ),
        maItemListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

::rtl::OUString UnoListBoxControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "listbox" );
}

// uno::XInterface
uno::Any UnoListBoxControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XListBox*, this ),
                                        SAL_STATIC_CAST( awt::XItemListener*, this ),
                                        SAL_STATIC_CAST( lang::XEventListener*, SAL_STATIC_CAST( awt::XItemListener*, this ) ),
                                        SAL_STATIC_CAST( awt::XLayoutConstrains*, this ),
                                        SAL_STATIC_CAST( awt::XTextLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoListBoxControl )
    getCppuType( ( uno::Reference< awt::XListBox>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XItemListener>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XLayoutConstrains>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XTextLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoListBoxControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aEvt );
    maItemListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoListBoxControl::ImplUpdateSelectedItemsProperty()
{
    if ( getPeer().is() )
    {
        uno::Reference < awt::XListBox > xListBox( getPeer(), uno::UNO_QUERY );
        DBG_ASSERT( xListBox.is(), "XListBox?" );

        uno::Sequence<sal_Int16> aSeq = xListBox->getSelectedItemsPos();
        uno::Any aAny;
        aAny <<= aSeq;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SELECTEDITEMS ), aAny, sal_False );
    }
}

void UnoListBoxControl::ImplSetPeerProperty( const ::rtl::OUString& rPropName, const uno::Any& rVal )
{
    UnoControl::ImplSetPeerProperty( rPropName, rVal );

    // Wenn die SelectedItems vor der StringItemList gesetzt werden,
    // hat das keine Auswirkung...
    if ( rPropName == GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) )
    {
        ::rtl::OUString aSelPropName = GetPropertyName( BASEPROPERTY_SELECTEDITEMS );
        uno::Any aVal = ImplGetPropertyValue( aSelPropName );
        if ( !( aVal.getValueType().getTypeClass() == uno::TypeClass_VOID ) )
        {
            uno::Reference< awt::XVclWindowPeer > xW( getPeer(), uno::UNO_QUERY );
            if (xW.is())
                // same comment as in UnoControl::ImplSetPeerProperty - see there
                xW->setProperty( aSelPropName, aVal );
        }
    }
}

void UnoListBoxControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XListBox >  xListBox( getPeer(), uno::UNO_QUERY );
    xListBox->addItemListener( this );

    if ( maActionListeners.getLength() )
        xListBox->addActionListener( &maActionListeners );
}

void UnoListBoxControl::addActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    maActionListeners.addInterface( l );
    if( getPeer().is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XListBox >  xListBox( getPeer(), uno::UNO_QUERY );
        xListBox->addActionListener( &maActionListeners );
    }
}

void UnoListBoxControl::removeActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    if( getPeer().is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XListBox >  xListBox( getPeer(), uno::UNO_QUERY );
        xListBox->removeActionListener( &maActionListeners );
    }
    maActionListeners.removeInterface( l );
}

void UnoListBoxControl::addItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.addInterface( l );
}

void UnoListBoxControl::removeItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.removeInterface( l );
}

void UnoListBoxControl::addItem( const ::rtl::OUString& aItem, sal_Int16 nPos ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString> aSeq( 1 );
    aSeq.getArray()[0] = aItem;
    addItems( aSeq, nPos );
}

void UnoListBoxControl::addItems( const uno::Sequence< ::rtl::OUString>& aItems, sal_Int16 nPos ) throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nNewItems = (sal_uInt16)aItems.getLength();
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    sal_uInt16 nNewLen = nOldLen + nNewItems;

    uno::Sequence< ::rtl::OUString> aNewSeq( nNewLen );
    ::rtl::OUString* pNewData = aNewSeq.getArray();
    ::rtl::OUString* pOldData = aSeq.getArray();

    if ( nPos > nOldLen )
        nPos = (sal_uInt16) nOldLen;

    sal_uInt16 n;
    // Items vor der Einfuege-Position
    for ( n = 0; n < nPos; n++ )
        pNewData[n] = pOldData[n];

    // Neue Items
    for ( n = 0; n < nNewItems; n++ )
        pNewData[nPos+n] = aItems.getConstArray()[n];

    // Rest der alten Items
    for ( n = nPos; n < nOldLen; n++ )
        pNewData[nNewItems+n] = pOldData[n];

    uno::Any aAny;
    aAny <<= aNewSeq;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ), aAny, sal_True );
}

void UnoListBoxControl::removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    if ( nOldLen && ( nPos < nOldLen ) )
    {
        if ( nCount > ( nOldLen-nPos ) )
            nCount = nOldLen-nPos;

        sal_uInt16 nNewLen = nOldLen - nCount;

        uno::Sequence< ::rtl::OUString> aNewSeq( nNewLen );
        ::rtl::OUString* pNewData = aNewSeq.getArray();
        ::rtl::OUString* pOldData = aSeq.getArray();

        sal_uInt16 n;
        // Items vor der Entfern-Position
        for ( n = 0; n < nPos; n++ )
            pNewData[n] = pOldData[n];

        // Rest der Items
        for ( n = nPos; n < (nOldLen-nCount); n++ )
            pNewData[n] = pOldData[n+nCount];

        uno::Any aAny;
        aAny <<= aNewSeq;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ), aAny, sal_True );
    }
}

sal_Int16 UnoListBoxControl::getItemCount() throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    return (sal_Int16)aSeq.getLength();
}

::rtl::OUString UnoListBoxControl::getItem( sal_Int16 nPos ) throw(uno::RuntimeException)
{
    ::rtl::OUString aItem;
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    if ( nPos < aSeq.getLength() )
        aItem = aSeq.getConstArray()[nPos];
    return aItem;
}

uno::Sequence< ::rtl::OUString> UnoListBoxControl::getItems() throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    return aSeq;
}

sal_Int16 UnoListBoxControl::getSelectedItemPos() throw(uno::RuntimeException)
{
    sal_Int16 n = -1;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XListBox >  xListBox( getPeer(), uno::UNO_QUERY );
        n = xListBox->getSelectedItemPos();
    }
    return n;
}

uno::Sequence<sal_Int16> UnoListBoxControl::getSelectedItemsPos() throw(uno::RuntimeException)
{
    uno::Sequence<sal_Int16> aSeq;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XListBox >  xListBox( getPeer(), uno::UNO_QUERY );
        aSeq = xListBox->getSelectedItemsPos();
    }
    return aSeq;
}

::rtl::OUString UnoListBoxControl::getSelectedItem() throw(uno::RuntimeException)
{
    ::rtl::OUString aItem;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XListBox >  xListBox( getPeer(), uno::UNO_QUERY );
        aItem = xListBox->getSelectedItem();
    }
    return aItem;
}

uno::Sequence< ::rtl::OUString> UnoListBoxControl::getSelectedItems() throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString> aSeq;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XListBox >  xListBox( getPeer(), uno::UNO_QUERY );
        aSeq = xListBox->getSelectedItems();
    }
    return aSeq;
}

void UnoListBoxControl::selectItemPos( sal_Int16 nPos, sal_Bool bSelect ) throw(uno::RuntimeException)
{
    if ( getPeer().is() )
    {
        uno::Reference < awt::XListBox >  xListBox( getPeer(), uno::UNO_QUERY );
        xListBox->selectItemPos( nPos, bSelect );
    }
    ImplUpdateSelectedItemsProperty();
}

void UnoListBoxControl::selectItemsPos( const uno::Sequence<sal_Int16>& aPositions, sal_Bool bSelect ) throw(uno::RuntimeException)
{
    if ( getPeer().is() )
    {
        uno::Reference < awt::XListBox >  xListBox( getPeer(), uno::UNO_QUERY );
        xListBox->selectItemsPos( aPositions, bSelect );
    }
    ImplUpdateSelectedItemsProperty();
}

void UnoListBoxControl::selectItem( const ::rtl::OUString& aItem, sal_Bool bSelect ) throw(uno::RuntimeException)
{
    if ( getPeer().is() )
    {
        uno::Reference < awt::XListBox >  xListBox( getPeer(), uno::UNO_QUERY );
        xListBox->selectItem( aItem, bSelect );
    }
    ImplUpdateSelectedItemsProperty();
}

void UnoListBoxControl::makeVisible( sal_Int16 nEntry ) throw(uno::RuntimeException)
{
    if ( getPeer().is() )
    {
        uno::Reference < awt::XListBox >  xListBox( getPeer(), uno::UNO_QUERY );
        xListBox->makeVisible( nEntry );
    }
}

void UnoListBoxControl::setDropDownLineCount( sal_Int16 nLines ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= (sal_Int16)nLines;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LINECOUNT ), aAny, sal_True );
}

sal_Int16 UnoListBoxControl::getDropDownLineCount() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT16( BASEPROPERTY_LINECOUNT );
}

sal_Bool UnoListBoxControl::isMutipleMode() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_MULTISELECTION );
}

void UnoListBoxControl::setMultipleMode( sal_Bool bMulti ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= bMulti;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_MULTISELECTION ), aAny, sal_True );
}

void UnoListBoxControl::itemStateChanged( const awt::ItemEvent& rEvent ) throw(uno::RuntimeException)
{
    ImplUpdateSelectedItemsProperty();
    if ( maItemListeners.getLength() )
        maItemListeners.itemStateChanged( rEvent );
}

awt::Size UnoListBoxControl::getMinimumSize(  ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

awt::Size UnoListBoxControl::getPreferredSize(  ) throw(uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

awt::Size UnoListBoxControl::calcAdjustedSize( const awt::Size& rNewSize ) throw(uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

awt::Size UnoListBoxControl::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize( nCols, nLines );
}

void UnoListBoxControl::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(uno::RuntimeException)
{
    Impl_getColumnsAndLines( nCols, nLines );
}

//  ----------------------------------------------------
//  class UnoControlComboBoxModel
//  ----------------------------------------------------
UnoControlComboBoxModel::UnoControlComboBoxModel()
{
    ImplRegisterProperty( BASEPROPERTY_AUTOCOMPLETE );
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_DROPDOWN );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LINECOUNT );
    ImplRegisterProperty( BASEPROPERTY_MAXTEXTLEN );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_STRINGITEMLIST );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
}

::rtl::OUString UnoControlComboBoxModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlComboBoxModel );
}

uno::Any UnoControlComboBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlComboBox );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}


::cppu::IPropertyArrayHelper& UnoControlComboBoxModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlComboBoxModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoComboBoxControl
//  ----------------------------------------------------
UnoComboBoxControl::UnoComboBoxControl()
    :   maActionListeners( *this ),
        maItemListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

::rtl::OUString UnoComboBoxControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "combobox" );
}

// uno::XInterface
uno::Any UnoComboBoxControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XComboBox*, this ) );
    return (aRet.hasValue() ? aRet : UnoEditControl::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoComboBoxControl )
    getCppuType( ( uno::Reference< awt::XComboBox>* ) NULL ),
    UnoEditControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoComboBoxControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aEvt );
    maItemListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoComboBoxControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoEditControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XComboBox >  xComboBox( getPeer(), uno::UNO_QUERY );
    if ( maActionListeners.getLength() )
        xComboBox->addActionListener( &maActionListeners );
    if ( maItemListeners.getLength() )
        xComboBox->addItemListener( &maItemListeners );
}

void UnoComboBoxControl::addActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    maActionListeners.addInterface( l );
    if( getPeer().is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XComboBox >  xComboBox( getPeer(), uno::UNO_QUERY );
        xComboBox->addActionListener( &maActionListeners );
    }
}

void UnoComboBoxControl::removeActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    if( getPeer().is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XComboBox >  xComboBox( getPeer(), uno::UNO_QUERY );
        xComboBox->removeActionListener( &maActionListeners );
    }
    maActionListeners.removeInterface( l );
}

void UnoComboBoxControl::addItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.addInterface( l );
    if( getPeer().is() && maItemListeners.getLength() == 1 )
    {
        uno::Reference < awt::XComboBox >  xComboBox( getPeer(), uno::UNO_QUERY );
        xComboBox->addItemListener( &maItemListeners );
    }
}

void UnoComboBoxControl::removeItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    if( getPeer().is() && maItemListeners.getLength() == 1 )
    {
        uno::Reference < awt::XComboBox >  xComboBox( getPeer(), uno::UNO_QUERY );  // MT: Mal alles so umstellen, schoener als Ref anlegen und query rufen
        xComboBox->removeItemListener( &maItemListeners );
    }
    maItemListeners.removeInterface( l );
}

void UnoComboBoxControl::addItem( const ::rtl::OUString& aItem, sal_Int16 nPos ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString> aSeq( 1 );
    aSeq.getArray()[0] = aItem;
    addItems( aSeq, nPos );
}

void UnoComboBoxControl::addItems( const uno::Sequence< ::rtl::OUString>& aItems, sal_Int16 nPos ) throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nNewItems = (sal_uInt16)aItems.getLength();
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    sal_uInt16 nNewLen = nOldLen + nNewItems;

    uno::Sequence< ::rtl::OUString> aNewSeq( nNewLen );
    ::rtl::OUString* pNewData = aNewSeq.getArray();
    const ::rtl::OUString* pOldData = aSeq.getConstArray();

    if ( nPos > nOldLen )
        nPos = (sal_uInt16) nOldLen;

    sal_uInt16 n;
    // Items vor der Einfuege-Position
    for ( n = 0; n < nPos; n++ )
        pNewData[n] = pOldData[n];

    // Neue Items
    for ( n = 0; n < nNewItems; n++ )
        pNewData[nPos+n] = aItems.getConstArray()[n];

    // Rest der alten Items
    for ( n = nPos; n < nOldLen; n++ )
        pNewData[nNewItems+n] = pOldData[n];

    uno::Any aAny;
    aAny <<= aNewSeq;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ), aAny, sal_True );
}

void UnoComboBoxControl::removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    if ( nOldLen && ( nPos < nOldLen ) )
    {
        if ( nCount > ( nOldLen-nPos ) )
            nCount = nOldLen-nPos;

        sal_uInt16 nNewLen = nOldLen - nCount;

        uno::Sequence< ::rtl::OUString> aNewSeq( nNewLen );
        ::rtl::OUString* pNewData = aNewSeq.getArray();
        ::rtl::OUString* pOldData = aSeq.getArray();

        sal_uInt16 n;
        // Items vor der Entfern-Position
        for ( n = 0; n < nPos; n++ )
            pNewData[n] = pOldData[n];

        // Rest der Items
        for ( n = nPos; n < (nOldLen-nCount); n++ )
            pNewData[n] = pOldData[n+nCount];

        uno::Any aAny;
        aAny <<= aNewSeq;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ), aAny, sal_True );
    }
}

sal_Int16 UnoComboBoxControl::getItemCount() throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    return (sal_Int16)aSeq.getLength();
}

::rtl::OUString UnoComboBoxControl::getItem( sal_Int16 nPos ) throw(uno::RuntimeException)
{
    ::rtl::OUString aItem;
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    if ( nPos < aSeq.getLength() )
        aItem = aSeq.getConstArray()[nPos];
    return aItem;
}

uno::Sequence< ::rtl::OUString> UnoComboBoxControl::getItems() throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    return aSeq;
}

void UnoComboBoxControl::setDropDownLineCount( sal_Int16 nLines ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= nLines;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LINECOUNT ), aAny, sal_True );
}

sal_Int16 UnoComboBoxControl::getDropDownLineCount() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT16( BASEPROPERTY_LINECOUNT );
}


//  ----------------------------------------------------
//  UnoSpinFieldControl
//  ----------------------------------------------------
UnoSpinFieldControl::UnoSpinFieldControl() : maSpinListeners( *this )
{
    mbRepeat = sal_False;
}

// uno::XInterface
uno::Any UnoSpinFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XSpinField*, this ) );
    return (aRet.hasValue() ? aRet : UnoEditControl::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoSpinFieldControl )
    getCppuType( ( uno::Reference< awt::XSpinField>* ) NULL ),
    UnoEditControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoSpinFieldControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoEditControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XSpinField > xField( getPeer(), uno::UNO_QUERY );
    xField->enableRepeat( mbRepeat );
    if ( maSpinListeners.getLength() )
        xField->addSpinListener( &maSpinListeners );
}

    // ::com::sun::star::awt::XSpinField
void UnoSpinFieldControl::addSpinListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinListener >& l ) throw(::com::sun::star::uno::RuntimeException)
{
    maSpinListeners.addInterface( l );
    if( getPeer().is() && maSpinListeners.getLength() == 1 )
    {
        uno::Reference < awt::XSpinField > xField( getPeer(), uno::UNO_QUERY );
        xField->addSpinListener( &maSpinListeners );
    }
}

void UnoSpinFieldControl::removeSpinListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XSpinListener >& l ) throw(::com::sun::star::uno::RuntimeException)
{
    if( getPeer().is() && maSpinListeners.getLength() == 1 )
    {
        uno::Reference < awt::XSpinField > xField( getPeer(), uno::UNO_QUERY );
        xField->removeSpinListener( &maSpinListeners );
    }
    maSpinListeners.removeInterface( l );
}

void UnoSpinFieldControl::up() throw(::com::sun::star::uno::RuntimeException)
{
    uno::Reference < awt::XSpinField > xField( getPeer(), uno::UNO_QUERY );
    if ( xField.is() )
        xField->up();
}

void UnoSpinFieldControl::down() throw(::com::sun::star::uno::RuntimeException)
{
    uno::Reference < awt::XSpinField > xField( getPeer(), uno::UNO_QUERY );
    if ( xField.is() )
        xField->down();
}

void UnoSpinFieldControl::first() throw(::com::sun::star::uno::RuntimeException)
{
    uno::Reference < awt::XSpinField > xField( getPeer(), uno::UNO_QUERY );
    if ( xField.is() )
        xField->first();
}

void UnoSpinFieldControl::last() throw(::com::sun::star::uno::RuntimeException)
{
    uno::Reference < awt::XSpinField > xField( getPeer(), uno::UNO_QUERY );
    if ( xField.is() )
        xField->last();
}

void UnoSpinFieldControl::enableRepeat( sal_Bool bRepeat ) throw(::com::sun::star::uno::RuntimeException)
{
    mbRepeat = bRepeat;

    uno::Reference < awt::XSpinField > xField( getPeer(), uno::UNO_QUERY );
    if ( xField.is() )
        xField->enableRepeat( bRepeat );
}

//  ----------------------------------------------------
//  class UnoControlDateFieldModel
//  ----------------------------------------------------
UnoControlDateFieldModel::UnoControlDateFieldModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DATE );
    ImplRegisterProperty( BASEPROPERTY_DATEMAX );
    ImplRegisterProperty( BASEPROPERTY_DATEMIN );
    ImplRegisterProperty( BASEPROPERTY_DATESHOWCENTURY );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_DROPDOWN );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_EXTDATEFORMAT );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_SPIN );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
}

::rtl::OUString UnoControlDateFieldModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlDateFieldModel );
}

uno::Any UnoControlDateFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlDateField );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}


::cppu::IPropertyArrayHelper& UnoControlDateFieldModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlDateFieldModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoDateFieldControl
//  ----------------------------------------------------
UnoDateFieldControl::UnoDateFieldControl()
{
    mnFirst = Date( 1, 1, 1900 ).GetDate();
    mnLast = Date( 31, 12, 2200 ).GetDate();
    mbLongFormat = 2;
}

::rtl::OUString UnoDateFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "datefield" );
}

// uno::XInterface
uno::Any UnoDateFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XDateField*, this ) );
    return (aRet.hasValue() ? aRet : UnoSpinFieldControl::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoDateFieldControl )
    getCppuType( ( uno::Reference< awt::XDateField>* ) NULL ),
    UnoSpinFieldControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoDateFieldControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoSpinFieldControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XDateField > xField( getPeer(), uno::UNO_QUERY );
    xField->setFirst( mnFirst );
    xField->setLast( mnLast );
    if ( mbLongFormat != 2 )    // not set
        xField->setLongFormat( mbLongFormat );
}


void UnoDateFieldControl::textChanged( const awt::TextEvent& e ) throw(uno::RuntimeException)
{
    uno::Reference < awt::XDateField > xField( getPeer(), uno::UNO_QUERY );
    uno::Any aValue;
    if ( !xField->isEmpty() )
        aValue <<= xField->getDate();

    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_DATE ), aValue, sal_False );

    if ( GetTextListeners().getLength() )
        GetTextListeners().textChanged( e );
}

void UnoDateFieldControl::setDate( sal_Int32 Date ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Date;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_DATE ), aAny, sal_True );
}

sal_Int32 UnoDateFieldControl::getDate() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_DATE );
}

void UnoDateFieldControl::setMin( sal_Int32 Date ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Date;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_DATEMIN ), aAny, sal_True );
}

sal_Int32 UnoDateFieldControl::getMin() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_DATEMIN );
}

void UnoDateFieldControl::setMax( sal_Int32 Date ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Date;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_DATEMAX ), aAny, sal_True );
}

sal_Int32 UnoDateFieldControl::getMax() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_DATEMAX );
}

void UnoDateFieldControl::setFirst( sal_Int32 Date ) throw(uno::RuntimeException)
{
    mnFirst = Date;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XDateField > xField( getPeer(), uno::UNO_QUERY );
        xField->setFirst( Date );
    }
}

sal_Int32 UnoDateFieldControl::getFirst() throw(uno::RuntimeException)
{
    return mnFirst;
}

void UnoDateFieldControl::setLast( sal_Int32 Date ) throw(uno::RuntimeException)
{
    mnLast = Date;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XDateField > xField( getPeer(), uno::UNO_QUERY );
        xField->setLast( Date );
    }
}

sal_Int32 UnoDateFieldControl::getLast() throw(uno::RuntimeException)
{
    return mnLast;
}

void UnoDateFieldControl::setLongFormat( sal_Bool bLong ) throw(uno::RuntimeException)
{
    mbLongFormat = bLong;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XDateField > xField( getPeer(), uno::UNO_QUERY );
        xField->setLongFormat( bLong );
    }
}

sal_Bool UnoDateFieldControl::isLongFormat() throw(uno::RuntimeException)
{
    return ( mbLongFormat != 2 ) ? mbLongFormat : sal_False;
}

void UnoDateFieldControl::setEmpty() throw(uno::RuntimeException)
{
    if ( getPeer().is() )
    {
        uno::Reference < awt::XDateField >  xField( getPeer(), uno::UNO_QUERY );
        xField->setEmpty();
    }
}

sal_Bool UnoDateFieldControl::isEmpty() throw(uno::RuntimeException)
{
    sal_Bool bEmpty = sal_False;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XDateField > xField( getPeer(), uno::UNO_QUERY );
        bEmpty = xField->isEmpty();
    }
    return bEmpty;
}

void UnoDateFieldControl::setStrictFormat( sal_Bool bStrict ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= bStrict;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRICTFORMAT ), aAny, sal_True );
}

sal_Bool UnoDateFieldControl::isStrictFormat() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_STRICTFORMAT );
}

//  ----------------------------------------------------
//  class UnoControlTimeFieldModel
//  ----------------------------------------------------
UnoControlTimeFieldModel::UnoControlTimeFieldModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED);
    ImplRegisterProperty( BASEPROPERTY_EXTTIMEFORMAT );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_SPIN );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TIME );
    ImplRegisterProperty( BASEPROPERTY_TIMEMAX );
    ImplRegisterProperty( BASEPROPERTY_TIMEMIN );
}

::rtl::OUString UnoControlTimeFieldModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlTimeFieldModel );
}

uno::Any UnoControlTimeFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlTimeField );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}


::cppu::IPropertyArrayHelper& UnoControlTimeFieldModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlTimeFieldModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoTimeFieldControl
//  ----------------------------------------------------
UnoTimeFieldControl::UnoTimeFieldControl()
{
    mnFirst = Time( 0, 0 ).GetTime();
    mnLast = Time( 23, 59, 59, 99 ).GetTime();
}

::rtl::OUString UnoTimeFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "timefield" );
}

// uno::XInterface
uno::Any UnoTimeFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XTimeField*, this ) );
    return (aRet.hasValue() ? aRet : UnoSpinFieldControl::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoTimeFieldControl )
    getCppuType( ( uno::Reference< awt::XTimeField>* ) NULL ),
    UnoSpinFieldControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoTimeFieldControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoSpinFieldControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XTimeField > xField( getPeer(), uno::UNO_QUERY );
    xField->setFirst( mnFirst );
    xField->setLast( mnLast );
}

void UnoTimeFieldControl::textChanged( const awt::TextEvent& e ) throw(uno::RuntimeException)
{
    uno::Reference < awt::XTimeField >  xField( getPeer(), uno::UNO_QUERY );
    uno::Any aValue;
    if ( !xField->isEmpty() )
        aValue <<= xField->getTime();
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TIME ), aValue, sal_False );

    if ( GetTextListeners().getLength() )
        GetTextListeners().textChanged( e );
}

void UnoTimeFieldControl::setTime( sal_Int32 Time ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Time;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TIME ), aAny, sal_True );
}

sal_Int32 UnoTimeFieldControl::getTime() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_TIME );
}

void UnoTimeFieldControl::setMin( sal_Int32 Time ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Time;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TIMEMIN ), aAny, sal_True );
}

sal_Int32 UnoTimeFieldControl::getMin() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_TIMEMIN );
}

void UnoTimeFieldControl::setMax( sal_Int32 Time ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Time;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TIMEMAX ), aAny, sal_True );
}

sal_Int32 UnoTimeFieldControl::getMax() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_TIMEMAX );
}

void UnoTimeFieldControl::setFirst( sal_Int32 Time ) throw(uno::RuntimeException)
{
    mnFirst = Time;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XTimeField > xField( getPeer(), uno::UNO_QUERY );
        xField->setFirst( mnFirst );
    }
}

sal_Int32 UnoTimeFieldControl::getFirst() throw(uno::RuntimeException)
{
    return mnFirst;
}

void UnoTimeFieldControl::setLast( sal_Int32 Time ) throw(uno::RuntimeException)
{
    mnLast = Time;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XTimeField > xField( getPeer(), uno::UNO_QUERY );
        xField->setFirst( mnLast );
    }
}

sal_Int32 UnoTimeFieldControl::getLast() throw(uno::RuntimeException)
{
    return mnLast;
}

void UnoTimeFieldControl::setEmpty() throw(uno::RuntimeException)
{
    if ( getPeer().is() )
    {
        uno::Reference < awt::XTimeField >  xField( getPeer(), uno::UNO_QUERY );
        xField->setEmpty();
    }
}

sal_Bool UnoTimeFieldControl::isEmpty() throw(uno::RuntimeException)
{
    sal_Bool bEmpty = sal_False;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XTimeField >  xField( getPeer(), uno::UNO_QUERY );
        bEmpty = xField->isEmpty();
    }
    return bEmpty;
}

void UnoTimeFieldControl::setStrictFormat( sal_Bool bStrict ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= bStrict;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRICTFORMAT ), aAny, sal_True );
}

sal_Bool UnoTimeFieldControl::isStrictFormat() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_STRICTFORMAT );
}

//  ----------------------------------------------------
//  class UnoControlNumericFieldModel
//  ----------------------------------------------------
UnoControlNumericFieldModel::UnoControlNumericFieldModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DECIMALACCURACY );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_NUMSHOWTHOUSANDSEP );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_SPIN );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_VALUEMAX_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUEMIN_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUESTEP_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUE_DOUBLE );
}

::rtl::OUString UnoControlNumericFieldModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlNumericFieldModel );
}

uno::Any UnoControlNumericFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlNumericField );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}


::cppu::IPropertyArrayHelper& UnoControlNumericFieldModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlNumericFieldModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoNumericFieldControl
//  ----------------------------------------------------
UnoNumericFieldControl::UnoNumericFieldControl()
{
    mnFirst = 0;
    mnLast = 0x7FFFFFFF;
}

::rtl::OUString UnoNumericFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "numericfield" );
}

// uno::XInterface
uno::Any UnoNumericFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XNumericField*, this ) );
    return (aRet.hasValue() ? aRet : UnoSpinFieldControl::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoNumericFieldControl )
    getCppuType( ( uno::Reference< awt::XNumericField>* ) NULL ),
    UnoSpinFieldControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoNumericFieldControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoSpinFieldControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XNumericField > xField( getPeer(), uno::UNO_QUERY );
    xField->setFirst( mnFirst );
    xField->setLast( mnLast );
}


void UnoNumericFieldControl::textChanged( const awt::TextEvent& e ) throw(uno::RuntimeException)
{
    uno::Reference < awt::XNumericField >  xField( getPeer(), uno::UNO_QUERY );
    uno::Any aAny;
    aAny <<= xField->getValue();
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUE_DOUBLE ), aAny, sal_False );

    if ( GetTextListeners().getLength() )
        GetTextListeners().textChanged( e );
}

void UnoNumericFieldControl::setValue( double Value ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUE_DOUBLE ), aAny, sal_True );
}

double UnoNumericFieldControl::getValue() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUE_DOUBLE );
}

void UnoNumericFieldControl::setMin( double Value ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUEMIN_DOUBLE ), aAny, sal_True );
}

double UnoNumericFieldControl::getMin() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUEMIN_DOUBLE );
}

void UnoNumericFieldControl::setMax( double Value ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUEMAX_DOUBLE ), aAny, sal_True );
}

double UnoNumericFieldControl::getMax() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUEMAX_DOUBLE );
}

void UnoNumericFieldControl::setFirst( double Value ) throw(uno::RuntimeException)
{
    mnFirst = Value;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XNumericField > xField( getPeer(), uno::UNO_QUERY );
        xField->setFirst( mnFirst );
    }
}

double UnoNumericFieldControl::getFirst() throw(uno::RuntimeException)
{
    return mnFirst;
}

void UnoNumericFieldControl::setLast( double Value ) throw(uno::RuntimeException)
{
    mnLast = Value;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XNumericField > xField( getPeer(), uno::UNO_QUERY );
        xField->setLast( mnLast );
    }
}

double UnoNumericFieldControl::getLast() throw(uno::RuntimeException)
{
    return mnLast;
}

void UnoNumericFieldControl::setStrictFormat( sal_Bool bStrict ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= bStrict;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRICTFORMAT ), aAny, sal_True );
}

sal_Bool UnoNumericFieldControl::isStrictFormat() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_STRICTFORMAT );
}

void UnoNumericFieldControl::setSpinSize( double Digits ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Digits;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUESTEP_DOUBLE ), aAny, sal_True );
}

double UnoNumericFieldControl::getSpinSize() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUESTEP_DOUBLE );
}

void UnoNumericFieldControl::setDecimalDigits( sal_Int16 Digits ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Digits;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_DECIMALACCURACY ), aAny, sal_True );
}

sal_Int16 UnoNumericFieldControl::getDecimalDigits() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT16( BASEPROPERTY_DECIMALACCURACY );
}

//  ----------------------------------------------------
//  class UnoControlCurrencyFieldModel
//  ----------------------------------------------------
UnoControlCurrencyFieldModel::UnoControlCurrencyFieldModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_CURRENCYSYMBOL );
    ImplRegisterProperty( BASEPROPERTY_CURSYM_POSITION );
    ImplRegisterProperty( BASEPROPERTY_DECIMALACCURACY );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_NUMSHOWTHOUSANDSEP );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_SPIN );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_VALUEMAX_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUEMIN_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUESTEP_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUE_DOUBLE );
}

::rtl::OUString UnoControlCurrencyFieldModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlCurrencyFieldModel );
}

uno::Any UnoControlCurrencyFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlCurrencyField );
        return aAny;
    }
    if ( nPropId == BASEPROPERTY_CURSYM_POSITION )
    {
        uno::Any aAny;
        aAny <<= (sal_Bool)sal_False;
        return aAny;
    }

    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlCurrencyFieldModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlCurrencyFieldModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//  ----------------------------------------------------
//  class UnoCurrencyFieldControl
//  ----------------------------------------------------
UnoCurrencyFieldControl::UnoCurrencyFieldControl()
{
    mnFirst = 0;
    mnLast = 0x7FFFFFFF;
}

::rtl::OUString UnoCurrencyFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "longcurrencyfield" );
}

// uno::XInterface
uno::Any UnoCurrencyFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XCurrencyField*, this ) );
    return (aRet.hasValue() ? aRet : UnoSpinFieldControl::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoCurrencyFieldControl )
    getCppuType( ( uno::Reference< awt::XCurrencyField>* ) NULL ),
    UnoSpinFieldControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoCurrencyFieldControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoSpinFieldControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XCurrencyField > xField( getPeer(), uno::UNO_QUERY );
    xField->setFirst( mnFirst );
    xField->setLast( mnLast );
}

void UnoCurrencyFieldControl::textChanged( const awt::TextEvent& e ) throw(uno::RuntimeException)
{
    uno::Reference < awt::XCurrencyField >  xField( getPeer(), uno::UNO_QUERY );
    uno::Any aAny;
    aAny <<= xField->getValue();
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUE_DOUBLE ), aAny, sal_False );

    if ( GetTextListeners().getLength() )
        GetTextListeners().textChanged( e );
}

void UnoCurrencyFieldControl::setValue( double Value ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUE_DOUBLE ), aAny, sal_True );
}

double UnoCurrencyFieldControl::getValue() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUE_DOUBLE );
}

void UnoCurrencyFieldControl::setMin( double Value ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUEMIN_DOUBLE ), aAny, sal_True );
}

double UnoCurrencyFieldControl::getMin() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUEMIN_DOUBLE );
}

void UnoCurrencyFieldControl::setMax( double Value ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUEMAX_DOUBLE ), aAny, sal_True );
}

double UnoCurrencyFieldControl::getMax() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUEMAX_DOUBLE );
}

void UnoCurrencyFieldControl::setFirst( double Value ) throw(uno::RuntimeException)
{
    mnFirst = Value;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XCurrencyField > xField( getPeer(), uno::UNO_QUERY );
        xField->setFirst( mnFirst );
    }
}

double UnoCurrencyFieldControl::getFirst() throw(uno::RuntimeException)
{
    return mnFirst;
}

void UnoCurrencyFieldControl::setLast( double Value ) throw(uno::RuntimeException)
{
    mnLast = Value;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XCurrencyField > xField( getPeer(), uno::UNO_QUERY );
        xField->setLast( mnLast );
    }
}

double UnoCurrencyFieldControl::getLast() throw(uno::RuntimeException)
{
    return mnLast;
}

void UnoCurrencyFieldControl::setStrictFormat( sal_Bool bStrict ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= bStrict;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRICTFORMAT ), aAny, sal_True );
}

sal_Bool UnoCurrencyFieldControl::isStrictFormat() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_STRICTFORMAT );
}

void UnoCurrencyFieldControl::setSpinSize( double Digits ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Digits;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUESTEP_DOUBLE ), aAny, sal_True );
}

double UnoCurrencyFieldControl::getSpinSize() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUESTEP_DOUBLE );
}

void UnoCurrencyFieldControl::setDecimalDigits( sal_Int16 Digits ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Digits;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_DECIMALACCURACY ), aAny, sal_True );
}

sal_Int16 UnoCurrencyFieldControl::getDecimalDigits() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT16( BASEPROPERTY_DECIMALACCURACY );
}

//  ----------------------------------------------------
//  class UnoControlPatternFieldModel
//  ----------------------------------------------------
UnoControlPatternFieldModel::UnoControlPatternFieldModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_EDITMASK );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LITERALMASK );
    ImplRegisterProperty( BASEPROPERTY_MAXTEXTLEN );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
}

::rtl::OUString UnoControlPatternFieldModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlPatternFieldModel );
}

uno::Any UnoControlPatternFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlPatternField );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlPatternFieldModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlPatternFieldModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


//  ----------------------------------------------------
//  class UnoPatternFieldControl
//  ----------------------------------------------------
UnoPatternFieldControl::UnoPatternFieldControl()
{
}

::rtl::OUString UnoPatternFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "patternfield" );
}

void UnoPatternFieldControl::ImplSetPeerProperty( const ::rtl::OUString& rPropName, const uno::Any& rVal )
{
    sal_uInt16 nType = GetPropertyId( rPropName );
    if ( ( nType == BASEPROPERTY_TEXT ) || ( nType == BASEPROPERTY_EDITMASK ) || ( nType == BASEPROPERTY_LITERALMASK ) )
    {
        // Die Masken koennen nicht nacheinander gesetzt werden.
        ::rtl::OUString Text = ImplGetPropertyValue_UString( BASEPROPERTY_TEXT );
        ::rtl::OUString EditMask = ImplGetPropertyValue_UString( BASEPROPERTY_EDITMASK );
        ::rtl::OUString LiteralMask = ImplGetPropertyValue_UString( BASEPROPERTY_LITERALMASK );

        uno::Reference < awt::XPatternField >  xPF( getPeer(), uno::UNO_QUERY );
        if (xPF.is())
        {   // same comment as in UnoControl::ImplSetPeerProperty - see there
            xPF->setString( Text );
            xPF->setMasks( EditMask, LiteralMask );
        }
    }
    else
        UnoSpinFieldControl::ImplSetPeerProperty( rPropName, rVal );
}


// uno::XInterface
uno::Any UnoPatternFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XPatternField*, this ) );
    return (aRet.hasValue() ? aRet : UnoSpinFieldControl::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoPatternFieldControl )
    getCppuType( ( uno::Reference< awt::XPatternField>* ) NULL ),
    UnoSpinFieldControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoPatternFieldControl::setString( const ::rtl::OUString& rString ) throw(uno::RuntimeException)
{
    setText( rString );
}

::rtl::OUString UnoPatternFieldControl::getString() throw(uno::RuntimeException)
{
    return getText();
}

void UnoPatternFieldControl::setMasks( const ::rtl::OUString& EditMask, const ::rtl::OUString& LiteralMask ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= EditMask;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_EDITMASK ), aAny, sal_True );
    aAny <<= LiteralMask;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LITERALMASK ), aAny, sal_True );
}

void UnoPatternFieldControl::getMasks( ::rtl::OUString& EditMask, ::rtl::OUString& LiteralMask ) throw(uno::RuntimeException)
{
    EditMask = ImplGetPropertyValue_UString( BASEPROPERTY_EDITMASK );
    LiteralMask = ImplGetPropertyValue_UString( BASEPROPERTY_LITERALMASK );
}

void UnoPatternFieldControl::setStrictFormat( sal_Bool bStrict ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= bStrict;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRICTFORMAT ), aAny, sal_True );
}

sal_Bool UnoPatternFieldControl::isStrictFormat() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_STRICTFORMAT );
}


//  ----------------------------------------------------
//  class UnoControlProgressBarModel
//  ----------------------------------------------------
UnoControlProgressBarModel::UnoControlProgressBarModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FILLCOLOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_PROGRESSVALUE );
    ImplRegisterProperty( BASEPROPERTY_PROGRESSVALUE_MAX );
    ImplRegisterProperty( BASEPROPERTY_PROGRESSVALUE_MIN );
}

::rtl::OUString UnoControlProgressBarModel::getServiceName( ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlProgressBarModel );
}

uno::Any UnoControlProgressBarModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlProgressBar );
        return aAny;
    }

    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlProgressBarModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlProgressBarModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


//  ----------------------------------------------------
//  class UnoProgressBarControl
//  ----------------------------------------------------
UnoProgressBarControl::UnoProgressBarControl()
{
}

::rtl::OUString UnoProgressBarControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "ProgressBar" );
}

// uno::XInterface
uno::Any UnoProgressBarControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XProgressBar*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoProgressBarControl )
    getCppuType( ( uno::Reference< awt::XProgressBar>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

// ::com::sun::star::awt::XProgressBar
void UnoProgressBarControl::setForegroundColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= nColor;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_FILLCOLOR ), aAny, sal_True );
}

void UnoProgressBarControl::setBackgroundColor( sal_Int32 nColor ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= nColor;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_BACKGROUNDCOLOR ), aAny, sal_True );
}

void UnoProgressBarControl::setValue( sal_Int32 nValue ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= nValue;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_PROGRESSVALUE ), aAny, sal_True );
}

void UnoProgressBarControl::setRange( sal_Int32 nMin, sal_Int32 nMax ) throw(::com::sun::star::uno::RuntimeException )
{
    uno::Any aMin;
    uno::Any aMax;

    if ( nMin < nMax )
    {
        // take correct min and max
        aMin <<= nMin;
        aMax <<= nMax;
    }
    else
    {
        // change min and max
        aMin <<= nMax;
        aMax <<= nMin;
    }

    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_PROGRESSVALUE_MIN ), aMin, sal_True );
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_PROGRESSVALUE_MAX ), aMax, sal_True );
}

sal_Int32 UnoProgressBarControl::getValue() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_PROGRESSVALUE );
}


//  ----------------------------------------------------
//  class UnoControlScrollBarModel
//  ----------------------------------------------------
UnoControlScrollBarModel::UnoControlScrollBarModel()
{
    ImplRegisterProperty( BASEPROPERTY_BLOCKINCREMENT );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LINEINCREMENT );
    ImplRegisterProperty( BASEPROPERTY_ORIENTATION );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_SCROLLVALUE );
    ImplRegisterProperty( BASEPROPERTY_SCROLLVALUE_MAX );
    ImplRegisterProperty( BASEPROPERTY_VISIBLESIZE );
}

::rtl::OUString UnoControlScrollBarModel::getServiceName( ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlScrollBarModel );
}

uno::Any UnoControlScrollBarModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlScrollBar );
        return aAny;
    }

    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlScrollBarModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlScrollBarModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


//  ----------------------------------------------------
//  class UnoScrollBarControl
//  ----------------------------------------------------
UnoScrollBarControl::UnoScrollBarControl()
    : maAdjustmentListeners( *this )
{
}

::rtl::OUString UnoScrollBarControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "ScrollBar" );
}

// ::com::sun::star::uno::XInterface
uno::Any UnoScrollBarControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XAdjustmentListener*, this ),
                                        SAL_STATIC_CAST( awt::XScrollBar*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoScrollBarControl )
    getCppuType( ( uno::Reference< awt::XAdjustmentListener>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XScrollBar>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoScrollBarControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maAdjustmentListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoScrollBarControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XScrollBar >  xScrollBar( getPeer(), uno::UNO_QUERY );
    xScrollBar->addAdjustmentListener( this );
}

// ::com::sun::star::awt::XAdjustmentListener
void UnoScrollBarControl::adjustmentValueChanged( const ::com::sun::star::awt::AdjustmentEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException)
{
    switch ( rEvent.Type )
    {
        case ::com::sun::star::awt::AdjustmentType_ADJUST_LINE:
        case ::com::sun::star::awt::AdjustmentType_ADJUST_PAGE:
        case ::com::sun::star::awt::AdjustmentType_ADJUST_ABS:
        {
            uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );

            if ( xScrollBar.is() )
            {
                uno::Any aAny;
                aAny <<= xScrollBar->getValue();
                ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE ), aAny, sal_False );
            }
        }
        break;
        default:
        {
            DBG_ERROR( "UnoScrollBarControl::adjustmentValueChanged - unknown Type" );

        }
    }

    if ( maAdjustmentListeners.getLength() )
        maAdjustmentListeners.adjustmentValueChanged( rEvent );
}

// ::com::sun::star::awt::XScrollBar
void UnoScrollBarControl::addAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    maAdjustmentListeners.addInterface( l );
}

void UnoScrollBarControl::removeAdjustmentListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XAdjustmentListener > & l ) throw(::com::sun::star::uno::RuntimeException)
{
    maAdjustmentListeners.removeInterface( l );
}

void UnoScrollBarControl::setValue( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= n;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE ), aAny, sal_True );
}

void UnoScrollBarControl::setValues( sal_Int32 nValue, sal_Int32 nVisible, sal_Int32 nMax ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= nValue;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE ), aAny, sal_True );
    aAny <<= nVisible;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VISIBLESIZE ), aAny, sal_True );
    aAny <<= nMax;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE_MAX ), aAny, sal_True );
}

sal_Int32 UnoScrollBarControl::getValue() throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32 n;
    if ( getPeer().is() )
    {
        uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
        n = xScrollBar->getValue();
    }
    return n;
}

void UnoScrollBarControl::setMaximum( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= n;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SCROLLVALUE_MAX ), aAny, sal_True );
}

sal_Int32 UnoScrollBarControl::getMaximum() throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32 n;
    if ( getPeer().is() )
    {
        uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
        n = xScrollBar->getMaximum();
    }
    return n;
}

void UnoScrollBarControl::setLineIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= n;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LINEINCREMENT ), aAny, sal_True );
}

sal_Int32 UnoScrollBarControl::getLineIncrement() throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32 n;
    if ( getPeer().is() )
    {
        uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
        n = xScrollBar->getLineIncrement();
    }
    return n;
}

void UnoScrollBarControl::setBlockIncrement( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= n;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_BLOCKINCREMENT ), aAny, sal_True );
}

sal_Int32 UnoScrollBarControl::getBlockIncrement() throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32 n;
    if ( getPeer().is() )
    {
        uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
        n = xScrollBar->getBlockIncrement();
    }
    return n;
}

void UnoScrollBarControl::setVisibleSize( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= n;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VISIBLESIZE ), aAny, sal_True );
}

sal_Int32 UnoScrollBarControl::getVisibleSize() throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32 n;
    if ( getPeer().is() )
    {
        uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
        n = xScrollBar->getVisibleSize();
    }
    return n;
}

void UnoScrollBarControl::setOrientation( sal_Int32 n ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= n;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_ORIENTATION ), aAny, sal_True );
}

sal_Int32 UnoScrollBarControl::getOrientation() throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int32 n;
    if ( getPeer().is() )
    {
        uno::Reference< awt::XScrollBar > xScrollBar( getPeer(), uno::UNO_QUERY );
        n = xScrollBar->getOrientation();
    }
    return n;
}


//  ----------------------------------------------------
//  class UnoControlFixedLineModel
//  ----------------------------------------------------
UnoControlFixedLineModel::UnoControlFixedLineModel()
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_ORIENTATION );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
}

::rtl::OUString UnoControlFixedLineModel::getServiceName( ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlFixedLineModel );
}

uno::Any UnoControlFixedLineModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlFixedLine );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlFixedLineModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlFixedLineModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//  ----------------------------------------------------
//  class UnoFixedLineControl
//  ----------------------------------------------------
UnoFixedLineControl::UnoFixedLineControl()
{
    maComponentInfos.nWidth = 100;      // ??
    maComponentInfos.nHeight = 100;     // ??
}

::rtl::OUString UnoFixedLineControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "FixedLine" );
}

sal_Bool UnoFixedLineControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}
