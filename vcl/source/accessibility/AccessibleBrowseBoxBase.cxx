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

#include <utility>
#include <vcl/accessibletableprovider.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/accessibility/AccessibleBrowseBoxBase.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>


using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;

using namespace com::sun::star::accessibility::AccessibleStateType;


// Ctor/Dtor/disposing

AccessibleBrowseBoxBase::AccessibleBrowseBoxBase(
    const css::uno::Reference<css::accessibility::XAccessible>& xParent,
    ::vcl::IAccessibleTableProvider& rBrowseBox,
    const css::uno::Reference<css::awt::XWindow>& xFocusWindow, AccessibleBrowseBoxObjType eObjType)
    : AccessibleBrowseBoxBase(xParent, rBrowseBox, xFocusWindow, eObjType,
                              rBrowseBox.GetAccessibleObjectName(eObjType),
                              rBrowseBox.GetAccessibleObjectDescription(eObjType))
{
}

AccessibleBrowseBoxBase::AccessibleBrowseBoxBase(
        css::uno::Reference< css::accessibility::XAccessible >  rxParent,
        ::vcl::IAccessibleTableProvider&                      rBrowseBox,
        css::uno::Reference< css::awt::XWindow >  _xFocusWindow,
        AccessibleBrowseBoxObjType      eObjType,
        OUString           rName,
        OUString           rDescription ) :
    ImplInheritanceHelper(),
    mxParent(std::move( rxParent )),
    mpBrowseBox( &rBrowseBox ),
    m_xFocusWindow(std::move(_xFocusWindow)),
    maName(std::move( rName )),
    maDescription(std::move( rDescription )),
    meObjType( eObjType )
{
    if ( m_xFocusWindow.is() )
        m_xFocusWindow->addFocusListener( this );
}

void SAL_CALL AccessibleBrowseBoxBase::disposing()
{
    ::osl::MutexGuard aGuard( getMutex() );
    if ( m_xFocusWindow.is() )
    {
        SolarMutexGuard aSolarGuard;
        m_xFocusWindow->removeFocusListener( this );
    }

    OAccessibleComponentHelper::disposing();

    mxParent = nullptr;
    mpBrowseBox = nullptr;
}

css::awt::Rectangle AccessibleBrowseBoxBase::implGetBounds()
{
    return vcl::unohelper::ConvertToAWTRect(implGetBoundingBox());
}

// css::accessibility::XAccessible

Reference<css::accessibility::XAccessibleContext>
    SAL_CALL AccessibleBrowseBoxBase::getAccessibleContext()
{
    osl::MutexGuard aGuard(getMutex());
    ensureIsAlive();
    return this;
}

// css::accessibility::XAccessibleContext

Reference< css::accessibility::XAccessible > SAL_CALL AccessibleBrowseBoxBase::getAccessibleParent()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    return mxParent;
}

sal_Int64 SAL_CALL AccessibleBrowseBoxBase::getAccessibleIndexInParent()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();

    // -1 for child not found/no parent (according to specification)
    sal_Int64 nRet = -1;

    css::uno::Reference< uno::XInterface > xMeMyselfAndI( static_cast< css::accessibility::XAccessibleContext* >( this ), uno::UNO_QUERY );

    //  iterate over parent's children and search for this object
    if( mxParent.is() )
    {
        css::uno::Reference< css::accessibility::XAccessibleContext >
            xParentContext( mxParent->getAccessibleContext() );
        if( xParentContext.is() )
        {
            css::uno::Reference< uno::XInterface > xChild;

            sal_Int64 nChildCount = xParentContext->getAccessibleChildCount();
            for( sal_Int64 nChild = 0; nChild < nChildCount; ++nChild )
            {
                xChild.set(xParentContext->getAccessibleChild( nChild ), css::uno::UNO_QUERY);

                if ( xMeMyselfAndI.get() == xChild.get() )
                {
                    nRet = nChild;
                    break;
                }
            }
        }
    }
    return nRet;
}

OUString SAL_CALL AccessibleBrowseBoxBase::getAccessibleDescription()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    return maDescription;
}

OUString SAL_CALL AccessibleBrowseBoxBase::getAccessibleName()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    return maName;
}

Reference< css::accessibility::XAccessibleRelationSet > SAL_CALL
AccessibleBrowseBoxBase::getAccessibleRelationSet()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    // BrowseBox does not have relations.
    return new utl::AccessibleRelationSetHelper;
}

sal_Int64 SAL_CALL
AccessibleBrowseBoxBase::getAccessibleStateSet()
{
    SolarMethodGuard aGuard( getMutex() );
    // don't check whether alive -> StateSet may contain DEFUNC
    return implCreateStateSet();
}

lang::Locale SAL_CALL AccessibleBrowseBoxBase::getLocale()
{
    ::osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    if( mxParent.is() )
    {
        css::uno::Reference< css::accessibility::XAccessibleContext >
            xParentContext( mxParent->getAccessibleContext() );
        if( xParentContext.is() )
            return xParentContext->getLocale();
    }
    throw IllegalAccessibleComponentStateException();
}

// css::accessibility::XAccessibleComponent

void SAL_CALL AccessibleBrowseBoxBase::focusGained( const css::awt::FocusEvent& )
{
    css::uno::Any aFocused;
    css::uno::Any aEmpty;
    aFocused <<= FOCUSED;

    commitEvent(AccessibleEventId::STATE_CHANGED,aFocused,aEmpty);
}


void SAL_CALL AccessibleBrowseBoxBase::focusLost( const css::awt::FocusEvent& )
{
    css::uno::Any aFocused;
    css::uno::Any aEmpty;
    aFocused <<= FOCUSED;

    commitEvent(AccessibleEventId::STATE_CHANGED,aEmpty,aFocused);
}

// XTypeProvider

Sequence< sal_Int8 > SAL_CALL AccessibleBrowseBoxBase::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// XServiceInfo

sal_Bool SAL_CALL AccessibleBrowseBoxBase::supportsService(
        const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL AccessibleBrowseBoxBase::getSupportedServiceNames()
{
    return { u"com.sun.star.accessibility.AccessibleContext"_ustr };
}

// other public methods

void AccessibleBrowseBoxBase::setAccessibleName( const OUString& rName )
{
    ::osl::ClearableMutexGuard aGuard( getMutex() );
    Any aOld;
    aOld <<= maName;
    maName = rName;

    aGuard.clear();

    commitEvent(
        AccessibleEventId::NAME_CHANGED,
        uno::Any( maName ),
        aOld );
}

void AccessibleBrowseBoxBase::setAccessibleDescription( const OUString& rDescription )
{
    ::osl::ClearableMutexGuard aGuard( getMutex() );
    Any aOld;
    aOld <<= maDescription;
    maDescription = rDescription;

    aGuard.clear();

    commitEvent(
        AccessibleEventId::DESCRIPTION_CHANGED,
        uno::Any( maDescription ),
        aOld );
}

// internal virtual methods

bool AccessibleBrowseBoxBase::implIsShowing()
{
    bool bShowing = false;
    if( mxParent.is() )
    {
        css::uno::Reference< css::accessibility::XAccessibleComponent >
            xParentComp( mxParent->getAccessibleContext(), uno::UNO_QUERY );
        if( xParentComp.is() )
            bShowing = implGetBoundingBox().Overlaps(
                vcl::unohelper::ConvertToVCLRect(xParentComp->getBounds()));
    }
    return bShowing;
}

sal_Int64 AccessibleBrowseBoxBase::implCreateStateSet()
{
    sal_Int64 nStateSet = 0;

    if( isAlive() )
    {
        // SHOWING done with mxParent
        if( implIsShowing() )
            nStateSet |= AccessibleStateType::SHOWING;
        // BrowseBox fills StateSet with states depending on object type
        mpBrowseBox->FillAccessibleStateSet( nStateSet, getType() );
    }
    else
        nStateSet |= AccessibleStateType::DEFUNC;

    return nStateSet;
}

// internal helper methods

bool AccessibleBrowseBoxBase::isAlive() const
{
    return !rBHelper.bDisposed && !rBHelper.bInDispose && mpBrowseBox;
}

void AccessibleBrowseBoxBase::ensureIsAlive() const
{
    if( !isAlive() )
        throw lang::DisposedException();
}

void AccessibleBrowseBoxBase::commitEvent(
        sal_Int16 _nEventId, const Any& _rNewValue, const Any& _rOldValue )
{
    NotifyAccessibleEvent(_nEventId, _rOldValue, _rNewValue);
}

sal_Int16 SAL_CALL AccessibleBrowseBoxBase::getAccessibleRole()
{
    osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    sal_Int16 nRole = AccessibleRole::UNKNOWN;
    switch ( meObjType )
    {
        case AccessibleBrowseBoxObjType::RowHeaderCell:
            nRole = AccessibleRole::ROW_HEADER;
            break;
        case AccessibleBrowseBoxObjType::ColumnHeaderCell:
            nRole = AccessibleRole::COLUMN_HEADER;
            break;
        case AccessibleBrowseBoxObjType::ColumnHeaderBar:
        case AccessibleBrowseBoxObjType::RowHeaderBar:
        case AccessibleBrowseBoxObjType::Table:
            nRole = AccessibleRole::TABLE;
            break;
        case AccessibleBrowseBoxObjType::TableCell:
            nRole = AccessibleRole::TABLE_CELL;
            break;
        case AccessibleBrowseBoxObjType::BrowseBox:
            nRole = AccessibleRole::PANEL;
            break;
        case AccessibleBrowseBoxObjType::CheckBoxCell:
            nRole = AccessibleRole::CHECK_BOX;
            break;
    }
    return nRole;
}

Reference<XAccessible > SAL_CALL AccessibleBrowseBoxBase::getAccessibleAtPoint( const css::awt::Point& )
{
    return nullptr;
}

void SAL_CALL AccessibleBrowseBoxBase::disposing( const css::lang::EventObject& )
{
    m_xFocusWindow = nullptr;
}

sal_Int32 SAL_CALL AccessibleBrowseBoxBase::getForeground(  )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    Color nColor;
    vcl::Window* pInst = mpBrowseBox->GetWindowInstance();
    if ( pInst )
    {
        if ( pInst->IsControlForeground() )
            nColor = pInst->GetControlForeground();
        else
        {
            vcl::Font aFont;
            if ( pInst->IsControlFont() )
                aFont = pInst->GetControlFont();
            else
                aFont = pInst->GetFont();
            nColor = aFont.GetColor();
        }
    }

    return sal_Int32(nColor);
}

sal_Int32 SAL_CALL AccessibleBrowseBoxBase::getBackground(  )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();

    Color nColor;
    vcl::Window* pInst = mpBrowseBox->GetWindowInstance();
    if ( pInst )
    {
        if ( pInst->IsControlBackground() )
            nColor = pInst->GetControlBackground();
        else
            nColor = pInst->GetBackground().GetColor();
    }

    return sal_Int32(nColor);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
