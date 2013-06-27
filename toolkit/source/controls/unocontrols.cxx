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

#include <com/sun/star/awt/XTextArea.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/awt/LineEndFormat.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/GraphicObject.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>


#include <toolkit/controls/formattedcontrol.hxx>
#include <toolkit/controls/roadmapcontrol.hxx>
#include <toolkit/controls/unocontrols.hxx>
#include <toolkit/controls/stdtabcontroller.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/imagealign.hxx>

// for introspection
#include <toolkit/awt/vclxwindows.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/extract.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/date.hxx>
#include <tools/time.hxx>

#include <algorithm>
#include <functional>

using namespace css;
using namespace css::awt;
using namespace css::lang;
using namespace css::uno;
using ::com::sun::star::graphic::XGraphic;
using ::com::sun::star::uno::Reference;
using namespace ::toolkit;

#define IMPL_SERVICEINFO_DERIVED( ImplName, BaseClass, ServiceName ) \
    OUString SAL_CALL ImplName::getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException) { return OUString( "stardiv.Toolkit." #ImplName ); } \
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL ImplName::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException) \
                            { \
                                ::com::sun::star::uno::Sequence< OUString > aNames = BaseClass::getSupportedServiceNames( ); \
                                aNames.realloc( aNames.getLength() + 1 ); \
                                aNames[ aNames.getLength() - 1 ] = OUString::createFromAscii( ServiceName ); \
                                return aNames; \
                            } \


uno::Reference< graphic::XGraphic >
ImageHelper::getGraphicAndGraphicObjectFromURL_nothrow( uno::Reference< graphic::XGraphicObject >& xOutGraphicObj, const OUString& _rURL )
{
    if ( _rURL.startsWith( UNO_NAME_GRAPHOBJ_URLPREFIX ) )
    {
        // graphic manager uniqueid
        OUString sID = _rURL.copy( sizeof( UNO_NAME_GRAPHOBJ_URLPREFIX ) - 1 );
        xOutGraphicObj = graphic::GraphicObject::createWithId( ::comphelper::getProcessComponentContext(), sID );
    }
    else // linked
        xOutGraphicObj = NULL; // release the GraphicObject

    return ImageHelper::getGraphicFromURL_nothrow( _rURL );
}

::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphic >
ImageHelper::getGraphicFromURL_nothrow( const OUString& _rURL )
{
    uno::Reference< graphic::XGraphic > xGraphic;
    if ( _rURL.isEmpty() )
        return xGraphic;

    try
    {
        uno::Reference< uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
        uno::Reference< graphic::XGraphicProvider > xProvider( graphic::GraphicProvider::create(xContext) );
        uno::Sequence< beans::PropertyValue > aMediaProperties(1);
        aMediaProperties[0].Name = OUString( "URL" );
        aMediaProperties[0].Value <<= _rURL;
        xGraphic = xProvider->queryGraphic( aMediaProperties );
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return xGraphic;
}
//  ----------------------------------------------------
//  class UnoControlEditModel
//  ----------------------------------------------------
UnoControlEditModel::UnoControlEditModel( const Reference< XComponentContext >& rxContext )
    :UnoControlModel( rxContext )
{
    UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXEdit );
}

OUString UnoControlEditModel::getServiceName( ) throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlEditModel );
}

uno::Any UnoControlEditModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    uno::Any aReturn;

    switch ( nPropId )
    {
    case BASEPROPERTY_LINE_END_FORMAT:
        aReturn <<= (sal_Int16)awt::LineEndFormat::LINE_FEED;   // LF
        break;
    case BASEPROPERTY_DEFAULTCONTROL:
        aReturn <<= OUString::createFromAscii( szServiceName_UnoControlEdit );
        break;
    default:
        aReturn = UnoControlModel::ImplGetDefaultValue( nPropId );
        break;
    }
    return aReturn;
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
    :UnoControlBase()
    ,maTextListeners( *this )
    ,mnMaxTextLen( 0 )
    ,mbSetTextInPeer( sal_False )
    ,mbSetMaxTextLenInPeer( sal_False )
    ,mbHasTextProperty( sal_False )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
    mnMaxTextLen = 0;
    mbSetMaxTextLenInPeer = sal_False;
}

uno::Any SAL_CALL UnoEditControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aReturn = UnoControlBase::queryAggregation( rType );
    if ( !aReturn.hasValue() )
        aReturn = UnoEditControl_Base::queryInterface( rType );
    return aReturn;
}

uno::Any SAL_CALL UnoEditControl::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException)
{
    return UnoControlBase::queryInterface( rType );
}

void SAL_CALL UnoEditControl::acquire(  ) throw ()
{
    UnoControlBase::acquire();
}

void SAL_CALL UnoEditControl::release(  ) throw ()
{
    UnoControlBase::release();
}

IMPLEMENT_FORWARD_XTYPEPROVIDER2( UnoEditControl, UnoControlBase, UnoEditControl_Base )

OUString UnoEditControl::GetComponentServiceName()
{
    // by default, we want a simple edit field
    OUString sName( "Edit" );

    // but maybe we are to display multi-line text?
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_MULTILINE ) );
    sal_Bool b = sal_Bool();
    if ( ( aVal >>= b ) && b )
        sName= OUString("MultiLineEdit");

    return sName;
}

sal_Bool SAL_CALL UnoEditControl::setModel(const uno::Reference< awt::XControlModel >& _rModel) throw ( uno::RuntimeException )
{
    sal_Bool bReturn = UnoControlBase::setModel( _rModel );
    mbHasTextProperty = ImplHasProperty( BASEPROPERTY_TEXT );
    return bReturn;
}

void UnoEditControl::ImplSetPeerProperty( const OUString& rPropName, const uno::Any& rVal )
{
    sal_Bool bDone = sal_False;
    if ( GetPropertyId( rPropName ) == BASEPROPERTY_TEXT )
    {
        // #96986# use setText(), or text listener will not be called.
        uno::Reference < awt::XTextComponent > xTextComponent( getPeer(), uno::UNO_QUERY );
        if ( xTextComponent.is() )
        {
            OUString sText;
            rVal >>= sText;
            ImplCheckLocalize( sText );
            xTextComponent->setText( sText );
            bDone = sal_True;
        }
    }

    if ( !bDone )
        UnoControlBase::ImplSetPeerProperty( rPropName, rVal );
}

void UnoEditControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt( *this );
    maTextListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoEditControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference< awt::XTextComponent > xText( getPeer(), uno::UNO_QUERY );
    if ( xText.is() )
    {
    xText->addTextListener( this );

    if ( mbSetMaxTextLenInPeer )
        xText->setMaxTextLen( mnMaxTextLen );
    if ( mbSetTextInPeer )
        xText->setText( maText );
    }
}

void UnoEditControl::textChanged(const awt::TextEvent& e) throw(uno::RuntimeException)
{
    uno::Reference< awt::XTextComponent > xText( getPeer(), uno::UNO_QUERY );

    if ( mbHasTextProperty )
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

void UnoEditControl::setText( const OUString& aText ) throw(uno::RuntimeException)
{
    if ( mbHasTextProperty )
    {
        uno::Any aAny;
        aAny <<= aText;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TEXT ), aAny, sal_True );
    }
    else
    {
        maText = aText;
        mbSetTextInPeer = sal_True;
            uno::Reference < awt::XTextComponent > xText( getPeer(), uno::UNO_QUERY );
        if ( xText.is() )
            xText->setText( maText );
        }

    // Setting the property to the VCLXWindow doesn't call textChanged
    if ( maTextListeners.getLength() )
    {
        awt::TextEvent aEvent;
        aEvent.Source = *this;
        maTextListeners.textChanged( aEvent );
    }
}

namespace
{
    static void lcl_normalize( awt::Selection& _rSel )
    {
        if ( _rSel.Min > _rSel.Max )
            ::std::swap( _rSel.Min, _rSel.Max );
    }
}

void UnoEditControl::insertText( const awt::Selection& rSel, const OUString& rNewText ) throw(uno::RuntimeException)
{
    // normalize the selection - OUString::replaceAt has a strange behaviour if the min is greater than the max
    awt::Selection aSelection( rSel );
    lcl_normalize( aSelection );

    // preserve the selection resp. cursor position
    awt::Selection aNewSelection( getSelection() );
#ifdef ALSO_PRESERVE_COMPLETE_SELECTION
        // (not sure - looks uglier ...)
    sal_Int32 nDeletedCharacters = ( aSelection.Max - aSelection.Min ) - rNewText.getLength();
    if ( aNewSelection.Min > aSelection.Min )
        aNewSelection.Min -= nDeletedCharacters;
    if ( aNewSelection.Max > aSelection.Max )
        aNewSelection.Max -= nDeletedCharacters;
#else
    aNewSelection.Max = ::std::min( aNewSelection.Min, aNewSelection.Max ) + rNewText.getLength();
    aNewSelection.Min = aNewSelection.Max;
#endif

    OUString aOldText = getText();
    OUString  aNewText = aOldText.replaceAt( aSelection.Min, aSelection.Max - aSelection.Min, rNewText );
    setText( aNewText );

    setSelection( aNewSelection );
}

OUString UnoEditControl::getText() throw(uno::RuntimeException)
{
    OUString aText = maText;

    if ( mbHasTextProperty )
        aText = ImplGetPropertyValue_UString( BASEPROPERTY_TEXT );
    else
    {
        uno::Reference< awt::XTextComponent > xText( getPeer(), uno::UNO_QUERY );
        if ( xText.is() )
            aText = xText->getText();
    }

    return aText;
}

OUString UnoEditControl::getSelectedText( void ) throw(uno::RuntimeException)
{
    OUString sSelected;
        uno::Reference< awt::XTextComponent > xText( getPeer(), uno::UNO_QUERY );
    if ( xText.is() )
        sSelected = xText->getSelectedText();

    return sSelected;
}

void UnoEditControl::setSelection( const awt::Selection& aSelection ) throw(uno::RuntimeException)
{
        uno::Reference< awt::XTextComponent > xText( getPeer(), uno::UNO_QUERY );
    if ( xText.is() )
        xText->setSelection( aSelection );
}

awt::Selection UnoEditControl::getSelection( void ) throw(uno::RuntimeException)
{
    awt::Selection aSel;
        uno::Reference< awt::XTextComponent > xText( getPeer(), uno::UNO_QUERY );
    if ( xText.is() )
        aSel = xText->getSelection();
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

    if ( ImplHasProperty( BASEPROPERTY_MAXTEXTLEN ) )
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
        mbSetMaxTextLenInPeer = sal_True;
            uno::Reference < awt::XTextComponent > xText( getPeer(), uno::UNO_QUERY );
        if ( xText.is() )
            xText->setMaxTextLen( mnMaxTextLen );
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

OUString UnoEditControl::getImplementationName(  ) throw(uno::RuntimeException)
{
    return OUString( "stardiv.Toolkit.UnoEditControl" );
}

uno::Sequence< OUString > UnoEditControl::getSupportedServiceNames() throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aNames = UnoControlBase::getSupportedServiceNames( );
    aNames.realloc( aNames.getLength() + 1 );
    aNames[ aNames.getLength() - 1 ] = OUString::createFromAscii( szServiceName2_UnoControlEdit );
    return aNames;
}

//  ----------------------------------------------------
//  class UnoControlFileControlModel
//  ----------------------------------------------------
UnoControlFileControlModel::UnoControlFileControlModel( const Reference< XComponentContext >& rxContext )
    :UnoControlModel( rxContext )
{
    ImplRegisterProperty( BASEPROPERTY_ALIGN );
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_BORDERCOLOR );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_ENABLEVISIBLE );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
    ImplRegisterProperty( BASEPROPERTY_VERTICALALIGN );
    ImplRegisterProperty( BASEPROPERTY_WRITING_MODE );
    ImplRegisterProperty( BASEPROPERTY_CONTEXT_WRITING_MODE );
    ImplRegisterProperty( BASEPROPERTY_HIDEINACTIVESELECTION );
}

OUString UnoControlFileControlModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlFileControlModel );
}

uno::Any UnoControlFileControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= OUString::createFromAscii( szServiceName_UnoControlFileControl );
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
    :UnoEditControl()
{
}

OUString UnoFileControl::GetComponentServiceName()
{
    return OUString("filecontrol");
}

//  ----------------------------------------------------
//  class GraphicControlModel
//  ----------------------------------------------------
uno::Any GraphicControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_GRAPHIC )
        return uno::makeAny( uno::Reference< graphic::XGraphic >() );

    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

void SAL_CALL GraphicControlModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception)
{
    UnoControlModel::setFastPropertyValue_NoBroadcast( nHandle, rValue );

    // - ImageAlign and ImagePosition need to correspond to each other
    // - Graphic and ImageURL need to correspond to each other
    try
    {
        switch ( nHandle )
        {
        case BASEPROPERTY_IMAGEURL:
            if ( !mbAdjustingGraphic && ImplHasProperty( BASEPROPERTY_GRAPHIC ) )
            {
                mbAdjustingGraphic = true;
                OUString sImageURL;
                OSL_VERIFY( rValue >>= sImageURL );
                setDependentFastPropertyValue( BASEPROPERTY_GRAPHIC, uno::makeAny( ImageHelper::getGraphicFromURL_nothrow( sImageURL ) ) );
                mbAdjustingGraphic = false;
            }
            break;

        case BASEPROPERTY_GRAPHIC:
            if ( !mbAdjustingGraphic && ImplHasProperty( BASEPROPERTY_IMAGEURL ) )
            {
                mbAdjustingGraphic = true;
                setDependentFastPropertyValue( BASEPROPERTY_IMAGEURL, uno::makeAny( OUString() ) );
                mbAdjustingGraphic = false;
            }
            break;

        case BASEPROPERTY_IMAGEALIGN:
            if ( !mbAdjustingImagePosition && ImplHasProperty( BASEPROPERTY_IMAGEPOSITION ) )
            {
                mbAdjustingImagePosition = true;
                sal_Int16 nUNOValue = 0;
                OSL_VERIFY( rValue >>= nUNOValue );
                setDependentFastPropertyValue( BASEPROPERTY_IMAGEPOSITION, uno::makeAny( getExtendedImagePosition( nUNOValue ) ) );
                mbAdjustingImagePosition = false;
            }
            break;
        case BASEPROPERTY_IMAGEPOSITION:
            if ( !mbAdjustingImagePosition && ImplHasProperty( BASEPROPERTY_IMAGEALIGN ) )
            {
                mbAdjustingImagePosition = true;
                sal_Int16 nUNOValue = 0;
                OSL_VERIFY( rValue >>= nUNOValue );
                setDependentFastPropertyValue( BASEPROPERTY_IMAGEALIGN, uno::makeAny( getCompatibleImageAlign( translateImagePosition( nUNOValue ) ) ) );
                mbAdjustingImagePosition = false;
            }
            break;
        }
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        OSL_FAIL( "GraphicControlModel::setFastPropertyValue_NoBroadcast: caught an exception while aligning the ImagePosition/ImageAlign properties!" );
        mbAdjustingImagePosition = sal_False;
    }
}

//  ----------------------------------------------------
//  class UnoControlButtonModel
//  ----------------------------------------------------
UnoControlButtonModel::UnoControlButtonModel( const Reference< XComponentContext >& rxContext )
    :GraphicControlModel( rxContext )
{
    UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXButton );

    osl_atomic_increment( &m_refCount );
    {
        setFastPropertyValue_NoBroadcast( BASEPROPERTY_IMAGEPOSITION, ImplGetDefaultValue( BASEPROPERTY_IMAGEPOSITION ) );
        // this ensures that our ImagePosition is consistent with our ImageAlign property (since both
        // defaults are not per se consistent), since both are coupled in setFastPropertyValue_NoBroadcast
    }
    osl_atomic_decrement( &m_refCount );
}

OUString UnoControlButtonModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlButtonModel );
}

uno::Any UnoControlButtonModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    switch ( nPropId )
    {
    case BASEPROPERTY_DEFAULTCONTROL:
        return uno::makeAny( OUString::createFromAscii( szServiceName_UnoControlButton ) );
    case BASEPROPERTY_TOGGLE:
        return uno::makeAny( (sal_Bool)sal_False );
    case BASEPROPERTY_ALIGN:
        return uno::makeAny( (sal_Int16)PROPERTY_ALIGN_CENTER );
    case BASEPROPERTY_FOCUSONCLICK:
        return uno::makeAny( (sal_Bool)sal_True );
    }

    return GraphicControlModel::ImplGetDefaultValue( nPropId );
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

//  ----------------------------------------------------
//  class UnoButtonControl
//  ----------------------------------------------------
UnoButtonControl::UnoButtonControl()
    :UnoButtonControl_Base()
    ,maActionListeners( *this )
    ,maItemListeners( *this )
{
    maComponentInfos.nWidth = 50;
    maComponentInfos.nHeight = 14;
}

OUString UnoButtonControl::GetComponentServiceName()
{
    OUString aName( "pushbutton" );
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_PUSHBUTTONTYPE ) );
    sal_Int16 n = sal_Int16();
    if ( ( aVal >>= n ) && n )
    {
        // Use PushButtonType later when available...
        switch ( n )
        {
            case 1 /*PushButtonType::OK*/:      aName= OUString("okbutton");
                                                break;
            case 2 /*PushButtonType::CANCEL*/:  aName= OUString("cancelbutton");
                                                break;
            case 3 /*PushButtonType::HELP*/:    aName= OUString("helpbutton");
                                                break;
            default:
            {
                OSL_FAIL( "Unknown Button Type!" );
            }
        }
    }
    return aName;
}

void UnoButtonControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aEvt );
    maItemListeners.disposeAndClear( aEvt );
    UnoControlBase::dispose();
}

void UnoButtonControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControlBase::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XButton > xButton( getPeer(), uno::UNO_QUERY );
    xButton->setActionCommand( maActionCommand );
    if ( maActionListeners.getLength() )
        xButton->addActionListener( &maActionListeners );

    uno::Reference< XToggleButton > xPushButton( getPeer(), uno::UNO_QUERY );
    if ( xPushButton.is() )
        xPushButton->addItemListener( this );
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

void UnoButtonControl::addItemListener(const uno::Reference< awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.addInterface( l );
}

void UnoButtonControl::removeItemListener(const uno::Reference< awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.removeInterface( l );
}

void SAL_CALL UnoButtonControl::disposing( const lang::EventObject& Source ) throw (uno::RuntimeException)
{
    UnoControlBase::disposing( Source );
}

void SAL_CALL UnoButtonControl::itemStateChanged( const awt::ItemEvent& rEvent ) throw (uno::RuntimeException)
{
    // forward to model
    uno::Any aAny;
    aAny <<= (sal_Int16)rEvent.Selected;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ), aAny, sal_False );

    // multiplex
    ItemEvent aEvent( rEvent );
    aEvent.Source = *this;
    maItemListeners.itemStateChanged( aEvent );
}

void UnoButtonControl::setLabel( const OUString&  rLabel ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= rLabel;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

void UnoButtonControl::setActionCommand( const OUString& rCommand ) throw(uno::RuntimeException)
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
UnoControlImageControlModel::UnoControlImageControlModel( const Reference< XComponentContext >& rxContext )
    :GraphicControlModel( rxContext )
    ,mbAdjustingImageScaleMode( false )
{
    UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXImageControl );
}

OUString UnoControlImageControlModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlImageControlModel );
}

uno::Any UnoControlImageControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
        return uno::makeAny( OUString::createFromAscii( szServiceName_UnoControlImageControl ) );

    if ( nPropId == BASEPROPERTY_IMAGE_SCALE_MODE )
        return makeAny( awt::ImageScaleMode::ANISOTROPIC );

    return GraphicControlModel::ImplGetDefaultValue( nPropId );
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

void SAL_CALL UnoControlImageControlModel::setFastPropertyValue_NoBroadcast( sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue ) throw (::com::sun::star::uno::Exception)
{
    GraphicControlModel::setFastPropertyValue_NoBroadcast( _nHandle, _rValue );

    // ScaleImage is an older (and less powerful) version of ScaleMode, but keep both in sync as far as possible
    try
    {
        switch ( _nHandle )
        {
        case BASEPROPERTY_IMAGE_SCALE_MODE:
            if ( !mbAdjustingImageScaleMode && ImplHasProperty( BASEPROPERTY_SCALEIMAGE ) )
            {
                mbAdjustingImageScaleMode = true;
                sal_Int16 nScaleMode( awt::ImageScaleMode::ANISOTROPIC );
                OSL_VERIFY( _rValue >>= nScaleMode );
                setDependentFastPropertyValue( BASEPROPERTY_SCALEIMAGE, uno::makeAny( sal_Bool( nScaleMode != awt::ImageScaleMode::NONE ) ) );
                mbAdjustingImageScaleMode = false;
            }
            break;
        case BASEPROPERTY_SCALEIMAGE:
            if ( !mbAdjustingImageScaleMode && ImplHasProperty( BASEPROPERTY_IMAGE_SCALE_MODE ) )
            {
                mbAdjustingImageScaleMode = true;
                sal_Bool bScale = sal_True;
                OSL_VERIFY( _rValue >>= bScale );
                setDependentFastPropertyValue( BASEPROPERTY_IMAGE_SCALE_MODE, uno::makeAny( bScale ? awt::ImageScaleMode::ANISOTROPIC : awt::ImageScaleMode::NONE ) );
                mbAdjustingImageScaleMode = false;
            }
            break;
        }
    }
    catch( const Exception& )
    {
        mbAdjustingImageScaleMode = false;
        throw;
    }
}

//  ----------------------------------------------------
//  class UnoImageControlControl
//  ----------------------------------------------------
UnoImageControlControl::UnoImageControlControl()
    :UnoImageControlControl_Base()
    ,maActionListeners( *this )
{
    // TODO: Where should I look for defaults?
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 100;
}

OUString UnoImageControlControl::GetComponentServiceName()
{
    return OUString("fixedimage");
}

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

//  ----------------------------------------------------
//  class UnoControlRadioButtonModel
//  ----------------------------------------------------
UnoControlRadioButtonModel::UnoControlRadioButtonModel( const Reference< XComponentContext >& rxContext )
    :GraphicControlModel( rxContext )
{
    UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXRadioButton );
}

OUString UnoControlRadioButtonModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlRadioButtonModel );
}

uno::Any UnoControlRadioButtonModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    switch ( nPropId )
    {
    case BASEPROPERTY_DEFAULTCONTROL:
        return uno::makeAny( OUString::createFromAscii( szServiceName_UnoControlRadioButton ) );

    case BASEPROPERTY_VISUALEFFECT:
        return uno::makeAny( (sal_Int16)awt::VisualEffect::LOOK3D );
    }

    return GraphicControlModel::ImplGetDefaultValue( nPropId );
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
    :UnoRadioButtonControl_Base()
    ,maItemListeners( *this )
    ,maActionListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

OUString UnoRadioButtonControl::GetComponentServiceName()
{
    return OUString("radiobutton");
}

void UnoRadioButtonControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aEvt );
    UnoControlBase::dispose();
}


sal_Bool UnoRadioButtonControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}

void UnoRadioButtonControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControlBase::createPeer( rxToolkit, rParentPeer );

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

void UnoRadioButtonControl::setLabel( const OUString&  rLabel ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= rLabel;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

void UnoRadioButtonControl::setActionCommand( const OUString& rCommand ) throw(uno::RuntimeException)
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

    // compatibility:
    // in OOo 1.0.x, when the user clicked a radio button in a group of buttons, this resulted
    // in _one_ itemStateChanged call for exactly the radio button which's state changed from
    // "0" to "1".
    // Nowadays, since the listener handling changed a lot towards 1.1 (the VCLXWindow reacts on more
    // basic events from the VCL-windows, not anymore on the Link-based events like in 1.0.x), this
    // isn't the case anymore: For instance, this method here gets called for the radio button
    // which is being implicitily _de_selected, too. This is pretty bad for compatibility.
    // Thus, we suppress all events with a new state other than "1". This is unlogical, and weird, when looking
    // from a pure API perspective, but it's _compatible_ with older product versions, and this is
    // all which matters here.
    // #i14703#
    if ( 1 == rEvent.Selected )
    {
        if ( maItemListeners.getLength() )
            maItemListeners.itemStateChanged( rEvent );
    }
        // note that speaking stricly, this is wrong: When in 1.0.x, the user would have de-selected
        // a radio button _without_ selecing another one, this would have caused a notification.
        // With the change done here, this today won't cause a notification anymore.
        //
        // Fortunately, it's not possible for the user to de-select a radio button without selecting another on,
        // at least not via the regular UI. It _would_ be possible via the Accessibility API, which
        // counts as "user input", too. But in 1.0.x, there was no Accessibility API, so there is nothing
        // to be inconsistent with.
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
UnoControlCheckBoxModel::UnoControlCheckBoxModel( const Reference< XComponentContext >& rxContext )
    :GraphicControlModel( rxContext )
{
    UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXCheckBox );
}

OUString UnoControlCheckBoxModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlCheckBoxModel );
}

uno::Any UnoControlCheckBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    switch ( nPropId )
    {
    case BASEPROPERTY_DEFAULTCONTROL:
        return uno::makeAny( OUString::createFromAscii( szServiceName_UnoControlCheckBox ) );

    case BASEPROPERTY_VISUALEFFECT:
        return uno::makeAny( (sal_Int16)awt::VisualEffect::LOOK3D );
    }

    return GraphicControlModel::ImplGetDefaultValue( nPropId );
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
    :UnoCheckBoxControl_Base()
    ,maItemListeners( *this ), maActionListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

OUString UnoCheckBoxControl::GetComponentServiceName()
{
    return OUString("checkbox");
}

void UnoCheckBoxControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aEvt );
    UnoControlBase::dispose();
}

sal_Bool UnoCheckBoxControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}

void UnoCheckBoxControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControlBase::createPeer( rxToolkit, rParentPeer );

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

void UnoCheckBoxControl::setActionCommand( const OUString& rCommand ) throw(uno::RuntimeException)
{
    maActionCommand = rCommand;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XButton > xButton( getPeer(), uno::UNO_QUERY );
        xButton->setActionCommand( rCommand );
    }
}


void UnoCheckBoxControl::setLabel( const OUString&  rLabel ) throw(uno::RuntimeException)
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
//  class UnoControlFixedHyperlinkModel
//  ----------------------------------------------------
UnoControlFixedHyperlinkModel::UnoControlFixedHyperlinkModel( const Reference< XComponentContext >& rxContext )
    :UnoControlModel( rxContext )
{
    UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXFixedHyperlink );
}

OUString UnoControlFixedHyperlinkModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlFixedHyperlinkModel );
}

uno::Any UnoControlFixedHyperlinkModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= OUString::createFromAscii( szServiceName_UnoControlFixedHyperlink );
        return aAny;
    }
    else if ( nPropId == BASEPROPERTY_BORDER )
    {
        uno::Any aAny;
        aAny <<= (sal_Int16)0;
        return aAny;
    }
    else if ( nPropId == BASEPROPERTY_URL )
    {
        uno::Any aAny;
        aAny <<= OUString();
        return aAny;
    }

    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlFixedHyperlinkModel::getInfoHelper()
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
uno::Reference< beans::XPropertySetInfo > UnoControlFixedHyperlinkModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//  ----------------------------------------------------
//  class UnoFixedHyperlinkControl
//  ----------------------------------------------------
UnoFixedHyperlinkControl::UnoFixedHyperlinkControl()
    :UnoControlBase()
    ,maActionListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

OUString UnoFixedHyperlinkControl::GetComponentServiceName()
{
    return OUString("fixedhyperlink");
}

// uno::XInterface
uno::Any UnoFixedHyperlinkControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< awt::XFixedHyperlink* >(this)),
                                        (static_cast< awt::XLayoutConstrains* >(this)) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoFixedHyperlinkControl )
    getCppuType( ( uno::Reference< awt::XFixedHyperlink>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

sal_Bool UnoFixedHyperlinkControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}

void UnoFixedHyperlinkControl::setText( const OUString& Text ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Text;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

OUString UnoFixedHyperlinkControl::getText() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_UString( BASEPROPERTY_LABEL );
}

void UnoFixedHyperlinkControl::setURL( const OUString& URL ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= URL;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_URL ), aAny, sal_True );
}

OUString UnoFixedHyperlinkControl::getURL(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ImplGetPropertyValue_UString( BASEPROPERTY_URL );
}

void UnoFixedHyperlinkControl::setAlignment( short nAlign ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= (sal_Int16)nAlign;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_ALIGN ), aAny, sal_True );
}

short UnoFixedHyperlinkControl::getAlignment() throw(uno::RuntimeException)
{
    short nAlign = 0;
    if ( mxModel.is() )
    {
        uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_ALIGN ) );
        aVal >>= nAlign;
    }
    return nAlign;
}

awt::Size UnoFixedHyperlinkControl::getMinimumSize(  ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

awt::Size UnoFixedHyperlinkControl::getPreferredSize(  ) throw(uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

awt::Size UnoFixedHyperlinkControl::calcAdjustedSize( const awt::Size& rNewSize ) throw(uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

void UnoFixedHyperlinkControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aEvt );
    UnoControlBase::dispose();
}

void UnoFixedHyperlinkControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControlBase::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XFixedHyperlink > xFixedHyperlink( getPeer(), uno::UNO_QUERY );
    if ( maActionListeners.getLength() )
        xFixedHyperlink->addActionListener( &maActionListeners );
}

void UnoFixedHyperlinkControl::addActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    maActionListeners.addInterface( l );
    if( getPeer().is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XFixedHyperlink >  xFixedHyperlink( getPeer(), uno::UNO_QUERY );
        xFixedHyperlink->addActionListener( &maActionListeners );
    }
}

void UnoFixedHyperlinkControl::removeActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    if( getPeer().is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XFixedHyperlink >  xFixedHyperlink( getPeer(), uno::UNO_QUERY );
        xFixedHyperlink->removeActionListener( &maActionListeners );
    }
    maActionListeners.removeInterface( l );
}

//  ----------------------------------------------------
//  class UnoControlFixedTextModel
//  ----------------------------------------------------
UnoControlFixedTextModel::UnoControlFixedTextModel( const Reference< XComponentContext >& rxContext )
    :UnoControlModel( rxContext )
{
    UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXFixedText );
}

OUString UnoControlFixedTextModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlFixedTextModel );
}

uno::Any UnoControlFixedTextModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= OUString::createFromAscii( szServiceName_UnoControlFixedText );
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
    :UnoControlBase()
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

OUString UnoFixedTextControl::GetComponentServiceName()
{
    return OUString("fixedtext");
}

// uno::XInterface
uno::Any UnoFixedTextControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< awt::XFixedText* >(this)),
                                        (static_cast< awt::XLayoutConstrains* >(this)) );
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

void UnoFixedTextControl::setText( const OUString& Text ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Text;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

OUString UnoFixedTextControl::getText() throw(uno::RuntimeException)
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
UnoControlGroupBoxModel::UnoControlGroupBoxModel( const Reference< XComponentContext >& rxContext )
    :UnoControlModel( rxContext )
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_ENABLEVISIBLE );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_WRITING_MODE );
    ImplRegisterProperty( BASEPROPERTY_CONTEXT_WRITING_MODE );
}

OUString UnoControlGroupBoxModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlGroupBoxModel );
}

uno::Any UnoControlGroupBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= OUString::createFromAscii( szServiceName_UnoControlGroupBox );
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
    :UnoControlBase()
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 100;
}

OUString UnoGroupBoxControl::GetComponentServiceName()
{
    return OUString("groupbox");
}

sal_Bool UnoGroupBoxControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}

// =====================================================================================================================
// = UnoControlListBoxModel_Data
// =====================================================================================================================
struct ListItem
{
    OUString ItemText;
    OUString ItemImageURL;
    Any             ItemData;

    ListItem()
        :ItemText()
        ,ItemImageURL()
        ,ItemData()
    {
    }

    ListItem( const OUString& i_rItemText )
        :ItemText( i_rItemText )
        ,ItemImageURL()
        ,ItemData()
    {
    }
};

typedef beans::Pair< OUString, OUString > UnoListItem;

struct StripItemData : public ::std::unary_function< ListItem, UnoListItem >
{
    UnoListItem operator()( const ListItem& i_rItem )
    {
        return UnoListItem( i_rItem.ItemText, i_rItem.ItemImageURL );
    }
};

struct UnoControlListBoxModel_Data
{
    UnoControlListBoxModel_Data( UnoControlListBoxModel& i_rAntiImpl )
        :m_bSettingLegacyProperty( false )
        ,m_rAntiImpl( i_rAntiImpl )
        ,m_aListItems()
    {
    }

    sal_Int32 getItemCount() const { return sal_Int32( m_aListItems.size() ); }

    const ListItem& getItem( const sal_Int32 i_nIndex ) const
    {
        if ( ( i_nIndex < 0 ) || ( i_nIndex >= sal_Int32( m_aListItems.size() ) ) )
            throw IndexOutOfBoundsException( OUString(), m_rAntiImpl );
        return m_aListItems[ i_nIndex ];
    }

    ListItem& getItem( const sal_Int32 i_nIndex )
    {
        return const_cast< ListItem& >( static_cast< const UnoControlListBoxModel_Data* >( this )->getItem( i_nIndex ) );
    }

    ListItem& insertItem( const sal_Int32 i_nIndex )
    {
        if ( ( i_nIndex < 0 ) || ( i_nIndex > sal_Int32( m_aListItems.size() ) ) )
            throw IndexOutOfBoundsException( OUString(), m_rAntiImpl );
        return *m_aListItems.insert( m_aListItems.begin() + i_nIndex, ListItem() );
    }

    Sequence< UnoListItem > getAllItems() const
    {
        Sequence< UnoListItem > aItems( sal_Int32( m_aListItems.size() ) );
        ::std::transform( m_aListItems.begin(), m_aListItems.end(), aItems.getArray(), StripItemData() );
        return aItems;
    }

    void copyItems( const UnoControlListBoxModel_Data& i_copySource )
    {
        m_aListItems = i_copySource.m_aListItems;
    }

    void    setAllItems( const ::std::vector< ListItem >& i_rItems )
    {
        m_aListItems = i_rItems;
    }

    void    removeItem( const sal_Int32 i_nIndex )
    {
        if ( ( i_nIndex < 0 ) || ( i_nIndex >= sal_Int32( m_aListItems.size() ) ) )
            throw IndexOutOfBoundsException( OUString(), m_rAntiImpl );
        m_aListItems.erase( m_aListItems.begin() + i_nIndex );
    }

    void removeAllItems()
    {
        ::std::vector< ListItem > aEmpty;
        m_aListItems.swap( aEmpty );
    }

public:
    bool                        m_bSettingLegacyProperty;

private:
    UnoControlListBoxModel&     m_rAntiImpl;
    ::std::vector< ListItem >   m_aListItems;
};

// =====================================================================================================================
// = UnoControlListBoxModel
// =====================================================================================================================
// ---------------------------------------------------------------------------------------------------------------------
UnoControlListBoxModel::UnoControlListBoxModel( const Reference< XComponentContext >& rxContext, ConstructorMode const i_mode )
    :UnoControlListBoxModel_Base( rxContext )
    ,m_pData( new UnoControlListBoxModel_Data( *this ) )
    ,m_aItemListListeners( GetMutex() )
{
    if ( i_mode == ConstructDefault )
    {
        UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXListBox );
    }
}
// ---------------------------------------------------------------------------------------------------------------------
UnoControlListBoxModel::UnoControlListBoxModel( const UnoControlListBoxModel& i_rSource )
    :UnoControlListBoxModel_Base( i_rSource )
    ,m_pData( new UnoControlListBoxModel_Data( *this ) )
    ,m_aItemListListeners( GetMutex() )
{
    m_pData->copyItems( *i_rSource.m_pData );
}
UnoControlListBoxModel::~UnoControlListBoxModel()
{
}
IMPL_SERVICEINFO_DERIVED( UnoControlListBoxModel, UnoControlModel, szServiceName2_UnoControlListBoxModel )
// ---------------------------------------------------------------------------------------------------------------------
OUString UnoControlListBoxModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlListBoxModel );
}

// ---------------------------------------------------------------------------------------------------------------------
uno::Any UnoControlListBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= OUString::createFromAscii( szServiceName_UnoControlListBox );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

// ---------------------------------------------------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------------------------------------------------
// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlListBoxModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

// ---------------------------------------------------------------------------------------------------------------------
namespace
{
    struct CreateListItem : public ::std::unary_function< OUString, ListItem >
    {
        ListItem operator()( const OUString& i_rItemText )
        {
            return ListItem( i_rItemText );
        }
    };
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoControlListBoxModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const uno::Any& rValue ) throw (uno::Exception)
{
    UnoControlModel::setFastPropertyValue_NoBroadcast( nHandle, rValue );

    if ( nHandle == BASEPROPERTY_STRINGITEMLIST )
    {
        // reset selection
        uno::Sequence<sal_Int16> aSeq;
        uno::Any aAny;
        aAny <<= aSeq;
        setDependentFastPropertyValue( BASEPROPERTY_SELECTEDITEMS, aAny );

        if ( !m_pData->m_bSettingLegacyProperty )
        {
            // synchronize the legacy StringItemList property with our list items
            Sequence< OUString > aStringItemList;
            Any aPropValue;
            getFastPropertyValue( aPropValue, BASEPROPERTY_STRINGITEMLIST );
            OSL_VERIFY( aPropValue >>= aStringItemList );

            ::std::vector< ListItem > aItems( aStringItemList.getLength() );
            ::std::transform(
                aStringItemList.getConstArray(),
                aStringItemList.getConstArray() + aStringItemList.getLength(),
                aItems.begin(),
                CreateListItem()
            );
            m_pData->setAllItems( aItems );

            // since an XItemListListener does not have a "all items modified" or some such method,
            // we simulate this by notifying removal of all items, followed by insertion of all new
            // items
            lang::EventObject aEvent;
            aEvent.Source = *this;
            m_aItemListListeners.notifyEach( &XItemListListener::itemListChanged, aEvent );
            // TODO: OPropertySetHelper calls into this method with the mutex locked ...
            // which is wrong for the above notifications ...
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void UnoControlListBoxModel::ImplNormalizePropertySequence( const sal_Int32 _nCount, sal_Int32* _pHandles,
    uno::Any* _pValues, sal_Int32* _pValidHandles ) const SAL_THROW(())
{
    // dependencies we know:
    // BASEPROPERTY_STRINGITEMLIST->BASEPROPERTY_SELECTEDITEMS
    ImplEnsureHandleOrder( _nCount, _pHandles, _pValues, BASEPROPERTY_STRINGITEMLIST, BASEPROPERTY_SELECTEDITEMS );

    UnoControlModel::ImplNormalizePropertySequence( _nCount, _pHandles, _pValues, _pValidHandles );
}

// ---------------------------------------------------------------------------------------------------------------------
::sal_Int32 SAL_CALL UnoControlListBoxModel::getItemCount() throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );
    return m_pData->getItemCount();
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoControlListBoxModel::insertItem( ::sal_Int32 i_nPosition, const OUString& i_rItemText, const OUString& i_rItemImageURL ) throw (IndexOutOfBoundsException, RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    // SYNCHRONIZED ----->
    ListItem& rItem( m_pData->insertItem( i_nPosition ) );
    rItem.ItemText = i_rItemText;
    rItem.ItemImageURL = i_rItemImageURL;

    impl_handleInsert( i_nPosition, i_rItemText, i_rItemImageURL, aGuard );
    // <----- SYNCHRONIZED
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoControlListBoxModel::insertItemText( ::sal_Int32 i_nPosition, const OUString& i_rItemText ) throw (IndexOutOfBoundsException, RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    // SYNCHRONIZED ----->
    ListItem& rItem( m_pData->insertItem( i_nPosition ) );
    rItem.ItemText = i_rItemText;

    impl_handleInsert( i_nPosition, i_rItemText, ::boost::optional< OUString >(), aGuard );
    // <----- SYNCHRONIZED
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoControlListBoxModel::insertItemImage( ::sal_Int32 i_nPosition, const OUString& i_rItemImageURL ) throw (IndexOutOfBoundsException, RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    // SYNCHRONIZED ----->
    ListItem& rItem( m_pData->insertItem( i_nPosition ) );
    rItem.ItemImageURL = i_rItemImageURL;

    impl_handleInsert( i_nPosition, ::boost::optional< OUString >(), i_rItemImageURL, aGuard );
    // <----- SYNCHRONIZED
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoControlListBoxModel::removeItem( ::sal_Int32 i_nPosition ) throw (IndexOutOfBoundsException, RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    // SYNCHRONIZED ----->
    m_pData->removeItem( i_nPosition );

    impl_handleRemove( i_nPosition, aGuard );
    // <----- SYNCHRONIZED
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoControlListBoxModel::removeAllItems(  ) throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    // SYNCHRONIZED ----->
    m_pData->removeAllItems();

    impl_handleRemove( -1, aGuard );
    // <----- SYNCHRONIZED
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoControlListBoxModel::setItemText( ::sal_Int32 i_nPosition, const OUString& i_rItemText ) throw (IndexOutOfBoundsException, RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    // SYNCHRONIZED ----->
    ListItem& rItem( m_pData->getItem( i_nPosition ) );
    rItem.ItemText = i_rItemText;

    impl_handleModify( i_nPosition, i_rItemText, ::boost::optional< OUString >(), aGuard );
    // <----- SYNCHRONIZED
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoControlListBoxModel::setItemImage( ::sal_Int32 i_nPosition, const OUString& i_rItemImageURL ) throw (IndexOutOfBoundsException, RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    // SYNCHRONIZED ----->
    ListItem& rItem( m_pData->getItem( i_nPosition ) );
    rItem.ItemImageURL = i_rItemImageURL;

    impl_handleModify( i_nPosition, ::boost::optional< OUString >(), i_rItemImageURL, aGuard );
    // <----- SYNCHRONIZED
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoControlListBoxModel::setItemTextAndImage( ::sal_Int32 i_nPosition, const OUString& i_rItemText, const OUString& i_rItemImageURL ) throw (IndexOutOfBoundsException, RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    // SYNCHRONIZED ----->
    ListItem& rItem( m_pData->getItem( i_nPosition ) );
    rItem.ItemText = i_rItemText;
    rItem.ItemImageURL = i_rItemImageURL;

    impl_handleModify( i_nPosition, i_rItemText, i_rItemImageURL, aGuard );
    // <----- SYNCHRONIZED
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoControlListBoxModel::setItemData( ::sal_Int32 i_nPosition, const Any& i_rDataValue ) throw (IndexOutOfBoundsException, RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    ListItem& rItem( m_pData->getItem( i_nPosition ) );
    rItem.ItemData = i_rDataValue;
}

// ---------------------------------------------------------------------------------------------------------------------
OUString SAL_CALL UnoControlListBoxModel::getItemText( ::sal_Int32 i_nPosition ) throw (IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );
    const ListItem& rItem( m_pData->getItem( i_nPosition ) );
    return rItem.ItemText;
}

// ---------------------------------------------------------------------------------------------------------------------
OUString SAL_CALL UnoControlListBoxModel::getItemImage( ::sal_Int32 i_nPosition ) throw (IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );
    const ListItem& rItem( m_pData->getItem( i_nPosition ) );
    return rItem.ItemImageURL;
}

// ---------------------------------------------------------------------------------------------------------------------
beans::Pair< OUString, OUString > SAL_CALL UnoControlListBoxModel::getItemTextAndImage( ::sal_Int32 i_nPosition ) throw (IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );
    const ListItem& rItem( m_pData->getItem( i_nPosition ) );
    return beans::Pair< OUString, OUString >( rItem.ItemText, rItem.ItemImageURL );
}

// ---------------------------------------------------------------------------------------------------------------------
Any SAL_CALL UnoControlListBoxModel::getItemData( ::sal_Int32 i_nPosition ) throw (IndexOutOfBoundsException, RuntimeException)
{
    ::osl::ClearableMutexGuard aGuard( GetMutex() );
    const ListItem& rItem( m_pData->getItem( i_nPosition ) );
    return rItem.ItemData;
}

// ---------------------------------------------------------------------------------------------------------------------
Sequence< beans::Pair< OUString, OUString > > SAL_CALL UnoControlListBoxModel::getAllItems(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( GetMutex() );
    return m_pData->getAllItems();
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoControlListBoxModel::addItemListListener( const uno::Reference< awt::XItemListListener >& i_Listener ) throw (uno::RuntimeException)
{
    if ( i_Listener.is() )
        m_aItemListListeners.addInterface( i_Listener );
}

// ---------------------------------------------------------------------------------------------------------------------
void SAL_CALL UnoControlListBoxModel::removeItemListListener( const uno::Reference< awt::XItemListListener >& i_Listener ) throw (uno::RuntimeException)
{
    if ( i_Listener.is() )
        m_aItemListListeners.removeInterface( i_Listener );
}

// ---------------------------------------------------------------------------------------------------------------------
void UnoControlListBoxModel::impl_getStringItemList( ::std::vector< OUString >& o_rStringItems ) const
{
    Sequence< OUString > aStringItemList;
    Any aPropValue;
    getFastPropertyValue( aPropValue, BASEPROPERTY_STRINGITEMLIST );
    OSL_VERIFY( aPropValue >>= aStringItemList );

    o_rStringItems.resize( size_t( aStringItemList.getLength() ) );
    ::std::copy(
        aStringItemList.getConstArray(),
        aStringItemList.getConstArray() + aStringItemList.getLength(),
        o_rStringItems.begin()
    );
}

// ---------------------------------------------------------------------------------------------------------------------
void UnoControlListBoxModel::impl_setStringItemList_nolck( const ::std::vector< OUString >& i_rStringItems )
{
    Sequence< OUString > aStringItems( i_rStringItems.size() );
    ::std::copy(
        i_rStringItems.begin(),
        i_rStringItems.end(),
        aStringItems.getArray()
    );
    m_pData->m_bSettingLegacyProperty = true;
    try
    {
        setFastPropertyValue( BASEPROPERTY_STRINGITEMLIST, uno::makeAny( aStringItems ) );
    }
    catch( const Exception& )
    {
        m_pData->m_bSettingLegacyProperty = false;
        throw;
    }
    m_pData->m_bSettingLegacyProperty = false;
}

// ---------------------------------------------------------------------------------------------------------------------
void UnoControlListBoxModel::impl_handleInsert( const sal_Int32 i_nItemPosition, const ::boost::optional< OUString >& i_rItemText,
        const ::boost::optional< OUString >& i_rItemImageURL, ::osl::ClearableMutexGuard& i_rClearBeforeNotify )
{
    // SYNCHRONIZED ----->
    // sync with legacy StringItemList property
    ::std::vector< OUString > aStringItems;
    impl_getStringItemList( aStringItems );
    OSL_ENSURE( size_t( i_nItemPosition ) <= aStringItems.size(), "UnoControlListBoxModel::impl_handleInsert" );
    if ( size_t( i_nItemPosition ) <= aStringItems.size() )
    {
        const OUString sItemText( !!i_rItemText ? *i_rItemText : OUString() );
        aStringItems.insert( aStringItems.begin() + i_nItemPosition, sItemText );
    }

    i_rClearBeforeNotify.clear();
    // <----- SYNCHRONIZED
    impl_setStringItemList_nolck( aStringItems );

    // notify ItemListListeners
    impl_notifyItemListEvent_nolck( i_nItemPosition, i_rItemText, i_rItemImageURL, &XItemListListener::listItemInserted );
}

// ---------------------------------------------------------------------------------------------------------------------
void UnoControlListBoxModel::impl_handleRemove( const sal_Int32 i_nItemPosition, ::osl::ClearableMutexGuard& i_rClearBeforeNotify )
{
    // SYNCHRONIZED ----->
    const bool bAllItems = ( i_nItemPosition < 0 );
    // sync with legacy StringItemList property
    ::std::vector< OUString > aStringItems;
    impl_getStringItemList( aStringItems );
    if ( !bAllItems )
    {
        OSL_ENSURE( size_t( i_nItemPosition ) < aStringItems.size(), "UnoControlListBoxModel::impl_handleRemove" );
        if ( size_t( i_nItemPosition ) < aStringItems.size() )
        {
            aStringItems.erase( aStringItems.begin() + i_nItemPosition );
        }
    }
    else
    {
        aStringItems.resize(0);
    }

    i_rClearBeforeNotify.clear();
    // <----- SYNCHRONIZED
    impl_setStringItemList_nolck( aStringItems );

    // notify ItemListListeners
    if ( bAllItems )
    {
        EventObject aEvent( *this );
        m_aItemListListeners.notifyEach( &XItemListListener::allItemsRemoved, aEvent );
    }
    else
    {
        impl_notifyItemListEvent_nolck( i_nItemPosition, ::boost::optional< OUString >(), ::boost::optional< OUString >(),
            &XItemListListener::listItemRemoved );
    }
}

// ---------------------------------------------------------------------------------------------------------------------
void UnoControlListBoxModel::impl_handleModify( const sal_Int32 i_nItemPosition, const ::boost::optional< OUString >& i_rItemText,
        const ::boost::optional< OUString >& i_rItemImageURL, ::osl::ClearableMutexGuard& i_rClearBeforeNotify )
{
    // SYNCHRONIZED ----->
    if ( !!i_rItemText )
    {
        // sync with legacy StringItemList property
        ::std::vector< OUString > aStringItems;
        impl_getStringItemList( aStringItems );
        OSL_ENSURE( size_t( i_nItemPosition ) < aStringItems.size(), "UnoControlListBoxModel::impl_handleModify" );
        if ( size_t( i_nItemPosition ) < aStringItems.size() )
        {
            aStringItems[ i_nItemPosition] = *i_rItemText;
        }

        i_rClearBeforeNotify.clear();
        // <----- SYNCHRONIZED
        impl_setStringItemList_nolck( aStringItems );
    }
    else
    {
        i_rClearBeforeNotify.clear();
        // <----- SYNCHRONIZED
    }

    // notify ItemListListeners
    impl_notifyItemListEvent_nolck( i_nItemPosition, i_rItemText, i_rItemImageURL, &XItemListListener::listItemModified );
}

// ---------------------------------------------------------------------------------------------------------------------
void UnoControlListBoxModel::impl_notifyItemListEvent_nolck( const sal_Int32 i_nItemPosition, const ::boost::optional< OUString >& i_rItemText,
    const ::boost::optional< OUString >& i_rItemImageURL,
    void ( SAL_CALL XItemListListener::*NotificationMethod )( const ItemListEvent& ) )
{
    ItemListEvent aEvent;
    aEvent.Source = *this;
    aEvent.ItemPosition = i_nItemPosition;
    if ( !!i_rItemText )
    {
        aEvent.ItemText.IsPresent = sal_True;
        aEvent.ItemText.Value = *i_rItemText;
    }
    if ( !!i_rItemImageURL )
    {
        aEvent.ItemImageURL.IsPresent = sal_True;
        aEvent.ItemImageURL.Value = *i_rItemImageURL;
    }

    m_aItemListListeners.notifyEach( NotificationMethod, aEvent );
}

//  ----------------------------------------------------
//  class UnoListBoxControl
//  ----------------------------------------------------
UnoListBoxControl::UnoListBoxControl()
    :UnoListBoxControl_Base()
    ,maActionListeners( *this )
    ,maItemListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

OUString UnoListBoxControl::GetComponentServiceName()
{
    return OUString("listbox");
}
IMPL_SERVICEINFO_DERIVED( UnoListBoxControl, UnoControlBase, szServiceName2_UnoControlListBox )

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

void UnoListBoxControl::updateFromModel()
{
    UnoControlBase::updateFromModel();

    Reference< XItemListListener > xItemListListener( getPeer(), UNO_QUERY );
    ENSURE_OR_RETURN_VOID( xItemListListener.is(), "UnoListBoxControl::updateFromModel: a peer which is no ItemListListener?!" );

    EventObject aEvent( getModel() );
    xItemListListener->itemListChanged( aEvent );

    // notify the change of the SelectedItems property, again. While our base class, in updateFromModel,
    // already did this, our peer(s) can only legitimately set the selection after they have the string
    // item list, which we just notified with the itemListChanged call.
    const OUString sSelectedItemsPropName( GetPropertyName( BASEPROPERTY_SELECTEDITEMS ) );
    ImplSetPeerProperty( sSelectedItemsPropName, ImplGetPropertyValue( sSelectedItemsPropName ) );
}

void UnoListBoxControl::ImplSetPeerProperty( const OUString& rPropName, const uno::Any& rVal )
{
    if ( rPropName == GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) )
        // do not forward this to our peer. We are a XItemListListener at our model, and changes in the string item
        // list (which is a legacy property) will, later, arrive as changes in the ItemList. Those latter changes
        // will be forwarded to the peer, which will update itself accordingly.
        return;

    UnoControl::ImplSetPeerProperty( rPropName, rVal );
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

void UnoListBoxControl::addItem( const OUString& aItem, sal_Int16 nPos ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString> aSeq( 1 );
    aSeq.getArray()[0] = aItem;
    addItems( aSeq, nPos );
}

void UnoListBoxControl::addItems( const uno::Sequence< OUString>& aItems, sal_Int16 nPos ) throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nNewItems = (sal_uInt16)aItems.getLength();
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    sal_uInt16 nNewLen = nOldLen + nNewItems;

    uno::Sequence< OUString> aNewSeq( nNewLen );
    OUString* pNewData = aNewSeq.getArray();
    OUString* pOldData = aSeq.getArray();

    if ( ( nPos < 0 ) || ( nPos > nOldLen ) )
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
    uno::Sequence< OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    if ( nOldLen && ( nPos < nOldLen ) )
    {
        if ( nCount > ( nOldLen-nPos ) )
            nCount = nOldLen-nPos;

        sal_uInt16 nNewLen = nOldLen - nCount;

        uno::Sequence< OUString> aNewSeq( nNewLen );
        OUString* pNewData = aNewSeq.getArray();
        OUString* pOldData = aSeq.getArray();

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
    uno::Sequence< OUString> aSeq;
    aVal >>= aSeq;
    return (sal_Int16)aSeq.getLength();
}

OUString UnoListBoxControl::getItem( sal_Int16 nPos ) throw(uno::RuntimeException)
{
    OUString aItem;
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< OUString> aSeq;
    aVal >>= aSeq;
    if ( nPos < aSeq.getLength() )
        aItem = aSeq.getConstArray()[nPos];
    return aItem;
}

uno::Sequence< OUString> UnoListBoxControl::getItems() throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< OUString> aSeq;
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

OUString UnoListBoxControl::getSelectedItem() throw(uno::RuntimeException)
{
    OUString aItem;
    if ( getPeer().is() )
    {
        uno::Reference < awt::XListBox >  xListBox( getPeer(), uno::UNO_QUERY );
        aItem = xListBox->getSelectedItem();
    }
    return aItem;
}

uno::Sequence< OUString> UnoListBoxControl::getSelectedItems() throw(uno::RuntimeException)
{
    uno::Sequence< OUString> aSeq;
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

void UnoListBoxControl::selectItem( const OUString& aItem, sal_Bool bSelect ) throw(uno::RuntimeException)
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
    {
        try
        {
            maItemListeners.itemStateChanged( rEvent );
        }
        catch( const Exception& e )
        {
#if OSL_DEBUG_LEVEL == 0
            (void) e; // suppress warning
#else
            OString sMessage( "UnoListBoxControl::itemStateChanged: caught an exception:\n" );
            sMessage += OString( e.Message.getStr(), e.Message.getLength(), RTL_TEXTENCODING_ASCII_US );
            OSL_FAIL( sMessage.getStr() );
#endif
        }
    }
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

sal_Bool SAL_CALL UnoListBoxControl::setModel( const uno::Reference< awt::XControlModel >& i_rModel ) throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( GetMutex() );

    const Reference< XItemList > xOldItems( getModel(), UNO_QUERY );
    OSL_ENSURE( xOldItems.is() || !getModel().is(), "UnoListBoxControl::setModel: illegal old model!" );
    const Reference< XItemList > xNewItems( i_rModel, UNO_QUERY );
    OSL_ENSURE( xNewItems.is() || !i_rModel.is(), "UnoListBoxControl::setModel: illegal new model!" );

    if ( !UnoListBoxControl_Base::setModel( i_rModel ) )
        return sal_False;

    if ( xOldItems.is() )
        xOldItems->removeItemListListener( this );
    if ( xNewItems.is() )
        xNewItems->addItemListListener( this );

    return sal_True;
}

void SAL_CALL UnoListBoxControl::listItemInserted( const awt::ItemListEvent& i_rEvent ) throw (uno::RuntimeException)
{
    const Reference< XItemListListener > xPeerListener( getPeer(), UNO_QUERY );
    OSL_ENSURE( xPeerListener.is() || !getPeer().is(), "UnoListBoxControl::listItemInserted: invalid peer!" );
    if ( xPeerListener.is() )
        xPeerListener->listItemInserted( i_rEvent );
}

void SAL_CALL UnoListBoxControl::listItemRemoved( const awt::ItemListEvent& i_rEvent ) throw (uno::RuntimeException)
{
    const Reference< XItemListListener > xPeerListener( getPeer(), UNO_QUERY );
    OSL_ENSURE( xPeerListener.is() || !getPeer().is(), "UnoListBoxControl::listItemRemoved: invalid peer!" );
    if ( xPeerListener.is() )
        xPeerListener->listItemRemoved( i_rEvent );
}

void SAL_CALL UnoListBoxControl::listItemModified( const awt::ItemListEvent& i_rEvent ) throw (uno::RuntimeException)
{
    const Reference< XItemListListener > xPeerListener( getPeer(), UNO_QUERY );
    OSL_ENSURE( xPeerListener.is() || !getPeer().is(), "UnoListBoxControl::listItemModified: invalid peer!" );
    if ( xPeerListener.is() )
        xPeerListener->listItemModified( i_rEvent );
}

void SAL_CALL UnoListBoxControl::allItemsRemoved( const lang::EventObject& i_rEvent ) throw (uno::RuntimeException)
{
    const Reference< XItemListListener > xPeerListener( getPeer(), UNO_QUERY );
    OSL_ENSURE( xPeerListener.is() || !getPeer().is(), "UnoListBoxControl::allItemsRemoved: invalid peer!" );
    if ( xPeerListener.is() )
        xPeerListener->allItemsRemoved( i_rEvent );
}

void SAL_CALL UnoListBoxControl::itemListChanged( const lang::EventObject& i_rEvent ) throw (uno::RuntimeException)
{
    const Reference< XItemListListener > xPeerListener( getPeer(), UNO_QUERY );
    OSL_ENSURE( xPeerListener.is() || !getPeer().is(), "UnoListBoxControl::itemListChanged: invalid peer!" );
    if ( xPeerListener.is() )
        xPeerListener->itemListChanged( i_rEvent );
}

//  ----------------------------------------------------
//  class UnoControlComboBoxModel
//  ----------------------------------------------------
UnoControlComboBoxModel::UnoControlComboBoxModel( const Reference< XComponentContext >& rxContext )
    :UnoControlListBoxModel( rxContext, ConstructWithoutProperties )
{
    UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXComboBox );
}

IMPL_SERVICEINFO_DERIVED( UnoControlComboBoxModel, UnoControlModel, szServiceName2_UnoControlComboBoxModel )

uno::Reference< beans::XPropertySetInfo > UnoControlComboBoxModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}
// ---------------------------------------------------------------------------------------------------------------------
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


OUString UnoControlComboBoxModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlComboBoxModel );
}
void SAL_CALL UnoControlComboBoxModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const uno::Any& rValue ) throw (uno::Exception)
{
    UnoControlModel::setFastPropertyValue_NoBroadcast( nHandle, rValue );

    if ( nHandle == BASEPROPERTY_STRINGITEMLIST && !m_pData->m_bSettingLegacyProperty)
    {
        // synchronize the legacy StringItemList property with our list items
        Sequence< OUString > aStringItemList;
        Any aPropValue;
        getFastPropertyValue( aPropValue, BASEPROPERTY_STRINGITEMLIST );
        OSL_VERIFY( aPropValue >>= aStringItemList );

        ::std::vector< ListItem > aItems( aStringItemList.getLength() );
        ::std::transform(
            aStringItemList.getConstArray(),
            aStringItemList.getConstArray() + aStringItemList.getLength(),
            aItems.begin(),
            CreateListItem()
        );
        m_pData->setAllItems( aItems );

        // since an XItemListListener does not have a "all items modified" or some such method,
        // we simulate this by notifying removal of all items, followed by insertion of all new
        // items
        lang::EventObject aEvent;
        aEvent.Source = *this;
        m_aItemListListeners.notifyEach( &XItemListListener::itemListChanged, aEvent );
        // TODO: OPropertySetHelper calls into this method with the mutex locked ...
        // which is wrong for the above notifications ...
    }
}

uno::Any UnoControlComboBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= OUString::createFromAscii( szServiceName_UnoControlComboBox );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

//  ----------------------------------------------------
//  class UnoComboBoxControl
//  ----------------------------------------------------
UnoComboBoxControl::UnoComboBoxControl()
    :UnoEditControl()
    ,maActionListeners( *this )
    ,maItemListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}
IMPL_SERVICEINFO_DERIVED( UnoComboBoxControl, UnoEditControl, szServiceName2_UnoControlComboBox )

OUString UnoComboBoxControl::GetComponentServiceName()
{
    return OUString("combobox");
}

void UnoComboBoxControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aEvt );
    maItemListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}
uno::Any UnoComboBoxControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< awt::XComboBox* >(this)) );
    if ( !aRet.hasValue() )
    {
        aRet = ::cppu::queryInterface( rType,
                                        (static_cast< awt::XItemListener* >(this)) );
        if ( !aRet.hasValue() )
        {
            aRet = ::cppu::queryInterface( rType,
                                            (static_cast< awt::XItemListListener* >(this)) );
        }
    }
    return (aRet.hasValue() ? aRet : UnoEditControl::queryAggregation( rType ));
}
// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoComboBoxControl )
    getCppuType( ( uno::Reference< awt::XComboBox>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XItemListener>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XItemListListener>* ) NULL ),
    UnoEditControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoComboBoxControl::updateFromModel()
{
    UnoEditControl::updateFromModel();

    Reference< XItemListListener > xItemListListener( getPeer(), UNO_QUERY );
    ENSURE_OR_RETURN_VOID( xItemListListener.is(), "UnoComboBoxControl::updateFromModel: a peer which is no ItemListListener?!" );

    EventObject aEvent( getModel() );
    xItemListListener->itemListChanged( aEvent );
}
void UnoComboBoxControl::ImplSetPeerProperty( const OUString& rPropName, const uno::Any& rVal )
{
    if ( rPropName == GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) )
        // do not forward this to our peer. We are a XItemListListener at our model, and changes in the string item
        // list (which is a legacy property) will, later, arrive as changes in the ItemList. Those latter changes
        // will be forwarded to the peer, which will update itself accordingly.
        return;

    UnoEditControl::ImplSetPeerProperty( rPropName, rVal );
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
        // This call is prettier than creating a Ref and calling query
        uno::Reference < awt::XComboBox >  xComboBox( getPeer(), uno::UNO_QUERY );
        xComboBox->removeItemListener( &maItemListeners );
    }
    maItemListeners.removeInterface( l );
}
void UnoComboBoxControl::itemStateChanged( const awt::ItemEvent& rEvent ) throw(uno::RuntimeException)
{
    if ( maItemListeners.getLength() )
    {
        try
        {
            maItemListeners.itemStateChanged( rEvent );
        }
        catch( const Exception& e )
        {
#if OSL_DEBUG_LEVEL == 0
            (void) e; // suppress warning
#else
            OString sMessage( "UnoComboBoxControl::itemStateChanged: caught an exception:\n" );
            sMessage += OString( e.Message.getStr(), e.Message.getLength(), RTL_TEXTENCODING_ASCII_US );
            OSL_FAIL( sMessage.getStr() );
#endif
        }
    }
}
sal_Bool SAL_CALL UnoComboBoxControl::setModel( const uno::Reference< awt::XControlModel >& i_rModel ) throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( GetMutex() );

    const Reference< XItemList > xOldItems( getModel(), UNO_QUERY );
    OSL_ENSURE( xOldItems.is() || !getModel().is(), "UnoComboBoxControl::setModel: illegal old model!" );
    const Reference< XItemList > xNewItems( i_rModel, UNO_QUERY );
    OSL_ENSURE( xNewItems.is() || !i_rModel.is(), "UnoComboBoxControl::setModel: illegal new model!" );

    if ( !UnoEditControl::setModel( i_rModel ) )
        return sal_False;

    if ( xOldItems.is() )
        xOldItems->removeItemListListener( this );
    if ( xNewItems.is() )
        xNewItems->addItemListListener( this );

    return sal_True;
}

void SAL_CALL UnoComboBoxControl::listItemInserted( const awt::ItemListEvent& i_rEvent ) throw (uno::RuntimeException)
{
    const Reference< XItemListListener > xPeerListener( getPeer(), UNO_QUERY );
    OSL_ENSURE( xPeerListener.is() || !getPeer().is(), "UnoComboBoxControl::listItemInserted: invalid peer!" );
    if ( xPeerListener.is() )
        xPeerListener->listItemInserted( i_rEvent );
}

void SAL_CALL UnoComboBoxControl::listItemRemoved( const awt::ItemListEvent& i_rEvent ) throw (uno::RuntimeException)
{
    const Reference< XItemListListener > xPeerListener( getPeer(), UNO_QUERY );
    OSL_ENSURE( xPeerListener.is() || !getPeer().is(), "UnoComboBoxControl::listItemRemoved: invalid peer!" );
    if ( xPeerListener.is() )
        xPeerListener->listItemRemoved( i_rEvent );
}

void SAL_CALL UnoComboBoxControl::listItemModified( const awt::ItemListEvent& i_rEvent ) throw (uno::RuntimeException)
{
    const Reference< XItemListListener > xPeerListener( getPeer(), UNO_QUERY );
    OSL_ENSURE( xPeerListener.is() || !getPeer().is(), "UnoComboBoxControl::listItemModified: invalid peer!" );
    if ( xPeerListener.is() )
        xPeerListener->listItemModified( i_rEvent );
}

void SAL_CALL UnoComboBoxControl::allItemsRemoved( const lang::EventObject& i_rEvent ) throw (uno::RuntimeException)
{
    const Reference< XItemListListener > xPeerListener( getPeer(), UNO_QUERY );
    OSL_ENSURE( xPeerListener.is() || !getPeer().is(), "UnoComboBoxControl::allItemsRemoved: invalid peer!" );
    if ( xPeerListener.is() )
        xPeerListener->allItemsRemoved( i_rEvent );
}

void SAL_CALL UnoComboBoxControl::itemListChanged( const lang::EventObject& i_rEvent ) throw (uno::RuntimeException)
{
    const Reference< XItemListListener > xPeerListener( getPeer(), UNO_QUERY );
    OSL_ENSURE( xPeerListener.is() || !getPeer().is(), "UnoComboBoxControl::itemListChanged: invalid peer!" );
    if ( xPeerListener.is() )
        xPeerListener->itemListChanged( i_rEvent );
}

void UnoComboBoxControl::addItem( const OUString& aItem, sal_Int16 nPos ) throw(uno::RuntimeException)
{
    uno::Sequence< OUString> aSeq( 1 );
    aSeq.getArray()[0] = aItem;
    addItems( aSeq, nPos );
}

void UnoComboBoxControl::addItems( const uno::Sequence< OUString>& aItems, sal_Int16 nPos ) throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nNewItems = (sal_uInt16)aItems.getLength();
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    sal_uInt16 nNewLen = nOldLen + nNewItems;

    uno::Sequence< OUString> aNewSeq( nNewLen );
    OUString* pNewData = aNewSeq.getArray();
    const OUString* pOldData = aSeq.getConstArray();

    if ( ( nPos < 0 ) || ( nPos > nOldLen ) )
        nPos = (sal_uInt16) nOldLen;

    sal_uInt16 n;
    // items before the insert position
    for ( n = 0; n < nPos; n++ )
        pNewData[n] = pOldData[n];

    // New items
    for ( n = 0; n < nNewItems; n++ )
        pNewData[nPos+n] = aItems.getConstArray()[n];

    // remainder of old items
    for ( n = nPos; n < nOldLen; n++ )
        pNewData[nNewItems+n] = pOldData[n];

    uno::Any aAny;
    aAny <<= aNewSeq;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ), aAny, sal_True );
}

void UnoComboBoxControl::removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    if ( nOldLen && ( nPos < nOldLen ) )
    {
        if ( nCount > ( nOldLen-nPos ) )
            nCount = nOldLen-nPos;

        sal_uInt16 nNewLen = nOldLen - nCount;

        uno::Sequence< OUString> aNewSeq( nNewLen );
        OUString* pNewData = aNewSeq.getArray();
        OUString* pOldData = aSeq.getArray();

        sal_uInt16 n;
        // items before the deletion position
        for ( n = 0; n < nPos; n++ )
            pNewData[n] = pOldData[n];

        // remainder of old items
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
    uno::Sequence< OUString> aSeq;
    aVal >>= aSeq;
    return (sal_Int16)aSeq.getLength();
}

OUString UnoComboBoxControl::getItem( sal_Int16 nPos ) throw(uno::RuntimeException)
{
    OUString aItem;
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< OUString> aSeq;
    aVal >>= aSeq;
    if ( nPos < aSeq.getLength() )
        aItem = aSeq.getConstArray()[nPos];
    return aItem;
}

uno::Sequence< OUString> UnoComboBoxControl::getItems() throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< OUString> aSeq;
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
UnoSpinFieldControl::UnoSpinFieldControl()
    :UnoEditControl()
    ,maSpinListeners( *this )
{
    mbRepeat = sal_False;
}

// uno::XInterface
uno::Any UnoSpinFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< awt::XSpinField* >(this)) );
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
UnoControlDateFieldModel::UnoControlDateFieldModel( const Reference< XComponentContext >& rxContext )
    :UnoControlModel( rxContext )
{
    UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXDateField );
}

OUString UnoControlDateFieldModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlDateFieldModel );
}

uno::Any UnoControlDateFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= OUString::createFromAscii( szServiceName_UnoControlDateField );
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
    :UnoSpinFieldControl()
{
    mnFirst = Date( 1, 1, 1900 ).GetDate();
    mnLast = Date( 31, 12, 2200 ).GetDate();
    mbLongFormat = 2;
}

OUString UnoDateFieldControl::GetComponentServiceName()
{
    return OUString("datefield");
}

// uno::XInterface
uno::Any UnoDateFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< awt::XDateField* >(this)) );
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
    uno::Reference< awt::XVclWindowPeer > xPeer( getPeer(), uno::UNO_QUERY );

    // also change the text property (#i25106#)
    if ( xPeer.is() )
    {
        OUString sTextPropertyName = GetPropertyName( BASEPROPERTY_TEXT );
        ImplSetPropertyValue( sTextPropertyName, xPeer->getProperty( sTextPropertyName ), sal_False );
    }

    // re-calc the Date property
    uno::Reference < awt::XDateField > xField( getPeer(), uno::UNO_QUERY );
    uno::Any aValue;
    if ( xField->isEmpty() )
    {
        // the field says it's empty
        sal_Bool bEnforceFormat = sal_True;
        if ( xPeer.is() )
            xPeer->getProperty( GetPropertyName( BASEPROPERTY_ENFORCE_FORMAT ) ) >>= bEnforceFormat;
        if ( !bEnforceFormat )
        {
            // and it also says that it is currently accepting invalid inputs, without
            // forcing it to a valid date
            uno::Reference< awt::XTextComponent > xText( xPeer, uno::UNO_QUERY );
            if ( xText.is() && xText->getText().getLength() )
                // and in real, the text of the peer is *not* empty
                // -> simulate an invalid date, which is different from "no date"
                aValue <<= util::Date();
        }
    }
    else
        aValue <<= xField->getDate();

    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_DATE ), aValue, sal_False );

    // multiplex the event
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
UnoControlTimeFieldModel::UnoControlTimeFieldModel( const Reference< XComponentContext >& rxContext )
    :UnoControlModel( rxContext )
{
    UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXTimeField );
}

OUString UnoControlTimeFieldModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlTimeFieldModel );
}

uno::Any UnoControlTimeFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= OUString::createFromAscii( szServiceName_UnoControlTimeField );
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
    :UnoSpinFieldControl()
{
    mnFirst = Time( 0, 0 ).GetTime();
    mnLast = Time( 23, 59, 59, 99 ).GetTime();
}

OUString UnoTimeFieldControl::GetComponentServiceName()
{
    return OUString("timefield");
}

// uno::XInterface
uno::Any UnoTimeFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< awt::XTimeField* >(this)) );
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
    // also change the text property (#i25106#)
    uno::Reference< awt::XVclWindowPeer > xPeer( getPeer(), uno::UNO_QUERY );
    OUString sTextPropertyName = GetPropertyName( BASEPROPERTY_TEXT );
    ImplSetPropertyValue( sTextPropertyName, xPeer->getProperty( sTextPropertyName ), sal_False );

    // re-calc the Time property
    uno::Reference < awt::XTimeField >  xField( getPeer(), uno::UNO_QUERY );
    uno::Any aValue;
    if ( !xField->isEmpty() )
        aValue <<= xField->getTime();
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TIME ), aValue, sal_False );

    // multiplex the event
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
UnoControlNumericFieldModel::UnoControlNumericFieldModel( const Reference< XComponentContext >& rxContext )
    :UnoControlModel( rxContext )
{
    UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXNumericField );
}

OUString UnoControlNumericFieldModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlNumericFieldModel );
}

uno::Any UnoControlNumericFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= OUString::createFromAscii( szServiceName_UnoControlNumericField );
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
    :UnoSpinFieldControl()
{
    mnFirst = 0;
    mnLast = 0x7FFFFFFF;
}

OUString UnoNumericFieldControl::GetComponentServiceName()
{
    return OUString("numericfield");
}

// uno::XInterface
uno::Any UnoNumericFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< awt::XNumericField* >(this)) );
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
UnoControlCurrencyFieldModel::UnoControlCurrencyFieldModel( const Reference< XComponentContext >& rxContext )
    :UnoControlModel( rxContext )
{
    UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXCurrencyField );
}

OUString UnoControlCurrencyFieldModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlCurrencyFieldModel );
}

uno::Any UnoControlCurrencyFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= OUString::createFromAscii( szServiceName_UnoControlCurrencyField );
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
    :UnoSpinFieldControl()
{
    mnFirst = 0;
    mnLast = 0x7FFFFFFF;
}

OUString UnoCurrencyFieldControl::GetComponentServiceName()
{
    return OUString("longcurrencyfield");
}

// uno::XInterface
uno::Any UnoCurrencyFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< awt::XCurrencyField* >(this)) );
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
UnoControlPatternFieldModel::UnoControlPatternFieldModel( const Reference< XComponentContext >& rxContext )
    :UnoControlModel( rxContext )
{
    UNO_CONTROL_MODEL_REGISTER_PROPERTIES( VCLXPatternField );
}

OUString UnoControlPatternFieldModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlPatternFieldModel );
}

uno::Any UnoControlPatternFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= OUString::createFromAscii( szServiceName_UnoControlPatternField );
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
    :UnoSpinFieldControl()
{
}

OUString UnoPatternFieldControl::GetComponentServiceName()
{
    return OUString("patternfield");
}

void UnoPatternFieldControl::ImplSetPeerProperty( const OUString& rPropName, const uno::Any& rVal )
{
    sal_uInt16 nType = GetPropertyId( rPropName );
    if ( ( nType == BASEPROPERTY_TEXT ) || ( nType == BASEPROPERTY_EDITMASK ) || ( nType == BASEPROPERTY_LITERALMASK ) )
    {
        // These masks cannot be set consecutively
        OUString Text = ImplGetPropertyValue_UString( BASEPROPERTY_TEXT );
        OUString EditMask = ImplGetPropertyValue_UString( BASEPROPERTY_EDITMASK );
        OUString LiteralMask = ImplGetPropertyValue_UString( BASEPROPERTY_LITERALMASK );

        uno::Reference < awt::XPatternField >  xPF( getPeer(), uno::UNO_QUERY );
        if (xPF.is())
        {
            // same comment as in UnoControl::ImplSetPeerProperty - see there
            OUString sText( Text );
            ImplCheckLocalize( sText );
            xPF->setString( sText );
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
                                        (static_cast< awt::XPatternField* >(this)) );
    return (aRet.hasValue() ? aRet : UnoSpinFieldControl::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoPatternFieldControl )
    getCppuType( ( uno::Reference< awt::XPatternField>* ) NULL ),
    UnoSpinFieldControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoPatternFieldControl::setString( const OUString& rString ) throw(uno::RuntimeException)
{
    setText( rString );
}

OUString UnoPatternFieldControl::getString() throw(uno::RuntimeException)
{
    return getText();
}

void UnoPatternFieldControl::setMasks( const OUString& EditMask, const OUString& LiteralMask ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= EditMask;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_EDITMASK ), aAny, sal_True );
    aAny <<= LiteralMask;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LITERALMASK ), aAny, sal_True );
}

void UnoPatternFieldControl::getMasks( OUString& EditMask, OUString& LiteralMask ) throw(uno::RuntimeException)
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
UnoControlProgressBarModel::UnoControlProgressBarModel( const Reference< XComponentContext >& rxContext )
    :UnoControlModel( rxContext )
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_BORDERCOLOR );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_ENABLEVISIBLE );
    ImplRegisterProperty( BASEPROPERTY_FILLCOLOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_PROGRESSVALUE );
    ImplRegisterProperty( BASEPROPERTY_PROGRESSVALUE_MAX );
    ImplRegisterProperty( BASEPROPERTY_PROGRESSVALUE_MIN );
}

OUString UnoControlProgressBarModel::getServiceName( ) throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlProgressBarModel );
}

uno::Any UnoControlProgressBarModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= OUString::createFromAscii( szServiceName_UnoControlProgressBar );
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
    :UnoControlBase()
{
}

OUString UnoProgressBarControl::GetComponentServiceName()
{
    return OUString("ProgressBar");
}

// uno::XInterface
uno::Any UnoProgressBarControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        (static_cast< awt::XProgressBar* >(this)) );
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
//  class UnoControlFixedLineModel
//  ----------------------------------------------------
UnoControlFixedLineModel::UnoControlFixedLineModel( const Reference< XComponentContext >& rxContext )
    :UnoControlModel( rxContext )
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_ENABLEVISIBLE );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_ORIENTATION );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
}

OUString UnoControlFixedLineModel::getServiceName( ) throw(::com::sun::star::uno::RuntimeException)
{
    return OUString::createFromAscii( szServiceName_UnoControlFixedLineModel );
}

uno::Any UnoControlFixedLineModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= OUString::createFromAscii( szServiceName_UnoControlFixedLine );
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
    :UnoControlBase()
{
    maComponentInfos.nWidth = 100;      // ??
    maComponentInfos.nHeight = 100;     // ??
}

OUString UnoFixedLineControl::GetComponentServiceName()
{
    return OUString("FixedLine");
}

sal_Bool UnoFixedLineControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
