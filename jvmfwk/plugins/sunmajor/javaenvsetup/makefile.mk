#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 19:29:10 $
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

PRJNAME=javaldx
TARGET=javaldx
TARGETTYPE=CUI
.IF "$(OS)"!="IRIX"
NO_DEFAULT_STL=TRUE
.ENDIF
LIBTARGET=NO
ENABLE_EXCEPTIONS=true

.IF "$(OS)" == "LINUX"
LINKFLAGSRUNPATH = -Wl,-rpath,\''$$ORIGIN/../lib:$$ORIGIN'\'
.ELIF "$(OS)" == "SOLARIS"
LINKFLAGSRUNPATH = -R\''$$ORIGIN/../lib:$$ORIGIN'\'
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(OS)"=="MACOSX"
STDLIBCUIMT+=-lstdc++
.ENDIF

.IF "$(SOLAR_JAVA)"==""
nojava:
    @echo "Not building jvmfwk/plugins/javaenvsetup/ because Java is disabled"
.ENDIF

# --- Files --------------------------------------------------------

OBJFILES=$(OBJ)$/javaldx.obj

APP1TARGET=javaldx
APP1OBJS=$(OBJFILES)
APP1STDLIBS=$(SALLIB) $(CPPULIB) $(CPPUHELPERLIB) $(JVMFWKLIB)


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
