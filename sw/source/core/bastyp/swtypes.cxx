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
#include "tools/string.hxx"

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

String aEmptyStr;    // constant string
OUString aEmptyOUStr;  // remove once aEmptyStr can be changed to OUString
OUString aDotStr('.'); // constant string

IMPL_FIXEDMEMPOOL_NEWDEL( SwAttrSet )
IMPL_FIXEDMEMPOOL_NEWDEL( SwStartNode )
IMPL_FIXEDMEMPOOL_NEWDEL( SwEndNode )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableBox )
IMPL_FIXEDMEMPOOL_NEWDEL( SwUndoDelete )
IMPL_FIXEDMEMPOOL_NEWDEL( SwUndoInsert )
IMPL_FIXEDMEMPOOL_NEWDEL( SwPaM )
IMPL_FIXEDMEMPOOL_NEWDEL( SwCursor )
IMPL_FIXEDMEMPOOL_NEWDEL( SwShellCrsr )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTxtNode )
IMPL_FIXEDMEMPOOL_NEWDEL( SwpHints )
IMPL_FIXEDMEMPOOL_NEWDEL( SwFntObj )
IMPL_FIXEDMEMPOOL_NEWDEL( SwFontObj )
IMPL_FIXEDMEMPOOL_NEWDEL( SwBorderAttrs )
IMPL_FIXEDMEMPOOL_NEWDEL( SwCellFrm )
IMPL_FIXEDMEMPOOL_NEWDEL( SwRowFrm )
IMPL_FIXEDMEMPOOL_NEWDEL( SwColumnFrm )
IMPL_FIXEDMEMPOOL_NEWDEL( SwSectionFrm )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTabFrm )
IMPL_FIXEDMEMPOOL_NEWDEL( SwPageFrm )
IMPL_FIXEDMEMPOOL_NEWDEL( SwBodyFrm )
IMPL_FIXEDMEMPOOL_NEWDEL( SwHeaderFrm )
IMPL_FIXEDMEMPOOL_NEWDEL( SwFooterFrm )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTxtFrm )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableFmt )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableLineFmt )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableBoxFmt )
IMPL_FIXEDMEMPOOL_NEWDEL( _SwCursor_SavePos )

Size GetGraphicSizeTwip( const Graphic& rGraphic, OutputDevice* pOutDev )
{
    const MapMode aMapTwip( MAP_TWIP );
    Size aSize( rGraphic.GetPrefSize() );
    if( MAP_PIXEL == rGraphic.GetPrefMapMode().GetMapUnit() )
    {
        if( !pOutDev )
            pOutDev = Application::GetDefaultDevice();
        aSize = pOutDev->PixelToLogic( aSize, aMapTwip );
    }
    else
    {
        aSize = OutputDevice::LogicToLogic( aSize,
                                            rGraphic.GetPrefMapMode(),
                                            aMapTwip );
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

uno::Reference< beans::XPropertySet >  GetLinguPropertySet()
{
    return LinguMgr::GetLinguPropertySet();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
