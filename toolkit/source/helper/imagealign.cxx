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
#include "precompiled_toolkit.hxx"
#include <toolkit/helper/imagealign.hxx>
#include <com/sun/star/awt/ImagePosition.hpp>
#include <com/sun/star/awt/ImageAlign.hpp>

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
