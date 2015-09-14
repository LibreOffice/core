#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(DEF$(TNR)TARGETN)"!=""

#to make intuitiv naming work
.IF "$(DEF$(TNR)LIBNAME)"!=""
DEFLIB$(TNR)NAME*=$(DEF$(TNR)LIBNAME)
.ENDIF			# "$(DEF$(TNR)LIBNAME)"!=""

.IF "$(DEFLIB$(TNR)NAME)"!=""
DEF$(TNR)DEPN+=$(foreach,i,$(DEFLIB$(TNR)NAME) $(SLB)/$(i).lib)
.ENDIF

.IF "$(SHL$(TNR)VERSIONMAP)"!=""
.IF "$(DEF$(TNR)EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF$(TNR)EXPORTFILE=$(MISC)/$(SHL$(TNR)VERSIONMAP:b)_$(SHL$(TNR)TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF$(TNR)EXPORTFILE) : $(SHL$(TNR)OBJS) $(SHL$(TNR)LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF$(TNR)EXPORTFILE) : $(SHL$(TNR)VERSIONMAP)
    $(COMMAND_ECHO)$(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(GREP) -v "\*\|?" $@ | $(SED) -e 's@#.*@@' > $@.exported-symbols
    $(COMMAND_ECHO)-$(GREP) "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
    $(COMMAND_ECHO)nm -gP $(SHL$(TNR)OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL$(TNR)LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
DEF$(TNR)EXPORTFILE=$(MISC)/$(SHL$(TNR)VERSIONMAP:b)_$(SHL$(TNR)TARGET).dxp
$(DEF$(TNR)EXPORTFILE) : $(SHL$(TNR)VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.ENDIF			# "$(GUI)"=="OS2"

.ENDIF			# "$(DEF$(TNR)EXPORTFILE)"==""
.ENDIF			# "$(SHL$(TNR)VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF$(TNR)FILTER=$(SOLARENV)/inc/dummy.flt

RMHACK$(TNR):=$(RM)

.IF "$(DEF$(TNR)CEXP)"!=""
EXPORT_ALL_SWITCH=-A $(DEF$(TNR)CEXP)
.ENDIF

.IF "$(link_always)"==""
$(DEF$(TNR)TARGETN) : \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF$(TNR)TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    @-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(EMQ)"$(SHL$(TNR)TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB$(TNR)NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(RM) $(MISC)/$(SHL$(TNR)TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL$(TNR)TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB$(TNR)NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL$(TNR)TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(RM) $(MISC)/$(SHL$(TNR)TARGET).exp
.ELSE
.IF "$(SHL$(TNR)USE_EXPORTS)"==""
    @-$(RMHACK$(TNR)) $(MISC)/$(SHL$(TNR)TARGET).exp
    @$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL$(TNR)TARGET).exp $(SLB)/$(DEFLIB$(TNR)NAME).lib
    @$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL$(TNR)TARGET).flt $(MISC)/$(SHL$(TNR)TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK$(TNR)) $(MISC)/$(SHL$(TNR)TARGET).exp
.ELSE			# "$(SHL$(TNR)USE_EXPORTS)"==""
    @$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB$(TNR)NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL$(TNR)TARGET).direct
    @$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF$(TNR)FILTER) $(MISC)/$(SHL$(TNR)TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL$(TNR)USE_EXPORTS)"==""
.ENDIF
.ENDIF				# "$(DEFLIB$(TNR)NAME)"!=""
.IF "$(DEF$(TNR)EXPORT1)"!=""
    @echo $(DEF$(TNR)EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT2)"!=""
    @echo $(DEF$(TNR)EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT3)"!=""
    @echo $(DEF$(TNR)EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT4)"!=""
    @echo $(DEF$(TNR)EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT5)"!=""
    @echo $(DEF$(TNR)EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT6)"!=""
    @echo $(DEF$(TNR)EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT7)"!=""
    @echo $(DEF$(TNR)EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT8)"!=""
    @echo $(DEF$(TNR)EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT9)"!=""
    @echo $(DEF$(TNR)EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT10)"!=""
    @echo $(DEF$(TNR)EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT11)"!=""
    @echo $(DEF$(TNR)EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT12)"!=""
    @echo $(DEF$(TNR)EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT13)"!=""
    @echo $(DEF$(TNR)EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT14)"!=""
    @echo $(DEF$(TNR)EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT15)"!=""
    @echo $(DEF$(TNR)EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT16)"!=""
    @echo $(DEF$(TNR)EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT17)"!=""
    @echo $(DEF$(TNR)EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT18)"!=""
    @echo $(DEF$(TNR)EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT19)"!=""
    @echo $(DEF$(TNR)EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT20)"!=""
    @echo $(DEF$(TNR)EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(TYPE) $(DEF$(TNR)EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF$(TNR)EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"

#21/02/2006 YD dll names must be 8.3, invoke fix script
#check osl/os2/module.c/osl_loadModule()
SHL$(TNR)TARGET8=$(shell @fix_shl.cmd $(SHL$(TNR)TARGETN:f))

DEF$(TNR)FILTER=$(SOLARENV)/inc/dummy.flt
DEF$(TNR)NAMELIST=$(foreach,i,$(DEFLIB$(TNR)NAME) $(SLB)/$(i).lib)

.IF "$(link_always)"==""
$(DEF$(TNR)TARGETN) : \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF$(TNR)TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    @+-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(SHL$(TNR)TARGET8) INITINSTANCE TERMINSTANCE	 >$@.tmpfile
    @echo DATA MULTIPLE	 >>$@.tmpfile
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo _GetVersionInfo		>$@.tmp_ord
.ENDIF

.IF "$(DEFLIB$(TNR)NAME)"!=""
    @+echo $(SLB)/$(DEFLIB$(TNR)NAME).lib
    @+emxexp $(DEF$(TNR)NAMELIST) | fix_exp_file >> $@.tmp_ord
.ENDIF				# "$(DEFLIB$(TNR)NAME)"!=""

.IF "$(DEF$(TNR)EXPORT1)"!=""
    @echo $(DEF$(TNR)EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT2)"!=""
    @echo $(DEF$(TNR)EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT3)"!=""
    @echo $(DEF$(TNR)EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT4)"!=""
    @echo $(DEF$(TNR)EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT5)"!=""
    @echo $(DEF$(TNR)EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT6)"!=""
    @echo $(DEF$(TNR)EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT7)"!=""
    @echo $(DEF$(TNR)EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT8)"!=""
    @echo $(DEF$(TNR)EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT9)"!=""
    @echo $(DEF$(TNR)EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT10)"!=""
    @echo $(DEF$(TNR)EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT11)"!=""
    @echo $(DEF$(TNR)EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT12)"!=""
    @echo $(DEF$(TNR)EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT13)"!=""
    @echo $(DEF$(TNR)EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT14)"!=""
    @echo $(DEF$(TNR)EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT15)"!=""
    @echo $(DEF$(TNR)EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT16)"!=""
    @echo $(DEF$(TNR)EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT17)"!=""
    @echo $(DEF$(TNR)EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT18)"!=""
    @echo $(DEF$(TNR)EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT19)"!=""
    @echo $(DEF$(TNR)EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT20)"!=""
    @echo $(DEF$(TNR)EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORTFILE)"!=""
    @fix_def_file < $(DEF$(TNR)EXPORTFILE) >> $@.tmp_ord
.ENDIF
    @sort < $@.tmp_ord | uniq > $@.exptmpfile
    @fix_def_ord < $@.exptmpfile >> $@.tmpfile
    @+-$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $@.tmpfile $@
    @+-$(RM) $@.tmp_ord
    @+-$(RM) $@.exptmpfile
.IF "$(SHL$(TNR)USE_EXPORTS)"=="" || "$(SHL$(TNR)USE_EXPORTS)"=="ordinal"
#03/11/2012  if 'name' is not used, export from def file (with ordinals)
    @echo Build import library from DEF file.
    @+-$(RM) $(LB)/$(SHL$(TNR)TARGETN:b).lib
    $(COMMAND_ECHO)+$(IMPLIB) $(IMPLIBFLAGS) $(LB)/$(SHL$(TNR)TARGETN:b).lib $@
.ENDIF			# "$(SHL$(TNR)USE_EXPORTS)"==""

.ENDIF			# "$(GUI)"=="OS2"

.IF "$(GUI)"=="UNX"
$(DEF$(TNR)TARGETN): \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
    @echo just a dummy for UNIX > $@
.ENDIF
.ENDIF			# "$(DEF$(TNR)TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll end
#######################################################

