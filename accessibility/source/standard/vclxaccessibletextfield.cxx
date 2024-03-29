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

#include <standard/vclxaccessibletextfield.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/toolkit/lstbox.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;


VCLXAccessibleTextField::VCLXAccessibleTextField (VCLXWindow* pVCLWindow, const Reference< XAccessible >& _xParent) :
    ImplInheritanceHelper (pVCLWindow),
    m_xParent( _xParent )

{
}


OUString VCLXAccessibleTextField::implGetText()
{
    OUString aText;
    VclPtr< ListBox > pListBox = GetAs< ListBox >();
    if (pListBox && !pListBox->IsInDropDown())
        aText = pListBox->GetSelectedEntry();

    return aText;
}


// XAccessible

Reference<XAccessibleContext> SAL_CALL
    VCLXAccessibleTextField::getAccessibleContext()
{
    return this;
}


// XAccessibleContext

sal_Int64 SAL_CALL VCLXAccessibleTextField::getAccessibleChildCount()
{
    return 0;
}


Reference<XAccessible> SAL_CALL VCLXAccessibleTextField::getAccessibleChild (sal_Int64)
{
    throw IndexOutOfBoundsException();
}


sal_Int16 SAL_CALL VCLXAccessibleTextField::getAccessibleRole()
{
    return AccessibleRole::TEXT;
}

Reference< XAccessible > SAL_CALL VCLXAccessibleTextField::getAccessibleParent(  )
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    return m_xParent;
}


// XServiceInfo

OUString VCLXAccessibleTextField::getImplementationName()
{
    return u"com.sun.star.comp.toolkit.AccessibleTextField"_ustr;
}


Sequence< OUString > VCLXAccessibleTextField::getSupportedServiceNames()
{
    return comphelper::concatSequences(VCLXAccessibleTextComponent::getSupportedServiceNames(),
                                       Sequence<OUString>{u"com.sun.star.accessibility.AccessibleTextField"_ustr});
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
