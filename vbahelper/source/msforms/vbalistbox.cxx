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

#include "vbalistbox.hxx"
#include "vbanewfont.hxx"
#include <com/sun/star/form/validation/XValidatableFormComponent.hpp>
#include <ooo/vba/msforms/fmMultiSelect.hpp>

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaListBox::ScVbaListBox( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< css::uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, std::unique_ptr<ov::AbstractGeometryAttributes> pGeomHelper )
    : ListBoxImpl_BASE(xParent, xContext, xControl, xModel, std::move(pGeomHelper))
    , m_nIndex(0)
{
    mpListHelper.reset( new ListControlHelper( m_xProps ) );
}

// Attributes
void SAL_CALL
ScVbaListBox::setListIndex( const uno::Any& _value )
{
    sal_Int32 nIndex = 0;
    _value >>= nIndex;
    uno::Reference< XPropValue > xPropVal( Selected( nIndex ), uno::UNO_QUERY_THROW );
    xPropVal->setValue( uno::makeAny( true ) );
}

uno::Any SAL_CALL
ScVbaListBox::getListIndex()
{
    uno::Sequence< sal_Int16 > sSelection;
    m_xProps->getPropertyValue( "SelectedItems" ) >>= sSelection;
    if ( sSelection.getLength() == 0 )
        return uno::Any( sal_Int32( -1 ) );
    return uno::Any( sSelection[ 0 ] );
}

uno::Any SAL_CALL
ScVbaListBox::getValue()
{
    uno::Sequence< sal_Int16 > sSelection;
    uno::Sequence< OUString > sItems;
    m_xProps->getPropertyValue( "SelectedItems" ) >>= sSelection;
    m_xProps->getPropertyValue( "StringItemList" ) >>= sItems;
    if( getMultiSelect() )
        throw uno::RuntimeException( "Attribute use invalid." );
    uno::Any aRet;
    if ( sSelection.getLength() )
        aRet <<= sItems[ sSelection[ 0 ] ];
    return aRet;
}

void SAL_CALL
ScVbaListBox::setValue( const uno::Any& _value )
{
    if( getMultiSelect() )
    {
        throw uno::RuntimeException( "Attribute use invalid." );
    }
    OUString sValue = getAnyAsString( _value );
    uno::Sequence< OUString > sList;
    m_xProps->getPropertyValue( "StringItemList" ) >>= sList;
    sal_Int16 nLength = static_cast<sal_Int16>( sList.getLength() );
    sal_Int16 nValue = -1;
    sal_Int16 i = 0;
    for( i = 0; i < nLength; i++ )
    {
        if( sList[i] == sValue )
        {
            nValue = i;
            break;
        }
    }
    if( nValue == -1 )
        throw uno::RuntimeException( "Attribute use invalid." );

    uno::Sequence< sal_Int16 > nSelectedIndices(1);
    uno::Sequence< sal_Int16 > nOldSelectedIndices;
    m_xProps->getPropertyValue( "SelectedItems" ) >>= nOldSelectedIndices;
    nSelectedIndices[ 0 ] = nValue;
    m_xProps->setPropertyValue( "SelectedItems", uno::makeAny( nSelectedIndices ) );
    if ( nSelectedIndices != nOldSelectedIndices )
        fireClickEvent();
}

OUString SAL_CALL
ScVbaListBox::getText()
{
    OUString result;
    getValue() >>= result;
    return result;
}

void SAL_CALL
ScVbaListBox::setText( const OUString& _text )
{
    setValue( uno::makeAny( _text ) ); // seems the same
}

sal_Int32 SAL_CALL
ScVbaListBox::getMultiSelect()
{
    bool bMultiSelect = false;
    m_xProps->getPropertyValue( "MultiSelection" ) >>= bMultiSelect;

    return bMultiSelect ? msforms::fmMultiSelect::fmMultiSelectMulti : msforms::fmMultiSelect::fmMultiSelectSingle;
}

void SAL_CALL
ScVbaListBox::setMultiSelect( sal_Int32 _multiselect )
{
    bool bBoolVal = false;
    switch ( _multiselect )
    {
        case  msforms::fmMultiSelect::fmMultiSelectMulti:
        case  msforms::fmMultiSelect::fmMultiSelectExtended:
            bBoolVal = true;
            break;
        case msforms::fmMultiSelect::fmMultiSelectSingle:
            bBoolVal = false;
            break;
        default:
            throw lang::IllegalArgumentException();
            break;
    }
    m_xProps->setPropertyValue( "MultiSelection" , uno::makeAny( bBoolVal ) );
}


css::uno::Any SAL_CALL
ScVbaListBox::Selected( sal_Int32 index )
{
    uno::Sequence< OUString > sList;
    m_xProps->getPropertyValue( "StringItemList" ) >>= sList;
    sal_Int16 nLength = static_cast< sal_Int16 >( sList.getLength() );
    // no choice but to do a horror cast as internally
    // the indices are but sal_Int16
    sal_Int16 nIndex = static_cast< sal_Int16 >( index );
    if( nIndex < 0 || nIndex >= nLength )
        throw uno::RuntimeException( "Error Number." );
    m_nIndex = nIndex;
    return uno::makeAny( uno::Reference< XPropValue > ( new ScVbaPropValue( this ) ) );
}

// Methods
void SAL_CALL
ScVbaListBox::AddItem( const uno::Any& pvargItem, const uno::Any& pvargIndex )
{
    mpListHelper->AddItem( pvargItem, pvargIndex );
        }

void SAL_CALL
ScVbaListBox::removeItem( const uno::Any& index )
{
    mpListHelper->removeItem( index );
}

void SAL_CALL
ScVbaListBox::Clear(  )
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
    bool bValue = false;
    if( !(value >>= bValue) )
        throw uno::RuntimeException( "Invalid type. need boolean." );
    uno::Sequence< sal_Int16 > nList;
    m_xProps->getPropertyValue( "SelectedItems" ) >>= nList;
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
                fireClickEvent();
                m_xProps->setPropertyValue( "SelectedItems", uno::makeAny( nList ) );
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
        fireClickEvent();
        m_xProps->setPropertyValue( "SelectedItems", uno::makeAny( nList ) );
    }
}

// this is called when something like the following vba code is used
// to determine the selected state of particular entries in the Listbox
// msgbox ListBox1.Selected( 3 )

css::uno::Any
ScVbaListBox::getValueEvent()
{
    uno::Sequence< sal_Int16 > nList;
    m_xProps->getPropertyValue( "SelectedItems" ) >>= nList;
    sal_Int32 nLength = nList.getLength();
    sal_Int32 nIndex = m_nIndex;

    for( sal_Int32 i = 0; i < nLength; i++ )
    {
        if( nList[i] == nIndex )
            return uno::makeAny( true );
    }

    return uno::makeAny( false );
}

void SAL_CALL
ScVbaListBox::setRowSource( const OUString& _rowsource )
{
    ScVbaControl::setRowSource( _rowsource );
    mpListHelper->setRowSource( _rowsource );
}

sal_Int32 SAL_CALL
ScVbaListBox::getListCount()
{
    return mpListHelper->getListCount();
}

uno::Any SAL_CALL
ScVbaListBox::List( const ::uno::Any& pvargIndex, const uno::Any& pvarColumn )
{
    return mpListHelper->List( pvargIndex, pvarColumn );
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaListBox::getFont()
{
    return new VbaNewFont( m_xProps );
}

OUString
ScVbaListBox::getServiceImplName()
{
    return OUString("ScVbaListBox");
}

uno::Sequence< OUString >
ScVbaListBox::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.msforms.ScVbaListBox"
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
