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

#include <accessibility/standard/vclxaccessibletextfield.hxx>
#include <vcl/lstbox.hxx>
#include <accessibility/helper/listboxhelper.hxx>

#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <vcl/svapp.hxx>
#include <vcl/combobox.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;




VCLXAccessibleTextField::VCLXAccessibleTextField (VCLXWindow* pVCLWindow, const Reference< XAccessible >& _xParent) :

    VCLXAccessibleTextComponent (pVCLWindow),

    m_xParent( _xParent )

{
}




VCLXAccessibleTextField::~VCLXAccessibleTextField (void)
{
}




OUString VCLXAccessibleTextField::implGetText (void)
{
    OUString aText;
    ListBox* pListBox = static_cast<ListBox*>(GetWindow());
    if (pListBox!=NULL && !pListBox->IsInDropDown())
        aText = pListBox->GetSelectEntry();

    return aText;
}




IMPLEMENT_FORWARD_XINTERFACE2(VCLXAccessibleTextField, VCLXAccessibleTextComponent, VCLXAccessible_BASE)
IMPLEMENT_FORWARD_XTYPEPROVIDER2(VCLXAccessibleTextField, VCLXAccessibleTextComponent, VCLXAccessible_BASE)


//=====  XAccessible  =========================================================

Reference<XAccessibleContext> SAL_CALL
    VCLXAccessibleTextField::getAccessibleContext (void)
    throw (RuntimeException)
{
    return this;
}


//=====  XAccessibleContext  ==================================================

sal_Int32 SAL_CALL VCLXAccessibleTextField::getAccessibleChildCount (void)
    throw (RuntimeException)
{
    return 0;
}




Reference<XAccessible> SAL_CALL VCLXAccessibleTextField::getAccessibleChild (sal_Int32)
    throw (IndexOutOfBoundsException, RuntimeException)
{
    throw IndexOutOfBoundsException();
}




sal_Int16 SAL_CALL VCLXAccessibleTextField::getAccessibleRole (void)
    throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return AccessibleRole::TEXT;
}

Reference< XAccessible > SAL_CALL VCLXAccessibleTextField::getAccessibleParent(  )
    throw (RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return m_xParent;
}



//===== XServiceInfo ==========================================================

OUString VCLXAccessibleTextField::getImplementationName (void)
    throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleTextField" );
}




Sequence< OUString > VCLXAccessibleTextField::getSupportedServiceNames (void)
    throw (RuntimeException)
{
    Sequence< OUString > aNames = VCLXAccessibleTextComponent::getSupportedServiceNames();
    sal_Int32 nLength = aNames.getLength();
    aNames.realloc( nLength + 1 );
    aNames[nLength] = "com.sun.star.accessibility.AccessibleTextField";
    return aNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
