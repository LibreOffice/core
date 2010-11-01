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

#ifndef _PREVIEWADAPTER_HXX_
#define _PREVIEWADAPTER_HXX_

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
#include <memory>

// forward declaration
class CPreviewAdapterImpl;

//---------------------------------------------
// A kind of a facade for the preview class.
// We want to hide the fact that the preview
// window may only become visible if there is
// a valid parent window (means, the FilePicker)
// is in execution mode. So unless someone sets
// the preview active with a valid parent
// window the preview may not be visible
//---------------------------------------------

class CPreviewAdapter
{
public:

    // ctor
    CPreviewAdapter(HINSTANCE instance);

    ~CPreviewAdapter();

    ::com::sun::star::uno::Sequence<sal_Int16> SAL_CALL getSupportedImageFormats();

    sal_Int32 SAL_CALL getTargetColorDepth();

    sal_Int32 SAL_CALL getAvailableWidth();

    sal_Int32 SAL_CALL getAvailableHeight();

    void SAL_CALL setImage(sal_Int16 aImageFormat, const ::com::sun::star::uno::Any& aImage)
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    sal_Bool SAL_CALL setShowState(sal_Bool bShowState);

    sal_Bool SAL_CALL getShowState();

    void SAL_CALL setParent(HWND parent);

    //--------------------------------------
    // notification from parent
    //--------------------------------------

    void SAL_CALL notifyParentShow(bool bShow);

    void SAL_CALL notifyParentSizeChanged();

    void SAL_CALL notifyParentWindowPosChanged();

private:
    // hide implementation details using the
    // bridge pattern
    std::auto_ptr<CPreviewAdapterImpl> m_pImpl;

// prevent copy and assignment
private:
    CPreviewAdapter(const CPreviewAdapter&);
    CPreviewAdapter& operator=(const CPreviewAdapter&);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
