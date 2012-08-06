/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRALLATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRALLATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <sal/types.h>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/sdrlinestartendattribute.hxx>
#include <drawinglayer/attribute/sdrshadowattribute.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC SdrLineFillShadowAttribute3D
        {
        private:
            const SdrLineAttribute          maLine;
            const SdrFillAttribute          maFill;
            const SdrLineStartEndAttribute  maLineStartEnd;
            const SdrShadowAttribute        maShadow;
            const FillGradientAttribute     maFillFloatTransGradient;

        public:
            /// constructors/assignmentoperator/destructor
            SdrLineFillShadowAttribute3D(
                const SdrLineAttribute& rLine,
                const SdrFillAttribute& rFill,
                const SdrLineStartEndAttribute& rLineStartEnd,
                const SdrShadowAttribute& rShadow,
                const FillGradientAttribute& rFillFloatTransGradient);
            SdrLineFillShadowAttribute3D();

            // compare operator
            bool operator==(const SdrLineFillShadowAttribute3D& rCandidate) const;

            // data read access
            const SdrLineAttribute& getLine() const { return maLine; }
            const SdrFillAttribute& getFill() const { return maFill; }
            const SdrLineStartEndAttribute& getLineStartEnd() const { return maLineStartEnd; }
            const SdrShadowAttribute& getShadow() const { return maShadow; }
            const FillGradientAttribute& getFillFloatTransGradient() const { return maFillFloatTransGradient; }
        };
    } // end of namespace overlay
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRALLATTRIBUTE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
