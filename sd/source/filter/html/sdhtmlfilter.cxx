/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdhtmlfilter.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:43:56 $
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

#include <unotools/localfilehelper.hxx>
#include <tools/errinf.hxx>
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/metaact.hxx>
#include <vcl/virdev.hxx>
#include <svtools/FilterConfigItem.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/frame.hxx>
#include <svx/impgrf.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xoutbmp.hxx>

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "sdattr.hxx"
#include "htmlex.hxx"
#include "sdhtmlfilter.hxx"

// ---------------
// - SdPPTFilter -
// ---------------

SdHTMLFilter::SdHTMLFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell, sal_Bool bShowProgress ) :
    SdFilter( rMedium, rDocShell, bShowProgress )
{
    mbHideSpell = mrDocument.GetHideSpell();
    mrDocument.SetHideSpell(TRUE);
}

// -----------------------------------------------------------------------------

SdHTMLFilter::~SdHTMLFilter()
{
    mrDocument.SetHideSpell(mbHideSpell);
}

// -----------------------------------------------------------------------------

sal_Bool SdHTMLFilter::Export()
{
    mrMedium.Close();
    mrMedium.Commit();

    SfxItemSet *pSet = mrMedium.GetItemSet();

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aParams;

    const SfxPoolItem* pItem;
    if ( pSet->GetItemState( SID_FILTER_DATA, sal_False, &pItem ) == SFX_ITEM_SET )
        ((SfxUnoAnyItem*)pItem)->GetValue() >>= aParams;

    delete( new HtmlExport( mrMedium.GetName(), aParams, &mrDocument, &mrDocShell ) );

    return true;
}
