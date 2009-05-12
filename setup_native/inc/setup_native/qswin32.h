/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: qswin32.h,v $
 * $Revision: 1.4 $
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

#ifndef _QSWIN32_H
#define _QSWIN32_H

#define QUICKSTART_CLASSNAMEA           "SO Listener Class"
#define QUICKSTART_WINDOWNAMEA          "SO Listener Window"
#define SHUTDOWN_QUICKSTART_MESSAGEA    "SO KillTray"

#define QUICKSTART_CLASSNAMEW           L##QUICKSTART_CLASSNAMEA
#define QUICKSTART_WINDOWNAMEW          L##QUICKSTART_WINDOWNAMEA
#define SHUTDOWN_QUICKSTART_MESSAGEW    L##SHUTDOWN_QUICKSTART_MESSAGEA

#ifdef UNICODE
#   define QUICKSTART_CLASSNAME             QUICKSTART_CLASSNAMEW
#   define QUICKSTART_WINDOWNAME            QUICKSTART_WINDOWNAMEW
#   define SHUTDOWN_QUICKSTART_MESSAGE      SHUTDOWN_QUICKSTART_MESSAGEW
#else
#   define QUICKSTART_CLASSNAME             QUICKSTART_CLASSNAMEA
#   define QUICKSTART_WINDOWNAME            QUICKSTART_WINDOWNAMEA
#   define SHUTDOWN_QUICKSTART_MESSAGE      SHUTDOWN_QUICKSTART_MESSAGEA
#endif

#endif /* _QSWIN32_H */
