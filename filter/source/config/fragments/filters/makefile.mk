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
# $Revision: 1.9 $
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

PRJ     = ..$/..$/..$/..
TARGET  = FCFGPkgFilters
PRJNAME = filter

# -----------------------------------------------------------------------------
# include global settings
# -----------------------------------------------------------------------------

.INCLUDE: settings.mk

# -----------------------------------------------------------------------------
# include all package definition files
# -----------------------------------------------------------------------------

.INCLUDE: ..$/packagedef.mk

.INCLUDE: target.mk

ALLTAR: $(ALL_UI_FILTERS)

.IF "$(WITH_LANG)"!=""
$(DIR_LOCFRAG)$/filters$/%.xcu : %.xcu
    -$(MKDIRHIER) $(@:d)
    $(WRAPCMD) $(CFGEX) -p $(PRJNAME) -i $(@:f) -o $@ -m $(LOCALIZESDF) -l all

.IF "$(ALL_UI_FILTERS)"!=""
$(ALL_UI_FILTERS) : $(LOCALIZESDF)
.ENDIF          # "$(ALL_UI_FILTERS)"!=""

.ENDIF 			# "$(WITH_LANG)"!=""
