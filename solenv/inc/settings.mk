#*************************************************************************
#
#   $RCSfile: settings.mk,v $
#
#   $Revision: 1.30 $
#
#   last change: $Author: hjs $ $Date: 2001-03-14 17:50:31 $
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
.INCLUDE : unitools.mk

# --- set SOLARVERSION for older workspaces

.IF "$(SOLARVERSION)"==""
SOLARVERSION=$(SOLARVER)$/$(UPD)
.ENDIF

.INCLUDE : minor.mk


%minor.mk :
.IF "$(SOURCEVERSION)"=="$(WORK_STAMP)"
    @+-$(MKDIRHIER) $(SOLARVERSION)$/$(INPATH)$/inc >& $(NULLDEV)
    @+$(COPY) $(SOLARENV)$/inc$/minor.mk $(SOLARVERSION)$/$(UPD)minor.mk >& $(NULLDEV)
.ELSE			# "$(SOURCEVERSION)"=="$(WORK_STAMP)"
    @+echo "#"
    @+echo "#"
    @+echo "#" ERROR: minor.mk in solenv\inc does not match your version!
    @+echo "#"
    @+echo "#"
    force_dmake_to_error
.ENDIF			# "$(SOURCEVERSION)"=="$(WORK_STAMP)"
.IF "$(GUI)"=="UNX"
    @+tr -d "\015" < $(SOLARVERSION)$/$(UPD)minor.mk > $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/$(UPD)minor.mk
.ELSE			# "$(GUI)"=="UNX"
    @+$(COPY) $(SOLARVERSION)$/$(UPD)minor.mk $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/$(UPD)minor.mk >& $(NULLDEV)
.ENDIF			# "$(GUI)"=="UNX"

.INCLUDE : $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/$(UPD)minor.mk


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

CC=
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

LINK=
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
CXX=
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

# can be set on the command line. we shouldn't delete them!
#PROFILE=
#profile=
#REMOTE=
#remote=
# ===========================================================================
# unter NT werden Variablennamen an untergeordnete makefiles UPPERCASE
# weitergereicht, aber case significant ausgewertet!
# ---------------------------------------------------------------------------

.IF "$(TFDEF)"!=""
tfdef=$(TFDEF)
TF_STARONE=TRUE
tf_starone=TRUE
TF_CHAOS=TRUE
tf_chaos=TRUE
.ENDIF

DMAKE_WORK_DIR*:=$(PWD)

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

#.IF "$(DELOPT)"!=""
#delopt*=$(DELOPT)
#.ENDIF

.IF "$(DEBUG)"!=""
debug*=$(DEBUG)
.ENDIF

.IF "$(debug)"!=""
DEBUG*=$(debug)
.ENDIF

.IF "$(GROUP)"!=""
group*=$(GROUP)
.ENDIF

.IF "$(group)"!=""
GROUP*=$(group)
.ENDIF

.IF "$(NODEBUG)"!=""
nodebug=$(NODEBUG)
.ENDIF

.IF "$(PROFILE)"!=""
profile=$(PROFILE)
.ENDIF

.IF "$(hbtoolkit)"!=""
HBTOOLKIT=$(hbtoolkit)
.ENDIF

.IF "$(REMOTE)"!=""
remote=$(REMOTE)
.ENDIF

.IF "$(DBGUTIL)"!=""
dbgutil=$(DBGUTIL)
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

.IF "$(VCL)" != ""
vcl=$(VCL)
DONT_USE_OLE2=TRUE
.ENDIF

.IF "$(SO3)" != ""
so3=$(SO3)
.ENDIF

# --- env flags nicht case sensitiv --------------------------------

.IF "$(envcflags)"!=""
ENVCFLAGS*=$(envcflags)
.ENDIF

.IF "$(envcflagscxx)"!=""
ENVCFLAGSCXX*=$(envcflagscxx)
.ENDIF


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

# --- Parameter Einstellungen ueberpruefen und umsetzen ------------

# profile immer mit product
.IF "$(profile)"!=""
.IF "$(product)"==""
product=full
.ENDIF
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
.ELSE
dbgutil!=true
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
TARGETTYPE=GUI
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
.IF "$(NOSMARTUNO)"==""
.SUFFIXES : .exe .lst .lin .smr .dll .obj .dlo .asm .lib .c .hxx .cxx .res .rc .src .srs .hlp .y .yxx .odl .java .class .hid .cpp
.ELSE
.SUFFIXES : .exe .lst .lin .dll .obj .dlo .asm .lib .c .hxx .cxx .res .rc .src .srs .hlp .y .yxx .odl .idl .java .class .hid .cpp
.ENDIF

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
    @+_mkout $(OUT)
.IF "$(GUI)"=="UNX"
    @+echo \# > $(OUT)$/inc$/myworld.mk
.ELSE			# "$(GUI)"=="UNX"
    @+echo # > $(OUT)$/inc$/myworld.mk
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(common_build)"!=""
    @+_mkout $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(OUT))
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
    @+_mkout -r
    @+echo # > $(OUT)$/inc$/myworldremote.mk
.IF "$(common_build)"!=""
    @+_mkout -r $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(OUT))
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
RPACKAGEOBJ=$(ROUT)$/obj$(SMARTPRE)$/$(IDLPACKAGE)
RSLO=$(ROUT)$/slo
RPACKAGESLO=$(ROUT)$/slo$(SMARTPRE)$/$(IDLPACKAGE)
.ELSE
OBJ=$(OUT)$/dbo
SLO=$(OUT)$/dso
ROBJ=$(ROUT)$/dbo
RPACKAGEOBJ=$(ROUT)$/dbo$/$(IDLPACKAGE)
RSLO=$(ROUT)$/dso
RPACKAGESLO=$(ROUT)$/dso$/$(IDLPACKAGE)
.ENDIF
.ELSE
OBJ=$(OUT)$/obj
SLO=$(OUT)$/slo
ROBJ=$(ROUT)$/obj
RPACKAGEOBJ=$(ROUT)$/obj$(SMARTPRE)$/$(IDLPACKAGE)
RSLO=$(ROUT)$/slo
RPACKAGESLO=$(ROUT)$/slo$(SMARTPRE)$/$(IDLPACKAGE)
.ENDIF

# Particle Path
PAR=$(OUT)$/par

# Library-Pfad
LB=$(OUT)$/lib
SLB=$(OUT)$/slb

# wir haben ein ucr verzeichnis
UCR=$(OUT)$/ucr

SOLARIDLDIR=$(SOLARVERSION)$/$(INPATH)$/idl$(EXT_UPDMINOR)

# $(L) nur noch pfad ins solver\upd\...\lib
#L:={$(LB);$(SLB);$(ILIB)}
L=$(SOLARLIBDIR)

#.EXPORT : SLB LB L LIB
.EXPORT : PRJ

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
.INCLUDE : libs.mk

.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2" 
.IF "$(UPDATER)"=="YES"
VERSIONOBJ=$(SLO)$/_version.obj
.ELSE
VERSIONOBJ=_version.obj
.ENDIF
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(UPDATER)"=="YES"
VERSIONOBJ=$(SLO)$/_version.o
.ELSE
VERSIONOBJ=$(SOLARENV)$/$(OUTPATH)$/lib$/_version.o
.ENDIF
.ENDIF


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

# fuer die Segmentierung kann das z.B. fuer dg*.dll uebergebuegelt werden
.IF "$(SEGDEFS_HXX)" == ""
SEGDEFS_HXX=$(INC)$/segdefs_.hxx
.ENDIF

# fuer die Segmentierung kann das z.B. fuer dg*.dll uebergebuegelt werden
.IF "$(FUNCORD)" == ""
.IF "$(USE_FUNCORD)"=="FALSE"
FUNCORD=emptyorg.seg
.ELSE
FUNCORD=funcord.seg
.ENDIF
.ENDIF

# damit gezielt Abhaengigkeiten auf s: angegeben werden koennen

.IF "$(UPDMINOR)" != ""
EXT_UPDMINOR=.$(UPDMINOR)
.ELSE
EXT_UPDMINOR=
.ENDIF
SOLARRESDIR=$(SOLARVERSION)$/$(INPATH)$/res$(EXT_UPDMINOR)
SOLARRESXDIR=$(SOLARVERSION)$/$(INPATH)$/res$(EXT_UPDMINOR)
SOLARLIBDIR=$(SOLARVERSION)$/$(INPATH)$/lib$(EXT_UPDMINOR)
SOLARIDLDIR=$(SOLARVERSION)$/$(INPATH)$/idl$(EXT_UPDMINOR)
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

.IF "$(UPD)">="616"
.IF "$(PRE)"==""
JARDIR=$(CLASSDIR)
.ELSE
JARDIR=$(PRE)$/class
.ENDIF
.ELSE			# "$(UPD)">="616"
# alles mit $(BIN) durch?
.IF "$(PRE)"==""
JARDIR=$(BIN)
.ELSE
JARDIR=$(PRE)$/bin
.ENDIF
.ENDIF			# "$(UPD)">="616"

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

UNOIDLDEFS+=-DSUPD=$(UPD) -DUPD=$(UPD)

UNOIDLDEPFLAGS=-Mdepend=$(SOLARVER)

.IF "$(TF_ONE51)"=="$(WORK_STAMP)"
UNOIDLDEFS+=-DTF_ONE51
.ENDIF

.IF "$(PRE)"!=""
UNOIDLINC!:=-I$(PRE)$/idl $(UNOIDLINC)
.ENDIF
UNOIDLINC+=-I. -I.. -I$(PRJ) -I$(PRJ)$/inc -I$(PRJ)$/$(INPATH)$/idl -I$(OUT)$/inc -I$(SOLARIDLDIR) -I$(SOLARINCDIR)

.IF "$(remote)" != ""
CDEFS= -D$(OS) -D$(GUI) -D$(GVER) -D$(COM) -D$(CVER) -D$(CPUNAME) -D$(REMOTEDEF)
.ELSE
CDEFS= -D$(OS) -D$(GUI) -D$(GVER) -D$(COM) -D$(CVER) -D$(CPUNAME)
.ENDIF

#to be removed soon!!!
.IF "$(BUILD_SOSL)" == ""
.IF "$(USE_XPRINTER)" == ""
.IF "$(OS)" != "MACOSX"
CDEFS += -DUSE_PSPRINT
PSPRINT=true
.ENDIF
.ENDIF
.ENDIF

#to be removed soon!!!
.IF "$(TF_SVDATA)" != ""
CDEFS+=-DTF_SVDATA
.ENDIF

.IF "$(TF_UCB)" != ""
CDEFS+=-DTF_UCB
.ENDIF

#to be removed soon!!!
.IF "$(ENABLEUNICODE)" != ""
CDEFS+=-DENABLEUNICODE
.ENDIF

#to be removed soon!!!
.IF "$(TF_SDBAPI)" != ""
CDEFS+=-DTF_SDBAPI
.ENDIF

#to be removed soon!!!
.IF "$(TF_CFG)" != ""
CDEFS+=-DTF_CFG
.ENDIF

#to be removed soon!!!
.IF "$(TF_FILTER)" != ""
CDEFS+=-DTF_FILTER
.ENDIF

.IF "$(GUI)"=="UNX"
CDEFS+=-DCVER=$(CVER)
.ENDIF
.IF "$(TFDEF)"!=""
CDEFS+= -D$(TFDEF)
.ENDIF

.IF "$(USE_NAMESPACE)"==""
CDEFS+=-D_USE_NO_NAMESPACE
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

MKDEPFLAGS=-I$(INCDEPN:s/ / -I/)
MKDEPALLINC=$(SOLARINCLUDES:s/-I/ -I/)
MKDEPPREINC=-I$(PREPATH)$/$(INPATH)$/inc$(UPDMINOREXT)
MKDEPSOLENV=-I$(SOLARENV)$/inc -I$(SOLARENV)$/$(GUI)$(CVER)$(COMEX)$/inc
MKDEPSOLVER=-I$(SOLARVERSION)$/$(INPATH)$/inc.$(UPDMINOR) -I$(SOLARVERSION)$/$(INPATH)$/inc
MKDEPLOCAL=-I$(INCCOM)

BISON=bison
YACCFLAGS*=-d -o 

.IF "$(GUI)"=="UNX"
SVIDL=svidl
.ELSE
SVIDL=svidl.exe
.ENDIF

.IF "$(GUI)"=="WIN"
.IF "$(product)"=="full"
LDUMP=ldump -Gy
.ENDIF
.ELSE
LDUMP=ldump2
LDUMP2=ldump2
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

.IF "$(make_srs_deps)"!=""
.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
RSC=$(SOLARENV)$/$(GUI)$(CPU)$/rscdep
.ELSE
RSC=$(SOLARENV)$/$(OUTPATH)$/bin$/rscdep
.ENDIF
.ELSE
RSC=rsc
.ENDIF

RSCUPDVER=$(UPD)$(UPDMINOR)
RSCFLAGS=-s
.IF "$(remote)" != ""
RSCDEFS=-D$(GUI) -D$(GVER) -D$(COM) -D$(CVER) -DSUPD=$(UPD) -DBUILD=$(BUILD) -D$(REMOTEDEF) $(JAVADEF)
.ELSE
RSCDEFS=-D$(GUI) -D$(GVER) -D$(COM) -D$(CVER) -DSUPD=$(UPD) -DBUILD=$(BUILD) $(JAVADEF)
.ENDIF
# RSCLINKFLAGS=-lg$(RSCLANG) -l
#RSCGLOINC=$(SOLARENV)$/res
#RSCGLOINC=$(PRJ)$/..$/res

.IF "$(rscres)"!=""
RSCRES=$(rscres)
.ENDIF
.IF "$(RSCRES)"!=""
RSCGLOINC=$(RSCRES)
.ELSE
.IF "$(GUI)" != "UNX"
RSCGLOINC=o:\$(WORK_STAMP)\src\res
.ENDIF
.ENDIF

RSCLOCINC=$(PRJ)$/win$/res
RSCEXTINC=.

.IF "$(DEBUG)" != ""
RSCDEFS+= -DDEBUG
.ENDIF
.IF "$(VCL)"!=""
RSCDEFS+= -DVCL
.ENDIF
.IF "$(SO3)"!=""
RSCDEFS+= -DSO3
.ENDIF
.IF "$(OLD_CHAOS)"!=""
RSCDEFS+= -DOLD_CHAOS
.ENDIF

#to be removed soon!!!
.IF "$(TF_UCB)" != ""
RSCDEFS+=-DTF_UCB
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


# --- Neues Environment erweitern ----------------------------------
CDEFS+= -DSUPD=$(UPD) -DBUILD=$(BUILD)

.IF "$(SOLAR_PLUGIN)"!=""
CDEFS+= -DSOLAR_PLUGIN
RSCDEFS+= -DSOLAR_PLUGIN
.ENDIF

.IF "$(VCL)" != ""
CDEFS+= -DVCL
.ENDIF
.IF "$(SO3)" != ""
CDEFS+= -DSO3
.ENDIF
.IF "$(OLD_CHAOS)"!=""
CDEFS+= -DOLD_CHAOS
.ENDIF


.IF "$(SWEEPER)"!=""
CDEFS+= -DSWEEPER
.ENDIF

.IF "$(T_FORCE)"!=""
CDEFS+= -D$(T_FORCE)
.ENDIF

.IF "$(ACTIVEX)"!=""
CDEFS+= -DACTIVEX
.ENDIF

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
RSCDEFS+=-DDBG_UTIL -D_DEBUG
.ENDIF

.IF "$(dbgutil)$(debug)"!=""
CDEFS+=-D_DEBUG
.ENDIF

.IF "$(product)"!=""
.IF "$(dbgutil)"==""
CDEFS+= -DPRODUCT -DNDEBUG
HDEFS+= -D:PRODUCT
RSCDEFS+= -DPRODUCT
CDEFS+=-DPRODUCT_FULL
HDEFS+=-D:PRODUCT_FULL
RSCDEFS+= -DPRODUCT_FULL -DNDEBUG
.ENDIF
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

.IF "$(dbcs)"!=""
CDEFS+=-DENABLEUNICODE
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
STDSHL=$(STDSHLGUIMT)

.IF "$(TARGETTHREAD)"=="MT"
CDEFSOBJ+= $(CDEFSMT) $(CDEFSOBJMT)
CFLAGSOBJ+=$(CFLAGSOBJGUIMT)
STDLIB=$(STDLIBGUIMT)
.ELSE
CDEFSOBJ+= $(CDEFSST) $(CDEFSOBJST)
CFLAGSOBJ+=$(CFLAGSOBJGUIST)
STDLIB=$(STDLIBGUIST)
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
STDSHL=$(STDSHLCUIMT)

.IF "$(TARGETTHREAD)" == "MT" || "$(TARGETTHREAD)"=="mt"
CDEFSOBJ+=$(CDEFSMT) $(CDEFSOBJMT)
CFLAGSOBJ+=$(CFLAGSOBJCUIMT)
STDLIB=$(STDLIBCUIMT)
.ELSE
CDEFSOBJ+=$(CDEFSST) $(CDEFSOBJST)
CFLAGSOBJ+=$(CFLAGSOBJCUIST)
STDLIB=$(STDLIBCUIST)
.ENDIF

.ENDIF

.IF "$(NO_DEFAULT_STL)"==""
STDLIB+=$(LIBSTLPORT)
STDSHL+=$(LIBSTLPORT)
.ENDIF			# "$(NO_DEFAULT_STL)"==""

.IF "$(SOLAR_JAVA)"!=""
CDEFS+=$(JAVADEF)
.ENDIF

.IF "$(WORK_STAMP)"!=""
CDEFS+=-D$(WORK_STAMP)
.ENDIF

.INCLUDE .IGNORE: $(UPD)$(LAST_MINOR).mk

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


.IF "$(TF_CNTEX)"=="$(WORK_STAMP)"
CDEFS+= -DTF_CNTEX
.ENDIF

.IF "$(TF_PACKAGES)"=="$(WORK_STAMP)"
CDEFS+= -DTF_PACKAGES
TF_PACKAGES_DEF=-DTF_PACKAGES
.ENDIF

.IF "$(TF_ONE51)"=="$(WORK_STAMP)"
CDEFS+= -DTF_ONE51
.ENDIF

#.IF "$(UPDATER)"=="YES"
.IF "$(PRJNAME)"=="chaos" || "$(PRJNAME)"=="svtools"
CDEFS+=-DUPD=\"$(UPD)\" -DMINOR=\"$(LAST_MINOR)\" -DBUILD_ID=\"$(BUILD)\"
.ENDIF
#.ENDIF

.IF "$(TF_NEWEX)"!=""
CDEFS+= -DTF_NEWEX
.ENDIF

.IF "$(UPDATER)"=="YES"
SVXLIGHT=TRUE
.ENDIF

#defaults for UCR HEADER
UNOUCRBASE*=UCR
UNOUCRRDB*=$(SOLARBINDIR)$/applicat.rdb
UNOUCROUT*=$(OUT)$/inc

# --- Compiler -----------------------------------------------------
.INCLUDE : rules.mk

# --- include typelists --------------------------------------------

.IF "$(COMP1TYPELIST)"!=""
.INCLUDE : $(MISC)$/$(COMP1TYPELIST).mk
.ENDIF

.IF "$(COMP2TYPELIST)"!=""
.INCLUDE : $(MISC)$/$(COMP2TYPELIST).mk
.ENDIF

.IF "$(COMP3TYPELIST)"!=""
.INCLUDE : $(MISC)$/$(COMP3TYPELIST).mk
.ENDIF

.IF "$(COMP4TYPELIST)"!=""
.INCLUDE : $(MISC)$/$(COMP4TYPELIST).mk
.ENDIF

.IF "$(COMP5TYPELIST)"!=""
.INCLUDE : $(MISC)$/$(COMP5TYPELIST).mk
.ENDIF

.IF "$(COMP6TYPELIST)"!=""
.INCLUDE : $(MISC)$/$(COMP6TYPELIST).mk
.ENDIF

.IF "$(COMP7TYPELIST)"!=""
.INCLUDE : $(MISC)$/$(COMP7TYPELIST).mk
.ENDIF

.IF "$(COMP8TYPELIST)"!=""
.INCLUDE : $(MISC)$/$(COMP8TYPELIST).mk
.ENDIF

.IF "$(COMP9TYPELIST)"!=""
.INCLUDE : $(MISC)$/$(COMP9TYPELIST).mk
.ENDIF

