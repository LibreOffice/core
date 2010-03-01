/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strokeattribute.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/attribute/fontattribute.hxx>
#include <tools/string.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpFontAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            /// core data
            String                                      maFamilyName;       // Font Family Name
            String                                      maStyleName;        // Font Style Name
            sal_uInt16                                  mnWeight;           // Font weight

            /// bitfield
            unsigned                                    mbSymbol : 1;       // Symbol Font Flag
            unsigned                                    mbVertical : 1;     // Vertical Text Flag
            unsigned                                    mbItalic : 1;       // Italic Flag
            unsigned                                    mbOutline : 1;      // Outline Flag
            unsigned                                    mbRTL : 1;          // RTL Flag
            unsigned                                    mbBiDiStrong : 1;   // BiDi Flag

            ImpFontAttribute(
                const String& rFamilyName,
                const String& rStyleName,
                sal_uInt16 nWeight,
                bool bSymbol,
                bool bVertical,
                bool bItalic,
                bool bOutline,
                bool bRTL,
                bool bBiDiStrong)
            :   mnRefCount(0),
                maFamilyName(rFamilyName),
                maStyleName(rStyleName),
                mnWeight(nWeight),
                mbSymbol(bSymbol),
                mbVertical(bVertical),
                mbItalic(bItalic),
                mbOutline(bOutline),
                mbRTL(bRTL),
                mbBiDiStrong(bBiDiStrong)
            {
            }

            // data read access
            const String& getFamilyName() const { return maFamilyName; }
            const String& getStyleName() const { return maStyleName; }
            sal_uInt16 getWeight() const { return mnWeight; }
            bool getSymbol() const { return mbSymbol; }
            bool getVertical() const { return mbVertical; }
            bool getItalic() const { return mbItalic; }
            bool getOutline() const { return mbOutline; }
            bool getRTL() const { return mbRTL; }
            bool getBiDiStrong() const { return mbBiDiStrong; }

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
                    && getBiDiStrong() == rCompare.getBiDiStrong());
            }

            static ImpFontAttribute* get_global_default()
            {
                static ImpFontAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpFontAttribute(
                        String(), String(),
                        0,
                        false, false, false, false, false, false);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        FontAttribute::FontAttribute(
            const String& rFamilyName,
            const String& rStyleName,
            sal_uInt16 nWeight,
            bool bSymbol,
            bool bVertical,
            bool bItalic,
            bool bOutline,
            bool bRTL,
            bool bBiDiStrong)
        :   mpFontAttribute(new ImpFontAttribute(
                rFamilyName, rStyleName, nWeight, bSymbol, bVertical, bItalic, bOutline, bRTL, bBiDiStrong))
        {
        }

        FontAttribute::FontAttribute()
        :   mpFontAttribute(ImpFontAttribute::get_global_default())
        {
            mpFontAttribute->mnRefCount++;
        }

        FontAttribute::FontAttribute(const FontAttribute& rCandidate)
        :   mpFontAttribute(rCandidate.mpFontAttribute)
        {
            mpFontAttribute->mnRefCount++;
        }

        FontAttribute::~FontAttribute()
        {
            if(mpFontAttribute->mnRefCount)
            {
                mpFontAttribute->mnRefCount--;
            }
            else
            {
                delete mpFontAttribute;
            }
        }

        bool FontAttribute::isDefault() const
        {
            return mpFontAttribute == ImpFontAttribute::get_global_default();
        }

        FontAttribute& FontAttribute::operator=(const FontAttribute& rCandidate)
        {
            if(rCandidate.mpFontAttribute != mpFontAttribute)
            {
                if(mpFontAttribute->mnRefCount)
                {
                    mpFontAttribute->mnRefCount--;
                }
                else
                {
                    delete mpFontAttribute;
                }

                mpFontAttribute = rCandidate.mpFontAttribute;
                mpFontAttribute->mnRefCount++;
            }

            return *this;
        }

        bool FontAttribute::operator==(const FontAttribute& rCandidate) const
        {
            if(rCandidate.mpFontAttribute == mpFontAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpFontAttribute == *mpFontAttribute);
        }

        const String& FontAttribute::getFamilyName() const
        {
            return mpFontAttribute->getFamilyName();
        }

        const String& FontAttribute::getStyleName() const
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

    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
