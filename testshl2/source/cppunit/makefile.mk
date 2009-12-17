#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile$
#
# $Revision$
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
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

PRJNAME=testshl2
TARGET=testshl2
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
# ENABLE_RTTI=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

# CXXFILES = \
# 		  querytemplate.cxx	\
# 		  stringhelper.cxx

# ENVCFLAGSCXX+=-DCPPUNIT_BUILD_DLL

CPPUNIT_TESTSHL_SLOFILES = \
    $(SLO)$/joblist.obj \
    $(SLO)$/t_print.obj \
    $(SLO)$/signaltest.obj

CPPUNIT_STATIC_SOLFILES = \
    $(SLO)$/cmdlinebits.obj \
    $(SLO)$/tresregister.obj \
    $(SLO)$/tresstatewrapper.obj \
    $(SLO)$/registertestfunction.obj


SLOFILES = \
    $(CPPUNIT_TESTSHL_SLOFILES) \
    $(CPPUNIT_STATIC_SOLFILES)
#-------------------------------------------------------------------------------
# This is statically cppunit library
LIB1TARGET= $(LB)$/$(TARGET).lib
LIB1OBJFILES= \
    $(SLOFILES)

.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
LIB1ARCHIV=$(LB)$/lib$(TARGET)$(DLLPOSTFIX).a
.ENDIF
LIB1FILES=$(LB)$/c5t_winstuff.lib
.ENDIF

.IF "$(GUI)" == "UNX"
LIB1ARCHIV=$(LB)$/lib$(TARGET)$(DLLPOSTFIX).a
.ENDIF

#-------------------------------------------------------------------------------
TARGET2=c5t_no_regallfkt
LIB2TARGET= $(LB)$/$(TARGET2).lib
LIB2OBJFILES= \
    $(CPPUNIT_TESTSHL_SLOFILES)

.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
LIB2ARCHIV=$(LB)$/lib$(TARGET2)$(DLLPOSTFIX).a
.ENDIF
.ENDIF

.IF "$(GUI)" == "UNX"
LIB2ARCHIV=$(LB)$/lib$(TARGET2)$(DLLPOSTFIX).a
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

