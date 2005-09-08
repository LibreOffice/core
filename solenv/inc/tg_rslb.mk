#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_rslb.mk,v $
#
#   $Revision: 1.19 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:47:43 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************


.IF "$(MULTI_RESLIB_FLAG)" == ""
$(RESLIB1TARGETN) .NULL : RESLIB1

$(RESLIB2TARGETN) .NULL : RESLIB2

$(RESLIB3TARGETN) .NULL : RESLIB3

$(RESLIB4TARGETN) .NULL : RESLIB4

$(RESLIB5TARGETN) .NULL : RESLIB5

$(RESLIB6TARGETN) .NULL : RESLIB6

$(RESLIB7TARGETN) .NULL : RESLIB7

$(RESLIB8TARGETN) .NULL : RESLIB8

$(RESLIB9TARGETN) .NULL : RESLIB9
.ENDIF


.IF "$(MULTI_RESLIB_FLAG)"==""
RESLIB1 RESLIB2 RESLIB3 RESLIB4 RESLIB5 RESLIB6 RESLIB7 RESLIB8 RESLIB9:
    @dmake $(RESLIB$(TNR)TARGETN) $(HIDRES$(TNR)PARTICLE) MULTI_RESLIB_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE


#######################################################
# unroll begin

.IF "$(RESLIB$(TNR)TARGETN)"!=""

.IF "$(BUILDHIDS)"!=""
HIDRES$(TNR)PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(RESLIB$(TNR)NAME)_res.hid

#HACK cut off the dirty srs files which are included from solver
#RESLIB$(TNR)HIDFILESx=$(shell @+echo $(RESLIB$(TNR)SRSFILES:t"\n") | $(GREP) -v "$(SOLARRESDIR:s/\/\\/)" )
RESLIB$(TNR)HIDFILES:=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,.srs,_srs.hid $(RESLIB$(TNR)SRSFILES)))
$(HIDRES$(TNR)PARTICLE): $(RESLIB$(TNR)HIDFILES)
    @echo ------------------------------
    @echo Making: $@
    @+$(IFEXIST) $@ $(THEN) $(RM) $@
# need to strip since solaris cannot handle tab-only whitespace here
    +$(TYPE) $(mktmp  $(strip, $(subst,$/,/ $(RESLIB$(TNR)HIDFILES))) )| xargs -s 1000 cat > $@.$(ROUT).tmp
    @+$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDRES$(TNR)PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

$(RSC_MULTI$(TNR)) : \
        $(RESLIB$(TNR)SRSFILES) \
        $(RESLIB$(TNR)TARGETN) \
        $(RESLIB$(TNR)BMPS)
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARSRC)$/$(RSCDEFIMG)$/$(PRJNAME) $(RESLIB$(TNR)IMAGES)) -lip={$j}$/$i \
    -lip={$j} ) \
    -lip=$(SOLARSRC)$/$(RSCDEFIMG)$/res$/$i -lip=$(SOLARSRC)$/$(RSCDEFIMG)$/res ) \
    -subMODULE=$(SOLARSRC)$/$(RSCDEFIMG) \
    -subGLOBALRES=$(SOLARSRC)$/$(RSCDEFIMG)$/res \
    -oil={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft=$@ \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(BIN)$/$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARSRC)$/$(RSCDEFIMG)$/$(PRJNAME) $(RESLIB$(TNR)IMAGES)) -lip={$j}$/$i \
    -lip={$j} ) \
    -lip=$(SOLARSRC)$/$(RSCDEFIMG)$/res$/$i -lip=$(SOLARSRC)$/$(RSCDEFIMG)$/res ) \
    -subMODULE=$(SOLARSRC)$/$(RSCDEFIMG) \
    -subGLOBALRES=$(SOLARSRC)$/$(RSCDEFIMG)$/res \
    -oil=$(BIN) \
    -ft=$@ \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB$(TNR)TARGETN): \
        $(RESLIB$(TNR)SRSFILES) \
        $(RESLIB$(TNR)BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI$(TNR)) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI$(TNR)) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI$(TNR)) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI$(TNR)) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB$(TNR)TARGETN)"!=""


# unroll end
#######################################################

.ENDIF				# "$(MULTI_RESLIB_FLAG)"==""
