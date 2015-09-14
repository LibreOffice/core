# *************************************************************
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
# *************************************************************
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
    $(COMMAND_ECHO)nm -gP $(SHL1OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL1LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
DEF1EXPORTFILE=$(MISC)/$(SHL1VERSIONMAP:b)_$(SHL1TARGET).dxp
$(DEF1EXPORTFILE) : $(SHL1VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.ENDIF			# "$(GUI)"=="OS2"

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
    @-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(EMQ)"$(SHL1TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB1NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(RM) $(MISC)/$(SHL1TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL1TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB1NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL1TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(RM) $(MISC)/$(SHL1TARGET).exp
.ELSE
.IF "$(SHL1USE_EXPORTS)"==""
    @-$(RMHACK1) $(MISC)/$(SHL1TARGET).exp
    @$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL1TARGET).exp $(SLB)/$(DEFLIB1NAME).lib
    @$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL1TARGET).flt $(MISC)/$(SHL1TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK1) $(MISC)/$(SHL1TARGET).exp
.ELSE			# "$(SHL1USE_EXPORTS)"==""
    @$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB1NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL1TARGET).direct
    @$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF1FILTER) $(MISC)/$(SHL1TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL1USE_EXPORTS)"==""
.ENDIF
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
    $(COMMAND_ECHO)$(TYPE) $(DEF1EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF1EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"

#21/02/2006 YD dll names must be 8.3, invoke fix script
#check osl/os2/module.c/osl_loadModule()
SHL1TARGET8=$(shell @fix_shl.cmd $(SHL1TARGETN:f))

DEF1FILTER=$(SOLARENV)/inc/dummy.flt
DEF1NAMELIST=$(foreach,i,$(DEFLIB1NAME) $(SLB)/$(i).lib)

.IF "$(link_always)"==""
$(DEF1TARGETN) : \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF1TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    @+-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(SHL1TARGET8) INITINSTANCE TERMINSTANCE	 >$@.tmpfile
    @echo DATA MULTIPLE	 >>$@.tmpfile
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo _GetVersionInfo		>$@.tmp_ord
.ENDIF

.IF "$(DEFLIB1NAME)"!=""
    @+echo $(SLB)/$(DEFLIB1NAME).lib
    @+emxexp $(DEF1NAMELIST) | fix_exp_file >> $@.tmp_ord
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
    @fix_def_file < $(DEF1EXPORTFILE) >> $@.tmp_ord
.ENDIF
    @sort < $@.tmp_ord | uniq > $@.exptmpfile
    @fix_def_ord < $@.exptmpfile >> $@.tmpfile
    @+-$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $@.tmpfile $@
    @+-$(RM) $@.tmp_ord
    @+-$(RM) $@.exptmpfile
.IF "$(SHL1USE_EXPORTS)"=="" || "$(SHL1USE_EXPORTS)"=="ordinal"
#03/11/2012  if 'name' is not used, export from def file (with ordinals)
    @echo Build import library from DEF file.
    @+-$(RM) $(LB)/$(SHL1TARGETN:b).lib
    $(COMMAND_ECHO)+$(IMPLIB) $(IMPLIBFLAGS) $(LB)/$(SHL1TARGETN:b).lib $@
.ENDIF			# "$(SHL1USE_EXPORTS)"==""

.ENDIF			# "$(GUI)"=="OS2"

.IF "$(GUI)"=="UNX"
$(DEF1TARGETN): \
        $(DEF1DEPN) \
        $(DEF1EXPORTFILE)
    @echo just a dummy for UNIX > $@
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
    $(COMMAND_ECHO)nm -gP $(SHL2OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL2LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
DEF2EXPORTFILE=$(MISC)/$(SHL2VERSIONMAP:b)_$(SHL2TARGET).dxp
$(DEF2EXPORTFILE) : $(SHL2VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.ENDIF			# "$(GUI)"=="OS2"

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
    @-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(EMQ)"$(SHL2TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB2NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(RM) $(MISC)/$(SHL2TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL2TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB2NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL2TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(RM) $(MISC)/$(SHL2TARGET).exp
.ELSE
.IF "$(SHL2USE_EXPORTS)"==""
    @-$(RMHACK2) $(MISC)/$(SHL2TARGET).exp
    @$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL2TARGET).exp $(SLB)/$(DEFLIB2NAME).lib
    @$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL2TARGET).flt $(MISC)/$(SHL2TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK2) $(MISC)/$(SHL2TARGET).exp
.ELSE			# "$(SHL2USE_EXPORTS)"==""
    @$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB2NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL2TARGET).direct
    @$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF2FILTER) $(MISC)/$(SHL2TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL2USE_EXPORTS)"==""
.ENDIF
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
    $(COMMAND_ECHO)$(TYPE) $(DEF2EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF2EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"

#21/02/2006 YD dll names must be 8.3, invoke fix script
#check osl/os2/module.c/osl_loadModule()
SHL2TARGET8=$(shell @fix_shl.cmd $(SHL2TARGETN:f))

DEF2FILTER=$(SOLARENV)/inc/dummy.flt
DEF2NAMELIST=$(foreach,i,$(DEFLIB2NAME) $(SLB)/$(i).lib)

.IF "$(link_always)"==""
$(DEF2TARGETN) : \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF2TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    @+-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(SHL2TARGET8) INITINSTANCE TERMINSTANCE	 >$@.tmpfile
    @echo DATA MULTIPLE	 >>$@.tmpfile
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo _GetVersionInfo		>$@.tmp_ord
.ENDIF

.IF "$(DEFLIB2NAME)"!=""
    @+echo $(SLB)/$(DEFLIB2NAME).lib
    @+emxexp $(DEF2NAMELIST) | fix_exp_file >> $@.tmp_ord
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
    @fix_def_file < $(DEF2EXPORTFILE) >> $@.tmp_ord
.ENDIF
    @sort < $@.tmp_ord | uniq > $@.exptmpfile
    @fix_def_ord < $@.exptmpfile >> $@.tmpfile
    @+-$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $@.tmpfile $@
    @+-$(RM) $@.tmp_ord
    @+-$(RM) $@.exptmpfile
.IF "$(SHL2USE_EXPORTS)"=="" || "$(SHL2USE_EXPORTS)"=="ordinal"
#03/11/2012  if 'name' is not used, export from def file (with ordinals)
    @echo Build import library from DEF file.
    @+-$(RM) $(LB)/$(SHL2TARGETN:b).lib
    $(COMMAND_ECHO)+$(IMPLIB) $(IMPLIBFLAGS) $(LB)/$(SHL2TARGETN:b).lib $@
.ENDIF			# "$(SHL2USE_EXPORTS)"==""

.ENDIF			# "$(GUI)"=="OS2"

.IF "$(GUI)"=="UNX"
$(DEF2TARGETN): \
        $(DEF2DEPN) \
        $(DEF2EXPORTFILE)
    @echo just a dummy for UNIX > $@
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
    $(COMMAND_ECHO)nm -gP $(SHL3OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL3LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
DEF3EXPORTFILE=$(MISC)/$(SHL3VERSIONMAP:b)_$(SHL3TARGET).dxp
$(DEF3EXPORTFILE) : $(SHL3VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.ENDIF			# "$(GUI)"=="OS2"

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
    @-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(EMQ)"$(SHL3TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB3NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(RM) $(MISC)/$(SHL3TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL3TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB3NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL3TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(RM) $(MISC)/$(SHL3TARGET).exp
.ELSE
.IF "$(SHL3USE_EXPORTS)"==""
    @-$(RMHACK3) $(MISC)/$(SHL3TARGET).exp
    @$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL3TARGET).exp $(SLB)/$(DEFLIB3NAME).lib
    @$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL3TARGET).flt $(MISC)/$(SHL3TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK3) $(MISC)/$(SHL3TARGET).exp
.ELSE			# "$(SHL3USE_EXPORTS)"==""
    @$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB3NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL3TARGET).direct
    @$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF3FILTER) $(MISC)/$(SHL3TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL3USE_EXPORTS)"==""
.ENDIF
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
    $(COMMAND_ECHO)$(TYPE) $(DEF3EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF3EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"

#21/02/2006 YD dll names must be 8.3, invoke fix script
#check osl/os2/module.c/osl_loadModule()
SHL3TARGET8=$(shell @fix_shl.cmd $(SHL3TARGETN:f))

DEF3FILTER=$(SOLARENV)/inc/dummy.flt
DEF3NAMELIST=$(foreach,i,$(DEFLIB3NAME) $(SLB)/$(i).lib)

.IF "$(link_always)"==""
$(DEF3TARGETN) : \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF3TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    @+-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(SHL3TARGET8) INITINSTANCE TERMINSTANCE	 >$@.tmpfile
    @echo DATA MULTIPLE	 >>$@.tmpfile
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo _GetVersionInfo		>$@.tmp_ord
.ENDIF

.IF "$(DEFLIB3NAME)"!=""
    @+echo $(SLB)/$(DEFLIB3NAME).lib
    @+emxexp $(DEF3NAMELIST) | fix_exp_file >> $@.tmp_ord
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
    @fix_def_file < $(DEF3EXPORTFILE) >> $@.tmp_ord
.ENDIF
    @sort < $@.tmp_ord | uniq > $@.exptmpfile
    @fix_def_ord < $@.exptmpfile >> $@.tmpfile
    @+-$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $@.tmpfile $@
    @+-$(RM) $@.tmp_ord
    @+-$(RM) $@.exptmpfile
.IF "$(SHL3USE_EXPORTS)"=="" || "$(SHL3USE_EXPORTS)"=="ordinal"
#03/11/2012  if 'name' is not used, export from def file (with ordinals)
    @echo Build import library from DEF file.
    @+-$(RM) $(LB)/$(SHL3TARGETN:b).lib
    $(COMMAND_ECHO)+$(IMPLIB) $(IMPLIBFLAGS) $(LB)/$(SHL3TARGETN:b).lib $@
.ENDIF			# "$(SHL3USE_EXPORTS)"==""

.ENDIF			# "$(GUI)"=="OS2"

.IF "$(GUI)"=="UNX"
$(DEF3TARGETN): \
        $(DEF3DEPN) \
        $(DEF3EXPORTFILE)
    @echo just a dummy for UNIX > $@
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
    $(COMMAND_ECHO)nm -gP $(SHL4OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL4LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
DEF4EXPORTFILE=$(MISC)/$(SHL4VERSIONMAP:b)_$(SHL4TARGET).dxp
$(DEF4EXPORTFILE) : $(SHL4VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.ENDIF			# "$(GUI)"=="OS2"

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
    @-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(EMQ)"$(SHL4TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB4NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(RM) $(MISC)/$(SHL4TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL4TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB4NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL4TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(RM) $(MISC)/$(SHL4TARGET).exp
.ELSE
.IF "$(SHL4USE_EXPORTS)"==""
    @-$(RMHACK4) $(MISC)/$(SHL4TARGET).exp
    @$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL4TARGET).exp $(SLB)/$(DEFLIB4NAME).lib
    @$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL4TARGET).flt $(MISC)/$(SHL4TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK4) $(MISC)/$(SHL4TARGET).exp
.ELSE			# "$(SHL4USE_EXPORTS)"==""
    @$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB4NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL4TARGET).direct
    @$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF4FILTER) $(MISC)/$(SHL4TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL4USE_EXPORTS)"==""
.ENDIF
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
    $(COMMAND_ECHO)$(TYPE) $(DEF4EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF4EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"

#21/02/2006 YD dll names must be 8.3, invoke fix script
#check osl/os2/module.c/osl_loadModule()
SHL4TARGET8=$(shell @fix_shl.cmd $(SHL4TARGETN:f))

DEF4FILTER=$(SOLARENV)/inc/dummy.flt
DEF4NAMELIST=$(foreach,i,$(DEFLIB4NAME) $(SLB)/$(i).lib)

.IF "$(link_always)"==""
$(DEF4TARGETN) : \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF4TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    @+-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(SHL4TARGET8) INITINSTANCE TERMINSTANCE	 >$@.tmpfile
    @echo DATA MULTIPLE	 >>$@.tmpfile
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo _GetVersionInfo		>$@.tmp_ord
.ENDIF

.IF "$(DEFLIB4NAME)"!=""
    @+echo $(SLB)/$(DEFLIB4NAME).lib
    @+emxexp $(DEF4NAMELIST) | fix_exp_file >> $@.tmp_ord
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
    @fix_def_file < $(DEF4EXPORTFILE) >> $@.tmp_ord
.ENDIF
    @sort < $@.tmp_ord | uniq > $@.exptmpfile
    @fix_def_ord < $@.exptmpfile >> $@.tmpfile
    @+-$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $@.tmpfile $@
    @+-$(RM) $@.tmp_ord
    @+-$(RM) $@.exptmpfile
.IF "$(SHL4USE_EXPORTS)"=="" || "$(SHL4USE_EXPORTS)"=="ordinal"
#03/11/2012  if 'name' is not used, export from def file (with ordinals)
    @echo Build import library from DEF file.
    @+-$(RM) $(LB)/$(SHL4TARGETN:b).lib
    $(COMMAND_ECHO)+$(IMPLIB) $(IMPLIBFLAGS) $(LB)/$(SHL4TARGETN:b).lib $@
.ENDIF			# "$(SHL4USE_EXPORTS)"==""

.ENDIF			# "$(GUI)"=="OS2"

.IF "$(GUI)"=="UNX"
$(DEF4TARGETN): \
        $(DEF4DEPN) \
        $(DEF4EXPORTFILE)
    @echo just a dummy for UNIX > $@
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
    $(COMMAND_ECHO)nm -gP $(SHL5OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL5LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
DEF5EXPORTFILE=$(MISC)/$(SHL5VERSIONMAP:b)_$(SHL5TARGET).dxp
$(DEF5EXPORTFILE) : $(SHL5VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.ENDIF			# "$(GUI)"=="OS2"

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
    @-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(EMQ)"$(SHL5TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB5NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(RM) $(MISC)/$(SHL5TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL5TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB5NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL5TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(RM) $(MISC)/$(SHL5TARGET).exp
.ELSE
.IF "$(SHL5USE_EXPORTS)"==""
    @-$(RMHACK5) $(MISC)/$(SHL5TARGET).exp
    @$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL5TARGET).exp $(SLB)/$(DEFLIB5NAME).lib
    @$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL5TARGET).flt $(MISC)/$(SHL5TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK5) $(MISC)/$(SHL5TARGET).exp
.ELSE			# "$(SHL5USE_EXPORTS)"==""
    @$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB5NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL5TARGET).direct
    @$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF5FILTER) $(MISC)/$(SHL5TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL5USE_EXPORTS)"==""
.ENDIF
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
    $(COMMAND_ECHO)$(TYPE) $(DEF5EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF5EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"

#21/02/2006 YD dll names must be 8.3, invoke fix script
#check osl/os2/module.c/osl_loadModule()
SHL5TARGET8=$(shell @fix_shl.cmd $(SHL5TARGETN:f))

DEF5FILTER=$(SOLARENV)/inc/dummy.flt
DEF5NAMELIST=$(foreach,i,$(DEFLIB5NAME) $(SLB)/$(i).lib)

.IF "$(link_always)"==""
$(DEF5TARGETN) : \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF5TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    @+-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(SHL5TARGET8) INITINSTANCE TERMINSTANCE	 >$@.tmpfile
    @echo DATA MULTIPLE	 >>$@.tmpfile
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo _GetVersionInfo		>$@.tmp_ord
.ENDIF

.IF "$(DEFLIB5NAME)"!=""
    @+echo $(SLB)/$(DEFLIB5NAME).lib
    @+emxexp $(DEF5NAMELIST) | fix_exp_file >> $@.tmp_ord
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
    @fix_def_file < $(DEF5EXPORTFILE) >> $@.tmp_ord
.ENDIF
    @sort < $@.tmp_ord | uniq > $@.exptmpfile
    @fix_def_ord < $@.exptmpfile >> $@.tmpfile
    @+-$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $@.tmpfile $@
    @+-$(RM) $@.tmp_ord
    @+-$(RM) $@.exptmpfile
.IF "$(SHL5USE_EXPORTS)"=="" || "$(SHL5USE_EXPORTS)"=="ordinal"
#03/11/2012  if 'name' is not used, export from def file (with ordinals)
    @echo Build import library from DEF file.
    @+-$(RM) $(LB)/$(SHL5TARGETN:b).lib
    $(COMMAND_ECHO)+$(IMPLIB) $(IMPLIBFLAGS) $(LB)/$(SHL5TARGETN:b).lib $@
.ENDIF			# "$(SHL5USE_EXPORTS)"==""

.ENDIF			# "$(GUI)"=="OS2"

.IF "$(GUI)"=="UNX"
$(DEF5TARGETN): \
        $(DEF5DEPN) \
        $(DEF5EXPORTFILE)
    @echo just a dummy for UNIX > $@
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
    $(COMMAND_ECHO)nm -gP $(SHL6OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL6LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
DEF6EXPORTFILE=$(MISC)/$(SHL6VERSIONMAP:b)_$(SHL6TARGET).dxp
$(DEF6EXPORTFILE) : $(SHL6VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.ENDIF			# "$(GUI)"=="OS2"

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
    @-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(EMQ)"$(SHL6TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB6NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(RM) $(MISC)/$(SHL6TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL6TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB6NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL6TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(RM) $(MISC)/$(SHL6TARGET).exp
.ELSE
.IF "$(SHL6USE_EXPORTS)"==""
    @-$(RMHACK6) $(MISC)/$(SHL6TARGET).exp
    @$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL6TARGET).exp $(SLB)/$(DEFLIB6NAME).lib
    @$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL6TARGET).flt $(MISC)/$(SHL6TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK6) $(MISC)/$(SHL6TARGET).exp
.ELSE			# "$(SHL6USE_EXPORTS)"==""
    @$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB6NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL6TARGET).direct
    @$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF6FILTER) $(MISC)/$(SHL6TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL6USE_EXPORTS)"==""
.ENDIF
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
    $(COMMAND_ECHO)$(TYPE) $(DEF6EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF6EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"

#21/02/2006 YD dll names must be 8.3, invoke fix script
#check osl/os2/module.c/osl_loadModule()
SHL6TARGET8=$(shell @fix_shl.cmd $(SHL6TARGETN:f))

DEF6FILTER=$(SOLARENV)/inc/dummy.flt
DEF6NAMELIST=$(foreach,i,$(DEFLIB6NAME) $(SLB)/$(i).lib)

.IF "$(link_always)"==""
$(DEF6TARGETN) : \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF6TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    @+-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(SHL6TARGET8) INITINSTANCE TERMINSTANCE	 >$@.tmpfile
    @echo DATA MULTIPLE	 >>$@.tmpfile
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo _GetVersionInfo		>$@.tmp_ord
.ENDIF

.IF "$(DEFLIB6NAME)"!=""
    @+echo $(SLB)/$(DEFLIB6NAME).lib
    @+emxexp $(DEF6NAMELIST) | fix_exp_file >> $@.tmp_ord
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
    @fix_def_file < $(DEF6EXPORTFILE) >> $@.tmp_ord
.ENDIF
    @sort < $@.tmp_ord | uniq > $@.exptmpfile
    @fix_def_ord < $@.exptmpfile >> $@.tmpfile
    @+-$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $@.tmpfile $@
    @+-$(RM) $@.tmp_ord
    @+-$(RM) $@.exptmpfile
.IF "$(SHL6USE_EXPORTS)"=="" || "$(SHL6USE_EXPORTS)"=="ordinal"
#03/11/2012  if 'name' is not used, export from def file (with ordinals)
    @echo Build import library from DEF file.
    @+-$(RM) $(LB)/$(SHL6TARGETN:b).lib
    $(COMMAND_ECHO)+$(IMPLIB) $(IMPLIBFLAGS) $(LB)/$(SHL6TARGETN:b).lib $@
.ENDIF			# "$(SHL6USE_EXPORTS)"==""

.ENDIF			# "$(GUI)"=="OS2"

.IF "$(GUI)"=="UNX"
$(DEF6TARGETN): \
        $(DEF6DEPN) \
        $(DEF6EXPORTFILE)
    @echo just a dummy for UNIX > $@
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
    $(COMMAND_ECHO)nm -gP $(SHL7OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL7LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
DEF7EXPORTFILE=$(MISC)/$(SHL7VERSIONMAP:b)_$(SHL7TARGET).dxp
$(DEF7EXPORTFILE) : $(SHL7VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.ENDIF			# "$(GUI)"=="OS2"

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
    @-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(EMQ)"$(SHL7TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB7NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(RM) $(MISC)/$(SHL7TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL7TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB7NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL7TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(RM) $(MISC)/$(SHL7TARGET).exp
.ELSE
.IF "$(SHL7USE_EXPORTS)"==""
    @-$(RMHACK7) $(MISC)/$(SHL7TARGET).exp
    @$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL7TARGET).exp $(SLB)/$(DEFLIB7NAME).lib
    @$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL7TARGET).flt $(MISC)/$(SHL7TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK7) $(MISC)/$(SHL7TARGET).exp
.ELSE			# "$(SHL7USE_EXPORTS)"==""
    @$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB7NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL7TARGET).direct
    @$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF7FILTER) $(MISC)/$(SHL7TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL7USE_EXPORTS)"==""
.ENDIF
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
    $(COMMAND_ECHO)$(TYPE) $(DEF7EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF7EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"

#21/02/2006 YD dll names must be 8.3, invoke fix script
#check osl/os2/module.c/osl_loadModule()
SHL7TARGET8=$(shell @fix_shl.cmd $(SHL7TARGETN:f))

DEF7FILTER=$(SOLARENV)/inc/dummy.flt
DEF7NAMELIST=$(foreach,i,$(DEFLIB7NAME) $(SLB)/$(i).lib)

.IF "$(link_always)"==""
$(DEF7TARGETN) : \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF7TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    @+-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(SHL7TARGET8) INITINSTANCE TERMINSTANCE	 >$@.tmpfile
    @echo DATA MULTIPLE	 >>$@.tmpfile
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo _GetVersionInfo		>$@.tmp_ord
.ENDIF

.IF "$(DEFLIB7NAME)"!=""
    @+echo $(SLB)/$(DEFLIB7NAME).lib
    @+emxexp $(DEF7NAMELIST) | fix_exp_file >> $@.tmp_ord
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
    @fix_def_file < $(DEF7EXPORTFILE) >> $@.tmp_ord
.ENDIF
    @sort < $@.tmp_ord | uniq > $@.exptmpfile
    @fix_def_ord < $@.exptmpfile >> $@.tmpfile
    @+-$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $@.tmpfile $@
    @+-$(RM) $@.tmp_ord
    @+-$(RM) $@.exptmpfile
.IF "$(SHL7USE_EXPORTS)"=="" || "$(SHL7USE_EXPORTS)"=="ordinal"
#03/11/2012  if 'name' is not used, export from def file (with ordinals)
    @echo Build import library from DEF file.
    @+-$(RM) $(LB)/$(SHL7TARGETN:b).lib
    $(COMMAND_ECHO)+$(IMPLIB) $(IMPLIBFLAGS) $(LB)/$(SHL7TARGETN:b).lib $@
.ENDIF			# "$(SHL7USE_EXPORTS)"==""

.ENDIF			# "$(GUI)"=="OS2"

.IF "$(GUI)"=="UNX"
$(DEF7TARGETN): \
        $(DEF7DEPN) \
        $(DEF7EXPORTFILE)
    @echo just a dummy for UNIX > $@
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
    $(COMMAND_ECHO)nm -gP $(SHL8OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL8LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
DEF8EXPORTFILE=$(MISC)/$(SHL8VERSIONMAP:b)_$(SHL8TARGET).dxp
$(DEF8EXPORTFILE) : $(SHL8VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.ENDIF			# "$(GUI)"=="OS2"

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
    @-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(EMQ)"$(SHL8TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB8NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(RM) $(MISC)/$(SHL8TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL8TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB8NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL8TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(RM) $(MISC)/$(SHL8TARGET).exp
.ELSE
.IF "$(SHL8USE_EXPORTS)"==""
    @-$(RMHACK8) $(MISC)/$(SHL8TARGET).exp
    @$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL8TARGET).exp $(SLB)/$(DEFLIB8NAME).lib
    @$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL8TARGET).flt $(MISC)/$(SHL8TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK8) $(MISC)/$(SHL8TARGET).exp
.ELSE			# "$(SHL8USE_EXPORTS)"==""
    @$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB8NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL8TARGET).direct
    @$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF8FILTER) $(MISC)/$(SHL8TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL8USE_EXPORTS)"==""
.ENDIF
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
    $(COMMAND_ECHO)$(TYPE) $(DEF8EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF8EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"

#21/02/2006 YD dll names must be 8.3, invoke fix script
#check osl/os2/module.c/osl_loadModule()
SHL8TARGET8=$(shell @fix_shl.cmd $(SHL8TARGETN:f))

DEF8FILTER=$(SOLARENV)/inc/dummy.flt
DEF8NAMELIST=$(foreach,i,$(DEFLIB8NAME) $(SLB)/$(i).lib)

.IF "$(link_always)"==""
$(DEF8TARGETN) : \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF8TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    @+-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(SHL8TARGET8) INITINSTANCE TERMINSTANCE	 >$@.tmpfile
    @echo DATA MULTIPLE	 >>$@.tmpfile
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo _GetVersionInfo		>$@.tmp_ord
.ENDIF

.IF "$(DEFLIB8NAME)"!=""
    @+echo $(SLB)/$(DEFLIB8NAME).lib
    @+emxexp $(DEF8NAMELIST) | fix_exp_file >> $@.tmp_ord
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
    @fix_def_file < $(DEF8EXPORTFILE) >> $@.tmp_ord
.ENDIF
    @sort < $@.tmp_ord | uniq > $@.exptmpfile
    @fix_def_ord < $@.exptmpfile >> $@.tmpfile
    @+-$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $@.tmpfile $@
    @+-$(RM) $@.tmp_ord
    @+-$(RM) $@.exptmpfile
.IF "$(SHL8USE_EXPORTS)"=="" || "$(SHL8USE_EXPORTS)"=="ordinal"
#03/11/2012  if 'name' is not used, export from def file (with ordinals)
    @echo Build import library from DEF file.
    @+-$(RM) $(LB)/$(SHL8TARGETN:b).lib
    $(COMMAND_ECHO)+$(IMPLIB) $(IMPLIBFLAGS) $(LB)/$(SHL8TARGETN:b).lib $@
.ENDIF			# "$(SHL8USE_EXPORTS)"==""

.ENDIF			# "$(GUI)"=="OS2"

.IF "$(GUI)"=="UNX"
$(DEF8TARGETN): \
        $(DEF8DEPN) \
        $(DEF8EXPORTFILE)
    @echo just a dummy for UNIX > $@
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
    $(COMMAND_ECHO)nm -gP $(SHL9OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL9LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
DEF9EXPORTFILE=$(MISC)/$(SHL9VERSIONMAP:b)_$(SHL9TARGET).dxp
$(DEF9EXPORTFILE) : $(SHL9VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.ENDIF			# "$(GUI)"=="OS2"

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
    @-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(EMQ)"$(SHL9TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB9NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(RM) $(MISC)/$(SHL9TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL9TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB9NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL9TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(RM) $(MISC)/$(SHL9TARGET).exp
.ELSE
.IF "$(SHL9USE_EXPORTS)"==""
    @-$(RMHACK9) $(MISC)/$(SHL9TARGET).exp
    @$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL9TARGET).exp $(SLB)/$(DEFLIB9NAME).lib
    @$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL9TARGET).flt $(MISC)/$(SHL9TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK9) $(MISC)/$(SHL9TARGET).exp
.ELSE			# "$(SHL9USE_EXPORTS)"==""
    @$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB9NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL9TARGET).direct
    @$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF9FILTER) $(MISC)/$(SHL9TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL9USE_EXPORTS)"==""
.ENDIF
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
    $(COMMAND_ECHO)$(TYPE) $(DEF9EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF9EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"

#21/02/2006 YD dll names must be 8.3, invoke fix script
#check osl/os2/module.c/osl_loadModule()
SHL9TARGET8=$(shell @fix_shl.cmd $(SHL9TARGETN:f))

DEF9FILTER=$(SOLARENV)/inc/dummy.flt
DEF9NAMELIST=$(foreach,i,$(DEFLIB9NAME) $(SLB)/$(i).lib)

.IF "$(link_always)"==""
$(DEF9TARGETN) : \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF9TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    @+-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(SHL9TARGET8) INITINSTANCE TERMINSTANCE	 >$@.tmpfile
    @echo DATA MULTIPLE	 >>$@.tmpfile
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo _GetVersionInfo		>$@.tmp_ord
.ENDIF

.IF "$(DEFLIB9NAME)"!=""
    @+echo $(SLB)/$(DEFLIB9NAME).lib
    @+emxexp $(DEF9NAMELIST) | fix_exp_file >> $@.tmp_ord
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
    @fix_def_file < $(DEF9EXPORTFILE) >> $@.tmp_ord
.ENDIF
    @sort < $@.tmp_ord | uniq > $@.exptmpfile
    @fix_def_ord < $@.exptmpfile >> $@.tmpfile
    @+-$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $@.tmpfile $@
    @+-$(RM) $@.tmp_ord
    @+-$(RM) $@.exptmpfile
.IF "$(SHL9USE_EXPORTS)"=="" || "$(SHL9USE_EXPORTS)"=="ordinal"
#03/11/2012  if 'name' is not used, export from def file (with ordinals)
    @echo Build import library from DEF file.
    @+-$(RM) $(LB)/$(SHL9TARGETN:b).lib
    $(COMMAND_ECHO)+$(IMPLIB) $(IMPLIBFLAGS) $(LB)/$(SHL9TARGETN:b).lib $@
.ENDIF			# "$(SHL9USE_EXPORTS)"==""

.ENDIF			# "$(GUI)"=="OS2"

.IF "$(GUI)"=="UNX"
$(DEF9TARGETN): \
        $(DEF9DEPN) \
        $(DEF9EXPORTFILE)
    @echo just a dummy for UNIX > $@
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
    $(COMMAND_ECHO)nm -gP $(SHL10OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL10LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
DEF10EXPORTFILE=$(MISC)/$(SHL10VERSIONMAP:b)_$(SHL10TARGET).dxp
$(DEF10EXPORTFILE) : $(SHL10VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.ENDIF			# "$(GUI)"=="OS2"

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
    @-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(EMQ)"$(SHL10TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB10NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(RM) $(MISC)/$(SHL10TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL10TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB10NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL10TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(RM) $(MISC)/$(SHL10TARGET).exp
.ELSE
.IF "$(SHL10USE_EXPORTS)"==""
    @-$(RMHACK10) $(MISC)/$(SHL10TARGET).exp
    @$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL10TARGET).exp $(SLB)/$(DEFLIB10NAME).lib
    @$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL10TARGET).flt $(MISC)/$(SHL10TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK10) $(MISC)/$(SHL10TARGET).exp
.ELSE			# "$(SHL10USE_EXPORTS)"==""
    @$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB10NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL10TARGET).direct
    @$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF10FILTER) $(MISC)/$(SHL10TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL10USE_EXPORTS)"==""
.ENDIF
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
    $(COMMAND_ECHO)$(TYPE) $(DEF10EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF10EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"

#21/02/2006 YD dll names must be 8.3, invoke fix script
#check osl/os2/module.c/osl_loadModule()
SHL10TARGET8=$(shell @fix_shl.cmd $(SHL10TARGETN:f))

DEF10FILTER=$(SOLARENV)/inc/dummy.flt
DEF10NAMELIST=$(foreach,i,$(DEFLIB10NAME) $(SLB)/$(i).lib)

.IF "$(link_always)"==""
$(DEF10TARGETN) : \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF10TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    @+-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    @echo LIBRARY	  $(SHL10TARGET8) INITINSTANCE TERMINSTANCE	 >$@.tmpfile
    @echo DATA MULTIPLE	 >>$@.tmpfile
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo _GetVersionInfo		>$@.tmp_ord
.ENDIF

.IF "$(DEFLIB10NAME)"!=""
    @+echo $(SLB)/$(DEFLIB10NAME).lib
    @+emxexp $(DEF10NAMELIST) | fix_exp_file >> $@.tmp_ord
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
    @fix_def_file < $(DEF10EXPORTFILE) >> $@.tmp_ord
.ENDIF
    @sort < $@.tmp_ord | uniq > $@.exptmpfile
    @fix_def_ord < $@.exptmpfile >> $@.tmpfile
    @+-$(RM) $@
    $(COMMAND_ECHO)+$(RENAME) $@.tmpfile $@
    @+-$(RM) $@.tmp_ord
    @+-$(RM) $@.exptmpfile
.IF "$(SHL10USE_EXPORTS)"=="" || "$(SHL10USE_EXPORTS)"=="ordinal"
#03/11/2012  if 'name' is not used, export from def file (with ordinals)
    @echo Build import library from DEF file.
    @+-$(RM) $(LB)/$(SHL10TARGETN:b).lib
    $(COMMAND_ECHO)+$(IMPLIB) $(IMPLIBFLAGS) $(LB)/$(SHL10TARGETN:b).lib $@
.ENDIF			# "$(SHL10USE_EXPORTS)"==""

.ENDIF			# "$(GUI)"=="OS2"

.IF "$(GUI)"=="UNX"
$(DEF10TARGETN): \
        $(DEF10DEPN) \
        $(DEF10EXPORTFILE)
    @echo just a dummy for UNIX > $@
.ENDIF
.ENDIF			# "$(DEF10TARGETN)"!=""


# Anweisungen fuer das Linken
