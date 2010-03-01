/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strokeattribute.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:16 $
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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FONTATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_FONTATTRIBUTE_HXX

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
        class FontAttribute
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
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_FONTATTRIBUTE_HXX

// eof
