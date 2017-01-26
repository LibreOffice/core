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

#include "previewbase.hxx"

#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#include <com/sun/star/uno/Sequence.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;


PreviewBase::PreviewBase() :
    m_ImageFormat(css::ui::dialogs::FilePreviewImageFormats::BITMAP),
    m_bShowState(false)
{
}


PreviewBase::~PreviewBase()
{
}


sal_Int32 SAL_CALL PreviewBase::getTargetColorDepth()
{
    return 0;
}


sal_Int32 SAL_CALL PreviewBase::getAvailableWidth()
{
    return 0;
}


sal_Int32 SAL_CALL PreviewBase::getAvailableHeight()
{
    return 0;
}


void SAL_CALL PreviewBase::setImage( sal_Int16 aImageFormat, const css::uno::Any& aImage )
{
    if (aImageFormat != css::ui::dialogs::FilePreviewImageFormats::BITMAP)
        throw IllegalArgumentException(
            "unsupported image format", nullptr, 1);

    if (aImage.hasValue() && (aImage.getValueType() != cppu::UnoType<Sequence<sal_Int8>>::get()))
        throw IllegalArgumentException(
            "invalid image data", nullptr, 2);

     // save the new image data and force a redraw
    m_ImageData   = aImage;
    m_ImageFormat = aImageFormat;
}


void SAL_CALL PreviewBase::getImage(sal_Int16& aImageFormat,css::uno::Any& aImage)
{
    aImageFormat = m_ImageFormat;
    aImage       = m_ImageData;
}


bool SAL_CALL PreviewBase::setShowState( bool bShowState )
{
    m_bShowState = bShowState;
    return true;
}


bool SAL_CALL PreviewBase::getShowState()
{
    return false;
}


bool SAL_CALL PreviewBase::getImaginaryShowState() const
{
    return m_bShowState;
}


HWND SAL_CALL PreviewBase::getWindowHandle() const
{
    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
