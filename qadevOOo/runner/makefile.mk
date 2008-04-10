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
# $Revision: 1.14 $
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
PRJNAME = OOoRunner
TARGET  = $(PRJNAME)

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

SUBDIRS_TESTS = mod ifc
SUBDIRS_RUNNER = util share stats lib complexlib helper basicrunner \
            base org$/openoffice convwatch

JARCOMMANDS_TESTS = $(foreach,i,$(SUBDIRS_TESTS) -C $(CLASSDIR) $i)
JARCOMMANDS_RUNNER = $(foreach,i,$(SUBDIRS_RUNNER) -C $(CLASSDIR) $i)

# --- Targets ------------------------------------------------------
.IF "$(SOLAR_JAVA)"=="TRUE"
OWNJAR: ALLTAR


.INCLUDE :  target.mk

# LLA: parameter v is only verbose, need too long!
OWNJAR: LIGHT
    jar cfm $(CLASSDIR)$/$(TARGET).jar manifest -C $(PRJ) objdsc $(JARCOMMANDS_TESTS) $(JARCOMMANDS_RUNNER)

# LLA: parameter v is only verbose, need too long!
LIGHT:
    jar cfm $(CLASSDIR)$/$(TARGET)Light.jar manifest $(JARCOMMANDS_RUNNER)

.ELSE
all:
        @echo "no java"
.ENDIF
