#*************************************************************************
#
#   $RCSfile: target.mk,v $
#
#   $Revision: 1.145 $
#
#   last change: $Author: rt $ $Date: 2004-07-13 13:28:40 $
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

MKFILENAME:=TARGET.MK

# ------------------------------------------------------------------
# INCLUDE-Path
# ------------------------------------------------------------------

.IF "$(TARGET)"==""
target_empty=warn_target_empty
.ENDIF

# all use solarinc
# no standalone -I flags!!
INCEXT*=.
INCPRE*=.
INCPOST*=.
.IF "$(PRE)"!=""
ENVINCPRE+=-I$(PRE)$/inc
.ENDIF			# "$(PRE)"!=""
.IF "$(LOCAL_SOLENV)"!=""
SOLARINC+=$(JDKINCS)
SOLARINC+=$(ORCLINC)
SOLARINC+=$(DB2INC)
SOLARINC+=$(DAOINC)
.ENDIF "$(LOCAL_SOLENV)"!=""
.IF "$(PRJINC)"!=""
INCLUDE!:=-I. $(ENVINCPRE) $(INCPRE:^"-I":s/-I-I/-I/) -I$(INCLOCAL) $(INCLOCPRJ:^"-I":s/-I-I/-I/) -I$(INC) -I$(INCGUI) -I$(INCCOM) $(SOLARINC) -I$(INCEXT) -I$(PRJ)$/res -I$(INCPOST)
.ELSE		# "$(PRJINC)"!=""
INCLUDE!:=-I. $(ENVINCPRE) $(INCPRE:^"-I":s/-I-I/-I/) -I$(INCLOCAL) -I$(INC) -I$(INCGUI) -I$(INCCOM) $(SOLARINC) -I$(INCEXT) -I$(PRJ)$/res -I$(INCPOST)
.ENDIF		# "$(PRJINC)"!=""
.EXPORT : LIB
# --- Compiler -----------------------------------------------------

#needed for initial build of dpc files
.IF "$(MAKEFILERC)"!=""
PRJ=$(TEMP)
#TARGET=makefilerc
#PRJNAME=makefilerc
DPCTARGET=
.ELSE
#.IF "$(SLOFILES)$(OBJFILES)$(DEPOBJFILES)$(PARFILES)" != ""
DPCTARGET=$(MISC)$/$(TARGET).dpc
#.ENDIF          # "$(SLOFILES)$(OBJFILES)$(DEPOBJFILES)$(PARFILES)" != ""
.ENDIF
.IF "$(RC_SUBDIRS)"!=""
SUBDIRS:=$(RC_SUBDIRS)
SUBDIRSDEPS=$(RC_SUBDIRSDEPS)
.ENDIF

.INCLUDE : pstrules.mk

.INCLUDE : tg_yxx.mk

.IF "$(nodep)"==""
.IF "$(TESTOBJECTS)"!=""
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(subst,$(OBJ)$/,$(MISC)$/o_ $(LIB1OBJFILES:s/.obj/.dpcc/)))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(subst,$(OBJ)$/,$(MISC)$/o_ $(LIB2OBJFILES:s/.obj/.dpcc/)))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(subst,$(OBJ)$/,$(MISC)$/o_ $(LIB3OBJFILES:s/.obj/.dpcc/)))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(subst,$(OBJ)$/,$(MISC)$/o_ $(LIB4OBJFILES:s/.obj/.dpcc/)))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(subst,$(OBJ)$/,$(MISC)$/o_ $(LIB5OBJFILES:s/.obj/.dpcc/)))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(subst,$(OBJ)$/,$(MISC)$/o_ $(LIB6OBJFILES:s/.obj/.dpcc/)))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(subst,$(OBJ)$/,$(MISC)$/o_ $(LIB7OBJFILES:s/.obj/.dpcc/)))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(subst,$(OBJ)$/,$(MISC)$/o_ $(LIB8OBJFILES:s/.obj/.dpcc/)))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(subst,$(OBJ)$/,$(MISC)$/o_ $(LIB9OBJFILES:s/.obj/.dpcc/)))

DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(SHL1OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(SHL2OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(SHL3OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(SHL4OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(SHL5OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(SHL6OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(SHL7OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(SHL8OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(SLO)$/,$(MISC)$/s_ $(SHL9OBJS:s/.obj/.dpcc/))

DEPFILES_TEST+=$(subst,$(OBJ)$/,$(MISC)$/o_ $(APP1OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(OBJ)$/,$(MISC)$/o_ $(APP2OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(OBJ)$/,$(MISC)$/o_ $(APP3OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(OBJ)$/,$(MISC)$/o_ $(APP4OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(OBJ)$/,$(MISC)$/o_ $(APP5OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(OBJ)$/,$(MISC)$/o_ $(APP6OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(OBJ)$/,$(MISC)$/o_ $(APP7OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(OBJ)$/,$(MISC)$/o_ $(APP8OBJS:s/.obj/.dpcc/))
DEPFILES_TEST+=$(subst,$(OBJ)$/,$(MISC)$/o_ $(APP9OBJS:s/.obj/.dpcc/))
DEPFILESx+=$(uniq $(DEPFILES_TEST))
.ENDIF			# "$(TESTOBJECTS)"!=""

#DEPFILESx+=$(CXXFILES) $(CFILES) $(HXXFILES) $(RCFILES)
.IF "$(L10N_framework)"==""
DEPFILESx+=$(subst,$(SLO)$/,$(MISC)$/s_ $(subst,$(OBJ)$/,$(MISC)$/o_ $(DEPOBJFILES:s/.obj/.dpcc/)))
DEPFILESx+=$(subst,$(OBJ)$/,$(MISC)$/o_ $(OBJFILES:s/.obj/.dpcc/))
DEPFILESx+=$(subst,$(SLO)$/,$(MISC)$/s_ $(SLOFILES:s/.obj/.dpcc/))
.ENDIF			# "$(L10N_framework)"==""
DEPFILESx+=$(subst,$(PAR),$(MISC) $(ALLPARFILES:s/.par/.dpsc/))
.IF "$(L10N_framework)"==""
.IF "$(RCFILES)"!=""
.IF "$(RESNAME)"!=""
DEPFILESx+=$(MISC)$/$(RESNAME).dpcc
.ELSE			# "$(RESNAME)"!=""
DEPFILESx+=$(MISC)$/$(TARGET).dprc
.ENDIF			# "$(RESNAME)"!=""
.ENDIF			# "$(RCFILES)"!=""
.ENDIF          # "$(L10N_framework)"==""
DEPFILES=$(uniq $(DEPFILESx))
.ENDIF			# "$(nodep)"==""

.IF "$(TESTOBJECTS)"!=""
.IF "$(strip $(DEPFILES_TEST))"!=""
.IF "$(DEPFILES)"=="$(strip $(DEPFILESx))"
something_wrong_with_objects :
    @+echo --------------------------------------------------
    @+echo make sure that every object appears in either	
    @+echo     OBJFILES,
    @+echo     SLOFILES
    @+echo  or DEPOBJFILES
    @+echo --------------------------------------------------
#	@+echo ooo$(strip $(DEPFILES_TEST))ooo
#	@+echo $(DEPFILES)
#	@+echo $(DEPFILESx)
    force_dmake_to_error
.ENDIF			# "$(DEPFILES)"!="$(strip $(DEPFILESX))"
.ENDIF			# "$(DEPFILES_TEST)"!=""
.ENDIF			# "$(TESTOBJECTS)"!=""

.INCLUDE : postset.mk

# --- add L10N_framework codes -------------------------------------

alllangiso+=$(L10N_framework:s/,/ /)
completelangiso+=$(L10N_framework:s/,/ /)
RSC_LANG_ISO+:=$(completelangiso)
.EXPORT : RSC_LANG_ISO

.IF "$(depend)" == ""

# -------
# - ALL -
# -------

# with VCL no resources are appended to the application
APP1RES=
APP2RES=
APP3RES=
APP4RES=
APP5RES=
APP6RES=
APP7RES=
APP8RES=
APP9RES=
# unfortunatly there are some applications which don't VCL resources but have
# resources of their own
.IF "$(APP1NOSVRES)" != ""
APP1RES=$(APP1NOSVRES)
.ENDIF
.IF "$(APP2NOSVRES)" != ""
APP2RES=$(APP2NOSVRES)
.ENDIF
.IF "$(APP3NOSVRES)" != ""
APP3RES=$(APP3NOSVRES)
.ENDIF

.IF "$(NO_APP)"!=""
APP1TARGET=
APP2TARGET=
APP3TARGET=
APP4TARGET=
APP5TARGET=
APP6TARGET=
APP7TARGET=
APP8TARGET=
APP9TARGET=
.ENDIF

.IF "$(BUILD_URD_ONLY)" != ""
NOURD=
URD=TRUE
.ENDIF			# "$(URD_ONLY)" != ""

.IF "$(IDLFILES)"!=""

.IF "$(URD)" != ""
URDTARGET:=$(MISC)$/urd_$(TARGET).don
URDFILES+=$(foreach,i,$(IDLFILES:s/.idl/.urd/:f) $(OUT)$/ucr$/$(IDLPACKAGE)$/$i)
UNOIDLTARGETS+=$(URDFILES)

.IF "$(URDFILES)"!=""
LOCALDBTARGET=$(OUT)$/ucr$/$(TARGET).db
.ENDIF

.IF "$(URDDOC)" != ""
URDDOCTARGET:=$(MISC)$/urddoc_$(TARGET).don
URDDOCFILES+=$(foreach,i,$(IDLFILES:s/.idl/.urd/:f) $(OUT)$/ucrdoc$/$(IDLPACKAGE)$/$i)
UNOIDLTARGETS+=$(URDDOCFILES)

.IF "$(URDDOCFILES)"!=""
LOCALDOCDBTARGET=$(OUT)$/ucrdoc$/$(TARGET).db
.ENDIF

.ENDIF			# "$(URDDOC)" != ""
.ENDIF			# "$(URD)" != ""

.ENDIF		# "$(IDLFILES)"!=""

.IF "$(NOURD)"==""
.IF "$(UNOIDLDBFILES)"!=""
UNOIDLDBTARGET=$(OUT)$/ucr$/$(PRJNAME).db
.IF "$(URDDOC)" != ""
UNOIDLDBDOCFILES:=$(subst,$/ucr$/,$/ucrdoc$/ $(UNOIDLDBFILES))
UNOIDLDBDOCTARGET=$(OUT)$/ucrdoc$/$(PRJNAME)_doc.db
.IF "$(UNOIDLDBREGS)"!=""
UNOIDLDBDOCREGS=$(foreach,i,$(UNOIDLDBREGS) $(i:d)$(i:b)_doc$(i:e))
.ENDIF			# "$(UNOIDLDBREGS)"!=""
.ENDIF			# "$(URDDOC)" != ""
.IF "$(UPDATER)"!=""
# SOLARREGDB=$(SOLARBINDIR)$/registry.db
.ENDIF		# "$(UPDATER)"!=""
.ENDIF		# "$(UNOIDLDBFILES)"!=""
.ENDIF		# "$(NOURD)"==""

.IF "$(GUI)"=="WNT"
.IF "$(DOCIDLFILES)"!=""
.IF "$(UPDATER)"=="YES"
UNODOCTARGET=make_uno_doc
.ENDIF
.ENDIF
.ENDIF

.IF "$(IDLFILES)"!=""
.IF "$(PACKAGE)"!=""
IDLDIRS=$(SOLARIDLDIR)$/$(PACKAGE)
.ELSE			# "$(PACKAGE)"!=""
IDLDIRS=$(SOLARIDLDIR)
.ENDIF			# "$(PACKAGE)"!=""

.IF "$(LOCALIDLFILES)$(EXTERNIDLFILES)"!=""
.IF "$(EXTERNIDLFILES)"!=""
DEPIDLFILES:=$(foreach,i,$(IDLFILES) $(IDLDIRS)$/$i)
.ELSE			# "$(EXTERNIDLFILES)"!=""
DEPIDLFILES:=$(IDLFILES)
.ENDIF			# "$(EXTERNIDLFILES)"!=""
.ELSE			# "$(LOCALIDLFILES)$(EXTERNIDLFILES)"!=""
.IF "$(GUI)"=="WNT"
DEPIDLFILES:=$(foreach,i,$(IDLFILES) $(!null,$(shell $(FIND) . -name $i) $i $(shell ($(FIND) $(IDLDIRS) -name $(i:f)) | $(SED) s/\//\\/g )))
.ELSE			# "$(GUI)"=="WNT"
DEPIDLFILES:=$(foreach,i,$(IDLFILES) $(!null,$(shell $(FIND) . -name $i -print) $i $(shell $(FIND) $(IDLDIRS) -name $(i:f) -print )  ))
.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(LOCALIDLFILES)$(EXTERNIDLFILES)"!=""
.ENDIF			# "$(IDLFILES)"!=""

.IF "$(L10N_framework)"==""
.IF "$(JARFILES)"!=""
NEWCLASS:=$(foreach,i,$(JARFILES) $(null,$(shell -+ls -1 $(JARDIR) | $(GREP) $i) $(SOLARBINDIR)$/$i $(JARDIR)$/$i))
.ENDIF			# "$(JARFILES)"!=""
.IF "$(EXTRAJARFILES)"!=""
.IF "$(LOCAL_SOLENV)"!=""
NEWCLASS+=$(foreach,i,$(EXTRAJARFILES) $(COMMON_BUILD_TOOLS)$/$i)
.ELSE			# "$(LOCAL_SOLENV)"!=""
NEWCLASS+=$(foreach,i,$(EXTRAJARFILES) $(SOLARENV)$/bin$/$i)
.ENDIF			# "$(LOCAL_SOLENV)"!=""
.ENDIF			# "$(EXTRAJARFILES)"!=""
.IF "$(GENJAVACLASSFILES)"!=""
NEWCLASS+=$(CLASSGENDIR)
.ENDIF			# "$(GENJAVACLASSFILES)"!=""
.IF "$(NEWCLASS)"!=""
.IF "$(GUI)"=="UNX"
CLASSPATH:=.:$(CLASSDIR):$(XCLASSPATH):$(NEWCLASS:s/ /:/)
.ELSE
CLASSPATH:=.;$(CLASSDIR);$(XCLASSPATH);$(NEWCLASS:s/ /;/)
.ENDIF
.ENDIF			# "$(NEWCLASS)"!=""
.ENDIF			# "$(L10N_framework)"==""

.IF "$(NOOPTFILES)" != ""
NOOPTTARGET=do_it_noopt
.ENDIF

.IF "$(EXCEPTIONSFILES)" != ""
EXCEPTIONSTARGET=do_it_exceptions
.ENDIF

.IF "$(EXCEPTIONSNOOPTFILES)" != ""
EXCEPTIONSNOOPTTARGET=do_it_exceptions_noopt
.ENDIF

.IF "$(LIBTARGET)"==""
.IF "$(OBJFILES)$(IDLOBJFILES)"!=""
OBJTARGET=$(LB)$/$(TARGET).lib
.ENDIF

.IF "$(SLOFILES)$(IDLSLOFILES)"!=""
SLOTARGET=$(SLB)$/$(TARGET).lib
.ENDIF
.ENDIF			# "$(LIBTARGET)"==""

.IF "$(SVXLIGHT)"!=""
.IF "$(SVXLIGHTSLOFILES)"!=""
.IF "$(LIBTARGET)"==""
SVXLIGHTSLOTARGET=$(SLB)$/sxl_$(TARGET).lib
.ENDIF			# "$(LIBTARGET)"==""
REAL_SVXLIGHTSLOFILES=$(foreach,i,$(SVXLIGHTSLOFILES) $(i:d)sxl_$(i:f))
.ENDIF

.IF "$(SVXLIGHTOBJFILES)"!=""
.IF "$(LIBTARGET)"==""
SVXLIGHTOBJTARGET=$(LB)$/sxl_$(TARGET).lib
.ENDIF			# "$(LIBTARGET)"==""
REAL_SVXLIGHTOBJFILES=$(foreach,i,$(SVXLIGHTOBJFILES) $(i:d)sxl_$(i:f))
.ENDIF
.ENDIF			# "$(SVXLIGHT)"!=""

.IF "$(SECOND_BUILD)"!=""
.IF "$($(SECOND_BUILD)_SLOFILES)"!=""
.IF "$(LIBTARGET)"==""
$(SECOND_BUILD)SLOTARGET=$(SLB)$/$(SECOND_BUILD)_$(TARGET).lib
.ENDIF			# "$(LIBTARGET)"==""
REAL_$(SECOND_BUILD)_SLOFILES=$(foreach,i,$($(SECOND_BUILD)_SLOFILES) $(i:d)$(SECOND_BUILD)_$(i:f))
.ENDIF

.IF "$($(SECOND_BUILD)_OBJFILES)"!=""
.IF "$(LIBTARGET)"==""
$(SECOND_BUILD)OBJTARGET=$(LB)$/$(SECOND_BUILD)_$(TARGET).lib
.ENDIF			# "$(LIBTARGET)"==""
REAL_$(SECOND_BUILD)_OBJFILES=$(foreach,i,$($(SECOND_BUILD)_OBJFILES) $(i:d)$(SECOND_BUILD)_$(i:f))
.ENDIF
.ENDIF			# "$(SECOND_BUILD)"!=""

.IF "$(NOLIBOBJTARGET)$(NOLIBSLOTARGET)"!=""
dont_set_NOLIBOBJTARGET:
    @+echo --------------------------------------------------
    @+echo setting NOLIBOBJTARGET or NOLIBSLOTARGET in your makefile
    @+echo will cause serious problems!
    @+echo use DEPOBJFILES instead.
    @+echo --------------------------------------------------
    force_dmake_to_error
.ENDIF			# "$(NOLIBTARGET)"!=""
.IF "$(OBJFILES)"!=""
.IF "$(LIBTARGET)" != ""
NOLIBOBJTARGET=$(OBJFILES)
.ENDIF
.ENDIF

.IF "$(SLOFILES)"!=""
.IF "$(LIBTARGET)" != ""
NOLIBSLOTARGET=$(SLOFILES)
.ENDIF
.ENDIF

.IF "$(SRC1FILES)"!=""
SRC1TARGET=$(SRS)$/$(SRS1NAME).srs
DEPSRS1FILE+=$(MISC)$/$(PWD:f).$(SRS1NAME).dprr
DEPSRSFILES+=$(DEPSRS1FILE)
.ENDIF

.IF "$(SRC2FILES)"!=""
SRC2TARGET=$(SRS)$/$(SRS2NAME).srs
DEPSRS2FILE+=$(MISC)$/$(PWD:f).$(SRS2NAME).dprr
DEPSRSFILES+=$(DEPSRS2FILE)
.ENDIF

.IF "$(SRC3FILES)"!=""
SRC3TARGET=$(SRS)$/$(SRS3NAME).srs
DEPSRS3FILE+=$(MISC)$/$(PWD:f).$(SRS3NAME).dprr
DEPSRSFILES+=$(DEPSRS3FILE)
.ENDIF

.IF "$(SRC4FILES)"!=""
SRC4TARGET=$(SRS)$/$(SRS4NAME).srs
DEPSRS4FILE+=$(MISC)$/$(PWD:f).$(SRS4NAME).dprr
DEPSRSFILES+=$(DEPSRS4FILE)
.ENDIF

.IF "$(SRC5FILES)"!=""
SRC5TARGET=$(SRS)$/$(SRS5NAME).srs
DEPSRS5FILE+=$(MISC)$/$(PWD:f).$(SRS5NAME).dprr
DEPSRSFILES+=$(DEPSRS5FILE)
.ENDIF

.IF "$(SRC6FILES)"!=""
SRC6TARGET=$(SRS)$/$(SRS6NAME).srs
DEPSRS6FILE+=$(MISC)$/$(PWD:f).$(SRS6NAME).dprr
DEPSRSFILES+=$(DEPSRS6FILE)
.ENDIF

.IF "$(SRC7FILES)"!=""
SRC7TARGET=$(SRS)$/$(SRS7NAME).srs
DEPSRS7FILE+=$(MISC)$/$(PWD:f).$(SRS7NAME).dprr
DEPSRSFILES+=$(DEPSRS7FILE)
.ENDIF

.IF "$(SRC8FILES)"!=""
SRC8TARGET=$(SRS)$/$(SRS8NAME).srs
DEPSRS8FILE+=$(MISC)$/$(PWD:f).$(SRS8NAME).dprr
DEPSRSFILES+=$(DEPSRS8FILE)
.ENDIF

.IF "$(SRC9FILES)"!=""
SRC9TARGET=$(SRS)$/$(SRS9NAME).srs
DEPSRS9FILE+=$(MISC)$/$(PWD:f).$(SRS9NAME).dprr
DEPSRSFILES+=$(DEPSRS9FILE)
.ENDIF

.IF "$(SOLAR_JAVA)"!=""
.IF "$(JAVAFILES)$(JAVACLASSFILES)$(GENJAVAFILES)"!=""
.IF "$(JAVAFILES)$(JAVACLASSFILES)"=="$(JAVAFILES)"
JAVACLASSFILES=	$(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)
.ELSE			# "$(JAVAFILES)$(JAVACLASSFILES)"=="$(JAVAFILES)"
.IF "$(JAVAFILES)$(JAVACLASSFILES)"=="$(JAVACLASSFILES)"
JAVAFILES=	$(foreach,i,$(JAVACLASSFILES) $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(i:b).java))
.ENDIF			# "$(JAVAFILES)$(JAVACLASSFILES)"=="$(JAVACLASSFILES)"
.ENDIF			# "$(JAVAFILES)$(JAVACLASSFILES)"=="$(JAVAFILES)"
JAVATARGET:=$(MISC)$/$(TARGET)_dummy.java
.ENDIF			# "$(JAVAFILES)$(JAVACLASSFILES)$(GENJAVAFILES)"!=""

.IF "$(JARTARGET)"!=""
JARCLASSDIRS*=.
.IF "$(NEW_JAR_PACK)"!=""
JARMANIFEST*=$(CLASSDIR)$/META-INF$/MANIFEST.MF
.ENDIF			# "$(NEW_JAR_PACK)"!=""
JARTARGETN=$(CLASSDIR)$/$(JARTARGET)
.IF "$(NOJARDEP)$(NEW_JAR_PACK)"==""
JARTARGETDEP=$(JARTARGET).dep
JARTARGETDEPN=$(MISC)$/$(JARTARGET).dep
.ENDIF			# "$(NOJARDEP)$(NEW_JAR_PACK)"==""
.ENDIF			# "$(JARTARGET)"!=""
.ELSE			# "$(SOLAR_JAVA)"!=""
JAVACLASSFILES:=
JAVAFILES:=
javauno:=
.ENDIF			# "$(SOLAR_JAVA)"!=""

.IF "$(L10N_framework)"==""
.IF "$(RCFILES)"!=""
RESNAME*=$(TARGET)
.IF "$(RCFILES)" != "verinfo.rc"
RCTARGET=$(RES)$/$(RESNAME).res
.ELSE           # "$(RCFILES)" != "verinfo.rc"
RCFILES=
.ENDIF          # "$(RCFILES)" != "verinfo.rc"
.ENDIF          # "$(RCFILES)"!=""
.ENDIF          # "$(L10N_framework)"==""

LOCALIZE_ME_DEST:=$(foreach,i,$(LOCALIZE_ME) $(INCCOM)$/$(i:f:s/_tmpl//))

.IF "$(SCP1TARGET)"!=""
SCP1TARGETN:=$(foreach,i,$(SCP1LINK_PRODUCT_TYPE) $(BIN)$/$i$/$(SCP1TARGET)$(SCPPOST))
SCP1 ?= TNR!:=1
.ENDIF

.IF "$(SCP2TARGET)"!=""
SCP2TARGETN:=$(foreach,i,$(SCP2LINK_PRODUCT_TYPE) $(BIN)$/$i$/$(SCP2TARGET)$(SCPPOST))
SCP2 ?= TNR!:=2
.ENDIF

.IF "$(SCP3TARGET)"!=""
SCP3TARGETN:=$(foreach,i,$(SCP3LINK_PRODUCT_TYPE) $(BIN)$/$i$/$(SCP3TARGET)$(SCPPOST))
SCP3 ?= TNR!:=3
.ENDIF

.IF "$(SCP4TARGET)"!=""
SCP4TARGETN:=$(foreach,i,$(SCP4LINK_PRODUCT_TYPE) $(BIN)$/$i$/$(SCP4TARGET)$(SCPPOST))
SCP4 ?= TNR!:=4
.ENDIF

.IF "$(SCP5TARGET)"!=""
SCP5TARGETN:=$(foreach,i,$(SCP5LINK_PRODUCT_TYPE) $(BIN)$/$i$/$(SCP5TARGET)$(SCPPOST))
SCP5 ?= TNR!:=5
.ENDIF

.IF "$(SCP6TARGET)"!=""
SCP6TARGETN:=$(foreach,i,$(SCP6LINK_PRODUCT_TYPE) $(BIN)$/$i$/$(SCP6TARGET)$(SCPPOST))
SCP6 ?= TNR!:=6
.ENDIF

.IF "$(SCP7TARGET)"!=""
SCP7TARGETN:=$(foreach,i,$(SCP7LINK_PRODUCT_TYPE) $(BIN)$/$i$/$(SCP7TARGET)$(SCPPOST))
SCP7 ?= TNR!:=7
.ENDIF

.IF "$(SCP8TARGET)"!=""
SCP8TARGETN:=$(foreach,i,$(SCP8LINK_PRODUCT_TYPE) $(BIN)$/$i$/$(SCP8TARGET)$(SCPPOST))
SCP8 ?= TNR!:=8
.ENDIF

.IF "$(SCP9TARGET)"!=""
SCP9TARGETN:=$(foreach,i,$(SCP9LINK_PRODUCT_TYPE) $(BIN)$/$i$/$(SCP9TARGET)$(SCPPOST))
SCP9 ?= TNR!:=9
.ENDIF

.IF "$(ZIP1TARGET)$(ZIP2TARGET)$(ZIP3TARGET)$(ZIP4TARGET)$(ZIP5TARGET)$(ZIP6TARGET)$(ZIP7TARGET)$(ZIP8TARGET)$(ZIP9TARGET)"!="$(ZIP1LIST:s/LANGDIR//)$(ZIP2LIST:s/LANGDIR//)$(ZIP3LIST:s/LANGDIR//)$(ZIP4LIST:s/LANGDIR//)$(ZIP5LIST:s/LANGDIR//)$(ZIP6LIST:s/LANGDIR//)$(ZIP7LIST:s/LANGDIR//)$(ZIP8LIST:s/LANGDIR//)$(ZIP9LIST:s/LANGDIR//)"

ZIPALL=ZIPALLTARGET

.ENDIF			#

.IF "$(ZIP1TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP1LIST:s/LANGDIR//)" == "$(ZIP1LIST)"
ZIP1TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP1TARGET).zip
.ELSE
ZIP1TARGETN=$(foreach,i,$(zip1alllangiso) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP1TARGET)_$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP1LIST:s/LANGDIR//)" == "$(ZIP1LIST)"
ZIP1TARGETN=$(BIN)$/$(ZIP1TARGET).zip
.ELSE
ZIP1TARGETN=$(foreach,i,$(zip1alllangiso) $(BIN)$/$(ZIP1TARGET)_$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP1DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,$/bin$/,$/misc$/ $(ZIP1TARGETN:s/.zip/.dpzz)))
ZIPDEPFILES+=$(ZIP1DEPFILE)
.ENDIF

.IF "$(ZIP2TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP2LIST:s/LANGDIR//)" == "$(ZIP2LIST)"
ZIP2TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP2TARGET).zip
.ELSE
ZIP2TARGETN=$(foreach,i,$(zip2alllangiso) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP2TARGET)_$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP2LIST:s/LANGDIR//)" == "$(ZIP2LIST)"
ZIP2TARGETN=$(BIN)$/$(ZIP2TARGET).zip
.ELSE
ZIP2TARGETN=$(foreach,i,$(zip2alllangiso) $(BIN)$/$(ZIP2TARGET)_$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP2DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,$/bin$/,$/misc$/ $(ZIP2TARGETN:s/.zip/.dpzz)))
ZIPDEPFILES+=$(ZIP2DEPFILE)
.ENDIF

.IF "$(ZIP3TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP3LIST:s/LANGDIR//)" == "$(ZIP3LIST)"
ZIP3TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP3TARGET).zip
.ELSE
ZIP3TARGETN=$(foreach,i,$(zip3alllangiso) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP3TARGET)_$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP3LIST:s/LANGDIR//)" == "$(ZIP3LIST)"
ZIP3TARGETN=$(BIN)$/$(ZIP3TARGET).zip
.ELSE
ZIP3TARGETN=$(foreach,i,$(zip3alllangiso) $(BIN)$/$(ZIP3TARGET)_$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP3DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,$/bin$/,$/misc$/ $(ZIP3TARGETN:s/.zip/.dpzz)))
ZIPDEPFILES+=$(ZIP3DEPFILE)
.ENDIF

.IF "$(ZIP4TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP4LIST:s/LANGDIR//)" == "$(ZIP4LIST)"
ZIP4TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP4TARGET).zip
.ELSE
ZIP4TARGETN=$(foreach,i,$(zip4alllangiso) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP4TARGET)_$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP4LIST:s/LANGDIR//)" == "$(ZIP4LIST)"
ZIP4TARGETN=$(BIN)$/$(ZIP4TARGET).zip
.ELSE
ZIP4TARGETN=$(foreach,i,$(zip4alllangiso) $(BIN)$/$(ZIP4TARGET)_$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP4DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,$/bin$/,$/misc$/ $(ZIP4TARGETN:s/.zip/.dpzz)))
ZIPDEPFILES+=$(ZIP4DEPFILE)
.ENDIF

.IF "$(ZIP5TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP5LIST:s/LANGDIR//)" == "$(ZIP5LIST)"
ZIP5TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP5TARGET).zip
.ELSE
ZIP5TARGETN=$(foreach,i,$(zip5alllangiso) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP5TARGET)_$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP5LIST:s/LANGDIR//)" == "$(ZIP5LIST)"
ZIP5TARGETN=$(BIN)$/$(ZIP5TARGET).zip
.ELSE
ZIP5TARGETN=$(foreach,i,$(zip5alllangiso) $(BIN)$/$(ZIP5TARGET)_$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP5DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,$/bin$/,$/misc$/ $(ZIP5TARGETN:s/.zip/.dpzz)))
ZIPDEPFILES+=$(ZIP5DEPFILE)
.ENDIF

.IF "$(ZIP6TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP6LIST:s/LANGDIR//)" == "$(ZIP6LIST)"
ZIP6TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP6TARGET).zip
.ELSE
ZIP6TARGETN=$(foreach,i,$(zip6alllangiso) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP6TARGET)_$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP6LIST:s/LANGDIR//)" == "$(ZIP6LIST)"
ZIP6TARGETN=$(BIN)$/$(ZIP6TARGET).zip
.ELSE
ZIP6TARGETN=$(foreach,i,$(zip6alllangiso) $(BIN)$/$(ZIP6TARGET)_$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP6DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,$/bin$/,$/misc$/ $(ZIP6TARGETN:s/.zip/.dpzz)))
ZIPDEPFILES+=$(ZIP6DEPFILE)
.ENDIF

.IF "$(ZIP7TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP7LIST:s/LANGDIR//)" == "$(ZIP7LIST)"
ZIP7TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP7TARGET).zip
.ELSE
ZIP7TARGETN=$(foreach,i,$(zip7alllangiso) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP7TARGET)_$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP7LIST:s/LANGDIR//)" == "$(ZIP7LIST)"
ZIP7TARGETN=$(BIN)$/$(ZIP7TARGET).zip
.ELSE
ZIP7TARGETN=$(foreach,i,$(zip7alllangiso) $(BIN)$/$(ZIP7TARGET)_$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP7DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,$/bin$/,$/misc$/ $(ZIP7TARGETN:s/.zip/.dpzz)))
ZIPDEPFILES+=$(ZIP7DEPFILE)
.ENDIF

.IF "$(ZIP8TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP8LIST:s/LANGDIR//)" == "$(ZIP8LIST)"
ZIP8TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP8TARGET).zip
.ELSE
ZIP8TARGETN=$(foreach,i,$(zip8alllangiso) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP8TARGET)_$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP8LIST:s/LANGDIR//)" == "$(ZIP8LIST)"
ZIP8TARGETN=$(BIN)$/$(ZIP8TARGET).zip
.ELSE
ZIP8TARGETN=$(foreach,i,$(zip8alllangiso) $(BIN)$/$(ZIP8TARGET)_$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP8DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,$/bin$/,$/misc$/ $(ZIP8TARGETN:s/.zip/.dpzz)))
ZIPDEPFILES+=$(ZIP8DEPFILE)
.ENDIF

.IF "$(ZIP9TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP9LIST:s/LANGDIR//)" == "$(ZIP9LIST)"
ZIP9TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP9TARGET).zip
.ELSE
ZIP9TARGETN=$(foreach,i,$(zip9alllangiso) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP9TARGET)_$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP9LIST:s/LANGDIR//)" == "$(ZIP9LIST)"
ZIP9TARGETN=$(BIN)$/$(ZIP9TARGET).zip
.ELSE
ZIP9TARGETN=$(foreach,i,$(zip9alllangiso) $(BIN)$/$(ZIP9TARGET)_$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP9DEPFILE=$(subst,$(COMMON_OUTDIR),$(OUTPATH) $(subst,$/bin$/,$/misc$/ $(ZIP9TARGETN:s/.zip/.dpzz)))
ZIPDEPFILES+=$(ZIP9DEPFILE)
.ENDIF

.IF "$(APP1TARGET)"!=""
APP1TARGETN=$(BIN)$/$(APP1TARGET)$(EXECPOST)
APP1 ?= TNR!:=1
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(APP1BASE)"==""
APP1BASE=$(BASE)
.ENDIF
APP1BASEX=/BASE:$(APP1BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(APP2TARGET)"!=""
APP2TARGETN=$(BIN)$/$(APP2TARGET)$(EXECPOST)
APP2 ?= TNR!:=2
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(APP2BASE)"==""
APP2BASE=$(BASE)
.ENDIF
APP2BASEX=/BASE:$(APP2BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(APP3TARGET)"!=""
APP3TARGETN=$(BIN)$/$(APP3TARGET)$(EXECPOST)
APP3 ?= TNR!:=3
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(APP3BASE)"==""
APP3BASE=$(BASE)
.ENDIF
APP3BASEX=/BASE:$(APP3BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(APP4TARGET)"!=""
APP4TARGETN=$(BIN)$/$(APP4TARGET)$(EXECPOST)
APP4 ?= TNR!:=4
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(APP4BASE)"==""
APP4BASE=$(BASE)
.ENDIF
APP4BASEX=/BASE:$(APP4BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(APP5TARGET)"!=""
APP5TARGETN=$(BIN)$/$(APP5TARGET)$(EXECPOST)
APP5 ?= TNR!:=5
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(APP5BASE)"==""
APP5BASE=$(BASE)
.ENDIF
APP5BASEX=/BASE:$(APP5BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(APP6TARGET)"!=""
APP6TARGETN=$(BIN)$/$(APP6TARGET)$(EXECPOST)
APP6 ?= TNR!:=6
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(APP6BASE)"==""
APP6BASE=$(BASE)
.ENDIF
APP6BASEX=/BASE:$(APP6BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(APP7TARGET)"!=""
APP7TARGETN=$(BIN)$/$(APP7TARGET)$(EXECPOST)
APP7 ?= TNR!:=7
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(APP7BASE)"==""
APP7BASE=$(BASE)
.ENDIF
APP7BASEX=/BASE:$(APP7BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(APP8TARGET)"!=""
APP8TARGETN=$(BIN)$/$(APP8TARGET)$(EXECPOST)
APP8 ?= TNR!:=8
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(APP8BASE)"==""
APP8BASE=$(BASE)
.ENDIF
APP8BASEX=/BASE:$(APP8BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(APP9TARGET)"!=""
APP9TARGETN=$(BIN)$/$(APP9TARGET)$(EXECPOST)
APP9 ?= TNR!:=9
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(APP9BASE)"==""
APP9BASE=$(BASE)
.ENDIF
APP9BASEX=/BASE:$(APP9BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(SHL1TARGET)"!=""
.IF "$(WINVERSIONNAMES)"!=""
SHL1TARGET!:=$(SHL1TARGET)$($(WINVERSIONNAMES)_MAJOR)
.ENDIF			# "$(WINVERSIONNAMES)"!=""
SHL1TARGETN=$(DLLDEST)$/$(DLLPRE)$(SHL1TARGET)$(DLLPOST)
SHL1 ?= TNR!:=1
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(SHL1BASE)"==""
SHL1BASE=$(BASE)
.ENDIF
SHL1BASEX=/BASE:$(SHL1BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(SHL1IMPLIB)"!=""
SHL1IMPLIBN=$(LB)$/$(SHL1IMPLIB).lib
SHL1IMP ?= TNR!:=1
.ENDIF

.IF "$(SHL2TARGET)"!=""
.IF "$(WINVERSIONNAMES)"!=""
SHL2TARGET!:=$(SHL2TARGET)$($(WINVERSIONNAMES)_MAJOR)
.ENDIF			# "$(WINVERSIONNAMES)"!=""
SHL2TARGETN=$(DLLDEST)$/$(DLLPRE)$(SHL2TARGET)$(DLLPOST)
SHL2 ?= TNR!:=2
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(SHL2BASE)"==""
SHL2BASE=$(BASE)
.ENDIF
SHL2BASEX=/BASE:$(SHL2BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(SHL2IMPLIB)"!=""
SHL2IMPLIBN=$(LB)$/$(SHL2IMPLIB).lib
SHL2IMP ?= TNR!:=2
.ENDIF

.IF "$(SHL3TARGET)"!=""
.IF "$(WINVERSIONNAMES)"!=""
SHL3TARGET!:=$(SHL3TARGET)$($(WINVERSIONNAMES)_MAJOR)
.ENDIF			# "$(WINVERSIONNAMES)"!=""
SHL3TARGETN=$(DLLDEST)$/$(DLLPRE)$(SHL3TARGET)$(DLLPOST)
SHL3 ?= TNR!:=3
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(SHL3BASE)"==""
SHL3BASE=$(BASE)
.ENDIF
SHL3BASEX=/BASE:$(SHL3BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(SHL3IMPLIB)"!=""
SHL3IMPLIBN=$(LB)$/$(SHL3IMPLIB).lib
SHL3IMP ?= TNR!:=3
.ENDIF

.IF "$(SHL4TARGET)"!=""
.IF "$(WINVERSIONNAMES)"!=""
SHL4TARGET!:=$(SHL4TARGET)$($(WINVERSIONNAMES)_MAJOR)
.ENDIF			# "$(WINVERSIONNAMES)"!=""
SHL4TARGETN=$(DLLDEST)$/$(DLLPRE)$(SHL4TARGET)$(DLLPOST)
SHL4 ?= TNR!:=4
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(SHL4BASE)"==""
SHL4BASE=$(BASE)
.ENDIF
SHL4BASEX=/BASE:$(SHL4BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(SHL4IMPLIB)"!=""
SHL4IMPLIBN=$(LB)$/$(SHL4IMPLIB).lib
SHL4IMP ?= TNR!:=4
.ENDIF

.IF "$(SHL5TARGET)"!=""
.IF "$(WINVERSIONNAMES)"!=""
SHL5TARGET!:=$(SHL5TARGET)$($(WINVERSIONNAMES)_MAJOR)
.ENDIF			# "$(WINVERSIONNAMES)"!=""
SHL5TARGETN=$(DLLDEST)$/$(DLLPRE)$(SHL5TARGET)$(DLLPOST)
SHL5 ?= TNR!:=5
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(SHL5BASE)"==""
SHL5BASE=$(BASE)
.ENDIF
SHL5BASEX=/BASE:$(SHL5BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(SHL5IMPLIB)"!=""
SHL5IMPLIBN=$(LB)$/$(SHL5IMPLIB).lib
SHL5IMP ?= TNR!:=5
.ENDIF

.IF "$(SHL6TARGET)"!=""
.IF "$(WINVERSIONNAMES)"!=""
SHL6TARGET!:=$(SHL6TARGET)$($(WINVERSIONNAMES)_MAJOR)
.ENDIF			# "$(WINVERSIONNAMES)"!=""
SHL6TARGETN=$(DLLDEST)$/$(DLLPRE)$(SHL6TARGET)$(DLLPOST)
SHL6 ?= TNR!:=6
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(SHL6BASE)"==""
SHL6BASE=$(BASE)
.ENDIF
SHL6BASEX=/BASE:$(SHL6BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(SHL6IMPLIB)"!=""
SHL6IMPLIBN=$(LB)$/$(SHL6IMPLIB).lib
SHL6IMP ?= TNR!:=6
.ENDIF

.IF "$(SHL7TARGET)"!=""
.IF "$(WINVERSIONNAMES)"!=""
SHL7TARGET!:=$(SHL7TARGET)$($(WINVERSIONNAMES)_MAJOR)
.ENDIF			# "$(WINVERSIONNAMES)"!=""
SHL7TARGETN=$(DLLDEST)$/$(DLLPRE)$(SHL7TARGET)$(DLLPOST)
SHL7 ?= TNR!:=7
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(SHL7BASE)"==""
SHL7BASE=$(BASE)
.ENDIF
SHL7BASEX=/BASE:$(SHL7BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(SHL7IMPLIB)"!=""
SHL7IMPLIBN=$(LB)$/$(SHL7IMPLIB).lib
SHL7IMP ?= TNR!:=7
.ENDIF

.IF "$(SHL8TARGET)"!=""
.IF "$(WINVERSIONNAMES)"!=""
SHL8TARGET!:=$(SHL8TARGET)$($(WINVERSIONNAMES)_MAJOR)
.ENDIF			# "$(WINVERSIONNAMES)"!=""
SHL8TARGETN=$(DLLDEST)$/$(DLLPRE)$(SHL8TARGET)$(DLLPOST)
SHL8 ?= TNR!:=8
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(SHL8BASE)"==""
SHL8BASE=$(BASE)
.ENDIF
SHL8BASEX=/BASE:$(SHL8BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(SHL8IMPLIB)"!=""
SHL8IMPLIBN=$(LB)$/$(SHL8IMPLIB).lib
SHL8IMP ?= TNR!:=8
.ENDIF

.IF "$(SHL9TARGET)"!=""
.IF "$(WINVERSIONNAMES)"!=""
SHL9TARGET!:=$(SHL9TARGET)$($(WINVERSIONNAMES)_MAJOR)
.ENDIF			# "$(WINVERSIONNAMES)"!=""
SHL9TARGETN=$(DLLDEST)$/$(DLLPRE)$(SHL9TARGET)$(DLLPOST)
SHL9 ?= TNR!:=9
.IF "$(BASE)" != ""
.IF "$(GUI)"=="WNT"
.IF "$(SHL9BASE)"==""
SHL9BASE=$(BASE)
.ENDIF
SHL9BASEX=/BASE:$(SHL9BASE)
.ENDIF
.ENDIF
.ENDIF

.IF "$(SHL9IMPLIB)"!=""
SHL9IMPLIBN=$(LB)$/$(SHL9IMPLIB).lib
SHL9IMP ?= TNR!:=9
.ENDIF

# disable all descriptions
.IF "$(NO_SHL_DESCRIPTION)"!=""
NO_SHL1DESCRIPTION=TRUE
NO_SHL2DESCRIPTION=TRUE
NO_SHL3DESCRIPTION=TRUE
NO_SHL4DESCRIPTION=TRUE
NO_SHL5DESCRIPTION=TRUE
NO_SHL6DESCRIPTION=TRUE
NO_SHL7DESCRIPTION=TRUE
NO_SHL8DESCRIPTION=TRUE
NO_SHL9DESCRIPTION=TRUE
.ENDIF			# "$(NO_SHLDESCRIPTION)"!=""

.IF "$(COMP1TYPELIST)$(COMP2TYPELIST)$(COMP3TYPELIST)$(COMP4TYPELIST)$(COMP5TYPELIST)$(COMP6TYPELIST)$(COMP7TYPELIST)$(COMP8TYPELIST)$(COMP9TYPELIST)"!=""
.IF "$(UNOUCRRDB)"!=""
COMPRDB*:=$(UNOUCRRDB)
.ELSE			# "$(UNOUCRRDB)"!=""
COMPRDB*:=$(SOLARBINDIR)$/types.rdb
.ENDIF			# "$(UNOUCRRDB)"!=""
.ENDIF          # "$(COMP1TYPELIST)$(COMP2TYPELIST)$(COMP3TYPELIST)$(COMP4TYPELIST)$(COMP5TYPELIST)$(COMP6TYPELIST)$(COMP7TYPELIST)$(COMP8TYPELIST)$(COMP9TYPELIST)"!=""
.IF "$(COMP1TYPELIST)"!=""
UNOTYPES+=$($(COMP1TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL1DESCRIPTION)"==""
SHL1DESCRIPTIONOBJ*=$(SLO)$/$(COMP1TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL1DESCRIPTION)"==""
COMP1RDBTARGETN:=$(BIN)$/$(COMP1TYPELIST).rdb
#COMP1TYPELISTN:=$(MISC)$/$(COMP1TYPELIST).xml
.ENDIF

.IF "$(COMP2TYPELIST)"!=""
UNOTYPES+=$($(COMP2TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL2DESCRIPTION)"==""
SHL2DESCRIPTIONOBJ*=$(SLO)$/$(COMP2TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL2DESCRIPTION)"==""
COMP2RDBTARGETN:=$(BIN)$/$(COMP2TYPELIST).rdb
#COMP2TYPELISTN:=$(MISC)$/$(COMP2TYPELIST).xml
.ENDIF

.IF "$(COMP3TYPELIST)"!=""
UNOTYPES+=$($(COMP3TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL3DESCRIPTION)"==""
SHL3DESCRIPTIONOBJ*=$(SLO)$/$(COMP3TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL3DESCRIPTION)"==""
COMP3RDBTARGETN:=$(BIN)$/$(COMP3TYPELIST).rdb
#COMP3TYPELISTN:=$(MISC)$/$(COMP3TYPELIST).xml
.ENDIF

.IF "$(COMP4TYPELIST)"!=""
UNOTYPES+=$($(COMP4TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL4DESCRIPTION)"==""
SHL4DESCRIPTIONOBJ*=$(SLO)$/$(COMP4TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL4DESCRIPTION)"==""
COMP4RDBTARGETN:=$(BIN)$/$(COMP4TYPELIST).rdb
#COMP4TYPELISTN:=$(MISC)$/$(COMP4TYPELIST).xml
.ENDIF

.IF "$(COMP5TYPELIST)"!=""
UNOTYPES+=$($(COMP5TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL5DESCRIPTION)"==""
SHL5DESCRIPTIONOBJ*=$(SLO)$/$(COMP5TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL5DESCRIPTION)"==""
COMP5RDBTARGETN:=$(BIN)$/$(COMP5TYPELIST).rdb
#COMP5TYPELISTN:=$(MISC)$/$(COMP5TYPELIST).xml
.ENDIF

.IF "$(COMP6TYPELIST)"!=""
UNOTYPES+=$($(COMP6TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL6DESCRIPTION)"==""
SHL6DESCRIPTIONOBJ*=$(SLO)$/$(COMP6TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL6DESCRIPTION)"==""
COMP6RDBTARGETN:=$(BIN)$/$(COMP6TYPELIST).rdb
#COMP6TYPELISTN:=$(MISC)$/$(COMP6TYPELIST).xml
.ENDIF

.IF "$(COMP7TYPELIST)"!=""
UNOTYPES+=$($(COMP7TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL7DESCRIPTION)"==""
SHL7DESCRIPTIONOBJ*=$(SLO)$/$(COMP7TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL7DESCRIPTION)"==""
COMP7RDBTARGETN:=$(BIN)$/$(COMP7TYPELIST).rdb
#COMP7TYPELISTN:=$(MISC)$/$(COMP7TYPELIST).xml
.ENDIF

.IF "$(COMP8TYPELIST)"!=""
UNOTYPES+=$($(COMP8TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL8DESCRIPTION)"==""
SHL8DESCRIPTIONOBJ*=$(SLO)$/$(COMP8TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL8DESCRIPTION)"==""
COMP8RDBTARGETN:=$(BIN)$/$(COMP8TYPELIST).rdb
#COMP8TYPELISTN:=$(MISC)$/$(COMP8TYPELIST).xml
.ENDIF

.IF "$(COMP9TYPELIST)"!=""
UNOTYPES+=$($(COMP9TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL9DESCRIPTION)"==""
SHL9DESCRIPTIONOBJ*=$(SLO)$/$(COMP9TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL9DESCRIPTION)"==""
COMP9RDBTARGETN:=$(BIN)$/$(COMP9TYPELIST).rdb
#COMP9TYPELISTN:=$(MISC)$/$(COMP9TYPELIST).xml
.ENDIF

UNOTYPES!:=$(strip $(UNOTYPES))

#moved here to get UNOTYPES from COMPxTYPELIST
.IF "$(UNOTYPES)" != ""
UNOUCRHEADER=$(foreach,j,$(subst,.,$/ $(UNOTYPES)) $(UNOUCROUT)$/$(j:+".hpp"))
UNOUCRTARGET:=$(UNOUCRHEADER)
.ENDIF			# "$(UNOTYPES)" != ""

.IF "$(HELPIDFILES)"!=""
MAKE_HELPIDS=$(MISC)$/helpids.don
.ENDIF

.IF "$(SHXXFILES)"!=""
SHXXTARGETN=$(INCCOM)$/$(TARGET).don
.ENDIF

.IF "$(HXX1TARGET)"!=""
HXX1TARGETN=$(INCCOM)$/$(HXX1TARGET).$(HXX1EXT)
HXX1 ?= TNR!:=1
.ENDIF

.IF "$(HXX2TARGET)"!=""
HXX2TARGETN=$(INCCOM)$/$(HXX2TARGET).$(HXX2EXT)
HXX2 ?= TNR!:=2
.ENDIF

.IF "$(HXX3TARGET)"!=""
HXX3TARGETN=$(INCCOM)$/$(HXX3TARGET).$(HXX3EXT)
HXX3 ?= TNR!:=3
.ENDIF

.IF "$(HXX4TARGET)"!=""
HXX4TARGETN=$(INCCOM)$/$(HXX4TARGET).$(HXX4EXT)
HXX4 ?= TNR!:=4
.ENDIF

.IF "$(HXX5TARGET)"!=""
HXX5TARGETN=$(INCCOM)$/$(HXX5TARGET).$(HXX5EXT)
HXX5 ?= TNR!:=5
.ENDIF

.IF "$(HXX6TARGET)"!=""
HXX6TARGETN=$(INCCOM)$/$(HXX6TARGET).$(HXX6EXT)
HXX6 ?= TNR!:=6
.ENDIF

.IF "$(HXX7TARGET)"!=""
HXX7TARGETN=$(INCCOM)$/$(HXX7TARGET).$(HXX7EXT)
HXX7 ?= TNR!:=7
.ENDIF

.IF "$(HXX8TARGET)"!=""
HXX8TARGETN=$(INCCOM)$/$(HXX8TARGET).$(HXX8EXT)
HXX8 ?= TNR!:=8
.ENDIF

.IF "$(HXX9TARGET)"!=""
HXX9TARGETN=$(INCCOM)$/$(HXX9TARGET).$(HXX9EXT)
HXX9 ?= TNR!:=9
.ENDIF

.IF "$(HXX10TARGET)"!=""
HXX10TARGETN=$(INCCOM)$/$(HXX10TARGET).$(HXX10EXT)
HXX10 ?= TNR!:=10
.ENDIF

.IF "$(HXX11TARGET)"!=""
HXX11TARGETN=$(INCCOM)$/$(HXX11TARGET).$(HXX11EXT)
HXX11 ?= TNR!:=11
.ENDIF

.IF "$(HXX12TARGET)"!=""
HXX12TARGETN=$(INCCOM)$/$(HXX12TARGET).$(HXX12EXT)
HXX12 ?= TNR!:=12
.ENDIF

.IF "$(HXX13TARGET)"!=""
HXX13TARGETN=$(INCCOM)$/$(HXX13TARGET).$(HXX13EXT)
HXX13 ?= TNR!:=13
.ENDIF

.IF "$(HXX14TARGET)"!=""
HXX14TARGETN=$(INCCOM)$/$(HXX14TARGET).$(HXX14EXT)
HXX14 ?= TNR!:=14
.ENDIF

.IF "$(HXX15TARGET)"!=""
HXX15TARGETN=$(INCCOM)$/$(HXX15TARGET).$(HXX15EXT)
HXX15 ?= TNR!:=15
.ENDIF

.IF "$(HXX16TARGET)"!=""
HXX16TARGETN=$(INCCOM)$/$(HXX16TARGET).$(HXX16EXT)
HXX16 ?= TNR!:=16
.ENDIF

.IF "$(HXX17TARGET)"!=""
HXX17TARGETN=$(INCCOM)$/$(HXX17TARGET).$(HXX17EXT)
HXX17 ?= TNR!:=17
.ENDIF

.IF "$(HXX18TARGET)"!=""
HXX18TARGETN=$(INCCOM)$/$(HXX18TARGET).$(HXX18EXT)
HXX18 ?= TNR!:=18
.ENDIF

.IF "$(HXX19TARGET)"!=""
HXX19TARGETN=$(INCCOM)$/$(HXX19TARGET).$(HXX19EXT)
HXX19 ?= TNR!:=19
.ENDIF

.IF "$(HXX20TARGET)"!=""
HXX20TARGETN=$(INCCOM)$/$(HXX20TARGET).$(HXX20EXT)
HXX20 ?= TNR!:=20
.ENDIF

.IF "$(HXX21TARGET)"!=""
HXX21TARGETN=$(INCCOM)$/$(HXX21TARGET).$(HXX21EXT)
HXX21 ?= TNR!:=21
.ENDIF

.IF "$(HXX22TARGET)"!=""
HXX22TARGETN=$(INCCOM)$/$(HXX22TARGET).$(HXX22EXT)
HXX22 ?= TNR!:=22
.ENDIF

.IF "$(HXX23TARGET)"!=""
HXX23TARGETN=$(INCCOM)$/$(HXX23TARGET).$(HXX23EXT)
HXX23 ?= TNR!:=23
.ENDIF

.IF "$(HXX24TARGET)"!=""
HXX24TARGETN=$(INCCOM)$/$(HXX24TARGET).$(HXX24EXT)
HXX24 ?= TNR!:=24
.ENDIF

.IF "$(HXX25TARGET)"!=""
HXX25TARGETN=$(INCCOM)$/$(HXX25TARGET).$(HXX25EXT)
HXX25 ?= TNR!:=25
.ENDIF

.IF "$(HXX26TARGET)"!=""
HXX26TARGETN=$(INCCOM)$/$(HXX26TARGET).$(HXX26EXT)
HXX26 ?= TNR!:=26
.ENDIF

.IF "$(HXX27TARGET)"!=""
HXX27TARGETN=$(INCCOM)$/$(HXX27TARGET).$(HXX27EXT)
HXX27 ?= TNR!:=27
.ENDIF

.IF "$(HXX28TARGET)"!=""
HXX28TARGETN=$(INCCOM)$/$(HXX28TARGET).$(HXX28EXT)
HXX28 ?= TNR!:=28
.ENDIF

.IF "$(HXX29TARGET)"!=""
HXX29TARGETN=$(INCCOM)$/$(HXX29TARGET).$(HXX29EXT)
HXX29 ?= TNR!:=29
.ENDIF

.IF "$(HXX30TARGET)"!=""
HXX30TARGETN=$(INCCOM)$/$(HXX30TARGET).$(HXX30EXT)
HXX30 ?= TNR!:=30
.ENDIF

.IF "$(HXX31TARGET)"!=""
HXX31TARGETN=$(INCCOM)$/$(HXX31TARGET).$(HXX31EXT)
HXX31 ?= TNR!:=31
.ENDIF

.IF "$(HXX32TARGET)"!=""
HXX32TARGETN=$(INCCOM)$/$(HXX32TARGET).$(HXX32EXT)
HXX32 ?= TNR!:=32
.ENDIF

.IF "$(HXX33TARGET)"!=""
HXX33TARGETN=$(INCCOM)$/$(HXX33TARGET).$(HXX33EXT)
HXX33 ?= TNR!:=33
.ENDIF

.IF "$(HXX34TARGET)"!=""
HXX34TARGETN=$(INCCOM)$/$(HXX34TARGET).$(HXX34EXT)
HXX34 ?= TNR!:=34
.ENDIF

.IF "$(HXX35TARGET)"!=""
HXX35TARGETN=$(INCCOM)$/$(HXX35TARGET).$(HXX35EXT)
HXX35 ?= TNR!:=35
.ENDIF

.IF "$(HXX36TARGET)"!=""
HXX36TARGETN=$(INCCOM)$/$(HXX36TARGET).$(HXX36EXT)
HXX36 ?= TNR!:=36
.ENDIF

.IF "$(HXX37TARGET)"!=""
HXX37TARGETN=$(INCCOM)$/$(HXX37TARGET).$(HXX37EXT)
HXX37 ?= TNR!:=37
.ENDIF

.IF "$(HXX38TARGET)"!=""
HXX38TARGETN=$(INCCOM)$/$(HXX38TARGET).$(HXX38EXT)
HXX38 ?= TNR!:=38
.ENDIF

.IF "$(HXX39TARGET)"!=""
HXX39TARGETN=$(INCCOM)$/$(HXX39TARGET).$(HXX39EXT)
HXX39 ?= TNR!:=39
.ENDIF


.IF "$(HXXCOPYFILES)" != ""
HXXCOPYTARGET=	copy_hxxcopyfiles
.ENDIF

.IF "$(LIB1TARGET)" != ""
LIB1 ?= TNR!:=1
LIB1TARGETN=$(LIB1TARGET)
.ENDIF

.IF "$(LIB2TARGET)" != ""
LIB2 ?= TNR!:=2
LIB2TARGETN=$(LIB2TARGET)
.ENDIF

.IF "$(LIB3TARGET)" != ""
LIB3 ?= TNR!:=3
LIB3TARGETN=$(LIB3TARGET)
.ENDIF

.IF "$(LIB4TARGET)" != ""
LIB4 ?= TNR!:=4
LIB4TARGETN=$(LIB4TARGET)
.ENDIF

.IF "$(LIB5TARGET)" != ""
LIB5 ?= TNR!:=5
LIB5TARGETN=$(LIB5TARGET)
.ENDIF

.IF "$(LIB6TARGET)" != ""
LIB6 ?= TNR!:=6
LIB6TARGETN=$(LIB6TARGET)
.ENDIF

.IF "$(LIB7TARGET)" != ""
LIB7 ?= TNR!:=7
LIB7TARGETN=$(LIB7TARGET)
.ENDIF

.IF "$(LIB8TARGET)" != ""
LIB8 ?= TNR!:=8
LIB8TARGETN=$(LIB8TARGET)
.ENDIF

.IF "$(LIB9TARGET)" != ""
LIB9 ?= TNR!:=9
LIB9TARGETN=$(LIB9TARGET)
.ENDIF

.IF "$(GUI)"=="WNT"
LIB1ARCHIV=
LIB2ARCHIV=
LIB3ARCHIV=
LIB4ARCHIV=
LIB5ARCHIV=
LIB6ARCHIV=
LIB7ARCHIV=
LIB8ARCHIV=
LIB9ARCHIV=
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(RESLIB1NAME)" != ""
RESLIB1 ?= TNR!:=1
.IF "$(RESLIB1NOVERSION)"==""
RESLIB1VERSION:=$(UPD)
.ENDIF			# "$(RESLIB1NOVERSION)"==""
.IF "$(common_build_reslib)"!=""
RESLIB1TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB1NAME)$(RESLIB1VERSION)LANGEXT.res
RSC_MULTI1=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/rsc_$(RESLIB1NAME)
.ELSE			# "$(common_build_reslib)"!=""
RESLIB1TARGETN=$(BIN)$/$(RESLIB1NAME)$(RESLIB1VERSION)LANGEXT.res
RSC_MULTI1=$(MISC)$/rsc_$(RESLIB1NAME)
.ENDIF			# "$(common_build_reslib)"!=""
# change to iso if resmgr is changed
RESLIB1TARGETN!:=$(foreach,i,$(alllangiso) $(subst,LANGEXT,$i $(RESLIB1TARGETN)))
.ENDIF

.IF "$(RESLIB2NAME)" != ""
RESLIB2 ?= TNR!:=2
.IF "$(RESLIB2NOVERSION)"==""
RESLIB2VERSION:=$(UPD)
.ENDIF			# "$(RESLIB2NOVERSION)"==""
.IF "$(common_build_reslib)"!=""
RESLIB2TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB2NAME)$(RESLIB2VERSION)LANGEXT.res
RSC_MULTI2=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/rsc_$(RESLIB2NAME)
.ELSE			# "$(common_build_reslib)"!=""
RESLIB2TARGETN=$(BIN)$/$(RESLIB2NAME)$(RESLIB2VERSION)LANGEXT.res
RSC_MULTI2=$(MISC)$/rsc_$(RESLIB2NAME)
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB2TARGETN!:=$(foreach,i,$(alllangiso) $(subst,LANGEXT,$i $(RESLIB2TARGETN)))
.ENDIF

.IF "$(RESLIB3NAME)" != ""
RESLIB3 ?= TNR!:=3
.IF "$(RESLIB3NOVERSION)"==""
RESLIB3VERSION:=$(UPD)
.ENDIF			# "$(RESLIB3NOVERSION)"==""
.IF "$(common_build_reslib)"!=""
RESLIB3TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB3NAME)$(RESLIB3VERSION)LANGEXT.res
RSC_MULTI3=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/rsc_$(RESLIB3NAME)
.ELSE			# "$(common_build_reslib)"!=""
RESLIB3TARGETN=$(BIN)$/$(RESLIB3NAME)$(RESLIB3VERSION)LANGEXT.res
RSC_MULTI3=$(MISC)$/rsc_$(RESLIB3NAME)
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB3TARGETN!:=$(foreach,i,$(alllangiso) $(subst,LANGEXT,$i $(RESLIB3TARGETN)))
.ENDIF

.IF "$(RESLIB4NAME)" != ""
RESLIB4 ?= TNR!:=4
.IF "$(RESLIB4NOVERSION)"==""
RESLIB4VERSION:=$(UPD)
.ENDIF			# "$(RESLIB4NOVERSION)"==""
.IF "$(common_build_reslib)"!=""
RESLIB4TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB4NAME)$(RESLIB4VERSION)LANGEXT.res
RSC_MULTI4=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/rsc_$(RESLIB4NAME)
.ELSE			# "$(common_build_reslib)"!=""
RESLIB4TARGETN=$(BIN)$/$(RESLIB4NAME)$(RESLIB4VERSION)LANGEXT.res
RSC_MULTI4=$(MISC)$/rsc_$(RESLIB4NAME)
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB4TARGETN!:=$(foreach,i,$(alllangiso) $(subst,LANGEXT,$i $(RESLIB4TARGETN)))
.ENDIF

.IF "$(RESLIB5NAME)" != ""
RESLIB5 ?= TNR!:=5
.IF "$(RESLIB5NOVERSION)"==""
RESLIB5VERSION:=$(UPD)
.ENDIF			# "$(RESLIB5NOVERSION)"==""
.IF "$(common_build_reslib)"!=""
RESLIB5TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB5NAME)$(RESLIB5VERSION)LANGEXT.res
RSC_MULTI5=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/rsc_$(RESLIB5NAME)
.ELSE			# "$(common_build_reslib)"!=""
RESLIB5TARGETN=$(BIN)$/$(RESLIB5NAME)$(RESLIB5VERSION)LANGEXT.res
RSC_MULTI5=$(MISC)$/rsc_$(RESLIB5NAME)
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB5TARGETN!:=$(foreach,i,$(alllangiso) $(subst,LANGEXT,$i $(RESLIB5TARGETN)))
.ENDIF

.IF "$(RESLIB6NAME)" != ""
RESLIB6 ?= TNR!:=6
.IF "$(RESLIB6NOVERSION)"==""
RESLIB6VERSION:=$(UPD)
.ENDIF			# "$(RESLIB6NOVERSION)"==""
.IF "$(common_build_reslib)"!=""
RESLIB6TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB6NAME)$(RESLIB6VERSION)LANGEXT.res
RSC_MULTI6=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/rsc_$(RESLIB6NAME)
.ELSE			# "$(common_build_reslib)"!=""
RESLIB6TARGETN=$(BIN)$/$(RESLIB6NAME)$(RESLIB6VERSION)LANGEXT.res
RSC_MULTI6=$(MISC)$/rsc_$(RESLIB6NAME)
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB6TARGETN!:=$(foreach,i,$(alllangiso) $(subst,LANGEXT,$i $(RESLIB6TARGETN)))
.ENDIF

.IF "$(RESLIB7NAME)" != ""
RESLIB7 ?= TNR!:=7
.IF "$(RESLIB7NOVERSION)"==""
RESLIB7VERSION:=$(UPD)
.ENDIF			# "$(RESLIB7NOVERSION)"==""
.IF "$(common_build_reslib)"!=""
RESLIB7TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB7NAME)$(RESLIB7VERSION)LANGEXT.res
RSC_MULTI7=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/rsc_$(RESLIB7NAME)
.ELSE			# "$(common_build_reslib)"!=""
RESLIB7TARGETN=$(BIN)$/$(RESLIB7NAME)$(RESLIB7VERSION)LANGEXT.res
RSC_MULTI7=$(MISC)$/rsc_$(RESLIB7NAME)
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB7TARGETN!:=$(foreach,i,$(alllangiso) $(subst,LANGEXT,$i $(RESLIB7TARGETN)))
.ENDIF

.IF "$(RESLIB8NAME)" != ""
RESLIB8 ?= TNR!:=8
.IF "$(RESLIB8NOVERSION)"==""
RESLIB8VERSION:=$(UPD)
.ENDIF			# "$(RESLIB8NOVERSION)"==""
.IF "$(common_build_reslib)"!=""
RESLIB8TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB8NAME)$(RESLIB8VERSION)LANGEXT.res
RSC_MULTI8=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/rsc_$(RESLIB8NAME)
.ELSE			# "$(common_build_reslib)"!=""
RESLIB8TARGETN=$(BIN)$/$(RESLIB8NAME)$(RESLIB8VERSION)LANGEXT.res
RSC_MULTI8=$(MISC)$/rsc_$(RESLIB8NAME)
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB8TARGETN!:=$(foreach,i,$(alllangiso) $(subst,LANGEXT,$i $(RESLIB8TARGETN)))
.ENDIF

.IF "$(RESLIB9NAME)" != ""
RESLIB9 ?= TNR!:=9
.IF "$(RESLIB9NOVERSION)"==""
RESLIB9VERSION:=$(UPD)
.ENDIF			# "$(RESLIB9NOVERSION)"==""
.IF "$(common_build_reslib)"!=""
RESLIB9TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB9NAME)$(RESLIB9VERSION)LANGEXT.res
RSC_MULTI9=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/rsc_$(RESLIB9NAME)
.ELSE			# "$(common_build_reslib)"!=""
RESLIB9TARGETN=$(BIN)$/$(RESLIB9NAME)$(RESLIB9VERSION)LANGEXT.res
RSC_MULTI9=$(MISC)$/rsc_$(RESLIB9NAME)
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB9TARGETN!:=$(foreach,i,$(alllangiso) $(subst,LANGEXT,$i $(RESLIB9TARGETN)))
.ENDIF

.IF "$(INDPRESLIB1NAME)"!=""
INDPRESLIB1TARGETN=$(BIN)$/$(INDPRESLIB1NAME).dll
.ENDIF

.IF "$(RESLIBSPLIT1NAME)"!=""
RESLIBSPLIT1TARGETN=$(BIN)$/$(RESLIBSPLIT1NAME)$(UPD)$(LANGEXT).dll
.ENDIF

.IF "$(DEF1NAME)"!=""
DEF1TARGETN=$(MISC)$/$(DEF1NAME).def
DEF1 ?= TNR!:=1
.ENDIF

.IF "$(DEF2NAME)"!=""
DEF2TARGETN=$(MISC)$/$(DEF2NAME).def
DEF2 ?= TNR!:=2
.ENDIF

.IF "$(DEF3NAME)"!=""
DEF3TARGETN=$(MISC)$/$(DEF3NAME).def
DEF3 ?= TNR!:=3
.ENDIF

.IF "$(DEF4NAME)"!=""
DEF4TARGETN=$(MISC)$/$(DEF4NAME).def
DEF4 ?= TNR!:=4
.ENDIF

.IF "$(DEF5NAME)"!=""
DEF5TARGETN=$(MISC)$/$(DEF5NAME).def
DEF5 ?= TNR!:=5
.ENDIF

.IF "$(DEF6NAME)"!=""
DEF6TARGETN=$(MISC)$/$(DEF6NAME).def
DEF6 ?= TNR!:=6
.ENDIF

.IF "$(DEF7NAME)"!=""
DEF7TARGETN=$(MISC)$/$(DEF7NAME).def
DEF7 ?= TNR!:=7
.ENDIF

.IF "$(DEF8NAME)"!=""
DEF8TARGETN=$(MISC)$/$(DEF8NAME).def
DEF8 ?= TNR!:=8
.ENDIF

.IF "$(DEF9NAME)"!=""
DEF9TARGETN=$(MISC)$/$(DEF9NAME).def
DEF9 ?= TNR!:=9
.ENDIF

.IF "$(SDINAME)"!=""
.DIRCACHE=no
SDITARGET=$(MISC)$/$(SDINAME).don
HIDSIDPARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(SDINAME)_sid.hid
SDI0 ?= TNR!:=
.ENDIF

.IF "$(SDI1NAME)"!=""
.DIRCACHE=no
SDI1TARGET=$(MISC)$/$(SDI1NAME).don
HIDSID1PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(SDI1NAME)_sid.hid
SDI1 ?= TNR!:=1
.ENDIF

.IF "$(SDI2NAME)"!=""
.DIRCACHE=no
SDI2TARGET=$(MISC)$/$(SDI2NAME).don
HIDSID2PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(SDI2NAME)_sid.hid
SDI2 ?= TNR!:=2
.ENDIF

.IF "$(SDI3NAME)"!=""
.DIRCACHE=no
SDI3TARGET=$(MISC)$/$(SDI3NAME).don
HIDSID3PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(SDI3NAME)_sid.hid
SDI3 ?= TNR!:=3
.ENDIF

.IF "$(SDI4NAME)"!=""
.DIRCACHE=no
SDI4TARGET=$(MISC)$/$(SDI4NAME).don
HIDSID4PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(SDI4NAME)_sid.hid
SDI4 ?= TNR!:=4
.ENDIF

.IF "$(SDI5NAME)"!=""
.DIRCACHE=no
SDI5TARGET=$(MISC)$/$(SDI5NAME).don
HIDSID5PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(SDI5NAME)_sid.hid
SDI5 ?= TNR!:=5
.ENDIF

.IF "$(XMLPROPERTIES)"!=""
XMLPROPERTIESN:=$(foreach,i,$(XMLPROPERTIES) $(COMMONMISC)$/$(TARGET)_$(i:s/.xrb/.done/))
.ENDIF			# "$(XMLPROPERTIES)"!=""

.IF "$(XMLXULRES)"!=""
XMLXULRESN:=$(foreach,i,$(XMLXULRES) $(MISC)$/$(TARGET)_xxl_$(i:s/.xxl/.done/))
.ENDIF			# "$(XMLXULRES)"!=""

.IF "$(UNIXTEXT)"!=""
.IF "$(GUI)"=="UNX"
CONVERTUNIXTEXT:=$(UNIXTEXT)
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF			# "$(UNIXTEXT)"!=""

.IF "$(JAVACLASSFILES:s/DEFINED//)"!="" || "$(javauno)"!=""
.IF "$(L10N_framework)"==""
TARGETDPJ=$(MISC)$/$(TARGET).dpj
.ENDIF			# "$(L10N_framework)"==""
.ENDIF

.IF "$(no_hids)$(NO_HIDS)"==""
.IF "$(GUI)"=="WNT"
.IF "$(USE_SHELL)"=="4nt"
BUILDHIDS:=TRUE
.IF "$(GEN_HID_OTHER)"!=""
PRJHIDOTHERTARGET=$(SRS)$/hidother.hid
COMMONPRJHIDOTHERTARGET=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC)$/$(TARGET)_othr.hid)
.ENDIF
.IF "$(GEN_HID)"!=""
PRJHIDTARGET=$(MISC)$/$(PRJNAME).hid
.ENDIF
.IF "$(GEN_HID2)"!=""
PRJHID2TARGET=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/hid.lst
.ENDIF
.ENDIF			# "$(USE_SHELL)"=="4NT"
.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(no_hids)$(NO_HIDS)"==""

.IF "$(OS2_SOLENV_INC)"!=""
OS2_COPY_MK=do_copy_mk
.ENDIF

.IF "$(make_srs_deps)"==""
.IF "$(SRCFILES)" != "" || "$(SRC1FILES)" != "" || "$(SRC2FILES)" != ""
DPRTARGET= $(MISC)$/$(TARGET).dpr
.ENDIF
.ENDIF

.IF "$(make_zip_deps)"==""
.IF "$(ZIP1TARGET)" != "" || "$(ZIP2TARGET)" != "" || "$(ZIP3TARGET)" != ""
DPZTARGET= $(MISC)$/$(TARGET).dpz
.ENDIF			# "$(ZIP1TARGET)" != "" || "$(ZIP2TARGET)" != "" || "$(ZIP3TARGET)" != ""
.ENDIF			# "$(make_zip_deps)"==""

.IF "$(SOLAR_JAVA)"!=""
ALL_JAVA_TARGETS= \
        $(GENJAVAFILES) \
        $(JAVACLASSFILES) $(JAVA1CLASSFILES)	\
        $(JAVA2CLASSFILES)	$(JAVA3CLASSFILES) $(JAVA4CLASSFILES)\
        $(JAVA5CLASSFILES)	$(JAVA6CLASSFILES) $(JAVA7CLASSFILES)\
        $(JAVA8CLASSFILES)	$(JAVA9CLASSFILES) $(JAVA10CLASSFILES)\
        $(JAVA11CLASSFILES)	\
        $(JAVATARGET)	\
        $(TARGETDPJ)
.ENDIF          # "$(SOLAR_JAVA)"!=""
# -------
# - DEF -
# -------

# bei MAKE...DIR die gleiche Reihenfolge wie in settings.mak einhalten!

.IF "$(lintit)"==""
.IF "$(L10N_framework)"!=""
ALLTAR:	\
        $(OS2_COPY_MK)		\
        $(SUBDIRS)		\
        $(DPRTARGET) \
        $(DPZTARGET) \
        $(ZIPALL) \
        $(SDITARGET)	$(SDI1TARGET)	$(SDI2TARGET)		\
        $(SDI3TARGET)	$(SDI4TARGET)	$(SDI5TARGET)		\
        $(XMLPROPERTIESN) \
        $(XMLXULRESN)	\
        $(RCTARGET) \
        $(SCP_PRODUCT_TYPE) \
        $(ALLPARFILES) \
        $(SCP1TARGETN) \
        $(SCP2TARGETN) \
        $(SCP3TARGETN) \
        $(SCP4TARGETN) \
        $(SCP5TARGETN) \
        $(SCP6TARGETN) \
        $(SCP7TARGETN) \
        $(SCP8TARGETN) \
        $(SCP9TARGETN) \
        $(SRC1TARGET)  \
        $(SRC2TARGET)  \
        $(SRC3TARGET)  \
        $(SRC4TARGET)	$(SRC5TARGET)	$(SRC6TARGET)		\
        $(SRC7TARGET)	$(SRC8TARGET)	$(SRC9TARGET)		\
        $(SRC10TARGET)	$(SRC11TARGET)	$(SRC12TARGET)		\
        $(SRC13TARGET)	$(SRC14TARGET)	$(SRC15TARGET)		\
        $(SRC16TARGET) \
        $(RSC_MULTI1) \
        $(RSC_MULTI2) \
        $(RSC_MULTI3) \
        $(RSC_MULTI4) \
        $(RSC_MULTI5) \
        $(RSC_MULTI6) \
        $(RSC_MULTI7) \
        $(RSC_MULTI8) \
        $(RSC_MULTI9) \
        $(INDPRESLIB1TARGETN) \
        $(RESLIB1TARGETN) $(RESLIB2TARGETN) \
        $(RESLIB3TARGETN) $(RESLIB4TARGETN) \
        $(RESLIB5TARGETN) $(RESLIB6TARGETN) \
        $(RESLIB7TARGETN) $(RESLIB8TARGETN) \
        $(RESLIBSPLIT1TARGETN) $(RESLIBSPLIT2TARGETN)\
        $(RESLIBSPLIT3TARGETN) $(RESLIBSPLIT4TARGETN)\
        $(RESLIBSPLIT5TARGETN) $(RESLIBSPLIT6TARGETN)\
        $(RESLIBSPLIT7TARGETN)\
        $(COMMONPRJHIDOTHERTARGET) \
        $(PRJHIDTARGET) \
        $(PRJHID2TARGET) \
        $(REMOTE_BUILD)\
        $(LOCALIZE_ME_DEST)\
        last_target

.ELSE			# "$(L10N_framework)"!=""
#		$(NOOPTTARGET) $(EXCEPTIONSTARGET)

ALLTAR: $(MAKEDEMODIR)	$(MAKECOMPDIR) $(MAKEXLDIR)	\
        $(COMPVERMK) \
        $(JAVAVERMK) \
        $(target_empty) \
        $(OS2_COPY_MK)		\
        $(SUBDIRS)		\
        $(ALLMOZ) \
        $(DELDEFS)		\
        $(YACCTARGET)	\
        $(UNOUCRTARGET)	\
        $(UNOIDLDEPTARGETS) \
        $(URDTARGET) \
        $(URDDOCTARGET) \
        $(UNOIDLTARGETS) \
        $(DEPFILES) $(DPCTARGET) \
        $(DPRTARGET) \
        $(DPZTARGET) \
        $(ZIPALL) \
        $(SDITARGET)	\
        $(UNODOCTARGET)	\
        $(LOCALDBTARGET)	\
        $(LOCALDOCDBTARGET)	\
        $(UNOIDLDBTARGET)	\
        $(UNOIDLDBDOCTARGET)	\
        $(COMP1RDBTARGETN) \
        $(COMP2RDBTARGETN) \
        $(COMP3RDBTARGETN) \
        $(COMP4RDBTARGETN) \
        $(COMP5RDBTARGETN) \
        $(COMP6RDBTARGETN) \
        $(COMP7RDBTARGETN) \
        $(COMP8RDBTARGETN) \
        $(COMP9RDBTARGETN) \
        $(IDL1TARGET)	$(IDL2TARGET)	$(IDL3TARGET)		\
        $(IDL4TARGET)	$(IDL5TARGET)		\
        $(SDI1TARGET)	$(SDI2TARGET)	$(SDI3TARGET)		\
        $(SDI4TARGET)	$(SDI5TARGET)		\
        $(SHXXTARGETN)	\
        $(HXXALLTARGET) \
        $(HXXCOPYTARGET) \
        $(NOLIBOBJTARGET) \
        $(NOLIBSLOTARGET) \
        $(OTHERTARRGET) \
        $(XMLPROPERTIESN) \
        $(XMLXULRESN)	\
        $(ALL_JAVA_TARGETS) \
        $(OBJTARGET)	$(SLOTARGET)	$(SMRSLOTARGET)		\
        $(SVXLIGHTSLOTARGET) \
        $(SVXLIGHTOBJTARGET) \
        $($(SECOND_BUILD)SLOTARGET) \
        $($(SECOND_BUILD)OBJTARGET) \
        $(LIB1TARGET)	$(LIB2TARGET)	$(LIB3TARGET)		\
        $(LIB4TARGET)	$(LIB5TARGET)	$(LIB6TARGET)		\
        $(LIB7TARGET)	$(LIB8TARGET)	$(LIB9TARGET)		\
        $(LIB1ARCHIV)	$(LIB2ARCHIV)	$(LIB3ARCHIV)		\
        $(LIB4ARCHIV)	$(LIB5ARCHIV)	$(LIB6ARCHIV)		\
        $(LIB7ARCHIV)	$(LIB8ARCHIV)	$(LIB9ARCHIV)		\
        $(DEF1TARGETN)	$(DEF2TARGETN)	$(DEF3TARGETN)		\
        $(DEF4TARGETN)	$(DEF5TARGETN)	$(DEF6TARGETN)		\
        $(RCTARGET) \
        $(SHL1TARGETN) \
        $(SHL2TARGETN) \
        $(SHL3TARGETN) \
        $(SHL4TARGETN) \
        $(SHL5TARGETN) \
        $(SHL6TARGETN) \
        $(SHL7TARGETN) \
        $(SHL8TARGETN) \
        $(SHL9TARGETN) \
        $(SHL1IMPLIBN) \
        $(SHL2IMPLIBN) \
        $(SHL3IMPLIBN) \
        $(SHL4IMPLIBN) \
        $(SHL5IMPLIBN) \
        $(SHL6IMPLIBN) \
        $(SHL7IMPLIBN) \
        $(SHL8IMPLIBN) \
        $(SHL9IMPLIBN) \
        $(SCP_PRODUCT_TYPE) \
        $(ALLPARFILES) \
        $(SCP1TARGETN) \
        $(SCP2TARGETN) \
        $(SCP3TARGETN) \
        $(SCP4TARGETN) \
        $(SCP5TARGETN) \
        $(SCP6TARGETN) \
        $(SCP7TARGETN) \
        $(SCP8TARGETN) \
        $(SCP9TARGETN) \
        $(APP1TARGETN)	$(APP2TARGETN)	$(APP3TARGETN)		\
        $(APP4TARGETN)	$(APP5TARGETN)	$(APP6TARGETN)		\
        $(APP7TARGETN)	$(APP8TARGETN)	$(APP9TARGETN)		\
        $(JARTARGETN)	\
        $(JARTARGETDEPN)	\
        $(SRC1TARGET)	\
        $(SRC2TARGET)	\
        $(SRC3TARGET)	\
        $(SRC4TARGET)	$(SRC5TARGET)	$(SRC6TARGET)		\
        $(SRC7TARGET)	$(SRC8TARGET)	$(SRC9TARGET)		\
        $(SRC10TARGET)	$(SRC11TARGET)	$(SRC12TARGET)		\
        $(SRC13TARGET)	$(SRC14TARGET)	$(SRC15TARGET)		\
        $(SRC16TARGET) \
        $(RSC_MULTI1) \
        $(RSC_MULTI2) \
        $(RSC_MULTI3) \
        $(RSC_MULTI4) \
        $(RSC_MULTI5) \
        $(RSC_MULTI6) \
        $(RSC_MULTI7) \
        $(RSC_MULTI8) \
        $(RSC_MULTI9) \
        $(INDPRESLIB1TARGETN) \
        $(RESLIB1TARGETN) $(RESLIB2TARGETN) \
        $(RESLIB3TARGETN) $(RESLIB4TARGETN) \
        $(RESLIB5TARGETN) $(RESLIB6TARGETN) \
        $(RESLIB7TARGETN) $(RESLIB8TARGETN) \
        $(RESLIB9TARGETN) \
        $(RESLIBSPLIT1TARGETN) $(RESLIBSPLIT2TARGETN)\
        $(RESLIBSPLIT3TARGETN) $(RESLIBSPLIT4TARGETN)\
        $(RESLIBSPLIT5TARGETN) $(RESLIBSPLIT6TARGETN)\
        $(RESLIBSPLIT7TARGETN) \
        $(COMMONPRJHIDOTHERTARGET) \
        $(PRJHIDTARGET) \
        $(PRJHID2TARGET) \
                $(SIGNFORNETSCAPE) \
                $(SIGNFOREXPLORER) \
                $(SIGNFORJARSIGNER) \
        $(CONVERTUNIXTEXT) \
        $(REMOTE_BUILD)\
        $(LOCALIZE_ME_DEST)\
        last_target

.IF "$(EXCEPTIONSNOOPT_FLAG)"==""
TARGETDEPS+=$(EXCEPTIONSNOOPTTARGET)
.ENDIF

.IF "$(NOOPT_FLAG)"==""
TARGETDEPS+=$(NOOPTTARGET)
.ENDIF

.IF "$(EXCEPTIONS_FLAG)"==""
TARGETDEPS+=$(EXCEPTIONSTARGET)
.ENDIF

#don't override .TARGETS when called with targets
.IF "$(MAKETARGETS)$(TNR)$(EXCEPTIONSNOOPT_FLAG)$(EXCEPTIONS_FLAG)$(NOOPT_FLAG)"==""
.IF "$(TARGETDEPS)"!=""
#.TARGETS .SEQUENTIAL :- $(TARGETDEPS) ALLTAR
.INIT .SEQUENTIAL : $(TARGETDEPS) $(NULLPRQ)
.ENDIF
.ENDIF

# -Gc breaks the dependency chain and causes indefinite nummbers of $(CPPUMAKER)
CPPUMAKERFLAGS*=-L

.IF "$(UNOTYPES)" != ""
.IF "$(SINGLE_SHOT)"==""
# makeing all in one
.DIRCACHE=no
.IF "$(ENVINCPRE))"!=""
MKDEPFLAGS+=-I:$(ENVINCPRE)
.ENDIF			# "$(ENVINCPRE))"!=""
.IF "$(OBJFILES)"!=""
$(OBJFILES) : $(UNOUCRHEADER)
.ENDIF			# "$(OBJFILES)"!=""
.IF "$(SLOFILES)"!=""
$(SLOFILES) : $(UNOUCRHEADER)
.ENDIF			# "$(SLOFILES)"!=""
.IF "$(DEPOBJFILES)"!=""
$(DEPOBJFILES) : $(UNOUCRHEADER)
.ENDIF			# "$(SLOFILES)"!=""
$(UNOUCRTARGET) : $(UNOUCRDEP)
.IF "$(XML2MK_FILES)"!=""
    @+-$(RM) $(foreach,i,$(XML2MK_FILES) $(MISC)$/$(i).mk) >& $(NULLDEV)
.ENDIF			# "$(XML2MK_FILES)"!=""
    +$(CPPUMAKER) @$(mktmp $(CPPUMAKERFLAGS) -B$(UNOUCRBASE) -O$(UNOUCROUT) $(UNOTYPES:^"-T")  $(UNOUCRRDB))
.ENDIF			# "$(SINGLE_SHOT)" == ""
.ENDIF			# "$(UNOTYPES)" != ""

.ENDIF			# "$(L10N_framework)"!=""
.ELSE			# "$(lintit)"==""

ALLTAR: $(OBJFILES) $(SLOFILES)

.ENDIF			# "$(lintit)"==""

.IF "$(SDINAME)"!=""
$(OBJ)$/$(CINTERNAME).obj : $(SDITARGET)
.ENDIF

.IF "$(SDI1NAME)"!=""
$(OBJ)$/$(CINTER1NAME).obj : $(SDI1TARGET)
.ENDIF

.IF "$(SDI2NAME)"!=""
$(OBJ)$/$(CINTER2NAME).obj : $(SDI2TARGET)
.ENDIF

.IF "$(SDI3NAME)"!=""
$(OBJ)$/$(CINTER3NAME).obj : $(SDI3TARGET)
.ENDIF

.IF "$(SDI4NAME)"!=""
$(OBJ)$/$(CINTER4NAME).obj : $(SDI4TARGET)
.ENDIF

.IF "$(SDI5NAME)"!=""
$(OBJ)$/$(CINTER5NAME).obj : $(SDI5TARGET)
.ENDIF

.IF "$(SOLAR_JAVA)"!=""
.IF "$(GENJAVAFILES)"!=""
$(GENJAVAFILES) : $(RDB)
$(JAVATARGET) : $(GENJAVAFILES)
.ENDIF			# "$(GENJAVAFILES)"!=""
.ENDIF          # "$(SOLAR_JAVA)"!=""

.INCLUDE : tg_dir.mk

.IF "$(UNIXTEXT)"!=""
$(UNIXTEXT) : $(UNIXTEXT:f)
    @+echo Making $@
    @+-$(RM) -f $@ >& $(NULLDEV)
    @+tr -d "\015" < $(@:f) > $@

.ENDIF			# "$(UNIXTEXT)"!=""

.IF "$(GUI)$(UPDATER)"=="WNTYES"
make_uno_doc:
#		+-$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Bdoc -P..$/$(PRJNAME)$/$(IDLPACKAGE) -OH$(PRJ)$/..$/unodoc $(DOCIDLFILES)
.ENDIF

makedoc:
        @+-mkdir $(OUT)$/ucrdoc >& $(NULLDEV)
        +$(IDLC) @$(mktmp $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -C -O$(OUT)$/ucrdoc$/$(IDLPACKAGE) $(DEPIDLFILES:+"\n"))		
#		+-$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Bdoc -P..$/$(PRJNAME)$/$(IDLPACKAGE) -OH$(PRJ)$/..$/unodoc $(DOCIDLFILES) $(IDLFILES)

.IF "$(LOCALDBTARGET)"!=""
$(LOCALDBTARGET) : $(URDFILES)
    +-$(RM) $@
    +$(REGMERGE) $@ UCR @$(mktmp $(URDFILES))
.ENDIF

.IF "$(LOCALDOCDBTARGET)"!=""
$(LOCALDOCDBTARGET) : $(URDDOCFILES)
    +-$(RM) $@
    +$(REGMERGE) $@ UCR @$(mktmp $(URDDOCFILES))
.ENDIF

.IF "$(UNOIDLDBTARGET)"!=""
$(UNOIDLDBTARGET) : $(UNOIDLDBFILES) $(UNOIDLDBREGS)
    +-$(RM) $@
    +$(REGMERGE) $@ / @$(mktmp $(UNOIDLDBFILES) $(UNOIDLDBREGS))
.IF "$(LOCALREGDB)"!=""
    +$(REGMERGE) $(LOCALREGDB) / $@
.ENDIF
.ENDIF			# "$(UNOIDLDBTARGET)"!=""

.IF "$(UNOIDLDBDOCTARGET)"!=""
$(UNOIDLDBDOCTARGET) : $(UNOIDLDBDOCFILES) $(UNOIDLDBDOCREGS)
    +-$(RM) $@
    +$(REGMERGE) $@ / @$(mktmp $(UNOIDLDBDOCFILES) $(UNOIDLDBDOCREGS))
.IF "$(LOCALREGDB)"!=""
    +$(REGMERGE) $(LOCALREGDB) / $@
.ENDIF
.ENDIF			# "$(UNOIDLDBDOCTARGET)"!=""

.IF "$(SCP_PRODUCT_TYPE)"!=""
$(SCP_PRODUCT_TYPE):
    @+-$(MKDIRHIER) $(PAR)$/$@ >& $(NULLDEV)
    @+-$(MKDIRHIER) $(BIN)$/$@ >& $(NULLDEV)

.ENDIF			# "$(PARFILES)"!=""

"$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/minormkchanged.flg" :
    +$(TOUCH) $@

.IF "$(COMPVERMK)"!=""
.IF "$(UPDATER)"!=""
COMPVTMP:=$(mktmp iii)
"$(COMPVERMK)" : $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/minormkchanged.flg
.IF "$(CCNUMVER)"!=""
    @echo COMNAME:=$(COMNAME) > $(COMPVTMP)
    @echo COMID:=$(COMID) >> $(COMPVTMP)
.IF "$(COM)"=="GCC"
    @echo SHORTSTDCPP3:=$(SHORTSTDCPP3) >> $(COMPVTMP)
.ENDIF
    @echo CCNUMVER:=$(CCNUMVER) >> $(COMPVTMP)
    @echo CDEFS+=-DCPPU_ENV=$(COMNAME) >> $(COMPVTMP)
    @+-$(RM) $@ >& $(NULLDEV)
    @+-$(RENAME) $(COMPVTMP) $@
.ELSE           # "$(CCNUMVER)"!=""
    @+-$(RM) $@ >& $(NULLDEV)
.ENDIF          # "$(CCNUMVER)"!=""
    
.ENDIF			# "$(COMPVERMK)"!=""
.ENDIF			# "$(UPDATER)"!=""

.IF "$(JAVAVERMK)"!=""
.IF "$(JAVALOCATION)"!="$(JAVA_HOME)"
"$(JAVAVERMK)" .PHONY :
.ELSE          # "$(JAVALOCATION)"!="$(JAVA_HOME)"
"$(JAVAVERMK)" : $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/minormkchanged.flg
.ENDIF          # "$(JAVALOCATION)"!="$(JAVA_HOME)"
    @+-$(RM) $@
    @echo JAVAVER:=$(JAVAVER) > $@
    @echo JAVANUMVER:=$(JAVANUMVER) >> $@
    @echo JAVALOCATION:=$(JAVA_HOME) >> $@
    
.ENDIF			# "$(JAVAVERMK)"!=""

# on recursive call there seems to be one blank in TARGETDEP
# which makes it not empty :-(
.IF "$(TARGETDEPS:s/ //)"!=""
$(TARGETDEPS) : $(LOCALIZE_ME_DEST)
.ENDIF          # "$(TARGETDEPS)"!=""

.IF "$(LOCALIZE_ME_DEST)"!=""
$(LOCALIZE_ME_DEST) : $(LOCALIZE_ME) localize.sdf 
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:b:+"_tmpl")$(@:e) -o $(@:d)$/$(@:b:+"_tmpl")$(@:e).$(INPATH) -m localize.sdf -l all
    +$(RENAME) $(@:d)$/$(@:b:+"_tmpl")$(@:e).$(INPATH) $@

.ENDIF          # "$(LOCALIZE_ME_DEST)"!=""

.IF "$(XMLPROPERTIES)"!=""
.IF "$(L10N_framework)"!=""
XML_ISO_CODE*=-ISO99 $(L10N_framework)
.ENDIF
$(COMMONMISC)$/$(TARGET)_%.done : $(COMMONMISC)$/$(TARGET)$/%.xrb
    @+-$(RM) $(COMMONMISC)$/$(<:b).interm$(TARGET) >& $(NULLDEV)
    +native2ascii -encoding UTF8 $< $(COMMONMISC)$/$(<:b).interm$(TARGET) && xmlex -i $(COMMONMISC)$/$(<:b).interm$(TARGET) -o $(CLASSDIR) $(XML_ISO_CODE) -g -d $@
    @+$(RM)  $(COMMONMISC)$/$(<:b).interm$(TARGET) >& $(NULLDEV)
.ENDIF			# "$(XMLPROPERTIES)"!=""

.IF "$(XMLXULRES)"!=""
.IF "$(L10N_framework)"!=""
XML_ISO_CODE*=-ISO99 $(L10N_framework)
.ENDIF
$(MISC)$/$(TARGET)_xxl_%.done : %.xxl
    xmlex -i $(<:b).xxl -o $(OUT)$/xul$/locale $(XML_ISO_CODE) -g:dtd -d $@
.ENDIF			# "$(XMLXULRES)"!=""

.INCLUDE : tg_sdi.mk

.IF "$(DEF1NAME)$(DEF2NAME)$(DEF3NAME)$(DEF4NAME)$(DEF5NAME)$(DEF6NAME)$(DEF7NAME)$(DEF8NAME)$(DEF9NAME)"!=""
.IF "$(MK_UNROLL)"!=""
.INCLUDE : _tg_def.mk
.ELSE
.INCLUDE : tg_def.mk
.ENDIF
.ENDIF

.IF "$(COMMONPRJHIDOTHERTARGET)"!=""
$(COMMONPRJHIDOTHERTARGET) : $(PRJHIDOTHERTARGET)
        @echo ------------------------------
        @echo Making: $@
        @+if exist $@ rm $@
        +$(TYPE) $(PRJHIDOTHERTARGET) > $@.$(ROUT).tmp 
        @+$(RENAME) $@.$(ROUT).tmp $@
.ENDIF	    

# -------
# - LIB -
# -------

.INCLUDE : tg_obj.mk

# -------
# - SLB -
# -------

.INCLUDE : tg_slo.mk

# --------
# - LIBS -
# --------

.IF "$(MK_UNROLL)"!=""
.INCLUDE : _tg_lib.mk
.ELSE
.INCLUDE : tg_lib.mk
.ENDIF

#.INCLUDE : tg_lib.mk

# -------
# - SRS -
# -------

.IF "$(SRS1NAME)$(SRS2NAME)$(SRS3NAME)$(SRS4NAME)$(SRS5NAME)$(SRS6NAME)$(SRS7NAME)$(SRS8NAME)$(SRS9NAME)"!=""
.IF "$(MK_UNROLL)"!=""
.INCLUDE : _tg_srs.mk
.ELSE
.INCLUDE : tg_srs.mk
.ENDIF
.ENDIF          # "$(SRS1NAME)$(SRS2NAME)$(SRS3NAME)$(SRS4NAME)$(SRS5NAME)$(SRS6NAME)$(SRS7NAME)$(SRS8NAME)$(SRS9NAME)"!=""

# -------
# - RES -
# -------

.IF "$(MK_UNROLL)"!=""
.INCLUDE : tg_res.mk
.ELSE
.INCLUDE : tg_res.mk
.ENDIF

# -------
# - APP -
# -------

#test:
#	echo test

.IF "$(APP1TARGETN)$(APP2TARGETN)$(APP3TARGETN)$(APP4TARGETN)$(APP5TARGETN)$(APP6TARGETN)$(APP7TARGETN)$(APP8TARGETN)$(APP9TARGETN)"!=""
.IF "$(MK_UNROLL)"!=""
.INCLUDE : _tg_app.mk
.ELSE
.INCLUDE : tg_app.mk
.ENDIF
.ENDIF

# -------
# - SCP -
# -------

.IF "$(SCP1TARGETN)$(SCP2TARGETN)$(SCP3TARGETN)$(SCP4TARGETN)$(SCP5TARGETN)$(SCP6TARGETN)$(SCP7TARGETN)$(SCP8TARGETN)$(SCP9TARGETN)"!=""

.IF "$(MK_UNROLL)"!=""
.INCLUDE : _tg_scp.mk
.ELSE
.INCLUDE : tg_scp.mk
.ENDIF
.ENDIF

# -------
# - ZIP -
# -------

.IF "$(ZIP1TARGET)$(ZIP2TARGET)$(ZIP3TARGET)$(ZIP4TARGET)$(ZIP5TARGET)$(ZIP6TARGET)$(ZIP7TARGET)$(ZIP8TARGET)$(ZIP9TARGET)"!=""
.IF "$(MK_UNROLL)"!=""
.INCLUDE : _tg_zip.mk
.ELSE
.INCLUDE : tg_zip.mk
.ENDIF
.ENDIF

# -------
# - RESLIBTARGET -
# -------

.IF "$(RESLIB1TARGETN)$(RESLIB2TARGETN)$(RESLIB3TARGETN)$(RESLIB4TARGETN)$(RESLIB5TARGETN)$(RESLIB6TARGETN)$(RESLIB7TARGETN)$(RESLIB8TARGETN)$(RESLIB9TARGETN)"!=""
.IF "$(MK_UNROLL)"!=""
.INCLUDE : _tg_rslb.mk
.ELSE
.INCLUDE : tg_rslb.mk
.ENDIF
.ENDIF

# -------
# - SHL -
# -------

.IF "$(MK_UNROLL)"!=""
.INCLUDE : _tg_shl.mk
.ELSE
.INCLUDE : tg_shl.mk
.ENDIF

.IF "$(USE_VERSIONH)"!=""
.INIT .SEQUENTIAL : $(USE_VERSIONH) $(NULLPRQ)
.ENDIF          # "$(USE_VERSIONH)"!=""

# -------
# - HXX -
# -------

.INCLUDE : tg_hxx.mk


# ------------------
# - INCLUDE DEPEND -
# ------------------

.IF "$(MAKEFILERC)"==""
.IF "$(CXXFILES)$(CFILES)$(RCFILES)$(SLOFILES)$(OBJFILES)$(DEPOBJFILES)$(PARFILES)" != ""
.IF "$(DEPFILES)" != ""
.INCLUDE : $(DEPFILES)
.ENDIF			# "$(DEPFILES)" != ""
.INCLUDE : $(MISC)$/$(TARGET).dpc
.ENDIF
.ELSE		# MAKEFILERC
.ENDIF		# MAKEFILERC

.IF "$(make_srs_deps)"==""
.IF "$(SRCFILES)" != "" || "$(SRC1FILES)" != "" || "$(SRC2FILES)" != ""
.IF "$(nodep)"==""
.INCLUDE : $(MISC)$/$(TARGET).dpr
.ENDIF			# "$(nodep)"==""
.ENDIF
.ENDIF

.IF "$(make_zip_deps)"==""
.IF "$(ZIP1TARGET)" != "" || "$(ZIP2TARGET)" != "" || "$(ZIP3TARGET)" != ""
.IF "$(nodep)"==""
.INCLUDE : $(MISC)$/$(TARGET).dpz
missing_zipdep_langs=$(alllangiso)
some_dummy_var:=$(foreach,i,$(zipdep_langs) $(assign missing_zipdep_langs:=$(strip $(subst,$i, $(missing_zipdep_langs)))))
.IF "$(missing_zipdep_langs)"!=""
ZIPDEPPHONY=.PHONY
.ENDIF			# "$(missing_zipdep_langs)"!=""
.ENDIF			# "$(nodep)"==""
.ENDIF
.ENDIF

.IF "$(LAZY_DEPS)"!=""
LAZY_DEPS_WARNING=warn_lazy_deps
.ENDIF			# "$(LAZY_DEPS)"!=""

last_target: $(LAZY_DEPS_WARNING)
    @+echo -------------

$(MISC)$/$(TARGET)genjava.mk: 	$(IDLFILES)

.IF "$(IDLFILES)"!=""
.IF "$(URD)"!=""

$(URDTARGET) : $(DEPIDLFILES)
.IF "$(MAXPROCESS)"<="1"
        +$(IDLC) @$(mktmp $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -O$(OUT)$/ucr$/$(IDLPACKAGE) $(DEPIDLFILES:+"\n"))
#		+$(UNOIDL) @$(mktmp -Wb,c $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Burd -OH$(OUT)$/ucr$/$(IDLPACKAGE) $(DEPIDLFILES:+"\n"))
.ENDIF			# "$(MAXPROCESS)"<="1"
        @+echo > $@

.IF "$(URDDOC)"!=""

$(URDDOCTARGET) : $(DEPIDLFILES)
        @+-mkdir $(OUT)$/ucrdoc >& $(NULLDEV)
.IF "$(MAXPROCESS)"<="1"
        +$(IDLC) @$(mktmp $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -C -O$(OUT)$/ucrdoc$/$(IDLPACKAGE) $(DEPIDLFILES:+"\n"))		
#		+$(UNOIDL) @$(mktmp $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Burd -OH$(OUT)$/ucrdoc$/$(IDLPACKAGE) $(DEPIDLFILES:+"\n"))
.ENDIF			# "$(MAXPROCESS)"<="1"
        @+echo > $@
.ENDIF			# "$(URDDOC)"!=""
.ENDIF			# "$(URD)"!=""
.ENDIF			# "$(IDLFILES)"!=""

.IF "$(JAVACLASSFILES:s/DEFINED//)"!=""
.IF "$(L10N_framework)"==""
.INCLUDE .IGNORE : $(MISC)$/$(TARGET).dpj
$(TARGETDPJ) : $(JAVAFILES) $(JAVATARGET)
.ENDIF			# "$(L10N_framework)"==""
.ENDIF

.INCLUDE : tg_jar.mk


# ----------------------------------
# auto update of local environment
# ----------------------------------

.IF "$(SOLARVERSION)"!="$(SHARED_SOLARVERSION)"
.INCLUDE .IGNORE : $(SOLAR_ENV_ROOT)$/etools$/env_up.mk
.ENDIF			# "$(SOLARVERSION)"!="$(SHARED_SOLARVERSION)"

# ----------------------------------
# - NOOPT - files ohne optimierung -
# ----------------------------------

.IF "$(NOOPTTARGET)" != ""
.IF "$(NOOPT_FLAG)" == ""

#$(SLOFILES) $(OBJFILES) $(IDLSLOFILES) $(IDLOBJFILES) $(S2USLOFILES) $(SMRSLOFILES) $(SVXLIGHTSLOFILES) $(SVXLIGHTOBJFILES) $($(SECOND_BUILD)_SLOFILES) $($(SECOND_BUILD)_OBJFILES) : $(NOOPTTARGET)

$(NOOPTTARGET):
    @+echo --- NOOPTFILES ---
    @dmake $(MFLAGS) $(MAKEFILE) nopt=true $(NOOPTFILES) NOOPT_FLAG=TRUE $(CALLMACROS)
    @+echo --- NOOPTFILES OVER ---

$(NOOPTFILES):
    @+echo --- NOOPT ---
    @dmake $(MFLAGS) $(MAKEFILE) nopt=true NOOPT_FLAG=TRUE $(CALLMACROS) $@
    @+echo --- NOOPT OVER ---
.ENDIF
.ENDIF


# ----------------------------------
# - EXCEPTIONS - files mit exceptions -
# ----------------------------------

.IF "$(EXCEPTIONSTARGET)" != ""
.IF "$(EXCEPTIONS_FLAG)" == ""

#$(SLOFILES) $(OBJFILES) $(IDLSLOFILES) $(IDLOBJFILES) $(S2USLOFILES) $(SMRSLOFILES) $(SVXLIGHTSLOFILES) $(SVXLIGHTOBJFILES) $($(SECOND_BUILD)_SLOFILES) $($(SECOND_BUILD)_OBJFILES) : $(EXCEPTIONSTARGET)

$(EXCEPTIONSTARGET):
    @+echo --- EXCEPTIONSFILES ---
    @dmake $(MFLAGS) $(MAKEFILE) ENABLE_EXCEPTIONS=true $(EXCEPTIONSFILES) EXCEPTIONS_FLAG=TRUE $(CALLMACROS)
    @+echo --- EXCEPTIONSFILES OVER ---

$(EXCEPTIONSFILES):
    @+echo --- EXCEPTIONS ---
    @dmake $(MFLAGS) $(MAKEFILE) ENABLE_EXCEPTIONS=true EXCEPTIONS_FLAG=TRUE $(CALLMACROS) $@
    @+echo --- EXCEPTIONS OVER ---


.ENDIF
.ENDIF

# ----------------------------------
# - EXCEPTIONSNOOPT - files with exceptions, without optimisation -
# ----------------------------------

.IF "$(EXCEPTIONSNOOPTTARGET)" != ""
.IF "$(EXCEPTIONSNOOPT_FLAG)" == ""

#$(SLOFILES) $(OBJFILES) $(IDLSLOFILES) $(IDLOBJFILES) $(S2USLOFILES) $(SMRSLOFILES) $(SVXLIGHTSLOFILES) $(SVXLIGHTOBJFILES) $($(SECOND_BUILD)_SLOFILES) $($(SECOND_BUILD)_OBJFILES) : $(EXCEPTIONSTARGET)

$(EXCEPTIONSNOOPTTARGET):
    @+echo --- EXCEPTIONSNOOPTFILES ---
    @dmake $(MFLAGS) $(MAKEFILE) ENABLE_EXCEPTIONS=true $(EXCEPTIONSNOOPTFILES) EXCEPTIONSNOOPT_FLAG=TRUE nopt=true $(CALLMACROS)
    @+echo --- EXCEPTIONSNOOPTFILES OVER ---

$(EXCEPTIONSNOOPTFILES):
    @+echo --- EXCEPTIONSNOOPT ---
    @dmake $(MFLAGS) $(MAKEFILE) ENABLE_EXCEPTIONS=true EXCEPTIONSNOOPT_FLAG=TRUE nopt=true $(CALLMACROS) $@
    @+echo --- EXCEPTIONSNOOPT OVER ---


.ENDIF
.ENDIF

# ----------------------------------
# - REMOTE_BUILD - build remote vcl -
# ----------------------------------

.IF "$(REMOTE_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(PRJNAME)"=="vcl"
.IF "$(REMOTE_BUILD_FLAG)" == ""
.IF "$(remote)" == ""
$(REMOTE_BUILD):
    @+echo --- REMOTE_BUILD ---
    @dmake $(MFLAGS) $(MAKEFILE) remote=true REMOTE_BUILD_FLAG=TRUE $(CALLMACROS)
    @+echo --- REMOTE_BUILD OVER ---
.ENDIF          # "$(remote)" == ""
.ENDIF          # "$(REMOTE_BUILD_FLAG)" == ""
.ENDIF          # "$(PRJNAME)"=="vcl"
.ENDIF          # "$(UPDATER)"!=""
.ENDIF			# "$(REMOTE_BUILD)"!=""

.IF "$(LAZY_DEPS)"!=""
warn_lazy_deps:
    @+echo -
    @+echo -----------------------------------------------
    @+echo -
    @+echo - You have used LAZY_DEPS. Your dependencies
    @+echo - will not be updated anymore!
    @+echo -
    @+echo -----------------------------------------------
.ENDIF			# "$(LAZY_DEPS)"!=""

# ----------------------------------
# - OTHER - alles wofuer rules da sind -
# ----------------------------------

OTHERTARGET : $(OTHER)

# -------------------------
# - diverse kill commands -
# -------------------------

"$(TMP)$/makedt.don":
    @$(TOUCH) $(TMP)$/makedt.don

do_copy_mk .IGNORE .SILENT :
    @+-$(MKDIR) $(OS2_SOLENV_INC) >& $(NULLDEV)
    @+-$(MKDIR) $(OS2_SOLENV_INC)$/startup >& $(NULLDEV)
    @+-$(MKDIR) $(OS2_SOLENV_INC)$/startup$/os2 >& $(NULLDEV)
    @+-attrib -rs $(OS2_SOLENV_INC)$/*.mk >& $(NULLDEV)
    @+-$(COPY) /u $(SOLARENV)$/inc$/*.mk $(OS2_SOLENV_INC) >& $(NULLDEV)
    @+-$(COPY) /u $(SOLARENV)$/inc$/startup$/*.mk $(OS2_SOLENV_INC)$/startup >& $(NULLDEV)
    @+-$(COPY) /u $(SOLARENV)$/inc$/startup$/os2$/*.mk $(OS2_SOLENV_INC)$/startup$/os2 >& $(NULLDEV)

killbin:
.IF "$(OS)$(USE_SHELL)"=="WNT4nt"
    @+if exist $(BIN)\$(SHL1TARGET).dll @del $(BIN)\$(SHL1TARGET).dll
    @+if exist $(BIN)\$(SHL2TARGET).dll @del $(BIN)\$(SHL2TARGET).dll
    @+if exist $(BIN)\$(SHL3TARGET).dll @del $(BIN)\$(SHL3TARGET).dll
    @+if exist $(BIN)\$(SHL4TARGET).dll @del $(BIN)\$(SHL4TARGET).dll
    @+if exist $(BIN)\$(SHL5TARGET).dll @del $(BIN)\$(SHL5TARGET).dll
    @+if exist $(BIN)\$(SHL6TARGET).dll @del $(BIN)\$(SHL6TARGET).dll
    @+if exist $(BIN)\$(SHL7TARGET).dll @del $(BIN)\$(SHL7TARGET).dll
    @+if exist $(BIN)\$(SHL8TARGET).dll @del $(BIN)\$(SHL8TARGET).dll
    @+if exist $(BIN)\$(SHL9TARGET).dll @del $(BIN)\$(SHL9TARGET).dll
    @+if exist $(BIN)\$(APP1TARGET)$(EXECPOST) @del $(BIN)\$(APP1TARGET)$(EXECPOST)
    @+if exist $(BIN)\$(APP2TARGET)$(EXECPOST) @del $(BIN)\$(APP2TARGET)$(EXECPOST)
    @+if exist $(BIN)\$(APP3TARGET)$(EXECPOST) @del $(BIN)\$(APP3TARGET)$(EXECPOST)
    @+if exist $(BIN)\$(APP4TARGET)$(EXECPOST) @del $(BIN)\$(APP4TARGET)$(EXECPOST)
    @+if exist $(BIN)\$(APP5TARGET)$(EXECPOST) @del $(BIN)\$(APP5TARGET)$(EXECPOST)
    @+if exist $(BIN)\$(APP6TARGET)$(EXECPOST) @del $(BIN)\$(APP6TARGET)$(EXECPOST)
    @+if exist $(BIN)\$(APP7TARGET)$(EXECPOST) @del $(BIN)\$(APP7TARGET)$(EXECPOST)
    @+if exist $(BIN)\$(APP8TARGET)$(EXECPOST) @del $(BIN)\$(APP8TARGET)$(EXECPOST)
    @+if exist $(BIN)\$(APP9TARGET)$(EXECPOST) @del $(BIN)\$(APP9TARGET)$(EXECPOST)
    
.ELSE			# "$(GUI)"=="WNT"
.IF "$(SHL1TARGET)"!=""
    @+-$(RM) $(LB)/$(DLLPRE)$(SHL1TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL2TARGET)"!=""
    @+-$(RM) $(LB)/$(DLLPRE)$(SHL2TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL3TARGET)"!=""
    @+-$(RM) $(LB)/$(DLLPRE)$(SHL3TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL4TARGET)"!=""
    @+-$(RM) $(LB)/$(DLLPRE)$(SHL4TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL5TARGET)"!=""
    @+-$(RM) $(LB)/$(DLLPRE)$(SHL5TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL6TARGET)"!=""
    @+-$(RM) $(LB)/$(DLLPRE)$(SHL6TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL7TARGET)"!=""
    @+-$(RM) $(LB)/$(DLLPRE)$(SHL7TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL8TARGET)"!=""
    @+-$(RM) $(LB)/$(DLLPRE)$(SHL8TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL9TARGET)"!=""
    @+-$(RM) $(LB)/$(DLLPRE)$(SHL9TARGET)$(DLLPOST)
.ENDIF
.IF "$(APP1TARGET)"!=""
    @+-$(RM) $(BIN)/$(APP1TARGET)$(EXECPOST)
.ENDIF
.IF "$(APP2TARGET)"!=""
    @+-$(RM) $(BIN)/$(APP2TARGET)$(EXECPOST)
.ENDIF
.IF "$(APP3TARGET)"!=""
    @+-$(RM) $(BIN)/$(APP3TARGET)$(EXECPOST)
.ENDIF
.IF "$(APP4TARGET)"!=""
    @+-$(RM) $(BIN)/$(APP4TARGET)$(EXECPOST)
.ENDIF
.IF "$(APP5TARGET)"!=""
    @+-$(RM) $(BIN)/$(APP5TARGET)$(EXECPOST)
.ENDIF
.IF "$(APP6TARGET)"!=""
    @+-$(RM) $(BIN)/$(APP6TARGET)$(EXECPOST)
.ENDIF
.IF "$(APP7TARGET)"!=""
    @+-$(RM) $(BIN)/$(APP7TARGET)$(EXECPOST)
.ENDIF
.IF "$(APP8TARGET)"!=""
    @+-$(RM) $(BIN)/$(APP8TARGET)$(EXECPOST)
.ENDIF
.IF "$(APP9TARGET)"!=""
    @+-$(RM) $(BIN)/$(APP9TARGET)$(EXECPOST)
.ENDIF
.ENDIF			# "$(GUI)"=="WNT"

killobj:
.IF "$(SLOFILES)" != ""
    +-cd $(SLO) && $(TYPE) $(mktmp  $(SLOFILES:f)) | xargs -n 20 rm 
    +-cd $(SLO) && $(TYPE) $(mktmp  $(SLOFILES:s/.obj/.o/:f)) | xargs -n 20 rm
.ENDIF
.IF "$(OBJFILES)" != ""
    +-cd $(OBJ) && $(TYPE) $(mktmp  $(OBJFILES:f)) | xargs -n 20 rm 
    +-cd $(OBJ) && $(TYPE) $(mktmp  $(OBJFILES:s/.obj/.o/:f)) | xargs -n 20 rm
.ENDIF
.IF "$(SVXLIGHT)"!=""
.IF "$(REAL_SVXLIGHTSLOFILES)" != ""
    +-cd $(REAL_SVXLIGHTSLO) && $(TYPE) $(mktmp  $(REAL_SVXLIGHTSLOFILES:f)) | xargs -n 20 rm 
    +-cd $(REAL_SVXLIGHTSLO) && $(TYPE) $(mktmp  $(REAL_SVXLIGHTSLOFILES:s/.obj/.o/:f)) | xargs -n 20 rm
.ENDIF
.IF "$(REAL_SVXLIGHTOBJFILES)" != ""
    +-cd $(REAL_SVXLIGHTOBJ) && $(TYPE) $(mktmp  $(REAL_SVXLIGHTOBJFILES:f)) | xargs -n 20 rm 
    +-cd $(REAL_SVXLIGHTOBJ) && $(TYPE) $(mktmp  $(REAL_SVXLIGHTOBJFILES:s/.obj/.o/:f)) | xargs -n 20 rm
.ENDIF
.ENDIF			# "$(SVXLIGHT)"!=""
.IF "$(REAL_$(SECOND_BUILD)_SLOFILES)" != ""
    +-cd $(REAL_$(SECOND_BUILD)_SLO) && $(TYPE) $(mktmp  $(REAL_$(SECOND_BUILD)_SLOFILES:f)) | xargs -n 20 rm
    +-cd $(REAL_$(SECOND_BUILD)_SLO) && $(TYPE) $(mktmp  $(REAL_$(SECOND_BUILD)_SLOFILES:s/.obj/.o/:f)) | xargs -n 20 rm
.ENDIF
.IF "$(REAL_$(SECOND_BUILD)_OBJFILES)" != ""
    +-cd $(REAL_$(SECOND_BUILD)_OBJ) && $(TYPE) $(mktmp  $(REAL_$(SECOND_BUILD)_OBJFILES:f)) | xargs -n 20 rm 
    +-cd $(REAL_$(SECOND_BUILD)_OBJ) && $(TYPE) $(mktmp  $(REAL_$(SECOND_BUILD)_OBJFILES:s/.obj/.o/:f)) | xargs -n 20 rm
.ENDIF
.IF "$(DEPOBJFILES)" != ""
    +-cd $(SLO) && $(TYPE) $(mktmp  $(DEPOBJFILES:f)) | xargs -n 20 rm
    +-cd $(SLO) && $(TYPE) $(mktmp  $(DEPOBJFILES:s/.obj/.o/:f)) | xargs -n 20 rm
    +-cd $(OBJ) && $(TYPE) $(mktmp  $(DEPOBJFILES:f)) | xargs -n 20 rm
    +-cd $(OBJ) && $(TYPE) $(mktmp  $(DEPOBJFILES:s/.obj/.o/:f)) | xargs -n 20 rm
.ENDIF
    @+echo objects weg!

killsrs:
# doesn't work - fix me!
.IF "$(SRSFILES)" != ""
    +$(RM) $(SRSFILES)
.ENDIF
    @+echo srsfiles weg!

killres:
.IF "$(RESLIB1TARGETN)$(RESLIB2TARGETN)$(RESLIB3TARGETN)$(RESLIB4TARGETN)$(RESLIB5TARGETN)$(RESLIB6TARGETN)$(RESLIB7TARGETN)$(RESLIB8TARGETN)$(RESLIB9TARGETN)"!=""
    +$(RM) $(RESLIB1TARGETN) $(RESLIB2TARGETN) $(RESLIB3TARGETN) $(RESLIB4TARGETN) $(RESLIB5TARGETN) $(RESLIB6TARGETN) $(RESLIB7TARGETN) $(RESLIB8TARGETN) $(RESLIB9TARGETN)
    @+echo resource files removed!
.ELSE			# "$(RESLIB1TARGETN)$(RESLIB2TARGETN)$(RESLIB3TARGETN)$(RESLIB4TARGETN)$(RESLIB5TARGETN)$(RESLIB6TARGETN)$(RESLIB7TARGETN)$(RESLIB8TARGETN)$(RESLIB9TARGETN)"!=""
    @+echo no resource files defined!
.ENDIF			# "$(RESLIB1TARGETN)$(RESLIB2TARGETN)$(RESLIB3TARGETN)$(RESLIB4TARGETN)$(RESLIB5TARGETN)$(RESLIB6TARGETN)$(RESLIB7TARGETN)$(RESLIB8TARGETN)$(RESLIB9TARGETN)"!=""

killdef:
.IF "$(DEFTARGETN)" != ""
    +$(RM) $(DEFTARGETN)
.ENDIF
    @+echo deffiles weg!

killlib:
.IF "$(LIB1TARGETN)$(LIB2TARGETN)$(LIB3TARGETN)$(LIB4TARGETN)$(LIB5TARGETN)$(LIB6TARGETN)$(LIB7TARGETN)$(LIB8TARGETN)$(LIB9TARGETN)"!=""
    +$(RM) $(LIB1TARGETN) $(LIB2TARGETN) $(LIB3TARGETN) $(LIB4TARGETN) $(LIB5TARGETN) $(LIB6TARGETN) $(LIB7TARGETN) $(LIB8TARGETN) $(LIB9TARGETN)
.IF "$(LIB1ARCHIV)$(LIB2ARCHIV)$(LIB3ARCHIV)$(LIB4ARCHIV)$(LIB5ARCHIV)$(LIB6ARCHIV)$(LIB7ARCHIV)$(LIB8ARCHIV)$(LIB9ARCHIV)"!=""
    +$(RM) $(LIB1ARCHIV) $(LIB2ARCHIV) $(LIB3ARCHIV) $(LIB4ARCHIV) $(LIB5ARCHIV) $(LIB6ARCHIV) $(LIB7ARCHIV) $(LIB8ARCHIV) $(LIB9ARCHIV)
.ENDIF			# "$(LIB1ARCHIV)$(LIB2ARCHIV)$(LIB3ARCHIV)$(LIB4ARCHIV)$(LIB5ARCHIV)$(LIB6ARCHIV)$(LIB7ARCHIV)$(LIB8ARCHIV)$(LIB9ARCHIV)"!=""
    @+echo lib/archive files removed!
.ENDIF			# "$(LIB1TARGETN)$(LIB2TARGETN)$(LIB3TARGETN)$(LIB4TARGETN)$(LIB5TARGETN)$(LIB6TARGETN)$(LIB7TARGETN)$(LIB8TARGETN)$(LIB9TARGETN)"!=""
.IF "$(SLOTARGET)$(OBJTARGET)"!=""
    +$(RM) $(SLOTARGET) $(OBJTARGET)
    @+echo default lib files removed!
.ENDIF			# "$(SLOTARGET)$(OBJTARGET)"!=""
.IF "$(SVXLIGHTSLOTARGET)$(SVXLIGHTOBJTARGET)"!=""
    +$(RM) $(SVXLIGHTSLOTARGET) $(SVXLIGHTOBJTARGET)
.ENDIF			# "$(SVXLIGHTSLOTARGET)$(SVXLIGHTOBJTARGET)"!=""
    @+echo done!

clean_misc :
.IF "$(MISC)"!=""
    +rm -rf $(MISC)$/*
    @+echo misc is gone!
.ELSE			# "$(MISC)"!=""
    @+echo can\'t be done! $$(MISC) not defined.
.ENDIF			# "$(MISC)"!=""

clean_all :
.IF "$(OUT)"!=""
.IF "$(GUI)"=="UNX"
    +test -f $(PRJ)$/prj/build.lst && rm -rf $(OUT)
.ELSE			# "$(GUI)"=="UNX"
    +if exist $(PRJ)$/prj/build.lst del /sxyz $(OUT)
.ENDIF			# "$(GUI)"=="UNX"
    @+echo local output tree is gone!
.ELSE			# "$(OUT)"!=""
    @+echo can\'t be done! $$(OUT) not defined.
.ENDIF			# "$(OUT)"!=""


SRCALLTARGET:	\
        $(SDITARGET)	$(SDI1TARGET)	$(SDI2TARGET)		\
        $(SDI3TARGET)	$(SDI4TARGET)	$(SDI5TARGET)		\
        $(SRC1TARGET)	\
        $(SRC2TARGET)	\
        $(SRC3TARGET)	$(RCTARGET)		  \
        $(SRC4TARGET)	$(SRC5TARGET)	$(SRC6TARGET)		\
        $(SRC7TARGET)	$(SRC8TARGET)	$(SRC9TARGET)		\
        $(SRC10TARGET)	$(SRC11TARGET)	$(SRC12TARGET)		\
        $(SRC13TARGET)	$(SRC14TARGET)	$(SRC15TARGET)		\
        $(SRC16TARGET)

.IF "$(ZIP1TARGETN)$(ZIP2TARGETN)$(ZIP3TARGETN)$(ZIP4TARGETN)$(ZIP5TARGETN)$(ZIP6TARGETN)$(ZIP7TARGETN)$(ZIP8TARGETN)$(ZIP9TARGETN)"!=""
ZIPALLTARGET: \
        $(ZIP1TARGETN) \
        $(ZIP2TARGETN) \
        $(ZIP3TARGETN) \
        $(ZIP4TARGETN) \
        $(ZIP5TARGETN) \
        $(ZIP6TARGETN) \
        $(ZIP7TARGETN) \
        $(ZIP8TARGETN) \
        $(ZIP9TARGETN)
.ELSE
ZIPALLTARGET:
    @+$(ECHO) ---
    @+echo nothing to zip for activated languages!
    @+$(ECHO) ---
.ENDIF


#temporary workaround for non-existing delzip in extras
delzip:
    +echo > $@

$(MISC)$/helpids.don: $(HELPIDFILES)
    @echo Making helpids:
    @echo ---------------
    @echo r:\bat\mkhids.btm $(PRJ)\$(INPATH)\misc\help.id $(HELPIDFILES)
#	rem @copy /uq r:\bat\mkhids.btm r:\bat\mkhids.bat
#	rem @r:\bat\mkhids.bat $(PRJ)\$(INPATH)\misc\help.id $(HELPIDFILES)
    @$(TOUCH) $(MISC)\helpids.don

.IF "$(depend)"==""
.IF "$(SVXLIGHT)"!=""
$(MISC)$/$(TARGET).dpc : \
    $(foreach,i,$(SVXLIGHTSLOFILES) $(i:d)sxl_$(i:f)) \
    $(foreach,i,$(SVXLIGHTOBJFILES) $(i:d)sxl_$(i:f))
.ENDIF
.IF "$(SECOND_BUILD)"!=""
$(MISC)$/$(TARGET).dpc : \
    $(foreach,i,$($(SECOND_BUILD)_SLOFILES) $(i:d)$(SECOND_BUILD)_$(i:f)) \
    $(foreach,i,$($(SECOND_BUILD)_OBJFILES) $(i:d)$(SECOND_BUILD)_$(i:f))
.ENDIF
.ENDIF			# "$(depend)"==""

.IF "$(make_srs_deps)"==""
$(MISC)$/$(TARGET).dpr : $(SRCFILES) $(SRC1FILES) $(SRC2FILES) $(SRC3FILES)
.ENDIF

.IF "$(make_zip_deps)"==""
$(MISC)$/$(TARGET).dpz $(ZIPDEPPHONY) : $(ZIP1TARGETN) $(ZIP2TARGETN) $(ZIP3TARGETN) $(ZIP4TARGETN) $(ZIP5TARGETN) $(ZIP6TARGETN) $(ZIP7TARGETN) $(ZIP8TARGETN) $(ZIP9TARGETN)
.ENDIF

VERSIONTMP:=$(mktmp iii)
$(INCCOM)$/_version.h : $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/minormkchanged.flg
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    @+echo "#define" _BUILD \"$(BUILD)\"	> $(VERSIONTMP)
    @+echo "#define" _UPD \"$(UPD)\"		>> $(VERSIONTMP)
    @+echo "#define" _LAST_MINOR \"$(LAST_MINOR)\"	>> $(VERSIONTMP)
    @+echo '#define _RSCREVISION "$(RSCREVISION)"' >> $(VERSIONTMP)
    @+echo "#define" _INPATH \"$(INPATH)\"	>> $(VERSIONTMP)
.ELSE
    @+echo #define _BUILD "$(BUILD)"	> $(VERSIONTMP)
    @+echo #define _UPD "$(UPD)"		>> $(VERSIONTMP)
    @+echo #define _LAST_MINOR "$(LAST_MINOR)"	>> $(VERSIONTMP)
    @+echo #define _DLL_POSTFIX "$(DLL_POSTFIX)">> $(VERSIONTMP)
    @+echo #define _RSCREVISION "$(RSCREVISION)">> $(VERSIONTMP)
    @+echo #define _INPATH "$(INPATH)"	>> $(VERSIONTMP)
.ENDIF
    @+-$(RM) $@ >& $(NULLDEV)
    @+-$(RENAME) $(VERSIONTMP) $@

.IF "$(MAKEFILERC)"==""
warn_target_empty:
    @+echo '*'
    @+echo '* error $$(TARGET) is empty - this will cause problems'
    @+echo '*'
    force_dmake_to_error
.ELSE
warn_target_empty:
    @+echo generated makefile.rc detected
.ENDIF

.IF "$(UNOTYPES)" != ""
UNOUCRDEPxxx : $(UNOUCRDEP);
.ENDIF			# "$(UNOTYPES)" != ""

$(MISC)$/$(PRJNAME).hid : \
        $(RESLIB1SRSFILES)  $(RESLIB2SRSFILES) \
        $(RESLIB3SRSFILES)  $(RESLIB4SRSFILES) \
        $(RESLIB5SRSFILES)  $(RESLIB6SRSFILES) \
        $(RESLIB7SRSFILES)  $(RESLIB8SRSFILES) \
        $(RESLIB9SRSFILES) \
        $(PRJHIDOTHERTARGET) $(HIDSRSPARTICLE)
    @echo Making $@ :
    @echo ---------------
    @+if exist $@ del $@
    @$(TOUCH) $@
    @+if exist $(SRS)\*.hid dir $(SRS)\*.hid
    @+if exist $(SRS)\*.hid type $(SRS)\*.hid >> $@
#   @+if exist $(MISC)\*.lst $(ENV_TOOLS)$/slothid.bat $(MISC)\*.lst $@ $(INPATH)
    @+if exist $(MISC)\*.* $(ENV_TOOLS)$/slothid.bat $(MISC)\*.lst $@ $(INPATH)
    @echo done Making $@

#new hid.lst trigger with GEN_HID2=TRUE
$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/hid.lst .PHONY :
    @echo Making $@ :
    @echo ---------------
    @+if exist $@ $(RM) $@
    @echo $(WORK_STAMP).$(LAST_MINOR) 010101010101010> $@.$(ROUT).tmp
    $(TYPE) $(SOLARCOMMONBINDIR)$/hid$/*.hid | $(SORT) -u >> $@.$(ROUT).tmp 
    @+$(RENAME) $@.$(ROUT).tmp $@


.IF "$(SOLAR_JAVA)"!=""
.INCLUDE : tg_java.mk
.ENDIF          # "$(SOLAR_JAVA)"!=""

.INCLUDE : tg_merge.mk

wordcount:
    +wc *.* >> $(TMP)$/wc.lst

testt:
    @echo test

.ELSE			# "$(depend)" == ""

# ----------
# - DEPEND -
# ----------

ALLTAR : ALLDEP \
        $(REMOTE_DEPEND) \
        $(SUBDIRS)

.INCLUDE : tg_dep.mk

# ----------------------------------
# - REMOTE_DEPEND - remote vcl dependencies -
# ----------------------------------

.IF "$(REMOTE_DEPEND)"!=""
.IF "$(UPDATER)"!=""
.IF "$(PRJNAME)"=="vcl"
.IF "$(REMOTE_BUILD_FLAG)" == ""
.IF "$(remote)" == ""
$(REMOTE_DEPEND):
    @+echo --- REMOTE_DEPEND ---
    @dmake $(MFLAGS) $(MAKEFILE) remote=true depend=t REMOTE_BUILD_FLAG=TRUE $(CALLMACROS)
    @+echo --- REMOTE_DEPEND OVER ---
.ENDIF          # "$(remote)" == ""
.ENDIF          # "$(REMOTE_BUILD_FLAG)" == ""
.ENDIF          # "$(PRJNAME)"=="vcl"
.ENDIF          # "$(UPDATER)"!=""
.ENDIF			# "$(REMOTE_DEPEND)"!=""

.ENDIF			# "$(depend)" == ""

.IF "$(SUBDIRS)"!=""

$(SUBDIRS) : $(SUBDIRSDEPS)

.IF "$(mk_tmp)$(BSCLIENT)"!=""
$(SUBDIRS)  .PHONY :
    @+echo ignoring SUBDIRS

.ELSE			# "$(mk_tmp)$(BSCLIENT)"!=""
#.IF "$(PRJNAME)"!="sw"
.IF "$(GUI)"!="UNX"
$(SUBDIRS) .PHONY :
    @[
        cd $@
        cd
        @$(MAKECMD) subdmake=true $(MFLAGS) $(CALLMACROS)
    ]
.ELSE			# "$(GUI)"!="UNX"
$(SUBDIRS) .PHONY :
    +cd $@; $(MAKECMD) subdmake=true $(MFLAGS) $(CALLMACROS)
.ENDIF			# "$(GUI)"!="UNX"
#.ENDIF
.ENDIF			# "$(mk_tmp)$(BSCLIENT)"!=""
.ENDIF			# "$(SUBDIRS)"!=""

# workaround for strange dmake bug:
# if the previous block was a rule or a target, "\#" isn't recognized
# as an escaped "#". if it was an assignment, escaping works...
some_unique_variable_name:=1

