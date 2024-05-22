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
#include <utility>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

/// @throws uno::RuntimeException
static uno::Sequence< style::TabStop > lcl_getTabStops( const uno::Reference< beans::XPropertySet >& xParaProps )
{
    uno::Sequence< style::TabStop > aSeq;
    xParaProps->getPropertyValue(u"ParaTabStops"_ustr) >>= aSeq;
    return aSeq;
}

/// @throws uno::RuntimeException
static void lcl_setTabStops( const uno::Reference< beans::XPropertySet >& xParaProps, const uno::Sequence< style::TabStop >& aSeq )
{
    xParaProps->setPropertyValue(u"ParaTabStops"_ustr, uno::Any( aSeq ) );
}

namespace {

class TabStopsEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    sal_Int32 m_nIndex;

public:
    explicit TabStopsEnumWrapper( uno::Reference< container::XIndexAccess > xIndexAccess ) : mxIndexAccess(std::move( xIndexAccess )), m_nIndex( 0 )
    {
    }
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return ( m_nIndex < mxIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if( m_nIndex < mxIndexAccess->getCount() )
        {
            return mxIndexAccess->getByIndex( m_nIndex++ );
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
    TabStopCollectionHelper( css::uno::Reference< ov::XHelperInterface > xParent, css::uno::Reference< css::uno::XComponentContext > xContext, const css::uno::Reference< css::beans::XPropertySet >& xParaProps ): mxParent(std::move( xParent )), mxContext(std::move( xContext )), mnTabStops(lcl_getTabStops( xParaProps ).getLength())
    {
    }

    virtual sal_Int32 SAL_CALL getCount(  ) override
    {
        return mnTabStops;
    }
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override
    {
        if ( Index < 0 || Index >= getCount() )
            throw css::lang::IndexOutOfBoundsException();

        return uno::Any( uno::Reference< word::XTabStop >( new SwVbaTabStop( mxParent, mxContext ) ) );
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

}

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
                DebugHelper::basicexception( ERRCODE_BASIC_NOT_IMPLEMENTED, {} );
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
    auto [begin, end] = asNonConstRange(aOldTabs);

    style::TabStop* pOldTab = std::find_if(begin, end,
        [nPosition](const style::TabStop& rTab) { return rTab.Position == nPosition; });
    bool bOverWriter = pOldTab != end;
    if( bOverWriter )
    {
        *pOldTab = aTab;
        lcl_setTabStops( mxParaProps, aOldTabs );
    }
    else
    {
        sal_Int32 nTabs = aOldTabs.getLength();
        uno::Sequence< style::TabStop > aNewTabs( nTabs + 1 );

        auto it = aNewTabs.getArray();
        *it = aTab;
        std::copy(begin, end, std::next(it));
        lcl_setTabStops( mxParaProps, aNewTabs );
    }

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
    return u"SwVbaTabStops"_ustr;
}

css::uno::Sequence<OUString>
SwVbaTabStops::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.word.TabStops"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
