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

.IF "$(USE_PCH)"!=""
ENABLE_PCH=TRUE
.ENDIF			# "$(USE_PCH)"!=""

.IF "$(ENABLE_PCH)"!="" && "$(BUILD_SPECIAL)"!=""
.IF "$(SOLARSRC)"=="$(SRC_ROOT)"
NETWORK_BUILD:=TRUE
.ENDIF			# "$(SOLARSRC)"=="$(SRC_ROOT)"
.ENDIF			# "$(ENABLE_PCH)"!="" && "$(BUILD_SPECIAL)"!=""

.INCLUDE : unitools.mk

.INCLUDE : minor.mk

.INCLUDE .IGNORE : rtlbootstrap.mk

PLATFORMID = $(RTL_OS:l)_$(RTL_ARCH:l)
EXTNAME*=$(EXTENSIONNAME)_in

.IF "$(UPDATER)"!="" || "$(CWS_WORK_STAMP)"!=""

.IF "$(SOURCEVERSION)"!="$(WORK_STAMP)"
.ERROR : ; @echo Forced error: minor.mk in solenv/inc does not match your version!
WRONG_SOURCEVERSION
.ENDIF

# Create $(SOLARVERSION)/$(INPATH)/inc$(UPDMINOREXT)/$(UPD)minor.mk if needed
%minor.mk : $(SOLARENV)/inc/minor.mk
    @@-$(MKDIRHIER) $(SOLARVERSION)/$(INPATH)/inc$(UPDMINOREXT)
    @@$(COPY) $(SOLARENV)/inc/minor.mk $(SOLARVERSION)/$(INPATH)/inc$(UPDMINOREXT)/$(UPD)minor.mk
    @@$(TOUCH) $(SOLARVERSION)/$(INPATH)/inc$(UPDMINOREXT)/minormkchanged.flg
.ENDIF          # "$(UPDATER)"!="" || "$(CWS_WORK_STAMP)"!=""

# Force creation of $(SOLARVERSION)/$(INPATH)/inc$(UPDMINOREXT)/
# $(UPD)minor.mk could be empty as it's contents were already included from minor.mk
.INCLUDE : $(SOLARVERSION)/$(INPATH)/inc$(UPDMINOREXT)/$(UPD)minor.mk

.IF "$(BSCLIENT)"=="TRUE"
.IF "$(UPDATER)"!="YES"
incorrect_settings:
    @echo "#"
    @echo "#" ERROR: setsolar option -bsclient used but UPDATER=YES not set!
    @echo "#"
    force_dmake_to_error
.ENDIF
.ENDIF

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
.IF "$(JDK)" != "gcj" && $(JAVACISKAFFE) != "yes"
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

JAVAFLAGS+=$(JAVA_TARGET_FLAG)

#END JAVA

CDEFS=
CXXDEFS=
CDEFSCXX=
CDEFSOBJ=
CDEFSSLO=
CDEFSGUI=
CDEFSCUI=
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

# ===========================================================================
# unter NT werden Variablennamen an untergeordnete makefiles UPPERCASE
# weitergereicht, aber case significant ausgewertet!
# ---------------------------------------------------------------------------

DMAKE_WORK_DIR*:=$(subst,/,/ $(PWD))
.IF "$(GUI)"=="WNT"
posix_PWD:=/cygdrive/$(PWD:s/://)
.ELSE			#GUI)"=="WNT"
posix_PWD:=$(PWD)
.ENDIF			#GUI)"=="WNT"


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

.IF "$(hbtoolkit)"!=""
HBTOOLKIT=$(hbtoolkit)
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

#.IF "$(COMMON_BUILD)"!=""
#common_build*=$(COMMON_BUILD)
#.ENDIF
common_build:=
COMMON_BUILD:=

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
USE_DEBUG_RUNTIME*=TRUE
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
ROUT=$(OUTPATH).cap
.ELSE

.IF "$(product)"!=""
OUT=$(PRJ)/$(OUTPATH).pro
ROUT=$(OUTPATH).pro

.ELSE
.IF "$(profile)"!=""
OUT=$(PRJ)/$(OUTPATH).cap
ROUT=$(OUTPATH).cap
.ENDIF
.IF "$(dbcs)"!=""
OUT=$(PRJ)/$(OUTPATH).w
ROUT=$(OUTPATH).w
.ENDIF
# could already be set by makefile.mk
.IF "$(OUT)" == ""
OUT*=$(PRJ)/$(OUTPATH)
ROUT*=$(OUTPATH)
.ENDIF
.ENDIF
.ENDIF

.IF "$(bndchk)" != ""
OUT:=$(PRJ)/$(OUTPATH).bnd
ROUT=$(OUTPATH).bnd
.ENDIF
.IF "$(truetime)" != ""
OUT=$(PRJ)/$(OUTPATH).tt
ROUT=$(OUTPATH).tt
.ENDIF
.IF "$(hbtoolkit)"!=""
OUT=$(PRJ)/$(OUTPATH).tlk
ROUT=$(OUTPATH).tlk
.ENDIF

.IF "$(PRJ)"!="."
.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
PATH_IN_MODULE:=\
    $(subst,$(normpath $(shell @+cd $(PRJ) && pwd $(PWDFLAGS)))/, $(PWD))
.ELSE			# "$(GUI)"=="WNT"
PATH_IN_MODULE:=$(subst,$(shell @+cd $(PRJ) && pwd $(PWDFLAGS))/, $(PWD))
.ENDIF			# "$(GUI)"=="WNT"
.ELSE			# "$(PRJ)"!="."
PATH_IN_MODULE:=
.ENDIF			# "$(PRJ)"!="."

## common output tree
#.IF "$(common_build)"!=""
#COMMON_OUTDIR*=common
#.IF "$(no_common_build_reslib)"==""
#common_build_reslib=true
#.ENDIF			# "$(no_common_build_reslib)"==""
#.IF "$(no_common_build_zip)"==""
#common_build_zip=true
#.ENDIF			# "$(no_common_build_zip)"==""
#.IF "$(no_common_build_sign_jar)"==""
#common_build_sign_jar=true
#.ENDIF			# "$(no_common_build_sign_jar)"==""
#.IF "$(no_common_build_srs)"==""
#common_build_srs=true
#.ENDIF			# "$(no_common_build_srs)"==""
#.ELSE			# "$(common_build)"!=""
COMMON_OUTDIR:=$(OUTPATH)
#.ENDIF			# "$(common_build)"!=""

LOCAL_OUT:=$(OUT)
LOCAL_COMMON_OUT:=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(OUT))
.EXPORT : LOCAL_OUT LOCAL_COMMON_OUT

# --- generate output tree -----------------------------------------

# disable for makefiles wrapping a gnumake module
.IF "$(TARGET)"!="prj"
# As this is not part of the initial startup makefile we define an infered
# target instead of using $(OUT)/inc/myworld.mk as target name.
# (See iz62795)
$(posix_PWD)/$(OUT)/inc/%world.mk :
    @$(MKOUT) $(ROUT)
    @echo $(EMQ)# > $@

.INCLUDE :  $(posix_PWD)/$(OUT)/inc/myworld.mk

.IF "$(common_build)"!=""
$(posix_PWD)/$(LOCAL_COMMON_OUT)/inc/%world.mk :
    @$(MKOUT) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(ROUT))
    @echo $(EMQ)# > $@

.INCLUDE : $(posix_PWD)/$(LOCAL_COMMON_OUT)/inc/myworld.mk
.ENDIF			# "$(common_build)"!=""
.ENDIF          # "$(TARGET)"!="prj"

.INCLUDE .IGNORE : office.mk

# Misc-Pfad
.IF "$(UNR)"!=""
MISCX=$(OUT)/umisc
MISC=$(OUT)/umisc
.ELSE
MISC=$(OUT)/misc
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

.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
VERSIONOBJ=$(SLO)/_version.obj
.ENDIF

.IF "$(GUI)"=="UNX"
VERSIONOBJ=$(SLO)/_version.o
.ENDIF

.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
WINVERSIONNAMES=$(UNIXVERSIONNAMES)
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
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
INCPCH=$(PRJ)/inc/pch
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

.IF "$(common_build_srs)"!=""
SRS=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(OUT))/srs
SRSX=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(OUT))/srs
.ELSE          # "$(common_build_srs)"!=""
SRS=$(OUT)/srs
SRSX=$(OUT)/srs
.ENDIF          # "$(common_build_srs)"!=""

# Resource-Pfad fuer .RC und .RES
RES=$(OUT)/res

# das normale MISC wird nicht an LDMISC angepasst, stattdessen MISCX

.IF "$(make_xl)"!=""
BIN=$(PRJ)/$(OUTPATH).xl/bin
.ENDIF

# damit gezielt Abhaengigkeiten auf s: angegeben werden koennen

.IF "$(common_build)"!=""
SOLARIDLDIR=$(SOLARVERSION)/common$(PROEXT)/idl$(UPDMINOREXT)
.ELSE
SOLARIDLDIR=$(SOLARVERSION)/$(INPATH)/idl$(UPDMINOREXT)
.ENDIF

#.IF "$(UPDMINOR)" != ""
#UPDMINOREXT=.$(UPDMINOR)
#.ELSE
#UPDMINOREXT=
#.ENDIF
SOLARRESDIR=$(SOLARVERSION)/$(INPATH)/res$(UPDMINOREXT)
SOLARRESXDIR=$(SOLARVERSION)/$(INPATH)/res$(UPDMINOREXT)
SOLARLIBDIR=$(SOLARVERSION)/$(INPATH)/lib$(UPDMINOREXT)
SOLARJAVADIR=$(SOLARVERSION)/$(INPATH)/java$(UPDMINOREXT)
SOLARINCDIR=$(SOLARVERSION)/$(INPATH)/inc$(UPDMINOREXT)
SOLARINCXDIR=$(SOLARVERSION)/$(INPATH)/inc$(UPDMINOREXT)
.IF "$(SOLARLANG)"!="deut"
.IF "$(SOLARLANG)" != ""
SOLARINCXDIR=$(SOLARVERSION)/$(INPATH)/inc$(UPDMINOREXT)/$(SOLARLANG)
SOLARRESXDIR=$(SOLARVERSION)/$(INPATH)/res$(UPDMINOREXT)/$(SOLARLANG)
.ENDIF
.ENDIF
SOLARBINDIR:=$(SOLARVERSION)/$(INPATH)/bin$(UPDMINOREXT)
SOLARUCRDIR=$(SOLARVERSION)/$(INPATH)/ucr$(UPDMINOREXT)
SOLARPARDIR=$(SOLARVERSION)/$(INPATH)/par$(UPDMINOREXT)
SOLARXMLDIR=$(SOLARVERSION)/$(INPATH)/xml$(UPDMINOREXT)
SOLARDOCDIR=$(SOLARVERSION)/$(INPATH)/doc$(UPDMINOREXT)
SOLARPCKDIR=$(SOLARVERSION)/$(INPATH)/pck$(UPDMINOREXT)
SOLARSDFDIR=$(SOLARVERSION)/$(INPATH)/sdf$(UPDMINOREXT)
SOLARCOMMONBINDIR=$(SOLARVERSION)/common$(PROEXT)/bin$(UPDMINOREXT)
SOLARCOMMONRESDIR=$(SOLARVERSION)/common$(PROEXT)/res$(UPDMINOREXT)
SOLARCOMMONPCKDIR=$(SOLARVERSION)/common$(PROEXT)/pck$(UPDMINOREXT)
SOLARCOMMONSDFDIR=$(SOLARVERSION)/common$(PROEXT)/sdf$(UPDMINOREXT)
.IF "$(common_build)"==""
SOLARCOMMONBINDIR=$(SOLARBINDIR)
SOLARCOMMONRESDIR=$(SOLARRESDIR)
SOLARCOMMONPCKDIR=$(SOLARPCKDIR)
SOLARCOMMONSDFDIR=$(SOLARSDFDIR)
.ENDIF

.EXPORT : SOLARBINDIR

L10N_MODULE*=$(SRC_ROOT)/translations
ALT_L10N_MODULE*=$(SOLARSRC)$/l10n_so

.IF "$(WITH_LANG)"!=""
.INCLUDE .IGNORE: $(L10N_MODULE)/$(COMMON_OUTDIR)$(PROEXT)/inc/localization_present.mk
.INCLUDE .IGNORE: $(ALT_L10N_MODULE)/$(COMMON_OUTDIR)$(PROEXT)/inc/localization_present.mk

# check for localizations not hosted in l10n module. if a file exists there
# it won't in l10n
.IF "$(ALT_LOCALIZATION_FOUND)"!=""
TRYALTSDF:=$(ALT_L10N_MODULE)$/$(COMMON_OUTDIR)$(PROEXT)$/misc/sdf$/$(PRJNAME)$/$(PATH_IN_MODULE)$/localize.sdf
LOCALIZESDF:=$(strip $(shell @+$(IFEXIST) $(TRYALTSDF) $(THEN) echo $(TRYALTSDF) $(FI)))
.ENDIF			# "$(ALT_LOCALIZATION_FOUND)"!=""
# if the l10n module exists, use split localize.sdf directly from there
.IF "$(LOCALIZATION_FOUND)"!="" && "$(LOCALIZESDF)"==""
# still check for existence as there may be no localization yet
TRYSDF:=$(L10N_MODULE)$/$(COMMON_OUTDIR)$(PROEXT)$/misc/sdf$/$(PRJNAME)$/$(PATH_IN_MODULE)$/localize.sdf
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
# extend library path for OS/2 gcc/wlink
.IF "$(GUI)"=="OS2"
LIB:=$(LB);$(SLB);$(ILIB)
.ENDIF


UNOIDLDEFS+=-DSUPD=$(UPD) -DUPD=$(UPD)

UNOIDLDEPFLAGS=-Mdepend=$(SOLARVER)

UNOIDLINC+=-I. -I.. -I$(PRJ) -I$(PRJ)/inc -I$(PRJ)/$(INPATH)/idl -I$(OUT)/inc -I$(SOLARIDLDIR) -I$(SOLARINCDIR)

CDEFS= -D$(OS) -D$(GUI) -D$(GVER) -D$(COM) -D$(CVER) -D$(CPUNAME)

.IF "$(USE_DEBUG_RUNTIME)" != "" && "$(GUI)"!="OS2"
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
CDEFSSLO=-DSHAREDLIB -D_DLL_
CDEFSGUI=-DGUI
CDEFSCUI=-DCUI
#CDEFSMT=-DMULTITHREAD
CDEFSPROF=-DPROFILE
.IF "$(dbg_build_only)" == ""
CDEFSDEBUG=-DDEBUG
.ENDIF
CDEFSDBGUTIL=-DDBG_UTIL
CDEFSOPT=-DOPTIMIZE

MKDEPFLAGS+=-I$(INCDEPN:s/ / -I/:s/-I-I/-I/)
MKDEPALLINC=$(SOLARINC:s/-I/ -I/)
MKDEPPREINC=-I$(PREPATH)/$(INPATH)/inc$(UPDMINOREXT)
MKDEPSOLENV=-I$(SOLARENV)/inc -I$(SOLARENV)/$(GUI)$(CVER)$(COMEX)/inc
MKDEPSOLVER=-I$(SOLARVERSION)/$(INPATH)/inc$(UPDMINOREXT) -I$(SOLARVERSION)/$(INPATH)/inc
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
RSCDEFS=-D$(GUI) -D$(GVER) -D$(COM) -D$(CVER) $(JAVADEF)

.IF "$(BUILD_SPECIAL)"!=""
RSCDEFS+=-DBUILD_SPECIAL=$(BUILD_SPECIAL)
.ENDIF

RSCDEFIMG*=default_images

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

.IF "$(GUI)" == "OS2"
.INCLUDE : os2.mk
.ENDIF

.IF "$(OOO_LIBRARY_PATH_VAR)" != ""
# Add SOLARLIBDIR at the begin of a (potentially previously undefined) library
# path (LD_LIBRARY_PATH, PATH, etc.; prepending avoids fetching libraries from
# an existing office/URE installation; the ": &&" enables this to work at the
# start of a recipe line that is not prefixed by "+" as well as in the middle of
# an existing && chain:
AUGMENT_LIBRARY_PATH = : && \
    $(OOO_LIBRARY_PATH_VAR)=$(normpath, $(SOLARSHAREDBIN))$${{$(OOO_LIBRARY_PATH_VAR):+:$${{$(OOO_LIBRARY_PATH_VAR)}}}}
AUGMENT_LIBRARY_PATH_LOCAL = : && \
    $(OOO_LIBRARY_PATH_VAR)=$(normpath, $(PWD)/$(DLLDEST)):$(normpath, $(SOLARSHAREDBIN))$${{$(OOO_LIBRARY_PATH_VAR):+:$${{$(OOO_LIBRARY_PATH_VAR)}}}}
.END

# for multiprocess building in external modules
# allow seperate handling
EXTMAXPROCESS*=$(MAXPROCESS)

GDBTRACE=gdb -nx --command=$(SOLARENV)/bin/gdbtrycatchtrace --args

#use with export VALGRIND=memcheck, that method of invocation is used because
#hunspell will aslo run its own unit tests under valgrind when this variable is
#set.
.IF "$(VALGRIND)" != ""
VALGRINDTOOL=valgrind --tool=$(VALGRIND) --leak-check=yes --num-callers=50
G_SLICE*:=always-malloc
.EXPORT : G_SLICE
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

CPPUNITTESTER=$(AUGMENT_LIBRARY_PATH_LOCAL) $(GDBCPPUNITTRACE) $(VALGRINDTOOL) $(SOLARBINDIR)/cppunit/cppunittester
HELPEX=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/helpex
LNGCONVEX=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/lngconvex
HELPLINKER=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/HelpLinker

.IF "$(JAVAINTERPRETER)" == ""
JAVA*:=java
.ELSE
JAVA*:=$(JAVAINTERPRETER)
.ENDIF
.EXPORT : JAVA JAVAI

# Define SCPCOMP without wrapper because pre2par.pl chokes on DOS style
# pathes. (See iz57443)
SCPCOMP=$(PERL) $(SOLARENV)/bin/pre2par.pl
SCPLINK=$(PERL) $(SOLARENV)/bin/par2script.pl
LZIP*=lzip
CPPLCC*=$(AUGMENT_LIBRARY_PATH) $(SOLARBINDIR)/cpplcc

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
.IF ("$(ENABLE_CRASHDUMP)"!="" && "$(ENABLE_CRASHDUMP)"!="DUMMY") || "$(ENABLE_SYMBOLS)"!=""
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
CDEFS+= -DPRODUCT -DNDEBUG
RSCDEFS+= -DPRODUCT 
RSCDEFS+= -DNDEBUG
.ENDIF

.IF "$(DBG_LEVEL)"!=""
CDEFS+=-DOSL_DEBUG_LEVEL=$(DBG_LEVEL)
RSCDEFS+=-DOSL_DEBUG_LEVEL=$(DBG_LEVEL)
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

.IF "$(ENABLE_LAYOUT)" == "TRUE"
CDEFS+=-DENABLE_LAYOUT=1
.ELSE # ENABLE_LAYOUT != TRUE
CFLAGS+=-DENABLE_LAYOUT=0
.ENDIF # ENABLE_LAYOUT != TRUE

.IF "$(ENABLE_LAYOUT_EXPERIMENTAL)" == "TRUE"
CDEFS+=-DENABLE_LAYOUT_EXPERIMENTAL=1
.ELSE # ENABLE_LAYOUT_EXPERIMENTAL != TRUE
CFLAGS+=-DENABLE_LAYOUT_EXPERIMENTAL=0
.ENDIF # ENABLE_LAYOUT_EXPERIMENTAL != TRUE

.IF "$(ENABLE_GSTREAMER)" == "TRUE"
CDEFS+=-DGSTREAMER
.ENDIF

# compose flags and defines for GUI
.IF "$(TARGETTYPE)"=="GUI"
CDEFS+= $(CDEFSGUI)
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
CDEFS+= $(CDEFSCUI)
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
.IF "$(COMPILER_WARN_ERRORS)"!="" && "$(EXTERNAL_WARNINGS_NOT_ERRORS)"==""
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
LINKFLAGSRUNPATH_SDK*=
LINKFLAGSRUNPATH_BRAND*=
LINKFLAGSRUNPATH_OXT*=
LINKFLAGSRUNPATH_BOXT*=
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

COMPONENTPREFIX_URE_NATIVE = vnd.sun.star.expand:$$URE_INTERNAL_LIB_DIR/
COMPONENTPREFIX_URE_JAVA = vnd.sun.star.expand:$$URE_INTERNAL_JAVA_DIR/
.IF "$(OS)" == "WNT"
COMPONENTPREFIX_BASIS_NATIVE = vnd.sun.star.expand:$$BRAND_BASE_DIR/program/
.ELSE
COMPONENTPREFIX_BASIS_NATIVE = vnd.sun.star.expand:$$OOO_BASE_DIR/program/
.END
COMPONENTPREFIX_BASIS_JAVA = vnd.sun.star.expand:$$OOO_BASE_DIR/program/classes/
COMPONENTPREFIX_BASIS_PYTHON = vnd.openoffice.pymodule:
COMPONENTPREFIX_INBUILD_NATIVE = \
    vnd.sun.star.expand:$$OOO_INBUILD_SHAREDLIB_DIR/
COMPONENTPREFIX_INBUILD_JAVA = vnd.sun.star.expand:$$OOO_INBUILD_JAR_DIR/
COMPONENTPREFIX_EXTENSION = ./

# workaround for strange dmake bug:
# if the previous block was a rule or a target, "\#" isn't recognized
# as an escaped "#". if it was an assignment, escaping works...
some_unique_variable_name:=1
