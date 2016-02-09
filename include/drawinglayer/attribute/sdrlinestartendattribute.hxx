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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLINESTARTENDATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLINESTARTENDATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <o3tl/cow_wrapper.hxx>


// predefines

namespace basegfx {
    class B2DPolyPolygon;
}

namespace drawinglayer { namespace attribute {
    class ImpSdrLineStartEndAttribute;
}}


namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC SdrLineStartEndAttribute
        {
        public:
            typedef o3tl::cow_wrapper< ImpSdrLineStartEndAttribute > ImplType;

        private:
            ImplType mpSdrLineStartEndAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrLineStartEndAttribute(
                const basegfx::B2DPolyPolygon& rStartPolyPolygon,
                const basegfx::B2DPolyPolygon& rEndPolyPolygon,
                double fStartWidth,
                double fEndWidth,
                bool bStartActive,
                bool bEndActive,
                bool bStartCentered,
                bool bEndCentered);
            SdrLineStartEndAttribute();
            SdrLineStartEndAttribute(const SdrLineStartEndAttribute& rCandidate);
            SdrLineStartEndAttribute& operator=(const SdrLineStartEndAttribute& rCandidate);
            ~SdrLineStartEndAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrLineStartEndAttribute& rCandidate) const;

            // data read access
            const basegfx::B2DPolyPolygon& getStartPolyPolygon() const;
            const basegfx::B2DPolyPolygon& getEndPolyPolygon() const;
            double getStartWidth() const;
            double getEndWidth() const;
            bool isStartActive() const;
            bool isEndActive() const;
            bool isStartCentered() const;
            bool isEndCentered() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer


#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLINESTARTENDATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
