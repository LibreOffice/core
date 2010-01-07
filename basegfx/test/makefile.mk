#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.7 $
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

PRJ=..

PRJNAME=basegfx
TARGET=tests

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Common ----------------------------------------------------------

SHL1OBJS=  \
    $(SLO)$/basegfx1d.obj \
    $(SLO)$/basegfx2d.obj \
    $(SLO)$/basegfx3d.obj \
    $(SLO)$/testtools.obj

# linking statically against basegfx parts
SHL1LIBS=\
    $(SLB)$/curve.lib	\
    $(SLB)$/matrix.lib	\
    $(SLB)$/numeric.lib	\
    $(SLB)$/point.lib	\
    $(SLB)$/polygon.lib	\
    $(SLB)$/range.lib	\
    $(SLB)$/tuple.lib	\
    $(SLB)$/tools.lib	\
    $(SLB)$/color.lib	\
    $(SLB)$/vector.lib

SHL1TARGET= basegfx_tests
SHL1STDLIBS= \
                $(SALLIB)        \
                $(CPPUHELPERLIB) \
                        $(CPPULIB)       \
                                $(TESTSHL2LIB) \
                $(CPPUNITLIB)

SHL1IMPLIB= i$(SHL1TARGET)

DEF1NAME    =$(SHL1TARGET)
SHL1VERSIONMAP = export.map

# END ------------------------------------------------------------------

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
SLOFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
.INCLUDE : _cppunit.mk

# --- Enable testshl2 execution in normal build ------------------------

$(MISC)$/unittest_succeeded : $(SHL1TARGETN)
        @echo ----------------------------------------------------------
        @echo - start unit test on library $(SHL1TARGETN)
        @echo ----------------------------------------------------------
        $(TESTSHL2) -sf $(mktmp ) -forward $(BIN)$/ $(SHL1TARGETN)
        $(TOUCH) $@

ALLTAR : $(MISC)$/unittest_succeeded
