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

#include "system.h"
#include "sockimpl.h"
#include "secimpl.h"

#include <osl/diagnose.h>

#ifndef GCC
sal_uInt32 _System _DLL_InitTerm( sal_uInt32 nModule,
                                     sal_uInt32 nFlag )
{
    switch( nFlag )
    {
        case 0:
        {
            /* initialize C runtime library */
            _CRT_init();
            {
                LONG fhToAdd = 0;
                ULONG fhOld = 0;
                ULONG ngLastError = DosSetRelMaxFH(&fhToAdd, &fhOld);
                if (fhOld < 200)
                    ngLastError = DosSetMaxFH(200);
            }

            /* turn off hardware-errors and exception popups */
            DosError(FERR_DISABLEHARDERR | FERR_DISABLEEXCEPTION);

            break;
        }

        case 1:
        {
            /* unload libs (sockets) */
            ImplFreeTCPIP();

            /* unload libs (security) */
            ImplFreeUPM();

            break;
        }
    }

    return (sal_True);
}
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
