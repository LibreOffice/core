# unroll begin

.IF "$(DEF1TARGETN)"!=""

#to make intuitiv naming work
.IF "$(DEF1LIBNAME)"!=""
DEFLIB1NAME*=$(DEF1LIBNAME)
.ENDIF			# "$(DEF1LIBNAME)"!=""

.IF "$(DEFLIB1NAME)"!=""
DEF1DEPN+=$(foreach,i,$(DEFLIB1NAME) $(SLB)/$(i).lib)
.ENDIF

.IF "$(SHL1VERSIONMAP)"!=""
.IF "$(DEF1EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF1EXPORTFILE=$(MISC)/$(SHL1VERSIONMAP:b)_$(SHL1TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF1EXPORTFILE) : $(SHL1OBJS) $(SHL1LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF1EXPORTFILE) : $(SHL1VERSIONMAP)
    $(COMMAND_ECHO)$(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(GREP) -v "\*\|?" $@ | $(SED) -e 's@#.*@@' > $@.exported-symbols
    $(COMMAND_ECHO)-$(GREP) "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
    $(COMMAND_ECHO)$(NM) -gP $(SHL1OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL1LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.ENDIF			# "$(DEF1EXPORTFILE)"==""
.ENDIF			# "$(SHL1VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF1FILTER=$(SOLARENV)/inc/dummy.flt

RMHACK1:=$(RM)

.IF "$(DEF1CEXP)"!=""
EXPORT_ALL_SWITCH=-A $(DEF1CEXP)
.ENDIF

.IF "$(link_always)"==""
$(DEF1TARGETN) : \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF1TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    $(COMMAND_ECHO)-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    $(COMMAND_ECHO)echo LIBRARY	  $(EMQ)"$(SHL1TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    $(COMMAND_ECHO)echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    $(COMMAND_ECHO)echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    $(COMMAND_ECHO)echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB1NAME)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL1TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL1TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB1NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL1TARGET).exp | sed '/^;/d' >>$@.tmpfile
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL1TARGET).exp
.ELSE
.IF "$(SHL1USE_EXPORTS)"==""
    $(COMMAND_ECHO)-$(RMHACK1) $(MISC)/$(SHL1TARGET).exp
    $(COMMAND_ECHO)$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL1TARGET).exp $(SLB)/$(DEFLIB1NAME).lib
    $(COMMAND_ECHO)$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL1TARGET).flt $(MISC)/$(SHL1TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK1) $(MISC)/$(SHL1TARGET).exp
.ELSE			# "$(SHL1USE_EXPORTS)"==""
    $(COMMAND_ECHO)$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB1NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL1TARGET).direct
    $(COMMAND_ECHO)$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF1FILTER) $(MISC)/$(SHL1TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL1USE_EXPORTS)"==""
.ENDIF
.ENDIF				# "$(DEFLIB1NAME)"!=""
.IF "$(DEF1EXPORT1)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT2)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT3)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT4)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT5)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT6)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT7)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT8)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT9)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT10)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT11)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT12)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT13)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT14)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT15)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT16)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT17)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT18)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT19)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORT20)"!=""
    $(COMMAND_ECHO)echo $(DEF1EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF1EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(TYPE) $(DEF1EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF1EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF1TARGETN): \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
    $(COMMAND_ECHO)echo just a dummy for UNIX > $@
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
DEF2DEPN+=$(foreach,i,$(DEFLIB2NAME) $(SLB)/$(i).lib)
.ENDIF

.IF "$(SHL2VERSIONMAP)"!=""
.IF "$(DEF2EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF2EXPORTFILE=$(MISC)/$(SHL2VERSIONMAP:b)_$(SHL2TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF2EXPORTFILE) : $(SHL2OBJS) $(SHL2LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF2EXPORTFILE) : $(SHL2VERSIONMAP)
    $(COMMAND_ECHO)$(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(GREP) -v "\*\|?" $@ | $(SED) -e 's@#.*@@' > $@.exported-symbols
    $(COMMAND_ECHO)-$(GREP) "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
    $(COMMAND_ECHO)$(NM) -gP $(SHL2OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL2LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.ENDIF			# "$(DEF2EXPORTFILE)"==""
.ENDIF			# "$(SHL2VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF2FILTER=$(SOLARENV)/inc/dummy.flt

RMHACK2:=$(RM)

.IF "$(DEF2CEXP)"!=""
EXPORT_ALL_SWITCH=-A $(DEF2CEXP)
.ENDIF

.IF "$(link_always)"==""
$(DEF2TARGETN) : \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF2TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    $(COMMAND_ECHO)-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    $(COMMAND_ECHO)echo LIBRARY	  $(EMQ)"$(SHL2TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    $(COMMAND_ECHO)echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    $(COMMAND_ECHO)echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    $(COMMAND_ECHO)echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB2NAME)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL2TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL2TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB2NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL2TARGET).exp | sed '/^;/d' >>$@.tmpfile
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL2TARGET).exp
.ELSE
.IF "$(SHL2USE_EXPORTS)"==""
    $(COMMAND_ECHO)-$(RMHACK2) $(MISC)/$(SHL2TARGET).exp
    $(COMMAND_ECHO)$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL2TARGET).exp $(SLB)/$(DEFLIB2NAME).lib
    $(COMMAND_ECHO)$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL2TARGET).flt $(MISC)/$(SHL2TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK2) $(MISC)/$(SHL2TARGET).exp
.ELSE			# "$(SHL2USE_EXPORTS)"==""
    $(COMMAND_ECHO)$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB2NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL2TARGET).direct
    $(COMMAND_ECHO)$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF2FILTER) $(MISC)/$(SHL2TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL2USE_EXPORTS)"==""
.ENDIF
.ENDIF				# "$(DEFLIB2NAME)"!=""
.IF "$(DEF2EXPORT1)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT2)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT3)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT4)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT5)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT6)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT7)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT8)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT9)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT10)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT11)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT12)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT13)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT14)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT15)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT16)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT17)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT18)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT19)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORT20)"!=""
    $(COMMAND_ECHO)echo $(DEF2EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF2EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(TYPE) $(DEF2EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF2EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF2TARGETN): \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
    $(COMMAND_ECHO)echo just a dummy for UNIX > $@
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
DEF3DEPN+=$(foreach,i,$(DEFLIB3NAME) $(SLB)/$(i).lib)
.ENDIF

.IF "$(SHL3VERSIONMAP)"!=""
.IF "$(DEF3EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF3EXPORTFILE=$(MISC)/$(SHL3VERSIONMAP:b)_$(SHL3TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF3EXPORTFILE) : $(SHL3OBJS) $(SHL3LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF3EXPORTFILE) : $(SHL3VERSIONMAP)
    $(COMMAND_ECHO)$(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(GREP) -v "\*\|?" $@ | $(SED) -e 's@#.*@@' > $@.exported-symbols
    $(COMMAND_ECHO)-$(GREP) "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
    $(COMMAND_ECHO)$(NM) -gP $(SHL3OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL3LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.ENDIF			# "$(DEF3EXPORTFILE)"==""
.ENDIF			# "$(SHL3VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF3FILTER=$(SOLARENV)/inc/dummy.flt

RMHACK3:=$(RM)

.IF "$(DEF3CEXP)"!=""
EXPORT_ALL_SWITCH=-A $(DEF3CEXP)
.ENDIF

.IF "$(link_always)"==""
$(DEF3TARGETN) : \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF3TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    $(COMMAND_ECHO)-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    $(COMMAND_ECHO)echo LIBRARY	  $(EMQ)"$(SHL3TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    $(COMMAND_ECHO)echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    $(COMMAND_ECHO)echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    $(COMMAND_ECHO)echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB3NAME)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL3TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL3TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB3NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL3TARGET).exp | sed '/^;/d' >>$@.tmpfile
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL3TARGET).exp
.ELSE
.IF "$(SHL3USE_EXPORTS)"==""
    $(COMMAND_ECHO)-$(RMHACK3) $(MISC)/$(SHL3TARGET).exp
    $(COMMAND_ECHO)$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL3TARGET).exp $(SLB)/$(DEFLIB3NAME).lib
    $(COMMAND_ECHO)$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL3TARGET).flt $(MISC)/$(SHL3TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK3) $(MISC)/$(SHL3TARGET).exp
.ELSE			# "$(SHL3USE_EXPORTS)"==""
    $(COMMAND_ECHO)$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB3NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL3TARGET).direct
    $(COMMAND_ECHO)$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF3FILTER) $(MISC)/$(SHL3TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL3USE_EXPORTS)"==""
.ENDIF
.ENDIF				# "$(DEFLIB3NAME)"!=""
.IF "$(DEF3EXPORT1)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT2)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT3)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT4)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT5)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT6)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT7)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT8)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT9)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT10)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT11)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT12)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT13)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT14)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT15)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT16)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT17)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT18)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT19)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORT20)"!=""
    $(COMMAND_ECHO)echo $(DEF3EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF3EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(TYPE) $(DEF3EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF3EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF3TARGETN): \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
    $(COMMAND_ECHO)echo just a dummy for UNIX > $@
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
DEF4DEPN+=$(foreach,i,$(DEFLIB4NAME) $(SLB)/$(i).lib)
.ENDIF

.IF "$(SHL4VERSIONMAP)"!=""
.IF "$(DEF4EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF4EXPORTFILE=$(MISC)/$(SHL4VERSIONMAP:b)_$(SHL4TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF4EXPORTFILE) : $(SHL4OBJS) $(SHL4LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF4EXPORTFILE) : $(SHL4VERSIONMAP)
    $(COMMAND_ECHO)$(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(GREP) -v "\*\|?" $@ | $(SED) -e 's@#.*@@' > $@.exported-symbols
    $(COMMAND_ECHO)-$(GREP) "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
    $(COMMAND_ECHO)$(NM) -gP $(SHL4OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL4LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.ENDIF			# "$(DEF4EXPORTFILE)"==""
.ENDIF			# "$(SHL4VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF4FILTER=$(SOLARENV)/inc/dummy.flt

RMHACK4:=$(RM)

.IF "$(DEF4CEXP)"!=""
EXPORT_ALL_SWITCH=-A $(DEF4CEXP)
.ENDIF

.IF "$(link_always)"==""
$(DEF4TARGETN) : \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF4TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    $(COMMAND_ECHO)-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    $(COMMAND_ECHO)echo LIBRARY	  $(EMQ)"$(SHL4TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    $(COMMAND_ECHO)echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    $(COMMAND_ECHO)echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    $(COMMAND_ECHO)echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB4NAME)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL4TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL4TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB4NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL4TARGET).exp | sed '/^;/d' >>$@.tmpfile
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL4TARGET).exp
.ELSE
.IF "$(SHL4USE_EXPORTS)"==""
    $(COMMAND_ECHO)-$(RMHACK4) $(MISC)/$(SHL4TARGET).exp
    $(COMMAND_ECHO)$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL4TARGET).exp $(SLB)/$(DEFLIB4NAME).lib
    $(COMMAND_ECHO)$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL4TARGET).flt $(MISC)/$(SHL4TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK4) $(MISC)/$(SHL4TARGET).exp
.ELSE			# "$(SHL4USE_EXPORTS)"==""
    $(COMMAND_ECHO)$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB4NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL4TARGET).direct
    $(COMMAND_ECHO)$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF4FILTER) $(MISC)/$(SHL4TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL4USE_EXPORTS)"==""
.ENDIF
.ENDIF				# "$(DEFLIB4NAME)"!=""
.IF "$(DEF4EXPORT1)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT2)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT3)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT4)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT5)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT6)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT7)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT8)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT9)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT10)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT11)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT12)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT13)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT14)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT15)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT16)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT17)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT18)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT19)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORT20)"!=""
    $(COMMAND_ECHO)echo $(DEF4EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF4EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(TYPE) $(DEF4EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF4EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF4TARGETN): \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
    $(COMMAND_ECHO)echo just a dummy for UNIX > $@
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
DEF5DEPN+=$(foreach,i,$(DEFLIB5NAME) $(SLB)/$(i).lib)
.ENDIF

.IF "$(SHL5VERSIONMAP)"!=""
.IF "$(DEF5EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF5EXPORTFILE=$(MISC)/$(SHL5VERSIONMAP:b)_$(SHL5TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF5EXPORTFILE) : $(SHL5OBJS) $(SHL5LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF5EXPORTFILE) : $(SHL5VERSIONMAP)
    $(COMMAND_ECHO)$(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(GREP) -v "\*\|?" $@ | $(SED) -e 's@#.*@@' > $@.exported-symbols
    $(COMMAND_ECHO)-$(GREP) "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
    $(COMMAND_ECHO)$(NM) -gP $(SHL5OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL5LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.ENDIF			# "$(DEF5EXPORTFILE)"==""
.ENDIF			# "$(SHL5VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF5FILTER=$(SOLARENV)/inc/dummy.flt

RMHACK5:=$(RM)

.IF "$(DEF5CEXP)"!=""
EXPORT_ALL_SWITCH=-A $(DEF5CEXP)
.ENDIF

.IF "$(link_always)"==""
$(DEF5TARGETN) : \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF5TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    $(COMMAND_ECHO)-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    $(COMMAND_ECHO)echo LIBRARY	  $(EMQ)"$(SHL5TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    $(COMMAND_ECHO)echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    $(COMMAND_ECHO)echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    $(COMMAND_ECHO)echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB5NAME)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL5TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL5TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB5NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL5TARGET).exp | sed '/^;/d' >>$@.tmpfile
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL5TARGET).exp
.ELSE
.IF "$(SHL5USE_EXPORTS)"==""
    $(COMMAND_ECHO)-$(RMHACK5) $(MISC)/$(SHL5TARGET).exp
    $(COMMAND_ECHO)$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL5TARGET).exp $(SLB)/$(DEFLIB5NAME).lib
    $(COMMAND_ECHO)$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL5TARGET).flt $(MISC)/$(SHL5TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK5) $(MISC)/$(SHL5TARGET).exp
.ELSE			# "$(SHL5USE_EXPORTS)"==""
    $(COMMAND_ECHO)$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB5NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL5TARGET).direct
    $(COMMAND_ECHO)$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF5FILTER) $(MISC)/$(SHL5TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL5USE_EXPORTS)"==""
.ENDIF
.ENDIF				# "$(DEFLIB5NAME)"!=""
.IF "$(DEF5EXPORT1)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT2)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT3)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT4)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT5)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT6)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT7)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT8)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT9)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT10)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT11)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT12)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT13)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT14)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT15)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT16)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT17)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT18)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT19)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORT20)"!=""
    $(COMMAND_ECHO)echo $(DEF5EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF5EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(TYPE) $(DEF5EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF5EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF5TARGETN): \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
    $(COMMAND_ECHO)echo just a dummy for UNIX > $@
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
DEF6DEPN+=$(foreach,i,$(DEFLIB6NAME) $(SLB)/$(i).lib)
.ENDIF

.IF "$(SHL6VERSIONMAP)"!=""
.IF "$(DEF6EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF6EXPORTFILE=$(MISC)/$(SHL6VERSIONMAP:b)_$(SHL6TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF6EXPORTFILE) : $(SHL6OBJS) $(SHL6LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF6EXPORTFILE) : $(SHL6VERSIONMAP)
    $(COMMAND_ECHO)$(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(GREP) -v "\*\|?" $@ | $(SED) -e 's@#.*@@' > $@.exported-symbols
    $(COMMAND_ECHO)-$(GREP) "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
    $(COMMAND_ECHO)$(NM) -gP $(SHL6OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL6LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.ENDIF			# "$(DEF6EXPORTFILE)"==""
.ENDIF			# "$(SHL6VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF6FILTER=$(SOLARENV)/inc/dummy.flt

RMHACK6:=$(RM)

.IF "$(DEF6CEXP)"!=""
EXPORT_ALL_SWITCH=-A $(DEF6CEXP)
.ENDIF

.IF "$(link_always)"==""
$(DEF6TARGETN) : \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF6TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    $(COMMAND_ECHO)-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    $(COMMAND_ECHO)echo LIBRARY	  $(EMQ)"$(SHL6TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    $(COMMAND_ECHO)echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    $(COMMAND_ECHO)echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    $(COMMAND_ECHO)echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB6NAME)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL6TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL6TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB6NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL6TARGET).exp | sed '/^;/d' >>$@.tmpfile
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL6TARGET).exp
.ELSE
.IF "$(SHL6USE_EXPORTS)"==""
    $(COMMAND_ECHO)-$(RMHACK6) $(MISC)/$(SHL6TARGET).exp
    $(COMMAND_ECHO)$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL6TARGET).exp $(SLB)/$(DEFLIB6NAME).lib
    $(COMMAND_ECHO)$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL6TARGET).flt $(MISC)/$(SHL6TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK6) $(MISC)/$(SHL6TARGET).exp
.ELSE			# "$(SHL6USE_EXPORTS)"==""
    $(COMMAND_ECHO)$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB6NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL6TARGET).direct
    $(COMMAND_ECHO)$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF6FILTER) $(MISC)/$(SHL6TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL6USE_EXPORTS)"==""
.ENDIF
.ENDIF				# "$(DEFLIB6NAME)"!=""
.IF "$(DEF6EXPORT1)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT2)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT3)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT4)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT5)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT6)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT7)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT8)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT9)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT10)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT11)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT12)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT13)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT14)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT15)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT16)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT17)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT18)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT19)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORT20)"!=""
    $(COMMAND_ECHO)echo $(DEF6EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF6EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(TYPE) $(DEF6EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF6EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF6TARGETN): \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
    $(COMMAND_ECHO)echo just a dummy for UNIX > $@
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
DEF7DEPN+=$(foreach,i,$(DEFLIB7NAME) $(SLB)/$(i).lib)
.ENDIF

.IF "$(SHL7VERSIONMAP)"!=""
.IF "$(DEF7EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF7EXPORTFILE=$(MISC)/$(SHL7VERSIONMAP:b)_$(SHL7TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF7EXPORTFILE) : $(SHL7OBJS) $(SHL7LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF7EXPORTFILE) : $(SHL7VERSIONMAP)
    $(COMMAND_ECHO)$(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(GREP) -v "\*\|?" $@ | $(SED) -e 's@#.*@@' > $@.exported-symbols
    $(COMMAND_ECHO)-$(GREP) "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
    $(COMMAND_ECHO)$(NM) -gP $(SHL7OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL7LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.ENDIF			# "$(DEF7EXPORTFILE)"==""
.ENDIF			# "$(SHL7VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF7FILTER=$(SOLARENV)/inc/dummy.flt

RMHACK7:=$(RM)

.IF "$(DEF7CEXP)"!=""
EXPORT_ALL_SWITCH=-A $(DEF7CEXP)
.ENDIF

.IF "$(link_always)"==""
$(DEF7TARGETN) : \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF7TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    $(COMMAND_ECHO)-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    $(COMMAND_ECHO)echo LIBRARY	  $(EMQ)"$(SHL7TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    $(COMMAND_ECHO)echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    $(COMMAND_ECHO)echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    $(COMMAND_ECHO)echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB7NAME)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL7TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL7TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB7NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL7TARGET).exp | sed '/^;/d' >>$@.tmpfile
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL7TARGET).exp
.ELSE
.IF "$(SHL7USE_EXPORTS)"==""
    $(COMMAND_ECHO)-$(RMHACK7) $(MISC)/$(SHL7TARGET).exp
    $(COMMAND_ECHO)$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL7TARGET).exp $(SLB)/$(DEFLIB7NAME).lib
    $(COMMAND_ECHO)$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL7TARGET).flt $(MISC)/$(SHL7TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK7) $(MISC)/$(SHL7TARGET).exp
.ELSE			# "$(SHL7USE_EXPORTS)"==""
    $(COMMAND_ECHO)$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB7NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL7TARGET).direct
    $(COMMAND_ECHO)$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF7FILTER) $(MISC)/$(SHL7TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL7USE_EXPORTS)"==""
.ENDIF
.ENDIF				# "$(DEFLIB7NAME)"!=""
.IF "$(DEF7EXPORT1)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT2)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT3)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT4)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT5)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT6)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT7)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT8)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT9)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT10)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT11)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT12)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT13)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT14)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT15)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT16)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT17)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT18)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT19)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORT20)"!=""
    $(COMMAND_ECHO)echo $(DEF7EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF7EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(TYPE) $(DEF7EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF7EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF7TARGETN): \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
    $(COMMAND_ECHO)echo just a dummy for UNIX > $@
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
DEF8DEPN+=$(foreach,i,$(DEFLIB8NAME) $(SLB)/$(i).lib)
.ENDIF

.IF "$(SHL8VERSIONMAP)"!=""
.IF "$(DEF8EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF8EXPORTFILE=$(MISC)/$(SHL8VERSIONMAP:b)_$(SHL8TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF8EXPORTFILE) : $(SHL8OBJS) $(SHL8LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF8EXPORTFILE) : $(SHL8VERSIONMAP)
    $(COMMAND_ECHO)$(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(GREP) -v "\*\|?" $@ | $(SED) -e 's@#.*@@' > $@.exported-symbols
    $(COMMAND_ECHO)-$(GREP) "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
    $(COMMAND_ECHO)$(NM) -gP $(SHL8OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL8LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.ENDIF			# "$(DEF8EXPORTFILE)"==""
.ENDIF			# "$(SHL8VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF8FILTER=$(SOLARENV)/inc/dummy.flt

RMHACK8:=$(RM)

.IF "$(DEF8CEXP)"!=""
EXPORT_ALL_SWITCH=-A $(DEF8CEXP)
.ENDIF

.IF "$(link_always)"==""
$(DEF8TARGETN) : \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF8TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    $(COMMAND_ECHO)-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    $(COMMAND_ECHO)echo LIBRARY	  $(EMQ)"$(SHL8TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    $(COMMAND_ECHO)echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    $(COMMAND_ECHO)echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    $(COMMAND_ECHO)echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB8NAME)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL8TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL8TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB8NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL8TARGET).exp | sed '/^;/d' >>$@.tmpfile
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL8TARGET).exp
.ELSE
.IF "$(SHL8USE_EXPORTS)"==""
    $(COMMAND_ECHO)-$(RMHACK8) $(MISC)/$(SHL8TARGET).exp
    $(COMMAND_ECHO)$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL8TARGET).exp $(SLB)/$(DEFLIB8NAME).lib
    $(COMMAND_ECHO)$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL8TARGET).flt $(MISC)/$(SHL8TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK8) $(MISC)/$(SHL8TARGET).exp
.ELSE			# "$(SHL8USE_EXPORTS)"==""
    $(COMMAND_ECHO)$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB8NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL8TARGET).direct
    $(COMMAND_ECHO)$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF8FILTER) $(MISC)/$(SHL8TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL8USE_EXPORTS)"==""
.ENDIF
.ENDIF				# "$(DEFLIB8NAME)"!=""
.IF "$(DEF8EXPORT1)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT2)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT3)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT4)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT5)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT6)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT7)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT8)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT9)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT10)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT11)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT12)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT13)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT14)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT15)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT16)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT17)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT18)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT19)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORT20)"!=""
    $(COMMAND_ECHO)echo $(DEF8EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF8EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(TYPE) $(DEF8EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF8EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF8TARGETN): \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
    $(COMMAND_ECHO)echo just a dummy for UNIX > $@
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
DEF9DEPN+=$(foreach,i,$(DEFLIB9NAME) $(SLB)/$(i).lib)
.ENDIF

.IF "$(SHL9VERSIONMAP)"!=""
.IF "$(DEF9EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF9EXPORTFILE=$(MISC)/$(SHL9VERSIONMAP:b)_$(SHL9TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF9EXPORTFILE) : $(SHL9OBJS) $(SHL9LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF9EXPORTFILE) : $(SHL9VERSIONMAP)
    $(COMMAND_ECHO)$(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(GREP) -v "\*\|?" $@ | $(SED) -e 's@#.*@@' > $@.exported-symbols
    $(COMMAND_ECHO)-$(GREP) "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
    $(COMMAND_ECHO)$(NM) -gP $(SHL9OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL9LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.ENDIF			# "$(DEF9EXPORTFILE)"==""
.ENDIF			# "$(SHL9VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF9FILTER=$(SOLARENV)/inc/dummy.flt

RMHACK9:=$(RM)

.IF "$(DEF9CEXP)"!=""
EXPORT_ALL_SWITCH=-A $(DEF9CEXP)
.ENDIF

.IF "$(link_always)"==""
$(DEF9TARGETN) : \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF9TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    $(COMMAND_ECHO)-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    $(COMMAND_ECHO)echo LIBRARY	  $(EMQ)"$(SHL9TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    $(COMMAND_ECHO)echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    $(COMMAND_ECHO)echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    $(COMMAND_ECHO)echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB9NAME)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL9TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL9TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB9NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL9TARGET).exp | sed '/^;/d' >>$@.tmpfile
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL9TARGET).exp
.ELSE
.IF "$(SHL9USE_EXPORTS)"==""
    $(COMMAND_ECHO)-$(RMHACK9) $(MISC)/$(SHL9TARGET).exp
    $(COMMAND_ECHO)$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL9TARGET).exp $(SLB)/$(DEFLIB9NAME).lib
    $(COMMAND_ECHO)$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL9TARGET).flt $(MISC)/$(SHL9TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK9) $(MISC)/$(SHL9TARGET).exp
.ELSE			# "$(SHL9USE_EXPORTS)"==""
    $(COMMAND_ECHO)$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB9NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL9TARGET).direct
    $(COMMAND_ECHO)$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF9FILTER) $(MISC)/$(SHL9TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL9USE_EXPORTS)"==""
.ENDIF
.ENDIF				# "$(DEFLIB9NAME)"!=""
.IF "$(DEF9EXPORT1)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT2)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT3)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT4)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT5)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT6)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT7)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT8)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT9)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT10)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT11)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT12)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT13)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT14)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT15)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT16)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT17)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT18)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT19)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORT20)"!=""
    $(COMMAND_ECHO)echo $(DEF9EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF9EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(TYPE) $(DEF9EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF9EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF9TARGETN): \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
    $(COMMAND_ECHO)echo just a dummy for UNIX > $@
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
DEF10DEPN+=$(foreach,i,$(DEFLIB10NAME) $(SLB)/$(i).lib)
.ENDIF

.IF "$(SHL10VERSIONMAP)"!=""
.IF "$(DEF10EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF10EXPORTFILE=$(MISC)/$(SHL10VERSIONMAP:b)_$(SHL10TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF10EXPORTFILE) : $(SHL10OBJS) $(SHL10LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF10EXPORTFILE) : $(SHL10VERSIONMAP)
    $(COMMAND_ECHO)$(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(GREP) -v "\*\|?" $@ | $(SED) -e 's@#.*@@' > $@.exported-symbols
    $(COMMAND_ECHO)-$(GREP) "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
    $(COMMAND_ECHO)$(NM) -gP $(SHL10OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL10LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.ENDIF			# "$(DEF10EXPORTFILE)"==""
.ENDIF			# "$(SHL10VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF10FILTER=$(SOLARENV)/inc/dummy.flt

RMHACK10:=$(RM)

.IF "$(DEF10CEXP)"!=""
EXPORT_ALL_SWITCH=-A $(DEF10CEXP)
.ENDIF

.IF "$(link_always)"==""
$(DEF10TARGETN) : \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF10TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    $(COMMAND_ECHO)-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    $(COMMAND_ECHO)echo LIBRARY	  $(EMQ)"$(SHL10TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    $(COMMAND_ECHO)echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    $(COMMAND_ECHO)echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    $(COMMAND_ECHO)echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB10NAME)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL10TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL10TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB10NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL10TARGET).exp | sed '/^;/d' >>$@.tmpfile
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL10TARGET).exp
.ELSE
.IF "$(SHL10USE_EXPORTS)"==""
    $(COMMAND_ECHO)-$(RMHACK10) $(MISC)/$(SHL10TARGET).exp
    $(COMMAND_ECHO)$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL10TARGET).exp $(SLB)/$(DEFLIB10NAME).lib
    $(COMMAND_ECHO)$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL10TARGET).flt $(MISC)/$(SHL10TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK10) $(MISC)/$(SHL10TARGET).exp
.ELSE			# "$(SHL10USE_EXPORTS)"==""
    $(COMMAND_ECHO)$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB10NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL10TARGET).direct
    $(COMMAND_ECHO)$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF10FILTER) $(MISC)/$(SHL10TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL10USE_EXPORTS)"==""
.ENDIF
.ENDIF				# "$(DEFLIB10NAME)"!=""
.IF "$(DEF10EXPORT1)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT2)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT3)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT4)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT5)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT6)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT7)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT8)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT9)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT10)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT11)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT12)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT13)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT14)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT15)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT16)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT17)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT18)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT19)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORT20)"!=""
    $(COMMAND_ECHO)echo $(DEF10EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF10EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(TYPE) $(DEF10EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF10EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF10TARGETN): \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
    $(COMMAND_ECHO)echo just a dummy for UNIX > $@
.ENDIF
.ENDIF			# "$(DEF10TARGETN)"!=""


# Anweisungen fuer das Linken
