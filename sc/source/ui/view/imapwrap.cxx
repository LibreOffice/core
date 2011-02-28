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
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <svx/imapdlg.hxx>
#include <sfx2/viewfrm.hxx>


sal_uInt16 ScIMapChildWindowId()
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




