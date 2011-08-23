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

#ifndef _SWERROR_H
#define _SWERROR_H

#define ERR_SWGBASE 25000

/*** Error Codes **********************************************************/
#define ERR_VAR_IDX					(ERR_SWGBASE+ 0)
#define ERR_OUTOFSCOPE      		(ERR_SWGBASE+ 1)

/* Error Codes fuer Numerierungsregeln */
#define ERR_NUMLEVEL				(ERR_SWGBASE+ 2)

/* Error Codes fuer TxtNode */
#define ERR_NOHINTS					(ERR_SWGBASE+ 3)

// von _START bis _END sind in der COREDL.DLL entsprechende Strings
// abgelegt, die erfragt werden koennen.
#define ERR_SWGMSG_START			(ERR_VAR_IDX)
#define ERR_SWGMSG_END				(ERR_NOHINTS)



#endif
