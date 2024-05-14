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

#include "vbalistcontrolhelper.hxx"
#include <utility>
#include <vector>
#include <vbahelper/vbapropvalue.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/sequence.hxx>

using namespace com::sun::star;
using namespace ooo::vba;

namespace {

class ListPropListener : public PropListener
{
private:
    uno::Reference< beans::XPropertySet > m_xProps;
    uno::Any m_pvargIndex;
    uno::Any m_pvarColumn;

public:
    ListPropListener( uno::Reference< beans::XPropertySet > xProps, uno::Any  pvargIndex, uno::Any varColumn );
    virtual ~ListPropListener() { };
    virtual void setValueEvent( const css::uno::Any& value ) override;
    virtual css::uno::Any getValueEvent() override;
};

}

ListPropListener::ListPropListener( uno::Reference< beans::XPropertySet > xProps, uno::Any vargIndex, uno::Any varColumn ) : m_xProps(std::move( xProps )), m_pvargIndex(std::move( vargIndex )), m_pvarColumn(std::move( varColumn ))
{
}

void ListPropListener::setValueEvent( const uno::Any& value )
{
    if( m_pvargIndex.hasValue() || m_pvarColumn.hasValue() )
        throw uno::RuntimeException( u"Bad argument"_ustr );

    m_xProps->setPropertyValue( u"StringItemList"_ustr, value );
}

uno::Any ListPropListener::getValueEvent()
{
    uno::Sequence< OUString > sList;
    m_xProps->getPropertyValue( u"StringItemList"_ustr ) >>= sList;
    sal_Int16 nLength = static_cast< sal_Int16 >( sList.getLength() );
    uno::Any aRet;
    if ( m_pvargIndex.hasValue() )
    {
        sal_Int16 nIndex = -1;
        m_pvargIndex >>= nIndex;
        if( nIndex < 0 || nIndex >= nLength )
            throw uno::RuntimeException( u"Bad row Index"_ustr );
        aRet <<= sList[ nIndex ];
    }
    else if ( m_pvarColumn.hasValue() ) // pvarColumn on its own would be bad
            throw uno::RuntimeException( u"Bad column Index"_ustr );
    else // List() ( e.g. no args )
    {
        uno::Sequence< uno::Sequence< OUString > > sReturnArray( nLength );
        auto pReturnArray = sReturnArray.getArray();
        for ( sal_Int32 i = 0; i < nLength; ++i )
        {
            pReturnArray[ i ].realloc( 10 );
            pReturnArray[ i ].getArray()[ 0 ] = sList[ i ];
        }
        aRet <<= sReturnArray;
    }
    return aRet;
}

void
ListControlHelper::AddItem( const uno::Any& pvargItem, const uno::Any& pvargIndex )
{
    if ( !pvargItem.hasValue()  )
        return;

    uno::Sequence< OUString > sList;
    m_xProps->getPropertyValue( u"StringItemList"_ustr ) >>= sList;

    sal_Int32 nIndex = sList.getLength();

    if ( pvargIndex.hasValue() )
        pvargIndex >>= nIndex;

    OUString sString = getAnyAsString( pvargItem );

    // if no index specified or item is to be appended to end of
    // list just realloc the array and set the last item
    if ( nIndex  == sList.getLength() )
    {
        sal_Int32 nOldSize = sList.getLength();
        sList.realloc( nOldSize + 1 );
        sList.getArray()[ nOldSize ] = sString;
    }
    else
    {
        // just copy those elements above the one to be inserted
        std::vector< OUString > sVec;
        // reserve just the amount we need to copy
        sVec.reserve( sList.getLength() - nIndex + 1);

        // insert the new element
        sVec.push_back( sString );

        // point at first element to copy
        sVec.insert( sVec.end(), std::next(std::cbegin(sList), nIndex), std::cend(sList) );

        sList.realloc(  sList.getLength() + 1 );

        // point at first element to be overwritten
        std::copy(sVec.begin(), sVec.end(), std::next(sList.getArray(), nIndex));
    }

    m_xProps->setPropertyValue( u"StringItemList"_ustr, uno::Any( sList ) );
}

void
ListControlHelper::removeItem( const uno::Any& index )
{
    sal_Int32 nIndex = 0;
    // for int index
    if ( !(index >>= nIndex)  )
        return;

    uno::Sequence< OUString > sList;
    m_xProps->getPropertyValue( u"StringItemList"_ustr ) >>= sList;
    if( nIndex < 0 || nIndex > ( sList.getLength() - 1 ) )
        throw uno::RuntimeException( u"Invalid index"_ustr , uno::Reference< uno::XInterface > () );
    if( sList.hasElements() )
    {
        if( sList.getLength() == 1 )
        {
            Clear();
            return;
        }

        comphelper::removeElementAt(sList, nIndex);
    }

    m_xProps->setPropertyValue( u"StringItemList"_ustr, uno::Any( sList ) );
}

void
ListControlHelper::Clear(  )
{
    // urk, setValue doesn't seem to work !!
    //setValue( uno::makeAny( sal_Int16() ) );
    m_xProps->setPropertyValue( u"StringItemList"_ustr, uno::Any( uno::Sequence< OUString >() ) );
}

void
ListControlHelper::setRowSource( std::u16string_view _rowsource )
{
    if ( _rowsource.empty() )
        Clear();
}

sal_Int32
ListControlHelper::getListCount()
{
    uno::Sequence< OUString > sList;
    m_xProps->getPropertyValue( u"StringItemList"_ustr ) >>= sList;
    return sList.getLength();
}

uno::Any
ListControlHelper::List( const ::uno::Any& pvargIndex, const uno::Any& pvarColumn )
{
    return uno::Any( uno::Reference< XPropValue > ( new ScVbaPropValue( new ListPropListener( m_xProps, pvargIndex, pvarColumn ) ) ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
