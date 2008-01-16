#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_dep.mk,v $
#
#   $Revision: 1.29 $
#
#   last change: $Author: ihi $ $Date: 2008-01-16 14:27:44 $
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

.IF "$(SLOFILES)$(OBJFILES)$(DEPOBJFILES)$(SRCFILES)$(SRC1FILES)$(SRC2FILES)$(SRC3FILES)$(RCFILES)$(HDBDEPNTARGET)$(IDLFILES)$(PARFILES)$(ZIP1TARGET)$(ZIP2TARGET)$(ZIP3TARGET)$(ZIP4TARGET)$(ZIP5TARGET)$(ZIP6TARGET)$(ZIP7TARGET)$(ZIP8TARGET)$(ZIP9TARGET)$(COMP1TYPELIST)$(COMP2TYPELIST)$(COMP3TYPELIST)$(COMP4TYPELIST)$(COMP5TYPELIST)$(COMP6TYPELIST)$(COMP7TYPELIST)$(COMP8TYPELIST)$(COMP9TYPELIST)"!=""
ALLDEP .PHONY: 
.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"
    @-$(IFEXIST) $(SRS)$/$(PWD:f).*.dpr $(THEN) $(RM:s/+//) $(SRS)$/$(PWD:f).*.dpr >& $(NULLDEV) $(FI)
.ELSE
    @@-$(RM) $(SRS)$/$(PWD:f).*.dpr
.ENDIF
    @@-$(RM) $(MISC)$/$(TARGET).dpr
    @@-$(RM) $(MISC)$/$(PWD:f).*.dprr
    @@-$(RM) $(MISC)$/$(TARGET).dpj
    @@-$(RM) $(MISC)$/$(TARGET).dpz
    @@-$(RM) $(MISC)$/$(COMP1TYPELIST).mk $(MISC)$/$(COMP2TYPELIST).mk $(MISC)$/$(COMP3TYPELIST).mk $(MISC)$/$(COMP4TYPELIST).mk $(MISC)$/$(COMP5TYPELIST).mk $(MISC)$/$(COMP6TYPELIST).mk $(MISC)$/$(COMP7TYPELIST).mk $(MISC)$/$(COMP8TYPELIST).mk $(MISC)$/$(COMP9TYPELIST).mk
.IF "$(nodep)"==""
.IF "$(DEPFILE_SLO)"!=""	
    @@-$(RM) $(DEPFILE_SLO)
    $(foreach,i,$(all_local_slo) $(shell @$(MAKEDEPEND) @$(mktmp -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $i ) > $(MISC)$/s_$(i:b).dpcc ))
    $(foreach,i,$(all_misc_slo) $(shell @$(MAKEDEPEND) @$(mktmp -f - -p$(SLO) $(MKDEPFLAGS) $(CDEFS) $(CDEFSSLO) $(CDEFSMT) $i ) > $(MISC)$/s_$(i:b).dpcc ))
.ENDIF			# "$(DEPFILE_SLO)"!=""	
.IF "$(DEPFILE_OBJ)"!=""	
    @@-$(RM) $(DEPFILE_OBJ)
    $(foreach,i,$(all_local_obj) $(shell @$(MAKEDEPEND) @$(mktmp -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $i ) > $(MISC)$/o_$(i:b).dpcc ))
    $(foreach,i,$(all_misc_obj) $(shell @$(MAKEDEPEND) @$(mktmp -f - -p$(OBJ) $(MKDEPFLAGS) $(CDEFS) $(CDEFSOBJ) $(CDEFSMT) $i ) > $(MISC)$/o_$(i:b).dpcc ))
.ENDIF			# "$(DEPFILE_OBJ)"!=""	
.ENDIF			# "$(nodep)"==""
.IF "$(DEPFILES)" != ""
    echo xxx$(DEPFILES)xxx
#to keep win9x happy
.IF "$(GROUPSHELL:b:l)"=="4dos"
    @@-echo $(foreach,i,$(DEPFILES) $(shell @@$(4nt_force_shell)-del $i))
.ELSE			# "$(GROUPSHELL:b)"=="4dos"
.IF "$(USE_SHELL)"=="4nt"
    @@-echo $(foreach,i,$(DEPFILES) $(shell @@$(4nt_force_shell)-del $i ))
.ELSE			# "$(USE_SHELL)"=="4nt"
    @@-$(RM) $(DEPFILES)
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF			# "$(GROUPSHELL:b)"=="4dos"
.ENDIF			# "$(DEPFILES)" != ""
    @echo ---
    @echo      $(PWD) - Old dependency files removed or updated
    @echo ---


ALLDPC: \
        $(DEPFILES) \
        $(RCFILES) \
        $(IDLFILES) \
        $(HDBDEPNTARGET)
.IF "$(nodep)"!=""
    @echo NOT-Making: Depend-Lists
    @echo ttt: ppp > $(MISC)$/$(TARGET).dpc
.ELSE
    @echo Making : Dependencies 
.IF "$(CFILES)$(CXXFILES)"!=""
.IF "$(SLOFILES)$(OBJFILES)$(DEPOBJFILES)"==""
    @echo ++++++++++++++++++++++++++++++++++++++++++++++++
    @echo 		something is wrong!!
    @echo 		source files but no obj
    @echo ++++++++++++++++++++++++++++++++++++++++++++++++
    force_dmake_to_error
.ENDIF
.ENDIF
.IF "$(HDBDEPNTARGET)$(OBJFILES)$(SLOFILES)$(DEPOBJFILES)$(RCFILES)$(PARFILES)"!=""
    @$(IFEXIST) $(MISC)$/$(TARGET).dpj $(THEN) $(RM:s/+//) $(MISC)$/$(TARGET).dpj >& $(NULLDEV) $(FI)
    @$(IFEXIST) $(MISC)$/genjava.mk $(THEN) $(RM:s/+//) $(MISC)$/genjava.mk >& $(NULLDEV) $(FI)
    $(TOUCH) $(MISC)$/$(TARGET).dpc
.IF "$($(SECOND_BUILD)_SLOFILES)"!=""
    @$(TYPE) $(mktmp $(foreach,i,$($(SECOND_BUILD)_SLOFILES) $(i:d:^"\n")$(SECOND_BUILD)_$(i:f) : $i )) >> $(MISC)$/$(TARGET).dpc
.ENDIF
.IF "$($(SECOND_BUILD)_OBJFILES)"!=""
    @$(TYPE) $(mktmp $(foreach,i,$($(SECOND_BUILD)_OBJFILES) $(i:d:^"\n")$(SECOND_BUILD)_$(i:f) : $(i:d:s/obj/slo/)$(i:b).obj )) >> $(MISC)$/$(TARGET).dpc
.ENDIF
.ELSE			# "$(HDBDEPNTARGET)$(OBJFILES)$(SLOFILES)$(DEPOBJFILES)$(RCFILES)"!=""
    @echo ttt: ppp > $(MISC)$/$(TARGET).dpc
.ENDIF			# "$(HDBDEPNTARGET)$(OBJFILES)$(SLOFILES)$(DEPOBJFILES)$(RCFILES)"!=""
.ENDIF			# "$(nodep)"!=""
.ELSE		# anything requiring dependencies


ALLDPC:
    @echo ------------------------------
    @echo No Dependencies
    @echo $(EMQ)# > $(MISC)$/$(TARGET).dpc

ALLDEP:
    @echo ------------------------------
    @echo No Dependencies

.ENDIF

