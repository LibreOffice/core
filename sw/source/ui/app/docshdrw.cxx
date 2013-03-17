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


#include <hintids.hxx>
#include <svx/svxids.hrc>
#include <svl/stritem.hxx>
#include <svx/drawitem.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdoutl.hxx>
#include <svx/xtable.hxx>

#include <docsh.hxx>
#include <doc.hxx>

using namespace ::com::sun::star;

// Load Document
void  SwDocShell::InitDraw()
{
    SdrModel *pDrDoc = pDoc->GetDrawModel();
    if( pDrDoc )
    {
        // create Lists and Tables DocShell's ItemSet
        PutItem( SvxGradientListItem( pDrDoc->GetGradientList(), SID_GRADIENT_LIST ) );
        PutItem( SvxHatchListItem( pDrDoc->GetHatchList(), SID_HATCH_LIST ) );
        PutItem( SvxBitmapListItem( pDrDoc->GetBitmapList(), SID_BITMAP_LIST ) );
        PutItem( SvxDashListItem( pDrDoc->GetDashList(), SID_DASH_LIST ) );
        PutItem( SvxLineEndListItem( pDrDoc->GetLineEndList(), SID_LINEEND_LIST ) );

        Outliner& rOutliner = pDrDoc->GetDrawOutliner();
        uno::Reference<linguistic2::XHyphenator> xHyphenator( ::GetHyphenator() );
        rOutliner.SetHyphenator( xHyphenator );
    }
    else
        PutItem( SvxColorListItem( XColorList::GetStdColorList(), SID_COLOR_TABLE ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
