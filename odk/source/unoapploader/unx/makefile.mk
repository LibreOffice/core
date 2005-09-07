#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:19:41 $
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

PRJ=..$/..$/..

PRJNAME=odk
TARGET=unoapploader
LIBTARGET=NO

LIBSALCPPRT=

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(OS)$(COMID)"=="SOLARISC52"
LINK=cc
LINKFLAGS=
LINKFLAGSAPP=
STDLIB= -lX11 -ldl -lc -lm
.ENDIF          # "$(OS)$(COMID)"=="SOLARISC52"

.IF "$(COM)"=="GCC"
#LINK=gcc
LINK=$(CC)
LINKFLAGS=
LINKFLAGSAPP=
  .IF "$(OS)"=="FREEBSD"
  STDLIB= -lX11 -lc -lm
  .ELSE
  STDLIB= -lX11 -ldl -lc -lm
.ENDIF # "$(OS)"=="FREEBSD"
.ENDIF          # "$(OS)$(COMID)"=="GCC"

APP1NOSAL=TRUE

APP1TARGET= $(TARGET)

APP1OBJS=   $(OBJ)$/unoapploader.obj

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
