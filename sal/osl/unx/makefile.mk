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



PRJ=..$/..

PRJNAME=sal
.IF "$(WORK_STAMP)"=="MIX364"
TARGET=cppsal
.ELSE
TARGET=cpposl
.ENDIF
USE_LDUMP2=TRUE

PROJECTPCH4DLL=TRUE
PROJECTPCH=cont_pch
PROJECTPCHSOURCE=cont_pch

TARGETTYPE=CUI


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Files --------------------------------------------------------

SLOFILES= \
            $(SLO)$/conditn.obj  \
            $(SLO)$/diagnose.obj \
            $(SLO)$/semaphor.obj \
            $(SLO)$/socket.obj   \
            $(SLO)$/interlck.obj \
            $(SLO)$/mutex.obj    \
            $(SLO)$/nlsupport.obj \
            $(SLO)$/thread.obj   \
            $(SLO)$/module.obj   \
            $(SLO)$/process.obj  \
            $(SLO)$/security.obj \
            $(SLO)$/profile.obj  \
            $(SLO)$/time.obj     \
            $(SLO)$/signal.obj   \
            $(SLO)$/pipe.obj   	 \
            $(SLO)$/system.obj	 \
            $(SLO)$/util.obj	 \
            $(SLO)$/tempfile.obj\
            $(SLO)$/file.obj     \
            $(SLO)$/file_misc.obj\
            $(SLO)$/file_url.obj\
            $(SLO)$/file_error_transl.obj\
            $(SLO)$/file_path_helper.obj\
            $(SLO)$/file_stat.obj \
            $(SLO)$/file_volume.obj \
            $(SLO)$/uunxapi.obj\
            $(SLO)$/process_impl.obj\
            $(SLO)$/salinit.obj


#.IF "$(UPDATER)"=="YES"
OBJFILES=   $(OBJ)$/conditn.obj  \
            $(OBJ)$/diagnose.obj \
            $(OBJ)$/semaphor.obj \
            $(OBJ)$/socket.obj   \
            $(OBJ)$/interlck.obj \
            $(OBJ)$/mutex.obj    \
            $(OBJ)$/nlsupport.obj \
            $(OBJ)$/thread.obj   \
            $(OBJ)$/module.obj   \
            $(OBJ)$/process.obj  \
            $(OBJ)$/security.obj \
            $(OBJ)$/profile.obj  \
            $(OBJ)$/time.obj     \
            $(OBJ)$/signal.obj   \
            $(OBJ)$/pipe.obj   	 \
            $(OBJ)$/system.obj	 \
            $(OBJ)$/util.obj	 \
            $(OBJ)$/tempfile.obj\
            $(OBJ)$/file.obj     \
            $(OBJ)$/file_misc.obj\
            $(OBJ)$/file_url.obj\
            $(OBJ)$/file_error_transl.obj\
            $(OBJ)$/file_path_helper.obj\
            $(OBJ)$/file_stat.obj \
            $(OBJ)$/file_volume.obj \
            $(OBJ)$/uunxapi.obj\
            $(OBJ)$/process_impl.obj\
            $(OBJ)$/salinit.obj
            
#.ENDIF

.IF "$(OS)"=="MACOSX"
SLOFILES += $(SLO)$/osxlocale.obj
.ENDIF

.IF "$(OS)"=="SOLARIS" || "$(OS)"=="FREEBSD" || "$(OS)"=="NETBSD" || "$(OS)$(CPU)"=="LINUXS" || "$(OS)"=="MACOSX"
SLOFILES += $(SLO)$/backtrace.obj
OBJFILES += $(OBJ)$/backtrace.obj
.ENDIF

# --- Targets ------------------------------------------------------

.IF "$(COM)"=="C50"
APP1STDLIBS+=-lC
.ENDIF

.IF "$(OS)" == "LINUX"
.IF "$(PAM)" == "NO"
CFLAGS+=-DNOPAM
.IF "$(NEW_SHADOW_API)" == "YES"
CFLAGS+=-DNEW_SHADOW_API
.ENDIF
.ENDIF
.IF "$(PAM_LINK)" == "YES"
CFLAGS+=-DPAM_LINK
.ENDIF
.IF "$(CRYPT_LINK)" == "YES"
CFLAGS+=-DCRYPT_LINK
.ENDIF
.ENDIF

.IF "$(ENABLE_CRASHDUMP)" != "" || "$(PRODUCT)" == ""
CFLAGS+=-DSAL_ENABLE_CRASH_REPORT
.ENDIF

.INCLUDE :  target.mk

.IF "$(OS)$(CPU)"=="SOLARISU" || "$(OS)$(CPU)"=="SOLARISS" || "$(OS)$(CPU)"=="NETBSDS" || "$(OS)$(CPU)"=="LINUXS"

$(SLO)$/interlck.obj: $(SLO)$/interlck.o
     touch $(SLO)$/interlck.obj

$(OBJ)$/interlck.obj: $(OBJ)$/interlck.o
     touch $(OBJ)$/interlck.obj

$(SLO)$/interlck.o: $(MISC)$/interlck_sparc.s
    $(ASM) $(AFLAGS) -o $@ $<

$(OBJ)$/interlck.o: $(MISC)$/interlck_sparc.s
    $(ASM) $(AFLAGS) -o $@ $<

$(MISC)$/interlck_sparc.s: asm/interlck_sparc.s
    tr -d "\015" < $< > $@

.ENDIF

.IF "$(OS)$(CPU)"=="SOLARISI"

$(SLO)$/interlck.obj: $(SLO)$/interlck.o
    touch $(SLO)$/interlck.obj

$(OBJ)$/interlck.obj: $(OBJ)$/interlck.o
    touch $(OBJ)$/interlck.obj

$(SLO)$/interlck.o: $(MISC)$/interlck_x86.s
    $(ASM) $(AFLAGS) -o $@ $<

$(OBJ)$/interlck.o: $(MISC)$/interlck_x86.s
    $(ASM) $(AFLAGS) -o $@ $<

$(MISC)$/interlck_x86.s: asm/interlck_x86.s
    tr -d "\015" < $< > $@

.ENDIF
