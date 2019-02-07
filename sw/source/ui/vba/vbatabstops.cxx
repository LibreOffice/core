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
#include "vbatabstops.hxx"
#include "vbatabstop.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/TabAlign.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <ooo/vba/word/WdTabLeader.hpp>
#include <ooo/vba/word/WdTabAlignment.hpp>
#include <basic/sberrors.hxx>
#include <cppuhelper/implbase.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

/// @throws uno::RuntimeException
static uno::Sequence< style::TabStop > lcl_getTabStops( const uno::Reference< beans::XPropertySet >& xParaProps )
{
    uno::Sequence< style::TabStop > aSeq;
    xParaProps->getPropertyValue("ParaTabStops") >>= aSeq;
    return aSeq;
}

/// @throws uno::RuntimeException
static void lcl_setTabStops( const uno::Reference< beans::XPropertySet >& xParaProps, const uno::Sequence< style::TabStop >& aSeq )
{
    xParaProps->setPropertyValue("ParaTabStops", uno::makeAny( aSeq ) );
}

class TabStopsEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    sal_Int32 nIndex;

public:
    explicit TabStopsEnumWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess ) : mxIndexAccess( xIndexAccess ), nIndex( 0 )
    {
    }
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( nIndex < mxIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if( nIndex < mxIndexAccess->getCount() )
        {
            return mxIndexAccess->getByIndex( nIndex++ );
        }
        throw container::NoSuchElementException();
    }
};

class TabStopCollectionHelper : public ::cppu::WeakImplHelper< container::XIndexAccess,
                                                               container::XEnumerationAccess >
{
private:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    sal_Int32 mnTabStops;

public:
    /// @throws css::uno::RuntimeException
    TabStopCollectionHelper( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& xParaProps ): mxParent( xParent ), mxContext( xContext )
    {
        mnTabStops = lcl_getTabStops( xParaProps ).getLength();
    }

    virtual sal_Int32 SAL_CALL getCount(  ) override
    {
        return mnTabStops;
    }
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override
    {
        if ( Index < 0 || Index >= getCount() )
            throw css::lang::IndexOutOfBoundsException();

        return uno::makeAny( uno::Reference< word::XTabStop >( new SwVbaTabStop( mxParent, mxContext ) ) );
    }
    virtual uno::Type SAL_CALL getElementType(  ) override
    {
        return cppu::UnoType<word::XTabStop>::get();
    }
    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        return true;
    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) override
    {
        return new TabStopsEnumWrapper( this );
    }
};

SwVbaTabStops::SwVbaTabStops( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< beans::XPropertySet >& xParaProps ) : SwVbaTabStops_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( new TabStopCollectionHelper( xParent, xContext, xParaProps ) ) ), mxParaProps( xParaProps )
{
}

uno::Reference< word::XTabStop > SAL_CALL SwVbaTabStops::Add( float Position, const uno::Any& Alignment, const uno::Any& Leader )
{
    sal_Int32 nPosition = Millimeter::getInHundredthsOfOneMillimeter( Position );

    style::TabAlign nAlign = style::TabAlign_LEFT;
    if( Alignment.hasValue() )
    {
        sal_Int32 wdAlign = word::WdTabAlignment::wdAlignTabLeft;
        Alignment >>= wdAlign;
        switch( wdAlign )
        {
            case word::WdTabAlignment::wdAlignTabLeft:
            {
                nAlign = style::TabAlign_LEFT;
                break;
            }
            case word::WdTabAlignment::wdAlignTabRight:
            {
                nAlign = style::TabAlign_RIGHT;
                break;
            }
            case word::WdTabAlignment::wdAlignTabCenter:
            {
                nAlign = style::TabAlign_CENTER;
                break;
            }
            case word::WdTabAlignment::wdAlignTabDecimal:
            {
                nAlign = style::TabAlign_DECIMAL;
                break;
            }
            case word::WdTabAlignment::wdAlignTabBar:
            case word::WdTabAlignment::wdAlignTabList:
            {
                DebugHelper::basicexception( ERRCODE_BASIC_NOT_IMPLEMENTED, OUString() );
                break;
            }
            default:
            {
                //left
            }
        }
    }

    sal_Unicode cLeader = ' '; // default is space
    if( Leader.hasValue() )
    {
        sal_Int32 wdLeader = word::WdTabLeader::wdTabLeaderSpaces;
        Leader >>= wdLeader;
        switch( wdLeader )
        {
            case word::WdTabLeader::wdTabLeaderSpaces:
            {
                cLeader = ' ';
                break;
            }
            case word::WdTabLeader::wdTabLeaderMiddleDot:
            {
                cLeader = 183; // U+00B7 MIDDLE DOT
                break;
            }
            case word::WdTabLeader::wdTabLeaderDots:
            {
                cLeader = '.';
                break;
            }
            case word::WdTabLeader::wdTabLeaderDashes:
            case word::WdTabLeader::wdTabLeaderHeavy:
            case word::WdTabLeader::wdTabLeaderLines:
            {
                cLeader = '_';
                break;
            }
            default:
            {
                //left
            }
        }
    }

    style::TabStop aTab;
    aTab.Position = nPosition;
    aTab.Alignment = nAlign;
    aTab.DecimalChar = '.'; // default value
    aTab.FillChar = cLeader;

    uno::Sequence< style::TabStop > aOldTabs = lcl_getTabStops( mxParaProps );
    bool bOverWriter = false;

    sal_Int32 nTabs = aOldTabs.getLength();
    uno::Sequence< style::TabStop > aNewTabs( nTabs + 1 );

    style::TabStop* pOldTab = aOldTabs.getArray();
    style::TabStop* pNewTab = aNewTabs.getArray();
    pNewTab[0] = aTab;
    for (sal_Int32 nIndex = 0; nIndex < nTabs; nIndex++)
    {
        if( pOldTab[nIndex].Position == nPosition )
        {
            bOverWriter = true;
            pOldTab[nIndex] = aTab;
            break;
        }
        pNewTab[ nIndex+1 ] = pOldTab[ nIndex ];
    }
    if( bOverWriter )
        lcl_setTabStops( mxParaProps, aOldTabs );
    else
        lcl_setTabStops( mxParaProps, aNewTabs );

    return uno::Reference< word::XTabStop >( new SwVbaTabStop( this, mxContext ) );
}

void SAL_CALL SwVbaTabStops::ClearAll()
{
    uno::Sequence< style::TabStop > aSeq;
    lcl_setTabStops( mxParaProps, aSeq );
}

// XEnumerationAccess
uno::Type
SwVbaTabStops::getElementType()
{
    return cppu::UnoType<word::XTabStop>::get();
}
uno::Reference< container::XEnumeration >
SwVbaTabStops::createEnumeration()
{
    return new TabStopsEnumWrapper( m_xIndexAccess );
}

uno::Any
SwVbaTabStops::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource;
}

OUString
SwVbaTabStops::getServiceImplName()
{
    return OUString("SwVbaTabStops");
}

css::uno::Sequence<OUString>
SwVbaTabStops::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        "ooo.vba.word.TabStops"
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
