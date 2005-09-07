#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 16:29:48 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..

PRJNAME=setup_native
TARGET=getuid

NO_DEFAULT_STL=TRUE
LIBSALCPPRT=$(0)
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

# no 'lib' prefix
DLLPRE=

# no _version.o
VERSIONOBJ=

.IF "$(OS)" == "LINUX" || "$(OS)" == "SOLARIS"

.IF "$(OS)" == "LINUX" 
CFLAGS+=-D_GNU_SOURCE
.ENDIF

.IF "$(OS)" == "SOLARIS"
.IF "$(CPU)" == "I"
.IF "$(COM)" != "GCC"
CFLAGS+=-D_KERNEL
.ENDIF
.ENDIF
.ENDIF

# this object must not be a Ultra SPARC binary, this does not
# work with /usr/bin/sort and such.
.IF "$(OS)" == "SOLARIS"
.IF "$(CPU)" == "S"
.IF "$(COM)" != "GCC"
ENVCFLAGS=-xarch=v8
.ENDIF
.ENDIF
.ENDIF

SLOFILES = $(SLO)$/getuid.obj

SHL1OBJS=$(SLOFILES)
SHL1DESCRIPTIONOBJ=
SHL1TARGET=$(TARGET)
SHL1STDLIBS=-ldl

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

