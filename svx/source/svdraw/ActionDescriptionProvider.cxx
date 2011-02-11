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
#include "precompiled_svx.hxx"

#include "svx/ActionDescriptionProvider.hxx"
#include "svx/svdglob.hxx"
#include "svx/svdstr.hrc"

::rtl::OUString ActionDescriptionProvider::createDescription( ActionType eActionType
                        , const ::rtl::OUString& rObjectName )
{
    sal_uInt16 nResID=0;
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
    aStr.SearchAndReplaceAscii("%1", aName);
    return rtl::OUString(aStr);
}
