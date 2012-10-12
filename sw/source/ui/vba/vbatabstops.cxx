/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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
#include "vbatabstops.hxx"
#include "vbatabstop.hxx"
#include <com/sun/star/style/TabAlign.hpp>
#include <ooo/vba/word/WdTabLeader.hpp>
#include <ooo/vba/word/WdTabAlignment.hpp>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static uno::Sequence< style::TabStop > lcl_getTabStops( const uno::Reference< beans::XPropertySet >& xParaProps ) throw (uno::RuntimeException)
{
    uno::Sequence< style::TabStop > aSeq;
    xParaProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ParaTabStops") ) ) >>= aSeq;
    return aSeq;
}

static void lcl_setTabStops( const uno::Reference< beans::XPropertySet >& xParaProps, const uno::Sequence< style::TabStop >& aSeq ) throw (uno::RuntimeException)
{
    xParaProps->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ParaTabStops") ), uno::makeAny( aSeq ) );
}

typedef ::cppu::WeakImplHelper2< container::XIndexAccess, container::XEnumerationAccess > TabStopCollectionHelper_Base;

class TabStopsEnumWrapper : public EnumerationHelper_BASE
{
    uno::Reference< container::XIndexAccess > mxIndexAccess;
    sal_Int32 nIndex;

public:
    TabStopsEnumWrapper( const uno::Reference< container::XIndexAccess >& xIndexAccess ) : mxIndexAccess( xIndexAccess ), nIndex( 0 )
    {
    }
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return ( nIndex < mxIndexAccess->getCount() );
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if( nIndex < mxIndexAccess->getCount() )
        {
            return mxIndexAccess->getByIndex( nIndex++ );
        }
        throw container::NoSuchElementException();
    }
};

class TabStopCollectionHelper : public TabStopCollectionHelper_Base
{
private:
    uno::Reference< XHelperInterface > mxParent;
    uno::Reference< uno::XComponentContext > mxContext;
    uno::Reference< beans::XPropertySet > mxParaProps;
    uno::Sequence< style::TabStop > maTabStops;

public:
    TabStopCollectionHelper( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::beans::XPropertySet >& xParaProps ) throw ( css::uno::RuntimeException ): mxParent( xParent ), mxContext( xContext ), mxParaProps( xParaProps )
    {
        maTabStops = lcl_getTabStops( xParaProps );
    }

    virtual ~TabStopCollectionHelper() {}

    virtual sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        return maTabStops.getLength();
    }
    virtual uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( Index < 0 || Index >= getCount() )
            throw css::lang::IndexOutOfBoundsException();

        const style::TabStop* pTabs = maTabStops.getConstArray();
        return uno::makeAny( uno::Reference< word::XTabStop >( new SwVbaTabStop( mxParent, mxContext, mxParaProps, pTabs[ Index ] ) ) );
    }
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
    {
        return word::XTabStop::static_type(0);
    }
    virtual sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        return sal_True;
    }
    // XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException)
    {
        return new TabStopsEnumWrapper( this );
    }
};

SwVbaTabStops::SwVbaTabStops( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext > & xContext, const uno::Reference< beans::XPropertySet >& xParaProps ) throw (uno::RuntimeException) : SwVbaTabStops_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( new TabStopCollectionHelper( xParent, xContext, xParaProps ) ) ), mxParaProps( xParaProps )
{
}

uno::Reference< word::XTabStop > SAL_CALL SwVbaTabStops::Add( float Position, const uno::Any& Alignment, const uno::Any& Leader ) throw (uno::RuntimeException)
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
                DebugHelper::exception( SbERR_NOT_IMPLEMENTED, rtl::OUString() );
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

    sal_Char cDecimal = '.'; // default value

    style::TabStop aTab;
    aTab.Position = nPosition;
    aTab.Alignment = nAlign;
    aTab.DecimalChar = cDecimal;
    aTab.FillChar = cLeader;

    uno::Sequence< style::TabStop > aOldTabs = lcl_getTabStops( mxParaProps );
    sal_Bool bOverWriter = sal_False;

    sal_Int32 nTabs = aOldTabs.getLength();
    uno::Sequence< style::TabStop > aNewTabs( nTabs + 1 );

    style::TabStop* pOldTab = aOldTabs.getArray();
    style::TabStop* pNewTab = aNewTabs.getArray();
    pNewTab[0] = aTab;
    for( sal_Int32 nIndex = 0; nIndex < nTabs && !bOverWriter; nIndex++ )
    {
        if( pOldTab[nIndex].Position == nPosition )
        {
            bOverWriter = sal_True;
            pOldTab[nIndex] = aTab;
            break;
        }
        pNewTab[ nIndex+1 ] = pOldTab[ nIndex ];
    }
    if( bOverWriter )
        lcl_setTabStops( mxParaProps, aOldTabs );
    else
        lcl_setTabStops( mxParaProps, aNewTabs );

    return uno::Reference< word::XTabStop >( new SwVbaTabStop( this, mxContext, mxParaProps, aTab ) );
}

void SAL_CALL SwVbaTabStops::ClearAll() throw (uno::RuntimeException)
{
    uno::Sequence< style::TabStop > aSeq;
    lcl_setTabStops( mxParaProps, aSeq );
}

// XEnumerationAccess
uno::Type
SwVbaTabStops::getElementType() throw (uno::RuntimeException)
{
    return word::XTabStop::static_type(0);
}
uno::Reference< container::XEnumeration >
SwVbaTabStops::createEnumeration() throw (uno::RuntimeException)
{
    return new TabStopsEnumWrapper( m_xIndexAccess );
}

uno::Any
SwVbaTabStops::createCollectionObject( const css::uno::Any& aSource )
{
    return aSource;
}

rtl::OUString
SwVbaTabStops::getServiceImplName()
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SwVbaTabStops"));
}

css::uno::Sequence<rtl::OUString>
SwVbaTabStops::getServiceNames()
{
    static uno::Sequence< rtl::OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.TabStops") );
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
