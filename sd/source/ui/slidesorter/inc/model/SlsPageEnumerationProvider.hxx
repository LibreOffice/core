/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsPageEnumerationProvider.hxx,v $
 *
 * $Revision: 1.3 $
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

#ifndef SD_SLIDESORTER_PAGE_ENUMERATION_PROVIDER_HXX
#define SD_SLIDESORTER_PAGE_ENUMERATION_PROVIDER_HXX

#include "model/SlsPageEnumeration.hxx"

namespace sd { namespace slidesorter { namespace model {

class SlideSorterModel;

/** Collection of methods that create enumeration of slides.
*/
class PageEnumerationProvider
{
public:
    /** The returned enumeration of slides iterates over all slides of the
        given model.
    */
    static PageEnumeration CreateAllPagesEnumeration (const SlideSorterModel& rModel);

    /** The returned enumeration of slides iterates over the currently
        selected slides of the given model.
    */
    static PageEnumeration CreateSelectedPagesEnumeration (const SlideSorterModel& rModel);

    /** The returned enumeration of slides iterates over the slides
        (partially) inside the visible area.
    */
    static PageEnumeration CreateVisiblePagesEnumeration (const SlideSorterModel& rModel);
};

} } } // end of namespace ::sd::slidesorter::model

#endif
