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

#include <sal/config.h>

#include <memory>
#include <utility>

#include "AccessibleTextHelper.hxx"
#include "DrawViewWrapper.hxx"

#include <vcl/svapp.hxx>

#include <svx/AccessibleTextHelper.hxx>
#include <svx/unoshtxt.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <o3tl/make_unique.hxx>
using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

AccessibleTextHelper::AccessibleTextHelper(
    DrawViewWrapper * pDrawViewWrapper ) :
        impl::AccessibleTextHelper_Base( m_aMutex ),
        m_pTextHelper( nullptr ),
        m_pDrawViewWrapper( pDrawViewWrapper )
{}

AccessibleTextHelper::~AccessibleTextHelper()
{
}

// ____ XInitialization ____
void SAL_CALL AccessibleTextHelper::initialize( const Sequence< uno::Any >& aArguments )
{
    OUString aCID;
    Reference< XAccessible > xEventSource;
    Reference< awt::XWindow > xWindow;

    if( aArguments.getLength() >= 3 )
    {
        aArguments[0] >>= aCID;
        aArguments[1] >>= xEventSource;
        aArguments[2] >>= xWindow;
    }
    OSL_ENSURE( !aCID.isEmpty(), "Empty CID" );
    OSL_ENSURE( xEventSource.is(), "Empty Event Source" );
    OSL_ENSURE( xWindow.is(), "Empty Window" );
    if( !xEventSource.is() || aCID.isEmpty() )
        return;

    SolarMutexGuard aSolarGuard;

    m_pTextHelper.reset();

    VclPtr<vcl::Window> pWindow( VCLUnoHelper::GetWindow( xWindow ));
    if( pWindow )
    {
        SdrView * pView = m_pDrawViewWrapper;
        if( pView )
        {
            SdrObject * pTextObj = m_pDrawViewWrapper->getNamedSdrObject( aCID );
            if( pTextObj )
            {
                m_pTextHelper.reset( new ::accessibility::AccessibleTextHelper(o3tl::make_unique<SvxTextEditSource>(*pTextObj, nullptr, *pView, *pWindow)) );
                m_pTextHelper->SetEventSource( xEventSource );
            }
        }
    }

    OSL_ENSURE( m_pTextHelper, "Couldn't create text helper" );
}

// ____ XAccessibleContext ____
::sal_Int32 SAL_CALL AccessibleTextHelper::getAccessibleChildCount()
{
    if( m_pTextHelper )
    {
        SolarMutexGuard aSolarGuard;
        return m_pTextHelper->GetChildCount();
    }
    return 0;
}

Reference< XAccessible > SAL_CALL AccessibleTextHelper::getAccessibleChild( ::sal_Int32 i )
{
    if( m_pTextHelper )
    {
        SolarMutexGuard aSolarGuard;
        return m_pTextHelper->GetChild( i );
    }
    return Reference< XAccessible >();
}

Reference< XAccessible > SAL_CALL AccessibleTextHelper::getAccessibleParent()
{
    OSL_FAIL( "Not implemented in this helper" );
    return Reference< XAccessible >();
}

::sal_Int32 SAL_CALL AccessibleTextHelper::getAccessibleIndexInParent()
{
    OSL_FAIL( "Not implemented in this helper" );
    return -1;
}

::sal_Int16 SAL_CALL AccessibleTextHelper::getAccessibleRole()
{
    OSL_FAIL( "Not implemented in this helper" );
    return AccessibleRole::UNKNOWN;
}

OUString SAL_CALL AccessibleTextHelper::getAccessibleDescription()
{
    OSL_FAIL( "Not implemented in this helper" );
    return OUString();
}

OUString SAL_CALL AccessibleTextHelper::getAccessibleName()
{
    OSL_FAIL( "Not implemented in this helper" );
    return OUString();
}

Reference< XAccessibleRelationSet > SAL_CALL AccessibleTextHelper::getAccessibleRelationSet()
{
    OSL_FAIL( "Not implemented in this helper" );
    return Reference< XAccessibleRelationSet >();
}

Reference< XAccessibleStateSet > SAL_CALL AccessibleTextHelper::getAccessibleStateSet()
{
    OSL_FAIL( "Not implemented in this helper" );
    return Reference< XAccessibleStateSet >();
}

lang::Locale SAL_CALL AccessibleTextHelper::getLocale()
{
    OSL_FAIL( "Not implemented in this helper" );
    return lang::Locale();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
