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
    $(BIN)$/mozruntime.zip \
    $(MISC)$/replace_old_nss_libs
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

LIBLIST =
BIN_RUNTIMELIST = 
NSS_INCLUDE_LIST =
NSS_MODULE_RUNTIME_LIST =

.IF "$(ENABLE_NSS_MODULE)"=="YES"
# The prebuilt moz files include all the old NSS stuff from moz and not the new
# NSS libs, headers, etc, from the separate nss module. If we use the nss module
# then we must take care not to deliver the old NSS stuff from the "prebuilts"
.IF "$(GUI)" == "WNT"
    FREEBL_LIB_OLD=freebl3
.ELSE # "$(GUI)" == "WNT"
    .IF "$(OS)$(CPUNAME)" == "SOLARISSPARC"
        FREEBL_LIB_OLD=freebl_32fpu_3
    .ELSE # "$(OS)$(CPUNAME)" == "SOLARISSPARC"
        FREEBL_LIB_OLD=freebl3
    .ENDIF # "$(OS)$(CPUNAME)" == "SOLARISSPARC"
.ENDIF # "$(GUI)" == "WNT"

BIN_RUNTIMELIST= \
    $(FREEBL_LIB_OLD) \
    nspr4 \
    plc4 \
    plds4 \
    nss3 \
    ssl3 \
    softokn3 \
    smime3

.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
LIBLIST= \
    libnspr4.a \
    libnss3.a \
    libsmime3.a \
    libplc4.a \
    libplds4.a \
    libssl3.a

.ELSE
LIBLIST= \
    nspr4.lib \
    plc4.lib \
    plds4.lib \
    nss3.lib \
    ssl3.lib\
    smime3.lib
.ENDIF
.ELSE   #"$(GUI)"=="WNT"
LIBLIST= \
    lib$(FREEBL_LIB_OLD)$(DLLPOST) \
    libnspr4$(DLLPOST) \
    libsoftokn3$(DLLPOST) \
    libplc4$(DLLPOST) \
    libplds4$(DLLPOST) \
    libnss3$(DLLPOST) \
    libssl3$(DLLPOST) \
    libsmime3$(DLLPOST)

.ENDIF # .IF "$(GUI)"=="WNT"


NSS_INCLUDE_LIST= nspr nss


.IF "$(OS)" == "SOLARIS" 
.IF "$(CPU)" == "S" #32bit
FREEBL=freebl_32fpu_3 freebl_32int64_3 freebl_32int_3
.ELIF "$(CPU)" == "U" #64bit unxsolu4
FREEBL=freebl_64int_3 freebl_64fpu_3
.ELSE
FREEBL=freebl3
.ENDIF #"$(CPU)" == "S"

.ELSE # "$(OS)" == "SOLARIS" 
FREEBL=freebl3
.ENDIF # "$(OS)" == "SOLARIS" 


NSS_MODULE_RUNTIME_LIST:= \
    $(FREEBL) \
    nspr4 \
    nss3 \
    nssckbi \
    nssdbm3 \
    nssutil3 \
    plc4 \
    plds4 \
    smime3 \
    softokn3 \
    sqlite3 \
    ssl3


.ENDIF #.IF "$(ENABLE_NSS_MODULE)"=="YES"
# Remove the nss libs build in moz and those build in the nss module
$(MISC)$/replace_old_nss_libs : $(MISC)$/unpacked_$(TARGET)_lib \
$(MISC)$/unpacked_$(TARGET)_inc $(BIN)$/mozruntime.zip
    +$(foreach,dir,$(NSS_INCLUDE_LIST) $(RENAME:s/+//) $(INCCOM)$/$(dir) \
    $(INCCOM)$/$(dir)_remove_me &&)  echo >& $(NULLDEV)
    $(foreach,dir,$(NSS_INCLUDE_LIST) rm -r -f $(INCCOM)$/$(dir)_remove_me &&) \
        echo >& $(NULLDEV)
    $(foreach,lib,$(LIBLIST) rm -f $(LB)$/$(lib) &&) \
    echo >& $(NULLDEV)
    $(foreach,lib,$(BIN_RUNTIMELIST) zip -d $(BIN)$/mozruntime.zip $(DLLPRE)$(lib)$(DLLPOST) &&) \
    echo >& $(NULLDEV)
.IF "$(GUI)"=="WNT"
    $(foreach,lib,$(NSS_MODULE_RUNTIME_LIST) zip -g -j $(BIN)$/mozruntime.zip $(SOLARBINDIR)$/$(DLLPRE)$(lib)$(DLLPOST) &&) \
    echo >& $(NULLDEV)
.ELSE
    $(foreach,lib,$(NSS_MODULE_RUNTIME_LIST) zip -g -j $(BIN)$/mozruntime.zip $(SOLARLIBDIR)$/$(DLLPRE)$(lib)$(DLLPOST) &&) \
    echo >& $(NULLDEV)
.ENDIF
    $(TOUCH) $@     

.ENDIF   # unpack mozab zips

