/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imagealign.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:55:06 $
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

#ifndef TOOLKIT_INC_TOOLKIT_HELPER_IMAGEALIGN_HXX
#define TOOLKIT_INC_TOOLKIT_HELPER_IMAGEALIGN_HXX

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

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
