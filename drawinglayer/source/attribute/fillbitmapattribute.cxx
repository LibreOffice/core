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
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/attribute/fillbitmapattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        FillBitmapAttribute::FillBitmapAttribute(
            const BitmapEx& rBitmapEx,
            const basegfx::B2DPoint& rTopLeft,
            const basegfx::B2DVector& rSize,
            bool bTiling)
        :   maBitmapEx(rBitmapEx),
            maTopLeft(rTopLeft),
            maSize(rSize),
            mbTiling(bTiling)
        {
        }

        bool FillBitmapAttribute::operator==(const FillBitmapAttribute& rCandidate) const
        {
            return (getBitmapEx() == rCandidate.getBitmapEx()
                && getTopLeft() == rCandidate.getTopLeft()
                && getSize() == rCandidate.getSize()
                && getTiling() == rCandidate.getTiling());
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
