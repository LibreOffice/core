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
MKFILENAME:=SETTINGS.MK

# smaller/greater arithmetic's like ".IF 400<=200" are an OOo extention to
# the initial dmake 4.1PL0 (unfortunately called 4.10) version and are
# tested implicitly by the construction below.
.IF $(MAKEVERSION:s/-cvs//:s/.//)<=410
.ERROR : ; @echo Forced error: dmake version 4.11 or newer is needed!
force_dmake_to_error
.ENDIF

.INCLUDE .IGNORE : ooo_vendor.mk

# --- common tool makros --------------------------------------

.INCLUDE : unitools.mk

.INCLUDE : minor.mk

PLATFORMID = $(RTL_OS:l)_$(RTL_ARCH:l)
EXTNAME*=$(EXTENSIONNAME)_in

.IF "$(SOURCEVERSION)"!="$(WORK_STAMP)"
.ERROR : ; @echo Forced error: minor.mk in solenv/inc does not match your version!
WRONG_SOURCEVERSION
.ENDIF

# Force creation of $(SOLARVERSION)/$(INPATH)/inc/
# $(UPD)minor.mk could be empty as it's contents were already included from minor.mk
.INCLUDE : $(SOLARVERSION)/$(INPATH)/inc/$(UPD)minor.mk

.INCLUDE : udkversion.mk

# --- reset defined Environments --------------------
ASM=
AFLAGS=


#START JAVA

#interpreter location
.IF "$(JAVAINTERPRETER)" == ""
.IF "$(JDK)" == "J++"
JAVAINTERPRETER=jview.exe
.ELSE
.IF "$(JDK)" == "SCAFE"
.IF "$(JDK_VERSION)" != ""
JAVAINTERPRETER=$(DEVROOT)/vcafe11/java/bin/java.exe
.ELSE
JAVAINTERPRETER=$(DEVROOT)/cafepro/java/bin/java.exe
.ENDIF
.ELSE
.IF "$(JAVACISGCJ)" == "yes"
JAVAINTERPRETER=gij
.ELSE
JAVAINTERPRETER=java
.ENDIF
.ENDIF
.ENDIF
.ENDIF

#compiler location
.IF "$(JAVACOMPILER)" == ""
.IF "$(JDK)" == "J++"
JAVACOMPILER=jvc.exe
.ELSE
.IF "$(JDK)" == "SCAFE"
.IF "$(JDK_VERSION)" != ""
JAVACOMPILER=$(DEVROOT)/vcafe11/bin/sj.exe
.ELSE
JAVACOMPILER=$(DEVROOT)/cafepro/bin/sj.exe
.ENDIF
.ELSE
.IF "$(JAVACISGCJ)" == "yes"
JAVACOMPILER=gcj
.ELSE
JAVACOMPILER=javac
.ENDIF
.ENDIF
.ENDIF
.ENDIF

#if javadoc is not already set
.IF "$(JAVADOC)"==""
JAVADOC=javadoc -J-Xmx120m
.ENDIF

#required arguments
JAVAC=$(JAVACOMPILER)
JAVAI:=$(JAVAINTERPRETER)
.IF "$(JAVACISGCJ)" == "yes"
JAVAC+=--encoding=UTF-8 -O2 -fno-assert -Wno-deprecated -C
.ENDIF
.IF $(JAVACISKAFFE) != "yes"
JAVAC+=-source $(JAVA_SOURCE_VER) -target $(JAVA_TARGET_VER)
.ENDIF

#classpath and response
.IF "$(JDK)" == "J++"
JAVACPS=/cp
.ELSE
.IF "$(JDK)" == "SCAFE"
JAVACPS=-classpath
JAVARESPONSE=TRUE
.ELSE
JAVACPS=-classpath
JAVARESPONSE=
.ENDIF
.ENDIF

#END JAVA

CDEFS=
CXXDEFS=
CDEFSCXX=
CDEFSOBJ=
CDEFSSLO=
CDEFSMT=
CDEFSPROF=
CDEFSDEBUG=
CDEFSDBGUTIL=
CDEFSOPT=

CFLAGS=
CFLAGSCALL=
CFLAGSCXX=
CFLAGSCC=
CFLAGSOBJ=
CFLAGSSLO=
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=
CFLAGSSLOCUIMT=
CFLAGSPROF=
CFLAGSDEBUG=
CFLAGSDBGUTIL=
CFLAGSOPT=
CFLAGSNOOPT=
CFLAGSOUTOBJ=

CFLAGSINCXX=

LINKFLAGS=
LINKFLAGSAPPGUI=
LINKFLAGSSHLGUI=
LINKFLAGSAPPCUI=
LINKFLAGSSHLCUI=
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=
LINKFLAGSOPT=
LINKFLAGSAPP=
LINKFLAGSSHL=
LINKEXTENDLINE=

STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
STDLIBGUIMT=
STDLIBCUIMT=
STDSHLGUIMT=
STDSHLCUIMT=
STDOBJ=
STDSLO=
STDLIB=
STDSHL=

LIBFLAGS=

IMPLIBMGR=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RSC=
RSCFLAGS=
RSCDEFS=
RSCLANG=

RC=
RCFLAGS=
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=

HC=
HCFLAGS=

MKDEP=

AS=
BC=
COBOL=
CPP=
FOR=
PASCAL=

BFLAGS=
COBFLAGS=
CPPFLAGS=
CXXFLAGS=
FFLAGS=
PFLAGS=
RFLAGS=

LNT=
LNTFLAGS=
LNTFLAGSGUI=
LNTFLAGSCOM=

# doesn't make sense
#IDLSTATICOBJS=

E2P=
CAP=
cap=

# not to reset because possibly taken from environment
#CC=
#CXX=
#LINK=
#LIBMGR=


# avoid confusion with CUE PROFILE variable...
PROFILE=

# can be set on the command line. we shouldn't delete them!
#profile=

# reset as setting those manually is no lonjger supported
DBGUTIL=
dbgutil=

ZIPDEP:="zipdep.pl"


# ===========================================================================
# unter NT werden Variablennamen an untergeordnete makefiles UPPERCASE
# weitergereicht, aber case significant ausgewertet!
# ---------------------------------------------------------------------------

DMAKE_WORK_DIR*:=$(subst,/,/ $(PWD))
.IF "$(GUI_FOR_BUILD)"=="WNT"
posix_PWD:=/cygdrive/$(PWD:s/://)
.ELSE
posix_PWD:=$(PWD)
.ENDIF


.IF "$(TMP)"!=""
tmp*=$(TMP)
.ENDIF

.IF "$(tmp)"!=""
TMP*=$(tmp)
.ENDIF

.IF "$(TEMP)"!=""
temp*=$(TEMP)
.ENDIF

.IF "$(temp)"!=""
TEMP*=$(temp)
.ENDIF

.IF "$(DEPEND)"!=""
depend=$(DEPEND)
.ENDIF

.IF "$(OPTIMIZE)"!=""
optimize=$(OPTIMIZE)
.ENDIF

.IF "$(NOPT)"!=""
nopt*=$(NOPT)
.ENDIF

.IF "$(GROUP)"!=""
group*=$(GROUP)
.ENDIF

.IF "$(group)"!=""
GROUP*=$(group)
.ENDIF

# override dbglevel if set manually
.IF "$(dbglevel)"!=""
DBG_LEVEL=$(dbglevel)
.ENDIF

.IF "$(NODEBUG)"!=""
nodebug=$(NODEBUG)
.ENDIF

.IF "$(PRODUCT)"!=""
product*=$(PRODUCT)
.ENDIF

.IF "$(product)"!=""
PRODUCT*=$(product)
.ENDIF

.IF "$(VERBOSE)"!=""
verbose*=$(VERBOSE)
.ENDIF

.IF "$(verbose)"!=""
# normalize: "false" (any upper/lower case) and "0" mean FALSE, everything else means TRUE
VERBOSE_IS_FALSE:=$(or $(eq,"$(verbose:u)","FALSE" "false"  ) $(eq,"$(verbose)","0" "false"  ))
VERBOSE!:=$(eq,"$(VERBOSE_IS_FALSE)","t" FALSE TRUE)
.ENDIF

.IF "$(SOLARLANG)" != ""
solarlang*=$(SOLARLANG)
.ENDIF

.IF "$(solarlang)" != ""
SOLARLANG:=$(solarlang)
.ENDIF

.IF "$(wall)"!=""
COMPILER_WARN_ALL=TRUE
.ENDIF          # "$(wall)"!=""

.IF "$(werror)"!=""
COMPILER_WARN_ERRORS=TRUE
.ENDIF

.IF "$(USE_SHL_VERSIONS)"!=""
use_shl_versions*=$(USE_SHL_VERSIONS)
.ENDIF

.IF "$(noremove)"!=""
NOREMOVE*=$(noremove)
.ENDIF

# --- env flags not case sensitive --------------------------------

#.IF "$(envcflags)"!=""
ENVCFLAGS*=$(envcflags)
#.ENDIF

#.IF "$(envcflagscxx)"!=""
ENVCFLAGSCXX*=$(envcflagscxx)
#.ENDIF


.IF "$(envcdefs)"!=""
ENVCDEFS*=$(envcdefs)
.ENDIF

.IF "$(envlibflags)"!=""
ENVLIBFLAGS*=$(envlibflags)
.ENDIF

.IF "$(envcflagsincxx)"!=""
ENVCFLAGSINCXX*=$(envcflagsincxx)
.ENDIF

.IF "$(envlinkflags)"!=""
ENVLINKFLAGS*=$(envlinkflags)
.ENDIF

.IF "$(envrscflags)"!=""
ENVRSCFLAGS*=$(envrscflags)
.ENDIF

.IF "$(envrscdefs)"!=""
ENVRSCDEFS*=$(envrscdefs)
.ENDIF

.IF "$(envrsclinkflags)"!=""
# ENVRSCLINKFLAGS*=$(envrsclinkflags)
.ENDIF

.IF "$(envrcflags)"!=""
ENVRCFLAGS*=$(envrcflags)
.ENDIF

.IF "$(envrclinkflags)"!=""
ENVRCLINKFLAGS*=$(envrclinkflags)
.ENDIF

# --- set both L10N variables to keep things working ---------------

L10N-framework=$(L10N_framework)

# --- Parameter Einstellungen ueberpruefen und umsetzen ------------

# profile immer mit product
.IF "$(profile)"!=""
.IF "$(product)"==""
product=full
.ENDIF
.ENDIF

.IF "$(debug)"!=""
DBG_LEVEL*=1
.ENDIF

# Produkt auf einen Wert setzen (alles klein)
.IF "$(product)" != ""
optimize=true
.IF "$(product)" == "full" || "$(product)" == "Full" || "$(product)" == "FULL"
product!=full
.ENDIF
.IF "$(product)" == "demo" || "$(product)" == "Demo" || "$(product)" == "DEMO"
product!=demo
.ENDIF
.IF "$(product)" == "compact" || "$(product)" == "Compact" || "$(product)" == "COMPACT"
product!=compact
.ENDIF
DBG_LEVEL*=0
.ELSE
optimize!=true
dbgutil!=true
DBG_LEVEL*=1
.ENDIF

.IF "$(debug)"!=""
.IF "$(debug)"!="D_FORCE_OPT"
optimize=
OPTIMIZE=
.ENDIF
.ENDIF

.IF "$(nopt)"!=""
optimize=
OPTIMIZE=
.ENDIF

# Optimierung bei FinalCheck funktioniert nicht!
.IF "$(bndchk)" != ""
optimize=
OPTIMIZE=
.ENDIF

######################################################
#
# sprachabh. einstellungen
#
######################################################

.INCLUDE : lang.mk

.INCLUDE : postset.mk

RSC_LANG_ISO+:=$(completelangiso)
.EXPORT : RSC_LANG_ISO

######################################################

.IF "$(TARGETTYPE)"==""
TARGETTYPE=CUI
.ENDIF

# --- Pfade setzen -------------------------------------------------

# Output-Pfad
# profile has to be first for not getting the .pro extension
.IF "$(profile)"!=""
OUT=$(PRJ)/$(OUTPATH).cap
OUT_FOR_BUILD=$(PRJ)/$(OUTPATH_FOR_BUILD).cap
ROUT=$(OUTPATH).cap
.ELSE

.IF "$(product)"!=""
OUT=$(PRJ)/$(OUTPATH).pro
OUT_FOR_BUILD=$(PRJ)/$(OUTPATH_FOR_BUILD).pro
ROUT=$(OUTPATH).pro

.ELSE
.IF "$(profile)"!=""
OUT=$(PRJ)/$(OUTPATH).cap
OUT_FOR_BUILD=$(PRJ)/$(OUTPATH_FOR_BUILD).cap
ROUT=$(OUTPATH).cap
.ENDIF
.IF "$(dbcs)"!=""
OUT=$(PRJ)/$(OUTPATH).w
OUT_FOR_BUILD=$(PRJ)/$(OUTPATH_FOR_BUILD).w
ROUT=$(OUTPATH).w
.ENDIF
# could already be set by makefile.mk
.IF "$(OUT)" == ""
OUT*=$(PRJ)/$(OUTPATH)
OUT_FOR_BUILD*=$(PRJ)/$(OUTPATH_FOR_BUILD)
ROUT*=$(OUTPATH)
.ENDIF
.ENDIF
.ENDIF

.IF "$(bndchk)" != ""
OUT:=$(PRJ)/$(OUTPATH).bnd
OUT_FOR_BUILD=$(PRJ)/$(OUTPATH_FOR_BUILD).bnd
ROUT=$(OUTPATH).bnd
.ENDIF
.IF "$(truetime)" != ""
OUT=$(PRJ)/$(OUTPATH).tt
OUT_FOR_BUILD=$(PRJ)/$(OUTPATH_FOR_BUILD).tt
ROUT=$(OUTPATH).tt
.ENDIF

.IF "$(PRJ)"!="."
.IF "$(GUI)"=="WNT"
PATH_IN_MODULE:=\
    $(subst,$(normpath $(shell @+cd $(PRJ) && pwd $(PWDFLAGS)))/, $(PWD))
.ELSE			# "$(GUI)"=="WNT"
PATH_IN_MODULE:=$(subst,$(shell @+cd $(PRJ) && pwd $(PWDFLAGS))/, $(PWD))
.ENDIF			# "$(GUI)"=="WNT"
.ELSE			# "$(PRJ)"!="."
PATH_IN_MODULE:=
.ENDIF			# "$(PRJ)"!="."

COMMON_OUTDIR:=$(OUTPATH)

LOCAL_OUT:=$(OUT)
LOCAL_OUT_FOR_BUILD:=$(OUT_FOR_BUILD)
LOCAL_COMMON_OUT:=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(OUT))
.EXPORT : LOCAL_OUT LOCAL_COMMON_OUT

# --- generate output tree -----------------------------------------

# disable for makefiles wrapping a gnumake module
.IF "$(TARGET)"!="prj"
# As this is not part of the initial startup makefile we define an infered
# target instead of using $(OUT)/inc/myworld.mk as target name.
# (See iz62795)
$(posix_PWD)/$(OUT)/inc/%world.mk :
    @mkout.pl $(ROUT)
    @echo $(EMQ)# > $@

.INCLUDE :  $(posix_PWD)/$(OUT)/inc/myworld.mk

.ENDIF          # "$(TARGET)"!="prj"

.INCLUDE .IGNORE : office.mk

# Misc-Pfad
.IF "$(UNR)"!=""
MISCX=$(OUT)/umisc
MISC=$(OUT)/umisc
.ELSE
MISC=$(OUT)/misc
MISC_FOR_BUILD=$(OUT_FOR_BUILD)/misc
# pointing to misc in common output tree if exists
COMMONMISC={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))}
.ENDIF

OUTCXX=$(OUT)/cxx

.IF "$(PACKAGE)"!=""
IDLPACKAGE:=$(PACKAGE)
IDLPACKAGENAME:=$(IDLPACKAGE:sX\X/X)
.ELSE
IDLPACKAGE=$(PRJNAME)
IDLPACKAGENAME=$(PRJNAME)
.ENDIF

# Objekt-Pfad
OBJ=$(OUT)/obj
SLO=$(OUT)/slo
ROBJ=$(ROUT)/obj
RSLO=$(ROUT)/slo

# Particle Path
PAR=$(OUT)/par

# Library-Pfad
LB=$(OUT)/lib
SLB=$(OUT)/slb

# wir haben ein ucr verzeichnis
UCR=$(OUT)/ucr

# $(L) nur noch pfad ins solver\upd\...\lib
#L:={$(LB);$(SLB);$(ILIB)}
L=$(SOLARLIBDIR)

#.EXPORT : SLB LB L LIB
ENVPRJ:=$(PRJ)
#todo: check PRJ is still expected to exist in environment
.EXPORT : PRJ ENVPRJ

# Class-Path for java-classes
# obwohl der generierte Bytecode plattformunabhg. sein sollte
# generieren wir ihn ersteinmal ins abhaengige Verzeichnis
CLASSDIR=$(OUT)/class
CLASSPATH!:=.$(PATH_SEPERATOR)$(CLASSDIR)$(PATH_SEPERATOR)$(CLASSPATH)
STARJAR=java -classpath $(CLASSPATH)$(PATH_SEPERATOR)$(SOLARENV)/bin/jtools.jar com.sun.star.tool.starjar.StarJar

STARDEP=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/javadep
.IF "$(PRJNAME)"=="sj2"
CLASSPATH!:=$(CLASSPATH:s/sj2/no/)
.ENDIF
.IF "$(PRJNAME)"=="uno"
CLASSPATH!:=$(CLASSPATH:s/uno/no/)
.ENDIF
.IF "$(PRJNAME)"=="tkt"
CLASSPATH!:=$(CLASSPATH:s/tkt/no/)
.ENDIF

# default output directory when processing
# configuration files
PROCESSOUT*:=$(MISC)

# Makros fuer die Librarynamen des Solar
.INCLUDE : libs.mk

.IF "$(GUI)"=="WNT"
VERSIONOBJ=$(SLO)/_version.obj
.ENDIF

.IF "$(GUI)"=="UNX"
VERSIONOBJ=$(SLO)/_version.o
.ENDIF

.IF "$(GUI)"=="WNT"
WINVERSIONNAMES=$(UNIXVERSIONNAMES)
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
SHELLLIB=-lgdi32 -lshell32 -ladvapi32 -lcomdlg32
.ELSE
SHELLLIB=gdi32.lib shell32.lib advapi32.lib comdlg32.lib
.ENDIF
.ENDIF

# BIN-Pfad
.IF "$(UNR)"!=""
BIN=$(OUT)/ubin
.ELSE
BIN=$(OUT)/bin
.ENDIF
# pointing to misc in common output tree if exists
COMMONBIN=$(LOCAL_COMMON_OUT)/bin

# Include-Pfad
# still without -I here s.a. target.mk INCLUDE
INC=$(PRJ)/inc
INCLOCAL=../inc
INCGUI=$(PRJ)/$(GUIBASE)/inc
INCCOM=$(OUT)/inc
INCCOMX=$(OUT)/inc
INCUNOIDL=$(INCCOM)/$(PRJNAME)
INCDEPN=. $(INCGUI) $(INCLOCAL) $(INC) $(INC)/$(PRJNAME)
.IF "$(PRJINC)"!=""
INCLOCPRJ=$(foreach,i,$(PRJINC) $i/inc)
INCDEPN+=$(INCLOCPRJ)
.ENDIF
.IF "$(INCPRE)"!=""
INCDEPN+=$(INCPRE)
.ENDIF

# Resource-Pfad fuer .SRS

SRS=$(OUT)/srs
SRSX=$(OUT)/srs

# Resource-Pfad fuer .RC und .RES
RES=$(OUT)/res

# das normale MISC wird nicht an LDMISC angepasst, stattdessen MISCX

.IF "$(make_xl)"!=""
BIN=$(PRJ)/$(OUTPATH).xl/bin
.ENDIF

# damit gezielt Abhaengigkeiten auf s: angegeben werden koennen

SOLARIDLDIR=$(SOLARVERSION)/$(INPATH)/idl

SOLARRESDIR=$(SOLARVERSION)/$(INPATH)/res
SOLARRESXDIR=$(SOLARVERSION)/$(INPATH)/res
SOLARLIBDIR=$(SOLARVERSION)/$(INPATH)/lib
SOLARLIBDIR_FOR_BUILD=$(SOLARVERSION)/$(INPATH_FOR_BUILD)/lib
SOLARJAVADIR=$(SOLARVERSION)/$(INPATH)/java
SOLARINCDIR=$(SOLARVERSION)/$(INPATH)/inc
SOLARINCXDIR=$(SOLARVERSION)/$(INPATH)/inc
.IF "$(SOLARLANG)"!="deut"
.IF "$(SOLARLANG)" != ""
SOLARINCXDIR=$(SOLARVERSION)/$(INPATH)/inc/$(SOLARLANG)
SOLARRESXDIR=$(SOLARVERSION)/$(INPATH)/res/$(SOLARLANG)
.ENDIF
.ENDIF
SOLARBINDIR:=$(SOLARVERSION)/$(INPATH_FOR_BUILD)/bin
SOLARUCRDIR=$(SOLARVERSION)/$(INPATH)/ucr
SOLARPARDIR=$(SOLARVERSION)/$(INPATH)/par
SOLARXMLDIR=$(SOLARVERSION)/$(INPATH)/xml
SOLARDOCDIR=$(SOLARVERSION)/$(INPATH)/doc
SOLARPCKDIR=$(SOLARVERSION)/$(INPATH)/pck
SOLARSDFDIR=$(SOLARVERSION)/$(INPATH)/sdf
SOLARCOMMONBINDIR=$(SOLARVERSION)/common$(PROEXT)/bin
SOLARCOMMONRESDIR=$(SOLARVERSION)/common$(PROEXT)/res
SOLARCOMMONPCKDIR=$(SOLARVERSION)/common$(PROEXT)/pck
SOLARCOMMONSDFDIR=$(SOLARVERSION)/common$(PROEXT)/sdf
SOLARCOMMONBINDIR=$(SOLARBINDIR)
SOLARCOMMONRESDIR=$(SOLARRESDIR)
SOLARCOMMONPCKDIR=$(SOLARPCKDIR)
SOLARCOMMONSDFDIR=$(SOLARSDFDIR)

.EXPORT : SOLARBINDIR

.IF "$(WITH_LANG)"!=""
.INCLUDE .IGNORE: $(WORKDIR)$/CustomTarget$/translations$/localization_present.mk

# if the l10n module exists, use split localize.sdf directly from there
.IF "$(LOCALIZATION_FOUND)"!="" && "$(LOCALIZESDF)"==""
# still check for existence as there may be no localization yet
TRYSDF:=$(WORKDIR)$/CustomTarget$/translations$/translate$/sdf$/$(PRJNAME)$/$(PATH_IN_MODULE)$/localize.sdf
LOCALIZESDF:=$(strip $(shell @+$(IFEXIST) $(TRYSDF) $(THEN) echo $(TRYSDF) $(FI)))
.ENDIF			# "$(LOCALIZATION_FOUND)"!="" && "$(LOCALIZESDF)"==""
# else use localize.sdf from local output tree if localization .zip exists
.IF  "$(LOCALIZESDF)"==""
LOCALSDFFILE:=$(COMMONMISC)$/$(PRJNAME)$/$(PATH_IN_MODULE)$/localize.sdf
LOCALIZESDF:=$(strip $(shell @+$(IFEXIST) $(SOLARCOMMONSDFDIR)$/$(PRJNAME).zip $(THEN) echo $(LOCALSDFFILE) $(FI)))
.IF  "$(LOCALIZESDF)"==""
LOCALSDFFILE!:=
.ENDIF			# "$(LOCALIZESDF)"==""
.ENDIF			# "$(LOCALIZESDF)"==""
# dummy target to keep the build happy.
.IF  "$(LOCALIZESDF)"==""
LOCALIZESDF:=$(COMMONMISC)$/$(PRJNAME)$/dummy$/localize.sdf
.ENDIF			# "$(LOCALIZESDF)"==""
.ENDIF			# "$(WITH_LANG)"!=""

JARDIR=$(OUT)/class

# needs to be expanded!!!

# extend library path for MS Compiler
.IF "$(COM)"=="MSC"
LIB:=$(LB);$(SLB);$(ILIB)
.ENDIF

# extend library path for MinGW Compiler
.IF "$(GUI)$(COM)"=="WNTGCC"
LIB:=$(LB);$(BIN);$(ILIB)
.ENDIF

# --- Environment-Allgemein ----------------------------------------

.IF "$(CPUNAME)" == ""
CPUNAME=CPUNAME_HAS_TO_BE_SET_IN_ENVIRONMENT
.ENDIF

.IF "$(USE_DEBUG_RUNTIME)" != ""
SCPCDEFS+=-D_DEBUG_RUNTIME
.ENDIF

.IF "$(UDK_MAJOR)"!=""
SCPDEFS+=-DUDK_MAJOR=$(UDK_MAJOR)
.ENDIF			# "$(UDK_MAJOR)"!=""

SCPDEFS+=-U$(COMID) -DCOMID=$(COMID) -DCOMNAME=$(COMNAME) -D_$(COMID)
SCPDEFS+=-DCCNUMVER=$(CCNUMVER)
.IF "$(COM)"=="GCC"
SCPDEFS+=-DSHORTSTDCPP3=$(SHORTSTDCPP3) -DSHORTSTDC3=$(SHORTSTDC3)
.ENDIF			# "$(SHORTSTDCPP3)"!=""


UNOIDLDEFS+=-DSUPD=$(UPD) -DUPD=$(UPD)

UNOIDLDEPFLAGS=-Mdepend=$(SOLARVER)

UNOIDLINC+=-I. -I.. -I$(PRJ) -I$(PRJ)/inc -I$(PRJ)/$(INPATH)/idl -I$(OUT)/inc -I$(SOLARIDLDIR) -I$(SOLARINCDIR)

CDEFS=-D$(OS)
.IF "$(GUI)" != "$(OS)"
CDEFS+=-D$(GUI)
.ENDIF
CDEFS+=-D$(COM) -D$(CPUNAME)

.IF "$(USE_DEBUG_RUNTIME)" != ""
CDEFS+=-D_DEBUG_RUNTIME
.ENDIF

.IF "$(CDEFS_PRESET)" != ""
CDEFS+=$(CDEFS_PRESET)
.ENDIF

.IF "$(TIMELOG)" != ""
CDEFS+=-DTIMELOG
.ENDIF

CDEFSCXX=
CDEFSOBJ=
#CDEFSMT=-DMULTITHREAD
CDEFSPROF=-DPROFILE
.IF "$(dbg_build_only)" == ""
CDEFSDEBUG=-DDEBUG
.ENDIF
CDEFSDBGUTIL=-DDBG_UTIL
CDEFSOPT=-DOPTIMIZE

MKDEPFLAGS+=-I$(INCDEPN:s/ / -I/:s/-I-I/-I/)
MKDEPALLINC=$(SOLARINC:s/-I/ -I/)
MKDEPPREINC=-I$(PREPATH)/$(INPATH)/inc
MKDEPSOLENV=-I$(SOLARENV)/inc -I$(SOLARENV)/$(GUI)$(COMEX)/inc
MKDEPSOLVER=-I$(SOLARVERSION)/$(INPATH)/inc
MKDEPLOCAL=-I$(INCCOM)

.IF "$(MKDEPENDALL)"!=""
MKDEPFLAGS+=$(MKDEPALLINC)
.ENDIF

.IF "$(MKDEPENDSOLENV)"!=""
MKDEPFLAGS+=$(MKDEPSOLENV)
.ENDIF

.IF "$(MKDEPENDPREPATH)"!=""
MKDEPFLAGS+=$(MKDEPPREINC)
.ENDIF

.IF "$(MKDEPENDSOLVER)"!=""
MKDEPFLAGS+=$(MKDEPSOLVER)
.ENDIF
# unused - nodep should do the same now
#.IF "$(MKDEPENDLOCAL)"!="NO"
MKDEPFLAGS+=$(MKDEPLOCAL)
#.ENDIF

BISON*=bison
YACCFLAGS*=-d 

SVIDL=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/svidl

LDUMP2*=$(SOLARBINDIR)/ldump4

ZIPUPDATE=-u -j
ZIPFLAGS=$(ZIPUPDATE)
#hack for dynamic language subdirs
LANGDIR=LANGDIR

#scplinker flags
SCPLINKFLAGS=-i $(PAR),$(SOLARPARDIR)
.IF "$(SCPLINKVERSION)"!=""
SCPLINKFLAGS+=-v $(SCPLINKVERSION)
.ENDIF			# "$(SCPLINKVERSION)"!=""

.IF "$(make_srs_deps)"!=""
RSC=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/rscdep
.ELSE # "$(make_srs_deps)"!=""
RSC=$(AUGMENT_LIBRARY_PATH) $(FLIPCMD) $(SOLARBINDIR)/rsc
.ENDIF # "$(make_srs_deps)"!=""

.IF "$(VERBOSE)" == "TRUE"
    VERBOSITY=-verbose
.ELSE
    COMMAND_ECHO=@
    VERBOSITY=-quiet
    ZIP_VERBOSITY=-q
.ENDIF # "$(VERBOSE)" == "TRUE"
COMPILE_ECHO_SWITCH=
COMPILE_ECHO_FILE=$(<:f)

#new
RSCUPDVER=$(RSCREVISION)
RSCUPDVERDEF=-DUPDVER="$(RSCUPDVER)"

RSCFLAGS=-s
RSCDEFS=-D$(GUI) -D$(COM) $(JAVADEF)

RSCDEFIMG*=icon-themes/galaxy

RSCEXTINC=.

.IF "$(DEBUG)" != ""
RSCDEFS+= -DDEBUG
.ENDIF

# settings for mozilla idl compiler
XPIDL=xpidl
XPIDL_LINK=xpt_link

# alle bisher verwendeten Linker benutzen + um LIBs auf der naechsten Zeile
# weiter zu schreiben, wenn es da eine Ausnahme geben sollte, muss
# LINKEXTENDLINE compilerabhaengig definiert werden
LINKEXTENDLINE=+

LNT=$(DEVROOT)/lint/lint
#LNTFLAGS=+v -i$(DEVROOT)/lint\ -mL options.lnt -u
LNTFLAGS=+v -i$(DEVROOT)/lint/ -mL options.lnt -u
LNTFLAGSOUTOBJ=-os

.IF "$(GUI)"=="WNT"
.INCLUDE : wnt.mk
.ENDIF

.IF "$(GUI)" == "UNX"
.INCLUDE : unx.mk
.ENDIF

DLLPOSTFIX=lo

.IF "$(CROSS_COMPILING)" == "YES"
# Assume always cross-compiling from Unix
EXECPOST_FOR_BUILD=
.ELSE
EXECPOST_FOR_BUILD=$(EXECPOST)
.ENDIF

.IF "$(OOO_LIBRARY_PATH_VAR)" != ""
# Add SOLARLIBDIR at the begin of a (potentially previously undefined) library
# path (LD_LIBRARY_PATH, PATH, etc.; prepending avoids fetching libraries from
# an existing office/URE installation; the ": &&" enables this to work at the
# start of a recipe line that is not prefixed by "+" as well as in the middle of
# an existing && chain:
.IF "$(CROSS_COMPILING)"=="YES" && "$(OS)"=="WNT"
# Sigh, special-case cross-compiling to Windows. Here OOO_LIBRARY_PATH_VAR is the correct one
# for the BUILD platform but SOLARSHAREDBIN is the one for Windows, i.e. "foo/bin".
AUGMENT_LIBRARY_PATH = : && \
    $(OOO_LIBRARY_PATH_VAR)=$(normpath, $(SOLARLIBDIR_FOR_BUILD))$${{$(OOO_LIBRARY_PATH_VAR):+:$${{$(OOO_LIBRARY_PATH_VAR)}}}}
AUGMENT_LIBRARY_PATH_LOCAL = : && \
    $(OOO_LIBRARY_PATH_VAR)=$(normpath, $(PWD)/$(DLLDEST)):$(normpath, $(SOLARSHAREDBIN))$${{$(OOO_LIBRARY_PATH_VAR):+:$${{$(OOO_LIBRARY_PATH_VAR)}}}}
.ELSE
AUGMENT_LIBRARY_PATH = : && \
    $(OOO_LIBRARY_PATH_VAR)=$(normpath, $(SOLARSHAREDBIN))$${{$(OOO_LIBRARY_PATH_VAR):+:$${{$(OOO_LIBRARY_PATH_VAR)}}}}
AUGMENT_LIBRARY_PATH_LOCAL = : && \
    $(OOO_LIBRARY_PATH_VAR)=$(normpath, $(PWD)/$(DLLDEST)):$(normpath, $(SOLARSHAREDBIN))$${{$(OOO_LIBRARY_PATH_VAR):+:$${{$(OOO_LIBRARY_PATH_VAR)}}}}
.ENDIF
.END

# for multiprocess building in external modules
# allow seperate handling
EXTMAXPROCESS*=$(MAXPROCESS)

GDBTRACE=gdb -nx --command=$(SOLARENV)/bin/gdbtrycatchtrace --args

#use with export VALGRIND=memcheck, that method of invocation is used because
#hunspell will also run its own unit tests under valgrind when this variable is
#set.
.IF "$(VALGRIND)" != ""
VALGRINDTOOL=valgrind --tool=$(VALGRIND) --num-callers=50
.IF "$(VALGRIND)" == "memcheck"
G_SLICE*:=always-malloc
.EXPORT : G_SLICE
GLIBCXX_FORCE_NEW*:=1
.EXPORT : GLIBCXX_FORCE_NEW
.ENDIF
.ENDIF

IDLC*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/idlc
REGMERGE*=$(AUGMENT_LIBRARY_PATH) $(VALGRINDTOOL) $(SOLARBINDIR)/regmerge
REGCOMPARE*=$(AUGMENT_LIBRARY_PATH) $(VALGRINDTOOL) $(SOLARBINDIR)/regcompare

.IF "$(DEBUGREGCOMP)" != "" || "$(debugregcomp)" != ""
GDBREGCOMPTRACE=$(GDBTRACE)
.ENDIF

REGCOMP*=$(AUGMENT_LIBRARY_PATH_LOCAL) $(GDBREGCOMPTRACE) $(SOLARBINDIR)/regcomp
CPPUMAKER*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/cppumaker
JAVAMAKER*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/javamaker
RDBMAKER*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/rdbmaker
CLIMAKER*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/climaker

.IF "$(DEBUGCPPUNIT)" != "" || "$(debugcppunit)" != ""
GDBCPPUNITTRACE=$(GDBTRACE)
.ENDIF

.IF "$(CROSS_COMPILING)" == "YES"
CPPUNITTESTER=\#
.ELSE
CPPUNITTESTER=$(AUGMENT_LIBRARY_PATH_LOCAL) $(GDBCPPUNITTRACE) $(VALGRINDTOOL) $(SOLARBINDIR)/cppunit/cppunittester
.ENDIF
HELPEX=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/helpex
LNGCONVEX=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/lngconvex
HELPLINKER=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/HelpLinker
HELPINDEXER=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/HelpIndexer

.IF "$(JAVAINTERPRETER)" == ""
JAVA*:=java
.ELSE
JAVA*:=$(JAVAINTERPRETER)
.ENDIF
.EXPORT : JAVA JAVAI

# Define SCPCOMP without wrapper because pre2par.pl chokes on DOS style
# paths. (See iz57443)
SCPCOMP=$(PERL) $(SOLARENV)/bin/pre2par.pl
SCPLINK=$(PERL) $(SOLARENV)/bin/par2script.pl
LZIP*=lzip

# Note that $(CPPLCC) is oddly enough only used when $(OS)==WNT, see pstrules.mk.
# For the BUILD platform when cross-compiling (always Unix), it is called cpp.lcc
.IF "$(CROSS_COMPILING)" == "YES"
CPPLCC*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/cpp.lcc
.ELSE
CPPLCC*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/cpplcc
.ENDIF

.IF "$(DISABLE_ENHANCED_COMID)"==""
.INCLUDE : tg_compv.mk
.ELSE          # "$(DISABLE_ENHANCED_COMID)"==""
COMID=$(COM)
.ENDIF          # "$(DISABLE_ENHANCED_COMID)"==""
.IF "$(SOLAR_JAVA)"=="TRUE"
.IF "$(USE_JAVAVER)"!=""
.INCLUDE : tg_javav.mk
.ENDIF "$(USE_JAVAVER)"!=""
.ENDIF			# "$(SOLAR_JAVA)"=="TRUE"

.IF "$(COM)"=="GCC"
GXX_INCLUDE_PATH*:=$(COMPATH)/include/c++/$(CCVER)
.EXPORT : GXX_INCLUDE_PATH
CDEFS+= -DGXX_INCLUDE_PATH=$(GXX_INCLUDE_PATH)
.ENDIF

# --- extend new environment ----------------------------------
CDEFS+= -DSUPD=$(UPD)

# flags to enable build with symbols; required for crashdump feature
.IF ("$(ENABLE_CRASHDUMP)"!="" && "$(ENABLE_CRASHDUMP)"!="DUMMY") || ("$(ENABLE_SYMBOLS)"!="" && "$(ENABLE_SYMBOLS)"!="FALSE")
# if debug is enabled, this may enable less debug info than debug, so rely just on debug
.IF "$(debug)" == ""
CFLAGSENABLESYMBOLS_CC_ONLY*=$(CFLAGSENABLESYMBOLS)
CFLAGSCXX+=$(CFLAGSENABLESYMBOLS)
CFLAGSCC+=$(CFLAGSENABLESYMBOLS_CC_ONLY)
.ENDIF          # "$(DEBUG)" == ""
.ENDIF          # ("$(ENABLE_CRASHDUMP)"!="" && "$(ENABLE_CRASHDUMP)"!="DUMMY") || "$(ENABLE_SYMBOLS)"!=""

.IF "$(profile)"!=""
CDEFS+= $(CDEFSPROF)
CFLAGS+= $(CFLAGSPROF)
.IF "$(WST)"!=""
LINKFLAGS=$(LINKFLAGSWST)
LINKCFLAGS=$(LINKFLAGSWST)
.ELSE
LINKFLAGSADD+=$(LINKFLAGSPROF)
.ENDIF
.ENDIF

.IF "$(debug)"!=""
JAVAFLAGS+=$(JAVAFLAGSDEBUG)
CDEFS+= $(CDEFSDEBUG)
CFLAGS+= $(CFLAGSDEBUG)
LINKFLAGSADD+= $(LINKFLAGSDEBUG)
.ENDIF

.IF "$(dbgutil)"!=""
CDEFS+=$(CDEFSDBGUTIL)
CFLAGS+=$(CFLAGSDBGUTIL)
RSCDEFS+=-DDBG_UTIL
.ENDIF

.IF "$(product)"!=""
CDEFS+= -DPRODUCT
RSCDEFS+= -DPRODUCT
.IF "$(ASSERT_ALWAYS_ABORT)"=="FALSE"
CDEFS+=-DNDEBUG
RSCDEFS+= -DNDEBUG
.ENDIF
.ENDIF

.IF "$(DBG_LEVEL)"!=""
CDEFS+=-DOSL_DEBUG_LEVEL=$(DBG_LEVEL)
RSCDEFS+=-DOSL_DEBUG_LEVEL=$(DBG_LEVEL)
.IF "$(DBG_LEVEL)"!="0"
CDEFS+=-DSAL_LOG_INFO -DSAL_LOG_WARN
.ENDIF
.ENDIF

.IF "$(optimize)"!=""
CDEFS+=$(CDEFSOPT)
CFLAGS+=$(CFLAGSOPT)
LINKFLAGSADD+=$(LINKFLAGSOPT)
.ELSE
CFLAGS+=$(CFLAGSNOOPT)
.ENDIF

.IF "$(ENABLE_EXCEPTIONS)"!=""
GLOBAL_EXCEPTIONS_FLAGS+=$(CFLAGSEXCEPTIONS)
GLOBAL_EXCEPTIONS_FLAGS+=-DEXCEPTIONS_ON
.ELSE
GLOBAL_EXCEPTIONS_FLAGS+=$(CFLAGS_NO_EXCEPTIONS)
GLOBAL_EXCEPTIONS_FLAGS+=-DEXCEPTIONS_OFF
.ENDIF
# used if an object appears in the EXCEPTIONSFILES list
# see also rules.mk
LOCAL_EXCEPTIONS_FLAGS+=$(CFLAGSEXCEPTIONS)
LOCAL_EXCEPTIONS_FLAGS+=-DEXCEPTIONS_ON

.IF "$(ENABLE_GSTREAMER)" == "TRUE"
CDEFS+=-DGSTREAMER
.ENDIF

.IF "$(HAVE_THREADSAFE_STATICS)" == "TRUE"
CDEFS += -DHAVE_THREADSAFE_STATICS
.END

.IF "$(DISABLE_DYNLOADING)" == "TRUE"
CDEFS += -DDISABLE_DYNLOADING
.ENDIF

CDEFS += -DRTL_USING

# compose flags and defines for GUI
.IF "$(TARGETTYPE)"=="GUI"
CFLAGSSLO+=$(CFLAGSSLOGUIMT)
CDEFSSLO+=$(CDEFSSLOMT)
LINKFLAGSAPP=$(LINKFLAGSAPPGUI)
LINKFLAGSSHL=$(LINKFLAGSSHLGUI)
STDOBJ=$(STDOBJGUI)
STDSLO=$(STDSLOGUI)

CDEFSOBJ+= $(CDEFSMT) $(CDEFSOBJMT)
CFLAGSOBJ+=$(CFLAGSOBJGUIMT)
STDLIB=$(STDLIBGUIMT)
STDSHL=$(STDSHLGUIMT)

.ENDIF

# compose flags and defines for CUI
.IF "$(TARGETTYPE)" == "CUI" || "$(TARGETTYPE)"=="cui"
CFLAGSSLO+=$(CFLAGSSLOCUIMT)
CDEFSSLO+=$(CDEFSSLOMT)
LINKFLAGSAPP=$(LINKFLAGSAPPCUI)
LINKFLAGSSHL=$(LINKFLAGSSHLCUI)
STDOBJ=$(STDOBJCUI)
STDSLO=$(STDSLOCUI)

CDEFSOBJ+=$(CDEFSMT) $(CDEFSOBJMT)
CFLAGSOBJ+=$(CFLAGSOBJCUIMT)
STDLIB=$(STDLIBCUIMT)
STDSHL=$(STDSHLCUIMT)

.ENDIF

.EXPORT : PICSWITCH

# fill up unresolved symbols not found else where
.IF "$(FILLUPARC)"!=""
STDLIB+=$(FILLUPARC)
STDSHL+=$(FILLUPARC)
.ENDIF			# "$(FILUPARC)"!=""

.IF "$(SOLAR_JAVA)"!=""
CDEFS+=$(JAVADEF)
.ENDIF          # "$(SOLAR_JAVA)"!=""

.INCLUDE .IGNORE: $(UPD)$(LAST_MINOR).mk

# Once all modules on a given platform compile without warnings, the specific
# .mk file for that platform should set COMPILER_WARN_ERRORS=TRUE and no longer
# set MODULES_WITH_WARNINGS, and once no platform sets MODULES_WITH_WARNINGS any
# longer, this code can go away:
.IF "$(MODULES_WITH_WARNINGS)" == ""
COMPILER_WARN_ERRORS=TRUE
.ELSE
MODULES_WITH_WARNINGS_1 := $(foreach,i,$(MODULES_WITH_WARNINGS) .$(i).)
MODULES_WITH_WARNINGS_2 := $(subst,.$(PRJNAME)., $(MODULES_WITH_WARNINGS_1))
.IF "$(MODULES_WITH_WARNINGS_1)" == "$(MODULES_WITH_WARNINGS_2)"
COMPILER_WARN_ERRORS=TRUE
.ENDIF
.ENDIF

CFLAGSWARNCXX *= $(CFLAGSWARNCC)
CFLAGSWALLCXX *= $(CFLAGSWALLCC)
CFLAGSWERRCXX *= $(CFLAGSWERRCC)

.IF "$(ENVWARNFLAGS)"==""
.IF "$(COMPILER_WARN_ALL)"==""
CFLAGSCC+=$(CFLAGSWARNCC)
CFLAGSCXX+=$(CFLAGSWARNCXX)
.ELSE
CFLAGSCC+=$(CFLAGSWALLCC)
CFLAGSCXX+=$(CFLAGSWALLCXX)
.ENDIF
.IF "$(COMPILER_WARN_ERRORS)"!="" && "$(EXTERNAL_WARNINGS_NOT_ERRORS)"=="FALSE"
CFLAGSCC+=$(CFLAGSWERRCC)
CFLAGSCXX+=$(CFLAGSWERRCXX)
.ENDIF
.ELSE			# "$(ENVWARNFLAGS)"==""
CFLAGSAPPEND+=$(ENVWARNFLAGS)
.ENDIF			# "$(ENVWARNFLAGS)"==""

CDEFS+= $(ADDCDEFS)
CDEFS+= $(ENVCDEFS)
CFLAGS+= $(CFLAGSCALL) $(ENVCFLAGS)
CFLAGSCXX+= $(ENVCFLAGSCXX)
# andersrum wg. -TPxxxx
CFLAGSINCXX+:=$(ENVCFLAGSINCXX)
LIBFLAGS+= $(ENVLIBFLAGS)
LINKFLAGSADD+= $(ENVLINKFLAGS)
RSCFLAGS+= $(ENVRSCFLAGS)
RSCDEFS+= $(ENVRSCDEFS)
# RSCLINKFLAGS+= $(ENVRSCLINKFLAGS)
RCFLAGS+= $(ENVRCFLAGS)
RCLINKFLAGS+= $(ENVRCLINKFLAGS)

LINKFLAGSRUNPATH_URELIB*=
LINKFLAGSRUNPATH_UREBIN*=
LINKFLAGSRUNPATH_OOO*=
LINKFLAGSRUNPATH_SDKBIN*=
LINKFLAGSRUNPATH_OXT*=
LINKFLAGSRUNPATH_NONE*=

# make sure both linker variables are set
LINKC*=$(LINK)
LINKCFLAGS*=$(LINKFLAGS)
LINKFLAGS+=$(LINKFLAGSADD)
LINKCFLAGS+=$(LINKFLAGSADD)

#defaults for UCR HEADER
UNOUCRBASE*=UCR
UNOUCROUT*=$(OUT)/inc/$(TARGET)
INCPRE+=-I$(UNOUCROUT)
UNOUCRRDB*=$(SOLARBINDIR)/types.rdb

# location of trustedinfo.manifest for Windows Vista
.IF "$(COM)"=="MSC"
TRUSTED_MANIFEST_LOCATION*=$(SOLARENV)/inc
.ENDIF # "$(COM)"=="MSC"

# --- Compiler -----------------------------------------------------
.INCLUDE : rules.mk

# --- include typelists --------------------------------------------

.IF "$(COMP1TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)/$(COMP1TYPELIST).mk
$(COMP1TYPELIST)_XML2CMPTYPES:=$(shell @$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/xml2cmp -types stdout $(MISC)/$(COMP1TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP2TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)/$(COMP2TYPELIST).mk
$(COMP2TYPELIST)_XML2CMPTYPES:=$(shell @$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/xml2cmp -types stdout $(MISC)/$(COMP2TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP3TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)/$(COMP3TYPELIST).mk
$(COMP3TYPELIST)_XML2CMPTYPES:=$(shell @$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/xml2cmp -types stdout $(MISC)/$(COMP3TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP4TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)/$(COMP4TYPELIST).mk
$(COMP4TYPELIST)_XML2CMPTYPES:=$(shell @$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/xml2cmp -types stdout $(MISC)/$(COMP4TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP5TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)/$(COMP5TYPELIST).mk
$(COMP5TYPELIST)_XML2CMPTYPES:=$(shell @$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/xml2cmp -types stdout $(MISC)/$(COMP5TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP6TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)/$(COMP6TYPELIST).mk
$(COMP6TYPELIST)_XML2CMPTYPES:=$(shell @$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/xml2cmp -types stdout $(MISC)/$(COMP6TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP7TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)/$(COMP7TYPELIST).mk
$(COMP7TYPELIST)_XML2CMPTYPES:=$(shell @$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/xml2cmp -types stdout $(MISC)/$(COMP7TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP8TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)/$(COMP8TYPELIST).mk
$(COMP8TYPELIST)_XML2CMPTYPES:=$(shell @$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/xml2cmp -types stdout $(MISC)/$(COMP8TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP9TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)/$(COMP9TYPELIST).mk
$(COMP9TYPELIST)_XML2CMPTYPES:=$(shell @$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/xml2cmp -types stdout $(MISC)/$(COMP9TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

# some place to define these jars for SO environment
XML_APIS_JAR*=$(SOLARBINDIR)/xml-apis.jar
XERCES_JAR*=$(SOLARBINDIR)/xercesImpl.jar

.IF "$(SYSTEM_CPPUNIT)" != "YES"
CPPUNIT_CFLAGS =
.END

.IF "$(DISABLE_DYNLOADING)" == "TRUE"
COMPONENTPREFIX_URE_NATIVE =
COMPONENTPREFIX_BASIS_NATIVE =
COMPONENTPREFIX_INBUILD_NATIVE =
.ELSE
COMPONENTPREFIX_URE_NATIVE = vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/
COMPONENTPREFIX_BASIS_NATIVE = vnd.sun.star.expand:$$LO_LIB_DIR/
COMPONENTPREFIX_INBUILD_NATIVE = vnd.sun.star.expand:$$LO_LIB_DIR/
.ENDIF
COMPONENTPREFIX_URE_JAVA = vnd.sun.star.expand:$$URE_INTERNAL_JAVA_DIR/
COMPONENTPREFIX_BASIS_JAVA = vnd.sun.star.expand:$$LO_JAVA_DIR/
COMPONENTPREFIX_BASIS_PYTHON = vnd.openoffice.pymodule:
COMPONENTPREFIX_INBUILD_JAVA = vnd.sun.star.expand:$$LO_JAVA_DIR/
COMPONENTPREFIX_EXTENSION = ./

# workaround for strange dmake bug:
# if the previous block was a rule or a target, "\#" isn't recognized
# as an escaped "#". if it was an assignment, escaping works...
some_unique_variable_name:=1
