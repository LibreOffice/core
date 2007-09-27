/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docshdrw.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 10:16:30 $
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



#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif









#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SVX_DRAWITEM_HXX //autogen
#include <svx/drawitem.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDOUTL_HXX
#include <svx/svdoutl.hxx>
#endif
#include <svx/xtable.hxx>

#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif

using namespace ::com::sun::star;

/*--------------------------------------------------------------------
    Beschreibung: Document laden
 --------------------------------------------------------------------*/


void  SwDocShell::InitDraw()
{
    SdrModel *pDrDoc = pDoc->GetDrawModel();
    if( pDrDoc )
    {
        // Listen, bzw. Tables im ItemSet der DocShell anlegen
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
        PutItem( SvxColorTableItem( XColorTable::GetStdColorTable(), SID_COLOR_TABLE ));
}



