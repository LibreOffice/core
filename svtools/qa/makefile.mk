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
# $Revision: 1.5 $
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

PRJ = ..
PRJNAME = svtools
TARGET = qa

ENABLE_EXCEPTIONS = true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# BEGIN ----------------------------------------------------------------
# auto generated Target:job by codegen.pl
SHL1OBJS=  \
    $(SLO)$/test_URIHelper.obj

SHL1TARGET= URIHelper
SHL1STDLIBS=\
     $(CPPULIB) \
     $(CPPUHELPERLIB) \
     $(SALLIB) \
     $(SVTOOLLIB) \
     $(TOOLSLIB) \
     $(UNOTOOLSLIB) \
     $(TESTSHL2LIB)    \
     $(CPPUNITLIB)

SHL1IMPLIB= i$(SHL1TARGET)
DEF1NAME    =$(SHL1TARGET)
SHL1VERSIONMAP= export.map
# auto generated Target:job
# END ------------------------------------------------------------------

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
# SLOFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk

# LLA: old stuff
# USE_DEFFILE = true
#
# .INCLUDE: settings.mk
#
# .IF "$(OS)" == "WNT"
# REGEXP = "s/^[\#].*$$//"
# .ELSE # OS, WNT
# REGEXP = 's/^[\#].*$$//'
# .ENDIF # OS, WNT
#
# SHL1TARGET = URIHelper
# SHL1OBJS = \
# 	$(SLO)$/test_URIHelper.obj
# SHL1STDLIBS = \
#     $(CPPULIB) \
#     $(CPPUHELPERLIB) \
#     $(SALLIB) \
#     $(SVTOOLLIB) \
#     $(TOOLSLIB) \
#     $(UNOTOOLSLIB)
#
# DEF1NAME = $(SHL1TARGET)
# DEF1EXPORTFILE = $(MISC)$/$(SHL1TARGET).dxp
#
# .INCLUDE: target.mk
#
# $(MISC)$/$(SHL1TARGET).dxp: sce$/$(SHL1TARGET).sce
#     + $(TYPE) $< | sed $(REGEXP) > $@
#     + $(TYPE) $@ | sed "s/^/test_/" > $(MISC)$/$(SHL1TARGET).tst
#     + $(TYPE) $(MISC)$/$(SHL1TARGET).tst | sed "/test_./ w $@"
