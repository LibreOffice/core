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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FONTATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_FONTATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <o3tl/cow_wrapper.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace rtl {
   class OUString;
}

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
        public:
            typedef o3tl::cow_wrapper< ImpFontAttribute > ImplType;

        private:
            ImplType mpFontAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            /// TODO: pair kerning and CJK kerning
            FontAttribute(
                const rtl::OUString& rFamilyName,
                const rtl::OUString& rStyleName,
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
            const rtl::OUString& getFamilyName() const;
            const rtl::OUString& getStyleName() const;
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
