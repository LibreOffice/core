# unroll begin

.IF "$(DEF1TARGETN)"!=""

.IF "$(DEFLIB1NAME)"!=""
.IF "$(UPDATER)"!=""
DEF1DEPN+=$(SLB)$/$(DEFLIB1NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WNT"
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
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl update $(DEFSTAG)
.ENDIF			# "$(shell +echo %_disk)"=="O"
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL1TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF1DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB1NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL1TARGET).exp $(SLB)\$(DEFLIB1NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF1CEXP)"!=""
    @$(LDUMP2) -A $(DEF1CEXP) -E 20 -F $(MISC)\$(SHL1TARGET).flt $(SHL1TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL1TARGET).flt $(SHL1TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL1TARGET).flt $(SHL1TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL1TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl commit
.ENDIF			# "$(shell +echo %_disk)"=="O"
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
    $(TYPE) $(DEF1EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP1HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF1TARGETN): \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo just a dummy for UNIX
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF1TARGETN): \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF1TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF2TARGETN)"!=""

.IF "$(DEFLIB2NAME)"!=""
.IF "$(UPDATER)"!=""
DEF2DEPN+=$(SLB)$/$(DEFLIB2NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WNT"
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
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl update $(DEFSTAG)
.ENDIF			# "$(shell +echo %_disk)"=="O"
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL2TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF2DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB2NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL2TARGET).exp $(SLB)\$(DEFLIB2NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF2CEXP)"!=""
    @$(LDUMP2) -A $(DEF2CEXP) -E 20 -F $(MISC)\$(SHL2TARGET).flt $(SHL2TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL2TARGET).flt $(SHL2TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL2TARGET).flt $(SHL2TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL2TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl commit
.ENDIF			# "$(shell +echo %_disk)"=="O"
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
    $(TYPE) $(DEF2EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP2HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF2TARGETN): \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo just a dummy for UNIX
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF2TARGETN): \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF2TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF3TARGETN)"!=""

.IF "$(DEFLIB3NAME)"!=""
.IF "$(UPDATER)"!=""
DEF3DEPN+=$(SLB)$/$(DEFLIB3NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WNT"
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
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl update $(DEFSTAG)
.ENDIF			# "$(shell +echo %_disk)"=="O"
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL3TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF3DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB3NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL3TARGET).exp $(SLB)\$(DEFLIB3NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF3CEXP)"!=""
    @$(LDUMP2) -A $(DEF3CEXP) -E 20 -F $(MISC)\$(SHL3TARGET).flt $(SHL3TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL3TARGET).flt $(SHL3TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL3TARGET).flt $(SHL3TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL3TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl commit
.ENDIF			# "$(shell +echo %_disk)"=="O"
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
    $(TYPE) $(DEF3EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP3HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF3TARGETN): \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo just a dummy for UNIX
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF3TARGETN): \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF3TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF4TARGETN)"!=""

.IF "$(DEFLIB4NAME)"!=""
.IF "$(UPDATER)"!=""
DEF4DEPN+=$(SLB)$/$(DEFLIB4NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WNT"
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
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl update $(DEFSTAG)
.ENDIF			# "$(shell +echo %_disk)"=="O"
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL4TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF4DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB4NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL4TARGET).exp $(SLB)\$(DEFLIB4NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF4CEXP)"!=""
    @$(LDUMP2) -A $(DEF4CEXP) -E 20 -F $(MISC)\$(SHL4TARGET).flt $(SHL4TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL4TARGET).flt $(SHL4TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL4TARGET).flt $(SHL4TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL4TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl commit
.ENDIF			# "$(shell +echo %_disk)"=="O"
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
    $(TYPE) $(DEF4EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP4HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF4TARGETN): \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo just a dummy for UNIX
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF4TARGETN): \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF4TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF5TARGETN)"!=""

.IF "$(DEFLIB5NAME)"!=""
.IF "$(UPDATER)"!=""
DEF5DEPN+=$(SLB)$/$(DEFLIB5NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WNT"
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
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl update $(DEFSTAG)
.ENDIF			# "$(shell +echo %_disk)"=="O"
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL5TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF5DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB5NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL5TARGET).exp $(SLB)\$(DEFLIB5NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF5CEXP)"!=""
    @$(LDUMP2) -A $(DEF5CEXP) -E 20 -F $(MISC)\$(SHL5TARGET).flt $(SHL5TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL5TARGET).flt $(SHL5TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL5TARGET).flt $(SHL5TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL5TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl commit
.ENDIF			# "$(shell +echo %_disk)"=="O"
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
    $(TYPE) $(DEF5EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP5HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF5TARGETN): \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo just a dummy for UNIX
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF5TARGETN): \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF5TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF6TARGETN)"!=""

.IF "$(DEFLIB6NAME)"!=""
.IF "$(UPDATER)"!=""
DEF6DEPN+=$(SLB)$/$(DEFLIB6NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WNT"
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
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl update $(DEFSTAG)
.ENDIF			# "$(shell +echo %_disk)"=="O"
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL6TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF6DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB6NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL6TARGET).exp $(SLB)\$(DEFLIB6NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF6CEXP)"!=""
    @$(LDUMP2) -A $(DEF6CEXP) -E 20 -F $(MISC)\$(SHL6TARGET).flt $(SHL6TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL6TARGET).flt $(SHL6TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL6TARGET).flt $(SHL6TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL6TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl commit
.ENDIF			# "$(shell +echo %_disk)"=="O"
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
    $(TYPE) $(DEF6EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP6HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF6TARGETN): \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo just a dummy for UNIX
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF6TARGETN): \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF6TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF7TARGETN)"!=""

.IF "$(DEFLIB7NAME)"!=""
.IF "$(UPDATER)"!=""
DEF7DEPN+=$(SLB)$/$(DEFLIB7NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WNT"
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
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl update $(DEFSTAG)
.ENDIF			# "$(shell +echo %_disk)"=="O"
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL7TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF7DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB7NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL7TARGET).exp $(SLB)\$(DEFLIB7NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF7CEXP)"!=""
    @$(LDUMP2) -A $(DEF7CEXP) -E 20 -F $(MISC)\$(SHL7TARGET).flt $(SHL7TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL7TARGET).flt $(SHL7TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL7TARGET).flt $(SHL7TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL7TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl commit
.ENDIF			# "$(shell +echo %_disk)"=="O"
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
    $(TYPE) $(DEF7EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP7HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF7TARGETN): \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo just a dummy for UNIX
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF7TARGETN): \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF7TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF8TARGETN)"!=""

.IF "$(DEFLIB8NAME)"!=""
.IF "$(UPDATER)"!=""
DEF8DEPN+=$(SLB)$/$(DEFLIB8NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WNT"
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
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl update $(DEFSTAG)
.ENDIF			# "$(shell +echo %_disk)"=="O"
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL8TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF8DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB8NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL8TARGET).exp $(SLB)\$(DEFLIB8NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF8CEXP)"!=""
    @$(LDUMP2) -A $(DEF8CEXP) -E 20 -F $(MISC)\$(SHL8TARGET).flt $(SHL8TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL8TARGET).flt $(SHL8TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL8TARGET).flt $(SHL8TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL8TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl commit
.ENDIF			# "$(shell +echo %_disk)"=="O"
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
    $(TYPE) $(DEF8EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP8HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF8TARGETN): \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo just a dummy for UNIX
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF8TARGETN): \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF8TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF9TARGETN)"!=""

.IF "$(DEFLIB9NAME)"!=""
.IF "$(UPDATER)"!=""
DEF9DEPN+=$(SLB)$/$(DEFLIB9NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WNT"
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
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl update $(DEFSTAG)
.ENDIF			# "$(shell +echo %_disk)"=="O"
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL9TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF9DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB9NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL9TARGET).exp $(SLB)\$(DEFLIB9NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF9CEXP)"!=""
    @$(LDUMP2) -A $(DEF9CEXP) -E 20 -F $(MISC)\$(SHL9TARGET).flt $(SHL9TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL9TARGET).flt $(SHL9TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL9TARGET).flt $(SHL9TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL9TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl commit
.ENDIF			# "$(shell +echo %_disk)"=="O"
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
    $(TYPE) $(DEF9EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP9HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF9TARGETN): \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo just a dummy for UNIX
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF9TARGETN): \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF9TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(DEF10TARGETN)"!=""

.IF "$(DEFLIB10NAME)"!=""
.IF "$(UPDATER)"!=""
DEF10DEPN+=$(SLB)$/$(DEFLIB10NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WNT"
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
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl update $(DEFSTAG)
.ENDIF			# "$(shell +echo %_disk)"=="O"
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL10TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF10DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB10NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL10TARGET).exp $(SLB)\$(DEFLIB10NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF10CEXP)"!=""
    @$(LDUMP2) -A $(DEF10CEXP) -E 20 -F $(MISC)\$(SHL10TARGET).flt $(SHL10TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL10TARGET).flt $(SHL10TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL10TARGET).flt $(SHL10TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL10TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
# %_disk is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell +echo %_disk)"=="O"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl commit
.ENDIF			# "$(shell +echo %_disk)"=="O"
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
    $(TYPE) $(DEF10EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP10HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF10TARGETN): \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo just a dummy for UNIX
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF10TARGETN): \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF10TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
