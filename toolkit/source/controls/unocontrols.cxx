/*************************************************************************
 *
 *  $RCSfile: unocontrols.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-02 11:07:20 $
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


#include <toolkit/controls/unocontrols.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/helper/unomemorystream.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/helper/macros.hxx>

#include <cppuhelper/typeprovider.hxx>

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>

#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>

#include <tools/debug.hxx>

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
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_HSCROLL );
    ImplRegisterProperty( BASEPROPERTY_MAXTEXTLEN );
    ImplRegisterProperty( BASEPROPERTY_MULTILINE );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
    ImplRegisterProperty( BASEPROPERTY_VSCROLL );
}

::rtl::OUString UnoControlEditModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlEditModel );
}

::com::sun::star::uno::Any UnoControlEditModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        ::com::sun::star::uno::Any aAny;
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
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlEditModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
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
}

::rtl::OUString UnoEditControl::GetComponentServiceName()
{
    ::rtl::OUString aName( ::rtl::OUString::createFromAscii( "Edit" ) );
    ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_MULTILINE ) );
    sal_Bool b;
    if ( ( aVal >>= b ) && b )
        aName= ::rtl::OUString::createFromAscii( "MultiLineEdit" );
    return aName;
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoEditControl::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTextComponent*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTextListener*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XEventListener*, SAL_STATIC_CAST( ::com::sun::star::awt::XTextListener*, this ) ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XLayoutConstrains*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTextLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoEditControl )
getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent>* ) NULL ),
getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener>* ) NULL ),
getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains>* ) NULL ),
getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextLayoutConstrains>* ) NULL ),
UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END


void UnoEditControl::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maTextListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoEditControl::createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > & rxToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  & rParentPeer ) throw(::com::sun::star::uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent > xText( mxPeer, ::com::sun::star::uno::UNO_QUERY );
    xText->addTextListener( this );
}

void UnoEditControl::textChanged(const ::com::sun::star::awt::TextEvent& e) throw(::com::sun::star::uno::RuntimeException)
{
    // Neuen Text als ::com::sun::star::beans::Property ins Model treten.
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent > xText( mxPeer, ::com::sun::star::uno::UNO_QUERY );
    DBG_ASSERT( xText.is(), "TextComponentInterface?" );

    ::com::sun::star::uno::Any aAny;
    aAny <<= xText->getText();
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TEXT ), aAny, sal_False );

    if ( maTextListeners.getLength() )
        maTextListeners.textChanged( e );
}

void UnoEditControl::addTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    maTextListeners.addInterface( l );
}

void UnoEditControl::removeTextListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    maTextListeners.removeInterface( l );
}

void UnoEditControl::setText( const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= aText;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TEXT ), aAny, sal_True );
}

void UnoEditControl::insertText( const ::com::sun::star::awt::Selection& rSel, const ::rtl::OUString& aText ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= aText;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TEXT ), aAny, sal_True );
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent > xText( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xText->insertText( rSel, aText );
    }
}

::rtl::OUString UnoEditControl::getText() throw(::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString aText;
    ::rtl::OUString aLineBreakProp( GetPropertyName( BASEPROPERTY_HARDLINEBREAKS ) );
    if ( mxPeer.is() && ImplHasProperty( aLineBreakProp ) && ImplGetPropertyValue_BOOL( BASEPROPERTY_HARDLINEBREAKS ) )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextArea > xText( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        aText = xText->getTextLines();
    }
    else if ( mxPeer.is() )
    {
        // Peer fragen, weil es die ::com::sun::star::beans::Property "Text" nicht geben muss...
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent > xText( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        aText = xText->getText();
    }
    else
    {
        aText = ImplGetPropertyValue_UString( BASEPROPERTY_TEXT );
    }

    return aText;
}

::rtl::OUString UnoEditControl::getSelectedText( void ) throw(::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString aSelected;
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent > xText( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        aSelected = xText->getSelectedText();
    }
    return aSelected;
}

void UnoEditControl::setSelection( const ::com::sun::star::awt::Selection& aSelection ) throw(::com::sun::star::uno::RuntimeException)
{
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent > xText( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xText->setSelection( aSelection );
    }
}

::com::sun::star::awt::Selection UnoEditControl::getSelection( void ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::awt::Selection aSel;
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextComponent > xText( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        aSel = xText->getSelection();
    }
    return aSel;
}

sal_Bool UnoEditControl::isEditable( void ) throw(::com::sun::star::uno::RuntimeException)
{
    return !ImplGetPropertyValue_BOOL( BASEPROPERTY_READONLY );
}

void UnoEditControl::setEditable( sal_Bool bEditable ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= (sal_Bool)!bEditable;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_READONLY ), aAny, sal_True );
}

sal_Int16 UnoEditControl::getMaxTextLen() throw(::com::sun::star::uno::RuntimeException)
{
    return !ImplGetPropertyValue_INT16( BASEPROPERTY_MAXTEXTLEN );
}

void UnoEditControl::setMaxTextLen( sal_Int16 nLen ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= (sal_Int16)nLen;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_MAXTEXTLEN ), aAny, sal_True );
}

::com::sun::star::awt::Size UnoEditControl::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

::com::sun::star::awt::Size UnoEditControl::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

::com::sun::star::awt::Size UnoEditControl::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

::com::sun::star::awt::Size UnoEditControl::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getMinimumSize( nCols, nLines );
}

void UnoEditControl::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    Impl_getColumnsAndLines( nCols, nLines );
}

//  ----------------------------------------------------
//  class UnoControlFormattedFieldModel
//  ----------------------------------------------------
UnoControlFormattedFieldModel::UnoControlFormattedFieldModel()
{
    ImplRegisterProperty( BASEPROPERTY_ALIGN );
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_DEFAULT );
    ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_VALUE );
    ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_MAX );
    ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_MIN );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_FORMATKEY );
    ImplRegisterProperty( BASEPROPERTY_FORMATSSUPPLIER );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_MAXTEXTLEN );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_SPIN );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );

    ::com::sun::star::uno::Any aTreatAsNumber;
    aTreatAsNumber <<= (sal_Bool) sal_True;
    ImplRegisterProperty( BASEPROPERTY_TREATASNUMBER, aTreatAsNumber );
}

::rtl::OUString UnoControlFormattedFieldModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlFormattedFieldModel );
}

::com::sun::star::uno::Any UnoControlFormattedFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    ::com::sun::star::uno::Any aReturn;
    switch (nPropId)
    {
        case BASEPROPERTY_DEFAULTCONTROL: aReturn <<= ::rtl::OUString( ::rtl::OUString::createFromAscii( szServiceName_UnoControlFormattedField ) );

        case BASEPROPERTY_TREATASNUMBER: aReturn <<= (sal_Bool)sal_True; break;

        case BASEPROPERTY_EFFECTIVE_DEFAULT:
        case BASEPROPERTY_EFFECTIVE_VALUE:
        case BASEPROPERTY_EFFECTIVE_MAX:
        case BASEPROPERTY_EFFECTIVE_MIN:
        case BASEPROPERTY_FORMATKEY:
        case BASEPROPERTY_FORMATSSUPPLIER:
            // (void)
            break;

        default : aReturn = UnoControlModel::ImplGetDefaultValue( nPropId ); break;
    }

    return aReturn;
}

::cppu::IPropertyArrayHelper& UnoControlFormattedFieldModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlFormattedFieldModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//  ----------------------------------------------------
//  class UnoFormattedFieldControl
//  ----------------------------------------------------
UnoFormattedFieldControl::UnoFormattedFieldControl()
{
}

::rtl::OUString UnoFormattedFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "FormattedField" );
}

void UnoFormattedFieldControl::textChanged(const ::com::sun::star::awt::TextEvent& e) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >  xPeer(mxPeer, ::com::sun::star::uno::UNO_QUERY);
    DBG_ASSERT(xPeer.is(), "UnoFormattedFieldControl::textChanged : what kind of peer do I have ?");
    ::rtl::OUString sEffectiveValue = GetPropertyName( BASEPROPERTY_EFFECTIVE_VALUE );
    ImplSetPropertyValue( sEffectiveValue, xPeer->getProperty( sEffectiveValue ), sal_False );

    if ( GetTextListeners().getLength() )
        GetTextListeners().textChanged( e );
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
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

::rtl::OUString UnoControlFileControlModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlFileControlModel );
}

::com::sun::star::uno::Any UnoControlFileControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        ::com::sun::star::uno::Any aAny;
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
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlFileControlModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
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
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_IMAGEURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

::rtl::OUString UnoControlButtonModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlButtonModel );
}

::com::sun::star::uno::Any UnoControlButtonModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        ::com::sun::star::uno::Any aAny;
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
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlButtonModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
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
    return ::rtl::OUString::createFromAscii( "pushbutton" );
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoButtonControl::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XButton*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XImageConsumer*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XImageProducer*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoButtonControl )
getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XButton>* ) NULL ),
getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer>* ) NULL ),
getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer>* ) NULL ),
getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains>* ) NULL ),
UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoButtonControl::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoButtonControl::createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > & rxToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  & rParentPeer ) throw(::com::sun::star::uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    ::com::sun::star::uno::Reference < ::com::sun::star::awt::XButton >  xButton( mxPeer, ::com::sun::star::uno::UNO_QUERY );
    if ( maActionListeners.getLength() )
        xButton->addActionListener( &maActionListeners );
}

void UnoButtonControl::addActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    maActionListeners.addInterface( l );
    if( mxPeer.is() && maActionListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XButton >  xButton( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xButton->addActionListener( &maActionListeners );
    }
}

void UnoButtonControl::removeActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    if( mxPeer.is() && maActionListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XButton >  xButton( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xButton->removeActionListener( &maActionListeners );
    }
    maActionListeners.removeInterface( l );
}

void UnoButtonControl::setLabel( const ::rtl::OUString&  rLabel ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= rLabel;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

void UnoButtonControl::setActionCommand( const ::rtl::OUString& rCommand ) throw(::com::sun::star::uno::RuntimeException)
{
    maActionCommand = rCommand;
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XButton >  xButton( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xButton->setActionCommand( rCommand );
    }
}

::com::sun::star::awt::Size UnoButtonControl::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

::com::sun::star::awt::Size UnoButtonControl::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

::com::sun::star::awt::Size UnoButtonControl::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

void UnoButtonControl::init( sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException)
{
    maImageConsumer.Init( Width, Height );
}

void UnoButtonControl::setColorModel( sal_Int16 BitCount, const ::com::sun::star::uno::Sequence< sal_Int32 >& RGBAPal, sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask, sal_Int32 AlphaMask ) throw(::com::sun::star::uno::RuntimeException)
{
    maImageConsumer.SetColorModel( BitCount, RGBAPal.getLength(), (const unsigned long*)RGBAPal.getConstArray(), RedMask, GreenMask, BlueMask, AlphaMask );
}

void UnoButtonControl::setPixelsByBytes( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, const ::com::sun::star::uno::Sequence< sal_Int8 >& ProducerData, sal_Int32 Offset, sal_Int32 Scansize ) throw(::com::sun::star::uno::RuntimeException)
{
    maImageConsumer.SetPixelsByBytes( X, Y, Width, Height, (const unsigned char*)ProducerData.getConstArray(), Offset, Scansize );
    ImplUpdateImage( sal_True );
}

void UnoButtonControl::setPixelsByLongs( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, const ::com::sun::star::uno::Sequence< sal_Int32 >& ProducerData, sal_Int32 Offset, sal_Int32 Scansize ) throw(::com::sun::star::uno::RuntimeException)
{
    maImageConsumer.SetPixelsByLongs( X, Y, Width, Height, (const unsigned long*)ProducerData.getConstArray(), Offset, Scansize );
    ImplUpdateImage( sal_True );
}

void UnoButtonControl::complete( sal_Int32 Status, const ::com::sun::star::uno::Reference < ::com::sun::star::awt::XImageProducer > & Producer ) throw(::com::sun::star::uno::RuntimeException)
{
    maImageConsumer.Completed( Status );

    // MT: Controls sollen angemeldet bleiben...
//  Producer->removeConsumer( this );

    ImplUpdateImage( sal_True );
}

void UnoButtonControl::addConsumer( const ::com::sun::star::uno::Reference < ::com::sun::star::awt::XImageConsumer > & Consumer ) throw(::com::sun::star::uno::RuntimeException)
{
    if ( mxImageProducer.is() )
        mxImageProducer->addConsumer( Consumer );
}

void UnoButtonControl::removeConsumer( const ::com::sun::star::uno::Reference < ::com::sun::star::awt::XImageConsumer > & Consumer ) throw(::com::sun::star::uno::RuntimeException)
{
    if ( mxImageProducer.is() )
        mxImageProducer->removeConsumer( Consumer );
}

void UnoButtonControl::startProduction() throw(::com::sun::star::uno::RuntimeException)
{
    if ( mxImageProducer.is() )
    {
        UnoMemoryStream* pStrm = new UnoMemoryStream( 0x3FFF, 0x3FFF );
        (*pStrm) << maBitmap;

        ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream >  xIn = pStrm;
//      mxImageProducer->setImage( xIn );
        mxImageProducer->startProduction();
    }
}

void UnoButtonControl::ImplUpdateImage( sal_Bool bGetNewImage )
{
    sal_Bool bOK = bGetNewImage ? maImageConsumer.GetData( maBitmap ) : sal_True;
    if ( bOK && mxPeer.is() && mxImageProducer.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XImageConsumer >  xC( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        addConsumer( xC );
        startProduction();
        removeConsumer( xC );
    }
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
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_IMAGEURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
}

::rtl::OUString UnoControlImageControlModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlImageControlModel );
}

::com::sun::star::uno::Any UnoControlImageControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        ::com::sun::star::uno::Any aAny;
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
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlImageControlModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
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

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    ::com::sun::star::uno::Reference < ::com::sun::star::uno::XInterface > xI = xMSF->createInstance( ::rtl::OUString::createFromAscii( szServiceName_ImageProducer ) );
    if ( xI.is() )
        mxImageProducer = ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer >( xI, ::com::sun::star::uno::UNO_QUERY );
}

::rtl::OUString UnoImageControlControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "fixedimage" );
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoImageControlControl::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XImageConsumer*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XImageProducer*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoImageControlControl )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoImageControlControl::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

sal_Bool UnoImageControlControl::isTransparent() throw(::com::sun::star::uno::RuntimeException)
{
    return sal_True;
}

::com::sun::star::awt::Size UnoImageControlControl::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

::com::sun::star::awt::Size UnoImageControlControl::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

::com::sun::star::awt::Size UnoImageControlControl::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

void UnoImageControlControl::init( sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException)
{
    maImageConsumer.Init( Width, Height );
}

void UnoImageControlControl::setColorModel( sal_Int16 BitCount, const ::com::sun::star::uno::Sequence< sal_Int32 >& RGBAPal, sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask, sal_Int32 AlphaMask ) throw(::com::sun::star::uno::RuntimeException)
{
    maImageConsumer.SetColorModel( BitCount, RGBAPal.getLength(), (const unsigned long*)RGBAPal.getConstArray(), RedMask, GreenMask, BlueMask, AlphaMask );
}

void UnoImageControlControl::setPixelsByBytes( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, const ::com::sun::star::uno::Sequence< sal_Int8 >& ProducerData, sal_Int32 Offset, sal_Int32 Scansize ) throw(::com::sun::star::uno::RuntimeException)
{
    maImageConsumer.SetPixelsByBytes( X, Y, Width, Height, (const unsigned char*)ProducerData.getConstArray(), Offset, Scansize );
    ImplUpdateImage( sal_True );
}

void UnoImageControlControl::setPixelsByLongs( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, const ::com::sun::star::uno::Sequence< sal_Int32 >& ProducerData, sal_Int32 Offset, sal_Int32 Scansize ) throw(::com::sun::star::uno::RuntimeException)
{
    maImageConsumer.SetPixelsByLongs( X, Y, Width, Height, (const unsigned long*)ProducerData.getConstArray(), Offset, Scansize );
    ImplUpdateImage( sal_True );
}

void UnoImageControlControl::complete( sal_Int32 Status, const ::com::sun::star::uno::Reference < ::com::sun::star::awt::XImageProducer > & Producer ) throw(::com::sun::star::uno::RuntimeException)
{
    maImageConsumer.Completed( Status );

    // MT: Controls sollen angemeldet bleiben...
//  Producer->removeConsumer( this );

    ImplUpdateImage( sal_True );
}

void UnoImageControlControl::addConsumer( const ::com::sun::star::uno::Reference < ::com::sun::star::awt::XImageConsumer > & Consumer ) throw(::com::sun::star::uno::RuntimeException)
{
    if ( mxImageProducer.is() )
        mxImageProducer->addConsumer( Consumer );
}

void UnoImageControlControl::removeConsumer( const ::com::sun::star::uno::Reference < ::com::sun::star::awt::XImageConsumer > & Consumer ) throw(::com::sun::star::uno::RuntimeException)
{
    if ( mxImageProducer.is() )
        mxImageProducer->removeConsumer( Consumer );
}

void UnoImageControlControl::startProduction() throw(::com::sun::star::uno::RuntimeException)
{
    if ( mxImageProducer.is() )
    {
        UnoMemoryStream* pStrm = new UnoMemoryStream( 0x3FFF, 0x3FFF );
        (*pStrm) << maBitmap;

        ::com::sun::star::uno::Reference < ::com::sun::star::io::XInputStream >  xIn = pStrm;
//      mxImageProducer->setImage( xIn );
        mxImageProducer->startProduction();
    }
}

void UnoImageControlControl::ImplUpdateImage( sal_Bool bGetNewImage )
{
    sal_Bool bOK = bGetNewImage ? maImageConsumer.GetData( maBitmap ) : sal_True;
    if ( bOK && mxPeer.is() && mxImageProducer.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XImageConsumer >  xC( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        addConsumer( xC );
        startProduction();
        removeConsumer( xC );
    }
}

//  ----------------------------------------------------
//  class UnoControlRadioButtonModel
//  ----------------------------------------------------
UnoControlRadioButtonModel::UnoControlRadioButtonModel()
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_STATE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

::rtl::OUString UnoControlRadioButtonModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlRadioButtonModel );
}

::com::sun::star::uno::Any UnoControlRadioButtonModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        ::com::sun::star::uno::Any aAny;
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
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlRadioButtonModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoRadioButtonControl
//  ----------------------------------------------------
UnoRadioButtonControl::UnoRadioButtonControl()
    : maItemListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

::rtl::OUString UnoRadioButtonControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "radiobutton" );
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoRadioButtonControl::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XRadioButton*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XItemListener*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XEventListener*, SAL_STATIC_CAST( ::com::sun::star::awt::XItemListener*, this ) ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoRadioButtonControl )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRadioButton>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoRadioButtonControl::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}


sal_Bool UnoRadioButtonControl::isTransparent() throw(::com::sun::star::uno::RuntimeException)
{
    return sal_True;
}

void UnoRadioButtonControl::createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > & rxToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  & rParentPeer ) throw(::com::sun::star::uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    ::com::sun::star::uno::Reference < ::com::sun::star::awt::XRadioButton >  xRadioButton( mxPeer, ::com::sun::star::uno::UNO_QUERY );
    xRadioButton->addItemListener( this );

    // Toggle-Verhalten abklemmen, macht DG
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer >  xVclWindowPeer( mxPeer, ::com::sun::star::uno::UNO_QUERY );
    ::com::sun::star::uno::Any aAny;
    aAny <<= (sal_Bool)sal_False;
    xVclWindowPeer->setProperty( GetPropertyName( BASEPROPERTY_AUTOTOGGLE ), aAny );
}

void UnoRadioButtonControl::addItemListener(const ::com::sun::star::uno::Reference < ::com::sun::star::awt::XItemListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    maItemListeners.addInterface( l );
}

void UnoRadioButtonControl::removeItemListener(const ::com::sun::star::uno::Reference < ::com::sun::star::awt::XItemListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    maItemListeners.removeInterface( l );
}

void UnoRadioButtonControl::setLabel( const ::rtl::OUString&  rLabel ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= rLabel;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

void UnoRadioButtonControl::setState( sal_Bool bOn ) throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int16 nState = bOn ? 1 : 0;
    ::com::sun::star::uno::Any aAny;
    aAny <<= nState;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ), aAny, sal_True );
}

sal_Bool UnoRadioButtonControl::getState() throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int16 nState = 0;
    ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ) );
    aVal >>= nState;
    return nState ? sal_True : sal_False;
}

void UnoRadioButtonControl::itemStateChanged( const ::com::sun::star::awt::ItemEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= (sal_Int16)rEvent.Selected;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ), aAny, sal_False );

    if ( maItemListeners.getLength() )
        maItemListeners.itemStateChanged( rEvent );
}

::com::sun::star::awt::Size UnoRadioButtonControl::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

::com::sun::star::awt::Size UnoRadioButtonControl::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

::com::sun::star::awt::Size UnoRadioButtonControl::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
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
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_STATE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
    ImplRegisterProperty( BASEPROPERTY_TRISTATE );
}

::rtl::OUString UnoControlCheckBoxModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlCheckBoxModel );
}

::com::sun::star::uno::Any UnoControlCheckBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        ::com::sun::star::uno::Any aAny;
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
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlCheckBoxModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoCheckBoxControl
//  ----------------------------------------------------
UnoCheckBoxControl::UnoCheckBoxControl()
    : maItemListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

::rtl::OUString UnoCheckBoxControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "checkbox" );
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoCheckBoxControl::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XCheckBox*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XItemListener*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XEventListener*, SAL_STATIC_CAST( ::com::sun::star::awt::XItemListener*, this ) ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoCheckBoxControl )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XCheckBox>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoCheckBoxControl::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

sal_Bool UnoCheckBoxControl::isTransparent() throw(::com::sun::star::uno::RuntimeException)
{
    return sal_True;
}

void UnoCheckBoxControl::createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > & rxToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  & rParentPeer ) throw(::com::sun::star::uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    ::com::sun::star::uno::Reference < ::com::sun::star::awt::XCheckBox >  xCheckBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
    xCheckBox->addItemListener( this );
}

void UnoCheckBoxControl::addItemListener(const ::com::sun::star::uno::Reference < ::com::sun::star::awt::XItemListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    maItemListeners.addInterface( l );
}

void UnoCheckBoxControl::removeItemListener(const ::com::sun::star::uno::Reference < ::com::sun::star::awt::XItemListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    maItemListeners.removeInterface( l );
}

void UnoCheckBoxControl::setLabel( const ::rtl::OUString&  rLabel ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= rLabel;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

void UnoCheckBoxControl::setState( short n ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= (sal_Int16)n;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ), aAny, sal_True );
}

short UnoCheckBoxControl::getState() throw(::com::sun::star::uno::RuntimeException)
{
    short nState = 0;
    ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ) );
    aVal >>= nState;
    return nState;
}

void UnoCheckBoxControl::enableTriState( sal_Bool b ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= b;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TRISTATE ), aAny, sal_True );
}

void UnoCheckBoxControl::itemStateChanged( const ::com::sun::star::awt::ItemEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= (sal_Int16)rEvent.Selected;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ), aAny, sal_False );

    if ( maItemListeners.getLength() )
        maItemListeners.itemStateChanged( rEvent );
}

::com::sun::star::awt::Size UnoCheckBoxControl::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

::com::sun::star::awt::Size UnoCheckBoxControl::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

::com::sun::star::awt::Size UnoCheckBoxControl::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
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
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_MULTILINE );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

::rtl::OUString UnoControlFixedTextModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlFixedTextModel );
}

::com::sun::star::uno::Any UnoControlFixedTextModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        ::com::sun::star::uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlFixedText );
        return aAny;
    }
    else if ( nPropId == BASEPROPERTY_BORDER )
    {
        ::com::sun::star::uno::Any aAny;
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
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlFixedTextModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
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

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoFixedTextControl::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XFixedText*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoFixedTextControl )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFixedText>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

sal_Bool UnoFixedTextControl::isTransparent() throw(::com::sun::star::uno::RuntimeException)
{
    return sal_True;
}

void UnoFixedTextControl::setText( const ::rtl::OUString& Text ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= Text;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

::rtl::OUString UnoFixedTextControl::getText() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_UString( BASEPROPERTY_LABEL );
}

void UnoFixedTextControl::setAlignment( short nAlign ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= (sal_Int16)nAlign;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

short UnoFixedTextControl::getAlignment() throw(::com::sun::star::uno::RuntimeException)
{
    short nAlign = 0;
    if ( mxModel.is() )
    {
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_ALIGN ) );
        aVal >>= nAlign;
    }
    return nAlign;
}

::com::sun::star::awt::Size UnoFixedTextControl::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

::com::sun::star::awt::Size UnoFixedTextControl::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

::com::sun::star::awt::Size UnoFixedTextControl::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
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
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

::rtl::OUString UnoControlGroupBoxModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlGroupBoxModel );
}

::com::sun::star::uno::Any UnoControlGroupBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        ::com::sun::star::uno::Any aAny;
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
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlGroupBoxModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
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

sal_Bool UnoGroupBoxControl::isTransparent() throw(::com::sun::star::uno::RuntimeException)
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
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LINECOUNT );
    ImplRegisterProperty( BASEPROPERTY_MULTISELECTION );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_SELECTEDITEMS );
    ImplRegisterProperty( BASEPROPERTY_STRINGITEMLIST );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
}

::rtl::OUString UnoControlListBoxModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlListBoxModel );
}

::com::sun::star::uno::Any UnoControlListBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        ::com::sun::star::uno::Any aAny;
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
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlListBoxModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

void UnoControlListBoxModel::ImplPropertyChanged( sal_uInt16 nPropId )
{
    if ( nPropId == BASEPROPERTY_STRINGITEMLIST )
    {
        ::com::sun::star::uno::Sequence<sal_Int16> aSeq;
        ::com::sun::star::uno::Any aAny;
        aAny <<= aSeq;
        setPropertyValue( GetPropertyName( BASEPROPERTY_SELECTEDITEMS ), aAny );
    }
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

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoListBoxControl::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XListBox*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XItemListener*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::lang::XEventListener*, SAL_STATIC_CAST( ::com::sun::star::awt::XItemListener*, this ) ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XLayoutConstrains*, this ),
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTextLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoListBoxControl )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XListBox>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XItemListener>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XLayoutConstrains>* ) NULL ),
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTextLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoListBoxControl::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aEvt );
    maItemListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoListBoxControl::ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal )
{
    UnoControl::ImplSetPeerProperty( rPropName, rVal );

    // Wenn die SelectedItems vor der StringItemList gesetzt werden,
    // hat das keine Auswirkung...
    if ( rPropName == GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) )
    {
        ::rtl::OUString aSelPropName = GetPropertyName( BASEPROPERTY_SELECTEDITEMS );
        ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( aSelPropName );
        if ( !( aVal.getValueType().getTypeClass() == ::com::sun::star::uno::TypeClass_VOID ) )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XVclWindowPeer > xW( mxPeer, ::com::sun::star::uno::UNO_QUERY );
            xW->setProperty( aSelPropName, aVal );
        }
    }
}

void UnoListBoxControl::createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > & rxToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  & rParentPeer ) throw(::com::sun::star::uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    ::com::sun::star::uno::Reference < ::com::sun::star::awt::XListBox >  xListBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
    xListBox->addItemListener( this );

    if ( maActionListeners.getLength() )
        xListBox->addActionListener( &maActionListeners );
}

void UnoListBoxControl::addActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    maActionListeners.addInterface( l );
    if( mxPeer.is() && maActionListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XListBox >  xListBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xListBox->addActionListener( &maActionListeners );
    }
}

void UnoListBoxControl::removeActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    if( mxPeer.is() && maActionListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XListBox >  xListBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xListBox->removeActionListener( &maActionListeners );
    }
    maActionListeners.removeInterface( l );
}

void UnoListBoxControl::addItemListener(const ::com::sun::star::uno::Reference < ::com::sun::star::awt::XItemListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    maItemListeners.addInterface( l );
}

void UnoListBoxControl::removeItemListener(const ::com::sun::star::uno::Reference < ::com::sun::star::awt::XItemListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    maItemListeners.removeInterface( l );
}

void UnoListBoxControl::addItem( const ::rtl::OUString& aItem, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq( 1 );
    aSeq.getArray()[0] = aItem;
    addItems( aSeq, nPos );
}

void UnoListBoxControl::addItems( const ::com::sun::star::uno::Sequence< ::rtl::OUString>& aItems, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nNewItems = (sal_uInt16)aItems.getLength();
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    sal_uInt16 nNewLen = nOldLen + nNewItems;

    ::com::sun::star::uno::Sequence< ::rtl::OUString> aNewSeq( nNewLen );
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

    ::com::sun::star::uno::Any aAny;
    aAny <<= aNewSeq;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ), aAny, sal_True );
}

void UnoListBoxControl::removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    if ( nOldLen && ( nPos < nOldLen ) )
    {
        if ( nCount > ( nOldLen-nPos ) )
            nCount = nOldLen-nPos;

        sal_uInt16 nNewLen = nOldLen - nCount;

        ::com::sun::star::uno::Sequence< ::rtl::OUString> aNewSeq( nNewLen );
        ::rtl::OUString* pNewData = aNewSeq.getArray();
        ::rtl::OUString* pOldData = aSeq.getArray();

        sal_uInt16 n;
        // Items vor der Entfern-Position
        for ( n = 0; n < nPos; n++ )
            pNewData[n] = pOldData[n];

        // Rest der Items
        for ( n = nPos; n < (nOldLen-nCount); n++ )
            pNewData[n] = pOldData[n+nCount];

        ::com::sun::star::uno::Any aAny;
        aAny <<= aNewSeq;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ), aAny, sal_True );
    }
}

sal_Int16 UnoListBoxControl::getItemCount() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    return (sal_Int16)aSeq.getLength();
}

::rtl::OUString UnoListBoxControl::getItem( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString aItem;
    ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    if ( nPos < aSeq.getLength() )
        aItem = aSeq.getConstArray()[nPos];
    return aItem;
}

::com::sun::star::uno::Sequence< ::rtl::OUString> UnoListBoxControl::getItems() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    return aSeq;
}

sal_Int16 UnoListBoxControl::getSelectedItemPos() throw(::com::sun::star::uno::RuntimeException)
{
    sal_Int16 n = -1;
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XListBox >  xListBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        n = xListBox->getSelectedItemPos();
    }
    return n;
}

::com::sun::star::uno::Sequence<sal_Int16> UnoListBoxControl::getSelectedItemsPos() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence<sal_Int16> aSeq;
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XListBox >  xListBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        aSeq = xListBox->getSelectedItemsPos();
    }
    return aSeq;
}

::rtl::OUString UnoListBoxControl::getSelectedItem() throw(::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString aItem;
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XListBox >  xListBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        aItem = xListBox->getSelectedItem();
    }
    return aItem;
}

::com::sun::star::uno::Sequence< ::rtl::OUString> UnoListBoxControl::getSelectedItems() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XListBox >  xListBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        aSeq = xListBox->getSelectedItems();
    }
    return aSeq;
}

void UnoListBoxControl::selectItemPos( sal_Int16 nPos, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException)
{
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XListBox >  xListBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xListBox->selectItemPos( nPos, bSelect );
    }
}

void UnoListBoxControl::selectItemsPos( const ::com::sun::star::uno::Sequence<sal_Int16>& aPositions, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException)
{
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XListBox >  xListBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xListBox->selectItemsPos( aPositions, bSelect );
    }
}

void UnoListBoxControl::selectItem( const ::rtl::OUString& aItem, sal_Bool bSelect ) throw(::com::sun::star::uno::RuntimeException)
{
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XListBox >  xListBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xListBox->selectItem( aItem, bSelect );
    }
}

void UnoListBoxControl::makeVisible( sal_Int16 nEntry ) throw(::com::sun::star::uno::RuntimeException)
{
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XListBox >  xListBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xListBox->makeVisible( nEntry );
    }
}

void UnoListBoxControl::setDropDownLineCount( sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= (sal_Int16)nLines;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LINECOUNT ), aAny, sal_True );
}

sal_Int16 UnoListBoxControl::getDropDownLineCount() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_INT16( BASEPROPERTY_LINECOUNT );
}

sal_Bool UnoListBoxControl::isMutipleMode() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_STRINGITEMLIST );
}

void UnoListBoxControl::setMultipleMode( sal_Bool bMulti ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= bMulti;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_MULTISELECTION ), aAny, sal_True );
}

void UnoListBoxControl::itemStateChanged( const ::com::sun::star::awt::ItemEvent& rEvent ) throw(::com::sun::star::uno::RuntimeException)
{
    // Neue ::com::sun::star::uno::Sequence als ::com::sun::star::beans::Property ins Model treten.
    ::com::sun::star::uno::Reference < ::com::sun::star::awt::XListBox >  xListBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
    DBG_ASSERT( xListBox.is(), "XListBox?" );

    ::com::sun::star::uno::Sequence<sal_Int16> aSeq = xListBox->getSelectedItemsPos();
    ::com::sun::star::uno::Any aAny;
    aAny <<= aSeq;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SELECTEDITEMS ), aAny, sal_False );

    if ( maItemListeners.getLength() )
        maItemListeners.itemStateChanged( rEvent );
}

::com::sun::star::awt::Size UnoListBoxControl::getMinimumSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

::com::sun::star::awt::Size UnoListBoxControl::getPreferredSize(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

::com::sun::star::awt::Size UnoListBoxControl::calcAdjustedSize( const ::com::sun::star::awt::Size& rNewSize ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

::com::sun::star::awt::Size UnoListBoxControl::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    return Impl_getMinimumSize( nCols, nLines );
}

void UnoListBoxControl::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(::com::sun::star::uno::RuntimeException)
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
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LINECOUNT );
    ImplRegisterProperty( BASEPROPERTY_MAXTEXTLEN );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_STRINGITEMLIST );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

::rtl::OUString UnoControlComboBoxModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlComboBoxModel );
}

::com::sun::star::uno::Any UnoControlComboBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        ::com::sun::star::uno::Any aAny;
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
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlComboBoxModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
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

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoComboBoxControl::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XComboBox*, this ) );
    return (aRet.hasValue() ? aRet : UnoEditControl::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoComboBoxControl )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XComboBox>* ) NULL ),
    UnoEditControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoComboBoxControl::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aEvt );
    maItemListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoComboBoxControl::createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit > & rxToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  & rParentPeer ) throw(::com::sun::star::uno::RuntimeException)
{
    UnoEditControl::createPeer( rxToolkit, rParentPeer );

    ::com::sun::star::uno::Reference < ::com::sun::star::awt::XComboBox >  xComboBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
    if ( maActionListeners.getLength() )
        xComboBox->addActionListener( &maActionListeners );
    if ( maItemListeners.getLength() )
        xComboBox->addItemListener( &maItemListeners );
}

void UnoComboBoxControl::addActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    maActionListeners.addInterface( l );
    if( mxPeer.is() && maActionListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XComboBox >  xComboBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xComboBox->addActionListener( &maActionListeners );
    }
}

void UnoComboBoxControl::removeActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    if( mxPeer.is() && maActionListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XComboBox >  xComboBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xComboBox->removeActionListener( &maActionListeners );
    }
    maActionListeners.removeInterface( l );
}

void UnoComboBoxControl::addItemListener(const ::com::sun::star::uno::Reference < ::com::sun::star::awt::XItemListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    maItemListeners.addInterface( l );
    if( mxPeer.is() && maItemListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XComboBox >  xComboBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xComboBox->addItemListener( &maItemListeners );
    }
}

void UnoComboBoxControl::removeItemListener(const ::com::sun::star::uno::Reference < ::com::sun::star::awt::XItemListener > & l) throw(::com::sun::star::uno::RuntimeException)
{
    if( mxPeer.is() && maItemListeners.getLength() == 1 )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XComboBox >  xComboBox( mxPeer, ::com::sun::star::uno::UNO_QUERY );   // MT: Mal alles so umstellen, schoener als Ref anlegen und query rufen
        xComboBox->removeItemListener( &maItemListeners );
    }
    maItemListeners.removeInterface( l );
}

void UnoComboBoxControl::addItem( const ::rtl::OUString& aItem, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq( 1 );
    aSeq.getArray()[0] = aItem;
    addItems( aSeq, nPos );
}

void UnoComboBoxControl::addItems( const ::com::sun::star::uno::Sequence< ::rtl::OUString>& aItems, sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nNewItems = (sal_uInt16)aItems.getLength();
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    sal_uInt16 nNewLen = nOldLen + nNewItems;

    ::com::sun::star::uno::Sequence< ::rtl::OUString> aNewSeq( nNewLen );
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

    ::com::sun::star::uno::Any aAny;
    aAny <<= aNewSeq;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ), aAny, sal_True );
}

void UnoComboBoxControl::removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    if ( nOldLen && ( nPos < nOldLen ) )
    {
        if ( nCount > ( nOldLen-nPos ) )
            nCount = nOldLen-nPos;

        sal_uInt16 nNewLen = nOldLen - nCount;

        ::com::sun::star::uno::Sequence< ::rtl::OUString> aNewSeq( nNewLen );
        ::rtl::OUString* pNewData = aNewSeq.getArray();
        ::rtl::OUString* pOldData = aSeq.getArray();

        sal_uInt16 n;
        // Items vor der Entfern-Position
        for ( n = 0; n < nPos; n++ )
            pNewData[n] = pOldData[n];

        // Rest der Items
        for ( n = nPos; n < (nOldLen-nCount); n++ )
            pNewData[n] = pOldData[n+nCount];

        ::com::sun::star::uno::Any aAny;
        aAny <<= aNewSeq;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ), aAny, sal_True );
    }
}

sal_Int16 UnoComboBoxControl::getItemCount() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    return (sal_Int16)aSeq.getLength();
}

::rtl::OUString UnoComboBoxControl::getItem( sal_Int16 nPos ) throw(::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString aItem;
    ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    if ( nPos < aSeq.getLength() )
        aItem = aSeq.getConstArray()[nPos];
    return aItem;
}

::com::sun::star::uno::Sequence< ::rtl::OUString> UnoComboBoxControl::getItems() throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    ::com::sun::star::uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    return aSeq;
}

void UnoComboBoxControl::setDropDownLineCount( sal_Int16 nLines ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= nLines;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LINECOUNT ), aAny, sal_True );
}

sal_Int16 UnoComboBoxControl::getDropDownLineCount() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_INT16( BASEPROPERTY_LINECOUNT );
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
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_SPIN );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

::rtl::OUString UnoControlDateFieldModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlDateFieldModel );
}

::com::sun::star::uno::Any UnoControlDateFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        ::com::sun::star::uno::Any aAny;
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
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlDateFieldModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoDateFieldControl
//  ----------------------------------------------------
UnoDateFieldControl::UnoDateFieldControl()
{
}

::rtl::OUString UnoDateFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "datefield" );
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoDateFieldControl::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XDateField*, this ) );
    return (aRet.hasValue() ? aRet : UnoEditControl::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoDateFieldControl )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDateField>* ) NULL ),
    UnoEditControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoDateFieldControl::textChanged( const ::com::sun::star::awt::TextEvent& e ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference < ::com::sun::star::awt::XDateField >  xField( mxPeer, ::com::sun::star::uno::UNO_QUERY );
    ::com::sun::star::uno::Any aValue;
    if ( !xField->isEmpty() )
        aValue <<= xField->getDate();

    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_DATE ), aValue, sal_False );

    if ( GetTextListeners().getLength() )
        GetTextListeners().textChanged( e );
}

void UnoDateFieldControl::setDate( sal_Int32 Date ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= Date;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_DATE ), aAny, sal_True );
}

sal_Int32 UnoDateFieldControl::getDate() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_DATE );
}

void UnoDateFieldControl::setMin( sal_Int32 Date ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= Date;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_DATEMIN ), aAny, sal_True );
}

sal_Int32 UnoDateFieldControl::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_DATEMIN );
}

void UnoDateFieldControl::setMax( sal_Int32 Date ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= Date;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_DATEMAX ), aAny, sal_True );
}

sal_Int32 UnoDateFieldControl::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_DATEMAX );
}

void UnoDateFieldControl::setFirst( sal_Int32 Date ) throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoDateFieldControl::setFirst not supported" );
}

sal_Int32 UnoDateFieldControl::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoDateFieldControl::getFirst not supported" );
    return 0;
}

void UnoDateFieldControl::setLast( sal_Int32 Date ) throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoDateFieldControl::setLast not supported" );
}

sal_Int32 UnoDateFieldControl::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoDateFieldControl::getLast not supported" );
    return 0;
}

void UnoDateFieldControl::setLongFormat( sal_Bool bLong ) throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoDateFieldControl::setLongFormat not supported" );
}

sal_Bool UnoDateFieldControl::isLongFormat() throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoDateFieldControl::getLongFormat not supported" );
    return sal_False;
}

void UnoDateFieldControl::setEmpty() throw(::com::sun::star::uno::RuntimeException)
{
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XDateField >  xField( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xField->setEmpty();
    }
}

sal_Bool UnoDateFieldControl::isEmpty() throw(::com::sun::star::uno::RuntimeException)
{
    sal_Bool bEmpty = sal_False;
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XDateField > xField( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        bEmpty = xField->isEmpty();
    }
    return bEmpty;
}

void UnoDateFieldControl::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= bStrict;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRICTFORMAT ), aAny, sal_True );
}

sal_Bool UnoDateFieldControl::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
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
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_SPIN );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
    ImplRegisterProperty( BASEPROPERTY_TIME );
    ImplRegisterProperty( BASEPROPERTY_TIMEMAX );
    ImplRegisterProperty( BASEPROPERTY_TIMEMIN );
}

::rtl::OUString UnoControlTimeFieldModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlTimeFieldModel );
}

::com::sun::star::uno::Any UnoControlTimeFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        ::com::sun::star::uno::Any aAny;
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
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlTimeFieldModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoTimeFieldControl
//  ----------------------------------------------------
UnoTimeFieldControl::UnoTimeFieldControl()
{
}

::rtl::OUString UnoTimeFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "timefield" );
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoTimeFieldControl::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XTimeField*, this ) );
    return (aRet.hasValue() ? aRet : UnoEditControl::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoTimeFieldControl )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTimeField>* ) NULL ),
    UnoEditControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoTimeFieldControl::textChanged( const ::com::sun::star::awt::TextEvent& e ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference < ::com::sun::star::awt::XTimeField >  xField( mxPeer, ::com::sun::star::uno::UNO_QUERY );
    ::com::sun::star::uno::Any aValue;
    if ( !xField->isEmpty() )
        xField->getTime();
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TIME ), aValue, sal_False );

    if ( GetTextListeners().getLength() )
        GetTextListeners().textChanged( e );
}

void UnoTimeFieldControl::setTime( sal_Int32 Time ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= Time;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TIME ), aAny, sal_True );
}

sal_Int32 UnoTimeFieldControl::getTime() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_TIME );
}

void UnoTimeFieldControl::setMin( sal_Int32 Time ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= Time;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TIMEMIN ), aAny, sal_True );
}

sal_Int32 UnoTimeFieldControl::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_TIMEMIN );
}

void UnoTimeFieldControl::setMax( sal_Int32 Time ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= Time;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TIMEMAX ), aAny, sal_True );
}

sal_Int32 UnoTimeFieldControl::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_TIMEMAX );
}

void UnoTimeFieldControl::setFirst( sal_Int32 Time ) throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoTimeFieldControl::setFirst not supported" );
}

sal_Int32 UnoTimeFieldControl::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoTimeFieldControl::getFirst not supported" );
    return 0;
}

void UnoTimeFieldControl::setLast( sal_Int32 Time ) throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoTimeFieldControl::setLast not supported" );
}

sal_Int32 UnoTimeFieldControl::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoTimeFieldControl::getLast not supported" );
    return 0;
}

void UnoTimeFieldControl::setEmpty() throw(::com::sun::star::uno::RuntimeException)
{
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XTimeField >  xField( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xField->setEmpty();
    }
}

sal_Bool UnoTimeFieldControl::isEmpty() throw(::com::sun::star::uno::RuntimeException)
{
    sal_Bool bEmpty = sal_False;
    if ( mxPeer.is() )
    {
        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XTimeField >  xField( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        bEmpty = xField->isEmpty();
    }
    return bEmpty;
}

void UnoTimeFieldControl::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= bStrict;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRICTFORMAT ), aAny, sal_True );
}

sal_Bool UnoTimeFieldControl::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
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
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_NUMSHOWTHOUSANDSEP );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_SPIN );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
    ImplRegisterProperty( BASEPROPERTY_VALUEMAX_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUEMIN_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUESTEP_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUE_DOUBLE );
}

::rtl::OUString UnoControlNumericFieldModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlNumericFieldModel );
}

::com::sun::star::uno::Any UnoControlNumericFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        ::com::sun::star::uno::Any aAny;
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
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlNumericFieldModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoNumericFieldControl
//  ----------------------------------------------------
UnoNumericFieldControl::UnoNumericFieldControl()
{
}

::rtl::OUString UnoNumericFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "numericfield" );
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoNumericFieldControl::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XNumericField*, this ) );
    return (aRet.hasValue() ? aRet : UnoEditControl::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoNumericFieldControl )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XNumericField>* ) NULL ),
    UnoEditControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoNumericFieldControl::textChanged( const ::com::sun::star::awt::TextEvent& e ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference < ::com::sun::star::awt::XNumericField >  xField( mxPeer, ::com::sun::star::uno::UNO_QUERY );
    ::com::sun::star::uno::Any aAny;
    aAny <<= xField->getValue();
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUE_DOUBLE ), aAny, sal_False );

    if ( GetTextListeners().getLength() )
        GetTextListeners().textChanged( e );
}

void UnoNumericFieldControl::setValue( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUE_DOUBLE ), aAny, sal_True );
}

double UnoNumericFieldControl::getValue() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUE_DOUBLE );
}

void UnoNumericFieldControl::setMin( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUEMIN_DOUBLE ), aAny, sal_True );
}

double UnoNumericFieldControl::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUEMIN_DOUBLE );
}

void UnoNumericFieldControl::setMax( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUEMAX_DOUBLE ), aAny, sal_True );
}

double UnoNumericFieldControl::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUEMAX_DOUBLE );
}

void UnoNumericFieldControl::setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoNumericFieldControl::setFirst not supported" );
}

double UnoNumericFieldControl::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoNumericFieldControl::getFirst not supported" );
    return 0;
}

void UnoNumericFieldControl::setLast( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoNumericFieldControl::setLast not supported" );
}

double UnoNumericFieldControl::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoNumericFieldControl::getLast not supported" );
    return 0;
}

void UnoNumericFieldControl::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= bStrict;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRICTFORMAT ), aAny, sal_True );
}

sal_Bool UnoNumericFieldControl::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_STRICTFORMAT );
}

void UnoNumericFieldControl::setSpinSize( double Digits ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= Digits;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUESTEP_DOUBLE ), aAny, sal_True );
}

double UnoNumericFieldControl::getSpinSize() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUESTEP_DOUBLE );
}

void UnoNumericFieldControl::setDecimalDigits( sal_Int16 Digits ) throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoNumericFieldControl::setDecimalDigits not supported" );
}

sal_Int16 UnoNumericFieldControl::getDecimalDigits() throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoNumericFieldControl::getDecimalDigits not supported" );
    return 0;
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
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_NUMSHOWTHOUSANDSEP );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_SPIN );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
    ImplRegisterProperty( BASEPROPERTY_VALUEMAX_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUEMIN_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUESTEP_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUE_DOUBLE );
}

::rtl::OUString UnoControlCurrencyFieldModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlCurrencyFieldModel );
}

::com::sun::star::uno::Any UnoControlCurrencyFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        ::com::sun::star::uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlCurrencyField );
        return aAny;
    }
    if ( nPropId == BASEPROPERTY_CURSYM_POSITION )
    {
        ::com::sun::star::uno::Any aAny;
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
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlCurrencyFieldModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//  ----------------------------------------------------
//  class UnoCurrencyFieldControl
//  ----------------------------------------------------
UnoCurrencyFieldControl::UnoCurrencyFieldControl()
{
}

::rtl::OUString UnoCurrencyFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "longcurrencyfield" );
}

// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoCurrencyFieldControl::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XCurrencyField*, this ) );
    return (aRet.hasValue() ? aRet : UnoEditControl::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoCurrencyFieldControl )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XCurrencyField>* ) NULL ),
    UnoEditControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoCurrencyFieldControl::textChanged( const ::com::sun::star::awt::TextEvent& e ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference < ::com::sun::star::awt::XCurrencyField >  xField( mxPeer, ::com::sun::star::uno::UNO_QUERY );
    ::com::sun::star::uno::Any aAny;
    aAny <<= xField->getValue();
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUE_DOUBLE ), aAny, sal_False );

    if ( GetTextListeners().getLength() )
        GetTextListeners().textChanged( e );
}

void UnoCurrencyFieldControl::setValue( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUE_DOUBLE ), aAny, sal_True );
}

double UnoCurrencyFieldControl::getValue() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUE_DOUBLE );
}

void UnoCurrencyFieldControl::setMin( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUEMIN_DOUBLE ), aAny, sal_True );
}

double UnoCurrencyFieldControl::getMin() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUEMIN_DOUBLE );
}

void UnoCurrencyFieldControl::setMax( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUEMAX_DOUBLE ), aAny, sal_True );
}

double UnoCurrencyFieldControl::getMax() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUEMAX_DOUBLE );
}

void UnoCurrencyFieldControl::setFirst( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoCurrencyFieldControl::setFirst not supported" );
}

double UnoCurrencyFieldControl::getFirst() throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoCurrencyFieldControl::getFirst not supported" );
    return 0;
}

void UnoCurrencyFieldControl::setLast( double Value ) throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoCurrencyFieldControl::setLast not supported" );
}

double UnoCurrencyFieldControl::getLast() throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoCurrencyFieldControl::getLast not supported" );
    return 0;
}

void UnoCurrencyFieldControl::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= bStrict;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRICTFORMAT ), aAny, sal_True );
}

sal_Bool UnoCurrencyFieldControl::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_STRICTFORMAT );
}

void UnoCurrencyFieldControl::setSpinSize( double Digits ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= Digits;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUESTEP_DOUBLE ), aAny, sal_True );
}

double UnoCurrencyFieldControl::getSpinSize() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUESTEP_DOUBLE );
}

void UnoCurrencyFieldControl::setDecimalDigits( sal_Int16 Digits ) throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoCurrencyFieldControl::setDecimalDigits not supported" );
}

sal_Int16 UnoCurrencyFieldControl::getDecimalDigits() throw(::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING( "UnoCurrencyFieldControl::getDecimalDigits not supported" );
    return 0;
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
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LITERALMASK );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

::rtl::OUString UnoControlPatternFieldModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlPatternFieldModel );
}

::com::sun::star::uno::Any UnoControlPatternFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        ::com::sun::star::uno::Any aAny;
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
        ::com::sun::star::uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// ::com::sun::star::beans::XMultiPropertySet
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > UnoControlPatternFieldModel::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    static ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
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

void UnoPatternFieldControl::ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal )
{
    sal_uInt16 nType = GetPropertyId( rPropName );
    if ( ( nType == BASEPROPERTY_TEXT ) || ( nType == BASEPROPERTY_EDITMASK ) || ( nType == BASEPROPERTY_LITERALMASK ) )
    {
        // Die Masken koennen nicht nacheinander gesetzt werden.
        ::rtl::OUString Text = ImplGetPropertyValue_UString( BASEPROPERTY_TEXT );
        ::rtl::OUString EditMask = ImplGetPropertyValue_UString( BASEPROPERTY_EDITMASK );
        ::rtl::OUString LiteralMask = ImplGetPropertyValue_UString( BASEPROPERTY_LITERALMASK );

        ::com::sun::star::uno::Reference < ::com::sun::star::awt::XPatternField >  xPF( mxPeer, ::com::sun::star::uno::UNO_QUERY );
        xPF->setString( Text );
        xPF->setMasks( EditMask, LiteralMask );
    }
    else
        UnoControl::ImplSetPeerProperty( rPropName, rVal );
}


// ::com::sun::star::uno::XInterface
::com::sun::star::uno::Any UnoPatternFieldControl::queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( ::com::sun::star::awt::XPatternField*, this ) );
    return (aRet.hasValue() ? aRet : UnoEditControl::queryAggregation( rType ));
}

// ::com::sun::star::lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoPatternFieldControl )
    getCppuType( ( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XPatternField>* ) NULL ),
    UnoEditControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoPatternFieldControl::setString( const ::rtl::OUString& rString ) throw(::com::sun::star::uno::RuntimeException)
{
    setText( rString );
}

::rtl::OUString UnoPatternFieldControl::getString() throw(::com::sun::star::uno::RuntimeException)
{
    return getText();
}

void UnoPatternFieldControl::setMasks( const ::rtl::OUString& EditMask, const ::rtl::OUString& LiteralMask ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= EditMask;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_EDITMASK ), aAny, sal_True );
    aAny <<= LiteralMask;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LITERALMASK ), aAny, sal_True );
}

void UnoPatternFieldControl::getMasks( ::rtl::OUString& EditMask, ::rtl::OUString& LiteralMask ) throw(::com::sun::star::uno::RuntimeException)
{
    EditMask = ImplGetPropertyValue_UString( BASEPROPERTY_EDITMASK );
    LiteralMask = ImplGetPropertyValue_UString( BASEPROPERTY_LITERALMASK );
}

void UnoPatternFieldControl::setStrictFormat( sal_Bool bStrict ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aAny;
    aAny <<= bStrict;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRICTFORMAT ), aAny, sal_True );
}

sal_Bool UnoPatternFieldControl::isStrictFormat() throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_STRICTFORMAT );
}



