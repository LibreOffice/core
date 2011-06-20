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
#include <svx/svdograf.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xoutbmp.hxx>
#include <osl/file.hxx>

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
}

// -----------------------------------------------------------------------------

SdHTMLFilter::~SdHTMLFilter()
{
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
