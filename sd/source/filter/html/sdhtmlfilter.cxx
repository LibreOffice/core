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

#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxsids.hrc>

#include "htmlex.hxx"
#include <sdhtmlfilter.hxx>


SdHTMLFilter::SdHTMLFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell ) :
    SdFilter( rMedium, rDocShell )
{
}

SdHTMLFilter::~SdHTMLFilter()
{
}

bool SdHTMLFilter::Export()
{
    mrMedium.Close();
    mrMedium.Commit();

    SfxItemSet *pSet = mrMedium.GetItemSet();

    css::uno::Sequence< css::beans::PropertyValue > aParams;

    const SfxPoolItem* pItem;

    if ( pSet->GetItemState( SID_FILTER_DATA, false, &pItem ) == SfxItemState::SET )
        static_cast<const SfxUnoAnyItem*>(pItem)->GetValue() >>= aParams;

    HtmlExport aExport(mrMedium.GetName(), aParams, &mrDocument, &mrDocShell);

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
