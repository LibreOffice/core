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

#ifndef _SDR_ATTRIBUTE_SDRFORMTEXTATTRIBUTE_HXX
#define _SDR_ATTRIBUTE_SDRFORMTEXTATTRIBUTE_HXX

#include <sal/types.h>
#include <svx/xenum.hxx>
#include <tools/color.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

class SfxItemSet;

namespace drawinglayer { namespace attribute {
    class ImpSdrFormTextAttribute;
    class SdrFormTextOutlineAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrFormTextAttribute
        {
        private:
            ImpSdrFormTextAttribute*            mpSdrFormTextAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrFormTextAttribute(const SfxItemSet& rSet);
            SdrFormTextAttribute();
            SdrFormTextAttribute(const SdrFormTextAttribute& rCandidate);
            SdrFormTextAttribute& operator=(const SdrFormTextAttribute& rCandidate);
            ~SdrFormTextAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrFormTextAttribute& rCandidate) const;

            // data read access
            sal_Int32 getFormTextDistance() const;
            sal_Int32 getFormTextStart() const;
            sal_Int32 getFormTextShdwXVal() const;
            sal_Int32 getFormTextShdwYVal() const;
            sal_uInt16 getFormTextShdwTransp() const;
            XFormTextStyle getFormTextStyle() const;
            XFormTextAdjust getFormTextAdjust() const;
            XFormTextShadow getFormTextShadow() const;
            Color getFormTextShdwColor() const;
            const SdrFormTextOutlineAttribute& getOutline() const;
            const SdrFormTextOutlineAttribute& getShadowOutline() const;
            bool getFormTextMirror() const;
            bool getFormTextOutline() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_ATTRIBUTE_SDRFORMTEXTATTRIBUTE_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
