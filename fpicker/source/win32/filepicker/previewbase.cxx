/*************************************************************************
 *
 *  $RCSfile: previewbase.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2002-03-21 07:12:04 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

sal_Int16 SAL_CALL PreviewBase::getImaginaryShowState() const
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
