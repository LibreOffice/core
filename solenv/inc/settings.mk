#*************************************************************************
#
#   $RCSfile: settings.mk,v $
#
#   $Revision: 1.136 $
#
#   last change: $Author: hjs $ $Date: 2003-08-18 14:48:49 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

MKFILENAME:=SETTINGS.MK

# check for new dmake features

.IF 400<=200
dmake_test_version:
    @+echo dmake version too old!
    force_dmake_to_error
.ENDIF


.IF "$(USE_COMMENT)"!=""
.INCLUDE : comment.mak
.ENDIF

# --- common tool makros --------------------------------------

# moved temporary wrapper here as it is used in unitools.mk
.IF "$(USE_SHELL)"!=""
use_shell*=$(USE_SHELL)
.ENDIF

.INCLUDE : unitools.mk

#have SCRIPTEXT already defined in product.mk but available for all
.IF "$(GUI)"=="WNT"
SCRIPTEXT=.btm
.ELSE           # "$(GUI)"=="WNT"
SCRIPTEXT=
.ENDIF          # "$(GUI)"=="WNT"

.IF "$(PRODUCTNAME)"!=""
.INCLUDE : product.mk
.ENDIF			# "$(PRODUCTNAME)"!=""

# --- set SOLARVERSION for older workspaces

.IF "$(SOLARVERSION)"==""
SOLARVERSION=$(SOLARVER)$/$(UPD)
.ENDIF

.INCLUDE : minor.mk

.IF "$(UPDATER)"!=""
%minor.mk :
.IF "$(SOURCEVERSION)"=="$(WORK_STAMP)"
    @+-$(MKDIRHIER) $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT) >& $(NULLDEV)
.ELSE			# "$(SOURCEVERSION)"=="$(WORK_STAMP)"
    @+echo "#"
    @+echo "#"
    @+echo "#" ERROR: minor.mk in solenv\inc does not match your version!
    @+echo "#"
    @+echo "#"
    force_dmake_to_error
.ENDIF			# "$(SOURCEVERSION)"=="$(WORK_STAMP)"
.IF "$(GUI)"=="UNX"
    @+tr -d "\015" < $(SOLARENV)$/inc$/minor.mk > $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/$(UPD)minor.mk
    @+$(TOUCH) $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/minormkchanged.flg >& $(NULLDEV)
.ELSE			# "$(GUI)"=="UNX"
    @+$(COPY) $(SOLARENV)$/inc$/minor.mk $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/$(UPD)minor.mk >& $(NULLDEV)
    @+$(TOUCH) $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/minormkchanged.flg >& $(NULLDEV)
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF          # "$(UPDATER)"!=""

.IF "$(BSCLIENT)"=="TRUE"
.IF "$(UPDATER)"!="YES"
incorrect_settings:
    @+echo "#"
    @+echo "#" ERROR: setsolar option -bsclient used but UPDATER=YES not set!
    @+echo "#"
    force_dmake_to_error
.ENDIF
.ENDIF

.INCLUDE : $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/$(UPD)minor.mk

%udkversion.mk : $(PRJ)$/inc$/udkversion.mk
    @+$(COPY) $(PRJ)$/inc$/udkversion.mk $@

.INCLUDE .IGNORE : $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/udkversion.mk

# --- reset defined Environments --------------------
.SUFFIXES:

ASM=
AFLAGS=


#JAVA
.IF "$(JDK)" == "J++"
JAVAC=jvc.exe
JAVAI=jview.exe
JAVACPS=/cp
.ELSE
.IF "$(JDK)" == "SCAFE"
.IF "$(JDK_VERSION)" != ""
JAVAC=$(DEVROOT)$/vcafe11$/bin$/sj.exe
JAVAI=$(DEVROOT)$/vcafe11$/java$/bin$/java.exe
.ELSE
JAVAC=$(DEVROOT)$/cafepro$/bin$/sj.exe
JAVAI=$(DEVROOT)$/cafepro$/java$/bin$/java.exe
.ENDIF
JAVACPS=-classpath
JAVARESPONSE=TRUE
.ELSE
#JAVAC=javac.exe
#JAVAI=java.exe
JAVAC=javac
JAVAI=java
JAVACPS=-classpath
JAVARESPONSE=
.ENDIF
.ENDIF

CDEFS=
CXXDEFS=
CDEFSCXX=
CDEFSOBJ=
CDEFSSLO=
CDEFSGUI=
CDEFSCUI=
CDEFSST=
CDEFSMT=
CDEFSPROF=
CDEFSDEBUG=
CDEFSDBGUTIL=
CDEFSOPT=
HDEFS=

#CC=
.IF "$(add_cflags)"!=""
ENVCFLAGS+=$(add_cflags)
.ENDIF
.IF "$(add_cflagscxx)"!=""
ENVCFLAGSCXX+=$(add_cflagscxx)
.ENDIF
CFLAGS=
CFLAGSCALL=
CFLAGSCXX=
CFLAGSCC=
CFLAGSOBJ=
CFLAGSSLO=
CFLAGSOBJGUIST=
CFLAGSOBJCUIST=
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
CFLAGSPCHC=

PCHOBJFLAGSC=
PCHSLOFLAGSC=
PCHOBJFLAGSU=
PCHSLOFLAGSU=

CFLAGSINCXX=

#LINK=
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
STDLIBGUIST=
STDLIBCUIST=
STDLIBGUIMT=
STDLIBCUIMT=
STDSHLGUIMT=
STDSHLCUIMT=
STDOBJ=
STDSLO=
STDLIB=
STDSHL=

LIBMGR=
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
MKDEPFLAGS=
MKDEPALLINC=
MKDEPSOLENV=
MKDEPSOLVER=
MKDEPPREPATH=

AS=
BC=
COBOL=
CPP=
#CXX=
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

MAKELANGDIR=
# doesn't make sense
#IDLSTATICOBJS=

E2P=
CAP=
cap=

# avoid confusion with CUE PROFILE variable...
PROFILE=

# can be set on the command line. we shouldn't delete them!
#profile=
#REMOTE=
#remote=

# reset as setting those manually is no lonjger supported
DBGUTIL=
dbgutil=

# ===========================================================================
# unter NT werden Variablennamen an untergeordnete makefiles UPPERCASE
# weitergereicht, aber case significant ausgewertet!
# ---------------------------------------------------------------------------

DMAKE_WORK_DIR*:=$(subst,/,$/ $(PWD))

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

.IF "$(ADDOPT)"!=""
addopt*=$(ADDOPT)
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

.IF "$(REMOTE)"!=""
remote=$(REMOTE)
.ENDIF

.IF "$(PRJPCH)"!=""
prjpch*=$(PRJPCH)
.ENDIF

.IF "$(prjpch)"!=""
PRJPCH*=$(prjpch)
.ENDIF

.IF "$(PRODUCT)"!=""
product*=$(PRODUCT)
.ENDIF

.IF "$(product)"!=""
PRODUCT*=$(product)
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

.IF "$(RSC_ONCE)"!=""
rsc_once*=$(RSC_ONCE)
.ENDIF

.IF "$(COMMON_BUILD)"!=""
common_build*=$(COMMON_BUILD)
.ENDIF

.IF "$(USE_SHL_VERSIONS)"!=""
use_shl_versions*=$(USE_SHL_VERSIONS)
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
DBG_LEVEL*=2
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

.IF "$(product)"==""
.IF "$(RES_ENUS)"==""
SRSDEFAULT=-SrsDefault
.ENDIF
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


.IF "$(addopt)"!=""
OLD_EXEPTIONS=TRUE
add_opt=
ADD_OPT=
.ENDIF


#.IF "$(delopt)"!=""
#del_opt=
#DEL_OPT=
#.ENDIF


# Optimierung bei FinalCheck funktioniert nicht!
.IF "$(bndchk)" != ""
optimize=
OPTIMIZE=
.ENDIF

.IF "$(USE_NEWCHARSET)"!=""
.IF "$(USE_NEWCHARSET)"!="SRC510"
NEWCHARSET=TRUE
.ENDIF
.ENDIF

######################################################
#
# sprachabh. einstellungen
#
######################################################

.INCLUDE : lang.mk

######################################################

.IF "$(TARGETTYPE)"==""
TARGETTYPE=CUI
.ENDIF

.IF "$(TARGETTHREAD)"==""
.IF "$(GUI)" == "UNX" || "$(GUI)"=="OS2"
TARGETTHREAD=MT
.ELSE
.IF "$(MULTITHREAD_OBJ)"!=""
TARGETTHREAD=MT
.ELSE
TARGETTHREAD=ST
.ENDIF
.ENDIF
.ENDIF

.IF "$(GUIBASE)" == "DOS"
TARGETTYPE=CUI
.ENDIF

# Neues Enironment setzen
.SUFFIXES : .exe .lst .lin .dll .obj .dlo .asm .lib .c .hxx .cxx .res .rc .src .srs .hlp .y .yxx .odl .idl .java .class .hid .cpp

# --- Pfade setzen -------------------------------------------------

# Output-Pfad
# profile has to be first for not getting the .pro extension
.IF "$(profile)"!=""
OUT=$(PRJ)$/$(OUTPATH).cap
ROUT=$(OUTPATH).cap
.ELSE

.IF "$(product)"!=""
OUT=$(PRJ)$/$(OUTPATH).pro
ROUT=$(OUTPATH).pro

.ELSE
.IF "$(profile)"!=""
OUT=$(PRJ)$/$(OUTPATH).cap
ROUT=$(OUTPATH).cap
.ENDIF
.IF "$(dbcs)"!=""
OUT=$(PRJ)$/$(OUTPATH).w
ROUT=$(OUTPATH).w
.ENDIF
# could already be set by makefile.mk
.IF "$(OUT)" == ""
OUT*=$(PRJ)$/$(OUTPATH)
ROUT*=$(OUTPATH)
.ENDIF
.ENDIF
.ENDIF

.IF "$(bndchk)" != ""
OUT:=$(PRJ)$/$(OUTPATH).bnd
ROUT=$(OUTPATH).bnd
.ENDIF
.IF "$(truetime)" != ""
OUT=$(PRJ)$/$(OUTPATH).tt
ROUT=$(OUTPATH).tt
.ENDIF
.IF "$(hbtoolkit)"!=""
OUT=$(PRJ)$/$(OUTPATH).tlk
ROUT=$(OUTPATH).tlk
.ENDIF

# common output tree
COMMON_OUTDIR*=common
.IF "$(common_build)"!=""
.IF "$(no_common_build_reslib)"==""
common_build_reslib=true
.ENDIF			# "$(no_common_build_reslib)"==""
.IF "$(no_common_build_zip)"==""
common_build_zip=true
.ENDIF			# "$(no_common_build_zip)"==""
.IF "$(no_common_build_sign_jar)"==""
common_build_sign_jar=true
.ENDIF			# "$(no_common_build_sign_jar)"==""
.ENDIF			# "$(common_build)"!=""

# --- generate output tree -----------------------------------------

%world.mk :
    @+$(MKOUT) $(ROUT)
.IF "$(GUI)"=="UNX"
    @+echo \# > $(OUT)$/inc$/myworld.mk
.ELSE			# "$(GUI)"=="UNX"
    @+echo # > $(OUT)$/inc$/myworld.mk
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(common_build)"!=""
    @+$(MKOUT) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(ROUT))
.IF "$(GUI)"=="UNX"
    @+echo \# > {$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(OUT))}$/inc$/myworld.mk
.ELSE			# "$(GUI)"=="UNX"
    @+echo # > {$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(OUT))}$/inc$/myworld.mk
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF			# "$(common_build)"!=""

.INCLUDE .IGNORE : $(OUT)$/inc$/myworld.mk
.IF "$(common_build)"!=""
.INCLUDE .IGNORE : {$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(OUT))}$/inc$/myworld.mk
.ENDIF			# "$(common_build)"!=""

# --- ausgabebaum remote erzeugen ----------------------------------
.IF "$(remote)"!=""

%worldremote.mk :
    @+$(MKOUT) -r $(ROUT)
    @+echo # > $(OUT)$/inc$/myworldremote.mk
.IF "$(common_build)"!=""
    @+$(MKOUT) -r $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(ROUT))
    @+echo # > {$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(OUT))}$/inc$/myworldremote.mk
.ENDIF			# "$(common_build)"!=""

.INCLUDE .IGNORE : $(OUT)$/inc$/myworldremote.mk
.IF "$(common_build)"!=""
.INCLUDE .IGNORE : {$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(OUT))}$/inc$/myworldremote.mk
.ENDIF			# "$(common_build)"!=""

.ENDIF			# "$(remote)"!=""

# Misc-Pfad
.IF "$(UNR)"!=""
MISCX=$(OUT)$/umisc
MISC=$(OUT)$/umisc
.ELSE
MISC=$(OUT)$/misc
# Misc-Pfad zur Erzeugung und Verwendung von lang/demo/compact etc.
# abhaengigen .DEF, .MAP und .LNK Files
MISCX=$(OUT)$/misc
.ENDIF

OUTCXX=$(OUT)$/cxx

.IF "$(PACKAGE)"!=""
IDLPACKAGE=$(PACKAGE)
.IF "$(GUI)"=="UNX"
IDLPACKAGENAME:=$(shell, +echo $(IDLPACKAGE) | $(SED) 's/\\/\//g' )
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WNT"
IDLPACKAGENAME:=$(shell, +echo $(IDLPACKAGE) | $(SED) s/\\/\//g )
.ENDIF			# "$(GUI)"=="WNT"
.ELSE
IDLPACKAGE=$(PRJNAME)
IDLPACKAGENAME=$(PRJNAME)
.ENDIF

# Objekt-Pfad
.IF "$(debug)"!=""
.IF "$(GROUP)"!="WRITER"
OBJ=$(OUT)$/obj
SLO=$(OUT)$/slo
ROBJ=$(ROUT)$/obj
RSLO=$(ROUT)$/slo
.ELSE
OBJ=$(OUT)$/dbo
SLO=$(OUT)$/dso
ROBJ=$(ROUT)$/dbo
RSLO=$(ROUT)$/dso
.ENDIF
.ELSE
OBJ=$(OUT)$/obj
SLO=$(OUT)$/slo
ROBJ=$(ROUT)$/obj
RSLO=$(ROUT)$/slo
.ENDIF

# Particle Path
PAR=$(OUT)$/par

# Library-Pfad
LB=$(OUT)$/lib
SLB=$(OUT)$/slb

# wir haben ein ucr verzeichnis
UCR=$(OUT)$/ucr

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
CLASSDIR=$(OUT)$/class
CLASSPATH!:=.$(PATH_SEPERATOR)$(CLASSDIR)$(PATH_SEPERATOR)$(CLASSPATH)
STARJAR=java -classpath $(CLASSPATH)$(PATH_SEPERATOR)$(SOLARENV)$/bin$/jtools.jar com.sun.star.tool.starjar.StarJar

STARDEP=javadep
.IF "$(PRJNAME)"=="sj2"
CLASSPATH!:=$(CLASSPATH:s/sj2/no/)
.ENDIF
.IF "$(PRJNAME)"=="uno"
CLASSPATH!:=$(CLASSPATH:s/uno/no/)
.ENDIF
.IF "$(PRJNAME)"=="tkt"
CLASSPATH!:=$(CLASSPATH:s/tkt/no/)
.ENDIF

# Makros fuer die Librarynamen des Solar
.INCLUDE .IGNORE : office.mk
.IF "$(OFFICEUPD)"==""
OFFICEUPD*:=$(UPD)
.ENDIF			# "$(OFFICEUPD)"==""
.INCLUDE : libs.mk

.IF "$(GUI)"=="WNT"
VERSIONOBJ=$(SLO)$/_version.obj
.ENDIF

.IF "$(GUI)"=="UNX"
VERSIONOBJ=$(SLO)$/_version.o
.ENDIF

.IF "$(GUI)"=="WNT"
WINVERSIONNAMES=$(UNIXVERSIONNAMES)
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="WNT"
SHELLLIB=$(LIBPRE) gdi32.lib $(LIBPRE) shell32.lib $(LIBPRE) advapi32.lib $(LIBPRE) comdlg32.lib
.ENDIF
.IF "$(GUI)"=="WIN"
SHELLLIB=$(LIBPRE) shell.lib
.ENDIF

.IF "$(GUI)"=="WIN"
DDEMLLIB=$(LIBPRE) ddeml.lib
.ENDIF

.IF "$(GUI)"=="WIN"
QELIB=$(LIBPRE) qelib.lib
.ENDIF
.IF "$(GUI)"=="OS2"
QELIB=$(LIBPRE) qelib32.lib
.ENDIF

.IF "$(GUI)" != "MAC"
SOLARLIBDEPN=\
    $(L)$/basic.lib\
    $(L)$/dg.lib\
    $(L)$/sfxdebug.lib\
    $(L)$/so2.lib\
    $(SVLIBDEPEND)\
    $(L)$/sfx.lib\
    $(L)$/isfx.lib\
    $(L)$/svmem.lib\
    $(L)$/svtool.lib\
    $(L)$/svx.lib\
    $(L)$/tools.lib
.ELSE
SOLARLIBDEPN=\
    $(SOLARLIB)
.ENDIF


# BIN-Pfad
.IF "$(UNR)"!=""
BIN=$(OUT)$/ubin
.ELSE
BIN=$(OUT)$/bin
.ENDIF

# Include-Pfad
# still without -I here s.a. target.mk INCLUDE
INC=$(PRJ)$/inc
INCLOCAL=..$/inc
INCGUI=$(PRJ)$/$(GUIBASE)$/inc
INCCOM=$(OUT)$/inc
INCCOMX=$(OUT)$/inc
INCUNOIDL=$(INCCOM)$/$(PRJNAME)
EXT_INCLUDE:=$(INCLUDE)
I={$(INCLUDE)}
INCDEPN=. $(INCGUI) $(INCLOCAL) $(INC)
.IF "$(PRJINC)"!=""
INCLOCPRJ=$(PRJINC)$/inc
INCDEPN+=$(INCLOCPRJ)
.ENDIF

# Resource-Pfad fuer .SRS
SRS=$(OUT)$/srs
SRSX=$(OUT)$/srs

# Resource-Pfad fuer .RC und .RES
RES=$(OUT)$/res

LDBIN=$(OUT)$/bin
LDRES=$(OUT)$/res
LDMISC=$(OUT)$/misc
LDSRS=$(OUT)$/srs
LDINCCOM=$(OUT)$/inc
.IF "$(NO_REC_RES)"==""
.IF "$(solarlang)" != "deut"
# die "language" BaseDir Pfade, werden von MAKE...DIR in tg_dir.mk gebraucht
LDBIN=$(OUT)$/bin
LDRES=$(OUT)$/res$/$(solarlang)
LDSRS=$(OUT)$/srs$/$(solarlang)
LDMISC=$(OUT)$/misc$/$(solarlang)
LDINC=$(OUT)$/inc$/$(solarlang)
BIN=$(LDBIN)
RES=$(LDRES)
MISCX=$(LDMISC)
SRSX=$(LDSRS)
INCCOMX=$(LDINC)
.ENDIF			# "$(solarlang)" != "deut"
.ENDIF			# "$(NO_REC_RES)"==""

#bmp itself
BMP=bmp
# Bitmap-Pfad fuer bmp
BMP_IN*=$(PRJ)$/res
BMP_OUT*=$(RES)

# das normale MISC wird nicht an LDMISC angepasst, stattdessen MISCX

.IF "$(make_xl)"!=""
BIN=$(PRJ)$/$(OUTPATH).xl$/bin
.ENDIF

.IF "$(product)" == "demo"
BIN=$(LDBIN)$/demo
RES=$(LDRES)$/demo
MISCX=$(LDMISC)$/demo
SRSX=$(LDSRS)$/demo
INCCOMX=$(LDINC)$/demo
.ENDIF

.IF "$(remote)"!=""
BIN=$(LDBIN)$/remote
SLO=$(OUT)$/slo$/remote
OBJ=$(OUT)$/obj$/remote
RES=$(LDRES)$/remote
RSLO=$(INPATH)$/slo$/remote
ROBJ=$(INPATH)$/obj$/remote
MISCX=$(LDMISC)$/remote
MISC=$(OUT)$/misc$/remote
#SRSX=$(SRSX)$/remote
#INCCOMX=$(INCCOMX)$/remote
LB=$(OUT)$/lib$/remote
SLB=$(OUT)$/slb$/remote
REMOTEDEF=REMOTE_APPSERVER
REMOTELIB=$(SOLARVERSION)$/$(INPATH)$/lib$/remote
.ENDIF

.IF "$(product)" == "compact"
BIN=$(LDBIN)$/comp
RES=$(LDRES)$/comp
MISCX=$(LDMISC)$/comp
SRSX=$(LDSRS)$/comp
INCCOMX=$(LDINC)$/comp
.ENDIF

# damit gezielt Abhaengigkeiten auf s: angegeben werden koennen

.IF "$(common_build)"!=""
SOLARIDLDIR=$(SOLARVERSION)$/common$(PROEXT)$/idl$(EXT_UPDMINOR)
.ELSE
SOLARIDLDIR=$(SOLARVERSION)$/$(INPATH)$/idl$(EXT_UPDMINOR)
.ENDIF

.IF "$(UPDMINOR)" != ""
EXT_UPDMINOR=.$(UPDMINOR)
.ELSE
EXT_UPDMINOR=
.ENDIF
SOLARRESDIR=$(SOLARVERSION)$/$(INPATH)$/res$(EXT_UPDMINOR)
SOLARRESXDIR=$(SOLARVERSION)$/$(INPATH)$/res$(EXT_UPDMINOR)
SOLARLIBDIR=$(SOLARVERSION)$/$(INPATH)$/lib$(EXT_UPDMINOR)
SOLARJAVADIR=$(SOLARVERSION)$/$(INPATH)$/java$(EXT_UPDMINOR)
SOLARINCDIR=$(SOLARVERSION)$/$(INPATH)$/inc$(EXT_UPDMINOR)
SOLARINCXDIR=$(SOLARVERSION)$/$(INPATH)$/inc$(EXT_UPDMINOR)
.IF "$(SOLARLANG)"!="deut"
.IF "$(SOLARLANG)" != ""
SOLARINCXDIR=$(SOLARVERSION)$/$(INPATH)$/inc$(EXT_UPDMINOR)$/$(SOLARLANG)
SOLARRESXDIR=$(SOLARVERSION)$/$(INPATH)$/res$(EXT_UPDMINOR)$/$(SOLARLANG)
.ENDIF
.ENDIF
SOLARBINDIR=$(SOLARVERSION)$/$(INPATH)$/bin$(EXT_UPDMINOR)
SOLARUCRDIR=$(SOLARVERSION)$/$(INPATH)$/ucr$(EXT_UPDMINOR)
SOLARPARDIR=$(SOLARVERSION)$/$(INPATH)$/par$(EXT_UPDMINOR)
SOLARXMLDIR=$(SOLARVERSION)$/$(INPATH)$/xml$(EXT_UPDMINOR)
SOLARDOCDIR=$(SOLARVERSION)$/$(INPATH)$/doc$(EXT_UPDMINOR)
SOLARPCKDIR=$(SOLARVERSION)$/$(INPATH)$/pck$(EXT_UPDMINOR)
SOLARCOMMONBINDIR=$(SOLARVERSION)$/common$(PROEXT)$/bin$(EXT_UPDMINOR)
.IF "$(common_build)"==""
SOLARCOMMONBINDIR=$(SOLARBINDIR)
.ENDIF


# Full-Debug Pfade
.IF "$(debug)" != ""
.IF "$(FULLDEBUG)" != ""
OBJ=$(OUT)$/dbo
SLO=$(OUT)$/dso
ROBJ=$(INPATH)$/dbo
RSLO=$(INPATH)$/dso
SLB=$(OUT)$/dlb
LB=$(OUT)$/dib
BIN=$(BIN)$/dbg
.ENDIF
.ENDIF

.IF "$(PRE)"==""
#JARDIR=$(CLASSDIR)
JARDIR=$(OUT)$/class
.ELSE
JARDIR=$(PRE)$/class
.ENDIF

#now mess around with SOLARINC, SOLARINCLUDES, SOLARLIB and LIB
.IF "$(UDKVERSION)"!=""
SOLARINC!:=-I$(SOLARROOT)$/udk$/$(UDKVERSION)$/$(OUTPATH)$/inc $(SOLARINC)
SOLARINCLUDES!:=-I$(SOLARROOT)$/udk$/$(UDKVERSION)$/$(OUTPATH)$/inc $(SOLARINCLUDES)
SOLARLIB!:=-L$(SOLARROOT)$/udk$/$(UDKVERSION)$/$(OUTPATH)$/lib $(SOLARLIB)
ILIB!:=$(SOLARROOT)$/udk$/$(UDKVERSION)$/$(OUTPATH)$/lib;$(ILIB)
.ENDIF			# "$(UDKVERSION)"!=""

# needs to be expanded!!!

# NETBSD_SDO
.IF "$(OS)"=="NETBSD"
LIB:=	$(LB) $(SLB) $(ILIB)
.ELSE
LIB:=$(LB);$(SLB);$(ILIB)
.ENDIF

# --- Environment-Allgemein ----------------------------------------

.IF "$(CPUNAME)" == ""
CPUNAME=CPUNAME_HAS_TO_BE_SET_IN_ENVIRONMENT
.ENDIF

.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
DATESTRING:=$(shell date +%d%m%Y)
.ELSE			# "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
#can't override 4nt internal date to use date.exe :-(
DATESTRING:=$(shell +echo %@IF[%@LEN[%_DAY%]==1,0%_DAY%,%_DAY%]%@IF[%@LEN[%_MONTH%]==1,0%_MONTH%,%_MONTH%]%_YEAR%)
.ENDIF			# "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
SCPDEFS+=-DSCP_DATESTRING=$(DATESTRING)


.IF "$(UDK_MAJOR)"!=""
SCPDEFS+=-DUDK_MAJOR=$(UDK_MAJOR)
.ENDIF			# "$(UDK_MAJOR)"!=""
.IF "$(UDK_MINOR)"!=""
SCPDEFS+=-DUDK_MINOR=$(UDK_MINOR)
.ENDIF			# "$(UDK_MINOR)"!=""
.IF "$(UDK_MICRO)"!=""
SCPDEFS+=-DUDK_MICRO=$(UDK_MICRO)
.ENDIF			# "$(UDK_MICRO)"!=""

.IF "$(OFFICEUPD)"!=""
SCPDEFS+=-DOFFICEUPD=$(OFFICEUPD)
.ENDIF			# "$(OFFICEUPD)"!=""

.IF "$(BUILD_SPECIAL)"!=""
SCPDEFS+=-DBUILD_SPECIAL=$(BUILD_SPECIAL)
.ENDIF			# "$(BUILD_SPECIAL)"!=""

.IF "$(L10N_framework)"!=""
SCPDEFS+=-DISO_CODE=$(L10N_framework)
.ENDIF			# "$(L10N_framework)"!=""

SCPDEFS+=-U$(COMID) -DCOMID=$(COMID) -DCOMNAME=$(COMNAME) -D_$(COMID)
SCPDEFS+=-DCCNUMVER=$(CCNUMVER)
.IF "$(COM)"=="GCC"
SCPDEFS+=-DSHORTSTDCPP3=$(SHORTSTDCPP3)
.ENDIF			# "$(SHORTSTDCPP3)"!=""

UNOIDLDEFS+=-DSUPD=$(UPD) -DUPD=$(UPD)

UNOIDLDEPFLAGS=-Mdepend=$(SOLARVER)

.IF "$(PRE)"!=""
UNOIDLINC!:=-I$(PRE)$/idl $(UNOIDLINC)
.ENDIF
UNOIDLINC+=-I. -I.. -I$(PRJ) -I$(PRJ)$/inc -I$(PRJ)$/$(INPATH)$/idl -I$(OUT)$/inc -I$(SOLARIDLDIR) -I$(SOLARINCDIR)

.IF "$(remote)" != ""
CDEFS= -D$(OS) -D$(GUI) -D$(GVER) -D$(COM) -D$(CVER) -D$(CPUNAME) -D$(REMOTEDEF)
.ELSE
CDEFS= -D$(OS) -D$(GUI) -D$(GVER) -D$(COM) -D$(CVER) -D$(CPUNAME)
.ENDIF

.IF "$(TIMELOG)" != ""
CDEFS+=-DTIMELOG
.ENDIF

.IF "$(GUI)"=="UNX"
CDEFS+=-DCVER=$(CVER)
.ENDIF

.IF "$(USE_NAMESPACE)"==""
CDEFS+=-D_USE_NO_NAMESPACE
.ELSE
CDEFS+=-D_USE_NAMESPACE
.ENDIF

CDEFSCXX=
CDEFSOBJ=
.IF "$(GUI)"=="DOS"
CDEFSSLO=
.ELSE
CDEFSSLO=-DSHAREDLIB -D_DLL_
.ENDIF
CDEFSGUI=-DGUI
CDEFSCUI=-DCUI
CDEFSST=-DSINGLETHREAD
CDEFSMT=-DMULTITHREAD
CDEFSPROF=-DPROFILE
.IF "$(dbg_build_only)" == ""
CDEFSDEBUG=-DDEBUG
.ENDIF
CDEFSDBGUTIL=-DDBG_UTIL
CDEFSOPT=-DOPTIMIZE
HDEFS=-D:$(GUI) -D:$(COM)

MKDEPFLAGS=-D_SOLAR__PRIVATE -I$(INCDEPN:s/ / -I/)
MKDEPALLINC=$(SOLARINC:s/-I/ -I/) $(INCPRE:^"-I":s/-I-I/-I/)
MKDEPPREINC=-I$(PREPATH)$/$(INPATH)$/inc$(UPDMINOREXT)
MKDEPSOLENV=-I$(SOLARENV)$/inc -I$(SOLARENV)$/$(GUI)$(CVER)$(COMEX)$/inc
MKDEPSOLVER=-I$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT) -I$(SOLARVERSION)$/$(INPATH)$/inc
MKDEPLOCAL=-I$(INCCOM)

BISON=bison
YACCFLAGS*=-d -o 

.IF "$(GUI)"=="UNX"
SVIDL=svidl
.ELSE
.IF "$(USE_SHELL)"!="4nt"
SVIDL=guw.pl svidl.exe
.ELSE
SVIDL=svidl.exe
.ENDIF
.ENDIF

.IF "$(GUI)"=="WIN"
.IF "$(product)"=="full"
LDUMP=ldump -Gy
.ENDIF
.ELSE
.IF "$(USE_SHELL)"!="4nt"
LDUMP=guw.pl ldump4
LDUMP2=guw.pl ldump4
.ELSE
LDUMP=ldump4
LDUMP2=ldump4
.ENDIF
.ENDIF

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
RSC=rscdep
.IF "$(GUI)"=="WNT"
.IF "$(USE_SHELL)"!="4nt"
RSC=guw.pl rscdep
.ENDIF
.ENDIF
.ELSE
RSC=rsc
.IF "$(GUI)"=="WNT"
.IF "$(USE_SHELL)"!="4nt"
RSC=guw.pl rsc
.ENDIF
.ENDIF
.ENDIF

RSCUPDVER=$(UPD)$(UPDMINOR)
RSCFLAGS=-s
.IF "$(remote)" != ""
RSCDEFS=-D$(GUI) -D$(GVER) -D$(COM) -D$(CVER) -DSUPD=$(UPD) -DBUILD=$(BUILD) -D$(REMOTEDEF) $(JAVADEF)
.ELSE
RSCDEFS=-D$(GUI) -D$(GVER) -D$(COM) -D$(CVER) -DSUPD=$(UPD) -DBUILD=$(BUILD) $(JAVADEF)
.ENDIF

.IF "$(rscres)"!=""
RSCRES=$(rscres)
.ENDIF
.IF "$(RSCRES)"!=""
RSCGLOINC=$(RSCRES)
.ELSE
RSCGLOINC=$(SOLARSRC)$/res
.ENDIF

RSCLOCINC=$(PRJ)$/win$/res
RSCEXTINC=.

.IF "$(DEBUG)" != ""
RSCDEFS+= -DDEBUG
.ENDIF

UNOIDL=unoidl

# settings for mozilla idl compiler
XPIDL=xpidl
XPIDL_LINK=xpt_link

# alle bisher verwendeten Linker benutzen + um LIBs auf der naechsten Zeile
# weiter zu schreiben, wenn es da eine Ausnahme geben sollte, muss
# LINKEXTENDLINE compilerabhaengig definiert werden
LINKEXTENDLINE=+

LNT=$(DEVROOT)$/lint$/lint
#LNTFLAGS=+v -i$(DEVROOT)$/lint\ -mL options.lnt -u
LNTFLAGS=+v -i$(DEVROOT)$/lint$/ -mL options.lnt -u
LNTFLAGSOUTOBJ=-os

# --- StandardLibraries --------------------------------------------

SVLIBS=tools $(SVLIB) svtool
SFXLIBS=sfx so2 basic
SVXLIBS=svxitems dialogs editeng svrtf svdraw outliner xout si basicide \
        tbxctrls

.IF "$(GUI)" == "DOS"
.INCLUDE : dos.mk
.ENDIF

.IF "$(GUI)" == "WIN"
.INCLUDE : win.mk
.ENDIF

.IF "$(GUI)"=="WNT"
.INCLUDE : wnt.mk
.ENDIF

.IF "$(GUI)" == "AS4"
.INCLUDE : as4.mk
.ENDIF

.IF "$(GUI)" == "OS2"
.INCLUDE : os2.mk
.ENDIF

.IF "$(GUI)" == "UNX"
.INCLUDE : unx.mk
.ENDIF

.IF "$(GUI)" == "MAC"
.INCLUDE : mac.mk
.ENDIF

IDLC*=idlc
REGMERGE*=regmerge
REGCOMPARE*=regcompare
REGCOMP*=regcomp
CPPUMAKER*=cppumaker
JAVAMAKER*=javamaker
RDBMAKER*=rdbmaker
JAVA*=java
SCPCOMP*=scpcomp
SCPLINK*=scplink
LZIP*=lzip
CPPLCC*=cpplcc

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

# --- extend new environment ----------------------------------
CDEFS+= -DSUPD=$(UPD) -DBUILD=$(BUILD)

# flags to enable build with symbols; required for crashdump feature
.IF "$(ENABLE_CRASHDUMP)"!="" || "$(ENABLE_SYMBOLS)"!=""
CFLAGS+=$(CFLAGSENABLESYMBOLS)
.ENDIF          # "$(ENABLE_CRASHDUMP)"!="" || "$(ENABLE_SYMBOLS)"!=""

.IF "$(profile)"!=""
CDEFS+= $(CDEFSPROF)
CFLAGS+= $(CFLAGSPROF)
.IF "$(WST)"!=""
LINKFLAGS=$(LINKFLAGSWST)
.ELSE
LINKFLAGS+=$(LINKFLAGSPROF)
.ENDIF
.ENDIF

.IF "$(debug)"!=""
JAVAFLAGS+=$(JAVAFLAGSDEBUG)
CDEFS+= $(CDEFSDEBUG)
CFLAGS+= $(CFLAGSDEBUG)
.IF "$(COM)"=="WTC"
LINK+= $(LINKFLAGSDEBUG)
.ELSE
LINKFLAGS+= $(LINKFLAGSDEBUG)
.ENDIF
.ENDIF

.IF "$(dbgutil)"!=""
CDEFS+=$(CDEFSDBGUTIL)
CFLAGS+=$(CFLAGSDBGUTIL)
RSCDEFS+=-DDBG_UTIL
.ENDIF

.IF "$(product)"!=""
CDEFS+= -DPRODUCT -DNDEBUG
HDEFS+= -D:PRODUCT
RSCDEFS+= -DPRODUCT 
CDEFS+=-DPRODUCT_FULL
HDEFS+=-D:PRODUCT_FULL
RSCDEFS+= -DPRODUCT_FULL -DNDEBUG
.ENDIF

.IF "$(DBG_LEVEL)"!=""
CDEFS+=-DOSL_DEBUG_LEVEL=$(DBG_LEVEL)
RSCDEFS+=-DOSL_DEBUG_LEVEL=$(DBG_LEVEL)
.ENDIF

.IF "$(svx_light)"!=""
CDEFS+=-DSVX_LIGHT
.ENDIF

.IF "$(optimize)"!=""
CDEFS+=$(CDEFSOPT)
CFLAGS+=$(CFLAGSOPT)
LINKFLAGS+=$(LINKFLAGSOPT)
.ELSE
CFLAGS+=$(CFLAGSNOOPT)
.ENDIF

.IF "$(ENABLE_EXCEPTIONS)"!=""
CFLAGSCXX+=$(CFLAGSEXCEPTIONS)
CDEFS+=-DEXCEPTIONS_ON
.ELSE
.IF "$(OLD_EXCEPTIONS)"==""
CDEFS+=-DEXCEPTIONS_OFF
CFLAGSCXX+=$(CFLAGS_NO_EXCEPTIONS)
.ELSE
CDEFS+=-DEXCEPTIONS_ON
.ENDIF
.ENDIF

.IF "$(PRJPCH)"!=""
.IF "$(PROJECTPCH)"==""
PROJECTPCH=$(TARGET)
.ENDIF
.IF "$(PROJECTPCHSOURCE)"==""
PROJECTPCHSOURCE=$(PROJECTPCH)
.ENDIF
.IF "$(debug)"!=""
.IF "$(PDBTARGET)"==""
PROJECTPCHTARGET=$(MISC)$/$(TARGET).pcd
.ELSE
PROJECTPCHTARGET=$(MISC)$/$(PROJECTPCH).pcd
.ENDIF
.ELSE
PROJECTPCHTARGET=$(MISC)$/$(PROJECTPCH).pch
.ENDIF
.IF "$(EXCEPTIONS_FLAG)"!=""
PROJECTPCHTARGET!:=$(PROJECTPCHTARGET:s/.pc/.xc/)
.ENDIF
.IF "$(COM)"=="MSC"
.IF "$(PROJECTPCH4DLL)"!=""
PCHOBJFLAGSC=
PCHOBJFLAGSU=
PCHSLOFLAGSC=-Yc -Yd -Fp$(PROJECTPCHTARGET)
PCHSLOFLAGSU=-Yu -Yd -Fp$(PROJECTPCHTARGET)
.ELSE
PCHSLOFLAGSC=
PCHSLOFLAGSU=
PCHOBJFLAGSC=-Yc -Yd -Fp$(PROJECTPCHTARGET)
PCHOBJFLAGSU=-Yu -Yd -Fp$(PROJECTPCHTARGET)
.ENDIF
.ENDIF
.ENDIF		# "$(PRJPCH)"!=""
#
# Zusammenbau der Flags und CDefs fuer GUI
.IF "$(TARGETTYPE)"=="GUI"
CDEFS+= $(CDEFSGUI)
CFLAGSSLO+=$(CFLAGSSLOGUIMT)
CDEFSSLO+=$(CDEFSSLOMT)
LINKFLAGSAPP=$(LINKFLAGSAPPGUI)
LINKFLAGSSHL=$(LINKFLAGSSHLGUI)
STDOBJ=$(STDOBJGUI)
STDSLO=$(STDSLOGUI)

.IF "$(TARGETTHREAD)"=="MT"
CDEFSOBJ+= $(CDEFSMT) $(CDEFSOBJMT)
CFLAGSOBJ+=$(CFLAGSOBJGUIMT)
STDLIB=$(STDLIBGUIMT)
STDSHL=$(STDSHLGUIMT)
.ELSE
CDEFSOBJ+= $(CDEFSST) $(CDEFSOBJST)
CFLAGSOBJ+=$(CFLAGSOBJGUIST)
STDLIB=$(STDLIBGUIST)
STDSHL=$(STDSHLGUIST)
.ENDIF

.ENDIF

# Zusammenbau der Flags und CDefs fuer CUI
.IF "$(TARGETTYPE)" == "CUI" || "$(TARGETTYPE)"=="cui"
CDEFS+= $(CDEFSCUI)
CFLAGSSLO+=$(CFLAGSSLOCUIMT)
CDEFSSLO+=$(CDEFSSLOMT)
LINKFLAGSAPP=$(LINKFLAGSAPPCUI)
LINKFLAGSSHL=$(LINKFLAGSSHLCUI)
STDOBJ=$(STDOBJCUI)
STDSLO=$(STDSLOCUI)

.IF "$(TARGETTHREAD)" == "MT" || "$(TARGETTHREAD)"=="mt"
CDEFSOBJ+=$(CDEFSMT) $(CDEFSOBJMT)
CFLAGSOBJ+=$(CFLAGSOBJCUIMT)
STDLIB=$(STDLIBCUIMT)
STDSHL=$(STDSHLCUIMT)
.ELSE
CDEFSOBJ+=$(CDEFSST) $(CDEFSOBJST)
CFLAGSOBJ+=$(CFLAGSOBJCUIST)
STDLIB=$(STDLIBCUIST)
STDSHL=$(STDSHLCUIST)
.ENDIF

.ENDIF

.IF "$(NO_DEFAULT_STL)"==""
STDLIB+=$(LIBSTLPORT)
STDSHL+=$(LIBSTLPORT)
.ENDIF			# "$(NO_DEFAULT_STL)"==""

# fill up unresolved symbols not found else where
.IF "$(FILLUPARC)"!=""
STDLIB+=$(FILLUPARC)
STDSHL+=$(FILLUPARC)
.ENDIF			# "$(FILUPARC)"!=""

.IF "$(DISABLE_JAVA)"==""
.IF "$(SOLAR_JAVA)"!=""
CDEFS+=$(JAVADEF)
.ENDIF          # "$(SOLAR_JAVA)"!=""
.ELSE           # "$(DISABLE_JAVA)"==""
SOLAR_JAVA!:=
.EXPORT : SOLAR_JAVA
.IF "$(JDKPATH)"!=""
environment_confusion:
    @+echo ----------------------------------------------------------
    @+echo -
    @+echo - Error!
    @+echo -
    @+echo - $$JDKPATH and $$DISABLE_JAVA are set. this will lead
    @+echo - to impropper results.
    @+echo -
    @+echo ----------------------------------------------------------
    force_dmake_to_error
.ENDIF          # "$(JDKPATH)"!=""
.ENDIF          # "$(DISABLE_JAVA)"==""

.IF "$(WORK_STAMP)"!=""
CDEFS+=-D$(WORK_STAMP)
.ENDIF

.INCLUDE .IGNORE: $(UPD)$(LAST_MINOR).mk

.IF "$(ENVWARNFLAGS)"==""
.IF "$(COMPILER_WARN_ALL)"!=""
CFLAGSAPPEND+=$(CFLAGSWALL)
.ELSE           # "$(WARN_ALL)"!=""
CFLAGSAPPEND+=$(CFLAGSDFLTWARN)
.ENDIF          # "$(WARN_ALL)"!=""
.ELSE			# "$(ENVWARNFLAGS)"==""
CFLAGSAPPEND+=$(ENVWARNFLAGS)
.ENDIF			# "$(ENVWARNFLAGS)"==""

CDEFS+= $(ENVCDEFS)
CFLAGS+= $(CFLAGSCALL) $(ENVCFLAGS)
CFLAGSCXX+= $(ENVCFLAGSCXX)
# andersrum wg. -TPxxxx
CFLAGSINCXX+:=$(ENVCFLAGSINCXX)
LIBFLAGS+= $(ENVLIBFLAGS)
LINKFLAGS+= $(ENVLINKFLAGS)
RSCFLAGS+= $(ENVRSCFLAGS)
RSCDEFS+= $(ENVRSCDEFS)
# RSCLINKFLAGS+= $(ENVRSCLINKFLAGS)
RCFLAGS+= $(ENVRCFLAGS)
RCLINKFLAGS+= $(ENVRCLINKFLAGS)


#to be removed soon!!!
.IF "$(TF_CNTEX)"=="$(WORK_STAMP)"
CDEFS+= -DTF_CNTEX
.ENDIF

#to be removed soon!!!
.IF "$(TF_CAPELLA)"!=""
CDEFS+= -DTF_CAPELLA
.ENDIF

#.IF "$(UPDATER)"=="YES"
.IF "$(PRJNAME)"=="chaos" || "$(PRJNAME)"=="svtools"
CDEFS+=-DUPD=\"$(UPD)\" -DMINOR=\"$(LAST_MINOR)\" -DBUILD_ID=\"$(BUILD)\"
.ENDIF
#.ENDIF

#to be removed soon!!!
.IF "$(TF_NEWEX)"!=""
CDEFS+= -DTF_NEWEX
.ENDIF

#.IF "$(UPDATER)"=="YES"
#SVXLIGHT=TRUE
#.ENDIF

#defaults for UCR HEADER
UNOUCRBASE*=UCR
UNOUCROUT*=$(OUT)$/inc
UNOUCRRDB*=$(SOLARBINDIR)$/types.rdb
UNOUCRDEP*=$(SOLARBINDIR)$/types.rdb

# --- Compiler -----------------------------------------------------
.INCLUDE : rules.mk

# --- include typelists --------------------------------------------

.IF "$(COMP1TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)$/$(COMP1TYPELIST).mk
$(COMP1TYPELIST)_XML2CMPTYPES:=$(shell xml2cmp -types stdout $(MISC)$/$(COMP1TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP2TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)$/$(COMP2TYPELIST).mk
$(COMP2TYPELIST)_XML2CMPTYPES:=$(shell xml2cmp -types stdout $(MISC)$/$(COMP2TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP3TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)$/$(COMP3TYPELIST).mk
$(COMP3TYPELIST)_XML2CMPTYPES:=$(shell xml2cmp -types stdout $(MISC)$/$(COMP3TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP4TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)$/$(COMP4TYPELIST).mk
$(COMP4TYPELIST)_XML2CMPTYPES:=$(shell xml2cmp -types stdout $(MISC)$/$(COMP4TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP5TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)$/$(COMP5TYPELIST).mk
$(COMP5TYPELIST)_XML2CMPTYPES:=$(shell xml2cmp -types stdout $(MISC)$/$(COMP5TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP6TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)$/$(COMP6TYPELIST).mk
$(COMP6TYPELIST)_XML2CMPTYPES:=$(shell xml2cmp -types stdout $(MISC)$/$(COMP6TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP7TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)$/$(COMP7TYPELIST).mk
$(COMP7TYPELIST)_XML2CMPTYPES:=$(shell xml2cmp -types stdout $(MISC)$/$(COMP7TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP8TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)$/$(COMP8TYPELIST).mk
$(COMP8TYPELIST)_XML2CMPTYPES:=$(shell xml2cmp -types stdout $(MISC)$/$(COMP8TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

.IF "$(COMP9TYPELIST)"!=""
.INCLUDE .IGNORE : $(MISC)$/$(COMP9TYPELIST).mk
$(COMP9TYPELIST)_XML2CMPTYPES:=$(shell xml2cmp -types stdout $(MISC)$/$(COMP9TYPELIST)$($(WINVERSIONNAMES)_MAJOR).xml)
.ENDIF

# workaround for strange dmake bug:
# if the previous block was a rule or a target, "\#" isn't recognized
# as an escaped "#". if it was an assignment, escaping works...
some_unique_variable_name:=1

