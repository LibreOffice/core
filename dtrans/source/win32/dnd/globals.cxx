/*************************************************************************
 *
 *  $RCSfile: globals.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-23 09:39:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_DNDCONSTANTS_HPP_
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#endif

#include "globals.hxx"

//--> TRA
#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

// used as shortcut when drag-source and drop-target are the same
::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable > g_XTransferable;

//<-- TRA

using namespace com::sun::star::datatransfer::dnd::DNDConstants;

sal_Int8 dndOleKeysToAction( DWORD grfKeyState)
{
    sal_Int8 ret= 0;

    // no MK_ALT, MK_CONTROL, MK_SHIFT
    if( !(grfKeyState & MK_CONTROL) &&
        !(grfKeyState & MK_ALT)    &&
        !(grfKeyState & MK_RBUTTON) )
    {
        ret= ACTION_MOVE;
    }
    else if ( grfKeyState & MK_CONTROL &&
              !(grfKeyState & MK_SHIFT) )
    {
        ret= ACTION_COPY;
    }
    else if ( grfKeyState & MK_CONTROL &&
              grfKeyState & MK_SHIFT)
    {
        ret= ACTION_LINK;
    }
    else if ( grfKeyState & MK_RBUTTON |
              grfKeyState & MK_ALT)
    {
        ret= ACTION_COPY_OR_MOVE | ACTION_LINK;
    }
    return ret;
}


sal_Int8 dndOleDropEffectsToActions( DWORD dwEffect)
{
    sal_Int8 ret= ACTION_NONE;
    if( dwEffect & DROPEFFECT_COPY)
        ret |= ACTION_COPY;
    if( dwEffect & DROPEFFECT_MOVE)
        ret |= ACTION_MOVE;
    if( dwEffect & DROPEFFECT_LINK)
        ret |= ACTION_LINK;

    return ret;
}

DWORD dndActionsToDropEffects( sal_Int8 actions)
{
    DWORD ret= DROPEFFECT_NONE;
    if( actions & ACTION_MOVE)
        ret |= DROPEFFECT_MOVE;
    if( actions & ACTION_COPY)
        ret |= DROPEFFECT_COPY;
    if( actions & ACTION_LINK)
        ret |= DROPEFFECT_LINK;
    return ret;
}

DWORD dndActionsToSingleDropEffect( sal_Int8 actions)
{
    DWORD effects= dndActionsToDropEffects( actions);

    sal_Int8 countEffect= 0;

    if( effects & DROPEFFECT_MOVE)
        countEffect++;
    if( effects & DROPEFFECT_COPY)
        countEffect++;
    if( effects & DROPEFFECT_LINK)
        countEffect++;

    // DROPEFFECT_MOVE is the default effect
    DWORD retVal= countEffect > 1 ? DROPEFFECT_MOVE : effects;
    return retVal;
}
