/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: qswin32.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:29:05 $
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
