/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: sdrtextattribute.hxx,v $
 *
 * $Revision: 1.2 $
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
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrFormTextOutlineAttribute
        {
        private:
            // one set of attributes for FormText (FontWork) outline visualisation
            LineAttribute                       maLineAttribute;
            StrokeAttribute                     maStrokeAttribute;
            sal_uInt8                           mnTransparence;

        public:
            SdrFormTextOutlineAttribute(
                const LineAttribute& rLineAttribute,
                const StrokeAttribute& rStrokeAttribute,
                sal_uInt8 nTransparence);

            // compare operator
            bool operator==(const SdrFormTextOutlineAttribute& rCandidate) const;

            // data read access
            const LineAttribute& getLineAttribute() const { return maLineAttribute; }
            const StrokeAttribute getStrokeAttribute() const { return maStrokeAttribute; }
            sal_uInt8 getTransparence() const { return mnTransparence; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_ATTRIBUTE_SDRFORMTEXTOUTLINEATTRIBUTE_HXX

// eof
