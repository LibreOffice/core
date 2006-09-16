/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsTransferable.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 19:08:14 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "SlsTransferable.hxx"

#include "SlideSorterViewShell.hxx"
#include "View.hxx"

namespace sd { namespace slidesorter { namespace controller {

Transferable::Transferable (
    SdDrawDocument* pSrcDoc,
    ::sd::View* pWorkView,
    BOOL bInitOnGetData,
    SlideSorterViewShell* pViewShell)
    : SdTransferable (pSrcDoc, pWorkView, bInitOnGetData),
      mpViewShell(pViewShell)
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




} } } // end of namespace ::sd::slidesorter::controller
