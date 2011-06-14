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

#ifndef _SDR_ATTRIBUTE_SDRFORMTEXTOUTLINEATTRIBUTE_HXX
#define _SDR_ATTRIBUTE_SDRFORMTEXTOUTLINEATTRIBUTE_HXX

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace attribute {
    class ImpSdrFormTextOutlineAttribute;
    class LineAttribute;
    class StrokeAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrFormTextOutlineAttribute
        {
        private:
            ImpSdrFormTextOutlineAttribute*     mpSdrFormTextOutlineAttribute;

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

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_ATTRIBUTE_SDRFORMTEXTOUTLINEATTRIBUTE_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
