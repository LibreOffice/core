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
#ifndef _ACCSELECTIONHELPER_HXX_
#define _ACCSELECTIONHELPER_HXX_
class SwAccessibleContext;
class SwFEShell;
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>

class SwAccessibleSelectionHelper
{
    /// the context on which this helper works
    SwAccessibleContext& rContext;


    /// get FE-Shell
    SwFEShell* GetFEShell();

    void throwIndexOutOfBoundsException()
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException );

public:

    SwAccessibleSelectionHelper( SwAccessibleContext& rContext );
    ~SwAccessibleSelectionHelper();


    //=====  XAccessibleSelection  ============================================

    void selectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );

    sal_Bool isAccessibleChildSelected(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );
    void clearAccessibleSelection(  )
        throw ( ::com::sun::star::uno::RuntimeException );
    void selectAllAccessibleChildren(  )
        throw ( ::com::sun::star::uno::RuntimeException );
    sal_Int32 getSelectedAccessibleChildCount(  )
        throw ( ::com::sun::star::uno::RuntimeException );
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > getSelectedAccessibleChild(
        sal_Int32 nSelectedChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException);
    // --> OD 2004-11-16 #111714# - index has to be treated as global child index.
    void deselectAccessibleChild(
        sal_Int32 nChildIndex )
        throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
                ::com::sun::star::uno::RuntimeException );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
