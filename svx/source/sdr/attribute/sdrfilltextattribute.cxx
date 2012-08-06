/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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


#include <svx/sdr/attribute/sdrfilltextattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrFillTextAttribute::SdrFillTextAttribute(
            const SdrFillAttribute& rFill,
            const FillGradientAttribute& rFillFloatTransGradient,
            const SdrTextAttribute& rTextAttribute)
        :   maFill(rFill),
            maFillFloatTransGradient(rFillFloatTransGradient),
            maTextAttribute(rTextAttribute)
        {
        }

        SdrFillTextAttribute::SdrFillTextAttribute()
        :   maFill(),
            maFillFloatTransGradient(),
            maTextAttribute()
        {
        }

        SdrFillTextAttribute::SdrFillTextAttribute(const SdrFillTextAttribute& rCandidate)
        :   maFill(rCandidate.getFill()),
            maFillFloatTransGradient(rCandidate.getFillFloatTransGradient()),
            maTextAttribute(rCandidate.getText())
        {
        }

        SdrFillTextAttribute& SdrFillTextAttribute::operator=(const SdrFillTextAttribute& rCandidate)
        {
            maFill = rCandidate.getFill();
            maFillFloatTransGradient = rCandidate.getFillFloatTransGradient();
            maTextAttribute = rCandidate.getText();

            return *this;
        }

        bool SdrFillTextAttribute::operator==(const SdrFillTextAttribute& rCandidate) const
        {
            return(getFill() == rCandidate.getFill()
                && getFillFloatTransGradient() == rCandidate.getFillFloatTransGradient()
                && getText() == rCandidate.getText());
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
