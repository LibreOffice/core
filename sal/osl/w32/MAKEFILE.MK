#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: MAKEFILE.MK,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: vg $ $Date: 2007-10-15 12:49:06 $
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

PRJNAME=sal
TARGET=cpposl
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
            $(SLO)$/pipeimpl.obj \
            $(SLO)$/pipe.obj     \
            $(SLO)$/util.obj     \
            $(SLO)$/file.obj\
            $(SLO)$/path_helper.obj\
            $(SLO)$/procimpl.obj \
            $(SLO)$/salinit.obj

OBJFILES=   $(OBJ)$/conditn.obj  \
            $(OBJ)$/diagnose.obj \
            $(OBJ)$/libutil.obj  \
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
            $(OBJ)$/pipeimpl.obj \
            $(OBJ)$/pipe.obj     \
            $(OBJ)$/util.obj     \
            $(OBJ)$/file.obj\
            $(OBJ)$/path_helper.obj\
            $(OBJ)$/procimpl.obj \
            $(OBJ)$/salinit.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


