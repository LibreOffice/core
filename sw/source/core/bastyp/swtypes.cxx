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
String aDotStr('.'); // constant string

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
