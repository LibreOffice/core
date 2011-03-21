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

#ifndef _PREVIEWBASE_HXX_
#define _PREVIEWBASE_HXX_

#include <sal/types.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/Any.hxx>

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//---------------------------------------------
// Common interface for previews
//---------------------------------------------

class PreviewBase
{
public:
    PreviewBase();

    // dtor
    virtual ~PreviewBase();

    virtual sal_Int32 SAL_CALL getTargetColorDepth()
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAvailableWidth()
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getAvailableHeight()
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setImage( sal_Int16 aImageFormat, const ::com::sun::star::uno::Any& aImage )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL setShowState( sal_Bool bShowState )
        throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL getShowState()
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL getImage(sal_Int16& aImageFormat,com::sun::star::uno::Any& aImage);

    sal_Bool SAL_CALL getImaginaryShowState() const;

    virtual HWND SAL_CALL getWindowHandle() const;

protected:
    ::com::sun::star::uno::Any m_ImageData;
    sal_Int16                  m_ImageFormat;
    sal_Bool                   m_bShowState;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
