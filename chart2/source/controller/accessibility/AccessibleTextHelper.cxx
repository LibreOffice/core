/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "AccessibleTextHelper.hxx"
#include "DrawViewWrapper.hxx"

#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

#include <svx/AccessibleTextHelper.hxx>
#include <svx/unoshtxt.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

AccessibleTextHelper::AccessibleTextHelper(
    DrawViewWrapper * pDrawViewWrapper ) :
        impl::AccessibleTextHelper_Base( m_aMutex ),
        m_pTextHelper( 0 ),
        m_pDrawViewWrapper( pDrawViewWrapper )
{}

AccessibleTextHelper::~AccessibleTextHelper()
{
    if( m_pTextHelper )
        delete m_pTextHelper;
}

// ____ XInitialization ____
void SAL_CALL AccessibleTextHelper::initialize( const Sequence< uno::Any >& aArguments )
    throw (uno::Exception,
           uno::RuntimeException)
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
    OSL_ENSURE( aCID.getLength() > 0, "Empty CID" );
    OSL_ENSURE( xEventSource.is(), "Empty Event Source" );
    OSL_ENSURE( xWindow.is(), "Empty Window" );
    if( !xEventSource.is() || aCID.getLength() == 0 )
        return;

    SolarMutexGuard aSolarGuard;

    if( m_pTextHelper )
        delete m_pTextHelper;

    Window* pWindow( VCLUnoHelper::GetWindow( xWindow ));
    if( pWindow )
    {
        SdrView * pView = m_pDrawViewWrapper;
        if( pView )
        {
            SdrObject * pTextObj = m_pDrawViewWrapper->getNamedSdrObject( aCID );
            if( pTextObj )
            {
                SvxEditSource * pEditSource = new SvxTextEditSource( *pTextObj, 0, *pView, *pWindow );
                m_pTextHelper = new ::accessibility::AccessibleTextHelper(
                    ::std::auto_ptr< SvxEditSource >( pEditSource ));
                if( m_pTextHelper )
                    m_pTextHelper->SetEventSource( xEventSource );
            }
        }
    }

    OSL_ENSURE( m_pTextHelper, "Couldn't create text helper" );
}

// ____ XAccessibleContext ____
::sal_Int32 SAL_CALL AccessibleTextHelper::getAccessibleChildCount()
    throw (uno::RuntimeException)
{
    if( m_pTextHelper )
    {
        SolarMutexGuard aSolarGuard;
        return m_pTextHelper->GetChildCount();
    }
    return 0;
}

Reference< XAccessible > SAL_CALL AccessibleTextHelper::getAccessibleChild( ::sal_Int32 i )
    throw (lang::IndexOutOfBoundsException,
           uno::RuntimeException)
{
    if( m_pTextHelper )
    {
        SolarMutexGuard aSolarGuard;
        return m_pTextHelper->GetChild( i );
    }
    return Reference< XAccessible >();
}

Reference< XAccessible > SAL_CALL AccessibleTextHelper::getAccessibleParent()
    throw (uno::RuntimeException)
{
    OSL_FAIL( "Not implemented in this helper" );
    return Reference< XAccessible >();
}

::sal_Int32 SAL_CALL AccessibleTextHelper::getAccessibleIndexInParent()
    throw (uno::RuntimeException)
{
    OSL_FAIL( "Not implemented in this helper" );
    return -1;
}

::sal_Int16 SAL_CALL AccessibleTextHelper::getAccessibleRole()
    throw (uno::RuntimeException)
{
    OSL_FAIL( "Not implemented in this helper" );
    return AccessibleRole::UNKNOWN;
}

OUString SAL_CALL AccessibleTextHelper::getAccessibleDescription()
    throw (uno::RuntimeException)
{
    OSL_FAIL( "Not implemented in this helper" );
    return OUString();
}

OUString SAL_CALL AccessibleTextHelper::getAccessibleName()
    throw (uno::RuntimeException)
{
    OSL_FAIL( "Not implemented in this helper" );
    return OUString();
}

Reference< XAccessibleRelationSet > SAL_CALL AccessibleTextHelper::getAccessibleRelationSet()
    throw (uno::RuntimeException)
{
    OSL_FAIL( "Not implemented in this helper" );
    return Reference< XAccessibleRelationSet >();
}

Reference< XAccessibleStateSet > SAL_CALL AccessibleTextHelper::getAccessibleStateSet()
    throw (uno::RuntimeException)
{
    OSL_FAIL( "Not implemented in this helper" );
    return Reference< XAccessibleStateSet >();
}

lang::Locale SAL_CALL AccessibleTextHelper::getLocale()
    throw (IllegalAccessibleComponentStateException,
           uno::RuntimeException)
{
    OSL_FAIL( "Not implemented in this helper" );
    return lang::Locale();
}



} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
