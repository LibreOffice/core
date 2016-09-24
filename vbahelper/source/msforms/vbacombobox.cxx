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

ScVbaComboBox::ScVbaComboBox( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, AbstractGeometryAttributes* pGeomHelper ) : ComboBoxImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
    mpListHelper.reset( new ListControlHelper( m_xProps ) );
    try
    {
       // grab the default value property name
       m_xProps->getPropertyValue( "DataFieldProperty" ) >>= sSourceName;
    }
    catch( uno::Exception& )
    {
    }
    if( sSourceName.isEmpty() )
        sSourceName = "Text";
}

// Attributes


// Value, [read] e.g. getValue returns the value of ooo Text propery e.g. the value in
// the drop down
uno::Any SAL_CALL
ScVbaComboBox::getValue() throw (uno::RuntimeException, std::exception)
{
    return m_xProps->getPropertyValue( sSourceName );
}

void SAL_CALL
ScVbaComboBox::setListIndex( const uno::Any& _value ) throw (uno::RuntimeException, std::exception)
{
    sal_Int16 nIndex = 0;
    if( _value >>= nIndex )
    {
        sal_Int32 nOldIndex = -1;
        getListIndex() >>= nOldIndex;
        uno::Sequence< OUString > sItems;
        m_xProps->getPropertyValue( "StringItemList" ) >>= sItems;
        if( ( nIndex >= 0 ) && ( sItems.getLength() > nIndex ) )
        {
            OUString sText = sItems[ nIndex ];
            m_xProps->setPropertyValue( "Text", uno::makeAny( sText ) );

            // fire the _Change event
            if( nOldIndex != nIndex )
                fireClickEvent();
        }
    }
}

uno::Any SAL_CALL
ScVbaComboBox::getListIndex() throw (uno::RuntimeException, std::exception)
{
    uno::Sequence< OUString > sItems;
    m_xProps->getPropertyValue( "StringItemList" ) >>= sItems;
    // should really return the item that has focus regardless of
    // it been selected
    if ( sItems.getLength() > 0 )
    {
        OUString sText = getText();
        sal_Int32 nLen = sItems.getLength();
        for ( sal_Int32 index = 0; !sText.isEmpty() && index < nLen; ++index )
        {
            if ( sItems[ index ].equals( sText ) )
            {
                SAL_INFO("vbahelper", "getListIndex returning " << index );
                return uno::makeAny( index );
            }

        }
     }
    SAL_INFO("vbahelper", "getListIndex returning -1" );
    return uno::makeAny( sal_Int32( -1 ) );
}

// Value, [write]e.g. setValue sets the value in the drop down, and if the value is one
// of the values in the list then the selection is also set
void SAL_CALL
ScVbaComboBox::setValue( const uno::Any& _value ) throw (uno::RuntimeException, std::exception)
{
    // booleans are converted to uppercase strings
    OUString oldValue = extractStringFromAny( getValue(), OUString(), true );
    m_xProps->setPropertyValue( sSourceName, uno::Any( extractStringFromAny( _value, OUString(), true ) ) );
    OUString newValue = extractStringFromAny( getValue(), OUString(), true );
    if ( oldValue != newValue )
    {
        sal_Int32 index = 0;
        uno::Any aIndex = getListIndex();
        aIndex >>= index;
        if ( index < 0 )
            fireChangeEvent();
        else
            fireClickEvent();
    }
}

// see Value

OUString SAL_CALL
ScVbaComboBox::getText() throw (uno::RuntimeException, std::exception)
{
    OUString result;
    getValue() >>= result;
    return result;
}

void SAL_CALL
ScVbaComboBox::setText( const OUString& _text ) throw (uno::RuntimeException, std::exception)
{
    setValue( uno::makeAny( _text ) ); // seems the same
}

// Methods
void SAL_CALL
ScVbaComboBox::AddItem( const uno::Any& pvargItem, const uno::Any& pvargIndex ) throw (uno::RuntimeException, std::exception)
{
    mpListHelper->AddItem( pvargItem, pvargIndex );
}

void SAL_CALL
ScVbaComboBox::removeItem( const uno::Any& index ) throw (uno::RuntimeException, std::exception)
{
    mpListHelper->removeItem( index );
}

void SAL_CALL
ScVbaComboBox::Clear(  ) throw (uno::RuntimeException, std::exception)
{
    mpListHelper->Clear();
}

void SAL_CALL
ScVbaComboBox::setRowSource( const OUString& _rowsource ) throw (css::uno::RuntimeException, std::exception)
{
    ScVbaControl::setRowSource( _rowsource );
    mpListHelper->setRowSource( _rowsource );
}

sal_Int32 SAL_CALL
ScVbaComboBox::getListCount() throw (uno::RuntimeException, std::exception)
{
    return mpListHelper->getListCount();
}

uno::Any SAL_CALL
ScVbaComboBox::List( const ::uno::Any& pvargIndex, const uno::Any& pvarColumn ) throw (uno::RuntimeException, std::exception)
{
    return mpListHelper->List( pvargIndex, pvarColumn );
}

sal_Int32 SAL_CALL ScVbaComboBox::getStyle() throw (uno::RuntimeException, std::exception)
{
    return msforms::fmStyle::fmStyleDropDownCombo;
}

void SAL_CALL ScVbaComboBox::setStyle( sal_Int32 /*nStyle*/ ) throw (uno::RuntimeException, std::exception)
{
}

sal_Int32 SAL_CALL ScVbaComboBox::getDropButtonStyle() throw (uno::RuntimeException, std::exception)
{
    return msforms::fmDropButtonStyle::fmDropButtonStyleArrow;
}

void SAL_CALL ScVbaComboBox::setDropButtonStyle( sal_Int32 /*nDropButtonStyle*/ ) throw (uno::RuntimeException, std::exception)
{
}

sal_Int32 SAL_CALL ScVbaComboBox::getDragBehavior() throw (uno::RuntimeException, std::exception)
{
    return msforms::fmDragBehavior::fmDragBehaviorDisabled;
}

void SAL_CALL ScVbaComboBox::setDragBehavior( sal_Int32 /*nDragBehavior*/ ) throw (uno::RuntimeException, std::exception)
{
}

sal_Int32 SAL_CALL ScVbaComboBox::getEnterFieldBehavior() throw (uno::RuntimeException, std::exception)
{
    return msforms::fmEnterFieldBehavior::fmEnterFieldBehaviorSelectAll;
}

void SAL_CALL ScVbaComboBox::setEnterFieldBehavior( sal_Int32 /*nEnterFieldBehavior*/ ) throw (uno::RuntimeException, std::exception)
{
}

sal_Int32 SAL_CALL ScVbaComboBox::getListStyle() throw (uno::RuntimeException, std::exception)
{
    return msforms::fmListStyle::fmListStylePlain;
}

void SAL_CALL ScVbaComboBox::setListStyle( sal_Int32 /*nListStyle*/ ) throw (uno::RuntimeException, std::exception)
{
}

sal_Int32 SAL_CALL ScVbaComboBox::getTextAlign() throw (uno::RuntimeException, std::exception)
{
    return msforms::fmTextAlign::fmTextAlignLeft;
}

void SAL_CALL ScVbaComboBox::setTextAlign( sal_Int32 /*nTextAlign*/ ) throw (uno::RuntimeException, std::exception)
{
}

sal_Int32 SAL_CALL ScVbaComboBox::getTextLength() throw (uno::RuntimeException, std::exception)
{
    return getText().getLength();
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaComboBox::getFont() throw (uno::RuntimeException, std::exception)
{
    return new VbaNewFont( m_xProps );
}

OUString
ScVbaComboBox::getServiceImplName()
{
    return OUString("ScVbaComboBox");
}

sal_Int32 SAL_CALL ScVbaComboBox::getBackColor() throw (uno::RuntimeException, std::exception)
{
    return ScVbaControl::getBackColor();
}

void SAL_CALL ScVbaComboBox::setBackColor( sal_Int32 nBackColor ) throw (uno::RuntimeException, std::exception)
{
    ScVbaControl::setBackColor( nBackColor );
}

sal_Bool SAL_CALL ScVbaComboBox::getAutoSize() throw (uno::RuntimeException, std::exception)
{
    return ScVbaControl::getAutoSize();
}

void SAL_CALL ScVbaComboBox::setAutoSize( sal_Bool bAutoSize ) throw (uno::RuntimeException, std::exception)
{
    ScVbaControl::setAutoSize( bAutoSize );
}

sal_Bool SAL_CALL ScVbaComboBox::getLocked() throw (uno::RuntimeException, std::exception)
{
    return ScVbaControl::getLocked();
}

void SAL_CALL ScVbaComboBox::setLocked( sal_Bool bLocked ) throw (uno::RuntimeException, std::exception)
{
    ScVbaControl::setLocked( bLocked );
}

OUString SAL_CALL ScVbaComboBox::getLinkedCell() throw (uno::RuntimeException, std::exception)
{
    return ScVbaControl::getControlSource();
}

void SAL_CALL ScVbaComboBox::setLinkedCell( const OUString& _linkedcell ) throw (uno::RuntimeException, std::exception)
{
    ScVbaControl::setControlSource( _linkedcell );
}

uno::Sequence< OUString >
ScVbaComboBox::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.msforms.ComboBox";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
