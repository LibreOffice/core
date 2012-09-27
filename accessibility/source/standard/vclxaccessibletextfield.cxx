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
