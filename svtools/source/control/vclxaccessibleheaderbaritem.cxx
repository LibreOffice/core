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
#include <vclxaccessibleheaderbaritem.hxx>

#include <svtools/headbar.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <toolkit/awt/vclxfont.hxx>
#include <toolkit/helper/externallock.hxx>
#include <toolkit/helper/convert.hxx>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


//  class AccessibleTabBar


VCLXAccessibleHeaderBarItem::VCLXAccessibleHeaderBarItem( HeaderBar*    pHeadBar, sal_Int32 _nIndexInParent )
    :OAccessibleExtendedComponentHelper( new VCLExternalSolarLock() )
    ,m_pHeadBar( pHeadBar )
    ,m_nIndexInParent(_nIndexInParent + 1)

{
    m_pExternalLock = static_cast< VCLExternalSolarLock* >( getExternalLock() );
}

VCLXAccessibleHeaderBarItem::~VCLXAccessibleHeaderBarItem()
{
    delete m_pExternalLock;
    m_pExternalLock = nullptr;
}

void VCLXAccessibleHeaderBarItem::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    if ( m_pHeadBar )
    {
        if ( m_pHeadBar->IsEnabled() )
            rStateSet.AddState( AccessibleStateType::ENABLED );

        if ( m_pHeadBar->IsVisible() )
        {
            rStateSet.AddState( AccessibleStateType::VISIBLE );
        }
        rStateSet.AddState( AccessibleStateType::SELECTABLE );
        rStateSet.AddState( AccessibleStateType::RESIZABLE );
    }
}

// OCommonAccessibleComponent
awt::Rectangle VCLXAccessibleHeaderBarItem::implGetBounds()
{
    awt::Rectangle aBounds;
    OExternalLockGuard aGuard( this );

    if ( m_pHeadBar )
        aBounds = AWTRectangle( m_pHeadBar->GetItemRect( sal_uInt16( m_nIndexInParent ) ) );

    return aBounds;
}


// XInterface


IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleHeaderBarItem, OAccessibleExtendedComponentHelper, VCLXAccessibleHeaderBarItem_BASE )


// XTypeProvider


IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleHeaderBarItem, OAccessibleExtendedComponentHelper, VCLXAccessibleHeaderBarItem_BASE )


// XServiceInfo
OUString VCLXAccessibleHeaderBarItem::getImplementationName()
{
    return OUString("com.sun.star.comp.svtools.AccessibleHeaderBarItem");
}

sal_Bool VCLXAccessibleHeaderBarItem::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

Sequence< OUString > VCLXAccessibleHeaderBarItem::getSupportedServiceNames()
{
    Sequence< OUString > aNames { "com.sun.star.awt.AccessibleHeaderBarItem" };
    return aNames;
}

// XAccessible
Reference< XAccessibleContext > VCLXAccessibleHeaderBarItem::getAccessibleContext()
{
    OExternalLockGuard aGuard( this );

    return this;
}


// XAccessibleContext


sal_Int32 VCLXAccessibleHeaderBarItem::getAccessibleChildCount()
{
    OExternalLockGuard aGuard( this );

    return 0;
}


Reference< XAccessible > VCLXAccessibleHeaderBarItem::getAccessibleChild( sal_Int32 i )
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    return Reference< XAccessible >();
}


Reference< XAccessible > VCLXAccessibleHeaderBarItem::getAccessibleParent()
{
    OExternalLockGuard aGuard( this );

    Reference< XAccessible > xParent;
    if ( m_pHeadBar )
    {
        xParent = m_pHeadBar->GetAccessible();
    }

    return xParent;
}


sal_Int32 VCLXAccessibleHeaderBarItem::getAccessibleIndexInParent()
{
    OExternalLockGuard aGuard( this );
    return m_nIndexInParent - 1;
}


sal_Int16 VCLXAccessibleHeaderBarItem::getAccessibleRole()
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::COLUMN_HEADER;
}


OUString VCLXAccessibleHeaderBarItem::getAccessibleDescription()
{
    OExternalLockGuard aGuard( this );
    OUString sDescription;
    return sDescription;
}


OUString VCLXAccessibleHeaderBarItem::getAccessibleName()
{
    OExternalLockGuard aGuard( this );

    OUString sName;
    if(m_pHeadBar)
        sName = m_pHeadBar->GetItemText( sal_uInt16( m_nIndexInParent ) );
    return sName;
}


Reference< XAccessibleRelationSet > VCLXAccessibleHeaderBarItem::getAccessibleRelationSet(  )
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
    return xSet;
}


Reference< XAccessibleStateSet > VCLXAccessibleHeaderBarItem::getAccessibleStateSet(  )
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
    Reference< XAccessibleStateSet > xSet = pStateSetHelper;

    if ( !rBHelper.bDisposed && !rBHelper.bInDispose )
    {
        FillAccessibleStateSet( *pStateSetHelper );
    }
    else
    {
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );
    }

    return xSet;
}


css::lang::Locale VCLXAccessibleHeaderBarItem::getLocale()
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetLanguageTag().getLocale();
}


// XAccessibleComponent


Reference< XAccessible > VCLXAccessibleHeaderBarItem::getAccessibleAtPoint( const awt::Point& )
{
    OExternalLockGuard aGuard( this );

    return Reference< XAccessible >();
}


sal_Int32 VCLXAccessibleHeaderBarItem::getForeground()
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    return nColor;
}


sal_Int32 VCLXAccessibleHeaderBarItem::getBackground()
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    return nColor;
}


// XAccessibleExtendedComponent


Reference< awt::XFont > VCLXAccessibleHeaderBarItem::getFont()
{
    OExternalLockGuard aGuard( this );

    Reference< awt::XFont > xFont;
    return xFont;
}


OUString VCLXAccessibleHeaderBarItem::getTitledBorderText()
{
    OExternalLockGuard aGuard( this );

    OUString sText;
    return sText;
}


OUString VCLXAccessibleHeaderBarItem::getToolTipText()
{
    OExternalLockGuard aGuard( this );

    OUString sText;
    if ( m_pHeadBar )
        sText = m_pHeadBar->GetQuickHelpText();

    return sText;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
