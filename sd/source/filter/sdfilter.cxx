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

#include <com/sun/star/task/XStatusIndicatorFactory.hpp>

#include <memory>

#include <osl/module.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/progress.hxx>
#include <svl/itemset.hxx>

#include "../ui/inc/DrawDocShell.hxx"
#include "../ui/inc/strings.hrc"

#include "sdresid.hxx"
#include "pres.hxx"
#include "drawdoc.hxx"
#include "sdfilter.hxx"

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::frame;

// ------------
// - SdFilter -
// ------------

SdFilter::SdFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell, sal_Bool bShowProgress )
:   mxModel( rDocShell.GetModel() )
,   mrMedium( rMedium )
,   mrDocShell( rDocShell )
,   mrDocument( *rDocShell.GetDoc() )
,   mbIsDraw( rDocShell.GetDocumentType() == DOCUMENT_TYPE_DRAW )
,   mbShowProgress( bShowProgress )
{
}

// -----------------------------------------------------------------------------

SdFilter::~SdFilter()
{
}

// -----------------------------------------------------------------------------

::rtl::OUString SdFilter::ImplGetFullLibraryName( const ::rtl::OUString& rLibraryName ) const
{
    String aTemp(::rtl::OUString(SVLIBRARY("?")));
    xub_StrLen nIndex = aTemp.Search( (sal_Unicode)'?' );
    aTemp.Replace( nIndex, 1, rLibraryName );
    ::rtl::OUString aLibraryName( aTemp );
    return aLibraryName;
}

// -----------------------------------------------------------------------------

#ifndef DISABLE_DYNLOADING
extern "C" { static void SAL_CALL thisModule() {} }

::osl::Module* SdFilter::OpenLibrary( const ::rtl::OUString& rLibraryName ) const
{
    std::auto_ptr< osl::Module > mod(new osl::Module);
    return mod->loadRelative(&thisModule, ImplGetFullLibraryName(rLibraryName),
                             SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_LAZY)
        ? mod.release() : 0;
}

#endif

// -----------------------------------------------------------------------------

void SdFilter::CreateStatusIndicator()
{
    // The status indicator must be retrieved from the provided medium arguments
    const SfxUnoAnyItem* pStatusBarItem = static_cast<const SfxUnoAnyItem*>(
            mrMedium.GetItemSet()->GetItem(SID_PROGRESS_STATUSBAR_CONTROL) );

    if ( pStatusBarItem )
        pStatusBarItem->GetValue() >>= mxStatusIndicator;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
