/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imapwrap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:59:59 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <svx/imapdlg.hxx>
#include <sfx2/viewfrm.hxx>


USHORT ScIMapChildWindowId()
{
    return SvxIMapDlgChildWindow::GetChildWindowId();
}

SvxIMapDlg* ScGetIMapDlg()
{
    //! pass view frame here and in SVXIMAPDLG()

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if( pViewFrm && pViewFrm->HasChildWindow( SvxIMapDlgChildWindow::GetChildWindowId() ) )
        return SVXIMAPDLG();
    else
        return NULL;
}

void ScIMapDlgSet( const Graphic& rGraphic, const ImageMap* pImageMap,
                    const TargetList* pTargetList, void* pEditingObj )
{
    SvxIMapDlgChildWindow::UpdateIMapDlg( rGraphic, pImageMap, pTargetList, pEditingObj );
}

const void* ScIMapDlgGetObj( SvxIMapDlg* pDlg )
{
    if ( pDlg )
        return pDlg->GetEditingObject();
    else
        return NULL;
}

const ImageMap& ScIMapDlgGetMap( SvxIMapDlg* pDlg )
{
    return pDlg->GetImageMap();
}




