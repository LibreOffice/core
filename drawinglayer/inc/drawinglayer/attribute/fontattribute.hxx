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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FONTATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_FONTATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <sal/types.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

class String;

namespace drawinglayer { namespace attribute {
    class ImpFontAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        /** FontAttribute class

            This attribute class is able to hold all parameters needed/used
            to completely define the parametrisation of a text portion.
         */
        class DRAWINGLAYER_DLLPUBLIC FontAttribute
        {
        private:
            ImpFontAttribute*               mpFontAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            /// TODO: pair kerning and CJK kerning
            FontAttribute(
                const String& rFamilyName,
                const String& rStyleName,
                sal_uInt16 nWeight,
                bool bSymbol = false,
                bool bVertical = false,
                bool bItalic = false,
                bool bMonospaced = false,
                bool bOutline = false,
                bool bRTL = false,
                bool bBiDiStrong = false);
            FontAttribute();
            FontAttribute(const FontAttribute& rCandidate);
            FontAttribute& operator=(const FontAttribute& rCandidate);
            ~FontAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const FontAttribute& rCandidate) const;

            /// data read access
            const String& getFamilyName() const;
            const String& getStyleName() const;
            sal_uInt16 getWeight() const;
            bool getSymbol() const;
            bool getVertical() const;
            bool getItalic() const;
            bool getOutline() const;
            bool getRTL() const;
            bool getBiDiStrong() const;
            bool getMonospaced() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_FONTATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
