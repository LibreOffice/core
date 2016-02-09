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

#ifndef INCLUDED_SVX_INC_SDR_ATTRIBUTE_SDRFORMTEXTOUTLINEATTRIBUTE_HXX
#define INCLUDED_SVX_INC_SDR_ATTRIBUTE_SDRFORMTEXTOUTLINEATTRIBUTE_HXX

#include <sal/types.h>
#include <o3tl/cow_wrapper.hxx>

// predefines

namespace drawinglayer { namespace attribute {
    class ImpSdrFormTextOutlineAttribute;
    class LineAttribute;
    class StrokeAttribute;
}}


namespace drawinglayer
{
    namespace attribute
    {
        class SdrFormTextOutlineAttribute
        {
        public:
            typedef o3tl::cow_wrapper< ImpSdrFormTextOutlineAttribute > ImplType;

        private:
            ImplType mpSdrFormTextOutlineAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrFormTextOutlineAttribute(
                const LineAttribute& rLineAttribute,
                const StrokeAttribute& rStrokeAttribute,
                sal_uInt8 nTransparence);
            SdrFormTextOutlineAttribute();
            SdrFormTextOutlineAttribute(const SdrFormTextOutlineAttribute& rCandidate);
            SdrFormTextOutlineAttribute& operator=(const SdrFormTextOutlineAttribute& rCandidate);
            ~SdrFormTextOutlineAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrFormTextOutlineAttribute& rCandidate) const;

            // data read access
            const LineAttribute& getLineAttribute() const;
            const StrokeAttribute& getStrokeAttribute() const;
            sal_uInt8 getTransparence() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer


#endif // INCLUDED_SVX_INC_SDR_ATTRIBUTE_SDRFORMTEXTOUTLINEATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
