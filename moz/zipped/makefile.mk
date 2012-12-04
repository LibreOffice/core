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

PRJ=..

PRJNAME=moz
TARGET=moz_unzip

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(WITH_MOZAB4WIN)" == "NO"

dummy:
    @echo "No need to unpack the prebuilt mozab packages"

.ELSE # unpack mozab zips
.INCLUDE :  target.mk

.IF "$(BUILD_MOZAB)"=="NO"
ALLTAR:
    @echo "Does not need build mozab zipped!"
.ELSE
ALLTAR: \
    $(MISC)$/unpacked_$(TARGET)_inc \
    $(MISC)$/unpacked_$(TARGET)_lib \
    $(BIN)$/mozruntime.zip \
    $(MISC)$/remove_old_nss_libs
.ENDIF # BUILD_MOZAB 

$(MISC)$/unpacked_$(TARGET)_lib : $(OS)$(COM)$(CPU)lib.zip
    unzip -o -d $(LB) $(OS)$(COM)$(CPU)lib.zip && \
    $(TOUCH) $@

$(MISC)$/unpacked_$(TARGET)_inc : $(OS)$(COM)$(CPU)inc.zip
    unzip -o -d $(INCCOM) $(OS)$(COM)$(CPU)inc.zip && $(TOUCH)	$@

.ENDIF # WITH_MOZAB4WIN

$(BIN)$/mozruntime%zip : $(OS)$(COM)$(CPU)runtime.zip
    $(COPY) $(OS)$(COM)$(CPU)runtime.zip $(BIN)$/mozruntime.zip


# The old prebuilt moz files include all the old NSS stuff from moz but we
# always build the toplevel nss module, so we must delete all these
LIBLIST= \
    nspr4.lib \
    plc4.lib \
    plds4.lib \
    nss3.lib \
    ssl3.lib\
    smime3.lib

NSS_RUNTIMELIST:= \
    freebl3 \
    nspr4 \
    nss3 \
    nssckbi \
    nssdbm3 \
    nssutil3 \
    plc4 \
    plds4 \
    sqlite3 \
    smime3 \
    softokn3 \
    ssl3

# Remove the nss libs
$(MISC)$/remove_old_nss_libs : $(MISC)$/unpacked_$(TARGET)_lib \
$(MISC)$/unpacked_$(TARGET)_inc $(BIN)$/mozruntime.zip
    $(foreach,lib,$(LIBLIST) rm -f $(LB)$/$(lib) &&) \
    	echo >& $(NULLDEV)
    chmod u+w $(BIN)$/mozruntime.zip && zip -d $(BIN)$/mozruntime.zip $(foreach,lib,$(NSS_RUNTIMELIST) $(DLLPRE)$(lib:f)$(DLLPOST)) && \
    	echo >& $(NULLDEV)
    rm -r -f $(INCCOM)$/nss && \
        echo >& $(NULLDEV)
    rm -r -f $(INCCOM)$/nspr && \
        echo >& $(NULLDEV)
    $(TOUCH) $@
