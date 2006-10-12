/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: previewbase.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 10:54:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"

#ifndef _PREVIEWBASE_HXX_
#include "previewbase.hxx"
#endif

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
