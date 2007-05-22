/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ActionDescriptionProvider.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 15:19:55 $
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
#include "precompiled_svx.hxx"

#include "ActionDescriptionProvider.hxx"
#include "svdglob.hxx"
#include "svdstr.hrc"

::rtl::OUString ActionDescriptionProvider::createDescription( ActionType eActionType
                        , const ::rtl::OUString& rObjectName )
{
    USHORT nResID=0;
    switch( eActionType )
    {
    case INSERT:
        nResID=STR_UndoInsertObj;
        break;
    case DELETE:
        nResID= STR_EditDelete;
        break;
    case CUT:
        nResID= STR_ExchangeClpCut;
        break;
    case MOVE:
        nResID= STR_EditMove;
        break;
    case RESIZE:
        nResID= STR_EditResize;
        break;
    case ROTATE:
        nResID= STR_EditRotate;
        break;
    case TRANSFORM:
        nResID= STR_EditTransform;
        break;
    case FORMAT:
        nResID= STR_EditSetAttributes;
        break;
    case MOVE_TOTOP:
        nResID= STR_EditMovToTop;
        break;
    case MOVE_TOBOTTOM:
        nResID= STR_EditMovToBtm;
        break;
    case POS_SIZE:
        nResID = STR_EditPosSize;
        break;
    }
    if(!nResID)
        return rtl::OUString();

    XubString aStr(ImpGetResStr(nResID));
    XubString aName(rObjectName);
    aStr.SearchAndReplaceAscii("%O", aName);
    return rtl::OUString(aStr);
}
