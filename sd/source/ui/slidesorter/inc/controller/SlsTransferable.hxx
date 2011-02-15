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

#ifndef SD_SLIDESORTER_TRANSFERABLE_HXX
#define SD_SLIDESORTER_TRANSFERABLE_HXX

#include "sdxfer.hxx"

class SdDrawDocument;
namespace sd { namespace slidesorter {
class SlideSorterViewShell;
} }

namespace sd { namespace slidesorter { namespace controller {


/** This class exists to have DragFinished call the correct object: the
    SlideSorterViewShell instead of the old SlideView.
*/
class Transferable
    : public SdTransferable
{
public:
    class Representative
    {
    public:
        Representative (const Bitmap& rBitmap, const bool bIsExcluded)
            : maBitmap(rBitmap), mbIsExcluded(bIsExcluded) {}
        Representative (const Representative& rOther)
            : maBitmap(rOther.maBitmap), mbIsExcluded(rOther.mbIsExcluded) {}
        Representative operator= (Representative& rOther)
        {   if (&rOther != this) {maBitmap = rOther.maBitmap; mbIsExcluded = rOther.mbIsExcluded; }
            return *this;
        }

        Bitmap maBitmap;
        bool mbIsExcluded;
    };


    Transferable (
        SdDrawDocument* pSrcDoc,
        ::sd::View* pWorkView,
        sal_Bool bInitOnGetData,
        SlideSorterViewShell* pViewShell,
        const ::std::vector<Representative>& rRepresentatives);

    virtual ~Transferable (void);

    virtual void DragFinished (sal_Int8 nDropAction);

    const ::std::vector<Representative>& GetRepresentatives (void) const;

private:
    SlideSorterViewShell* mpViewShell;
    const ::std::vector<Representative> maRepresentatives;

    virtual void Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif
