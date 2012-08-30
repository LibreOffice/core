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

#include "vbacombobox.hxx"
#include <vector>
#include <filter/msfilter/msvbahelper.hxx>
#include <basic/sbstar.hxx>
#include <basic/sbmod.hxx>
#include "vbanewfont.hxx"
#include <ooo/vba/msforms/fmStyle.hpp>
#include <ooo/vba/msforms/fmDropButtonStyle.hpp>
#include <ooo/vba/msforms/fmDragBehavior.hpp>
#include <ooo/vba/msforms/fmEnterFieldBehavior.hpp>
#include <ooo/vba/msforms/fmListStyle.hpp>
#include <ooo/vba/msforms/fmTextAlign.hpp>

using namespace com::sun::star;
using namespace ooo::vba;


//SelectedItems list of integer indexes
//StringItemList list of items

const static rtl::OUString TEXT( RTL_CONSTASCII_USTRINGPARAM("Text") );
const static rtl::OUString ITEMS( RTL_CONSTASCII_USTRINGPARAM("StringItemList") );
const static rtl::OUString CONTROLSOURCEPROP( RTL_CONSTASCII_USTRINGPARAM("DataFieldProperty") );

ScVbaComboBox::ScVbaComboBox( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, AbstractGeometryAttributes* pGeomHelper ) : ComboBoxImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
    mpListHelper.reset( new ListControlHelper( m_xProps ) );
    try
    {
       // grab the default value property name
       m_xProps->getPropertyValue( CONTROLSOURCEPROP ) >>= sSourceName;
    }
    catch( uno::Exception& )
    {
    }
    if( sSourceName.isEmpty() )
        sSourceName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Text" ) );
}

// Attributes


// Value, [read] e.g. getValue returns the value of ooo Text propery e.g. the value in
// the drop down
uno::Any SAL_CALL
ScVbaComboBox::getValue() throw (uno::RuntimeException)
{
    return m_xProps->getPropertyValue( sSourceName );
}

void SAL_CALL
ScVbaComboBox::setListIndex( const uno::Any& _value ) throw (uno::RuntimeException)
{
    sal_Int16 nIndex = 0;
    if( _value >>= nIndex )
    {
        sal_Int32 nOldIndex = -1;
        getListIndex() >>= nOldIndex;
        uno::Sequence< rtl::OUString > sItems;
        m_xProps->getPropertyValue( ITEMS ) >>= sItems;
        if( ( nIndex >= 0 ) && ( sItems.getLength() > nIndex ) )
        {
            rtl::OUString sText = sItems[ nIndex ];
            m_xProps->setPropertyValue( TEXT, uno::makeAny( sText ) );

            // fire the _Change event
            if( nOldIndex != nIndex )
                fireClickEvent();
        }
    }
}

uno::Any SAL_CALL
ScVbaComboBox::getListIndex() throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > sItems;
    m_xProps->getPropertyValue( ITEMS ) >>= sItems;
    // should really return the item that has focus regardless of
    // it been selected
    if ( sItems.getLength() > 0 )
    {
        rtl::OUString sText = getText();
        sal_Int32 nLen = sItems.getLength();
        for ( sal_Int32 index = 0; !sText.isEmpty() && index < nLen; ++index )
        {
            if ( sItems[ index ].equals( sText ) )
            {
                OSL_TRACE("getListIndex returning %d", index );
                return uno::makeAny( index );
            }

        }
     }
    OSL_TRACE("getListIndex returning %d", -1 );
    return uno::makeAny( sal_Int32( -1 ) );
}

// Value, [write]e.g. setValue sets the value in the drop down, and if the value is one
// of the values in the list then the selection is also set
void SAL_CALL
ScVbaComboBox::setValue( const uno::Any& _value ) throw (uno::RuntimeException)
{
    rtl::OUString sOldValue, sNewValue;
    // booleans are converted to uppercase strings
    sOldValue =  extractStringFromAny( getValue(), ::rtl::OUString(), true );
    // booleans are converted to uppercase strings
    sNewValue =  extractStringFromAny( _value, ::rtl::OUString(), true );

    m_xProps->setPropertyValue( sSourceName, uno::Any( sNewValue ) );

    if ( sNewValue != sOldValue )
    {
        // If the new value is in current list, we should fire click event, otherwise fire the change event.
        sal_Int32 nListIndex = -1;
        getListIndex() >>= nListIndex;
        sal_Bool bIsInList = ( nListIndex >= 0 );
        if ( bIsInList )
        {
            fireClickEvent();
        }
        else
        {
            fireChangeEvent();
        }
    }
}

// see Value

::rtl::OUString SAL_CALL
ScVbaComboBox::getText() throw (uno::RuntimeException)
{
    rtl::OUString result;
    getValue() >>= result;
    return result;
}

void SAL_CALL
ScVbaComboBox::setText( const ::rtl::OUString& _text ) throw (uno::RuntimeException)
{
    setValue( uno::makeAny( _text ) ); // seems the same
}

// Methods
void SAL_CALL
ScVbaComboBox::AddItem( const uno::Any& pvargItem, const uno::Any& pvargIndex ) throw (uno::RuntimeException)
{
    mpListHelper->AddItem( pvargItem, pvargIndex );
}

void SAL_CALL
ScVbaComboBox::removeItem( const uno::Any& index ) throw (uno::RuntimeException)
{
    mpListHelper->removeItem( index );
}

void SAL_CALL
ScVbaComboBox::Clear(  ) throw (uno::RuntimeException)
{
    mpListHelper->Clear();
}

void SAL_CALL
ScVbaComboBox::setRowSource( const rtl::OUString& _rowsource ) throw (css::uno::RuntimeException)
{
    ScVbaControl::setRowSource( _rowsource );
    mpListHelper->setRowSource( _rowsource );
}

sal_Int32 SAL_CALL
ScVbaComboBox::getListCount() throw (uno::RuntimeException)
{
    return mpListHelper->getListCount();
}

uno::Any SAL_CALL
ScVbaComboBox::List( const ::uno::Any& pvargIndex, const uno::Any& pvarColumn ) throw (uno::RuntimeException)
{
    return mpListHelper->List( pvargIndex, pvarColumn );
}

sal_Int32 SAL_CALL ScVbaComboBox::getStyle() throw (uno::RuntimeException)
{
    return msforms::fmStyle::fmStyleDropDownCombo;
}

void SAL_CALL ScVbaComboBox::setStyle( sal_Int32 /*nStyle*/ ) throw (uno::RuntimeException)
{
}

sal_Int32 SAL_CALL ScVbaComboBox::getDropButtonStyle() throw (uno::RuntimeException)
{
    return msforms::fmDropButtonStyle::fmDropButtonStyleArrow;
}

void SAL_CALL ScVbaComboBox::setDropButtonStyle( sal_Int32 /*nDropButtonStyle*/ ) throw (uno::RuntimeException)
{
}

sal_Int32 SAL_CALL ScVbaComboBox::getDragBehavior() throw (uno::RuntimeException)
{
    return msforms::fmDragBehavior::fmDragBehaviorDisabled;
}

void SAL_CALL ScVbaComboBox::setDragBehavior( sal_Int32 /*nDragBehavior*/ ) throw (uno::RuntimeException)
{
}

sal_Int32 SAL_CALL ScVbaComboBox::getEnterFieldBehavior() throw (uno::RuntimeException)
{
    return msforms::fmEnterFieldBehavior::fmEnterFieldBehaviorSelectAll;
}

void SAL_CALL ScVbaComboBox::setEnterFieldBehavior( sal_Int32 /*nEnterFieldBehavior*/ ) throw (uno::RuntimeException)
{
}

sal_Int32 SAL_CALL ScVbaComboBox::getListStyle() throw (uno::RuntimeException)
{
    return msforms::fmListStyle::fmListStylePlain;
}

void SAL_CALL ScVbaComboBox::setListStyle( sal_Int32 /*nListStyle*/ ) throw (uno::RuntimeException)
{
}

sal_Int32 SAL_CALL ScVbaComboBox::getTextAlign() throw (uno::RuntimeException)
{
    return msforms::fmTextAlign::fmTextAlignLeft;
}

void SAL_CALL ScVbaComboBox::setTextAlign( sal_Int32 /*nTextAlign*/ ) throw (uno::RuntimeException)
{
}

sal_Int32 SAL_CALL ScVbaComboBox::getTextLength() throw (uno::RuntimeException)
{
    return getText().getLength();
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaComboBox::getFont() throw (uno::RuntimeException)
{
    return new VbaNewFont( this, mxContext, m_xProps );
}

rtl::OUString
ScVbaComboBox::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScVbaComboBox"));
}

sal_Int32 SAL_CALL ScVbaComboBox::getBackColor() throw (uno::RuntimeException)
{
    return ScVbaControl::getBackColor();
}

void SAL_CALL ScVbaComboBox::setBackColor( sal_Int32 nBackColor ) throw (uno::RuntimeException)
{
    ScVbaControl::setBackColor( nBackColor );
}

sal_Bool SAL_CALL ScVbaComboBox::getAutoSize() throw (uno::RuntimeException)
{
    return ScVbaControl::getAutoSize();
}

void SAL_CALL ScVbaComboBox::setAutoSize( sal_Bool bAutoSize ) throw (uno::RuntimeException)
{
    ScVbaControl::setAutoSize( bAutoSize );
}

sal_Bool SAL_CALL ScVbaComboBox::getLocked() throw (uno::RuntimeException)
{
    return ScVbaControl::getLocked();
}

void SAL_CALL ScVbaComboBox::setLocked( sal_Bool bLocked ) throw (uno::RuntimeException)
{
    ScVbaControl::setLocked( bLocked );
}

uno::Sequence< rtl::OUString >
ScVbaComboBox::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msforms.ComboBox" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
