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

#include <extended/AccessibleBrowseBox.hxx>
#include <extended/AccessibleBrowseBoxTable.hxx>
#include <extended/AccessibleBrowseBoxHeaderBar.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <utility>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/accessibletableprovider.hxx>
#include <vcl/unohelp.hxx>
#include <sal/types.h>


namespace accessibility
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;

// Ctor/Dtor/disposing

AccessibleBrowseBox::AccessibleBrowseBox(
            const css::uno::Reference< css::accessibility::XAccessible >& _rxParent, const css::uno::Reference< css::accessibility::XAccessible >& _rxCreator,
            ::vcl::IAccessibleTableProvider& _rBrowseBox )
    : AccessibleBrowseBoxBase( _rxParent, _rBrowseBox,nullptr, AccessibleBrowseBoxObjType::BrowseBox ),
      m_aCreator(_rxCreator)
{
    m_xFocusWindow = VCLUnoHelper::GetInterface(mpBrowseBox->GetWindowInstance());
}

void AccessibleBrowseBox::setCreator( const css::uno::Reference< css::accessibility::XAccessible >& _rxCreator )
{
#if OSL_DEBUG_LEVEL > 0
    css::uno::Reference< css::accessibility::XAccessible > xCreator(m_aCreator);
    OSL_ENSURE( !xCreator.is(), "extended/AccessibleBrowseBox::setCreator: creator already set!" );
#endif
    m_aCreator = _rxCreator;
}


AccessibleBrowseBox::~AccessibleBrowseBox()
{
}


void SAL_CALL AccessibleBrowseBox::disposing()
{
    ::osl::MutexGuard aGuard( getMutex() );

    m_aCreator.clear();

    if ( mxTable.is() )
    {
        mxTable->dispose();
        mxTable.clear();
    }
    if ( mxRowHeaderBar.is() )
    {
        mxRowHeaderBar->dispose();
        mxRowHeaderBar.clear();
    }
    if ( mxColumnHeaderBar.is() )
    {
        mxColumnHeaderBar->dispose();
        mxColumnHeaderBar.clear();
    }

    AccessibleBrowseBoxBase::disposing();
}


// css::accessibility::XAccessibleContext

sal_Int64 SAL_CALL AccessibleBrowseBox::getAccessibleChildCount()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    return vcl::BBINDEX_FIRSTCONTROL + mpBrowseBox->GetAccessibleControlCount();
}


css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
AccessibleBrowseBox::getAccessibleChild( sal_Int64 nChildIndex )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    if (nChildIndex < 0 || nChildIndex >= getAccessibleChildCount())
        throw lang::IndexOutOfBoundsException();

    css::uno::Reference< css::accessibility::XAccessible > xRet;
    if( nChildIndex >= 0 )
    {
        if( nChildIndex < vcl::BBINDEX_FIRSTCONTROL )
            xRet = implGetFixedChild( nChildIndex );
        else
        {
            // additional controls
            nChildIndex -= vcl::BBINDEX_FIRSTCONTROL;
            if( nChildIndex < mpBrowseBox->GetAccessibleControlCount() )
                xRet = mpBrowseBox->CreateAccessibleControl( nChildIndex );
        }
    }

    if( !xRet.is() )
        throw lang::IndexOutOfBoundsException();
    return xRet;
}

// css::accessibility::XAccessibleComponent

css::uno::Reference< css::accessibility::XAccessible > SAL_CALL
AccessibleBrowseBox::getAccessibleAtPoint( const awt::Point& rPoint )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    sal_Int32 nIndex = 0;
    if (mpBrowseBox->ConvertPointToControlIndex(nIndex, vcl::unohelper::ConvertToVCLPoint(rPoint)))
        return mpBrowseBox->CreateAccessibleControl(nIndex);

    // try whether point is in one of the fixed children
    // (table, header bars, corner control)
    Point aPoint(vcl::unohelper::ConvertToVCLPoint(rPoint));
    for (nIndex = 0; nIndex < vcl::BBINDEX_FIRSTCONTROL; ++nIndex)
    {
        css::uno::Reference< css::accessibility::XAccessible > xCurrChild(implGetFixedChild(nIndex));
        css::uno::Reference< css::accessibility::XAccessibleComponent >
            xCurrChildComp( xCurrChild, uno::UNO_QUERY );

        if (xCurrChildComp.is()
            && vcl::unohelper::ConvertToVCLRect(xCurrChildComp->getBounds()).Contains(aPoint))
            return xCurrChild;
    }
    return nullptr;
}


void SAL_CALL AccessibleBrowseBox::grabFocus()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    mpBrowseBox->GrabFocus();
}

// XServiceInfo
OUString SAL_CALL AccessibleBrowseBox::getImplementationName()
{
    return u"com.sun.star.comp.svtools.AccessibleBrowseBox"_ustr;
}

// internal virtual methods
tools::Rectangle AccessibleBrowseBox::implGetBoundingBox()
{
    vcl::Window* pParent = mpBrowseBox->GetAccessibleParentWindow();
    assert(pParent && "implGetBoundingBox - missing parent window");
    return mpBrowseBox->GetWindowExtentsRelative( *pParent );
}

AbsoluteScreenPixelRectangle AccessibleBrowseBox::implGetBoundingBoxOnScreen()
{
    return mpBrowseBox->GetWindowExtentsAbsolute();
}

// internal helper methods
css::uno::Reference< css::accessibility::XAccessible > AccessibleBrowseBox::implGetTable()
{
    if( !mxTable.is() )
    {
        mxTable = createAccessibleTable();

    }
    return mxTable;
}

css::uno::Reference< css::accessibility::XAccessible >
AccessibleBrowseBox::implGetHeaderBar(AccessibleBrowseBoxObjType eObjType)
{
    if( eObjType == AccessibleBrowseBoxObjType::RowHeaderBar )
    {
        if (!mxRowHeaderBar.is())
            mxRowHeaderBar = new AccessibleBrowseBoxHeaderBar(m_aCreator, *mpBrowseBox, eObjType);
        return mxRowHeaderBar;
    }
    else if( eObjType == AccessibleBrowseBoxObjType::ColumnHeaderBar )
    {
        if (!mxColumnHeaderBar.is())
            mxColumnHeaderBar = new AccessibleBrowseBoxHeaderBar(m_aCreator, *mpBrowseBox, eObjType);
        return mxColumnHeaderBar;
    }

    return css::uno::Reference<css::accessibility::XAccessible>();
}

css::uno::Reference< css::accessibility::XAccessible >
AccessibleBrowseBox::implGetFixedChild( sal_Int64 nChildIndex )
{
    css::uno::Reference< css::accessibility::XAccessible > xRet;
    switch( nChildIndex )
    {
        case vcl::BBINDEX_COLUMNHEADERBAR:
            xRet = implGetHeaderBar( AccessibleBrowseBoxObjType::ColumnHeaderBar );
        break;
        case vcl::BBINDEX_ROWHEADERBAR:
            xRet = implGetHeaderBar( AccessibleBrowseBoxObjType::RowHeaderBar );
        break;
        case vcl::BBINDEX_TABLE:
            xRet = implGetTable();
        break;
    }
    return xRet;
}

rtl::Reference<AccessibleBrowseBoxTable> AccessibleBrowseBox::createAccessibleTable()
{
    css::uno::Reference< css::accessibility::XAccessible > xCreator(m_aCreator);
    OSL_ENSURE( xCreator.is(), "extended/AccessibleBrowseBox::createAccessibleTable: my creator died - how this?" );
    return new AccessibleBrowseBoxTable( xCreator, *mpBrowseBox );
}

void AccessibleBrowseBox::commitTableEvent(sal_Int16 _nEventId,const Any& _rNewValue,const Any& _rOldValue)
{
    if ( mxTable.is() )
    {
        mxTable->commitEvent(_nEventId,_rNewValue,_rOldValue);
    }
}

void AccessibleBrowseBox::commitHeaderBarEvent( sal_Int16 _nEventId,
                                                const Any& _rNewValue,
                                                const Any& _rOldValue,bool _bColumnHeaderBar)
{
    rtl::Reference< AccessibleBrowseBoxHeaderBar >& xHeaderBar = _bColumnHeaderBar ? mxColumnHeaderBar : mxRowHeaderBar;
    if ( xHeaderBar.is() )
        xHeaderBar->commitEvent(_nEventId,_rNewValue,_rOldValue);
}


// = AccessibleBrowseBoxAccess

AccessibleBrowseBoxAccess::AccessibleBrowseBoxAccess( css::uno::Reference< css::accessibility::XAccessible > _xParent, ::vcl::IAccessibleTableProvider& _rBrowseBox )
        :m_xParent(std::move( _xParent ))
        ,m_rBrowseBox( _rBrowseBox )
{
}


AccessibleBrowseBoxAccess::~AccessibleBrowseBoxAccess()
{
}


void AccessibleBrowseBoxAccess::dispose()
{
    std::unique_lock aGuard( m_aMutex );

    if (m_xContext.is())
    {
        m_xContext->dispose();
        m_xContext.clear();
    }
    m_xParent.clear();
}


css::uno::Reference< css::accessibility::XAccessibleContext > SAL_CALL AccessibleBrowseBoxAccess::getAccessibleContext()
{
    std::unique_lock aGuard( m_aMutex );

    // if the context died meanwhile (there is no listener, so it won't tell us explicitly when this happens),
    // then reset and re-create.
    if ( m_xContext.is() && !m_xContext->isAlive() )
        m_xContext = nullptr;

    if ( !m_xContext.is() )
        m_xContext = new AccessibleBrowseBox( m_xParent, this, m_rBrowseBox );

    return m_xContext;
}



} // namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
