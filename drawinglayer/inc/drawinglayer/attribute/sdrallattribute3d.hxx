/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrallattribute3d.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-08-09 16:36:37 $
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

#ifndef _DRAWINGLAYER_ATTRIBUTE_SDRALLATTRIBUTE3D_HXX
#define _DRAWINGLAYER_ATTRIBUTE_SDRALLATTRIBUTE3D_HXX

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace attribute {
    class sdrLineAttribute;
    class sdrFillAttribute;
    class sdrLineStartEndAttribute;
    class sdrShadowAttribute;
    class fillGradientAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class sdrLineFillShadowAttribute
        {
            // shadow, line, lineStartEnd, fill, transGradient and compound3D attributes
            sdrShadowAttribute*                 mpShadow;                   // shadow attributes (if used)
            sdrLineAttribute*                   mpLine;                     // line attributes (if used)
            sdrLineStartEndAttribute*           mpLineStartEnd;             // line start end (if used)
            sdrFillAttribute*                   mpFill;                     // fill attributes (if used)
            fillGradientAttribute*              mpFillFloatTransGradient;   // fill float transparence gradient (if used)

        public:
            sdrLineFillShadowAttribute(
                sdrLineAttribute* pLine = 0L,
                sdrFillAttribute* pFill = 0L,
                sdrLineStartEndAttribute* pLineStartEnd = 0L,
                sdrShadowAttribute* pShadow = 0L,
                fillGradientAttribute* pFillFloatTransGradient = 0L);
            ~sdrLineFillShadowAttribute();

            // copy constructor and assigment operator
            sdrLineFillShadowAttribute(const sdrLineFillShadowAttribute& rCandidate);
            sdrLineFillShadowAttribute& operator=(const sdrLineFillShadowAttribute& rCandidate);

            // compare operator
            bool operator==(const sdrLineFillShadowAttribute& rCandidate) const;

            // bool access
            bool isVisible() const { return (mpFill || mpLine); }

            // data access
            const sdrShadowAttribute* getShadow() const { return mpShadow; }
            const sdrLineAttribute* getLine() const { return mpLine; }
            const sdrLineStartEndAttribute* getLineStartEnd() const { return mpLineStartEnd; }
            const sdrFillAttribute* getFill() const { return mpFill; }
            const fillGradientAttribute* getFillFloatTransGradient() const { return mpFillFloatTransGradient; }
        };
    } // end of namespace overlay
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif // _DRAWINGLAYER_ATTRIBUTE_SDRALLATTRIBUTE3D_HXX

// eof
