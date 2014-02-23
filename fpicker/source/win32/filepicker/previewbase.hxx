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


// Common interface for previews


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
