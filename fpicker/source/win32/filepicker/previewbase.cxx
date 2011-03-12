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
#include "precompiled_fpicker.hxx"
#include "previewbase.hxx"

#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

using ::rtl::OUString;

//-------------------------------
//
//-------------------------------

PreviewBase::PreviewBase() :
    m_ImageFormat(::com::sun::star::ui::dialogs::FilePreviewImageFormats::BITMAP),
    m_bShowState(sal_False)
{
}

//-------------------------------
//
//-------------------------------

PreviewBase::~PreviewBase()
{
}

//-------------------------------
//
//-------------------------------

sal_Int32 SAL_CALL PreviewBase::getTargetColorDepth() throw (RuntimeException)
{
    return 0;
}

//-------------------------------
//
//-------------------------------

sal_Int32 SAL_CALL PreviewBase::getAvailableWidth() throw (RuntimeException)
{
    return 0;
}

//-------------------------------
//
//-------------------------------

sal_Int32 SAL_CALL PreviewBase::getAvailableHeight() throw (RuntimeException)
{
    return 0;
}

//-------------------------------
//
//-------------------------------

void SAL_CALL PreviewBase::setImage( sal_Int16 aImageFormat, const ::com::sun::star::uno::Any& aImage )
    throw (IllegalArgumentException, RuntimeException)
{
    if (aImageFormat != ::com::sun::star::ui::dialogs::FilePreviewImageFormats::BITMAP)
        throw IllegalArgumentException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("unsupported image format")), 0, 1);

    if (aImage.hasValue() && (aImage.getValueType() != getCppuType((Sequence<sal_Int8>*)0)))
        throw IllegalArgumentException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("invalid image data")), 0, 2);

     // save the new image data and force a redraw
    m_ImageData   = aImage;
    m_ImageFormat = aImageFormat;
}

//-------------------------------
//
//-------------------------------

void SAL_CALL PreviewBase::getImage(sal_Int16& aImageFormat,com::sun::star::uno::Any& aImage)
{
    aImageFormat = m_ImageFormat;
    aImage       = m_ImageData;
}

//-------------------------------
//
//-------------------------------

sal_Bool SAL_CALL PreviewBase::setShowState( sal_Bool bShowState ) throw (RuntimeException)
{
    m_bShowState = bShowState;
    return sal_True;
}

//-------------------------------
//
//-------------------------------

sal_Bool SAL_CALL PreviewBase::getShowState() throw (RuntimeException)
{
    return sal_False;
}

//-------------------------------
//
//-------------------------------

sal_Bool SAL_CALL PreviewBase::getImaginaryShowState() const
{
    return m_bShowState;
}

//-------------------------------
//
//-------------------------------

HWND SAL_CALL PreviewBase::getWindowHandle() const
{
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
