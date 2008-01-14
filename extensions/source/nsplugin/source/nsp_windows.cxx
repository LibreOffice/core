/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nsp_windows.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:44:49 $
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
#include "precompiled_extensions.hxx"

#ifdef WNT
    #pragma warning (push,1)
    #pragma warning (disable:4668)
    #include <windows.h>
    #pragma warning (pop)
#endif

int NSP_ResetWinStyl(long hParent)
{
    int ret = 0;
#ifdef WNT
    LONG dOldStyl = GetWindowLong( (HWND)hParent, GWL_STYLE);
    ret = dOldStyl;

    if(ret != 0)
    {
        LONG dNewStyl = dOldStyl|WS_CLIPCHILDREN;
        if(0 == SetWindowLong((HWND)hParent, GWL_STYLE, dNewStyl))
            ret = 0;
    }
#endif
    return ret;

}

int NSP_RestoreWinStyl(long hParent, long dOldStyle)
{
    int ret = 0;
#ifdef WNT
    ret = SetWindowLong((HWND)hParent, GWL_STYLE, dOldStyle);
#endif
    return ret;
}
