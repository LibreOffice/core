#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: ihi $ $Date: 2007-11-20 19:37:46 $
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
INCPRE+= $(PRJ)$/qa$/inc

PRJNAME=sal
TARGET=rtl_bootstrap

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(GUI)"=="WNT"
#BOOTSTRAPSCRIPT=bootstrap.bat
BOOTSTRAPINI=testshl2.ini
MY_SCRIPTCAT=cat
.ELSE
#BOOTSTRAPSCRIPT=bootstrap
BOOTSTRAPINI=testshl2rc
MY_SCRIPTCAT=tr -d "\015" <
.ENDIF

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# BEGIN ----------------------------------------------------------------
# auto generated Target:joblist by codegen.pl 
SHL1OBJS=  \
    $(SLO)$/rtl_Bootstrap.obj

SHL1TARGET= rtl_Bootstrap
SHL1STDLIBS=\
   $(SALLIB) \
   $(CPPUNITLIB)

SHL1IMPLIB= i$(SHL1TARGET)
# SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)
# DEF1EXPORTFILE= export.exp
SHL1VERSIONMAP= $(PRJ)$/qa$/export.map
# END ------------------------------------------------------------------

OBJ2FILES=$(OBJ)$/bootstrap_process.obj 
APP2TARGET=bootstrap_process
APP2OBJS=$(OBJ2FILES)

# .IF "$(GUI)" == "UNX"
# APP2STDLIBS=$(LB)$/libsal.so
# .ENDIF
# .IF "$(GUI)" == "WNT"
# APP2STDLIBS=$(KERNEL32LIB) $(LB)$/isal.lib
# .ENDIF
APP2STDLIBS=$(SALLIB)
#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
# SLOFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk

