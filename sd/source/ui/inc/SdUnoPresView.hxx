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

#ifndef SD_UNO_PRES_VIEW_HXX
#define SD_UNO_PRES_VIEW_HXX

#include "SdUnoDrawView.hxx"

namespace sd {

/** The sub controller for the slide show (or preview?)  It formerly reduced
    the property set inherited from SdUnoDrawView to just 'CurrentPage'.
    Now that we have to always support the whole set, we can as well try to
    do that as best as we can.  Therefore the inherited functionality is
    provided as is.
*/
class SdUnoPresView
    : public SdUnoDrawView
{
public:
    SdUnoPresView (
        DrawController& rController,
        DrawViewShell& rViewShell,
        View& rView) throw();
    virtual ~SdUnoPresView (void) throw();
};

} // end of namespace sd

#endif
