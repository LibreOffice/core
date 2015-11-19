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

#include "swtypes.hxx"
#include <rtl/ustring.hxx>

#include <bodyfrm.hxx>
#include <cellfrm.hxx>
#include <colfrm.hxx>
#include <editeng/unolingu.hxx>
#include <fntcache.hxx>
#include <frmfmt.hxx>
#include <frmtool.hxx>
#include <hffrm.hxx>
#include <ndtxt.hxx>
#include <pagefrm.hxx>
#include <rowfrm.hxx>
#include <sectfrm.hxx>
#include <swatrset.hxx>
#include <swfntcch.hxx>
#include <swtable.hxx>
#include <swtblfmt.hxx>
#include <tabfrm.hxx>
#include <txtfrm.hxx>
#include <UndoDelete.hxx>
#include <UndoInsert.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <viscrs.hxx>

using namespace com::sun::star;

OUString aEmptyOUStr;  // remove once aEmptyOUStr can be changed to OUString

IMPL_FIXEDMEMPOOL_NEWDEL( SwAttrSet )
IMPL_FIXEDMEMPOOL_NEWDEL( SwStartNode )
IMPL_FIXEDMEMPOOL_NEWDEL( SwEndNode )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableBox )
IMPL_FIXEDMEMPOOL_NEWDEL( SwUndoDelete )
IMPL_FIXEDMEMPOOL_NEWDEL( SwUndoInsert )
IMPL_FIXEDMEMPOOL_NEWDEL( SwPaM )
IMPL_FIXEDMEMPOOL_NEWDEL( SwCursor )
IMPL_FIXEDMEMPOOL_NEWDEL( SwShellCursor )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTextNode )
IMPL_FIXEDMEMPOOL_NEWDEL( SwpHints )
IMPL_FIXEDMEMPOOL_NEWDEL( SwFntObj )
IMPL_FIXEDMEMPOOL_NEWDEL( SwFontObj )
IMPL_FIXEDMEMPOOL_NEWDEL( SwBorderAttrs )
IMPL_FIXEDMEMPOOL_NEWDEL( SwCellFrame )
IMPL_FIXEDMEMPOOL_NEWDEL( SwRowFrame )
IMPL_FIXEDMEMPOOL_NEWDEL( SwColumnFrame )
IMPL_FIXEDMEMPOOL_NEWDEL( SwSectionFrame )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTabFrame )
IMPL_FIXEDMEMPOOL_NEWDEL( SwPageFrame )
IMPL_FIXEDMEMPOOL_NEWDEL( SwBodyFrame )
IMPL_FIXEDMEMPOOL_NEWDEL( SwHeaderFrame )
IMPL_FIXEDMEMPOOL_NEWDEL( SwFooterFrame )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTextFrame )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableFormat )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableLineFormat )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableBoxFormat )
IMPL_FIXEDMEMPOOL_NEWDEL( _SwCursor_SavePos )

Size GetGraphicSizeTwip(const Graphic& rGraphic, vcl::RenderContext* pOutDev)
{
    const MapMode aMapTwip(MAP_TWIP);
    Size aSize(rGraphic.GetPrefSize());
    if (MAP_PIXEL == rGraphic.GetPrefMapMode().GetMapUnit())
    {
        if (!pOutDev)
            pOutDev = Application::GetDefaultDevice();
        aSize = pOutDev->PixelToLogic(aSize, aMapTwip);
    }
    else
    {
        aSize = OutputDevice::LogicToLogic(aSize, rGraphic.GetPrefMapMode(), aMapTwip);
    }
    return aSize;
}

uno::Reference< linguistic2::XSpellChecker1 >  GetSpellChecker()
{
    return LinguMgr::GetSpellChecker();
}

uno::Reference< linguistic2::XHyphenator >  GetHyphenator()
{
    return LinguMgr::GetHyphenator();
}

uno::Reference< linguistic2::XThesaurus >  GetThesaurus()
{
    return LinguMgr::GetThesaurus();
}

uno::Reference< linguistic2::XLinguProperties >  GetLinguPropertySet()
{
    return LinguMgr::GetLinguPropertySet();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
