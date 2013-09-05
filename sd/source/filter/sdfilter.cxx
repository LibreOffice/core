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

OUString SdFilter::ImplGetFullLibraryName( const OUString& rLibraryName ) const
{
    OUString aTemp(SVLIBRARY("?"));
    return aTemp.replaceFirst( "?", rLibraryName );
}

// -----------------------------------------------------------------------------

#ifndef DISABLE_DYNLOADING
extern "C" { static void SAL_CALL thisModule() {} }

::osl::Module* SdFilter::OpenLibrary( const OUString& rLibraryName ) const
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
