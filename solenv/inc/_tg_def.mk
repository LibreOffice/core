# unroll begin

.IF "$(DEF1TARGETN)"!=""

#to make intuitiv naming work
.IF "$(DEF1LIBNAME)"!=""
DEFLIB1NAME*=$(DEF1LIBNAME)
.ENDIF			# "$(DEF1LIBNAME)"!=""

.IF "$(DEFLIB1NAME)"!=""
.IF "$(UPDATER)"!=""
DEF1DEPN+=$(foreach,i,$(DEFLIB1NAME) $(SLB)$/$(i).lib)
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL1VERSIONMAP)"!=""
.IF "$(DEF1EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF1EXPORTFILE=$(MISC)$/$(SHL1VERSIONMAP:b)_$(SHL1TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF1EXPORTFILE) : $(SHL1OBJS) $(SHL1LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF1EXPORTFILE) : $(SHL1VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    -grep -v "\*\|?" $@ > $@.exported-symbols
    -grep "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL1OBJS)"!=""
    -echo $(foreach,i,$(SHL1OBJS) $i) | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL1LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL1LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF1EXPORTFILE)"==""
.ENDIF			# "$(SHL1VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF1FILTER=$(SOLARENV)$/inc$/dummy.flt

RMHACK1:=$(RM)
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB1NAME)"!=""

DEF1UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE1:=$(shell @echo %_disk)
#BUILD_DRIVE1:=O

.IF "$(BUILD_DRIVE1)"=="O"
# in case of RE build, protect against failed lock
EXPORT1_PROTECT=$(TMP)$/$(DEF1UNIQE:b).bat &&
RMHACK1:=$(RM:s/+//)
.ELSE			# "$(BUILD_DRIVE1)"=="O"
.ENDIF			# "$(BUILD_DRIVE1)"=="O"

.ENDIF			# "$(DEFLIB1NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""

.IF "$(link_always)"==""
$(DEF1TARGETN) : \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF1TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB1NAME)"!=""
.IF "$(BUILD_DRIVE1)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF1UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE1)"=="O"
.ENDIF				# "$(DEFLIB1NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
#	-attrib -r defs$/$(OUTPATH)
    @-$(RM) $@.tmpfile
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(EMQ)"$(SHL1TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.IF "$(DEFLIB1NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(EXPORT1_PROTECT) $(RM) $(MISC)$/$(SHL1TARGET).exp
    dlltool --output-def $(MISC)$/$(SHL1TARGET).exp --export-all-symbols \
         `$(TYPE) $(foreach,i,$(DEFLIB1NAME) $(SLB)$/$(i).lib) | sed s#$(ROUT)#$(PRJ)$/$(ROUT)#g`
    tail --lines +3 $(MISC)$/$(SHL1TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(EXPORT1_PROTECT) $(RM) $(MISC)$/$(SHL1TARGET).exp
.ELSE
.IF "$(SHL1USE_EXPORTS)"==""
    @-$(EXPORT1_PROTECT) $(RMHACK1) $(MISC)$/$(SHL1TARGET).exp
    @$(EXPORT1_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(MISC)$/$(SHL1TARGET).exp $(SLB)$/$(DEFLIB1NAME).lib
.IF "$(DEF1CEXP)"!=""
    @$(EXPORT1_PROTECT) $(LDUMP2) -A $(DEF1CEXP) -E 20 -F $(MISC)$/$(SHL1TARGET).flt $(MISC)$/$(SHL1TARGET).exp			   >>$@.tmpfile
.ELSE
    @$(EXPORT1_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL1TARGET).flt $(MISC)$/$(SHL1TARGET).exp			   >>$@.tmpfile
.ENDIF
    $(EXPORT1_PROTECT) $(RMHACK1) $(MISC)$/$(SHL1TARGET).exp
.ELSE			# "$(SHL1USE_EXPORTS)"==""
    @$(EXPORT1_PROTECT) $(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB1NAME) $(SLB)$/$(i).lib) | $(GREP) EXPORT: > $(MISC)$/$(SHL1TARGET).direct
.IF "$(SHL1USE_EXPORTS)"!="name"
.IF "$(DEF1CEXP)"!=""
    @$(EXPORT1_PROTECT) $(LDUMP2) -D -A $(DEF1CEXP) -E 20 -F $(DEF1FILTER) $(MISC)$/$(SHL1TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT1_PROTECT) $(LDUMP2) -D -E 20 -F $(DEF1FILTER) $(MISC)$/$(SHL1TARGET).direct >>$@.tmpfile
.ENDIF
.ELSE			# "$(SHL1USE_EXPORTS)"!="name"
.IF "$(DEF1CEXP)"!=""
    @$(EXPORT1_PROTECT) $(LDUMP2) -N -D -A $(DEF1CEXP) -E 20 -F $(DEF1FILTER) $(MISC)$/$(SHL1TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT1_PROTECT) $(LDUMP2) -N -D -E 20 -F $(DEF1FILTER) $(MISC)$/$(SHL1TARGET).direct >>$@.tmpfile
.ENDIF
.ENDIF			# "$(SHL1USE_EXPORTS)"!="name"
.ENDIF			# "$(SHL1USE_EXPORTS)"==""
.ENDIF
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE1)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF1UNIQE:b) commit
    $(4nt_force_shell)$(TMP)$/$(DEF1UNIQE:b).bat && $(RM:s/+//) $(TMP)$/$(DEF1UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE1)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
.ENDIF				# "$(DEFLIB1NAME)"!=""
.IF "$(DEF1EXPORT1)"!=""
    @echo $(DEF1EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT2)"!=""
    @echo $(DEF1EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT3)"!=""
    @echo $(DEF1EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT4)"!=""
    @echo $(DEF1EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT5)"!=""
    @echo $(DEF1EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT6)"!=""
    @echo $(DEF1EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT7)"!=""
    @echo $(DEF1EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT8)"!=""
    @echo $(DEF1EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT9)"!=""
    @echo $(DEF1EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT10)"!=""
    @echo $(DEF1EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT11)"!=""
    @echo $(DEF1EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT12)"!=""
    @echo $(DEF1EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT13)"!=""
    @echo $(DEF1EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT14)"!=""
    @echo $(DEF1EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT15)"!=""
    @echo $(DEF1EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT16)"!=""
    @echo $(DEF1EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT17)"!=""
    @echo $(DEF1EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT18)"!=""
    @echo $(DEF1EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT19)"!=""
    @echo $(DEF1EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT20)"!=""
    @echo $(DEF1EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(TYPE) $(DEF1EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(TYPE) $(DEF1EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
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
DEF2DEPN+=$(foreach,i,$(DEFLIB2NAME) $(SLB)$/$(i).lib)
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL2VERSIONMAP)"!=""
.IF "$(DEF2EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF2EXPORTFILE=$(MISC)$/$(SHL2VERSIONMAP:b)_$(SHL2TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF2EXPORTFILE) : $(SHL2OBJS) $(SHL2LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF2EXPORTFILE) : $(SHL2VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    -grep -v "\*\|?" $@ > $@.exported-symbols
    -grep "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL2OBJS)"!=""
    -echo $(foreach,i,$(SHL2OBJS) $i) | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL2LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL2LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF2EXPORTFILE)"==""
.ENDIF			# "$(SHL2VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF2FILTER=$(SOLARENV)$/inc$/dummy.flt

RMHACK2:=$(RM)
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB2NAME)"!=""

DEF2UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE2:=$(shell @echo %_disk)
#BUILD_DRIVE2:=O

.IF "$(BUILD_DRIVE2)"=="O"
# in case of RE build, protect against failed lock
EXPORT2_PROTECT=$(TMP)$/$(DEF2UNIQE:b).bat &&
RMHACK2:=$(RM:s/+//)
.ELSE			# "$(BUILD_DRIVE2)"=="O"
.ENDIF			# "$(BUILD_DRIVE2)"=="O"

.ENDIF			# "$(DEFLIB2NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""

.IF "$(link_always)"==""
$(DEF2TARGETN) : \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF2TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB2NAME)"!=""
.IF "$(BUILD_DRIVE2)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF2UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE2)"=="O"
.ENDIF				# "$(DEFLIB2NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
#	-attrib -r defs$/$(OUTPATH)
    @-$(RM) $@.tmpfile
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(EMQ)"$(SHL2TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.IF "$(DEFLIB2NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(EXPORT2_PROTECT) $(RM) $(MISC)$/$(SHL2TARGET).exp
    dlltool --output-def $(MISC)$/$(SHL2TARGET).exp --export-all-symbols \
         `$(TYPE) $(foreach,i,$(DEFLIB2NAME) $(SLB)$/$(i).lib) | sed s#$(ROUT)#$(PRJ)$/$(ROUT)#g`
    tail --lines +3 $(MISC)$/$(SHL2TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(EXPORT2_PROTECT) $(RM) $(MISC)$/$(SHL2TARGET).exp
.ELSE
.IF "$(SHL2USE_EXPORTS)"==""
    @-$(EXPORT2_PROTECT) $(RMHACK2) $(MISC)$/$(SHL2TARGET).exp
    @$(EXPORT2_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(MISC)$/$(SHL2TARGET).exp $(SLB)$/$(DEFLIB2NAME).lib
.IF "$(DEF2CEXP)"!=""
    @$(EXPORT2_PROTECT) $(LDUMP2) -A $(DEF2CEXP) -E 20 -F $(MISC)$/$(SHL2TARGET).flt $(MISC)$/$(SHL2TARGET).exp			   >>$@.tmpfile
.ELSE
    @$(EXPORT2_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL2TARGET).flt $(MISC)$/$(SHL2TARGET).exp			   >>$@.tmpfile
.ENDIF
    $(EXPORT2_PROTECT) $(RMHACK2) $(MISC)$/$(SHL2TARGET).exp
.ELSE			# "$(SHL2USE_EXPORTS)"==""
    @$(EXPORT2_PROTECT) $(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB2NAME) $(SLB)$/$(i).lib) | $(GREP) EXPORT: > $(MISC)$/$(SHL2TARGET).direct
.IF "$(SHL2USE_EXPORTS)"!="name"
.IF "$(DEF2CEXP)"!=""
    @$(EXPORT2_PROTECT) $(LDUMP2) -D -A $(DEF2CEXP) -E 20 -F $(DEF2FILTER) $(MISC)$/$(SHL2TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT2_PROTECT) $(LDUMP2) -D -E 20 -F $(DEF2FILTER) $(MISC)$/$(SHL2TARGET).direct >>$@.tmpfile
.ENDIF
.ELSE			# "$(SHL2USE_EXPORTS)"!="name"
.IF "$(DEF2CEXP)"!=""
    @$(EXPORT2_PROTECT) $(LDUMP2) -N -D -A $(DEF2CEXP) -E 20 -F $(DEF2FILTER) $(MISC)$/$(SHL2TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT2_PROTECT) $(LDUMP2) -N -D -E 20 -F $(DEF2FILTER) $(MISC)$/$(SHL2TARGET).direct >>$@.tmpfile
.ENDIF
.ENDIF			# "$(SHL2USE_EXPORTS)"!="name"
.ENDIF			# "$(SHL2USE_EXPORTS)"==""
.ENDIF
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE2)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF2UNIQE:b) commit
    $(4nt_force_shell)$(TMP)$/$(DEF2UNIQE:b).bat && $(RM:s/+//) $(TMP)$/$(DEF2UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE2)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
.ENDIF				# "$(DEFLIB2NAME)"!=""
.IF "$(DEF2EXPORT1)"!=""
    @echo $(DEF2EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT2)"!=""
    @echo $(DEF2EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT3)"!=""
    @echo $(DEF2EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT4)"!=""
    @echo $(DEF2EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT5)"!=""
    @echo $(DEF2EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT6)"!=""
    @echo $(DEF2EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT7)"!=""
    @echo $(DEF2EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT8)"!=""
    @echo $(DEF2EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT9)"!=""
    @echo $(DEF2EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT10)"!=""
    @echo $(DEF2EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT11)"!=""
    @echo $(DEF2EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT12)"!=""
    @echo $(DEF2EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT13)"!=""
    @echo $(DEF2EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT14)"!=""
    @echo $(DEF2EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT15)"!=""
    @echo $(DEF2EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT16)"!=""
    @echo $(DEF2EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT17)"!=""
    @echo $(DEF2EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT18)"!=""
    @echo $(DEF2EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT19)"!=""
    @echo $(DEF2EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT20)"!=""
    @echo $(DEF2EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(TYPE) $(DEF2EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(TYPE) $(DEF2EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
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
DEF3DEPN+=$(foreach,i,$(DEFLIB3NAME) $(SLB)$/$(i).lib)
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL3VERSIONMAP)"!=""
.IF "$(DEF3EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF3EXPORTFILE=$(MISC)$/$(SHL3VERSIONMAP:b)_$(SHL3TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF3EXPORTFILE) : $(SHL3OBJS) $(SHL3LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF3EXPORTFILE) : $(SHL3VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    -grep -v "\*\|?" $@ > $@.exported-symbols
    -grep "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL3OBJS)"!=""
    -echo $(foreach,i,$(SHL3OBJS) $i) | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL3LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL3LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF3EXPORTFILE)"==""
.ENDIF			# "$(SHL3VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF3FILTER=$(SOLARENV)$/inc$/dummy.flt

RMHACK3:=$(RM)
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB3NAME)"!=""

DEF3UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE3:=$(shell @echo %_disk)
#BUILD_DRIVE3:=O

.IF "$(BUILD_DRIVE3)"=="O"
# in case of RE build, protect against failed lock
EXPORT3_PROTECT=$(TMP)$/$(DEF3UNIQE:b).bat &&
RMHACK3:=$(RM:s/+//)
.ELSE			# "$(BUILD_DRIVE3)"=="O"
.ENDIF			# "$(BUILD_DRIVE3)"=="O"

.ENDIF			# "$(DEFLIB3NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""

.IF "$(link_always)"==""
$(DEF3TARGETN) : \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF3TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB3NAME)"!=""
.IF "$(BUILD_DRIVE3)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF3UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE3)"=="O"
.ENDIF				# "$(DEFLIB3NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
#	-attrib -r defs$/$(OUTPATH)
    @-$(RM) $@.tmpfile
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(EMQ)"$(SHL3TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.IF "$(DEFLIB3NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(EXPORT3_PROTECT) $(RM) $(MISC)$/$(SHL3TARGET).exp
    dlltool --output-def $(MISC)$/$(SHL3TARGET).exp --export-all-symbols \
         `$(TYPE) $(foreach,i,$(DEFLIB3NAME) $(SLB)$/$(i).lib) | sed s#$(ROUT)#$(PRJ)$/$(ROUT)#g`
    tail --lines +3 $(MISC)$/$(SHL3TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(EXPORT3_PROTECT) $(RM) $(MISC)$/$(SHL3TARGET).exp
.ELSE
.IF "$(SHL3USE_EXPORTS)"==""
    @-$(EXPORT3_PROTECT) $(RMHACK3) $(MISC)$/$(SHL3TARGET).exp
    @$(EXPORT3_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(MISC)$/$(SHL3TARGET).exp $(SLB)$/$(DEFLIB3NAME).lib
.IF "$(DEF3CEXP)"!=""
    @$(EXPORT3_PROTECT) $(LDUMP2) -A $(DEF3CEXP) -E 20 -F $(MISC)$/$(SHL3TARGET).flt $(MISC)$/$(SHL3TARGET).exp			   >>$@.tmpfile
.ELSE
    @$(EXPORT3_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL3TARGET).flt $(MISC)$/$(SHL3TARGET).exp			   >>$@.tmpfile
.ENDIF
    $(EXPORT3_PROTECT) $(RMHACK3) $(MISC)$/$(SHL3TARGET).exp
.ELSE			# "$(SHL3USE_EXPORTS)"==""
    @$(EXPORT3_PROTECT) $(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB3NAME) $(SLB)$/$(i).lib) | $(GREP) EXPORT: > $(MISC)$/$(SHL3TARGET).direct
.IF "$(SHL3USE_EXPORTS)"!="name"
.IF "$(DEF3CEXP)"!=""
    @$(EXPORT3_PROTECT) $(LDUMP2) -D -A $(DEF3CEXP) -E 20 -F $(DEF3FILTER) $(MISC)$/$(SHL3TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT3_PROTECT) $(LDUMP2) -D -E 20 -F $(DEF3FILTER) $(MISC)$/$(SHL3TARGET).direct >>$@.tmpfile
.ENDIF
.ELSE			# "$(SHL3USE_EXPORTS)"!="name"
.IF "$(DEF3CEXP)"!=""
    @$(EXPORT3_PROTECT) $(LDUMP2) -N -D -A $(DEF3CEXP) -E 20 -F $(DEF3FILTER) $(MISC)$/$(SHL3TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT3_PROTECT) $(LDUMP2) -N -D -E 20 -F $(DEF3FILTER) $(MISC)$/$(SHL3TARGET).direct >>$@.tmpfile
.ENDIF
.ENDIF			# "$(SHL3USE_EXPORTS)"!="name"
.ENDIF			# "$(SHL3USE_EXPORTS)"==""
.ENDIF
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE3)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF3UNIQE:b) commit
    $(4nt_force_shell)$(TMP)$/$(DEF3UNIQE:b).bat && $(RM:s/+//) $(TMP)$/$(DEF3UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE3)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
.ENDIF				# "$(DEFLIB3NAME)"!=""
.IF "$(DEF3EXPORT1)"!=""
    @echo $(DEF3EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT2)"!=""
    @echo $(DEF3EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT3)"!=""
    @echo $(DEF3EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT4)"!=""
    @echo $(DEF3EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT5)"!=""
    @echo $(DEF3EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT6)"!=""
    @echo $(DEF3EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT7)"!=""
    @echo $(DEF3EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT8)"!=""
    @echo $(DEF3EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT9)"!=""
    @echo $(DEF3EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT10)"!=""
    @echo $(DEF3EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT11)"!=""
    @echo $(DEF3EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT12)"!=""
    @echo $(DEF3EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT13)"!=""
    @echo $(DEF3EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT14)"!=""
    @echo $(DEF3EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT15)"!=""
    @echo $(DEF3EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT16)"!=""
    @echo $(DEF3EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT17)"!=""
    @echo $(DEF3EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT18)"!=""
    @echo $(DEF3EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT19)"!=""
    @echo $(DEF3EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT20)"!=""
    @echo $(DEF3EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(TYPE) $(DEF3EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(TYPE) $(DEF3EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
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
DEF4DEPN+=$(foreach,i,$(DEFLIB4NAME) $(SLB)$/$(i).lib)
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL4VERSIONMAP)"!=""
.IF "$(DEF4EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF4EXPORTFILE=$(MISC)$/$(SHL4VERSIONMAP:b)_$(SHL4TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF4EXPORTFILE) : $(SHL4OBJS) $(SHL4LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF4EXPORTFILE) : $(SHL4VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    -grep -v "\*\|?" $@ > $@.exported-symbols
    -grep "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL4OBJS)"!=""
    -echo $(foreach,i,$(SHL4OBJS) $i) | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL4LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL4LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF4EXPORTFILE)"==""
.ENDIF			# "$(SHL4VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF4FILTER=$(SOLARENV)$/inc$/dummy.flt

RMHACK4:=$(RM)
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB4NAME)"!=""

DEF4UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE4:=$(shell @echo %_disk)
#BUILD_DRIVE4:=O

.IF "$(BUILD_DRIVE4)"=="O"
# in case of RE build, protect against failed lock
EXPORT4_PROTECT=$(TMP)$/$(DEF4UNIQE:b).bat &&
RMHACK4:=$(RM:s/+//)
.ELSE			# "$(BUILD_DRIVE4)"=="O"
.ENDIF			# "$(BUILD_DRIVE4)"=="O"

.ENDIF			# "$(DEFLIB4NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""

.IF "$(link_always)"==""
$(DEF4TARGETN) : \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF4TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB4NAME)"!=""
.IF "$(BUILD_DRIVE4)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF4UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE4)"=="O"
.ENDIF				# "$(DEFLIB4NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
#	-attrib -r defs$/$(OUTPATH)
    @-$(RM) $@.tmpfile
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(EMQ)"$(SHL4TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.IF "$(DEFLIB4NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(EXPORT4_PROTECT) $(RM) $(MISC)$/$(SHL4TARGET).exp
    dlltool --output-def $(MISC)$/$(SHL4TARGET).exp --export-all-symbols \
         `$(TYPE) $(foreach,i,$(DEFLIB4NAME) $(SLB)$/$(i).lib) | sed s#$(ROUT)#$(PRJ)$/$(ROUT)#g`
    tail --lines +3 $(MISC)$/$(SHL4TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(EXPORT4_PROTECT) $(RM) $(MISC)$/$(SHL4TARGET).exp
.ELSE
.IF "$(SHL4USE_EXPORTS)"==""
    @-$(EXPORT4_PROTECT) $(RMHACK4) $(MISC)$/$(SHL4TARGET).exp
    @$(EXPORT4_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(MISC)$/$(SHL4TARGET).exp $(SLB)$/$(DEFLIB4NAME).lib
.IF "$(DEF4CEXP)"!=""
    @$(EXPORT4_PROTECT) $(LDUMP2) -A $(DEF4CEXP) -E 20 -F $(MISC)$/$(SHL4TARGET).flt $(MISC)$/$(SHL4TARGET).exp			   >>$@.tmpfile
.ELSE
    @$(EXPORT4_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL4TARGET).flt $(MISC)$/$(SHL4TARGET).exp			   >>$@.tmpfile
.ENDIF
    $(EXPORT4_PROTECT) $(RMHACK4) $(MISC)$/$(SHL4TARGET).exp
.ELSE			# "$(SHL4USE_EXPORTS)"==""
    @$(EXPORT4_PROTECT) $(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB4NAME) $(SLB)$/$(i).lib) | $(GREP) EXPORT: > $(MISC)$/$(SHL4TARGET).direct
.IF "$(SHL4USE_EXPORTS)"!="name"
.IF "$(DEF4CEXP)"!=""
    @$(EXPORT4_PROTECT) $(LDUMP2) -D -A $(DEF4CEXP) -E 20 -F $(DEF4FILTER) $(MISC)$/$(SHL4TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT4_PROTECT) $(LDUMP2) -D -E 20 -F $(DEF4FILTER) $(MISC)$/$(SHL4TARGET).direct >>$@.tmpfile
.ENDIF
.ELSE			# "$(SHL4USE_EXPORTS)"!="name"
.IF "$(DEF4CEXP)"!=""
    @$(EXPORT4_PROTECT) $(LDUMP2) -N -D -A $(DEF4CEXP) -E 20 -F $(DEF4FILTER) $(MISC)$/$(SHL4TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT4_PROTECT) $(LDUMP2) -N -D -E 20 -F $(DEF4FILTER) $(MISC)$/$(SHL4TARGET).direct >>$@.tmpfile
.ENDIF
.ENDIF			# "$(SHL4USE_EXPORTS)"!="name"
.ENDIF			# "$(SHL4USE_EXPORTS)"==""
.ENDIF
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE4)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF4UNIQE:b) commit
    $(4nt_force_shell)$(TMP)$/$(DEF4UNIQE:b).bat && $(RM:s/+//) $(TMP)$/$(DEF4UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE4)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
.ENDIF				# "$(DEFLIB4NAME)"!=""
.IF "$(DEF4EXPORT1)"!=""
    @echo $(DEF4EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT2)"!=""
    @echo $(DEF4EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT3)"!=""
    @echo $(DEF4EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT4)"!=""
    @echo $(DEF4EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT5)"!=""
    @echo $(DEF4EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT6)"!=""
    @echo $(DEF4EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT7)"!=""
    @echo $(DEF4EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT8)"!=""
    @echo $(DEF4EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT9)"!=""
    @echo $(DEF4EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT10)"!=""
    @echo $(DEF4EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT11)"!=""
    @echo $(DEF4EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT12)"!=""
    @echo $(DEF4EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT13)"!=""
    @echo $(DEF4EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT14)"!=""
    @echo $(DEF4EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT15)"!=""
    @echo $(DEF4EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT16)"!=""
    @echo $(DEF4EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT17)"!=""
    @echo $(DEF4EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT18)"!=""
    @echo $(DEF4EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT19)"!=""
    @echo $(DEF4EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT20)"!=""
    @echo $(DEF4EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(TYPE) $(DEF4EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(TYPE) $(DEF4EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
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
DEF5DEPN+=$(foreach,i,$(DEFLIB5NAME) $(SLB)$/$(i).lib)
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL5VERSIONMAP)"!=""
.IF "$(DEF5EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF5EXPORTFILE=$(MISC)$/$(SHL5VERSIONMAP:b)_$(SHL5TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF5EXPORTFILE) : $(SHL5OBJS) $(SHL5LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF5EXPORTFILE) : $(SHL5VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    -grep -v "\*\|?" $@ > $@.exported-symbols
    -grep "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL5OBJS)"!=""
    -echo $(foreach,i,$(SHL5OBJS) $i) | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL5LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL5LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF5EXPORTFILE)"==""
.ENDIF			# "$(SHL5VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF5FILTER=$(SOLARENV)$/inc$/dummy.flt

RMHACK5:=$(RM)
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB5NAME)"!=""

DEF5UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE5:=$(shell @echo %_disk)
#BUILD_DRIVE5:=O

.IF "$(BUILD_DRIVE5)"=="O"
# in case of RE build, protect against failed lock
EXPORT5_PROTECT=$(TMP)$/$(DEF5UNIQE:b).bat &&
RMHACK5:=$(RM:s/+//)
.ELSE			# "$(BUILD_DRIVE5)"=="O"
.ENDIF			# "$(BUILD_DRIVE5)"=="O"

.ENDIF			# "$(DEFLIB5NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""

.IF "$(link_always)"==""
$(DEF5TARGETN) : \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF5TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB5NAME)"!=""
.IF "$(BUILD_DRIVE5)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF5UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE5)"=="O"
.ENDIF				# "$(DEFLIB5NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
#	-attrib -r defs$/$(OUTPATH)
    @-$(RM) $@.tmpfile
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(EMQ)"$(SHL5TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.IF "$(DEFLIB5NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(EXPORT5_PROTECT) $(RM) $(MISC)$/$(SHL5TARGET).exp
    dlltool --output-def $(MISC)$/$(SHL5TARGET).exp --export-all-symbols \
         `$(TYPE) $(foreach,i,$(DEFLIB5NAME) $(SLB)$/$(i).lib) | sed s#$(ROUT)#$(PRJ)$/$(ROUT)#g`
    tail --lines +3 $(MISC)$/$(SHL5TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(EXPORT5_PROTECT) $(RM) $(MISC)$/$(SHL5TARGET).exp
.ELSE
.IF "$(SHL5USE_EXPORTS)"==""
    @-$(EXPORT5_PROTECT) $(RMHACK5) $(MISC)$/$(SHL5TARGET).exp
    @$(EXPORT5_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(MISC)$/$(SHL5TARGET).exp $(SLB)$/$(DEFLIB5NAME).lib
.IF "$(DEF5CEXP)"!=""
    @$(EXPORT5_PROTECT) $(LDUMP2) -A $(DEF5CEXP) -E 20 -F $(MISC)$/$(SHL5TARGET).flt $(MISC)$/$(SHL5TARGET).exp			   >>$@.tmpfile
.ELSE
    @$(EXPORT5_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL5TARGET).flt $(MISC)$/$(SHL5TARGET).exp			   >>$@.tmpfile
.ENDIF
    $(EXPORT5_PROTECT) $(RMHACK5) $(MISC)$/$(SHL5TARGET).exp
.ELSE			# "$(SHL5USE_EXPORTS)"==""
    @$(EXPORT5_PROTECT) $(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB5NAME) $(SLB)$/$(i).lib) | $(GREP) EXPORT: > $(MISC)$/$(SHL5TARGET).direct
.IF "$(SHL5USE_EXPORTS)"!="name"
.IF "$(DEF5CEXP)"!=""
    @$(EXPORT5_PROTECT) $(LDUMP2) -D -A $(DEF5CEXP) -E 20 -F $(DEF5FILTER) $(MISC)$/$(SHL5TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT5_PROTECT) $(LDUMP2) -D -E 20 -F $(DEF5FILTER) $(MISC)$/$(SHL5TARGET).direct >>$@.tmpfile
.ENDIF
.ELSE			# "$(SHL5USE_EXPORTS)"!="name"
.IF "$(DEF5CEXP)"!=""
    @$(EXPORT5_PROTECT) $(LDUMP2) -N -D -A $(DEF5CEXP) -E 20 -F $(DEF5FILTER) $(MISC)$/$(SHL5TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT5_PROTECT) $(LDUMP2) -N -D -E 20 -F $(DEF5FILTER) $(MISC)$/$(SHL5TARGET).direct >>$@.tmpfile
.ENDIF
.ENDIF			# "$(SHL5USE_EXPORTS)"!="name"
.ENDIF			# "$(SHL5USE_EXPORTS)"==""
.ENDIF
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE5)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF5UNIQE:b) commit
    $(4nt_force_shell)$(TMP)$/$(DEF5UNIQE:b).bat && $(RM:s/+//) $(TMP)$/$(DEF5UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE5)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
.ENDIF				# "$(DEFLIB5NAME)"!=""
.IF "$(DEF5EXPORT1)"!=""
    @echo $(DEF5EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT2)"!=""
    @echo $(DEF5EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT3)"!=""
    @echo $(DEF5EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT4)"!=""
    @echo $(DEF5EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT5)"!=""
    @echo $(DEF5EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT6)"!=""
    @echo $(DEF5EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT7)"!=""
    @echo $(DEF5EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT8)"!=""
    @echo $(DEF5EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT9)"!=""
    @echo $(DEF5EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT10)"!=""
    @echo $(DEF5EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT11)"!=""
    @echo $(DEF5EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT12)"!=""
    @echo $(DEF5EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT13)"!=""
    @echo $(DEF5EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT14)"!=""
    @echo $(DEF5EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT15)"!=""
    @echo $(DEF5EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT16)"!=""
    @echo $(DEF5EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT17)"!=""
    @echo $(DEF5EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT18)"!=""
    @echo $(DEF5EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT19)"!=""
    @echo $(DEF5EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT20)"!=""
    @echo $(DEF5EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(TYPE) $(DEF5EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(TYPE) $(DEF5EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
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
DEF6DEPN+=$(foreach,i,$(DEFLIB6NAME) $(SLB)$/$(i).lib)
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL6VERSIONMAP)"!=""
.IF "$(DEF6EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF6EXPORTFILE=$(MISC)$/$(SHL6VERSIONMAP:b)_$(SHL6TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF6EXPORTFILE) : $(SHL6OBJS) $(SHL6LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF6EXPORTFILE) : $(SHL6VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    -grep -v "\*\|?" $@ > $@.exported-symbols
    -grep "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL6OBJS)"!=""
    -echo $(foreach,i,$(SHL6OBJS) $i) | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL6LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL6LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF6EXPORTFILE)"==""
.ENDIF			# "$(SHL6VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF6FILTER=$(SOLARENV)$/inc$/dummy.flt

RMHACK6:=$(RM)
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB6NAME)"!=""

DEF6UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE6:=$(shell @echo %_disk)
#BUILD_DRIVE6:=O

.IF "$(BUILD_DRIVE6)"=="O"
# in case of RE build, protect against failed lock
EXPORT6_PROTECT=$(TMP)$/$(DEF6UNIQE:b).bat &&
RMHACK6:=$(RM:s/+//)
.ELSE			# "$(BUILD_DRIVE6)"=="O"
.ENDIF			# "$(BUILD_DRIVE6)"=="O"

.ENDIF			# "$(DEFLIB6NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""

.IF "$(link_always)"==""
$(DEF6TARGETN) : \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF6TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB6NAME)"!=""
.IF "$(BUILD_DRIVE6)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF6UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE6)"=="O"
.ENDIF				# "$(DEFLIB6NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
#	-attrib -r defs$/$(OUTPATH)
    @-$(RM) $@.tmpfile
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(EMQ)"$(SHL6TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.IF "$(DEFLIB6NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(EXPORT6_PROTECT) $(RM) $(MISC)$/$(SHL6TARGET).exp
    dlltool --output-def $(MISC)$/$(SHL6TARGET).exp --export-all-symbols \
         `$(TYPE) $(foreach,i,$(DEFLIB6NAME) $(SLB)$/$(i).lib) | sed s#$(ROUT)#$(PRJ)$/$(ROUT)#g`
    tail --lines +3 $(MISC)$/$(SHL6TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(EXPORT6_PROTECT) $(RM) $(MISC)$/$(SHL6TARGET).exp
.ELSE
.IF "$(SHL6USE_EXPORTS)"==""
    @-$(EXPORT6_PROTECT) $(RMHACK6) $(MISC)$/$(SHL6TARGET).exp
    @$(EXPORT6_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(MISC)$/$(SHL6TARGET).exp $(SLB)$/$(DEFLIB6NAME).lib
.IF "$(DEF6CEXP)"!=""
    @$(EXPORT6_PROTECT) $(LDUMP2) -A $(DEF6CEXP) -E 20 -F $(MISC)$/$(SHL6TARGET).flt $(MISC)$/$(SHL6TARGET).exp			   >>$@.tmpfile
.ELSE
    @$(EXPORT6_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL6TARGET).flt $(MISC)$/$(SHL6TARGET).exp			   >>$@.tmpfile
.ENDIF
    $(EXPORT6_PROTECT) $(RMHACK6) $(MISC)$/$(SHL6TARGET).exp
.ELSE			# "$(SHL6USE_EXPORTS)"==""
    @$(EXPORT6_PROTECT) $(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB6NAME) $(SLB)$/$(i).lib) | $(GREP) EXPORT: > $(MISC)$/$(SHL6TARGET).direct
.IF "$(SHL6USE_EXPORTS)"!="name"
.IF "$(DEF6CEXP)"!=""
    @$(EXPORT6_PROTECT) $(LDUMP2) -D -A $(DEF6CEXP) -E 20 -F $(DEF6FILTER) $(MISC)$/$(SHL6TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT6_PROTECT) $(LDUMP2) -D -E 20 -F $(DEF6FILTER) $(MISC)$/$(SHL6TARGET).direct >>$@.tmpfile
.ENDIF
.ELSE			# "$(SHL6USE_EXPORTS)"!="name"
.IF "$(DEF6CEXP)"!=""
    @$(EXPORT6_PROTECT) $(LDUMP2) -N -D -A $(DEF6CEXP) -E 20 -F $(DEF6FILTER) $(MISC)$/$(SHL6TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT6_PROTECT) $(LDUMP2) -N -D -E 20 -F $(DEF6FILTER) $(MISC)$/$(SHL6TARGET).direct >>$@.tmpfile
.ENDIF
.ENDIF			# "$(SHL6USE_EXPORTS)"!="name"
.ENDIF			# "$(SHL6USE_EXPORTS)"==""
.ENDIF
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE6)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF6UNIQE:b) commit
    $(4nt_force_shell)$(TMP)$/$(DEF6UNIQE:b).bat && $(RM:s/+//) $(TMP)$/$(DEF6UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE6)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
.ENDIF				# "$(DEFLIB6NAME)"!=""
.IF "$(DEF6EXPORT1)"!=""
    @echo $(DEF6EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT2)"!=""
    @echo $(DEF6EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT3)"!=""
    @echo $(DEF6EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT4)"!=""
    @echo $(DEF6EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT5)"!=""
    @echo $(DEF6EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT6)"!=""
    @echo $(DEF6EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT7)"!=""
    @echo $(DEF6EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT8)"!=""
    @echo $(DEF6EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT9)"!=""
    @echo $(DEF6EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT10)"!=""
    @echo $(DEF6EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT11)"!=""
    @echo $(DEF6EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT12)"!=""
    @echo $(DEF6EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT13)"!=""
    @echo $(DEF6EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT14)"!=""
    @echo $(DEF6EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT15)"!=""
    @echo $(DEF6EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT16)"!=""
    @echo $(DEF6EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT17)"!=""
    @echo $(DEF6EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT18)"!=""
    @echo $(DEF6EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT19)"!=""
    @echo $(DEF6EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT20)"!=""
    @echo $(DEF6EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(TYPE) $(DEF6EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(TYPE) $(DEF6EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
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
DEF7DEPN+=$(foreach,i,$(DEFLIB7NAME) $(SLB)$/$(i).lib)
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL7VERSIONMAP)"!=""
.IF "$(DEF7EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF7EXPORTFILE=$(MISC)$/$(SHL7VERSIONMAP:b)_$(SHL7TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF7EXPORTFILE) : $(SHL7OBJS) $(SHL7LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF7EXPORTFILE) : $(SHL7VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    -grep -v "\*\|?" $@ > $@.exported-symbols
    -grep "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL7OBJS)"!=""
    -echo $(foreach,i,$(SHL7OBJS) $i) | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL7LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL7LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF7EXPORTFILE)"==""
.ENDIF			# "$(SHL7VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF7FILTER=$(SOLARENV)$/inc$/dummy.flt

RMHACK7:=$(RM)
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB7NAME)"!=""

DEF7UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE7:=$(shell @echo %_disk)
#BUILD_DRIVE7:=O

.IF "$(BUILD_DRIVE7)"=="O"
# in case of RE build, protect against failed lock
EXPORT7_PROTECT=$(TMP)$/$(DEF7UNIQE:b).bat &&
RMHACK7:=$(RM:s/+//)
.ELSE			# "$(BUILD_DRIVE7)"=="O"
.ENDIF			# "$(BUILD_DRIVE7)"=="O"

.ENDIF			# "$(DEFLIB7NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""

.IF "$(link_always)"==""
$(DEF7TARGETN) : \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF7TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB7NAME)"!=""
.IF "$(BUILD_DRIVE7)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF7UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE7)"=="O"
.ENDIF				# "$(DEFLIB7NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
#	-attrib -r defs$/$(OUTPATH)
    @-$(RM) $@.tmpfile
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(EMQ)"$(SHL7TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.IF "$(DEFLIB7NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(EXPORT7_PROTECT) $(RM) $(MISC)$/$(SHL7TARGET).exp
    dlltool --output-def $(MISC)$/$(SHL7TARGET).exp --export-all-symbols \
         `$(TYPE) $(foreach,i,$(DEFLIB7NAME) $(SLB)$/$(i).lib) | sed s#$(ROUT)#$(PRJ)$/$(ROUT)#g`
    tail --lines +3 $(MISC)$/$(SHL7TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(EXPORT7_PROTECT) $(RM) $(MISC)$/$(SHL7TARGET).exp
.ELSE
.IF "$(SHL7USE_EXPORTS)"==""
    @-$(EXPORT7_PROTECT) $(RMHACK7) $(MISC)$/$(SHL7TARGET).exp
    @$(EXPORT7_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(MISC)$/$(SHL7TARGET).exp $(SLB)$/$(DEFLIB7NAME).lib
.IF "$(DEF7CEXP)"!=""
    @$(EXPORT7_PROTECT) $(LDUMP2) -A $(DEF7CEXP) -E 20 -F $(MISC)$/$(SHL7TARGET).flt $(MISC)$/$(SHL7TARGET).exp			   >>$@.tmpfile
.ELSE
    @$(EXPORT7_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL7TARGET).flt $(MISC)$/$(SHL7TARGET).exp			   >>$@.tmpfile
.ENDIF
    $(EXPORT7_PROTECT) $(RMHACK7) $(MISC)$/$(SHL7TARGET).exp
.ELSE			# "$(SHL7USE_EXPORTS)"==""
    @$(EXPORT7_PROTECT) $(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB7NAME) $(SLB)$/$(i).lib) | $(GREP) EXPORT: > $(MISC)$/$(SHL7TARGET).direct
.IF "$(SHL7USE_EXPORTS)"!="name"
.IF "$(DEF7CEXP)"!=""
    @$(EXPORT7_PROTECT) $(LDUMP2) -D -A $(DEF7CEXP) -E 20 -F $(DEF7FILTER) $(MISC)$/$(SHL7TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT7_PROTECT) $(LDUMP2) -D -E 20 -F $(DEF7FILTER) $(MISC)$/$(SHL7TARGET).direct >>$@.tmpfile
.ENDIF
.ELSE			# "$(SHL7USE_EXPORTS)"!="name"
.IF "$(DEF7CEXP)"!=""
    @$(EXPORT7_PROTECT) $(LDUMP2) -N -D -A $(DEF7CEXP) -E 20 -F $(DEF7FILTER) $(MISC)$/$(SHL7TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT7_PROTECT) $(LDUMP2) -N -D -E 20 -F $(DEF7FILTER) $(MISC)$/$(SHL7TARGET).direct >>$@.tmpfile
.ENDIF
.ENDIF			# "$(SHL7USE_EXPORTS)"!="name"
.ENDIF			# "$(SHL7USE_EXPORTS)"==""
.ENDIF
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE7)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF7UNIQE:b) commit
    $(4nt_force_shell)$(TMP)$/$(DEF7UNIQE:b).bat && $(RM:s/+//) $(TMP)$/$(DEF7UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE7)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
.ENDIF				# "$(DEFLIB7NAME)"!=""
.IF "$(DEF7EXPORT1)"!=""
    @echo $(DEF7EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT2)"!=""
    @echo $(DEF7EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT3)"!=""
    @echo $(DEF7EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT4)"!=""
    @echo $(DEF7EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT5)"!=""
    @echo $(DEF7EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT6)"!=""
    @echo $(DEF7EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT7)"!=""
    @echo $(DEF7EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT8)"!=""
    @echo $(DEF7EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT9)"!=""
    @echo $(DEF7EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT10)"!=""
    @echo $(DEF7EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT11)"!=""
    @echo $(DEF7EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT12)"!=""
    @echo $(DEF7EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT13)"!=""
    @echo $(DEF7EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT14)"!=""
    @echo $(DEF7EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT15)"!=""
    @echo $(DEF7EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT16)"!=""
    @echo $(DEF7EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT17)"!=""
    @echo $(DEF7EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT18)"!=""
    @echo $(DEF7EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT19)"!=""
    @echo $(DEF7EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT20)"!=""
    @echo $(DEF7EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(TYPE) $(DEF7EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(TYPE) $(DEF7EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
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
DEF8DEPN+=$(foreach,i,$(DEFLIB8NAME) $(SLB)$/$(i).lib)
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL8VERSIONMAP)"!=""
.IF "$(DEF8EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF8EXPORTFILE=$(MISC)$/$(SHL8VERSIONMAP:b)_$(SHL8TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF8EXPORTFILE) : $(SHL8OBJS) $(SHL8LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF8EXPORTFILE) : $(SHL8VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    -grep -v "\*\|?" $@ > $@.exported-symbols
    -grep "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL8OBJS)"!=""
    -echo $(foreach,i,$(SHL8OBJS) $i) | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL8LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL8LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF8EXPORTFILE)"==""
.ENDIF			# "$(SHL8VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF8FILTER=$(SOLARENV)$/inc$/dummy.flt

RMHACK8:=$(RM)
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB8NAME)"!=""

DEF8UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE8:=$(shell @echo %_disk)
#BUILD_DRIVE8:=O

.IF "$(BUILD_DRIVE8)"=="O"
# in case of RE build, protect against failed lock
EXPORT8_PROTECT=$(TMP)$/$(DEF8UNIQE:b).bat &&
RMHACK8:=$(RM:s/+//)
.ELSE			# "$(BUILD_DRIVE8)"=="O"
.ENDIF			# "$(BUILD_DRIVE8)"=="O"

.ENDIF			# "$(DEFLIB8NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""

.IF "$(link_always)"==""
$(DEF8TARGETN) : \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF8TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB8NAME)"!=""
.IF "$(BUILD_DRIVE8)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF8UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE8)"=="O"
.ENDIF				# "$(DEFLIB8NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
#	-attrib -r defs$/$(OUTPATH)
    @-$(RM) $@.tmpfile
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(EMQ)"$(SHL8TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.IF "$(DEFLIB8NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(EXPORT8_PROTECT) $(RM) $(MISC)$/$(SHL8TARGET).exp
    dlltool --output-def $(MISC)$/$(SHL8TARGET).exp --export-all-symbols \
         `$(TYPE) $(foreach,i,$(DEFLIB8NAME) $(SLB)$/$(i).lib) | sed s#$(ROUT)#$(PRJ)$/$(ROUT)#g`
    tail --lines +3 $(MISC)$/$(SHL8TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(EXPORT8_PROTECT) $(RM) $(MISC)$/$(SHL8TARGET).exp
.ELSE
.IF "$(SHL8USE_EXPORTS)"==""
    @-$(EXPORT8_PROTECT) $(RMHACK8) $(MISC)$/$(SHL8TARGET).exp
    @$(EXPORT8_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(MISC)$/$(SHL8TARGET).exp $(SLB)$/$(DEFLIB8NAME).lib
.IF "$(DEF8CEXP)"!=""
    @$(EXPORT8_PROTECT) $(LDUMP2) -A $(DEF8CEXP) -E 20 -F $(MISC)$/$(SHL8TARGET).flt $(MISC)$/$(SHL8TARGET).exp			   >>$@.tmpfile
.ELSE
    @$(EXPORT8_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL8TARGET).flt $(MISC)$/$(SHL8TARGET).exp			   >>$@.tmpfile
.ENDIF
    $(EXPORT8_PROTECT) $(RMHACK8) $(MISC)$/$(SHL8TARGET).exp
.ELSE			# "$(SHL8USE_EXPORTS)"==""
    @$(EXPORT8_PROTECT) $(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB8NAME) $(SLB)$/$(i).lib) | $(GREP) EXPORT: > $(MISC)$/$(SHL8TARGET).direct
.IF "$(SHL8USE_EXPORTS)"!="name"
.IF "$(DEF8CEXP)"!=""
    @$(EXPORT8_PROTECT) $(LDUMP2) -D -A $(DEF8CEXP) -E 20 -F $(DEF8FILTER) $(MISC)$/$(SHL8TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT8_PROTECT) $(LDUMP2) -D -E 20 -F $(DEF8FILTER) $(MISC)$/$(SHL8TARGET).direct >>$@.tmpfile
.ENDIF
.ELSE			# "$(SHL8USE_EXPORTS)"!="name"
.IF "$(DEF8CEXP)"!=""
    @$(EXPORT8_PROTECT) $(LDUMP2) -N -D -A $(DEF8CEXP) -E 20 -F $(DEF8FILTER) $(MISC)$/$(SHL8TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT8_PROTECT) $(LDUMP2) -N -D -E 20 -F $(DEF8FILTER) $(MISC)$/$(SHL8TARGET).direct >>$@.tmpfile
.ENDIF
.ENDIF			# "$(SHL8USE_EXPORTS)"!="name"
.ENDIF			# "$(SHL8USE_EXPORTS)"==""
.ENDIF
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE8)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF8UNIQE:b) commit
    $(4nt_force_shell)$(TMP)$/$(DEF8UNIQE:b).bat && $(RM:s/+//) $(TMP)$/$(DEF8UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE8)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
.ENDIF				# "$(DEFLIB8NAME)"!=""
.IF "$(DEF8EXPORT1)"!=""
    @echo $(DEF8EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT2)"!=""
    @echo $(DEF8EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT3)"!=""
    @echo $(DEF8EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT4)"!=""
    @echo $(DEF8EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT5)"!=""
    @echo $(DEF8EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT6)"!=""
    @echo $(DEF8EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT7)"!=""
    @echo $(DEF8EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT8)"!=""
    @echo $(DEF8EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT9)"!=""
    @echo $(DEF8EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT10)"!=""
    @echo $(DEF8EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT11)"!=""
    @echo $(DEF8EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT12)"!=""
    @echo $(DEF8EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT13)"!=""
    @echo $(DEF8EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT14)"!=""
    @echo $(DEF8EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT15)"!=""
    @echo $(DEF8EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT16)"!=""
    @echo $(DEF8EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT17)"!=""
    @echo $(DEF8EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT18)"!=""
    @echo $(DEF8EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT19)"!=""
    @echo $(DEF8EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT20)"!=""
    @echo $(DEF8EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(TYPE) $(DEF8EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(TYPE) $(DEF8EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
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
DEF9DEPN+=$(foreach,i,$(DEFLIB9NAME) $(SLB)$/$(i).lib)
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL9VERSIONMAP)"!=""
.IF "$(DEF9EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF9EXPORTFILE=$(MISC)$/$(SHL9VERSIONMAP:b)_$(SHL9TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF9EXPORTFILE) : $(SHL9OBJS) $(SHL9LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF9EXPORTFILE) : $(SHL9VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    -grep -v "\*\|?" $@ > $@.exported-symbols
    -grep "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL9OBJS)"!=""
    -echo $(foreach,i,$(SHL9OBJS) $i) | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL9LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL9LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF9EXPORTFILE)"==""
.ENDIF			# "$(SHL9VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF9FILTER=$(SOLARENV)$/inc$/dummy.flt

RMHACK9:=$(RM)
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB9NAME)"!=""

DEF9UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE9:=$(shell @echo %_disk)
#BUILD_DRIVE9:=O

.IF "$(BUILD_DRIVE9)"=="O"
# in case of RE build, protect against failed lock
EXPORT9_PROTECT=$(TMP)$/$(DEF9UNIQE:b).bat &&
RMHACK9:=$(RM:s/+//)
.ELSE			# "$(BUILD_DRIVE9)"=="O"
.ENDIF			# "$(BUILD_DRIVE9)"=="O"

.ENDIF			# "$(DEFLIB9NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""

.IF "$(link_always)"==""
$(DEF9TARGETN) : \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF9TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB9NAME)"!=""
.IF "$(BUILD_DRIVE9)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF9UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE9)"=="O"
.ENDIF				# "$(DEFLIB9NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
#	-attrib -r defs$/$(OUTPATH)
    @-$(RM) $@.tmpfile
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(EMQ)"$(SHL9TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.IF "$(DEFLIB9NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(EXPORT9_PROTECT) $(RM) $(MISC)$/$(SHL9TARGET).exp
    dlltool --output-def $(MISC)$/$(SHL9TARGET).exp --export-all-symbols \
         `$(TYPE) $(foreach,i,$(DEFLIB9NAME) $(SLB)$/$(i).lib) | sed s#$(ROUT)#$(PRJ)$/$(ROUT)#g`
    tail --lines +3 $(MISC)$/$(SHL9TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(EXPORT9_PROTECT) $(RM) $(MISC)$/$(SHL9TARGET).exp
.ELSE
.IF "$(SHL9USE_EXPORTS)"==""
    @-$(EXPORT9_PROTECT) $(RMHACK9) $(MISC)$/$(SHL9TARGET).exp
    @$(EXPORT9_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(MISC)$/$(SHL9TARGET).exp $(SLB)$/$(DEFLIB9NAME).lib
.IF "$(DEF9CEXP)"!=""
    @$(EXPORT9_PROTECT) $(LDUMP2) -A $(DEF9CEXP) -E 20 -F $(MISC)$/$(SHL9TARGET).flt $(MISC)$/$(SHL9TARGET).exp			   >>$@.tmpfile
.ELSE
    @$(EXPORT9_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL9TARGET).flt $(MISC)$/$(SHL9TARGET).exp			   >>$@.tmpfile
.ENDIF
    $(EXPORT9_PROTECT) $(RMHACK9) $(MISC)$/$(SHL9TARGET).exp
.ELSE			# "$(SHL9USE_EXPORTS)"==""
    @$(EXPORT9_PROTECT) $(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB9NAME) $(SLB)$/$(i).lib) | $(GREP) EXPORT: > $(MISC)$/$(SHL9TARGET).direct
.IF "$(SHL9USE_EXPORTS)"!="name"
.IF "$(DEF9CEXP)"!=""
    @$(EXPORT9_PROTECT) $(LDUMP2) -D -A $(DEF9CEXP) -E 20 -F $(DEF9FILTER) $(MISC)$/$(SHL9TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT9_PROTECT) $(LDUMP2) -D -E 20 -F $(DEF9FILTER) $(MISC)$/$(SHL9TARGET).direct >>$@.tmpfile
.ENDIF
.ELSE			# "$(SHL9USE_EXPORTS)"!="name"
.IF "$(DEF9CEXP)"!=""
    @$(EXPORT9_PROTECT) $(LDUMP2) -N -D -A $(DEF9CEXP) -E 20 -F $(DEF9FILTER) $(MISC)$/$(SHL9TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT9_PROTECT) $(LDUMP2) -N -D -E 20 -F $(DEF9FILTER) $(MISC)$/$(SHL9TARGET).direct >>$@.tmpfile
.ENDIF
.ENDIF			# "$(SHL9USE_EXPORTS)"!="name"
.ENDIF			# "$(SHL9USE_EXPORTS)"==""
.ENDIF
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE9)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF9UNIQE:b) commit
    $(4nt_force_shell)$(TMP)$/$(DEF9UNIQE:b).bat && $(RM:s/+//) $(TMP)$/$(DEF9UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE9)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
.ENDIF				# "$(DEFLIB9NAME)"!=""
.IF "$(DEF9EXPORT1)"!=""
    @echo $(DEF9EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT2)"!=""
    @echo $(DEF9EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT3)"!=""
    @echo $(DEF9EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT4)"!=""
    @echo $(DEF9EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT5)"!=""
    @echo $(DEF9EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT6)"!=""
    @echo $(DEF9EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT7)"!=""
    @echo $(DEF9EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT8)"!=""
    @echo $(DEF9EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT9)"!=""
    @echo $(DEF9EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT10)"!=""
    @echo $(DEF9EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT11)"!=""
    @echo $(DEF9EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT12)"!=""
    @echo $(DEF9EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT13)"!=""
    @echo $(DEF9EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT14)"!=""
    @echo $(DEF9EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT15)"!=""
    @echo $(DEF9EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT16)"!=""
    @echo $(DEF9EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT17)"!=""
    @echo $(DEF9EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT18)"!=""
    @echo $(DEF9EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT19)"!=""
    @echo $(DEF9EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT20)"!=""
    @echo $(DEF9EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(TYPE) $(DEF9EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(TYPE) $(DEF9EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
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
DEF10DEPN+=$(foreach,i,$(DEFLIB10NAME) $(SLB)$/$(i).lib)
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL10VERSIONMAP)"!=""
.IF "$(DEF10EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF10EXPORTFILE=$(MISC)$/$(SHL10VERSIONMAP:b)_$(SHL10TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF10EXPORTFILE) : $(SHL10OBJS) $(SHL10LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF10EXPORTFILE) : $(SHL10VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    -grep -v "\*\|?" $@ > $@.exported-symbols
    -grep "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL10OBJS)"!=""
    -echo $(foreach,i,$(SHL10OBJS) $i) | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL10LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL10LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF10EXPORTFILE)"==""
.ENDIF			# "$(SHL10VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF10FILTER=$(SOLARENV)$/inc$/dummy.flt

RMHACK10:=$(RM)
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB10NAME)"!=""

DEF10UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE10:=$(shell @echo %_disk)
#BUILD_DRIVE10:=O

.IF "$(BUILD_DRIVE10)"=="O"
# in case of RE build, protect against failed lock
EXPORT10_PROTECT=$(TMP)$/$(DEF10UNIQE:b).bat &&
RMHACK10:=$(RM:s/+//)
.ELSE			# "$(BUILD_DRIVE10)"=="O"
.ENDIF			# "$(BUILD_DRIVE10)"=="O"

.ENDIF			# "$(DEFLIB10NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""

.IF "$(link_always)"==""
$(DEF10TARGETN) : \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF10TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB10NAME)"!=""
.IF "$(BUILD_DRIVE10)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF10UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE10)"=="O"
.ENDIF				# "$(DEFLIB10NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
#	-attrib -r defs$/$(OUTPATH)
    @-$(RM) $@.tmpfile
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(EMQ)"$(SHL10TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.IF "$(DEFLIB10NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(EXPORT10_PROTECT) $(RM) $(MISC)$/$(SHL10TARGET).exp
    dlltool --output-def $(MISC)$/$(SHL10TARGET).exp --export-all-symbols \
         `$(TYPE) $(foreach,i,$(DEFLIB10NAME) $(SLB)$/$(i).lib) | sed s#$(ROUT)#$(PRJ)$/$(ROUT)#g`
    tail --lines +3 $(MISC)$/$(SHL10TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(EXPORT10_PROTECT) $(RM) $(MISC)$/$(SHL10TARGET).exp
.ELSE
.IF "$(SHL10USE_EXPORTS)"==""
    @-$(EXPORT10_PROTECT) $(RMHACK10) $(MISC)$/$(SHL10TARGET).exp
    @$(EXPORT10_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(MISC)$/$(SHL10TARGET).exp $(SLB)$/$(DEFLIB10NAME).lib
.IF "$(DEF10CEXP)"!=""
    @$(EXPORT10_PROTECT) $(LDUMP2) -A $(DEF10CEXP) -E 20 -F $(MISC)$/$(SHL10TARGET).flt $(MISC)$/$(SHL10TARGET).exp			   >>$@.tmpfile
.ELSE
    @$(EXPORT10_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL10TARGET).flt $(MISC)$/$(SHL10TARGET).exp			   >>$@.tmpfile
.ENDIF
    $(EXPORT10_PROTECT) $(RMHACK10) $(MISC)$/$(SHL10TARGET).exp
.ELSE			# "$(SHL10USE_EXPORTS)"==""
    @$(EXPORT10_PROTECT) $(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB10NAME) $(SLB)$/$(i).lib) | $(GREP) EXPORT: > $(MISC)$/$(SHL10TARGET).direct
.IF "$(SHL10USE_EXPORTS)"!="name"
.IF "$(DEF10CEXP)"!=""
    @$(EXPORT10_PROTECT) $(LDUMP2) -D -A $(DEF10CEXP) -E 20 -F $(DEF10FILTER) $(MISC)$/$(SHL10TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT10_PROTECT) $(LDUMP2) -D -E 20 -F $(DEF10FILTER) $(MISC)$/$(SHL10TARGET).direct >>$@.tmpfile
.ENDIF
.ELSE			# "$(SHL10USE_EXPORTS)"!="name"
.IF "$(DEF10CEXP)"!=""
    @$(EXPORT10_PROTECT) $(LDUMP2) -N -D -A $(DEF10CEXP) -E 20 -F $(DEF10FILTER) $(MISC)$/$(SHL10TARGET).direct >>$@.tmpfile
.ELSE
    @$(EXPORT10_PROTECT) $(LDUMP2) -N -D -E 20 -F $(DEF10FILTER) $(MISC)$/$(SHL10TARGET).direct >>$@.tmpfile
.ENDIF
.ENDIF			# "$(SHL10USE_EXPORTS)"!="name"
.ENDIF			# "$(SHL10USE_EXPORTS)"==""
.ENDIF
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE10)"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    $(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF10UNIQE:b) commit
    $(4nt_force_shell)$(TMP)$/$(DEF10UNIQE:b).bat && $(RM:s/+//) $(TMP)$/$(DEF10UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE10)"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
.ENDIF				# "$(DEFLIB10NAME)"!=""
.IF "$(DEF10EXPORT1)"!=""
    @echo $(DEF10EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT2)"!=""
    @echo $(DEF10EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT3)"!=""
    @echo $(DEF10EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT4)"!=""
    @echo $(DEF10EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT5)"!=""
    @echo $(DEF10EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT6)"!=""
    @echo $(DEF10EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT7)"!=""
    @echo $(DEF10EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT8)"!=""
    @echo $(DEF10EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT9)"!=""
    @echo $(DEF10EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT10)"!=""
    @echo $(DEF10EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT11)"!=""
    @echo $(DEF10EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT12)"!=""
    @echo $(DEF10EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT13)"!=""
    @echo $(DEF10EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT14)"!=""
    @echo $(DEF10EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT15)"!=""
    @echo $(DEF10EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT16)"!=""
    @echo $(DEF10EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT17)"!=""
    @echo $(DEF10EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT18)"!=""
    @echo $(DEF10EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT19)"!=""
    @echo $(DEF10EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT20)"!=""
    @echo $(DEF10EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(TYPE) $(DEF10EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(TYPE) $(DEF10EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
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
.ENDIF			# "$(DEF10TARGETN)"!=""


# Anweisungen fuer das Linken
