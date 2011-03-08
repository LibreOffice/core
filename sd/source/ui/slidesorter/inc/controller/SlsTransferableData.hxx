/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SD_SLIDESORTER_TRANSFERABLE_DATA_HXX
#define SD_SLIDESORTER_TRANSFERABLE_DATA_HXX

#include "sdxfer.hxx"
#include <boost/function.hpp>
#include <vector>

class SdDrawDocument;
namespace sd { namespace slidesorter {
class SlideSorterViewShell;
} }

namespace sd { namespace slidesorter { namespace controller {

/** Represent previews and other information so that they can be
    attached to an existing transferable.
*/
class TransferableData
    : public SdTransferable::UserData,
      public SfxListener
{
public:
    class Representative
    {
    public:
        Representative (const Bitmap& rBitmap, const bool bIsExcluded)
            : maBitmap(rBitmap), mbIsExcluded(bIsExcluded) {}
        Representative (const Representative& rOther)
            : maBitmap(rOther.maBitmap), mbIsExcluded(rOther.mbIsExcluded) {}
        Representative operator= (Representative const& rOther)
        {   if (&rOther != this) {maBitmap = rOther.maBitmap; mbIsExcluded = rOther.mbIsExcluded; }
            return *this;
        }

        Bitmap maBitmap;
        bool mbIsExcluded;
    };

    static SdTransferable* CreateTransferable (
        SdDrawDocument* pSrcDoc,
        ::sd::View* pWorkView,
        sal_Bool bInitOnGetData,
        SlideSorterViewShell* pViewShell,
        const ::std::vector<TransferableData::Representative>& rRepresentatives);

    static ::boost::shared_ptr<TransferableData> GetFromTransferable (const SdTransferable* pTransferable);

    TransferableData (
        SlideSorterViewShell* pViewShell,
        const ::std::vector<TransferableData::Representative>& rRepresentatives);
    ~TransferableData (void);

    virtual void DragFinished (sal_Int8 nDropAction);

    const ::std::vector<Representative>& GetRepresentatives (void) const;

    /** Return the view shell for which the transferable was created.
    */
    SlideSorterViewShell* GetSourceViewShell (void) const;

private:
    SlideSorterViewShell* mpViewShell;
    const ::std::vector<Representative> maRepresentatives;
    typedef ::std::vector<boost::function<void(sal_uInt8)> > CallbackContainer;

    virtual void Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint);
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
