#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=..

PRJNAME=moz
TARGET=moz_unzip

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(PREBUILD_MOZAB)" == "" || "$(SYSTEM_MOZILLA)" == "YES" || "$(WITH_MOZILLA)" == "NO"

dummy:
    @echo "No need to unpack the prebuilt mozab packages"

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

$(MISC)$/unpacked_%_lib : $(OS)$(COM)$(CPU)lib.zip '$(OS)$(COM)$(CPU)runtime.zip'
    unzip -o -d $(LB) $(OS)$(COM)$(CPU)lib.zip && unzip -o -d $(LB) $(OS)$(COM)$(CPU)runtime.zip && $(TOUCH) $@
    chmod -R 775 $(LB)

$(MISC)$/unpacked_%_inc : $(OS)$(COM)$(CPU)inc.zip
    unzip -o -d $(INCCOM) $(OS)$(COM)$(CPU)inc.zip && $(TOUCH)	$@
    chmod -R 775 $(INCCOM)

.ELSE

$(MISC)$/unpacked_$(TARGET)_lib : $(OS)$(COM)$(CPU)lib.zip
    unzip -o -d $(LB) $(OS)$(COM)$(CPU)lib.zip && \
    $(TOUCH) $@

$(MISC)$/unpacked_$(TARGET)_inc : $(OS)$(COM)$(CPU)inc.zip
    unzip -o -d $(INCCOM) $(OS)$(COM)$(CPU)inc.zip && $(TOUCH)	$@

.ENDIF

$(BIN)$/mozruntime%zip : $(OS)$(COM)$(CPU)runtime.zip
    $(COPY) $(OS)$(COM)$(CPU)runtime.zip $(BIN)$/mozruntime.zip

# add alternative rules for universal binary moz-zips
.IF "$(GUIBASE)" == "aqua"
$(MISC)$/unpacked_%_lib : $(OS)$(COM)UBlib.zip '$(OS)$(COM)UBruntime.zip'
    unzip -o -d $(LB) $(OS)$(COM)UBlib.zip && unzip -o -d $(LB) $(OS)$(COM)UBruntime.zip && $(TOUCH) $@
    chmod -R 775 $(LB)
  
$(MISC)$/unpacked_%_inc : $(OS)$(COM)UBinc.zip
    unzip -o -d $(INCCOM) $(OS)$(COM)UBinc.zip && $(TOUCH)	$@
    chmod -R 775 $(INCCOM)

$(BIN)$/mozruntime%zip : $(OS)$(COM)UBruntime.zip
    $(COPY) $(OS)$(COM)UBruntime.zip $(BIN)$/mozruntime.zip

.ENDIF # "$(GUIBASE)"=="aqua"

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


# On MacOS no include dirs nspr and nss exists in the prebuilt zip, no need to remove them 
.IF "$(OS)" != "MACOSX" 
NSS_INCLUDE_LIST= nspr nss
.ENDIF

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


#On Linux/Unix sqlite is delivered to $(SOLARLIBDIR)/sqlite/libsqlite3.so
#See readme.txt  in module nss
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
    sqlite/sqlite3 \
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
    $(foreach,lib,$(BIN_RUNTIMELIST) zip -d $(BIN)$/mozruntime.zip $(DLLPRE)$(lib:f)$(DLLPOST) &&) \
    echo >& $(NULLDEV)
.IF "$(GUI)"=="WNT"
    +$(foreach,lib,$(NSS_MODULE_RUNTIME_LIST) zip -g -j $(BIN)$/mozruntime.zip $(SOLARBINDIR)$/$(DLLPRE)$(lib:f)$(DLLPOST) &&) \
    echo >& $(NULLDEV)
.ELSE
    +$(foreach,lib,$(NSS_MODULE_RUNTIME_LIST) zip -g -j $(BIN)$/mozruntime.zip $(SOLARLIBDIR)$/$(lib:d)$(DLLPRE)$(lib:f)$(DLLPOST) &&) \
    echo >& $(NULLDEV)
.ENDIF
    $(TOUCH) $@     

.ENDIF   # unpack mozab zips

