/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: imagealign.hxx,v $
 * $Revision: 1.4 $
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

#ifndef TOOLKIT_INC_TOOLKIT_HELPER_IMAGEALIGN_HXX
#define TOOLKIT_INC_TOOLKIT_HELPER_IMAGEALIGN_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <sal/types.h>

#include <vcl/button.hxx>

//........................................................................
namespace toolkit
{
//........................................................................

    /** translates a VCL ImageAlign value into an css.awt.ImagePosition value
    */
    sal_Int16 translateImagePosition( ImageAlign _eVCLAlign );

    /** translates a css.awt.ImagePosition value into an VCL ImageAlign
    */
    ImageAlign translateImagePosition( sal_Int16 _nImagePosition );

    /** translates a VCL ImageAlign value into a compatible css.awt.ImageAlign value
    */
    sal_Int16 getCompatibleImageAlign( ImageAlign _eAlign );

    /** translates a css.awt.ImageAlign value into a css.awt.ImagePosition value
    */
    sal_Int16 getExtendedImagePosition( sal_Int16 _nImageAlign );

//........................................................................
} // namespace toolkit
//........................................................................

#endif // TOOLKIT_INC_TOOLKIT_HELPER_IMAGEALIGN_HXX
