/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "vbalistbox.hxx"
#include "vbanewfont.hxx"
#include <comphelper/anytostring.hxx>
#include <com/sun/star/script/ArrayWrapper.hpp>
#include <com/sun/star/form/validation/XValidatableFormComponent.hpp>
#include <ooo/vba/msforms/fmMultiSelect.hpp>

using namespace com::sun::star;
using namespace ooo::vba;

const static OUString TEXT( "Text" );
const static OUString SELECTEDITEMS( "SelectedItems" );
const static OUString ITEMS( "StringItemList" );


ScVbaListBox::ScVbaListBox( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< css::uno::XInterface >& xControl, const uno::Reference< frame::XModel >& xModel, AbstractGeometryAttributes* pGeomHelper ) : ListBoxImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper )
{
    mpListHelper.reset( new ListControlHelper( m_xProps ) );
}


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
    uno::Sequence< OUString > sItems;
    m_xProps->getPropertyValue( SELECTEDITEMS ) >>= sSelection;
    m_xProps->getPropertyValue( ITEMS ) >>= sItems;
    if( getMultiSelect() )
        throw uno::RuntimeException( "Attribute use invalid." , uno::Reference< uno::XInterface >() );
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
        throw uno::RuntimeException( "Attribute use invalid." , uno::Reference< uno::XInterface >() );
    }
    OUString sValue = getAnyAsString( _value );
    uno::Sequence< OUString > sList;
    m_xProps->getPropertyValue( ITEMS ) >>= sList;
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
        throw uno::RuntimeException( "Attribute use invalid." , uno::Reference< uno::XInterface >() );

    uno::Sequence< sal_Int16 > nSelectedIndices(1);
    uno::Sequence< sal_Int16 > nOldSelectedIndices;
    m_xProps->getPropertyValue( SELECTEDITEMS ) >>= nOldSelectedIndices;
    nSelectedIndices[ 0 ] = nValue;
    m_xProps->setPropertyValue( SELECTEDITEMS, uno::makeAny( nSelectedIndices ) );
    if ( nSelectedIndices != nOldSelectedIndices )
        fireClickEvent();
}

OUString SAL_CALL
ScVbaListBox::getText() throw (uno::RuntimeException)
{
    OUString result;
    getValue() >>= result;
    return result;
}

void SAL_CALL
ScVbaListBox::setText( const OUString& _text ) throw (uno::RuntimeException)
{
    setValue( uno::makeAny( _text ) ); 
}

sal_Int32 SAL_CALL
ScVbaListBox::getMultiSelect() throw (css::uno::RuntimeException)
{
    sal_Bool bMultiSelect = sal_False;
    m_xProps->getPropertyValue( "MultiSelection" ) >>= bMultiSelect;

    return bMultiSelect ? msforms::fmMultiSelect::fmMultiSelectMulti : msforms::fmMultiSelect::fmMultiSelectSingle;
}

void SAL_CALL
ScVbaListBox::setMultiSelect( sal_Int32 _multiselect ) throw (css::uno::RuntimeException)
{
    sal_Bool bBoolVal = false;
    switch ( _multiselect )
    {
        case  msforms::fmMultiSelect::fmMultiSelectMulti:
        case  msforms::fmMultiSelect::fmMultiSelectExtended:
            bBoolVal = sal_True;
            break;
        case msforms::fmMultiSelect::fmMultiSelectSingle:
            bBoolVal = sal_False;
            break;
        default:
            throw lang::IllegalArgumentException();
            break;
    }
    m_xProps->setPropertyValue( "MultiSelection" , uno::makeAny( bBoolVal ) );
}


css::uno::Any SAL_CALL
ScVbaListBox::Selected( sal_Int32 index ) throw (css::uno::RuntimeException)
{
    uno::Sequence< OUString > sList;
    m_xProps->getPropertyValue( ITEMS ) >>= sList;
    sal_Int16 nLength = static_cast< sal_Int16 >( sList.getLength() );
    
    
    sal_Int16 nIndex = static_cast< sal_Int16 >( index );
    if( nIndex < 0 || nIndex >= nLength )
        throw uno::RuntimeException( "Error Number." , uno::Reference< uno::XInterface >() );
    m_nIndex = nIndex;
    return uno::makeAny( uno::Reference< XPropValue > ( new ScVbaPropValue( this ) ) );
}


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





void
ScVbaListBox::setValueEvent( const uno::Any& value )
{
    sal_Bool bValue = sal_False;
    if( !(value >>= bValue) )
        throw uno::RuntimeException( "Invalid type\n. need boolean." , uno::Reference< uno::XInterface >() );
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
                
                fireClickEvent();
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
        
        fireClickEvent();
        m_xProps->setPropertyValue( SELECTEDITEMS, uno::makeAny( nList ) );
    }
}





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
            return uno::makeAny( sal_True );
    }

    return uno::makeAny( sal_False );
}

void SAL_CALL
ScVbaListBox::setRowSource( const OUString& _rowsource ) throw (uno::RuntimeException)
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

OUString
ScVbaListBox::getServiceImplName()
{
    return OUString("ScVbaListBox");
}

uno::Sequence< OUString >
ScVbaListBox::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.msforms.ScVbaListBox";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
