/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"
#include "previewbase.hxx"

#ifndef _COM_SUN_STAR_UI_DIALOG_FILEPREVIEWIMAGEFORMATS_HPP_
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

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
            OUString::createFromAscii("unsupported image format"), 0, 1);

    if (aImage.hasValue() && (aImage.getValueType() != getCppuType((Sequence<sal_Int8>*)0)))
        throw IllegalArgumentException(
            OUString::createFromAscii("invalid image data"), 0, 2);

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
