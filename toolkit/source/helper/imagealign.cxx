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

#include <com/sun/star/awt/ImagePosition.hpp>
#include <com/sun/star/awt/ImageAlign.hpp>

#include "helper/imagealign.hxx"

namespace toolkit
{


    using namespace ::com::sun::star::awt::ImagePosition;
    using namespace ::com::sun::star::awt::ImageAlign;

    sal_Int16 translateImagePosition( ImageAlign _eVCLAlign )
    {
        sal_Int16 nReturn = AboveCenter;
        switch ( _eVCLAlign )
        {
        case ImageAlign::Left:           nReturn = LeftCenter; break;
        case ImageAlign::Top:            nReturn = AboveCenter;  break;
        case ImageAlign::Right:          nReturn = RightCenter; break;
        case ImageAlign::Bottom:         nReturn = BelowCenter; break;
        case ImageAlign::LeftTop:       nReturn = LeftTop; break;
        case ImageAlign::LeftBottom:    nReturn = LeftBottom; break;
        case ImageAlign::TopLeft:       nReturn = AboveLeft; break;
        case ImageAlign::TopRight:      nReturn = AboveRight; break;
        case ImageAlign::RightTop:      nReturn = RightTop; break;
        case ImageAlign::RightBottom:   nReturn = RightBottom; break;
        case ImageAlign::BottomLeft:    nReturn = BelowLeft; break;
        case ImageAlign::BottomRight:   nReturn = BelowRight; break;
        case ImageAlign::Center:         nReturn = Centered; break;
        default:
            OSL_FAIL( "translateImagePosition: unknown IMAGEALIGN value!" );
        }
        return nReturn;
    }

    ImageAlign translateImagePosition( sal_Int16 _eUNOAlign )
    {
        ImageAlign nReturn = ImageAlign::Top;
        switch ( _eUNOAlign )
        {
        case LeftCenter:  nReturn = ImageAlign::Left; break;
        case AboveCenter: nReturn = ImageAlign::Top;  break;
        case RightCenter: nReturn = ImageAlign::Right; break;
        case BelowCenter: nReturn = ImageAlign::Bottom; break;
        case LeftTop:     nReturn = ImageAlign::LeftTop; break;
        case LeftBottom:  nReturn = ImageAlign::LeftBottom; break;
        case AboveLeft:   nReturn = ImageAlign::TopLeft; break;
        case AboveRight:  nReturn = ImageAlign::TopRight; break;
        case RightTop:    nReturn = ImageAlign::RightTop; break;
        case RightBottom: nReturn = ImageAlign::RightBottom; break;
        case BelowLeft:   nReturn = ImageAlign::BottomLeft; break;
        case BelowRight:  nReturn = ImageAlign::BottomRight; break;
        case Centered:    nReturn = ImageAlign::Center; break;
        default:
            OSL_FAIL( "translateImagePosition: unknown css.awt.ImagePosition value!" );
        }
        return nReturn;
    }

    sal_Int16 getCompatibleImageAlign( ImageAlign _eAlign )
    {
        sal_Int16 nReturn = TOP;
        switch ( _eAlign )
        {
        case ImageAlign::LeftTop:
        case ImageAlign::Left:
        case ImageAlign::LeftBottom:    nReturn = LEFT; break;

        case ImageAlign::TopLeft:
        case ImageAlign::Top:
        case ImageAlign::TopRight:      nReturn = TOP; break;

        case ImageAlign::RightTop:
        case ImageAlign::Right:
        case ImageAlign::RightBottom:   nReturn = RIGHT; break;

        case ImageAlign::BottomLeft:
        case ImageAlign::Bottom:
        case ImageAlign::BottomRight:   nReturn = BOTTOM; break;

        case ImageAlign::Center:         nReturn = TOP; break;
        default:
            OSL_FAIL( "getCompatibleImageAlign: unknown IMAGEALIGN value!" );
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
            OSL_FAIL( "getExtendedImagePosition: unknown ImageAlign value!" );
        }
        return nReturn;
    }


}   // namespace toolkit


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
