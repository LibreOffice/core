# unroll begin

.IF "$(DEF1TARGETN)"!=""

#to make intuitiv naming work
.IF "$(DEF1LIBNAME)"!=""
DEFLIB1NAME*=$(DEF1LIBNAME)
.ENDIF			# "$(DEF1LIBNAME)"!=""

.IF "$(DEFLIB1NAME)"!=""
.IF "$(UPDATER)"!=""
DEF1DEPN+=$(SLB)$/$(DEFLIB1NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL1VERSIONMAP)"!=""
.IF "$(DEF1EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF1EXPORTFILE=$(MISC)$/$(SHL1VERSIONMAP:b)_$(SHL1TARGET).dxp
$(DEF1EXPORTFILE) : $(SHL1VERSIONMAP)
    +$(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF1EXPORTFILE)"==""
.ENDIF			# "$(SHL1VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB1NAME)"!=""

DEF1UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE1:=$(shell +echo %_disk)
#BUILD_DRIVE1:=O

.IF "$(BUILD_DRIVE1)"=="O"
# in case of RE build, protect against failed lock
EXPORT1_PROTECT=$(TMP)$/$(DEF1UNIQE:b).bat && 
.ENDIF			# "$(BUILD_DRIVE1)"=="O"

.ENDIF			# "$(DEFLIB1NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""

.IF "$(APP1HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF1TARGETN) : \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF1TARGETN) .PHONY : \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB1NAME)"!=""
.IF "$(BUILD_DRIVE1)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF1UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE1)"=="O"
.ENDIF				# "$(DEFLIB1NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
    +-attrib -r defs$/$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL1TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(NO_SHL1DESCRIPTION)"==""
    @echo component_getDescriptionFunc	>>$@
.ENDIF			# "$(NO_SHL1DESCRIPTION)"==""
.IF "$(DEFLIB1NAME)"!=""
    @+$(EXPORT1_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(SHL1TARGET).exp $(SLB)$/$(DEFLIB1NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF1CEXP)"!=""
    @+$(EXPORT1_PROTECT) $(LDUMP2) -A $(DEF1CEXP) -E 20 -F $(MISC)$/$(SHL1TARGET).flt $(SHL1TARGET).exp			   >>$@
.ELSE
    @+$(EXPORT1_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL1TARGET).flt $(SHL1TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @+$(EXPORT1_PROTECT) $(LDUMP) -E 20 -F$(MISC)$/$(SHL1TARGET).flt $(SHL1TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +$(EXPORT1_PROTECT) $(RM) $(SHL1TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(BUILD_SOSL)"==""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE1)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF1UNIQE:b) commit
    +$(TMP)$/$(DEF1UNIQE:b).bat && $(RM) $(TMP)$/$(DEF1UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE1)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
.ENDIF				# "$(DEFLIB1NAME)"!=""
.IF "$(DEF1EXPORT1)"!=""
    @echo $(DEF1EXPORT1)										>>$@
.ENDIF
.IF "$(DEF1EXPORT2)"!=""
    @echo $(DEF1EXPORT2)										>>$@
.ENDIF
.IF "$(DEF1EXPORT3)"!=""
    @echo $(DEF1EXPORT3)										>>$@
.ENDIF
.IF "$(DEF1EXPORT4)"!=""
    @echo $(DEF1EXPORT4)										>>$@
.ENDIF
.IF "$(DEF1EXPORT5)"!=""
    @echo $(DEF1EXPORT5)										>>$@
.ENDIF
.IF "$(DEF1EXPORT6)"!=""
    @echo $(DEF1EXPORT6)										>>$@
.ENDIF
.IF "$(DEF1EXPORT7)"!=""
    @echo $(DEF1EXPORT7)										>>$@
.ENDIF
.IF "$(DEF1EXPORT8)"!=""
    @echo $(DEF1EXPORT8)										>>$@
.ENDIF
.IF "$(DEF1EXPORT9)"!=""
    @echo $(DEF1EXPORT9)										>>$@
.ENDIF
.IF "$(DEF1EXPORT10)"!=""
    @echo $(DEF1EXPORT10)										>>$@
.ENDIF
.IF "$(DEF1EXPORT11)"!=""
    @echo $(DEF1EXPORT11)										>>$@
.ENDIF
.IF "$(DEF1EXPORT12)"!=""
    @echo $(DEF1EXPORT12)										>>$@
.ENDIF
.IF "$(DEF1EXPORT13)"!=""
    @echo $(DEF1EXPORT13)										>>$@
.ENDIF
.IF "$(DEF1EXPORT14)"!=""
    @echo $(DEF1EXPORT14)										>>$@
.ENDIF
.IF "$(DEF1EXPORT15)"!=""
    @echo $(DEF1EXPORT15)										>>$@
.ENDIF
.IF "$(DEF1EXPORT16)"!=""
    @echo $(DEF1EXPORT16)										>>$@
.ENDIF
.IF "$(DEF1EXPORT17)"!=""
    @echo $(DEF1EXPORT17)										>>$@
.ENDIF
.IF "$(DEF1EXPORT18)"!=""
    @echo $(DEF1EXPORT18)										>>$@
.ENDIF
.IF "$(DEF1EXPORT19)"!=""
    @echo $(DEF1EXPORT19)										>>$@
.ENDIF
.IF "$(DEF1EXPORT20)"!=""
    @echo $(DEF1EXPORT20)										>>$@
.ENDIF
.IF "$(DEF1EXPORTFILE)"!=""
    +$(TYPE) $(DEF1EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP1HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF1TARGETN): \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
    @+echo ------------------------------
    @+echo Making Module-Definitionfile : $@
    @+echo just a dummy for UNIX
    @+echo > $@
.ENDIF
.ENDIF			# "$(DEF1TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(DEF2TARGETN)"!=""

#to make intuitiv naming work
.IF "$(DEF2LIBNAME)"!=""
DEFLIB2NAME*=$(DEF2LIBNAME)
.ENDIF			# "$(DEF2LIBNAME)"!=""

.IF "$(DEFLIB2NAME)"!=""
.IF "$(UPDATER)"!=""
DEF2DEPN+=$(SLB)$/$(DEFLIB2NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL2VERSIONMAP)"!=""
.IF "$(DEF2EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF2EXPORTFILE=$(MISC)$/$(SHL2VERSIONMAP:b)_$(SHL2TARGET).dxp
$(DEF2EXPORTFILE) : $(SHL2VERSIONMAP)
    +$(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF2EXPORTFILE)"==""
.ENDIF			# "$(SHL2VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB2NAME)"!=""

DEF2UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE2:=$(shell +echo %_disk)
#BUILD_DRIVE2:=O

.IF "$(BUILD_DRIVE2)"=="O"
# in case of RE build, protect against failed lock
EXPORT2_PROTECT=$(TMP)$/$(DEF2UNIQE:b).bat && 
.ENDIF			# "$(BUILD_DRIVE2)"=="O"

.ENDIF			# "$(DEFLIB2NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""

.IF "$(APP2HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF2TARGETN) : \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF2TARGETN) .PHONY : \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB2NAME)"!=""
.IF "$(BUILD_DRIVE2)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF2UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE2)"=="O"
.ENDIF				# "$(DEFLIB2NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
    +-attrib -r defs$/$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL2TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF2DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(NO_SHL2DESCRIPTION)"==""
    @echo component_getDescriptionFunc	>>$@
.ENDIF			# "$(NO_SHL2DESCRIPTION)"==""
.IF "$(DEFLIB2NAME)"!=""
    @+$(EXPORT2_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(SHL2TARGET).exp $(SLB)$/$(DEFLIB2NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF2CEXP)"!=""
    @+$(EXPORT2_PROTECT) $(LDUMP2) -A $(DEF2CEXP) -E 20 -F $(MISC)$/$(SHL2TARGET).flt $(SHL2TARGET).exp			   >>$@
.ELSE
    @+$(EXPORT2_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL2TARGET).flt $(SHL2TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @+$(EXPORT2_PROTECT) $(LDUMP) -E 20 -F$(MISC)$/$(SHL2TARGET).flt $(SHL2TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +$(EXPORT2_PROTECT) $(RM) $(SHL2TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(BUILD_SOSL)"==""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE2)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF2UNIQE:b) commit
    +$(TMP)$/$(DEF2UNIQE:b).bat && $(RM) $(TMP)$/$(DEF2UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE2)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
.ENDIF				# "$(DEFLIB2NAME)"!=""
.IF "$(DEF2EXPORT1)"!=""
    @echo $(DEF2EXPORT1)										>>$@
.ENDIF
.IF "$(DEF2EXPORT2)"!=""
    @echo $(DEF2EXPORT2)										>>$@
.ENDIF
.IF "$(DEF2EXPORT3)"!=""
    @echo $(DEF2EXPORT3)										>>$@
.ENDIF
.IF "$(DEF2EXPORT4)"!=""
    @echo $(DEF2EXPORT4)										>>$@
.ENDIF
.IF "$(DEF2EXPORT5)"!=""
    @echo $(DEF2EXPORT5)										>>$@
.ENDIF
.IF "$(DEF2EXPORT6)"!=""
    @echo $(DEF2EXPORT6)										>>$@
.ENDIF
.IF "$(DEF2EXPORT7)"!=""
    @echo $(DEF2EXPORT7)										>>$@
.ENDIF
.IF "$(DEF2EXPORT8)"!=""
    @echo $(DEF2EXPORT8)										>>$@
.ENDIF
.IF "$(DEF2EXPORT9)"!=""
    @echo $(DEF2EXPORT9)										>>$@
.ENDIF
.IF "$(DEF2EXPORT10)"!=""
    @echo $(DEF2EXPORT10)										>>$@
.ENDIF
.IF "$(DEF2EXPORT11)"!=""
    @echo $(DEF2EXPORT11)										>>$@
.ENDIF
.IF "$(DEF2EXPORT12)"!=""
    @echo $(DEF2EXPORT12)										>>$@
.ENDIF
.IF "$(DEF2EXPORT13)"!=""
    @echo $(DEF2EXPORT13)										>>$@
.ENDIF
.IF "$(DEF2EXPORT14)"!=""
    @echo $(DEF2EXPORT14)										>>$@
.ENDIF
.IF "$(DEF2EXPORT15)"!=""
    @echo $(DEF2EXPORT15)										>>$@
.ENDIF
.IF "$(DEF2EXPORT16)"!=""
    @echo $(DEF2EXPORT16)										>>$@
.ENDIF
.IF "$(DEF2EXPORT17)"!=""
    @echo $(DEF2EXPORT17)										>>$@
.ENDIF
.IF "$(DEF2EXPORT18)"!=""
    @echo $(DEF2EXPORT18)										>>$@
.ENDIF
.IF "$(DEF2EXPORT19)"!=""
    @echo $(DEF2EXPORT19)										>>$@
.ENDIF
.IF "$(DEF2EXPORT20)"!=""
    @echo $(DEF2EXPORT20)										>>$@
.ENDIF
.IF "$(DEF2EXPORTFILE)"!=""
    +$(TYPE) $(DEF2EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP2HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF2TARGETN): \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
    @+echo ------------------------------
    @+echo Making Module-Definitionfile : $@
    @+echo just a dummy for UNIX
    @+echo > $@
.ENDIF
.ENDIF			# "$(DEF2TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(DEF3TARGETN)"!=""

#to make intuitiv naming work
.IF "$(DEF3LIBNAME)"!=""
DEFLIB3NAME*=$(DEF3LIBNAME)
.ENDIF			# "$(DEF3LIBNAME)"!=""

.IF "$(DEFLIB3NAME)"!=""
.IF "$(UPDATER)"!=""
DEF3DEPN+=$(SLB)$/$(DEFLIB3NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL3VERSIONMAP)"!=""
.IF "$(DEF3EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF3EXPORTFILE=$(MISC)$/$(SHL3VERSIONMAP:b)_$(SHL3TARGET).dxp
$(DEF3EXPORTFILE) : $(SHL3VERSIONMAP)
    +$(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF3EXPORTFILE)"==""
.ENDIF			# "$(SHL3VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB3NAME)"!=""

DEF3UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE3:=$(shell +echo %_disk)
#BUILD_DRIVE3:=O

.IF "$(BUILD_DRIVE3)"=="O"
# in case of RE build, protect against failed lock
EXPORT3_PROTECT=$(TMP)$/$(DEF3UNIQE:b).bat && 
.ENDIF			# "$(BUILD_DRIVE3)"=="O"

.ENDIF			# "$(DEFLIB3NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""

.IF "$(APP3HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF3TARGETN) : \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF3TARGETN) .PHONY : \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB3NAME)"!=""
.IF "$(BUILD_DRIVE3)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF3UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE3)"=="O"
.ENDIF				# "$(DEFLIB3NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
    +-attrib -r defs$/$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL3TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF3DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(NO_SHL3DESCRIPTION)"==""
    @echo component_getDescriptionFunc	>>$@
.ENDIF			# "$(NO_SHL3DESCRIPTION)"==""
.IF "$(DEFLIB3NAME)"!=""
    @+$(EXPORT3_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(SHL3TARGET).exp $(SLB)$/$(DEFLIB3NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF3CEXP)"!=""
    @+$(EXPORT3_PROTECT) $(LDUMP2) -A $(DEF3CEXP) -E 20 -F $(MISC)$/$(SHL3TARGET).flt $(SHL3TARGET).exp			   >>$@
.ELSE
    @+$(EXPORT3_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL3TARGET).flt $(SHL3TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @+$(EXPORT3_PROTECT) $(LDUMP) -E 20 -F$(MISC)$/$(SHL3TARGET).flt $(SHL3TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +$(EXPORT3_PROTECT) $(RM) $(SHL3TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(BUILD_SOSL)"==""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE3)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF3UNIQE:b) commit
    +$(TMP)$/$(DEF3UNIQE:b).bat && $(RM) $(TMP)$/$(DEF3UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE3)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
.ENDIF				# "$(DEFLIB3NAME)"!=""
.IF "$(DEF3EXPORT1)"!=""
    @echo $(DEF3EXPORT1)										>>$@
.ENDIF
.IF "$(DEF3EXPORT2)"!=""
    @echo $(DEF3EXPORT2)										>>$@
.ENDIF
.IF "$(DEF3EXPORT3)"!=""
    @echo $(DEF3EXPORT3)										>>$@
.ENDIF
.IF "$(DEF3EXPORT4)"!=""
    @echo $(DEF3EXPORT4)										>>$@
.ENDIF
.IF "$(DEF3EXPORT5)"!=""
    @echo $(DEF3EXPORT5)										>>$@
.ENDIF
.IF "$(DEF3EXPORT6)"!=""
    @echo $(DEF3EXPORT6)										>>$@
.ENDIF
.IF "$(DEF3EXPORT7)"!=""
    @echo $(DEF3EXPORT7)										>>$@
.ENDIF
.IF "$(DEF3EXPORT8)"!=""
    @echo $(DEF3EXPORT8)										>>$@
.ENDIF
.IF "$(DEF3EXPORT9)"!=""
    @echo $(DEF3EXPORT9)										>>$@
.ENDIF
.IF "$(DEF3EXPORT10)"!=""
    @echo $(DEF3EXPORT10)										>>$@
.ENDIF
.IF "$(DEF3EXPORT11)"!=""
    @echo $(DEF3EXPORT11)										>>$@
.ENDIF
.IF "$(DEF3EXPORT12)"!=""
    @echo $(DEF3EXPORT12)										>>$@
.ENDIF
.IF "$(DEF3EXPORT13)"!=""
    @echo $(DEF3EXPORT13)										>>$@
.ENDIF
.IF "$(DEF3EXPORT14)"!=""
    @echo $(DEF3EXPORT14)										>>$@
.ENDIF
.IF "$(DEF3EXPORT15)"!=""
    @echo $(DEF3EXPORT15)										>>$@
.ENDIF
.IF "$(DEF3EXPORT16)"!=""
    @echo $(DEF3EXPORT16)										>>$@
.ENDIF
.IF "$(DEF3EXPORT17)"!=""
    @echo $(DEF3EXPORT17)										>>$@
.ENDIF
.IF "$(DEF3EXPORT18)"!=""
    @echo $(DEF3EXPORT18)										>>$@
.ENDIF
.IF "$(DEF3EXPORT19)"!=""
    @echo $(DEF3EXPORT19)										>>$@
.ENDIF
.IF "$(DEF3EXPORT20)"!=""
    @echo $(DEF3EXPORT20)										>>$@
.ENDIF
.IF "$(DEF3EXPORTFILE)"!=""
    +$(TYPE) $(DEF3EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP3HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF3TARGETN): \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
    @+echo ------------------------------
    @+echo Making Module-Definitionfile : $@
    @+echo just a dummy for UNIX
    @+echo > $@
.ENDIF
.ENDIF			# "$(DEF3TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(DEF4TARGETN)"!=""

#to make intuitiv naming work
.IF "$(DEF4LIBNAME)"!=""
DEFLIB4NAME*=$(DEF4LIBNAME)
.ENDIF			# "$(DEF4LIBNAME)"!=""

.IF "$(DEFLIB4NAME)"!=""
.IF "$(UPDATER)"!=""
DEF4DEPN+=$(SLB)$/$(DEFLIB4NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL4VERSIONMAP)"!=""
.IF "$(DEF4EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF4EXPORTFILE=$(MISC)$/$(SHL4VERSIONMAP:b)_$(SHL4TARGET).dxp
$(DEF4EXPORTFILE) : $(SHL4VERSIONMAP)
    +$(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF4EXPORTFILE)"==""
.ENDIF			# "$(SHL4VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB4NAME)"!=""

DEF4UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE4:=$(shell +echo %_disk)
#BUILD_DRIVE4:=O

.IF "$(BUILD_DRIVE4)"=="O"
# in case of RE build, protect against failed lock
EXPORT4_PROTECT=$(TMP)$/$(DEF4UNIQE:b).bat && 
.ENDIF			# "$(BUILD_DRIVE4)"=="O"

.ENDIF			# "$(DEFLIB4NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""

.IF "$(APP4HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF4TARGETN) : \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF4TARGETN) .PHONY : \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB4NAME)"!=""
.IF "$(BUILD_DRIVE4)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF4UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE4)"=="O"
.ENDIF				# "$(DEFLIB4NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
    +-attrib -r defs$/$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL4TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF4DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(NO_SHL4DESCRIPTION)"==""
    @echo component_getDescriptionFunc	>>$@
.ENDIF			# "$(NO_SHL4DESCRIPTION)"==""
.IF "$(DEFLIB4NAME)"!=""
    @+$(EXPORT4_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(SHL4TARGET).exp $(SLB)$/$(DEFLIB4NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF4CEXP)"!=""
    @+$(EXPORT4_PROTECT) $(LDUMP2) -A $(DEF4CEXP) -E 20 -F $(MISC)$/$(SHL4TARGET).flt $(SHL4TARGET).exp			   >>$@
.ELSE
    @+$(EXPORT4_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL4TARGET).flt $(SHL4TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @+$(EXPORT4_PROTECT) $(LDUMP) -E 20 -F$(MISC)$/$(SHL4TARGET).flt $(SHL4TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +$(EXPORT4_PROTECT) $(RM) $(SHL4TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(BUILD_SOSL)"==""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE4)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF4UNIQE:b) commit
    +$(TMP)$/$(DEF4UNIQE:b).bat && $(RM) $(TMP)$/$(DEF4UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE4)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
.ENDIF				# "$(DEFLIB4NAME)"!=""
.IF "$(DEF4EXPORT1)"!=""
    @echo $(DEF4EXPORT1)										>>$@
.ENDIF
.IF "$(DEF4EXPORT2)"!=""
    @echo $(DEF4EXPORT2)										>>$@
.ENDIF
.IF "$(DEF4EXPORT3)"!=""
    @echo $(DEF4EXPORT3)										>>$@
.ENDIF
.IF "$(DEF4EXPORT4)"!=""
    @echo $(DEF4EXPORT4)										>>$@
.ENDIF
.IF "$(DEF4EXPORT5)"!=""
    @echo $(DEF4EXPORT5)										>>$@
.ENDIF
.IF "$(DEF4EXPORT6)"!=""
    @echo $(DEF4EXPORT6)										>>$@
.ENDIF
.IF "$(DEF4EXPORT7)"!=""
    @echo $(DEF4EXPORT7)										>>$@
.ENDIF
.IF "$(DEF4EXPORT8)"!=""
    @echo $(DEF4EXPORT8)										>>$@
.ENDIF
.IF "$(DEF4EXPORT9)"!=""
    @echo $(DEF4EXPORT9)										>>$@
.ENDIF
.IF "$(DEF4EXPORT10)"!=""
    @echo $(DEF4EXPORT10)										>>$@
.ENDIF
.IF "$(DEF4EXPORT11)"!=""
    @echo $(DEF4EXPORT11)										>>$@
.ENDIF
.IF "$(DEF4EXPORT12)"!=""
    @echo $(DEF4EXPORT12)										>>$@
.ENDIF
.IF "$(DEF4EXPORT13)"!=""
    @echo $(DEF4EXPORT13)										>>$@
.ENDIF
.IF "$(DEF4EXPORT14)"!=""
    @echo $(DEF4EXPORT14)										>>$@
.ENDIF
.IF "$(DEF4EXPORT15)"!=""
    @echo $(DEF4EXPORT15)										>>$@
.ENDIF
.IF "$(DEF4EXPORT16)"!=""
    @echo $(DEF4EXPORT16)										>>$@
.ENDIF
.IF "$(DEF4EXPORT17)"!=""
    @echo $(DEF4EXPORT17)										>>$@
.ENDIF
.IF "$(DEF4EXPORT18)"!=""
    @echo $(DEF4EXPORT18)										>>$@
.ENDIF
.IF "$(DEF4EXPORT19)"!=""
    @echo $(DEF4EXPORT19)										>>$@
.ENDIF
.IF "$(DEF4EXPORT20)"!=""
    @echo $(DEF4EXPORT20)										>>$@
.ENDIF
.IF "$(DEF4EXPORTFILE)"!=""
    +$(TYPE) $(DEF4EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP4HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF4TARGETN): \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
    @+echo ------------------------------
    @+echo Making Module-Definitionfile : $@
    @+echo just a dummy for UNIX
    @+echo > $@
.ENDIF
.ENDIF			# "$(DEF4TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(DEF5TARGETN)"!=""

#to make intuitiv naming work
.IF "$(DEF5LIBNAME)"!=""
DEFLIB5NAME*=$(DEF5LIBNAME)
.ENDIF			# "$(DEF5LIBNAME)"!=""

.IF "$(DEFLIB5NAME)"!=""
.IF "$(UPDATER)"!=""
DEF5DEPN+=$(SLB)$/$(DEFLIB5NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL5VERSIONMAP)"!=""
.IF "$(DEF5EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF5EXPORTFILE=$(MISC)$/$(SHL5VERSIONMAP:b)_$(SHL5TARGET).dxp
$(DEF5EXPORTFILE) : $(SHL5VERSIONMAP)
    +$(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF5EXPORTFILE)"==""
.ENDIF			# "$(SHL5VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB5NAME)"!=""

DEF5UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE5:=$(shell +echo %_disk)
#BUILD_DRIVE5:=O

.IF "$(BUILD_DRIVE5)"=="O"
# in case of RE build, protect against failed lock
EXPORT5_PROTECT=$(TMP)$/$(DEF5UNIQE:b).bat && 
.ENDIF			# "$(BUILD_DRIVE5)"=="O"

.ENDIF			# "$(DEFLIB5NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""

.IF "$(APP5HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF5TARGETN) : \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF5TARGETN) .PHONY : \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB5NAME)"!=""
.IF "$(BUILD_DRIVE5)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF5UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE5)"=="O"
.ENDIF				# "$(DEFLIB5NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
    +-attrib -r defs$/$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL5TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF5DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(NO_SHL5DESCRIPTION)"==""
    @echo component_getDescriptionFunc	>>$@
.ENDIF			# "$(NO_SHL5DESCRIPTION)"==""
.IF "$(DEFLIB5NAME)"!=""
    @+$(EXPORT5_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(SHL5TARGET).exp $(SLB)$/$(DEFLIB5NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF5CEXP)"!=""
    @+$(EXPORT5_PROTECT) $(LDUMP2) -A $(DEF5CEXP) -E 20 -F $(MISC)$/$(SHL5TARGET).flt $(SHL5TARGET).exp			   >>$@
.ELSE
    @+$(EXPORT5_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL5TARGET).flt $(SHL5TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @+$(EXPORT5_PROTECT) $(LDUMP) -E 20 -F$(MISC)$/$(SHL5TARGET).flt $(SHL5TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +$(EXPORT5_PROTECT) $(RM) $(SHL5TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(BUILD_SOSL)"==""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE5)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF5UNIQE:b) commit
    +$(TMP)$/$(DEF5UNIQE:b).bat && $(RM) $(TMP)$/$(DEF5UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE5)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
.ENDIF				# "$(DEFLIB5NAME)"!=""
.IF "$(DEF5EXPORT1)"!=""
    @echo $(DEF5EXPORT1)										>>$@
.ENDIF
.IF "$(DEF5EXPORT2)"!=""
    @echo $(DEF5EXPORT2)										>>$@
.ENDIF
.IF "$(DEF5EXPORT3)"!=""
    @echo $(DEF5EXPORT3)										>>$@
.ENDIF
.IF "$(DEF5EXPORT4)"!=""
    @echo $(DEF5EXPORT4)										>>$@
.ENDIF
.IF "$(DEF5EXPORT5)"!=""
    @echo $(DEF5EXPORT5)										>>$@
.ENDIF
.IF "$(DEF5EXPORT6)"!=""
    @echo $(DEF5EXPORT6)										>>$@
.ENDIF
.IF "$(DEF5EXPORT7)"!=""
    @echo $(DEF5EXPORT7)										>>$@
.ENDIF
.IF "$(DEF5EXPORT8)"!=""
    @echo $(DEF5EXPORT8)										>>$@
.ENDIF
.IF "$(DEF5EXPORT9)"!=""
    @echo $(DEF5EXPORT9)										>>$@
.ENDIF
.IF "$(DEF5EXPORT10)"!=""
    @echo $(DEF5EXPORT10)										>>$@
.ENDIF
.IF "$(DEF5EXPORT11)"!=""
    @echo $(DEF5EXPORT11)										>>$@
.ENDIF
.IF "$(DEF5EXPORT12)"!=""
    @echo $(DEF5EXPORT12)										>>$@
.ENDIF
.IF "$(DEF5EXPORT13)"!=""
    @echo $(DEF5EXPORT13)										>>$@
.ENDIF
.IF "$(DEF5EXPORT14)"!=""
    @echo $(DEF5EXPORT14)										>>$@
.ENDIF
.IF "$(DEF5EXPORT15)"!=""
    @echo $(DEF5EXPORT15)										>>$@
.ENDIF
.IF "$(DEF5EXPORT16)"!=""
    @echo $(DEF5EXPORT16)										>>$@
.ENDIF
.IF "$(DEF5EXPORT17)"!=""
    @echo $(DEF5EXPORT17)										>>$@
.ENDIF
.IF "$(DEF5EXPORT18)"!=""
    @echo $(DEF5EXPORT18)										>>$@
.ENDIF
.IF "$(DEF5EXPORT19)"!=""
    @echo $(DEF5EXPORT19)										>>$@
.ENDIF
.IF "$(DEF5EXPORT20)"!=""
    @echo $(DEF5EXPORT20)										>>$@
.ENDIF
.IF "$(DEF5EXPORTFILE)"!=""
    +$(TYPE) $(DEF5EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP5HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF5TARGETN): \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
    @+echo ------------------------------
    @+echo Making Module-Definitionfile : $@
    @+echo just a dummy for UNIX
    @+echo > $@
.ENDIF
.ENDIF			# "$(DEF5TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(DEF6TARGETN)"!=""

#to make intuitiv naming work
.IF "$(DEF6LIBNAME)"!=""
DEFLIB6NAME*=$(DEF6LIBNAME)
.ENDIF			# "$(DEF6LIBNAME)"!=""

.IF "$(DEFLIB6NAME)"!=""
.IF "$(UPDATER)"!=""
DEF6DEPN+=$(SLB)$/$(DEFLIB6NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL6VERSIONMAP)"!=""
.IF "$(DEF6EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF6EXPORTFILE=$(MISC)$/$(SHL6VERSIONMAP:b)_$(SHL6TARGET).dxp
$(DEF6EXPORTFILE) : $(SHL6VERSIONMAP)
    +$(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF6EXPORTFILE)"==""
.ENDIF			# "$(SHL6VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB6NAME)"!=""

DEF6UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE6:=$(shell +echo %_disk)
#BUILD_DRIVE6:=O

.IF "$(BUILD_DRIVE6)"=="O"
# in case of RE build, protect against failed lock
EXPORT6_PROTECT=$(TMP)$/$(DEF6UNIQE:b).bat && 
.ENDIF			# "$(BUILD_DRIVE6)"=="O"

.ENDIF			# "$(DEFLIB6NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""

.IF "$(APP6HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF6TARGETN) : \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF6TARGETN) .PHONY : \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB6NAME)"!=""
.IF "$(BUILD_DRIVE6)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF6UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE6)"=="O"
.ENDIF				# "$(DEFLIB6NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
    +-attrib -r defs$/$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL6TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF6DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(NO_SHL6DESCRIPTION)"==""
    @echo component_getDescriptionFunc	>>$@
.ENDIF			# "$(NO_SHL6DESCRIPTION)"==""
.IF "$(DEFLIB6NAME)"!=""
    @+$(EXPORT6_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(SHL6TARGET).exp $(SLB)$/$(DEFLIB6NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF6CEXP)"!=""
    @+$(EXPORT6_PROTECT) $(LDUMP2) -A $(DEF6CEXP) -E 20 -F $(MISC)$/$(SHL6TARGET).flt $(SHL6TARGET).exp			   >>$@
.ELSE
    @+$(EXPORT6_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL6TARGET).flt $(SHL6TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @+$(EXPORT6_PROTECT) $(LDUMP) -E 20 -F$(MISC)$/$(SHL6TARGET).flt $(SHL6TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +$(EXPORT6_PROTECT) $(RM) $(SHL6TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(BUILD_SOSL)"==""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE6)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF6UNIQE:b) commit
    +$(TMP)$/$(DEF6UNIQE:b).bat && $(RM) $(TMP)$/$(DEF6UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE6)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
.ENDIF				# "$(DEFLIB6NAME)"!=""
.IF "$(DEF6EXPORT1)"!=""
    @echo $(DEF6EXPORT1)										>>$@
.ENDIF
.IF "$(DEF6EXPORT2)"!=""
    @echo $(DEF6EXPORT2)										>>$@
.ENDIF
.IF "$(DEF6EXPORT3)"!=""
    @echo $(DEF6EXPORT3)										>>$@
.ENDIF
.IF "$(DEF6EXPORT4)"!=""
    @echo $(DEF6EXPORT4)										>>$@
.ENDIF
.IF "$(DEF6EXPORT5)"!=""
    @echo $(DEF6EXPORT5)										>>$@
.ENDIF
.IF "$(DEF6EXPORT6)"!=""
    @echo $(DEF6EXPORT6)										>>$@
.ENDIF
.IF "$(DEF6EXPORT7)"!=""
    @echo $(DEF6EXPORT7)										>>$@
.ENDIF
.IF "$(DEF6EXPORT8)"!=""
    @echo $(DEF6EXPORT8)										>>$@
.ENDIF
.IF "$(DEF6EXPORT9)"!=""
    @echo $(DEF6EXPORT9)										>>$@
.ENDIF
.IF "$(DEF6EXPORT10)"!=""
    @echo $(DEF6EXPORT10)										>>$@
.ENDIF
.IF "$(DEF6EXPORT11)"!=""
    @echo $(DEF6EXPORT11)										>>$@
.ENDIF
.IF "$(DEF6EXPORT12)"!=""
    @echo $(DEF6EXPORT12)										>>$@
.ENDIF
.IF "$(DEF6EXPORT13)"!=""
    @echo $(DEF6EXPORT13)										>>$@
.ENDIF
.IF "$(DEF6EXPORT14)"!=""
    @echo $(DEF6EXPORT14)										>>$@
.ENDIF
.IF "$(DEF6EXPORT15)"!=""
    @echo $(DEF6EXPORT15)										>>$@
.ENDIF
.IF "$(DEF6EXPORT16)"!=""
    @echo $(DEF6EXPORT16)										>>$@
.ENDIF
.IF "$(DEF6EXPORT17)"!=""
    @echo $(DEF6EXPORT17)										>>$@
.ENDIF
.IF "$(DEF6EXPORT18)"!=""
    @echo $(DEF6EXPORT18)										>>$@
.ENDIF
.IF "$(DEF6EXPORT19)"!=""
    @echo $(DEF6EXPORT19)										>>$@
.ENDIF
.IF "$(DEF6EXPORT20)"!=""
    @echo $(DEF6EXPORT20)										>>$@
.ENDIF
.IF "$(DEF6EXPORTFILE)"!=""
    +$(TYPE) $(DEF6EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP6HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF6TARGETN): \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
    @+echo ------------------------------
    @+echo Making Module-Definitionfile : $@
    @+echo just a dummy for UNIX
    @+echo > $@
.ENDIF
.ENDIF			# "$(DEF6TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(DEF7TARGETN)"!=""

#to make intuitiv naming work
.IF "$(DEF7LIBNAME)"!=""
DEFLIB7NAME*=$(DEF7LIBNAME)
.ENDIF			# "$(DEF7LIBNAME)"!=""

.IF "$(DEFLIB7NAME)"!=""
.IF "$(UPDATER)"!=""
DEF7DEPN+=$(SLB)$/$(DEFLIB7NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL7VERSIONMAP)"!=""
.IF "$(DEF7EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF7EXPORTFILE=$(MISC)$/$(SHL7VERSIONMAP:b)_$(SHL7TARGET).dxp
$(DEF7EXPORTFILE) : $(SHL7VERSIONMAP)
    +$(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF7EXPORTFILE)"==""
.ENDIF			# "$(SHL7VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB7NAME)"!=""

DEF7UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE7:=$(shell +echo %_disk)
#BUILD_DRIVE7:=O

.IF "$(BUILD_DRIVE7)"=="O"
# in case of RE build, protect against failed lock
EXPORT7_PROTECT=$(TMP)$/$(DEF7UNIQE:b).bat && 
.ENDIF			# "$(BUILD_DRIVE7)"=="O"

.ENDIF			# "$(DEFLIB7NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""

.IF "$(APP7HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF7TARGETN) : \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF7TARGETN) .PHONY : \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB7NAME)"!=""
.IF "$(BUILD_DRIVE7)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF7UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE7)"=="O"
.ENDIF				# "$(DEFLIB7NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
    +-attrib -r defs$/$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL7TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF7DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(NO_SHL7DESCRIPTION)"==""
    @echo component_getDescriptionFunc	>>$@
.ENDIF			# "$(NO_SHL7DESCRIPTION)"==""
.IF "$(DEFLIB7NAME)"!=""
    @+$(EXPORT7_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(SHL7TARGET).exp $(SLB)$/$(DEFLIB7NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF7CEXP)"!=""
    @+$(EXPORT7_PROTECT) $(LDUMP2) -A $(DEF7CEXP) -E 20 -F $(MISC)$/$(SHL7TARGET).flt $(SHL7TARGET).exp			   >>$@
.ELSE
    @+$(EXPORT7_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL7TARGET).flt $(SHL7TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @+$(EXPORT7_PROTECT) $(LDUMP) -E 20 -F$(MISC)$/$(SHL7TARGET).flt $(SHL7TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +$(EXPORT7_PROTECT) $(RM) $(SHL7TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(BUILD_SOSL)"==""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE7)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF7UNIQE:b) commit
    +$(TMP)$/$(DEF7UNIQE:b).bat && $(RM) $(TMP)$/$(DEF7UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE7)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
.ENDIF				# "$(DEFLIB7NAME)"!=""
.IF "$(DEF7EXPORT1)"!=""
    @echo $(DEF7EXPORT1)										>>$@
.ENDIF
.IF "$(DEF7EXPORT2)"!=""
    @echo $(DEF7EXPORT2)										>>$@
.ENDIF
.IF "$(DEF7EXPORT3)"!=""
    @echo $(DEF7EXPORT3)										>>$@
.ENDIF
.IF "$(DEF7EXPORT4)"!=""
    @echo $(DEF7EXPORT4)										>>$@
.ENDIF
.IF "$(DEF7EXPORT5)"!=""
    @echo $(DEF7EXPORT5)										>>$@
.ENDIF
.IF "$(DEF7EXPORT6)"!=""
    @echo $(DEF7EXPORT6)										>>$@
.ENDIF
.IF "$(DEF7EXPORT7)"!=""
    @echo $(DEF7EXPORT7)										>>$@
.ENDIF
.IF "$(DEF7EXPORT8)"!=""
    @echo $(DEF7EXPORT8)										>>$@
.ENDIF
.IF "$(DEF7EXPORT9)"!=""
    @echo $(DEF7EXPORT9)										>>$@
.ENDIF
.IF "$(DEF7EXPORT10)"!=""
    @echo $(DEF7EXPORT10)										>>$@
.ENDIF
.IF "$(DEF7EXPORT11)"!=""
    @echo $(DEF7EXPORT11)										>>$@
.ENDIF
.IF "$(DEF7EXPORT12)"!=""
    @echo $(DEF7EXPORT12)										>>$@
.ENDIF
.IF "$(DEF7EXPORT13)"!=""
    @echo $(DEF7EXPORT13)										>>$@
.ENDIF
.IF "$(DEF7EXPORT14)"!=""
    @echo $(DEF7EXPORT14)										>>$@
.ENDIF
.IF "$(DEF7EXPORT15)"!=""
    @echo $(DEF7EXPORT15)										>>$@
.ENDIF
.IF "$(DEF7EXPORT16)"!=""
    @echo $(DEF7EXPORT16)										>>$@
.ENDIF
.IF "$(DEF7EXPORT17)"!=""
    @echo $(DEF7EXPORT17)										>>$@
.ENDIF
.IF "$(DEF7EXPORT18)"!=""
    @echo $(DEF7EXPORT18)										>>$@
.ENDIF
.IF "$(DEF7EXPORT19)"!=""
    @echo $(DEF7EXPORT19)										>>$@
.ENDIF
.IF "$(DEF7EXPORT20)"!=""
    @echo $(DEF7EXPORT20)										>>$@
.ENDIF
.IF "$(DEF7EXPORTFILE)"!=""
    +$(TYPE) $(DEF7EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP7HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF7TARGETN): \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
    @+echo ------------------------------
    @+echo Making Module-Definitionfile : $@
    @+echo just a dummy for UNIX
    @+echo > $@
.ENDIF
.ENDIF			# "$(DEF7TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(DEF8TARGETN)"!=""

#to make intuitiv naming work
.IF "$(DEF8LIBNAME)"!=""
DEFLIB8NAME*=$(DEF8LIBNAME)
.ENDIF			# "$(DEF8LIBNAME)"!=""

.IF "$(DEFLIB8NAME)"!=""
.IF "$(UPDATER)"!=""
DEF8DEPN+=$(SLB)$/$(DEFLIB8NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL8VERSIONMAP)"!=""
.IF "$(DEF8EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF8EXPORTFILE=$(MISC)$/$(SHL8VERSIONMAP:b)_$(SHL8TARGET).dxp
$(DEF8EXPORTFILE) : $(SHL8VERSIONMAP)
    +$(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF8EXPORTFILE)"==""
.ENDIF			# "$(SHL8VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB8NAME)"!=""

DEF8UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE8:=$(shell +echo %_disk)
#BUILD_DRIVE8:=O

.IF "$(BUILD_DRIVE8)"=="O"
# in case of RE build, protect against failed lock
EXPORT8_PROTECT=$(TMP)$/$(DEF8UNIQE:b).bat && 
.ENDIF			# "$(BUILD_DRIVE8)"=="O"

.ENDIF			# "$(DEFLIB8NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""

.IF "$(APP8HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF8TARGETN) : \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF8TARGETN) .PHONY : \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB8NAME)"!=""
.IF "$(BUILD_DRIVE8)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF8UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE8)"=="O"
.ENDIF				# "$(DEFLIB8NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
    +-attrib -r defs$/$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL8TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF8DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(NO_SHL8DESCRIPTION)"==""
    @echo component_getDescriptionFunc	>>$@
.ENDIF			# "$(NO_SHL8DESCRIPTION)"==""
.IF "$(DEFLIB8NAME)"!=""
    @+$(EXPORT8_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(SHL8TARGET).exp $(SLB)$/$(DEFLIB8NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF8CEXP)"!=""
    @+$(EXPORT8_PROTECT) $(LDUMP2) -A $(DEF8CEXP) -E 20 -F $(MISC)$/$(SHL8TARGET).flt $(SHL8TARGET).exp			   >>$@
.ELSE
    @+$(EXPORT8_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL8TARGET).flt $(SHL8TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @+$(EXPORT8_PROTECT) $(LDUMP) -E 20 -F$(MISC)$/$(SHL8TARGET).flt $(SHL8TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +$(EXPORT8_PROTECT) $(RM) $(SHL8TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(BUILD_SOSL)"==""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE8)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF8UNIQE:b) commit
    +$(TMP)$/$(DEF8UNIQE:b).bat && $(RM) $(TMP)$/$(DEF8UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE8)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
.ENDIF				# "$(DEFLIB8NAME)"!=""
.IF "$(DEF8EXPORT1)"!=""
    @echo $(DEF8EXPORT1)										>>$@
.ENDIF
.IF "$(DEF8EXPORT2)"!=""
    @echo $(DEF8EXPORT2)										>>$@
.ENDIF
.IF "$(DEF8EXPORT3)"!=""
    @echo $(DEF8EXPORT3)										>>$@
.ENDIF
.IF "$(DEF8EXPORT4)"!=""
    @echo $(DEF8EXPORT4)										>>$@
.ENDIF
.IF "$(DEF8EXPORT5)"!=""
    @echo $(DEF8EXPORT5)										>>$@
.ENDIF
.IF "$(DEF8EXPORT6)"!=""
    @echo $(DEF8EXPORT6)										>>$@
.ENDIF
.IF "$(DEF8EXPORT7)"!=""
    @echo $(DEF8EXPORT7)										>>$@
.ENDIF
.IF "$(DEF8EXPORT8)"!=""
    @echo $(DEF8EXPORT8)										>>$@
.ENDIF
.IF "$(DEF8EXPORT9)"!=""
    @echo $(DEF8EXPORT9)										>>$@
.ENDIF
.IF "$(DEF8EXPORT10)"!=""
    @echo $(DEF8EXPORT10)										>>$@
.ENDIF
.IF "$(DEF8EXPORT11)"!=""
    @echo $(DEF8EXPORT11)										>>$@
.ENDIF
.IF "$(DEF8EXPORT12)"!=""
    @echo $(DEF8EXPORT12)										>>$@
.ENDIF
.IF "$(DEF8EXPORT13)"!=""
    @echo $(DEF8EXPORT13)										>>$@
.ENDIF
.IF "$(DEF8EXPORT14)"!=""
    @echo $(DEF8EXPORT14)										>>$@
.ENDIF
.IF "$(DEF8EXPORT15)"!=""
    @echo $(DEF8EXPORT15)										>>$@
.ENDIF
.IF "$(DEF8EXPORT16)"!=""
    @echo $(DEF8EXPORT16)										>>$@
.ENDIF
.IF "$(DEF8EXPORT17)"!=""
    @echo $(DEF8EXPORT17)										>>$@
.ENDIF
.IF "$(DEF8EXPORT18)"!=""
    @echo $(DEF8EXPORT18)										>>$@
.ENDIF
.IF "$(DEF8EXPORT19)"!=""
    @echo $(DEF8EXPORT19)										>>$@
.ENDIF
.IF "$(DEF8EXPORT20)"!=""
    @echo $(DEF8EXPORT20)										>>$@
.ENDIF
.IF "$(DEF8EXPORTFILE)"!=""
    +$(TYPE) $(DEF8EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP8HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF8TARGETN): \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
    @+echo ------------------------------
    @+echo Making Module-Definitionfile : $@
    @+echo just a dummy for UNIX
    @+echo > $@
.ENDIF
.ENDIF			# "$(DEF8TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(DEF9TARGETN)"!=""

#to make intuitiv naming work
.IF "$(DEF9LIBNAME)"!=""
DEFLIB9NAME*=$(DEF9LIBNAME)
.ENDIF			# "$(DEF9LIBNAME)"!=""

.IF "$(DEFLIB9NAME)"!=""
.IF "$(UPDATER)"!=""
DEF9DEPN+=$(SLB)$/$(DEFLIB9NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL9VERSIONMAP)"!=""
.IF "$(DEF9EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF9EXPORTFILE=$(MISC)$/$(SHL9VERSIONMAP:b)_$(SHL9TARGET).dxp
$(DEF9EXPORTFILE) : $(SHL9VERSIONMAP)
    +$(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF9EXPORTFILE)"==""
.ENDIF			# "$(SHL9VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB9NAME)"!=""

DEF9UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE9:=$(shell +echo %_disk)
#BUILD_DRIVE9:=O

.IF "$(BUILD_DRIVE9)"=="O"
# in case of RE build, protect against failed lock
EXPORT9_PROTECT=$(TMP)$/$(DEF9UNIQE:b).bat && 
.ENDIF			# "$(BUILD_DRIVE9)"=="O"

.ENDIF			# "$(DEFLIB9NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""

.IF "$(APP9HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF9TARGETN) : \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF9TARGETN) .PHONY : \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB9NAME)"!=""
.IF "$(BUILD_DRIVE9)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF9UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE9)"=="O"
.ENDIF				# "$(DEFLIB9NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
    +-attrib -r defs$/$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL9TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF9DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(NO_SHL9DESCRIPTION)"==""
    @echo component_getDescriptionFunc	>>$@
.ENDIF			# "$(NO_SHL9DESCRIPTION)"==""
.IF "$(DEFLIB9NAME)"!=""
    @+$(EXPORT9_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(SHL9TARGET).exp $(SLB)$/$(DEFLIB9NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF9CEXP)"!=""
    @+$(EXPORT9_PROTECT) $(LDUMP2) -A $(DEF9CEXP) -E 20 -F $(MISC)$/$(SHL9TARGET).flt $(SHL9TARGET).exp			   >>$@
.ELSE
    @+$(EXPORT9_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL9TARGET).flt $(SHL9TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @+$(EXPORT9_PROTECT) $(LDUMP) -E 20 -F$(MISC)$/$(SHL9TARGET).flt $(SHL9TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +$(EXPORT9_PROTECT) $(RM) $(SHL9TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(BUILD_SOSL)"==""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE9)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF9UNIQE:b) commit
    +$(TMP)$/$(DEF9UNIQE:b).bat && $(RM) $(TMP)$/$(DEF9UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE9)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
.ENDIF				# "$(DEFLIB9NAME)"!=""
.IF "$(DEF9EXPORT1)"!=""
    @echo $(DEF9EXPORT1)										>>$@
.ENDIF
.IF "$(DEF9EXPORT2)"!=""
    @echo $(DEF9EXPORT2)										>>$@
.ENDIF
.IF "$(DEF9EXPORT3)"!=""
    @echo $(DEF9EXPORT3)										>>$@
.ENDIF
.IF "$(DEF9EXPORT4)"!=""
    @echo $(DEF9EXPORT4)										>>$@
.ENDIF
.IF "$(DEF9EXPORT5)"!=""
    @echo $(DEF9EXPORT5)										>>$@
.ENDIF
.IF "$(DEF9EXPORT6)"!=""
    @echo $(DEF9EXPORT6)										>>$@
.ENDIF
.IF "$(DEF9EXPORT7)"!=""
    @echo $(DEF9EXPORT7)										>>$@
.ENDIF
.IF "$(DEF9EXPORT8)"!=""
    @echo $(DEF9EXPORT8)										>>$@
.ENDIF
.IF "$(DEF9EXPORT9)"!=""
    @echo $(DEF9EXPORT9)										>>$@
.ENDIF
.IF "$(DEF9EXPORT10)"!=""
    @echo $(DEF9EXPORT10)										>>$@
.ENDIF
.IF "$(DEF9EXPORT11)"!=""
    @echo $(DEF9EXPORT11)										>>$@
.ENDIF
.IF "$(DEF9EXPORT12)"!=""
    @echo $(DEF9EXPORT12)										>>$@
.ENDIF
.IF "$(DEF9EXPORT13)"!=""
    @echo $(DEF9EXPORT13)										>>$@
.ENDIF
.IF "$(DEF9EXPORT14)"!=""
    @echo $(DEF9EXPORT14)										>>$@
.ENDIF
.IF "$(DEF9EXPORT15)"!=""
    @echo $(DEF9EXPORT15)										>>$@
.ENDIF
.IF "$(DEF9EXPORT16)"!=""
    @echo $(DEF9EXPORT16)										>>$@
.ENDIF
.IF "$(DEF9EXPORT17)"!=""
    @echo $(DEF9EXPORT17)										>>$@
.ENDIF
.IF "$(DEF9EXPORT18)"!=""
    @echo $(DEF9EXPORT18)										>>$@
.ENDIF
.IF "$(DEF9EXPORT19)"!=""
    @echo $(DEF9EXPORT19)										>>$@
.ENDIF
.IF "$(DEF9EXPORT20)"!=""
    @echo $(DEF9EXPORT20)										>>$@
.ENDIF
.IF "$(DEF9EXPORTFILE)"!=""
    +$(TYPE) $(DEF9EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP9HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF9TARGETN): \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
    @+echo ------------------------------
    @+echo Making Module-Definitionfile : $@
    @+echo just a dummy for UNIX
    @+echo > $@
.ENDIF
.ENDIF			# "$(DEF9TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(DEF10TARGETN)"!=""

#to make intuitiv naming work
.IF "$(DEF10LIBNAME)"!=""
DEFLIB10NAME*=$(DEF10LIBNAME)
.ENDIF			# "$(DEF10LIBNAME)"!=""

.IF "$(DEFLIB10NAME)"!=""
.IF "$(UPDATER)"!=""
DEF10DEPN+=$(SLB)$/$(DEFLIB10NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL10VERSIONMAP)"!=""
.IF "$(DEF10EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF10EXPORTFILE=$(MISC)$/$(SHL10VERSIONMAP:b)_$(SHL10TARGET).dxp
$(DEF10EXPORTFILE) : $(SHL10VERSIONMAP)
    +$(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF10EXPORTFILE)"==""
.ENDIF			# "$(SHL10VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB10NAME)"!=""

DEF10UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE10:=$(shell +echo %_disk)
#BUILD_DRIVE10:=O

.IF "$(BUILD_DRIVE10)"=="O"
# in case of RE build, protect against failed lock
EXPORT10_PROTECT=$(TMP)$/$(DEF10UNIQE:b).bat && 
.ENDIF			# "$(BUILD_DRIVE10)"=="O"

.ENDIF			# "$(DEFLIB10NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""

.IF "$(APP10HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF10TARGETN) : \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF10TARGETN) .PHONY : \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
.IF "$(BSCLIENT)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB10NAME)"!=""
.IF "$(BUILD_DRIVE10)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF10UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE10)"=="O"
.ENDIF				# "$(DEFLIB10NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
    +-attrib -r defs$/$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL10TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF10DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(NO_SHL10DESCRIPTION)"==""
    @echo component_getDescriptionFunc	>>$@
.ENDIF			# "$(NO_SHL10DESCRIPTION)"==""
.IF "$(DEFLIB10NAME)"!=""
    @+$(EXPORT10_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(SHL10TARGET).exp $(SLB)$/$(DEFLIB10NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF10CEXP)"!=""
    @+$(EXPORT10_PROTECT) $(LDUMP2) -A $(DEF10CEXP) -E 20 -F $(MISC)$/$(SHL10TARGET).flt $(SHL10TARGET).exp			   >>$@
.ELSE
    @+$(EXPORT10_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL10TARGET).flt $(SHL10TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @+$(EXPORT10_PROTECT) $(LDUMP) -E 20 -F$(MISC)$/$(SHL10TARGET).flt $(SHL10TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +$(EXPORT10_PROTECT) $(RM) $(SHL10TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(BUILD_SOSL)"==""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE10)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF10UNIQE:b) commit
    +$(TMP)$/$(DEF10UNIQE:b).bat && $(RM) $(TMP)$/$(DEF10UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE10)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(BUILD_SOSL)"==""
.ENDIF				# "$(DEFLIB10NAME)"!=""
.IF "$(DEF10EXPORT1)"!=""
    @echo $(DEF10EXPORT1)										>>$@
.ENDIF
.IF "$(DEF10EXPORT2)"!=""
    @echo $(DEF10EXPORT2)										>>$@
.ENDIF
.IF "$(DEF10EXPORT3)"!=""
    @echo $(DEF10EXPORT3)										>>$@
.ENDIF
.IF "$(DEF10EXPORT4)"!=""
    @echo $(DEF10EXPORT4)										>>$@
.ENDIF
.IF "$(DEF10EXPORT5)"!=""
    @echo $(DEF10EXPORT5)										>>$@
.ENDIF
.IF "$(DEF10EXPORT6)"!=""
    @echo $(DEF10EXPORT6)										>>$@
.ENDIF
.IF "$(DEF10EXPORT7)"!=""
    @echo $(DEF10EXPORT7)										>>$@
.ENDIF
.IF "$(DEF10EXPORT8)"!=""
    @echo $(DEF10EXPORT8)										>>$@
.ENDIF
.IF "$(DEF10EXPORT9)"!=""
    @echo $(DEF10EXPORT9)										>>$@
.ENDIF
.IF "$(DEF10EXPORT10)"!=""
    @echo $(DEF10EXPORT10)										>>$@
.ENDIF
.IF "$(DEF10EXPORT11)"!=""
    @echo $(DEF10EXPORT11)										>>$@
.ENDIF
.IF "$(DEF10EXPORT12)"!=""
    @echo $(DEF10EXPORT12)										>>$@
.ENDIF
.IF "$(DEF10EXPORT13)"!=""
    @echo $(DEF10EXPORT13)										>>$@
.ENDIF
.IF "$(DEF10EXPORT14)"!=""
    @echo $(DEF10EXPORT14)										>>$@
.ENDIF
.IF "$(DEF10EXPORT15)"!=""
    @echo $(DEF10EXPORT15)										>>$@
.ENDIF
.IF "$(DEF10EXPORT16)"!=""
    @echo $(DEF10EXPORT16)										>>$@
.ENDIF
.IF "$(DEF10EXPORT17)"!=""
    @echo $(DEF10EXPORT17)										>>$@
.ENDIF
.IF "$(DEF10EXPORT18)"!=""
    @echo $(DEF10EXPORT18)										>>$@
.ENDIF
.IF "$(DEF10EXPORT19)"!=""
    @echo $(DEF10EXPORT19)										>>$@
.ENDIF
.IF "$(DEF10EXPORT20)"!=""
    @echo $(DEF10EXPORT20)										>>$@
.ENDIF
.IF "$(DEF10EXPORTFILE)"!=""
    +$(TYPE) $(DEF10EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP10HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF10TARGETN): \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
    @+echo ------------------------------
    @+echo Making Module-Definitionfile : $@
    @+echo just a dummy for UNIX
    @+echo > $@
.ENDIF
.ENDIF			# "$(DEF10TARGETN)"!=""


# Anweisungen fuer das Linken
