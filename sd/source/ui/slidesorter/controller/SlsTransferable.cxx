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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "controller/SlsTransferable.hxx"

#include "SlideSorterViewShell.hxx"
#include "View.hxx"

namespace sd { namespace slidesorter { namespace controller {

Transferable::Transferable (
    SdDrawDocument* pSrcDoc,
    ::sd::View* pWorkView,
    sal_Bool bInitOnGetData,
    SlideSorterViewShell* pViewShell,
    const ::std::vector<Representative>& rRepresentatives)
    : SdTransferable (pSrcDoc, pWorkView, bInitOnGetData),
      mpViewShell(pViewShell),
      maRepresentatives(rRepresentatives)
{
    if (mpViewShell != NULL)
        StartListening(*mpViewShell);
}




Transferable::~Transferable (void)
{
    if (mpViewShell != NULL)
        EndListening(*mpViewShell);
}




void Transferable::DragFinished (sal_Int8 nDropAction)
{
    if (mpViewShell != NULL)
        mpViewShell->DragFinished(nDropAction);
}




void Transferable::Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint)
{
    if (rHint.ISA(SfxSimpleHint) && mpViewShell!=NULL)
    {
        SfxSimpleHint& rSimpleHint (*PTR_CAST(SfxSimpleHint, &rHint));
        if (rSimpleHint.GetId() == SFX_HINT_DYING)
        {
            // This hint may come either from the ViewShell or from the
            // document (registered by SdTransferable).  We do not know
            // which but both are sufficient to disconnect from the
            // ViewShell.
            EndListening(*mpViewShell);
            mpViewShell = NULL;
        }
    }

    SdTransferable::Notify(rBroadcaster, rHint);
}




const ::std::vector<Transferable::Representative>& Transferable::GetRepresentatives (void) const
{
    return maRepresentatives;
}


} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
