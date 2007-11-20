#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: ihi $ $Date: 2007-11-20 19:46:49 $
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

PRJNAME=sal
TARGET=qa_rtl_textenc

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- BEGIN --------------------------------------------------------
SHL1OBJS=  \
    $(SLO)$/rtl_textcvt.obj
SHL1TARGET= rtl_textcvt
SHL1STDLIBS=\
    $(SALLIB) \
    $(CPPUNITLIB)

SHL1IMPLIB= i$(SHL1TARGET)
DEF1NAME    =$(SHL1TARGET)
.IF "$(OS)$(CPU)$(COMNAME)" == "LINUXIgcc3"
SHL1VERSIONMAP = linuxigcc3_export.map
.ELSE
SHL1VERSIONMAP = $(PRJ)$/qa$/export.map
.ENDIF

# END --------------------------------------------------------------

# --- BEGIN --------------------------------------------------------
SHL2OBJS=  \
    $(SLO)$/rtl_tencinfo.obj
SHL2TARGET= rtl_tencinfo
SHL2STDLIBS=\
    $(SALLIB) \
    $(CPPUNITLIB)

SHL2IMPLIB= i$(SHL2TARGET)
DEF2NAME    =$(SHL2TARGET)
.IF "$(OS)$(CPU)$(COMNAME)" == "LINUXIgcc3"
SHL2VERSIONMAP = linuxigcc3_export.map
.ELSE
SHL2VERSIONMAP = $(PRJ)$/qa$/export.map
.ENDIF

# END --------------------------------------------------------------

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
# SLOFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk
