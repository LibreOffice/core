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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSTRANSFERABLEDATA_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_CONTROLLER_SLSTRANSFERABLEDATA_HXX

#include <sdxfer.hxx>

#include <vcl/bitmapex.hxx>

#include <vector>

class SdDrawDocument;
namespace sd::slidesorter { class SlideSorterViewShell; }

namespace sd::slidesorter::controller {

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
        Representative (const BitmapEx& rBitmap, const bool bIsExcluded)
            : maBitmap(rBitmap), mbIsExcluded(bIsExcluded) {}

        BitmapEx maBitmap;
        bool mbIsExcluded;
    };

    static rtl::Reference<SdTransferable> CreateTransferable (
        SdDrawDocument* pSrcDoc,
        SlideSorterViewShell* pViewShell,
        const ::std::vector<TransferableData::Representative>& rRepresentatives);

    static std::shared_ptr<TransferableData> GetFromTransferable (const SdTransferable* pTransferable);

    TransferableData (
        SlideSorterViewShell* pViewShell,
        const ::std::vector<TransferableData::Representative>& rRepresentatives);
    virtual ~TransferableData() override;

    const ::std::vector<Representative>& GetRepresentatives() const { return maRepresentatives;}

    /** Return the view shell for which the transferable was created.
    */
    SlideSorterViewShell* GetSourceViewShell() const { return mpViewShell;}

private:
    SlideSorterViewShell* mpViewShell;
    const ::std::vector<Representative> maRepresentatives;

    virtual void Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint) override;
};

} // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
