#*************************************************************************
#
#   $RCSfile: target.mk,v $
#
#   $Revision: 1.36 $
#
#   last change: $Author: hjs $ $Date: 2001-02-12 16:17:43 $
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
INCPRE+=-I$(PRE)$/inc
.ENDIF			# "$(PRE)"!=""
.IF "$(LOCAL_SOLENV)"!=""
SOLARINC+=$(JDKINCS)
SOLARINC+=$(ORCLINC)
SOLARINC+=$(DB2INC)
SOLARINC+=$(DAOINC)
.ENDIF "$(LOCAL_SOLENV)"!=""
.IF "$(PRJINC)"!=""
INCLUDE:=-I$(INCPRE) -I. -I$(INCLOCAL) -I$(INCLOCPRJ) -I$(INC) -I$(INCGUI) -I$(INCCOM) $(SOLARINC) -I$(INCEXT) -I$(PRJ)$/res -I$(INCPOST)
.ELSE		# "$(PRJINC)"!=""
INCLUDE:=-I. -I$(INCPRE) -I$(INCLOCAL) -I$(INC) -I$(INCGUI) -I$(INCCOM) $(SOLARINC) -I$(INCEXT) -I$(PRJ)$/res -I$(INCPOST)
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
DPCTARGET=$(MISC)$/$(TARGET).dpc
.ENDIF
.IF "$(RC_SUBDIRS)"!=""
SUBDIRS:=$(RC_SUBDIRS)
SUBDIRSDEPS=$(RC_SUBDIRSDEPS)
.ENDIF

.INCLUDE : pstrules.mk

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
DEPFILESx+=$(subst,$(SLO)$/,$(MISC)$/s_ $(subst,$(OBJ)$/,$(MISC)$/o_ $(DEPOBJFILES:s/.obj/.dpcc/)))
DEPFILESx+=$(subst,$(OBJ)$/,$(MISC)$/o_ $(OBJFILES:s/.obj/.dpcc/))
DEPFILESx+=$(subst,$(SLO)$/,$(MISC)$/s_ $(SLOFILES:s/.obj/.dpcc/))
DEPFILESx+=$(subst,$(PAR),$(MISC) $(ALLPARFILES:s/.par/.dpsc/))
.IF "$(RCFILES)"!=""
.IF "$(RESNAME)"!=""
#RCTARGET!:=$(foreach,i,$(alllangext) $(RES)$/$i$/$(RESNAME).res)
DEPFILESx+=$(MISC)$/$(RESNAME).dpcc
.ELSE			# "$(RESNAME)"!=""
DEPFILESx+=$(foreach,i,$(alllangext) $(MISC)$/$i$/$(TARGET).dprc)
.ENDIF			# "$(RESNAME)"!=""
.ENDIF			# "$(RCFILES)"!=""
DEPFILES=$(uniq $(DEPFILESx))

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

.IF "$(depend)" == ""

# -------
# - ALL -
# -------

# Bei VCL werden keine Resouren an die Apps geklemmt
.IF "$(VCL)" != ""
APP1RES=
APP2RES=
APP3RES=
APP4RES=
APP5RES=
APP6RES=
APP7RES=
APP8RES=
APP9RES=
# Leider gibt es auch Applikationen die nicht VCLApp's sind, und Resourcen nicht von
# VCL beziehen
.IF "$(APP1NOSVRES)" != ""
APP1RES=$(APP1NOSVRES)
.ENDIF
.IF "$(APP2NOSVRES)" != ""
APP2RES=$(APP2NOSVRES)
.ENDIF
.IF "$(APP3NOSVRES)" != ""
APP3RES=$(APP3NOSVRES)
.ENDIF
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

.IF "$(TF_PACKAGES)"==""

.IF "$(IDLFILES)"!=""
.IF "$(GUI)"=="UNX"
MISC_:=$(MISC)/
INC_:=$(INCCOM)$/$(PRJNAME)/
SLO_:=$(SLO)/
.ELSE			# "$(GUI)"=="UNX"
MISC_:=$(MISC)$(/)$/
INC_:=$(INCCOM)$/$(PRJNAME)$(/)$/
SLO_:=$(SLO)$(/)$/
.ENDIF			# "$(GUI)"=="UNX"
.IF "$(NODEFAULTUNO)"==""
.IF "$(NOSMARTUNO)"==""
UNOIDLTARGETS+=$(IDLFILES:s/.idl/.smr/:f:^$(MISC_:s/\r/\\r/))
SMRTARGETS+=$(IDLFILES:s/.idl/.smr/:f:^$(MISC_:s/\r/\\r/))
.ENDIF			# "$(NOSMARTUNO)"==""
UNOIDLTARGETS+=$(IDLFILES:s/.idl/.cxx/:f:^$(MISC_:s/\r/\\r/))
UNOIDLTARGETS+=$(IDLFILES:s/.idl/.obj/:f:^$(SLO_:s/\r/\\r/))
SLOFILES+=$(IDLFILES:s/.idl/.obj/:f:^$(SLO_:s/\r/\\r/))
.ENDIF			# "$(NODEFAULTUNO)"==""
.ENDIF			# "$(IDLFILES)"!=""

.IF "$(s2u)"!=""
SLOPRE_:=$(SLO)$(/)s2u_
UNOIDLTARGETS+=$(MISC)$/s2u_$(TARGET).don
SLOFILES+=$(IDLFILES:s/.idl/.obj/:f:^$(SLOPRE_:s/\r/\\r/))
.ENDIF

.ELSE		# "$(TF_PACKAGES)"==""

.IF "$(BUILD_URD_ONLY)" != ""
NOURD=
URD=TRUE
NODEFAULTUNO=TRUE
s2u=
javauno=
.ENDIF			# "$(URD_ONLY)" != ""

.IF "$(IDLFILES)"!=""
.IF "$(NODEFAULTUNO)"==""

.IF "$(NOSMARTUNO)"==""
INC_:=$(INCCOM)$(UNOPRE)$/$(IDLPACKAGE)$/
SLO_=$(SLO)$(SMARTPRE)$/$(IDLPACKAGE)$/
MISC_:=$(MISC)$(SMARTPRE)$/$(IDLPACKAGE)$/
CXX_:=$(OUTCXX)$(SMARTPRE)$/$(IDLPACKAGE)$/
_PACKAGE=$(IDLPACKAGE)
#UNOIDLTARGETS+=$(foreach,i,$(IDLFILES:s/.idl/.hxx/:f) $(INC_)$i)
SMRSLOFILES+=$(foreach,i,$(IDLFILES:s/.idl/.obj/:f) $(SLO_)$i)
SMRFILES+=$(foreach,i,$(IDLFILES:s/.idl/.smr/:f) $(MISC_)$i)
SMRSLOTARGET=$(SLB)$/smr$(TARGET).lib

.IF "$(TF_PACKAGES_NO_S2U)"==""
.IF "$(s2u)"!=""
SLOPRE_=$(SLO)$(SMARTPRE)$/$(IDLPACKAGE)$(/)s2u_

# does this context make sense?
UNOIDLTARGETS+=$(foreach,i,$(IDLFILES:s/.idl/.h/:f) $(INC_)$i)

S2USLOFILES+=$(foreach,i,$(IDLFILES:s/.idl/.obj/:f) $(SLOPRE_)$i)
S2USLOTARGET=$(SLB)$/s2u$(TARGET).lib
.ENDIF		# "$(s2u)"!=""
.ENDIF 		# "$(TF_PACKAGES_NO_S2U)"==""

.ELSE		# "$(NOSMARTUNO)"==""
.IF "$(PACKAGE)"==""
MISC_:=$(MISC)$/
CXX_:=$(MISC)$/
INC_:=$(INCCOM)$/$(PRJNAME)$/
SLO_=$(SLO)$/
OBJ_=$(OBJ)$/
UNOIDLTARGETS+=$(foreach,i,$(IDLFILES:s/.idl/.cxx/:f) $(MISC_)$i)
.IF "$(IDLSTATICOBJS)"==""
SLOFILES+=$(foreach,i,$(IDLFILES:s/.idl/.obj/:f) $(SLO_)$i)
.ELSE
OBJFILES+=$(foreach,i,$(IDLFILES:s/.idl/.obj/:f) $(OBJ_)$i)
.ENDIF
.ELSE			# "$(PACKAGE)"==""
INC_:=$(INCCOM)$/$(IDLPACKAGE)$/
SLO_=$(SLO)$/$(IDLPACKAGE)$/
OBJ_=$(OBJ)$/$(IDLPACKAGE)$/
MISC_:=$(MISC)$/$(IDLPACKAGE)$/
CXX_:=$(OUTCXX)$/$(IDLPACKAGE)$/
_PACKAGE=$(IDLPACKAGE)
UNOIDLTARGETS+=$(foreach,i,$(IDLFILES:s/.idl/.cxx/:f) $(CXX_)$i)
.IF "$(IDLSTATICOBJS)"==""
.IF "$(GUI)"=="UNX"
IDLSLOFILES+=$(foreach,i,$(IDLFILES:s/.idl/.obj/:f) $(SLO_)$i)
.ELSE
SLOFILES+=$(foreach,i,$(IDLFILES:s/.idl/.obj/:f) $(SLO_)$i)
.ENDIF		# "$(GUI)"=="UNX"
.ELSE			# "$(IDLSTATICOBJS)"==""
.IF "$(GUI)"=="UNX"
IDLOBJFILES+=$(foreach,i,$(IDLFILES:s/.idl/.obj/:f) $(OBJ_)$i)
.ELSE
OBJFILES+=$(foreach,i,$(IDLFILES:s/.idl/.obj/:f) $(OBJ_)$i)
.ENDIF		# "$(GUI)"=="UNX"
.ENDIF			# "$(IDLSTATICOBJS)"==""

.ENDIF		# "$(PACKAGE)"==""
.ENDIF		# "$(NOSMARTUNO)"==""

.IF "$(NOURD)"==""
URD=TRUE
.ENDIF

.ENDIF		# "$(NODEFAULTUNO)"==""

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
.ENDIF		# "$(TF_PACKAGES)"==""

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
.IF "$(GUI)"=="OS2"
DEPIDLFILES:=$(foreach,i,$(IDLFILES) $(!null,$(shell $(FIND) . -name $i -print) $i $(shell $(FIND) $(IDLDIRS) -name $(i:f) -print) ))
.ELSE			# "$(GUI)"=="OS2"
.IF "$(GUI)"=="WNT" || "$(GUI)"=="WIN"
DEPIDLFILES:=$(foreach,i,$(IDLFILES) $(!null,$(shell $(FIND) . -name $i) $i $(shell ($(FIND) $(IDLDIRS) -name $(i:f)) | $(SED) s/\//\\/g )))
.ELSE			# "$(GUI)"=="WNT" || "$(GUI)"=="WIN"
DEPIDLFILES:=$(foreach,i,$(IDLFILES) $(!null,$(shell $(FIND) . -name $i -print) $i $(shell $(FIND) $(IDLDIRS) -name $(i:f) -print )  ))
.ENDIF			# "$(GUI)"=="WNT" || "$(GUI)"=="WIN"
.ENDIF			# "$(GUI)"=="OS2"
.ENDIF			# "$(LOCALIDLFILES)$(EXTERNIDLFILES)"!=""
.ENDIF			# "$(IDLFILES)"!=""

.IF "$(JARFILES)"!=""
NEWCLASS:=$(foreach,i,$(JARFILES) $(null,$(shell $(FIND) $(JARDIR) -name $i) $(SOLARBINDIR)$/$i $(shell $(FIND) $(JARDIR) -name $i)))
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
CLASSPATH=.:$(CLASSDIR):$(XCLASSPATH):$(NEWCLASS:s/ /:/)
.ELSE
CLASSPATH=.;$(CLASSDIR);$(XCLASSPATH);$(NEWCLASS:s/ /;/)
.ENDIF
.ENDIF			# "$(NEWCLASS)"!=""

.IF "$(NOOPTFILES)" != ""
NOOPTTARGET=do_it_noopt
.ENDIF

.IF "$(EXCEPTIONSFILES)" != ""
EXCEPTIONSTARGET=do_it_exceptions
.ENDIF

.IF "$(EXCEPTIONSNOOPTFILES)" != ""
EXCEPTIONSNOOPTTARGET=do_it_exceptions_noopt
.ENDIF

.IF "$(ADDOPTFILES)" != ""
ADDOPTTARGET=do_it_add
.ENDIF


#.IF "$(DELOPTFILES)" != ""
#DELOPTTARGET=do_it_del
#.ENDIF


.IF "$(NOPCHFILES)" != ""
NOPCHTARGET=do_itpch
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

.IF "$(NOLIBOBJTARGET)"!=""
dont_set_NOLIBOBJTARGET:
    @+echo --------------------------------------------------
    @+echo setting NOLIBOBJTARGET in your makefile will cause serious problems!
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

.IF "$(UNR)" != ""
.IF "$(GUI)" == "WIN"
OPTLINKS=
MAPSYM=
.ENDIF
.ENDIF

.IF "$(OPTLINKS)" != ""
.IF "$(GUI)" ==  "WIN"
LINK=$(DEVROOT)$/s70$/bin$/link
.ENDIF
.ENDIF

.IF "$(OPTLINKS1)" != ""
.IF "$(GUI)" ==  "WIN"
LINK=$(DEVROOT)$/bin$/optlinks$/optlinks
.ENDIF
.ENDIF

.IF "$(OPTLINKS2)" != ""
.IF "$(GUI)" ==  "WIN"
LINK=$(DEVROOT)$/bin$/optlinks$/neu$/link
.ENDIF
.ENDIF

.IF "$(SRCFILES)"!=""
SRCTARGET=$(SRS)$/$(TARGET).srs
.ENDIF

.IF "$(SRSFILES)"!=""
SRSTARGET=$(RES)$/$(TARGET).res
.ENDIF

.IF "$(SRC1FILES)"!=""
SRC1TARGET=$(SRS)$/$(SRS1NAME).srs
.ENDIF

.IF "$(SRS1FILES)"!=""
.IF "$(VCL)" != ""
SRS1TARGET=$(BIN)$/$(RES1TARGET).res
.ELSE
SRS1TARGET=$(RES)$/$(RES1TARGET).res
.ENDIF
.ENDIF

.IF "$(SRC2FILES)"!=""
SRC2TARGET=$(SRS)$/$(SRS2NAME).srs
.ENDIF

.IF "$(SRS2FILES)"!=""
.IF "$(VCL)" != ""
SRS2TARGET=$(BIN)$/$(RES2TARGET).res
.ELSE
SRS2TARGET=$(RES)$/$(RES2TARGET).res
.ENDIF
.ENDIF

.IF "$(SRC3FILES)"!=""
SRC3TARGET=$(SRS)$/$(SRS3NAME).srs
.ENDIF

.IF "$(SRS3FILES)"!=""
.IF "$(VCL)" != ""
SRS3TARGET=$(BIN)$/$(RES3TARGET).res
.ELSE
SRS3TARGET=$(RES)$/$(RES3TARGET).res
.ENDIF
.ENDIF

.IF "$(SRC4FILES)"!=""
SRC4TARGET=$(SRS)$/$(SRS4NAME).srs
.ENDIF

.IF "$(SRS1FILES)"!=""
.IF "$(VCL)" != ""
SRS4TARGET=$(BIN)$/$(RES4TARGET).res
.ELSE
SRS4TARGET=$(RES)$/$(RES4TARGET).res
.ENDIF
.ENDIF

.IF "$(SRC5FILES)"!=""
SRC5TARGET=$(SRS)$/$(SRS5NAME).srs
.ENDIF

.IF "$(SRS5FILES)"!=""
.IF "$(VCL)" != ""
SRS5TARGET=$(BIN)$/$(RES5TARGET).res
.ELSE
SRS5TARGET=$(RES)$/$(RES5TARGET).res
.ENDIF
.ENDIF

.IF "$(SRC6FILES)"!=""
SRC6TARGET=$(SRS)$/$(SRS6NAME).srs
.ENDIF

.IF "$(SRS6FILES)"!=""
.IF "$(VCL)" != ""
SRS6TARGET=$(BIN)$/$(RES6TARGET).res
.ELSE
SRS6TARGET=$(RES)$/$(RES6TARGET).res
.ENDIF
.ENDIF

.IF "$(SRC7FILES)"!=""
SRC7TARGET=$(SRS)$/$(SRS7NAME).srs
.ENDIF

.IF "$(SRS7FILES)"!=""
.IF "$(VCL)" != ""
SRS7TARGET=$(BIN)$/$(RES7TARGET).res
.ELSE
SRS7TARGET=$(RES)$/$(RES7TARGET).res
.ENDIF
.ENDIF

.IF "$(SRC8FILES)"!=""
SRC8TARGET=$(SRS)$/$(SRS8NAME).srs
.ENDIF

.IF "$(SRS8FILES)"!=""
.IF "$(VCL)" != ""
SRS8TARGET=$(BIN)$/$(RES8TARGET).res
.ELSE
SRS8TARGET=$(RES)$/$(RES8TARGET).res
.ENDIF
.ENDIF

.IF "$(SRC9FILES)"!=""
SRC9TARGET=$(SRS)$/$(SRS9NAME).srs
.ENDIF

.IF "$(SRS9FILES)"!=""
.IF "$(VCL)" != ""
SRS9TARGET=$(BIN)$/$(RES9TARGET).res
.ELSE
SRS9TARGET=$(RES)$/$(RES9TARGET).res
.ENDIF
.ENDIF

.IF "$(SOLAR_JAVA)"!=""
.IF "$(JAVAFILES)$(JAVACLASSFILES)$(GENJAVAFILES)"!=""
.IF "$(JAVAFILES)$(JAVACLASSFILES)"=="$(JAVAFILES)"
JAVACLASSFILES=	$(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)
.ENDIF			# "$(JAVAFILES)$(JAVACLASSFILES)"=="$(JAVAFILES)"
JAVATARGET:=$(MISC)$/$(TARGET)_dummy.java
.ENDIF			# "$(JAVAFILES)$(JAVACLASSFILES)$(GENJAVAFILES)"!=""
.IF "$(javauno)"!=""
.IF "$(IDLFILES)"!=""
JAVATARGET=$(MISC)$/$(TARGET)_dummy.java
UNOIDLTARGETS+=$(MISC)$/$(TARGET)genjava.don
.ENDIF			# "$(IDLFILES)"!=""
.ENDIF			# "$(javauno)"!=""

.IF "$(JARTARGET)"!=""
JARCLASSDIRS*=.
.IF "$(NEW_JAR_PACK)"!=""
JARMANIFEST*=META-INF$/MANIFEST.MF
.ENDIF			# "$(NEW_JAR_PACK)"!=""
JARTARGETN=$(JARTARGET)
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

.IF "$(RCFILES)"!=""
RESNAME*=$(TARGET)
.IF "$(solarlang)" == "deut"
RCTARGET=$(RES)$/$(RESNAME).res
.IF "$(NO_REC_RES)"!=""
RCTARGET!:=$(foreach,i,$(alllangext) $(RES)$/$i$/$(RESNAME).res)
.ENDIF
.ELSE
.IF "$(RCFILES)" != "verinfo.rc"
RCTARGET=$(RES)$/$(RESNAME).res
.IF "$(NO_REC_RES)"!=""
RCTARGET!:=$(foreach,i,$(alllangext) $(RES)$/$i$/$(RESNAME).res)
.ENDIF
.ELSE
RCFILES=
.ENDIF
.ENDIF
.ENDIF

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
.IF "$(GUI)"=="UNX"
ziplangdirs:=$(shell +find . -type d ! -name CVS ! -name "." | sed "s/\.\///" )
.ELSE			# "$(GUI)"=="UNX"
ziplangdirs:=$(subst,CVS, $(shell +dir /ba:d ))
.ENDIF			# "$(GUI)"=="UNX"
zipalllangext=$(foreach,i,$(alllangext) $(eq,{$(subst,$(longlang_$i), $(ziplangdirs))},{$(ziplangdirs)} $(null) $i ))

.ENDIF			#

.IF "$(ZIP1TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP1LIST:s/LANGDIR//)" == "$(ZIP1LIST)"
ZIP1TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP1TARGET).zip
.ELSE
ZIP1TARGETN:=$(foreach,i,$(zipalllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP1TARGET)$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP1LIST:s/LANGDIR//)" == "$(ZIP1LIST)"
ZIP1TARGETN=$(BIN)$/$(ZIP1TARGET)
.ELSE
ZIP1TARGETN=$(foreach,i,$(alllangext) $(BIN)$/$(ZIP1TARGET)$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP1 ?= TNR!:=1
.ENDIF

.IF "$(ZIP2TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP2LIST:s/LANGDIR//)" == "$(ZIP2LIST)"
ZIP2TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP2TARGET).zip
.ELSE
ZIP2TARGETN:=$(foreach,i,$(zipalllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP2TARGET)$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP2LIST:s/LANGDIR//)" == "$(ZIP2LIST)"
ZIP2TARGETN=$(BIN)$/$(ZIP2TARGET)
.ELSE
ZIP2TARGETN=$(foreach,i,$(alllangext) $(BIN)$/$(ZIP2TARGET)$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP2 ?= TNR!:=2
.ENDIF

.IF "$(ZIP3TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP3LIST:s/LANGDIR//)" == "$(ZIP3LIST)"
ZIP3TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP3TARGET).zip
.ELSE
ZIP3TARGETN:=$(foreach,i,$(zipalllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP3TARGET)$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP3LIST:s/LANGDIR//)" == "$(ZIP3LIST)"
ZIP3TARGETN=$(BIN)$/$(ZIP3TARGET)
.ELSE
ZIP3TARGETN=$(foreach,i,$(alllangext) $(BIN)$/$(ZIP3TARGET)$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP3 ?= TNR!:=3
.ENDIF

.IF "$(ZIP4TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP4LIST:s/LANGDIR//)" == "$(ZIP4LIST)"
ZIP4TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP4TARGET).zip
.ELSE
ZIP4TARGETN:=$(foreach,i,$(zipalllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP4TARGET)$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP4LIST:s/LANGDIR//)" == "$(ZIP4LIST)"
ZIP4TARGETN=$(BIN)$/$(ZIP4TARGET)
.ELSE
ZIP4TARGETN=$(foreach,i,$(alllangext) $(BIN)$/$(ZIP4TARGET)$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP4 ?= TNR!:=4
.ENDIF

.IF "$(ZIP5TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP5LIST:s/LANGDIR//)" == "$(ZIP5LIST)"
ZIP5TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP5TARGET).zip
.ELSE
ZIP5TARGETN:=$(foreach,i,$(zipalllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP5TARGET)$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP5LIST:s/LANGDIR//)" == "$(ZIP5LIST)"
ZIP5TARGETN=$(BIN)$/$(ZIP5TARGET)
.ELSE
ZIP5TARGETN=$(foreach,i,$(alllangext) $(BIN)$/$(ZIP5TARGET)$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP5 ?= TNR!:=5
.ENDIF

.IF "$(ZIP6TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP6LIST:s/LANGDIR//)" == "$(ZIP6LIST)"
ZIP6TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP6TARGET).zip
.ELSE
ZIP6TARGETN:=$(foreach,i,$(zipalllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP6TARGET)$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP6LIST:s/LANGDIR//)" == "$(ZIP6LIST)"
ZIP6TARGETN=$(BIN)$/$(ZIP6TARGET)
.ELSE
ZIP6TARGETN=$(foreach,i,$(alllangext) $(BIN)$/$(ZIP6TARGET)$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP6 ?= TNR!:=6
.ENDIF

.IF "$(ZIP7TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP7LIST:s/LANGDIR//)" == "$(ZIP7LIST)"
ZIP7TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP7TARGET).zip
.ELSE
ZIP7TARGETN:=$(foreach,i,$(zipalllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP7TARGET)$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP7LIST:s/LANGDIR//)" == "$(ZIP7LIST)"
ZIP7TARGETN=$(BIN)$/$(ZIP7TARGET)
.ELSE
ZIP7TARGETN=$(foreach,i,$(alllangext) $(BIN)$/$(ZIP7TARGET)$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP7 ?= TNR!:=7
.ENDIF

.IF "$(ZIP8TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP8LIST:s/LANGDIR//)" == "$(ZIP8LIST)"
ZIP8TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP8TARGET).zip
.ELSE
ZIP8TARGETN:=$(foreach,i,$(zipalllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP8TARGET)$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP8LIST:s/LANGDIR//)" == "$(ZIP8LIST)"
ZIP8TARGETN=$(BIN)$/$(ZIP8TARGET)
.ELSE
ZIP8TARGETN=$(foreach,i,$(alllangext) $(BIN)$/$(ZIP8TARGET)$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP8 ?= TNR!:=8
.ENDIF

.IF "$(ZIP9TARGET)"!=""
.IF "$(common_build_zip)"!=""
.IF "$(ZIP9LIST:s/LANGDIR//)" == "$(ZIP9LIST)"
ZIP9TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP9TARGET).zip
.ELSE
ZIP9TARGETN:=$(foreach,i,$(zipalllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(ZIP9TARGET)$i.zip )
.ENDIF
.ELSE			# "$(common_build_zip)"!=""
.IF "$(ZIP9LIST:s/LANGDIR//)" == "$(ZIP9LIST)"
ZIP9TARGETN=$(BIN)$/$(ZIP9TARGET)
.ELSE
ZIP9TARGETN=$(foreach,i,$(alllangext) $(BIN)$/$(ZIP9TARGET)$i.zip )
.ENDIF
.ENDIF			# "$(common_build_zip)"!=""
ZIP9 ?= TNR!:=9
.ENDIF

.IF "$(APP1TARGET)"!=""
APP1TARGETN=$(BIN)$/$(APP1TARGET)$(EXECPOST)
.IF "$(NO_REC_RES)"!=""
.IF "$(APP1NOSVRES)"!=""
APP1TARGETN!:=$(foreach,i,$(alllangext) $(BIN)$/$i$/$(APP1TARGET)$(EXECPOST))
.ENDIF
.ENDIF
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
.IF "$(NO_REC_RES)"!=""
.IF "$(APP2NOSVRES)"!=""
APP2TARGETN!:=$(foreach,i,$(alllangext) $(BIN)$/$i$/$(APP2TARGET)$(EXECPOST))
.ENDIF
.ENDIF
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
.IF "$(NO_REC_RES)"!=""
.IF "$(APP3NOSVRES)"!=""
APP3TARGETN!:=$(foreach,i,$(alllangext) $(BIN)$/$i$/$(APP3TARGET)$(EXECPOST))
.ENDIF
.ENDIF
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
COMPRDB*:=$(SOLARBINDIR)$/applicat.rdb
.ENDIF			# "$(UNOUCRRDB)"!=""
.ENDIF
.IF "$(COMP1TYPELIST)"!=""
UNOTYPES+=$($(COMP1TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL1DESCRIPTION)"==""
SHL1DESCRIPTIONOBJ*=$(SLO)$/$(COMP1TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL1DESCRIPTION)"==""
COMP1RDBTARGETN:=$(BIN)$/$(COMP1TYPELIST).rdb
.ENDIF

.IF "$(COMP2TYPELIST)"!=""
UNOTYPES+=$($(COMP2TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL2DESCRIPTION)"==""
SHL2DESCRIPTIONOBJ*=$(SLO)$/$(COMP2TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL2DESCRIPTION)"==""
COMP2RDBTARGETN:=$(BIN)$/$(COMP2TYPELIST).rdb
.ENDIF

.IF "$(COMP3TYPELIST)"!=""
UNOTYPES+=$($(COMP3TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL3DESCRIPTION)"==""
SHL3DESCRIPTIONOBJ*=$(SLO)$/$(COMP3TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL3DESCRIPTION)"==""
COMP3RDBTARGETN:=$(BIN)$/$(COMP3TYPELIST).rdb
.ENDIF

.IF "$(COMP4TYPELIST)"!=""
UNOTYPES+=$($(COMP4TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL4DESCRIPTION)"==""
SHL4DESCRIPTIONOBJ*=$(SLO)$/$(COMP4TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL4DESCRIPTION)"==""
COMP4RDBTARGETN:=$(BIN)$/$(COMP4TYPELIST).rdb
.ENDIF

.IF "$(COMP5TYPELIST)"!=""
UNOTYPES+=$($(COMP5TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL5DESCRIPTION)"==""
SHL5DESCRIPTIONOBJ*=$(SLO)$/$(COMP5TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL5DESCRIPTION)"==""
COMP5RDBTARGETN:=$(BIN)$/$(COMP5TYPELIST).rdb
.ENDIF

.IF "$(COMP6TYPELIST)"!=""
UNOTYPES+=$($(COMP6TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL6DESCRIPTION)"==""
SHL6DESCRIPTIONOBJ*=$(SLO)$/$(COMP6TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL6DESCRIPTION)"==""
COMP6RDBTARGETN:=$(BIN)$/$(COMP6TYPELIST).rdb
.ENDIF

.IF "$(COMP7TYPELIST)"!=""
UNOTYPES+=$($(COMP7TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL7DESCRIPTION)"==""
SHL7DESCRIPTIONOBJ*=$(SLO)$/$(COMP7TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL7DESCRIPTION)"==""
COMP7RDBTARGETN:=$(BIN)$/$(COMP7TYPELIST).rdb
.ENDIF

.IF "$(COMP8TYPELIST)"!=""
UNOTYPES+=$($(COMP8TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL8DESCRIPTION)"==""
SHL8DESCRIPTIONOBJ*=$(SLO)$/$(COMP8TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL8DESCRIPTION)"==""
COMP8RDBTARGETN:=$(BIN)$/$(COMP8TYPELIST).rdb
.ENDIF

.IF "$(COMP9TYPELIST)"!=""
UNOTYPES+=$($(COMP9TYPELIST)_XML2CMPTYPES)
.IF "$(NO_SHL9DESCRIPTION)"==""
SHL9DESCRIPTIONOBJ*=$(SLO)$/$(COMP9TYPELIST)_description.obj
.ENDIF			# "$(NO_SHL9DESCRIPTION)"==""
COMP9RDBTARGETN:=$(BIN)$/$(COMP9TYPELIST).rdb
.ENDIF

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

.IF "$(IMGLST_SRS)"!=""
.IF "$(NO_REC_RES)"==""
IMGLSTTARGET=$(MISCX)$/$(TARGET)_img.don
.ELSE
.IF "$(make_srs_deps)"==""
.IF "$(common_build_reslib)"!=""
IMGLSTTARGET=$(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)_img$i.don)
.ELSE			# "$(common_build_reslib)"!=""
IMGLSTTARGET=$(foreach,i,$(alllangext) $(MISC)$/$(TARGET)_img$i.don)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF			# "$(make_srs_deps)"==""
.ENDIF			# "$(NO_REC_RES)"==""
.ENDIF			# "$(IMGLST_SRS)"!=""

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
.IF "$(VCL)" != ""
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
RESLIB1TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB1NAME)$(UPD)LANGEXT.res
.ELSE			# "$(common_build_reslib)"!=""
RESLIB1TARGETN=$(BIN)$/$(RESLIB1NAME)$(UPD)LANGEXT.res
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB1TARGETN!:=$(foreach,i,$(alllangext) $(RESLIB1TARGETN:s/LANGEXT/$i/))
.ELSE			# "$(NO_REC_RES)"!=""
RESLIB1TARGETN=$(BIN)$/$(RESLIB1NAME)$(UPD)$(LANGEXT).res
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE
RESLIB1TARGETN=$(BIN)$/$(RESLIB1NAME)$(UPD)$(LANGEXT).dll
.ENDIF
.ENDIF

.IF "$(RESLIB2NAME)" != ""
RESLIB2 ?= TNR!:=2
.IF "$(VCL)" != ""
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
RESLIB2TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB2NAME)$(UPD)LANGEXT.res
.ELSE			# "$(common_build_reslib)"!=""
RESLIB2TARGETN=$(BIN)$/$(RESLIB2NAME)$(UPD)LANGEXT.res
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB2TARGETN!:=$(foreach,i,$(alllangext) $(RESLIB2TARGETN:s/LANGEXT/$i/))
.ELSE			# "$(NO_REC_RES)"!=""
RESLIB2TARGETN=$(BIN)$/$(RESLIB2NAME)$(UPD)$(LANGEXT).res
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE
RESLIB2TARGETN=$(BIN)$/$(RESLIB2NAME)$(UPD)$(LANGEXT).dll
.ENDIF
.ENDIF

.IF "$(RESLIB3NAME)" != ""
RESLIB3 ?= TNR!:=3
.IF "$(VCL)" != ""
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
RESLIB3TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB3NAME)$(UPD)LANGEXT.res
.ELSE			# "$(common_build_reslib)"!=""
RESLIB3TARGETN=$(BIN)$/$(RESLIB3NAME)$(UPD)LANGEXT.res
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB3TARGETN!:=$(foreach,i,$(alllangext) $(RESLIB3TARGETN:s/LANGEXT/$i/))
.ELSE			# "$(NO_REC_RES)"!=""
RESLIB3TARGETN=$(BIN)$/$(RESLIB3NAME)$(UPD)$(LANGEXT).res
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE
RESLIB3TARGETN=$(BIN)$/$(RESLIB3NAME)$(UPD)$(LANGEXT).dll
.ENDIF
.ENDIF

.IF "$(RESLIB4NAME)" != ""
RESLIB4 ?= TNR!:=4
.IF "$(VCL)" != ""
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
RESLIB4TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB4NAME)$(UPD)LANGEXT.res
.ELSE			# "$(common_build_reslib)"!=""
RESLIB4TARGETN=$(BIN)$/$(RESLIB4NAME)$(UPD)LANGEXT.res
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB4TARGETN!:=$(foreach,i,$(alllangext) $(RESLIB4TARGETN:s/LANGEXT/$i/))
.ELSE			# "$(NO_REC_RES)"!=""
RESLIB4TARGETN=$(BIN)$/$(RESLIB4NAME)$(UPD)$(LANGEXT).res
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE
RESLIB4TARGETN=$(BIN)$/$(RESLIB4NAME)$(UPD)$(LANGEXT).dll
.ENDIF
.ENDIF

.IF "$(RESLIB5NAME)" != ""
RESLIB5 ?= TNR!:=5
.IF "$(VCL)" != ""
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
RESLIB5TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB5NAME)$(UPD)LANGEXT.res
.ELSE			# "$(common_build_reslib)"!=""
RESLIB5TARGETN=$(BIN)$/$(RESLIB5NAME)$(UPD)LANGEXT.res
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB5TARGETN!:=$(foreach,i,$(alllangext) $(RESLIB5TARGETN:s/LANGEXT/$i/))
.ELSE			# "$(NO_REC_RES)"!=""
RESLIB5TARGETN=$(BIN)$/$(RESLIB5NAME)$(UPD)$(LANGEXT).res
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE
RESLIB5TARGETN=$(BIN)$/$(RESLIB5NAME)$(UPD)$(LANGEXT).dll
.ENDIF
.ENDIF

.IF "$(RESLIB6NAME)" != ""
RESLIB6 ?= TNR!:=6
.IF "$(VCL)" != ""
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
RESLIB6TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB6NAME)$(UPD)LANGEXT.res
.ELSE			# "$(common_build_reslib)"!=""
RESLIB6TARGETN=$(BIN)$/$(RESLIB6NAME)$(UPD)LANGEXT.res
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB6TARGETN!:=$(foreach,i,$(alllangext) $(RESLIB6TARGETN:s/LANGEXT/$i/))
.ELSE			# "$(NO_REC_RES)"!=""
RESLIB6TARGETN=$(BIN)$/$(RESLIB6NAME)$(UPD)$(LANGEXT).res
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE
RESLIB6TARGETN=$(BIN)$/$(RESLIB6NAME)$(UPD)$(LANGEXT).dll
.ENDIF
.ENDIF

.IF "$(RESLIB7NAME)" != ""
RESLIB7 ?= TNR!:=7
.IF "$(VCL)" != ""
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
RESLIB7TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB7NAME)$(UPD)LANGEXT.res
.ELSE			# "$(common_build_reslib)"!=""
RESLIB7TARGETN=$(BIN)$/$(RESLIB7NAME)$(UPD)LANGEXT.res
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB7TARGETN!:=$(foreach,i,$(alllangext) $(RESLIB7TARGETN:s/LANGEXT/$i/))
.ELSE			# "$(NO_REC_RES)"!=""
RESLIB7TARGETN=$(BIN)$/$(RESLIB7NAME)$(UPD)$(LANGEXT).res
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE
RESLIB7TARGETN=$(BIN)$/$(RESLIB7NAME)$(UPD)$(LANGEXT).dll
.ENDIF
.ENDIF

.IF "$(RESLIB8NAME)" != ""
RESLIB8 ?= TNR!:=8
.IF "$(VCL)" != ""
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
RESLIB8TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB8NAME)$(UPD)LANGEXT.res
.ELSE			# "$(common_build_reslib)"!=""
RESLIB8TARGETN=$(BIN)$/$(RESLIB8NAME)$(UPD)LANGEXT.res
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB8TARGETN!:=$(foreach,i,$(alllangext) $(RESLIB8TARGETN:s/LANGEXT/$i/))
.ELSE			# "$(NO_REC_RES)"!=""
RESLIB8TARGETN=$(BIN)$/$(RESLIB8NAME)$(UPD)$(LANGEXT).res
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE
RESLIB8TARGETN=$(BIN)$/$(RESLIB8NAME)$(UPD)$(LANGEXT).dll
.ENDIF
.ENDIF

.IF "$(RESLIB9NAME)" != ""
RESLIB9 ?= TNR!:=9
.IF "$(VCL)" != ""
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
RESLIB9TARGETN=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB9NAME)$(UPD)LANGEXT.res
.ELSE			# "$(common_build_reslib)"!=""
RESLIB9TARGETN=$(BIN)$/$(RESLIB9NAME)$(UPD)LANGEXT.res
.ENDIF			# "$(common_build_reslib)"!=""
RESLIB9TARGETN!:=$(foreach,i,$(alllangext) $(RESLIB9TARGETN:s/LANGEXT/$i/))
.ELSE			# "$(NO_REC_RES)"!=""
RESLIB9TARGETN=$(BIN)$/$(RESLIB9NAME)$(UPD)$(LANGEXT).res
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE
RESLIB9TARGETN=$(BIN)$/$(RESLIB9NAME)$(UPD)$(LANGEXT).dll
.ENDIF
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

# MISCX for NO_REC_RES uncritical here
.IF "$(IDLNAME)"!=""
IDLTARGET=$(MISCX)$/$(IDLNAME).don
IDL0 ?= TNR!:=
.ENDIF

.IF "$(IDL1NAME)"!=""

IDL1TARGET=$(MISCX)$/$(IDL1NAME).don
IDL1 ?= TNR!:=1
.ENDIF

.IF "$(IDL2NAME)"!=""
IDL2TARGET=$(MISCX)$/$(IDL2NAME).don
IDL2 ?= TNR!:=2
.ENDIF

.IF "$(IDL3NAME)"!=""
IDL3TARGET=$(MISCX)$/$(IDL3NAME).don
IDL3 ?= TNR!:=3
.ENDIF

.IF "$(IDL4NAME)"!=""
IDL4TARGET=$(MISCX)$/$(IDL4NAME).don
IDL4 ?= TNR!:=4
.ENDIF

.IF "$(IDL5NAME)"!=""
IDL5TARGET=$(MISCX)$/$(IDL5NAME).don
IDL5 ?= TNR!:=5
.ENDIF

.IF "$(SDINAME)"!=""
.DIRCACHE=no
SDITARGET=$(MISCX)$/$(SDINAME).don
SDI0 ?= TNR!:=
.ENDIF

.IF "$(SDI1NAME)"!=""
.DIRCACHE=no
SDI1TARGET=$(MISCX)$/$(SDI1NAME).don
SDI1 ?= TNR!:=1
.ENDIF

.IF "$(SDI2NAME)"!=""
.DIRCACHE=no
SDI2TARGET=$(MISCX)$/$(SDI2NAME).don
SDI2 ?= TNR!:=2
.ENDIF

.IF "$(SDI3NAME)"!=""
.DIRCACHE=no
SDI3TARGET=$(MISCX)$/$(SDI3NAME).don
SDI3 ?= TNR!:=3
.ENDIF

.IF "$(SDI4NAME)"!=""
.DIRCACHE=no
SDI4TARGET=$(MISCX)$/$(SDI4NAME).don
SDI4 ?= TNR!:=4
.ENDIF

.IF "$(SDI5NAME)"!=""
.DIRCACHE=no
SDI5TARGET=$(MISCX)$/$(SDI5NAME).don
SDI5 ?= TNR!:=5
.ENDIF

.IF "$(XMLPROPERTIES)"!=""
XMLPROPERTIESN:=$(foreach,i,$(XMLPROPERTIES) $(MISC)$/$(TARGET)_$(i:s/.xrb/.done/))
.ENDIF			# "$(XMLPROPERTIES)"!=""

.IF "$(XMLXULRES)"!=""
XMLXULRESN:=$(foreach,i,$(XMLXULRES) $(MISC)$/$(TARGET)_xxl_$(i:s/.xxl/.done/))
.ENDIF			# "$(XMLXULRES)"!=""

.IF "$(UNIXTEXT)"!=""
.IF "$(GUI)"=="UNX"
CONVERTUNIXTEXT:=$(UNIXTEXT)
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF			# "$(UNIXTEXT)"!=""

.IF "$(IDLFILES)"!=""
.IF "$(TF_PACKAGES)"!=""
.IF "$(NODEFAULTUNO)"==""
UNOIDLDEPTARGETS+=$(MISC)$/$(TARGET).dp1
.ENDIF
.IF "$(NOSMARTUNO)"==""
.IF "$(s2u)"!=""
UNOIDLDEPTARGETS+=$(MISC)$/$(TARGET).dp2
.ENDIF			# "$(s2u)"!=""
.ENDIF			# "$(NOSMARTUNO)"==""
.IF "$(javauno)"!=""
UNOIDLDEPTARGETS+=$(MISC)$/$(TARGET).dp3
.ENDIF			# "$(javauno)"!=""
.ENDIF			# "$(TF_PACKAGES)"!=""
.ENDIF			# "$(IDLFILES)"!=""

.IF "$(JAVACLASSFILES:s/DEFINED//)"!="" || "$(javauno)"!=""
TARGETDPJ=$(MISC)$/$(TARGET).dpj
.ENDIF

.IF "$(UPDATER)"=="YES"
.IF "$(product)"!=""
.IF "$(GEN_HID)"!=""
.IF "$(no_hids)"==""
.IF "$(BUILD_SOSL)" == ""
.IF "$(GUI)"=="WNT"
PRJHIDTARGET=$(MISC)$/$(PRJNAME).hid
.ENDIF
.ENDIF
.ENDIF
.ENDIF
.ENDIF
.ENDIF

.IF "$(linkinc)"!=""
LINKINCTARGETS+=$(MISC)$/linkinc.ls
.ENDIF

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

# -------
# - DEF -
# -------

# bei MAKE...DIR die gleiche Reihenfolge wie in settings.mak einhalten!

.IF "$(NO_REC_RES)"==""
.IF "$(solarlang)" != "deut"
MAKELANGDIR=makelang.dir
.ENDIF
.ENDIF

.IF "$(lintit)"==""

#		$(NOOPTTARGET) $(EXCEPTIONSTARGET)

ALLTAR: $(MAKELANGDIR)	$(MAKEDEMODIR)	$(MAKECOMPDIR) $(MAKEXLDIR)	\
        $(target_empty) \
        $(OS2_COPY_MK)		\
        $(SUBDIRS)		\
        $(DELDEFS)		\
        $(YACCTARGET)	\
        $(UNOUCRTARGET)	\
        $(NOPCHTARGET)						\
        $(UNOIDLDEPTARGETS) \
        $(URDTARGET) \
        $(URDDOCTARGET) \
        $(UNOIDLTARGETS) \
        $(PROJECTPCHTARGET) \
        $(ADDOPTTARGET) $(DELOPTTARGET) \
        $(DEPFILES) $(DPCTARGET) \
        $(DPRTARGET) \
        $(DPZTARGET) \
        $(IDLTARGET)	$(SDITARGET)	\
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
        $(GENJAVAFILES) \
        $(JAVATARGET)	\
        $(JAVACLASSFILES) $(JAVA1CLASSFILES)	\
        $(JAVA2CLASSFILES)	$(JAVA3CLASSFILES) $(JAVA4CLASSFILES)\
        $(JAVA5CLASSFILES)	$(JAVA6CLASSFILES) $(JAVA7CLASSFILES)\
        $(JAVA8CLASSFILES)	$(JAVA9CLASSFILES) $(JAVA10CLASSFILES)\
        $(JAVA11CLASSFILES)	\
        $(TARGETDPJ) \
        $(OBJTARGET)	$(SLOTARGET)	$(SMRSLOTARGET)		\
        $(S2USLOTARGET) \
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
        $(SRCTARGET)	$(SRSTARGET) \
        $(SRC1TARGET)	$(SRS1TARGET) \
        $(SRC2TARGET)	$(SRS2TARGET) \
        $(SRC3TARGET)	$(SRS3TARGET)		  \
        $(SRC4TARGET)	$(SRC5TARGET)	$(SRC6TARGET)		\
        $(SRC7TARGET)	$(SRC8TARGET)	$(SRC9TARGET)		\
        $(SRC10TARGET)	$(SRC11TARGET)	$(SRC12TARGET)		\
        $(SRC13TARGET)	$(SRC14TARGET)	$(SRC15TARGET)		\
        $(SRC16TARGET) \
        $(IMGLSTTARGET) \
        $(INDPRESLIB1TARGETN) \
        $(RESLIB1TARGETN) $(RESLIB2TARGETN) \
        $(RESLIB3TARGETN) $(RESLIB4TARGETN) \
        $(RESLIB5TARGETN) $(RESLIB6TARGETN) \
        $(RESLIB7TARGETN) $(RESLIB8TARGETN) \
        $(RESLIBSPLIT1TARGETN) $(RESLIBSPLIT2TARGETN)\
        $(RESLIBSPLIT3TARGETN) $(RESLIBSPLIT4TARGETN)\
        $(RESLIBSPLIT5TARGETN) $(RESLIBSPLIT6TARGETN)\
        $(RESLIBSPLIT7TARGETN) \
        $(PRJHIDTARGET) \
        $(DO_JS)$(SIGNFORNETSCAPE) \
        $(DO_JS)$(SIGNFOREXPLORER) \
        $(CONVERTUNIXTEXT) \
        last_target

.IF "$(EXCEPTIONSNOOPT_FLAG)"==""
TARGETDEPS+=$(EXCEPTIONSNOOPTTARGET)
.ENDIF

.IF "$(NOOPT_FLAG)"==""
TARGETDEP+=$(NOOPTTARGET)
.ENDIF

.IF "$(EXCEPTIONS_FLAG)"==""
TARGETDEPS+=$(EXCEPTIONSTARGET)
.ENDIF

.IF "$(ADDOPT_FLAG)"==""
TARGETDEPS+=$(ADDOPTTARGET)
.ENDIF

#don't override .TARGETS when called with targets
.IF "$(MAKETARGETS)$(TNR)$(ADDOPT_FLAG)$(EXCEPTIONSNOOPT_FLAG)$(EXCEPTIONS_FLAG)$(NOOPT_FLAG)"==""
.IF "$(TARGETDEPS)"!=""
#.TARGETS .SEQUENTIAL :- $(TARGETDEPS) ALLTAR
.INIT .SEQUENTIAL : $(TARGETDEPS) $(NULLPRQ)
.ENDIF
.ENDIF

.IF "$(NO_REC_RES)"==""
ALLTAR: \
    make_all_current_resources
.ENDIF


.IF "$(GUI)"=="WNT"
CPPUMAKERFLAGS*=-L
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(UNOTYPES)" != ""
.IF "$(SINGLE_SHOT)"==""
# makeing all in one
.DIRCACHE=no
.IF "$(INCPRE))"!=""
MKDEPFLAGS+=-I:$(INCPRE)
.ENDIF			# "$(INCPRE))"!=""
.IF "$(OBJFILES)"!=""
$(OBJFILES) : $(UNOUCRHEADER)
.ENDIF			# "$(OBJFILES)"!=""
.IF "$(SLOFILES)"!=""
$(SLOFILES) : $(UNOUCRHEADER)
.ENDIF			# "$(SLOFILES)"!=""
$(UNOUCRTARGET) : $(UNOUCRDEP)
    +cppumaker @$(mktmp $(CPPUMAKERFLAGS) -B$(UNOUCRBASE) -O$(UNOUCROUT) $(UNOTYPES:^"-T")  $(UNOUCRRDB))
.ENDIF			# "$(SINGLE_SHOT)" == ""
.ENDIF			# "$(UNOTYPES)" != ""

last_target:
    @+echo -------------

.ELSE

ALLTAR: $(OBJFILES) $(SLOFILES)

.ENDIF

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

.IF "$(GENJAVAFILES)"!=""
$(GENJAVAFILES) : $(RDB)
$(JAVATARGET) : $(GENJAVAFILES)
.ENDIF			# "$(GENJAVAFILES)"!=""

#mhtest:
#	@echo MYUNOIDLTARGETS : $(MYUNOIDLTARGETS) $(SLOFILES)
#	@echo MHMISC_ : $(UNOIDLTARGETS) $(IDLPACKAGE) xxxx $(_PACKAGE)
#	@+-echo "#define" SUPD "$(UPD)" >> ttt.mk

.INCLUDE : tg_dir.mk

.INCLUDE : tg_idl.mk


.IF "$(TF_PACKAGES)"==""

.IF "$(s2u)"!=""
$(MISC)$/s2u_$(TARGET).don:
    +-$(RM) $@
    +$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINC) -Wb,c -OH$(INCCOM)$/$(PACKAGE:s/ttt/\/) $(DEPIDLFILES)
    +$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINC) -Bs2u -P$(PACKAGE:s/ttt/\/) -OH$(INCCOM)$/$(PACKAGE) -OI$(MISC)  $(SMRTARGETS:s/ttt/\/)
    +-$(TOUCH) $@
.ENDIF

.ENDIF			# "$(TF_PACKAGES)"==""

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
        +-$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Bdoc -P..$/$(PRJNAME)$/$(IDLPACKAGE) -OH$(PRJ)$/..$/unodoc $(DOCIDLFILES) $(IDLFILES)

.IF "$(LOCALDBTARGET)"!=""
$(LOCALDBTARGET) : $(URDFILES)
    +-$(RM) $@
    +regmerge $@ UCR @$(mktmp $(URDFILES))
.ENDIF

.IF "$(LOCALDOCDBTARGET)"!=""
$(LOCALDOCDBTARGET) : $(URDDOCFILES)
    +-$(RM) $@
    +regmerge $@ UCR @$(mktmp $(URDDOCFILES))
.ENDIF

.IF "$(UNOIDLDBTARGET)"!=""
$(UNOIDLDBTARGET) : $(UNOIDLDBFILES)
    +-$(RM) $@
    +regmerge $@ / @$(mktmp $(UNOIDLDBFILES) $(UNOIDLDBREGS))
.IF "$(LOCALREGDB)"!=""
    +regmerge $(LOCALREGDB) / $@
.ENDIF
.ENDIF			# "$(UNOIDLDBTARGET)"!=""

.IF "$(UNOIDLDBDOCTARGET)"!=""
$(UNOIDLDBDOCTARGET) : $(UNOIDLDBDOCFILES)
    +-$(RM) $@
    +regmerge $@ / @$(mktmp $(UNOIDLDBDOCFILES) $(UNOIDLDBDOCREGS))
.IF "$(LOCALREGDB)"!=""
    +regmerge $(LOCALREGDB) / $@
.ENDIF
.ENDIF			# "$(UNOIDLDBDOCTARGET)"!=""

.IF "$(SCP_PRODUCT_TYPE)"!=""
$(SCP_PRODUCT_TYPE):
    @+-$(MKDIRHIER) $(PAR)$/$@ >& $(NULLDEV)
    @+-$(MKDIRHIER) $(BIN)$/$@ >& $(NULLDEV)

.ENDIF			# "$(PARFILES)"!=""

.IF "$(IMGLSTTARGET)"!=""
$(IMGLSTTARGET): $(IMGLST_SRS)
    @+echo -----------------
    @+echo Making Imagelists:
    @+echo -----------------
    @+-$(RM) $@ >& $(NULLDEV)
.IF "$(NO_REC_RES)"==""
    +bmp $(IMGLST_SRS) $(BMP_IN) $(BMP_OUT) $(solarlang)
.ELSE
.IF "$(common_build_reslib)"!=""
    @-+$(MKDIR) $(RES)$/$(langext_{$(subst,$(TARGET)_img, $(@:b))}) >& $(NULLDEV)
    @-+$(MKDIR) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))$/$(langext_{$(subst,$(TARGET)_img, $(@:b))}) >& $(NULLDEV)
    +bmp $(IMGLST_SRS) $(BMP_IN) $(BMP_OUT)$/$(langext_{$(subst,$(TARGET)_img, $(@:b))}) $(lang_{$(subst,$(TARGET)_img, $(@:b))}) -f $@
    -+$(GNUCOPY) -pub $(RES)$/$(langext_{$(subst,$(TARGET)_img, $(@:b))})/* $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))$/$(langext_{$(subst,$(TARGET)_img, $(@:b))}) >& $(NULLDEV)
    +-$(RM) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))$/$(langext_{$(subst,$(TARGET)_img, $(@:b))})$/*.bmp~
.ELSE			# "$(common_build_reslib)"!=""
    @-+$(MKDIR) $(RES)$/$(langext_{$(subst,$(TARGET)_img, $(@:b))}) >& $(NULLDEV)
    +bmp $(IMGLST_SRS) $(BMP_IN) $(BMP_OUT)$/$(langext_{$(subst,$(TARGET)_img, $(@:b))}) $(lang_{$(subst,$(TARGET)_img, $(@:b))}) -f $@
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF
.IF "$(BMP_WRITES_FLAG)"==""
    @+echo > $@
.ENDIF
.ENDIF

.IF "$(XMLPROPERTIES)"!=""
$(MISC)$/$(TARGET)_%.done : %.xrb
    native2ascii -encoding UTF8 $< $(MISC)$/$(<:b).interm$(TARGET)
    @xmlex -i $(MISC)$/$(<:b).interm$(TARGET) -o $(CLASSDIR) -g -d $@
    @+$(RM)  $(MISC)$/$(<:b).interm$(TARGET) >& $(NULLDEV)
.ENDIF			# "$(XMLPROPERTIES)"!=""

.IF "$(XMLXULRES)"!=""
$(MISC)$/$(TARGET)_xxl_%.done : %.xxl
    @xmlex -i $(<:b).xxl -o $(OUT)$/xul$/locale -g:dtd -d $@
.ENDIF			# "$(XMLXULRES)"!=""

.INCLUDE : tg_sdi.mk

.IF "$(DEF1NAME)$(DEF2NAME)$(DEF3NAME)$(DEF4NAME)$(DEF5NAME)$(DEF6NAME)$(DEF7NAME)$(DEF8NAME)$(DEF9NAME)"!=""
.IF "$(MK_UNROLL)"!=""
.INCLUDE : _tg_def.mk
.ELSE
.INCLUDE : tg_def.mk
.ENDIF
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

.INCLUDE : tg_srs.mk

# -------
# - RES -
# -------

.IF "$(MK_UNROLL)"!=""
.INCLUDE : tg_res.mk
.ELSE
.INCLUDE : tg_res.mk
.ENDIF

# -------
# - YXX -
# -------

.INCLUDE : tg_yxx.mk

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

.IF "$(ZIP1TARGETN)$(ZIP2TARGETN)$(ZIP3TARGETN)$(ZIP4TARGETN)$(ZIP5TARGETN)$(ZIP6TARGETN)$(ZIP7TARGETN)$(ZIP8TARGETN)$(ZIP9TARGETN)"!=""
.IF "$(MK_UNROLL)"!=""
.INCLUDE : _tg_zip.mk
.ELSE
.INCLUDE : tg_zip.mk
.ENDIF
.ENDIF

# -------
# - RESLIBTARGET -
# -------

.IF "$(MK_UNROLL)"!=""
.INCLUDE : _tg_rslb.mk
.ELSE
.INCLUDE : tg_rslb.mk
.ENDIF

# -------
# - SHL -
# -------

.IF "$(MK_UNROLL)"!=""
.INCLUDE : _tg_shl.mk
.ELSE
.INCLUDE : tg_shl.mk
.ENDIF


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
.IF "$(GROUP)"=="WRITER"
.IF "$(debug)"!=""
.IF "$(depend)"==""
.INCLUDE : $(MISC)$/$(TARGET).dpw
.ENDIF			# "$(depend)"==""
.ENDIF			# "$(debug)"!=""
.ENDIF			# "$(GROUP)"=="WRITER"
.ENDIF
.ELSE		# MAKEFILERC
.ENDIF		# MAKEFILERC

.IF "$(make_srs_deps)"==""
.IF "$(SRCFILES)" != "" || "$(SRC1FILES)" != "" || "$(SRC2FILES)" != ""
.INCLUDE : $(MISC)$/$(TARGET).dpr
.ENDIF
.ENDIF

.IF "$(make_zip_deps)"==""
.IF "$(ZIP1TARGET)" != "" || "$(ZIP2TARGET)" != "" || "$(ZIP3TARGET)" != ""
.INCLUDE : $(MISC)$/$(TARGET).dpz
.ENDIF
.ENDIF

$(MISC)$/$(TARGET)genjava.mk: 	$(IDLFILES)

.IF "$(IDLFILES)"!=""
.IF "$(TF_PACKAGES)"!=""

.IF "$(URD)"!=""

$(URDTARGET) : $(DEPIDLFILES)
        +$(UNOIDL) @$(mktmp -Wb,c $(UNOIDLDEFS) $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Burd -OH$(OUT)$/ucr$/$(IDLPACKAGE) $(DEPIDLFILES:+"\n"))
        @+echo > $@

.IF "$(URDDOC)"!=""

$(URDDOCTARGET) : $(DEPIDLFILES)
        @+-mkdir $(OUT)$/ucrdoc >& $(NULLDEV)
        +$(UNOIDL) @$(mktmp $(UNOIDLDEFS) $(TF_PACKAGES_DEF) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Burd -OH$(OUT)$/ucrdoc$/$(IDLPACKAGE) $(DEPIDLFILES:+"\n"))
        @+echo > $@
.ENDIF			# "$(URDDOC)"!=""
.ENDIF			# "$(URD)"!=""

.IF "$(NODEFAULTUNO)"==""
.INCLUDE : $(MISC)$/$(TARGET).dp1
$(MISC)$/$(TARGET).dp1: $(IDLFILES)
#makefile.mk
.ENDIF

# nosmart, but s2u ????
#.IF "$(NOSMARTUNO)"==""
.IF "$(s2u)"!=""
.INCLUDE : $(MISC)$/$(TARGET).dp2
$(MISC)$/$(TARGET).dp2: $(IDLFILES) $(foreach,i,$(IDLFILES:s/.idl/.smr/) $(MISC)$(SMARTPRE)$/$(IDLPACKAGE)$/$i)
#makefile.mk
.ENDIF			# "$(s2u)"!=""
#.ENDIF			# "$(NOSMARTUNO)"==""

.IF "$(javauno)"!=""
$(MISC)$/$(TARGET)genjava.don: $(IDLFILES)
# $(MISC)$/$(TARGET).dp3 : $(IDLFILES)
.ENDIF			# "$(javauno)"!=""


.IF "$(javauno)"!=""
.INCLUDE : $(MISC)$/$(TARGET).dp3
$(MISC)$/$(TARGET).dp3 : $(IDLFILES)
#makefile.mk

JAVAFILES+=$(IDL_JAVA_FILES)
JAVACLASSFILES+=$(subst,misc$/java,class $(IDL_JAVA_FILES:s/.java/.class/))
JAVATARGET*=$(MISC)$/$(TARGET)_dummy.java

.ENDIF			# "$(javauno)"!=""

.IF "$(NODEFAULTUNO)"==""
# line moved
#.INCLUDE : $(MISC)$/$(TARGET).dp1
.IF "$(NOSMARTUNO)"==""
CXXFILES+=$(IDL_SMART_CXX_FILES)
.ELSE
CXXFILES+=$(IDL_CPP_CXX_FILES)
.ENDIF
.ENDIF			# "$(NODEFAULTUNO)"==""

.IF "$(NOSMARTUNO)"==""
.IF "$(s2u)"!=""
# line moved
#.INCLUDE : $(MISC)$/$(TARGET).dp2
CXXFILES+=$(IDL_S2U_FILES)
.ENDIF			# "$(s2u)"!=""
.ENDIF			# "$(NOSMARTUNO)"==""
.ENDIF			# "$(TF_PACKAGES)"!=""
.ENDIF			# "$(IDLFILES)"!=""

.IF "$(JAVACLASSFILES:s/DEFINED//)"!=""
.INCLUDE .IGNORE : $(MISC)$/$(TARGET).dpj
$(TARGETDPJ) : $(JAVAFILES) $(JAVATARGET)
.ENDIF

.INCLUDE : tg_jar.mk
.INCLUDE : tg_jarsign.mk

# ----------------------------------
# - NOPCH - files ohne PCH -
# ----------------------------------

.IF "$(NOPCHTARGET)" != ""
.IF "$(NOPCH_FLAG)" == ""
$(NOPCHTARGET):
    @+echo --- NOPCH ---
    @dmake $(MFLAGS) PCHSLOFLAGSU= PCHOBJFLAGSU= $(NOPCHFILES) NOPCH_FLAG=TRUE $(CALLMACROS)
.ENDIF
.ENDIF

# --------------------------------
# - PROJECT pre compiled headers -
# --------------------------------
.IF "$(PROJECTPCHTARGET)"!=""
.IF "$(uniq $(TARGETDEPS))"!=""
.INIT .SEQUENTIAL  :- $(PROJECTPCHTARGET) $(TARGETDEPS)
.ENDIF

.IF "$(PROJECTPCH_FLAG)"==""
$(PROJECTPCHTARGET) .PHONY :
    dmake $(MFLAGS) $@ PROJECTPCH_FLAG=TRUE $(CALLMACROS)
.ELSE			# "$(PROJECTPCH_FLAG)"!=""
$(PROJECTPCHTARGET) : $(PROJECTPCHSOURCE).cxx
.IF "$(COM)"=="MSC" || "(COM)"=="BLC"
.IF "$(PROJECTPCH4DLL)" != ""
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSSLO) $(PCHSLOFLAGSC) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $(CFLAGSOUTOBJ)$(SLO)$/$(PROJECTPCH).obj $(PROJECTPCHSOURCE).cxx
.ELSE
    $(CC) $(CFLAGS) $(CFLAGSCXX) $(CFLAGSOBJ) $(PCHOBJFLAGSC) $(CDEFS) $(CDEFSOBJ) $(CFLAGSOUTOBJ)$(OBJ)$/$(PROJECTPCH).obj $(PROJECTPCHSOURCE).cxx
.ENDIF
.ENDIF
.ENDIF			# "$(PROJECTPCH_FLAG)"!=""
.ENDIF

# ----------------------------------
# - NOOPT - files ohne optimierung -
# ----------------------------------

.IF "$(NOOPTTARGET)" != ""
.IF "$(NOOPT_FLAG)" == ""

#$(SLOFILES) $(OBJFILES) $(IDLSLOFILES) $(IDLOBJFILES) $(S2USLOFILES) $(SMRSLOFILES) $(SVXLIGHTSLOFILES) $(SVXLIGHTOBJFILES) $($(SECOND_BUILD)_SLOFILES) $($(SECOND_BUILD)_OBJFILES) : $(NOOPTTARGET)

$(NOOPTTARGET):
    @+echo --- NOOPTFILES ---
    @dmake $(MFLAGS) nopt=true $(NOOPTFILES) NOOPT_FLAG=TRUE $(CALLMACROS)
    @+echo --- NOOPTFILES OVER ---

$(NOOPTFILES):
    @+echo --- NOOPT ---
    @dmake $(MFLAGS) nopt=true NOOPT_FLAG=TRUE $(CALLMACROS) $@
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
    @dmake $(MFLAGS) ENABLE_EXCEPTIONS=true $(EXCEPTIONSFILES) EXCEPTIONS_FLAG=TRUE $(CALLMACROS)
    @+echo --- EXCEPTIONSFILES OVER ---

$(EXCEPTIONSFILES):
    @+echo --- EXCEPTIONS ---
    @dmake $(MFLAGS) ENABLE_EXCEPTIONS=true EXCEPTIONS_FLAG=TRUE $(CALLMACROS) $@
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
    @dmake $(MFLAGS) ENABLE_EXCEPTIONS=true $(EXCEPTIONSNOOPTFILES) EXCEPTIONSNOOPT_FLAG=TRUE nopt=true $(CALLMACROS)
    @+echo --- EXCEPTIONSNOOPTFILES OVER ---

$(EXCEPTIONSNOOPTFILES):
    @+echo --- EXCEPTIONSNOOPT ---
    @dmake $(MFLAGS) ENABLE_EXCEPTIONS=true EXCEPTIONSNOOPT_FLAG=TRUE nopt=true $(CALLMACROS) $@
    @+echo --- EXCEPTIONSNOOPT OVER ---


.ENDIF
.ENDIF

# ----------------------------------
# - ADDOPT - files ohne optimierung -
# ----------------------------------

.IF "$(ADDOPTTARGET)" != ""
.IF "$(ADDOPT_FLAG)" == ""

#$(SLOFILES) $(OBJFILES) $(IDLSLOFILES) $(IDLOBJFILES) $(S2USLOFILES) $(SMRSLOFILES) $(SVXLIGHTSLOFILES) $(SVXLIGHTOBJFILES) : $(ADDOPTTIONSTARGET)

$(ADDOPTTARGET):
    @+echo --- ADDOPT ---
.IF "$(WORK_STAMP)"!="SRV506"
    @+echo no longer supported
    force_dmake_to_error
.ELSE
    @dmake $(MFLAGS) add_cflags=$(add_cflags) add_cflagscxx=$(add_cflagscxx) addopt=true $(ADDOPTFILES) ADDOPT_FLAG=TRUE $(CALLMACROS)
.ENDIF

.ENDIF
.ENDIF


# ----------------------------------
# - DELOPT - files ohne optimierung -
# ----------------------------------

.IF "$(DELOPTTARGET)" != ""
.IF "$(DELOPT_FLAG)" == ""

#$(SLOFILES) $(OBJFILES) $(IDLSLOFILES) $(IDLOBJFILES) $(S2USLOFILES) $(SMRSLOFILES) $(SVXLIGHTSLOFILES) $(SVXLIGHTOBJFILES) : $(DELOPTTIONSTARGET)

$(DELOPTTARGET):
    @+echo --- DELOPT ---
.IF "$(WORK_STAMP)"!="SRV506"
    @+echo no longer supported
    force_dmake_to_error
.ELSE
    @dmake $(MFLAGS) delopt=true $(DELOPTFILES) DELOPT_FLAG=TRUE $(CALLMACROS)
.ENDIF

.ENDIF
.ENDIF
# ----------------------------------
# - OTHER - alles wofuer rules da sind -
# ----------------------------------

OTHERTARGET : $(OTHER)

# -------------------------
# - diverse kill commands -
# -------------------------

$(TMP)\makedt.don:
    @$(TOUCH) $(TMP)\makedt.don

do_copy_mk .IGNORE .SILENT :
    @+-$(MKDIR) $(OS2_SOLENV_INC) >& $(NULLDEV)
    @+-$(MKDIR) $(OS2_SOLENV_INC)$/startup >& $(NULLDEV)
    @+-$(MKDIR) $(OS2_SOLENV_INC)$/startup$/os2 >& $(NULLDEV)
    @+-attrib -rs $(OS2_SOLENV_INC)$/*.mk >& $(NULLDEV)
    @+-$(COPY) /u $(SOLARENV)$/inc$/*.mk $(OS2_SOLENV_INC) >& $(NULLDEV)
    @+-$(COPY) /u $(SOLARENV)$/inc$/startup$/*.mk $(OS2_SOLENV_INC)$/startup >& $(NULLDEV)
    @+-$(COPY) /u $(SOLARENV)$/inc$/startup$/os2$/*.mk $(OS2_SOLENV_INC)$/startup$/os2 >& $(NULLDEV)

killbin:
.IF "$(GUI)"=="WNT"
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
    
.ELSE			# "$(GUI)"=="WNT"A
.IF "$(SHL1TARGET)"!=""
    @+-$(RM) $(LB)/$(SHL1TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL2TARGET)"!=""
    @+-$(RM) $(LB)/$(SHL2TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL3TARGET)"!=""
    @+-$(RM) $(LB)/$(SHL3TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL4TARGET)"!=""
    @+-$(RM) $(LB)/$(SHL4TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL5TARGET)"!=""
    @+-$(RM) $(LB)/$(SHL5TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL6TARGET)"!=""
    @+-$(RM) $(LB)/$(SHL6TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL7TARGET)"!=""
    @+-$(RM) $(LB)/$(SHL7TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL8TARGET)"!=""
    @+-$(RM) $(LB)/$(SHL8TARGET)$(DLLPOST)
.ENDIF
.IF "$(SHL9TARGET)"!=""
    @+-$(RM) $(LB)/$(SHL9TARGET)$(DLLPOST)
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
    +-$(RM) $(SLOFILES)
    +-$(RM) $(SLOFILES:s/.obj/.o/)
.ENDIF
.IF "$(OBJFILES)" != ""
    +-$(RM) $(OBJFILES)
    +-$(RM) $(OBJFILES:s/.obj/.o/)
.ENDIF
.IF "$(DEPOBJFILES)" != ""
    +-$(RM) $(DEPOBJFILES)
    +-$(RM) $(DEPOBJFILES:s/.obj/.o/)
.ENDIF
    @+echo objects weg!

killsrs:
.IF "$(SRSFILES)" != ""
    +$(RM) $(SRSFILES)
.ENDIF
    @+echo srsfiles weg!

killres:
.IF "$(RESFILES)" != ""
    +$(RM) $(RESFILES)
.ENDIF
    @+echo res files weg!

killdef:
.IF "$(DEFTARGETN)" != ""
    +$(RM) $(DEFTARGETN)
.ENDIF
    @+echo deffiles weg!

SRCALLTARGET:	\
        $(OS2_COPY_MK)		\
        $(IDLTARGET)	$(IDL1TARGET)	$(IDL2TARGET)		\
        $(IDL3TARGET)	$(IDL4TARGET)	$(IDL5TARGET)		\
        $(SDITARGET)	$(SDI1TARGET)	$(SDI2TARGET)		\
        $(SDI3TARGET)	$(SDI4TARGET)	$(SDI5TARGET)		\
        $(SRCTARGET)	$(SRSTARGET) \
        $(SRC1TARGET)	$(SRS1TARGET) \
        $(SRC2TARGET)	$(SRS2TARGET) \
        $(SRC3TARGET)	$(SRS3TARGET)	  $(RCTARGET)		  \
        $(SRC4TARGET)	$(SRC5TARGET)	$(SRC6TARGET)		\
        $(SRC7TARGET)	$(SRC8TARGET)	$(SRC9TARGET)		\
        $(SRC10TARGET)	$(SRC11TARGET)	$(SRC12TARGET)		\
        $(SRC13TARGET)	$(SRC14TARGET)	$(SRC15TARGET)		\
        $(SRC16TARGET) \
        $(IMGLSTTARGET)

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


#temporary workaround for non-existing delzip in extras
delzip:
    +echo > $@

$(MISC)$/helpids.don: $(HELPIDFILES)
    @echo Making helpids:
    @echo ---------------
    @echo r:\bat\mkhids.btm $(PRJ)\$(INPATH)\misc\help.id $(HELPIDFILES)
    rem @copy /uq r:\bat\mkhids.btm r:\bat\mkhids.bat
    rem @r:\bat\mkhids.bat $(PRJ)\$(INPATH)\misc\help.id $(HELPIDFILES)
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
$(MISC)$/$(TARGET).dpz : $(ZIP1TARGETN) $(ZIP2TARGETN) $(ZIP3TARGETN) $(ZIP4TARGETN) $(ZIP5TARGETN) $(ZIP6TARGETN) $(ZIP7TARGETN) $(ZIP8TARGETN) $(ZIP9TARGETN)
.ENDIF

$(INCCOM)$/_version.h : $(SOLARVERSION)$/$(UPD)minor.mk
.IF "$(GUI)"=="UNX"
        @+echo "#define" _BUILD \"$(BUILD)\"	> $@
        @+echo "#define" _UPD \"$(UPD)\"		>> $@
        @+echo "#define" _LAST_MINOR \'$(LAST_MINOR)\'	>> $@
        @+echo '#define _RSCREVISION "$(RSCREVISION)"' >> $@
        @+echo "#define" _INPATH \"$(INPATH)\"	>> $@
.ELSE
        @+echo #define _BUILD "$(BUILD)"	> $@
        @+echo #define _UPD "$(UPD)"		>> $@
        @+echo #define _LAST_MINOR '$(LAST_MINOR)'	>> $@
        @+echo #define _DLL_POSTFIX "$(DLL_POSTFIX)">> $@
        @+echo #define _RSCREVISION "$(RSCREVISION)">> $@
        @+echo #define _INPATH "$(INPATH)"	>> $@
.ENDIF

.IF "$(MAKEFILERC)"==""
warn_target_empty:
    @+echo *
    @+echo * error $$(TARGET) is empty - this will cause problems
    @+echo *
    force_dmake_to_error
.ELSE
warn_target_empty:
    @+echo generated makefile.rc detected
.ENDIF

$(MISC)$/$(TARGET)genjava.mk: 	# $(IDLFILES)
    @+echo *
    @+echo *
    @+echo *	bitte aendern - HJS
    @+echo *
    @+echo *
        @-+$(RM) $@
.IF "$(SOLAR_JAVA)"!=""
        @+-$(RM) this.is.a.dummy.java
        @+-$(TOUCH) this.is.a.dummy.java
        +$(UNOIDL) $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Bjava -P$(PRJNAME) -OH$(MISC)$/java $(IDLFILES)
.IF "$(GUI)"=="UNX"
        @+echo JAVAFILES+= \\ > $@
        find $(MISC)$/java -type f | awk -f ./nodollar.awk >> $@
        @+echo "" >> $@
.ELSE
        @+echo JAVAFILES+= \ > $@
        @+dir /hbsa:-d $(MISC)$/java | gawk '!/.*[!$$].*/ { print $$0 " \\" }' >> $@
        @+echo this.is.a.dummy.java >> $@
.ENDIF
        dmake BUILDJAVA=t $(MFLAGS) $(CALLMACROS)
        @+-$(RM) this.is.a.dummy.java
.ELSE
        @+-$(TOUCH) $@
        dmake BUILDJAVA=t $(MFLAGS) $(CALLMACROS)
.ENDIF

.IF "$(IDLFILES)"!=""
.IF "$(javauno)"!=""
$(JAVACLASSFILES) : $(JAVAFILES)
$(JAVAFILES) : $(MISC)$/$(TARGET)genjava.don

$(MISC)$/$(TARGET)genjava.don :
    @-+$(RM) $@
.IF "$(SOLAR_JAVA)"!=""
    +$(UNOIDL) @$(mktmp $(UNOIDLDEFS) $(UNOIDLINCEXTRA) $(UNOIDLINC) -Bjava -P$(PRJNAME) -OH$(MISC)$/java $(DEPIDLFILES))
.ELSE
    @+echo SOLAR_JAVA nicht gesetzt
.ENDIF
    @+-$(TOUCH) $@
.ENDIF
.ENDIF

.IF "$(UNOTYPES)" != ""
UNOUCRDEPxxx : $(UNOUCRDEP);
.ENDIF			# "$(UNOTYPES)" != ""

$(MISC)$/$(PRJNAME).hid : $(RESLIB1SRSFILES)
    @echo Making $@ :
    @echo ---------------
    @+if exist $@ del $@
    @$(TOUCH) $@
    @+if exist $(SRS)\*.hid type $(SRS)\*.hid >> $@
    @+if exist $(MISC)\*.lst $(ENV_TOOLS)$/slothid.bat $(MISC)\*.lst $@ $(INPATH)

.IF "$(linkinc)"!=""
.IF "$(GUI)"=="WNT"
$(MISC)$/linkinc.ls:
        +sed -f r:\bat\chrel.sed $(MISC)$/*.lin >> $@
.ENDIF
.ELSE
$(MISC)$/linkinc.ls:
    echo . > $@
.ENDIF


.INCLUDE : tg_java.mk

$(MISC)\$(TARGET).lck:
    @vlog -BL *.??v > $(MISC)\$(TARGET).lck

.IF "$(COM)"=="WTC"
libr:
.ENDIF

# dependencies from *.lng to par-files, this dependency is to much
# but better than nothing
.IF "$(PARFILES)"!=""
$(ALLPARFILES): $(LNGFILES)
.ENDIF

wordcount:
    +wc *.* >> $(TMP)$/wc.lst

testt:
    @echo test

.ELSE

# ----------
# - DEPEND -
# ----------

ALLTAR : ALLDEP \
        $(SUBDIRS)

.INCLUDE : tg_dep.mk

.ENDIF

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

