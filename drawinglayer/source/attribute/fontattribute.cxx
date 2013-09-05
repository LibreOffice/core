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

#include <drawinglayer/attribute/fontattribute.hxx>
#include <rtl/instance.hxx>
#include <tools/string.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpFontAttribute
        {
        public:
            /// core data
            OUString                                    maFamilyName;       // Font Family Name
            OUString                                    maStyleName;        // Font Style Name
            sal_uInt16                                  mnWeight;           // Font weight

            /// bitfield
            unsigned                                    mbSymbol : 1;       // Symbol Font Flag
            unsigned                                    mbVertical : 1;     // Vertical Text Flag
            unsigned                                    mbItalic : 1;       // Italic Flag
            unsigned                                    mbOutline : 1;      // Outline Flag
            unsigned                                    mbRTL : 1;          // RTL Flag
            unsigned                                    mbBiDiStrong : 1;   // BiDi Flag
            unsigned                                    mbMonospaced : 1;

            ImpFontAttribute(
                const OUString& rFamilyName,
                const OUString& rStyleName,
                sal_uInt16 nWeight,
                bool bSymbol,
                bool bVertical,
                bool bItalic,
                bool bMonospaced,
                bool bOutline,
                bool bRTL,
                bool bBiDiStrong)
            :   maFamilyName(rFamilyName),
                maStyleName(rStyleName),
                mnWeight(nWeight),
                mbSymbol(bSymbol),
                mbVertical(bVertical),
                mbItalic(bItalic),
                mbOutline(bOutline),
                mbRTL(bRTL),
                mbBiDiStrong(bBiDiStrong),
                mbMonospaced(bMonospaced)
            {
            }

            ImpFontAttribute()
            :   maFamilyName(),
                maStyleName(),
                mnWeight(0),
                mbSymbol(false),
                mbVertical(false),
                mbItalic(false),
                mbOutline(false),
                mbRTL(false),
                mbBiDiStrong(false),
                mbMonospaced(false)
            {
            }

            // data read access
            const OUString& getFamilyName() const { return maFamilyName; }
            const OUString& getStyleName() const { return maStyleName; }
            sal_uInt16 getWeight() const { return mnWeight; }
            bool getSymbol() const { return mbSymbol; }
            bool getVertical() const { return mbVertical; }
            bool getItalic() const { return mbItalic; }
            bool getOutline() const { return mbOutline; }
            bool getRTL() const { return mbRTL; }
            bool getBiDiStrong() const { return mbBiDiStrong; }
            bool getMonospaced() const { return mbMonospaced; }

            bool operator==(const ImpFontAttribute& rCompare) const
            {
                return (getFamilyName() == rCompare.getFamilyName()
                    && getStyleName() == rCompare.getStyleName()
                    && getWeight() == rCompare.getWeight()
                    && getSymbol() == rCompare.getSymbol()
                    && getVertical() == rCompare.getVertical()
                    && getItalic() == rCompare.getItalic()
                    && getOutline() == rCompare.getOutline()
                    && getRTL() == rCompare.getRTL()
                    && getBiDiStrong() == rCompare.getBiDiStrong()
                    && getMonospaced() == rCompare.getMonospaced());
            }
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< FontAttribute::ImplType, theGlobalDefault > {};
        }

        FontAttribute::FontAttribute(
            const OUString& rFamilyName,
            const OUString& rStyleName,
            sal_uInt16 nWeight,
            bool bSymbol,
            bool bVertical,
            bool bItalic,
            bool bMonospaced,
            bool bOutline,
            bool bRTL,
            bool bBiDiStrong)
        :   mpFontAttribute(ImpFontAttribute(
                rFamilyName, rStyleName, nWeight, bSymbol, bVertical, bItalic, bMonospaced, bOutline, bRTL, bBiDiStrong))
        {
        }

        FontAttribute::FontAttribute()
        :   mpFontAttribute(theGlobalDefault::get())
        {
        }

        FontAttribute::FontAttribute(const FontAttribute& rCandidate)
        :   mpFontAttribute(rCandidate.mpFontAttribute)
        {
        }

        FontAttribute::~FontAttribute()
        {
        }

        FontAttribute& FontAttribute::operator=(const FontAttribute& rCandidate)
        {
            mpFontAttribute = rCandidate.mpFontAttribute;
            return *this;
        }

        bool FontAttribute::operator==(const FontAttribute& rCandidate) const
        {
            return rCandidate.mpFontAttribute == mpFontAttribute;
        }

        const OUString& FontAttribute::getFamilyName() const
        {
            return mpFontAttribute->getFamilyName();
        }

        const OUString& FontAttribute::getStyleName() const
        {
            return mpFontAttribute->getStyleName();
        }

        sal_uInt16 FontAttribute::getWeight() const
        {
            return mpFontAttribute->getWeight();
        }

        bool FontAttribute::getSymbol() const
        {
            return mpFontAttribute->getSymbol();
        }

        bool FontAttribute::getVertical() const
        {
            return mpFontAttribute->getVertical();
        }

        bool FontAttribute::getItalic() const
        {
            return mpFontAttribute->getItalic();
        }

        bool FontAttribute::getOutline() const
        {
            return mpFontAttribute->getOutline();
        }

        bool FontAttribute::getRTL() const
        {
            return mpFontAttribute->getRTL();
        }

        bool FontAttribute::getBiDiStrong() const
        {
            return mpFontAttribute->getBiDiStrong();
        }

        bool FontAttribute::getMonospaced() const
        {
            return mpFontAttribute->getMonospaced();
        }


    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
