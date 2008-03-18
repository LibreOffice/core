/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swtypes.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 15:54:41 $
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
#include "precompiled_sw.hxx"

#include "swtypes.hxx"

#ifndef _STRING_HXX
#include "tools/string.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif

#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _SWATRSET_HXX
#include <swatrset.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _FNTCACHE_HXX
#include <fntcache.hxx>
#endif
#ifndef _SWFNTCCH_HXX
#include <swfntcch.hxx>
#endif
#ifndef _HFFRM_HXX
#include <hffrm.hxx>
#endif
#ifndef _COLFRM_HXX
#include <colfrm.hxx>
#endif
#ifndef _BODYFRM_HXX
#include <bodyfrm.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _SWTBLFMT_HXX
#include <swtblfmt.hxx>
#endif
#ifndef _ROWFRM_HXX
#include <rowfrm.hxx>
#endif
#ifndef _CELLFRM_HXX
#include <cellfrm.hxx>
#endif
#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>
#endif

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


