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
#ifndef _BINDING_HXX
#define _BINDING_HXX

#ifndef __RSC

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

namespace binfilter {
typedef ULONG SvBindStatus;

typedef ULONG SvStatusCallbackType;
#define SVBSCF_FIRSTDATANOTIFICATION          0x01
#define SVBSCF_LASTDATANOTIFICATION           0x02
#define SVBSCF_INTERMEDIATEDATANOTIFICATION   0x03
#define SVBSCF_RELOADAVAILABLENOTIFICATION    0x04
#define SVBSCF_NEWPARTAVAILABLENOTIFICATION   0x08

typedef ULONG SvBindMode;
#define SVBIND_ASYNC             0x01
#define SVBIND_ASYNCSTORAGE      0x02
#define SVBIND_NEWESTVERSION     0x04
#define SVBIND_SUCCESSONERRORDOC 0x08

enum SvBindAction
{
    BINDACTION_GET    = 0,
    BINDACTION_POST   = 0x1,
    BINDACTION_PUT    = 0x2,
    BINDACTION_CUSTOM = 0x3
};

}

#endif /* __RSC */

#endif /* _BINDING_HXX */
