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


#include "nsp_windows.hxx"

int NSP_ResetWinStyl(LONG_PTR hParent)
{
    int ret = 0;
#ifdef WNT
    LONG_PTR dOldStyl = GetWindowLongPtr( (HWND)hParent, GWL_STYLE);
    ret = dOldStyl;

    if(ret != 0)
    {
        LONG_PTR dNewStyl = dOldStyl|WS_CLIPCHILDREN;
        if(0 == SetWindowLongPtr((HWND)hParent, GWL_STYLE, dNewStyl))
            ret = 0;
    }
#endif
    return ret;

}

int NSP_RestoreWinStyl(LONG_PTR hParent, LONG_PTR dOldStyle)
{
    int ret = 0;
#ifdef WNT
    ret = SetWindowLongPtr((HWND)hParent, GWL_STYLE, dOldStyle);
#endif
    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
