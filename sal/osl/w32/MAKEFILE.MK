#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=sal
TARGET=cpposl
ENABLE_EXCEPTIONS=TRUE
USE_LDUMP2=TRUE

PROJECTPCH4DLL=TRUE
PROJECTPCH=cont_pch
PROJECTPCHSOURCE=cont_pch

MULTITHREAD_OBJ=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# Uncomment the following line if you want to compile with -Wall
# it disables lots of unnecessary warnings comming from the Win32
# header files (disabling warnings via command line only works with
# MSVC >= 7.x)
#
# CFLAGS+=/wd4668 /wd4711 /wd4201 /wd4710 /wd4619 /wd4514 /wd4820

# --- Files --------------------------------------------------------
.IF "$(CCNUMVER)" >= "001400000000"
CDEFS+=-D_CRT_NON_CONFORMING_SWPRINTFS
.ENDIF

.IF "$(header)" == ""

SLOFILES=   $(SLO)$/conditn.obj  \
            $(SLO)$/diagnose.obj \
            $(SLO)$/dllentry.obj \
            $(SLO)$/semaphor.obj \
            $(SLO)$/socket.obj   \
            $(SLO)$/interlck.obj \
            $(SLO)$/nlsupport.obj\
            $(SLO)$/mutex.obj    \
            $(SLO)$/thread.obj   \
            $(SLO)$/module.obj   \
            $(SLO)$/process.obj  \
            $(SLO)$/security.obj \
            $(SLO)$/profile.obj  \
            $(SLO)$/time.obj     \
            $(SLO)$/signal.obj   \
            $(SLO)$/pipe.obj     \
            $(SLO)$/util.obj     \
            $(SLO)$/file.obj\
            $(SLO)$/file_dirvol.obj\
            $(SLO)$/file_error.obj\
            $(SLO)$/file_url.obj\
            $(SLO)$/tempfile.obj\
            $(SLO)$/path_helper.obj\
            $(SLO)$/procimpl.obj \
            $(SLO)$/salinit.obj

OBJFILES=   $(OBJ)$/conditn.obj  \
            $(OBJ)$/diagnose.obj \
            $(OBJ)$/semaphor.obj \
            $(OBJ)$/socket.obj   \
            $(OBJ)$/interlck.obj \
            $(OBJ)$/nlsupport.obj\
            $(OBJ)$/mutex.obj    \
            $(OBJ)$/thread.obj   \
            $(OBJ)$/module.obj   \
            $(OBJ)$/process.obj  \
            $(OBJ)$/security.obj \
            $(OBJ)$/profile.obj  \
            $(OBJ)$/time.obj     \
            $(OBJ)$/signal.obj   \
            $(OBJ)$/pipe.obj     \
            $(OBJ)$/util.obj     \
            $(OBJ)$/file.obj\
            $(OBJ)$/file_dirvol.obj\
            $(OBJ)$/file_error.obj\
            $(OBJ)$/file_url.obj\
            $(OBJ)$/tempfile.obj\
            $(OBJ)$/path_helper.obj\
            $(OBJ)$/procimpl.obj \
            $(OBJ)$/salinit.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
