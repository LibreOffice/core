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

#include "vbacombobox.hxx"
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
const static rtl::OUString SELECTEDITEMS( RTL_CONSTASCII_USTRINGPARAM("SelectedItems") );
const static rtl::OUString ITEMS( RTL_CONSTASCII_USTRINGPARAM("StringItemList") );
const static rtl::OUString CONTROLSOURCEPROP( RTL_CONSTASCII_USTRINGPARAM("DataFieldProperty") );

ScVbaComboBox::ScVbaComboBox( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, AbstractGeometryAttributes* pGeomHelper, bool bDialogType ) : ComboBoxImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper ), mbDialogType( bDialogType )
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
    if( sSourceName.getLength() == 0 )
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
        uno::Sequence< rtl::OUString > sItems;
        m_xProps->getPropertyValue( ITEMS ) >>= sItems;
        if( ( nIndex >= 0 ) && ( sItems.getLength() > nIndex ) )
        {
            rtl::OUString sText = sItems[ nIndex ];
            m_xProps->setPropertyValue( TEXT, uno::makeAny( sText ) );
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
        for ( sal_Int32 index = 0; sText.getLength() && index < nLen; ++index )
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
    // booleans are converted to uppercase strings
    m_xProps->setPropertyValue( sSourceName, uno::Any( extractStringFromAny( _value, ::rtl::OUString(), true ) ) );
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

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaComboBox::getFont() throw (uno::RuntimeException)
{
    return new VbaNewFont( this, mxContext, m_xProps );
}

rtl::OUString&
ScVbaComboBox::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaComboBox") );
    return sImplName;
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
