/*************************************************************************
 *
 *  $RCSfile: imagealign.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-05 15:57:26 $
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

#ifndef TOOLKIT_INC_TOOLKIT_HELPER_IMAGEALIGN_HXX
#include <toolkit/helper/imagealign.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_IMAGEPOSITION_HPP_
#include <com/sun/star/awt/ImagePosition.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_IMAGEALIGN_HPP_
#include <com/sun/star/awt/ImageAlign.hpp>
#endif

//........................................................................
namespace toolkit
{
//........................................................................

    using namespace ::com::sun::star::awt::ImagePosition;
    using namespace ::com::sun::star::awt::ImageAlign;

    sal_Int16 translateImagePosition( ImageAlign _eVCLAlign )
    {
        sal_Int16 nReturn = AboveCenter;
        switch ( _eVCLAlign )
        {
        case IMAGEALIGN_LEFT:           nReturn = LeftCenter; break;
        case IMAGEALIGN_TOP:            nReturn = AboveCenter;  break;
        case IMAGEALIGN_RIGHT:          nReturn = RightCenter; break;
        case IMAGEALIGN_BOTTOM:         nReturn = BelowCenter; break;
        case IMAGEALIGN_LEFT_TOP:       nReturn = LeftTop; break;
        case IMAGEALIGN_LEFT_BOTTOM:    nReturn = LeftBottom; break;
        case IMAGEALIGN_TOP_LEFT:       nReturn = AboveLeft; break;
        case IMAGEALIGN_TOP_RIGHT:      nReturn = AboveRight; break;
        case IMAGEALIGN_RIGHT_TOP:      nReturn = RightTop; break;
        case IMAGEALIGN_RIGHT_BOTTOM:   nReturn = RightBottom; break;
        case IMAGEALIGN_BOTTOM_LEFT:    nReturn = BelowLeft; break;
        case IMAGEALIGN_BOTTOM_RIGHT:   nReturn = BelowRight; break;
        case IMAGEALIGN_CENTER:         nReturn = Centered; break;
        default:
            OSL_ENSURE( sal_False, "translateImagePosition: unknown IMAGEALIGN value!" );
        }
        return nReturn;
    }

    ImageAlign translateImagePosition( sal_Int16 _eUNOAlign )
    {
        ImageAlign nReturn = IMAGEALIGN_TOP;
        switch ( _eUNOAlign )
        {
        case LeftCenter:  nReturn = IMAGEALIGN_LEFT; break;
        case AboveCenter: nReturn = IMAGEALIGN_TOP;  break;
        case RightCenter: nReturn = IMAGEALIGN_RIGHT; break;
        case BelowCenter: nReturn = IMAGEALIGN_BOTTOM; break;
        case LeftTop:     nReturn = IMAGEALIGN_LEFT_TOP; break;
        case LeftBottom:  nReturn = IMAGEALIGN_LEFT_BOTTOM; break;
        case AboveLeft:   nReturn = IMAGEALIGN_TOP_LEFT; break;
        case AboveRight:  nReturn = IMAGEALIGN_TOP_RIGHT; break;
        case RightTop:    nReturn = IMAGEALIGN_RIGHT_TOP; break;
        case RightBottom: nReturn = IMAGEALIGN_RIGHT_BOTTOM; break;
        case BelowLeft:   nReturn = IMAGEALIGN_BOTTOM_LEFT; break;
        case BelowRight:  nReturn = IMAGEALIGN_BOTTOM_RIGHT; break;
        case Centered:    nReturn = IMAGEALIGN_CENTER; break;
        default:
            OSL_ENSURE( sal_False, "translateImagePosition: unknown css.awt.ImagePosition value!" );
        }
        return nReturn;
    }

    sal_Int16 getCompatibleImageAlign( ImageAlign _eAlign )
    {
        sal_Int16 nReturn = TOP;
        switch ( _eAlign )
        {
        case IMAGEALIGN_LEFT_TOP:
        case IMAGEALIGN_LEFT:
        case IMAGEALIGN_LEFT_BOTTOM:    nReturn = LEFT; break;

        case IMAGEALIGN_TOP_LEFT:
        case IMAGEALIGN_TOP:
        case IMAGEALIGN_TOP_RIGHT:      nReturn = TOP; break;

        case IMAGEALIGN_RIGHT_TOP:
        case IMAGEALIGN_RIGHT:
        case IMAGEALIGN_RIGHT_BOTTOM:   nReturn = RIGHT; break;

        case IMAGEALIGN_BOTTOM_LEFT:
        case IMAGEALIGN_BOTTOM:
        case IMAGEALIGN_BOTTOM_RIGHT:   nReturn = BOTTOM; break;

        case IMAGEALIGN_CENTER:         nReturn = TOP; break;
        default:
            OSL_ENSURE( sal_False, "getCompatibleImageAlign: unknown IMAGEALIGN value!" );
        }
        return nReturn;
    }

    sal_Int16 getExtendedImagePosition( sal_Int16 _nImageAlign )
    {
        sal_Int16 nReturn = AboveCenter;
        switch ( _nImageAlign )
        {
        case LEFT:   nReturn = LeftCenter; break;
        case TOP:    nReturn = AboveCenter; break;
        case RIGHT:  nReturn = RightCenter; break;
        case BOTTOM: nReturn = BelowCenter; break;
        default:
            OSL_ENSURE( sal_False, "getExtendedImagePosition: unknown ImageAlign value!" );
        }
        return nReturn;
    }

//........................................................................
}   // namespace toolkit
//........................................................................
