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

#include "vbalistbox.hxx"
#include "vbanewfont.hxx"
#include <comphelper/anytostring.hxx>
#include <com/sun/star/script/ArrayWrapper.hpp>
#include <com/sun/star/form/validation/XValidatableFormComponent.hpp>

using namespace com::sun::star;
using namespace ooo::vba;

const static rtl::OUString TEXT( RTL_CONSTASCII_USTRINGPARAM("Text") );
const static rtl::OUString SELECTEDITEMS( RTL_CONSTASCII_USTRINGPARAM("SelectedItems") );
const static rtl::OUString ITEMS( RTL_CONSTASCII_USTRINGPARAM("StringItemList") );


ScVbaListBox::ScVbaListBox( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< css::uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, AbstractGeometryAttributes* pGeomHelper ) : ListBoxImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
    mpListHelper.reset( new ListControlHelper( m_xProps ) );
}

// Attributes
void SAL_CALL
ScVbaListBox::setListIndex( const uno::Any& _value ) throw (uno::RuntimeException)
{
    sal_Int32 nIndex = 0;
    _value >>= nIndex;
    uno::Reference< XPropValue > xPropVal( Selected( nIndex ), uno::UNO_QUERY_THROW );
    xPropVal->setValue( uno::makeAny( sal_True ) );
}

uno::Any SAL_CALL
ScVbaListBox::getListIndex() throw (uno::RuntimeException)
{
    uno::Sequence< sal_Int16 > sSelection;
    m_xProps->getPropertyValue( SELECTEDITEMS ) >>= sSelection;
    if ( sSelection.getLength() == 0 )
        return uno::Any( sal_Int32( -1 ) );
    return uno::Any( sSelection[ 0 ] );
}

uno::Any SAL_CALL
ScVbaListBox::getValue() throw (uno::RuntimeException)
{
    uno::Sequence< sal_Int16 > sSelection;
    uno::Sequence< rtl::OUString > sItems;
    m_xProps->getPropertyValue( SELECTEDITEMS ) >>= sSelection;
    m_xProps->getPropertyValue( ITEMS ) >>= sItems;
    if( getMultiSelect() )
        throw uno::RuntimeException( rtl::OUString::createFromAscii(
                    "Attribute use invalid." ), uno::Reference< uno::XInterface >() );
    uno::Any aRet;
    if ( sSelection.getLength() )
        aRet = uno::makeAny( sItems[ sSelection[ 0 ] ] );
    return aRet;
}

void SAL_CALL
ScVbaListBox::setValue( const uno::Any& _value ) throw (uno::RuntimeException)
{
    if( getMultiSelect() )
    {
        throw uno::RuntimeException( rtl::OUString::createFromAscii(
                    "Attribute use invalid." ), uno::Reference< uno::XInterface >() );
    }
    rtl::OUString sValue = getAnyAsString( _value );
    uno::Sequence< rtl::OUString > sList;
    m_xProps->getPropertyValue( ITEMS ) >>= sList;
    uno::Sequence< sal_Int16 > nList;
    sal_Int16 nLength = static_cast<sal_Int16>( sList.getLength() );
    sal_Int16 nValue = -1;
    sal_Int16 i = 0;
    for( i = 0; i < nLength; i++ )
    {
        if( sList[i].equals( sValue ) )
        {
            nValue = i;
            break;
        }
    }
    if( nValue == -1 )
        throw uno::RuntimeException( rtl::OUString::createFromAscii(
                    "Attribute use invalid." ), uno::Reference< uno::XInterface >() );

    uno::Sequence< sal_Int16 > nSelectedIndices(1);
    nSelectedIndices[ 0 ] = nValue;
    m_xProps->setPropertyValue( SELECTEDITEMS, uno::makeAny( nSelectedIndices ) );
    m_xProps->setPropertyValue( TEXT, uno::makeAny( sValue ) );
}

::rtl::OUString SAL_CALL
ScVbaListBox::getText() throw (uno::RuntimeException)
{
    rtl::OUString result;
    getValue() >>= result;
    return result;
}

void SAL_CALL
ScVbaListBox::setText( const ::rtl::OUString& _text ) throw (uno::RuntimeException)
{
    setValue( uno::makeAny( _text ) ); // seems the same
}

sal_Bool SAL_CALL
ScVbaListBox::getMultiSelect() throw (css::uno::RuntimeException)
{
    sal_Bool bMultiSelect = sal_False;
    m_xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MultiSelection" ) ) ) >>= bMultiSelect;
    return bMultiSelect;
}

void SAL_CALL
ScVbaListBox::setMultiSelect( sal_Bool _multiselect ) throw (css::uno::RuntimeException)
{
    m_xProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "MultiSelection" ) ), uno::makeAny( _multiselect ) );
}

css::uno::Any SAL_CALL
ScVbaListBox::Selected( sal_Int32 index ) throw (css::uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > sList;
    m_xProps->getPropertyValue( ITEMS ) >>= sList;
    sal_Int16 nLength = static_cast< sal_Int16 >( sList.getLength() );
    // no choice but to do a horror cast as internally
    // the indices are but sal_Int16
    sal_Int16 nIndex = static_cast< sal_Int16 >( index );
    if( nIndex < 0 || nIndex >= nLength )
        throw uno::RuntimeException( rtl::OUString::createFromAscii(
                    "Error Number." ), uno::Reference< uno::XInterface >() );
    m_nIndex = nIndex;
    return uno::makeAny( uno::Reference< XPropValue > ( new ScVbaPropValue( this ) ) );
}

// Methods
void SAL_CALL
ScVbaListBox::AddItem( const uno::Any& pvargItem, const uno::Any& pvargIndex ) throw (uno::RuntimeException)
{
    mpListHelper->AddItem( pvargItem, pvargIndex );
        }

void SAL_CALL
ScVbaListBox::removeItem( const uno::Any& index ) throw (uno::RuntimeException)
{
    mpListHelper->removeItem( index );
}

void SAL_CALL
ScVbaListBox::Clear(  ) throw (uno::RuntimeException)
{
    mpListHelper->Clear();
}

// this is called when something like the following vba code is used
// to set the selected state of particular entries in the Listbox
// ListBox1.Selected( 3 ) = false
//PropListener
void
ScVbaListBox::setValueEvent( const uno::Any& value )
{
    sal_Bool bValue = sal_False;
    if( !(value >>= bValue) )
        throw uno::RuntimeException( rtl::OUString::createFromAscii(
                    "Invalid type\n. need boolean." ), uno::Reference< uno::XInterface >() );
    uno::Sequence< sal_Int16 > nList;
    m_xProps->getPropertyValue( SELECTEDITEMS ) >>= nList;
    sal_Int16 nLength = static_cast<sal_Int16>( nList.getLength() );
    sal_Int16 nIndex = m_nIndex;
    for( sal_Int16 i = 0; i < nLength; i++ )
    {
        if( nList[i] == nIndex )
        {
            if( bValue )
                return;
            else
            {
                for( ; i < nLength - 1; i++ )
                {
                    nList[i] = nList[i + 1];
                }
                nList.realloc( nLength - 1 );
                //m_xProps->setPropertyValue( sSourceName, uno::makeAny( nList ) );
        m_xProps->setPropertyValue( SELECTEDITEMS, uno::makeAny( nList ) );
                return;
            }
        }
    }
    if( bValue )
    {
        if( getMultiSelect() )
        {
            nList.realloc( nLength + 1 );
            nList[nLength] = nIndex;
        }
        else
        {
            nList.realloc( 1 );
            nList[0] = nIndex;
        }
        //m_xProps->setPropertyValue( sSourceName, uno::makeAny( nList ) );
        m_xProps->setPropertyValue( SELECTEDITEMS, uno::makeAny( nList ) );
    }
}

// this is called when something like the following vba code is used
// to determine the selected state of particular entries in the Listbox
// msgbox ListBox1.Selected( 3 )

css::uno::Any
ScVbaListBox::getValueEvent()
{
    uno::Sequence< sal_Int16 > nList;
    m_xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SelectedItems" ) ) ) >>= nList;
    sal_Int32 nLength = nList.getLength();
    sal_Int32 nIndex = m_nIndex;

    for( sal_Int32 i = 0; i < nLength; i++ )
    {
        if( nList[i] == nIndex )
            return uno::makeAny( sal_True );
    }

    return uno::makeAny( sal_False );
}

void SAL_CALL
ScVbaListBox::setRowSource( const rtl::OUString& _rowsource ) throw (uno::RuntimeException)
{
    ScVbaControl::setRowSource( _rowsource );
    mpListHelper->setRowSource( _rowsource );
}

sal_Int32 SAL_CALL
ScVbaListBox::getListCount() throw (uno::RuntimeException)
{
    return mpListHelper->getListCount();
}

uno::Any SAL_CALL
ScVbaListBox::List( const ::uno::Any& pvargIndex, const uno::Any& pvarColumn ) throw (uno::RuntimeException)
{
    return mpListHelper->List( pvargIndex, pvarColumn );
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaListBox::getFont() throw (uno::RuntimeException)
{
    return new VbaNewFont( this, mxContext, m_xProps );
}

rtl::OUString&
ScVbaListBox::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaListBox") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaListBox::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.msforms.ScVbaListBox" ) );
    }
    return aServiceNames;
}
