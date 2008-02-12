#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: vg $ $Date: 2008-02-12 16:25:46 $
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
    $(SLB)$/vector.lib

SHL1TARGET= tests
SHL1STDLIBS= \
                $(SALLIB)        \
                $(CPPUHELPERLIB) \
                        $(CPPULIB)       \
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
        testshl2 -forward $(BIN)$/ $(SHL1TARGETN)
        $(TOUCH) $@

ALLTAR : $(MISC)$/unittest_succeeded
