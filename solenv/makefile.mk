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

PRJ=.
PRJNAME=solenv
TARGET=init

# --- Settings ----------------------------------
.INCLUDE : settings.mk
.INCLUDE : target.mk
.INCLUDE : versionlist.mk

ALLTAR : $(INCCOM)/versionlist.hrc

$(INCCOM)/versionlist.hrc : ./inc/version.lst
    $(COMMAND_ECHO)echo "#define VERSION $(OOOBASEVERSIONMAJOR)" > $@
    $(COMMAND_ECHO)echo "#define SUBVERSION $(OOOBASEVERSIONMINOR)" >> $@
    $(COMMAND_ECHO)echo "#define MICROVERSION $(OOOBASEVERSIONMICRO)" >> $@
    $(COMMAND_ECHO)echo "#define VER_DAY $(OOOBASEVERSIONDAY)" >> $@
    $(COMMAND_ECHO)echo "#define VER_MONTH $(OOOBASEVERSIONMONTH)" >> $@
    $(COMMAND_ECHO)echo "#define VER_YEAR $(OOOBASEVERSIONYEAR)" >> $@
