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
#include "precompiled_sw.hxx"

#include "swtypes.hxx"
#include "tools/string.hxx"
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/graph.hxx>
#include <editeng/unolingu.hxx>
#include <pagefrm.hxx>
#include <swatrset.hxx>
#include <frmfmt.hxx>
#include <frmtool.hxx>
#include <ndtxt.hxx>
#include <UndoDelete.hxx>
#include <UndoInsert.hxx>
#include <swtable.hxx>
#include <viscrs.hxx>
#include <fntcache.hxx>
#include <swfntcch.hxx>
#include <hffrm.hxx>
#include <colfrm.hxx>
#include <bodyfrm.hxx>
#include <tabfrm.hxx>
#include <txtfrm.hxx>
#include <swtblfmt.hxx>
#include <rowfrm.hxx>
#include <cellfrm.hxx>
#include <sectfrm.hxx>

using namespace com::sun::star;

ByteString aEmptyByteStr;       // Konstante Strings
String aEmptyStr;               // Konstante Strings
String aDotStr('.');            // Konstante Strings

IMPL_FIXEDMEMPOOL_NEWDEL( SwAttrSet, 25, 25 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwStartNode, 20, 20 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwEndNode,   20, 20 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableBox, 50, 50 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwUndoDelete, 10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwUndoInsert, 10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwPaM, 10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwCursor, 10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwShellCrsr, 10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTxtNode, 50, 50 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwpHints, 25, 25 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwFntObj, 50, 50 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwFontObj, 50, 50 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwBorderAttrs, 100, 100 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwCellFrm,    50, 50 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwRowFrm,     10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwColumnFrm,  40, 40 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwSectionFrm, 20, 20 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTabFrm, 10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwPageFrm,    20, 20 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwBodyFrm,    20, 20 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwHeaderFrm,  20, 20 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwFooterFrm,  20, 20 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTxtFrm,     50,  50 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableFmt, 10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableLineFmt, 10, 10 )
IMPL_FIXEDMEMPOOL_NEWDEL( SwTableBoxFmt, 50, 50 )
IMPL_FIXEDMEMPOOL_NEWDEL( _SwCursor_SavePos, 20, 20 )


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
        aSize = OutputDevice::LogicToLogic( aSize,
                                        rGraphic.GetPrefMapMode(), aMapTwip );
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


