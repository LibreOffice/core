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
#include <standard/vclxaccessibleheaderbaritem.hxx>

#include <vcl/headbar.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <toolkit/helper/convert.hxx>
#include <i18nlangtag/languagetag.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;




VCLXAccessibleHeaderBarItem::VCLXAccessibleHeaderBarItem( HeaderBar*    pHeadBar, sal_Int32 _nIndexInParent )
    :m_pHeadBar( pHeadBar )
    ,m_nIndexInParent(_nIndexInParent + 1)

{
}

VCLXAccessibleHeaderBarItem::~VCLXAccessibleHeaderBarItem()
{
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
    return "com.sun.star.comp.svtools.AccessibleHeaderBarItem";
}

sal_Bool VCLXAccessibleHeaderBarItem::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

Sequence< OUString > VCLXAccessibleHeaderBarItem::getSupportedServiceNames()
{
    return { "com.sun.star.awt.AccessibleHeaderBarItem" };
}

// XAccessible
Reference< XAccessibleContext > VCLXAccessibleHeaderBarItem::getAccessibleContext()
{
    return this;
}


// XAccessibleContext


sal_Int32 VCLXAccessibleHeaderBarItem::getAccessibleChildCount()
{
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
    return AccessibleRole::COLUMN_HEADER;
}


OUString VCLXAccessibleHeaderBarItem::getAccessibleDescription()
{
    return OUString();
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

    return new utl::AccessibleRelationSetHelper;
}


Reference< XAccessibleStateSet > VCLXAccessibleHeaderBarItem::getAccessibleStateSet(  )
{
    OExternalLockGuard aGuard( this );

    rtl::Reference<utl::AccessibleStateSetHelper> pStateSetHelper = new utl::AccessibleStateSetHelper;

    if ( !rBHelper.bDisposed && !rBHelper.bInDispose )
    {
        FillAccessibleStateSet( *pStateSetHelper );
    }
    else
    {
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );
    }

    return pStateSetHelper;
}


css::lang::Locale VCLXAccessibleHeaderBarItem::getLocale()
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetLanguageTag().getLocale();
}


// XAccessibleComponent


Reference< XAccessible > VCLXAccessibleHeaderBarItem::getAccessibleAtPoint( const awt::Point& )
{
    return Reference< XAccessible >();
}


sal_Int32 VCLXAccessibleHeaderBarItem::getForeground()
{
    return 0;
}


sal_Int32 VCLXAccessibleHeaderBarItem::getBackground()
{
    return 0;
}


// XAccessibleExtendedComponent


Reference< awt::XFont > VCLXAccessibleHeaderBarItem::getFont()
{
    return Reference< awt::XFont >();
}


OUString VCLXAccessibleHeaderBarItem::getTitledBorderText()
{
    return OUString();
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
