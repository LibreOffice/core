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
#ifndef _ACCPAGE_HXX
#define _ACCPAGE_HXX

#include "acccontext.hxx"


/**
 * accessibility implementation for the page (SwPageFrm)
 * The page is _only_ visible in the page preview. For the regular
 * document view, it doesn't make sense to add this additional element
 * into the hierarchy. For the page preview, however, the page is the
 * important.
 */
class SwAccessiblePage : public SwAccessibleContext
{
    sal_Bool    bIsSelected;    // protected by base class mutex

    sal_Bool    IsSelected();

    using SwAccessibleFrame::GetBounds;

protected:

    // return the bounding box for the page in page preview mode
    SwRect GetBounds( /* const SwFrm *pFrm =0 */ );

    // Set states for getAccessibleStateSet.
    // This drived class additionaly sets
    // FOCUSABLE(1) and FOCUSED(+)
    virtual void GetStates( ::utl::AccessibleStateSetHelper& rStateSet );

    virtual void _InvalidateCursorPos();
    virtual void _InvalidateFocus();

    virtual ~SwAccessiblePage();

public:
    // convenience constructor to avoid typecast;
    // may only be called with SwPageFrm argument
    SwAccessiblePage( SwAccessibleMap* pInitMap, const SwFrm* pFrame );



    //
    // XAccessibleContext methods that need to be overridden
    //

    virtual ::rtl::OUString SAL_CALL getAccessibleDescription (void)
        throw (::com::sun::star::uno::RuntimeException);

    //
    // XServiceInfo
    //

    virtual ::rtl::OUString SAL_CALL getImplementationName (void)
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService (
        const ::rtl::OUString& sServiceName)
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
        getSupportedServiceNames (void)
        throw (::com::sun::star::uno::RuntimeException);

    //=====  XTypeProvider  ====================================================
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool HasCursor();   // required by map to remember that object
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
