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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRALLATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRALLATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/sdrlinestartendattribute.hxx>
#include <drawinglayer/attribute/sdrshadowattribute.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>


// predefines



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



#endif // INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRALLATTRIBUTE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
