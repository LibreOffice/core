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
# $Revision: 1.17 $
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

PRJNAME=moz
TARGET=moz_unzip

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(PREBUILD_MOZAB)" == "" || "$(SYSTEM_MOZILLA)" == "YES" || "$(WITH_MOZILLA)" == "NO"

dummy:
    @echo "No need to unpack the prebuild mozab packages"

.ELSE # unpack mozab zips
.INCLUDE :  target.mk

.IF "$(BUILD_MOZAB)"=="TRUE"
ALLTAR:
    @echo "Does not need build mozab zipped!"
.ELSE
ALLTAR: \
    $(MISC)$/unpacked_$(TARGET)_inc \
    $(MISC)$/unpacked_$(TARGET)_lib \
    $(BIN)$/mozruntime.zip
.ENDIF
.IF "$(GUI)" == "UNX"

$(MISC)$/unpacked_$(TARGET)_lib : $(OS)$(COM)$(CPU)lib.zip $(OS)$(COM)$(CPU)runtime.zip
    unzip -o -d $(LB) $(OS)$(COM)$(CPU)lib.zip && unzip -o -d $(LB) $(OS)$(COM)$(CPU)runtime.zip && $(TOUCH) $@
    chmod -R 775 $(LB)

$(MISC)$/unpacked_$(TARGET)_inc : $(OS)$(COM)$(CPU)inc.zip
    unzip -o -d $(INCCOM) $(OS)$(COM)$(CPU)inc.zip && $(TOUCH)	$@
    chmod -R 775 $(INCCOM)

.ELSE

$(MISC)$/unpacked_$(TARGET)_lib : $(OS)$(COM)$(CPU)lib.zip
    unzip -o -d $(LB) $(OS)$(COM)$(CPU)lib.zip && \
    $(TOUCH) $@

$(MISC)$/unpacked_$(TARGET)_inc : $(OS)$(COM)$(CPU)inc.zip
    unzip -o -d $(INCCOM) $(OS)$(COM)$(CPU)inc.zip && $(TOUCH)	$@

.ENDIF

$(BIN)$/mozruntime.zip : $(OS)$(COM)$(CPU)runtime.zip
    $(COPY) $(OS)$(COM)$(CPU)runtime.zip $(BIN)$/mozruntime.zip


.ENDIF	 # unpack mozab zips

